/** file vp_open_vxc.c
 *
 *  This file contains the test timer algorithm.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)
#if defined (VP880_INCLUDE_TESTLINE_CODE)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_api_testline_int.h"

typedef enum
{
    TIMER_TEST_SETUP        = VP880_TESTLINE_GLB_STRT_STATE,
    TIMER_TEST_END          = VP880_TESTLINE_GLB_END_STATE,
    TIMER_TEST_QUIT         = VP880_TESTLINE_GLB_QUIT_STATE,
    TIMER_TEST_ENUM_SIZE    = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpTimerTestStateTypes;

/*------------------------------------------------------------------------------
 * Vp880TestTimer
 * This functions implements the Timer Test primitive ...
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  handle          - unique test handle
 *  callback        - indicates if this function was called via a callback
 *  testId          - test id number
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
VpStatusType
Vp880TestTimer (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    int16 *pTestState = &pTestInfo->testState;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestTimer(ts:%i)", *pTestState));
    if (FALSE == callback) {
        *pTestState = TIMER_TEST_SETUP;
    }

    switch (*pTestState) {
        case TIMER_TEST_SETUP:

            if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
                /* make sure that starting test did not generate any errorCodes */
                VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
                VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestTimer(ec:%i): bailed",
                    pDevObj->testResults.errorCode));
                return VP_STATUS_SUCCESS;
            } else {
                VpTestTimerType *pTestInput = (VpTestTimerType*)pArgsUntyped;
                uint16 timeInPcmSamples =pTestInput->timerVal;
                uint16 tickrate = pDevObj->devProfileData.tickRate;
                uint16 ticks = PCM_TIME_TO_API_TICKS(timeInPcmSamples, tickrate);

                pDevObj->currentTest.testId = testId;
                pDevObj->currentTest.handle = handle;

                if (0 < ticks) {
                    VpSetTestStateAndTimer(pDevObj, pTestState, TIMER_TEST_END, ticks);
                    break;
                }
            }

        case TIMER_TEST_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestTimer(): complete"));
            break;

        case TIMER_TEST_QUIT:
            /* The test has eneded early due to the current errorCode */
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestTimer(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            /* The test has entered an unsuppoted state */
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestTimer(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

#endif /* VP880_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_880_SERIES */




