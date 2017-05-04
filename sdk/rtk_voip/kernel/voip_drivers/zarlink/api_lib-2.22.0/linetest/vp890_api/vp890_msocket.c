/** file vp890_msocket.c
 *
 * This file contains the master socket (capacitance + resistance) primitive algorithm
 *
 * Copyright (c) 2009, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5522 $
 * $LastChangedDate: 2009-08-31 10:31:31 -0500 (Mon, 31 Aug 2009) $
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

#define VP890_MSOCKET_SETTLING_TIME     10      /* 10ms */
#define FEED_CHANGE_SETTLINGE_ERR       2       /* +/- 2 adu err*/
#define MAX_ITERATION                   100     /* Max measurement iterations */

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif
typedef enum
{
    COMMOM_TEST_SETUP       = VP890_TESTLINE_GLB_STRT_STATE,
    MSOCKET_SETUP           = 5,
    MSOCKET_CHANGE_FEED     = 10,
    MSOCKET_ADC_SETUP       = 15,
    MSOCKET_COLLECT_DATA    = 20,
    MSOCKET_GET_RESULT      = 25,
    MSOCKET_END             = VP890_TESTLINE_GLB_END_STATE,
    MSOCKET_QUIT            = VP890_TESTLINE_GLB_QUIT_STATE,
    MSOCKET_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpMSocketTestStateTypes;

typedef enum
{
    MSOCKET_IMT_NORMAL_FEED,
    MSOCKET_VAG_NORMAL_FEED,
    MSOCKET_VBG_NORMAL_FEED,
    MSOCKET_VAG_REVERSE_FEED_1,
    MSOCKET_VBG_REVERSE_FEED_1,
    MSOCKET_IMT_REVERSE_FEED_1,
    MSOCKET_VAG_REVERSE_FEED_2,
    MSOCKET_VBG_REVERSE_FEED_2,
    MSOCKET_IMT_REVERSE_FEED_2,
    MSOCKET_VAG_REVERSE_FEED_3,
    MSOCKET_VBG_REVERSE_FEED_3,
    MSOCKET_IMT_REVERSE_FEED_3,
    MSOCKET_MAX_ADC,
    MSOCKET_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpMSocketAdcOrderTypes;


static VpStatusType
VpMSocketArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpMSocketSetup(
    Vp890DeviceObjectType *pDevObj,
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap);

static uint16
VpMSocketChangeFeed(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    uint8 adcState);

static void
VpMSocketAdcSetup(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static void
VpMSocketCollectData(
    VpLineCtxType *pLineCtx,
    Vp890TestHeapType *pTestHeap,
    uint8 adcState);

static void
VpMSocketGetResult(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    int16 *pTestState,
    uint8 *pAdcState,
    uint8 chanId);

static bool
VpMSocketStoreResult(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState);

/*------------------------------------------------------------------------------
 * Vp890TestMSocket()
 * This functions implements the master socket (capacitance + resistance) primitive
 *
 * Parameters:
 *  pLineCtx     - pointer to the line context
 *  pArgsUntyped - pointer to the test specific inputs.
 *  handle       - unique test handle
 *  callback     - indicates if this function was called via a callback
 *  testId       - test identifier
 *
 * Returns:
 * Current test status
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp890TestMSocket(
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
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint16 time;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMSocket(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return VpMSocketArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    switch (*pTestState) {
        case COMMOM_TEST_SETUP:
            Vp890CommonTestSetup(pLineCtx, deviceId);
            /* no break */

        case MSOCKET_SETUP:
            VpMSocketSetup(pDevObj, pLineObj, pTestHeap);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, MSOCKET_CHANGE_FEED,
                MS_TO_TICKRATE(VP890_MSOCKET_SETTLING_TIME, tick) );
            break;

        case MSOCKET_CHANGE_FEED:
            /* change the current feed condtion if need be */
            time = VpMSocketChangeFeed(pLineCtx, pDevObj, *pAdcState);
            if (time > 0) {
                Vp890SetTestStateAndTimer(pDevObj, pTestState, MSOCKET_ADC_SETUP,
                    MS_TO_TICKRATE(time, tick));
                break;
            }

        case MSOCKET_ADC_SETUP:
            VpMSocketAdcSetup(pLineCtx, *pAdcState);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, MSOCKET_COLLECT_DATA,
                MS_TO_TICKRATE(VP890_MSOCKET_SETTLING_TIME, tick));
            break;

        case MSOCKET_COLLECT_DATA:
            /* Start the PCM collect process */
            VpMSocketCollectData(pLineCtx, pTestHeap, *pAdcState);
            break;

        case MSOCKET_GET_RESULT: {
            /* Store the PCM data in the results structure */
            VpMSocketGetResult(pLineCtx, pDevObj, pTestState, pAdcState, chanId);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState,
                MS_TO_TICKRATE(VP890_MSOCKET_SETTLING_TIME, tick));
            break;
        }

        case MSOCKET_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMSocket(): complete"));
            break;

        case MSOCKET_QUIT:
            /* The test has eneded early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestMSocket(ec:%i): quit", *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestMSocket(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpMSocketArgCheck()
 * This functions is called by the Vp890TestMSocket() function if
 * Vp890TestMSocket() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the MSocket primitive are legal. The function also sets up
 * the MSocket state machine for the first state and initializes the results
 * from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  handle          - unique test handle
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *  testId          - test identifier
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
VpMSocketArgCheck(
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
    VpTestMSocketType *pMSocketData = &(pTestInfo->pTestHeap->testArgs).mSocket;
    const VpTestMSocketType *pTestInput = pArgsUntyped;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpMSocketArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */
    *pAdcState = MSOCKET_IMT_NORMAL_FEED;
    pDevObj->testResults.result.mSock.imnf = 0;
    pDevObj->testResults.result.mSock.vanf = 0;
    pDevObj->testResults.result.mSock.vbnf = 0;
    pDevObj->testResults.result.mSock.varf1 = 0;
    pDevObj->testResults.result.mSock.vbrf1 = 0;
    pDevObj->testResults.result.mSock.imrf1 = 0;
    pDevObj->testResults.result.mSock.varf2 = 0;
    pDevObj->testResults.result.mSock.vbrf2 = 0;
    pDevObj->testResults.result.mSock.imrf2 = 0;
    pDevObj->testResults.result.mSock.varf3 = 0;
    pDevObj->testResults.result.mSock.vbrf3 = 0;
    pDevObj->testResults.result.mSock.imrf3 = 0;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* This test cannot run if the isolate relay is open */
    if (VP_RELAY_RESET == pLineObj->relayState) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpMSocketArgCheck(): bailed openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    pMSocketData->dummy = pTestInput->dummy;

    /*
     * Force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE, NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpMSocketSetup()
 * This function is called by the Vp890TestMSocket() state machine during
 * the MSOCKET_SETUP state.
 *
 * This function is used to configure the line in low gain mode. Also forcing
 * the the battery voltage if possible and the VOC voltage to +6V.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  pLineObj        - pointer to the line object.
 *  pTestHeap       - pointer to the test heap.
 *
 * Returns:
 * --
 * Result:
 *
 *----------------------------------------------------------------------------*/
