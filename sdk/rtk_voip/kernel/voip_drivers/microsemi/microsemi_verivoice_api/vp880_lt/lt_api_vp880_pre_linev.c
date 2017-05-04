/** file lt_api_vp880_pre_linev.c
 *
 *  This file contains the PASS/FAIL Line Voltage test.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_PRE_LINE_V)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"
typedef enum {
    PRE_LINE_V_INIT             = LT_GLOBAL_FIRST_STATE,
    PRE_LINE_V_CHECK_STATUS1,
    PRE_LINE_V_REVERSE_POLARITY,
    PRE_LINE_V_CHECK_STATUS2,
    PRE_LINE_V_CONCLUDE,
    PRE_LINE_V_COMPLETE,
    PRE_LINE_V_ABORT            = LT_GLOBAL_ABORTED_STATE,
    PRE_LINE_V_ENUM_SIZE        = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880PreLineVStates;

#define VP880_LOW_PREP_SETTLE 320 /* 40 ms */
#define VP880_LOW_POLREV_SETTLE 1200 /* 150 ms */

#ifndef VP880_DC_FEED_LEN
#define VP880_DC_FEED_LEN   0x02
#endif

extern LtTestStatusType
LtVp880PreLineVHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType    retval          = LT_STATUS_ERROR_UNKNOWN;
    LtVp880TestTempType *pTemp880       = &pTestCtx->pTemp->vp880Temp;
    LtPreLineVResultType *pResults      = &pTestCtx->pResult->result.preLineV;
    int                 *pState         = &(pTestCtx->state);
    bool                internalTest    = pTestCtx->pTemp->vp880Temp.internalTest;

    LT_DOUT(LT_DBG_INFO, ("LtVp880PreLineVHandler(testState:%i)", *pState));

    /* Check for, store and return any VP-API or LT_API errors */
    if (TRUE == Vp880EventErrorCheck(pTestCtx, pEvent, &retval)) {
        return retval;
    }

    switch (*pState) {
        case PRE_LINE_V_INIT: {
            /* set ila to its max value to help with high REN loads*/
            /*
             * TO DO: This also sets VOC = 42V, and VAS = 9V. VAS is not likely
             * correct, but should VOC be set to 42V or just keep where it is?
             */
            uint8 dcfeed[VP880_DC_FEED_LEN] = {0x0A, 0x1F};

            LT_DOUT(LT_DBG_INFO, ("LtVp880PreLineVHandler(termType%i)", \
                pTemp880->termType ));

            if ( VP_STATUS_SUCCESS !=
                VpLowLevelCmd(pTestCtx->pLineCtx, dcfeed, VP880_DC_FEED_LEN,
                    pTestCtx->handle)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /*
             * do to the time it takes to for a line to get out of low
             * power mode during test prepare, we must wait for the
             * circuit to settle before running the measurements
             */
            pTemp880->testTimer.timerVal = VP880_LOW_PREP_SETTLE;
            *pState = PRE_LINE_V_CHECK_STATUS1;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,  &pTemp880->testTimer);

            break;
        }

        case PRE_LINE_V_CHECK_STATUS1: {
            bool hook = FALSE;
            bool gkey = FALSE;

            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &hook)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_GKEY, &gkey)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            if (hook || gkey) {
                pTemp880->testTimer.timerVal = LT_VP880_NEXT_TICK;
                *pState = PRE_LINE_V_CONCLUDE;
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY,
                    &pTemp880->testTimer);
                pResults->fltMask = LT_PLVM_FAILED;
                break;
            }
            /* no break */
        }

        case PRE_LINE_V_REVERSE_POLARITY: {
            VpLineStateType lineState;

            if ( VP_STATUS_SUCCESS !=
                VpGetLineState(pTestCtx->pLineCtx, &lineState)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            /* go to normal active unless in a pol rev state */
            if ((lineState == VP_LINE_ACTIVE_POLREV) ||
                (lineState == VP_LINE_TALK_POLREV) ||
                (lineState == VP_LINE_OHT_POLREV)) {
                lineState = VP_LINE_ACTIVE;
            } else {
                lineState = VP_LINE_ACTIVE_POLREV;
            }

            if ( VP_STATUS_SUCCESS !=
                VpSetLineState(pTestCtx->pLineCtx, lineState)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            pTemp880->testTimer.timerVal = VP880_LOW_POLREV_SETTLE;
            *pState = PRE_LINE_V_CHECK_STATUS2;
            retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp880->testTimer);
            break;
        }

        case PRE_LINE_V_CHECK_STATUS2: {
            bool hook = FALSE;
            bool gkey = FALSE;

            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_RAW_HOOK, &hook)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }
            if ( VP_STATUS_SUCCESS !=
                VpGetLineStatus(pTestCtx->pLineCtx, VP_INPUT_GKEY, &gkey)) {
                return LT_STATUS_ERROR_UNKNOWN;
            }

            if (hook || gkey) {
                pResults->fltMask = LT_PLVM_FAILED;
            } else {
                pResults->fltMask = LT_TEST_PASSED;
            }
            /* no break */
        }

        case PRE_LINE_V_CONCLUDE:
            if (FALSE == internalTest) {
                VpTestConcludeType conclude = { FALSE };
                /* conclude the test if not an internal test*/
                retval = Vp880TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                    &conclude);
                *pState = PRE_LINE_V_COMPLETE;
                break;
            }
            /* no break */

        case PRE_LINE_V_COMPLETE:
            retval = LT_STATUS_DONE;
            break;

        case PRE_LINE_V_ABORT:
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
