/** file vp_loop_conditions.c
 *
 *  This file contains the dc rloop primitive algorithm
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 10559 $
 * $LastChangedDate: 2012-10-29 15:30:05 -0500 (Mon, 29 Oct 2012) $
 */


#include "vp_api_cfg.h"

#if defined(VP_CC_880_SERIES) && defined(VP880_INCLUDE_TESTLINE_CODE)
#if (VP880_INCLUDE_LINE_TEST_PACKAGE == VP880_LINE_TEST_PROFESSIONAL)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_api_testline_int.h"

typedef enum
{
    LOOP_COND_SETUP          = VP880_TESTLINE_GLB_STRT_STATE,
    LOOP_COND_AD_SETUP       = 15,
    LOOP_COND_COLLECT_DATA   = 20,
    LOOP_COND_GET_RESULT     = 25,
    LOOP_COND_END            = VP880_TESTLINE_GLB_END_STATE,
    LOOP_COND_QUIT           = VP880_TESTLINE_GLB_QUIT_STATE,
    LOOP_COND_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpOpenVdcTestStateTypes;

static VpStatusType
VpLoopCondArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpLoopCondSetup(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static bool
VpLoopCondAdcSetup(
    VpLineCtxType *pLineCtx,
    VpLoopCondTestType loopCond);

static void
VpLoopCondGetResult(
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId,
    int16 *pTestState,
    VpTestStatusType *pErrorCode,
    VpLoopCondTestType loopCond);

static void
VpLoopCondFillLimitValue(
    VpLineCtxType *pLineCtx,
    VpLoopCondTestType loopCond);

static uint8
VpLoopCondGetBatSetting(
    VpLineCtxType *pLineCtx,
    VpLoopCondTestType loopCond);

/*------------------------------------------------------------------------------
 * Vp880TestGetLoopCond
 * This functions implements the LOOP CONDITIONS Test primitive ...
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  pArgsUntyped- pointer to the test specific inputs.
 *  handle      - unique test handle
 *  callbak     - indicates if this function was called via a callback
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp880TestGetLoopCond(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    VpTestLoopCondType *pLoopCondData = &(pTestHeap->testArgs).loopCond;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint8 adc;
    bool adcErr;

    if (FALSE == callback) {
        return VpLoopCondArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestGetLoopCond(ts:%i)", *pTestState));
    switch (*pTestState) {

        case LOOP_COND_SETUP: {
            VpLoopCondSetup(pLineObj, pTestHeap, deviceId);

            if (pTestInfo->nonIntrusiveTest == FALSE) {
                VpSetTestStateAndTimer(pDevObj, pTestState, LOOP_COND_AD_SETUP,
                    MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME, tick));
            } else {
                VpSetTestStateAndTimer(pDevObj, pTestState, LOOP_COND_AD_SETUP, NEXT_TICK);
            }
            break;
        }

        case LOOP_COND_AD_SETUP:
            if ( TRUE == VpLoopCondAdcSetup(pLineCtx, pLoopCondData->loopCond) ) {
                if (pTestInfo->nonIntrusiveTest == FALSE) {
                    VpSetTestStateAndTimer(pDevObj, pTestState, LOOP_COND_COLLECT_DATA,
                        MS_TO_TICKRATE(VP880_ADC_CHANGE_SETTLING_TIME, tick));
                } else {
                    VpSetTestStateAndTimer(pDevObj, pTestState, LOOP_COND_COLLECT_DATA,
                        NEXT_TICK);
                }
            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                VpSetTestStateAndTimer(pDevObj, pTestState, LOOP_COND_QUIT,
                    NEXT_TICK);
            }
            break;

        case LOOP_COND_COLLECT_DATA:
            VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                LOOP_COND_GET_RESULT, pLoopCondData->settlingTime,
                pLoopCondData->integrateTime);
            break;

        case LOOP_COND_GET_RESULT:
            VpLoopCondGetResult(pDevObj, chanId, pTestState, pErrorCode, pLoopCondData->loopCond);

            /* Check the ADC configuration, if not as expected -> issue an error */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN, &adc);
            adc = adc & 0x0F;
            switch (pLoopCondData->loopCond) {
                case VP_LOOP_COND_TEST_VSAB:
                    adcErr = (adc != VP880_METALLIC_DC_V) ? TRUE : FALSE;
                    break;
                case VP_LOOP_COND_TEST_VSAG:
                    adcErr = (adc != VP880_TIP_TO_GND_V) ? TRUE : FALSE;
                    break;
                case VP_LOOP_COND_TEST_VSBG:
                    adcErr = (adc != VP880_RING_TO_GND_V) ? TRUE : FALSE;
                    break;
                case VP_LOOP_COND_TEST_IMT:
                    adcErr = (adc != VP880_METALLIC_DC_I) ? TRUE : FALSE;
                    break;
                case VP_LOOP_COND_TEST_ILG:
                    adcErr = (adc != VP880_LONGITUDINAL_DC_I) ? TRUE : FALSE;
                    break;
                default:
                    adcErr = FALSE;
            }
            
            if (adcErr == TRUE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestGetLoopCond(): invalid ADC setting"));
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                *pTestState = LOOP_COND_QUIT;
            }

            VpSetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;

        case LOOP_COND_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestGetLoopCond(): complete"));
            break;

        case LOOP_COND_QUIT:
            /* The test has eneded early due to the current errorCode */
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestGetLoopCond(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestGetLoopCond(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpLoopCondArgCheck
 * This functions is called by the Vp880TestGetLoopCond() function if
 * Vp880TestGetLoopCond() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the Get Loop Conditions primitive are legal. The function also
 * sets up the primitives state machine for the first state and initializes
 * the results from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  handle          - unique test handle
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *
 * Returns:
 * VpStatusType
 *
 * Result:
 *  This function initializes all relevent result values to 0. Stores the
 *  TestId and handle into the device Object as well as modifes the current
 *  ADC state to the first measurement.
 *
 *----------------------------------------------------------------------------*/
VpStatusType
VpLoopCondArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTestLoopCondType *pTestInput = (VpTestLoopCondType*)pArgsUntyped;
    VpTestLoopCondType *pLoopCondData = &(pTestInfo->pTestHeap->testArgs).loopCond;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpLoopCondArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */

    pDevObj->testResults.result.loopCond.data = VP_INT16_MAX;
    pDevObj->testResults.result.loopCond.condition = pTestInput->loopCond;
    pDevObj->testResults.result.loopCond.calibrated = TRUE;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* This test cannot run if the isolate relay is open (except ReadLoopCondition) */
    if (((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
         (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) &&
        (VP_RELAY_RESET == pLineObj->relayState) &&
        (pTestInfo->nonIntrusiveTest == FALSE)) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpLoopCondArgCheck(): bailed openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    if ((pTestInput->loopCond != VP_LOOP_COND_TEST_VSAB) &&
        (pTestInput->loopCond != VP_LOOP_COND_TEST_VSAG) &&
        (pTestInput->loopCond != VP_LOOP_COND_TEST_VSBG) &&
        (pTestInput->loopCond != VP_LOOP_COND_TEST_IMT)  &&
        (pTestInput->loopCond != VP_LOOP_COND_TEST_ILG)  &&
        (pTestInput->loopCond != VP_LOOP_COND_TEST_BAT1) &&
        (pTestInput->loopCond != VP_LOOP_COND_TEST_BAT2) &&
        (pTestInput->loopCond != VP_LOOP_COND_TEST_BAT3) ) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpLoopCondArgCheck(): bailed invalid loopCond"));
        return VP_STATUS_INVALID_ARG;
    }

    /*
     * if this is a tracker part ensure that a proper battery is requested
     * if any battery is requested
     */
    pLoopCondData->calMode = pTestInput->calMode;
    pLoopCondData->integrateTime = pTestInput->integrateTime;
    pLoopCondData->settlingTime = pTestInput->settlingTime;
    pLoopCondData->loopCond = pTestInput->loopCond;

    /* Fill in the limit value result */
    VpLoopCondFillLimitValue(pLineCtx, pLoopCondData->loopCond);

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    VpSetTestStateAndTimer(pDevObj, pTestState, LOOP_COND_SETUP, NEXT_TICK);

    /* if the battery calibration factors are null, no calibration will apply */
    if ((pDevObj->vp880SysCalData.swyOffset[0] == 0)
        && (pDevObj->vp880SysCalData.swyOffset[1] == 0)
        && (pTestInput->loopCond == VP_LOOP_COND_TEST_BAT1)) {
        pDevObj->testResults.result.loopCond.calibrated = FALSE;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpLoopCondSetup
 * This function is called by the Vp880TestGetLoopCond() state machine during
 * the LOOP_COND_SETUP state.
 *
 * This function is used to configure the line for the condition test.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *
 * Returns:
 * --
 * Result:
 * --
 *----------------------------------------------------------------------------*/
static void
VpLoopCondSetup(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    uint8 disn = 0x00;
    uint8 opCond = ( VP880_RXPATH_DIS | VP880_HIGH_PASS_DIS );
    uint8 opFunction = VP880_LINEAR_CODEC;
    uint8 adc;
    uint8 ssCfg = ( VP880_ZXR_DIS | VP880_AUTO_SSC_DIS);

    uint8 mpiBuffer[VP880_CONV_CFG_LEN+1 +
                     VP880_SLIC_STATE_LEN+1 +
                     VP880_DISN_LEN+1 +
                     VP880_OP_COND_LEN+1 +
                     VP880_OP_FUNC_LEN+1 +
                     VP880_ICR2_LEN+1 +
                     VP880_ICR3_LEN+1 +
                     VP880_ICR4_LEN+1 +
                     VP880_SS_CONFIG_LEN+1];
    uint8 mpiIndex = 0;

    pLineObj->icr2Values[0] = 0x0C;
    pLineObj->icr2Values[1] = 0x0C;
    if ((pLineObj->lineState.currentState == VP_LINE_DISCONNECT) &&
        ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
        (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP) ||
        (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP))) {
        pLineObj->icr2Values[2] &= 0x0C;
        pLineObj->icr2Values[2] |= 0x20;
        pLineObj->icr2Values[3] &= 0x0C;
    } else {
        pLineObj->icr2Values[2] &= 0x0C;
        pLineObj->icr2Values[3] &= 0x0C;
    }

    pLineObj->icr3Values[0] |= 0x21;
    pLineObj->icr3Values[1] |= 0x21;

    pLineObj->icr4Values[0] |= 0x01;
    pLineObj->icr4Values[1] |= 0x01;

    /* Get Slac State */
    pLineObj->slicValueCache |= VP880_SS_ACTIVATE_MASK;

    /* Get the adc reg setting */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN, &adc);
    adc &= ~VP880_CONV_CONNECT_BITS;
    adc |= VP880_NO_CONNECT;


    /*
     * FORCE ADC to metalic AC:
     * THIS is a required workaround. If the device is in idle and
     * the ADC is forced on in ICR4, the data from the ADC will be
     * railed. However, if the converter configuration register is
     * set to NO_CONNECT first, then this issue does not exist.
    */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_CONV_CFG_WRT,
        VP880_CONV_CFG_LEN, &adc);


    /* Remove disn */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DISN_WRT,
        VP880_DISN_LEN, &disn);

    /* Disable hi-pass filter and cutoff receive path */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_COND_WRT,
        VP880_OP_COND_LEN, &opCond);
    pLineObj->opCond[0] = opCond;

    /* Set to read linear data */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_FUNC_WRT,
        VP880_OP_FUNC_LEN, &opFunction);

    /* VOC on, A/B sns on, incr ADC rg */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
        VP880_ICR2_LEN, pLineObj->icr2Values);

    /* Force line control circuits on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR3_WRT,
        VP880_ICR3_LEN, pLineObj->icr3Values);

    /* Force ADC on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR4_WRT,
        VP880_ICR4_LEN, pLineObj->icr4Values);

    /* Prevent ADC from changing state setup during testing */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SS_CONFIG_WRT,
        VP880_SS_CONFIG_LEN, &ssCfg);

    /* State Change*/
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SLIC_STATE_WRT,
        VP880_SLIC_STATE_LEN, &pLineObj->slicValueCache);


    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * VpAcRloopAdcSetup
 * This function is called by the Vp880TestGetLoopCond() state machine during
 * the LOOP_COND_AD_SETUP state.
 *
 * This function is used to call the Vp880AdcSetup() depending on the
 * current requested loop condition state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  loopCond        - which codition to measure.
 *
 * Returns:
 *  FALSE if something goes wrong during the adc setup.
 *  TRUE if everything goes ok.
 *
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static bool
VpLoopCondAdcSetup(
    VpLineCtxType *pLineCtx,
    VpLoopCondTestType loopCond)
{
    bool adcFlag;
    switch (loopCond) {
        case VP_LOOP_COND_TEST_VSAB:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_V, FALSE);
            break;
        case VP_LOOP_COND_TEST_VSAG:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_TIP_TO_GND_V, FALSE);
            break;
        case VP_LOOP_COND_TEST_VSBG:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_RING_TO_GND_V, FALSE);
            break;
        case VP_LOOP_COND_TEST_IMT:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_I, FALSE);
            break;
        case VP_LOOP_COND_TEST_ILG:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_LONGITUDINAL_DC_I, FALSE);
            break;
        case VP_LOOP_COND_TEST_BAT1:
        case VP_LOOP_COND_TEST_BAT2:
        case VP_LOOP_COND_TEST_BAT3:
            adcFlag = Vp880AdcSetup(pLineCtx,
                VpLoopCondGetBatSetting(pLineCtx, loopCond), FALSE);
            break;
        default:
            return FALSE;
    }

    return adcFlag;
}

