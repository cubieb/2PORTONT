/** file vp_calibrate.c
 *
 *  This file contains the functions used in the Vp890 Test Line.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2011-04-19 10:29:19 -0500 (Tue, 19 Apr 2011) $
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

#define VP890_CALIBRATE_SETTLE 40
#define VP890_CALIBRATE_SAMPLES 40

typedef enum
{
    CAL_SAVE_STATE          = VP890_TESTLINE_GLB_STRT_STATE,
    CAL_SETUP               = 5,
    CAL_ADC_SETUP           = 10,
    CAL_COLLECT_DATA        = 15,
    CAL_GET_RESULT          = 20,
    CAL_START_RESTORE       = 30,
    CAL_RESTORE_LINE_STATE  = 40,
    CAL_END                 = VP890_TESTLINE_GLB_END_STATE,
    CAL_QUIT                = VP890_TESTLINE_GLB_QUIT_STATE,
    VP_CAL_TEST_STATE_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpCalTestStateTypes;

typedef enum
{
    CAL_VAH,
    CAL_VBH,
    CAL_VAB,
    CAL_BAT,
    CAL_NULL,
    CAL_MAX_ADC,
    VP_CAL_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpCalAdcOrderTypes;

static VpStatusType
VpCalArgCheck(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpCalSetup(
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId,
    uint8 adcState);

static void
VpCalRestore(
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static bool
VpCalAdcSetup(
    VpLineCtxType *pLineCtx,
    Vp890DeviceObjectType *pDevObj,
    uint8 adcState);

static vpCalTestStateTypes
VpCalGetResult(
    Vp890DeviceObjectType *pDevObj,
    VpTestStatusType *pErrorCode,
    uint8 chanId,
    uint8 *pAdcState);

static void
VpCalStoreResult(
    Vp890DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState);

/*------------------------------------------------------------------------------
 * Vp890TestCalibrate
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
EXTERN VpStatusType
Vp890TestCalibrate(
    VpLineCtxType *pLineCtx,
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

    uint8 *pAdcState = &pTestHeap->adcState;
    int16 *pTestState = &pTestInfo->testState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return VpCalArgCheck(pLineCtx, handle, pAdcState, testId);
    }

    switch (*pTestState) {
        case CAL_SAVE_STATE:
            /* no break */

        case CAL_SETUP:
            /* prep the line */
            VpCalSetup(pLineObj, pTestHeap, deviceId, *pAdcState);

            /* Allow circuits to settle */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, CAL_ADC_SETUP,
                MS_TO_TICKRATE(VP890_FEED_DISCHARGE_TIME, tick));
            break;

        case CAL_ADC_SETUP:
            /* setup the ADC for the current calState*/
            if ( TRUE == VpCalAdcSetup(pLineCtx, pDevObj, *pAdcState) ) {
                Vp890SetTestStateAndTimer(pDevObj, pTestState, CAL_COLLECT_DATA,
                    MS_TO_TICKRATE(10, tick));
            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                Vp890SetTestStateAndTimer(pDevObj, pTestState, CAL_QUIT, NEXT_TICK);
            }
            break;

        case CAL_COLLECT_DATA:
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE, CAL_GET_RESULT,
                VP890_CALIBRATE_SETTLE, VP890_CALIBRATE_SAMPLES);
            break;

        case CAL_GET_RESULT:
            *pTestState = VpCalGetResult(pDevObj, pErrorCode, chanId, pAdcState);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;

        case CAL_START_RESTORE:
            /* restore coeffs */
             VpCalRestore(pLineObj, pTestHeap, deviceId);
             Vp890SetTestStateAndTimer(pDevObj, pTestState, CAL_RESTORE_LINE_STATE,
                NEXT_TICK);
            break;

        case CAL_RESTORE_LINE_STATE:
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SLIC_STATE_WRT,
                VP890_SLIC_STATE_LEN, &pTestHeap->calRegs.slacState);
            pLineObj->slicValueCache = pTestHeap->calRegs.slacState;

        case CAL_END:
            /* restore coeffs */
            VpCalRestore(pLineObj, pTestHeap, deviceId);

            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            pDevObj->testResults.result.calFailed = FALSE;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate(): complete"));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate() nullOffset: %d",
                pDevObj->calOffsets[chanId].nullOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate() vabOffset: %d",
                pDevObj->calOffsets[chanId].vabOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate() vahOffset: %d",
                pDevObj->calOffsets[chanId].vahOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate() vbhOffset: %d",
                pDevObj->calOffsets[chanId].vbhOffset));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate() batOffset: %d",
                pDevObj->calOffsets[chanId].batOffset));
            break;

        case CAL_QUIT:
            /* restore icr6 */
            VpCalRestore(pLineObj, pTestHeap, deviceId);

            /* The test has ended early due to the current errorCode */
            pDevObj->testResults.result.calFailed = TRUE;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate(ec:%i): quit",
                 *pErrorCode));

            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            pDevObj->testResults.result.calFailed = TRUE;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestCalibrate(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpOpenVdcArgCheck
 * This functions is called by the Vp890TestCalibrate() function if
 * Vp890TestCalibrate() was not called via a callback.
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
static VpStatusType
VpCalArgCheck(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpCalArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    *pAdcState = CAL_VAH;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpCalSetup
 * This function is called by the Vp890TestCalibrate() state machine during
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
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId,
    uint8 adcState)
{
    Vp890LineTestCalType *pCalRegs = &pTestHeap->calRegs;
    uint8 disn = 00;
    uint8 opCond = ( VP890_RXPATH_DIS | VP890_HIGH_PASS_DIS );
    uint8 opFunction = VP890_LINEAR_CODEC;
    uint8 slacState = VP890_SS_DISCONNECT | VP890_SS_ACTIVATE_MASK;

    uint8 mpiBuffer[VP890_SLIC_STATE_LEN+1 +
                     VP890_DISN_LEN+1 +
                     VP890_OP_COND_LEN+1 +
                     VP890_OP_FUNC_LEN+1 +
                     VP890_ICR2_LEN+1 +
                     VP890_ICR3_LEN+1 +
                     VP890_ICR4_LEN+1 +
                     VP890_DC_CAL_REG_LEN+1];
    uint8 mpiIndex = 0;

    /* store all of the register that will be changed by cal */
    pCalRegs->slacState = pLineObj->slicValueCache;
    pCalRegs->opCond = pLineObj->opCond[0];

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_FUNC_RD,
        VP890_OP_FUNC_LEN, &pCalRegs->opFunction);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_VP_GAIN_RD,
        VP890_VP_GAIN_LEN, &pCalRegs->vpGain);

    VpMemCpy(pCalRegs->icr2, pLineObj->icr2Values, VP890_ICR2_LEN);
    VpMemCpy(pCalRegs->icr3, pLineObj->icr3Values, VP890_ICR3_LEN);
    VpMemCpy(pCalRegs->icr4, pLineObj->icr4Values, VP890_ICR4_LEN);
    VpMemCpy(pCalRegs->icr6, pLineObj->dcCalValues, VP890_DC_CAL_REG_LEN);

    if (CAL_NULL != adcState) {
        pLineObj->dcCalValues[1] |= (VP890_C_YBAT_SNS_CUT
                                   | VP890_C_RING_SNS_CUT
                                   | VP890_C_TIP_SNS_CUT);
    }

    pLineObj->icr2Values[0] = 0x0C;
    pLineObj->icr2Values[1] = 0x0C;
    pLineObj->icr2Values[2] &= 0x0C;
    pLineObj->icr2Values[2] |= 0x20; /* disable battery */
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
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SLIC_STATE_WRT,
        VP890_SLIC_STATE_LEN, &slacState);
    pLineObj->slicValueCache = slacState;

    /* Remove disn */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DISN_WRT,
        VP890_DISN_LEN, &disn);

    /* Disable hi-pass filter and cutoff receive path */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
        VP890_OP_COND_LEN, &opCond);
    pLineObj->opCond[0] = opCond;

    /* Set to read linear data */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
        VP890_OP_FUNC_LEN, &opFunction);

    /* Allow VOC, A/B sense on*/
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
        VP890_ICR2_LEN, pLineObj->icr2Values);

    /* Force line control circuits on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
        VP890_ICR3_LEN, pLineObj->icr3Values);

    /* Force ADC on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
        VP890_ICR4_LEN, pLineObj->icr4Values);

    /* Connect null calibration input */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_CAL_REG_WRT,
         VP890_DC_CAL_REG_LEN, pLineObj->dcCalValues);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * VpCalRestore
 * This function is called by the Vp890TestCalibrate() state machine during
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
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    Vp890LineTestCalType *pCalRegs = &pTestHeap->calRegs;

    uint8 mpiBuffer[ VP890_VP_GAIN_LEN+1 +
                     VP890_OP_COND_LEN+1 +
                     VP890_OP_FUNC_LEN+1 +
                     VP890_ICR2_LEN+1 +
                     VP890_ICR3_LEN+1 +
                     VP890_ICR4_LEN+1 +
                     VP890_DC_CAL_REG_LEN+1 +
                     VP890_VP_GAIN_LEN+1];
    uint8 mpiIndex = 0;

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_VP_GAIN_WRT,
        VP890_VP_GAIN_LEN, &pCalRegs->vpGain);

    if (pLineObj->opCond[0] != pCalRegs->opCond) {
        pLineObj->opCond[0] = pCalRegs->opCond;
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
            VP890_OP_COND_LEN, &pCalRegs->opCond);
    }

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
        VP890_OP_FUNC_LEN, &pCalRegs->opFunction);

    if (VpMemCpyCheck(pLineObj->icr2Values, pCalRegs->icr2, VP890_ICR2_LEN) == TRUE) {
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
            VP890_ICR2_LEN, pLineObj->icr2Values);
    }

    if (VpMemCpyCheck(pLineObj->icr3Values, pCalRegs->icr3, VP890_ICR3_LEN) == TRUE) {
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
            VP890_ICR3_LEN, pCalRegs->icr3);
    }

    if (VpMemCpyCheck(pLineObj->icr4Values, pCalRegs->icr4, VP890_ICR4_LEN) == TRUE) {
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
            VP890_ICR4_LEN, pCalRegs->icr4);
    }

    if (VpMemCpyCheck(pLineObj->dcCalValues, pCalRegs->icr6, VP890_DC_CAL_REG_LEN) == TRUE) {
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_CAL_REG_WRT,
            VP890_DC_CAL_REG_LEN, pLineObj->dcCalValues);
    }

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_VP_GAIN_WRT,
         VP890_VP_GAIN_LEN, &pCalRegs->vpGain);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * VpCalAdcSetup
 * This function is called by the Vp890TestCalibrate() state machine during
 * the CAL_ADC_SETUP state.
 *
 * This function is used to call the Vp890AdcSetup() depending on the
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
    Vp890DeviceObjectType *pDevObj,
    uint8 adcState)
{
    bool adcFlag;

    switch (adcState) {
        case CAL_NULL:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_NO_CONNECT, FALSE);
            break;
        case CAL_VAB:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_V, FALSE);
            break;
        case CAL_VAH:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_TIP_TO_GND_V, FALSE);
            break;
        case CAL_VBH:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_RING_TO_GND_V, FALSE);
            break;
        case CAL_BAT:
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_SWITCHER_Y, FALSE);
            break;
        default:
            return FALSE;
    }

    return adcFlag;
}