static void
VpMSocketSetup(
    Vp890DeviceObjectType *pDevObj,
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap)
{
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 dcfeed[VP890_DC_FEED_LEN] = {0x23, 0x1F};
    uint8 slacState =  VP890_SS_ACTIVATE_MASK | VP890_SS_LOW_GAIN;
    uint8 switchRegParam[VP890_REGULATOR_PARAM_LEN] = {0x0A, 0x04, 0x04}; /* battery 25V fixed */
    uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
        {0x00, 0x09, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* +6V */
    uint8 lpSuper[VP890_LOOP_SUP_LEN];
    uint8 mpiBuffer[VP890_LOOP_SUP_LEN+1 +
                    VP890_REGULATOR_PARAM_LEN+1 +
                    VP890_GEN_CTRL_LEN+1 +
                    VP890_DC_FEED_LEN+1 +
                    VP890_ICR1_LEN+1 +
                    VP890_ICR4_LEN+1 +
                    VP890_ICR2_LEN+1 +
                    VP890_ICR3_LEN+1 +
                    VP890_SYS_STATE_LEN+1 +
                    VP890_SIGA_PARAMS_LEN+1];
    uint8 mpiIndex = 0;

    switchRegParam[0] = (pTestHeap->switchReg[0] & 0xF5) | switchRegParam[0];
    switchRegParam[1] = (pTestHeap->switchReg[1] & 0xE0) | switchRegParam[1];
    switchRegParam[2] = (pTestHeap->switchReg[2] & 0xE0) | switchRegParam[2];

    lpSuper[0] = pTestHeap->lpSuper[0] | 0x07;
    lpSuper[1] = 0xFF;
    lpSuper[2] = pTestHeap->lpSuper[2];
    lpSuper[3] = pTestHeap->lpSuper[3];

    pLineObj->icr1Values[0] = 0x0F;
    pLineObj->icr1Values[1] = 0x01;
    pLineObj->icr1Values[2] = 0x00;
    pLineObj->icr1Values[3] = 0x00;

    pLineObj->icr2Values[0] = 0x1C;
    pLineObj->icr2Values[1] = 0x1C;
    pLineObj->icr2Values[2] &= 0x0C;
    pLineObj->icr2Values[2] |= 0x01;
    pLineObj->icr2Values[3] &= 0x0C;
    pLineObj->icr2Values[3] |= 0x01;

    pLineObj->icr3Values[0] = 0x00;
    pLineObj->icr3Values[1] = 0x00;
    pLineObj->icr3Values[2] = 0x10;
    pLineObj->icr3Values[3] = 0x00;

    pLineObj->icr4Values[0] = 0xFF;
    pLineObj->icr4Values[1] = 0x03;
    pLineObj->icr4Values[2] = 0xFF;
    pLineObj->icr4Values[3] = 0x2C;

    /* Masking the hook detection by maxing out TSH */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_LOOP_SUP_WRT,
        VP890_LOOP_SUP_LEN, lpSuper);

    /* Set the floor voltage */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_REGULATOR_PARAM_WRT,
        VP890_REGULATOR_PARAM_LEN, switchRegParam);
    VpMemCpy(pDevObj->swParamsCache, switchRegParam, VP890_REGULATOR_PARAM_LEN);

    /* Turn on DC bias */
    pLineObj->sigGenCtrl[0] = VP890_GEN_CTRL_EN_BIAS;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_GEN_CTRL_WRT,
        VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);

    /* LI=1, min VOC; max curr lim */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
        VP890_DC_FEED_LEN, dcfeed);

    /* Increase SLIC bias current */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR1_WRT,
        VP890_ICR1_LEN, pLineObj->icr1Values);

    /* Prevent the hook detection during the test */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
        VP890_ICR4_LEN, pLineObj->icr4Values);

    /* Increase SLIC bias current */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
        VP890_ICR2_LEN, pLineObj->icr2Values);

    /* Turn off long MBAT bias */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
        VP890_ICR3_LEN, pLineObj->icr3Values);

    /* Lo-gain normal active state */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SYS_STATE_WRT,
        VP890_SYS_STATE_LEN, &slacState);

    /* Set VOC = +6V */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SIGA_PARAMS_WRT,
        VP890_SIGA_PARAMS_LEN, sigGenA);

    /* Send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * VpMSocketChangeFeed()
 * This function is called by the Vp890TestMSocket() state machine during
 * the MSOCKET_CHANGE_FEED state.
 *
 * This function is used to change the the feed (VOC +6V -> -6V).
 * It is also used to initialize some variables
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pDevObj         - pointer to the device object.
 *  adcState        - current value of the adc setup state.
 *
 * Returns:
 *  Duration of the next timer.
 * Result:
 * If the incoming adc state is not a feed condition change then nothing
 * happens and the next timer duration is set to 0. Other wise the device
 * is changed according to the feed state and the time argument is set to
 * state change settling time.
 *----------------------------------------------------------------------------*/
