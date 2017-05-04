/** file lt_api_vp890_flt_dscrm.c
 *
 *  This file contains the PASS/FAIL Fault Discrimination Test.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_FLT_DSCRM)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"
typedef enum {
    FLT_DSCRM_INIT             = LT_GLOBAL_FIRST_STATE,
    FLT_DSCRM_CALIBRATE,
    FLT_DSCRM_APPLY_FEED,
    FLT_DSCRM_CONCLUDE,
    FLT_DSCRM_PRIM,
    FLT_DSCRM_COMPLETE,
    FLT_DSCRM_ABORT            = LT_GLOBAL_ABORTED_STATE,
    FLT_DSCRM_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890PreLineVStates;

extern LtTestStatusType
LtVp890FltDscrmHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    VpTestResultFltDscrmType *pTempFltDscrm = &pTemp890->tempData.fltDscrm.vpApiFltDscrmRslt;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890FltDscrmHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case FLT_DSCRM_INIT:
        case FLT_DSCRM_CALIBRATE:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CALIBRATE, VP_NULL);
            *pState = FLT_DSCRM_APPLY_FEED;
            break;

        case FLT_DSCRM_APPLY_FEED:
            *pState = FLT_DSCRM_PRIM;
            pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE;
            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);
            break;


        case FLT_DSCRM_PRIM:
            *pState = FLT_DSCRM_CONCLUDE;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_FLT_DSCRM,  NULL);
            break;

        case FLT_DSCRM_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pState = FLT_DSCRM_COMPLETE;
            /* copy the data from the VP-API into the LT_API */
            LtMemCpy(pTempFltDscrm, &pVpTestResult->result.fltDscrm, sizeof(VpTestResultFltDscrmType));
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            break;
        }

        case FLT_DSCRM_COMPLETE:
            LtVp890CalculateResults(pTestCtx, LT_TID_FLT_DSCRM);
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
#endif /*LT_VP890_VVP_PACKAGE */

#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
