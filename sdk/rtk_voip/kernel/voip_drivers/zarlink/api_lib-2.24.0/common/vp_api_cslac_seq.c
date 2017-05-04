/** \file vp_api_cslac_seq.c
 * vp_api_cslac_seq.c
 *
 *  This file contains functions that are required to run the CSLAC sequencer.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 10725 $
 * $LastChangedDate: 2013-01-24 18:41:02 -0600 (Thu, 24 Jan 2013) $
 */

#include "vp_api_cfg.h"
#if (defined (VP_CC_880_SERIES) || defined (VP_CC_890_SERIES) || \
    defined (VP_CC_580_SERIES) || defined (VP_CC_790_SERIES)) && defined (VP_CSLAC_SEQ_EN)

#if defined(VP_CC_790_SERIES) || defined(VP_CC_880_SERIES) || \
    defined(VP_CC_890_SERIES) || defined(VP_CC_580_SERIES)


/* INCLUDES */
#include "vp_api.h"     /* Typedefs and function prototypes for API */
#include "vp_api_cslac_seq.h"
#include "vp_api_int.h" /* Device specific typedefs and function prototypes */
#include "sys_service.h"

#if defined (VP_CC_790_SERIES) || \
    (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) || \
    (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT))

static bool
VpFSKGeneratorReady(
    VpLineCtxType *pLineCtx);

static bool
VpDTMFGeneratorReady(
    VpLineCtxType *pLineCtx);

static VpStatusType
VpCtrlSetCliTone(
    VpLineCtxType *pLineCtx,
    bool mode);

static VpCliEncodedDataType
VpCliGetEncodedByte(
    VpLineCtxType *pLineCtx,
    uint8 *pByte);

static bool
VpCtrlSetFSKGen(
    VpLineCtxType *pLineCtx,
    VpCidGeneratorControlType mode,
    uint8 digit);

static VpDigitType
VpConvertCharToDigitType(
    char digit);

static void
VpCtrlSetDTMFGen(
    VpLineCtxType *pLineCtx,
    VpCidGeneratorControlType mode,
    VpDigitType digit);

static VpStatusType
VpCtrlDetectDTMF(
    VpLineCtxType *pLineCtx,
    bool mode);
#endif

#if defined(VP_CC_790_SERIES) || (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT))
static VpStatusType
AddMeteringSection(
    VpLineCtxType *pLineCtx,
    uint8 *pIntSequence,
    uint8 *pIndex,
    uint16 tickRate,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters);

#endif

#if (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) || \
    (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT))
static void
VpCSLACComputeHowlerFreqStep(
    uint32 sweepInterval,
    VpSeqDataType *cadence,
    uint16 updateInterval);
#endif

static void
VpCtrlMuteChannel(
    VpLineCtxType *pLineCtx,
    bool mode);

/**
 * VpSeq()
 *  This function calls the appropriate sequencer function based on the current
 * position in the sequencer and the device type.
 *
 * Preconditions:
 *  The profile passed must be pointing to an instruction that is supported by
 * the device type.
 *
 * Postconditions:
 *  The instruction specified by the profile data is called.  The line context
 * is passed to the called function.  Note:  The line context may be valid or
 * VP_NULL, this function is not affected.  This function returns the success
 * code as long as the pointer is pointing to an instruction that is supported
 * by the device.
 */
VpStatusType
VpSeq(
    VpLineCtxType *pLineCtx,    /**< Line that has an active sequencer */
    VpProfilePtrType pProfile)  /**< Sequence profile, pointing to current
                                 * location in sequence, not typically the
                                 * starting address
                                 */
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;

    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("+VpSeq()"));

    /*
     * This function is passed a pointer that starts at the current position of
     * the cadence sequence, controlled by the API
     */
    if (pProfile == VP_NULL) {
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("NULL Sequence Profile"));
        return VP_STATUS_INVALID_ARG;
    }

    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Sequence Command 0x%02X 0x%02X",
        pProfile[0], pProfile[1]));

    switch(pProfile[0] & VP_SEQ_OPERATOR_MASK) {
        case VP_SEQ_SPRCMD_COMMAND_INSTRUCTION:
            switch(pDevCtx->deviceType) {
#if defined (VP_CC_890_SERIES)
                case VP_DEV_890_SERIES:
                    return Vp890CommandInstruction(pLineCtx, pProfile);
#endif

#if defined (VP_CC_880_SERIES)
                case VP_DEV_880_SERIES:
                    return Vp880CommandInstruction(pLineCtx, pProfile);
#endif

#if defined (VP_CC_790_SERIES)
                case VP_DEV_790_SERIES:
                    return Vp790CommandInstruction(pLineCtx, pProfile);
#endif

#if defined (VP_CC_580_SERIES)
                case VP_DEV_580_SERIES:
                    return Vp580CommandInstruction(pLineCtx, pProfile);
#endif

                default:
                    return VP_STATUS_INVALID_ARG;
            }

        case VP_SEQ_SPRCMD_TIME_INSTRUCTION:
            return VpTimeInstruction(pLineCtx, pProfile);

        case VP_SEQ_SPRCMD_BRANCH_INSTRUCTION:
            return VpBranchInstruction(pLineCtx, pProfile);

        default:
            return VP_STATUS_INVALID_ARG;
    }
} /* VpSeq() */

/**
 * VpServiceSeq()
 *  This function tests the line status for an active sequence, and calls the
 * VpSeq function if there is an active sequence.  However, this function does
 * not check to see if the operation being pointed to by the current sequence is
 * supported.
 *
 * Preconditions:
 *  The device context cannot be VP_NULL and only CSLAC devices supported.
 *
 * Postconditions:
 *  If there is an active cadence that is supported by the line, then it is
 * called (via VpSeq).  If the current operation is not supported, the line
 * object active cadence is removed (i.e., set to inactive).
 */
bool
VpServiceSeq(
    VpDevCtxType *pDevCtx)  /**< Device that has a sequence.  The sequence may
                             * not be active
                             */
{
    uint8 channelId, maxChannels;
    VpDeviceInfoType deviceInfo;
    VpLineCtxType *pLineCtx;
    void *pLineObj;

    /*
     * pCadence is initialized to VP_NULL to remove compiler warnings
     * (.. pCadence might be used uninitialized..), but this function is called
     * only from API functions for devices that require cadence support.
     * Therefore, pCadence is initialized by the line object association below
     */
    VpSeqDataType *pCadence = VP_NULL;

    deviceInfo.pDevCtx = pDevCtx;
    deviceInfo.pLineCtx = VP_NULL;
    VpGetDeviceInfo(&deviceInfo);

    maxChannels = deviceInfo.numLines;

#if defined (VP_CC_890_SERIES)
    if (pDevCtx->deviceType == VP_DEV_890_SERIES) {
        /*
         * Override the maxChannel value from Get Device Info
         * because the physical looping needs to go from 0:1, even
         * if the device is only a single line (w/channel = 1).
         */
        maxChannels = VP890_MAX_NUM_CHANNELS;
    }
#endif

    for (channelId = 0; channelId < maxChannels; channelId++) {
        pLineCtx = pDevCtx->pLineCtx[channelId];

        if (pLineCtx != VP_NULL) {
            pLineObj = pLineCtx->pLineObj;

            switch(pDevCtx->deviceType) {
#if defined (VP_CC_890_SERIES)
                case VP_DEV_890_SERIES:
                    pCadence = &((Vp890LineObjectType *)pLineObj)->cadence;
                    break;
#endif
#if defined (VP_CC_880_SERIES)
                case VP_DEV_880_SERIES:
                    pCadence = &((Vp880LineObjectType *)pLineObj)->cadence;
                    break;
#endif
#if defined (VP_CC_790_SERIES)
                case VP_DEV_790_SERIES:
                    pCadence = &((Vp790LineObjectType *)pLineObj)->cadence;
                    break;
#endif

#if defined (VP_CC_580_SERIES)
                case VP_DEV_580_SERIES:
                    pCadence = &((Vp580LineObjectType *)pLineObj)->cadence;
                    break;
#endif

                default:
                    return FALSE;
            }

            if((pCadence->status & VP_CADENCE_STATUS_ACTIVE) == VP_CADENCE_STATUS_ACTIVE ) {
                VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Line Object 0x%04X on Channel %d",
                    pCadence->status, channelId));

                if(VpSeq(pLineCtx, pCadence->pCurrentPos) != VP_STATUS_SUCCESS) {
                    pCadence->status &= ~VP_CADENCE_STATUS_ACTIVE;
                    pCadence->pActiveCadence = VP_PTABLE_NULL;
                }
            }
        }
    }

    return TRUE;
} /* VpServiceSeq() */

/**
 * VpBranchInstruction()
 *  This function implements the Sequencer Branch instruction for the CSLAC
 * device types.
 *
 * Preconditions:
 *  The line must first be initialized and the sequencer data must be valid.
 *
 * Postconditions:
 *  The branch count is either set if this is the first time for this branch, or
 * the branch count is decremented if this branch instruction has been executed
 * before.  If the branch count is decremented to 0, the sequencer index is
 * increased.  If the branch count is 0 at the first time the particular branch
 * is executed, the branch is repeated forever.  If the branch count is not 0,
 * the sequencer is set back to the instruction specified in the profile.  This
 * function can only return VP_SUCCESS since any valid combination of "branch
 * to" and "branch count" is valid.
 */
VpStatusType
VpBranchInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;
    VpSeqDataType *pCadence;
    VpOptionEventMaskType *pLineEvents;
    uint8 length, index;
    uint16 *pEventData;
    VpLineStateType lineState;
    uint8 branchDepth;

    void *pLineObj = pLineCtx->pLineObj;
    void *pDevObj = pDevCtx->pDevObj;
    uint8 *pIntSeqType;
#if (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER)
    uint16 timeStamp = 0;
