/** file lt_api_vp886_roh.c
 *
 *  This file contains the VP886 receiver off-hook test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10848 $
 * $LastChangedDate: 2013-03-05 10:01:42 -0600 (Tue, 05 Mar 2013) $
 */
#include "lt_api.h"

#ifdef LT_ROH_886

#include "lt_api_internal.h"

typedef enum {
    ROH_INIT            = LT_GLOBAL_FIRST_STATE,
    ROH_GO_TO_ACTIVE,
    ROH_LINE_STATE_SETTLE,
    ROH_CHECK_FOR_ABORT_GNDKEY,
    ROH_CHECK_FOR_ABORT_HOOK,
    ROH_RUN_DC_RLOOP1,
    ROH_GET_DC_RLOOP1,
    ROH_RUN_DC_RLOOP2,
    ROH_GET_DC_RLOOP2,
    ROH_RUN_DC_RLOOP3,
    ROH_GET_DC_RLOOP3,
    ROH_RUN_DC_RLOOP4,
    ROH_GET_DC_RLOOP4,
    ROH_REMOVE_DRIVE_CURRENT,
    ROH_CONCLUDE,
    ROH_COMPLETE,
    ROH_ABORT           = LT_GLOBAL_ABORTED_STATE,
    ROH_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886RohStates;


static bool
RohHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static LtTestStatusType
MeasureDriveCurrnt(
    LtTestCtxType *pTestCtx,
    bool calMode,
    uint16 iTime,
    uint16 sTime,
    int16 uAdriveCurrent);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886RohAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RohTempType *pTempRoh = &pTemp886->tempData.roh;

    /* initialize internal flags */
    pTempRoh->quickStop = FALSE;

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = RohHandlerSM;
    return TRUE;
}

static bool
RohHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RohTempType *pTempRoh = &pTemp886->tempData.roh;
    LtRohResultType *pResults = &pTestCtx->pResult->result.roh;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("RohHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case ROH_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_ROH);
            pTestCtx->state = ROH_GO_TO_ACTIVE;
            requestNextState = TRUE;
            break;

        case ROH_GO_TO_ACTIVE:
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            pTestCtx->state = ROH_LINE_STATE_SETTLE;
            requestNextState = TRUE;
            break;

        case ROH_LINE_STATE_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = ROH_CHECK_FOR_ABORT_GNDKEY;
            break;


        case ROH_CHECK_FOR_ABORT_GNDKEY: {
            bool gndkey = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_GKEY, &gndkey);

            /* If the ground key bit is true then test is done early due to ilg */
            if (gndkey) {
                LT_DOUT(LT_DBG_INFO, ("RohHandlerSM() quick stop due to ground key"));
                pTempRoh->quickStop = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
                pTestCtx->state = ROH_CONCLUDE;
            } else {
                pTestCtx->state = ROH_CHECK_FOR_ABORT_HOOK;
            }
            requestNextState = TRUE;
            break;
        }

        case ROH_CHECK_FOR_ABORT_HOOK: {
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            /* If the raw hook bit is false (on-hook) then test is done early*/
            if (!rawHook) {
                LT_DOUT(LT_DBG_INFO, ("RohHandlerSM() quick stop due to on-hook"));
                pTempRoh->quickStop = TRUE;
                pTestCtx->state = ROH_CONCLUDE;
                pResults->fltMask = LT_TEST_PASSED;
            } else {
                pTestCtx->state = ROH_RUN_DC_RLOOP1;
            }
            requestNextState = TRUE;
            break;
        }


        /* Measuring dc rloop while driving a 30mA imt */
        case ROH_RUN_DC_RLOOP1:
            *pRetval = MeasureDriveCurrnt(pTestCtx, TRUE, 160, 800, LT_VP886_ROH_DRIVE_CURRENT_1);
            pTestCtx->state = ROH_GET_DC_RLOOP1;
            break;

        case ROH_GET_DC_RLOOP1: {
            pTempRoh->vab1 = pTestPrimRslts->result.dcvab.vab;
            pTempRoh->imt1 = LT_VP886_ROH_DRIVE_CURRENT_1;

            if (ABS(pTempRoh->vab1) > LT_VP886_ROH_QUICK_V_CHECK) {
                /* If vab exceeds on-hook limit if it does then stop the test */
                bool rawHook = FALSE;
                *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

                /*
                 * If the raw hook bit is false (on-hook) then test is done early
                 * and is passing. However, if the hook indication is TRUE (off-hhok)
                 * then we need to report a LONG RES LOOP
                 */
                if (rawHook) {
                    pResults->fltMask = LT_ROHM_OUT_OF_RANGE_LOOP;
                } else {
                    pResults->fltMask = LT_TEST_PASSED;
                }

                pTempRoh->quickStop = TRUE;
                pTestCtx->state = ROH_REMOVE_DRIVE_CURRENT;

            } else if (ABS(pTestPrimRslts->result.dcvab.ilg) > LT_VP886_ROH_QUICK_I_CHECK ) {
                /* If a logitudinal fault is present then get out  */
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
                pTempRoh->quickStop = TRUE;
                pTestCtx->state = ROH_REMOVE_DRIVE_CURRENT;
            } else {
                pTestCtx->state = ROH_RUN_DC_RLOOP2;
            }

            LT_DOUT(LT_DBG_INFO, ("RohHandlerSM() vab1 %i imt1 %i", pTempRoh->vab1, pTempRoh->imt1));
            requestNextState = TRUE;
            break;
        }


        /* Measuring dc rloop while driving a 20mA imt */
        case ROH_RUN_DC_RLOOP2:
            *pRetval = MeasureDriveCurrnt(pTestCtx, FALSE, 160, 800, LT_VP886_ROH_DRIVE_CURRENT_2);
            pTestCtx->state = ROH_GET_DC_RLOOP2;
            break;

        case ROH_GET_DC_RLOOP2:
            pTempRoh->vab2 = pTestPrimRslts->result.dcvab.vab;
            pTempRoh->imt2 = LT_VP886_ROH_DRIVE_CURRENT_2;
            pTestCtx->state = ROH_RUN_DC_RLOOP3;
            requestNextState = TRUE;
            LT_DOUT(LT_DBG_INFO, ("RohHandlerSM() vab2 %i imt2 %i", pTempRoh->vab2, pTempRoh->imt2));
            break;


        /* Measuring dc rloop while driving a 5mA imt */
        case ROH_RUN_DC_RLOOP3:
            *pRetval = MeasureDriveCurrnt(pTestCtx, FALSE, 160, 800, LT_VP886_ROH_DRIVE_CURRENT_3);
            pTestCtx->state = ROH_GET_DC_RLOOP3;
            break;

        case ROH_GET_DC_RLOOP3:
            pTempRoh->vab3 = pTestPrimRslts->result.dcvab.vab;
            pTempRoh->imt3 = LT_VP886_ROH_DRIVE_CURRENT_3;
            pTestCtx->state = ROH_RUN_DC_RLOOP4;
            requestNextState = TRUE;
            LT_DOUT(LT_DBG_INFO, ("RohHandlerSM() vab3 %i imt3 %i", pTempRoh->vab3, pTempRoh->imt3));
            break;


        /* Measuring dc rloop while driving a -5mA imt */
        case ROH_RUN_DC_RLOOP4:
            *pRetval = MeasureDriveCurrnt(pTestCtx, FALSE, 160, 800, LT_VP886_ROH_DRIVE_CURRENT_4);
            pTestCtx->state = ROH_GET_DC_RLOOP4;
            break;

        case ROH_GET_DC_RLOOP4:
            pTempRoh->vab4 = pTestPrimRslts->result.dcvab.vab;
            pTempRoh->imt4 = LT_VP886_ROH_DRIVE_CURRENT_4;
            pTestCtx->state = ROH_REMOVE_DRIVE_CURRENT;
            requestNextState = TRUE;
            LT_DOUT(LT_DBG_INFO, ("RohHandlerSM() vab4 %i imt4 %i", pTempRoh->vab4, pTempRoh->imt4));
            break;


        case ROH_REMOVE_DRIVE_CURRENT: {
            *pRetval = MeasureDriveCurrnt(pTestCtx, FALSE, 10, 10, 0);
            pTestCtx->state = ROH_CONCLUDE;
            break;
        }

        /* Start the conclude primitive if this test not a part of an internal test */
        case ROH_CONCLUDE:
            if (!pTestCtx->pTemp->vp886Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            } else {
                requestNextState = TRUE;
            }
            pTestCtx->state = ROH_COMPLETE;
            break;

        case ROH_COMPLETE:
            /* Calculate results and end the test */
            if (!pTempRoh->quickStop) {
                CalculateResults(pTestCtx);
            }
            *pRetval = LT_STATUS_DONE;
            break;

        case ROH_ABORT:
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

