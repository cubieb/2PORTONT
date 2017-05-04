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
    PREPARE_SETUP               = 0,
    PREPARE_QUICKCAL_START     = 1,
    PREPARE_QUICKCAL_RUN       = 2,
    PREPARE_DONE                = 3,
    PREPARE_MAX                 = 3,
    PREPARE_ENUM_SIZE           = FORCE_STANDARD_C_ENUM_SIZE
} vpPrepareTestStateTypes;

bool Vp886TestPrepareSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

/*******************************************************************************
 * Vp886TestPrepare -
 *
 *  Prepare the line object for subsequent primitive calls. 
 *  The main steps of this primitive are:
 *
 *    - acquire test heap memory from the system service layer
 *    - read/store any register values that may be changed by other primtives
  *
 ******************************************************************************/
VpStatusType
Vp886TestPrepare (
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    bool callback)
{
    if (!callback) {
        const VpTestPrepareType *pInputArgs = (VpTestPrepareType *)pArgsUntyped;
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        Vp886DeviceObjectType *pDevObj = pDevCtx->pDevObj;

        if (pDevObj->dynamicInfo.clkFault == TRUE) {
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_LINE_NOT_READY);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestPrepare: Cannot run test during a clock fault"));
            return VP_STATUS_SUCCESS;
        }

        /* Cannot run if Adaptive Ringing is currently adapting */
        if ((pDevObj->options.adaptiveRinging.power != VP_ADAPTIVE_RINGING_DISABLED) && 
             ((pLineObj->lineState.usrCurrent == VP_LINE_RINGING) ||
              (pLineObj->lineState.usrCurrent == VP_LINE_RINGING_POLREV)) ) {
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_LINE_NOT_READY);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestPrepare: Cannot run test in ringing with Adaptive Ringing enabled"));
            return VP_STATUS_SUCCESS;
        }

        /* obtain a test heap and heap id */
        pTestInfo->pTestHeap =
            (Vp886TestHeapType*)VpSysTestHeapAcquire(&pTestInfo->testHeapId);

        /* generate a test event as resource not available if the sys service layer fails*/
        if (pTestInfo->pTestHeap == VP_NULL) {
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886TestPrepare no test heap acquired"));
            return VP_STATUS_SUCCESS;
        }

        /* store the prepare arguments */
        if ((pInputArgs == NULL) || pInputArgs->intrusive) {
            pLineObj->testInfo.nonIntrusiveTest = FALSE;
        } else {
            pLineObj->testInfo.nonIntrusiveTest = TRUE;
        }
        
        pTestInfo->pTestHeap->nextState = PREPARE_SETUP;
    }

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestPrepareSM(pLineCtx, pLineObj, pTestInfo, pTestInfo->pTestHeap));

    /* if an error was generated then we are done concluding */
    if (pLineObj->testResults.errorCode != VP_TEST_STATUS_SUCCESS) {
        pTestInfo->concluding = FALSE;
    }

    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp886TestPrepareSM
 *----------------------------------------------------------------------------*/
bool Vp886TestPrepareSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    bool requestNextState = FALSE;
    pLineObj->testResults.errorCode = VP_TEST_STATUS_SUCCESS;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestPrepareSM state: %i", pTestHeap->nextState));
    switch (pTestHeap->nextState) {

        case PREPARE_SETUP: {
            /* Disable API automatic state changes during line test */
            pLineObj->inLineTest = TRUE;

            /* remember that we are now prepared to run other primitives */
            pTestInfo->prepared = TRUE;

            /* Allow the API to clean up any actions it is performing before
               line test starts. */
            Vp886TestPrepareCleanup(pLineCtx);

            /* force ringing battery levels for ABS.  The Vp886SetABSRingingBattFlag()
               function will always set the flag to TRUE during line test */
            if (VP886_IS_ABS(pDevObj) && pLineObj->testInfo.nonIntrusiveTest == FALSE) {
                Vp886SetABSRingingBattFlag(pLineCtx, FALSE);
                Vp886ManageABSRingingBatt(pDevCtx, TRUE, TRUE);
            }

            /* cache all register that may be touched by the primtives */
            VpSlacRegRead(NULL, pLineCtx, VP886_R_STATE_RD, VP886_R_STATE_LEN, pTestHeap->scratch.sysState);
            VpMemCpy(pLineObj->registers.sysState, pTestHeap->scratch.sysState, VP886_R_STATE_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_LOOPSUP_RD, VP886_R_LOOPSUP_LEN, pTestHeap->scratch.loopSup);
            VpMemCpy(pLineObj->registers.loopSup, pTestHeap->scratch.loopSup, VP886_R_LOOPSUP_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_DISN_RD, VP886_R_DISN_LEN, pTestHeap->scratch.disn);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_SWPARAM_RD, VP886_R_SWPARAM_LEN, pTestHeap->scratch.swParam);
            VpMemCpy(pLineObj->registers.swParam, pTestHeap->scratch.swParam, VP886_R_SWPARAM_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_OPCOND_RD, VP886_R_OPCOND_LEN, pTestHeap->scratch.opCond);
            VpMemCpy(pLineObj->registers.opCond, pTestHeap->scratch.opCond, VP886_R_OPCOND_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR1_RD, VP886_R_ICR1_LEN, pTestHeap->scratch.icr1);
            VpMemCpy(pLineObj->registers.icr1, pTestHeap->scratch.icr1, VP886_R_ICR1_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR2_RD, VP886_R_ICR2_LEN, pTestHeap->scratch.icr2);
            VpMemCpy(pLineObj->registers.icr2, pTestHeap->scratch.icr2, VP886_R_ICR2_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR3_RD, VP886_R_ICR3_LEN, pTestHeap->scratch.icr3);
            VpMemCpy(pLineObj->registers.icr3, pTestHeap->scratch.icr3, VP886_R_ICR3_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR4_RD, VP886_R_ICR4_LEN, pTestHeap->scratch.icr4);
            VpMemCpy(pLineObj->registers.icr4, pTestHeap->scratch.icr4, VP886_R_ICR4_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_DCFEED_RD, VP886_R_DCFEED_LEN, pTestHeap->scratch.dcFeed);
            VpMemCpy(pLineObj->registers.dcFeed, pTestHeap->scratch.dcFeed, VP886_R_DCFEED_LEN);    
    
            VpSlacRegRead(NULL, pLineCtx, VP886_R_B_FIR_FILT_RD, VP886_R_B_FIR_FILT_LEN, pTestHeap->scratch.bfir);
            VpSlacRegRead(NULL, pLineCtx, VP886_R_B_IIR_FILT_RD, VP886_R_B_IIR_FILT_LEN, pTestHeap->scratch.biir);
            VpSlacRegRead(NULL, pLineCtx, VP886_R_RINGGEN_RD, VP886_R_RINGGEN_LEN, pTestHeap->scratch.ringGen);
            VpSlacRegRead(NULL, pLineCtx, VP886_R_VADC_RD, VP886_R_VADC_LEN, pTestHeap->scratch.vadc);
            VpSlacRegRead(NULL, pLineCtx, VP886_R_VPGAIN_RD, VP886_R_VPGAIN_LEN, pTestHeap->scratch.vpGain);
            VpSlacRegRead(NULL, pLineCtx, VP886_R_SIGCTRL_RD, VP886_R_SIGCTRL_LEN, pTestHeap->scratch.sigGenCtrl);
    
            VpSlacRegRead(NULL, pLineCtx, VP886_R_SSCFG_RD, VP886_R_SSCFG_LEN, pTestHeap->scratch.ssCfg);
            VpMemCpy(pLineObj->registers.ssCfg, pTestHeap->scratch.ssCfg, VP886_R_SSCFG_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_NORMCAL_RD, VP886_R_NORMCAL_LEN, pTestHeap->scratch.normCal);
            VpMemCpy(pLineObj->registers.normCal, pTestHeap->scratch.normCal, VP886_R_NORMCAL_LEN);

            VpSlacRegRead(NULL, pLineCtx, VP886_R_REVCAL_RD, VP886_R_REVCAL_LEN, pTestHeap->scratch.revCal);
            VpMemCpy(pLineObj->registers.revCal, pTestHeap->scratch.revCal, VP886_R_REVCAL_LEN);

            /* affected by state change so we have to store the start value */
            pTestHeap->scratch.opFunc[0] =  pLineObj->registers.opFunc[0];

            /* save the current event mask */
            pTestHeap->preTestEventMask = pLineObj->options.eventMask;

            /* mask all non-test events until test conclude */
            pLineObj->options.eventMask.faults = 0xFFFF;
            pLineObj->options.eventMask.signaling = 0xFFFF;
            pLineObj->options.eventMask.response = 0xFFFF;
            pLineObj->options.eventMask.test = ~(VP_LINE_EVID_TEST_CMP | VP_LINE_EVID_ABORT);
            pLineObj->options.eventMask.process = 0xFFFF;
            pLineObj->options.eventMask.fxo = 0xFFFF;
            pLineObj->options.eventMask.packet = 0xFFFF;

            /* Save the pulse decoding option */
            pTestHeap->preTestPulseMode = pLineObj->options.pulseMode;
    
            /* Disable pulse decoding during the test */
            pLineObj->options.pulseMode = VP_OPTION_PULSE_DECODE_OFF;
            Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_PULSE_DECODE, 0, FALSE);
            VpPulseDecodeInit(&pLineObj->pulseDecodeData);

            /* Save the current line state info (usrCurrent, currentState, and
               condition flags) so that it can all be fully restored at the end.  By
               saving the current line condition flags we can tell if something is
               different at the end of the test and generate appropriate events */
            pTestHeap->lineState = pLineObj->lineState;

            /* Save the term type, since we may change it if it is low power */
            pTestHeap->termType = pLineObj->termType;

            /* If ILR is programmed below 50mA, set it to 50mA during line testing */
            if (pLineObj->testInfo.nonIntrusiveTest == FALSE) {
                if (pLineObj->registers.loopSup[0] & VP886_R_LOOPSUP_GKEY_ABS) {
                    pLineObj->registers.loopSup[0] &= ~VP886_R_LOOPSUP_GKEY_ABS;
                    VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN,
                        pLineObj->registers.loopSup);
                }
                if (pLineObj->lowIlr) {
                    pLineObj->registers.loopSup[3] &= ~VP886_R_LOOPSUP_RING_CUR_LIM;  
                    VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN,
                        pLineObj->registers.loopSup);

                    pLineObj->registers.icr2[0] &= ~VP886_R_ICR2_DAC_RING_LEVELS;
                    VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN,
                        pLineObj->registers.icr2);
                }
            }

            /* get out of low power slic state if needed */
            if (pLineObj->testInfo.nonIntrusiveTest == FALSE) {
                if (pLineObj->termType == VP_TERM_FXS_LOW_PWR) {
                    /* Change the termination type to non-lowpower during test.  This will
                       need to be a bit smarter if we add more low power termination types
                       like ISOLATE_LP */
                    pLineObj->termType = VP_TERM_FXS_GENERIC;
                }
                
                /* Go to disconnect to run sense recalibration */
                Vp886SetLineStateFxsInt(pLineCtx, VP_LINE_DISCONNECT);

                pTestHeap->nextState = PREPARE_QUICKCAL_START;
            } else {
                /* Don't perform sense recalibration if this is a non-intrusive
                   test. */
                pTestHeap->nextState = PREPARE_DONE;
            }

            requestNextState = TRUE;
            break;
        }

        case PREPARE_QUICKCAL_START: {
            Vp886QuickCalStart(pLineCtx, VP886_TIMERID_LINE_TEST, PREPARE_QUICKCAL_RUN, VP886_LINE_TEST, FALSE);
            pTestHeap->nextState = PREPARE_QUICKCAL_RUN;
            break;
        }

        case PREPARE_QUICKCAL_RUN: {
            Vp886QuickCalHandler(pLineCtx);
            
            /* If it has finished, proceed to the next prepare state */
            if (pLineObj->quickCal.state == VP886_QUICKCAL_INACTIVE) {
                /* Restore the line state */
                Vp886SetLineStateFxsInt(pLineCtx, pTestHeap->lineState.currentState);
                requestNextState = TRUE;
                pTestHeap->nextState = PREPARE_DONE;
            }
            break;
        }

        case PREPARE_DONE: {
            /* generate a successful event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestPrepareSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */





