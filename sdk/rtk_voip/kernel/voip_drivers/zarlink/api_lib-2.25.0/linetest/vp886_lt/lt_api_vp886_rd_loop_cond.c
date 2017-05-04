/** file lt_api_vp886_rd_loop_cond.c
 *
 *  This file contains the VP886 read loop condition test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10837 $
 * $LastChangedDate: 2013-03-01 15:42:35 -0600 (Fri, 01 Mar 2013) $
 */
#include "lt_api.h"

#ifdef LT_RD_LOOP_COND_886

#include "lt_api_internal.h"

typedef enum {
    RD_LOOP_COND_INIT            = LT_GLOBAL_FIRST_STATE,
    RD_LOOP_COND_REQUEST_VAB,
    RD_LOOP_COND_STORE_VAB,
    RD_LOOP_COND_REQUEST_VAG,
    RD_LOOP_COND_STORE_VAG,
    RD_LOOP_COND_REQUEST_VBG,
    RD_LOOP_COND_STORE_VBG,
    RD_LOOP_COND_REQUEST_IMT,
    RD_LOOP_COND_STORE_IMT,
    RD_LOOP_COND_REQUEST_ILG,
    RD_LOOP_COND_STORE_ILG,
    RD_LOOP_COND_REQUEST_BAT1,
    RD_LOOP_COND_STORE_BAT1,
    RD_LOOP_COND_REQUEST_BAT2,
    RD_LOOP_COND_STORE_BAT2,
    RD_LOOP_COND_REQUEST_BAT3,
    RD_LOOP_COND_STORE_BAT3,
    RD_LOOP_COND_CONCLUDE,
    RD_LOOP_COND_COMPLETE,
    RD_LOOP_COND_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RD_LOOP_COND_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886RdLoopCondStates;


static bool
RdLoopCondHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886RdLoopCondAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RdLoopCondTempType *pTempRdLoopCond = &pTemp886->tempData.rdLoopCond;

    pTempRdLoopCond->input.lineState = LT_RD_LOOP_COND_DFLTI_LINE_STATE;
    pTempRdLoopCond->criteria.rloadErr = LT_RD_LOOP_COND_CRT_IMP_PERCENT;

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = RdLoopCondHandlerSM;
    return TRUE;
}

static bool
RdLoopCondHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RdLoopCondTempType *pTempRdLoopCond = &pTemp886->tempData.rdLoopCond;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("RdLoopCondHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case RD_LOOP_COND_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_RD_LOOP_COND);
            pTestCtx->state = RD_LOOP_COND_REQUEST_VAB;
            requestNextState = TRUE;
            break;


        /* Measuring Tip - Ring Voltage */
        case RD_LOOP_COND_REQUEST_VAB:
            /* setup the common loop cond struct to get vab voltage */
            pTemp886->loopCond.calMode = TRUE;
            pTemp886->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp886->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_VAB;
            break;

        case RD_LOOP_COND_STORE_VAB:
            /* store return value from VSAB read */
            pTempRdLoopCond->vab = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_LOOP_COND_REQUEST_VAG;
            requestNextState = TRUE;
            break;


        /* Measuring Tip to Ground Voltage */
        case RD_LOOP_COND_REQUEST_VAG:
            pTemp886->loopCond.calMode = FALSE; /* no need for the api to setup again */
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_VSAG;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_VAG;
            break;

        case RD_LOOP_COND_STORE_VAG:
            pTempRdLoopCond->vag = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_LOOP_COND_REQUEST_VBG;
            requestNextState = TRUE;
            break;


        /* Measuring Ring to Ground Voltage */
        case RD_LOOP_COND_REQUEST_VBG:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_VSBG;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_VBG;
            break;

        case RD_LOOP_COND_STORE_VBG:
            pTempRdLoopCond->vbg = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_LOOP_COND_REQUEST_IMT;
            requestNextState = TRUE;
            break;


        /* Measuring Metallic Current if we are not in disconnect */
        case RD_LOOP_COND_REQUEST_IMT:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_IMT;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_IMT;
            break;

        case RD_LOOP_COND_STORE_IMT:
            pTempRdLoopCond->imt = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_LOOP_COND_REQUEST_ILG;
            requestNextState = TRUE;
            break;


        /* Measuring Longitudinal Current if we are not in disconnect */
        case RD_LOOP_COND_REQUEST_ILG:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_ILG;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_ILG;
            break;

        case RD_LOOP_COND_STORE_ILG:
            pTempRdLoopCond->ilg = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_LOOP_COND_REQUEST_BAT1;
            requestNextState = TRUE;
            break;


        /* Measuring Battery 1  Voltage */
        case RD_LOOP_COND_REQUEST_BAT1:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_BAT1;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_BAT1;
            break;

        case RD_LOOP_COND_STORE_BAT1:
            pTempRdLoopCond->vbat1 = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_LOOP_COND_REQUEST_BAT2;
            requestNextState = TRUE;
            break;


        /* Measuring Battery 2  Voltage */
        case RD_LOOP_COND_REQUEST_BAT2:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_BAT2;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_BAT2;
            break;

        case RD_LOOP_COND_STORE_BAT2:
            pTempRdLoopCond->vbat2 = (int32)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = RD_LOOP_COND_REQUEST_BAT3;
            requestNextState = TRUE;
            break;


        /* Measuring Battery 3 Voltage */
        case RD_LOOP_COND_REQUEST_BAT3:
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_BAT3;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = RD_LOOP_COND_STORE_BAT3;
            break;

        case RD_LOOP_COND_STORE_BAT3:
            pTempRdLoopCond->vbat3 = (int32)pTestPrimRslts->result.loopCond.data;
            if (pTemp886->internalTest == TRUE) {
                pTestCtx->state = RD_LOOP_COND_COMPLETE;
            } else {
                pTestCtx->state = RD_LOOP_COND_CONCLUDE;
            }
            requestNextState = TRUE;
            break;



        /* Start the conclude primitive if this test not a part of an internal test */
        case RD_LOOP_COND_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = RD_LOOP_COND_COMPLETE;
            break;
        }

        case RD_LOOP_COND_COMPLETE:
            /* Calculate results and end the test */
            CalculateResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case RD_LOOP_COND_ABORT:
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
    LtVp886RdLoopCondTempType *pTempRdLoopCond = &pTemp886->tempData.rdLoopCond;
    LtRdLoopCondResultType *pResults = &pTestCtx->pResult->result.rdLoopCond;
    VpLineTopologyType topInfo;

    /* need topology info to correct for resistance in the loop */
    VpQueryImmediate(pTestCtx->pLineCtx, VP_QUERY_ID_LINE_TOPOLOGY, &topInfo);
    LT_DOUT(LT_DBG_INFO, ("rOutsideDcSense %i Ohms", topInfo.rOutsideDcSense));

    pResults->fltMask = LT_TEST_PASSED;

    if (pTempRdLoopCond->vab != VP_INT16_MAX) {
        pResults->vab = VP886_UNIT_CONVERT(pTempRdLoopCond->vab,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    if (pTempRdLoopCond->vag != VP_INT16_MAX) {
        pResults->vag = VP886_UNIT_CONVERT(pTempRdLoopCond->vag,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    if (pTempRdLoopCond->vbg != VP_INT16_MAX) {
        pResults->vbg = VP886_UNIT_CONVERT(pTempRdLoopCond->vbg,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    if (pTempRdLoopCond->imt != VP_INT16_MAX) {
        if ((pTemp886->resources.initialLineState == VP_LINE_RINGING) ||
            (pTemp886->resources.initialLineState == VP_LINE_RINGING_POLREV)) {
            /* Ringing state so result is half the value */
            pResults->imt = VP886_UNIT_CONVERT(pTempRdLoopCond->imt,
                    VP886_UNIT_ADC_IMET_RING, VP886_UNIT_UA);
        } else {
            pResults->imt = VP886_UNIT_CONVERT(pTempRdLoopCond->imt,
                    VP886_UNIT_ADC_IMET_NOR, VP886_UNIT_UA);
        }
    }

    if (pTempRdLoopCond->ilg != VP_INT16_MAX) {
        pResults->ilg = VP886_UNIT_CONVERT(pTempRdLoopCond->ilg,
            VP886_UNIT_ADC_ILG, VP886_UNIT_UA);
    }

    if (pTempRdLoopCond->vbat1 != VP_INT16_MAX) {
        pResults->vbat1 = VP886_UNIT_CONVERT(pTempRdLoopCond->vbat1,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    if (pTempRdLoopCond->vbat2 != VP_INT16_MAX) {
        pResults->vbat2 = VP886_UNIT_CONVERT(pTempRdLoopCond->vbat2,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    if (pTempRdLoopCond->vbat3 != VP_INT16_MAX) {
        pResults->vbat3 = VP886_UNIT_CONVERT(pTempRdLoopCond->vbat3,
            VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    }

    /* Compute rloop */
    if (pResults->imt != LT_CURRENT_NOT_MEASURED) {
        if (pResults->imt == 0) {
            pResults->rloop = LT_IMPEDANCE_OPEN_CKT;
        } else {
            pResults->rloop  = ((pResults->vab * 10000L) / pResults->imt) -
                (10 * 2 * topInfo.rOutsideDcSense);
            if (pResults->rloop < 0) {
                pResults->rloop *= -1;
            }
            if (pResults->rloop > 250000L) {
                pResults->rloop = LT_IMPEDANCE_OPEN_CKT;
            }
        }
    }

    return;
}

#endif /* LT_RD_LOOP_COND_886 */
