/** file lt_api_vp880_flt_dscrm.c
 *
 *  This file contains the PASS/FAIL Fault Discrimination Test.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_FLT_DSCRM)

#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"
typedef enum {
    FLT_DSCRM_INIT             = LT_GLOBAL_FIRST_STATE,
    FLT_DSCRM_CALIBRATE,
    FLT_DSCRM_APPLY_FEED,
    FLT_DSCRM_CONCLUDE,
    FLT_DSCRM_PRIM,
    FLT_DSCRM_COMPLETE,
    FLT_DSCRM_ABORT            = LT_GLOBAL_ABORTED_STATE,
    FLT_DSCRM_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880PreLineVStates;

extern LtTestStatusType
LtVp880FltDscrmHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    VpTestResultFltDscrmType *pTempFltDscrm = &pTemp880->tempData.fltDscrm.vpApiFltDscrmRslt;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880FltDscrmHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case FLT_DSCRM_INIT:
        case FLT_DSCRM_CALIBRATE:
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CALIBRATE, VP_NULL);
            *pState = FLT_DSCRM_APPLY_FEED;
            break;

        case FLT_DSCRM_APPLY_FEED:
            *pState = FLT_DSCRM_PRIM;
            pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE;
            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            break;


        case FLT_DSCRM_PRIM:
            *pState = FLT_DSCRM_CONCLUDE;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_FLT_DSCRM,  NULL);
            break;

        case FLT_DSCRM_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pState = FLT_DSCRM_COMPLETE;
            /* copy the data from the VP-API into the LT_API */
            LtMemCpy(pTempFltDscrm, &pVpTestResult->result.fltDscrm, sizeof(VpTestResultFltDscrmType));
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            break;
        }

        case FLT_DSCRM_COMPLETE:
            LtVp880CalculateResults(pTestCtx, LT_TID_FLT_DSCRM);
            retval = LT_STATUS_DONE;
            break;

        case FLT_DSCRM_ABORT:
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
