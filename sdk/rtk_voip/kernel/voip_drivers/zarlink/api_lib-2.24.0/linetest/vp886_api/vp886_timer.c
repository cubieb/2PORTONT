/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10214 $
 * $LastChangedDate: 2012-06-26 10:55:02 -0500 (Tue, 26 Jun 2012) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

/*******************************************************************************
 * Vp886TestTimer
 *
 *  Sets up an internal API timer to generate a test complete.
 * 
 ******************************************************************************/
EXTERN VpStatusType
Vp886TestTimer(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        uint16 timerValMs = 0;
        const VpTestTimerType *pTimerArgs = (VpTestTimerType *)pArgsUntyped;

        if (pTimerArgs == NULL) {
            timerValMs = 100;
        } else {
            timerValMs = pTimerArgs->timerVal / 8;
        }

        /* setup the timer */
        if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, timerValMs, 0, pTestInfo->handle)) {
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
            VP_TEST(VpLineCtxType, pLineCtx,
                ("Vp886TestTimer: Cannot add to timer que."));
            return status;
        }
        VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestTimer(): begin"));
        return status;

    } else {
        /* This code only runs once the 886 timer expires and calls back into this function */
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
        VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestTimer(): complete"));
    }

    return VP_STATUS_SUCCESS;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
