/** file lt_api_vp880_calibrate.c
 *
 *  This file contains the VP880 loopback routine.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_LOOPBACK)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    LOOPBACK_INIT           = LT_GLOBAL_FIRST_STATE,
    LOOPBACK_SETUP,
    LOOPBACK_CONCLUDE,
    LOOPBACK_COMPLETE,
    LOOPBACK_ABORT          = LT_GLOBAL_ABORTED_STATE,
    LOOPBACK_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880LoopbackStates;

extern LtTestStatusType
LtVp880LoopbackHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{

    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    LtLoopbackResultType *results = &pTestCtx->pResult->result.loopback;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880LoopbackTempType *pTempLoopback = &pTemp880->tempData.loopback;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880LoopbackHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case LOOPBACK_INIT:
        case LOOPBACK_SETUP:
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOPBACK,
                &pTempLoopback->vpApiInput);
            *pState = LOOPBACK_CONCLUDE;
            break;

        case LOOPBACK_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            results->pApplicationInfo =
                pVpTestResult->result.loopback.pApplicationInfo;

            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                &conclude);
            *pState = LOOPBACK_COMPLETE;
            break;
        }
        case LOOPBACK_COMPLETE:
            results->fltMask = LT_TEST_PASSED;
            retval = LT_STATUS_DONE;
            break;

        case LOOPBACK_ABORT:
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
