/** file lt_api_vp886_loopback.c
 *
 *  This file contains the VP886 loop back test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10729 $
 * $LastChangedDate: 2013-01-25 14:36:43 -0600 (Fri, 25 Jan 2013) $
 */
#include "lt_api.h"

#ifdef LT_LOOPBACK_886

#include "lt_api_internal.h"

typedef enum {
    LOOPBACK_INIT           = LT_GLOBAL_FIRST_STATE,
    LOOPBACK_CONCLUDE,
    LOOPBACK_COMPLETE,
    LOOPBACK_ABORT          = LT_GLOBAL_ABORTED_STATE,
    LOOPBACK_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886LoopbackStates;


static bool
LoopbackHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886LoopbackAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886LoopbackTempType *pTempLoopback = &pTemp886->tempData.loopback;
    LtLoopbackInputType *ppInputs =
        (LtLoopbackInputType*)pTemp886->attributes.pInputs;

    uint32 time;
    LtLoopbakTestType loopBack;

    /* If the inputs are not defined then use defaults */
    if (VP_NULL == ppInputs) {
        time = LT_LOOPBACK_DFLT_WAIT_TIME/125;
        pTempLoopback->vpApiInput.waitTime =  (uint16)time;

        time = LT_LOOPBACK_DFLT_TEST_TIME/125;
        pTempLoopback->vpApiInput.loopbackTime =  (uint16)time;
        loopBack = LT_LOOPBACK_DFLT_TEST_TYPE;

    } else {

        /* Check for valid loopbacks */
        if (LT_LOOPBACK_TEST_NUM_TYPES <= ppInputs->loopbackType) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886LoopbackAttributesCheck(loopbackType:%i) : "\
                "invalid", ppInputs->loopbackType));
            return FALSE;
        }

        /* Ensure that the max wait and loopback times are not exceeded */
        if ((LT_LOOPBACK_MAX_TIME < ppInputs->waitTime) ||
            (LT_LOOPBACK_MAX_TIME < ppInputs->loopbackTime) ) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886LoopbackAttributesCheck(waitTime:%li||"\
                "loopbackTime:%li) : invalid", ppInputs->waitTime,
                ppInputs->loopbackTime));
            return FALSE;
        }

        /* Save input data into temp structure */
        time = ppInputs->waitTime/125;
        pTempLoopback->vpApiInput.waitTime =  (uint16)time;

        time = ppInputs->loopbackTime/125;
        pTempLoopback->vpApiInput.loopbackTime =  (uint16)time;

        loopBack = ppInputs->loopbackType;
    }

    /* set the API struct based on input or default loopback*/
    switch (loopBack) {
        case LT_LOOPBACK_CODEC:
            pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_CODEC;
            break;

        case LT_LOOPBACK_ANALOG:
            pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_ANALOG;
            break;

        case LT_LOOPBACK_BFILTER:
            pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_BFILTER;
            break;

        case LT_LOOPBACK_TIMESLOT:
            pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_TIMESLOT;
            break;

        default:
            LT_DOUT(LT_DBG_ERRR, ("Vp886LoopbackAttributesCheck(loopbackType:%i) : "\
                "invalid", ppInputs->loopbackType));
            return FALSE;
    }

    /* Loopback test must be run from an active state (except in TIMESLOT mode) */
    if ((loopBack != LT_LOOPBACK_TIMESLOT) &&
        (pTemp886->resources.initialLineState != VP_LINE_ACTIVE) &&
        (pTemp886->resources.initialLineState != VP_LINE_ACTIVE_POLREV) &&
        (pTemp886->resources.initialLineState != VP_LINE_TALK) &&
        (pTemp886->resources.initialLineState != VP_LINE_TALK_POLREV) &&
        (pTemp886->resources.initialLineState != VP_LINE_OHT) &&
        (pTemp886->resources.initialLineState != VP_LINE_OHT_POLREV)) {

        LT_DOUT(LT_DBG_ERRR, ("Vp886LoopbackAttributesCheck(lineState:%i) : "\
            "invalid", pTemp886->resources.initialLineState));
        return FALSE;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = LoopbackHandlerSM;

    return TRUE;
}

static bool
LoopbackHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886LoopbackTempType *pTempLoopback = &pTemp886->tempData.loopback;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("LoopbackHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case LOOPBACK_INIT:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOPBACK, &pTempLoopback->vpApiInput);

            if (pTemp886->internalTest == TRUE) {
                pTestCtx->state = LOOPBACK_COMPLETE;
            } else {
                pTestCtx->state = LOOPBACK_CONCLUDE;
            }
            break;


        /* Start the conclude primitive if this test not a part of an internal test */
        case LOOPBACK_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = LOOPBACK_COMPLETE;
            requestNextState = TRUE;
            break;
        }

        case LOOPBACK_COMPLETE:
            /* Calculate results and end the test */
            CalculateResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case LOOPBACK_ABORT:
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
    LtLoopbackResultType *pResults = &pTestCtx->pResult->result.loopback;

    pResults->fltMask = LT_TEST_PASSED;

    return;
}

#endif /* LT_LOOPBACK_886 */