#endif

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            if (!(((Vp790DeviceObjectType *)pDevObj)->status.state & VP_DEV_INIT_CMP)) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }
            pCadence = &((Vp790LineObjectType *)pLineObj)->cadence;
            pLineEvents = &((Vp790LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp790LineObjectType *)pLineObj)->processData;
            lineState = ((Vp790LineObjectType *)pLineObj)->lineState.usrCurrent;
            pIntSeqType =  &((Vp790LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
#if (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER)
            timeStamp = ((Vp790DeviceObjectType *)pDevObj)->timeStamp;
#endif
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            if (!(((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_INIT_CMP)) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            pCadence = &((Vp880LineObjectType *)pLineObj)->cadence;
            pLineEvents = &((Vp880LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp880LineObjectType *)pLineObj)->processData;
            lineState = ((Vp880LineObjectType *)pLineObj)->lineState.usrCurrent;
            pIntSeqType =  &((Vp880LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
#if (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER)
            timeStamp = ((Vp880DeviceObjectType *)pDevObj)->timeStamp;
#endif
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            if (!(((Vp890DeviceObjectType *)pDevObj)->state & VP_DEV_INIT_CMP)) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp890DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            pCadence = &((Vp890LineObjectType *)pLineObj)->cadence;
            pLineEvents = &((Vp890LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp890LineObjectType *)pLineObj)->processData;
            lineState = ((Vp890LineObjectType *)pLineObj)->lineState.usrCurrent;
            pIntSeqType =  &((Vp890LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
#if (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER)
            timeStamp = ((Vp890DeviceObjectType *)pDevObj)->timeStamp;
#endif
            break;
#endif


#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES:
            if (!(((Vp580DeviceObjectType *)pDevObj)->status.state & VP_DEV_INIT_CMP)) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }
            pCadence = &((Vp580LineObjectType *)pLineObj)->cadence;
            pLineEvents = &((Vp580LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp580LineObjectType *)pLineObj)->processData;
            lineState = ((Vp580LineObjectType *)pLineObj)->lineState.usrCurrent;
            pIntSeqType =  &((Vp580LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
#if (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER)
            timeStamp = ((Vp580DeviceObjectType *)pDevObj)->timeStamp;
#endif
            break;
#endif
        default:
            return VP_STATUS_INVALID_ARG;
    }

    length = pCadence->length;
    index = pCadence->index;

    if (pCadence->status & VP_CADENCE_STATUS_BRANCHING) {
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Branching Length %d Index %d At %d Device Time %d",
            length, index, pCadence->branchAt, timeStamp));

        /*
         * We're already branching, but possibly at a step after this point. In
         * other words, we may have been branched back to a branch step
         * Determine if we are repeating this step, or if we are being branched
         * back from a later step
         */
        if (index < pCadence->branchAt) {
             /*
              * We're at an earlier step in the branch loop, so use the second
              * set of branch timers
              */
             branchDepth = VP_CSLAC_BRANCH_LVL_1;

             if (!(pCadence->status & VP_CADENCE_STATUS_BRANCHING_LVL2)) {
                 pCadence->status |= VP_CADENCE_STATUS_BRANCHING_LVL2;
                 pCadence->count[branchDepth] = pSeqData[1];
             }
        } else {
            /* This is a continuation from this branch */
            branchDepth = VP_CSLAC_BRANCH_LVL_0;
        }

        if (pCadence->count[branchDepth] > 0) {
            /*
             * If the repeat value set in the profile is = 0, this means repeat
             * forever.  Therefore, don't decrement the actual count value
             */
            if (pSeqData[1] != 0) {
                pCadence->count[branchDepth]--;
            }

            /* Send the profile pointer back to the branch location */
            /* Account for header offset */
            pCadence->index = (((pSeqData[0] & 0x1F) * 2)
                + VP_PROFILE_TYPE_SEQUENCER_START);
            pCadence->pCurrentPos =
                &(pCadence->pActiveCadence[pCadence->index]);
        } else {
            /*
             * We don't need to repeat this branch.  Just see if the profile is
             * complete
             */

            index+=2;
            if (index < (length + VP_PROFILE_LENGTH + 1)) {
                pCadence->index = index;
                pCadence->pCurrentPos+=2;
                if (pCadence->status & VP_CADENCE_STATUS_BRANCHING_LVL2) {
                    pCadence->status &= ~VP_CADENCE_STATUS_BRANCHING_LVL2;
                } else {
                    pCadence->status &= ~VP_CADENCE_STATUS_BRANCHING;
                }
            } else {  /* The profile is complete. */
                switch(pCadence->pActiveCadence[VP_PROFILE_TYPE_LSB]) {
                    case VP_PRFWZ_PROFILE_METERING_GEN:
                        pLineEvents->process |= VP_LINE_EVID_MTR_CMP;
                        break;

                    case VP_PRFWZ_PROFILE_RINGCAD:
                        pLineEvents->process |= VP_LINE_EVID_RING_CAD;
                        *pEventData = VP_RING_CAD_DONE;
                        break;

                    case VP_PRFWZ_PROFILE_TONECAD:
                        pLineEvents->process |= VP_LINE_EVID_TONE_CAD;
                        break;

                    case VP_PRFWZ_PROFILE_HOOK_FLASH_DIG_GEN:
                        pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                        *pEventData = VP_SENDSIG_HOOK_FLASH;
                        break;

                    case VP_PRFWZ_PROFILE_DIAL_PULSE_DIG_GEN:
                        pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                        *pEventData = VP_SENDSIG_PULSE_DIGIT;
                        break;

                    case VP_PRFWZ_PROFILE_DTMF_DIG_GEN:
                        pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                        *pEventData = VP_SENDSIG_DTMF_DIGIT;
                        VpCtrlMuteChannel(pLineCtx, FALSE);
                        break;

                    case VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT:
                        pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                        *pEventData = VP_SENDSIG_MSG_WAIT_PULSE;
                        VpSetLineState(pLineCtx, lineState);
                        *pIntSeqType = 0;
                        break;

                    default:
                        break;

                }
                pCadence->status = VP_CADENCE_RESET_VALUE;
            }
        }
    } else {
        /*
         * We are not branching, so this is the first branching loop. Set the
         * parameter to indicate this step is branching.
         */
        branchDepth = VP_CSLAC_BRANCH_LVL_0;
        pCadence->count[branchDepth] = pSeqData[1];
        pCadence->branchAt = index;

        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Branch To %d, Count %d",
            pCadence->branchAt, pCadence->count[branchDepth]));

        if(pCadence->count[branchDepth] == 0) {
            /*
             * This means branch forever.  Implement by setting to max value
             * here, and not decreasing in steps above
             */
            pCadence->count[branchDepth] = 0xFF;
        } else {
            /* Repeat already (following lines) */
            pCadence->count[branchDepth]--;
        }
        /* Account for header offset */
        pCadence->index =
            (((pSeqData[0] & 0x1F) * 2) + VP_PROFILE_TYPE_SEQUENCER_START);
        pCadence->pCurrentPos = &(pCadence->pActiveCadence[pCadence->index]);
        pCadence->status |= VP_CADENCE_STATUS_BRANCHING;
    }

    /* If we've disabled the cadence, clear the active cadence pointer */
    if (!(pCadence->status & VP_CADENCE_STATUS_ACTIVE)) {
        pCadence->pActiveCadence = VP_PTABLE_NULL;
    }

    return VP_STATUS_SUCCESS;
}

/**
 * VpTimeInstruction()
 *  This function implements the Sequencer Time instruction for the CSLAC device
 * types.
 *
 * Preconditions:
 *  The line must first be initialized and the sequencer data must be valid.
 *
 * Postconditions:
 *  The timer is decremented and when it decreases to 0, the pointer in the
 * sequence profile (passed) is updated to the next command past the time
 * operator currently being executed. If there are no more operators, then
 * the cadence is stopped.
 */
VpStatusType
VpTimeInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;
    VpSeqDataType *pCadence;

#if defined (VP_CC_790_SERIES) \
|| (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) \
|| (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT))
    VpCallerIdType *pCid = VP_NULL;
    VpLineStateType lineState = VP_LINE_DISCONNECT;
#endif

    VpOptionEventMaskType *pLineEvents;
    uint16 *pEventData;
    uint16 tickRate;
    bool forever = FALSE;

    void *pLineObj = pLineCtx->pLineObj;
    void *pDevObj = pDevCtx->pDevObj;
    uint8 *pIntSeqType;
    uint16 msInTick;

    /* Time in sequence is in 5mS incremements.  We need to convert to TICKS */
    uint16 timeInSeq = ( (( (uint16)pSeqData[0] & 0x1F) << 8) | (uint16)pSeqData[1]);

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            if (!(((Vp790DeviceObjectType *)pDevObj)->status.state & VP_DEV_INIT_CMP)) {

                return VP_STATUS_DEV_NOT_INITIALIZED;
            }
            pCadence = &((Vp790LineObjectType *)pLineObj)->cadence;
            tickRate =
                ((Vp790DeviceObjectType *)pDevObj)->devProfileData.tickRate;
            pCid = &((Vp790LineObjectType *)pLineObj)->callerId;
            pLineEvents = &((Vp790LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp790LineObjectType *)pLineObj)->processData;
            lineState = ((Vp790LineObjectType *)pLineObj)->lineState.usrCurrent;
            pIntSeqType =  &((Vp790LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            if (!(((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_INIT_CMP)) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            pCadence = &((Vp880LineObjectType *)pLineObj)->cadence;
            tickRate =
                ((Vp880DeviceObjectType *)pDevObj)->devProfileData.tickRate;
#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
            pCid = &((Vp880LineObjectType *)pLineObj)->callerId;
            lineState = ((Vp880LineObjectType *)pLineObj)->lineState.usrCurrent;
#endif
            pLineEvents = &((Vp880LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp880LineObjectType *)pLineObj)->processData;
            pIntSeqType =  &((Vp880LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            if (!(((Vp890DeviceObjectType *)pDevObj)->state & VP_DEV_INIT_CMP)) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp890DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            pCadence = &((Vp890LineObjectType *)pLineObj)->cadence;
            tickRate =
                ((Vp890DeviceObjectType *)pDevObj)->devProfileData.tickRate;
#if defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)
            pCid = &((Vp890LineObjectType *)pLineObj)->callerId;
            lineState = ((Vp890LineObjectType *)pLineObj)->lineState.usrCurrent;
#endif
            pLineEvents = &((Vp890LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp890LineObjectType *)pLineObj)->processData;
            pIntSeqType =  &((Vp890LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
            break;
#endif

#if defined (VP_CC_580_SERIES)
        case VP_DEV_580_SERIES:
            if (!(((Vp580DeviceObjectType *)pDevObj)->status.state & VP_DEV_INIT_CMP)) {
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }
            pCadence = &((Vp580LineObjectType *)pLineObj)->cadence;
            tickRate =
                ((Vp580DeviceObjectType *)pDevObj)->devProfileData.tickRate;
            pLineEvents = &((Vp580LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp580LineObjectType *)pLineObj)->processData;
            pIntSeqType =  &((Vp580LineObjectType *)pLineObj)->intSequence[VP_PROFILE_TYPE_LSB];
            break;
#endif
        default:
            return VP_STATUS_INVALID_ARG;
    }

    VP_SEQUENCER(VpLineCtxType, pLineCtx,
        ("Time Operator: Pre-Processed Time Remain: %d from Starting Time %d",
        pCadence->timeRemain, timeInSeq));

    if (pCadence->status & VP_CADENCE_STATUS_MID_TIMER) {
        if (pCadence->timeRemain) {
            pCadence->timeRemain--;
            VP_SEQUENCER(VpLineCtxType, pLineCtx,
                ("Time Operator: Decreasing time remain to %d", pCadence->timeRemain));
        }
    } else {
        /*
         * This operation truncates times rather than rounds them off. Algorithms that use this
         * timer need to take that into account.
         */
        pCadence->status |= VP_CADENCE_STATUS_MID_TIMER;
        pCadence->timeRemain = MS_TO_TICKRATE((timeInSeq * 5), tickRate);

        VP_SEQUENCER(VpLineCtxType, pLineCtx,
            ("Time Operator: Conversion MS_TO_TICKRATE Time Remain: %d from Starting Time %d",
            pCadence->timeRemain, timeInSeq));

        if (pCadence->timeRemain == 0) {
            /* Always is selected.  End the cadence and leave the state as is */
            pCadence->status = VP_CADENCE_RESET_VALUE;
            forever = TRUE;
        } else {
            /*
             * Find out how long in ms 1 "tick" is, then subtract that amount
             * from the time required in the cadence. Lower limit 1 tick.
             */

            msInTick = TICKS_TO_MS(1, tickRate);
            VP_SEQUENCER(VpLineCtxType, pLineCtx,
                ("Time Operator: msInTick (%d) based on tickRate (%d)", msInTick, tickRate));

            /*
             * If the time specified in the sequence can be executed with at
             * least one tick, then subtract one "tick" worth of time
             */
            if ((timeInSeq * 5) >= msInTick) {
                pCadence->timeRemain =
                    MS_TO_TICKRATE(((timeInSeq * 5) - msInTick), tickRate);
                VP_SEQUENCER(VpLineCtxType, pLineCtx,
                    ("Time Operator: Adjusting timRemain to (%d)", pCadence->timeRemain));
            }
        }
    }
    VP_SEQUENCER(VpLineCtxType, pLineCtx,
        ("Time Operator: Post-Processed Time Remain: %d from Starting Time %d",
        pCadence->timeRemain, timeInSeq));

    /* If the time is over, move on to the next sequence if there is one */
    if (pCadence->timeRemain == 0) {
        pCadence->index+=2;

        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("1. Time Operator Increment Index to %d",
            pCadence->index));

        if (pCadence->index < (pCadence->length + VP_PROFILE_LENGTH + 1)) {
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Time Operator -- Current Data 0x%02X 0x%02X pCad 0x%02X 0x%02X",
                pSeqData[0], pSeqData[1], pCadence->pCurrentPos[0], pCadence->pCurrentPos[1]));
            pSeqData+=2;
            pCadence->pCurrentPos = pSeqData;
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Time Operator -- Next Data 0x%02X 0x%02X pCad 0x%02X 0x%02X",
                pSeqData[0], pSeqData[1], pCadence->pCurrentPos[0], pCadence->pCurrentPos[1]));

        } else {  /* The profile is complete. */
            switch(pCadence->pActiveCadence[VP_PROFILE_TYPE_LSB]) {
                case VP_PRFWZ_PROFILE_METERING_GEN:
                    pLineEvents->process |= VP_LINE_EVID_MTR_CMP;
                    break;

                case VP_PRFWZ_PROFILE_RINGCAD:
                    if (forever == FALSE) {
                        pLineEvents->process |= VP_LINE_EVID_RING_CAD;
                        *pEventData = VP_RING_CAD_DONE;
                    }
                    break;

                case VP_PRFWZ_PROFILE_TONECAD:
                    pLineEvents->process |= VP_LINE_EVID_TONE_CAD;
                    break;

                case VP_PRFWZ_PROFILE_HOOK_FLASH_DIG_GEN:
                    pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                    *pEventData = VP_SENDSIG_HOOK_FLASH;
                    break;

                case VP_PRFWZ_PROFILE_DIAL_PULSE_DIG_GEN:
                    pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                    *pEventData = VP_SENDSIG_PULSE_DIGIT;
                    break;

                case VP_PRFWZ_PROFILE_DTMF_DIG_GEN:
                    pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                    *pEventData = VP_SENDSIG_DTMF_DIGIT;
                    VpCtrlMuteChannel(pLineCtx, FALSE);
                    break;

                case VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT:
                    pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                    *pEventData = VP_SENDSIG_MSG_WAIT_PULSE;
                    *pIntSeqType = 0;
                    break;

                case VP_PRFWZ_PROFILE_FWD_DISC_INT:
                    pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                    *pEventData = VP_SENDSIG_FWD_DISCONNECT;
                    *pIntSeqType = 0;
                    break;

                case VP_PRFWZ_PROFILE_TIP_OPEN_INT:
                    pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                    *pEventData = VP_SENDSIG_TIP_OPEN_PULSE;
                    *pIntSeqType = 0;
                    break;

                case VP_PRFWZ_PROFILE_POLREV_PULSE_INT:
                    pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                    *pEventData = VP_SENDSIG_POLREV_PULSE;
                    *pIntSeqType = 0;
                    break;

                default:
                    break;

            }
            pCadence->status = VP_CADENCE_RESET_VALUE;
        }
        pCadence->status &= ~VP_CADENCE_STATUS_MID_TIMER;
    } else {
        /* Check to see if we're in the middle of a Wait on function. If so,
         * check to see if we still need to wait on CID (only supported wait
         * on operator). If CID is complete, terminate the timer function.
         * If not, continue..
         */
#if defined (VP_CC_790_SERIES) \
|| (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) \
|| (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT))

        if ((pCid != VP_NULL) && (pCid->status & VP_CID_WAIT_ON_ACTIVE)) {
            VP_CID(VpLineCtxType, pLineCtx, ("CID Status 0x%08lX", pCid->status));
            if (pCid->status & VP_CID_IN_PROGRESS) {
                /* Do nothing */
                VP_CID(VpLineCtxType, pLineCtx, ("CID In Progress"));
            } else {
                VP_CID(VpLineCtxType, pLineCtx, ("Terminating Timer"));

                /* Terminate this timer operation and the wait on */
                pCid->status &= ~ VP_CID_WAIT_ON_ACTIVE;
                pCadence->status &= ~VP_CADENCE_STATUS_MID_TIMER;
                pCadence->timeRemain = 0;
                VP_SEQUENCER(VpLineCtxType, pLineCtx, ("2. Time Operator Increment Index to %d",
                    pCadence->index));

                pCadence->index+=2;

                if (pCadence->index <
                    (pCadence->length + VP_PROFILE_LENGTH + 1)) {
                    pSeqData+=2;
                    pCadence->pCurrentPos = pSeqData;
                } else {  /* The profile is complete. */
                    switch(pCadence->pActiveCadence[VP_PROFILE_TYPE_LSB]) {
                        case VP_PRFWZ_PROFILE_METERING_GEN:
                            pLineEvents->process |= VP_LINE_EVID_MTR_CMP;
                            break;

                        case VP_PRFWZ_PROFILE_RINGCAD:
                            pLineEvents->process |= VP_LINE_EVID_RING_CAD;
                            *pEventData = VP_RING_CAD_DONE;
                            break;

                        case VP_PRFWZ_PROFILE_TONECAD:
                            pLineEvents->process |= VP_LINE_EVID_TONE_CAD;
                            break;

                        case VP_PRFWZ_PROFILE_HOOK_FLASH_DIG_GEN:
                            pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                            *pEventData = VP_SENDSIG_HOOK_FLASH;
                            break;

                        case VP_PRFWZ_PROFILE_DIAL_PULSE_DIG_GEN:
                            pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                            *pEventData = VP_SENDSIG_PULSE_DIGIT;
                            break;

                        case VP_PRFWZ_PROFILE_DTMF_DIG_GEN:
                            pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                            *pEventData = VP_SENDSIG_DTMF_DIGIT;
                            VpCtrlMuteChannel(pLineCtx, FALSE);
                            break;

                        case VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT:
                            pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                            *pEventData = VP_SENDSIG_MSG_WAIT_PULSE;
                            VpSetLineState(pLineCtx, lineState);
                            *pIntSeqType = 0;
                            break;

                        case VP_PRFWZ_PROFILE_FWD_DISC_INT:
                            pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                            *pEventData = VP_SENDSIG_FWD_DISCONNECT;
                            *pIntSeqType = 0;
                            break;

                        case VP_PRFWZ_PROFILE_TIP_OPEN_INT:
                            pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                            *pEventData = VP_SENDSIG_TIP_OPEN_PULSE;
                            *pIntSeqType = 0;
                            break;

                        case VP_PRFWZ_PROFILE_POLREV_PULSE_INT:
                            pLineEvents->process |= VP_LINE_EVID_SIGNAL_CMP;
                            *pEventData = VP_SENDSIG_POLREV_PULSE;
                            *pIntSeqType = 0;
                            break;

                        default:
                            break;

                    }
                    pCadence->status = VP_CADENCE_RESET_VALUE;
                }
            }
        }
#endif
    }
    /* If we've disabled the cadence, clear the active cadence pointer */
    if (!(pCadence->status & VP_CADENCE_STATUS_ACTIVE)) {
        pCadence->pActiveCadence = VP_PTABLE_NULL;
    }
    return VP_STATUS_SUCCESS;
}

#if (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)) || \
    (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT))
/**
 * VpCSLACHowlerInit()
 *  This function fills in the cadence structure for special howler tones. Special Howler Tones
 *  are those howler tones which require frequency, amplitude or both increasing or decreasing
 *  during the cadencing operation. Supporting these in the VP-API-II may require updates to the
 *  Signal Generators at some regular interval. The Special Howler Tones supported by this function
 *  are:
 *
 *      #define VP_CSLAC_SPECIAL_TONE_MASK          (0x1C)
 *      #define VP_CSLAC_HOWLER_TONE                (0x04)  UK Howler from BTNR 1080, Version 15
 *      #define VP_CSLAC_UK_HOWLER_TONE_VER15       VP_CSLAC_HOWLER_TONE
 *      #define VP_CSLAC_AUS_HOWLER_TONE            (0x08)
 *      #define VP_CSLAC_NTT_HOWLER_TONE            (0x0C)  NTT Edition 5
 *      #define VP_CSLAC_UK_HOWLER_TONE_DRAFT_G:    (0x10)  UK Howler from BTNR 1080, Draft 960-G
 *
 *  The Profile Wizard generates a special tone cadence when one of these howler tone types are
 *  required. A bit-field indicates the type of tone, but more important is the cadence is as
 *  follows:
 *
 *      0. Generator On
 *      1. Generator Ramp
 *      2. Delay 10ms
 *      3. Repeat Forever from step 1.
 *
 *  This creates an updated interval of 10ms + 2 x tickrate, where the tick duration occurs
 *  between step 2-3, and step 3-1. The "Repeat" operation only adjusts the cadence pointers, it
 *  does not perform the next operation until the next tick. To maintain precise sweep interval,
 *  the step values need to account for the tickrate.
 *
 * Preconditions:
 *  The calling device specific function should pre-mask the PW output and check if the result
 *  is one that is supported prior to calling this function. The input to this function that
 *  specifies the Special Howler Tone required MUST be exact.
 *
 * Postconditions:
 *  The cadence structure is filled with start, stop, and step information required to implement
 *  the special howler cadence passed. The Howler Tone itself is started outside this function.
 */
bool
VpCSLACHowlerInit(
    VpSeqDataType *cadence, /**< I/O:
                             *   INPUT: The type of Howler Tone being performed.
                             *   OUTPUT: Frequency and Level Paramaters necessary to perform the
                             *           the specified Howler tone assuming a specific cadence
                             *           profile format
                             */
    uint16 tickRate)        /**< INPUT: Used to determine step sizes since the adjustment intervals
                             *          are a function of the tickrate.
                             */
{
    bool returnValue = TRUE;
    uint16 updateInterval = (2 * tickRate);  /* In scale of the device profile */
    uint16 tickCount = MS_TO_TICKRATE(10, tickRate);

    if (tickRate > 0xA00) {
        updateInterval += (tickRate * 2 * tickCount);
    } else {
        updateInterval += (tickRate * tickCount);
    }

    VP_SEQUENCER(None, VP_NULL,
        ("VpCSLACHowlerInit(): TickCount %d Update Interval 0x%04X", tickCount, updateInterval));

    /*
     * "levelStep" is a bit-mask of add/shift to the previous level in 1.15 format.
     * +3dB per step is 1.414215 mutltiple which works out to 0xB505. Values < 0x8000 are
     * level reductions and will be implemented as subtract/shift.
     */
    switch(cadence->toneType) {
        case VP_CSLAC_UK_HOWLER_TONE_VER15:     /* UK Howler from BTNR 1080, Version 15 */
        case VP_CSLAC_UK_HOWLER_TONE_DRAFT_G:   /* UK Howler from BTNR 1080, Draft 960-G */
            /*
             * Frequency Sweep Rate = 1 second period (500ms each sweep direction)
             *
             *  Issue 15: Frequency Range = [800Hz to 3.2kHz]
             *       startFreq = 0x0888 (800Hz)
             *       stopFreq =  0x2222 (3200Hz)
             *       flat frequency response between 800Hz to 3200Hz
             *
             *  Draft 960-G: Frequency Range = [800Hz to 2.5kHz]
             *       startFreq = 0x0888 (800Hz)
             *       stopFreq =  0x1AA9 (2500Hz)
             *       Also, level change 3dB from 800Hz to 2.5KHz
             */
             cadence->startFreq = 0x0888;

            /*
             * Earlier versions supported a compile-time option only to select UK Holwer Tone Type.
             * For backwared compatibility until the compile-time selection is deprecated, the logic
             * to determine the UK Howler Type the customer is choosing should be by compile-value
             * first priority, then the profile. Something like:
             *
             *      if (compile setting = Version15) {  // Default setting
             *          - Let the Tone Cadence Value Override
             *      } else {    // Compile Setting was changed by user (Draft 960-G selected).
             *                  // Therefore, we should assume the user wants their compile-time
             *                  // setting to be used.
             *          Override profile setting. Provide Draft 960-G Type - compile-time value.
             *      }
             */

#if (VP_UK_HOWLER_IN_USE == VP_UK_HOWLER_BTNR_VER15)
            /* This is the default setting. Use what the profile has. */
            if (cadence->toneType == VP_CSLAC_UK_HOWLER_TONE_DRAFT_G) {
                /* Since Draft-G is the non-default value, make 'IT' the non-default case. */
                VP_SEQUENCER(None, VP_NULL,
                    ("\\********** VP_CSLAC_HOWLER_TONE - VP_UK_HOWLER_BTNR_DRAFT_G **********\\"));
                cadence->stopFreq = 0x1AA9;
            } else {    /* VP_CSLAC_UK_HOWLER_TONE_VER15 */
                VP_SEQUENCER(None, VP_NULL,
                    ("\\********** VP_CSLAC_HOWLER_TONE - VP_UK_HOWLER_BTNR_VER15 **********\\"));
                cadence->stopFreq = 0x2222;
            }
#elif (VP_UK_HOWLER_IN_USE == VP_UK_HOWLER_BTNR_DRAFT_G)
            /*
             * This is different than how the VP-API-II is provided by default, so we should use
             * what the customer has changed it to (Draft 960-G in this case). Force the type being
             * used to the correc type just to make sure the logic downstream will operate correctly
             * (and won't have to repeat this same logic).
             */
            cadence->toneType = VP_CSLAC_UK_HOWLER_TONE_DRAFT_G;
            VP_SEQUENCER(None, VP_NULL,
                ("\\********** VP_CSLAC_HOWLER_TONE - VP_UK_HOWLER_BTNR_DRAFT_G **********\\"));
            cadence->stopFreq = 0x1AA9;
#else
    #error "VP_UK_HOWLER_IN_USE must be set to either VP_UK_HOWLER_BTNR_VER15 or VP_UK_HOWLER_BTNR_DRAFT_G"
#endif
            /* Sweep interval passed to this funciton is 510ms in device profile tickrate scale */
            VP_SEQUENCER(None, VP_NULL,
                ("Computing Howler Params: sweepRange (510ms = 0x1FE00), updateInterval (0x%04X)",
                 updateInterval));

            VpCSLACComputeHowlerFreqStep(0x1FE00, cadence, updateInterval);

            /*
             * Level Sweep is over a 36dB range. The maximum level is 0x7FFF, so the minimum value
             * is 36dB less: (0x7FFF * 0.015849 = 519.32 (0x207)).
             *
             *      startLevel = 0x0207;
             *      stopLevel = 0x7FFF;
             */
            cadence->startLevel = 0x0207;
            cadence->stopLevel = 0x7FFF;

            /*
             * The entire level sweep for UK Howler Tone is 12+/-2 seconds so if we want to use the
             * freuency transition point as an indicator when to make the level change, then we
             * need to make each level step 36dB/12 = 3dB ideally. However, the frequency steps
             * are not ideally 1 second intervals (generally 1.02 seconds) so don't divide by 12.
             * Instead, divide by 12/1.02 = 11.7647 for 36dB/11.7647 = 3.06dB. Converting gives:
             * 1.422328787 which 1.15-bit format is 1.422332764 using 0xB60F.
             */
            cadence->levelStep = 0xB60F;
            break;

        case VP_CSLAC_AUS_HOWLER_TONE:  /* Australian Standard - ACIF S002:2001 */
            VP_SEQUENCER(None, VP_NULL,
                ("\\******************** VP_CSLAC_AUS_HOWLER_TONE ********************\\"));
            /*
             * Frequency Sweep Rate = 1 second period (500ms each sweep direction)
             *
             *  Frequency Range = [1500Hz to 3.2kHz]
             *      startFreq = 0x1000 (1500Hz)
             *      stopFreq =  0x2222 (3200Hz)
             */
            cadence->startFreq = 0x1000;    /* 1500Hz */
            cadence->stopFreq = 0x2222;     /* 3200Hz */

            /* Sweep interval passed to this funciton is 500ms in device profile tickrate scale */
            VpCSLACComputeHowlerFreqStep(0x1F400, cadence, updateInterval);

            /*
             * Level Sweep is over a 30dB range from -10dBm to +20dBm. Max is = 0x7FFF (by
             * definition), so min being 30dB less is (32,767 * 0.0316227 = 1036.184 (0x40D rounded
             * up)).
             *
             *      startLevel = 0x040D;
             *      stopLevel = 0x7FFF;
             */
            cadence->startLevel = 0x040D;
            cadence->stopLevel = 0x7FFF;

            /*
             * The entire level sweep for AUS Howler Tone is 20+/-5 seconds so if we want to use the
             * freuency transition point as an indicator when to make the level change, then we
             * need to make each level step 30dB/20 = 1.5dB ideally. However, the frequency steps
             * are not ideally 1 second intervals (genally 1.02 seconds) so don't divide by 20.
             * Instead, divide by 20/1.02 = 19.6078 for 30dB/19.6078 = 1.53dB. Converting gives:
             * 1.19261 which 1.15-bit format is 1.530092402 using 0x98A8.
             */
            cadence->levelStep = 0x98A8;
            break;

        case VP_CSLAC_NTT_HOWLER_TONE:  /* NTT Edition 5 */
            VP_SEQUENCER(None, VP_NULL,
                ("\\******************** VP_CSLAC_NTT_HOWLER_TONE ********************\\"));
            /*
             * NTT Howler is specified as a single frequency (400Hz) with continuously increasing
             * level over a period of [3-15 seconds] up to <= 36dBm and is output for 10 - 22
             * seconds. This algorithm manages the frequency programming and level sweep portion
             * only. It is up to the application to stop NTT Howler tone in <= 22 seconds in order
             * to meet Edition 5 Requirements.
             *
             * The requirements mentioned above are shown in NTT Edition 5, Table 3.3.8 Electrical
             * Conditions for Audible Tones sent by the Network. The requirement for the total level
             * sweep is unclear. The API implements a nominal 30dB increase.
             *
             * Due to rounding errors (for using 16-bit fixed point math), the API does not target
             * the nominal 15 second sweep limit. Instead, it targets 14.7 seconds which has been
             * computed (from tickrates 5 - 10ms in 0.5ms step sizes) and measured (5, 6, 7, 8,
             * 8.33ms, 9ms, and 10ms tickrate) to meet the <= 15 second sweep requirement.
             */
            /*
             * Frequency = 400Hz fixed
             *      startFreq = 0x0444 (400Hz)
             *      stopFreq = 0x0444 (400Hz)
             *      freqStep = 0
             */
            cadence->startFreq = 0x0444;    /* 400Hz */
            cadence->freqStep = 0x0000;
            cadence->stopFreq = 0x0444;     /* 400Hz */

            /*
             * Level Sweep is over a 30dB range from min to max. Maximum level is specified only to
             * <= 36dBm per NTT Edition 5. It would be good to see if a more precise requirement
             * exists. The max sillicon program level is = 0x7FFF, so with the min being 30dB less
             * means the starting level is (32,767 * 0.0316227 = 1036.184 (0x40C)).
             *
             *      startLevel = 0x040C;
             *      stopLevel = 0x7FFF;
             */
            cadence->startLevel = 0x040C;
            cadence->stopLevel = 0x7FFF;

            /*
             * The level is ramped continuously in dB over a 14.7 second interval (NTT Edition 5
             * states 3-15 second interval). If we were to compute this at run-time, we would have
             * to divide 30dB by the number of available adjustment steps in a 14.7-second window
             * then convert the result from dB to voltage gain. As of P2.19.0 which use this
             * function only for VE880 and VE890 API, both of which require tickrate <= 12ms and
             * in no known customer use have tickrate < 5ms, it's more efficient to simply use a
             * lookup table. This also removes all tickrate related uncertainty and can be 100%
             * validated. Note that for tickrate values not exactly at the 0.5ms point (i.e., the
             * steps used to create the lookup table), the algorithm will select a level increase
             * that will reduce the total ramp duration.
             */

            /*
             * Initialize to the value that will reach max amplitude the fastest in case of
             * algorithm failure below. This ensures that the Howler Tone will be heard at peak
             * volume by the customer before being disabled.
             */
            cadence->levelStep = 33148;
            {
#define NTT_HOWLER_LUT_MAX  (17)
                uint8 loopCount;
                uint16 levelStepLut[NTT_HOWLER_LUT_MAX][2] = {
                    {1280,   32923},   /* Use for tickrates <= 5ms */
                    {1408,   32938},   /* Use for tickrates (5ms < tickrate <= 5.5ms) */
                    {1536,   32954},   /* Use for tickrates (5.5ms < tickrate <= 6ms) */
                    {1664,   32969},   /* Use for tickrates (6ms < tickrate <= 6.5ms) */

                    /*
                     * Transition from 7ms to just under 7ms is important because the API cadence
                     * time steps are 5ms. So at 7ms it starts to use a second tick to meet the
                     * time required. This has a significant impact on the desired step size
                     */
                    {1791,   32985},   /* Use for tickrates (6.5ms < tickrate < 7ms) */
                    {1792,   32931},   /* Use for tickrates = 7ms */

                    {1920,   32942},   /* Use for tickrates (7ms < tickrate <= 7.5ms) */
                    {2048,   32954},   /* Use for tickrates (7.5ms < tickrate <= 8ms) */
                    {2176,   32966},   /* Use for tickrates (8ms < tickrate <= 8.5ms) */
                    {2304,   32977},   /* Use for tickrates (8.5ms < tickrate <= 9ms) */
                    {2432,   32989},   /* Use for tickrates (9ms < tickrate <= 9.5ms) */
                    {2560,   33000},   /* Use for tickrates (9.5ms < tickrate <= 10ms) */
                    {2688,   33012},   /* Use for tickrates (10ms < tickrate <= 10.5ms) */
                    {2816,   33024},   /* Use for tickrates (10.5ms < tickrate <= 11ms) */
                    {2944,   33035},   /* Use for tickrates (11ms < tickrate <= 11.5ms) */
                    {3072,   33047},   /* Use for tickrates (11.5ms < tickrate <= 12ms) */
                    {0xFFFF, 33224}    /* Use for tickrates > 12ms */
                };

                for (loopCount = 0; loopCount < NTT_HOWLER_LUT_MAX; loopCount++) {
                    if (tickRate <= levelStepLut[loopCount][0]) {
                        cadence->levelStep = levelStepLut[loopCount][1];
                        break;
                    }
                }
             }
            break;

        default:
            returnValue = FALSE;
            break;
    }

    /*
     * Configure the starting direction for frequency changes. The Howler state machine uses the
     * transition from frequency decrease to frequency increase as the indicator for end of sweep.
     * This is the point in UK and AUS Howler Tones where the level is increased (doesn't matter
     * for NTT). So if this is set incorrectly, the level adjustments will be off by one full sweep.
     */
    cadence->isFreqIncrease = TRUE;

    VP_SEQUENCER(None, VP_NULL,
        ("Special Howler Paramaters: Freq Start: 0x%04X, Freq Stop: 0x%04X, Freq Step: 0x%04X",
        cadence->startFreq, cadence->stopFreq, cadence->freqStep));
    VP_SEQUENCER(None, VP_NULL,
        ("Special Howler Paramaters: Level Start: 0x%04X, Level Stop: 0x%04X, Level Step: 0x%04X",
        cadence->startLevel, cadence->stopLevel, cadence->levelStep));
    return returnValue;
}

/**
 * VpCSLACComputeHowlerFreqStep()
 *  This is a helper function for VpCSLACHowlerInit() used to determine the frequency step value
 *  (used for UK and AUS Howler Tones) that will provide a sweep frequency duration of approximately
 *  that specified by "sweepInterval".
 */
void
VpCSLACComputeHowlerFreqStep(
    uint32 sweepInterval,   /**< INPUT: Specifies time to sweep from freqStart to freqStop */
    VpSeqDataType *cadence, /**< I/O: Provides the start/stop frequencies as input. Fills in the
                             * frequency step parameter as output.
                             */
    uint16 updateInterval)  /**< INPUT: Specifies the sequencer update rate (based on tickrate) */
{
    uint16 numUpdateSteps = 0;
    /*
     * In case the sweep interval is not an exact multiple of the update interval, compute
     * the error that will be used to adjust for the actual sweepInterval.
     */
    uint16 stepError = (uint16)(sweepInterval % (uint32)updateInterval);

    /*
     * To offset the rounding down of the frequency steps below, always round down on the
     * sweepInterval.
     */
    sweepInterval -= stepError;

    /* Computation for number of steps should now be an exact integer value.*/
    numUpdateSteps = (uint16)(sweepInterval / (uint32)updateInterval);
    VP_SEQUENCER(None, VP_NULL,
                 ("Num Steps %d From sweepInteval 0x%08lX and updateInterval 0x%04X",
                  numUpdateSteps, sweepInterval, updateInterval));
    /*
     * Round down to the nearest frequency step. The total time should be around nonminal
     * since we rounded down the sweep interval as well.
     */
    stepError = ((cadence->stopFreq - cadence->startFreq) % numUpdateSteps);
    VP_SEQUENCER(None, VP_NULL,
                 ("Frequency Step Error 0x%04X",
                  ((cadence->stopFreq - cadence->startFreq) % numUpdateSteps)));
    cadence->freqStep = ((cadence->stopFreq - cadence->startFreq - stepError) / numUpdateSteps);
}

/**
 * VpDecimalMultiply()
 *  This function returns the result of: (value * byteMask) where: value is in 16-bit format, and
 *  byteMask is the 1.15-bit multiplier. It is a helper function for VE880 and VE890 used in case
 *  of generating Special Howler Tones. These tones are special in that the levels of these tones
 *  increase "regularly" throughout the tone generation sequence.
 *
 * Input arguements are:
 *
 *      value (16.0-bit format)
 *      bitMask (1.15-bit format)
 *
 *   where:
 *      byteMask" comes from the "levelStep" cadence value initialized in CSLACHowlerInit()
 *      "value" comes from the silicon Signal Generator
 */
uint16                  /**< Result of value * bitMask (note the bit representations) */
VpDecimalMultiply(
    uint16 value,       /**< INPUT: First multiplication value in 16.0-bit format */
    uint16 byteMask)    /**< INPUT: Second multiplication value in 1.15-bit format */
{
    uint32 multiplyResult = (value * byteMask);
    uint16 errorResult = (multiplyResult % 32768);

    VP_SEQUENCER(None, VP_NULL,
                 ("Converting 0x%04X times byteMask 0x%04X (1.15 format)", value, byteMask));

    /* Scale the 16 x 16 result to 1.15 format */
    multiplyResult = (multiplyResult / 32768);

    /* Round off */
    if (errorResult > 0x4000) {
        multiplyResult+=1;
    }

    VP_SEQUENCER(None, VP_NULL,
             ("Result of 0x%04X times byteMask 0x%04X (1.15 format) with error (0x%04X) is 0x%08lX",
              value, byteMask, errorResult, multiplyResult));

    return (uint16)multiplyResult;
}

/**
 * VpCSLACProcessRampGenerators()
 *  This function manages the tone generators for the SPecial Howler Tones in the VE880/890 API.
 *  The special howler tones are those that ramp the frequency, amplitude, or both.
 */
bool
VpCSLACProcessRampGenerators(
    VpSeqDataType *cadence)
{
    uint16 tempLevel;
    bool freqCycleComplete = FALSE;
    bool updateLevel = FALSE;
    bool updateFreq = FALSE;

    if (cadence->freqStep != 0) {
        uint16 excessFreq;  /* Used when frequency limits are reached and changing direction */
        uint16 tempFreq = cadence->regData[3];
        tempFreq = ((tempFreq << 8) & 0xFF00);
        tempFreq |= cadence->regData[4];

        /*
         * Non-Zero frequency steps means we're always changing the frequency. Set
         * flag to indicate that the Signal Generator WILL be updated at the end of
         * this case condition.
         */
        updateFreq = TRUE;

        /* Currently performing frequency increases */
        if (cadence->isFreqIncrease == TRUE) {
            /* Check if we're about to exceed the max frequency */
            if ((tempFreq + cadence->freqStep) > cadence->stopFreq) {
                VP_SEQUENCER(None, NULL,
                    ("Can no longer increase Frequency for Howler Tone. Decrease by 1 step."));
                VP_SEQUENCER(None, NULL,
                    ("Current Freq (%d) Max (%d)", tempFreq, cadence->stopFreq));
                excessFreq = (tempFreq + cadence->freqStep) - cadence->stopFreq;
                tempFreq = cadence->stopFreq - excessFreq;
                cadence->isFreqIncrease = FALSE;
            } else {
                /* Not exceeding the max frequency yet. Keep'a going...*/
                tempFreq += cadence->freqStep;
            }
        } else {
            /* Check if we're about to exceed the min frequency */
            if ((tempFreq - cadence->freqStep) < cadence->startFreq) {
                VP_SEQUENCER(None, NULL,
                    ("Can no longer decrease Frequency for Howler Tone. Increase by 1 step."));
                VP_SEQUENCER(None, NULL,
                    ("Current Freq (%d) Min (%d) Step (%d)", tempFreq, cadence->startFreq, cadence->freqStep));
                excessFreq = cadence->startFreq - (tempFreq - cadence->freqStep);
                tempFreq = cadence->startFreq + excessFreq;
                VP_SEQUENCER(None, NULL, ("Increasing to New Freq (%d)", tempFreq));
                cadence->isFreqIncrease = TRUE;
                freqCycleComplete = TRUE;   /* Indicate that a full cycle has completed */
            } else {
                tempFreq -= cadence->freqStep;
            }
        }
        cadence->regData[3] = (tempFreq >> 8) & 0xFF;
        cadence->regData[4] = tempFreq & 0xFF;
    }

    /* Start work on the level adjustments required -- if any */
    tempLevel = (cadence->regData[5] << 8);
    tempLevel |= cadence->regData[6];

    /*
     * Criteria 1 for making a level increase:
     * ---------------------------------------
     *     Current levels in the signal generator must be less than the specified maximum level.
     *     Also, the value of the level step must be non-zero.
     */
    if ((tempLevel < cadence->stopLevel) && (cadence->levelStep > 0)) {
        /*
         * Just because we're not at max AND have a tone that specifies a level increase at some
         * point in the sequence, doesn't mean it's ready to be adjusted. For UK and AUS Howler
         * Tones the level adjustments occur only at the 1 second frequency sweep points. For all
         * other tones, the level steps occur everytime this operation is performed.
         */
        if ((cadence->toneType == VP_CSLAC_UK_HOWLER_TONE_VER15) ||
            (cadence->toneType == VP_CSLAC_UK_HOWLER_TONE_DRAFT_G) ||
            (cadence->toneType == VP_CSLAC_AUS_HOWLER_TONE)) {
            /*
             * For UK and AUS Howler Tones, update when a frequency sweep cycle has just been
             * completed.
             */
            updateLevel = freqCycleComplete;
        } else { /* cadence->toneType == VP_CSLAC_NTT_HOWLER_TONE and all other */
            /*
             * NTT uses a fixed frequency with Linear level increases. Update every chance we get.
             * For all other tones where we don't know any better, level step is applied at every
             * command step.
             */
            updateLevel = TRUE;
        }

        /*
         * If making an update, compute the new value and make sure not to exceed the maximum level
         * specified.
         */
        if (updateLevel) {
            tempLevel = VpDecimalMultiply(tempLevel, cadence->levelStep);
            if (tempLevel > cadence->stopLevel) {
                /* We're at the max, no updates required */
                cadence->levelStep = 0;
                tempLevel = cadence->stopLevel;
            }
            cadence->regData[5] = (tempLevel >> 8) & 0xFF;
            cadence->regData[6] = tempLevel & 0xFF;
        }
    }

    if ((updateLevel) || (updateFreq)) {
        return TRUE;
    } else {
        return FALSE;
    }
}   /* VpCSLACProcessRampGenerators() */

#endif

#if defined (VP_CC_790_SERIES) || \
   (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) || \
   (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT))

/**
 * VpCSLACInitCidStruct()
 *  This function initializes the Caller ID structure to start CID.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  The caller ID struct is initialized. Caller ID will start on the next tick.
 */
void
VpCSLACInitCidStruct(
    VpCallerIdType *pCidStruct,
    uint8 sequenceData)
{
    pCidStruct->status |= VP_CID_IN_PROGRESS;
    if ((sequenceData & VP_SEQ_SUBTYPE_MASK) == VP_SEQ_SUBCMD_WAIT_ON) {
        pCidStruct->status |= VP_CID_WAIT_ON_ACTIVE;
    }
    pCidStruct->cliTimer = 1;
    pCidStruct->cliIndex = 0;
    pCidStruct->cliMPIndex = 0;
    pCidStruct->cliMSIndex = 0;

    pCidStruct->status |= VP_CID_PRIMARY_IN_USE;
    pCidStruct->status &=
        (uint16)(~(VP_CID_FSK_GEN_VALID | VP_CID_TERM_FSK | VP_CID_SECONDARY_IN_USE));
    pCidStruct->currentData = VP_FSK_NONE;
}   /* VpCSLACInitCidStruct() */

/**
 * VpCidSeq()
 *  This function services an active Caller ID Timer. This function runs when
 * the CLI timer for the passed channel is active. The function reads the
 * current caller ID sequence that is pointed to in the caller ID structure for
 * the given channel. This pointer is assigned when ever the function
 * CliStartCli is called. Additionally, the CliStartCli function sets the timer
 * to 1 to seed the CLI process.
 *
 *  This routine is broken up into two functional stages. The first stage
 * handles CLI tasks that are not time related while the second stage handles
 * time related task. Non-time related tasks include things such as muting a
 * channel, pol-rev and EOT (end of transmission). Time related tasks include
 * timing of the MARK signal, the SEIZURE signal and ACK detection as well as
 * timing for sending encoded data bytes to the device FSK generator.
 *
 * Preconditions:
 *  This Function must be called from the ApiTick function.
 *
 * Postconditions:
 *  The Caller ID State Machine is updated. Returns TRUE, if a user defined
 * event was encountered
 */
