/** file lt_api_vp880_cross_connect.c
 *
 *  This file contains the Cross Connect Detection Test.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 6419 $
 * $LastChangedDate: 2010-02-12 16:40:10 -0600 (Fri, 12 Feb 2010) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_XCONNECT)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    XCONNECT_INIT            = LT_GLOBAL_FIRST_STATE,
    XCONNECT_CALIBRATE,
    XCONNECT_START,
    XCONNECT_GET_RESULTS,
    XCONNECT_CALCULATE,
    XCONNECT_CONCLUDE,
    XCONNECT_COMPLETE,
    XCONNECT_ABORT           = LT_GLOBAL_ABORTED_STATE,
    XCONNECT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880MStates;

extern LtTestStatusType
LtVp880XConnectHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880XConnectTempType *pTempXConnect = &pTemp880->tempData.xConnect;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880XConnectHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case XCONNECT_INIT:
            pTempXConnect->phase = VP880_XCONNECT_PHASE1;
            /* no break */

        case XCONNECT_CALIBRATE:
            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

        case XCONNECT_START: {
            VpTestXConnectType xCinput;

            xCinput.integrateTime = 800;            /* 800 / 125us = 100ms */
            xCinput.settlingTime = 400;             /* 400 / 125us = 50ms */

            switch (pTempXConnect->phase) {
                case VP880_XCONNECT_PHASE1:
                    xCinput.settlingTime = 1600;
                    xCinput.measReq = VP_XCONNECT_DISC_V;
                    xCinput.phase = 0;
                    break;

                case VP880_XCONNECT_PHASE2:
                    xCinput.measReq = VP_XCONNECT_12VVOC_I;
                    xCinput.isFeedPositive = pTempXConnect->isEMFPositive;
                    xCinput.phase = 1;
                    break;

                case VP880_XCONNECT_PHASE3:
                    xCinput.measReq = VP_XCONNECT_DISC_V;
                    xCinput.phase = 2;
                    break;

                default:
                    break;
            }

            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_XCONNECT, &xCinput);
            *pState = XCONNECT_GET_RESULTS;
            break;
        }

        case XCONNECT_GET_RESULTS:
            /* store all results into the temp structure */
            LtMemCpy(&pTempXConnect->vpApiXConnectResults, &pVpTestResult->result.xConnect,
                sizeof(VpTestResultXConnectType));
            /* no break */

        case XCONNECT_CALCULATE:
            /* Calculate results */
            LtVp880CalculateResults(pTestCtx, LT_TID_XCONNECT);

            /* Check if extra steps are needed to detect the cross connect */
            if (pTempXConnect->phase != VP880_XCONNECT_DONE) {
                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp880->testTimer);
                *pState = XCONNECT_START;
                break;
            }
            /* no break */

        case XCONNECT_CONCLUDE:
            if (FALSE == pTestCtx->pTemp->vp880Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
                *pState = XCONNECT_COMPLETE;
                break;
             }

        case XCONNECT_COMPLETE:
            retval = LT_STATUS_DONE;
            break;

        case XCONNECT_ABORT:
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