static uint16
VpMSocketChangeFeed(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    uint8 adcState)
{
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint16 *loopDuration = &pTestHeap->speedupTime;
    int16 *previousValue = &pTestHeap->vabComputed;
    int16 *settlingCnt = &pTestHeap->previousAvg;
    uint8 *loopCnt = &pTestHeap->loopCnt;

    /* Initialize the 'previous' measurement used in he settling algorithm */
    *previousValue = 0;

    /* Initialize the settling counter and the ADC retry counter */
    *settlingCnt = 0;
    *loopCnt = 0;

    /* Initialize the loop duration */
    if (adcState == MSOCKET_IMT_NORMAL_FEED) {
        *loopDuration = pDevObj->timeStamp / 2;
    }

    /* Reverse the feed voltage if needed */
    if (adcState == MSOCKET_VBG_REVERSE_FEED_1) {
        uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
            {0x00, 0xF6, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* -6V */

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SIGA_PARAMS_WRT, VP890_SIGA_PARAMS_LEN,
            sigGenA);

        return VP890_STATE_CHANGE_SETTLING_TIME_SHORT;  /* 50ms */
    } else {
        return 0;   /* no reason to wait if not changing feed states */
    }
}

/*------------------------------------------------------------------------------
 * VpMSocketAdcSetup()
 * This function is called by the Vp890TestMSocket() state machine during
 * the MSOCKET_ADC_SETUP state.
 *
 * This function is used to call the Vp890AdcSetup() depending on the
 * current ac rloop adc state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  adcState        - current value of the adc setup state.
 *
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static void
VpMSocketAdcSetup(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    switch (adcState) {
        case MSOCKET_IMT_NORMAL_FEED:
        case MSOCKET_IMT_REVERSE_FEED_1:
        case MSOCKET_IMT_REVERSE_FEED_2:
        case MSOCKET_IMT_REVERSE_FEED_3:
            Vp890AdcSetup(pLineCtx, VP890_LONGITUDINAL_DC_I, FALSE); /* ilg->imt in low gain */
            break;
        case MSOCKET_VAG_NORMAL_FEED:
        case MSOCKET_VAG_REVERSE_FEED_1:
        case MSOCKET_VAG_REVERSE_FEED_2:
        case MSOCKET_VAG_REVERSE_FEED_3:
            Vp890AdcSetup(pLineCtx, VP890_TIP_TO_GND_V, FALSE);
            break;
        case MSOCKET_VBG_NORMAL_FEED:
        case MSOCKET_VBG_REVERSE_FEED_1:
        case MSOCKET_VBG_REVERSE_FEED_2:
        case MSOCKET_VBG_REVERSE_FEED_3:
            Vp890AdcSetup(pLineCtx, VP890_RING_TO_GND_V, FALSE);
            break;
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketAdcSetup(): invalid ADC state"));
            break;
    }
    return;
}

