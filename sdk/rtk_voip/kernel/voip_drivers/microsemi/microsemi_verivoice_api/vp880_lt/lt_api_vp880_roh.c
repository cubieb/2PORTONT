/** file lt_api_vp880_roh.c
 *
 *  This file contains the Line Test implemenation for Ringer Off-hook test.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9866 $
 * $LastChangedDate: 2012-04-18 16:32:00 -0500 (Wed, 18 Apr 2012) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_ROH)

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    ROH_INIT = LT_GLOBAL_FIRST_STATE,
    ROH_GO_TO_ACTIVE,
    ROH_CHECK_FOR_ABORT_HOOK,
    ROH_SETUP,
    ROH_GET_RESULTS,
    ROH_REMOVE_TARGET_CURRENT,
    ROH_CONCLUDE,
    ROH_COMPLETE,
    ROH_ABORT = LT_GLOBAL_ABORTED_STATE,
    ROH_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880RohStates;

LtTestStatusType
LtVp880RohMeasureDcRloop(
    LtTestCtxType *pTestCtx);

extern LtTestStatusType
LtVp880RohHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880RohTempType *pTempRoh = &pTemp880->tempData.roh;
    LtRohResultType *pResults = &pTestCtx->pResult->result.roh;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880RohHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case ROH_INIT:
            if (pTemp880->internalTest == TRUE) {
                LtTestResultType resTmp;

                LtInitOutputStruct(&resTmp, LT_TID_ROH);
                LtMemCpy(pResults, &(resTmp.result.roh), sizeof(LtRohResultType));
            }
            /* no break */

        case ROH_GO_TO_ACTIVE:
            *pState = ROH_CHECK_FOR_ABORT_HOOK;
            pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE * 2;
            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            break;

        case ROH_CHECK_FOR_ABORT_HOOK: {
            bool temp;
            /* Check groundkey status and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_GKEY, &temp)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* If the ground key bit is true then test is done early due to ilg */
            if (TRUE == temp) {
                LT_DOUT(LT_DBG_INFO, ("LtVp880RohHandler() quick stop due to ground key"));
                pTempRoh->quickStop = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;

                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                *pState = ROH_CONCLUDE;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);

                break;
            }

            /* Check raw off-hook and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &temp)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* If the raw hook bit is false (on-hook) then test is done early*/
            if (FALSE == temp) {
                LT_DOUT(LT_DBG_INFO, ("LtVp880RohHandler() quick stop due to on-hook"));
                pTempRoh->quickStop = TRUE;
                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                *pState = ROH_CONCLUDE;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);
                break;
            }
            /* no break if raw off-hook detected */
        }

        case ROH_SETUP:
            pTempRoh->innerState = VP880_ROH_DC_RLOOP1;
            pTempRoh->quickStop = FALSE;
            pTempRoh->imt1 = LT_VP880_ROH_DRIVE_CURRENT_1;
            pTempRoh->imt2 = LT_VP880_ROH_DRIVE_CURRENT_2;
            pTempRoh->imt3 = LT_VP880_ROH_DRIVE_CURRENT_3;

            retval = LtVp880RohMeasureDcRloop(pTestCtx);

            *pState = ROH_GET_RESULTS;
            break;

        case ROH_GET_RESULTS: {
            /* get the api result */
            int16 vabResult = pVpTestResult->result.dcvab.vab;
            int16 ilgResult = pVpTestResult->result.dcvab.ilg;
            bool failFlag = 0;
            retval = LT_STATUS_RUNNING;

            /* store the api result for later calculations*/
            switch (pTempRoh->innerState) {
                case VP880_ROH_DC_RLOOP1:
                    pTempRoh->vab1 = vabResult;
                    break;
                case VP880_ROH_DC_RLOOP2:
                    pTempRoh->vab2 = vabResult;
                    break;
                default:
                    pTempRoh->vab3 = vabResult;
                    break;
            }

            /*
             * Do a quick check to see if vab exceeds on-hook limit
             * if it does then stop the test
             */
            if (LT_VP880_ROH_QUICK_V_CHECK < ABS(vabResult)) {
                bool hook;
                /* Check raw off-hook and error if something goes wrong */
                if ( VP_STATUS_SUCCESS !=
                    VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &hook)) {
                    return LT_STATUS_ERROR_UNKNOWN;
                }
                /*
                 * If the raw hook bit is false (on-hook) then test is done early
                 * and is passing. However, if the hook indication is TRUE (off-hhok)
                 * then we need to report a LONG RES LOOP
                 */
                if (FALSE == hook) {
                    pResults->fltMask = LT_TEST_PASSED;
                } else {
                    pResults->fltMask = LT_ROHM_OUT_OF_RANGE_LOOP;
                }
                pTempRoh->quickStop = TRUE;
                failFlag = 1;
            } else if (LT_VP880_ROH_QUICK_I_CHECK < ABS(ilgResult)) {
                /*
                 * Check that there are no logitudinal faults
                 */
                pTempRoh->quickStop = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
                failFlag = 1;
            }

            if ((TRUE != failFlag) && ((pTempRoh->innerState)++ < VP880_ROH_DC_RLOOP3)) {
                /* move back to the prepare state until all data is collected */
                retval = LtVp880RohMeasureDcRloop(pTestCtx);
                break;
            }
            /*  no break needed once all 3 tests data points are collected. */
        }

        case ROH_REMOVE_TARGET_CURRENT:
            *pState = ROH_CONCLUDE;
            pTempRoh->innerState = VP880_ROH_DC_RLOOP4;
            retval = LtVp880RohMeasureDcRloop(pTestCtx);
            break;

        case ROH_CONCLUDE:

            if (FALSE == pTestCtx->pTemp->vp880Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = ROH_COMPLETE;
                break;
            }

        case ROH_COMPLETE:

            if ((pResults->measStatus == LT_MSRMNT_STATUS_PASSED) &&
                (pResults->fltMask != LT_ROHM_OUT_OF_RANGE_LOOP)) {
                pResults->fltMask = LT_TEST_PASSED;
            }

            if (FALSE == pTempRoh->quickStop) {
                LtVp880CalculateResults(pTestCtx, LT_TID_ROH);
            }
            retval = LT_STATUS_DONE;
            break;

        case ROH_ABORT:
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

