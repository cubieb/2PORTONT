/** file lt_api_vp880_rd_loop_cond.c
 *
 *  This file contains the VP880 read loop condition test routine.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */
#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_RD_LOOP_COND)

#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    RD_LOOP_COND_INIT            = LT_GLOBAL_FIRST_STATE,
    RD_LOOP_COND_CALIBRATE,
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
} LtVp880RdLoopCondStates;

extern LtTestStatusType
LtVp880RdLoopCondHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880RdLoopCondTempType *pTempRdLoopCond = &pTemp880->tempData.rdLoopCond;
    int *pState = &(pTestCtx->state);
    static bool currentMeasurable = TRUE;

    LT_DOUT(LT_DBG_INFO, ("LtVp880RdLoopCondHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case RD_LOOP_COND_INIT:
            pTempRdLoopCond->vab = LT_VOLTAGE_NOT_MEASURED;
            pTempRdLoopCond->vag = LT_VOLTAGE_NOT_MEASURED;
            pTempRdLoopCond->vbg = LT_VOLTAGE_NOT_MEASURED;
            pTempRdLoopCond->imt = LT_CURRENT_NOT_MEASURED;
            pTempRdLoopCond->ilg = LT_CURRENT_NOT_MEASURED;
            pTempRdLoopCond->vbat1 = LT_VOLTAGE_NOT_MEASURED;
            pTempRdLoopCond->vbat2 = LT_VOLTAGE_NOT_MEASURED;
            pTempRdLoopCond->vbat3 = LT_VOLTAGE_NOT_MEASURED;

            if ((pTemp880->resources.initialLineState == VP_LINE_DISCONNECT) ||
                ((pTemp880->resources.initialLineState == VP_LINE_STANDBY) &&
                 ((pTemp880->termType == VP_TERM_FXS_LOW_PWR) ||
                  (pTemp880->termType == VP_TERM_FXS_ISOLATE_LP) ||
                  (pTemp880->termType == VP_TERM_FXS_SPLITTER_LP)))) {
                currentMeasurable = FALSE;
            }
            /* no break */

        case RD_LOOP_COND_CALIBRATE:
            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

        case RD_LOOP_COND_REQUEST_VAB:
            /* setup the common loop cond struct to get vab voltage */
            pTemp880->loopCond.calMode = FALSE;
            pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            *pState = RD_LOOP_COND_STORE_VAB;
            break;

        case RD_LOOP_COND_STORE_VAB:
            /* store return value from VSAB read */
            pTempRdLoopCond->vab = (int32)pVpTestResult->result.loopCond.data;
            /* no break */

        case RD_LOOP_COND_REQUEST_VAG:
            /* setup the common loop cond struct to get vag voltage */
            pTemp880->loopCond.calMode = FALSE;
            pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_VSAG;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);

            *pState = RD_LOOP_COND_STORE_VAG;
            break;

        case RD_LOOP_COND_STORE_VAG:
            /* store return value from VSAG read */
            pTempRdLoopCond->vag = (int32)pVpTestResult->result.loopCond.data;
            /* no break */

        case RD_LOOP_COND_REQUEST_VBG:
            /* setup the common loop cond struct to get vbg voltage */
            pTemp880->loopCond.calMode = FALSE;
            pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_VSBG;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);

            *pState = RD_LOOP_COND_STORE_VBG;
            break;

        case RD_LOOP_COND_STORE_VBG:
            /* store return value from VSBG read */
            pTempRdLoopCond->vbg = (int32)pVpTestResult->result.loopCond.data;
            /* no break */

        case RD_LOOP_COND_REQUEST_IMT:
            if (currentMeasurable) {
                /* modify loop cond struct to get loop current */
                pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
                pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
                pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_IMT;

                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                    &pTemp880->loopCond);
                *pState = RD_LOOP_COND_STORE_IMT;
                break;
            }

        case RD_LOOP_COND_STORE_IMT:
            if (currentMeasurable) {
                VpLineStateType lineStateCurrent = pTemp880->resources.initialLineState;

                /* store return value from IMT read */
                pTempRdLoopCond->imt = (int32)pVpTestResult->result.loopCond.data;
                /* imt scale is different in ringing */
                if ((lineStateCurrent == VP_LINE_RINGING) ||
                    (lineStateCurrent == VP_LINE_RINGING_POLREV)) {
                    pTempRdLoopCond->imt *= 2;
                }
                /* no break */
            }

        case RD_LOOP_COND_REQUEST_ILG:
            if (currentMeasurable) {
                /* modify loop cond struct to get longitudinal current */
                pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
                pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
                pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_ILG;

                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                    &pTemp880->loopCond);
                *pState = RD_LOOP_COND_STORE_ILG;
                break;
            }

        case RD_LOOP_COND_STORE_ILG:
            if (currentMeasurable) {
                /* store return value from IMT read */
                pTempRdLoopCond->ilg = (int32)pVpTestResult->result.loopCond.data;
                /* no break */
            }

        case RD_LOOP_COND_REQUEST_BAT1:
            /* modify loop cond struct to get bat1 voltage */
            pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_BAT1;

            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);

            *pState = RD_LOOP_COND_STORE_BAT1;
            break;

        case RD_LOOP_COND_STORE_BAT1:
            /* store return value from VBAT1 read */
            pTempRdLoopCond->vbat1 = (int32)pVpTestResult->result.loopCond.data;
            if (pVpTestResult->result.loopCond.calibrated == FALSE) {
                LtRdLoopCondResultType *pResults = &pTestCtx->pResult->result.rdLoopCond;

                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            /* no break */

        case RD_LOOP_COND_REQUEST_BAT2:
            /* modify loop cond struct to get bat2 voltage */
            pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_BAT2;

            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            *pState = RD_LOOP_COND_STORE_BAT2;
            break;

        case RD_LOOP_COND_STORE_BAT2:
            pTempRdLoopCond->vbat2 = (int32)pVpTestResult->result.loopCond.data;
            /* no break */

        case RD_LOOP_COND_REQUEST_BAT3:
            /* modify loop cond struct to get bat3 voltage */
            pTemp880->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp880->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_BAT3;

            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            *pState = RD_LOOP_COND_STORE_BAT3;
            break;

        case RD_LOOP_COND_STORE_BAT3:
            /* Tracker, channel 0: VBAT1 = SWITCHER_Y */
            if (pVpTestResult->result.loopCond.data == VP_INT16_MIN) {
                pTempRdLoopCond->vbat2 = LT_VOLTAGE_NOT_MEASURED;
            /* Tracker, channel 1: VBAT2 = SWITCHER_Z */
            } else if (pVpTestResult->result.loopCond.data == VP_INT16_MAX) {
                pTempRdLoopCond->vbat1 = LT_VOLTAGE_NOT_MEASURED;
            /* ABS */
            } else {
                pTempRdLoopCond->vbat3 = (int32)pVpTestResult->result.loopCond.data;
            }
            /* no break */

        case RD_LOOP_COND_CONCLUDE:
            if (FALSE == pTemp880->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = RD_LOOP_COND_COMPLETE;
                break;
            }
            /* no break */

        case RD_LOOP_COND_COMPLETE:
            /* Calculate results and end the test */
            LtVp880CalculateResults(pTestCtx, LT_TID_RD_LOOP_COND);
            retval = LT_STATUS_DONE;
            break;

        case RD_LOOP_COND_ABORT:
            /* Test aborted, results may indicate source of error. */
            retval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            LT_DOUT(LT_DBG_ERRR, ("This should never happen -> LT_STATUS_ERROR_UNKNOWN"));
            retval = LT_STATUS_ERROR_UNKNOWN;
            break;
     }

    return retval;
}


#endif /* LT_VP880_VVP_PACKAGE */
#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
