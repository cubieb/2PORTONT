/** file lt_api_vp880_dc_feed.c
 *  This file contains the VP880 loopback routine.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */


#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_RD_BAT_COND)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    RD_BAT_COND_INIT            = LT_GLOBAL_FIRST_STATE,
    RD_BAT_COND_REQUEST_BAT1,
    RD_BAT_COND_STORE_BAT1,
    RD_BAT_COND_REQUEST_BAT2,
    RD_BAT_COND_STORE_BAT2,
    RD_BAT_COND_REQUEST_BAT3,
    RD_BAT_COND_STORE_BAT3,
    RD_BAT_COND_CONCLUDE,
    RD_BAT_COND_COMPLETE,
    RD_BAT_COND_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RD_BAT_COND_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880RdBatCondStates;

extern LtTestStatusType
LtVp880RdBatCondHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880RdBatCondTempType *pTempResults = &pTemp880->tempData.rdBatCond;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880RdBatCondHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case RD_BAT_COND_INIT:
            /* This test will use this loop cond struct over and over. */
            pTemp880->loopCond.calMode = FALSE;
            pTemp880->loopCond.integrateTime =  LT_VP880_RD_BAT_COND_INTEGRATE_TIME;
            pTemp880->loopCond.settlingTime = LT_VP880_RD_BAT_COND_SETTLE_TIME;
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_BAT1;
           /* no break */

        case RD_BAT_COND_REQUEST_BAT1:
            *pState = RD_BAT_COND_STORE_BAT1;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            break;

        case RD_BAT_COND_STORE_BAT1:
            LtMemCpy(&pTempResults->vpApiLpCnd1Rslt,
                &pVpTestResult->result.loopCond,
                sizeof(VpTestResultLoopCondType));
            if (pVpTestResult->result.loopCond.calibrated == FALSE) {
                LtReadBatCondResultType *pResults = &pTestCtx->pResult->result.readBatCond;

                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            /* no break */

        case RD_BAT_COND_REQUEST_BAT2:
            *pState = RD_BAT_COND_STORE_BAT2;
            pTemp880->loopCond.settlingTime = LT_VP880_RD_BAT_COND_SETTLE_TIME;
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_BAT2;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            break;

        case RD_BAT_COND_STORE_BAT2:
            LtMemCpy(&pTempResults->vpApiLpCnd2Rslt,
                &pVpTestResult->result.loopCond,
                sizeof(VpTestResultLoopCondType));
            /* no break */

        case RD_BAT_COND_REQUEST_BAT3:
            *pState = RD_BAT_COND_STORE_BAT3;
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_BAT3;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            break;

        case RD_BAT_COND_STORE_BAT3:
            /* The primitive returns VP_INT16_MIN or VP_INT16_MAX depending of the channel */
            /* To be compliant with the cli, we need to return only VP_INT16_MAX */
            if (pVpTestResult->result.loopCond.data == VP_INT16_MIN) {
                pVpTestResult->result.loopCond.data = VP_INT16_MAX;
            }
            LtMemCpy(&pTempResults->vpApiLpCnd3Rslt,
                &pVpTestResult->result.loopCond,
                sizeof(VpTestResultLoopCondType));
            /* no break */

        case RD_BAT_COND_CONCLUDE:  {
            VpTestConcludeType conclude = { FALSE };
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                &conclude);
            *pState = RD_BAT_COND_COMPLETE;
            break;
        }

        case RD_BAT_COND_COMPLETE:
            /* Calculate results and end the test */
            LtVp880CalculateResults(pTestCtx, LT_TID_RD_BAT_COND);
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
#endif /* LT_VP880_VVP_PACKAGE */
#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