/*------------------------------------------------------------------------------
 * VpMSocketCollectData()
 * This function is called by the Vp890TestMSocket() state machine during
 * the MSOCKET_COLLECT_DATA state.
 *
 * This function is used to set the proper settling and integration time
 * zaacording to the ADC state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *  adcState        - current value of the adc setup state.
 *
 * Result:
 * --
 *----------------------------------------------------------------------------*/
static void
VpMSocketCollectData(
    VpLineCtxType *pLineCtx,
    Vp890TestHeapType *pTestHeap,
    uint8 adcState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint16 *loopDuration = &pTestHeap->speedupTime;
    uint16 settlingTime ;    /* PcmCollect settling time in milliseconds */

    switch (adcState) {
        case MSOCKET_IMT_NORMAL_FEED:
            settlingTime = 100 - Vp890ReturnElapsedTime(pDevObj, *loopDuration);
            *loopDuration = pDevObj->timeStamp / 2;
            break;
        case MSOCKET_VAG_NORMAL_FEED:
            settlingTime = 1;
            break;
        case MSOCKET_VBG_NORMAL_FEED:
            settlingTime = 1;
            break;
        case MSOCKET_VAG_REVERSE_FEED_1:
            settlingTime = 100;
            /* Reset the loop duration to synchronize the measurements in reverse feed */
            *loopDuration = pDevObj->timeStamp / 2;
            break;
        case MSOCKET_VBG_REVERSE_FEED_1:
            settlingTime = 1;
            break;
        case MSOCKET_IMT_REVERSE_FEED_1:
            settlingTime = 1;
            break;
        case MSOCKET_VAG_REVERSE_FEED_2:
            settlingTime = 600 - Vp890ReturnElapsedTime(pDevObj, *loopDuration);
            *loopDuration = pDevObj->timeStamp / 2;
            break;
        case MSOCKET_VBG_REVERSE_FEED_2:
            settlingTime = 1;
            break;
        case MSOCKET_IMT_REVERSE_FEED_2:
            settlingTime = 1;
            break;
        case MSOCKET_VAG_REVERSE_FEED_3:
            settlingTime = 600 - Vp890ReturnElapsedTime(pDevObj, *loopDuration);
            *loopDuration = pDevObj->timeStamp / 2;
            break;
        case MSOCKET_VBG_REVERSE_FEED_3:
            settlingTime = 1;
            break;
        case MSOCKET_IMT_REVERSE_FEED_3:
            settlingTime = 1;
            break;
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketCollectData(): invalid ADC state"));
            settlingTime = 0;
            break;
    }
    /* Run the PCM collect with a variable settling time and 10ms for integration time */
    Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE, MSOCKET_GET_RESULT, settlingTime * 8, 80);

    return;
}

