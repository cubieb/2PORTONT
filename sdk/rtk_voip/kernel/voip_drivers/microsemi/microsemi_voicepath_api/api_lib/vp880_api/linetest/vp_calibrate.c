/** file vp_calibrate.c
 *
 *  This file contains the functions used in the Vp880 Test Line.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
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

#define VP880_CALIBRATE_SETTLE 40
#define VP880_CALIBRATE_SAMPLES 40

typedef enum
{
    CAL_SETUP               = VP880_TESTLINE_GLB_STRT_STATE,
    CAL_ADC_SETUP           = 10,
    CAL_COLLECT_DATA        = 15,
    CAL_GET_RESULT          = 20,
    CAL_START_RESTORE       = 30,
    CAL_RESTORE_LINE_STATE  = 40,
    CAL_END                 = VP880_TESTLINE_GLB_END_STATE,
    CAL_QUIT                = VP880_TESTLINE_GLB_QUIT_STATE,
    VP_CAL_TEST_STATE_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpCalTestStateTypes;

typedef enum
{
    CAL_VAH,
    CAL_VBH,
    CAL_VAB,
    CAL_NULL,
    CAL_MAX_ADC,
    VP_CAL_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpCalAdcOrderTypes;

VpStatusType
VpCalArgCheck(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpCalSetup(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId,
    uint8 adcState,
    uint8 deviceRcn);

static void
VpCalRestore(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static bool
VpCalAdcSetup(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    uint8 adcState);

static vpCalTestStateTypes
VpCalGetResult(
    Vp880DeviceObjectType *pDevObj,
    VpTestStatusType *pErrorCode,
    uint8 chanId,
    uint8 *pAdcState);

static void
VpCalStoreResult(
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState);

static bool
VpStoreAllOffsetsAsGeneric(
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId);

/*------------------------------------------------------------------------------
 * Vp880TestCalibrate
 * This functions implements the Calibrate Test primitive ...
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  handle      - unique test handle
 *  callbak     - indicates if this function was called via a callback
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
VpStatusType
Vp880TestCalibrate(
    VpLineCtxType *pLineCtx,
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

    uint8 *pAdcState = &pTestHeap->adcState;
    int16 *pTestState = &pTestInfo->testState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint8 deviceRcn = (uint8)(pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]);
    uint16 settleTimer = VP880_FEED_DISCHARGE_TIME;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return VpCalArgCheck(pLineCtx, handle, pAdcState, testId);
    }

    switch (*pTestState) {
        case CAL_SETUP:
            /* prep the line */
            VpCalSetup(pLineObj, pTestHeap, deviceId, *pAdcState, deviceRcn);

            /* Allow circuits to settle */
            if (pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] & VP_ACTIVATE_TIMER) {
                settleTimer = pLineObj->lineTimers.timers.timer[VP_LINE_DISCONNECT_EXIT] &= ~VP_ACTIVATE_TIMER;
            }
            VpSetTestStateAndTimer(pDevObj, pTestState, CAL_ADC_SETUP,
                MS_TO_TICKRATE(settleTimer, tick) + 1);
            break;

        case CAL_ADC_SETUP:
            /* setup the ADC for the current calState*/
            if ( TRUE == VpCalAdcSetup(pLineCtx, pDevObj, *pAdcState) ) {
                VpSetTestStateAndTimer(pDevObj, pTestState, CAL_COLLECT_DATA,
                    MS_TO_TICKRATE(10, tick));
            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                VpSetTestStateAndTimer(pDevObj, pTestState, CAL_QUIT, NEXT_TICK);
            }
            break;

        case CAL_COLLECT_DATA:
            VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE, CAL_GET_RESULT,
                VP880_CALIBRATE_SETTLE, VP880_CALIBRATE_SAMPLES);
            break;

        case CAL_GET_RESULT:
            *pTestState = VpCalGetResult(pDevObj, pErrorCode, chanId, pAdcState);
            VpSetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;

        case CAL_START_RESTORE:
            /* restore coeffs */
             VpCalRestore(pLineObj, pTestHeap, deviceId);
             VpSetTestStateAndTimer(pDevObj, pTestState, CAL_RESTORE_LINE_STATE,
                NEXT_TICK);
            break;

        case CAL_RESTORE_LINE_STATE:
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SLIC_STATE_WRT,
                VP880_SLIC_STATE_LEN, &pTestHeap->calRegs.slacState);
            pLineObj->slicValueCache = pTestHeap->calRegs.slacState;

       case CAL_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            pDevObj->testResults.result.calFailed = FALSE;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(null): %i", 
                pDevObj->calOffsets[chanId].nullOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(vab): %i", 
                pDevObj->calOffsets[chanId].vabOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(vah): %i", 
                pDevObj->calOffsets[chanId].vahOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(vbh): %i", 
                pDevObj->calOffsets[chanId].vbhOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(): complete"));
            break;

        case CAL_QUIT:
            /* restore icr6 */
            VpCalRestore(pLineObj, pTestHeap, deviceId);

            /* The test has ended early due to the current errorCode */
            pDevObj->testResults.result.calFailed = TRUE;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            pDevObj->testResults.result.calFailed = TRUE;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestCalibrate(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpOpenVdcArgCheck
 * This functions is called by the Vp880TestCalibrate() function if
 * Vp880TestCalibrate() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the Calibrate primitive are legal. The function also sets up
 * the Calibrate state machine for the first state and initializes the results
 * from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  handle          - unique test handle
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
VpStatusType
VpCalArgCheck(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpCalArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    if (pDevObj->stateInt & VP880_HAS_CALIBRATE_CIRCUIT) {
        *pAdcState = CAL_VAH;
    } else {
        *pAdcState = CAL_NULL;
    }
    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    VpSetTestStateAndTimer(pDevObj, pTestState, VP880_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpCalSetup
 * This function is called by the Vp880TestCalibrate() state machine during
 * the CAL_SETUP state.
 *
 *
 * Parameters:
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *  adcState        - current value of the adc setup state.
 *
 * Returns:
 *  FALSE if something goes wrong during the adc setup.
 *  TRUE if everything goes ok.
 *
 * Result:
 *  setup the device to read calibration coeffs. as a single mpi block
 *----------------------------------------------------------------------------*/

static void
VpCalSetup(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId,
    uint8 adcState,
    uint8 deviceRcn)
{
    Vp880LineTestCalType *pCalRegs = &pTestHeap->calRegs;
    uint8 disn = 00;
    uint8 opFunction = VP880_LINEAR_CODEC;

    uint8 mpiBuffer[VP880_SLIC_STATE_LEN+1 +
                     VP880_DISN_LEN+1 +
                     VP880_OP_COND_LEN+1 +
                     VP880_OP_FUNC_LEN+1 +
                     VP880_ICR2_LEN+1 +
                     VP880_ICR3_LEN+1 +
                     VP880_ICR4_LEN+1 +
                     VP880_ICR6_LEN+1];
    uint8 mpiIndex = 0;

    /* store all of the register that will be changed by cal */
    pCalRegs->slacState = pLineObj->slicValueCache;
    pCalRegs->opCond = pLineObj->opCond[0];

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_OP_FUNC_RD,
        VP880_OP_FUNC_LEN, &pCalRegs->opFunction);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_VP_GAIN_RD,
        VP880_VP_GAIN_LEN, &pCalRegs->vpGain);

    VpMemCpy(pCalRegs->icr2, pLineObj->icr2Values, VP880_ICR2_LEN);
    VpMemCpy(pCalRegs->icr3, pLineObj->icr3Values, VP880_ICR3_LEN);
    VpMemCpy(pCalRegs->icr4, pLineObj->icr4Values, VP880_ICR4_LEN);
    VpMemCpy(pCalRegs->icr6, pLineObj->icr6Values, VP880_ICR6_LEN);

    if (CAL_NULL != adcState) {
        pLineObj->icr6Values[0] |= 0x04;
        pLineObj->icr6Values[1] |= 0x64;
    }

    pLineObj->icr2Values[0] = 0x0C;
    pLineObj->icr2Values[1] = 0x0C;
    pLineObj->icr2Values[2] &= 0x0C;
    pLineObj->icr2Values[3] &= 0x0C;

    pLineObj->icr3Values[0] = 0x20;
    pLineObj->icr3Values[1] = 0x20;
    pLineObj->icr3Values[2] = 0x00;
    pLineObj->icr3Values[3] = 0x00;

    pLineObj->icr4Values[0] = 0x01;
    pLineObj->icr4Values[1] = 0x01;
    pLineObj->icr4Values[2] = 0x00;
    pLineObj->icr4Values[3] = 0x00;

    /* Activate the CODEC  */
    pLineObj->slicValueCache = VP880_SS_DISCONNECT | VP880_SS_ACTIVATE_MASK;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SLIC_STATE_WRT,
        VP880_SLIC_STATE_LEN, &pLineObj->slicValueCache);

    /* Remove disn */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DISN_WRT,
        VP880_DISN_LEN, &disn);

    /* Disable hi-pass filter and cutoff receive path */
    pLineObj->opCond[0] = (VP880_RXPATH_DIS | VP880_HIGH_PASS_DIS);
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_COND_WRT,
        VP880_OP_COND_LEN, pLineObj->opCond);

    /* Set to read linear data */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_FUNC_WRT,
        VP880_OP_FUNC_LEN, &opFunction);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
        VP880_ICR2_LEN, pLineObj->icr2Values);

    /* Force line control circuits on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR3_WRT,
        VP880_ICR3_LEN, pLineObj->icr3Values);

    /* Force ADC on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR4_WRT,
        VP880_ICR4_LEN, pLineObj->icr4Values);

    /* Connect null calibration input */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR6_WRT,
         VP880_ICR6_LEN, pLineObj->icr6Values);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * VpCalRestore
 * This function is called by the Vp880TestCalibrate() state machine during
 * the CAL_SETUP state.
 *
 *
 * Parameters:
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *
 * Returns:
 *  FALSE if something goes wrong during the adc setup.
 *  TRUE if everything goes ok.
 *
 * Result:
 *  Restores all device registers back to original state as one big mpi block.
 *  (exception SLAC STATE)
 *
 *----------------------------------------------------------------------------*/