LtTestStatusType
LtVp880RohMeasureDcRloop(
    LtTestCtxType *pTestCtx)
{
    LtVp880RohTempType *pTempRoh = &pTestCtx->pTemp->vp880Temp.tempData.roh;

    VpTestDcRLoopType dcRloopInput;
    dcRloopInput.calMode = FALSE;

    switch (pTempRoh->innerState) {
        case VP880_ROH_DC_RLOOP1:
            dcRloopInput.calMode = TRUE; /* extra setup */
            dcRloopInput.iTestLevel =
                (int16)VP880_UNIT_CONVERT(pTempRoh->imt1, VP880_UNIT_UA, VP880_UNIT_ADC_IMET_NOR_JA);
            dcRloopInput.integrateTime = 160; /* 20  ms */
            dcRloopInput.settlingTime = 80;   /* 10  ms */
            break;
        case VP880_ROH_DC_RLOOP2:
            dcRloopInput.iTestLevel =
                (int16)VP880_UNIT_CONVERT(pTempRoh->imt2, VP880_UNIT_UA, VP880_UNIT_ADC_IMET_NOR_JA);
            dcRloopInput.integrateTime = 160; /* 20  ms */
            dcRloopInput.settlingTime = 800;  /* 100 ms */
            break;
        case VP880_ROH_DC_RLOOP3:
            dcRloopInput.iTestLevel =
                (int16)VP880_UNIT_CONVERT(pTempRoh->imt3, VP880_UNIT_UA, VP880_UNIT_ADC_IMET_NOR_JA);
            dcRloopInput.integrateTime = 160; /* 20  ms */
            dcRloopInput.settlingTime = 800;  /* 100 ms */
            break;
        default:
            /* used to remove the target current */
            dcRloopInput.iTestLevel = 0;
            dcRloopInput.integrateTime = 0;
            dcRloopInput.settlingTime = 0;
            break;
    }
    /* start the DC_RLOOP measurment */
    return Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DC_RLOOP, &dcRloopInput);
}

#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
