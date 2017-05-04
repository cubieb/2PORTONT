/** file lt_api_vp890_roh.c
 *
 *  This file contains the Line Test implemenation for Ringer Off-hook test.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_ROH)

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

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
} LtVp890RohStates;

LtTestStatusType
LtVp890RohMeasureDcRloop(
    LtTestCtxType *pTestCtx);

extern LtTestStatusType
LtVp890RohHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890RohTempType *pTempRoh = &pTemp890->tempData.roh;
    LtRohResultType *pResults = &pTestCtx->pResult->result.roh;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890RohHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case ROH_INIT:
            if (pTemp890->internalTest == TRUE) {
                LtTestResultType resTmp;

                LtInitOutputStruct(&resTmp, LT_TID_ROH);
                LtMemCpy(pResults, &(resTmp.result.roh), sizeof(LtRohResultType));
            }
            /* no break */

        case ROH_GO_TO_ACTIVE:
            *pState = ROH_CHECK_FOR_ABORT_HOOK;
            pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE * 2;
            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);
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
                pTempRoh->quickStop = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;

                pTemp890->testTimer.timerVal = LT_VP890_NEXT_TICK;
                *pState = ROH_CONCLUDE;
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp890->testTimer);

                break;
            }

            /* Check raw off-hook and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &temp)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* If the raw hook bit is false (on-hook) then test is done early*/
            if (FALSE == temp) {
                pTempRoh->quickStop = TRUE;

                pTemp890->testTimer.timerVal = LT_VP890_NEXT_TICK;
                *pState = ROH_CONCLUDE;
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp890->testTimer);
                break;
            }
            /* no break if raw off-hook detected */
        }

        case ROH_SETUP:
            pTempRoh->innerState = VP890_ROH_DC_RLOOP1;
            pTempRoh->quickStop = FALSE;
            pTempRoh->imt1 = LT_VP890_ROH_DRIVE_CURRENT_1;
            pTempRoh->imt2 = LT_VP890_ROH_DRIVE_CURRENT_2;
            pTempRoh->imt3 = LT_VP890_ROH_DRIVE_CURRENT_3;

            retval = LtVp890RohMeasureDcRloop(pTestCtx);

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
                case VP890_ROH_DC_RLOOP1:
                    pTempRoh->vab1 = vabResult;
                    break;
                case VP890_ROH_DC_RLOOP2:
                    pTempRoh->vab2 = vabResult;
                    break;
                default:
                    pTempRoh->vab3 = vabResult;
                    break;
            }

            if (LT_VP890_ROH_QUICK_V_CHECK < ABS(vabResult)) {
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
            } else if (LT_VP890_ROH_QUICK_I_CHECK < ABS(ilgResult)) {
                /*
                 * Check that there are no logitudinal faults
                 */
                pTempRoh->quickStop = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
                failFlag = 1;
            }

            if ((TRUE != failFlag) && ((pTempRoh->innerState)++ < VP890_ROH_DC_RLOOP3)) {
                /* move back to the prepare state until all data is collected */
                retval = LtVp890RohMeasureDcRloop(pTestCtx);
                break;
            }
            /*  no break needed once all 3 tests data points are collected. */
        }

        case ROH_REMOVE_TARGET_CURRENT:
            *pState = ROH_CONCLUDE;
            pTempRoh->innerState = VP890_ROH_DC_RLOOP4;
            retval = LtVp890RohMeasureDcRloop(pTestCtx);
            break;

        case ROH_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                /* conclude the test if not an internal test*/
                VpTestConcludeType conclude = { FALSE };
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
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
                LtVp890CalculateResults(pTestCtx, LT_TID_ROH);
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
LtVp890RohMeasureDcRloop(
    LtTestCtxType *pTestCtx)
{
    LtVp890RohTempType *pTempRoh = &pTestCtx->pTemp->vp890Temp.tempData.roh;

    VpTestDcRLoopType dcRloopInput;
    dcRloopInput.calMode = FALSE;
    dcRloopInput.integrateTime = LT_VP890_ROH_INTEGRATE_TIME;
    dcRloopInput.settlingTime = LT_VP890_ROH_SETTLE_TIME;

    switch (pTempRoh->innerState) {
        case VP890_ROH_DC_RLOOP1:
            dcRloopInput.calMode = TRUE;
            dcRloopInput.iTestLevel =
                (int16)VP890_UNIT_CONVERT(pTempRoh->imt1, VP890_UNIT_UA, VP890_UNIT_ADC_IMET_NOR);
            break;
        case VP890_ROH_DC_RLOOP2:
            dcRloopInput.iTestLevel =
                (int16)VP890_UNIT_CONVERT(pTempRoh->imt2, VP890_UNIT_UA, VP890_UNIT_ADC_IMET_NOR);
            break;
        case VP890_ROH_DC_RLOOP3:
            dcRloopInput.iTestLevel =
                (int16)VP890_UNIT_CONVERT(pTempRoh->imt3, VP890_UNIT_UA, VP890_UNIT_ADC_IMET_NOR);
            break;
        default:
            /* used to remove the target current */
            dcRloopInput.iTestLevel = 0;
            dcRloopInput.integrateTime = 0;
            dcRloopInput.settlingTime = 0;
            break;
    }
    /* start the DC_RLOOP measurment */
    return Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DC_RLOOP, &dcRloopInput);
}

#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */





