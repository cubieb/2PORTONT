/** file lt_api_vp890_ringing_st.c
 *
 *  This file contains the VP890 ringing selft test routine.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE) && defined(LT_RINGING_ST)
#ifdef LT_VP890_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp890.h"

typedef enum {
    RINGING_ST_INIT             = LT_GLOBAL_FIRST_STATE,
    RINGING_ST_CHECK_FOR_ABORT_HOOK,
    RINGING_ST_LOAD_PROFILES,
    RINGING_ST_GO_TO_RINGING,
    RINGING_ST_APPLY_TEST_SWITCH,
    RINGING_ST_CHECK_RING_TRIP,
    RINGING_ST_REQUEST_AC_RLOOP_1,
    RINGING_ST_STORE_AC_LOOP_1_RESULTS,
    RINGING_ST_REMOVE_TEST_SWITCH,
    RINGING_ST_REQUEST_AC_RLOOP_2,
    RINGING_ST_STORE_AC_LOOP_2_RESULTS,
    RINGING_ST_CONCLUDE,
    RINGING_ST_COMPLETE,
    RINGING_ST_ABORT            = LT_GLOBAL_ABORTED_STATE,
    RINGING_ST_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RingingSTStates;


bool
LtVp890RingingloadProfiles(
    VpLineCtxType *pLineCtx,
    LtVp890RingingStTempType *pTempRingingSt);

extern LtTestStatusType
LtVp890RingingStHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp890TestTempType *pTemp890 = &pTestCtx->pTemp->vp890Temp;
    LtVp890RingingStTempType *pTempRingingSt = &pTemp890->tempData.ringingSt;
    LtRingingSTResultType *pResults =
        &pTestCtx->pResult->result.ringingST;
    int *pState = &(pTestCtx->state);
    bool rawHook;

    LT_DOUT(LT_DBG_INFO, ("LtVp890RingingStHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp890EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case RINGING_ST_INIT:
        case RINGING_ST_CHECK_FOR_ABORT_HOOK:
            /* Check abort hook bit and error if something goes wrong */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &rawHook)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* If the device raw hook bit is true (off-hook) then test is aborted */
            if (TRUE == rawHook) {
                pTempRingingSt->offhook = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pTemp890->testTimer.timerVal = LT_VP890_NEXT_TICK;
                *pState = RINGING_ST_CONCLUDE;
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp890->testTimer);
                break;
            }
            /* no break if on-hook */

        case RINGING_ST_LOAD_PROFILES:
            if (LtVp890RingingloadProfiles(pTestCtx->pLineCtx, pTempRingingSt)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            /* no break */

        case RINGING_ST_GO_TO_RINGING:
            pTemp890->testTimer.timerVal = LT_VP890_STATE_CHANGE_SETTLE;

            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, VP_LINE_RINGING)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            *pState = RINGING_ST_APPLY_TEST_SWITCH;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);
            break;

        case RINGING_ST_APPLY_TEST_SWITCH:
            pTemp890->testTimer.timerVal = LT_VP890_RINGTRIP_DELAY;

            if (LT_STATUS_RUNNING !=
                Vp890SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST)) {
                return retval;
            }
            *pState = RINGING_ST_CHECK_RING_TRIP;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);
            break;

        case RINGING_ST_CHECK_RING_TRIP:
            /* Check hook bit */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK,
                    &pTempRingingSt->ringTrip)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            /* no break */

        case RINGING_ST_REQUEST_AC_RLOOP_1:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                &pTempRingingSt->vpApiAcRloopInput);
            *pState = RINGING_ST_STORE_AC_LOOP_1_RESULTS;
            break;

        case RINGING_ST_STORE_AC_LOOP_1_RESULTS:
            LtMemCpy(&pTempRingingSt->vpApiAcRloop1Rslt,
                &pVpTestResult->result.acimt,
                sizeof(VpTestResultAcRlType));
            /* no break */

        case RINGING_ST_REMOVE_TEST_SWITCH:
            pTemp890->testTimer.timerVal = LT_VP890_TEST_SWITCH_SETTLE;

            if (LT_STATUS_RUNNING !=
                Vp890SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL)) {
                return retval;
            }
            *pState = RINGING_ST_REQUEST_AC_RLOOP_2;
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp890->testTimer);
            break;

        case RINGING_ST_REQUEST_AC_RLOOP_2:
            retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                &pTempRingingSt->vpApiAcRloopInput);
            *pState = RINGING_ST_STORE_AC_LOOP_2_RESULTS;
            break;

        case RINGING_ST_STORE_AC_LOOP_2_RESULTS:
            LtMemCpy(&pTempRingingSt->vpApiAcRloop2Rslt,
                &pVpTestResult->result.acimt,
                sizeof(VpTestResultAcRlType));
            /* no break */

        case RINGING_ST_CONCLUDE:
            if (FALSE == pTemp890->internalTest) {
                VpTestConcludeType conclude = { FALSE };
                retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = RINGING_ST_COMPLETE;
                break;
            }

        case RINGING_ST_COMPLETE:
            /* Calculate results and end the test */
            LtVp890CalculateResults(pTestCtx, LT_TID_RINGING_ST);
            retval = LT_STATUS_DONE;
            break;

        case RINGING_ST_ABORT:
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

bool
LtVp890RingingloadProfiles(
    VpLineCtxType *pLineCtx,
    LtVp890RingingStTempType *pTempRingingSt)
{
    if ((VP_PTABLE_NULL == pTempRingingSt->input.pRingProfileForTest) ||
        (VP_PTABLE_NULL == pTempRingingSt->input.pDcProfileForTest)) {

        /* Sine, 20 Hz, 1.00 Vpk, -25.00 Bias */
        const VpProfileDataType RING_SELF_TEST[] = {
         0x00, 0x04, 0x01, 0x12, 0x00, 0x0E, 0xD2, 0x00, 0xEB, 0x46, 0x00, 0x36,
         0x00, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00};

        /* DC Profile */
        const VpProfileDataType DC_13MA[] = {
         0x00, 0x01, 0x00, 0x0A, 0x00, 0x08, 0xC2, 0x1B, 0x84, 0x1A, 0x05,
         0xC6, 0x2B, 0x08};

        if (VP_STATUS_SUCCESS !=
            VpConfigLine(pLineCtx, VP_PTABLE_NULL, DC_13MA, RING_SELF_TEST))
        {
            return 1;
        }
    } else {
        /* Load user ring and dc profiles */
        if (VP_STATUS_SUCCESS !=
            VpConfigLine(pLineCtx, VP_PTABLE_NULL,
             pTempRingingSt->input.pDcProfileForTest,
             pTempRingingSt->input.pRingProfileForTest))
        {
            return 1;
        }
    }
    return 0;
}


#endif /* LT_VP890_VVP_PACKAGE */
#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