static void
VpCalRestore(
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    Vp880LineTestCalType *pCalRegs = &pTestHeap->calRegs;

    uint8 mpiBuffer[ VP880_VP_GAIN_LEN+1 +
                     VP880_OP_COND_LEN+1 +
                     VP880_OP_FUNC_LEN+1 +
                     VP880_ICR2_LEN+1 +
                     VP880_ICR3_LEN+1 +
                     VP880_ICR4_LEN+1 +
                     VP880_ICR6_LEN+1 +
                     VP880_VP_GAIN_LEN+1];
    uint8 mpiIndex = 0;

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_VP_GAIN_WRT,
        VP880_VP_GAIN_LEN, &pCalRegs->vpGain);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_COND_WRT,
        VP880_OP_COND_LEN, &pCalRegs->opCond);
    pLineObj->opCond[0] = pCalRegs->opCond;

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_FUNC_WRT,
        VP880_OP_FUNC_LEN, &pCalRegs->opFunction);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
        VP880_ICR2_LEN, pCalRegs->icr2);
    VpMemCpy(pLineObj->icr2Values, pCalRegs->icr2, VP880_ICR2_LEN);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR3_WRT,
        VP880_ICR3_LEN, pCalRegs->icr3);
    VpMemCpy(pLineObj->icr3Values, pCalRegs->icr3, VP880_ICR3_LEN);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR4_WRT,
        VP880_ICR4_LEN, pCalRegs->icr4);
    VpMemCpy(pLineObj->icr4Values, pCalRegs->icr4, VP880_ICR4_LEN);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR6_WRT,
         VP880_ICR6_LEN, pCalRegs->icr6);
    VpMemCpy(pLineObj->icr6Values, pCalRegs->icr6, VP880_ICR6_LEN);
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_VP_GAIN_WRT,
         VP880_VP_GAIN_LEN, &pCalRegs->vpGain);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}