static LtTestStatusType
MeasureDriveCurrnt(
    LtTestCtxType *pTestCtx,
    bool calMode,
    uint16 iTime,
    uint16 sTime,
    int16 uAdriveCurrent)
{
    VpTestDcRLoopType dcRloopInput;

    dcRloopInput.calMode = calMode;
    dcRloopInput.integrateTime = iTime;
    dcRloopInput.settlingTime = sTime;
    dcRloopInput.iTestLevel = (int16)VP886_UNIT_CONVERT(uAdriveCurrent,
        VP886_UNIT_UA, VP886_UNIT_ADC_IMET_NOR);

    return Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DC_RLOOP, &dcRloopInput);
}

static void
CalculateResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RohTempType *pTempRoh = &pTemp886->tempData.roh;
    LtRohResultType *pResults = &pTestCtx->pResult->result.roh;

    int32 percentErrorLimit = 15;
    int32 percentError = 0;

    const int32 sFactor = ((VP886_UNIT_ADC_VAB * 10000) / VP886_UNIT_MV);

    int32 res1 = ((pTempRoh->vab2 - pTempRoh->vab1) * sFactor) /
        (pTempRoh->imt2 - pTempRoh->imt1);

    int32 res2 = ((pTempRoh->vab4 - pTempRoh->vab3) * sFactor) /
        (pTempRoh->imt4 - pTempRoh->imt3);

    VpLineTopologyType topInfo;

    /* need topology info to correct for resistance in the loop */
    VpQueryImmediate(pTestCtx->pLineCtx, VP_QUERY_ID_LINE_TOPOLOGY, &topInfo);
    LT_DOUT(LT_DBG_INFO, ("rOutsideDcSense %i Ohms", topInfo.rOutsideDcSense));
    LT_DOUT(LT_DBG_INFO, ("res1 %li Ohms", res1));
    LT_DOUT(LT_DBG_INFO, ("res2 %li Ohms", res2));

    res1 -= (10 * 2 * topInfo.rOutsideDcSense);
    res2 -= (10 * 2 * topInfo.rOutsideDcSense);

    if (res2 != 0) {
        percentError = (((res2 - res1) * 10000) / res2);
    }
    if ((ABS(res2) < 500) && (ABS(res1) < 500)) {
        /* values are so small it cannot be a phone off-hook */
        pResults->fltMask = LT_ROHM_RES_LOOP;
    } else if (ABS(percentError) > (percentErrorLimit * 100)) {
        pResults->fltMask = LT_ROHM_OFF_HOOK;
    } else {
        pResults->fltMask = LT_ROHM_RES_LOOP;
    }

    pResults->rLoop1 = ABS(res1);
    pResults->rLoop2 = ABS(res2);

    return;
}


#endif /* LT_ROH_886 */
