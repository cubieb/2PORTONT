/** file lt_api_vp886_pre_line_v.c
 *
 *  This file contains the VP886 pre line voltage test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10606 $
 * $LastChangedDate: 2012-11-12 17:10:35 -0600 (Mon, 12 Nov 2012) $
 */
#include "lt_api.h"

#ifdef LT_PRE_LINE_V_886

#include "lt_api_internal.h"

#define VP886_LINE_STATE_SETTLE 1200 /* 150 ms */

typedef enum {
    PRE_LINE_V_INIT                 = LT_GLOBAL_FIRST_STATE,
    PRE_LINE_V_SET_LINE_STATE,
    PRE_LINE_V_LINE_STATE_SETTLE1,
    PRE_LINE_V_CHECK_HOOK_STATUS1,
    PRE_LINE_V_CHECK_GKEY_STATUS1,
    PRE_LINE_V_REVERSE_POLARITY,
    PRE_LINE_V_LINE_STATE_SETTLE2,
    PRE_LINE_V_CHECK_HOOK_STATUS2,
    PRE_LINE_V_CHECK_GKEY_STATUS2,
    PRE_LINE_V_CONCLUDE,
    PRE_LINE_V_COMPLETE,
    PRE_LINE_V_ABORT                = LT_GLOBAL_ABORTED_STATE,
    PRE_LINE_V_ENUM_SIZE            = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886PreLineVStates;


static bool
PreLineVHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);


extern bool 
LtVp886PreLineVAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886PreLineVTempType *pTempPreLineV = &pTemp886->tempData.preLineV;
    VpLineStateType lineStateCurrent = pTemp886->resources.initialLineState;


    /* Pre line v must be run from an active state */
    if ((lineStateCurrent == VP_LINE_DISCONNECT) || 
        ((pTemp886->termType == VP_TERM_FXS_LOW_PWR) && (lineStateCurrent == VP_LINE_STANDBY))) {
         pTempPreLineV->polarity = 0;
    } else if (
        (lineStateCurrent == VP_LINE_STANDBY_POLREV) ||
        (lineStateCurrent == VP_LINE_ACTIVE_POLREV) ||
        (lineStateCurrent == VP_LINE_TALK_POLREV) ||
        (lineStateCurrent == VP_LINE_OHT_POLREV)) {
        pTempPreLineV->polarity = -1;
    } else if (
        (lineStateCurrent == VP_LINE_STANDBY) ||
        (lineStateCurrent == VP_LINE_ACTIVE) ||
        (lineStateCurrent == VP_LINE_TALK) ||
        (lineStateCurrent == VP_LINE_OHT)) {
        pTempPreLineV->polarity = 1;        
    } else {
            LT_DOUT(LT_DBG_ERRR, ("LtVp886PreLineVAttributeCheck(lineState:%i) : "\
                "invalid", pTemp886->resources.initialLineState));
            return FALSE;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = PreLineVHandlerSM;
    return TRUE;
}

static bool
PreLineVHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886PreLineVTempType *pTempPreLineV = &pTemp886->tempData.preLineV;
    LtPreLineVResultType *pResults = &pTestCtx->pResult->result.preLineV;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("PreLineVHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case PRE_LINE_V_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_PRE_LINE_V);
            pTestCtx->state = PRE_LINE_V_SET_LINE_STATE;
            requestNextState = TRUE;
            break;

        case PRE_LINE_V_SET_LINE_STATE: {
            if (pTempPreLineV->polarity == 0) {
                *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
                pTestCtx->state = PRE_LINE_V_LINE_STATE_SETTLE1;
                pTempPreLineV->polarity = 1;
            } else {
                pTestCtx->state = PRE_LINE_V_CHECK_HOOK_STATUS1;
            }

            requestNextState = TRUE;
            break;
        }

        case PRE_LINE_V_LINE_STATE_SETTLE1:
            /* wait for the line to settle */
            pTestCtx->state = PRE_LINE_V_CHECK_HOOK_STATUS1;
            pTemp886->testTimer.timerVal = VP886_LINE_STATE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            break;

        case PRE_LINE_V_CHECK_HOOK_STATUS1: {
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            if (rawHook) {
                pTestCtx->state = PRE_LINE_V_CONCLUDE;
                pResults->fltMask = LT_PLVM_FAILED;
                LT_DOUT(LT_DBG_INFO, ("PreLineVHandlerSM() off-hook"));
            } else {
                pTestCtx->state = PRE_LINE_V_CHECK_GKEY_STATUS1;
            }
            requestNextState = TRUE;
            break;
        }

        case PRE_LINE_V_CHECK_GKEY_STATUS1: {
            bool gndkey = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_GKEY, &gndkey);

            if (gndkey) {
                pTestCtx->state = PRE_LINE_V_CONCLUDE;
                pResults->fltMask = LT_PLVM_FAILED;
                LT_DOUT(LT_DBG_INFO, ("PreLineVHandlerSM() ground-key"));
            } else {
                pTestCtx->state = PRE_LINE_V_REVERSE_POLARITY;
            }
            requestNextState = TRUE;
            break;
        }

        case PRE_LINE_V_REVERSE_POLARITY: {

            /* go to opposite polarity */
            if (pTempPreLineV->polarity > 0) {
                *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE_POLREV);
            } else {
                *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            }
            pTestCtx->state = PRE_LINE_V_LINE_STATE_SETTLE2;
            requestNextState = TRUE;
            break;
        }

        case PRE_LINE_V_LINE_STATE_SETTLE2:
            /* wait for the line to settle */
            pTestCtx->state = PRE_LINE_V_CHECK_HOOK_STATUS2;
            pTemp886->testTimer.timerVal = VP886_LINE_STATE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            break;

        case PRE_LINE_V_CHECK_HOOK_STATUS2: {
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            if (rawHook) {
                pResults->fltMask = LT_PLVM_FAILED;
                pTestCtx->state = PRE_LINE_V_CONCLUDE;
                LT_DOUT(LT_DBG_INFO, ("PreLineVHandlerSM() off-hook"));
            } else {
                pTestCtx->state = PRE_LINE_V_CHECK_GKEY_STATUS2;
            }
            pTestCtx->state = PRE_LINE_V_CHECK_GKEY_STATUS2;
            requestNextState = TRUE;
            break;
        }

        case PRE_LINE_V_CHECK_GKEY_STATUS2: {
            bool gndkey = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_GKEY, &gndkey);

            if (gndkey) {
                pResults->fltMask = LT_PLVM_FAILED;
                LT_DOUT(LT_DBG_INFO, ("PreLineVHandlerSM() ground-key"));
            } else {
                pResults->fltMask = LT_TEST_PASSED;
            }

            pTestCtx->state = PRE_LINE_V_CONCLUDE;
            requestNextState = TRUE;
            break;
        }

        /* Start the conclude primitive */
        case PRE_LINE_V_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = PRE_LINE_V_COMPLETE;
            break;
        }

        case PRE_LINE_V_COMPLETE:
            *pRetval = LT_STATUS_DONE;
            break;

        case PRE_LINE_V_ABORT:
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



#endif /* LT_PRE_LINE_V_886 */
