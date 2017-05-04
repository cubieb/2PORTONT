/** file vp_ac_rloop.c
 *
 *  This file contains the ac rloop primitive algorithm
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2010-10-04 14:26:37 -0500 (Mon, 04 Oct 2010) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_890_SERIES)
#if defined (VP890_INCLUDE_TESTLINE_CODE)

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
    COMMON_TEST_SETUP       = VP890_TESTLINE_GLB_STRT_STATE,
    AC_RLOOP_SETUP          = 5,
    AC_RLOOP_ADC_SETUP      = 10,
    AC_RLOOP_COLLECT_DATA   = 15,
    AC_RLOOP_GET_RESULT     = 20,
    AC_RLOOP_END            = VP890_TESTLINE_GLB_END_STATE,
    AC_RLOOP_QUIT           = VP890_TESTLINE_GLB_QUIT_STATE,
    AC_RLOOP_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpOpenVacTestStateTypes;

typedef enum
{
    AC_RLOOP_IMT,
    AC_RLOOP_VAB,
    AC_RLOOP_ILG,
    AC_RLOOP_MAX_ADC,
    VP_AC_RLOOP_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpAcRloopAdcOrderTypes;


static VpStatusType
VpAcRloopArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpAcRloopSetup(
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static bool
VpAcRloopAdcSetup(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    uint8 adcState,
    bool  axEnabled);

static void
VpAcRloopGetResult(
    Vp890DeviceObjectType *pDevObj,
    int16 *pTestState,
    uint8 *pAdcState);

/*------------------------------------------------------------------------------
 * Vp890TestOpenVdc
 * This functions implements the AC RLOOP Test primitive ...
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
Vp890TestAcRloop(
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

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint16 tick =  pDevObj->devProfileData.tickRate;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestAcRloop(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return VpAcRloopArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    switch (*pTestState) {

        case COMMON_TEST_SETUP:
            if ( pTestHeap->testArgs.acRloop.calMode) {

                uint32 swz = VP890_UNIT_CONVERT(VP890_AC_RLOOP_MAX_BAT_LVL,
                    VP890_UNIT_MV, VP890_UNIT_TRACK_SWZ);

                uint8 switchParam[VP890_REGULATOR_PARAM_LEN] = {0, 0, 0};
                /* ensure we are in fixed battery mode */
                switchParam[0] = pTestHeap->switchReg[0] | VP890_YRING_TRACK_DIS;
                /* ensure we have -25V floor for duration of the test */
                switchParam[1] = (pTestHeap->switchReg[1] & ~(VP890_FLOOR_VOLTAGE_MASK)) | 0x04; 
                /* force the ringing battery to be 75V */
                switchParam[2] = (0xe0 & pTestHeap->switchReg[2]) | (uint8)swz;

                VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestAcRloop() swy = 0x%02x%02x%02x , swz = 0x%lx",
                    switchParam[0], switchParam[1], switchParam[2], swz));

                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_REGULATOR_PARAM_WRT,
                    VP890_REGULATOR_PARAM_LEN, switchParam);
                VpMemCpy(pDevObj->swParamsCache, switchParam,
                    VP890_REGULATOR_PARAM_LEN);
            }
            Vp890CommonTestSetup(pLineCtx, deviceId);
            /* no break */

        case AC_RLOOP_SETUP:
            VpAcRloopSetup(pLineObj, pTestHeap, deviceId);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, AC_RLOOP_ADC_SETUP,
                MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME *2, tick) );
            break;

        case AC_RLOOP_ADC_SETUP:
            if ( TRUE == VpAcRloopAdcSetup(pLineCtx, pDevObj, *pAdcState,
                    pTestHeap->testArgs.acRloop.calMode) ) {

                Vp890SetTestStateAndTimer(pDevObj, pTestState,
                    AC_RLOOP_COLLECT_DATA,
                    MS_TO_TICKRATE(VP890_ADC_CHANGE_SETTLING_TIME, tick));
            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                Vp890SetTestStateAndTimer(pDevObj, pTestState, AC_RLOOP_QUIT,
                    NEXT_TICK);
            }
            break;

        case AC_RLOOP_COLLECT_DATA: {
            VpTestAcRLoopType *pAcRloopData = &(pTestHeap->testArgs).acRloop;
            Vp890StartPcmCollect(pLineCtx,
                (VP_PCM_OPERATION_RMS | VP_PCM_OPERATION_FREQ),
                AC_RLOOP_GET_RESULT, pAcRloopData->settlingTime,
                pAcRloopData->integrateTime);
            break;
        }

        case AC_RLOOP_GET_RESULT:
            VpAcRloopGetResult(pDevObj, pTestState, pAdcState);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;

        case AC_RLOOP_END:
            /*
             * We need to make sure the channel is placed back into the original
             * tracking mode before this test was run
             */
            if ( pTestHeap->testArgs.acRloop.calMode) {

                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_REGULATOR_PARAM_WRT,
                    VP890_REGULATOR_PARAM_LEN, pTestHeap->switchReg);
                VpMemCpy(pDevObj->swParamsCache, pTestHeap->switchReg,
                    VP890_REGULATOR_PARAM_LEN);
            }

            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestAcRloop(): complete"));
            break;

        case AC_RLOOP_QUIT:
            /* The test has eneded early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestAcRloop(ec:%i): quit", *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestAcRloop(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpAcRloopArgCheck
 * This functions is called by the Vp890TestAcRloop() function if
 * Vp890TestAcRloop() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the AC Rloop primitive are legal. The function also sets up
 * the AC Rloop state machine for the first state and initializes the results
 * from this fucntion to 0.
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
static VpStatusType
VpAcRloopArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTestAcRLoopType *pTestInput = (VpTestAcRLoopType*)pArgsUntyped;
    VpTestAcRLoopType *pAcRloopData = &(pTestInfo->pTestHeap->testArgs).acRloop;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpAcRloopArgCheck bailed ec:%i",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */
    *pAdcState = AC_RLOOP_IMT;

    pDevObj->testResults.result.acimt.vab = 0;
    pDevObj->testResults.result.acimt.imt = 0;
    pDevObj->testResults.result.acimt.ilg = 0;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* This test cannot run if the isolate relay is open*/
    if (VP_RELAY_RESET == pLineObj->relayState) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResLGArgCheck bailed: openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    /*
     * We are using the calMode member to allow the calling code to
     * indicate whether or not to use high gain mode when setting up the ADC
     * and applying the ring signal. The measured result is subsequently halved
     * before it is returned to the user.
     */
    pAcRloopData->calMode = pTestInput->calMode;
    pAcRloopData->integrateTime = pTestInput->integrateTime;
    pAcRloopData->settlingTime = pTestInput->settlingTime;
    pAcRloopData->freq = pTestInput->freq;
    pAcRloopData->vTestLevel = pTestInput->vTestLevel;
    pAcRloopData->vBias = pTestInput->vBias;
    pAcRloopData->tip = pTestInput->tip;
    if (pAcRloopData->tip > VP_TEST_TIP_RING) {
        pAcRloopData->tip = VP_TEST_TIP_RING;
    }

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, COMMON_TEST_SETUP, NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpOpenVdcSetup
 * This function is called by the Vp890TestAcRloop() state machine during
 * the AC_RLOOP_SETUP state.
 *
 * This function is used to configure the line for the ac rloop test.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *
 * Returns:
 * --
 * Result:
 *  Signal Generator A will be setup and slic will be put into
 *  balanced ringing. The loop super vision ringtrip threshold and current
 *  will be lowered in order not to cause a ring tirp during the test.
 *----------------------------------------------------------------------------*/
static void
VpAcRloopSetup(
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    VpTestAcRLoopType *pAcRloopData = &pTestHeap->testArgs.acRloop;
    uint8 slacState;
    uint16 freq = pAcRloopData->freq;
    int16 voltage = pAcRloopData->vTestLevel;
    int16 bias = pAcRloopData->vBias;
    uint8 lpSuper[VP890_LOOP_SUP_LEN] = {0x1B, 0x84, 0x10, 0x00};
    uint8 dcfeed[VP890_DC_FEED_LEN] = {0x12, 0x08};
    uint8 sigGenAB[VP890_SIGA_PARAMS_LEN]
        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    uint8 byte1, byte2;
    uint8 index = 1;

    /* convert input freq to proper number */
    freq = (uint16)(((10000ul * freq) / 3662ul) + 1);

    /* load bias into siggen */
    if (VP_TEST_TIP_RING != pAcRloopData->tip) {
        bias = voltage + 0x0835;
    }
    byte1 = (uint8)((bias >> 8) & 0xFF);
    byte2 = (uint8)(bias & 0x00FF);
    sigGenAB[index++] = byte1;
    sigGenAB[index++] = byte2;

    /* load freq into siggen */
    byte1 = (uint8)((freq >> 8) & 0xFF);
    byte2 = (uint8)(freq & 0x00FF);
    sigGenAB[index++] = byte1;
    sigGenAB[index++] = byte2;

    /* load amplitude into siggen */
    byte1 = (uint8)((voltage >> 8) & 0xFF);
    byte2 = (uint8)(voltage & 0x00FF);
    sigGenAB[index++] = byte1;
    sigGenAB[index++] = byte2;


    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SIGA_PARAMS_WRT, VP890_SIGA_PARAMS_LEN,
        sigGenAB);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_LOOP_SUP_WRT, VP890_LOOP_SUP_LEN, lpSuper);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DC_FEED_RD, VP890_DC_FEED_LEN, dcfeed);
    dcfeed[0] = (dcfeed[0] & 0x03) | 0x12;
    dcfeed[0] = (dcfeed[1] & 0xC0) | 0x08;
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DC_FEED_WRT, VP890_DC_FEED_LEN, dcfeed);

    if (VP_TEST_TIP_RING == pAcRloopData->tip) {
        slacState = VP890_SS_ACTIVATE_MASK | VP890_SS_BALANCED_RINGING;
    } else if (VP_TEST_TIP == pAcRloopData->tip) {
        slacState = VP890_SS_ACTIVATE_MASK | VP890_SS_UNBALANCED_RINGING_PR;
        pLineObj->icr3Values[0] = 0x24;
        pLineObj->icr3Values[1] = 0x20;
        pLineObj->icr3Values[2] = 0x00;
        pLineObj->icr3Values[3] = 0x00;
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);
    } else {
        slacState = VP890_SS_ACTIVATE_MASK | VP890_SS_UNBALANCED_RINGING;
    }

    /* enable ringing */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SYS_STATE_WRT,
        VP890_SYS_STATE_LEN, &slacState);
    pLineObj->slicValueCache = slacState;

    return;
}

