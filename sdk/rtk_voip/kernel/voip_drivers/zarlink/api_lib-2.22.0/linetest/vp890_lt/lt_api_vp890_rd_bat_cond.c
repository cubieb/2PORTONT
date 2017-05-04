/** file lt_api_vp890_dc_feed.c
 *  This file contains the VP890 loopback routine.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */


#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_RD_BAT_COND)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    RD_BAT_COND_INIT            = LT_GLOBAL_FIRST_STATE,
    RD_BAT_COND_CALIBRATE,
    RD_BAT_COND_REQUEST_BAT1,
    RD_BAT_COND_STORE_BAT1,
    RD_BAT_COND_CONCLUDE,
    RD_BAT_COND_COMPLETE,
    RD_BAT_COND_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RD_BAT_COND_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RdBatCondStates;

extern LtTestStatusType
LtVp890RdBatCondHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890RdBatCondTempType *pTempResults = &pTemp890->tempData.rdBatCond;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890RdBatCondHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case RD_BAT_COND_INIT:
            pTempResults->vpApiLpCnd1Rslt.data = VP_INT16_MAX;
            pTempResults->vpApiLpCnd2Rslt.data = VP_INT16_MAX;
            pTempResults->vpApiLpCnd3Rslt.data = VP_INT16_MAX;

            /* This test will use this loop cond struct over and over. */
            pTemp890->loopCond.calMode = FALSE;
            pTemp890->loopCond.integrateTime =  LT_VP890_RD_BAT_COND_INTEGRATE_TIME;
            pTemp890->loopCond.settlingTime = 80;
            pTemp890->loopCond.loopCond = VP_LOOP_COND_TEST_BAT1;

        case RD_BAT_COND_CALIBRATE:
            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

        case RD_BAT_COND_REQUEST_BAT1:
            *pState = RD_BAT_COND_STORE_BAT1;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp890->loopCond);
            break;

        case RD_BAT_COND_STORE_BAT1:
            LtMemCpy(&pTempResults->vpApiLpCnd1Rslt, &pVpTestResult->result.loopCond,
                sizeof(VpTestResultLoopCondType));
            if (pVpTestResult->result.loopCond.calibrated == FALSE) {
                LtReadBatCondResultType *pResults = &pTestCtx->pResult->result.readBatCond;

                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            /* no break */

        case RD_BAT_COND_CONCLUDE: {
                VpTestConcludeType conclude = { FALSE };
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = RD_BAT_COND_COMPLETE;
                break;
            }

        case RD_BAT_COND_COMPLETE:
            /* Calculate results and end the test */
            LtVp890CalculateResults(pTestCtx, LT_TID_RD_BAT_COND);
            retval = LT_STATUS_DONE;
            break;

        case RD_BAT_COND_ABORT:
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
#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
