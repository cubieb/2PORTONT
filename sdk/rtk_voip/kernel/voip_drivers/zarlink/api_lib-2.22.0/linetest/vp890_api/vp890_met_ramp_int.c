/*
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2011-03-23 17:18:34 -0500 (Wed, 23 Mar 2011) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_890_SERIES)
#if defined (VP890_INCLUDE_TESTLINE_CODE)
#if (VP890_INCLUDE_LINE_TEST_PACKAGE == VP890_LINE_TEST_PROFESSIONAL)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp890_api.h"
#include "vp890_api_int.h"
#include "vp890_testline_int.h"

typedef enum
{
    COMMON_TEST_SETUP   = VP890_TESTLINE_GLB_STRT_STATE,
    INIT_RAMP_SETUP     = 5,
    INIT_RAMP_CHARGE    = 10,
    INIT_RAMP_START     = 15,
    INIT_RAMP_END       = VP890_TESTLINE_GLB_END_STATE,
    INIT_RAMP_QUIT      = VP890_TESTLINE_GLB_QUIT_STATE,
    INIT_RAMP_TEST_STATE_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpInitRampTestStateTypes;


static VpStatusType
VpInitMetRampArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId);

static void
VpInitMetRampSetup(
    Vp890LineObjectType *pLineObj,
    VpDeviceIdType deviceId);

EXTERN VpStatusType
Vp890TestInitMetRamp(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    VpTestRampInitType *pRampInput = &(pTestHeap->testArgs).rampInit;

    int16 *pTestState = &pTestInfo->testState;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint8 chanId = pLineObj->channelId;

    if (FALSE == callback) {
        return VpInitMetRampArgCheck(pLineCtx, pArgsUntyped, handle, testId);
    }

    switch (*pTestState) {
        case COMMON_TEST_SETUP:
            Vp890CommonTestSetup(pLineCtx, deviceId);
            /* no break */

        case INIT_RAMP_SETUP:
            /* Once the common test setup is complete move on */
            VpInitMetRampSetup(pLineObj, deviceId);
            /* no break */

        case INIT_RAMP_CHARGE: {
            /*
             * Due to the fact that the VoicePort device's internal ramp
             * generator has "memory" of where it used to be, this step will
             * attempt to internally set SigGenA to match the DC-Feed
             * value after the common test setup (45V). No real check of the
             * feed value is being performed so if the feed value changes in the
             * common test setup then the SigGenA values need to also be changed.
             */
            int16 sigGenAAmp, bias;
            uint8 byte1, byte2;
            uint8 sigGenAB[VP890_SIGA_PARAMS_LEN] =
                {0x03, 0x00, 0x00, 0x7F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            if (0 <= pRampInput->dcVstart) {
                pRampInput->dcVstart *= -1;
            }

            bias = (int16)VP890_UNIT_CONVERT(pRampInput->bias,
                VP890_UNIT_ADC_VAB, VP890_UNIT_DAC_RING);
            sigGenAAmp = (int16)VP890_UNIT_CONVERT(pRampInput->dcVstart,
                VP890_UNIT_ADC_VAB, VP890_UNIT_DAC_RING);

            /* load calibrated ampl into siggen A*/
            if (pLineObj->slicValueCache & VP890_SS_POLARITY_MASK) {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(polrev)"));
                bias += (pDevObj->vp890SysCalData.sigGenAError[chanId][0] -
                    pDevObj->vp890SysCalData.vocOffset[chanId][VP890_REV_POLARITY]) * 16 / 10;
            } else {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(normal)"));
                bias -= (pDevObj->vp890SysCalData.sigGenAError[chanId][0] -
                    pDevObj->vp890SysCalData.vocOffset[chanId][VP890_NORM_POLARITY]) * 16 / 10;
            }

            byte1 = (uint8)((bias >> 8) & 0xFF);
            byte2 = (uint8)(bias & 0x00FF);
            sigGenAB[1] = byte1;
            sigGenAB[2] = byte2;

            /* load ampl into siggen A*/
            byte1 = (uint8)((sigGenAAmp >> 8) & 0xFF);
            byte2 = (uint8)(sigGenAAmp & 0x00FF);
            sigGenAB[5] = byte1;
            sigGenAB[6] = byte2;

            /* Program SigGenAB (must program siggen before enabling it)*/
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SIGA_PARAMS_WRT,
                VP890_SIGA_PARAMS_LEN, sigGenAB);

            /* Enable SigGenA*/
            if (pLineObj->sigGenCtrl[0] != VP890_GENA_EN) {
                pLineObj->sigGenCtrl[0] = VP890_GENA_EN;
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_GEN_CTRL_WRT,
                    VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }

            Vp890SetTestStateAndTimer(pDevObj, pTestState, INIT_RAMP_START,
                MS_TO_TICKRATE(10, tick));

            break;
        }

        case INIT_RAMP_START: {
            /*
             * This step is going to swap out the DtoA with SigGenAB.
             * The line will now be driven by Signal GenAB in the ringing state.
             */
            int16 startAmpDAC, endAmpDAC, sigGenAAmp, ampADC, sigGenAFreq, bias;
            uint8 byte1, byte2;
            uint8 slacState;
            uint8 sigGenAB[VP890_SIGA_PARAMS_LEN] =
                {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            if (0 <= pRampInput->dcVstart) {
                pRampInput->dcVstart *= -1;
            }

            bias = (int16)VP890_UNIT_CONVERT(pRampInput->bias,
                VP890_UNIT_ADC_VAB, VP890_UNIT_DAC_RING);
            startAmpDAC = (int16)VP890_UNIT_CONVERT(pRampInput->dcVstart,
                VP890_UNIT_ADC_VAB, VP890_UNIT_DAC_RING);
            endAmpDAC = (int16)VP890_UNIT_CONVERT(pRampInput->dcVend,
                VP890_UNIT_ADC_VAB, VP890_UNIT_DAC_RING);
            sigGenAAmp = endAmpDAC - startAmpDAC;

            ampADC = pRampInput->dcVend - pRampInput->dcVstart;

            /* prevent divide by 0 error */
            if (0 == ampADC) {
                ampADC = 1;
            }
            sigGenAFreq = VP890_SLOPE_TO_FREQ(ampADC,pRampInput->vRate);

            /* load calibrated ampl into siggen A*/
            if (pLineObj->slicValueCache & VP890_SS_POLARITY_MASK) {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(polrev)"));
                bias += (pDevObj->vp890SysCalData.sigGenAError[chanId][0] -
                    pDevObj->vp890SysCalData.vocOffset[chanId][VP890_REV_POLARITY]) * 16 / 10;
            } else {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(normal)"));
                bias -= (pDevObj->vp890SysCalData.sigGenAError[chanId][0] -
                    pDevObj->vp890SysCalData.vocOffset[chanId][VP890_NORM_POLARITY]) * 16 / 10;
            }

            byte1 = (uint8)((bias >> 8) & 0xFF);
            byte2 = (uint8)(bias & 0x00FF);
            sigGenAB[1] = byte1;
            sigGenAB[2] = byte2;

            /* ensure value is non-negative integer */
            sigGenAFreq = (0 == sigGenAFreq) ? 1 : ABS(sigGenAFreq);
            /* load freq into siggen A*/
            byte1 = (uint8)((sigGenAFreq >> 8) & 0xFF);
            byte2 = (uint8)(sigGenAFreq & 0x00FF);
            sigGenAB[3] = byte1;
            sigGenAB[4] = byte2;

            /* load ampl into siggen A*/
            byte1 = (uint8)((sigGenAAmp >> 8) & 0xFF);
            byte2 = (uint8)(sigGenAAmp & 0x00FF);
            sigGenAB[5] = byte1;
            sigGenAB[6] = byte2;

            slacState = pLineObj->slicValueCache;

            /* prepare slac state */
            if (VP_TEST_TIP_RING == pRampInput->tip) {
                slacState = (slacState & ~VP890_SS_LINE_FEED_MASK) |
                    VP890_SS_BALANCED_RINGING;
            } else {
                if (VP_TEST_TIP == pRampInput->tip) {
                    pLineObj->icr3Values[0] = 0x24;
                    pLineObj->icr3Values[1] = 0x20;
                    pLineObj->icr3Values[2] = 0x00;
                    pLineObj->icr3Values[3] = 0x00;
                    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_ICR3_WRT,
                        VP890_ICR3_LEN, pLineObj->icr3Values);
                    slacState =  VP890_SS_ACTIVATE_MASK | VP890_SS_UNBALANCED_RINGING_PR;
                } else {
                    slacState =  VP890_SS_ACTIVATE_MASK | VP890_SS_UNBALANCED_RINGING;
                }
            }

            /* Program SigGenAB */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SIGA_PARAMS_WRT,
                VP890_SIGA_PARAMS_LEN, sigGenAB);

            /* Go to balanced ringing with correct polarity*/
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SYS_STATE_WRT,
                VP890_SYS_STATE_LEN, &slacState);
            pLineObj->slicValueCache = slacState;

            /* wait for ramp to complete */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, INIT_RAMP_END,
                MS_TO_TICKRATE((10 + 1366/sigGenAFreq), tick));

            break;
        }

        case INIT_RAMP_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestInitMetRamp(): complete"));
            break;

        case INIT_RAMP_QUIT:
            /* The test has ended early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestInitMetRamp(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            /* The test has entered an unsuppoted state */
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            pDevObj->testResults.result.calFailed = TRUE;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestInitMetRamp(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpInitMetRampArgCheck
 * This functions is called by the Vp890TestInitMetRamp() function if
 * Vp890TestInitMetRamp() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the Open VDC primitive are legal. The function also sets up
 * the Init Met Ramp state machine for the first state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  handle          - unique test handle
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
static VpStatusType
VpInitMetRampArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    int16 *pTestState = &pTestInfo->testState;
    VpTestRampInitType *pTestInput = (VpTestRampInitType*)pArgsUntyped;
    VpTestRampInitType *pRampInput = &(pTestInfo->pTestHeap->testArgs).rampInit;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpInitMetRampArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /*
     * ensure that requested start v is not greater than |154.368|
     */
    if (VP890_UNIT_DAC_RING < ABS(pTestInput->dcVstart)) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResHGArgCheck(dcVs:%i>%li): bailed",
            ABS(pTestInput->dcVstart), VP890_UNIT_DAC_RING));
        return VP_STATUS_INVALID_ARG;
    }

    /*
     * ensure that requested final v is not greater than |154.368|
     */
    if (VP890_UNIT_DAC_RING < ABS(pTestInput->dcVend)) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResHGArgCheck(dcVe:%i>%li): bailed",
            ABS(pTestInput->dcVend), VP890_UNIT_DAC_RING));
        return VP_STATUS_INVALID_ARG;
    }

    pRampInput->dcVstart = pTestInput->dcVstart;
    pRampInput->dcVend = pTestInput->dcVend;
    pRampInput->vRate = pTestInput->vRate;
    pRampInput->bias = pTestInput->bias;
    pRampInput->tip = pTestInput->tip;
    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

static void
VpInitMetRampSetup(
    Vp890LineObjectType *pLineObj,
    VpDeviceIdType deviceId)
{
    uint8 workAroundState = VP890_SS_IDLE;
    uint8 ssCfg = ( VP890_ACFS_MASK | VP890_ZXR_DIS | VP890_AUTO_SSC_DIS |
        VP890_AUTO_BAT_SHUTDOWN_DIS);

    pLineObj->slicValueCache |= VP890_SS_ACTIVATE_MASK;

    /* 
     * Work around:
     * Prior to this test running, some event has caused a false
     * DC value to be injected into the DAC. This was only seen
     * in wideband. The issue can be fixed if the codec is momentarily
     * disabled and then re-enabled. We can only disable the 
     * codec bit if we go to idle also
    */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SLIC_STATE_WRT,
        VP890_SLIC_STATE_LEN, &workAroundState);

    /* State Change*/
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SLIC_STATE_WRT,
        VP890_SLIC_STATE_LEN, &pLineObj->slicValueCache);

    /* Disable automatic state changes */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SS_CONFIG_WRT,
        VP890_SS_CONFIG_LEN, &ssCfg);

    return;
}

#endif
#endif /* VP890_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_890_SERIES */
