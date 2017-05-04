/** file lt_api_vp890_loopback.c
 *
 *  This file contains the loop back algrothim
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2010-10-04 18:53:18 -0500 (Mon, 04 Oct 2010) $
 */


#include "vp_api_cfg.h"

#if defined (VP_CC_890_SERIES) && defined (VP890_INCLUDE_TESTLINE_CODE)
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
    LOOPBACK_SETUP          = VP890_TESTLINE_GLB_STRT_STATE,
    LOOPBACK_DAC_SETUP      = 5,
    LOOPBACK_COLLECT_DATA   = 10,
    LOOPBACK_END            = VP890_TESTLINE_GLB_END_STATE,
    LOOPBACK_QUIT           = VP890_TESTLINE_GLB_QUIT_STATE,
    LOOPBACK_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpLoopbackTestStateTypes;


static VpStatusType
VpLoopbackArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpLoopbackSetup(
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

/*------------------------------------------------------------------------------
 * Vp890TestLoopback
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
Vp890TestLoopback(
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
    VpTestLoopbackType *pLoopbackData = &(pTestHeap->testArgs).loopback;
    Vp890LineTestCalType *pCalRegs = &pTestHeap->calRegs;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint8 *hookStatus = &pTestHeap->loopCnt;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestLoopback(ts:%i)", *pTestState));
    if (FALSE == callback) {
        /* Store the current line state */
        pCalRegs->slacState = pLineObj->slicValueCache;

        /* Store the current hook status */
        *hookStatus = pDevObj->intReg[chanId] & VP890_HOOK_MASK;

        return VpLoopbackArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    } else {
        /* Abort if the line state changes during the test */
        /* Abort if an off-hook was detected during the test */
        if (((pCalRegs->slacState & 0x0F) != (pLineObj->slicValueCache & 0x0F)) ||
            ((pDevObj->intReg[chanId] & VP890_HOOK_MASK) != *hookStatus)) {
            *pErrorCode = VP_TEST_STATUS_ABORTED;
            *pTestState = LOOPBACK_QUIT;
        }
    }

    switch (*pTestState) {

        case LOOPBACK_SETUP:
            VpLoopbackSetup(pLineObj, pTestHeap, deviceId);

            Vp890SetTestStateAndTimer(pDevObj, pTestState, LOOPBACK_DAC_SETUP,
                MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME, tick));
            break;

        case LOOPBACK_DAC_SETUP:
            if (VP_LOOPBACK_TEST_CODEC == pTestHeap->testArgs.loopback.loopback) {
                Vp890AdcSetup(pLineCtx, VP890_VOICE_DAC, FALSE);
                Vp890SetTestStateAndTimer(pDevObj, pTestState, LOOPBACK_COLLECT_DATA,
                    MS_TO_TICKRATE(VP890_ADC_CHANGE_SETTLING_TIME, tick));
                break;
            }

            if (VP_LOOPBACK_TEST_BFILTER == pTestHeap->testArgs.loopback.loopback) {
                Vp890AdcSetup(pLineCtx, VP890_NO_CONNECT, FALSE);
                Vp890SetTestStateAndTimer(pDevObj, pTestState, LOOPBACK_COLLECT_DATA,
                    MS_TO_TICKRATE(VP890_ADC_CHANGE_SETTLING_TIME, tick));
                break;
            }

            *pTestState = LOOPBACK_COLLECT_DATA;
            /* no break */

        case LOOPBACK_COLLECT_DATA:
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_APP_SPECIFIC,
                LOOPBACK_END, pLoopbackData->waitTime, pLoopbackData->loopbackTime);
            break;

        case LOOPBACK_END:
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_APP_SPECIFIC, FALSE, FALSE, chanId,
                VP890_NO_CONNECT, pDevObj->testResults.result.loopback.pApplicationInfo);

            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestLoopback(): complete"));
            break;

        case LOOPBACK_QUIT:
            /* The test has eneded early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestLoopback(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestLoopback(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpLoopbackArgCheck
 * This functions is called by the Vp890TestLoopback() function if
 * Vp890TestLoopback() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the Get Loopback primitive are legal. The function also
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
static VpStatusType
VpLoopbackArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTestLoopbackType *pTestInput = (VpTestLoopbackType*)pArgsUntyped;
    VpTestLoopbackType *pLoopbackData = &(pTestInfo->pTestHeap->testArgs).loopback;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpLoopbackArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */
    pDevObj->testResults.result.loopback.pApplicationInfo = VP_NULL;
    pDevObj->testResults.result.loopback.loopback = pTestInput->loopback;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* Check input arguments before assigining them to test struct */
    if (((VP_LOOPBACK_TEST_CODEC | VP_LOOPBACK_TEST_ANALOG | VP_LOOPBACK_TEST_BFILTER |
        VP_LOOPBACK_TEST_TIMESLOT) & pTestInput->loopback) == 0) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpLoopbackArgCheck(): bailed invalid loopback"));
        return VP_STATUS_INVALID_ARG;
    }

    pLoopbackData->waitTime = pTestInput->waitTime;
    pLoopbackData->loopbackTime = pTestInput->loopbackTime;
    pLoopbackData->loopback = pTestInput->loopback;

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, LOOPBACK_SETUP, NEXT_TICK);

    return VP_STATUS_SUCCESS;
}


