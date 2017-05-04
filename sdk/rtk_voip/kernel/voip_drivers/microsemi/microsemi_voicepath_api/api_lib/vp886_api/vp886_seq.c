/** \file vp886_sequencer.c
 * vp886_sequencer.c
 *
 *  This file contains the VP886 functions called by the API-II Caller ID or
 * sequencer. It is seperated from "normal" API functions for users that want
 * to remove this section of code from the API-II.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 11450 $
 * $LastChangedDate: 2014-06-06 16:30:41 -0500 (Fri, 06 Jun 2014) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#ifdef VP_CSLAC_SEQ_EN

/* INCLUDES */
#include "vp_api_types.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp886_api.h"
#include "vp886_api_int.h"
#include "sys_service.h"

static VpStatusType
Vp886CadenceHandleCommand(
    VpLineCtxType *pLineCtx,
    bool *pKeepGoing);

static VpStatusType
Vp886CadenceHandleTimer(
    VpLineCtxType *pLineCtx,
    uint32 timerOverrun,
    bool *pKeepGoing);

static VpStatusType
Vp886CadenceHandleBranch(
    VpLineCtxType *pLineCtx);

static bool
Vp886CidHandleMarkOrSeizure(
    VpLineCtxType *pLineCtx,
    uint8 type);

static bool
Vp886CidHandleFskMsg(
    VpLineCtxType *pLineCtx);

static bool
Vp886CidHandleFskMsgMarkout(
    VpLineCtxType *pLineCtx);

static bool
Vp886CidHandleDtmfMsg(
    VpLineCtxType *pLineCtx);

static uint8
Vp886CidFskBufferSpace(
    VpLineCtxType *pLineCtx,
    uint8 cidParamReg);

static VpStatusType
Vp886SendSignalInt(
    VpLineCtxType *pLineCtx,
    VpSendSignalType signalType,
    void *pSignalData);

static VpStatusType
Vp886SendSignalMsgWaitPulse(
    VpLineCtxType *pLineCtx,
    VpSendMsgWaitType *pMsgWait);

static VpStatusType
Vp886StartMeterInt(
    VpLineCtxType *pLineCtx,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters);



/** Vp886CadenceStart()
  Begins a tone or ringing cadence.
  
  First initializes the cadence status structure, then begins executing the
  cadence instructions via Vp886CadenceHandler().
*/
VpStatusType
Vp886CadenceStart(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pProfile,
    VpCadenceStatusType flags)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;

    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Starting Cadence"));

    pLineObj->cadence.pActiveCadence = pProfile;
    pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
    pLineObj->cadence.pCurrentPos = &pProfile[VP_PROFILE_TYPE_SEQUENCER_START];
    pLineObj->cadence.length = pProfile[VP_PROFILE_LENGTH];
    pLineObj->cadence.status = VP_CADENCE_STATUS_ACTIVE;
    pLineObj->cadence.status |= flags;
    if (pProfile[VP_PROFILE_MPI_LEN] & 0x01) {
        pLineObj->cadence.status |= VP_CADENCE_STATUS_IGNORE_POLARITY;
    }
    pLineObj->cadence.branchDepth = 0;
    pLineObj->cadence.toneType = VP_CSLAC_STD_TONE;
    pLineObj->cadence.lineState = pLineObj->lineState.currentState;
    
#ifdef VP_HIGH_GAIN_MODE_SUPPORTED
    /* If high gain mode (howler line state) is active, adjust R and GR based
       on the tone type */
    if (pLineObj->inHighGainMode) {
        Vp886HighGainSetRFilter(pLineCtx);
    }
#endif /* VP_HIGH_GAIN_MODE_SUPPORTED */

    return Vp886CadenceHandler(pLineCtx, 0);
}


/** Vp886CadenceStop()
  Ends a tone or ringing cadence.  This is used whether the cadence has run to
  completion or is terminated by another command.
  
  The input arguments modify the behavior of this function to accommodate
  different circumstances for ending the cadence:
    aborted:
        If TRUE, this function will generate the appropriate CAD event.
        If FALSE, no CAD event will be generated.
    restoreLineState:
        If TRUE, the line state will be restored to the last state set by the
        application.
        If FALSE, the line will remain in whatever state the cadence may have put it in.
    disableTones:
        If TRUE, turns off all tone generators.
        If FALSE, tone generators that may have been enabled by the cadence will
        remain on.
*/
void
Vp886CadenceStop(
    VpLineCtxType *pLineCtx,
    bool aborted,
    bool restoreLineState,
    bool disableTones)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    Vp886SeqDataType *pCadence = &pLineObj->cadence;

    if (!(pCadence->status & VP_CADENCE_STATUS_ACTIVE)) {
        return;
    }
    
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Stopping Cadence"));

    pCadence->status = VP_CADENCE_RESET_VALUE;
    
    /* Cancel cadence timers */
    Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_CADENCE, 0, FALSE);
    if (pCadence->toneType != VP_CSLAC_STD_TONE) {
        Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_HOWLER, 0, FALSE);
    }
    
    /* Generate an appropriate event if the cadence ended naturally */
    if (!aborted) {
        switch(pCadence->pActiveCadence[VP_PROFILE_TYPE_LSB]) {
            case VP_PRFWZ_PROFILE_RINGCAD:
                Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_RING_CAD,
                    VP_RING_CAD_DONE, Vp886GetTimestamp(pDevCtx), FALSE);
                break;
            case VP_PRFWZ_PROFILE_TONECAD:
                Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_TONE_CAD,
                    0, Vp886GetTimestamp(pDevCtx), FALSE);
                break;
            default:
                break;
        }
    }
    
    pCadence->pActiveCadence = VP_PTABLE_NULL;
    pCadence->pCurrentPos = VP_PTABLE_NULL;

    if (restoreLineState) {
        Vp886SetLineStateFxsInt(pLineCtx, pLineObj->lineState.usrCurrent);
    } else {
        pLineObj->lineState.usrCurrent = pLineObj->lineState.currentState;
    }
    
    if (disableTones) {
        Vp886SetToneCtrl(pLineCtx, FALSE, FALSE, FALSE, FALSE, FALSE);
    }

    return;
}


/** Vp886CadenceHandler()
  Processes cadence instructions until reaching a delay, "wait on" instruction,
  or the end of the cadence.
*/
VpStatusType
Vp886CadenceHandler(
    VpLineCtxType *pLineCtx,
    uint32 timerOverrun)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886SeqDataType *pCadence = &pLineObj->cadence;
    bool keepGoing = TRUE;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!(pCadence->status & VP_CADENCE_STATUS_ACTIVE)) {
        return VP_STATUS_SUCCESS;
    }

    if (pCadence->pActiveCadence == VP_PTABLE_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CadenceHandler - invalid NULL cadence"));
        return VP_STATUS_FAILURE;
    }

    if (pCadence->status & VP_CADENCE_STATUS_WAITING_ON_CID) {
        pCadence->status &= ~VP_CADENCE_STATUS_WAITING_ON_CID;
    }

    while (keepGoing) {
        if (pCadence->index >= (pCadence->length + VP_PROFILE_LENGTH + 1)) {
            Vp886CadenceStop(pLineCtx, FALSE, FALSE, FALSE);
            return status;
        }
        pCadence->pCurrentPos = &pCadence->pActiveCadence[pCadence->index];
    
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Vp886CadenceHandler - instr %d: %02X %02X, time %d",
            (pCadence->index - VP_PROFILE_TYPE_SEQUENCER_START) / 2,
            pCadence->pCurrentPos[0], pCadence->pCurrentPos[1], Vp886GetTimestamp(pLineCtx->pDevCtx)));
    
        switch(pCadence->pCurrentPos[0] & VP_SEQ_OPERATOR_MASK) {
            case VP_SEQ_SPRCMD_COMMAND_INSTRUCTION:
                status = Vp886CadenceHandleCommand(pLineCtx, &keepGoing);
                break;
            case VP_SEQ_SPRCMD_TIME_INSTRUCTION:
                status = Vp886CadenceHandleTimer(pLineCtx, timerOverrun, &keepGoing);
                break;
            case VP_SEQ_SPRCMD_BRANCH_INSTRUCTION:
                status = Vp886CadenceHandleBranch(pLineCtx);
                break;
            default:
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CadenceHandler - invalid operator %02X",
                    pCadence->pCurrentPos[0]));
                status = VP_STATUS_INVALID_ARG;
                break;
        }
        if (status != VP_STATUS_SUCCESS) {
            Vp886CadenceStop(pLineCtx, TRUE, TRUE, TRUE);
            return status;
        }
#ifdef VP886_LEGACY_SEQUENCER
        /* In legacy mode, simulate the tick-per-instruction delay of the old
           CSLAC sequencer by setting a short timer where we would normally
           proceed directly to the next instruction. */
        if (keepGoing) {
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CADENCE, 2, 0, 0);
            keepGoing = FALSE;
        }
#endif
    }
    
    return status;
}


/** Vp886CadenceHandleCommand()
  Processes a cadence command instruction.
*/
VpStatusType
Vp886CadenceHandleCommand(
    VpLineCtxType *pLineCtx,
    bool *pKeepGoing)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    Vp886SeqDataType *pCadence = &pLineObj->cadence;
    VpProfilePtrType pSeq = pCadence->pCurrentPos;
    uint8 subCmd = (pSeq[0] & VP_SEQ_SUBTYPE_MASK);
    uint8 subCmdData = pSeq[1];
    VpStatusType status = VP_STATUS_SUCCESS;

    *pKeepGoing = TRUE;

    switch (subCmd) {
        case VP_SEQ_SUBCMD_LINE_STATE: {
            VpLineStateType state;
            Vp886LineStateInfoType newStateInfo;
            Vp886LineStateInfoType userStateInfo;
            Vp886LineStateInfoType currentStateInfo;
            
            state = ConvertPrfWizState2ApiState(subCmdData);
            newStateInfo = Vp886LineStateInfo(state);
            userStateInfo = Vp886LineStateInfo(pLineObj->lineState.usrCurrent);
            currentStateInfo = Vp886LineStateInfo(pLineObj->cadence.lineState);
            
            if (pCadence->status & VP_CADENCE_STATUS_IGNORE_POLARITY) {
                if (userStateInfo.polrev) {
                    state = newStateInfo.polrevEquiv;
                } else {
                    state = newStateInfo.normalEquiv;
                }
            }
            
            if (pLineObj->cid.active) {
                Vp886CidStop(pLineCtx);
            }

            if (currentStateInfo.normalEquiv == VP_LINE_RINGING &&
                newStateInfo.normalEquiv != VP_LINE_RINGING)
            {
                Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_RING_CAD,
                    VP_RING_CAD_BREAK, Vp886GetTimestamp(pDevCtx), FALSE);
            }
            if (currentStateInfo.normalEquiv != VP_LINE_RINGING &&
                newStateInfo.normalEquiv == VP_LINE_RINGING)
            {
                Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_RING_CAD,
                    VP_RING_CAD_MAKE, Vp886GetTimestamp(pDevCtx), FALSE);
            }
            
            pCadence->lineState = state;
            status = Vp886SetLineStateFxsInt(pLineCtx, state);
            break;
        }
        case VP_SEQ_SUBCMD_SIGGEN: {
            /* Enable the signal generators that are requested by the cadence
               AND enabled in the tone profile.  If the tone profile included
               DC Bias, the bias generator will be enabled when the cadence
               requests -any- tone enabled. */
            Vp886SetToneCtrl(pLineCtx,
                (subCmdData != 0x00) && (pLineObj->toneGens & VP886_TONEGEN_BIAS),
                (subCmdData & 0x01) && (pLineObj->toneGens & VP886_TONEGEN_A),
                (subCmdData & 0x02) && (pLineObj->toneGens & VP886_TONEGEN_B),
                (subCmdData & 0x04) && (pLineObj->toneGens & VP886_TONEGEN_C),
                (subCmdData & 0x08) && (pLineObj->toneGens & VP886_TONEGEN_D));
            break;
        }
        case VP_SEQ_SUBCMD_START_CID: {
            /* Run CID in parallel with this sequence */
            Vp886CidStart(pLineCtx);
            break;
        }
        case VP_SEQ_SUBCMD_WAIT_ON: {
            uint8 nextInstr;
            uint8 nextIndex = pCadence->index + 2;

            /* Start CID if it isn't already running */
            if (!pLineObj->cid.active) {
                Vp886CidStart(pLineCtx);
            }

            /* If this flag is set when CID ends for any reason, Vp886CidStop()
               will resume the sequence. */
            pCadence->status |= VP_CADENCE_STATUS_WAITING_ON_CID;

            /* If the next instruction is a timer, move on normally.  When CID
               completes, it will cancel the timer and resume the sequence.  If
               the timer expires before CID completes, the sequence will resume
               on its own.
               If the next instruction is not a timer, suspend the sequence.
               When CID completes, it will resume the sequence. */
            if (nextIndex < (pCadence->length + VP_PROFILE_LENGTH + 1)) {
                nextInstr = (pCadence->pActiveCadence[nextIndex]) & VP_SEQ_OPERATOR_MASK;
                
                if (nextInstr != VP_SEQ_SPRCMD_TIME_INSTRUCTION) {
                    *pKeepGoing = FALSE;
                }
            } else {
                *pKeepGoing = FALSE;
            }
            break;
        }
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CadenceHandleCommand - invalid subcommand %d",
                subCmd));
            status = VP_STATUS_INVALID_ARG;
            break;
    }

    pCadence->index += 2;

    return status;
}