/*------------------------------------------------------------------------------
 * VpLoopCondGetResult
 * This function is called by the Vp880TestGetLoopCond() state machine during
 * the LOOP_COND_GET_RESULT state.
 *
 * This function is used to collect the result from the PCM integrator. And load
 * the result into the results structure.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  chanId          - current channel number
 *  pTestState      - pointer to the device current test state.
 *  pErrorCode      - pointer to the device results errorCode member
 *  loopCond        - currently measured loop condition
 *
 * Returns:
 *  --
 *
 * Result:
 *----------------------------------------------------------------------------*/
static void
VpLoopCondGetResult(
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId,
    int16 *pTestState,
    VpTestStatusType *pErrorCode,
    VpLoopCondTestType loopCond)
{
    int16 *pResult;
    uint8 aToDRoute = VP880_NO_CONNECT;
    bool removeOffset = FALSE;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    bool LI;

    *pTestState = LOOP_COND_END;
    pResult = &pDevObj->testResults.result.loopCond.data;
    LI = pTestHeap->dcFeed[0] & VP880_LONG_IMP_MASK;    /* usefull to compute ILG */

    switch (loopCond) {
        case VP_LOOP_COND_TEST_VSAB:
            aToDRoute = VP880_METALLIC_DC_V;
            removeOffset = TRUE;
            break;
        case VP_LOOP_COND_TEST_VSAG:
            aToDRoute = VP880_TIP_TO_GND_V;
            removeOffset = TRUE;
            break;
        case VP_LOOP_COND_TEST_VSBG:
            aToDRoute = VP880_RING_TO_GND_V;
            removeOffset = TRUE;
            break;
        case VP_LOOP_COND_TEST_IMT:
            aToDRoute = VP880_METALLIC_DC_I;
            removeOffset = TRUE;
            break;
        case VP_LOOP_COND_TEST_ILG:
            aToDRoute = VP880_LONGITUDINAL_DC_I;
            removeOffset = TRUE;
            break;
        case VP_LOOP_COND_TEST_BAT1:
            aToDRoute = VP880_SWITCHER_Y;
            removeOffset = TRUE;
            break;
        case VP_LOOP_COND_TEST_BAT2:
            aToDRoute = VP880_SWITCHER_Z;
            removeOffset = TRUE;
            break;
        case VP_LOOP_COND_TEST_BAT3:
            aToDRoute = VP880_XBR;
            removeOffset = TRUE;
            break;
        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            *pTestState = LOOP_COND_QUIT;
            return;
    }

    VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, removeOffset, chanId,
        aToDRoute, pResult);

    if ((loopCond == VP_LOOP_COND_TEST_ILG) && LI) {
        /* the ILG scale go from 1.3uA to 2.6uA */
        if (*pResult > (VP_INT16_MAX / 2)) {
            *pResult = VP_INT16_MAX;
            VP_WARNING(None, NULL, ("VpLoopCondGetResult(): ILG value = VP_INT16_MAX"));
        } else {
            *pResult *= 2;
        }
    }

    return;
}