/*------------------------------------------------------------------------------
 * VpCalAdcSetup
 * This function is called by the Vp880TestCalibrate() state machine during
 * the CAL_ADC_SETUP state.
 *
 * This function is used to call the Vp880AdcSetup() depending on the
 * calState.
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
VpCalAdcSetup(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    uint8 adcState)
{
    bool adcFlag;

    switch (adcState) {
        case CAL_NULL:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_NO_CONNECT, FALSE);
            break;
        case CAL_VAB:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_V, FALSE);
            break;
        case CAL_VAH:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_TIP_TO_GND_V, FALSE);
            break;
        case CAL_VBH:
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_RING_TO_GND_V, FALSE);
            break;
        default:
            return FALSE;
    }

    return adcFlag;
}

/*------------------------------------------------------------------------------
 * VpCalGetResult
 * This function is called by the Vp880TestCalibrate() state machine during
 * the CAL_GET_RESULT state.
 *
 * This function is used to collect the result form the PCM integrator.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  pTestState      - pointer to the device current test state.
 *  pErrorCode      - pointer to the device results errorCode member
 *  chanId          - ec value
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *
 * Returns:
 *  --
 *
 * Result:
 *  Three possible results of running this function are:
 *  1) the generic cal state has been run and all line object line offsets
 *      are not equal to a generic offset.
 *  2) not all adc connections have been made so advance the adcState and set
 *     the testState to CAL_ADC_SETUP.
 *  3) all adc connections have been made jump to primivie end.
 *----------------------------------------------------------------------------*/
