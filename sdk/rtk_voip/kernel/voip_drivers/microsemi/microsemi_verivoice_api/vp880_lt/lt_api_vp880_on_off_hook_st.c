/** \lt_api_test_on_off_hook_st.h
 * lt_api_test_on_off_hook_st.h
 *
 * This file implements the on/off hook self test.
 *
 * This file assumes a certain include order to compile.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_ON_OFF_HOOK_ST)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

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
} LtVp880OnOffHookStates;

static LtTestStatusType
LtVp880OnOffHookConclude(
    LtTestCtxType *pTestCtx);

extern LtTestStatusType
LtVp880OnOffHookStHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtOnOffHookSTResultType *pResults = &pTestCtx->pResult->result.onOffHookST;
    int *pState = &(pTestCtx->state);
    bool rawHook;

    LT_DOUT(LT_DBG_INFO, ("LtVp880OnOffHookStHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case HOOK_ST_INIT: {
            VpDeviceInfoType devInfo;
            VpStatusType vpStatus;

            devInfo.pLineCtx = pTestCtx->pLineCtx;
            vpStatus = VpGetDeviceInfo(&devInfo);
            if (VP_STATUS_SUCCESS != vpStatus) {
                pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
                LT_DOUT(LT_DBG_ERRR, ("LtVp880RingingStHandler(testState:%i,vpStatus%i)",
                    *pState, vpStatus));
                return LT_STATUS_ERROR_VP_GENERAL;
            }
            if (devInfo.featureList.testLoadSwitch != VP_AVAILABLE &&
                devInfo.featureList.internalTestTermination != VP_AVAILABLE) {
                /* If the device does not have the test load switch and cannot
                 * support the internal test termination, this test can not
                 * run */
                return LT_STATUS_TEST_NOT_SUPPORTED;
            }

            pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE;

            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            *pState = HOOK_ST_CHECK_FOR_ABORT_HOOK;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            break;
        }

        case HOOK_ST_CHECK_FOR_ABORT_HOOK:
            /* Check abort hook bit and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &rawHook)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* If the device raw hook bit is true (off-hook) then test is aborted */
            if (TRUE == rawHook) {
                pResults->fltMask = LT_ON_OFF_HOOK_STM_TEST_ABORTED;
                retval = LtVp880OnOffHookConclude(pTestCtx);
                break;
            }
            /* no break if no off-hook */

        case HOOK_ST_APPLY_TEST_SWITCH:
            /* Throw Test switch and wait for hook debounce */
            pTemp880->testTimer.timerVal = LT_VP880_TEST_SWITCH_SETTLE;

            if (LT_STATUS_RUNNING !=
                Vp880SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST)) {
                return retval;
            }
            *pState = HOOK_ST_CHECK_FOR_OFF_HOOK;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);

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
                retval = LtVp880OnOffHookConclude(pTestCtx);
                LT_DOUT(LT_DBG_INFO, ("LtVp880OnOffHookStHandler(rawHook:FALSE)" \
                    "should be off-hook"));
                break;
            }

            /* no break needed if off-hook found*/

        case HOOK_ST_REMOVE_TEST_SWITCH:
            /* Remove Test switch and wait for hook debounce */
            if (LT_STATUS_RUNNING !=
                Vp880SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL)) {
                return retval;
            }
            *pState = HOOK_ST_CHECK_FOR_ON_HOOK;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);

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
                LT_DOUT(LT_DBG_INFO, ("LtVp880OnOffHookStHandler(rawHook:TRUE)" \
                    "should be on-hook"));
            } else {
                pResults->fltMask = LT_TEST_PASSED;
            }
            /* no break needed */

        case HOOK_ST_CONCLUDE:
            retval = LtVp880OnOffHookConclude(pTestCtx);
            break;

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
LtVp880OnOffHookConclude(
    LtTestCtxType *pTestCtx)
{
    VpTestConcludeType conclude = { FALSE };
    pTestCtx->state = HOOK_ST_COMPLETE;
    Vp880SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);

    /* Check the Hook Bit and Conclude the test*/
    return Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);

}
#endif /* LT_VP880_VVP_PACKAGE */
#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