static void
VpLoopCondFillLimitValue(
    VpLineCtxType *pLineCtx,
    VpLoopCondTestType loopCond)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    switch (loopCond) {
        case VP_LOOP_COND_TEST_IMT: {
            int32 targetIla;

            /* Retrieve the DC Profile target ILA value */
            targetIla = (pLineObj->calLineData.dcFeedRef[1] & 0x1F);

            /* Convert to PCM scale */
            targetIla *= ((VP880_ILA_SCALE_2MA / 2) * 10); /* 5682; */
            targetIla += (VP880_ILA_SCALE_18MA * 10L); /* Base of 18ma 102272; */
            targetIla /= 10;

            pDevObj->testResults.result.loopCond.limit = (int16)targetIla;

            break;
        }
        default: {
            break;
        }
    }
}

static uint8
VpLoopCondGetBatSetting(
    VpLineCtxType *pLineCtx,
    VpLoopCondTestType loopCond)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 adcval;

    if (VP880_IS_ABS & pDevObj->stateInt) {
        uint8 adcMap[3];

        /*
         * Determine battery order for normal configuration
         * BAT1= Low, BAT2= High, BAT3= Med
         */
        if (pDevObj->swParamsCache[0] & VP880_BSI_LX_MY_HZ) {
            adcMap[0] = VP880_XBR;
            adcMap[1] = VP880_SWITCHER_Z;
            adcMap[2] = VP880_SWITCHER_Y;
        } else if (pDevObj->swParamsCache[0] & VP880_BSI_LY_MZ_HX) {
            adcMap[0] = VP880_SWITCHER_Y;
            adcMap[1] = VP880_XBR;
            adcMap[2] = VP880_SWITCHER_Z;
        } else {
            adcMap[0] = VP880_SWITCHER_Y;
            adcMap[1] = VP880_SWITCHER_Z;
            adcMap[2] = VP880_XBR;
        }

        /* determine the correct adc setting based on requested battery */
        if (VP_LOOP_COND_TEST_BAT1 == loopCond) {
            adcval = adcMap[0];
        } else if (VP_LOOP_COND_TEST_BAT2 == loopCond) {
            adcval = adcMap[1];
        } else {
            adcval = adcMap[2];
        }

    } else {
        /* don't recheck the channel number just return the requested info*/
        if (VP_LOOP_COND_TEST_BAT1 == loopCond) {
            adcval = VP880_SWITCHER_Y;
        } else if (VP_LOOP_COND_TEST_BAT2 == loopCond) {
            adcval = VP880_SWITCHER_Z;
        } else {
            adcval = VP880_XBR;
        }
    }

    return adcval;
}

#endif /* VP880_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_880_SERIES */