/*------------------------------------------------------------------------------
 * VpCalGetResult
 * This function is called by the Vp890TestCalibrate() state machine during
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
    Vp890DeviceObjectType *pDevObj,
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
    Vp890DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState)
{
    switch (adcState) {
        case CAL_NULL:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP890_NO_CONNECT, &pDevObj->calOffsets[chanId].nullOffset);
            break;
        case CAL_VAB:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP890_METALLIC_DC_V, &pDevObj->calOffsets[chanId].vabOffset);
            break;
        case CAL_VAH:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP890_TIP_TO_GND_V, &pDevObj->calOffsets[chanId].vahOffset);
                /* We need to adjust 1.5V for the TDC and RDC pin input offset
                 * controls which are present for 880 but not 890. */
                pDevObj->calOffsets[chanId].vahOffset += 205;
            break;
        case CAL_VBH:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP890_RING_TO_GND_V, &pDevObj->calOffsets[chanId].vbhOffset);
                /* We need to adjust 1.5V for the TDC and RDC pin input offset
                 * controls which are present for 880 but not 890. */
                pDevObj->calOffsets[chanId].vbhOffset += 205;
            break;
        case CAL_BAT:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, chanId,
                VP890_SWITCHER_Y, &pDevObj->calOffsets[chanId].batOffset);
            break;
        default:
            return;
    }
    return;
}

#endif /* VP890_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_890_SERIES */