/*------------------------------------------------------------------------------
 * VpAcRloopAdcSetup
 * This function is called by the Vp890TestAcRloop() state machine during
 * the AC_RLOOP_ADC_SETUP state.
 *
 * This function is used to call the Vp890AdcSetup() depending on the
 * current ac rloop adc state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pDevObj         - pointer to the device object.
 *  adcState        - current value of the adc setup state.
 *
 * Returns:
 *  FALSE if something goes wrong during the adc setup.
 *  TRUE if everything goes ok.
 *
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static bool
VpAcRloopAdcSetup(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    uint8 adcState,
    bool  axEnabled)
{
    bool adcFlag;
    switch (adcState) {
        case AC_RLOOP_VAB:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_V, axEnabled);
            break;
        case AC_RLOOP_IMT:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_I, axEnabled);
            break;
        case AC_RLOOP_ILG:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_LONGITUDINAL_DC_I, axEnabled);
            break;
        default:
            return FALSE;
    }

    return adcFlag;
}

/*------------------------------------------------------------------------------
 * VpAcRloopGetResult
 * This function is called by the Vp890TestAcRloop() state machine during
 * the AC_RLOOP_GET_RESULT state.
 *
 * This function is used to collect the result form the PCM integrator. And load
 * the result into the results structure.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  pTestState      - pointer to the device current test state.
 *  pErrorCode      - pointer to the device results errorCode member
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *
 * Returns:
 *  --
 *
 * Result:
 *  Two possible results of running this function are:
 *  1) not all adc connections have been made so advance the pAdcState and set
 *     the testState to AC_RLOOP_AD_SETUP.
 *  3) all adc connections have been made jump to AC_RLOOP_END.
 *----------------------------------------------------------------------------*/
static void
VpAcRloopGetResult(
    Vp890DeviceObjectType *pDevObj,
    int16 *pTestState,
    uint8 *pAdcState)
{
    switch (*pAdcState) {
        case AC_RLOOP_IMT:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, 0,
                VP890_METALLIC_DC_I, &pDevObj->testResults.result.acimt.imt);
            VP_TEST(None, NULL, ("VpAcRloopGetResult(imt:%i)", \
                pDevObj->testResults.result.acimt.imt));
            break;
        case AC_RLOOP_ILG:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, 0,
                VP890_LONGITUDINAL_DC_I, &pDevObj->testResults.result.acimt.ilg);
            VP_TEST(None, NULL, ("VpAcRloopGetResult(ilg:%i)", \
                pDevObj->testResults.result.acimt.ilg));
            break;
        default: /* AC_RLOOP_VAB */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, 0,
                VP890_METALLIC_DC_V, &pDevObj->testResults.result.acimt.vab);
            VP_TEST(None, NULL, ("VpAcRloopGetResult(vab:%i)", \
                pDevObj->testResults.result.acimt.vab));

            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_FREQ, FALSE, FALSE, 0,
                VP890_METALLIC_DC_V, &pDevObj->testResults.result.acimt.freq);
            VP_TEST(None, NULL, ("VpAcRloopGetResult(freq:%li)", \
                pDevObj->testResults.result.acimt.freq));

            break;
    }

    /* move to the next ADC measurment state */
    if (AC_RLOOP_MAX_ADC > ++(*pAdcState)) {
        *pTestState = AC_RLOOP_ADC_SETUP;
    } else {
        *pTestState = AC_RLOOP_END;
    }
    return;
}

#endif /* VP890_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_890_SERIES */
