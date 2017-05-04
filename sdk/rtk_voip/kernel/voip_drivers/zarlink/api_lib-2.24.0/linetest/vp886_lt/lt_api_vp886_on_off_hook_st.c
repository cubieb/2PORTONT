/** file lt_api_vp886_on_off_hook_st.c
 *
 *  This file contains the VP886 on off hook self test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10187 $
 * $LastChangedDate: 2012-06-21 16:19:24 -0500 (Thu, 21 Jun 2012) $
 */
#include "lt_api.h"

#ifdef LT_ON_OFF_HOOK_ST_886

#include "lt_api_internal.h"

typedef enum {
    HOOK_ST_INIT            = LT_GLOBAL_FIRST_STATE,
    HOOK_GO_TO_ACTIVE,
    HOOK_LINE_STATE_SETTLE,
    HOOK_CHECK_FOR_ABORT_HOOK,
    HOOK_ST_APPLY_TEST_SWITCH,
    HOOK_ST_APPLY_TEST_SWITCH_SETTLE,
    HOOK_ST_CHECK_FOR_OFF_HOOK,
    HOOK_ST_REMOVE_TEST_SWITCH,
    HOOK_ST_REMOVE_TEST_SWITCH_SETTLE,
    HOOK_ST_CHECK_FOR_ON_HOOK,
    HOOK_ST_CONCLUDE,
    HOOK_ST_COMPLETE,
    HOOK_ST_ABORT           = LT_GLOBAL_ABORTED_STATE,
    HOOK_ST_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886OnOffHookStStates;


static bool
OnOffHookStHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

extern bool 
LtVp886OnOffHookStAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtOnOffHookSTInputType *ppInputs  =
        (LtOnOffHookSTInputType*)pTemp886->attributes.pInputs;

    LtVp886OnOffHookStTempType *pTempOnOffHook = &pTemp886->tempData.onOffHookSt;


    /* Check input args*/
    if (VP_NULL == ppInputs) {
        pTempOnOffHook->input.overrideOffHook = LT_ON_OFF_HOOK_ST_DFLT_OVRRD_FLG;
    } else {
        /*
         * Check the override bit. Currently there are no VP886 termination
         * types that support this bit so error out.
         */
        if (TRUE != ppInputs->overrideOffHook) {
            pTempOnOffHook->input.overrideOffHook = ppInputs->overrideOffHook;
        } else {
            LT_DOUT(LT_DBG_ERRR, ("LtVp886OnOffHookStAttributeCheck(overrideOffHook:%i) : "
                "invalid", ppInputs->overrideOffHook));
            return FALSE;
        }
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = OnOffHookStHandlerSM;
    return TRUE;
}

static bool
OnOffHookStHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{

    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtOnOffHookSTResultType *pResults = &pTestCtx->pResult->result.onOffHookST;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("OnOffHookStHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case HOOK_ST_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_ON_OFF_HOOK_ST);
            pTestCtx->state = HOOK_GO_TO_ACTIVE;
            requestNextState = TRUE;
            break;

        case HOOK_GO_TO_ACTIVE:
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            pTestCtx->state = HOOK_LINE_STATE_SETTLE;
            requestNextState = TRUE;
            break;

        case HOOK_LINE_STATE_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = HOOK_CHECK_FOR_ABORT_HOOK;
            break;

        case HOOK_CHECK_FOR_ABORT_HOOK: {
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            /* If the raw hook bit is true (off-hook) then test is done early*/
            if (rawHook) {
                LT_DOUT(LT_DBG_INFO, ("OnOffHookStHandlerSM() aborted due to off-hook"));
                pResults->fltMask = LT_ON_OFF_HOOK_STM_TEST_ABORTED;
                pTestCtx->state = HOOK_ST_CONCLUDE;
            } else {
                pTestCtx->state = HOOK_ST_APPLY_TEST_SWITCH;
            }
            requestNextState = TRUE;
            break;
        }

        case HOOK_ST_APPLY_TEST_SWITCH:
            /* Throw Test switch */
            *pRetval = Vp886SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST);
            pTestCtx->state = HOOK_ST_APPLY_TEST_SWITCH_SETTLE;
            requestNextState = TRUE;
            break;

        case HOOK_ST_APPLY_TEST_SWITCH_SETTLE:
            /* wait for switch to debounce */
            pTemp886->testTimer.timerVal = LT_VP886_TEST_SWITCH_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = HOOK_ST_CHECK_FOR_OFF_HOOK;
            break;

        case HOOK_ST_CHECK_FOR_OFF_HOOK: {
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            /* If the device raw hook bit is not TRUE (off-hook), test failed */
            if (!rawHook) {
                LT_DOUT(LT_DBG_INFO, ("OnOffHookStHandlerSM(rawHook != TRUE)"));
                pResults->fltMask = LT_ON_OFF_HOOK_STM_TEST_HW_FAULT;
                pTestCtx->state = HOOK_ST_CONCLUDE;
            } else {
                pTestCtx->state = HOOK_ST_REMOVE_TEST_SWITCH;
            }
            requestNextState = TRUE;

            break;
        }

        case HOOK_ST_REMOVE_TEST_SWITCH:
            /* Remove Test switch */
            *pRetval = Vp886SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);
            pTestCtx->state = HOOK_ST_REMOVE_TEST_SWITCH_SETTLE;
            requestNextState = TRUE;
            break;

        case HOOK_ST_REMOVE_TEST_SWITCH_SETTLE:
            /* wait for switch to debounce */
            pTemp886->testTimer.timerVal = LT_VP886_TEST_SWITCH_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = HOOK_ST_CHECK_FOR_ON_HOOK;
            break;


        case HOOK_ST_CHECK_FOR_ON_HOOK: {
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            /* If the device raw hook bit is not FALSE (on-hook), test failed */
            if (rawHook) {
                pResults->fltMask = LT_ON_OFF_HOOK_STM_TEST_HW_FAULT;
                LT_DOUT(LT_DBG_INFO, ("OnOffHookStHandlerSM(rawHook != FALSE)"));
            } else {
                pResults->fltMask = LT_TEST_PASSED;
            }
            pTestCtx->state = HOOK_ST_CONCLUDE;
            requestNextState = TRUE;
            break;
        }


        /* Start the conclude primitive if this test not a part of an internal test */
        case HOOK_ST_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            Vp886SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = HOOK_ST_COMPLETE;
            break;
        }

        case HOOK_ST_COMPLETE:
            *pRetval = LT_STATUS_DONE;
            break;

        case HOOK_ST_ABORT:
            /* Test aborted, results may indicate source of error. */
            *pRetval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            LT_DOUT(LT_DBG_ERRR, ("This should never happen -> LT_STATUS_ERROR_UNKNOWN"));
            *pRetval = LT_STATUS_ERROR_UNKNOWN;
            break;
    }
    return requestNextState;
}

#endif /* LT_ON_OFF_HOOK_ST_886 */
