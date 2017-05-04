/** file lt_api_vp886_xconnect.c
 *
 *  This file contains the VP886 cross connect test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10288 $
 * $LastChangedDate: 2012-07-16 14:52:14 -0500 (Mon, 16 Jul 2012) $
 */
#include "lt_api.h"

#ifdef LT_XCONNECT_886

#include "lt_api_internal.h"

typedef enum {
    XCONNECT_INIT            = LT_GLOBAL_FIRST_STATE,
    XCONNECT_START,
    XCONNECT_GET_RESULTS,
    XCONNECT_CALCULATE,
    XCONNECT_CONCLUDE,
    XCONNECT_COMPLETE,
    XCONNECT_ABORT           = LT_GLOBAL_ABORTED_STATE,
    XCONNECT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886XConnectStates;


static bool
XConnectHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886XConnectAttributeCheck(
    LtTestCtxType *pTestCtx)
{

    /* no input or critera for this test */

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = XConnectHandlerSM;
    return TRUE;
}

static bool
XConnectHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886XConnectTempType *pTempXConnect = &pTemp886->tempData.xConnect;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("XConnectHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case XCONNECT_INIT:
            pTempXConnect->phase = VP886_XCONNECT_PHASE0;
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_XCONNECT);
            pTestCtx->state = XCONNECT_START;
            requestNextState = TRUE;
            break;

        case XCONNECT_START: {
            VpTestXConnectType xCinput;

            xCinput.integrateTime = 800;            /* 800 / 125us = 100ms */
            xCinput.settlingTime = 400;             /* 400 / 125us = 50ms */

            switch (pTempXConnect->phase) {
                case VP886_XCONNECT_PHASE0:
                    xCinput.settlingTime = 1600;
                    xCinput.measReq = VP_XCONNECT_DISC_V;
                    xCinput.phase = 0;
                    break;

                case VP886_XCONNECT_PHASE1:
                    xCinput.measReq = VP_XCONNECT_12VVOC_I;
                    xCinput.isFeedPositive = pTempXConnect->isEMFPositive;
                    xCinput.phase = 1;
                    break;

                case VP886_XCONNECT_PHASE2:
                    xCinput.measReq = VP_XCONNECT_DISC_V;
                    xCinput.phase = 2;
                    break;

                default:
                    break;
            }

            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_XCONNECT, &xCinput);
            pTestCtx->state = XCONNECT_GET_RESULTS;
            break;
        }

        case XCONNECT_GET_RESULTS:
            /* store all results into the temp structure */
            LtMemCpy(&pTempXConnect->vpApiXConnectResults, &pTestPrimRslts->result.xConnect,
                sizeof(VpTestResultXConnectType));
            pTestCtx->state = XCONNECT_CALCULATE;
            requestNextState = TRUE;
            break;

        case XCONNECT_CALCULATE:
            /* Calculate results */
            CalculateResults(pTestCtx);

            /* Check if extra steps are needed to detect the cross connect */
            if (pTempXConnect->phase != VP886_XCONNECT_DONE) {
                pTestCtx->state = XCONNECT_START;
                requestNextState = TRUE;
            } else {
                pTestCtx->state = XCONNECT_CONCLUDE;
                requestNextState = TRUE;
            }
            break;

        /* Start the conclude primitive if this test not a part of an internal test */
        case XCONNECT_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = XCONNECT_COMPLETE;
            break;
        }

        case XCONNECT_COMPLETE:
            *pRetval = LT_STATUS_DONE;
            break;

        case XCONNECT_ABORT:
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
    LtVp886XConnectTempType *pTempXConnect = &pTemp886->tempData.xConnect;
    LtXConnectResultType *pResults = &pTestCtx->pResult->result.xConnect;

    const int32 data = (int32)pTempXConnect->vpApiXConnectResults.data;

    pResults->fltMask = LT_XCON_TEST_PASSED;
    pResults->measStatus = LT_MSRMNT_STATUS_PASSED;

    switch (pTempXConnect->phase) {
        case VP886_XCONNECT_PHASE0:
            /* Measure the metalic voltage in disconnect, should be less than 35V */
            LT_DOUT(LT_DBG_INFO, ("VP886_XCONNECT_PHASE0: (data=%li | %liV)", data,
                (data * 732 + 50000) / 100000));

            if (ABS(data) < 4782) {
                pResults->fltMask = LT_XCON_TEST_PASSED;
                pTempXConnect->phase = VP886_XCONNECT_DONE;
            } else {
                pTempXConnect->phase = VP886_XCONNECT_PHASE1;
            }

            /* Store the metalic voltage sign for the next phase */
            if (data > 0) {
                pTempXConnect->isEMFPositive = TRUE;
            } else {
                pTempXConnect->isEMFPositive = FALSE;
            }
            break;

        case VP886_XCONNECT_PHASE1:
            /* Measure the metalic current in active with DCFeed=12V, should be less than 15mA */
            LT_DOUT(LT_DBG_INFO, ("VP886_XCONNECT_PHASE1: (data=%li | %limA)", data,
                (data * 176 + 50000) / 100000));

            if (ABS(data) < 8522) {
                pResults->fltMask = LT_XCON_TEST_PASSED;
                pTempXConnect->phase = VP886_XCONNECT_DONE;
            } else {
                pTempXConnect->phase = VP886_XCONNECT_PHASE2;
            }
            break;

        case VP886_XCONNECT_PHASE2:
            /* Measure the metalic voltage in disconnect, should be less than 35V */
            LT_DOUT(LT_DBG_INFO, ("VP886_XCONNECT_PHASE2: (data=%li | %liV)", data,
                (data * 732 + 50000) / 100000));

            if (ABS(data) < 4782) {
                pResults->fltMask = LT_XCON_TEST_PASSED;
            } else {
                if (pTempXConnect->isEMFPositive == TRUE) {
                    pResults->fltMask = LT_XCON_REVERSE_POLARITY;
                } else {
                    pResults->fltMask = LT_XCON_NORMAL_POLARITY;
                }
            }
            pTempXConnect->phase = VP886_XCONNECT_DONE;
            break;

        default:
            LT_DOUT(LT_DBG_INFO, ("Invalid XCONNECT phase state %d", pTempXConnect->phase));
            pResults->fltMask = LT_XCON_FAULT;
            break;
    }

    return;
}


#endif /* LT_XCONNECT_886 */
