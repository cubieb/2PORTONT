/** file lt_api_vp890_calibrate.c
 *
 *  This file contains the VP890 loopback routine.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_LOOPBACK)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    LOOPBACK_INIT           = LT_GLOBAL_FIRST_STATE,
    LOOPBACK_SETUP,
    LOOPBACK_CONCLUDE,
    LOOPBACK_COMPLETE,
    LOOPBACK_ABORT          = LT_GLOBAL_ABORTED_STATE,
    LOOPBACK_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890LoopbackStates;

extern LtTestStatusType
LtVp890LoopbackHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    LtLoopbackResultType *results = &pTestCtx->pResult->result.loopback;
    LtVp890LoopbackTempType *pTempLoopback = &pTemp890->tempData.loopback;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890LoopbackHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case LOOPBACK_INIT:
        case LOOPBACK_SETUP:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_LOOPBACK,
                &pTempLoopback->vpApiInput);
            *pState = LOOPBACK_CONCLUDE;
            break;

        case LOOPBACK_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                results->pApplicationInfo =
                    pVpTestResult->result.loopback.pApplicationInfo;

                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
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

#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
