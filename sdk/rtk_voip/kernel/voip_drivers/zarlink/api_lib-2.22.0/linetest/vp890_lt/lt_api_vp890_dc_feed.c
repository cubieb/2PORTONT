/** file lt_api_vp890_dc_feed.c
 *
 *  This file contains the VP890 dc feed self test routine.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_DC_FEED_ST)

#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    DC_FEED_INIT            = LT_GLOBAL_FIRST_STATE,
    DC_FEED_CALIBRATE,
    DC_FEED_APPLY_FEED,
    DC_FEED_APPLY_TEST_SWITCH,
    DC_FEED_REQUEST_VAB,
    DC_FEED_STORE_VAB,
    DC_FEED_REQUEST_IMT,
    DC_FEED_STORE_IMT,
    DC_FEED_REMOVE_TEST_SWITCH,
    DC_FEED_CONCLUDE,
    DC_FEED_COMPLETE,
    DC_FEED_ABORT           = LT_GLOBAL_ABORTED_STATE,
    DC_FEED_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890DcFeedStates;

extern LtTestStatusType
LtVp890DcFeedHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890DcFeedSTTempType *pTempDcFeedSt = &pTemp890->tempData.dcFeedST;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890DcFeedHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case DC_FEED_INIT:
            /* setup the common loop cond struct for multiple measurements */
            pTemp890->loopCond.calMode = FALSE;
            pTemp890->loopCond.integrateTime =  LT_VP890_DC_FEED_INTEGRATE_TIME;
            pTemp890->loopCond.settlingTime = LT_VP890_DC_FEED_SETTLE_TIME;
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;

            /*
             * Cannot run the calibrate routine in the low power state
             * Therefore it is necessary to put the line into active
             * if the termination type is low power and (the current
             * line state is standby or the requested state is standby)
             */
            if ((pTemp890->termType == VP_TERM_FXS_LOW_PWR) ||
                (pTemp890->termType == VP_TERM_FXS_SPLITTER_LP) ||
                (pTemp890->termType == VP_TERM_FXS_ISOLATE_LP)) {
                if ((pTemp890->resources.initialLineState == LT_DC_FEED_ST_DFLTI_LINE_STATE) ||
                    (pTemp890->resources.initialLineState == VP_LINE_STANDBY) ||
                    (pTempDcFeedSt->input.lineState == VP_LINE_STANDBY)) {

                    if ( VP_STATUS_SUCCESS != VpSetLineState(pTestCtx->pLineCtx,
                        VP_LINE_ACTIVE)) {
                        return LT_STATUS_ERROR_UNKNOWN;
                    }

                    pTemp890->testTimer.timerVal = LT_VP890_LOW_POWER_SETTLE;
                    *pState = DC_FEED_CALIBRATE;
                    retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                        &pTemp890->testTimer);
                    break;
                }
             }
            /* no break */

        case DC_FEED_CALIBRATE:
            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

        case DC_FEED_APPLY_FEED:
            /*
             * Use the current line state if the user does not specify
             * the lineState input argument
             */

            if (pTempDcFeedSt->input.lineState != LT_DC_FEED_ST_DFLTI_LINE_STATE) {

                pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE;

                if ( VP_STATUS_SUCCESS != VpSetLineState(pTestCtx->pLineCtx,
                    pTempDcFeedSt->input.lineState)) {
                    return LT_STATUS_ERROR_UNKNOWN;
                }

                *pState = DC_FEED_APPLY_TEST_SWITCH;
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp890->testTimer);
                break;
            }
            /* no break if no line state change */

        case DC_FEED_APPLY_TEST_SWITCH:
            pTemp890->testTimer.timerVal = LT_VP890_TEST_SWITCH_SETTLE;

            if (LT_STATUS_RUNNING !=
                Vp890SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST)) {
                return retval;
            }
            *pState = DC_FEED_REQUEST_VAB;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);
            break;

        case DC_FEED_REQUEST_VAB:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            *pState = DC_FEED_STORE_VAB;
            break;

        case DC_FEED_STORE_VAB:
            /* store return value from VSAB read */
            pTempDcFeedSt->vsab = (uint16)pVpTestResult->result.loopCond.data;
            /* no break */

        case DC_FEED_REQUEST_IMT:
            /* modify loop cond struct to get loop current */
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_IMT;

            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            *pState = DC_FEED_STORE_IMT;
            break;

        case DC_FEED_STORE_IMT:
            /* store return value from IMT read */
            pTempDcFeedSt->imt = (uint16)pVpTestResult->result.loopCond.data;
            /* store target value (ILA) */
            pTempDcFeedSt->ila = (int16)pVpTestResult->result.loopCond.limit;
            /* no break */

        case DC_FEED_REMOVE_TEST_SWITCH:
            retval = Vp890SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);
            /* no break */

        case DC_FEED_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = DC_FEED_COMPLETE;
                break;
            }

        case DC_FEED_COMPLETE:
            /* Calculate results and end the test */
            LtVp890CalculateResults(pTestCtx, LT_TID_DC_FEED_ST);
            retval = LT_STATUS_DONE;
            break;

        case DC_FEED_ABORT:
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


#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
