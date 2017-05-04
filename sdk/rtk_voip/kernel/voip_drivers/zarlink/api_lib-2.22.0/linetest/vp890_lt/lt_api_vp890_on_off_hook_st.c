/** \lt_api_test_on_off_hook_st.h
 * lt_api_test_on_off_hook_st.h
 *
 * This file implements the on/off hook self test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_ON_OFF_HOOK_ST)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    HOOK_ST_INIT            = LT_GLOBAL_FIRST_STATE,
    HOOK_ST_CHECK_FOR_ABORT_HOOK,
    HOOK_ST_APPLY_TEST_SWITCH,
    HOOK_ST_CHECK_FOR_OFF_HOOK,
    HOOK_ST_REMOVE_TEST_SWITCH,
    HOOK_ST_CHECK_FOR_ON_HOOK,
    HOOK_ST_CONCLUDE,
    HOOK_ST_COMPLETE,
    HOOK_ST_ABORT          = LT_GLOBAL_ABORTED_STATE,
    HOOK_ST_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890OnOffHookStates;

static LtTestStatusType
LtVp890OnOffHookConclude(
    LtTestCtxType *pTestCtx);

extern LtTestStatusType
LtVp890OnOffHookStHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtOnOffHookSTResultType *pResults = &pTestCtx->pResult->result.onOffHookST;
    int *pState = &(pTestCtx->state);
    bool rawHook;

    LT_DOUT(LT_DBG_INFO, ("LtVp890OnOffHookStHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case HOOK_ST_INIT:
            pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE;

            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /*
             * The power term type hook detection algorithim takes ~300ms
             * to start working when transitioning from standby to active.
             */
            if ((pTemp890->termType == VP_TERM_FXS_LOW_PWR) ||
                (pTemp890->termType == VP_TERM_FXS_SPLITTER_LP) ||
                (pTemp890->termType == VP_TERM_FXS_ISOLATE_LP)) {
                pTemp890->testTimer.timerVal = (LT_VP890_STATE_CHANGE_SETTLE * 3);
            }

            *pState = HOOK_ST_CHECK_FOR_ABORT_HOOK;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);
            break;


        case HOOK_ST_CHECK_FOR_ABORT_HOOK:
            /* Check abort hook bit and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &rawHook)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* If the device raw hook bit is true (off-hook) then test is aborted */
            if (TRUE == rawHook) {
                pResults->fltMask = LT_ON_OFF_HOOK_STM_TEST_ABORTED;
                retval = LtVp890OnOffHookConclude(pTestCtx);
                break;
            }
            /* no break if no off-hook */

        case HOOK_ST_APPLY_TEST_SWITCH:
            /* Throw Test switch and wait for hook debounce */
            pTemp890->testTimer.timerVal = LT_VP890_TEST_SWITCH_SETTLE;

            if (LT_STATUS_RUNNING !=
                Vp890SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST)) {
                return retval;
            }
            *pState = HOOK_ST_CHECK_FOR_OFF_HOOK;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);

            break;

        case HOOK_ST_CHECK_FOR_OFF_HOOK:
            /* Check for hook bit */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &rawHook)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            /* If the device raw hook bit is not TRUE (off-hook), test failed */
            if (TRUE != rawHook) {
                pResults->fltMask = LT_ON_OFF_HOOK_STM_TEST_HW_FAULT;
                retval = LtVp890OnOffHookConclude(pTestCtx);
                LT_DOUT(LT_DBG_INFO, ("LtVp890OnOffHookStHandler(rawHook:FALSE)" \
                    "should be off-hook"));
                break;
            }

            /* no break needed if off-hook found*/

        case HOOK_ST_REMOVE_TEST_SWITCH:
            /* Remove Test switch and wait for hook debounce */
            if (LT_STATUS_RUNNING !=
                Vp890SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL)) {
                return retval;
            }
            *pState = HOOK_ST_CHECK_FOR_ON_HOOK;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);

            break;

        case HOOK_ST_CHECK_FOR_ON_HOOK:
            /* Check for hook bit */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &rawHook)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            /* If the device raw hook bit is not FALSE (on-hook), test failed */
            if (FALSE != rawHook) {
                pResults->fltMask = LT_ON_OFF_HOOK_STM_TEST_HW_FAULT;
                LT_DOUT(LT_DBG_INFO, ("LtVp890OnOffHookStHandler(rawHook:TRUE)" \
                    "should be on-hook"));
            } else {
                pResults->fltMask = LT_TEST_PASSED;
            }

            /* no break needed */

        case HOOK_ST_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                retval = LtVp890OnOffHookConclude(pTestCtx);
                break;
            }

        case HOOK_ST_COMPLETE:
            retval = LT_STATUS_DONE;
            break;

        case HOOK_ST_ABORT:
            /* Test aborted, results may indicate source of error. */
            retval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            retval = LT_STATUS_ERROR_UNKNOWN;
            break;
     }

    return retval;
}

static LtTestStatusType
LtVp890OnOffHookConclude(
    LtTestCtxType *pTestCtx)
{
    VpTestConcludeType conclude = { FALSE };

    pTestCtx->state = HOOK_ST_COMPLETE;
    Vp890SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);

    /* Check the Hook Bit and Conclude the test*/
    return Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);

}
#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
