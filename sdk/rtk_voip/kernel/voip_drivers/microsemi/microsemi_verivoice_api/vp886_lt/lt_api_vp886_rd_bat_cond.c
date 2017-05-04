/** file lt_api_vp886_rd_bat_cond.c
 *
 *  This file contains the VP886 read battery conditions test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10385 $
 * $LastChangedDate: 2012-08-14 15:20:39 -0500 (Tue, 14 Aug 2012) $
 */
#include "lt_api.h"

#ifdef LT_RD_BAT_COND_886

#include "lt_api_internal.h"

typedef enum {
    RD_BAT_COND_INIT            = LT_GLOBAL_FIRST_STATE,
    RD_BAT_COND_REQUEST_BAT1,
    RD_BAT_COND_STORE_BAT1,
    RD_BAT_COND_REQUEST_BAT2,
    RD_BAT_COND_STORE_BAT2,
    RD_BAT_COND_REQUEST_BAT3,
    RD_BAT_COND_STORE_BAT3,
    RD_BAT_COND_CONCLUDE,
    RD_BAT_COND_COMPLETE,
    RD_BAT_COND_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RD_BAT_COND_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886RdBatCondStates;


static bool
RdBatCondHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886RdBatCondAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = RdBatCondHandlerSM;
    return TRUE;
}

static bool
RdBatCondHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RdBatCondTempType *pTempRdBatCond = &pTemp886->tempData.rdBatCond;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("RdBatCondHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case RD_BAT_COND_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_RD_BAT_COND);

            pTemp886->loopCond.calMode = FALSE;
            pTemp886->loopCond.integrateTime =  LT_VP886_RD_BAT_COND_INTEGRATE_TIME;
            pTemp886->loopCond.settlingTime = LT_VP886_RD_BAT_COND_SETTLE_TIME;

            pTestCtx->state = RD_BAT_COND_REQUEST_BAT1;
            requestNextState = TRUE;
            break;

        /* Measuring Battery 1  Voltage */
        case RD_BAT_COND_REQUEST_BAT1:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_BAT1;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_BAT_COND_STORE_BAT1;
            break;

        case RD_BAT_COND_STORE_BAT1:
            pTempRdBatCond->bat1 = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_BAT_COND_REQUEST_BAT2;
            requestNextState = TRUE;
            break;


        /* Measuring Battery 2  Voltage */
        case RD_BAT_COND_REQUEST_BAT2:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_BAT2;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_BAT_COND_STORE_BAT2;
            break;

        case RD_BAT_COND_STORE_BAT2:
            pTempRdBatCond->bat2 = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_BAT_COND_REQUEST_BAT3;
            requestNextState = TRUE;
            break;


        /* Measuring Battery 3 Voltage (actually IO2 for this device) */
        case RD_BAT_COND_REQUEST_BAT3:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_BAT3;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_BAT_COND_STORE_BAT3;
            break;

        case RD_BAT_COND_STORE_BAT3:
            pTempRdBatCond->bat3 = (int32)pTestPrimRslts->result.loopCond.data;
            if (pTemp886->internalTest == TRUE) {
                pTestCtx->state = RD_BAT_COND_COMPLETE;
            } else {
                pTestCtx->state = RD_BAT_COND_CONCLUDE;
            }
            requestNextState = TRUE;
            break;


        /* Start the conclude primitive if this test not a part of an internal test */
        case RD_BAT_COND_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = RD_BAT_COND_COMPLETE;
            break;
        }

        case RD_BAT_COND_COMPLETE:
            /* Calculate results and end the test */
            CalculateResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case RD_BAT_COND_ABORT:
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
    LtVp886RdBatCondTempType *pTempRdBatCond = &pTemp886->tempData.rdBatCond;
    LtReadBatCondResultType *pResults = &pTestCtx->pResult->result.readBatCond;

    pTempRdBatCond->bat1 = (pTempRdBatCond->bat1 == VP_INT16_MAX) ? LT_VOLTAGE_NOT_MEASURED : pTempRdBatCond->bat1;
    if (pTempRdBatCond->bat1 != LT_VOLTAGE_NOT_MEASURED) {
        pResults->bat1 = VP886_UNIT_CONVERT(pTempRdBatCond->bat1,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    pTempRdBatCond->bat2 = (pTempRdBatCond->bat2 == VP_INT16_MAX) ? LT_VOLTAGE_NOT_MEASURED : pTempRdBatCond->bat2;
    if (pTempRdBatCond->bat2 != LT_VOLTAGE_NOT_MEASURED) {
        pResults->bat2 = VP886_UNIT_CONVERT(pTempRdBatCond->bat2,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    pTempRdBatCond->bat3 = (pTempRdBatCond->bat3 == VP_INT16_MAX) ? LT_VOLTAGE_NOT_MEASURED : pTempRdBatCond->bat3;
    if (pTempRdBatCond->bat3 != LT_VOLTAGE_NOT_MEASURED) {
        pResults->bat3 = VP886_UNIT_CONVERT(pTempRdBatCond->bat3,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    pResults->fltMask = LT_TEST_PASSED;

    return;
}

#endif /* LT_RD_BAT_COND_886 */