/*------------------------------------------------------------------------------
 * VpMSocketGetResult()
 * This function is called by the Vp890TestMSocket() state machine during
 * the MSOCKET_GET_RESULT state. Another measurement will be requested if
 * the ADC was reseted or if a measurement needs more timr to settle.
 *
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pDevObj         - pointer to the device object.
 *  pTestState      - pointer to the device current test state.
 *  pAdcState       - pointer to the value of the current ADC measurement state.
 *  chanId          - channel identifier.
 *
 * Returns:
 *  Three possible results of running this function are:
 *  1) The measured data is not valid, request a new one jump to MSOCKET_COLLECT_DATA.
 *  2) The ADC can't stay in the requested setting, bail-out with an error.
 *  3) The measurement can't settle within the required time, issue a warning
 *     and go to the next measurement.
 *----------------------------------------------------------------------------*/
static void
VpMSocketGetResult(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    int16 *pTestState,
    uint8 *pAdcState,
    uint8 chanId)
{
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    int16 *settlingCnt = &pTestHeap->previousAvg;
    int16 *previousValue = &pTestHeap->vabComputed;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;

    if (TRUE == VpMSocketStoreResult(pLineCtx, pDevObj, chanId, *pAdcState)) {
        if ((*settlingCnt < MAX_ITERATION) && (*loopCnt < MAX_ITERATION)) {
            /* An extra measurement is requested with the same ADC setting */
            *pTestState = MSOCKET_COLLECT_DATA;
            return;
        } else if (*loopCnt >= MAX_ITERATION) {
            /* The ADC can't settle */
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            *pTestState = MSOCKET_QUIT;
            VP_TEST(VpLineCtxType, pLineCtx, ("VpMSocketGetResult(): The ADC can't settle!"));
            return;
        } else {
            *previousValue = 0;
            VP_TEST(VpLineCtxType, pLineCtx,
                ("VpMSocketGetResult(): Took too long to settle, reduced accuracy"));
        }
    }

    /* move to the next ADC measurment state */
    if (MSOCKET_MAX_ADC > ++(*pAdcState)) {
        *pTestState = MSOCKET_CHANGE_FEED;
    } else {
        *pTestState = MSOCKET_END;
    }
    return;
}

/*------------------------------------------------------------------------------
 * VpMSocketStoreResult()
 * This function is called by the VpMSocketGetResult() function, it is
 * used to collect the result form the PCM integrator, load the result
 * into the results structure and check the settling of a measurement
 * if needed.
 *
 * Parameters:
 *  pLineCtx       - pointer to the line context.
 *  pDevObj        - pointer to the device object.
 *  chanId         - channel ID.
 *  adcState       - value of the current ADC state.
 *
 * Returns:
 * This function returns a boolean indicating if the measurement was successful
 *
 * Result:
 *  --
 *----------------------------------------------------------------------------*/
static bool
VpMSocketStoreResult(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState)
{
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 adcConf;
    int16 currentValue = 0;
    int16 *previousValue = &pTestHeap->vabComputed;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    int16 *settlingCnt = &pTestHeap->previousAvg;

    /* If the adc setup is not set as expected, set it and measure again */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD, VP890_CONV_CFG_LEN, &adcConf);
    switch (adcState) {
        case MSOCKET_IMT_NORMAL_FEED:   /* low gain il->im */
        case MSOCKET_IMT_REVERSE_FEED_1:
        case MSOCKET_IMT_REVERSE_FEED_2:
        case MSOCKET_IMT_REVERSE_FEED_3:
            if ((adcConf & 0x0F) != VP890_LONGITUDINAL_DC_I) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_LONGITUDINAL_DC_I, FALSE);
                return TRUE;
            }
            break;
        case MSOCKET_VAG_NORMAL_FEED:
        case MSOCKET_VAG_REVERSE_FEED_1:
        case MSOCKET_VAG_REVERSE_FEED_2:
        case MSOCKET_VAG_REVERSE_FEED_3:
            if ((adcConf & 0x0F) != VP890_TIP_TO_GND_V) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_TIP_TO_GND_V, FALSE);
                return TRUE;
            }
            break;
        case MSOCKET_VBG_NORMAL_FEED:
        case MSOCKET_VBG_REVERSE_FEED_1:
        case MSOCKET_VBG_REVERSE_FEED_2:
        case MSOCKET_VBG_REVERSE_FEED_3:
            if ((adcConf & 0x0F) != VP890_RING_TO_GND_V) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_RING_TO_GND_V, FALSE);
                return TRUE;
            }
            break;
        default:
            break;
    }

    switch (adcState) {
        case MSOCKET_IMT_NORMAL_FEED:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_NO_CONNECT, &pDevObj->testResults.result.mSock.imnf);
            currentValue = pDevObj->testResults.result.mSock.imnf;
            break;
        case MSOCKET_VAG_NORMAL_FEED:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_TIP_TO_GND_V, &pDevObj->testResults.result.mSock.vanf);
            break;
        case MSOCKET_VBG_NORMAL_FEED:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_RING_TO_GND_V, &pDevObj->testResults.result.mSock.vbnf);
            break;
        case MSOCKET_IMT_REVERSE_FEED_1:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_NO_CONNECT, &pDevObj->testResults.result.mSock.imrf1);
            break;
        case MSOCKET_IMT_REVERSE_FEED_2:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_NO_CONNECT, &pDevObj->testResults.result.mSock.imrf2);
            break;
        case MSOCKET_IMT_REVERSE_FEED_3:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_NO_CONNECT, &pDevObj->testResults.result.mSock.imrf3);
            break;
        case MSOCKET_VAG_REVERSE_FEED_1:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_TIP_TO_GND_V, &pDevObj->testResults.result.mSock.varf1);
            break;
        case MSOCKET_VAG_REVERSE_FEED_2:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_TIP_TO_GND_V, &pDevObj->testResults.result.mSock.varf2);
            break;
        case MSOCKET_VAG_REVERSE_FEED_3:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_TIP_TO_GND_V, &pDevObj->testResults.result.mSock.varf3);
            break;
        case MSOCKET_VBG_REVERSE_FEED_1:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_RING_TO_GND_V, &pDevObj->testResults.result.mSock.vbrf1);
            break;
        case MSOCKET_VBG_REVERSE_FEED_2:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_RING_TO_GND_V, &pDevObj->testResults.result.mSock.vbrf2);
            break;
        case MSOCKET_VBG_REVERSE_FEED_3:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_RING_TO_GND_V, &pDevObj->testResults.result.mSock.vbrf3);
            break;
        default:
            break;
    }

    switch (adcState) {
        case MSOCKET_IMT_NORMAL_FEED:
            if (ABS(currentValue - *previousValue) > FEED_CHANGE_SETTLINGE_ERR) {
                /* imt is not stable, need more time to settle */
                *previousValue = currentValue;
                (*settlingCnt)++;
                return TRUE;
            }
            *previousValue = 0;
            break;

        default:
            break;
    }

    return FALSE;
}

#endif /* VP890_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_890_SERIES */