VpStatusType
VpCidSeq(
    VpLineCtxType *pLineCtx)    /**< Line that has an active CID sequence */
{
    VpStatusType retFlag = VP_STATUS_SUCCESS;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;

    VpSetLineStateFuncPtrType SetLineState;

    VpDeviceIdType deviceId;
    uint16 tickRate;
    uint8 ecVal;
    uint8 indexVal;  /* Use this when several index vals are needed */

    void *pLineObj = pLineCtx->pLineObj;
    void *pDevObj = pDevCtx->pDevObj;

    VpCallerIdType *pCidStruct;
    VpLineStateType lineState;

    VpDigitType digit;

    uint16 uiCliOpCode;
    uint8 startOfCliData, mpiLen;
    uint16 cliTimer = 0;
    uint16 tempDebounceTime = 0;

    uint16 index;
    uint8 scratchData[1];

#if (VP_CC_DEBUG_SELECT & VP_DBG_CID)
    uint16 timeStamp = 0;
#endif

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()+"));

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            SetLineState = Vp790SetLineStateInt;
            lineState = ((Vp790LineObjectType *)pLineObj)->lineState.currentState;

            pCidStruct = &((Vp790LineObjectType *)pLineObj)->callerId;
            if (!(((Vp790DeviceObjectType *)pDevObj)->status.state & VP_DEV_INIT_CMP)) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }
            tickRate =
                ((Vp790DeviceObjectType *)pDevObj)->devProfileData.tickRate;
            deviceId = ((Vp790DeviceObjectType *)pDevObj)->deviceId;
#if (VP_CC_DEBUG_SELECT & VP_DBG_CID)
            timeStamp = ((Vp790DeviceObjectType *)pDevObj)->timeStamp;
#endif

            switch(((Vp790LineObjectType *)pLineObj)->channelId) {
                case 0: ecVal = VP790_EC_CH1;   break;
                case 1: ecVal = VP790_EC_CH2;   break;
                case 2: ecVal = VP790_EC_CH3;   break;
                case 3: ecVal = VP790_EC_CH4;   break;
                default:
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                    return VP_STATUS_FAILURE;
            }
            break;

#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            SetLineState = Vp880SetLineStateInt;
            lineState = ((Vp880LineObjectType *)pLineObj)->lineState.currentState;
            pCidStruct = &((Vp880LineObjectType *)pLineObj)->callerId;

            if (!(((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_INIT_CMP)) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            tickRate =
                ((Vp880DeviceObjectType *)pDevObj)->devProfileData.tickRate;
#if (VP_CC_DEBUG_SELECT & VP_DBG_CID)
            timeStamp = ((Vp880DeviceObjectType *)pDevObj)->timeStamp;
#endif
            deviceId = ((Vp880DeviceObjectType *)pDevObj)->deviceId;
            ecVal = ((Vp880LineObjectType *)pLineObj)->ecVal;
            break;
#endif

#if defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)
        case VP_DEV_890_SERIES:
            SetLineState = Vp890SetFxsLineState;

            lineState = ((Vp890LineObjectType *)pLineObj)->lineState.currentState;

            pCidStruct = &((Vp890LineObjectType *)pLineObj)->callerId;
            if (!(((Vp890DeviceObjectType *)pDevObj)->state & VP_DEV_INIT_CMP)) {
                VP_CID(VpLineCtxType, pLineCtx, ("1. VpCidSeq() -- VP_STATUS_DEV_NOT_INITIALIZED"));
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp890DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                VP_CID(VpLineCtxType, pLineCtx, ("2. VpCidSeq() -- VP_STATUS_DEV_NOT_INITIALIZED"));
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                return VP_STATUS_DEV_NOT_INITIALIZED;
            }

            tickRate =
                ((Vp890DeviceObjectType *)pDevObj)->devProfileData.tickRate;
#if (VP_CC_DEBUG_SELECT & VP_DBG_CID)
            timeStamp = ((Vp890DeviceObjectType *)pDevObj)->timeStamp;
#endif
            deviceId = ((Vp890DeviceObjectType *)pDevObj)->deviceId;
            ecVal = ((Vp890LineObjectType *)pLineObj)->ecVal;
            break;
#endif

        default:
            return VP_STATUS_INVALID_ARG;
    }

    /* Determine if the timer is running. */
    if(pCidStruct->cliTimer > 0) {
        pCidStruct->cliTimer--;
        if (pCidStruct->status & VP_CID_REPEAT_MSG) {
            if(VpFSKGeneratorReady(pLineCtx)) {
                while(VpCtrlSetFSKGen(pLineCtx, VP_CID_GENERATOR_KEYED_CHAR,
                    pCidStruct->currentData) != 0);
            };
        }

        if(pCidStruct->cliTimer != 0) {
            VP_CID(VpLineCtxType, pLineCtx, ("1. VpCidSeq() -- Running Timer %d at time %d",
                pCidStruct->cliTimer, timeStamp));
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
            return VP_STATUS_SUCCESS;
        } else {
            /*
             * The CLI tone generators are enabled for Alerting Tone which uses
             * the caller id timer to control on-time (as opposed to using the
             * sequencer timers). So we need to disable these generators at the
             * end of Alterting tone time. For both VE880 and VE890, if the
             * Alerting Tone generators were previously disabled the silicon is
             * not accessed by this function. The required values are cached in
             * the line objects. Only for VE790 will a read occur, but a write
             * will not if the "previous" and "new" control values are the same.
             */
            VpCtrlSetCliTone(pLineCtx, FALSE);
        }
    } else {
        VP_CID(VpLineCtxType, pLineCtx,
            ("VpCidSeq() -- Timer NOT running at time %d", timeStamp));
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
    }
    pCidStruct->status &= ~VP_CID_REPEAT_MSG;

    /*
     * Find where the start of the CLI command data is (excluding the MPI
     * command/data used to set the tone generator(s)
     */
    mpiLen = pCidStruct->pCliProfile[VP_CID_PROFILE_FSK_PARAM_LEN];

    /*
     * Start of CLI commands on the LSB (word aligned). Exact location found
     * by adding the start elements offset to the end of the mpi command data
     * (found from the location of the mpi command length + the actual length
     * of the mpi data).
     */
    startOfCliData = VP_CID_PROFILE_FSK_PARAM_LEN + mpiLen +
        VP_CID_PROFILE_START_OF_ELEMENTS_LSB;

    /* Get the current index for the CLI profile. */
    index = pCidStruct->cliIndex;
    pCidStruct->cliDebounceTime = 0;

    /*
     * This section of code tests to see if a CPE ACK was received.
     * If the variable cliAwaitTone is TRUE, then test to see if the ACK
     * was received prior to the timeout specified in the CLI_DETECT portion
     * of the CLI profile. If the ACK was not received, you can not send the
     * CID information so terminate the CLI sequence.
     */
    if (pCidStruct->status & VP_CID_AWAIT_TONE) {
        VpCtrlDetectDTMF(pLineCtx, FALSE);
        /*
         * This would have been set to VP_DIG_NONE prior to starting the DTMF
         * digit detection. So any other value (whether in make or break
         * interval) is indication of DTMF digit detected during detection
         * interval.
         */

        digit = pCidStruct->digitDet;

        if ((digit == pCidStruct->cliDetectTone1)
         || (digit == pCidStruct->cliDetectTone2)) {
            /* The ACK tone was detected, continue with Caller ID */
        } else {
            /* Ack tone not detected, stop Caller ID */
            VpCliStopCli(pLineCtx);
            VP_CID(VpLineCtxType, pLineCtx, ("Ack Tone Not Detected"));
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
            return VP_STATUS_SUCCESS;
        }
    }

    /*
     * If previously started a termination sequence, see if the FSK generator
     * is needed for the next step before disabling.
     */
    if (pCidStruct->status & VP_CID_TERM_FSK) {
        pCidStruct->status &= ~VP_CID_TERM_FSK;
        pCidStruct->cliTimer = MS_TO_TICKRATE((tickRate >> 8), tickRate);

        switch(pCidStruct->pCliProfile[startOfCliData + index]) {
            case VP_CLI_MESSAGE:
            case VP_CLI_CHANSEIZURE:
            case VP_CLI_MARKSIGNAL:
                break;

            default:
                VP_CID(VpLineCtxType, pLineCtx,
                    ("VpCidSeq(): Timeout complete. Terminating FSK at time %d", timeStamp));
                VpCtrlSetFSKGen(pLineCtx, VP_CID_SIGGEN_EOT, 1);
                break;
        }
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Handle zero-time-length codes. Zero time codes are op-codes in the
     * profile that are executed but have no time associated with them. In
     * other words the CLI sequence immediately moves on to the next state.
     * they include POL-REV, Channel mute, and EOT. The while loop churns
     * through the profile until a time related element is encountered.
     */
    uiCliOpCode = pCidStruct->pCliProfile[startOfCliData + index];

    VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code 0x%02X at index %d time %d",
        uiCliOpCode, index, timeStamp));

    while ( (index <= pCidStruct->pCliProfile[startOfCliData - 2]) &&
            ( (uiCliOpCode == VP_CLI_POLREV) ||
              (uiCliOpCode == VP_CLI_EOT) ||
              (uiCliOpCode == VP_CLI_MUTEON) ||
              (uiCliOpCode == VP_CLI_MUTEOFF)) ) {

        switch (uiCliOpCode) {
            /* Mute both the upstream and down stream transmission paths. */
            case VP_CLI_MUTEON:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_MUTEON"));
                index+=2;
                pCidStruct->status |= VP_CID_MUTE_ON;
                VpCtrlMuteChannel(pLineCtx, TRUE);
                break;

            /*
             * Re-enable audio transmission in both the upstream and downstream
             * directions.
             */
            case VP_CLI_MUTEOFF:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_MUTEOFF"));
                index+=2;
                pCidStruct->status &= ~(VP_CID_MUTE_ON);
                VpCtrlMuteChannel(pLineCtx, FALSE);
                break;

            /* Invert the polarity of the line. */
            case VP_CLI_POLREV:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_POLREV"));
                index+=2;
                /*
                 * VpGetReverseState() returns the reversal polarity equivalent of the state
                 * passe if it recognizes it, or returns the same state passed if it does not
                 * recognize it.
                 */
                SetLineState(pLineCtx, VpGetReverseState(lineState));
                break;

            /* Indicates the End Of Transmission for the CLI sequence */
            case VP_CLI_EOT:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_EOT at time %d",
                    timeStamp));
                VpCliStopCli(pLineCtx);
                return VP_STATUS_SUCCESS;

            default:
                index+=2;
                break;
        }
        uiCliOpCode = pCidStruct->pCliProfile[startOfCliData + index];
    }

    /*
     * Process all time based CLI profile codes. This includes timing of
     * channel seizure, ACK detect, MARK, and the message data.
     */
    if (index <= pCidStruct->pCliProfile[startOfCliData - 2]) {
        /* Switch on CLI Profile Element type at the current index. */
        switch (pCidStruct->pCliProfile[startOfCliData + index]) {

            /*
             * Set up the CLI sequence for detection of the CPE ACK. This state
             * will stop any running sequence, disable the sig gen, stop any
             * FSK activity, and set a time out for the tone detection. If
             * the time-out time is reached, the CPE did not ACK and the CLI
             * sequence will be aborted.
             */
            case VP_CLI_DETECT:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_DETECT"));

               /* Turn off the tone generator and turn FSK off */
                VpSetLineTone(pLineCtx, VP_PTABLE_NULL, VP_PTABLE_NULL, VP_NULL);

                /* Read the timeout time */
                index++;
                cliTimer = pCidStruct->pCliProfile[startOfCliData + index];
                cliTimer = ((cliTimer << 8) & 0xFF00);
                index++;
                cliTimer |=
                    (pCidStruct->pCliProfile[startOfCliData + index] & 0x00FF);
                cliTimer *= VP_CID_TIMESCALE;

                /* Read which tones to detect. */
                index+=2;

                /*
                 * The data from profile wizard can be shifted 4 right and be
                 * interpreted directly as a VpDigitType (other than 0xFF for
                 * Digit Type None)
                 */
                pCidStruct->cliDetectTone1 =
                    (VpDigitType)(pCidStruct->pCliProfile[startOfCliData + index]);
                if (pCidStruct->cliDetectTone1 != VP_DIG_NONE) {
                    pCidStruct->cliDetectTone1 =
                        (VpDigitType)((pCidStruct->cliDetectTone1 >> 4) & 0xFF);
                }
                if (VpIsDigit(pCidStruct->cliDetectTone1) == FALSE) {
                    VpCliStopCli(pLineCtx);
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                    return VP_STATUS_INVALID_ARG;
                }

                index+=2;
                pCidStruct->cliDetectTone2 =
                    (VpDigitType)(pCidStruct->pCliProfile[startOfCliData + index]);

                if (pCidStruct->cliDetectTone2 != VP_DIG_NONE) {
                    pCidStruct->cliDetectTone2 =
                        (VpDigitType)((pCidStruct->cliDetectTone2 >> 4) & 0xFF);
                }
                if (VpIsDigit(pCidStruct->cliDetectTone2) == FALSE) {
                    VpCliStopCli(pLineCtx);
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
                    return VP_STATUS_INVALID_ARG;
                }

                /* Start the tone detector. */
                index+=2;
                VpCtrlDetectDTMF(pLineCtx, TRUE);
                break;

            /*
             * This case sets up the signal generator to generate tones that
             * are defined in the profile. This will include things like the call
             * waiting beep. This case does not actually start the signal generator
             * it only sets it up.
             */
            case VP_CLI_ALERTTONE:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_ALERTTONE"));

                /* Stop any running sequences and disable any FSK activity. */
                VpSetLineTone(pLineCtx, VP_PTABLE_NULL, VP_PTABLE_NULL, VP_NULL);

                /*
                 * Set the timer to return as soon as possible before enabling the
                 * tone generator.
                 */
                index++;    /* Get to the length of MPI data to send */
                cliTimer = (tickRate >> 8);

                /*
                 * Send the MPI data to the device starting at the point after the
                 * length of data field
                 */
                indexVal = startOfCliData + index + 1;
                VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
                    pCidStruct->pCliProfile[startOfCliData + index],
                    (VpProfileDataType *)(&pCidStruct->pCliProfile[indexVal]));

                /*
                 * We don't know if the previous command modified the FSK
                 * Generator. To be safe, assume that it DID affect it. Next
                 * time the FSK generator is required, clearing this flag will
                 * force it to be reprogrammed.
                 */
                pCidStruct->status &= ~VP_CID_FSK_GEN_VALID;

                /* Get to the next command after the MPI data */
                index += pCidStruct->pCliProfile[startOfCliData + index];
                index += 2;
                break;

            /*
             * This case starts the signal generator for the time specified in
             * the CLI profile. It is assumed the signal generator was set up
             * first in the previous case.
             */
            case VP_CLI_ALERTTONE2:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_ALERTTONE2"));

                VpCtrlSetCliTone(pLineCtx, TRUE);
                index++;

                cliTimer = pCidStruct->pCliProfile[startOfCliData + index];
                cliTimer = ((cliTimer << 8) & 0xFF00);

                index++;
                cliTimer |=
                    (pCidStruct->pCliProfile[startOfCliData + index] & 0x00FF);
                cliTimer *= VP_CID_TIMESCALE;

                index+=2;
                break;

            /*
             * This case creates a silent period of the time specified in the
             * profile. This is done by disabling the signal generator and
             * FSK generator.
             */
            case VP_CLI_SILENCE:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_SILENCE"));

                VpSetLineTone(pLineCtx, VP_PTABLE_NULL, VP_PTABLE_NULL, VP_NULL);
                index++;

                cliTimer = pCidStruct->pCliProfile[startOfCliData + index];
                cliTimer = ((cliTimer << 8) & 0xFF00);

                index++;
                cliTimer |=
                    (pCidStruct->pCliProfile[startOfCliData + index] & 0x00FF);
                cliTimer *= VP_CID_TIMESCALE;

                index+=2;
                break;

             /*
              * This case creates a silent period and prevents hook switch from
              * being detected for cliDebouncing time. Currently the debounce
              * period does nothing and should be enabled by creating a mask hook
              * method in the LIU.
              */
            case VP_CLI_SILENCE_MASKHOOK:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_SILENCE_MASKHOOK"));

                VpSetLineTone(pLineCtx, VP_PTABLE_NULL, VP_PTABLE_NULL, VP_NULL);
                index++;

                cliTimer = pCidStruct->pCliProfile[startOfCliData + index];
                cliTimer = ((cliTimer << 8) & 0xFF00);

                index++;
                cliTimer |= (pCidStruct->pCliProfile[startOfCliData + index] & 0x00FF);
                cliTimer *= VP_CID_TIMESCALE;

                index++;
                tempDebounceTime = (pCidStruct->pCliProfile[startOfCliData + index] << 8) & 0xFF00;
                index++;
                tempDebounceTime |= (pCidStruct->pCliProfile[startOfCliData + index] & 0x00FF);
                tempDebounceTime *= VP_CID_TIMESCALE;

                pCidStruct->cliDebounceTime = MS_TO_TICKRATE(tempDebounceTime, tickRate);
                index+=2;
                break;

            /*
             * This case creates the channel seizure or mark signal for the time
             * specified. Channel Siezure is alternating 1/0 (0x55) so the
             * start bit has to be set = 1, stop bit set = 0. Mark signal is
             * all '1's so start bit = stop bit = 1.
             */
            case VP_CLI_CHANSEIZURE:
            case VP_CLI_MARKSIGNAL: {
                uint8 signalData;
                uint8 maxCount = 0;
                if (pCidStruct->pCliProfile[startOfCliData + index] == VP_CLI_CHANSEIZURE) {
                    signalData = VP_FSK_CHAN_SEIZURE;
                } else {
                    signalData = VP_FSK_MARK_SIGNAL;
                }
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code %s",
                    ((signalData == VP_FSK_CHAN_SEIZURE) ? "VP_CLI_CHANSEIZURE" : "VP_CLI_MARKSIGNAL")));

                /*
                 * See if the FSK generator is programmed correctly. If not,
                 * program based on data from the profile. Note: If not set,
                 * it is likely because this is the first time programming it
                 * for the current Caller ID profile OR the generator was
                 * possibly reprogrammed for Alterting Tone.
                 */
                if (!(pCidStruct->status & VP_CID_FSK_GEN_VALID)) {
                    VP_CID(VpLineCtxType, pLineCtx,
                        ("FSK Generator Programming Required"));
                    indexVal = VP_CID_PROFILE_FSK_PARAM_LEN + 1;
                    VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
                        pCidStruct->pCliProfile[VP_CID_PROFILE_FSK_PARAM_LEN],
                        (VpProfileDataType *)(&pCidStruct->pCliProfile[indexVal]));

                    pCidStruct->status |= VP_CID_FSK_GEN_VALID;
                }

                pCidStruct->markOutByteCount = 0;
                if (VpFSKGeneratorReady(pLineCtx)) {
                    maxCount = 1;
                    while(VpCtrlSetFSKGen(pLineCtx, VP_CID_GENERATOR_KEYED_CHAR, signalData) != 0) {
                        maxCount++;
                    }
                }
                VP_CID(VpLineCtxType, pLineCtx,
                    ("Provided %d bytes of Keyed Character to FSK Generator",
                    maxCount));

                pCidStruct->currentData = signalData;

                pCidStruct->status |= VP_CID_REPEAT_MSG;

                index++;

                cliTimer = pCidStruct->pCliProfile[startOfCliData + index];
                cliTimer = ((cliTimer << 8) & 0xFF00);

                index++;
                cliTimer |=
                    (pCidStruct->pCliProfile[startOfCliData + index] & 0x00FF);
                cliTimer *= VP_CID_TIMESCALE;

                /*
                 * cliTimer is now in ms. For low values, subtract off the
                 * time provided in the caller id data already loaded in the
                 * buffer to improve accuracy. High values (>300ms) are not
                 * generally used, and the accuracy is far less important.
                 * NOTE: Caller ID timescale is in ms and one byte is 8.33ms
                 */
                if ((cliTimer / 100) <= 655) {
                    if ((cliTimer * 100) < (833 * maxCount)) {
                        cliTimer = 0;
                    } else {
                        cliTimer = ((cliTimer * 100) - (833 * maxCount));
                        cliTimer /= 100;
                    }
                }

                index+=2;
                pCidStruct->status |= VP_CID_TERM_FSK;

                VP_CID(VpLineCtxType, pLineCtx,
                    ("Setting Keyed Character timer to %d ms", cliTimer));
                }
                break;

            /* This case sends the actual message data (FSK Format). */
            case VP_CLI_MESSAGE: {
                bool startEndFsk = FALSE;
                uint8 dataRemain;

                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_MESSAGE"));

                /* If the FSK Generator was not setup before, set it now */
                if (!(pCidStruct->status & VP_CID_FSK_GEN_VALID)) {
                    indexVal = VP_CID_PROFILE_FSK_PARAM_LEN + 1;
                    VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
                        pCidStruct->pCliProfile[VP_CID_PROFILE_FSK_PARAM_LEN],
                        (VpProfileDataType *)(&pCidStruct->pCliProfile[indexVal]));

                    pCidStruct->status |= VP_CID_FSK_GEN_VALID;
                }
                /*
                 * Limit the number of bytes we'll try to provide the device in
                 * order to avoid infinite looping. Infinite looping can only
                 * occur if the device is not responding to CID message data
                 * buffer (fill) commands OR if the VP-API-II cannot read from
                 * the device and is simply confused. The message data buffer
                 * won't respond in certain SLIC feed states - so this condition
                 * is certainly possible.
                 */
                if (VpFSKGeneratorReady(pLineCtx)) {
                    VpCliEncodedDataType encodeDataType;
                    dataRemain = 1;

                    while((dataRemain > 0) && (startEndFsk == FALSE)) {
                        /*
                         * For legacy reasons all CID commands are word aligned but
                         * none actually use the upper byte (always set to 0x00 by
                         * Profile Wizard). The FSK message type is expanded to
                         * support a Mark-Out signal by setting this byte to the
                         * number of Mark-Out bytes after FSK message data (and
                         * checksum) required. This makes it 100% compatible with
                         * legacy profiles since 0 means no mark-out required.
                         */
                        pCidStruct->markOutByteCount =
                            pCidStruct->pCliProfile[startOfCliData + index - 1];
                        encodeDataType = VpCliGetEncodedByte(pLineCtx, scratchData);

                        VP_CID(VpLineCtxType, pLineCtx,
                            ("VpCidSeq(): EncodedData (%s) Mark-Out Length (%d) Mark-Out Remain (%d) at time %d",
                            ((encodeDataType == VP_CLI_ENCODE_DATA) ? "VP_CLI_ENCODE_DATA" :
                             ((encodeDataType == VP_CLI_ENCODE_MARKOUT) ? "VP_CLI_ENCODE_MARKOUT" : "VP_CLI_ENCODE_END")),
                             pCidStruct->markOutByteCount,
                             pCidStruct->markOutByteRemain, timeStamp));

                        /* Determine (and send) next data/data type in the buffer */
                        if (encodeDataType == VP_CLI_ENCODE_DATA) {
                            /*
                             * The device level APIs at this point "know" if the
                             * this is the last byte being sent. If it is, these
                             * APIs (SetFSKGen) will start the End-Of-Message
                             * Sequence which creates a potential gap in the FSK
                             * signal. In case of providing a Mark-Out signal this
                             * gap has to be avoided, hence the EOM sequence has to
                             * be stopped before this function call. Logic in
                             * "Encode Byte" is where the message buffer is being
                             * evaluated and therefore is where this check must be
                             * performed.
                             */
                            pCidStruct->currentData = VP_FSK_DATA;
                            dataRemain = VpCtrlSetFSKGen(pLineCtx, VP_CID_GENERATOR_DATA,
                                scratchData[0]);
                        } else if ((encodeDataType == VP_CLI_ENCODE_MARKOUT)
                                && (pCidStruct->markOutByteRemain > 0)) {
                            /*
                             * Note that the "currentData" type is changed AFTER
                             * calling SetFSKGen(). This is to trigger the first
                             * time the Mark Signal is being sent to copy the
                             * markOutByteCount value to markOutByteRemain. If
                             * this value is already set to Mark Signal prior to
                             * this function, it only decrements markOutByteRemain
                             * until = 0.
                             */
                            VP_CID(VpLineCtxType, pLineCtx,
                                ("VpCidSeq(): Running Mark-Out (%d) with Remaining (%d) at time %d",
                                pCidStruct->markOutByteCount,
                                pCidStruct->markOutByteRemain, timeStamp));
                                dataRemain = VpCtrlSetFSKGen(pLineCtx, VP_CID_GENERATOR_KEYED_CHAR,
                                VP_FSK_MARK_SIGNAL);
                            pCidStruct->currentData = VP_FSK_MARK_SIGNAL;
                        } else {    /* VP_CLI_ENCODE_END    */
                            startEndFsk = TRUE;

                            /*
                             * We're done with FSK Message Data, but may still need
                             * to keep the FSK Generator on. We can only know this
                             * by peeking at the next element.
                             */
                            index+=2;   /* This has to be done anyway... */

                            if (index <= pCidStruct->pCliProfile[startOfCliData - 2]) {
                                /*
                                 * Peek at the next element. If FSK is NOT required,
                                 * start the ending sequence.
                                 */
                                switch (pCidStruct->pCliProfile[startOfCliData + index]) {
                                    case VP_CLI_MARKSIGNAL:
                                    case VP_CLI_CHANSEIZURE:
                                    case VP_CLI_MESSAGE:
                                        startEndFsk = FALSE;
                                        break;

                                    default:    /* FSK not required for next step */
                                        break;
                                }
                            }
                            if (startEndFsk) {
                                /*
                                 * Last value, '1' is a flag to the device specific
                                 * API-II that this step can tolerate suspending CID
                                 * until all generator data is sent. A '0' indicates
                                 * that CID cannot be suspended.
                                 */
                                VP_CID(VpLineCtxType, pLineCtx,
                                    ("VpCidSeq(): Proceeding with normal FSK Message Termination at time %d",
                                    timeStamp));
                                VpCtrlSetFSKGen(pLineCtx, VP_CID_SIGGEN_EOT, 1);
                                    dataRemain = 0;
                                pCidStruct->currentData = VP_FSK_NONE;
                            }   /* if (startEndFsk) */
                        }   /* else VP_CLI_ENCODEE_END */
                    }   /* while there is more data and NOT the start of FSK end */
                } /* if FSK Generator is Ready */
                /*
                 * Prevent the timer from running. If it times out, it will
                 * disable the tone generators (without added logic above to
                 * prevent it).
                 */
                cliTimer = 0;
                }
                break;

            /* This case sends the actual message data (DTMF Format). */
            case VP_CLI_DTMF_MESSAGE:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code VP_CLI_DTMF_MESSAGE"));

                if (VpDTMFGeneratorReady(pLineCtx) == TRUE) {
                    /* Send next data in buffer */
                    /*
                     * Clear the markOutByteCount intended only to be used with
                     * FSK message data.
                     */
                    pCidStruct->markOutByteCount = 0;

                    if(VpCliGetEncodedByte(pLineCtx, scratchData) == VP_CLI_ENCODE_DATA) {
                        VpCtrlSetDTMFGen(pLineCtx, VP_CID_GENERATOR_DATA,
                            VpConvertCharToDigitType(scratchData[0]));
                    } else {
                        /* We're done. Disable the Generators and go to the next element */
                        VpCtrlSetDTMFGen(pLineCtx, VP_CID_SIGGEN_EOT, VP_DIG_NONE);
                        index+=2;
                    }
                }

                /*
                 * Prevent the timer from running. If it times out, it will
                 * disable the tone generators (without added logic above to
                 * prevent it).
                 */
                cliTimer = 0;
                break;

            /* Shouldn't be possible */
            default:
                VP_CID(VpLineCtxType, pLineCtx, ("CID Op-Code ERROR"));

                index = 0xFFFF; /* Force stop */
                break;

        } /* End of Switch (CLI Element Type) */
    } /* if index <= number of elements */

    VP_CID(VpLineCtxType, pLineCtx, ("Next Index Value %d for Total Length %d",
        index, pCidStruct->pCliProfile[startOfCliData - 2]));

    /*
     * If the CLI sequencer has indexed passed the number of elements in the
     * the profile, then stop the CLI sequence. This condition is true if
     * the ACK was not received, the EOT marker in the profile was reached
     * or an error occurred. Otherwise, continue normally.
     */
    if (index > pCidStruct->pCliProfile[startOfCliData - 2]) {
        VP_CID(VpLineCtxType, pLineCtx, ("Stopping CID"));
        VpCliStopCli(pLineCtx);
    } else {
        pCidStruct->cliIndex = index;
        pCidStruct->cliTimer = MS_TO_TICKRATE(cliTimer, tickRate);
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("VpCidSeq()-"));
    return retFlag;
}   /* VpCidSeq() */

