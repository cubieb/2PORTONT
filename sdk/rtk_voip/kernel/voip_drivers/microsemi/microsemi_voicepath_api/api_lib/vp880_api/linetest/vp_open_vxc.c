/** file vp_open_vxc.c
 *
 *  This file contains the open VDC and VAC algorithms.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9485 $
 * $LastChangedDate: 2012-02-02 18:08:25 -0600 (Thu, 02 Feb 2012) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)
#if defined (VP880_INCLUDE_TESTLINE_CODE)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_api_testline_int.h"

/* Project defines */
#define MAX_COLLAPSE_TIME   2000    /* Maximum collapse time in ms */

typedef enum
{
    COMMON_TEST_SETUP           =  VP880_TESTLINE_GLB_STRT_STATE,
    OPEN_VXC_COLLAPSE_FEED      = 5,
    OPEN_VXC_CAL_SETUP          = 10,
    OPEN_VXC_SET_CAL_ADC        = 15,
    OPEN_VXC_COLLECT_CAL_DATA   = 20,
    OPEN_VXC_GET_CAL_RESULTS    = 25,
    OPEN_VXC_GET_RESTORE_SENSE  = 26,
    OPEN_VXC_DISABLE_FEED_WORK_AROUND = 30,
    OPEN_VXC_DISABLE_FEED       = 35,
    OPEN_VXC_OPEN_RELAY         = 40,
    OPEN_VXC_OPEN_PTC_1         = 45,
    OPEN_VXC_OPEN_PTC_2         = 50,
    OPEN_VXC_OPEN_PTC_3         = 55,
    OPEN_VXC_ADC_SETUP          = 60,
    OPEN_VXC_COLLECT_DATA       = 65,
    OPEN_VXC_GET_RESULTS        = 70,
    OPEN_VXC_END                = VP880_TESTLINE_GLB_END_STATE,
    OPEN_VXC_QUIT               = VP880_TESTLINE_GLB_QUIT_STATE,
    OPEN_VXC_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpOpenVxcTestStateTypes;


typedef enum {
    OPEN_VXC_CAL_VAH = 0,
    OPEN_VXC_CAL_VBH = 1,
    OPEN_VXC_CAL_VAB = 2,
    OPEN_VXC_CAL_NULL = 3,
    OPEN_VXC_CAL_MAX_ADC,
    OPEN_VXC_CAL_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpOpenVxcCalAdcOrderTypes;

static VpStatusType
ArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId,
    uint8 *pAdcState);

static void
CollapseFeed(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId);

static void
SetupCal(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId,
    uint8 adcState);

static bool
SetCalAdc(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static vpOpenVxcTestStateTypes
GetCalResults(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    VpTestStatusType *pErrorCode,
    uint8 chanId,
    uint8 *pAdcState,
    uint16 *pDelayMs);

static bool
SetLeadAdc(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj);

static void
GetResult(
    Vp880DeviceObjectType *pDevObj,
    VpTestStatusType *pErrorCode,
    uint8 chanId);

/*------------------------------------------------------------------------------
 * Vp880TestOpenVxc
 * This functions implements the Open VXC Test primitives ...
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  handle          - unique test handle
 *  test            - current test id (VAC or VDC)
 *  callbak         - indicates if this function was called via a callback
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
VpStatusType
Vp880TestOpenVxc (
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
    VpTestOpenVType *pOpenVData = &(pTestHeap->testArgs).openV;

    uint8 *pAdcState = &pTestHeap->adcState;
    int16 *pTestState = &pTestInfo->testState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint16 *rampDuration = &pTestHeap->speedupTime;
    uint8 *pLoopRoute = &pTestHeap->loopCnt;
    int16 *pPreviousTipVoltage = &pTestHeap->previousAvg;
    int16 result;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestOpenVxc(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return ArgCheck(pLineCtx, pArgsUntyped, handle, testId, pAdcState);
    }

    switch (*pTestState) {
        case COMMON_TEST_SETUP:
            if (TRUE == pOpenVData->calMode) {
                Vp880CommonTestSetup(pLineCtx, deviceId);
                *pLoopRoute = VP880_TIP_TO_GND_V;

                VpSetTestStateAndTimer(pDevObj, pTestState,
                    OPEN_VXC_COLLAPSE_FEED, MS_TO_TICKRATE(10, tick));

                break;
            }
            /* no break */

        case OPEN_VXC_COLLAPSE_FEED:
            if (pOpenVData->calMode == TRUE) {

                /* Store the time stamp (in ms) at the begining of the ramp */
                *rampDuration = pDevObj->timeStamp / 2;

                CollapseFeed(pDevObj, pLineObj, pLineCtx, deviceId);

                /* allow line to settle */
                VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_CAL_SETUP,
                    MS_TO_TICKRATE(10, tick));

                break;
            }
            /* no break */

        case OPEN_VXC_CAL_SETUP:
            if (pOpenVData->calMode == TRUE) {
                SetupCal(pLineObj, pTestHeap, deviceId, *pAdcState);

                VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_SET_CAL_ADC,
                    MS_TO_TICKRATE(10, tick));

                break;
            }
            /* no break */

        case OPEN_VXC_SET_CAL_ADC:
            if (pOpenVData->calMode == TRUE) {
                if ( SetCalAdc(pLineCtx, *pAdcState) == TRUE ) {
                    VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_COLLECT_CAL_DATA,
                        MS_TO_TICKRATE(10, tick));
                } else {
                    *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                    VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_QUIT, NEXT_TICK);
                }
                break;
            }
            /* no break */

        case OPEN_VXC_COLLECT_CAL_DATA:
            if (pOpenVData->calMode == TRUE) {
                VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                    OPEN_VXC_GET_CAL_RESULTS, 40, 40);
                break;
            }
            /* no break */

        case OPEN_VXC_GET_CAL_RESULTS:
            if (pOpenVData->calMode == TRUE) {
                uint16 delayMs;
                *pTestState = GetCalResults(pDevObj, pLineObj, pErrorCode,
                    chanId, pAdcState, &delayMs);

                VpSetTestStateAndTimer(pDevObj, pTestState, *pTestState,
                    MS_TO_TICKRATE(delayMs, tick));
                break;
            }
            /* no break */

        case OPEN_VXC_GET_RESTORE_SENSE:
            if (pOpenVData->calMode == TRUE) {
                pLineObj->icr6Values[0] = 0x00;
                pLineObj->icr6Values[1] = 0x00;

                VpMpiCmdWrapper(pDevObj->deviceId, pLineObj->ecVal, VP880_ICR6_WRT,
                    VP880_ICR6_LEN, pLineObj->icr6Values);

                /* If abs then restore battery selection */
                if (VP880_IS_ABS & pDevObj->stateInt) {
                    pLineObj->icr1Values[2] &= ~0x30;

                    VpMpiCmdWrapper(pDevObj->deviceId, pLineObj->ecVal, VP880_ICR1_WRT,
                        VP880_ICR1_LEN, pLineObj->icr1Values);
                }

                /* make sure we provide enough time for a heavy cap load to discharge */
                VpSetTestStateAndTimer(pDevObj, pTestState,
                    OPEN_VXC_DISABLE_FEED_WORK_AROUND, MS_TO_TICKRATE(100, tick));
                break;
            }


        case OPEN_VXC_DISABLE_FEED_WORK_AROUND:
            if (pOpenVData->calMode == TRUE) {
                /*
                 * Because it is not legal to go from a ringing state to a
                 * disconnect state with the CODEC's activate bit set, we
                 * must first set the line state to disconnect wait 5ms
                 * and then go to disconnect with activate bits set.
                 */
                if ((VP_TERM_FXS_LOW_PWR != pLineObj->termType) &&
                    (VP_TERM_FXS_ISOLATE_LP != pLineObj->termType) &&
                    (VP_TERM_FXS_SPLITTER_LP != pLineObj->termType)) {
                    pLineObj->slicValueCache = VP880_SS_DISCONNECT;

                    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SLIC_STATE_WRT,
                        VP880_SLIC_STATE_LEN, &pLineObj->slicValueCache);

                    VpSetTestStateAndTimer(pDevObj, pTestState,
                        OPEN_VXC_DISABLE_FEED, MS_TO_TICKRATE(10, tick));

                    break;
                }
            }
            /* no break */

        case OPEN_VXC_DISABLE_FEED:
            if (TRUE == pOpenVData->calMode) {

                if ((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
                    (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
                    (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) {
                    pLineObj->slicValueCache = VP880_SS_ACTIVATE_MASK | VP880_SS_SHUTDOWN;
                } else {
                    pLineObj->slicValueCache = VP880_SS_ACTIVATE_MASK | VP880_SS_DISCONNECT;
                }

                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SLIC_STATE_WRT,
                    VP880_SLIC_STATE_LEN, &pLineObj->slicValueCache);

                VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_OPEN_RELAY,
                    MS_TO_TICKRATE(VP880_FEED_DISCHARGE_TIME, tick));

                break;
            }
            /* no break */

        case OPEN_VXC_OPEN_RELAY:
            /*
             * TO DO:
             * Can we jump straight to here from OPEN_VXC_GET_CAL_RESULTS
             * if relays are present
             */
            if (TRUE == pOpenVData->calMode) {
                if (((VP_TERM_FXS_ISOLATE == pLineObj->termType) ||
                   (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType)  ||
                   (VP_TERM_FXS_SPLITTER == pLineObj->termType) ||
                   (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) &&
                   (VP_RELAY_RESET != pLineObj->relayState) ) {

                    VpSetRelayState(pLineCtx, VP_RELAY_RESET);

                    /* Allow voltage to settle */
                    VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_OPEN_PTC_1,
                        MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME, tick));
                    break;
                }
            }
            /* no break */

        case OPEN_VXC_OPEN_PTC_1:
            if (pOpenVData->calMode == TRUE) {
                Vp880AdcSetup(pLineCtx, *pLoopRoute, FALSE);
                VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_OPEN_PTC_2,
                    MS_TO_TICKRATE(20, tick));

                break;
            }
            /* no break */

        case OPEN_VXC_OPEN_PTC_2:
            if (pOpenVData->calMode == TRUE) {
                VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE, OPEN_VXC_OPEN_PTC_3, 0, 146);

                break;
            }
            /* no break */

        case OPEN_VXC_OPEN_PTC_3:
            if (pOpenVData->calMode == TRUE) {
                VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                *pLoopRoute, &result);

                if (*pLoopRoute == VP880_TIP_TO_GND_V) {
                    /* Measure and store the Tip -> Gnd voltage */
                    *pLoopRoute = VP880_RING_TO_GND_V;
                    *pPreviousTipVoltage = result;
                    VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_OPEN_PTC_1,
                        MS_TO_TICKRATE(10, tick));
                } else {
                    /*  continue to settle? */
                    int16 tipV = (int16)VP880_UNIT_CONVERT(*pPreviousTipVoltage,
                        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
                    int16 ringV = (int16)VP880_UNIT_CONVERT(result,
                        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

                    if ( (VpReturnElapsedTime(pDevObj, *rampDuration) < MAX_COLLAPSE_TIME) &&
                         (( (tipV  >= 0 ) && (tipV  >  2000) ) ||
                          ( (tipV  <  0 ) && (tipV  < -1400) ) ||
                          ( (ringV >= 0 ) && (ringV >  2000) ) ||
                          ( (ringV <  0 ) && (ringV < -1400) )) ) {
                        *pLoopRoute = VP880_TIP_TO_GND_V;
                        VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_OPEN_PTC_1,
                            MS_TO_TICKRATE(100, tick));
                    } else {
                        VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_ADC_SETUP,
                            MS_TO_TICKRATE(10, tick));
                    }
                }
                break;
            }
            /* no break */

        case OPEN_VXC_ADC_SETUP:
            if ( TRUE == SetLeadAdc(pLineCtx, pDevObj) ) {
                VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_COLLECT_DATA,
                    MS_TO_TICKRATE(10, tick));

            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_QUIT, NEXT_TICK);
            }
            break;

        case OPEN_VXC_COLLECT_DATA:
            VpStartPcmCollect(pLineCtx,
                (VP_PCM_OPERATION_AVERAGE | VP_PCM_OPERATION_RMS),
                OPEN_VXC_GET_RESULTS, pOpenVData->settlingTime,
                pOpenVData->integrateTime);
            break;

        case OPEN_VXC_GET_RESULTS:
            GetResult(pDevObj, pErrorCode, chanId);

            if (((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
                 (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
                 (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) &&
                ((pTestHeap->testArgs.openV.tip) == VP_TEST_TIP_RING) && TRUE) {
                uint8 slacState = VP880_SS_DISCONNECT;
                uint8 mpiBuffer[VP880_ICR2_LEN+1 +
                                VP880_SYS_STATE_LEN+1];
                uint8 mpiIndex = 0;

                pLineObj->icr2Values[0] = 0x0C;
                pLineObj->icr2Values[1] = 0x0C;
                pLineObj->icr2Values[2] &= 0x0C;
                pLineObj->icr2Values[2] |= 0x01;
                pLineObj->icr2Values[3] &= 0x0C;
                pLineObj->icr2Values[3] |= 0x01;

                /* Turn off feed voltage, limit power supply to 103V */
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
                    VP880_ICR2_LEN, pLineObj->icr2Values);

                /* Set line to Disconnect */
                pLineObj->slicValueCache = slacState;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SYS_STATE_WRT,
                    VP880_SYS_STATE_LEN, &slacState);

                /* send down the mpi commands */
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);
            }

            VpSetTestStateAndTimer(pDevObj, pTestState, OPEN_VXC_END, MS_TO_TICKRATE(10, tick));
            break;

        case OPEN_VXC_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestOpenVxc(): complete"));
            break;

        case OPEN_VXC_QUIT:
            /* The test has eneded early due to the current errorCode */
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestOpenVxc(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            /* The test has entered an unsuppoted state */
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestOpenVxc(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpOpenVxcArgCheck
 * This functions is called by the Vp880TestOpenVxc() function if
 * Vp880TestOpenVxc() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the Open VXC primitive are legal. The function also sets up
 * the open vdc state machine for the first state and initializes the results
 * from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  test            - current test id
 *  handle          - unique test handle
 *
 * Returns:
 * VpStatusType
 *----------------------------------------------------------------------------*/
static VpStatusType
ArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId,
    uint8 *pAdcState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTestOpenVType *pTestInput = (VpTestOpenVType*)pArgsUntyped;
    VpTestOpenVType *pOpenVxcData = &(pTestInfo->pTestHeap->testArgs).openV;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpOpenVxcArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only once per test.
     */
    pDevObj->testResults.result.vxc.vdc = 0;
    pDevObj->testResults.result.vxc.vac = 0;
    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* TODO: check input arguments before assigining them to test struct!!! */
    /* I am using the calMode flag to indicate feed collapse not calibration */
    pOpenVxcData->calMode = pTestInput->calMode;
    pOpenVxcData->integrateTime = pTestInput->integrateTime;
    pOpenVxcData->settlingTime = pTestInput->settlingTime;
    pOpenVxcData->tip = pTestInput->tip;

    /* decide what cal circuit to start with */
    if (pDevObj->stateInt & VP880_HAS_CALIBRATE_CIRCUIT) {
        *pAdcState = OPEN_VXC_CAL_VAH;
    } else {
        *pAdcState = OPEN_VXC_CAL_NULL;
    }

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    VpSetTestStateAndTimer(pDevObj, pTestState, VP880_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;
}


/*------------------------------------------------------------------------------
 * CollapseFeed
 * This function is used to collapes the tip/ring feed
 *----------------------------------------------------------------------------*/
static void
CollapseFeed(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId)
{
    uint8 mpiIndex = 0;
    uint8 mpiBuffer[50];

    /*
     * Drive tip/ring to 0 with sig gen A
     */
    uint8 slacState = {VP880_SS_UNBALANCED_RINGING};

    /*
     * a min amp of 2 must be set or the feed will not collapse
     */
    uint8 discModeSigA[VP880_RINGER_PARAMS_LEN] = {
        0x00, 0x00, 0x00, 0x0A, 0xAB, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00
    };

    /* Program Ringing Parameters */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_RINGER_PARAMS_WRT,
        VP880_RINGER_PARAMS_LEN, discModeSigA);

    /*
     * If term type is low power a 150kOhm R to battery is present.
     * In order to collapse feed voltage due to this setup we must shut
     * down the swither associated with the channel under test
     */
    if ((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
        (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
        (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) {

        pLineObj->icr2Values[0] = 0x20;
        pLineObj->icr2Values[1] = 0x00;
        pLineObj->icr2Values[2] &= 0x0C;
        pLineObj->icr2Values[2] |= 0x20;
        pLineObj->icr2Values[3] &= 0x0C;

        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
            VP880_ICR2_LEN, pLineObj->icr2Values);
    }

    /*
     * Force ABS line to low battery while the tip/ring sense is open
     */
    if (VP880_IS_ABS & pDevObj->stateInt) {
        pLineObj->icr1Values[2] |= 0x30;

        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR1_WRT,
            VP880_ICR1_LEN, pLineObj->icr1Values);
    }

    /* Set line to Unbalanced Ringing */
    pLineObj->slicValueCache = slacState;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SYS_STATE_WRT,
        VP880_SYS_STATE_LEN, &slacState);


    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
        mpiIndex-1, &mpiBuffer[1]);
    return;
}