/** Vp886CadenceHandleTimer()
  Processes a cadence timer instruction by setting the device timer.
*/
VpStatusType
Vp886CadenceHandleTimer(
    VpLineCtxType *pLineCtx,
    uint32 timerOverrun,
    bool *pKeepGoing)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886SeqDataType *pCadence = &pLineObj->cadence;
    VpProfilePtrType pSeq = pCadence->pCurrentPos;
    uint32 timerMs;

    timerMs = 5 * (((pSeq[0] & 0x1F) << 8) + pSeq[1]);
    
#ifdef VP886_LEGACY_SEQUENCER
    /* When the overrun is processed automatically, it can result in timer
       durations of 0.  We want to always have timers greater than 0 in
       legacy mode. */
    if (timerOverrun / 2 >= timerMs) {
        timerMs = 1;
    } else {
        timerMs = timerMs - (timerOverrun / 2);
    }
    timerOverrun = 0;
#endif

    if (timerMs == 0) {
        /* A timer instruction with a value of 0 means that the sequence stops
           there.  Treat this as the last command in the profile by moving the
           index to the end. */
        pCadence->index = (pCadence->length + VP_PROFILE_LENGTH + 1);
        *pKeepGoing = TRUE;
    } else {
        Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CADENCE, timerMs, timerOverrun, 0);
        pCadence->index += 2;
        *pKeepGoing = FALSE;
    }

    return VP_STATUS_SUCCESS;
}


/** Vp886CadenceHandleBranch()
  Processes a cadence branch instruction.
*/
VpStatusType
Vp886CadenceHandleBranch(
    VpLineCtxType *pLineCtx)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886SeqDataType *pCadence = &pLineObj->cadence;
    VpProfilePtrType pSeq = pCadence->pCurrentPos;
    uint8 branchTarget = (pSeq[0] & 0x1F);
    uint8 branchCount = pSeq[1];
    uint8 depthIndex = pCadence->branchDepth - 1;
    
    /* Handle entering a new loop depth */
    if (pCadence->branchDepth == 0 ||
        pCadence->index != pCadence->branchIdx[depthIndex])
    {
        if (pCadence->branchDepth >= VP886_MAX_BRANCH_DEPTH) {
            /* Exceeding max branch depth.  Skip this branch. */
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886CadenceHandleBranch - exceeded max branch depth at index %d",
                (pCadence->index - VP_PROFILE_TYPE_SEQUENCER_START) / 2));
            pCadence->index += 2;
            return VP_STATUS_SUCCESS;
        }
        
        pCadence->branchDepth++;
        depthIndex++;
        pCadence->branchCount[depthIndex] = branchCount;
        pCadence->branchIdx[depthIndex] = pCadence->index;

        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("New branch at %d to %d, count %d, new depth %d",
            (pCadence->index - VP_PROFILE_TYPE_SEQUENCER_START) / 2, branchTarget,
            branchCount, pCadence->branchDepth));

        pCadence->index = (branchTarget * 2) + VP_PROFILE_TYPE_SEQUENCER_START;
        return VP_STATUS_SUCCESS;
    }
    
    /* If the current branch is finished, move past it */
    if (pCadence->branchCount[depthIndex] == 1) {
        pCadence->branchDepth--;
        depthIndex--;

        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Finished branch at %d to %d, new depth %d",
            (pCadence->index - VP_PROFILE_TYPE_SEQUENCER_START) / 2,
            branchTarget, pCadence->branchDepth));

        pCadence->index += 2;
        return VP_STATUS_SUCCESS;
    }
    
    /* If the current branch is not infinite, decrement the count */
    if (pCadence->branchCount[depthIndex] != 0) {
        pCadence->branchCount[depthIndex]--;
    }
    
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Continuing branch at %d to %d, new count %d, depth %d",
        (pCadence->index - VP_PROFILE_TYPE_SEQUENCER_START) / 2, branchTarget,
        pCadence->branchCount[depthIndex], pCadence->branchDepth));

    /* Branch */
    pCadence->index = (branchTarget * 2) + VP_PROFILE_TYPE_SEQUENCER_START;
    
    return VP_STATUS_SUCCESS;
}


/** Vp886InitRing()
  Implements VpInitRing() to set up the ringing cadence and caller ID
  profiles for a line.

  See the VP-API-II Reference Guide for more details on VpInitRing().
*/
VpStatusType
Vp886InitRing(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pCadProfile,
    VpProfilePtrType pCidProfile)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpStatusType status;

    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886InitRing");

    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitRing");
        return status;
    }

    /* If a CID profile is specified, a ring cadence profile is required. */
    if ((pCadProfile == VP_NULL) && (pCidProfile != VP_NULL)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886InitRing(): If CID profile is specified, ring cadence profile is required"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitRing");
        return VP_STATUS_INVALID_ARG;
    }

    if (pCidProfile != VP_PTABLE_NULL) {
        /* Don't allow the CID profile to change if CID is currently running */
        if (pCidObj->active) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886InitRing - Cannot change CID profile while CID is running"));
            Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitRing");
            return VP_STATUS_DEVICE_BUSY;
        }
    }

    status = Vp886GetProfileArg(pDevCtx, VP_PROFILE_RINGCAD, &pCadProfile);
    if (status != VP_STATUS_SUCCESS) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitRing");
        return status;
    }
    status = Vp886GetProfileArg(pDevCtx, VP_PROFILE_CID, &pCidProfile);
    if (status != VP_STATUS_SUCCESS) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitRing");
        return status;
    }
    
    pLineObj->pRingingCadence = pCadProfile;
    if (pCidProfile != VP_PTABLE_NULL) {
        pCidObj->pProf = pCidProfile;
    }

    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitRing");
    return VP_STATUS_SUCCESS;
} /* Vp886InitRing */


/** Vp886InitCid()
  Implements VpInitCid() to set up the initial CID data to be used for CID
  during ringing.  This must be called EVERY time the line begins a ringing+CID
  cadence to set up the data buffer.

  See the VP-API-II Reference Guide for more details on VpInitCid().
*/
VpStatusType
Vp886InitCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    uint8p pCidData)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpStatusType status = VP_STATUS_SUCCESS;

    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886InitCid");

    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitCid");
        return status;
    }

    if (pCidData == VP_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886InitCid - invalid NULL pCidData"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitCid");
        return VP_STATUS_INVALID_ARG;
    }

    if (length > VP886_CID_BUFFER_SIZE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886InitCid - Length must be %d or less (%d)", VP886_CID_BUFFER_SIZE, length));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitCid");
        return VP_STATUS_INVALID_ARG;
    }

    VpMemCpy(pCidObj->msgBuf, pCidData, length);
    pCidObj->msgLen = length;
    pCidObj->msgIdx = 0;

    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitCid");
    return status;
}


/** Vp886SendCid()
  Implements VpSendCid() to start a caller ID sequence on-demand.  This function
  takes in the CID profile as well as the initial chunk of CID data.  The first
  step of the CID sequence is executed immediately, and the rest is timer- and
  interrupt-driven.

  See the VP-API-II Reference Guide for more details on VpSendCid().
*/
VpStatusType
Vp886SendCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    VpProfilePtrType pCidProfile,
    uint8p pCidData)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpStatusType status;

    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886SendCid");

    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendCid");
        return status;
    }

    if (pCidData == VP_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SendCid - invalid NULL pCidData"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendCid");
        return VP_STATUS_INVALID_ARG;
    }

    if (length > VP886_CID_BUFFER_SIZE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SendCid - Length must be %d or less (%d)", VP886_CID_BUFFER_SIZE, length));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendCid");
        return VP_STATUS_INVALID_ARG;
    }

    /* A CID profile is required. */
    if (pCidProfile == VP_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SendCid - Invalid NULL pCidProfile"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendCid");
        return VP_STATUS_INVALID_ARG;
    }

    /* Can't perform CID in non-codec states.  Check against the internal state,
       not the user state, to allow VpSendCid() during the off period of a
       ringing cadence. */
    if (!Vp886LineStateInfo(pLineObj->lineState.currentState).codec) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SendCid - Caller ID cannot be generated in the current line state (%d)",
            pLineObj->lineState.currentState));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendCid");
        return VP_STATUS_DEVICE_BUSY;
    }

    status = Vp886GetProfileArg(pDevCtx, VP_PROFILE_CID, &pCidProfile);
    if (status != VP_STATUS_SUCCESS) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendCid");
        return status;
    }

    if (pCidObj->active) {
        Vp886CidStop(pLineCtx);
    }

    pCidObj->pProf = pCidProfile;
    VpMemCpy(pCidObj->msgBuf, pCidData, length);
    pCidObj->msgLen = length;
    pCidObj->msgIdx = 0;
    Vp886CidStart(pLineCtx);

    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendCid");
    return VP_STATUS_SUCCESS;
}


/** Vp886ContinueCid()
  Implements VpContinueCid() to put more caller ID data into the API buffer,
  up to 16 bytes.

  See the VP-API-II Reference Guide for more details on VpContinueCid().
*/
VpStatusType
Vp886ContinueCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    uint8p pCidData)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    uint8 readIndex;
    uint8 writeIndex;
    VpStatusType status;

    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");

    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");
        return status;
    }
    
    if (length > 16) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886ContinueCid - Length must be 16 or less (%d)", length));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");
        return VP_STATUS_INVALID_ARG;
    }

    if (pCidData == VP_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886ContinueCid - invalid NULL pCidData"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");
        return VP_STATUS_INVALID_ARG;
    }

    if (!pCidObj->active) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886ContinueCid - Caller ID not running"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");
        return VP_STATUS_DEVICE_BUSY;
    }

    if (!pCidObj->needData) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886ContinueCid - Data not needed"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");
        return VP_STATUS_DEVICE_BUSY;
    }
    
    if (pCidObj->msgLen + length > VP886_CID_BUFFER_SIZE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886ContinueCid - Data overflow"));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");
        return VP_STATUS_DEVICE_BUSY;
    }
    
    writeIndex = (pCidObj->msgIdx + pCidObj->msgLen) % VP886_CID_BUFFER_SIZE;
    for (readIndex = 0; readIndex < length; readIndex++) {
        pCidObj->msgBuf[writeIndex] = pCidData[readIndex];
        pCidObj->msgLen++;
        writeIndex = (writeIndex + 1) % VP886_CID_BUFFER_SIZE;
    }
    
    pCidObj->needData = FALSE;

    VP_CID(VpLineCtxType, pLineCtx, ("Added more data (%d remaining)", pCidObj->msgLen));

    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886ContinueCid");
    return VP_STATUS_SUCCESS;
} /* Vp886ContinueCid() */


/** Vp886CidStart()
  Begins a caller ID sequence by initializing the CID status variables and
  processing the first instruction in the sequence.
  
  This may be called from VpSendCid() or from handling a Start CID instruction
  in a ringing cadence.
*/
void
Vp886CidStart(
    VpLineCtxType *pLineCtx)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886CidDataType *pCidObj = &pLineObj->cid;

    if (pCidObj->pProf == VP_PTABLE_NULL) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886CidStart - invalid NULL CID profile"));
        return;
    }

    pCidObj->mute = FALSE;
    pCidObj->fskEnabled = FALSE;
    pCidObj->dtmfEnabled = FALSE;
    pCidObj->needData = FALSE;
    pCidObj->checksum = 0;
    pCidObj->state = VP886_CID_ST_NEW_INSTR;
    pCidObj->profIdx = VP_CID_PROFILE_FSK_PARAM_LEN +
        pCidObj->pProf[VP_CID_PROFILE_FSK_PARAM_LEN] +
        VP_CID_PROFILE_START_OF_ELEMENTS_MSB;

    pCidObj->active = TRUE;
    
    VP_CID(VpLineCtxType, pLineCtx, ("Starting CID"));

    /* Can't perform CID in non-codec states.  This case should only be
       possible during a ring cadence, and the solution is to make sure the
       ring cadence sets an appropriate line state before starting CID. */
    if (!Vp886LineStateInfo(pLineObj->lineState.currentState).codec) {
        Vp886CidStop(pLineCtx);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886CidStart - Can't start CID in line state %d", pLineObj->lineState.currentState));
        return;
    }

    /* Can't perform CID if we have no data. Each time the application begins
       a ring cadence with CID included, it must first call VpInitCid(). */
    if (pCidObj->msgLen == 0) {
        Vp886CidStop(pLineCtx);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886CidStart - Can't start CID with no CID data"));
        return;
    }

    /* Start running the sequence */
    Vp886CidHandler(pLineCtx, 0);
    
    return;
}


/** Vp886CidStop()
  Ends a caller ID sequence:
    - clears the active flag
    - cancels any running CID timer
    - turns off tone generators
    - turns off FSK generation
    - restores normal PCM TX/RX settings
    - generates the CID done event
    - resumes ring cadencing if there was a "Wait on CID" command

  This should be used any time CID ends, whether is interrupted, aborted, or
  just ends normally.
*/
void
Vp886CidStop(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 channelId = pLineObj->channelId;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    uint8 cidParamReg;

    VP_CID(VpLineCtxType, pLineCtx, ("Stopping CID"));

    pCidObj->active = FALSE;

    Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_CID, 0, FALSE);

    Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_CID_DATA,
        VP_CID_DATA_TX_DONE, Vp886GetTimestamp(pDevCtx), FALSE);
    
    /* Stop any alert tone or DTMF data */
    Vp886SetToneCtrl(pLineCtx, FALSE, FALSE, FALSE, FALSE, FALSE);
    
    /* Tell the system to disable DTMF detection if in a detect interval */
    if (pCidObj->state == VP886_CID_ST_DETECT_INTERVAL) {
        VpSysDtmfDetDisable(pDevObj->deviceId, pLineObj->channelId);
    }

    /* Reset PCM TX/RX */
    Vp886ApplyPcmTxRx(pLineCtx);

    /* Disable FSK generation */
    if (pCidObj->fskEnabled) {
        cidParamReg = VP886_R_CIDPARAM_CID_DIS;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);
    }
    
    /* If CID is not being sent during a cadence, update the usrCurrent state
       in case the CID sequence changed the polarity */
    if (!(pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE)) {
        pLineObj->lineState.usrCurrent = pLineObj->lineState.currentState;
    }

    /* If an active cadence is waiting on CID to complete, cancel the timeout
       and resume the cadence */
    if ((pLineObj->cadence.status & VP_CADENCE_STATUS_ACTIVE) &&
        (pLineObj->cadence.status & VP_CADENCE_STATUS_WAITING_ON_CID))
    {
        Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_CADENCE, 0, FALSE);
        Vp886CadenceHandler(pLineCtx, 0);
    }

    return;
}


/** Vp886CidHandler()
  Processes instructions in a CID sequence until reaching an operation that
  requires time to complete.

  If an instruction requires multiple steps, this function will process the same
  instruction repeatedly until finished.  The substate of an instruction is kept
  track of in pCidObj->state.
*/
void
Vp886CidHandler(
    VpLineCtxType *pLineCtx,
    uint32 timerOverrun)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpProfilePtrType pProf = pCidObj->pProf;
    uint8 *pIdx = &pCidObj->profIdx;
    uint8 instr;
    bool keepGoing = TRUE;
    
    if (!pCidObj->active) {
        return;
    }
    
    if (pProf == VP_PTABLE_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CidHandler - invalid NULL sequence"));
        return;
    }

    while (keepGoing) {
        if (*pIdx > pProf[VP_PROFILE_LENGTH] + VP_PROFILE_LENGTH) {
            VP_CID(VpLineCtxType, pLineCtx, ("End of profile"));
            Vp886CidStop(pLineCtx);
            return;
        }
        
        instr = pProf[*pIdx + 1];
        
        switch (instr) {
            case VP_CLI_POLREV: {
                VpLineStateType state;
                state = Vp886LineStateInfo(pLineObj->lineState.currentState).oppositeEquiv;
                Vp886SetLineStateFxsInt(pLineCtx, state);
                VP_CID(VpLineCtxType, pLineCtx, ("Polrev"));

                *pIdx += 2;
                pCidObj->state = VP886_CID_ST_NEW_INSTR;
                keepGoing = TRUE;
                break;
            }
            case VP_CLI_MUTEON: {
                pCidObj->mute = TRUE;
                Vp886ApplyPcmTxRx(pLineCtx);
                VP_CID(VpLineCtxType, pLineCtx, ("Mute ON"));

                *pIdx += 2;
                pCidObj->state = VP886_CID_ST_NEW_INSTR;
                keepGoing = TRUE;
                break;
            }
            case VP_CLI_MUTEOFF: {
                pCidObj->mute = FALSE;
                Vp886ApplyPcmTxRx(pLineCtx);
                VP_CID(VpLineCtxType, pLineCtx, ("Mute OFF"));

                *pIdx += 2;
                pCidObj->state = VP886_CID_ST_NEW_INSTR;
                keepGoing = TRUE;
                break;
            }
            case VP_CLI_ALERTTONE: {
                /* Apply the signal generator parameters then move on */
                uint8 len = pProf[*pIdx + 2];
                VpSlacRegWrite(NULL, pLineCtx, VP886_NOOP, pProf[*pIdx + 2], &pProf[*pIdx + 3]);
                VP_CID(VpLineCtxType, pLineCtx, ("Tone programmed"));

                *pIdx += 3 + len;
                pCidObj->state = VP886_CID_ST_NEW_INSTR;
                keepGoing = TRUE;
                break;
            }
            case VP_CLI_ALERTTONE2: {
                if (pCidObj->state == VP886_CID_ST_NEW_INSTR) {
                    /* Enable siggen C and D, and set the timer */
                    uint32 timerMs = (pProf[*pIdx + 2] << 8) + pProf[*pIdx + 3];
                    Vp886SetToneCtrl(pLineCtx, FALSE, FALSE, FALSE, TRUE, TRUE);
                    VP_CID(VpLineCtxType, pLineCtx, ("Tone ON"));

                    Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CID, timerMs, timerOverrun, 0);
                    pCidObj->state = VP886_CID_ST_ALERT_TONE;
                    keepGoing = FALSE;

                } else if (pCidObj->state == VP886_CID_ST_ALERT_TONE) {
                    /* Turn siggens off then move on */
                    Vp886SetToneCtrl(pLineCtx, FALSE, FALSE, FALSE, FALSE, FALSE);
                    VP_CID(VpLineCtxType, pLineCtx, ("Tone OFF"));

                    *pIdx += 4;
                    pCidObj->state = VP886_CID_ST_NEW_INSTR;
                    keepGoing = TRUE;
                }
                break;
            }
            case VP_CLI_SILENCE: {
                if (pCidObj->state == VP886_CID_ST_NEW_INSTR) {
                    uint32 delayMs = (pProf[*pIdx + 2] << 8) + pProf[*pIdx + 3];
                    VP_CID(VpLineCtxType, pLineCtx, ("Delay %lu", delayMs));

                    Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CID, delayMs, timerOverrun, 0);
                    pCidObj->state = VP886_CID_ST_SILENT_INTERVAL;
                    keepGoing = FALSE;

                } else if (pCidObj->state == VP886_CID_ST_SILENT_INTERVAL) {
                    *pIdx += 4;
                    pCidObj->state = VP886_CID_ST_NEW_INSTR;
                    keepGoing = TRUE;
                }
                break;
            }
            case VP_CLI_SILENCE_MASKHOOK: {
                if (pCidObj->state == VP886_CID_ST_NEW_INSTR) {
                    uint32 delayMs = (pProf[*pIdx + 2] << 8) + pProf[*pIdx + 3];
                    uint32 hookMaskMs = (pProf[*pIdx + 4] << 8) + pProf[*pIdx + 5];

                    VP_CID(VpLineCtxType, pLineCtx, ("Delay %lu, hook freeze %lu", delayMs, hookMaskMs));

                    /* Start a hook freeze timer, extending any existing one.
                       The hook freeze will run in parallel with the sequence */
                    Vp886ExtendTimerMs(NULL, pLineCtx, VP886_TIMERID_HOOK_FREEZE, hookMaskMs, 0);
                    Vp886SetDetectMask(pLineCtx, VP_CSLAC_HOOK);

                    Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CID, delayMs, timerOverrun, 0);
                    pCidObj->state = VP886_CID_ST_SILENT_HOOK_MASK_INTERVAL;
                    keepGoing = FALSE;

                } else if (pCidObj->state == VP886_CID_ST_SILENT_HOOK_MASK_INTERVAL) {
                    *pIdx += 6;
                    pCidObj->state = VP886_CID_ST_NEW_INSTR;
                    keepGoing = TRUE;
                }
                break;
            }
            case VP_CLI_DETECT: {
                VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
                Vp886DeviceObjectType *pDevObj = pDevCtx->pDevObj;
                
                if (pCidObj->state == VP886_CID_ST_NEW_INSTR) {
                    uint32 delayMs = (pProf[*pIdx + 2] << 8) + pProf[*pIdx + 3];
                    VpDigitType digit;
                    /* In the profile, the digit values are shifted by 4 bits,
                       except for VP_DIG_NONE.  For example, VP_DIG_5 is 0x50
                       in the profile, instead of the VpDigitType value of of
                       0x05. */
                    digit = pProf[*pIdx + 5];
                    if (digit != VP_DIG_NONE) {
                        digit = digit >> 4;
                    }
                    pCidObj->detectDigit1 = digit;

                    digit = pProf[*pIdx + 7];
                    if (digit != VP_DIG_NONE) {
                        digit = digit >> 4;
                    }
                    pCidObj->detectDigit2 = digit;
                    
                    pCidObj->digitDetected = FALSE;
                    
                    VP_CID(VpLineCtxType, pLineCtx, ("Detect interval for 0x%02X and 0x%02X, timeout %lu",
                        pCidObj->detectDigit1, pCidObj->detectDigit2, delayMs));

                    /* Tell the system to enable DTMF detection */
                    VpSysDtmfDetEnable(pDevObj->deviceId, pLineObj->channelId);

                    /* If a digit is detected, Vp886DtmfDigitDetected() will
                       cancel this timeout and resume the sequence */
                    Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CID, delayMs, timerOverrun, 0);
                    pCidObj->state = VP886_CID_ST_DETECT_INTERVAL;
                    /* Make sure the TX path is turned on.  This must come after setting the CID state
                       so that Vp886ApplyPcmTxRx() knows to override TX */
                    Vp886ApplyPcmTxRx(pLineCtx);
                    keepGoing = FALSE;
                } else {
                    if (pCidObj->digitDetected) {
                        VP_CID(VpLineCtxType, pLineCtx, ("Valid digit detected, resuming sequence"));
                        *pIdx += 8;
                        pCidObj->state = VP886_CID_ST_NEW_INSTR;
                        /* Tell the system to disable DTMF detection */
                        VpSysDtmfDetDisable(pDevObj->deviceId, pLineObj->channelId);
                        /* Restore PCM TX/RX state.  This must come after setting the CID state */
                        Vp886ApplyPcmTxRx(pLineCtx);
                        keepGoing = TRUE;
                    } else {
                        VP_CID(VpLineCtxType, pLineCtx, ("Neither required digit detected, ending sequence"));
                        /* End the CID sequence.  Vp886CidStop() will call VpSysDtmfDetDisable(). */
                        Vp886CidStop(pLineCtx);
                        keepGoing = FALSE;
                    }
                }
                break;
            }
            case VP_CLI_CHANSEIZURE: {
                keepGoing = Vp886CidHandleMarkOrSeizure(pLineCtx, VP_FSK_CHAN_SEIZURE);
                break;
            }
            case VP_CLI_MARKSIGNAL: {
                keepGoing = Vp886CidHandleMarkOrSeizure(pLineCtx, VP_FSK_MARK_SIGNAL);
                break;
            }
            case VP_CLI_MESSAGE: {
                keepGoing = Vp886CidHandleFskMsg(pLineCtx);
                break;
            }
            case VP_CLI_DTMF_MESSAGE: {
                keepGoing = Vp886CidHandleDtmfMsg(pLineCtx);
                break;
            }
            case VP_CLI_EOT: {
                VP_CID(VpLineCtxType, pLineCtx, ("End of transmission"));
                Vp886CidStop(pLineCtx);
                keepGoing = FALSE;
                break;
            }
            default:
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CidHandler - unknown instruction %d", instr));
                Vp886CidStop(pLineCtx);
                keepGoing = FALSE;
                break;
        }
    }

    return;
}


/** Vp886CidHandleMarkOrSeizure()
  Processes mark and channel seizure instructions in a CID sequence.
  
  Substates:
    NEW_INSTR:
        Set up FSK generator, calculate number of bytes for the specified
        duration, and proceed to main state MARK_OR_SEIZURE.
    MARK_OR_SEIZURE:
        Read the device CID status to determine how much space is available in
        the device buffer.  Program that number of bytes from the API buffer
        into the device.  If the API buffer is empty, peek ahead to see if the
        next instruction is FSK or not.  If the next instruction is FSK, move
        on to the next instruction.  If not, go to MARK_OR_SEIZURE_ENDING.
    MARK_OR_SEIZURE_ENDING:
        Wait here until the device CID state becomes IDLE, then disable FSK and
        move on.
*/
bool
Vp886CidHandleMarkOrSeizure(
    VpLineCtxType *pLineCtx,
    uint8 type)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpProfilePtrType pProf = pCidObj->pProf;
    uint8 *pIdx = &pCidObj->profIdx;
    uint8 cidParamReg;
    uint8 bufferSpace;
    uint8 bytesAdded = 0;
    uint8 nextInstr;
    bool keepGoing = FALSE;

    if (pCidObj->state == VP886_CID_ST_MARK_OR_SEIZURE_ENDING) {
        /* Make sure the CID state is IDLE. If not, we will check again at the
           next CID interrupt or when the timer expires */
        VpSlacRegRead(NULL, pLineCtx, VP886_R_CIDPARAM_RD, VP886_R_CIDPARAM_LEN, &cidParamReg);
        if ((cidParamReg & VP886_R_CIDPARAM_ST) != VP886_R_CIDPARAM_ST_IDLE) {
            VP_CID(VpLineCtxType, pLineCtx, ("End of %s, not idle yet (0x%02X) (ts %u)",
                type == VP_FSK_CHAN_SEIZURE ? "Channel Seizure" : "Mark Signal",
                cidParamReg, Vp886GetTimestamp(pLineCtx->pDevCtx)));
            return FALSE;
        }

        /* FSK has now ended */
        cidParamReg = VP886_R_CIDPARAM_CID_DIS;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);
        VP_CID(VpLineCtxType, pLineCtx, ("End of %s, shutting down FSK (ts %u)",
            type == VP_FSK_CHAN_SEIZURE ? "Channel Seizure" : "Mark Signal",
            Vp886GetTimestamp(pLineCtx->pDevCtx)));

        pCidObj->fskEnabled = FALSE;
        /* Restore PCM TX/RX */
        Vp886ApplyPcmTxRx(pLineCtx);
        
        *pIdx += 4;
        pCidObj->state = VP886_CID_ST_NEW_INSTR;
        return TRUE;
    }
    
    if (pCidObj->state == VP886_CID_ST_NEW_INSTR) {
        uint16 duration;

        if (!pCidObj->fskEnabled) {
            /* Program siggen C/D for the FSK parameters */
            VP_CID(VpLineCtxType, pLineCtx, ("Starting FSK for %s (ts %u)",
                type == VP_FSK_CHAN_SEIZURE ? "Channel Seizure" : "Mark Signal",
                Vp886GetTimestamp(pLineCtx->pDevCtx)));
            VpSlacRegWrite(NULL, pLineCtx, NOOP_CMD,
                pProf[VP_CID_PROFILE_FSK_PARAM_LEN], &pProf[VP_CID_PROFILE_FSK_PARAM_LEN + 1]);

            pCidObj->fskEnabled = TRUE;

            /* PCM TX/RX should be disabled during FSK */
            Vp886ApplyPcmTxRx(pLineCtx);
        }
        
        cidParamReg = VP886_R_CIDPARAM_CID_EN;
        if (type == VP_FSK_CHAN_SEIZURE) {
            cidParamReg |= VP886_R_CIDPARAM_START_BIT_0;
            cidParamReg |= VP886_R_CIDPARAM_STOP_BIT_1;
        } else {
            cidParamReg |= VP886_R_CIDPARAM_START_BIT_1;
            cidParamReg |= VP886_R_CIDPARAM_STOP_BIT_1;
        }
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);
        
        
        /* The FSK generator works at 1200 bits per second.  Including framing
           bits, each byte written to the buffer counts for 10 bits sent, so
           we send 0.12 bytes per millisecond, rounded to the nearest byte. */
        duration = (pProf[*pIdx + 2] << 8) + pProf[*pIdx + 3];
        pCidObj->markOrSeizureBytes = VpRoundedDivide((uint32)duration * 12, 100);
        if (pCidObj->markOrSeizureBytes == 0) {
            pCidObj->markOrSeizureBytes = 1;
        }
        VP_CID(VpLineCtxType, pLineCtx, ("%s, %d bytes",
            type == VP_FSK_CHAN_SEIZURE ? "Channel Seizure" : "Mark Signal",
            pCidObj->markOrSeizureBytes));
        
        pCidObj->state = VP886_CID_ST_MARK_OR_SEIZURE;
    }
    
    VpSlacRegRead(NULL, pLineCtx, VP886_R_CIDPARAM_RD, VP886_R_CIDPARAM_LEN, &cidParamReg);
    bufferSpace = Vp886CidFskBufferSpace(pLineCtx, cidParamReg);
    
    while (bytesAdded < bufferSpace && pCidObj->markOrSeizureBytes > 1) {
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &type);
        pCidObj->markOrSeizureBytes--;
        bytesAdded++;
    }
    
    /* Handle the final byte if there's room left */
    if (pCidObj->markOrSeizureBytes == 1 && bytesAdded < bufferSpace) {
        /* If the next instruction is also FSK, we can move on seamlessly, with
           no interruption in loading data.  If it is not FSK, we need to load
           the last byte with the EOM bit set, then wait for it to finish
           before continuing. */
        if (*pIdx + 5 > pProf[VP_PROFILE_LENGTH] + VP_PROFILE_LENGTH) {
            nextInstr = VP_CLI_EOT;
        } else {
            nextInstr = pProf[*pIdx + 5];
        }
        switch (nextInstr) {
            case VP_CLI_CHANSEIZURE:
            case VP_CLI_MARKSIGNAL:
            case VP_CLI_MESSAGE: {
                /* FSK, add the last byte and move to the next instruction */
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &type);
                VP_CID(VpLineCtxType, pLineCtx, ("Last byte of %s, moving on to more FSK (ts %u)",
                    type == VP_FSK_CHAN_SEIZURE ? "Channel Seizure" : "Mark Signal",
                    Vp886GetTimestamp(pLineCtx->pDevCtx)));
                pCidObj->markOrSeizureBytes--;
                bytesAdded++;

                *pIdx += 4;
                pCidObj->state = VP886_CID_ST_NEW_INSTR;
                keepGoing = TRUE;
                break;
            }
            default: {
                uint32 timerMs;
                /* Non-FSK, set the EOM bit and add the last byte */
                cidParamReg |= VP886_R_CIDPARAM_EOM;
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &type);
                VP_CID(VpLineCtxType, pLineCtx, ("Last byte of %s, ending FSK by setting EOM (ts %u)",
                    type == VP_FSK_CHAN_SEIZURE ? "Channel Seizure" : "Mark Signal",
                    Vp886GetTimestamp(pLineCtx->pDevCtx)));
                pCidObj->markOrSeizureBytes--;
                bytesAdded++;

                /* Each byte takes 8.333ms to transmit.  Set a timer for 9ms per
                   byte programmed, including the byte that may be currently
                   transmitting */
                timerMs = 9 * (bytesAdded + (4 - bufferSpace));
                Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CID, timerMs, 0, 0);
                pCidObj->state = VP886_CID_ST_MARK_OR_SEIZURE_ENDING;
                break;
            }
        }
    }
    
    VP_CID(VpLineCtxType, pLineCtx, ("%s buffer space %d, bytes added %d (ts %u)",
            type == VP_FSK_CHAN_SEIZURE ? "Channel Seizure" : "Mark Signal",
            bufferSpace, bytesAdded, Vp886GetTimestamp(pLineCtx->pDevCtx)));

    return keepGoing;
}


/** Vp886CidHandleFskMsg()
  Processes an FSK message instruction in a CID sequence.
  
  Substates:
    NEW_INSTR:
        Set up FSK generator and proceed to main state MESSAGE_FSK.
    MESSAGE_FSK:
        Read the device CID status to determine how much space is available in
        the device buffer.  Program that number of bytes from the API buffer
        into the device.  If the API buffer is empty and markout is required,
        move on to MESSAGE_FSK_MARKOUT, which is handled by
        Vp886CidHandleFskMsgMarkout().  If markout is not required, peek ahead
        to see if the next instruction is FSK or not.  If the next instruction
        is FSK, move on to the next instruction.  If not, go to
        MESSAGE_FSK_ENDING.
    MESSAGE_FSK_ENDING:
        Wait here until the device CID state becomes IDLE, then disable FSK and
        move on.
*/
bool
Vp886CidHandleFskMsg(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpProfilePtrType pProf = pCidObj->pProf;
    uint8 *pIdx = &pCidObj->profIdx;
    uint8 cidParamReg;
    uint8 bufferSpace;
    uint8 bytesAdded = 0;
    uint8 checksumIdx;
    bool apiChecksum;
    bool lastByte;
    uint8 msgByte;
    uint8 nextInstr;
    bool keepGoing = FALSE;
    
    if (pCidObj->state == VP886_CID_ST_MESSAGE_FSK_MARKOUT) {
        return Vp886CidHandleFskMsgMarkout(pLineCtx);
    }
    
    if (pCidObj->state == VP886_CID_ST_MESSAGE_FSK_ENDING) {
        /* Make sure the CID state is IDLE. If not, we will check again at the
           next CID interrupt or when the timer expires */
        VpSlacRegRead(NULL, pLineCtx, VP886_R_CIDPARAM_RD, VP886_R_CIDPARAM_LEN, &cidParamReg);
        if ((cidParamReg & VP886_R_CIDPARAM_ST) != VP886_R_CIDPARAM_ST_IDLE) {
            VP_CID(VpLineCtxType, pLineCtx, ("End of message, not idle yet (0x%02X) (ts %u)",
                cidParamReg, Vp886GetTimestamp(pLineCtx->pDevCtx)));
            return FALSE;
        }

        /* FSK has now ended */
        cidParamReg = VP886_R_CIDPARAM_CID_DIS;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);
        VP_CID(VpLineCtxType, pLineCtx, ("End of message, shutting down FSK (ts %u)",
            Vp886GetTimestamp(pDevCtx)));

        pCidObj->fskEnabled = FALSE;
        /* Restore PCM TX/RX */
        Vp886ApplyPcmTxRx(pLineCtx);
        
        *pIdx += 2;
        pCidObj->state = VP886_CID_ST_NEW_INSTR;
        return TRUE;
    }

    if (pCidObj->state == VP886_CID_ST_NEW_INSTR) {
        if (!pCidObj->fskEnabled) {
            VP_CID(VpLineCtxType, pLineCtx, ("Starting FSK for message (ts %u)",
                Vp886GetTimestamp(pDevCtx)));
            /* Program siggen C/D for the FSK parameters */
            VpSlacRegWrite(NULL, pLineCtx, NOOP_CMD,
                pProf[VP_CID_PROFILE_FSK_PARAM_LEN], &pProf[VP_CID_PROFILE_FSK_PARAM_LEN + 1]);

            pCidObj->fskEnabled = TRUE;

            /* PCM TX/RX should be disabled during FSK */
            Vp886ApplyPcmTxRx(pLineCtx);
        }
        
        cidParamReg = VP886_R_CIDPARAM_CID_EN;
        cidParamReg |= VP886_R_CIDPARAM_START_BIT_0;
        cidParamReg |= VP886_R_CIDPARAM_STOP_BIT_1;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);

        pCidObj->state = VP886_CID_ST_MESSAGE_FSK;
        pCidObj->needData = FALSE;

        VP_CID(VpLineCtxType, pLineCtx, ("FSK Msg length %d", pCidObj->msgLen));
    }
    
    VpSlacRegRead(NULL, pLineCtx, VP886_R_CIDPARAM_RD, VP886_R_CIDPARAM_LEN, &cidParamReg);
    bufferSpace = Vp886CidFskBufferSpace(pLineCtx, cidParamReg);
    
    checksumIdx = VP_CID_PROFILE_FSK_PARAM_LEN +
        pCidObj->pProf[VP_CID_PROFILE_FSK_PARAM_LEN] +
        VP_CID_PROFILE_CHECKSUM_OFFSET_LSB;
    if (pProf[checksumIdx] == 0x01) {
        apiChecksum = TRUE;
    } else {
        apiChecksum = FALSE;
    }
    
    if (apiChecksum && pCidObj->msgLen == 0) {
        lastByte = TRUE;
    } else if (!apiChecksum && pCidObj->msgLen == 1) {
        lastByte = TRUE;
    } else {
        lastByte = FALSE;
    }
    while (bytesAdded < bufferSpace && !lastByte) {
        msgByte = pCidObj->msgBuf[pCidObj->msgIdx];
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &msgByte);
        /* VP_CID(VpLineCtxType, pLineCtx, ("FSK Msg byte: 0x%02X", msgByte)); */
        pCidObj->msgLen--;
        pCidObj->msgIdx = (pCidObj->msgIdx + 1) % VP886_CID_BUFFER_SIZE;
        bytesAdded++;
        pCidObj->checksum += msgByte;

        if (apiChecksum && pCidObj->msgLen == 0) {
            lastByte = TRUE;
        } else if (!apiChecksum && pCidObj->msgLen == 1) {
            lastByte = TRUE;
        } else {
            lastByte = FALSE;
        }
    }
    
    /* Handle the final byte if there's room left */
    if (lastByte && bytesAdded < bufferSpace) {
        if (apiChecksum) {
            /* Take the two's complement of the checksum (invert and add one) */
            msgByte = ~pCidObj->checksum + 1;
            VP_CID(VpLineCtxType, pLineCtx, ("API Checksum: ~0x%02X + 1 -> 0x%02X", pCidObj->checksum, msgByte));
        } else {
            msgByte = pCidObj->msgBuf[pCidObj->msgIdx];
        }
        
        /* If markout is required, send the last byte, but don't move on to
           the next instruction yet.  Set up the parameters for markout, and
           move on to the markout state. */
        if (pProf[*pIdx] > 0) {
            pCidObj->markOrSeizureBytes = pProf[*pIdx];
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &msgByte);
            VP_CID(VpLineCtxType, pLineCtx, ("Last msg byte 0x%02X, moving to Markout, %d bytes (ts %u)",
                msgByte, pCidObj->markOrSeizureBytes, Vp886GetTimestamp(pDevCtx)));
            pCidObj->msgLen--;
            bytesAdded++;
        
            cidParamReg = VP886_R_CIDPARAM_CID_EN;
            cidParamReg |= VP886_R_CIDPARAM_START_BIT_1;
            cidParamReg |= VP886_R_CIDPARAM_STOP_BIT_1;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);

            VP_CID(VpLineCtxType, pLineCtx, ("FSK Msg buffer space %d, bytes added %d (ts %u)", 
                bufferSpace, bytesAdded, Vp886GetTimestamp(pDevCtx)));

            pCidObj->state = VP886_CID_ST_MESSAGE_FSK_MARKOUT;
            return TRUE;
        }
        
        /* If the next instruction is also FSK, we can move on seamlessly, with
           no interruption in loading data.  If it is not FSK, we need to load
           the last byte with the EOM bit set, then wait for it to finish
           before continuing. */
        if (*pIdx + 3 > pProf[VP_PROFILE_LENGTH] + VP_PROFILE_LENGTH) {
            nextInstr = VP_CLI_EOT;
        } else {
            nextInstr = pProf[*pIdx + 3];
        }
        switch (nextInstr) {
            case VP_CLI_CHANSEIZURE:
            case VP_CLI_MARKSIGNAL:
            case VP_CLI_MESSAGE: {
                /* FSK, add the last byte and move to the next instruction */
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &msgByte);
                VP_CID(VpLineCtxType, pLineCtx, ("Last msg byte 0x%02X, moving on to more FSK (ts %u)", msgByte, Vp886GetTimestamp(pDevCtx)));
                pCidObj->msgLen--;
                bytesAdded++;

                *pIdx += 2;
                pCidObj->state = VP886_CID_ST_NEW_INSTR;
                keepGoing = TRUE;
                break;
            }
            default: {
                uint32 timerMs;
                /* Non-FSK, set the EOM bit and add the last byte */
                cidParamReg |= VP886_R_CIDPARAM_EOM;
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &msgByte);
                VP_CID(VpLineCtxType, pLineCtx, ("Last msg byte 0x%02X, ending FSK by setting EOM (ts %u)", msgByte, Vp886GetTimestamp(pDevCtx)));
                pCidObj->msgLen--;
                bytesAdded++;

                /* Each byte takes 8.333ms to transmit.  Set a timer for 9ms per
                   byte programmed, including the byte that may be currently
                   transmitting */
                timerMs = 9 * (bytesAdded + (4 - bufferSpace));
                Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CID, timerMs, 0, 0);
                pCidObj->state = VP886_CID_ST_MESSAGE_FSK_ENDING;
                break;
            }
        }
    }
    
    /* If 16 or more bytes are available in the API buffer and we're not already
       waiting for a VpContinueCid call, generate the NEED_MORE_DATA event */
    if (VP886_CID_BUFFER_SIZE - pCidObj->msgLen >= 16 && !pCidObj->needData) {
        VP_CID(VpLineCtxType, pLineCtx, ("Requesting more data (%d remaining)", pCidObj->msgLen));
        pCidObj->needData = TRUE;
        Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_CID_DATA,
            VP_CID_DATA_NEED_MORE_DATA, Vp886GetTimestamp(pDevCtx), FALSE);
    }
    
    VP_CID(VpLineCtxType, pLineCtx, ("FSK Msg buffer space %d, bytes added %d (ts %u)", 
        bufferSpace, bytesAdded, Vp886GetTimestamp(pDevCtx)));

    return keepGoing;
}


/** Vp886CidHandleFskMsgMarkout()
  Handles the MESSAGE_FSK_MARKOUT substate of an FSK message instruction in a
  CID sequence.
*/
bool
Vp886CidHandleFskMsgMarkout(
    VpLineCtxType *pLineCtx)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpProfilePtrType pProf = pCidObj->pProf;
    uint8 *pIdx = &pCidObj->profIdx;
    uint8 cidParamReg;
    uint8 bufferSpace;
    uint8 bytesAdded = 0;
    uint8 nextInstr;
    uint8 msgByte = VP_FSK_MARK_SIGNAL;
    bool keepGoing = FALSE;
    
    VpSlacRegRead(NULL, pLineCtx, VP886_R_CIDPARAM_RD, VP886_R_CIDPARAM_LEN, &cidParamReg);
    bufferSpace = Vp886CidFskBufferSpace(pLineCtx, cidParamReg);

    while (bytesAdded < bufferSpace && pCidObj->markOrSeizureBytes > 1) {
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &msgByte);
        pCidObj->markOrSeizureBytes--;
        bytesAdded++;
    }
    
    
    /* Handle the final byte if there's room left */
    if (pCidObj->markOrSeizureBytes == 1 && bytesAdded < bufferSpace) {
        /* If the next instruction is also FSK, we can move on seamlessly, with
           no interruption in loading data.  If it is not FSK, we need to load
           the last byte with the EOM bit set, then wait for it to finish
           before continuing. */
        if (*pIdx + 3 > pProf[VP_PROFILE_LENGTH] + VP_PROFILE_LENGTH) {
            nextInstr = VP_CLI_EOT;
        } else {
            nextInstr = pProf[*pIdx + 3];
        }
        switch (nextInstr) {
            case VP_CLI_CHANSEIZURE:
            case VP_CLI_MARKSIGNAL:
            case VP_CLI_MESSAGE: {
                /* FSK, add the last byte and move to the next instruction */
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &msgByte);
                VP_CID(VpLineCtxType, pLineCtx, ("Last byte of Markout, moving on to more FSK (ts %u)",
                    Vp886GetTimestamp(pLineCtx->pDevCtx)));
                pCidObj->markOrSeizureBytes--;
                bytesAdded++;

                *pIdx += 2;
                pCidObj->state = VP886_CID_ST_NEW_INSTR;
                keepGoing = TRUE;
                break;
            }
            default: {
                uint32 timerMs;
                /* Non-FSK, set the EOM bit and add the last byte */
                cidParamReg |= VP886_R_CIDPARAM_EOM;
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDPARAM_WRT, VP886_R_CIDPARAM_LEN, &cidParamReg);
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_CIDDATA_WRT, VP886_R_CIDDATA_LEN, &msgByte);
                VP_CID(VpLineCtxType, pLineCtx, ("Last byte of Markout, ending FSK by setting EOM (ts %u)",
                    Vp886GetTimestamp(pLineCtx->pDevCtx)));
                pCidObj->markOrSeizureBytes--;
                bytesAdded++;

                /* Each byte takes 8.333ms to transmit.  Set a timer for 9ms per
                   byte programmed, including the byte that may be currently
                   transmitting */
                timerMs = 9 * (bytesAdded + (4 - bufferSpace));
                Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_CID, timerMs, 0, 0);
                pCidObj->state = VP886_CID_ST_MESSAGE_FSK_ENDING;
                break;
            }
        }
    }

    VP_CID(VpLineCtxType, pLineCtx, ("Markout buffer space %d, bytes added %d (ts %u)",
            bufferSpace, bytesAdded, Vp886GetTimestamp(pLineCtx->pDevCtx)));

    return keepGoing;
}


/** Vp886CidHandleDtmfMsg()
  Processes a DTMF message instruction in a CID sequence.

  The timing is handled by the device cadencer, and we update the tones at the
  end of each on-time by responding to the CAD interrupts.
*/
bool
Vp886CidHandleDtmfMsg(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    Vp886CidDataType *pCidObj = &pLineObj->cid;
    VpProfilePtrType pProf = pCidObj->pProf;
    uint8 *pIdx = &pCidObj->profIdx;
    char digitChar;
    VpDigitType vpDigit;
    uint16 amplitude;
    uint8 sigctrlReg;
    
    if (pCidObj->state == VP886_CID_ST_NEW_INSTR) {
        uint16 onTime;
        uint16 offTime;
        uint8 cadenceReg[VP886_R_CADENCE_LEN];
        
        if (pCidObj->msgLen == 0) {
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886CidHandleDtmfMsg - No message data found"));
            *pIdx += 2;
            pCidObj->state = VP886_CID_ST_NEW_INSTR;
            return TRUE;
        }
        
        /* Set up the device's cadence timer for the DTMF on-time and off-time. */
        onTime = VP_CID_DTMF_ON_TIME / 5;
        offTime = VP_CID_DTMF_OFF_TIME / 5;
        cadenceReg[0] = (onTime >> 8) & VP886_R_CADENCE_MSB_MASK;
        cadenceReg[1] = onTime & 0x00FF;
        cadenceReg[2] = (offTime >> 8) & VP886_R_CADENCE_MSB_MASK;
        cadenceReg[3] = offTime & 0x00FF;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_CADENCE_WRT, VP886_R_CADENCE_LEN, cadenceReg);

        pCidObj->needData = FALSE;
        pCidObj->state = VP886_CID_ST_MESSAGE_DTMF;
    }
    
    if (pCidObj->msgLen == 0) {
        /* Message is finished, disable the signal generator */
        VP_CID(VpLineCtxType, pLineCtx, ("DTMF message finished"));
        sigctrlReg = 0x00;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigctrlReg);
        pCidObj->dtmfEnabled = FALSE;

        *pIdx += 2;
        pCidObj->state = VP886_CID_ST_NEW_INSTR;
        return TRUE;
    }
    
    digitChar = pCidObj->msgBuf[pCidObj->msgIdx];
    switch(digitChar) {
        case '0': vpDigit = VP_DIG_ZERO; break;
        case 'A': vpDigit = VP_DIG_A; break;
        case 'B': vpDigit = VP_DIG_B; break;
        case 'C': vpDigit = VP_DIG_C; break;
        case 'D': vpDigit = VP_DIG_D; break;
        case '*': vpDigit = VP_DIG_ASTER; break;
        case '#': vpDigit = VP_DIG_POUND; break;
        default: vpDigit = (VpDigitType)(digitChar-48); break;
    }
    pCidObj->msgIdx = (pCidObj->msgIdx + 1) % VP886_CID_BUFFER_SIZE;
    pCidObj->msgLen--;
    
    /* Extract the requested amplitude from the CID profile */
    amplitude = pProf[VP_CID_PROFILE_FSK_PARAM_CMD + 3];
    amplitude = (amplitude << 8) & 0xFF00;
    amplitude |= pProf[VP_CID_PROFILE_FSK_PARAM_CMD + 4];
    
    /* Program the digit to the signal generator parameters (C and D) */
    VP_CID(VpLineCtxType, pLineCtx, ("DTMF digit %c", digitChar));
    if (Vp886ProgramDTMFDigit(pLineCtx, vpDigit, amplitude) != VP_STATUS_SUCCESS) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp886CidHandleDtmfMsg - Invalid digit 0x%02X", digitChar));
        /* Skip invalid digits.  Returning TRUE here will get us right back to
           the top of this function. */
        return TRUE;
    }
    
    /* Enable the signal generators, using the device cadencer in continuous
       mode.  The CAD interrupts at the end of each on-time will prompt us to
       program each digit. */
    if (!pCidObj->dtmfEnabled) {
        sigctrlReg = VP886_R_SIGCTRL_CADENCE | VP886_R_SIGCTRL_MODE_CONT |
                     VP886_R_SIGCTRL_EN_SIGC | VP886_R_SIGCTRL_EN_SIGD;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigctrlReg);
        pCidObj->dtmfEnabled = TRUE;
    }
    
    /* If 16 or more bytes are available in the API buffer and we're not already
       waiting for a VpContinueCid call, generate the NEED_MORE_DATA event */
    if (VP886_CID_BUFFER_SIZE - pCidObj->msgLen >= 16 && !pCidObj->needData) {
        VP_CID(VpLineCtxType, pLineCtx, ("Requesting more data (%d remaining)", pCidObj->msgLen));
        pCidObj->needData = TRUE;
        Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_CID_DATA,
            VP_CID_DATA_NEED_MORE_DATA, Vp886GetTimestamp(pDevCtx), FALSE);
    }

    /* Wait for the cadence interrupt before sending the next digit or moving
       to the next instruction */
    return FALSE;
}


/** Vp886CidFskBufferSpace()
  Returns the number of bytes that can be programmed into the device CID data
  buffer based on the status of the CIDPARAM register.
*/
uint8
Vp886CidFskBufferSpace(
    VpLineCtxType *pLineCtx,
    uint8 cidParamReg)
{
    switch (cidParamReg & VP886_R_CIDPARAM_ST) {
        case VP886_R_CIDPARAM_ST_UNDERRUN:
            VP_WARNING(VpLineCtxType, pLineCtx, ("FSK buffer underrun"));
        case VP886_R_CIDPARAM_ST_IDLE:
        case VP886_R_CIDPARAM_ST_EMPTY:
            return 3;
        case VP886_R_CIDPARAM_ST_ONETHIRD:
            return 2;
        case VP886_R_CIDPARAM_ST_TWOTHIRDS:
            return 1;
        default:
            return 0;
    }
}


/** Vp886CidAckDetect()
  Processes a DTMF digit make or break detection.
*/
void
Vp886CidAckDetect(
    VpLineCtxType *pLineCtx,
    VpDigitType digit,
    VpDigitSenseType sense)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;

    VP_CID(VpLineCtxType, pLineCtx, ("Vp886CidAckDetect, digit 0x%02X %s",
        digit, sense == VP_DIG_SENSE_BREAK ? "break" : "make"));

    /* If the digit matches one of the specified digits, validate the
       detect interval. When the BREAK is detected, cancel the timeout and
       proceed with the CID sequence. */
    if (pLineObj->cid.state == VP886_CID_ST_DETECT_INTERVAL &&
        (digit == pLineObj->cid.detectDigit1 || digit == pLineObj->cid.detectDigit2))
    {
        pLineObj->cid.digitDetected = TRUE;
        if (sense == VP_DIG_SENSE_BREAK) {
            Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_CID, 0, FALSE);
            Vp886CidHandler(pLineCtx, 0);
        }
    }
    
    return;
}


/** Vp886SendSignal()
  Implements VpSendSignal() to generate different timed signals on a line.

  See the VP-API-II Reference Guide for more details on VpSendSignal().
*/
VpStatusType
Vp886SendSignal(
    VpLineCtxType *pLineCtx,
    VpSendSignalType signalType,
    void *pSignalData)
{
    VpStatusType status = VP_STATUS_SUCCESS;
    
    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886SendSignal");

    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendSignal");
        return status;
    }

    /* Check for valid signal type and corresponding signal data */
    switch (signalType) {
        case VP_SENDSIG_POLREV_PULSE:
        case VP_SENDSIG_TIP_OPEN_PULSE:
        case VP_SENDSIG_FWD_DISCONNECT:
            if (pSignalData == VP_NULL) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SendSignal - NULL pSignalData invalid for this signalType"));
                Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendSignal");
                return VP_STATUS_INVALID_ARG;
            }
            break;
        case VP_SENDSIG_MSG_WAIT_PULSE:
            break;
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SendSignal - unsupported signalType %d", signalType));
            Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendSignal");
            return VP_STATUS_INVALID_ARG;
    }

    status = Vp886SendSignalInt(pLineCtx, signalType, pSignalData);
    
    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886SendSignal");
    return status;
}


/** Vp886SendSignalInt()
  Performs the initial action of a VpSendSignal() call, based on the signalType.
  Further required actions are processed by Vp886SendSignalHandler().
*/
VpStatusType
Vp886SendSignalInt(
    VpLineCtxType *pLineCtx,
    VpSendSignalType signalType,
    void *pSignalData)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (pLineObj->sendSignal.active) {
        Vp886SendSignalStop(pLineCtx, FALSE);
    }

    pLineObj->sendSignal.type = signalType;

    switch (signalType) {
        case VP_SENDSIG_POLREV_PULSE: {
            VpLineStateType polrevState;
            polrevState = Vp886LineStateInfo(pLineObj->lineState.usrCurrent).polrevEquiv;
            Vp886SetLineStateFxsInt(pLineCtx, polrevState);
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_SENDSIGNAL, *((uint16*)pSignalData), 0, 0);
            break;
        }
        case VP_SENDSIG_TIP_OPEN_PULSE: {
            Vp886SetLineStateFxsInt(pLineCtx, VP_LINE_TIP_OPEN);
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_SENDSIGNAL, *((uint16*)pSignalData), 0, 0);
            break;
        }
        case VP_SENDSIG_FWD_DISCONNECT: {
            Vp886SetLineStateFxsInt(pLineCtx, VP_LINE_DISCONNECT);
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_SENDSIGNAL, *((uint16*)pSignalData), 0, 0);
            break;
        }
        case VP_SENDSIG_MSG_WAIT_PULSE: {
            VpSendMsgWaitType *pMsgWait = pSignalData;
            if (pMsgWait == VP_NULL || pMsgWait->onTime == 0) {
                /* Already stopped at the top of this function */
                return VP_STATUS_SUCCESS;
            }
            status = Vp886SendSignalMsgWaitPulse(pLineCtx, pMsgWait);
            if (status != VP_STATUS_SUCCESS) {
                return status;
            }
            break;
        }
        default:
            return VP_STATUS_INVALID_ARG;
    }

    pLineObj->sendSignal.active = TRUE;

    return status;
}


/** Vp886SendSignalMsgWaitPulse()
  Starts a VP_SENDSIG_MSG_WAIT_PULSE signal.  This signal uses the ringing
  generator parameters and the ringing line state, so we must save the
  ringing parameters to restore them later.  To save line object space, we
  store the ringing parameters register in the equivalent register for signal
  generator A&B.
*/
VpStatusType
Vp886SendSignalMsgWaitPulse(
    VpLineCtxType *pLineCtx,
    VpSendMsgWaitType *pMsgWait)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;
    uint8 ringParams[VP886_R_RINGGEN_LEN];
    int32 bias;
    VpLineStateType onState;
    
    /* Set up the ringing parameters with a very low amplitude so that it
       appears as a constant level when the DC bias is applied.
       Use a high frequency (400Hz) for a fairly quick "ring trip". */
    uint8 newRingParams[] = {0x00, 0x00, 0x00, 0x04, 0x44, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00};
    
    /* Disallow starting a msg wait pulse while offhook */
    if (pLineObj->lineState.condition & VP_CSLAC_HOOK) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Message wait pulse not allowed while offhook"));
        return VP_STATUS_DEVICE_BUSY;
    }

    /* Cache the ringing generator parameters in the siggen A register */
    VpSlacRegRead(NULL, pLineCtx, VP886_R_RINGGEN_RD, VP886_R_RINGGEN_LEN, ringParams);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, ringParams);

    /* Force ring trip mode to DC */
    pLineObj->registers.loopSup[2] &= ~VP886_R_LOOPSUP_RTRIP_ALG;
    pLineObj->registers.loopSup[2] |= VP886_R_LOOPSUP_RTRIP_ALG_DC;
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, pLineObj->registers.loopSup);
    
    bias = pMsgWait->voltage;

    /* If the current line state is a polrev state, invert the voltage and use
       RINGING_POLREV for the on-state to avoid line transient glitches */
    if (Vp886LineStateInfo(pLineObj->lineState.usrCurrent).polrev) {
        onState = VP_LINE_RINGING_POLREV;
        bias = -bias;
    } else {
        onState = VP_LINE_RINGING;
    }
    
    /* Ringing DC Bias is programmed in units of 4.711 mV
          B = V / 0.004711
       -> B = V / (4711 / 1000000)
       -> B = (1000000 * V) / 4711
    */
    bias = (bias * 1000000) / 4711;
    newRingParams[1] = (bias & 0x0000FF00) >> 8;
    newRingParams[2] = (bias & 0x000000FF);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_RINGGEN_WRT, VP886_R_RINGGEN_LEN, newRingParams);

    if (pMsgWait->offTime == 0) {
        /* Turn the signal on immediately and indefinitely */
        Vp886SetLineStateFxsInt(pLineCtx, onState);
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Msg wait pulse ON forever"));
        pLineObj->sendSignal.msgWait.on = TRUE;
        Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_SIGNAL_CMP,
            pLineObj->sendSignal.type, Vp886GetTimestamp(pDevCtx), FALSE);
        return VP_STATUS_SUCCESS;
    }

    pLineObj->sendSignal.active = TRUE;
    pLineObj->sendSignal.msgWait.voltage = pMsgWait->voltage;
    pLineObj->sendSignal.msgWait.onTime = pMsgWait->onTime;
    pLineObj->sendSignal.msgWait.offTime = pMsgWait->offTime;
    pLineObj->sendSignal.msgWait.cycles = pMsgWait->cycles;
    pLineObj->sendSignal.msgWait.on = FALSE;
    /* Begin the signal using the timer handler */
    Vp886SendSignalHandler(pLineCtx, 0);
    
    return VP_STATUS_SUCCESS;
}


/** Vp886SendSignalHandler()
  Handles timers to continue or complete signals started by VpSendSignal().
*/
void
Vp886SendSignalHandler(
    VpLineCtxType *pLineCtx,
    uint32 overrun)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;

    if (!pLineObj->sendSignal.active) {
        return;
    }

    switch (pLineObj->sendSignal.type) {
        case VP_SENDSIG_POLREV_PULSE:
        case VP_SENDSIG_TIP_OPEN_PULSE:
        case VP_SENDSIG_FWD_DISCONNECT: {
            Vp886SetLineStateFxsInt(pLineCtx, pLineObj->lineState.usrCurrent);
            Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_SIGNAL_CMP,
                pLineObj->sendSignal.type, Vp886GetTimestamp(pDevCtx), FALSE);
            pLineObj->sendSignal.active = FALSE;
            break;
        }
        case VP_SENDSIG_MSG_WAIT_PULSE: {
            if (pLineObj->sendSignal.msgWait.on == FALSE) {
                /* Turn signal on */
                VpLineStateType onState;
                if (Vp886LineStateInfo(pLineObj->lineState.usrCurrent).polrev) {
                    onState = VP_LINE_RINGING_POLREV;
                } else {
                    onState = VP_LINE_RINGING;
                }
                Vp886SetLineStateFxsInt(pLineCtx, onState);
                VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Msg wait pulse ON"));
                pLineObj->sendSignal.msgWait.on = TRUE;
                Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_SENDSIGNAL,
                    pLineObj->sendSignal.msgWait.onTime, overrun, 0);
            } else {
                /* Turn signal off */
                Vp886SetLineStateFxsInt(pLineCtx, pLineObj->lineState.usrCurrent);
                VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Msg wait pulse OFF"));
                if (pLineObj->sendSignal.msgWait.cycles == 1) {
                    /* Finishing the last remaining cycle */
                    Vp886SendSignalStop(pLineCtx, TRUE);
                    break;
                }
                if (pLineObj->sendSignal.msgWait.cycles != 0) {
                    pLineObj->sendSignal.msgWait.cycles--;
                }
                pLineObj->sendSignal.msgWait.on = FALSE;
                Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_SENDSIGNAL,
                    pLineObj->sendSignal.msgWait.offTime, overrun, 0);
            }
            break;
        }
        default:
            break;
    }

    return;
}


/** Vp886SendSignalStop()
  Ends and cleans up after a signal that was started by VpSendSignal().  This
  can be used when a signal is interrupted or aborted, or to clean up when it
  ends normally.
*/
void
Vp886SendSignalStop(
    VpLineCtxType *pLineCtx,
    bool restoreLineState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;

    Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_SENDSIGNAL, 0, FALSE);

    if (pLineObj->sendSignal.type == VP_SENDSIG_MSG_WAIT_PULSE) {
        uint8 ringParams[VP886_R_RINGGEN_LEN];
        
        /* Restore the ring trip mode */
        pLineObj->registers.loopSup[2] &= ~VP886_R_LOOPSUP_RTRIP_ALG;
        pLineObj->registers.loopSup[2] |= pLineObj->ringTripAlg;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, pLineObj->registers.loopSup);

        /* Always restore the line state here before restoring the ringing
           params if it's in the on state */
        if (pLineObj->sendSignal.msgWait.on) {
            Vp886SetLineStateFxsInt(pLineCtx, pLineObj->lineState.usrCurrent);
            restoreLineState = FALSE;
        }
        
        /* Restore the ringing generator parameters from the siggen A register */
        VpSlacRegRead(NULL, pLineCtx, VP886_R_SIGAB_RD, VP886_R_SIGAB_LEN, ringParams);
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_RINGGEN_WRT, VP886_R_RINGGEN_LEN, ringParams);
        
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Msg wait pulse deactivated"));

        Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_SIGNAL_CMP,
            pLineObj->sendSignal.type, Vp886GetTimestamp(pDevCtx), FALSE);
    }

    if (restoreLineState) {
        Vp886SetLineStateFxsInt(pLineCtx, pLineObj->lineState.usrCurrent);
    }

    pLineObj->sendSignal.active = FALSE;

    return;
}


/** Vp886InitMeter()
  Implements VpInitMeter() to apply a metering profile to a line.

  See the VP-API-II Reference Guide for more details on VpInitMeter().
*/
VpStatusType
Vp886InitMeter(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pMeterProfile)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpStatusType status = VP_STATUS_SUCCESS;

    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886InitMeter");

    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitMeter");
        return status;
    }

    /* Check for valid profile arguments. */
    if (Vp886GetProfileArg(pDevCtx, VP_PROFILE_METER, &pMeterProfile) != VP_STATUS_SUCCESS) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitMeter");
        return VP_STATUS_ERR_PROFILE;
    }

    /* Apply the metering profile */
    VpSlacRegWrite(NULL, pLineCtx, VP886_NOOP,
        pMeterProfile[VP_PROFILE_MPI_LEN], &pMeterProfile[VP_PROFILE_DATA_START]);

    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886InitMeter");
    return status;
}


/** Vp886StartMeter()
  Implements VpStartMeter() to begin a metering signal a line or stop metering
  if numMeters==0.

  See the VP-API-II Reference Guide for more details on VpStartMeter().
*/
VpStatusType
Vp886StartMeter(
    VpLineCtxType *pLineCtx,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpStatusType status = VP_STATUS_SUCCESS;
    
    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886StartMeter");

    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886StartMeter");
        return status;
    }

    /* Can't perform metering in non-codec states */
    if (!Vp886LineStateInfo(pLineObj->lineState.usrCurrent).codec) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886StartMeterInt - Metering cannot be generated in the current line state (%d)",
            pLineObj->lineState.usrCurrent));
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886StartMeter");
        return VP_STATUS_DEVICE_BUSY;
    }
    
    status = Vp886StartMeterInt(pLineCtx, onTime, offTime, numMeters);

    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886StartMeter");
    return status;
}


/** Vp886StartMeterInt()
  Begins a metering signal, or aborts a metering signal if numMeters==0.
*/
VpStatusType
Vp886StartMeterInt(
    VpLineCtxType *pLineCtx,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;

    /* If numMeters is 0, abort any ongoing metering */
    if (numMeters == 0) {
        if (pLineObj->metering.on && pLineObj->metering.onTime != 0) {
            /* Flag the metering signal for an abort, but allow the current
               pulse to finish first */
            pLineObj->metering.abort = TRUE;
            return VP_STATUS_SUCCESS;
        } else {
            Vp886MeterStop(pLineCtx, TRUE);
            return VP_STATUS_SUCCESS;
        }
    }

    /* Any currently running metering sequence must be aborted to start the
       new one */
    if (pLineObj->metering.active) {
        Vp886MeterStop(pLineCtx, FALSE);
    }
    
    pLineObj->metering.active = TRUE;
    pLineObj->metering.onTime = onTime;
    pLineObj->metering.offTime = offTime;
    pLineObj->metering.remaining = numMeters;
    pLineObj->metering.completed = 0;
    pLineObj->metering.abort = FALSE;
    
    /* If onTime is 0, turn metering on indefinitely */
    if (onTime == 0) {
        pLineObj->registers.sysState[0] |= VP886_R_STATE_METER;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState);
        pLineObj->metering.on = TRUE;
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Metering pulse ON forever"));
        return VP_STATUS_SUCCESS;
    }
    
    /* Otherwise, start metering by using the timer handler */
    pLineObj->metering.on = FALSE;
    Vp886MeterHandler(pLineCtx, 0);
    
    return VP_STATUS_SUCCESS;
}


/** Vp886MeterHandler()
  Handles metering on/off operations, driven by the metering timer.
*/
void
Vp886MeterHandler(
    VpLineCtxType *pLineCtx,
    uint32 overrun)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;

    if (!pLineObj->metering.active) {
        return;
    }
    
    if (pLineObj->metering.on) {
        /* Currently on, switch to off */
        pLineObj->registers.sysState[0] &= ~VP886_R_STATE_METER;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState);
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Metering pulse OFF"));
        
        if (pLineObj->metering.completed < 0xFFFF) {
            pLineObj->metering.completed++;
        }
        
        if (pLineObj->metering.remaining == 1) {
            /* This finishes the last remaining pulse */
            Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_MTR_CMP,
                pLineObj->metering.completed, Vp886GetTimestamp(pDevCtx), FALSE);
            pLineObj->metering.active = FALSE;
            return;
        }
        
        if (pLineObj->metering.abort) {
            Vp886MeterStop(pLineCtx, FALSE);
        }
        
        /* If not doing an infinite count, decrement remaining pulse count */
        if (pLineObj->metering.remaining != 0) {
            pLineObj->metering.remaining--;
        }
        pLineObj->metering.on = FALSE;
        Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_METERING,
            pLineObj->metering.offTime * 10, overrun, 0);

    } else {
        /* Currently off, switch to on */
        pLineObj->registers.sysState[0] |= VP886_R_STATE_METER;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState);
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Metering pulse ON"));

        pLineObj->metering.on = TRUE;
        Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_METERING,
            pLineObj->metering.onTime * 10, overrun, 0);
    }
}


/** Vp886MeterStop()
  Aborts an ongoing metering signal.
    - cancels the metering timer
    - restores the system state register (if requested)
    - resets status flags
    - generates MTR_ABORT event
*/
void
Vp886MeterStop(
    VpLineCtxType *pLineCtx,
    bool restoreLineState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 channelId = pLineObj->channelId;

    Vp886CancelTimer(NULL, pLineCtx, VP886_TIMERID_METERING, 0, FALSE);

    if (restoreLineState) {
        Vp886SetLineStateFxsInt(pLineCtx, pLineObj->lineState.usrCurrent);
    }

    Vp886PushEvent(pDevCtx, channelId, VP_EVCAT_PROCESS, VP_LINE_EVID_MTR_ABORT,
        pLineObj->metering.completed, Vp886GetTimestamp(pDevCtx), FALSE);

    pLineObj->metering.active = FALSE;
    pLineObj->metering.on = FALSE;
    pLineObj->metering.remaining = 0;
    pLineObj->metering.completed = 0;
    pLineObj->metering.abort = FALSE;

    return;
}


/** Vp886HowlerToneInit()
  Programs and turns on the tone generators to output a howler tone.

  Frequency modulation is handled in the device by programming the siggens for
  FM mode.  Amplitude stepping is done by the API in Vp886HowlerToneHandler().
*/
bool
Vp886HowlerToneInit(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pCadProfile,
    VpStatusType *pStatus)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886SeqDataType *pCadence = &pLineObj->cadence;
    uint8 toneType;
    
    toneType = (pCadProfile[VP_CSLAC_TONE_TYPE] & VP_CSLAC_SPECIAL_TONE_MASK);
    
    if (toneType == VP_CSLAC_STD_TONE) {
        return FALSE;
    }

    VpMemSet(pCadence->regData, 0, VPCSLAC_MAX_GENERATOR_DATA);
    
    switch(toneType) {
        case VP_CSLAC_UK_HOWLER_TONE_VER15: { /* UK Howler from BTNR 1080 */
            uint8 sigCD[VP886_R_SIGCD_LEN];
            /*  Issue 15: Frequency Range = [800Hz to 3.2kHz]
                     sweep middle = 2000 Hz
                     sweep range = +/-1200 Hz
              
                Use the frequency modulation mode of the signal generators,
                where the output of siggen A + bias determines the frequency of
                generator D.  Also use trapezoidal waveform for A, which
                changes the meaning of the register fields.
            */
            VpMemSet(sigCD, 0, VP886_R_SIGCD_LEN);
            sigCD[4] = VP886_R_SIGCD_FM_ENABLE;
            
            pCadence->regData[0] = VP886_R_SIGAB_WAVE_TRAP;
            
            /* Generator B frequency controls sweep rate
                FRQB = 8000 / Fsweep
               For a 1000ms (1Hz) cycle, FRQB = 0x1F40 */
            pCadence->regData[7] = 0x1F;
            pCadence->regData[8] = 0x40;

            /* Generator A frequency controls rise time
                FRQA = 2^15 / (Trise * 12000)
               We want a rise time of 500ms (half of the sweep) so that the
               tone changes continuously.  The closest values we can program
               are 546ms (0x0005) and 455ms (0x0006).  Using the slower rise
               time would cut the ramps short to 500ms, decreasing the
               amplitude.  Using the faster rise time would mean that we linger
               for a short time at the min and max frequencies.  It would be
               simpler to use the faster rise time, but to be more precise we
               will use the slower clipped rise time.  This will require
               adjustments to the amplitude and bias frequencies. */
            pCadence->regData[3] = 0x00;
            pCadence->regData[4] = 0x05;

            /* Generator A amplitude specifies the frequency sweep +/- range.
               We want a sweep of +/- 1200Hz, but whatever we program will be
               cut short at a factor of 500/546 because our ramp time (546ms) is
               more than half of the sweep period (1000ms).  To compensate, we
               multiply the range we want by 546/500 to get 1310.4Hz */
            pCadence->regData[5] = 0x0D;
            pCadence->regData[6] = 0xFA;

            /* The Bias parameter sets up the middle frequency of the sweep.
               The way the device handles cutting short the rise time, it does
               not keep the sweep centered.  It reaches the first full peak, but
               then is cut short afterward.  Since we will begin by ramping down
               from center, this means the lowest frequency will match up with
               the amplitude that we programmed but the high frequency will be
               much lower than what we programmed. To get both points to where
               we want them, we need to add the same offset to the bias that was
               added to the sweep range.  2000 + 110.4 = 2110.4Hz */
            pCadence->regData[1] = 0x16;
            pCadence->regData[2] = 0x83;

            /* If we start the tone immediately, it will begin at the middle
               frequency.  We want to start at the lowest frequency, so start
               with a silent tone.  When we reach the low end of the sweep, we
               will start up the output amplitude.  To bring the low frequency
               around faster, use a negative ramp slope.  This will cause it
               to decrease from the middle, and we reach the low frequency
               before 500ms. */
            pCadence->regData[0] |= VP886_R_SIGAB_SLOPE_NEG;

            /* Generator D amplitude controls the actual output amplitude.
               Make it silent for now, to be changed to startLevel after the
               initial silent sweep. */
            sigCD[6] = 0x00;
            sigCD[7] = 0x01;
            
            /* Level Sweep is over a 36dB range. Max is 0x7FFF (by definition),
               so the minimum is 36dB less:
                (0x7FFF * 0.015849 = 519.32 (0x0207). */
            pCadence->startLevel = 0x0207;
            pCadence->stopLevel = 0x7FFF;

            /* The entire level sweep for UK Howler Tone is 12+/-2 seconds
               with an increase between each frequency sweep, so we need to make
               each level step 36dB/12 = 3dB ideally.  3dB gives a
               multiplication factor (10^(3/20)) of 1.4125.  In 1.15 format
               (1.4125 * 32768) this factor is 46286 (0xB4CE) */
            pCadence->levelStep = 0xB4CE;
            
            /* Program signal generators, enable bias+A+D */
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, pCadence->regData);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCD_WRT, VP886_R_SIGCD_LEN, sigCD);
            Vp886SetToneCtrl(pLineCtx, TRUE, TRUE, FALSE, FALSE, TRUE);
            
            /* By 490ms we should be at the low end of the sweep */
            pLineObj->howlerState = VP886_HOWLER_ST_ALIGNMENT;
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_HOWLER, 490, 0, 0);
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Initializing UK howler Issue 15 (ts %u)",
                Vp886GetTimestamp(pLineCtx->pDevCtx)));
            break;
        }

        case VP_CSLAC_UK_HOWLER_TONE_DRAFT_G: { /* UK Howler from BTNR 1080 */
            uint8 sigCD[VP886_R_SIGCD_LEN];
            /*  Draft 960-G: Frequency Range = [800Hz to 2.5kHz]
                     sweep middle = 1650 Hz
                     sweep range = +/-850 Hz
                     
                Use the frequency modulation mode of the signal generators,
                where the output of siggen A + bias determines the frequency of
                generator D.  Also use trapezoidal waveform for A, which
                changes the meaning of the register fields.
            */
            VpMemSet(sigCD, 0, VP886_R_SIGCD_LEN);
            sigCD[4] = VP886_R_SIGCD_FM_ENABLE;
            
            pCadence->regData[0] = VP886_R_SIGAB_WAVE_TRAP;
            
            /* Generator B frequency controls sweep rate
                FRQB = 8000 / Fsweep
               For a 1000ms (1Hz) cycle, FRQB = 0x1F40 */
            pCadence->regData[7] = 0x1F;
            pCadence->regData[8] = 0x40;

            /* Generator A frequency controls rise time
                FRQA = 2^15 / (Trise * 12000)
               We want a rise time of 500ms (half of the sweep) so that the
               tone changes continuously.  The closest values we can program
               are 546ms (0x0005) and 455ms (0x0006).  Using the slower rise
               time would cut the ramps short to 500ms, decreasing the
               amplitude.  Using the faster rise time would mean that we linger
               for a short time at the min and max frequencies.  It would be
               simpler to use the faster rise time, but to be more precise we
               will use the slower clipped rise time.  This will require
               adjustments to the amplitude and bias frequencies. */
            pCadence->regData[3] = 0x00;
            pCadence->regData[4] = 0x05;

            /* Generator A amplitude specifies the frequency sweep +/- range.
               We want a sweep of +/- 1200Hz, but whatever we program will be
               cut short at a factor of 500/546 because our ramp time (546ms) is
               more than half of the sweep period (1000ms).  To compensate, we
               multiply the range we want by 546/500 to get 928.2Hz */
            pCadence->regData[5] = 0x09;
            pCadence->regData[6] = 0xE7;

            /* The Bias parameter sets up the middle frequency of the sweep.
               The way the device handles cutting short the rise time, it does
               not keep the sweep centered.  It reaches the first full peak, but
               then is cut short afterward.  Since we will begin by ramping down
               from center, this means the lowest frequency will match up with
               the amplitude that we programmed but the high frequency will be
               much lower than what we programmed. To get both points to where
               we want them, we need to add the same offset to the bias that was
               added to the sweep range.  1650 + 78.2 = 1728.2Hz */
            pCadence->regData[1] = 0x12;
            pCadence->regData[2] = 0x6F;

            /* If we start the tone immediately, it will begin at the middle
               frequency.  We want to start at the lowest frequency, so start
               with a silent tone.  When we reach the low end of the sweep, we
               will start up the output amplitude.  To bring the low frequency
               around faster, use a negative ramp slope.  This will cause it
               to decrease from the middle, and we reach the low frequency
               before 500ms. */
            pCadence->regData[0] |= VP886_R_SIGAB_SLOPE_NEG;

            /* Generator D amplitude controls the actual output amplitude.
               Make it silent for now, to be changed to startLevel after the
               initial silent sweep. */
            sigCD[6] = 0x00;
            sigCD[7] = 0x01;
            
            /* Level Sweep is over a 36dB range. Max is 0x7FFF (by definition),
               so the minimum is 36dB less:
                (0x7FFF * 0.015849 = 519.32 (0x0207). */
            pCadence->startLevel = 0x0207;
            pCadence->stopLevel = 0x7FFF;

            /* The entire level sweep for UK Howler Tone is 12+/-2 seconds
               with an increase between each frequency sweep, so we need to make
               each level step 36dB/12 = 3dB ideally.  3dB gives a
               multiplication factor (10^(3/20)) of 1.4125.  In 1.15 format
               (1.4125 * 32768) this factor is 46286 (0xB4CE) */
            pCadence->levelStep = 0xB4CE;
            
            /* Program signal generators, enable bias+A+D */
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, pCadence->regData);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCD_WRT, VP886_R_SIGCD_LEN, sigCD);
            Vp886SetToneCtrl(pLineCtx, TRUE, TRUE, FALSE, FALSE, TRUE);
            
            /* By 490ms we should be at the low end of the sweep */
            pLineObj->howlerState = VP886_HOWLER_ST_ALIGNMENT;
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_HOWLER, 490, 0, 0);
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Initializing UK howler Draft 960-G (ts %u)",
                Vp886GetTimestamp(pLineCtx->pDevCtx)));
            break;
        }

        case VP_CSLAC_AUS_HOWLER_TONE: { /* UK Howler from BTNR 1080 */
            uint8 sigCD[VP886_R_SIGCD_LEN];
            /*  Frequency Range = [1500Hz to 3.2kHz]
                     sweep middle = 2350 Hz
                     sweep range = +/-850 Hz
                     
                Use the frequency modulation mode of the signal generators,
                where the output of siggen A + bias determines the frequency of
                generator D.  Also use trapezoidal waveform for A, which
                changes the meaning of the register fields.
            */
            VpMemSet(sigCD, 0, VP886_R_SIGCD_LEN);
            sigCD[4] = VP886_R_SIGCD_FM_ENABLE;
            
            pCadence->regData[0] = VP886_R_SIGAB_WAVE_TRAP;
            
            /* Generator B frequency controls sweep rate
                FRQB = 8000 / Fsweep
               For a 1000ms (1Hz) cycle, FRQB = 0x1F40 */
            pCadence->regData[7] = 0x1F;
            pCadence->regData[8] = 0x40;

            /* Generator A frequency controls rise time
                FRQA = 2^15 / (Trise * 12000)
               We want a rise time of 500ms (half of the sweep) so that the
               tone changes continuously.  The closest values we can program
               are 546ms (0x0005) and 455ms (0x0006).  Using the slower rise
               time would cut the ramps short to 500ms, decreasing the
               amplitude.  Using the faster rise time would mean that we linger
               for a short time at the min and max frequencies.  It would be
               simpler to use the faster rise time, but to be more precise we
               will use the slower clipped rise time.  This will require
               adjustments to the amplitude and bias frequencies. */
            pCadence->regData[3] = 0x00;
            pCadence->regData[4] = 0x05;

            /* Generator A amplitude specifies the frequency sweep +/- range.
               We want a sweep of +/- 1200Hz, but whatever we program will be
               cut short at a factor of 500/546 because our ramp time (546ms) is
               more than half of the sweep period (1000ms).  To compensate, we
               multiply the range we want by 546/500 to get 928.2Hz */
            pCadence->regData[5] = 0x09;
            pCadence->regData[6] = 0xE7;

            /* The Bias parameter sets up the middle frequency of the sweep.
               The way the device handles cutting short the rise time, it does
               not keep the sweep centered.  It reaches the first full peak, but
               then is cut short afterward.  Since we will begin by ramping down
               from center, this means the lowest frequency will match up with
               the amplitude that we programmed but the high frequency will be
               much lower than what we programmed. To get both points to where
               we want them, we need to add the same offset to the bias that was
               added to the sweep range.  2350 + 78.2 = 2428.2Hz */
            pCadence->regData[1] = 0x19;
            pCadence->regData[2] = 0xE7;

            /* If we start the tone immediately, it will begin at the middle
               frequency.  We want to start at the lowest frequency, so start
               with a silent tone.  When we reach the low end of the sweep, we
               will start up the output amplitude.  To bring the low frequency
               around faster, use a negative ramp slope.  This will cause it
               to decrease from the middle, and we reach the low frequency
               before 500ms. */
            pCadence->regData[0] |= VP886_R_SIGAB_SLOPE_NEG;

            /* Generator D amplitude controls the actual output amplitude.
               Make it silent for now, to be changed to startLevel after the
               initial silent sweep. */
            sigCD[6] = 0x00;
            sigCD[7] = 0x01;
            
            /* Level Sweep is over a 30dB range. Max is 0x7FFF (by definition),
               so the minimum is 30dB less:
                (0x7FFF * 0.0316227 = 1036.184 (0x040D). */
            pCadence->startLevel = 0x040D;
            pCadence->stopLevel = 0x7FFF;

            /* The entire level sweep for UK Howler Tone is 20+/-5 seconds
               with an increase between each frequency sweep, so we need to make
               each level step 30dB/20 = 1.5dB ideally.  1.5dB gives a
               multiplication factor (10^(1.5/20)) of 1.1885.  In 1.15 format
               (1.1885 * 32768) this factor is 38945 (0x9821) */
            pCadence->levelStep = 0x9821;
            
            /* Program signal generators, enable bias+A+D */
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, pCadence->regData);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCD_WRT, VP886_R_SIGCD_LEN, sigCD);
            Vp886SetToneCtrl(pLineCtx, TRUE, TRUE, FALSE, FALSE, TRUE);

            /* By 490ms we should be at the low end of the sweep */
            pLineObj->howlerState = VP886_HOWLER_ST_ALIGNMENT;
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_HOWLER, 490, 0, 0);
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Initializing AUS howler (ts %u)",
                Vp886GetTimestamp(pLineCtx->pDevCtx)));
            break;
        }

        case VP_CSLAC_NTT_HOWLER_TONE: { /* NTT Edition 5 */
            /* NTT Howler is specified as a single frequency (400Hz) with continuously increasing
               level over a period of [3-15 seconds] up to <= 36dBm and is output for 10 - 22
               seconds. This algorithm manages the frequency programming and level sweep portion
               only. It is up to the application to stop NTT Howler tone in <= 22 seconds in order
               to meet Edition 5 Requirements.
              
               The requirements mentioned above are shown in NTT Edition 5, Table 3.3.8 Electrical
               Conditions for Audible Tones sent by the Network. The requirement for the total level
               sweep is unclear. The API implements a nominal 30dB increase. */
            /* Using siggen D for this tone because it requires less traffic to
               the device.  Also, new silicon will allow zero-phase amplitude
               stepping on generator D for less audible glitches */
            VpMemSet(pCadence->regData, 0, VP886_R_SIGCD_LEN);
            
            /* Frequency is a constant 400Hz (0x0444) */
            pCadence->regData[4] = 0x04;
            pCadence->regData[5] = 0x44;
            
            /* Level Sweep is over a 30dB range. Max is 0x7FFF (by definition),
               so the minimum is 30dB less:
                (0x7FFF * 0.0316227 = 1036.184 (0x040D). */
            pCadence->startLevel = 0x040D;
            pCadence->stopLevel = 0x7FFF;
            pLineObj->howlerLevel = pCadence->startLevel;
            pCadence->regData[6] = (pLineObj->howlerLevel & 0xFF00) >> 8;
            pCadence->regData[7] = (pLineObj->howlerLevel & 0x00FF);
            
            /* We want to finish the ramp BEFORE 15 seconds, so we should aim
               to finish at 14.9 to avoid going over the spec limit.
               To ramp up by 30dB in 14.9 seconds, we must step by 0.20134dB
               every 100ms. 100ms is simply a convenient interval that is not
               too frequent, but often enough to not notice individual steps by
               ear. An 0.20134dB step translates to an amplitude multiplication
               factor (10^(0.20134/20)) of 1.02345.  In 1.15 format
               (1.02345 * 32768) this factor is 33536.  We round up to
               33537 (0x8301) to make sure we finish under 15s instead of over,
               because there will be some rounding errors in the fixed point
               math while stepping. */
            pCadence->levelStep = 0x8301;

            /* Program signal generator D */
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCD_WRT, VP886_R_SIGCD_LEN, pCadence->regData);
            /* Enable siggen D */
            Vp886SetToneCtrl(pLineCtx, FALSE, FALSE, FALSE, FALSE, TRUE);
            
            /* Step the level every 100ms */
            pLineObj->howlerState = VP886_HOWLER_ST_AMP_STEPPING_NTT;
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_HOWLER, 100, 0, 0);
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Starting NTT howler (ts %u)",
                Vp886GetTimestamp(pLineCtx->pDevCtx)));
            break;
        }
        
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Invalid howler tone type %d", pCadence->toneType));
            *pStatus = VP_STATUS_INVALID_ARG;
            return FALSE;
    }
    
    /* We're not actually using the generic sequencer, but set up the parameters
       so that Vp886CadenceStop() will work to stop the howler. */
    pCadence->pActiveCadence = pCadProfile;
    pCadence->toneType = toneType;
    pCadence->index = VP_PROFILE_TYPE_SEQUENCER_START;
    pCadence->pCurrentPos = &pCadProfile[VP_PROFILE_TYPE_SEQUENCER_START];
    pCadence->length = pCadProfile[VP_PROFILE_LENGTH];
    pCadence->status = VP_CADENCE_STATUS_ACTIVE;
    pCadence->branchDepth = 0;
    