/*------------------------------------------------------------------------------
 * VpLoopbackSetup
 * This function is called by the Vp890TestLoopback() state machine during
 * the LOOPBACK_SETUP state.
 *
 * This function is used to configure the line for the loop back test.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *
 * Returns:
 * --
 * Result:
 *  Disable AISN
 *----------------------------------------------------------------------------*/
static void
VpLoopbackSetup(
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    uint8 opCond = (VP890_TXPATH_EN | VP890_RXPATH_EN);

    /* Enable paths for testing */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_COND_WRT, VP890_OP_COND_LEN, &opCond);
    pLineObj->opCond[0] = opCond;

    /* If the loopback is code shut down all filters */
    if (VP_LOOPBACK_TEST_CODEC == pTestHeap->testArgs.loopback.loopback) {
        uint8 disn = 0x00;
        uint8 opFunction;

        /* DISN = 0 */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DISN_WRT, VP890_DISN_LEN, &disn);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_FUNC_RD, VP890_OP_FUNC_LEN, &opFunction);
        opFunction &= ~VP890_ENABLE_LOADED_COEFFICIENTS;
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_FUNC_WRT, VP890_OP_FUNC_LEN, &opFunction);
    }

    if (VP_LOOPBACK_TEST_BFILTER == pTestHeap->testArgs.loopback.loopback) {
        uint8 mpiBuffer[VP890_ICR4_LEN+1 +
                        VP890_B1_FILTER_LEN+1 +
                        VP890_B2_FILTER_LEN+1 +
                        VP890_OP_FUNC_LEN+1];
        uint8 mpiIndex = 0;
        uint8 b1Filter[] =
            {0xBA, 0x3D, 0x21, 0xBA, 0x0B, 0xA2, 0x29, 0x72, 0x9F, 0x39, 0xF3, 0x9F, 0xC9, 0xF0};
        uint8 b2Filter[] = {0x2E, 0x01};
        uint8 opFunc = 0x33;

        pLineObj->icr4Values[0] |= 0x02;
        pLineObj->icr4Values[1] &= 0xFD;

        /* Disable the DAC output */
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
            VP890_ICR4_LEN, pLineObj->icr4Values);

        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_B1_FILTER_WRT,
            VP890_B1_FILTER_LEN, b1Filter);

        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_B2_FILTER_WRT,
            VP890_B2_FILTER_LEN, b2Filter);

        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
            VP890_OP_FUNC_LEN, &opFunc);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);
    }

    if (VP_LOOPBACK_TEST_TIMESLOT == pTestHeap->testArgs.loopback.loopback) {
        uint8 mpiBuffer[VP890_ICR4_LEN+1 +
                        VP890_OP_COND_LEN+1];
        uint8 mpiIndex = 0;

        pLineObj->icr4Values[0] |= 0x02;
        pLineObj->icr4Values[1] &= 0xFD;

        opCond =
            (VP890_TXPATH_EN | VP890_RXPATH_EN | VP890_HIGH_PASS_DIS | VP890_INTERFACE_LOOPBACK_EN);

        /* Disable the DAC output */
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
            VP890_ICR4_LEN, pLineObj->icr4Values);

        /* Enable paths for testing */
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
            VP890_OP_COND_LEN, &opCond);
        pLineObj->opCond[0] = opCond;

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);
    }

    /* Enable the Codec */
    pLineObj->slicValueCache |= VP890_SS_ACTIVATE_MASK;

    /* go to active if in anlog loopback */
    if (VP_LOOPBACK_TEST_ANALOG == pTestHeap->testArgs.loopback.loopback) {
        pLineObj->slicValueCache &= ~VP890_SS_LINE_FEED_MASK;
        pLineObj->slicValueCache |= VP890_SS_ACTIVE;
    }

    /* write the new state */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SYS_STATE_WRT,
        VP890_SYS_STATE_LEN, &pLineObj->slicValueCache);
    return;
}

#endif /* VP890_INCLUDE_LINE_TEST_PACKAGE */
#endif /* VP_CC_890_SERIES */