/*------------------------------------------------------------------------------
 * SetupCal
 * Preparing the device to take measurements with the tip ring sense open
 *----------------------------------------------------------------------------*/
static void
SetupCal(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId,
    uint8 adcState)
{
    uint8 mpiIndex = 0;
    uint8 mpiBuffer[30];

    pLineObj->icr2Values[0] |= 0x0C;
    pLineObj->icr2Values[1] |= 0x0C;
    pLineObj->icr2Values[2] |= 0x00;
    pLineObj->icr2Values[3] |= 0x00;

    pLineObj->icr3Values[0] |= 0x21;
    pLineObj->icr3Values[1] |= 0x21;
    pLineObj->icr3Values[2] |= 0x01;
    pLineObj->icr3Values[3] &= ~0x01;

    pLineObj->icr4Values[0] |= 0x01;
    pLineObj->icr4Values[1] |= 0x01;
    pLineObj->icr4Values[2] |= 0x00;
    pLineObj->icr4Values[3] |= 0x00;

    /* open the tip/ring sense paths */
    if (adcState != OPEN_VXC_CAL_NULL) {
        pLineObj->icr6Values[0] |= 0x04;
        pLineObj->icr6Values[1] |= 0x64;

        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR6_WRT,
             VP880_ICR6_LEN, pLineObj->icr6Values);
    }

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
        VP880_ICR2_LEN, pLineObj->icr2Values);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR3_WRT,
        VP880_ICR3_LEN, pLineObj->icr3Values);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR4_WRT,
        VP880_ICR4_LEN, pLineObj->icr4Values);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);
    return;
}


