/*
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2010-06-21 11:25:06 -0500 (Mon, 21 Jun 2010) $
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
#include "vp890_testline_int.h"

typedef enum
{
    CONCLUDE_DISCONNECT     = VP890_TESTLINE_GLB_STRT_STATE,
    CONCLUDE_RESTORE_DEV    = 5,
    CONCLUDE_RESTORE_STATE  = 10,
    CONCLUDE_END            = VP890_TESTLINE_GLB_END_STATE,
    CONCLUDE_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpConcludeTestStateTypes;

static VpStatusType
ConcludeFxs (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

static VpStatusType
ConcludeFxo (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);


/*******************************************************************************
 * Vp890TestConclude
 * Wrapper function for ConcludeFxs and ConcludeFxo
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the test's user inputs
 *  handle          - unique test handle
 *  callback        - indicates whether this is the first call to the test or a
 *                    timed callback
 *  testId          - Test ID for event generation
 *
 * Returns:
 * --
 ******************************************************************************/
VpStatusType
Vp890TestConclude (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;

    /* Flip LED10 to the original state at the end of the test */
    #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
        VpSysServiceToggleLed(10);
    #endif
    if (!(pLineObj->status & VP890_IS_FXO)) {
        return ConcludeFxs(pLineCtx, pArgsUntyped, handle, callback, testId);
    } else {
        return ConcludeFxo(pLineCtx, pArgsUntyped, handle, callback, testId);
    }
}

/*******************************************************************************
 * ConcludeFxs
 * This fuction will check the following for FXS lines:
 *
 * If there is a pending errorCode then the function will generate an event
 * indicating there was trouble and bail out without restoring any coeffs.
 *
 * If the abort argument is false while a different test primative is
 * already in progress the function will generate an event indicating the fact
 * and bail out without restoring any coeffs.
 *
 * If the abort flag is true or there are no errorCodes pending the
 * VP_DEV_TIMER_TESTLINE will be set to -1 to stop any pending callback,
 * all coeffs will be restored and an event will be generated indicating
 * all went well.
 *
 * Parameters:
 *  pLineCtx - pointer to the line context
 *  pTestInfo - pointer to the struct which contains test specfic info.
 *  handle   - unique test handle
 *
 * Returns:
 * --
 ******************************************************************************/
