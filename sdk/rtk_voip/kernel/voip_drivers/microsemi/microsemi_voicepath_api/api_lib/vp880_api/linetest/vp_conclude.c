/*
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
#include "vp_api_testline_int.h"

typedef enum
{
    CONCLUDE_DISCONNECT     = VP880_TESTLINE_GLB_STRT_STATE,
    CONCLUDE_RESTORE_DEV    = 5,
    CONCLUDE_RESTORE_STATE  = 10,
    CONCLUDE_END            = VP880_TESTLINE_GLB_END_STATE,
    CONCLUDE_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpConcludeTestStateTypes;


/**
 * Vp880TestConclude
 * This fuction will check the following:
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
 */
VpStatusType
Vp880TestConclude(
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
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestConcludeType *pTest = (VpTestConcludeType*)pArgsUntyped;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    int16 *pTestState = &pTestInfo->testState;
    uint16 tick =  pDevObj->devProfileData.tickRate;

    uint8 chanId = pLineObj->channelId;
    /*uint16 tick =  pDevObj->devProfileData.tickRate;*/
    bool abortTest = pTest->abortTest;
    uint8 mpiBuffer[ VP880_BAT_CALIBRATION_LEN+1 +
                     VP880_OP_FUNC_LEN+1 +
                     VP880_OP_COND_LEN+1 +
                     VP880_SS_CONFIG_LEN+1 +
                     VP880_VP_GAIN_LEN+1 +
                     VP880_DISN_LEN+1 +
                     VP880_DC_FEED_LEN+1 +
                     VP880_LOOP_SUP_LEN+1 +
                     VP880_SIGA_PARAMS_LEN+1 +
                     VP880_REGULATOR_CTRL_LEN+1 +
                     VP880_B1_FILTER_LEN+1 +
                     VP880_B2_FILTER_LEN+1 +
                     VP880_ICR1_LEN+1 +
                     VP880_ICR2_LEN+1 +
                     VP880_ICR3_LEN+1 +
                     VP880_ICR4_LEN+1 +
                     VP880_ICR6_LEN+1 +
                     VP880_GEN_CTRL_LEN+1 +
                     VP880_REGULATOR_PARAM_LEN+1];
    uint8 mpiIndex = 0;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestConclude(ts:%i)", *pTestState));
    if (FALSE == callback) {
        /* Flip LED10 to the original state at the end of the test */
        #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
            VpSysServiceToggleLed(10);
        #endif

        if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
            /*
             * Something about calling Conclude caused an errorCode. Two reasons
             * are Line Not Prepared or calling conclude on an invalid line.
             */
            VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestConclude(ec:%i): bailed(1)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;
        }

        if ((0 <= *pTestState) && (FALSE == abortTest)) {
            /*
             * A test is already running and user called conclude
             * without the abort flag.
             */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_TESTING;
            VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestConclude(ec:%i): bailed(2)",
                 pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;

        } else if ((FALSE == abortTest) &&
            (TRUE == pDevObj->currentTest.concluding)) {
            /* In the process of concluding so don't conclude again */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_TESTING;
            VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestConclude(ec:%i): bailed(3)",
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
        if (pDevObj->currentTest.nonIntrusiveTest == FALSE) {
            pDevObj->currentTest.testState = CONCLUDE_DISCONNECT;
        } else {
            pDevObj->currentTest.testState = CONCLUDE_RESTORE_DEV;
        }
        pDevObj->currentTest.handle = handle;
        pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] =  1 | VP_ACTIVATE_TIMER;
        return VP_STATUS_SUCCESS;
    }

    switch (*pTestState) {
        case CONCLUDE_DISCONNECT: {
            uint8 state = VP880_SS_DISCONNECT;
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, &state);
            pLineObj->slicValueCache = state;

            VpSetTestStateAndTimer(pDevObj, pTestState, CONCLUDE_RESTORE_DEV,
                MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME_SHORT/5, tick));
            break;
        }

        case CONCLUDE_RESTORE_DEV:

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_BAT_CALIBRATION_WRT,
                VP880_BAT_CALIBRATION_LEN, pTestHeap->batCal);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_FUNC_WRT,
                VP880_OP_FUNC_LEN, &pTestHeap->opFunction);

            if (pLineObj->opCond[0] != pTestHeap->opCond) {
                pLineObj->opCond[0] = pTestHeap->opCond;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_COND_WRT,
                    VP880_OP_COND_LEN, pLineObj->opCond);
            }

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SS_CONFIG_WRT,
                VP880_SS_CONFIG_LEN, &pTestHeap->sysConfig);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_VP_GAIN_WRT,
                VP880_VP_GAIN_LEN, &pTestHeap->vpGain);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DISN_WRT,
                VP880_DISN_LEN, &pTestHeap->disn);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DC_FEED_WRT,
                VP880_DC_FEED_LEN, pTestHeap->dcFeed);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_LOOP_SUP_WRT,
                VP880_LOOP_SUP_LEN, pTestHeap->lpSuper);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SIGA_PARAMS_WRT,
                VP880_SIGA_PARAMS_LEN, pTestHeap->sigGenAB);

            if (pLineObj->internalTestTermApplied == FALSE) {
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR1_WRT,
                    VP880_ICR1_LEN, pTestHeap->icr1);
            }
            VpMemCpy(pLineObj->icr1Values, pTestHeap->icr1, VP880_ICR1_LEN);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_REGULATOR_CTRL_WRT,
                VP880_REGULATOR_CTRL_LEN, &pTestHeap->SwRegCtrl);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_B1_FILTER_WRT,
                VP880_B1_FILTER_LEN, pTestHeap->b1Filter);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_B2_FILTER_WRT,
                VP880_B2_FILTER_LEN, pTestHeap->b2Filter);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
                VP880_ICR2_LEN, pTestHeap->icr2);
            VpMemCpy(pLineObj->icr2Values, pTestHeap->icr2, VP880_ICR2_LEN);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR3_WRT,
                VP880_ICR3_LEN, pTestHeap->icr3);
            VpMemCpy(pLineObj->icr3Values, pTestHeap->icr3, VP880_ICR3_LEN);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR4_WRT,
                VP880_ICR4_LEN, pTestHeap->icr4);
            VpMemCpy(pLineObj->icr4Values, pTestHeap->icr4, VP880_ICR4_LEN);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR6_WRT,
                VP880_ICR6_LEN, pTestHeap->icr6);
            VpMemCpy(pLineObj->icr6Values, pTestHeap->icr6, VP880_ICR6_LEN);

            pLineObj->sigGenCtrl[0] = pTestHeap->sigCtrl;
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_GEN_CTRL_WRT,
                VP880_GEN_CTRL_LEN, pLineObj->sigGenCtrl);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP880_REGULATOR_PARAM_WRT, VP880_REGULATOR_PARAM_LEN,
                pTestHeap->switchReg);
            VpMemCpy(pDevObj->swParamsCache, pTestHeap->switchReg,
                VP880_REGULATOR_PARAM_LEN);

            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* let the device settle before generating event */
            if (pDevObj->currentTest.nonIntrusiveTest == FALSE) {
                VpSetTestStateAndTimer(pDevObj, pTestState,
                    CONCLUDE_RESTORE_STATE, MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME_SHORT / 5,tick));
            } else {
                VpSetTestStateAndTimer(pDevObj, pTestState, CONCLUDE_RESTORE_STATE, NEXT_TICK);
            }
            break;

        case CONCLUDE_RESTORE_STATE:
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, &pTestHeap->slacState);
            pLineObj->slicValueCache = pTestHeap->slacState;

            if (pDevObj->currentTest.nonIntrusiveTest == FALSE) {
                VpSetTestStateAndTimer(pDevObj, pTestState,
                    CONCLUDE_END, MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME_SHORT,tick));
            } else {
                VpSetTestStateAndTimer(pDevObj, pTestState, CONCLUDE_END, NEXT_TICK);
            }
            break;

        case CONCLUDE_END:

            /* conclude is complete */
            pDevObj->currentTest.concluding = FALSE;

            /* test can now be prepared */
            pTestInfo->prepared = FALSE;

            #define VP_API_VER_DP_FIX (15)
            if (VP_API_VERSION_MINOR_NUM >= VP_API_VER_DP_FIX) {
                /*
                 * Update the test buffer channel selection so that pulse
                 * detection will work
                 */
                Vp880UpdateBufferChanSel(pDevObj, chanId, pTestHeap->slacState, TRUE);
            }

            /* restore line event mask */
            pLineObj->lineEventsMask = pTestHeap->preTestEventMask;

            /* Generate the event */
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);

            /* Release the Test Heap */
            VpSysTestHeapRelease(pTestInfo->testHeapId);

            /*
             * Run logic that changes the low power mode as necessary. Third
             * argument "0" is not used if the last is FALSE
             */
            Vp880LLSetSysState(deviceId, pLineCtx, 0, FALSE);

#ifdef VP880_LP_SUPPORT
            /* Force an update on the line */
            Vp880LowPowerMode(pDevCtx);
#endif

            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestConclude(): complete"));
            pDevObj->currentTest.nonIntrusiveTest = FALSE;
            break;

        default:
            pDevObj->testResults.errorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestConclude(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
}

#endif
#endif





