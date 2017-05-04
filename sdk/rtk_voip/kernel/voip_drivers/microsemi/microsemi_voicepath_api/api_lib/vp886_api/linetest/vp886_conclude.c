/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11350 $
 * $LastChangedDate: 2014-03-18 16:43:27 -0500 (Tue, 18 Mar 2014) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

typedef enum
{
    CONCLUDE_DISCONNECT         = 0,
    CONCLUDE_RESTORE_DEV        = 1,
    CONCLUDE_RESTORE_STATE      = 2,
    CONCLUDE_DONE               = 3,
    CONCLUDE_MAX                = 3,
    CONCLUDE_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
} vpConcludeTestStateTypes;

bool Vp886TestConcludeSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestConcludeType *pConcludeArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

/*******************************************************************************
 * Vp886TestConclude
 *
 *  Restore device register values.
 *  The main steps of this primitive are:
 *
 *    - write register values stored by prepare back to the device.
 *    - release the test heap memory back to the system.
 *
 ******************************************************************************/
VpStatusType
Vp886TestConclude (
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{
    VpTestConcludeType *pConcludeArgs = NULL;

    if (!callback) {
        const VpTestConcludeType *pInputArgs = (VpTestConcludeType *)pArgsUntyped;

        if ((pInputArgs == NULL) || (pInputArgs->abortTest)) {
            pTestHeap->testArgs.conclude.abortTest = TRUE;
        } else {
            pTestHeap->testArgs.conclude.abortTest = TRUE;
        }

        pTestHeap->nextState = CONCLUDE_DISCONNECT;
    }

    pLineObj->testInfo.concluding = TRUE;
    pConcludeArgs = &pTestHeap->testArgs.conclude;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestConcludeSM(pLineCtx, pLineObj, pConcludeArgs, pTestInfo, pTestHeap));

    /* if an error was generated then we are done concluding */
    if ( pLineObj->testResults.errorCode != VP_TEST_STATUS_SUCCESS) {
        pTestInfo->concluding = FALSE;
    }

    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp886TestConcludeSM
 *----------------------------------------------------------------------------*/
bool Vp886TestConcludeSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestConcludeType *pConcludeArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    bool requestNextState = FALSE;
    pLineObj->testResults.errorCode = VP_TEST_STATUS_SUCCESS;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestConcludeSM state: %i", pTestHeap->nextState));
    switch (pTestHeap->nextState) {

        case CONCLUDE_DISCONNECT: {
            /* Do not go in disconnect for the non-intrusive tests: RdLoopCond/Loopback */
            if (pLineObj->testInfo.nonIntrusiveTest) {
                requestNextState = TRUE;
            } else {
                uint8 sysState = (VP886_R_STATE_CODEC | VP886_R_STATE_SS_DISCONNECT);
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, &sysState);

                /* setup a 10 ms timer to ensure we get into disconnect before we try to restore registers */
                if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 10, 0, CONCLUDE_DISCONNECT)) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestConcludeSM: Cannot add to timer que."));
                }
            }

            pTestHeap->nextState = CONCLUDE_RESTORE_DEV;
            break;
        }

        case CONCLUDE_RESTORE_DEV: {
            /* restore all cached register from the test heap */
            VpMemCpy(pLineObj->registers.loopSup, pTestHeap->scratch.loopSup, VP886_R_LOOPSUP_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, pTestHeap->scratch.loopSup);

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_DISN_WRT, VP886_R_DISN_LEN, pTestHeap->scratch.disn);

            VpMemCpy(pLineObj->registers.opCond, pTestHeap->scratch.opCond, VP886_R_OPCOND_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, pTestHeap->scratch.opCond);

            VpMemCpy(pLineObj->registers.opFunc, pTestHeap->scratch.opFunc, VP886_R_OPFUNC_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPFUNC_WRT, VP886_R_OPFUNC_LEN, pTestHeap->scratch.opFunc);

            VpMemCpy(pLineObj->registers.swParam, pTestHeap->scratch.swParam, VP886_R_SWPARAM_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, pTestHeap->scratch.swParam);

            VpMemCpy(pLineObj->registers.icr1, pTestHeap->scratch.icr1, VP886_R_ICR1_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pTestHeap->scratch.icr1);

            VpMemCpy(pLineObj->registers.icr2, pTestHeap->scratch.icr2, VP886_R_ICR2_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pTestHeap->scratch.icr2);

            VpMemCpy(pLineObj->registers.icr3, pTestHeap->scratch.icr3, VP886_R_ICR3_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pTestHeap->scratch.icr3);

            VpMemCpy(pLineObj->registers.icr4, pTestHeap->scratch.icr4, VP886_R_ICR4_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pTestHeap->scratch.icr4);

            VpMemCpy(pLineObj->registers.dcFeed, pTestHeap->scratch.dcFeed, VP886_R_DCFEED_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, pTestHeap->scratch.dcFeed);

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_B_FIR_FILT_WRT, VP886_R_B_FIR_FILT_LEN, pTestHeap->scratch.bfir);

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_B_IIR_FILT_WRT, VP886_R_B_IIR_FILT_LEN, pTestHeap->scratch.biir);

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_RINGGEN_WRT, VP886_R_RINGGEN_LEN, pTestHeap->scratch.ringGen);

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_VADC_WRT, VP886_R_VADC_LEN, pTestHeap->scratch.vadc);

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_VPGAIN_WRT, VP886_R_VPGAIN_LEN, pTestHeap->scratch.vpGain);

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, pTestHeap->scratch.sigGenCtrl);

            VpMemCpy(pLineObj->registers.ssCfg, pTestHeap->scratch.ssCfg, VP886_R_SSCFG_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SSCFG_WRT, VP886_R_SSCFG_LEN, pTestHeap->scratch.ssCfg);

            VpMemCpy(pLineObj->registers.normCal, pTestHeap->scratch.normCal, VP886_R_NORMCAL_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_NORMCAL_WRT, VP886_R_NORMCAL_LEN, pTestHeap->scratch.normCal);

            VpMemCpy(pLineObj->registers.revCal, pTestHeap->scratch.revCal, VP886_R_REVCAL_LEN);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_REVCAL_WRT, VP886_R_REVCAL_LEN, pTestHeap->scratch.revCal);


            if (pLineObj->testInfo.nonIntrusiveTest) {
                requestNextState = TRUE;
            } else {
                /* setup a 10 ms timer to ensure we restore registers before restoring the line state */
                if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 10, 0, CONCLUDE_RESTORE_DEV)) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestConcludeSM: Cannot add to timer que."));
                }
            }

            pTestHeap->nextState = CONCLUDE_RESTORE_STATE;
            break;
        }

        case CONCLUDE_RESTORE_STATE: {
            /* Restore the line state and condition flags from the start of the
               test.  If any signals changed during the test and did NOT go back
               to how it was, restoring the status flags will cause the API to
               generate the appropriate events. */
            pLineObj->lineState = pTestHeap->lineState;

            /* Restore the termination type, in case it was changed away
               from low power in test prepare. */
            pLineObj->termType = pTestHeap->termType;

            if (pLineObj->testInfo.nonIntrusiveTest == FALSE) {
                /* Fully reset the line state */
                Vp886SetLineStateFxs(pLineCtx, pLineObj->lineState.usrCurrent);
            }

            requestNextState = TRUE;

            pTestHeap->nextState = CONCLUDE_DONE;
            break;
        }

        case CONCLUDE_DONE: {
            /* restore the current event mask */
            pLineObj->options.eventMask = pTestHeap->preTestEventMask;

            /* Restore the pulse decoding option */
            pLineObj->options.pulseMode = pTestHeap->preTestPulseMode;
            Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_PULSE_DECODE, 0, FALSE);
            VpPulseDecodeInit(&pLineObj->pulseDecodeData);

            /* Set a hook freeze to ignore any spurious hook events caused by
             * the register restore. */
            VP_HOOK(VpLineCtxType, pLineCtx, ("Test Conclude hook freeze for %dms", VP886_HOOKFREEZE_TEST_CONCLUDE));
            Vp886ExtendTimerMs(NULL, pLineCtx, VP886_TIMERID_HOOK_FREEZE, VP886_HOOKFREEZE_TEST_CONCLUDE, 0);
            Vp886SetDetectMask(pLineCtx, VP_CSLAC_HOOK);

            /* We are done with the test heap so free it */
            VpSysTestHeapRelease(pTestInfo->testHeapId);

            /* Re-enable API automatic state changes */
            pLineObj->inLineTest = FALSE;

            /* clear the prepared flag */
            pTestInfo->prepared = FALSE;

            /* clear the concluding flag */
            pTestInfo->concluding = FALSE;

            if (VP886_IS_ABS(pDevObj) && pLineObj->testInfo.nonIntrusiveTest == FALSE) {
                /* restore normal switcher power mode, since it was forced to
                   medium during test */
                pDevObj->absPowerReq[pLineObj->channelId] = Vp886GetABSPowerReq(pLineCtx, pLineObj->lineState.usrCurrent);
                Vp886ManageABSPower(pDevCtx);
                /* release control of ABS ringing battery levels */
                Vp886SetABSRingingBattFlag(pLineCtx, FALSE);
                Vp886ManageABSRingingBatt(pDevCtx, TRUE, TRUE);
            }

            /* Allow the API to resume any actions that it had to suspend when
               line test started. */
            Vp886TestConcludeCleanup(pLineCtx);

            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);

            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestConcludeSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */





