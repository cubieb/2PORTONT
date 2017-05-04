/** file lt_api_vp890_res_flt.c
 *
 *  This file contains the GR-909 PASS/FAIL Resistive Faults Test.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_RES_FLT)

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    RES_FLT_INIT            = LT_GLOBAL_FIRST_STATE,
    RES_FLT_START,
    RES_FLT_GET_RESULTS,
    RES_FLT_CALCULATE,
    RES_FLT_CONCLUDE,
    RES_FLT_COMPLETE,
    RES_FLT_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RES_FLT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RenStates;

extern LtTestStatusType
LtVp890ResFltHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890ResFltTempType *pTempResFlt = &pTemp890->tempData.resFlt;
    bool *pHighGainMode = &pTempResFlt->highGainMode;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890ResFltHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        bool hasErr = TRUE;
      /*
         * Checking for special error code:
         *
         * If the High Gain primitive returns VP_TEST_STATUS_NO_CONVERGENCE then we
         * need to intercept the error stop the test early and force the
         * results to excessive ilg.
         */
        if (pEvent != NULL) {
            if ((pVpTestResult->errorCode == VP_TEST_STATUS_NO_CONVERGENCE) &&
                (pVpTestResult->testId == VP_TEST_ID_3ELE_RES_HG)) {
                LT_DOUT(LT_DBG_INFO, ("LtVp890ResFltHandler(Special Case!!!)"));
                pTestCtx->pResult->result.resFlt.fltMask = LT_TEST_MEASUREMENT_ERROR;
                pTestCtx->pResult->result.resFlt.measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
                pTestCtx->pResult->result.resFlt.rtg = LT_IMPEDANCE_SHORT_CKT;
                pTestCtx->pResult->result.resFlt.rrg = LT_IMPEDANCE_SHORT_CKT;
                pTestCtx->pResult->result.resFlt.rtr = LT_IMPEDANCE_SHORT_CKT;
                pTestCtx->pResult->vpTestErrorCode = VP_TEST_STATUS_SUCCESS;
                *pState = RES_FLT_CONCLUDE;

                /*
                 * Forcing known failing errorCode back to a successfull value
                 * to allow sub 909 all tests to run
                 */
                if (TRUE == pTestCtx->pTemp->vp890Temp.internalTest) {
                    pVpTestResult->errorCode = VP_TEST_STATUS_SUCCESS;
                }
                hasErr = FALSE;
            }
        }
        if (hasErr) {
            LT_DOUT(LT_DBG_INFO, ("LtVp890ResFltHandler(ERROR!!!)"));
            return retval;
        }
    }

    switch (*pState) {
        case RES_FLT_INIT:
            if (pTemp890->internalTest == TRUE) {
                LtTestResultType resTmp;

                LtInitOutputStruct(&resTmp, LT_TID_RES_FLT);
                LtMemCpy(&pTestCtx->pResult->result.resFlt, &(resTmp.result.resFlt),
                    sizeof(LtResFltResultType));
            }
            *pHighGainMode = pTempResFlt->input.startHighGain;
            LT_DOUT(LT_DBG_INFO, ("startHighGain = %d", *pHighGainMode));

            /* Set line in active at the beginning of the low gain primitive */
            if (*pHighGainMode == FALSE) {
                if (VP_STATUS_SUCCESS != VpSetLineState(pTestCtx->pLineCtx, VP_LINE_TALK)) {
                    return LT_STATUS_ERROR_UNKNOWN;
                }
            }

            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

            *pState = RES_FLT_START;
            pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE;
            if (pTemp890->resources.initialLineState == VP_LINE_DISCONNECT) {
                /* Need more time to exit disconnect */
                pTemp890->testTimer.timerVal *= 4;
            }
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp890->testTimer);
            break;

        case RES_FLT_START: {
            VpTestIdType testId = VP_TEST_ID_3ELE_RES_LG;
            VpTest3EleResAltResType resFltInput;
            resFltInput.feedBias = 0x09F3;      /* +/- 6V of feed */
            resFltInput.shiftZeroFeed = TRUE;   /* shift the feed during the zero feed phase */
            resFltInput.integrateTime = 18;     /* default low gain integration time in ms */
            resFltInput.settlingTime = 2;       /* settling time in ms */
            if (TRUE == *pHighGainMode) {
                testId = VP_TEST_ID_3ELE_RES_HG;
            }

#ifdef LT_VP890_VVA_PACKAGE
            /* Marker value: run the low gain primitive up to completion */
            resFltInput.settlingTime |= 0x8000;
#endif

            retval = Vp890TestLineWrapper(pTestCtx, testId, &resFltInput);
            *pState = RES_FLT_GET_RESULTS;
            break;
        }

        case RES_FLT_GET_RESULTS:
            /* store all api resflt results into the temp structure */
            LtMemCpy(&pTempResFlt->vpApiResFltResults,
                &pVpTestResult->result.resAlt,
                sizeof(VpTestResultAltResType));

            /* no break */

        case RES_FLT_CALCULATE: {
            bool gainBeforeCalculate = *pHighGainMode;
            /* Calculate results */
            LtVp890CalculateResults(pTestCtx, LT_TID_RES_FLT);
            if ((TRUE == *pHighGainMode) && (FALSE == gainBeforeCalculate)) {
                /*
                 * Switch to High Gain Mode if requested by normal
                 * res flt state machine. This will only occur when using
                 * the VVP package and if the calculate routine requests it.
                 */
                pTestCtx->pTemp->vp890Temp.testTimer.timerVal = LT_VP890_NEXT_TICK;
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp890->testTimer);
                *pState = RES_FLT_START;
                break;
            }
            /* no break */
        }

        case RES_FLT_CONCLUDE:
            if (FALSE == pTestCtx->pTemp->vp890Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = RES_FLT_COMPLETE;
                break;
             }

        case RES_FLT_COMPLETE:
            retval = LT_STATUS_DONE;
            break;

        case RES_FLT_ABORT:
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

#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