/**
 * VpFSKGeneratorReady()
 *  This function returns TRUE if the FSK Generator is ready to accept the next
 * CID message byte, FALSE otherwise.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  None. Status of FSK Generator only is reported. Otherwise line is unaffected
 */
bool
VpFSKGeneratorReady(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            return Vp790FSKGeneratorReady(pLineCtx);
#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            return Vp880FSKGeneratorReady(pLineCtx);
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            return Vp890FSKGeneratorReady(pLineCtx);
#endif

        default:
            return FALSE;
    }
}   /* VpFSKGeneratorReady() */

/**
 * VpDTMFGeneratorReady()
 *  This function returns TRUE if the DTMF Generator is ready to accept the next
 * CID message byte, FALSE otherwise.
 *
 * Preconditions:
 *  None.
 *
 * Postconditions:
 *  None. Status of DTMF Generator only is reported. Otherwise line is unaffected
 */
bool
VpDTMFGeneratorReady(
    VpLineCtxType *pLineCtx)
{
    bool returnVal = TRUE;

#if defined (VP_CC_880_SERIES) || defined (VP_CC_890_SERIES)
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;
    uint16 tickRate;

    VpCslacTimers *pLineTimers;

    void *pLineObj = pLineCtx->pLineObj;
    void *pDevObj = pDevCtx->pDevObj;

    VpCallerIdType *pCidStruct = VP_NULL;

    switch (deviceType) {

#ifdef VP_CC_880_SERIES
        case VP_DEV_880_SERIES:
            pLineTimers = &((Vp880LineObjectType *)pLineObj)->lineTimers.timers;

#if defined (VP880_FXS_SUPPORT)
            pCidStruct = &((Vp880LineObjectType *)pLineObj)->callerId;
#endif
            tickRate =
                ((Vp880DeviceObjectType *)pDevObj)->devProfileData.tickRate;
            break;
#endif

#ifdef VP_CC_890_SERIES
        case VP_DEV_890_SERIES:
            pLineTimers = &((Vp890LineObjectType *)pLineObj)->lineTimers.timers;
#if defined (VP890_FXS_SUPPORT)
            pCidStruct = &((Vp890LineObjectType *)pLineObj)->callerId;
#endif
            tickRate =
                ((Vp890DeviceObjectType *)pDevObj)->devProfileData.tickRate;
            break;
#endif

        default:
            return TRUE;
    }

    if ((!(pLineTimers->timer[VP_LINE_TIMER_CID_DTMF] & VP_ACTIVATE_TIMER))&&
        (pCidStruct != VP_NULL)) {
        /*
         * Timer appears available. If we're in an on-time already, change to
         * the off-time and return FALSE (i.e., cannot program another DTMF
         * number yet). If it's in an off-time, it's now complete so return
         * TRUE.
         */
        if (pCidStruct->dtmfStatus & VP_CID_ACTIVE_ON_TIME) {
            pCidStruct->dtmfStatus &= ~VP_CID_ACTIVE_ON_TIME;

            /* Setup and activate/start the line timer for the off-time for DTMF CID */
            pLineTimers->timer[VP_LINE_TIMER_CID_DTMF] =
                (MS_TO_TICKS_ROUND_UP(VP_CID_DTMF_OFF_TIME, tickRate)) | VP_ACTIVATE_TIMER;

            pCidStruct->dtmfStatus |= VP_CID_ACTIVE_OFF_TIME;

            /* Disable the DTMF Generator */
            VpCtrlSetDTMFGen(pLineCtx, VP_CID_SIGGEN_EOT, VP_DIG_NONE);

            returnVal = FALSE;
        } else if (pCidStruct->dtmfStatus & VP_CID_ACTIVE_OFF_TIME) {
            pCidStruct->dtmfStatus &= ~VP_CID_ACTIVE_OFF_TIME;

            /* Nothing more to do here */
            returnVal = TRUE;
        }
    } else {
        returnVal = FALSE;
    }
#endif

    return returnVal;
}   /* VpDTMFGeneratorReady() */

#if defined (VP_CC_790_SERIES) || (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) || \
   (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT))
/**
 * VpCliStopCli()
 *  This function stops the CLI sequence on the passed line.
 *
 * Preconditions
 *  None
 *
 * Postconditions
 *  The caller ID sequence, if running, will be aborted.
 */
void
VpCliStopCli(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;

    void *pLineObj = pLineCtx->pLineObj;

    uint16 *pEventData;
    VpOptionEventMaskType *pLineEvents;

    VpCallerIdType *pCidStruct;

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            pCidStruct = &((Vp790LineObjectType *)pLineObj)->callerId;
            pCidStruct->cliTimer = 0;
            Vp790SetLineTone(pLineCtx, VP_PTABLE_NULL, VP_PTABLE_NULL, VP_NULL);
            pLineEvents = &((Vp790LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp790LineObjectType *)pLineObj)->processData;
            break;

#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            pCidStruct = &((Vp880LineObjectType *)pLineObj)->callerId;
            pCidStruct->cliTimer = 0;
            Vp880SetLineTone(pLineCtx, VP_PTABLE_NULL, VP_PTABLE_NULL, VP_NULL);
            pLineEvents = &((Vp880LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp880LineObjectType *)pLineObj)->processData;
            break;
#endif

#if defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)
        case VP_DEV_890_SERIES:
            pCidStruct = &((Vp890LineObjectType *)pLineObj)->callerId;
            pCidStruct->cliTimer = 0;
            Vp890SetLineTone(pLineCtx, VP_PTABLE_NULL, VP_PTABLE_NULL, VP_NULL);
            pLineEvents = &((Vp890LineObjectType *)pLineObj)->lineEvents;
            pEventData = &((Vp890LineObjectType *)pLineObj)->processData;
            break;
#endif

        default:
            return;
    }

    if ((pCidStruct->status & VP_CID_FSK_GEN_VALID) == VP_CID_FSK_GEN_VALID) {
        VpCtrlSetFSKGen(pLineCtx, VP_CID_SIGGEN_EOT, 1);
        pCidStruct->status &= ~(VP_CID_FSK_GEN_VALID);
    }
    VpCtrlDetectDTMF(pLineCtx, FALSE);
    pCidStruct->status &= ~(VP_CID_MUTE_ON);
    pCidStruct->currentData = VP_FSK_NONE;
    /*
     * This should be unnecessary because it's initialized in the SendCid() and InitCid()
     * functions, but seems like a good/simple precautionary measure.
     */
    pCidStruct->messageDataRemain = 0;

    VpCtrlMuteChannel(pLineCtx, FALSE);

    pCidStruct->status &= ~VP_CID_IN_PROGRESS;
    pLineEvents->process |= VP_LINE_EVID_CID_DATA;
    *pEventData = VP_CID_DATA_TX_DONE;
}   /* VpCliStopCli() */

/**
 * VpCliGetEncodedByte()
 *  This function returns an encoded byte of data that is suitable for writing
 * the FSK generator (device dependent).
 *
 * Preconditions
 *  Must have a valid CLI packet in to work from.
 *
 * Postconditions
 *  The per-channel caller ID buffer will be updated with encoded data.
 */
VpCliEncodedDataType
VpCliGetEncodedByte(
    VpLineCtxType *pLineCtx,
    uint8 *pByte)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            return Vp790CliGetEncodedByte(pLineCtx, pByte);
#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            return Vp880CliGetEncodedByte(pLineCtx, pByte);
#endif

#if defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)
        case VP_DEV_890_SERIES:
            return Vp890CliGetEncodedByte(pLineCtx, pByte);
#endif

        default:
            return VP_CLI_ENCODE_END;
    }
}   /* VpCliGetEncodedByte() */

/**
 * VpCSLACCliGetEncodedByte()
 *  This function provides the common code for VE880/890 silicon for managing
 * the FSK message buffer.
 *
 * Preconditions
 *
 * Postconditions
 */
VpCliEncodedDataType
VpCSLACCliGetEncodedByte(
    uint8 *pByte,
    VpCallerIdType *pCidStruct,
    uint16 *pProcessData,
    VpOptionEventMaskType *pLineEvents,
    uint8 checkSumIndex)
{
    uint8 nextByte = '\0';
    VpCliEncodedDataType returnStatus;

    VP_API_FUNC_INT(None, VP_NULL, ("+VpCSLACCliGetEncodedByte()"));

    /*
     * This logic is entered when the API previously sent the checksum (which
     * of course means it was not provided by the application). See if a
     * mark-out signal is required. If not, end FSK Message Data.
     */
    if (pCidStruct->status & VP_CID_MID_CHECKSUM) {
        VP_CID(None, VP_NULL, ("Just previously sent Checksum"));

        pCidStruct->status &= ~VP_CID_MID_CHECKSUM;
        *pByte = '\0';

        if (pCidStruct->markOutByteCount > 0) {
            /*
             * This means a mark-out signal is needed. If this is the first
             * time knowing this, initialize the value used to keep track of
             * the remaining number of mark bytes to send.
             */
            if (pCidStruct->currentData == VP_FSK_DATA) {
                /*
                 * This is the first time knowing Mark Out Signal is needed.
                 * Initialize the value used to keep track of the remaining
                 * number of mark bytes to send.
                 *
                 * Note that upon return from this function, currentData
                 * should be changed to MARK type assuming a mark signal is
                 * sent. That will avoid constant reset of this value. If
                 * any other type is sent - which would be an error - then
                 * currentData should be set to that type.
                 */
                VP_CID(None, VP_NULL, ("Entering Markout Period..."));
                pCidStruct->markOutByteRemain = pCidStruct->markOutByteCount;
            }

            /* The last markout byte when exists is always the last byte */
            if (pCidStruct->markOutByteRemain == 1) {
                VP_CID(None, VP_NULL, ("1. Last Markout Byte being sent"));
                pCidStruct->status |= VP_CID_END_OF_MSG;
            }
            returnStatus = VP_CLI_ENCODE_MARKOUT;
        } else {
            VP_CID(None, VP_NULL, ("FSK Message Data Complete"));
            pCidStruct->status |= VP_CID_END_OF_MSG;
            returnStatus = VP_CLI_ENCODE_END;
        }

        VP_API_FUNC_INT(None, VP_NULL, ("-VpCSLACCliGetEncodedByte()"));
        return returnStatus;
    }

    /*
     * This logic is entered regardless of where the checksum comes from and
     * doesn't need to check for mark-out signal. That can be done outside this
     * logic.
     */
    /* Check to determine which buffer is in use to index the message data */
    if (pCidStruct->status & VP_CID_PRIMARY_IN_USE) {
        /*
         * If the index is at the length of the buffer, we need to switch
         * buffers if there is more data
         */
        if (pCidStruct->cliMPIndex >= pCidStruct->primaryMsgLen) {
            /*
             * At the end of the Primary Buffer. Flag an event and indicate to
             * the API that this buffer is no longer being used and we can
             * accept more data
             */
            pCidStruct->status &= ~VP_CID_PRIMARY_IN_USE;
            pCidStruct->status &= ~VP_CID_PRIMARY_FULL;

            /*
             * The primary buffer is now empty. If the secondary buffer is full,
             * continue sending CID data. If it is also empty, determine if we
             * need to send the checksum.
             */
            if (pCidStruct->status & VP_CID_SECONDARY_FULL) {
                pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                *pProcessData = VP_CID_DATA_NEED_MORE_DATA;

                pCidStruct->status |= VP_CID_SECONDARY_IN_USE;
                pCidStruct->cliMSIndex = 1;
                *pByte = pCidStruct->secondaryBuffer[0];
                pCidStruct->messageDataRemain -= ((pCidStruct->messageDataRemain) ? 1 : 0);

                nextByte = pCidStruct->secondaryBuffer[1];

                VP_CID(None, VP_NULL,
                    ("CSLAC FSK Buffer -- Switching to Secondary 0x%02X", *pByte));
            } else {
                /*
                 * Secondary buffer is emppty. If a checksum is required flag
                 * the API that the current data being sent is the checksum.
                 * Otherwise, prepare to end FSK message data.
                 */
                if (pCidStruct->pCliProfile[checkSumIndex]) {
                    *pByte = (uint8)(~pCidStruct->cidCheckSum + 1);
                    pCidStruct->status |= VP_CID_MID_CHECKSUM;

                    VP_CID(None, VP_NULL, ("1. Preparing Checksum 0x%02X", *pByte));
                } else {
                    *pByte = '\0';
                }
            }
        } else {
            *pByte = pCidStruct->primaryBuffer[pCidStruct->cliMPIndex];
            pCidStruct->messageDataRemain -= ((pCidStruct->messageDataRemain) ? 1 : 0);

            /* Get the next byte to be sent after the current byte */
            if ((pCidStruct->cliMPIndex+1) >= pCidStruct->primaryMsgLen) {
                if (pCidStruct->status & VP_CID_SECONDARY_FULL) {
                    nextByte = pCidStruct->secondaryBuffer[0];

                    VP_CID(None, VP_NULL,
                        ("CSLAC FSK Buffer -- From Secondary 0x%02X", *pByte));
                }
            } else {
                nextByte = pCidStruct->primaryBuffer[pCidStruct->cliMPIndex+1];

                VP_CID(None, VP_NULL,
                    ("2. CSLAC FSK Buffer -- From Primary 0x%02X", *pByte));
            }
        }
        pCidStruct->cliMPIndex++;
    } else if (pCidStruct->status & VP_CID_SECONDARY_IN_USE) {
        /*
         * If the index is at the length of the buffer, we need to switch
         * buffers if there is more data
         */
        if (pCidStruct->cliMSIndex >= pCidStruct->secondaryMsgLen) {
            /*
             * At the end of the Secondary Buffer. Flag an event and indicate to
             * the API that this buffer is no longer being used and is empty
             */
            pLineEvents->process |= VP_LINE_EVID_CID_DATA;
            *pProcessData = VP_CID_DATA_NEED_MORE_DATA;

            pCidStruct->status &= ~VP_CID_SECONDARY_IN_USE;
            pCidStruct->status &= ~VP_CID_SECONDARY_FULL;

            if (pCidStruct->status & VP_CID_PRIMARY_FULL) {
                pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                *pProcessData = VP_CID_DATA_NEED_MORE_DATA;

                pCidStruct->status |= VP_CID_PRIMARY_IN_USE;
                pCidStruct->cliMPIndex = 1;
                *pByte = pCidStruct->primaryBuffer[0];
                pCidStruct->messageDataRemain -= ((pCidStruct->messageDataRemain) ? 1 : 0);
                nextByte = pCidStruct->primaryBuffer[1];
                VP_CID(None, VP_NULL,
                    ("CSLAC FSK Buffer -- Switching to Primary 0x%02X", *pByte));
            } else {
                /* There is no more data in either buffer */
                /*
                 * If a checksum is required flag the API that the current data
                 * being sent is the checksum. Otherwise, prepare to end FSK
                 * message data.
                 */
                if (pCidStruct->pCliProfile[checkSumIndex]) {
                    *pByte = (uint8)(~pCidStruct->cidCheckSum + 1);
                    pCidStruct->status |= VP_CID_MID_CHECKSUM;
                    VP_CID(None, VP_NULL,
                        ("2. Preparing Checksum 0x%02X", *pByte));
                } else {
                    *pByte = '\0';
                }
            }
        } else {
            *pByte = pCidStruct->secondaryBuffer[pCidStruct->cliMSIndex];
            pCidStruct->messageDataRemain -= ((pCidStruct->messageDataRemain) ? 1 : 0);

            /* Get the next byte to be sent after the current byte */
            if ((pCidStruct->cliMSIndex+1) >= pCidStruct->secondaryMsgLen) {
                if (pCidStruct->status & VP_CID_PRIMARY_FULL) {
                    nextByte = pCidStruct->primaryBuffer[0];
                    VP_CID(None, VP_NULL,
                        ("3. CSLAC FSK Buffer -- From Primary 0x%02X", *pByte));
                }
            } else {
                nextByte = pCidStruct->secondaryBuffer[pCidStruct->cliMSIndex+1];
                VP_CID(None, VP_NULL,
                    ("2. CSLAC FSK Buffer -- From Secondary 0x%02X", *pByte));
            }
        }
        pCidStruct->cliMSIndex++;
    }

    /*
     * Buffers are no longer in use when all user message data has been provided
     * to the silicon. Next can be the API provided checksum or Mark-Out signal
     * as part of FSK Message Data. Step by step...
     */
    if ((!(pCidStruct->status & VP_CID_PRIMARY_IN_USE))
     && (!(pCidStruct->status & VP_CID_SECONDARY_IN_USE))) {
        /*
         * We're here because the FSK message data buffers are empty. We don't
         * yet know if the checksum is provided by the API or if we need to
         * generate a mark-out.
         */
        if (pCidStruct->status & VP_CID_MID_CHECKSUM) {
            /*
             * This means the API is supposed to provide the checksum AND it is
             * now doing that. If we won't run Mark-Out this is the EOM.
             */
            VP_API_FUNC_INT(None, VP_NULL, ("-VpCSLACCliGetEncodedByte()"));

            /* Mark-Out will not be required. This is the EOM */
            if (pCidStruct->markOutByteCount == 0) {
                VP_CID(None, VP_NULL,
                    ("3. Marking EOM with pByte 0x%02X", *pByte));
                pCidStruct->status |= VP_CID_END_OF_MSG;
            }
            returnStatus = VP_CLI_ENCODE_DATA;
        } else {
            /*
             * This means the API may or may not have provided the checksum, but
             * in any case the checksum must have already been sent so we can
             * check if a mark-out signal if needed.
             */
            VP_CID(None, VP_NULL, ("Checking for MarkOut with Next Byte 0x%02X", nextByte));

            if (pCidStruct->markOutByteCount > 0) {
                /*
                 * This means a mark-out signal is needed. If this is the first
                 * time entering this transition, iniitialize the parameter used
                 * to keep track of the number of mark out bytes remaiming.
                 * Note that when this value == 1, it is the EOM byte.
                 */
                if (pCidStruct->currentData == VP_FSK_DATA) {
                    /* This is the first time entering this transition. */
                    /*
                     * Note the handshaking here - that upon return from this
                     * function, currentData should be changed to MARK type to
                     * prevent continued re-initialization of this parameter.
                     */
                    VP_CID(None, VP_NULL, ("Starting Markout with Next Byte 0x%02X", nextByte));
                    pCidStruct->markOutByteRemain = pCidStruct->markOutByteCount;
                    returnStatus = VP_CLI_ENCODE_MARKOUT;
                } else if (pCidStruct->markOutByteRemain >= 1) {
                    /*
                     * The last markout byte when exists is always when only 1 byte
                     * remains, even if only 1 byte was specified.
                     */
                    if (pCidStruct->markOutByteRemain == 1) {
                        VP_CID(None, VP_NULL,
                            ("2. Last Markout Byte being sent"));
                        pCidStruct->status |= VP_CID_END_OF_MSG;
                    }
                    returnStatus = VP_CLI_ENCODE_MARKOUT;
                } else {
                    returnStatus = VP_CLI_ENCODE_END;
                }
            } else {
                /*
                 * This means a mark-out signal is NOT needed so we can end FSK
                 * transmission now.
                 */
                returnStatus = VP_CLI_ENCODE_END;
            }
        }
        VP_API_FUNC_INT(None, VP_NULL, ("-VpCSLACCliGetEncodedByte()"));
        return returnStatus;
    /*
     * In the following cases, a message byte is being sent but detect whether
     * this is the last byte or not. If it's the last byte, flag EOM in the line
     * object which will cause SetFSKGen function to set the device EOM bit.
     * If we don't do this, a mark-byte will be added to the end of the message
     * that was not specified.
     */
            /* No More Bytes in Message Buffers --  */
    } else if ((pCidStruct->messageDataRemain == 0)
            /* AND Checksum Not Required -- */
           && (!(pCidStruct->pCliProfile[checkSumIndex]))
            /* AND Mark-Out Not Required = This is the last byte. */
           && (pCidStruct->markOutByteCount == 0)) {

        VP_CID(None, VP_NULL,
            ("Last Byte (0x%02X) Being Sent - No Checksum - No Markout", *pByte));
        pCidStruct->status |= VP_CID_END_OF_MSG;

            /* The byte now being sent is the checksum -- */
    } else if ((pCidStruct->status & VP_CID_MID_CHECKSUM)
            /* AND Mark-Out Not Required = This is the last byte. */
           && (pCidStruct->markOutByteCount == 0)) {

        VP_CID(None, VP_NULL,
            ("Last Byte is Checksum (0x%02x) Now Being Sent - No Markout Required", *pByte));
        pCidStruct->status |= VP_CID_END_OF_MSG;
    }

    (void)nextByte; /* prevent compiler warnings when debug is not enabled */
    VP_API_FUNC_INT(None, VP_NULL, ("-VpCSLACCliGetEncodedByte()"));
    return VP_CLI_ENCODE_DATA;
}   /* VpCSLACCliGetEncodedByte() */

/**
 * VpCtrlSetCliTone()
 *  This function is called by the API internally to enable or disable the
 * signal generator used for Caller ID.
 *
 * Preconditions:
 *  The line context must be valid
 *
 * Postconditions:
 *  The signal generator used for CID tones is enabled/disabled indicated by
 * the mode parameter passed.
 */
VpStatusType
VpCtrlSetCliTone(
    VpLineCtxType *pLineCtx,
    bool mode)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            return Vp790CtrlSetCliTone(pLineCtx, mode);
#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            return Vp880CtrlSetCliTone(pLineCtx, mode);
#endif

#if defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)
        case VP_DEV_890_SERIES:
            return Vp890CtrlSetCliTone(pLineCtx, mode);
#endif

        default:
            return VP_STATUS_FUNC_NOT_SUPPORTED;
    }
}   /* VpCtrlSetCliTone() */
#endif

/**
 * VpCtrlDetectDTMF()
 *  This function is called by the API internally to enable or disable the
 * system level dtmf detector (if present).
 *
 * Preconditions:
 *  The line context must be valid
 *
 * Postconditions:
 *  If implemented, DTMF detection is enabled (or disabled) in the system
 * services layer. This function does not call the system services function for
 * devices that support internal DTMF detection.
 */
VpStatusType
VpCtrlDetectDTMF(
    VpLineCtxType *pLineCtx,
    bool mode)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;
    VpDeviceIdType deviceId;
    uint8 channelId;
    VpCallerIdType *pCidStruct = VP_NULL;

    void *pDevObj = pDevCtx->pDevObj;
    void *pLineObj = pLineCtx->pLineObj;

    if ((pDevObj == VP_NULL) || (pLineObj == VP_NULL)) {
        return VP_STATUS_INVALID_ARG;
    }

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            deviceId = ((Vp790DeviceObjectType *)pDevObj)->deviceId;
            channelId = ((Vp790LineObjectType *)pLineObj)->channelId;
            pCidStruct = &((Vp790LineObjectType *)pLineObj)->callerId;
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            deviceId = ((Vp880DeviceObjectType *)pDevObj)->deviceId;
            channelId = ((Vp880LineObjectType *)pLineObj)->channelId;
#if defined (VP880_FXS_SUPPORT)
            pCidStruct = &((Vp880LineObjectType *)pLineObj)->callerId;
#endif
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            deviceId = ((Vp890DeviceObjectType *)pDevObj)->deviceId;
            channelId = ((Vp890LineObjectType *)pLineObj)->channelId;
#if defined (VP890_FXS_SUPPORT)
            pCidStruct = &((Vp890LineObjectType *)pLineObj)->callerId;
#endif
            break;
#endif

        default:
            return VP_STATUS_FUNC_NOT_SUPPORTED;
    }

    /*
     * IF enabling DTMF detection, make sure the TX PCM is enabled. Otherwise,
     * return TX/RX to states determined by Mute On/Off and linestate
     */
    if (mode == TRUE) {
        if (pCidStruct != VP_NULL) {
            pCidStruct->status |= VP_CID_AWAIT_TONE;
            pCidStruct->digitDet = VP_DIG_NONE;
        }
        VpCtrlMuteChannel(pLineCtx, TRUE);
        VpSysDtmfDetEnable(deviceId, channelId);
    } else {
        if (pCidStruct != VP_NULL) {
            pCidStruct->status &= ~(VP_CID_AWAIT_TONE);
        }
        VpSysDtmfDetDisable(deviceId, channelId);
        VpCtrlMuteChannel(pLineCtx, FALSE);
    }

    return VP_STATUS_SUCCESS;
}   /* VpCtrlDetectDTMF() */
#endif

/**
 * VpCtrlSetFSKGen()
 *  This function is called by the CID sequencer executed internally by the API
 *
 * Preconditions:
 *  The line context must be valid
 *
 * Postconditions:
 *  The data indicated by mode and data is applied to the line. Mode is used
 * to indicate whether the data is "message", or a special character. The
 * special characters are "channel siezure" (alt. 1/0), "mark" (all 1), or
 * "end of transmission".
 */
#if (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) || \
    (defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)) || \
    (defined (VP_CC_790_SERIES))
bool
VpCtrlSetFSKGen(
    VpLineCtxType *pLineCtx,
    VpCidGeneratorControlType mode,
    uint8 data)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;
    bool returnStatus = FALSE;

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            Vp790CtrlSetFSKGen(pLineCtx, mode, data);
            break;
#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            returnStatus = Vp880CtrlSetFSKGen(pLineCtx, mode, data);
            break;
#endif

#if defined (VP_CC_890_SERIES) && defined (VP890_FXS_SUPPORT)
        case VP_DEV_890_SERIES:
            returnStatus = Vp890CtrlSetFSKGen(pLineCtx, mode, data);
            break;
#endif

        default:
            break;
    }
    return returnStatus;
}   /* VpCtrlSetFSKGen() */

/**
 * VpConvertCharToDigitType()
 *  This function is called by the CID sequencer executed internally by the API.
 * It converts a character to a VpDigitType and is used for functions requiring
 * a VpDigitType specifically.
 *
 * Preconditions:
 *  None. Utility function only.
 *
 * Postconditions:
 *  The character passed is converted/returned as a VpDigitType
 */
VpDigitType
VpConvertCharToDigitType(
    char digit)
{
    VpDigitType vpDig;

    switch(digit) {
        case '0':
            vpDig = VP_DIG_ZERO;
            break;

        case 'A':
            vpDig = VP_DIG_A;
            break;

        case 'B':
            vpDig = VP_DIG_B;
            break;

        case 'C':
            vpDig = VP_DIG_C;
            break;

        case 'D':
            vpDig = VP_DIG_D;
            break;

        case '*':
            vpDig = VP_DIG_ASTER;
            break;

        case '#':
            vpDig = VP_DIG_POUND;
            break;

        default:
            vpDig = (VpDigitType)(digit-48);
            break;
    }
    return vpDig;
}   /* VpConvertCharToDigitType() */

/**
 * VpCtrlSetDTMFGen()
 *  This function sets the DTMF generators of the device and if DTMF message
 * data is in progress, disables the TX/RX PCM highway. If this is the end
 * of DTMF message data transmission, then the TX/RX PCM is re-enabled based
 * on the line state and the tx/rx mode set for "talk" states.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The DTMF signal generators are set to the CID specified level and the digit
 * passed is applied to the line (if mode == VP_CID_GENERATOR_DATA).
 */
void
VpCtrlSetDTMFGen(
    VpLineCtxType *pLineCtx,
    VpCidGeneratorControlType mode,
    VpDigitType digit)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;

    if (mode == VP_CID_GENERATOR_DATA) {
        VpCtrlMuteChannel(pLineCtx, TRUE);
    } else {
        VpCtrlMuteChannel(pLineCtx, FALSE);
    }

    switch (pDevCtx->deviceType) {
#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            Vp880SetDTMFGenerators(pLineCtx, mode, digit);
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            Vp890SetDTMFGenerators(pLineCtx, mode, digit);
            break;
#endif
        default:
            break;
    }
    return;
}   /* VpCtrlSetDTMFGen() */
#endif

/**
 * VpCtrlMuteChannel()
 *  This function disables the TX/RX PCM highway if mode == TRUE, otherwise it
 * enables the TX/RX PCM highway based on line state and the option set for
 * TX/RX enable mode in talk states.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The TX/RX PCM mode is set on the line.
 */
void
VpCtrlMuteChannel(
    VpLineCtxType *pLineCtx,
    bool mode)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            Vp790MuteChannel(pLineCtx, mode);
            break;
#endif

#if defined (VP_CC_880_SERIES)
        case VP_DEV_880_SERIES:
            Vp880MuteChannel(pLineCtx, mode);
            break;
#endif

#if defined (VP_CC_890_SERIES)
        case VP_DEV_890_SERIES:
            Vp890MuteChannel(pLineCtx, mode);
            break;
#endif
        default:
            break;
    }
    return;
}   /* VpCtrlMuteChannel() */

/**
 * VpCSLACInitMeter()
 *  This function is used to initialize metering parameters. See VP-API
 * reference guide for more information.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function.
 *
 * Postconditions:
 *  This function initializes metering parameters as per given profile.
 */
VpStatusType
VpCSLACInitMeter(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pMeterProfile)
{
#if defined(VP_CC_790_SERIES) || (defined(VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT))
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;

    void *pLineObj = pLineCtx->pLineObj;
    void *pDevObj = pDevCtx->pDevObj;

    VpDeviceIdType deviceId;
    VpProfilePtrType pMetering;
    VpCSLACDeviceProfileTableType *pDevProfTable;
    VpProfileDataType *pMpiData;

    uint8 channelId, ecVal, meterProfEntry;
    bool deviceInit = FALSE;
    int tableSize = VP_CSLAC_METERING_PROF_TABLE_SIZE;

    int meterIndex = VpGetProfileIndex(pMeterProfile);

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            deviceId = ((Vp790DeviceObjectType *)pDevObj)->deviceId;
            channelId = ((Vp790LineObjectType *)pLineObj)->channelId;
            pDevProfTable = &((Vp790DeviceObjectType *)pDevObj)->devProfileTable;
            deviceInit = (((Vp790DeviceObjectType *)pDevObj)->status.state
                & VP_DEV_INIT_CMP);
            meterProfEntry =
                ((Vp790DeviceObjectType *)pDevObj)->profEntry.meterProfEntry;

            switch(channelId) {
                case 0: ecVal = VP790_EC_CH1;   break;
                case 1: ecVal = VP790_EC_CH2;   break;
                case 2: ecVal = VP790_EC_CH3;   break;
                case 3: ecVal = VP790_EC_CH4;   break;
                default:
                    return VP_STATUS_FAILURE;
            }
            break;
#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            deviceId = ((Vp880DeviceObjectType *)pDevObj)->deviceId;
            channelId = ((Vp880LineObjectType *)pLineObj)->channelId;
            ecVal = ((Vp880LineObjectType *)pLineObj)->ecVal;
            pDevProfTable = &((Vp880DeviceObjectType *)pDevObj)->devProfileTable;
            deviceInit = (((Vp880DeviceObjectType *)pDevObj)->state
                & VP_DEV_INIT_CMP);

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                deviceInit = FALSE;
            }

            meterProfEntry =
                ((Vp880DeviceObjectType *)pDevObj)->profEntry.meterProfEntry;
            break;
#endif
        default:
            (void)channelId;  /* prevent compiler warnings */
            return VP_STATUS_INVALID_ARG;
    }

    if (!(deviceInit)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * If the profile passed is an index, make sure it's in the valid range
     * and if so, set the currently used profile to it.
     */
    if ((meterIndex >= 0) && (meterIndex < tableSize)) {
        if (!(meterProfEntry & (0x01 << meterIndex))) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_ERR_PROFILE;
        }

        pMetering = pDevProfTable->pMeteringProfileTable[meterIndex];
        /* Valid Cadence Entry. Set it if the profile has been initialized */
        if (pMetering != VP_PTABLE_NULL) {
            pMpiData = (VpProfileDataType *)(&pMetering[VP_PROFILE_MPI_LEN+1]);
            VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
                pMetering[VP_PROFILE_MPI_LEN], pMpiData);
        }
    } else if (meterIndex >= tableSize) {
        /* It's an index, but it's out of range */
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_ERR_PROFILE;
    } else {
        /* This is a valid metering pointer. Set it */
        pMpiData = (VpProfileDataType *)(&pMeterProfile[VP_PROFILE_MPI_LEN+1]);
        VpMpiCmdWrapper(deviceId, ecVal, NOOP_CMD,
            pMeterProfile[VP_PROFILE_MPI_LEN], pMpiData);
    }
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
#endif /* 880 || 790 */
    return VP_STATUS_SUCCESS;
}   /* VpCSLACInitMeter() */

/**
 * VpCSLACStartMeter()
 *  This function starts (can also abort) metering pulses on the line. See VP-API-II documentation
 *  for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized.
 *
 * Postconditions:
 *  Metering pulses are transmitted on the line.
 */
VpStatusType
VpCSLACStartMeter(
    VpLineCtxType *pLineCtx,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters)
{
#if defined(VP_CC_790_SERIES) || (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT))
    VpStatusType status;
    uint16 tickRate;
    uint8 *pIntSequence;
    uint16 bigLoops;
    uint16 remainder;
    uint8 branchTarget;
    VpSeqDataType *pCadence;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    VpDeviceType deviceType = pDevCtx->deviceType;
    VpOptionEventMaskType *pLineEvents;
    VpDeviceIdType deviceId;
    uint16 *pMeterCnt;

    bool deviceInit = FALSE;
    uint8 index;

    VpSetLineStateFuncPtrType SetLineState = pDevCtx->funPtrsToApiFuncs.SetLineState;
    VpProfileCadencerStateTypes profWizCurrentState;
    VpLineStateType currentState;
    uint8 seqLen;

    void *pLineObj = pLineCtx->pLineObj;
    void *pDevObj = pDevCtx->pDevObj;

    switch (deviceType) {
#if defined (VP_CC_790_SERIES)
        case VP_DEV_790_SERIES:
            deviceId = ((Vp790DeviceObjectType *)pDevObj)->deviceId;
            pIntSequence = &((Vp790LineObjectType *)pLineObj)->intSequence[0];
            pCadence = &((Vp790LineObjectType *)pLineObj)->cadence;
            pLineEvents = &((Vp790LineObjectType *)pLineObj)->lineEvents;
            currentState = ((Vp790LineObjectType *)pLineObj)->lineState.currentState;
            seqLen = VP790_INT_SEQ_LEN;
            deviceInit = (((Vp790DeviceObjectType *)pDevObj)->status.state & VP_DEV_INIT_CMP);
            pMeterCnt = &((Vp790LineObjectType *)pLineObj)->processData;
            tickRate = ((Vp790DeviceObjectType *)pDevObj)->devProfileData.tickRate;
            break;
#endif

#if defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)
        case VP_DEV_880_SERIES:
            deviceId = ((Vp880DeviceObjectType *)pDevObj)->deviceId;
            pIntSequence = &((Vp880LineObjectType *)pLineObj)->intSequence[0];
            pCadence = &((Vp880LineObjectType *)pLineObj)->cadence;
            pLineEvents = &((Vp880LineObjectType *)pLineObj)->lineEvents;
            currentState = ((Vp880LineObjectType *)pLineObj)->lineState.currentState;
            seqLen = VP880_INT_SEQ_LEN;
            deviceInit = (((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_INIT_CMP);

            /*
             * Do not proceed if the device calibration is in progress. This could
             * damage the device.
             */
            if (((Vp880DeviceObjectType *)pDevObj)->state & VP_DEV_IN_CAL) {
                deviceInit = FALSE;
            }

            pMeterCnt = &((Vp880LineObjectType *)pLineObj)->processData;
            tickRate = ((Vp880DeviceObjectType *)pDevObj)->devProfileData.tickRate;
            break;
#endif
        default:
            return VP_STATUS_INVALID_ARG;
    }

    if (!(deviceInit)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * If we're not in a valid line state where metering is possible, generate
     * an event that metering was aborted for FXS lines and return success.
     * Error on FXO lines.
     */
    switch(currentState) {
        case VP_LINE_TIP_OPEN:
        case VP_LINE_DISCONNECT:
            pLineEvents->process |= VP_LINE_EVID_MTR_ABORT;
            *pMeterCnt = pCadence->meteringBurst;
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_SUCCESS;

        case VP_LINE_FXO_OHT:
        case VP_LINE_FXO_LOOP_OPEN:
        case VP_LINE_FXO_LOOP_CLOSE:
        case VP_LINE_FXO_TALK:
        case VP_LINE_FXO_RING_GND:
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_INVALID_ARG;

        default:
            break;
    }

    /* Make sure we at least can convert the current state to a Profile Wizard State */
    profWizCurrentState = ConvertApiState2PrfWizState(currentState);
    if (profWizCurrentState == VP_PROFILE_CADENCE_STATE_UNKNOWN) {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_INVALID_ARG;
    }

    /*
     * Number of meters = 0 will stop metering. SetLineState() will manage
     * how and when the metering is actually stopped and generate the
     * appropriate event.
     */
    if (numMeters == 0) {
        SetLineState(pLineCtx, currentState);
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_SUCCESS;
    }

    /* Clear out the internal sequencer */
    VpMemSet(pIntSequence, 0, seqLen);

    /* Stop all other cadences if they were active */
    pCadence->status = VP_CADENCE_RESET_VALUE;

    /*
     * We are starting a new metering session. Reset the metering pulse count
     * that is used if metering is aborted.
     */
    pCadence->meteringBurst = 0;

    /*
     * Clear the pending abort flag.
     */
    pCadence->meterPendingAbort = FALSE;

    /*
     * Build the sequence in the order that it will be executed. Not necessary,
     * but makes reading this code easier.
     */

    /* Set the type of profile to "metering internal" */
    pIntSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_METERING_GEN;

    /* The branch command only allows a branch count up to 255, for a total
     * maximum of 256 iterations in a loop.  To implement numMeters > 256, we
     * will use a nested loop of ((numMeters / 256) * 256) followed by a
     * a section for the remaining (numMeters % 256).
     * Examples:
     *  numMeters = 60000:              numMeters = 500:
     *      [1]                             [1]
     *      <metering>                      <metering>
     *      <branch to 1 x256-1>            <branch to 1 x256-1>
     *      <branch to 1 x234-1>            [2]
     *      [2]                             <metering>
     *      <metering>                      <branch to 2 x244-1>
     *      <branch to 2 x96-1>
     *
     *  numMeters = 256:                numMeters = 200:
     *      [1]                             [2]
     *      <metering>                      <metering>
     *      <branch to 1 x256-1>            <branch to 2 x200-1>
     *
     *  numMeters = 1:
     *      <metering>
     */

    bigLoops = numMeters / 256;
    remainder = numMeters % 256;

    index = 0;

    /* Add the big nested loop of 256 pulses per iteration */
    if (bigLoops > 0) {
        branchTarget = index / 2;
        status = AddMeteringSection(pLineCtx, pIntSequence, &index, tickRate, onTime, offTime, 256);
        if (status != VP_STATUS_SUCCESS) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return status;
        }

        /* If needed, add the outer big loop branch */
        if (bigLoops > 1) {
            pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
                VP_SEQ_SPRCMD_BRANCH_INSTRUCTION | branchTarget;
            index++;

            pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] = bigLoops - 1;
            index++;
        }
    }

    /* Add the remaining pulses */
    if (remainder > 0) {
        status = AddMeteringSection(pLineCtx, pIntSequence, &index, tickRate,
                    onTime, offTime, remainder);
        if (status != VP_STATUS_SUCCESS) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return status;
        }
    }

    /*
     * When the metering is complete, return to the current line state. Note
     * that this step is not reached if metering is infinite (on-time = 0)
     */
    pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
        (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);
    index++;
    pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] = profWizCurrentState;
    index++;

    pIntSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = index;

    /*
     * Profile Length is always sequence lengh + 4 because of header
     * definition.
     */

    pIntSequence[VP_PROFILE_LENGTH] = pIntSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] + 4;
    pCadence->index = VP_PROFILE_TYPE_SEQUENCER_START;
    pCadence->length = pIntSequence[VP_PROFILE_LENGTH];

    pCadence->pActiveCadence = &pIntSequence[0];
    pCadence->pCurrentPos = &pIntSequence[8];

    pCadence->status |= VP_CADENCE_STATUS_ACTIVE;
    pCadence->status |= VP_CADENCE_STATUS_METERING;

#if (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER)
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Metering Cadence:"));
    for (index = 0; index < pIntSequence[VP_PROFILE_LENGTH]; index++) {
        VP_SEQUENCER(VpLineCtxType, pLineCtx,
                     (" 0x%02X", pIntSequence[VP_PROFILE_LENGTH + index + 1]));
    }
#endif /* (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER) */

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
#endif /* 790 || 880 */
    return VP_STATUS_SUCCESS;
}   /* VpCSLACStartMeter() */

#if defined(VP_CC_790_SERIES) || (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT))
/**
 * AddMeteringSection()
 *  This is a support function for VpCSLACStartMeter().
 */
static VpStatusType
AddMeteringSection(
    VpLineCtxType *pLineCtx,
    uint8 *pIntSequence,
    uint8 *pIndex,
    uint16 tickRate,
    uint16 onTime,
    uint16 offTime,
    uint16 numMeters)
{
    uint16 tempTime;
    uint32 timeRemaining;
    uint8 tickAdjust;
    uint8 branchTarget;
    uint8 index;

    index = *pIndex;

    branchTarget = index / 2;
    /* The maximum branch target that can be specified is 31, 0x1F.  To exceed
     * that, VpStartMeter() would need to be called with a total on+off time of
     * at least 110579, or 1105.79 seconds, and numMeters > 256. With a maximum
     * offtime of 65535 (655.35 seconds), that would still require an ontime of
     * 45044 (450.44 seconds), so this is an impractical case anyway. */
    if (branchTarget > 0x1F) {
        VP_ERROR(VpLineCtxType, pLineCtx,
            ("Could not build metering cadence with excessive on/off times (%d/%d) and numMeters > 256.",
            onTime, offTime));
        return VP_STATUS_FAILURE;
    }

    /* And set the starting command to start metering */
    pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_METERING);
    index++;
    pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START + index] = 0x01;
    index++;

    /*
     * Set the metering on time after metering is enabled. Metering time is
     * specified in 10ms incr, Cadence in 5ms.
     */
    timeRemaining = onTime * 2;

    if (timeRemaining == 0) {
        /* Infinite on-time */
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
            VP_SEQ_SPRCMD_TIME_INSTRUCTION;
        index++;
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] = 0x00;
        index++;
    } else {
        /* Adjust for tick timing.  The processing of the metering off command will
         * extend the on-time by one tick. */
        tickAdjust = TICKS_TO_MS(1, tickRate);
        if (tickAdjust % 5 > 2) {
            tickAdjust = (tickAdjust / 5) + 1;
        } else {
            tickAdjust = (tickAdjust / 5);
        }
        if (timeRemaining > tickAdjust) {
            timeRemaining -= tickAdjust;
        } else {
            timeRemaining = 0;
        }
    }

    /* The maximum time representable by a uint16 in 10ms units is 655.35
     * seconds.  One cadence time operator gives us at most 40.955 seconds, so
     * we potentially need to put several together to achieve longer times. */
    while (timeRemaining) {
        if (timeRemaining > 0x1FFF) {
            tempTime = 0x1FFF;
            timeRemaining -= 0x1FFF;
        } else {
            tempTime = timeRemaining;
            timeRemaining = 0;
        }

        /* Add time command (2 bytes) */
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
            VP_SEQ_SPRCMD_TIME_INSTRUCTION;
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] |=
            (tempTime & 0x1F00) >> 8;
        index++;
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
            (tempTime & 0x00FF);
        index++;
    }

    /*
     * If the on-time is = 0, the Sequencer automatically suspends indefinitely
     * at the current state. Otherwise, it will proceed to the next step.
     */

    /* Then turn the metering off */
    pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_METERING);
    index++;

    pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] = 0x00;
    index++;

    /*
     * Set the metering on time after metering is enabled. Metering time is
     * specified in 10ms incr, Cadence in 5ms.
     */
    timeRemaining = offTime * 2;

    if (timeRemaining == 0) {
        /* Infinite off-time */
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
            VP_SEQ_SPRCMD_TIME_INSTRUCTION;
        index++;
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] = 0x00;
        index++;
    } else {
        /* Adjust for tick timing.  The processing of the branch and metering on
         * commands will extend the off-time by two ticks. */
        tickAdjust = TICKS_TO_MS(2, tickRate);
        if (tickAdjust % 5 > 2) {
            tickAdjust = (tickAdjust / 5) + 1;
        } else {
            tickAdjust = (tickAdjust / 5);
        }
        if (timeRemaining > tickAdjust) {
            timeRemaining -= tickAdjust;
        } else {
            timeRemaining = 0;
        }
    }

    /* The maximum time representable by a uint16 in 10ms units is 655.35
     * seconds.  One cadence time operator gives us at most 40.955 seconds, so
     * we potentially need to put several together to achieve longer times. */
    while (timeRemaining) {
        if (timeRemaining > 0x1FFF) {
            tempTime = 0x1FFF;
            timeRemaining -= 0x1FFF;
        } else {
            tempTime = timeRemaining;
            timeRemaining = 0;
        }

        /* Add time command (2 bytes) */
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
            VP_SEQ_SPRCMD_TIME_INSTRUCTION;
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] |=
            (tempTime & 0x1F00) >> 8;
        index++;
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
            (tempTime & 0x00FF);
        index++;
    }

    /*
     * Condition of numMeters = 0 (stop metering) is taken care of at top.
     * Then, condition numMeters != 0 and on-time is (meter forever) is taken
     * care of as a consequence of the Sequencer (i.e., suspend for any time
     * operator set to 0).
     * All other operators will count some number of metering pulses, end, then
     * go back to the state the line was in when the sequence started.
     */

    /* Can only be 1 or > 1, not 0 */
    if (numMeters > 1) {
        /*
         * If more than 1, we'll branch back to the start of this metering
         * section until all metering pulses occur.
         */
        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] =
            VP_SEQ_SPRCMD_BRANCH_INSTRUCTION | branchTarget;
        index++;

        pIntSequence[VP_PROFILE_TYPE_SEQUENCER_START+index] = numMeters - 1;
        index++;
    } else {
        /* If exactly equal to 1, no branch is necessary. */
    }

    *pIndex = index;

    return VP_STATUS_SUCCESS;
}   /* AddMeteringSection() */
#endif /* defined(VP_CC_790_SERIES) || (defined (VP_CC_880_SERIES) && defined (VP880_FXS_SUPPORT)) */

#endif /* defined(VP_CC_790_SERIES) || defined(VP_CC_880_SERIES) || \
          defined(VP_CC_890_SERIES) || defined(VP_CC_580_SERIES) */

#endif  /* VP_CSLAC_SEQ_EN */
