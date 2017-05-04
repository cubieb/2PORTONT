/** file lt_api_vp886_dc_feed_st.c
 *
 *  This file contains the VP886 DC Feed Self Test test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10187 $
 * $LastChangedDate: 2012-06-21 16:19:24 -0500 (Thu, 21 Jun 2012) $
 */
#include "lt_api.h"

#ifdef LT_DC_FEED_ST_886

#include "lt_api_internal.h"

typedef enum {
    DC_FEED_ST_INIT                     = LT_GLOBAL_FIRST_STATE,
    DC_FEED_SWITCH_LINE_STATE,
    DC_FEED_LINE_STATE_SETTLE,
    DC_FEED_APPLY_TEST_SWITCH,
    DC_FEED_APPLY_TEST_SWITCH_SETTLE,
    DC_FEED_REQUEST_VAB,
    DC_FEED_STORE_VAB,
    DC_FEED_REQUEST_IMT,
    DC_FEED_STORE_IMT,
    DC_FEED_REMOVE_TEST_SWITCH,
    DC_FEED_REMOVE_TEST_SWITCH_SETTLE,
    DC_FEED_ST_CONCLUDE,
    DC_FEED_ST_COMPLETE,
    DC_FEED_ST_ABORT                    = LT_GLOBAL_ABORTED_STATE,
    DC_FEED_ST_ENUM_SIZE                = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886DcFeedStStates;


static bool
DcFeedStHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886DcFeedStAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtDcFeedSTInputType *ppInputs =
        (LtDcFeedSTInputType*)pTemp886->attributes.pInputs;

    LtDcFeedSTCriteriaType *ppCriteria =
        (LtDcFeedSTCriteriaType*)pTemp886->attributes.pCriteria;

    LtVp886DcFeedStTempType *pTempDcFeedSt = &pTemp886->tempData.dcFeedSt;

    /* Check input args */
    if (ppInputs == VP_NULL) {
        pTempDcFeedSt->input.lineState = LT_DC_FEED_ST_DFLTI_LINE_STATE;
    } else {
        pTempDcFeedSt->input.lineState = ppInputs->lineState;
    }

    if (pTempDcFeedSt->input.lineState == VP_LINE_DISCONNECT ||
        pTempDcFeedSt->input.lineState == VP_LINE_RINGING) {
            /* Do not run the test if the requested line state is ringing or disconnect */
            LT_DOUT(LT_DBG_ERRR, ("Vp886DcFeedAttributeCheck(:%i) : "
                "invalid requested linestate", pTempDcFeedSt->input.lineState));
        return FALSE;

    } else if (pTempDcFeedSt->input.lineState == LT_DC_FEED_ST_DFLTI_LINE_STATE &&
               (pTemp886->resources.initialLineState == VP_LINE_DISCONNECT ||
                pTemp886->resources.initialLineState == VP_LINE_RINGING) ) {
            /* Do not run the test if the current line state is ringing or disconnect */
            LT_DOUT(LT_DBG_ERRR, ("Vp886DcFeedAttributeCheck(%i) : "
                "invalid current linestate", pTemp886->resources.initialLineState ));
        return FALSE;
    }

    /* Check criteria args */
    if (ppCriteria == VP_NULL) {
        pTempDcFeedSt->criteria.rloadErr = LT_DC_FEED_ST_CRT_IMP_PERCENT;
    } else {
        if ((LT_0_PERCENT >= ppCriteria->rloadErr) ||
            (LT_100_PERCENT < ppCriteria->rloadErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886DcFeedAttributeCheck(rloadErr:%li) : "
                "invalid", ppCriteria->rloadErr));
            return FALSE;
        }
        pTempDcFeedSt->criteria.rloadErr = ppCriteria->rloadErr;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = DcFeedStHandlerSM;
    return TRUE;
}

static bool
DcFeedStHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{

    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886DcFeedStTempType *pTempDcFeedSt = &pTemp886->tempData.dcFeedSt;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("DcFeedStHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case DC_FEED_ST_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_DC_FEED_ST);

            /* setup the common loop cond struct for multiple measurements */
            pTemp886->loopCond.calMode = FALSE; /* never in a non-feed state so this is legal */
            pTemp886->loopCond.integrateTime =  LT_VP886_DC_FEED_INTEGRATE_TIME;
            pTemp886->loopCond.settlingTime = LT_VP886_DC_FEED_SETTLE_TIME;
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;

            if (pTempDcFeedSt->input.lineState != LT_DC_FEED_ST_DFLTI_LINE_STATE) {
                pTestCtx->state = DC_FEED_SWITCH_LINE_STATE;
            } else {
                pTestCtx->state = DC_FEED_APPLY_TEST_SWITCH;
            }
            requestNextState = TRUE;
            break;

        case DC_FEED_SWITCH_LINE_STATE:
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, pTempDcFeedSt->input.lineState);
            pTestCtx->state = DC_FEED_LINE_STATE_SETTLE;
            requestNextState = TRUE;
            break;

        case DC_FEED_LINE_STATE_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = DC_FEED_APPLY_TEST_SWITCH;
            break;


        case DC_FEED_APPLY_TEST_SWITCH:
            /* Throw Test switch */
            *pRetval = Vp886SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST);
            pTestCtx->state = DC_FEED_APPLY_TEST_SWITCH_SETTLE;
            requestNextState = TRUE;
            break;

        case DC_FEED_APPLY_TEST_SWITCH_SETTLE:
            /* wait for switch to debounce */
            pTemp886->testTimer.timerVal = LT_VP886_TEST_SWITCH_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = DC_FEED_REQUEST_VAB;
            break;


        case DC_FEED_REQUEST_VAB:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = DC_FEED_STORE_VAB;
            break;

        case DC_FEED_STORE_VAB:
            pTempDcFeedSt->vsab = (uint16)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = DC_FEED_REQUEST_IMT;
            requestNextState = TRUE;
            break;



        case DC_FEED_REQUEST_IMT:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_IMT;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = DC_FEED_STORE_IMT;
            break;

        case DC_FEED_STORE_IMT:
            pTempDcFeedSt->imt = (uint16)pTestPrimRslts->result.loopCond.data;
            pTempDcFeedSt->ila = (int16)pTestPrimRslts->result.loopCond.limit;
            pTestCtx->state = DC_FEED_REMOVE_TEST_SWITCH;
            requestNextState = TRUE;
            break;


        case DC_FEED_REMOVE_TEST_SWITCH:
            /* Remove Test switch */
            *pRetval = Vp886SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);
            pTestCtx->state = DC_FEED_REMOVE_TEST_SWITCH_SETTLE;
            requestNextState = TRUE;
            break;

        case DC_FEED_REMOVE_TEST_SWITCH_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_TEST_SWITCH_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = DC_FEED_ST_CONCLUDE;
            break;

        /* Start the conclude primitive if this test not a part of an internal test */
        case DC_FEED_ST_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = DC_FEED_ST_COMPLETE;
            break;
        }

        case DC_FEED_ST_COMPLETE:
            /* Calculate results and end the test */
            CalculateResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case DC_FEED_ST_ABORT:
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

static void
CalculateResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886DcFeedStTempType *pTempDcFeedSt = &pTemp886->tempData.dcFeedSt;
    LtDcFeedSTResultType *pResults = &pTestCtx->pResult->result.dcFeedST;
    LtCurrentType targetCurrent;
    LtPercentType percentErr;

    /* convert the voltage in the test load to a real world value */
    pResults->vTestLoad = VP886_UNIT_CONVERT(pTempDcFeedSt->vsab,
        VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    /* convert the current in the test load to a real world value */
    pResults->iTestLoad = VP886_UNIT_CONVERT(pTempDcFeedSt->imt,
        VP886_UNIT_ADC_IMET_NOR, VP886_UNIT_UA);

    /* calculate the resistance of the test load  */
    pResults->rTestLoad = ABS((pResults->vTestLoad * 10000L) /
                            pResults->iTestLoad);

    targetCurrent = VP886_UNIT_CONVERT(pTempDcFeedSt->ila,
        VP886_UNIT_ADC_IMET_NOR, VP886_UNIT_UA);

    percentErr = (LT_DC_FEED_ST_CRT_ILA_PERCENT / 1000);
    percentErr = (targetCurrent * percentErr) / 100;

    /* determine if the target current of the loop is within spec */
    if ( ABS(targetCurrent - ABS(pResults->iTestLoad) ) > percentErr) {
        pResults->fltMask = LT_DC_FEED_ST_TEST_FAILED;
    }

    pResults->fltMask = LT_TEST_PASSED;

    return;
}

#endif /* LT_DC_FEED_ST_886 */
