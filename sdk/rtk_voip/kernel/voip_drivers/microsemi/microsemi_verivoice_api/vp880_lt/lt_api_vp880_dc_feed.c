/** file lt_api_vp880_dc_feed.c
 *
 *  This file contains the VP880 dc feed self test routine.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_DC_FEED_ST)

#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    DC_FEED_INIT            = LT_GLOBAL_FIRST_STATE,
    DC_FEED_GO_ACTIVE,
    DC_FEED_CALIBRATE,
    DC_FEED_APPLY_FEED,
    DC_FEED_APPLY_TEST_SWITCH,
    DC_FEED_REQUEST_VAB,
    DC_FEED_STORE_VAB,
    DC_FEED_REQUEST_IMT,
    DC_FEED_STORE_IMT,
    DC_FEED_REMOVE_TEST_SWITCH,
    DC_FEED_CONCLUDE,
    DC_FEED_COMPLETE,
    DC_FEED_ABORT           = LT_GLOBAL_ABORTED_STATE,
    DC_FEED_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880DcFeedStates;

extern LtTestStatusType
LtVp880DcFeedHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880DcFeedSTTempType *pTempDcFeedSt = &pTemp880->tempData.dcFeedST;
    int *pState = &(pTestCtx->state);

    LT_DOUT(LT_DBG_INFO, ("LtVp880DcFeedHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case DC_FEED_INIT: {
            VpDeviceInfoType devInfo;
            VpStatusType vpStatus;
            devInfo.pLineCtx = pTestCtx->pLineCtx;
            vpStatus = VpGetDeviceInfo(&devInfo);
            if (VP_STATUS_SUCCESS != vpStatus) {
                pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
                LT_DOUT(LT_DBG_ERRR, ("LtVp880RingingStHandler(testState:%i,vpStatus%i)",
                    *pState, vpStatus));
                return LT_STATUS_ERROR_VP_GENERAL;
            }
            if (devInfo.featureList.testLoadSwitch == VP_AVAILABLE) {
                /* If the test load switch is available, the internal test
                 * termination is not used */
                pTempDcFeedSt->internalTestTerm = FALSE;
            } else if (devInfo.featureList.internalTestTermination == VP_AVAILABLE) {
                /* If the test load switch is NOT available and the revision is
                 * newer than VC (0x02), the internal test termination will be
                 * used */
                pTempDcFeedSt->internalTestTerm = TRUE;
            } else {
                /* If the device does not have the test load switch and cannot
                 * support the internal test termination, this test cannot be
                 * run */
                return LT_STATUS_TEST_NOT_SUPPORTED;
            }

            /* Do not run the test in disconnect or ringing if using default */
            if (pTempDcFeedSt->input.lineState == LT_DC_FEED_ST_DFLTI_LINE_STATE &&
                (pTemp880->resources.initialLineState == VP_LINE_DISCONNECT ||
                 pTemp880->resources.initialLineState == VP_LINE_RINGING) ) {
                /* Start the abort sequence forcing to conclude */
                LtVp880AbortTest(pTestCtx);
                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp880->testTimer);
                break;
            }

            /* setup the common loop cond struct for multiple measurements */
            pTemp880->loopCond.calMode = FALSE;
            pTemp880->loopCond.integrateTime =  LT_VP880_DC_FEED_INTEGRATE_TIME;
            pTemp880->loopCond.settlingTime = LT_VP880_DC_FEED_SETTLE_TIME;
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;

            *pState = DC_FEED_CALIBRATE;

            /* allow device to get completely out of low power */
            if ((pTemp880->termType == VP_TERM_FXS_LOW_PWR) ||
                (pTemp880->termType == VP_TERM_FXS_ISOLATE_LP) ||
                (pTemp880->termType == VP_TERM_FXS_SPLITTER_LP)) {
                pTemp880->testTimer.timerVal = LT_VP880_LOW_POWER_SETTLE;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);
                break;
            }

            /* no break */
        }

        case DC_FEED_CALIBRATE:
            /* VP_TEST_ID_USE_LINE_CAL does not generate an event -> no break */
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_USE_LINE_CAL, NULL);
            /* no break */

        case DC_FEED_APPLY_FEED:
            /*
             * Use the current line state if the user does not specify
             * the lineState input argument
             */

            if (pTempDcFeedSt->input.lineState != LT_DC_FEED_ST_DFLTI_LINE_STATE) {

                pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE;

                if ( VP_STATUS_SUCCESS != VpSetLineState(pTestCtx->pLineCtx,
                    pTempDcFeedSt->input.lineState)) {
                    return LT_STATUS_ERROR_UNKNOWN;
                }

                *pState = DC_FEED_APPLY_TEST_SWITCH;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);
                break;
            }
            /* no break if no line state change */

        case DC_FEED_APPLY_TEST_SWITCH:
            pTemp880->testTimer.timerVal = LT_VP880_TEST_SWITCH_SETTLE;

            if (LT_STATUS_RUNNING !=
                Vp880SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST)) {
                return retval;
            }
            *pState = DC_FEED_REQUEST_VAB;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            break;

        case DC_FEED_REQUEST_VAB:
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            *pState = DC_FEED_STORE_VAB;
            break;

        case DC_FEED_STORE_VAB:
            /* store return value from VSAB read */
            pTempDcFeedSt->vsab = (uint16)pVpTestResult->result.loopCond.data;
            /* no break */

        case DC_FEED_REQUEST_IMT:
            /* modify loop cond struct to get loop current */
            pTemp880->loopCond.loopCond = VP_LOOP_COND_TEST_IMT;

            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS,
                &pTemp880->loopCond);
            *pState = DC_FEED_STORE_IMT;
            break;

        case DC_FEED_STORE_IMT:
            /* store return value from IMT read */
            pTempDcFeedSt->imt = (uint16)pVpTestResult->result.loopCond.data;
            /* store target value (ILA) */
            pTempDcFeedSt->ila = (int16)pVpTestResult->result.loopCond.limit;
            /* no break */

        case DC_FEED_REMOVE_TEST_SWITCH:
            retval = Vp880SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);
            /* no break */

        case DC_FEED_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                &conclude);
            *pState = DC_FEED_COMPLETE;
            break;
        }
        case DC_FEED_COMPLETE:
            /* Calculate results and end the test */
            LtVp880CalculateResults(pTestCtx, LT_TID_DC_FEED_ST);
            retval = LT_STATUS_DONE;
            break;

        case DC_FEED_ABORT:
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