#ifdef VP_HIGH_GAIN_MODE_SUPPORTED
    /* If high gain mode (howler line state) is active, adjust R and GR based
       on the tone type */
    if (pLineObj->inHighGainMode) {
        Vp886HighGainSetRFilter(pLineCtx);
    }
#endif /* VP_HIGH_GAIN_MODE_SUPPORTED */
    
    return TRUE;
}


/** Vp886HowlerToneHandler()
  Handles amplitude stepping for howler tones at the required intervals.
*/
void
Vp886HowlerToneHandler(
    VpLineCtxType *pLineCtx,
    uint32 overrun)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886SeqDataType *pCadence = &pLineObj->cadence;
    
    if (!(pCadence->status & VP_CADENCE_STATUS_ACTIVE)) {
        return;
    }

    switch (pCadence->toneType) {
        case VP_CSLAC_UK_HOWLER_TONE_VER15:
        case VP_CSLAC_UK_HOWLER_TONE_DRAFT_G:
        case VP_CSLAC_AUS_HOWLER_TONE:
        case VP_CSLAC_NTT_HOWLER_TONE:
            break;
        default:
            return;
    }

    switch (pLineObj->howlerState) {
        case VP886_HOWLER_ST_ALIGNMENT: {
            uint8 sigCD[VP886_R_SIGCD_LEN];
            VpMemSet(sigCD, 0, VP886_R_SIGCD_LEN);
            sigCD[4] = VP886_R_SIGCD_FM_ENABLE;

            /* Should be at the minimum frequency now, start up the output
               amplitude at startLevel. */
            pLineObj->howlerLevel = pCadence->startLevel;
            sigCD[6] = (pLineObj->howlerLevel & 0xFF00) >> 8;
            sigCD[7] = (pLineObj->howlerLevel & 0x00FF);

            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Starting howler, level 0x%04X (ts %u)",
                pLineObj->howlerLevel, Vp886GetTimestamp(pLineCtx->pDevCtx)));

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, pCadence->regData);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCD_WRT, VP886_R_SIGCD_LEN, sigCD);
            
            /* Start stepping the amplitude every 1 second */
            pLineObj->howlerState = VP886_HOWLER_ST_AMP_STEPPING_UK_AUS;
            Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_HOWLER, 1000, overrun, 0);
            break;
        }
        case VP886_HOWLER_ST_AMP_STEPPING_UK_AUS: {
            uint32 tempLevel;
            uint8 sigCD[VP886_R_SIGCD_LEN];
            VpMemSet(sigCD, 0, VP886_R_SIGCD_LEN);
            sigCD[4] = VP886_R_SIGCD_FM_ENABLE;

            /* Multiply the current level by the precalculated step factor */
            tempLevel = pLineObj->howlerLevel * pCadence->levelStep;
            tempLevel = (tempLevel + 0x4000) / 0x8000;
            
            /* Cap the amplitude */
            if (tempLevel >= pCadence->stopLevel) {
                tempLevel = pCadence->stopLevel;
            }
            
            pLineObj->howlerLevel = tempLevel;
            sigCD[6] = (pLineObj->howlerLevel & 0xFF00) >> 8;
            sigCD[7] = (pLineObj->howlerLevel & 0x00FF);
            
            /* Program signal generator D */
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCD_WRT, VP886_R_SIGCD_LEN, sigCD);
            
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Howler level 0x%04X (ts %u)",
                pLineObj->howlerLevel, Vp886GetTimestamp(pLineCtx->pDevCtx)));

            /* Schedule the next step if we haven't reached the max */
            if (pLineObj->howlerLevel < pCadence->stopLevel) {
                Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_HOWLER, 1000, overrun, 0);
            } else {
                VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Finished howler amplitude steps (ts %u)",
                    Vp886GetTimestamp(pLineCtx->pDevCtx)));
                Vp886CadenceStop(pLineCtx, FALSE, FALSE, FALSE);
            }
            break;
        }
        case VP886_HOWLER_ST_AMP_STEPPING_NTT: {
            uint32 tempLevel;

            /* Multiply the current level by the precalculated step factor */
            tempLevel = pLineObj->howlerLevel * pCadence->levelStep;
            tempLevel = (tempLevel + 0x4000) / 0x8000;
            
            /* Cap the amplitude */
            if (tempLevel >= pCadence->stopLevel) {
                tempLevel = pCadence->stopLevel;
            }
            
            pLineObj->howlerLevel = tempLevel;
            pCadence->regData[6] = (pLineObj->howlerLevel & 0xFF00) >> 8;
            pCadence->regData[7] = (pLineObj->howlerLevel & 0x00FF);
            
            /* Program signal generator D */
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCD_WRT, VP886_R_SIGCD_LEN, pCadence->regData);
            
            /* Schedule the next step if we haven't reached the max */
            if (pLineObj->howlerLevel < pCadence->stopLevel) {
                Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_HOWLER, 100, overrun, 0);
            } else {
                VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Finished howler amplitude steps (ts %u)",
                    Vp886GetTimestamp(pLineCtx->pDevCtx)));
                Vp886CadenceStop(pLineCtx, FALSE, FALSE, FALSE);
            }
            break;
        }
        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Invalid howler state %d.  How did we get here?", pLineObj->howlerState));
            return;
    }

    return;
}

#endif  /* VP_CSLAC_SEQ_EN */
#endif  /* VP_CC_886_SERIES */