static VpStatusType
ConcludeFxs (
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
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestConcludeType *pTest = (VpTestConcludeType*)pArgsUntyped;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    int16 *pTestState = &pTestInfo->testState;

    /*uint16 tick =  pDevObj->devProfileData.tickRate;*/
    bool abortTest = pTest->abortTest;
    uint8 mpiBuffer[VP890_OP_FUNC_LEN+1 +
                    VP890_OP_COND_LEN+1 +
                    VP890_GEN_CTRL_LEN+1 +
                    VP890_SS_CONFIG_LEN+1 +
                    VP890_VP_GAIN_LEN+1 +
                    VP890_DISN_LEN+1 +
                    VP890_DC_FEED_LEN+1 +
                    VP890_B1_FILTER_LEN+1 +
                    VP890_B2_FILTER_LEN+1 +
                    VP890_ICR1_LEN+1 +
                    VP890_ICR2_LEN+1 +
                    VP890_ICR3_LEN+1 +
                    VP890_ICR4_LEN+1 +
                    VP890_DC_CAL_REG_LEN+1 +
                    VP890_LOOP_SUP_LEN+1 +
                    VP890_SIGA_PARAMS_LEN+1 +
                    VP890_REGULATOR_PARAM_LEN+1];
    uint8 mpiIndex = 0;
    uint16 tick =  pDevObj->devProfileData.tickRate;

    VP_TEST(VpLineCtxType, pLineCtx, ("ConcludeFxs(ts:%i)", *pTestState));
    if (FALSE == callback) {
        if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
            /*
             * Something about calling Conclude caused an errorCode. Two reasons
             * are Line Not Prepared or calling conclude on an invalid line.
             */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("ConcludeFxs(ec:%i): bailed(1)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;
        }

        if ((0 <= *pTestState) && (FALSE == abortTest)) {
            /*
             * A test is already running and user called conclude
             * without the abort flag.
             */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_TESTING;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("ConcludeFxs(ec:%i): bailed(2)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;

        } else if ((FALSE == abortTest) &&
            (TRUE == pDevObj->currentTest.concluding)) {
            /* In the process of concluding so don't conclude again */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_TESTING;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("ConcludeFxs(ec:%i): bailed(3)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;

        } else if ((TRUE == abortTest) && (0 <= *pTestState)) {
            /*
             * Test Conclude was called with the abort argument set.
             */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_ABORTED;
        }
        pDevObj->currentTest.concluding = TRUE;
        pDevObj->currentTest.testId = testId;
        pDevObj->currentTest.handle = handle;
        pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] =  1 | VP_ACTIVATE_TIMER;
        if (pDevObj->currentTest.nonIntrusiveTest == FALSE) {
            pDevObj->currentTest.testState = CONCLUDE_DISCONNECT;
        } else {
            pDevObj->currentTest.testState = CONCLUDE_RESTORE_DEV;
        }
        VP_TEST(VpLineCtxType, pLineCtx, ("ConcludeFxs(ts:%i)", *pTestState));
        return VP_STATUS_SUCCESS;
    }

    VP_TEST(VpLineCtxType, pLineCtx, ("ConcludeFxs(ts:%i)", *pTestState));
    switch (*pTestState) {
        case CONCLUDE_DISCONNECT: {
            uint8 state = VP890_SS_DISCONNECT;
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SYS_STATE_WRT,
                VP890_SYS_STATE_LEN, &state);
            pLineObj->slicValueCache = state;
            Vp890SetTestStateAndTimer(pDevObj, pTestState,
                CONCLUDE_RESTORE_DEV, MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME_SHORT/5,tick));
            VP_TEST(VpLineCtxType, pLineCtx, ("ConcludeFxs(ts:%i) disconnect", *pTestState));
            break;
        }

        case CONCLUDE_RESTORE_DEV:
            /* Restore registers as necessary for FXS tests */
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
                VP890_OP_FUNC_LEN, &pTestHeap->opFunction);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
                VP890_OP_COND_LEN, &pTestHeap->opCond);
            pLineObj->opCond[0] = pTestHeap->opCond;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SS_CONFIG_WRT,
                VP890_SS_CONFIG_LEN, &pTestHeap->sysConfig);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_VP_GAIN_WRT,
                VP890_VP_GAIN_LEN, &pTestHeap->vpGain);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DISN_WRT,
                VP890_DISN_LEN, &pTestHeap->disn);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
                VP890_DC_FEED_LEN, pTestHeap->dcFeed);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_LOOP_SUP_WRT,
                VP890_LOOP_SUP_LEN, pTestHeap->lpSuper);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SIGA_PARAMS_WRT,
                VP890_SIGA_PARAMS_LEN, pTestHeap->sigGenAB);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_B1_FILTER_WRT,
                VP890_B1_FILTER_LEN, pTestHeap->b1Filter);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_B2_FILTER_WRT,
                VP890_B2_FILTER_LEN, pTestHeap->b2Filter);

            if (pLineObj->internalTestTermApplied == FALSE) {
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR1_WRT,
                    VP890_ICR1_LEN, pTestHeap->icr1);
                VpMemCpy(pLineObj->icr1Values, pTestHeap->icr1, VP890_ICR1_LEN);
            }

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
                VP890_ICR2_LEN, pTestHeap->icr2);
            VpMemCpy(pLineObj->icr2Values, pTestHeap->icr2, VP890_ICR2_LEN);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
                VP890_ICR3_LEN, pTestHeap->icr3);
            VpMemCpy(pLineObj->icr3Values, pTestHeap->icr3, VP890_ICR3_LEN);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
                VP890_ICR4_LEN, pTestHeap->icr4);
            VpMemCpy(pLineObj->icr4Values, pTestHeap->icr4, VP890_ICR4_LEN);

            VpMemCpy(pLineObj->dcCalValues, pTestHeap->icr6, VP890_DC_CAL_REG_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_CAL_REG_WRT,
                VP890_DC_CAL_REG_LEN, pLineObj->dcCalValues);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_GEN_CTRL_WRT,
                VP890_GEN_CTRL_LEN, &pTestHeap->sigCtrl);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_REGULATOR_PARAM_WRT, VP890_REGULATOR_PARAM_LEN,
                pTestHeap->switchReg);
            VpMemCpy(pDevObj->swParamsCache, pTestHeap->switchReg,
                VP890_REGULATOR_PARAM_LEN);

            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* let the device settle before generating event */
            if (pDevObj->currentTest.nonIntrusiveTest == FALSE) {
                Vp890SetTestStateAndTimer(pDevObj, pTestState,
                    CONCLUDE_RESTORE_STATE, MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME_SHORT / 5,tick));
            } else {
                Vp890SetTestStateAndTimer(pDevObj, pTestState, CONCLUDE_RESTORE_STATE, NEXT_TICK);
            }
            break;

        case CONCLUDE_RESTORE_STATE:
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SYS_STATE_WRT,
                VP890_SYS_STATE_LEN, &pTestHeap->slacState);
            pLineObj->slicValueCache = pTestHeap->slacState;
            VP_INFO(VpDeviceIdType, &deviceId, ("testHeap->slacState: 0x%02X",pTestHeap->slacState));
            if (pDevObj->currentTest.nonIntrusiveTest == FALSE) {
                Vp890SetTestStateAndTimer(pDevObj, pTestState,
                    CONCLUDE_END, MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME_SHORT,tick));
            } else {
                Vp890SetTestStateAndTimer(pDevObj, pTestState, CONCLUDE_END, NEXT_TICK);
            }

            break;

        case CONCLUDE_END:
            /* conclude is complete */
            pDevObj->currentTest.concluding = FALSE;

            /* test can now be prepared */
            pTestInfo->prepared = FALSE;

            /* restore line event mask */
            pLineObj->lineEventsMask = pTestHeap->preTestEventMask;

            /* Generate the event */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);

            /* Release the Test Heap */
            VpSysTestHeapRelease(pTestInfo->testHeapId);

            /*
             * Run logic that changes the low power mode as necessary. Third
             * argument "0" is not used if the last is FALSE
             */
            Vp890LLSetSysState(deviceId, pLineCtx, 0, FALSE);

