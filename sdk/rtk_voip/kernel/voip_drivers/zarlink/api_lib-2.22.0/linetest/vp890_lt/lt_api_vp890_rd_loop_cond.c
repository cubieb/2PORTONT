/** file lt_api_vp890_rd_loop_cond.c
 *
 *  This file contains the VP890 read loop condition test routine.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */
#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_RD_LOOP_COND)

#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

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
    RD_LOOP_COND_CONCLUDE,
    RD_LOOP_COND_COMPLETE,
    RD_LOOP_COND_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RD_LOOP_COND_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RdLoopCondStates;

extern LtTestStatusType
LtVp890RdLoopCondHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890RdLoopCondTempType *pTempRdLoopCond = &pTemp890->tempData.rdLoopCond;
    int *pState = &(pTestCtx->state);
    static bool currentMeasurable = TRUE;

    LT_DOUT(LT_DBG_INFO, ("LtVp890RdLoopCondHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
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

            /* If the device state is disconnect both ILG and IMT can not be measured */
            if ((pTemp890->resources.initialLineState == VP_LINE_DISCONNECT)
             || ((pTemp890->resources.initialLineState == VP_LINE_STANDBY) &&
                 ((pTemp890->termType == VP_TERM_FXS_LOW_PWR) ||
                  (pTemp890->termType == VP_TERM_FXS_SPLITTER_LP) ||
                  (pTemp890->termType == VP_TERM_FXS_ISOLATE_LP)))) {
                currentMeasurable = FALSE;
            }
            /* no break */

        case RD_LOOP_COND_CALIBRATE:
            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

        case RD_LOOP_COND_REQUEST_VAB:
            /* setup the common loop cond struct to get vab voltage */
            pTemp890->loopCond.calMode = FALSE;
            pTemp890->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp890->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            *pState = RD_LOOP_COND_STORE_VAB;
            break;

        case RD_LOOP_COND_STORE_VAB:
            /* store return value from VSAB read */
            pTempRdLoopCond->vab = (int16)pVpTestResult->result.loopCond.data;
            /* no break */

        case RD_LOOP_COND_REQUEST_VAG:
            /* setup the common loop cond struct to get vag voltage */
            pTemp890->loopCond.calMode = FALSE;
            pTemp890->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp890->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_VSAG;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);

            *pState = RD_LOOP_COND_STORE_VAG;
            break;

        case RD_LOOP_COND_STORE_VAG:
            /* store return value from VSAG read */
            pTempRdLoopCond->vag = (int16)pVpTestResult->result.loopCond.data;
            /* no break */

        case RD_LOOP_COND_REQUEST_VBG:
            /* setup the common loop cond struct to get vbg voltage */
            pTemp890->loopCond.calMode = FALSE;
            pTemp890->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp890->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_VSBG;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);

            *pState = RD_LOOP_COND_STORE_VBG;
            break;

        case RD_LOOP_COND_STORE_VBG:
            /* store return value from VSBG read */
            pTempRdLoopCond->vbg = (int16)pVpTestResult->result.loopCond.data;
            /* no break */

        case RD_LOOP_COND_REQUEST_IMT:
            if (currentMeasurable) {
                /* modify loop cond struct to get loop current */
                pTemp890->loopCond.settlingTime = 80;    /* 10 ms */
                pTemp890->loopCond.integrateTime = 80;   /* 10 ms */
                pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_IMT;

                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                    &pTemp890->loopCond);
                *pState = RD_LOOP_COND_STORE_IMT;
                break;
            }

        case RD_LOOP_COND_STORE_IMT:
            if (currentMeasurable) {
                VpLineStateType lineStateCurrent = pTemp890->resources.initialLineState;

                /* store return value from IMT read */
                pTempRdLoopCond->imt = (int16)pVpTestResult->result.loopCond.data;
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
                pTemp890->loopCond.settlingTime = 80;    /* 10 ms */
                pTemp890->loopCond.integrateTime = 80;   /* 10 ms */
                pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_ILG;

                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                    &pTemp890->loopCond);
                *pState = RD_LOOP_COND_STORE_ILG;
                break;
            }

        case RD_LOOP_COND_STORE_ILG:
            if (currentMeasurable) {
                /* store return value from IMT read */
                pTempRdLoopCond->ilg = (int16)pVpTestResult->result.loopCond.data;
                /* no break */
            }

        case RD_LOOP_COND_REQUEST_BAT1:
            /* modify loop cond struct to get bat1 voltage */
            pTemp890->loopCond.settlingTime = 80;    /* 10 ms */
            pTemp890->loopCond.integrateTime = 80;   /* 10 ms */
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_BAT1;

            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            *pState = RD_LOOP_COND_STORE_BAT1;
            break;

        case RD_LOOP_COND_STORE_BAT1:
            /* store return value from VBAT1 read */
            pTempRdLoopCond->vbat1 = (int16)pVpTestResult->result.loopCond.data;
            if (pVpTestResult->result.loopCond.calibrated == FALSE) {
                LtRdLoopCondResultType *pResults = &pTestCtx->pResult->result.rdLoopCond;

                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            /* no break */

        case RD_LOOP_COND_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = RD_LOOP_COND_COMPLETE;
                break;
            }
            /* no break */

        case RD_LOOP_COND_COMPLETE:
            /* Calculate results and end the test */
            LtVp890CalculateResults(pTestCtx, LT_TID_RD_LOOP_COND);
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


#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