static bool
SetCalAdc(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    bool adcFlag;

    switch (adcState) {
        case OPEN_VXC_CAL_VAH:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_TIP_TO_GND_V, FALSE);
            break;

        case OPEN_VXC_CAL_VBH:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_RING_TO_GND_V, FALSE);
            break;

        case OPEN_VXC_CAL_VAB:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_V, FALSE);
            break;

        case OPEN_VXC_CAL_NULL:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_NO_CONNECT, FALSE);
            break;

        default:
            return FALSE;
    }

    return adcFlag;
}

static vpOpenVxcTestStateTypes
GetCalResults(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    VpTestStatusType *pErrorCode,
    uint8 chanId,
    uint8 *pAdcState,
    uint16 *pDelayMs)
{
    vpOpenVxcTestStateTypes nextState;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    uint16 *rampDuration = &pTestHeap->speedupTime;
    *pDelayMs = 10;

    /* get and store the result based on the current ADC setting */
    switch (*pAdcState) {
        case OPEN_VXC_CAL_VAH:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_TIP_TO_GND_V, &pDevObj->calOffsets[chanId].vahOffset);
            *pAdcState = OPEN_VXC_CAL_VBH;
            nextState = OPEN_VXC_SET_CAL_ADC;
            break;

        case OPEN_VXC_CAL_VBH:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_RING_TO_GND_V, &pDevObj->calOffsets[chanId].vbhOffset);
            *pAdcState = OPEN_VXC_CAL_VAB;
            nextState = OPEN_VXC_SET_CAL_ADC;
            break;

        case OPEN_VXC_CAL_VAB:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_METALLIC_DC_V, &pDevObj->calOffsets[chanId].vabOffset);
            *pAdcState = OPEN_VXC_CAL_MAX_ADC;
            nextState = OPEN_VXC_GET_RESTORE_SENSE;
            break;

        case OPEN_VXC_CAL_NULL:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_NO_CONNECT, &pDevObj->calOffsets[chanId].nullOffset);
            *pAdcState = OPEN_VXC_CAL_MAX_ADC;
            nextState = OPEN_VXC_GET_RESTORE_SENSE;

            /* apply generic offset to all cal coeffs for null adc */
            pDevObj->calOffsets[chanId].vabOffset = pDevObj->calOffsets[chanId].nullOffset;
            pDevObj->calOffsets[chanId].vahOffset = pDevObj->calOffsets[chanId].nullOffset;
            pDevObj->calOffsets[chanId].vbhOffset = pDevObj->calOffsets[chanId].nullOffset;
            break;
        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            nextState = OPEN_VXC_QUIT;
    }

    /* restore the tip/ring sense paths */
    if (nextState == OPEN_VXC_GET_RESTORE_SENSE) {
        /*
         * The worst case supply (fixed tracking with large output caps i.e. 
         * 22.47uF + 20%) needs a total of 1100 ms to collapse the feed below
         * the 0.6 mV datasheet requirment for the line voltage test.
         *
         * This step is also used to discharge large REN loads on Tip/Ring.
         */
        *pDelayMs = VpReturnElapsedTime(pDevObj, *rampDuration);

        if ((pDevObj->swParams[VP880_REGULATOR_TRACK_INDEX] & VP880_REGULATOR_FIXED_RING) &&
            (*pDelayMs < 1100)) {
            *pDelayMs = 1100 - *pDelayMs;
        } else if (((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
            (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
            (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) &&
            (*pDelayMs < 320)) {
            /* Low power needs more time, because it collapses the battery */
            *pDelayMs = 320 - *pDelayMs;
        } else if (*pDelayMs < 250) {
            *pDelayMs = 250 - *pDelayMs;
        } else {
            *pDelayMs = 10;
        }
    }

     return nextState;
}

/*------------------------------------------------------------------------------
 * VpAcOpenVxcAdcSetup
 * This function is called by the Vp880TestOpenVxc() state machine during
 * the OPEN_VXC_ADC_SETUP state.
 *
 * This function is used to call the Vp880AdcSetup() depending on the
 * requested lead.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pDevObj         - pointer to the device object.
 *
 * Returns:
 *  FALSE if something goes wrong during the adc setup.
 *  TRUE if everything goes ok.
 *
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static bool
SetLeadAdc(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj)
{
    VpTestTipSelectType tipOrRing = pDevObj->currentTest.pTestHeap->testArgs.openV.tip;
    bool adcFlag;

    switch (tipOrRing) {
        case VP_TEST_TIP:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_TIP_TO_GND_V, FALSE);
            break;
        case VP_TEST_RING:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_RING_TO_GND_V, FALSE);
            break;
        case VP_TEST_TIP_RING:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_V, FALSE);
            break;
        default:
            return FALSE;
    }

    return adcFlag;
}

static void
GetResult(
    Vp880DeviceObjectType *pDevObj,
    VpTestStatusType *pErrorCode,
    uint8 chanId)
{
    VpTestResultVxcType *pVxcResults = &pDevObj->testResults.result.vxc;
    VpTestTipSelectType tipOrRing = pDevObj->currentTest.pTestHeap->testArgs.openV.tip;
    uint8 aToDRoute =  VP880_NO_CONNECT;

    switch (tipOrRing) {
        case VP_TEST_TIP:
            aToDRoute = VP880_TIP_TO_GND_V;
            break;
        case VP_TEST_RING:
            aToDRoute = VP880_RING_TO_GND_V;
            break;
        case VP_TEST_TIP_RING:
            aToDRoute = VP880_METALLIC_DC_V;
            break;
        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            return;
    }

    /* get the calculated average and remove the offset */
    VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
        aToDRoute, &pVxcResults->vdc);

    /* get the calculated rms */
    VpGetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, chanId,
        aToDRoute, &pVxcResults->vac);


    return;
}

#endif /* VP880_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_880_SERIES */
