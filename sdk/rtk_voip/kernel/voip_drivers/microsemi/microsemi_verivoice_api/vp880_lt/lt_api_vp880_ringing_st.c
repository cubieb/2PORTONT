/** file lt_api_vp880_ringing_st.c
 *
 *  This file contains the VP880 ringing selft test routine.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 11494 $
 * $LastChangedDate: 2014-07-16 13:28:10 -0500 (Wed, 16 Jul 2014) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_RINGING_ST)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

typedef enum {
    RINGING_ST_INIT             = LT_GLOBAL_FIRST_STATE,
    RINGING_ST_GO_TO_FEED_ST,
    RINGING_ST_CHECK_FOR_ABORT_HOOK,
    RINGING_ST_LOAD_PROFILES,
    RINGING_ST_DC_RING_STATE,
    RINGING_ST_DC_RING_WAIT,
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
} LtVp880RingingSTStates;


bool
LtVp880RingingloadProfiles(
    LtTestCtxType *pTestCtx,
    LtVp880RingingStTempType *pTempRingingSt);

extern LtTestStatusType
LtVp880RingingStHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType retval = LT_STATUS_ERROR_UNKNOWN;
    VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880RingingStTempType *pTempRingingSt = &pTemp880->tempData.ringingSt;
    LtRingingSTResultType *pResults =
        &pTestCtx->pResult->result.ringingST;
    int *pState = &(pTestCtx->state);
    bool rawHook;

    LT_DOUT(LT_DBG_INFO, ("LtVp880RingingStHandler(testState:%i)", *pState));
    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case RINGING_ST_INIT: {
            VpDeviceInfoType devInfo;
            VpStatusType vpStatus;

            devInfo.pLineCtx = pTestCtx->pLineCtx;
            vpStatus = VpGetDeviceInfo(&devInfo);
            pTempRingingSt->offhook = FALSE;

            if (VP_STATUS_SUCCESS != vpStatus) {
                pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
                LT_DOUT(LT_DBG_ERRR, ("LtVp880RingingStHandler(testState:%i,vpStatus%i)",
                    *pState, vpStatus));
                return LT_STATUS_ERROR_VP_GENERAL;
            }
            if (devInfo.featureList.testLoadSwitch == VP_AVAILABLE) {
                /* If the test load switch is available, the internal test
                 * termination is not used */
                pTempRingingSt->internalTestTerm = FALSE;
            } else if (devInfo.featureList.internalTestTermination == VP_AVAILABLE) {
                /* If the test load switch is NOT available and the revision is
                 * newer than VC (0x02), the internal test termination will be
                 * used */
                pTempRingingSt->internalTestTerm = TRUE;
            } else {
                /* If the device does not have the test load switch and cannot
                 * support the internal test termination, this test cannot be
                 * run */
                return LT_STATUS_TEST_NOT_SUPPORTED;
            }

            /* no break */
        }
        case RINGING_ST_GO_TO_FEED_ST: {
            VpLineStateType lineState = VP_LINE_ACTIVE;
            VpLineStateType lineStateCurrent = pTemp880->resources.initialLineState;
            pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE;


            /* go to normal active unless in a pol rev state */
            if ((lineStateCurrent == VP_LINE_ACTIVE_POLREV) ||
                (lineStateCurrent == VP_LINE_TALK_POLREV) ||
                (lineStateCurrent == VP_LINE_OHT_POLREV)) {
                lineState = VP_LINE_ACTIVE_POLREV;
            }

            /* change the line state */
            if ( VP_STATUS_SUCCESS != VpSetLineState(pTestCtx->pLineCtx, lineState)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            *pState = RINGING_ST_CHECK_FOR_ABORT_HOOK;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp880->testTimer);
            break;
        }

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
                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                *pState = RINGING_ST_CONCLUDE;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);
                break;
            }
            /* no break if on-hook */

        case RINGING_ST_LOAD_PROFILES:
            if (LtVp880RingingloadProfiles(pTestCtx, pTempRingingSt)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            /* no break */

        case RINGING_ST_DC_RING_STATE: {
            /* Go to the ringing line state.  Do this directly instead of through 
             * VpSetLineState() to avoid using the customer's ring cadence. */

            uint8 mpiBuf = 0x27;
            VpTestVpMpiCmdType regInput;
            regInput.cmd = 0x56;
            regInput.cmdLen = 0x01;
            VpMemCpy(&regInput.buffer, &mpiBuf, regInput.cmdLen);

            *pState = RINGING_ST_DC_RING_WAIT;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_VP_MPI_CMD, &regInput);
            break;
        }
        case RINGING_ST_DC_RING_WAIT: {
            /* wait for the line to settle */
            pTemp880->testTimer.timerVal = LT_VP880_STATE_CHANGE_SETTLE;
            *pState = RINGING_ST_APPLY_TEST_SWITCH;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp880->testTimer);
            break;
        }

        case RINGING_ST_APPLY_TEST_SWITCH:
            pTemp880->testTimer.timerVal = LT_VP880_RINGTRIP_DELAY;

            if (LT_STATUS_RUNNING !=
                Vp880SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST)) {
                return retval;
            }
            *pState = RINGING_ST_CHECK_RING_TRIP;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            break;

        case RINGING_ST_CHECK_RING_TRIP:

            /* force cadence to stop */
            VpSetLineTone(pTestCtx->pLineCtx, NULL, NULL, NULL);

            /* Check hook bit */
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK,
                    &pTempRingingSt->ringTrip)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            /* no break */

        case RINGING_ST_REQUEST_AC_RLOOP_1:
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                &pTempRingingSt->vpApiAcRloopInput);
            *pState = RINGING_ST_STORE_AC_LOOP_1_RESULTS;
            break;

        case RINGING_ST_STORE_AC_LOOP_1_RESULTS:
            LtMemCpy(&pTempRingingSt->vpApiAcRloop1Rslt,
                &pVpTestResult->result.acimt,
                sizeof(VpTestResultAcRlType));
            /* no break */

        case RINGING_ST_REMOVE_TEST_SWITCH:
            pTemp880->testTimer.timerVal = LT_VP880_TEST_SWITCH_SETTLE;

            if (LT_STATUS_RUNNING !=
                Vp880SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL)) {
                return retval;
            }
            *pState = RINGING_ST_REQUEST_AC_RLOOP_2;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                &pTemp880->testTimer);
            break;

        case RINGING_ST_REQUEST_AC_RLOOP_2:
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP,
                &pTempRingingSt->vpApiAcRloopInput);
            *pState = RINGING_ST_STORE_AC_LOOP_2_RESULTS;
            break;

        case RINGING_ST_STORE_AC_LOOP_2_RESULTS:
            LtMemCpy(&pTempRingingSt->vpApiAcRloop2Rslt,
                &pVpTestResult->result.acimt,
                sizeof(VpTestResultAcRlType));
            /* no break */

        case RINGING_ST_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                &conclude);
            *pState = RINGING_ST_COMPLETE;
            break;
        }

        case RINGING_ST_COMPLETE:
            /* Calculate results and end the test */
            LtVp880CalculateResults(pTestCtx, LT_TID_RINGING_ST);
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
LtVp880RingingloadProfiles(
    LtTestCtxType *pTestCtx,
    LtVp880RingingStTempType *pTempRingingSt)
{

    VpLineCtxType *pLineCtx = pTestCtx->pLineCtx;

    if ((VP_PTABLE_NULL == pTempRingingSt->input.pRingProfileForTest) ||
        (VP_PTABLE_NULL == pTempRingingSt->input.pDcProfileForTest)) {

        /* Sine, 20 Hz, 1.00 Vpk, -25.00 Bias */
        const VpProfileDataType RING_SELF_TEST[] = {
         0x00, 0x04, 0x01, 0x12, 0x00, 0x0E, 0xD2, 0x00, 0xEB, 0x46, 0x00, 0x36,
         0x00, 0xD4, 0x00, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x00};

        switch (pTestCtx->pTemp->vp880Temp.productCode) {
            case 0x81:
            case 0x82:
            case 0x83:
            case 0x85:
            case 0x86:
            case 0x87:
            case 0xB1:
            case 0xB2:
            case 0xB3:
            case 0xB5:
            case 0xB6:
            case 0xB7:
            case 0xF3: {
                /* DC Profile */
                const VpProfileDataType DC_13MA[] = {
                 0x00, 0x01, 0x00, 0x0A, 0x00, 0x08, 0xC2, 0x1B, 0x84, 0x1A, 0x07,
                 0xC6, 0x29, 0x04};

                if (VP_STATUS_SUCCESS !=
                    VpConfigLine(pLineCtx, VP_PTABLE_NULL,
                        DC_13MA, RING_SELF_TEST)) {
                        return 1;
                }
                break;
            }

            default: {
                /* DC Profile */
                const VpProfileDataType DC_13MA[] = {
                 0x00, 0x01, 0x00, 0x0A, 0x00, 0x08, 0xC2, 0x1B, 0x84, 0x1A, 0x05,
                 0xC6, 0x2B, 0x08};

                if (VP_STATUS_SUCCESS !=
                    VpConfigLine(pLineCtx, VP_PTABLE_NULL, DC_13MA, RING_SELF_TEST))  {
                        return 1;
                }
                break;
            }
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


#endif /* LT_VP880_VVP_PACKAGE */
#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
