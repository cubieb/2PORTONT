/** file lt_api_vp880_res_flt.c
 *
 *  This file contains the GR-909 PASS/FAIL Resistive Faults Test.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_RES_FLT)

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    RES_FLT_INIT            = LT_GLOBAL_FIRST_STATE,
    RES_FLT_START,
    RES_FLT_GET_RESULTS,
    RES_FLT_CALCULATE,
    RES_FLT_CONCLUDE,
    RES_FLT_COMPLETE,
    RES_FLT_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RES_FLT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880RenStates;

extern LtTestStatusType
LtVp880ResFltHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880ResFltTempType *pTempResFlt = &pTemp880->tempData.resFlt;
    bool *pHighGainMode = &pTempResFlt->highGainMode;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880ResFltHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
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
                LT_DOUT(LT_DBG_INFO, ("LtVp880ResFltHandler(Special Case!!!)"));
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
                if (TRUE == pTestCtx->pTemp->vp880Temp.internalTest) {
                    pVpTestResult->errorCode = VP_TEST_STATUS_SUCCESS;
                }
                hasErr = FALSE;
            }
        }
        if (hasErr) {
            LT_DOUT(LT_DBG_INFO, ("LtVp880ResFltHandler(ERROR!!!)"));
            return retval;
        }
    }

    switch (*pState) {
        case RES_FLT_INIT:
            if (pTemp880->internalTest == TRUE) {
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
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

            *pState = RES_FLT_START;
            pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE;
            if (pTemp880->resources.initialLineState == VP_LINE_DISCONNECT) {
                /* Need more time to exit disconnect */
                pTemp880->testTimer.timerVal = LT_VP880_ENTER_DISC_SETTLE;
            }
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp880->testTimer);
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

#ifdef LT_VP880_VVA_PACKAGE
            /* Marker value: run the low gain primitive up to completion */
            resFltInput.settlingTime |= 0x8000;
#endif

            retval = Vp880TestLineWrapper(pTestCtx, testId, &resFltInput);
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
            LtVp880CalculateResults(pTestCtx, LT_TID_RES_FLT);
            if ((TRUE == *pHighGainMode) && (FALSE == gainBeforeCalculate)) {
                /*
                 * Switch to High Gain Mode if requested by normal
                 * res flt state machine. This will only occur when using
                 * the VVP package and if the calculate routine requests it.
                 */
                pTestCtx->pTemp->vp880Temp.testTimer.timerVal = LT_VP880_NEXT_TICK;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp880->testTimer);
                *pState = RES_FLT_START;
                break;
            }
            /* no break */
        }

        case RES_FLT_CONCLUDE:
            if (FALSE == pTestCtx->pTemp->vp880Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
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

#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