static vpCalTestStateTypes
VpCalGetResult(
    Vp880DeviceObjectType *pDevObj,
    VpTestStatusType *pErrorCode,
    uint8 chanId,
    uint8 *pAdcState)
{
    vpCalTestStateTypes nextState;
    VpCalStoreResult(pDevObj, chanId, *pAdcState);

    if (CAL_MAX_ADC <= *pAdcState) {
        *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
        nextState = CAL_QUIT;

    } else if (CAL_NULL == *pAdcState) {
        if (!(pDevObj->stateInt & VP880_HAS_CALIBRATE_CIRCUIT)) {
            VpStoreAllOffsetsAsGeneric(pDevObj, chanId);
        }
        nextState = CAL_START_RESTORE;

    } else {
        nextState = CAL_ADC_SETUP;
        *pAdcState += 1;
    }

    return nextState;
}

/*------------------------------------------------------------------------------
 * VpCalStoreResult
 * This function is called by the VpCalAvg() function and stores the cal offset
 * into the device object.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  chanId          - channel that the offset belongs to
 *  adcState        - value of the current adc state
 *
 * Returns:
 * This function returns a pointer to the place where the adc connection
 * data can be stored. NULL is returned if an invalid calState is requested.
 *
 * Result:
 *  --
 *----------------------------------------------------------------------------*/
static void
VpCalStoreResult(
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState)
{
    switch (adcState) {
        case CAL_NULL:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_NO_CONNECT, &pDevObj->calOffsets[chanId].nullOffset);
            break;
        case CAL_VAB:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_METALLIC_DC_V, &pDevObj->calOffsets[chanId].vabOffset);
            break;
        case CAL_VAH:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_TIP_TO_GND_V, &pDevObj->calOffsets[chanId].vahOffset);
            break;
        case CAL_VBH:
            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP880_RING_TO_GND_V, &pDevObj->calOffsets[chanId].vbhOffset);
            break;
        default:
            return;
    }
    return;
}

/*------------------------------------------------------------------------------
 * VpStoreAllOffsets
 * This function is called by the VpCalAvg() function if the cal state is
 * generic.
 *
 * This function is used to store a generic offset into all line object offsets.
 *
 * Parameters:
 *  pLineObj        - pointer to the line object being tested.
 *  pDevObj         - pointer to the device object.
 *
 * Returns:
 *  TRUE - always for now.
 *
 * Result:
 *  Once this function is complete, all lineObject calibration offsets will
 *  contain the generic offset voltage from the null adc connection.
 *  This info canbe/is used by all other primatives to improve accuracy.
 *----------------------------------------------------------------------------*/
static bool
VpStoreAllOffsetsAsGeneric(
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId)
{
    Vp880CalOffCoeffs *pCalOffsets = &pDevObj->calOffsets[chanId];
    int16 genOffset = pCalOffsets->nullOffset;

    pCalOffsets->vabOffset = genOffset;
    pCalOffsets->vahOffset = genOffset;
    pCalOffsets->vbhOffset = genOffset;
    pCalOffsets->nullOffset = genOffset;

    return TRUE;
}

#endif /* VP880_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_880_SERIES */