#ifdef VP890_LP_SUPPORT
            /* Force an update on the line */
            Vp890LowPowerMode(pDevCtx);
#endif

            VP_TEST(VpLineCtxType, pLineCtx, ("ConcludeFxs(): complete"));
            pDevObj->currentTest.nonIntrusiveTest = FALSE;
            break;

        default:
            pDevObj->testResults.errorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("ConcludeFxs(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
} /* ConcludeFxs() */

/*******************************************************************************
 * ConcludeFxo
 * This fuction will check the following for FXO lines:
 *
 * If there is a pending errorCode then the function will generate an event
 * indicating there was trouble and bail out without restoring any coeffs.
 *
 * If the abort argument is false while a different test primative is
 * already in progress the function will generate an event indicating the fact
 * and bail out without restoring any coeffs.
 *
 * If the abort flag is true or there are no errorCodes pending the
 * VP_DEV_TIMER_TESTLINE will be set to -1 to stop any pending callback,
 * all coeffs will be restored and an event will be generated indicating
 * all went well.
 *
 * Parameters:
 *  pLineCtx - pointer to the line context
 *  pTestInfo - pointer to the struct which contains test specfic info.
 *  handle   - unique test handle
 *
 * Returns:
 * --
 ******************************************************************************/
static VpStatusType
ConcludeFxo (
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
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestConcludeType *pTest = (VpTestConcludeType*)pArgsUntyped;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    int16 *pTestState = &pTestInfo->testState;

    /*uint16 tick =  pDevObj->devProfileData.tickRate;*/
    bool abortTest = pTest->abortTest;
    uint8 mpiBuffer[VP890_OP_FUNC_LEN   + 1 +
                    VP890_OP_COND_LEN   + 1 +
                    VP890_CONV_CFG_LEN  + 1 +
                    VP890_OP_MODE_LEN   + 1 ];
    uint8 mpiIndex = 0;
    uint16 tick = pDevObj->devProfileData.tickRate;

    VP_TEST(VpLineCtxType, pLineCtx, ("ConcludeFxo(ts:%i)", *pTestState));
    if (FALSE == callback) {
        if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
            /*
             * Something about calling Conclude caused an errorCode. Two reasons
             * are Line Not Prepared or calling conclude on an invalid line.
             */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("ConcludeFxo(ec:%i): bailed(1)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;
        }

        if ((0 <= *pTestState) && (FALSE == abortTest)) {
            /*
             * A test is already running and user called conclude
             * without the abort flag.
             */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_TESTING;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("ConcludeFxo(ec:%i): bailed(2)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;

        } else if ((FALSE == abortTest) &&
            (TRUE == pDevObj->currentTest.concluding)) {
            /* In the process of concluding so don't conclude again */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_TESTING;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("ConcludeFxo(ec:%i): bailed(3)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;

        } else if ((TRUE == abortTest) && (0 <= *pTestState)) {
            /*
             * Test Conclude was called with the abort argument set.
             */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_ABORTED;
        }
        pDevObj->currentTest.concluding = TRUE;
        pDevObj->currentTest.testId = testId;
        pDevObj->currentTest.testState = CONCLUDE_DISCONNECT;
        pDevObj->currentTest.handle = handle;
        pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] =  1 | VP_ACTIVATE_TIMER;
        return VP_STATUS_SUCCESS;
    }

    switch (*pTestState) {
        case CONCLUDE_DISCONNECT:
            /* For FXO we don't want to change the state, so fall through here */
        case CONCLUDE_RESTORE_DEV:
            /* Restore registers as necessary for FXO tests */
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
                VP890_OP_FUNC_LEN, &pTestHeap->opFunction);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
                VP890_OP_COND_LEN, &pTestHeap->opCond);
            pLineObj->opCond[0] = pTestHeap->opCond;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_CONV_CFG_WRT,
                VP890_CONV_CFG_LEN, &pTestHeap->convCfg);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_MODE_WRT,
                VP890_OP_MODE_LEN, &pTestHeap->opMode);

            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* let the device settle before generating event */
            Vp890SetTestStateAndTimer(pDevObj, pTestState,
                CONCLUDE_END, MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME_SHORT/5,tick));
            break;

        case CONCLUDE_END:

            /* conclude is complete */
            pDevObj->currentTest.concluding = FALSE;

            /* test can now be prepared */
            pTestInfo->prepared = FALSE;

            /* restore line event mask */
            pLineObj->lineEventsMask = pTestHeap->preTestEventMask;

            /* Generate the event */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);

            /* Release the Test Heap */
            VpSysTestHeapRelease(pTestInfo->testHeapId);

            /*
             * Run logic that changes the low power mode as necessary. Third
             * argument "0" is not used if the last is FALSE
             */
            Vp890LLSetSysState(deviceId, pLineCtx, 0, FALSE);

            VP_TEST(VpLineCtxType, pLineCtx, ("ConcludeFxo(): complete"));
            break;

        default:
            pDevObj->testResults.errorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("ConcludeFxo(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
} /* ConcludeFxo() */



#endif
#endif





