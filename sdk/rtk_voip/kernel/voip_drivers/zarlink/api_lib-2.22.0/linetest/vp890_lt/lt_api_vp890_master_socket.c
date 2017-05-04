/** file lt_api_vp890_master_socket.c
 *
 *  This file contains the Master socket test Test.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6419 $
 * $LastChangedDate: 2010-02-12 16:40:10 -0600 (Fri, 12 Feb 2010) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_MSOCKET)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    MSOCKET_INIT            = LT_GLOBAL_FIRST_STATE,
    MSOCKET_DIS,
    MSOCKET_START,
    MSOCKET_GET_RESULTS,
    MSOCKET_CALCULATE,
    MSOCKET_CONCLUDE,
    MSOCKET_COMPLETE,
    MSOCKET_ABORT           = LT_GLOBAL_ABORTED_STATE,
    MSOCKET_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890MStates;

extern LtTestStatusType
LtVp890MSocketHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890MSocketTempType *pTempMSocket = &pTemp890->tempData.mSocket;
    LtMSocketType mSocketType = pTempMSocket->input.mSocket;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp890MSocketHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case MSOCKET_INIT:
            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

        case MSOCKET_DIS: {
            pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE;
            if (VP_STATUS_SUCCESS != VpSetLineState(pTestCtx->pLineCtx, VP_LINE_ACTIVE)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            *pState = MSOCKET_START;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp890->testTimer);
            break;
        }

        case MSOCKET_START:
            switch (mSocketType) {
                case LT_MSOCKET_TYPE_1: {
                    VpTest3EleResAltResType lowGainInput;

                    lowGainInput.feedBias = 0x09F3;         /* +/- 6V of feed */
                    lowGainInput.shiftZeroFeed = FALSE;     /* constant longitudinal voltage */
                    lowGainInput.integrateTime = 18;        /* 18ms */
                    lowGainInput.settlingTime = 2;          /* 2ms */

                    retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_RES_LG, &lowGainInput);
                    break;
                }

                case LT_MSOCKET_TYPE_2: {
                    VpTestMSocketType mSInput;

                    mSInput.dummy = 2;

                    retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_MSOCKET_TYPE2, &mSInput);
                    break;
                }

                default:
                    LT_DOUT(LT_DBG_ERRR, ("Unknown master socket type"));
                    break;
            }
            *pState = MSOCKET_GET_RESULTS;
            break;

        case MSOCKET_GET_RESULTS:
            /* store all results into the temp structure */
            switch (mSocketType) {
                case LT_MSOCKET_TYPE_1:
                    LtMemCpy(&pTempMSocket->vpApiMSocketType1Results, &pVpTestResult->result.resAlt,
                        sizeof(VpTestResultAltResType));
                    break;

                case LT_MSOCKET_TYPE_2:
                    LtMemCpy(&pTempMSocket->vpApiMSocketType2Results, &pVpTestResult->result.mSock,
                        sizeof(VpTestResultMSockType));
                    break;

                default:
                    LT_DOUT(LT_DBG_ERRR, ("Unknown master socket type"));
                    break;
            }
            /* no break */

        case MSOCKET_CALCULATE:
            /* Calculate results */
            LtVp890CalculateResults(pTestCtx, LT_TID_MSOCKET);
            /* no break */

        case MSOCKET_CONCLUDE:
            if (FALSE == pTestCtx->pTemp->vp890Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = MSOCKET_COMPLETE;
                break;
             }

        case MSOCKET_COMPLETE:
            retval = LT_STATUS_DONE;
            break;

        case MSOCKET_ABORT:
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
