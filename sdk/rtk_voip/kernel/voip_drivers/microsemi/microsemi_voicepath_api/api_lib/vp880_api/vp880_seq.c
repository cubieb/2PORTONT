/** \file vp880_seq.c
 * vp880_seq.c
 *
 *  This file contains the VP880 functions called by the API-II Caller ID or
 * sequencer. It is seperated from "normal" API functions for users that want
 * to remove this section of code from the API-II.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10011 $
 * $LastChangedDate: 2012-05-16 16:41:01 -0500 (Wed, 16 May 2012) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)
#ifdef VP_CSLAC_SEQ_EN

/* INCLUDES */
#include "vp_api_types.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "sys_service.h"

/**< Function called by Send Signal only. Implements message waiting pulse. */
#if defined (VP880_FXS_SUPPORT)
static VpStatusType
Vp880SendMsgWaitPulse(
    VpLineCtxType *pLineCtx,
    VpSendMsgWaitType *pMsgWait);

/**< Function called by Send Signal only. Implements Polarity Reversal Pulse */
VpStatusType
Vp880SendPolRevPulse(
    VpLineCtxType *pLineCtx,
    uint16 timeIn1Ms);

static VpStatusType
Vp880SendPulse(
    VpLineCtxType *pLineCtx,
    VpSendSignalType type,
    uint16 timeInMs);
#endif

/**< Function called by Send Signal only. Implements Forward Disconnect and
 * Tip Open pulse.
 */
#if defined (VP880_FXO_SUPPORT)
/**< Function called by Send Signal only. Implements FXO digit generation */
static VpStatusType
Vp880SendDigit(
    VpLineCtxType *pLineCtx,
    VpDigitGenerationType digitType,
    VpDigitType digit);

/**< Function called by Send Signal only. Implements FXO Momentary Loop Open */
static VpStatusType
Vp880MomentaryLoopOpen(
    VpLineCtxType *pLineCtx);
#endif

/**
 * Vp880CommandInstruction()
 *  This function implements the Sequencer Command instruction for the Vp880
 * device type.
 *
 * Preconditions:
 *  The line must first be initialized and the sequencer data must be valid.
 *
 * Postconditions:
 *  The command instruction currently being pointed to by the sequencer
 * instruction passed is acted upon.  The sequencer may or may not be advanced,
 * depending on the specific command instruction being executed.
 */
VpStatusType
Vp880CommandInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = pLineObj->ecVal;
    uint8 channelId = pLineObj->channelId;
    uint8 sigGenCtrl;

#ifdef VP880_FXS_SUPPORT
    uint8 lineState;
    uint8 lsConfig[VP880_LOOP_SUP_LEN];
#endif

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880CommandInstruction()"));

    /*
     * We know the current value "pSeqData[0]" is 0, now we need to determine if
     * the next command is generator control operator followed by time, or a
     * Line state command -- No other options supported
     */
    switch (pSeqData[0] & VP_SEQ_SUBTYPE_MASK) {
        case VP_SEQ_SUBCMD_SIGGEN:
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Generator Control 0x%02X 0x%02X",
                pSeqData[0], pSeqData[1]));

            sigGenCtrl = VP880_GEN_ALLOFF;

            /* Get the signal generator bits and set. */
            sigGenCtrl |= ((pSeqData[1] & 0x01) ?  VP880_GENA_EN : 0);
            sigGenCtrl |= ((pSeqData[1] & 0x02) ?  VP880_GENB_EN : 0);
            sigGenCtrl |= ((pSeqData[1] & 0x04) ?  VP880_GENC_EN : 0);
            sigGenCtrl |= ((pSeqData[1] & 0x08) ?  VP880_GEND_EN : 0);

            if (sigGenCtrl != pLineObj->sigGenCtrl[0]) {
                pLineObj->sigGenCtrl[0] = sigGenCtrl;
                VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT,
                    VP880_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }
            break;

        case VP_SEQ_SUBCMD_LINE_STATE:
            VP_SEQUENCER(VpLineCtxType, pLineCtx,
                ("Line State Control 0x%02X 0x%02X at Time %d",
                pSeqData[0], pSeqData[1], pDevObj->timeStamp));

            switch(pSeqData[1]) {
#ifdef VP880_FXS_SUPPORT
                case VP_PROFILE_CADENCE_STATE_MSG_WAIT_NORM:
                case VP_PROFILE_CADENCE_STATE_MSG_WAIT_POLREV:
                    VpMemCpy(lsConfig, pLineObj->loopSup, VP880_LOOP_SUP_LEN);
                    if (lsConfig[VP880_LOOP_SUP_RT_MODE_BYTE]
                        & VP880_RING_TRIP_AC) {
                        if (!(pLineObj->status & VP880_BAD_LOOP_SUP)) {
                            pLineObj->status |= VP880_BAD_LOOP_SUP;
                        }

                        /* Force DC Trip */
                        lsConfig[VP880_LOOP_SUP_RT_MODE_BYTE] &=
                            ~VP880_RING_TRIP_AC;
                        VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT,
                            VP880_LOOP_SUP_LEN, lsConfig);
                    }

                    lineState =
                        (pSeqData[1] == VP_PROFILE_CADENCE_STATE_MSG_WAIT_NORM) ?
                        VP880_SS_BALANCED_RINGING :
                        VP880_SS_BALANCED_RINGING_PR;

                    Vp880LLSetSysState(deviceId, pLineCtx, lineState, TRUE);
                    break;
#endif

                default:
                    Vp880SetLineStateInt(pLineCtx,
                        ConvertPrfWizState2ApiState(pSeqData[1]));
                    break;
            }
            break;

#ifdef VP880_FXS_SUPPORT
        case VP_SEQ_SUBCMD_START_CID:
        case VP_SEQ_SUBCMD_WAIT_ON:
            if (pLineObj->pCidProfileType1 != VP_PTABLE_NULL) {
                pLineObj->callerId.pCliProfile = pLineObj->pCidProfileType1;
                VpCSLACInitCidStruct(&pLineObj->callerId, pSeqData[0]);
            }
            break;

        case VP_SEQ_SUBCMD_RAMP_GENERATORS:
            if(VpCSLACProcessRampGenerators(&pLineObj->cadence)) {
#if (VP_CC_DEBUG_SELECT & VP_DBG_SEQUENCER)
                uint16 actualLevel = pLineObj->cadence.regData[5];
                actualLevel = ((actualLevel << 8) & 0xFF00);
                actualLevel |= pLineObj->cadence.regData[6];

                VP_SEQUENCER(VpLineCtxType, pLineCtx,
                                 ("New Signal Generator Values: 0x%02X 0x%02X 0x%02X 0x%02X time %d Level (%d)",
                                  pLineObj->cadence.regData[3], pLineObj->cadence.regData[4],
                                  pLineObj->cadence.regData[5], pLineObj->cadence.regData[6],
                                  pDevObj->timeStamp, actualLevel));
                if (pLineObj->cadence.levelStep == 0) {
                    VP_SEQUENCER(VpLineCtxType, pLineCtx,
                                 ("Ramp complete at time %d", pDevObj->timeStamp));

                }
#endif
                VpMpiCmdWrapper(deviceId, ecVal, VP880_SIGA_PARAMS_WRT,
                    VP880_SIGA_PARAMS_LEN, pLineObj->cadence.regData);
                /* Clear flag to indicate the generators are NOT in a Ringing Mode */
                pLineObj->status &= ~(VP880_RING_GEN_NORM | VP880_RING_GEN_REV);
            }
            break;

        case VP_SEQ_SUBCMD_METERING:
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Metering Control 0x%02X 0x%02X",
                pSeqData[0], pSeqData[1]));

            if (pSeqData[1]) {  /* Metering On */
                pLineObj->slicValueCache |= VP880_SS_METERING_MASK;
                pLineObj->cadence.meteringBurst++;
            } else {    /* Metering Off */
                pLineObj->slicValueCache &= ~VP880_SS_METERING_MASK;
                if (pLineObj->cadence.meterPendingAbort) {
                    Vp880SetLineState(pLineCtx, pLineObj->cadence.meterAbortLineState);
                }
            }

            VpMpiCmdWrapper(deviceId, ecVal, VP880_SLIC_STATE_WRT,
                VP880_SLIC_STATE_LEN, &pLineObj->slicValueCache);
            break;
#endif
        default:
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880CommandInstruction()"));
            return VP_STATUS_INVALID_ARG;
    }

    /*
     * Check to see if there is more sequence data, and if so, move the
     * sequence pointer to the next command. Otherwise, end this cadence
     */
    pLineObj->cadence.index+=2;
    if (pLineObj->cadence.index <
       (pLineObj->cadence.length + VP_PROFILE_LENGTH + 1)) {
        pSeqData+=2;
        pLineObj->cadence.pCurrentPos = pSeqData;
    } else {
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Ending Cadence Length %d at Index %d",
            pLineObj->cadence.length, pLineObj->cadence.index));
        switch(pLineObj->cadence.pActiveCadence[VP_PROFILE_TYPE_LSB]) {
            case VP_PRFWZ_PROFILE_METERING_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_MTR_CMP;
                break;

            case VP_PRFWZ_PROFILE_RINGCAD:
                pLineObj->lineEvents.process |= VP_LINE_EVID_RING_CAD;
                pLineObj->processData = VP_RING_CAD_DONE;
                break;

            case VP_PRFWZ_PROFILE_TONECAD:
                pLineObj->lineEvents.process |= VP_LINE_EVID_TONE_CAD;
                break;

            case VP_PRFWZ_PROFILE_HOOK_FLASH_DIG_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_HOOK_FLASH;
                break;

            case VP_PRFWZ_PROFILE_DIAL_PULSE_DIG_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_PULSE_DIGIT;
                break;

            case VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_MOMENTARY_LOOP_OPEN;
                if (pDevObj->intReg[channelId] & VP880_LIU1_MASK) {
                    pLineObj->lineEventHandle = 1;
                } else {
                    pLineObj->lineEventHandle = 0;
                }
                VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT,
                    VP880_LOOP_SUP_LEN, pLineObj->loopSup);
                break;

            case VP_PRFWZ_PROFILE_DTMF_DIG_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_DTMF_DIGIT;
                Vp880MuteChannel(pLineCtx, FALSE);
                break;

            case VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_MSG_WAIT_PULSE;
                VpSetLineState(pLineCtx, pLineObj->lineState.usrCurrent);
                break;

            default:
                break;

        }
        pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
        pLineObj->cadence.pActiveCadence = VP_PTABLE_NULL;
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880CommandInstruction()"));
    return VP_STATUS_SUCCESS;
}

#ifdef VP880_FXS_SUPPORT
/**
 * Vp880InitRing()
 *  This function is used to initialize the ringing profile and caller ID
 * cadence on a given line.
 *
 * Preconditions:
 *  The device associated with this line must be initialized.
 *
 * Postconditions:
 *  The line pointed to by the line context passed is initialized with the
 * ringing and caller ID profile specified.  The profiles may be specified as
 * either an index into the devic profile table or by profile pointers. This
 * function returns the success code if the device has been initialized and both
 * indexes (if indexes are passed) are within the range of the device profile
 * table.
 */
VpStatusType
Vp880InitRing(
    VpLineCtxType *pLineCtx,        /**< Line Context to modify Ringing
                                     * Parameters for
                                     */
    VpProfilePtrType pCadProfile,   /**< Pointer of a Ringing Cadence profile,
                                     * or the index into the Ringing Cadence
                                     * profile table.
                                     */
    VpProfilePtrType pCidProfile)   /**< Pointer of a Caller ID profile, or the
                                     * index into the Caller ID profile table.
                                     */
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp880InitRing()"));

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitRing()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->state & VP_DEV_IN_CAL) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitRing()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pLineObj->status & VP880_IS_FXO) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitRing()"));
        return VP_STATUS_INVALID_ARG;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /* Check the legality of the Ring CAD profile */
    if (!VpCSLACIsProfileValid(VP_PROFILE_RINGCAD,
        VP_CSLAC_RING_CADENCE_PROF_TABLE_SIZE,
        pDevObj->profEntry.ringCadProfEntry,
        pDevObj->devProfileTable.pRingingCadProfileTable,
        pCadProfile, &pLineObj->pRingingCadence))
    {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitRing()"));
        return VP_STATUS_ERR_PROFILE;
    }

    /* Check the legality of the Ring CID profile */
    if (!VpCSLACIsProfileValid(VP_PROFILE_CID,
        VP_CSLAC_CALLERID_PROF_TABLE_SIZE,
        pDevObj->profEntry.cidCadProfEntry,
        pDevObj->devProfileTable.pCallerIdProfileTable,
        pCidProfile, &pLineObj->pCidProfileType1))
    {
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitRing()"));
        return VP_STATUS_ERR_PROFILE;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitRing()"));
    return VP_STATUS_SUCCESS;
} /* Vp880InitRing */

/**
 * Vp880InitCid()
 *  This function is used to send caller ID information. It accepts an amount of
 * CID message data up to a "full" buffer amount (2 times the amount of the
 * size used for ContinueCID). It low fills the primary buffer such that the
 * application is interrupted at the earliest time when the API is ready to
 * accept more data.
 *
 * Preconditions:
 *  The device and line context must be created and initialized before calling
 * this function. This function needs to be called before placing the line in to
 * ringing state.
 *
 * Postconditions:
 *  This function transmits the given CID information on the line (when the line
 * is placed in the ringing state).
 */
VpStatusType
Vp880InitCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    uint8p pCidData)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 primaryByteCount, secondaryByteCount;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp880InitCid()"));

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitCid()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->state & VP_DEV_IN_CAL) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitCid()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if ((pLineObj->status & VP880_IS_FXO) || (length > (2 * VP_SIZEOF_CID_MSG_BUFFER))) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitCid()"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitCid()"));
        return VP_STATUS_SUCCESS;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    pLineObj->callerId.cliIndex = 0;
    pLineObj->callerId.cliMPIndex = 0;
    pLineObj->callerId.cliMSIndex = 0;

    /* Stop CID if it was in progress */
    pLineObj->callerId.cliTimer = 0;
    pLineObj->callerId.status = VP_CID_RESET_VALUE;
    pLineObj->suspendCid = FALSE;
    pLineObj->callerId.dtmfStatus = VP_CID_DTMF_RESET_VALUE;
    pLineObj->callerId.cidCheckSum = 0;
    pLineObj->callerId.messageDataRemain = length;

    /*
     * If length is within the size of just the primary buffer size, then only
     * fill the primary buffer. Otherwise (the length exceeds the size of the
     * primary buffer size) "low fill" the primary buffer and max fill the
     * secondary buffer. This has the affect of causing a CID Data event
     * quickly and giving the application a maximum amount of time to refill
     * the message buffer
     */
    if (length <= VP_SIZEOF_CID_MSG_BUFFER) {
        pLineObj->callerId.primaryMsgLen = length;
        pLineObj->callerId.secondaryMsgLen = 0;
    } else {
        pLineObj->callerId.primaryMsgLen = (length - VP_SIZEOF_CID_MSG_BUFFER);
        pLineObj->callerId.secondaryMsgLen = VP_SIZEOF_CID_MSG_BUFFER;
    }

    /*
     * Copy the message data to the primary API buffer. If we're here, there's
     * at least one byte of primary message data. So a check is not necessary
     */
    pLineObj->callerId.status |= VP_CID_PRIMARY_FULL;
    for (primaryByteCount = 0;
         (primaryByteCount < pLineObj->callerId.primaryMsgLen);
         primaryByteCount++) {
        pLineObj->callerId.primaryBuffer[primaryByteCount]
            = pCidData[primaryByteCount];
        pLineObj->callerId.cidCheckSum += pCidData[primaryByteCount];
        pLineObj->callerId.cidCheckSum = pLineObj->callerId.cidCheckSum % 256;
    }

    /* Copy the message data to the secondary API buffer if there is any */
    if (pLineObj->callerId.secondaryMsgLen > 0) {
        pLineObj->callerId.status |= VP_CID_SECONDARY_FULL;
        for (secondaryByteCount = 0;
             (secondaryByteCount < pLineObj->callerId.secondaryMsgLen);
             secondaryByteCount++) {
            pLineObj->callerId.secondaryBuffer[secondaryByteCount] =
                pCidData[secondaryByteCount + primaryByteCount];
            pLineObj->callerId.cidCheckSum +=
                pCidData[secondaryByteCount + primaryByteCount];
            pLineObj->callerId.cidCheckSum =
                pLineObj->callerId.cidCheckSum % 256;
        }
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880InitCid()"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880SendCid()
 *  This function may be used to send Caller ID information on-demand. It
 * accepts an amount of CID message data up to a "full" buffer amount (2 times
 * the amount of the size used for ContinueCID). It low fills the primary buffer
 * such that the application is interrupted at the earliest time when the API
 * is ready to accept more data.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. The length of the
 * message (indicated by the length field passed) must not exceed the buffer
 * size.
 *
 * Postconditions:
 * Caller ID information is transmitted on the line.
 */
VpStatusType
Vp880SendCid(
    VpLineCtxType *pLineCtx,        /**< Line to send CID on */
    uint8 length,                   /**< Length of the current message data, not
                                     * to exceed the buffer size
                                     */
    VpProfilePtrType pCidProfile,   /**< CID Profile or Profile index to use */
    uint8p pCidData)                /**< CID Message data */
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpProfilePtrType pCidProfileLocal;

    uint8 primaryByteCount, secondaryByteCount;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp880SendCid()"));

    /* Proceed only if line has been initialized */
    if (!(pLineObj->status & VP880_INIT_COMPLETE)) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendCid()"));
        return VP_STATUS_LINE_NOT_CONFIG;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->state & VP_DEV_IN_CAL) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if ((pLineObj->status & VP880_IS_FXO) || (length > (2 * VP_SIZEOF_CID_MSG_BUFFER))) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendCid()"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendCid()"));
        return VP_STATUS_SUCCESS;
    }

    /* Check the legality of the CID profile */
    if (!VpCSLACIsProfileValid(VP_PROFILE_CID,
        VP_CSLAC_CALLERID_PROF_TABLE_SIZE,
        pDevObj->profEntry.cidCadProfEntry,
        pDevObj->devProfileTable.pCallerIdProfileTable,
        pCidProfile, &pCidProfileLocal))
    {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendCid()"));
        return VP_STATUS_ERR_PROFILE;
    }

    if (pCidProfileLocal == VP_PTABLE_NULL) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendCid()"));
        return VP_STATUS_ERR_PROFILE;
    }

    /* If we're here, all parameters passed are valid */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VpCSLACInitCidStruct(&pLineObj->callerId, 0);

    pLineObj->callerId.pCliProfile = pCidProfileLocal;
    pLineObj->callerId.status &= ~VP_CID_REPEAT_MSG;
    pLineObj->callerId.status &= ~VP_CID_END_OF_MSG;
    pLineObj->callerId.cidCheckSum = 0;
    pLineObj->callerId.messageDataRemain = length;

    /*
     * If length is within the size of just the primary buffer size, then only
     * fill the primary buffer. Otherwise (the length exceeds the size of the
     * primary buffer size) "low fill" the primary buffer and max fill the
     * secondary buffer. This has the affect of causing a CID Data event
     * quickly and giving the application a maximum amount of time to refill
     * the message buffer
     */
    if (length <= VP_SIZEOF_CID_MSG_BUFFER) {
        pLineObj->callerId.primaryMsgLen = length;
        pLineObj->callerId.secondaryMsgLen = 0;
    } else {
        pLineObj->callerId.primaryMsgLen = (length - VP_SIZEOF_CID_MSG_BUFFER);
        pLineObj->callerId.secondaryMsgLen = VP_SIZEOF_CID_MSG_BUFFER;
    }

    /*
     * Copy the message data to the primary API buffer. If we're here, there's
     * at least one byte of primary message data. So a check is not necessary
     */
    pLineObj->callerId.status |= VP_CID_PRIMARY_FULL;
    for (primaryByteCount = 0;
         (primaryByteCount < pLineObj->callerId.primaryMsgLen);
         primaryByteCount++) {
        pLineObj->callerId.primaryBuffer[primaryByteCount]
            = pCidData[primaryByteCount];
        pLineObj->callerId.cidCheckSum += pCidData[primaryByteCount];
        pLineObj->callerId.cidCheckSum = pLineObj->callerId.cidCheckSum % 256;
    }

    /* Copy the message data to the secondary API buffer if there is any */
    if (pLineObj->callerId.secondaryMsgLen > 0) {
        pLineObj->callerId.status |= VP_CID_SECONDARY_FULL;
        for (secondaryByteCount = 0;
             (secondaryByteCount < pLineObj->callerId.secondaryMsgLen);
             secondaryByteCount++) {
            pLineObj->callerId.secondaryBuffer[secondaryByteCount] =
                pCidData[secondaryByteCount + primaryByteCount];
            pLineObj->callerId.cidCheckSum +=
                pCidData[secondaryByteCount + primaryByteCount];
            pLineObj->callerId.cidCheckSum =
                pLineObj->callerId.cidCheckSum % 256;
        }
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendCid()"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880ContinueCid()
 *  This function is called to provide more caller ID data (in response to
 * Caller ID data event from the VP-API). See VP-API-II  documentation
 * for more information about this function.
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 * devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Continues to transmit Caller ID information on the line.
 */
VpStatusType
Vp880ContinueCid(
    VpLineCtxType *pLineCtx,    /**< Line to continue CID on */
    uint8 length,               /**< Length of data passed not to exceed the
                                 * buffer length
                                 */
    uint8p pCidData)            /**< CID message data */
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 byteCount = 0;
    uint8 *pMsgLen;
    uint8 *pBuffer;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    pLineObj->callerId.status &= ~VP_CID_END_OF_MSG;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp880ContinueCid()"));

    /* Proceed only if line has been initialized */
    if (!(pLineObj->status & VP880_INIT_COMPLETE)) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880ContinueCid()"));
        return VP_STATUS_LINE_NOT_CONFIG;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->state & VP_DEV_IN_CAL) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880ContinueCid()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if ((pLineObj->status & VP880_IS_FXO) || (length >  VP_SIZEOF_CID_MSG_BUFFER)) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880ContinueCid()"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880ContinueCid()"));
        return VP_STATUS_SUCCESS;
    }

    /*
     * When this function is called, the buffer that is in use is flagged
     * by the VpCliGetEncodeByte() function in vp_api_common.c file. That
     * function implements the logic of when to switch between the primary
     * and secondary buffer. This function just needs to fill the bufffer that
     * is not currently in use, starting with the primary (because the primary
     * buffer is also used first for the first part of the message).
     */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (!(pLineObj->callerId.status & VP_CID_PRIMARY_IN_USE)) {
        /* Fill the primary buffer */
        pLineObj->callerId.status |= VP_CID_PRIMARY_FULL;
        pMsgLen = &(pLineObj->callerId.primaryMsgLen);
        pBuffer = &(pLineObj->callerId.primaryBuffer[0]);
    } else {
        /* Fill the secondary buffer */
        pLineObj->callerId.status |= VP_CID_SECONDARY_FULL;
        pMsgLen = &(pLineObj->callerId.secondaryMsgLen);
        pBuffer = &(pLineObj->callerId.secondaryBuffer[0]);
    }

    *pMsgLen = length;
    pLineObj->callerId.messageDataRemain += length;

    /* Copy the message data to the API buffer */
    for (byteCount = 0; (byteCount < *pMsgLen); byteCount++) {
        pBuffer[byteCount] = pCidData[byteCount];

        pLineObj->callerId.cidCheckSum += pBuffer[byteCount];
        pLineObj->callerId.cidCheckSum = pLineObj->callerId.cidCheckSum % 256;
    }
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880ContinueCid()"));
    return VP_STATUS_SUCCESS;
} /* Vp880ContinueCid() */

/**
 * Vp880CtrlSetCliTone()
 *  This function is called by the API internally to enable or disable the
 * signal generator used for Caller ID.
 *
 * Preconditions:
 *  The line context must be valid (pointing to a Vp880 line object type
 *
 * Postconditions:
 *  The signal generator used for CID tones is enabled/disabled indicated by
 * the mode parameter passed.
 */
VpStatusType
Vp880CtrlSetCliTone(
    VpLineCtxType *pLineCtx,    /**< Line affected by the CLI tones */
    bool mode)                  /**< TRUE = enabled, FALSE = disable tones */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = pLineObj->ecVal;

    /* Pre-clear everything in the Signal Generator. Always set for Continuous */
    uint8 sigGenCtrl = 0;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880CtrlSetCliTone()"));

    /*
     * This function should only be called when the Caller ID sequence is
     * generating an alerting tone. We're using the C/D generators, so disable
     * A/B and enable C/D only (if mode == TRUE).
     */
    if (mode == TRUE) {
        sigGenCtrl |= (VP880_GENC_EN | VP880_GEND_EN);
    }

    if (sigGenCtrl != pLineObj->sigGenCtrl[0]) {
        pLineObj->sigGenCtrl[0] = sigGenCtrl;
        VP_CID(VpLineCtxType, pLineCtx, ("Writing 0x%02X to SignGen Ctrl", sigGenCtrl));
        VpMpiCmdWrapper(deviceId, ecVal, VP880_GEN_CTRL_WRT, VP880_GEN_CTRL_LEN,
            pLineObj->sigGenCtrl);
    }
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880CtrlSetCliTone()"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880CtrlSetFSKGen()
 *  This function is called by the CID sequencer executed internally by the API
 *
 * Preconditions:
 *  The line context must be valid (pointing to a VP880 line object type
 *
 * Postconditions:
 *  The data indicated by mode and data is applied to the line. Mode is used
 * to indicate whether the data is "message", or a special character. The
 * special characters are "channel siezure" (alt. 1/0), "mark" (all 1), or
 * "end of transmission".
 */
bool
Vp880CtrlSetFSKGen(
    VpLineCtxType *pLineCtx,        /**< Line affected by the mode and data */
    VpCidGeneratorControlType mode, /**< Indicates the type of data being sent.
                                     * Affects the start and stop bit used
                                     */
    uint8 data)                     /**< 8-bit message data */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = pLineObj->ecVal;
    bool moreData = TRUE;
    bool initFsk = FALSE;
    uint8 fskParam[VP880_CID_PARAM_LEN];
    bool returnStatus = FALSE;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880CtrlSetFSKGen()"));

    fskParam[0] = pLineObj->tickBeginState[0];
    fskParam[0] &= ~(VP880_CID_FRAME_BITS);
    if (fskParam[0] & VP880_CID_DIS) {
        initFsk = TRUE;
    }

    switch(mode) {
        case VP_CID_SIGGEN_EOT:
            VP_CID(VpLineCtxType, pLineCtx,
                ("Vp880CtrlSetFSKGen(): VP_CID_SIGGEN_EOT %d at time %d with tickBegin 0x%02X",
                data, pDevObj->timeStamp, pLineObj->tickBeginState[0]));
            /*
             * If CID State is in IDLE, can't write more CID Data. Otherwise, the line will
             * show 1 more data byte. Check this and prevent CID Data Write
             */
            if ((fskParam[0] & VP880_CID_STATE_MASK) == VP880_CID_STATE_IDLE) {
                /* Prevent writing the CID Data Register. */
                pLineObj->cidBytesRemain = 0;
                data = 0;
                initFsk = FALSE;
            } else {
                pLineObj->cidBytesRemain = 1;   /* Force count to 0 after wrting the current byte */
            }

            if (data == 0) {
                VP_CID(VpLineCtxType, pLineCtx,
                    ("Vp880CtrlSetFSKGen(): Disabling FSK at time %d", pDevObj->timeStamp));

                /* Stop Transmission Immediately */
                Vp880MuteChannel(pLineCtx, FALSE);
                fskParam[0] |= VP880_CID_DIS;
                moreData = FALSE;
            } else {
                if (pLineObj->suspendCid == FALSE) {
                   /* Wait until the device is complete */
                    VP_CID(VpLineCtxType, pLineCtx,
                        ("Vp880CtrlSetFSKGen(): Delaying FSK termination at time %d",
                        pDevObj->timeStamp));
                    pLineObj->suspendCid = TRUE;
                    /*
                     * Make sure the last byte is defined to be a "safe" mark
                     * signal and tell the silicon this is the last byte.
                     */
                    fskParam[0] &= ~VP880_CID_DIS;
                    fskParam[0] |=
                        (VP880_CID_FB_START_1 | VP880_CID_FB_STOP_1 | VP880_CID_EOM);
                    data = 0xFF;
                } else {
                    VP_CID(VpLineCtxType, pLineCtx,
                        ("Vp880CtrlSetFSKGen(): FSK Termination already in progress at time %d",
                        pDevObj->timeStamp));
                    return returnStatus;
                }
            }
            break;

        case VP_CID_GENERATOR_DATA:
            VP_CID(VpLineCtxType, pLineCtx,
                ("Vp880CtrlSetFSKGen(): VP_CID_GENERATOR_DATA 0x%02X at time %d with prior CID State: 0x%02X",
                data, pDevObj->timeStamp, pLineObj->tickBeginState[0]));

            pLineObj->callerId.status |= VP_CID_FSK_ACTIVE;
            Vp880MuteChannel(pLineCtx, TRUE);

            fskParam[0] |= (VP880_CID_FB_START_0 | VP880_CID_FB_STOP_1);
            fskParam[0] &= ~(VP880_CID_DIS);

            if (pLineObj->callerId.status & VP_CID_END_OF_MSG) {
                VP_CID(VpLineCtxType, pLineCtx,
                    ("Vp880CtrlSetFSKGen(): Sending EOM Signal to Silicon at time %d",
                    pDevObj->timeStamp));
                fskParam[0] |= VP880_CID_EOM;
            } else {
                fskParam[0] &= ~(VP880_CID_EOM);
            }
            break;

        case VP_CID_GENERATOR_KEYED_CHAR:
            VP_CID(VpLineCtxType, pLineCtx,
                ("Vp880CtrlSetFSKGen(): VP_CID_GENERATOR_KEYED_CHAR 0x%02X at time %d with prior CID State: 0x%02X",
                data, pDevObj->timeStamp, pLineObj->tickBeginState[0]));

            pLineObj->callerId.status |= VP_CID_FSK_ACTIVE;
            Vp880MuteChannel(pLineCtx, TRUE);
            fskParam[0] &= (uint8)(~(VP880_CID_EOM | VP880_CID_DIS));

            switch(data) {
                case VP_FSK_CHAN_SEIZURE:
                    fskParam[0] |= (VP880_CID_FB_START_0 | VP880_CID_FB_STOP_1);
                    break;

                case VP_FSK_MARK_SIGNAL:
                    fskParam[0] |= (VP880_CID_FB_START_1 | VP880_CID_FB_STOP_1);
                    if (pLineObj->callerId.markOutByteCount > 0) {
                        if (pLineObj->callerId.markOutByteRemain > 0) {
                            pLineObj->callerId.markOutByteRemain--;
                            VP_CID(VpLineCtxType, pLineCtx,
                                ("Vp880CtrlSetFSKGen(): Set markOutByteRemain to (%d) on ch (%d) at time %d",
                                 pLineObj->callerId.markOutByteRemain,
                                 pLineObj->channelId, pDevObj->timeStamp));
                            /*
                             * If we just decremented markOutByteRemain AND it is now == 0, this is
                             * the end of the FSK + Mark-Out Byte Message Data.
                             */
                            if (pLineObj->callerId.markOutByteRemain == 0) {
                                pLineObj->callerId.status |= VP_CID_END_OF_MSG;
                            }
                        }

                        if (pLineObj->callerId.status & VP_CID_END_OF_MSG) {
                            VP_CID(VpLineCtxType, pLineCtx,
                                ("Vp880CtrlSetFSKGen(): Last Mark-Out - Sending EOM Signal to Silicon at time %d",
                                pDevObj->timeStamp));
                            fskParam[0] |= VP880_CID_EOM;
                        }
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    /*
     * If the FSK generator was previously deactived (or never active) AND we're
     * now trying to disable it, this is redundant and we can immediately return
     */
    if (((pLineObj->callerId.status & VP_CID_FSK_ACTIVE) == 0)
      && (fskParam[0] & VP880_CID_DIS)) {
        VP_CID(VpLineCtxType, pLineCtx,
               ("Vp880CtrlSetFSKGen(): Redundant Disable Operation at time %d - Skipping Write",
                pDevObj->timeStamp));
        pLineObj->tickBeginState[0] = fskParam[0];
        return returnStatus;
    }

    if (fskParam[0] & VP880_CID_DIS) {
        pLineObj->callerId.status &= ~VP_CID_FSK_ACTIVE;
    }
    if ((pLineObj->tickBeginState[0] & VP880_CID_EOM) == VP880_CID_EOM) {
        VP_CID(VpLineCtxType, pLineCtx,
               ("Vp880CtrlSetFSKGen(): Identified EOM at time %d", pDevObj->timeStamp));
        return FALSE;
    }
    if (fskParam[0] != pLineObj->tickBeginState[0]) {
        pLineObj->tickBeginState[0] = fskParam[0];
        VP_CID(VpLineCtxType, pLineCtx, ("Vp880CtrlSetFSKGen(): Writing 0x%02X to CID Params",
            fskParam[0]));
        VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_WRT, VP880_CID_PARAM_LEN,
            fskParam);
    }
    if (moreData == TRUE) {
        VP_CID(VpLineCtxType, pLineCtx,
               ("Vp880CtrlSetFSKGen(): Writing 0x%02X to CID Data at Time %d",
                data, pDevObj->timeStamp));
        pLineObj->cidBytesRemain--;
        VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_DATA_WRT, VP880_CID_DATA_LEN,
            &data);

        if (initFsk == TRUE) {
            /*
             * Special Init case. The State machine updates in ~18us, so we
             * can't allow another byte write until we observe a change. It
             * will change to Empty, so check for that state as successfull
             * initialization.
             */
            uint8 safeFail = VP_CID_NORMAL_INIT_CNT;
            uint8 stateToExit;

            if (pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] <= VP880_REV_VC) {
                stateToExit = VP880_CID_STATE_RDY;
            } else {
                stateToExit = VP880_CID_STATE_EMPTY_D;
            }

            while (safeFail > 0) {
                VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_RD, VP880_CID_PARAM_LEN,
                    pLineObj->tickBeginState);

                if ((pLineObj->tickBeginState[0] & VP880_CID_STATE_MASK) == stateToExit) {
                    VP_CID(VpLineCtxType, pLineCtx,
                           ("Vp880CtrlSetFSKGen(): State = 0x%02X at Count %d Time %d - Return Now",
                            stateToExit, safeFail, pDevObj->timeStamp));
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880CtrlSetFSKGen()"));
                    return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
                } else {
                    VP_CID(VpLineCtxType, pLineCtx,
                           ("Vp880CtrlSetFSKGen(): Not Empty (state 0x%02X) on Count %d Time %d",
                           (pLineObj->tickBeginState[0] & VP880_CID_STATE_MASK), safeFail,
                            pDevObj->timeStamp));
                }
                safeFail--;
            }
            /* If we're here it's because the initialization check failed. */
            VP_CID(VpLineCtxType, pLineCtx,
                   ("Vp880CtrlSetFSKGen(): Did not find Good Transition During Init Time %d",
                    pDevObj->timeStamp));
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880CtrlSetFSKGen()"));
            return FALSE;
        } else {
#ifdef VP_CID_POLLING_ENABLED
            if ((pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION] > VP880_REV_VC) &&
                (mode != VP_CID_SIGGEN_EOT)) {
                uint8 tickEndState[VP880_CID_PARAM_LEN];
                uint8 retryCount;

                /*
                 * If the delay has been consumed prior to this point, means we already detected a
                 * state transition this tick. We should revert to using the number of bytes remain
                 * to determine whether we will return TRUE or FALSE.
                 */
                if (pLineObj->delayConsumed) {
                    return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
                }

                /*
                 * If we're here, means we have not seen a state change this tick. Look for one.
                 * Note that we already wrote one byte, so the state should be higher than the
                 * state read at the beginning of the tick.
                 */
                pLineObj->delayConsumed = TRUE;
                for(retryCount = 0; retryCount < VP_CID_NORMAL_MPI_CNT; retryCount++) {
                    VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_RD, VP880_CID_PARAM_LEN,
                        tickEndState);
                    VP_CID(VpLineCtxType, pLineCtx,
                           ("Vp880CtrlSetFSKGen(): CID Param Read 0x%02X Time %d",
                             tickEndState[0], pDevObj->timeStamp));
                    tickEndState[0] &= VP880_CID_STATE_MASK;

                    /*
                     * We've seen a transition toward more empty if these are the same since
                     * we wrote one byte earlier in this function. It means we can write more bytes.
                     */
                    if ((pLineObj->tickBeginState[0] & VP880_CID_STATE_MASK) == tickEndState[0]) {
                        pLineObj->cidBytesRemain++; /* Correct for state transition just observed */
                        VP_CID(VpLineCtxType, pLineCtx,
                               ("Vp880CtrlSetFSKGen(): Observed State Change to 0x%02X Time %d",
                                 tickEndState[0], pDevObj->timeStamp));
                        return TRUE;
                    }
                }
                /* If we're here, means we timed out and cannot write more bytes */
            }
            return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
#else
            return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
#endif
        }
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880CtrlSetFSKGen()"));
    return returnStatus;
}
#endif

/**
 * Vp880SendSignal()
 *  This function sends a signal on the line. The type of signal is specified
 * by the type parameter passed. The structure passed specifies the parameters
 * associated with the signal.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The signal specified is applied to the line.
 */
VpStatusType
Vp880SendSignal(
    VpLineCtxType *pLineCtx,
    VpSendSignalType type,
    void *pStruct)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDigitType *pDigit;
    VpDigitType digit = VP_DIG_NONE;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpStatusType status;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp880SendSignal()"));

    /* Proceed only if line has been initialized */
    if (!(pLineObj->status & VP880_INIT_COMPLETE)) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendSignal()"));
        return VP_STATUS_LINE_NOT_CONFIG;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->state & VP_DEV_IN_CAL) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendSignal()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pStruct == VP_NULL) {
        pDigit = &digit;
    } else {
        pDigit = pStruct;
    }

    switch(type) {
#if defined (VP880_FXO_SUPPORT)
        case VP_SENDSIG_DTMF_DIGIT:
            status = Vp880SendDigit(pLineCtx, VP_DIGIT_GENERATION_DTMF, *pDigit);
            break;

        case VP_SENDSIG_PULSE_DIGIT:
            pDigit = (VpDigitType *)pStruct;
            status = Vp880SendDigit(pLineCtx, VP_DIGIT_GENERATION_DIAL_PULSE,
                *pDigit);
            break;

        case VP_SENDSIG_HOOK_FLASH:
            /* prevent case of *pDigit when user passes VP_NULL */
            status = Vp880SendDigit(pLineCtx, VP_DIGIT_GENERATION_DIAL_HOOK_FLASH,
                VP_DIG_NONE);
            break;

        case VP_SENDSIG_MOMENTARY_LOOP_OPEN:
            status = Vp880MomentaryLoopOpen(pLineCtx);
            break;
#endif

#if defined (VP880_FXS_SUPPORT)
        case VP_SENDSIG_MSG_WAIT_PULSE:
            status = Vp880SendMsgWaitPulse(pLineCtx, pStruct);
            break;

        case VP_SENDSIG_FWD_DISCONNECT:
        case VP_SENDSIG_TIP_OPEN_PULSE:
            if (pStruct != VP_NULL) {
                VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Pulse Time %d", *((uint16 *)pStruct)));
                status = Vp880SendPulse(pLineCtx, type, *((uint16 *)pStruct));
            } else {
                VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendSignal()"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_SENDSIG_POLREV_PULSE:
            if (pStruct != VP_NULL) {
                status = Vp880SendPolRevPulse(pLineCtx, *((uint16 *)pStruct));
            } else {
                VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendSignal()"));
                return VP_STATUS_INVALID_ARG;
            }
            break;
#endif

        default:
            status = VP_STATUS_INVALID_ARG;
            break;
    }

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp880SendSignal()"));
    return status;
}

#if defined (VP880_FXS_SUPPORT)
/**
 * Vp880SendMsgWaitPulse()
 *  This function sends a message waiting pulse to the line specified by the
 * by the pMsgWait parameter passed. The structure specifies a voltage, on-time,
 * off-time, and number of pulses.
 *
 *    Times in this profile are supported as:
 *        Line State Set to Message Wiating Signal Level = On, + tick
 *        Delay for On-Time (in 5ms step size - fixed per cadencer definition)
 *        Line State Set to Off-State, +tick
 *        Delay for Off-Time
 *        Branch - Adjust cadencer values
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The message waiting signal specified is applied to the line.
 */
VpStatusType
Vp880SendMsgWaitPulse(
    VpLineCtxType *pLineCtx,
    VpSendMsgWaitType *pMsgWait)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = pLineObj->ecVal;

    uint16 tempTime, firstTimer, secondTimer, roundingError, tickRate_mS;
    uint16 tickRate = pDevObj->devProfileData.tickRate;

    VpLineStateType currentState = pLineObj->lineState.usrCurrent;

    uint8 branchCount;
    uint8 addStep = 0;

    uint32 aVolt;
    int32 userVolt;
    uint8 cmdLen = 0x08;  /* Minimum Cadence with infinite on */

    /*
     * Set the signal generator parameters to set the A amplitude and frequency
     * "very low". We'll adjust the bias to the user defined MsgWait voltage
     */
    uint8 sigGenBytes[VP880_SIGA_PARAMS_LEN] = {
        0x00, 0x29, 0x73, 0x04, 0x44, 0x00, 0x15, 0x7F, 0xFD, 0x00, 0x00};

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880SendMsgWaitPulse()"));

    if (pLineObj->status & VP880_IS_FXO) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendMsgWaitPulse()"));
        return VP_STATUS_INVALID_ARG;
    }

    /*
     * If we're already in Ringing, return a failure since we're using a
     * shared resource to accomplish this function.
     */
    if ((currentState == VP_LINE_RINGING) || (currentState == VP_LINE_RINGING_POLREV)) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendMsgWaitPulse()"));
        return VP_STATUS_DEVICE_BUSY;
    }

    /*
     * If the voltage is 0, it (previously) meant to use the maximum voltage
     * supported by the line. However, that function has been removed so instead
     * of stopping Message Waiting, just return error code to maintain a bit of
     * backward compatibility (max voltage isn't applied, but it isn't stopped
     * either).
     */
    if ((pMsgWait != VP_NULL) && (pMsgWait->voltage == 0)) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendMsgWaitPulse()"));
        return VP_STATUS_INVALID_ARG;
    }

    /* All parameters passed are good -- proceed */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP880_INT_SEQ_LEN);

    /*
     * If we were previously running a Message Waiting cadence, stop it and
     * generate the event.
     * If we were previously running another cadence, let it continue and
     * return.
     */
    if ((pMsgWait == VP_NULL) || (pMsgWait->onTime == 0)) {
        VpSetLineState(pLineCtx, currentState);
        pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
        pLineObj->cadence.pActiveCadence = VP_PTABLE_NULL;
        pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
        pLineObj->processData = VP_SENDSIG_MSG_WAIT_PULSE;
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendMsgWaitPulse()"));
        return VP_STATUS_SUCCESS;
    }

    /**************************************************************************
     * BEGIN >> METERING SIGNAL/VOLTAGE (RING GEN) PROCESSING
     **************************************************************************/
    /*
     * Compute the new signal generator A values from the voltage and set the
     * line state that is used to apply the message waiting pulse
     */
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    if (pMsgWait->voltage > 0) {
        userVolt = pMsgWait->voltage;
        pLineObj->intSequence[9] = VP_PROFILE_CADENCE_STATE_MSG_WAIT_NORM;
    } else {
        userVolt = -pMsgWait->voltage;
        pLineObj->intSequence[9] = VP_PROFILE_CADENCE_STATE_MSG_WAIT_POLREV;
    }

    /* Scale by same factor as bit resolution */
    aVolt = userVolt * (uint32)VP880_RINGING_BIAS_FACTOR;

    /* Scale down by the bit resolution of the device */
    aVolt /= (uint32)VP880_RINGING_BIAS_SCALE;

    sigGenBytes[VP880_SIGA_BIAS_MSB] = (aVolt >> 8) & 0xFF;
    sigGenBytes[VP880_SIGA_BIAS_LSB] = (aVolt & 0xFF);

    /* Write the new signal generator parameters */
    VpMpiCmdWrapper(deviceId, ecVal, VP880_RINGER_PARAMS_WRT,
       VP880_RINGER_PARAMS_LEN, sigGenBytes);

    /* Clear flag to indicate the generators are NOT in a Ringing Mode */
    pLineObj->status &= ~(VP880_RING_GEN_NORM | VP880_RING_GEN_REV);

    /**************************************************************************
     * END >> METERING SIGNAL/VOLTAGE (RING GEN) PROCESSING
     **************************************************************************/

    /*
     * Build the rest of the cadence defined by the user input (message state
     * set above). Start by setting the type of profile to an API Message Wait
     * Pulse type
     */
    pLineObj->intSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT;

    /**************************************************************************
     * BEGIN >> ON-TIME PROCESSING
     **************************************************************************/
    /*
     * The cadencer process will add 1 times the tickRate to the on-time. So
     * for an on-time of 10 (i.e., 50ms) and tickRate of 10ms, if we do nothing
     * else then the actual on-time would be 50ms + 10ms = 60ms. Based on
     * this, subtract off (tickRate) amount while not allowing the final
     * value to be set to 0. "0" is a special on-time value meaning "infinite".
     *
     * In case of non-integer tickrate round down to nearest ms. Round down is
     * preferred (from round, or round-up) because we're assuming the tick is
     * providing this much time and we need to ensure the minimum on-time. So
     * a lower (tickRate) value will cause this algorithm to provide a higher
     * cadence specified on-time. OK if reality is higher due to this tickRate
     * rounding.
     */
    roundingError = (tickRate % VP_CSLAC_TICKSTEP_1MS);
    tickRate_mS = (tickRate - roundingError);   /* In Dev Profile scale... */
    tickRate_mS /= VP_CSLAC_TICKSTEP_1MS;       /* ...now in ms scale. */
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Rounding %d for TickRate %d (ms)",
        roundingError, tickRate_mS));

    /*
     * The minimum possible time is (5ms + tickRate). If the input is less
     * than this, set to the minimum. Otherwise, subtract (tickRate) value
     * from the specified time. Rounding back to 5ms (i.e., cadencer steps) is
     * done later.
     */
    firstTimer = pMsgWait->onTime;
    if (firstTimer <= tickRate_mS) {
        /*
         * User specified a value that is less than what can be supported.
         * Set to the minimum while avoiding 0.
         */
        firstTimer = 5;
    } else {
        /* Subtract the tick processing time */
        firstTimer -= tickRate_mS;
    }
    VP_SEQUENCER(VpLineCtxType, pLineCtx,
        ("On-Time: %d (ms) after processing for min", firstTimer));

    /*
     * At this point, the "firstTimer" value is in mSec and may be less than
     * the (5ms) cadence step size. Now need to round up the time to ensure a
     * minimum on-time.
     */
    /* Compute the rounding error to add that will result in integer of 5ms steps */
    roundingError = (firstTimer % 5);

    /* Round-up to nearest 5ms step */
    if (roundingError > 0) {
        firstTimer += (5 - roundingError);
    }

    /* Convert from user input range (1ms) to cadence range (5ms) */
    firstTimer /= 5;

    /*
     * Special Handling for using 16-bit time in 14-bit data fields. This
     * mini-algorithm breaks the total on-time into smaller pieces wrapped
     * by an inside-loop branch operator.
     */
    branchCount = 0;
    if (firstTimer > 8192) {
        for (; firstTimer > 8192; branchCount++) {
            firstTimer = ((firstTimer >> 1) & 0x3FFF);
        }
        cmdLen+=2;
    }

    pLineObj->intSequence[10] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
    tempTime = ((firstTimer >> 8) & 0x1F);
    pLineObj->intSequence[10] |= tempTime;

    tempTime = (firstTimer & 0x00FF);
    pLineObj->intSequence[11] = tempTime;

    if (branchCount) {
        pLineObj->intSequence[12] = VP_SEQ_SPRCMD_BRANCH_INSTRUCTION;
        pLineObj->intSequence[12] |= 0x01;  /* On-Time is the step 1 (0 base) */
        pLineObj->intSequence[13] = branchCount;
        addStep+=2;
    }
    /**************************************************************************
     * END >> ON-TIME PROCESSING
     **************************************************************************/

    /**************************************************************************
     * BEGIN >> OFF-TIME PROCESSING
     **************************************************************************/
    /*
     * If the off-time is 0, we will stay in the previous state forever so the
     * cadencer needs to stop where it is
     */
    if (pMsgWait->offTime == 0) {
        pLineObj->intSequence[VP_PROFILE_LENGTH] = cmdLen;
        pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] =
            (0x04 + addStep);
    } else {
        cmdLen+=4;   /* Add two for the next state and two for the off-time */

        /* In-between pulses we'll return to the current state */
        pLineObj->intSequence[12+addStep]
            = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);
        pLineObj->intSequence[13+addStep] =
            ConvertApiState2PrfWizState(currentState);

        /*
         * For an off-time of any meaning (i.e., non-zero off-time and non-zero
         * number of message waiting pulses), the API affectively adds 2-tick
         * intervals to the off-time. Make sure to compensate this from the
         * user specified input.
         */
        secondTimer = pMsgWait->offTime;
        if (secondTimer <= (2 * tickRate_mS)) {
            /*
             * User specified a value that is less than what can be supported.
             * Set to the minimum while avoiding 0.
             */
            secondTimer = 5;
        } else {
            /* Subtract the tick processing time */
            secondTimer -= (2 * tickRate_mS);
        }
        VP_SEQUENCER(VpLineCtxType, pLineCtx,
            ("Off-Time: %d (ms) after processing", secondTimer));

        /*
         * Compute the rounding error used to modify the off-time to the nearest
         * 5ms step, avoid 0 after processing.
         */
        roundingError = (secondTimer % 5);
        if (roundingError > 3) {
            /* Round up is closer to requested value. */
            secondTimer += (5 - roundingError);
        } else {
            /* Round down is closer to requested value. */
            secondTimer -= roundingError;
            if (secondTimer == 0) {
                secondTimer = 5;
            }
        }

        /* Convert from user input range (1ms) to cadence range (5ms) */
        secondTimer /= 5;

        branchCount = 0;
        if (secondTimer > 8192) {
            cmdLen+=2;   /* Add two for the off-time branch loop */
            /* Special Handling for using 16-bit time in 14-bit data fields */
            for (; secondTimer > 8192; branchCount++) {
                secondTimer = ((secondTimer >> 1) & 0x3FFF);
            }
        }

        pLineObj->intSequence[14+addStep] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
        tempTime = ((secondTimer >> 8) & 0x1F);
        pLineObj->intSequence[14+addStep] |= tempTime;

        tempTime = (secondTimer & 0x00FF);
        pLineObj->intSequence[15+addStep] = tempTime;

        /**********************************************************************
         * BEGIN >> NUMBER OF MSG WAIT PULSE PROCESSING
         **********************************************************************/
        if (branchCount) {
            pLineObj->intSequence[16+addStep] = VP_SEQ_SPRCMD_BRANCH_INSTRUCTION;
            pLineObj->intSequence[16+addStep] |= (0x03 + (addStep / 2));
            pLineObj->intSequence[17+addStep] = branchCount;
            addStep+=2;
        }

        /*
         * If the number of cycles is 0, set the branch to repeat forever. If
         * it's 1, don't add a branch statement because the sequence should end
         * after the first cycle, otherwise subtract 1 from the total number of
         * cycles to force the correct number of "repeats" (branch)
         */

        if (pMsgWait->cycles != 1) {
            cmdLen+=2; /* Two more for this last branch operator */
            pLineObj->intSequence[16+addStep] = VP_SEQ_SPRCMD_BRANCH_INSTRUCTION;
            pLineObj->intSequence[17+addStep] = (pMsgWait->cycles) ?
                (pMsgWait->cycles - 1) : pMsgWait->cycles;
        }
        /**********************************************************************
         * END >> NUMBER OF MSG WAIT PULSE PROCESSING
         **********************************************************************/
    }
    /**********************************************************************
     * END >> OFF-TIME PROCESSING
     **********************************************************************/

    /*
     * Set the line object cadence variables to this sequence and activate the
     * sequencer
     */
    pLineObj->intSequence[VP_PROFILE_LENGTH] = cmdLen;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = cmdLen - 4;

    pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
    pLineObj->cadence.length = pLineObj->intSequence[VP_PROFILE_LENGTH];
    {
        uint8 cadenceIndex = 0;
        VP_SEQUENCER(VpLineCtxType, pLineCtx,
            ("Starting Message Waiting Sequence:"));
        for (cadenceIndex = 0;
             cadenceIndex < (VP_PROFILE_LENGTH + 1 + cmdLen);
             cadenceIndex++) {
            VP_SEQUENCER(VpLineCtxType, pLineCtx,
                (" 0x%02X", pLineObj->intSequence[cadenceIndex]));
        }
    }
    pLineObj->cadence.pActiveCadence = &pLineObj->intSequence[0];
    pLineObj->cadence.pCurrentPos = &pLineObj->intSequence[8];

    pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
    pLineObj->cadence.status |= VP_CADENCE_STATUS_SENDSIG;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendMsgWaitPulse()"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880SendPulse()
 *  This function sends either a forward disconnect or Tip Open pulse to the
 * line specified for a duration given in mS.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  A disconnect or tip open has been applied to the line, the line state is
 * restored to what it was prior to this function being called.
 */
VpStatusType
Vp880SendPulse(
    VpLineCtxType *pLineCtx,
    VpSendSignalType type,
    uint16 timeInMs)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpLineStateType currentState = pLineObj->lineState.usrCurrent;
    VpProfileCadencerStateTypes cadenceState;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 index, targetState, tickTime;
    uint16 timeIn5mS = 0;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880SendPulse()"));

    if (pLineObj->status & VP880_IS_FXO) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendPulse()"));
        return VP_STATUS_INVALID_ARG;
    }

    cadenceState = ConvertApiState2PrfWizState(currentState);

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP880_INT_SEQ_LEN);

    /* Set the cadence type and target state */
    if (type == VP_SENDSIG_FWD_DISCONNECT) {
        pLineObj->intSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_FWD_DISC_INT;
        targetState = VP_PROFILE_CADENCE_STATE_DISCONNECT;
    } else {
        pLineObj->intSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_TIP_OPEN_INT;
        targetState = VP_PROFILE_CADENCE_STATE_TIP_OPEN;
    }
    /* First step is to go to target state */
    index = 0;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = targetState;

    /*
     * The sequencer itself adds 1 tick of delay, so subtract that amount of
     * time unless time remaining is <= tick time.
     */
    tickTime = TICKS_TO_MS(1, pDevObj->devProfileData.tickRate);
    if (timeInMs > tickTime) {
        timeInMs-=tickTime;
    }

    /* Then wait for the time specified -- rounded to 5mS increments */
    if (timeInMs < 5) {
        timeIn5mS = 1;
    } else {
        timeIn5mS = timeInMs / 5;
    }
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        |= (timeIn5mS >> 8) & 0x1F;

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (timeIn5mS & 0xFF);

    /* Restore the line state */
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = cadenceState;

    /* Then wait for 100mS for the detector to become stable */
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = 20; /* 5mS per increment */

    index++; /* Adjust one more for length values */
    /*
     * Set the line object cadence variables to this sequence and activate the
     * sequencer
     */
    pLineObj->intSequence[VP_PROFILE_LENGTH] = index + 4;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = index;

    pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
    pLineObj->cadence.length = pLineObj->intSequence[VP_PROFILE_LENGTH];

    pLineObj->cadence.pActiveCadence = &pLineObj->intSequence[0];
    pLineObj->cadence.pCurrentPos =
        &pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START];

    pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
    pLineObj->cadence.status |= VP_CADENCE_STATUS_SENDSIG;
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendPulse()"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880SendPolRevPulse()
 *  This function sends a Pol Rev pulse to the line specified with a duration
 * given in mS.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  A Pol Rev pulse has been applied to the line, the line state is restored
 * to what it was prior to this function being called.
 */
VpStatusType
Vp880SendPolRevPulse(
    VpLineCtxType *pLineCtx,
    uint16 timeInMs)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpLineStateType currentState = pLineObj->lineState.usrCurrent;
    VpProfileCadencerStateTypes cadenceState, polRevState;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 index, tickTime;
    uint16 timeIn5mS = 0;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880SendPolRevPulse()"));

    if ((pLineObj->status & VP880_IS_FXO) || (currentState == VP_LINE_DISCONNECT)) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendPolRevPulse()"));
        return VP_STATUS_INVALID_ARG;
    }

    cadenceState = ConvertApiState2PrfWizState(currentState);
    polRevState = ConvertApiState2PrfWizState(VpGetReverseState(currentState));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP880_INT_SEQ_LEN);

    /* Set the cadence type */
    pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
        VP_PRFWZ_PROFILE_POLREV_PULSE_INT;

    /* First step is to go to polrev state */
    index = 0;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = polRevState;

    /*
     * The sequencer itself adds 1 tick of delay, so subtract that amount of
     * time unless time remaining is <= tick time.
     */
    tickTime = TICKS_TO_MS(1, pDevObj->devProfileData.tickRate);
    if (timeInMs > tickTime) {
        timeInMs-=tickTime;
    }

    /* Then wait for the time specified -- rounded to 5mS increments */
    if (timeInMs < 5) {
        timeIn5mS = 1;
    } else {
        timeIn5mS = timeInMs / 5;
    }
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        |= (timeIn5mS >> 8) & 0x1F;

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (timeIn5mS & 0xFF);

    /* Restore the line state */
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = cadenceState;

    /* Then wait for 100mS for the detector to become stable */
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = 20; /* 5mS per increment */

    index++; /* Adjust one more for length values */
    /*
     * Set the line object cadence variables to this sequence and activate the
     * sequencer
     */
    pLineObj->intSequence[VP_PROFILE_LENGTH] = index + 4;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = index;

    pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
    pLineObj->cadence.length = pLineObj->intSequence[VP_PROFILE_LENGTH];

    pLineObj->cadence.pActiveCadence = &pLineObj->intSequence[0];
    pLineObj->cadence.pCurrentPos =
        &pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START];

    pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
    pLineObj->cadence.status |= VP_CADENCE_STATUS_SENDSIG;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendPolRevPulse()"));
    return VP_STATUS_SUCCESS;
}
#endif

#if defined (VP880_FXO_SUPPORT)
/**
 * Vp880MomentaryLoopOpen()
 *  This function applies a Momentary Loop Open to an FXO line and tests for
 * a parallel off-hook.
 *
 * Preconditions:
 *  The line must first be initialized and must be of FXO type.
 *
 * Postconditions:
 *  A 10ms loop open is applied to the line and line state returns to previous
 * condition. An event is generated indicating if there exists a parallel phone
 * off-hook or not.
 */
VpStatusType
Vp880MomentaryLoopOpen(
    VpLineCtxType *pLineCtx)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpLineStateType currentState = pLineObj->lineState.usrCurrent;
    VpProfileCadencerStateTypes cadenceState;

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal = pLineObj->ecVal;

    uint8 index;
    uint16 timeIn5mS = 0;
    uint8 loopSup[VP880_LOOP_SUP_LEN] = {0x18, 0xE1, 0x79, 0xEB};

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880MomentaryLoopOpen()"));

    if (!(pLineObj->status & VP880_IS_FXO)) {
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880MomentaryLoopOpen()"));
        return VP_STATUS_INVALID_ARG;
    }

    cadenceState = ConvertApiState2PrfWizState(currentState);

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP880_INT_SEQ_LEN);

    /* Set the cadence type */
    pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
        VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT;

    VpMpiCmdWrapper(deviceId, ecVal, VP880_LOOP_SUP_WRT, VP880_LOOP_SUP_LEN,
        loopSup);

    /* First step is to go to Loop Open */
    index = 0;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN;

    /* Then wait for at least 10ms. The time is higher by 2*ApiTick value */
    timeIn5mS = 2;  /* Cadencer Tick is 5ms increments */

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        |= (timeIn5mS >> 8) & 0x1F;

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (timeIn5mS & 0xFF);

    /* Restore the line state */
    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = cadenceState;

    index++; /* Adjust for length values */
    /*
     * Set the line object cadence variables to this sequence and activate the
     * sequencer
     */
    pLineObj->intSequence[VP_PROFILE_LENGTH] = index + 4;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = index;

    pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
    pLineObj->cadence.length = pLineObj->intSequence[VP_PROFILE_LENGTH];

    pLineObj->cadence.pActiveCadence = &pLineObj->intSequence[0];
    pLineObj->cadence.pCurrentPos =
        &pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START];

    pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
    pLineObj->cadence.status |= VP_CADENCE_STATUS_SENDSIG;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880MomentaryLoopOpen()"));
    return VP_STATUS_SUCCESS;
}

/**
 * Vp880SendDigit()
 *  This function sends a DTMF or Dial Pulse digit on an FXO line. It creates
 * a sequencer compatible profile to control the FXO loop open, loop close, and
 * time operators.
 *
 * Preconditions:
 *  The line must first be initialized and must be of FXO type.
 *
 * Postconditions:
 *  The digit specified is sent on the line in the form specified (DTMF or Dial
 * Pulse).  This function returns the success code if the line is an FXO type of
 * line, if the digit is between 0 - 9, and if the digit type is either DTMF or
 * Dial Pulse.
 */
VpStatusType
Vp880SendDigit(
    VpLineCtxType *pLineCtx,            /**< Line to send a digit on */
    VpDigitGenerationType digitType,    /**< Type of digit to send. May indicate
                                         * DTMF, Dial Pulse, or Hook Flash
                                         */
    VpDigitType digit)                  /**< The digit to send. Used if type of
                                         * digit is DTMF or Dial Pulse
                                         */
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint16 tempTime, firstTimer, secondTimer;
    uint16 tickAdjustment;

    VpDeviceIdType deviceId = pDevObj->deviceId;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880SendDigit()"));

    if (!(pLineObj->status & VP880_IS_FXO)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Function invalid for FXS"));
        VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendDigit()"));
        return VP_STATUS_INVALID_ARG;
    }

    switch(digitType) {
        case VP_DIGIT_GENERATION_DIAL_PULSE:
            if ((pLineObj->lineState.currentState != VP_LINE_FXO_TALK)
             && (pLineObj->lineState.currentState != VP_LINE_FXO_LOOP_CLOSE)) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendDigit()"));
                return VP_STATUS_INVALID_ARG;
            }

        case VP_DIGIT_GENERATION_DTMF:
            if ((VpIsDigit(digit) == FALSE) || (digit == VP_DIG_NONE)) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Invalid digit"));
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendDigit()"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_DIGIT_GENERATION_DIAL_HOOK_FLASH:
            if ((pLineObj->lineState.currentState != VP_LINE_FXO_TALK)
             && (pLineObj->lineState.currentState != VP_LINE_FXO_LOOP_CLOSE)) {
                VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendDigit()"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Invalid digitType"));
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendDigit()"));
            return VP_STATUS_INVALID_ARG;
    }

    /* Parameters passed are good -- proceed */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP880_INT_SEQ_LEN);

    /* Tick adjustment in 5ms cadence time units */
    tickAdjustment = TICKS_TO_MS(1, pDevObj->devProfileData.tickRate) / 5;

    switch(digitType) {
        case VP_DIGIT_GENERATION_DTMF:
            Vp880MuteChannel(pLineCtx, TRUE);
            Vp880SetDTMFGenerators(pLineCtx, VP_CID_NO_CHANGE, digit);

            /* Fixed total length and sequence length for DTMF generation */
            pLineObj->intSequence[VP_PROFILE_LENGTH] = 0x0C;
            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = 0x08;

            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_SIGGEN);

            pLineObj->intSequence[12]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_SIGGEN);

            pLineObj->intSequence[9] =
                (VP_SEQ_SIGGEN_A_EN | VP_SEQ_SIGGEN_B_EN);

            pLineObj->intSequence[13] = VP_SEQ_SIGGEN_ALL_DISABLED;

            firstTimer = pLineObj->digitGenStruct.dtmfOnTime;
            if (firstTimer > tickAdjustment) {
                firstTimer -= tickAdjustment;
            } else {
                firstTimer = 1;
            }
            pLineObj->intSequence[10] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = (firstTimer >> 8) & 0x03;
            pLineObj->intSequence[10] |= tempTime;

            tempTime = (firstTimer & 0x00FF);
            pLineObj->intSequence[11] |= tempTime;

            secondTimer = pLineObj->digitGenStruct.dtmfOffTime;
            if (secondTimer > tickAdjustment) {
                secondTimer -= tickAdjustment;
            } else {
                secondTimer = 1;
            }
            pLineObj->intSequence[14] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = (secondTimer >> 8) & 0x03;
            pLineObj->intSequence[14] |= tempTime;

            tempTime = (secondTimer & 0x00FF);
            pLineObj->intSequence[15] |= tempTime;

            pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
                VP_PRFWZ_PROFILE_DTMF_DIG_GEN;
            break;

        case VP_DIGIT_GENERATION_DIAL_PULSE:
            /* Fixed total length and sequence length for DP generation */
            pLineObj->intSequence[VP_PROFILE_LENGTH] = 0x10;
            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = 0x0C;

            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

            pLineObj->intSequence[12]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

            if (pLineObj->lineState.currentState == VP_LINE_FXO_TALK) {
                pLineObj->intSequence[9] =
                    VP_PROFILE_CADENCE_STATE_FXO_OHT;
                pLineObj->intSequence[13] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_TALK;
            } else {
                pLineObj->intSequence[9] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN;
                pLineObj->intSequence[13] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_CLOSE;
            }

            firstTimer = pLineObj->digitGenStruct.breakTime;
            if (firstTimer > tickAdjustment) {
                firstTimer -= tickAdjustment;
            } else {
                firstTimer = 1;
            }
            pLineObj->intSequence[10] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = (firstTimer >> 8) & 0x03;
            pLineObj->intSequence[10] |= tempTime;

            tempTime = (firstTimer & 0x00FF);
            pLineObj->intSequence[11] |= tempTime;

            secondTimer = pLineObj->digitGenStruct.makeTime;
            if (secondTimer > tickAdjustment * 2) {
                secondTimer -= tickAdjustment * 2;
            } else {
                secondTimer = 1;
            }
            pLineObj->intSequence[14] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = (secondTimer >> 8) & 0x03;
            pLineObj->intSequence[14] |= tempTime;

            tempTime = (secondTimer & 0x00FF);
            pLineObj->intSequence[15] |= tempTime;

            firstTimer = pLineObj->digitGenStruct.dpInterDigitTime;
            if (digit > 1) {
                pLineObj->intSequence[16] = VP_SEQ_SPRCMD_BRANCH_INSTRUCTION;
                pLineObj->intSequence[17] = digit - 1;

                pLineObj->intSequence[18] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                tempTime = (firstTimer >> 8) & 0x03;
                pLineObj->intSequence[18] |= tempTime;
                tempTime = (firstTimer & 0x00FF);
                pLineObj->intSequence[19] |= tempTime;
            } else {
                pLineObj->intSequence[16] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                tempTime = (firstTimer >> 8) & 0x03;
                pLineObj->intSequence[16] |= tempTime;
                tempTime = (firstTimer & 0x00FF);
                pLineObj->intSequence[17] |= tempTime;

                pLineObj->intSequence[VP_PROFILE_LENGTH] = 0x0E;
                pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB]
                    = 0x0A;
            }

            pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
                VP_PRFWZ_PROFILE_DIAL_PULSE_DIG_GEN;
            break;

        case VP_DIGIT_GENERATION_DIAL_HOOK_FLASH:
            /* Fixed total length and sequence length for FLASH generation */
            pLineObj->intSequence[VP_PROFILE_LENGTH] = 0x0A;
            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = 0x06;

            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

            pLineObj->intSequence[12]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

            if (pLineObj->lineState.currentState == VP_LINE_FXO_TALK) {
                pLineObj->intSequence[9] =
                    VP_PROFILE_CADENCE_STATE_FXO_OHT;
                pLineObj->intSequence[13] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_TALK;
            } else {
                pLineObj->intSequence[9] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN;
                pLineObj->intSequence[13] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_CLOSE;
            }

            firstTimer = pLineObj->digitGenStruct.flashTime;
            if (firstTimer > tickAdjustment) {
                firstTimer -= tickAdjustment;
            } else {
                firstTimer = 1;
            }
            pLineObj->intSequence[10] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = (firstTimer >> 8) & 0x03;
            pLineObj->intSequence[10] |= tempTime;

            tempTime = (firstTimer & 0x00FF);
            pLineObj->intSequence[11] |= tempTime;

            pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
                VP_PRFWZ_PROFILE_HOOK_FLASH_DIG_GEN;
            break;

        default:
            /*
             * This can only occur if there is an error in the error checking
             * above.
             */
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_INVALID_ARG;
    }

    pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
    pLineObj->cadence.length = pLineObj->intSequence[VP_PROFILE_LENGTH];

    pLineObj->cadence.pActiveCadence = &pLineObj->intSequence[0];
    pLineObj->cadence.pCurrentPos = &pLineObj->intSequence[8];

    pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
    pLineObj->cadence.status |= VP_CADENCE_STATUS_SENDSIG;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880SendDigit()"));
    return VP_STATUS_SUCCESS;
}
#endif

#ifdef VP880_FXS_SUPPORT
/**
 * Vp880FSKGeneratorReady()
 *  This function is used for Caller ID to determine if the FSK generator is
 * ready to accept another byte. It uses the device caller ID state machine
 * and signaling (caller ID status) register. This function should be called
 * from an API internal function only.
 *
 * Returns:
 *  TRUE if the FSK generator for Caller ID can accept a byte, FALSE otherwise.
 */
bool
Vp880FSKGeneratorReady(
    VpLineCtxType *pLineCtx)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;

#ifdef VP_CID_POLLING_ENABLED
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal = pLineObj->ecVal;
    uint8 stateRetry;
#endif
    uint8 stateIndex;
    uint8 devRev = pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp880FSKGeneratorReady()"));

    if (devRev <= VP880_REV_VC) {
        uint8 numBytes[] = {
            /*
             * NOTE: The #bytes in IDLE state is 2 for this array init purposes,
             * but this should never be used. Why? Because the state machine is
             * only operating in the IDLE state with API CID State Machine
             * running and operating on FSK Data Type when the silicon level
             * state machine is disabled. In this case, the return value below
             * is forced to 3. So only if the state is IDLE and NOT Disabled is
             * this return value used - which should be impossible.
             * In other words, this is a safety mechanism only .. hopefully.
             */
            2,  /* 0 = #define VP880_CID_STATE_IDLE    0x00 */
            1,  /* 1 = #define VP880_CID_STATE_RDY     0x20 */
            0,  /* 2 = #define VP880_CID_STATE_FULL    0x40 */
            1,  /* 3 = #define VP880_CID_STATE_LBYTE   0x60 */
            0,  /* 4 = #define VP880_CID_STATE_L2BYTE  0x80 */
            2,  /* 5 = #define VP880_CID_STATE_URUN    0xA0 */
            0,  /* 6 = RSVD */
            0   /* 7 = RSVD */
        };

        /* Check the Generator State */
        stateIndex = (((pLineObj->tickBeginState[0] & VP880_CID_STATE_MASK) >> 5) & 0x7);
        pLineObj->cidBytesRemain = numBytes[stateIndex];
        VP_CID(VpLineCtxType, pLineCtx, ("CID Param Read 0x%02X", pLineObj->tickBeginState[0]));
        } else {
        uint8 cidState[VP880_CID_PARAM_LEN];
        uint8 numBytes[] = {
            3,  /* 0 = Idle                 #define VP880_CID_STATE_IDLE        0x00 */
            2,  /* 1 = Empty                #define VP880_CID_STATE_EMPTY_D     0x20 */
            1,  /* 2 = Half-Full            #define VP880_CID_STATE_HALF_FULL_D 0x40 */
            2,  /* 3 = Last Byte - EOM      #define VP880_CID_STATE_LBYTE_D     0x60 */
            1,  /* 4 = Last 2 Byte - EOM    #define VP880_CID_STATE_L2BYTE_D    0x80 */
            2,  /* 5 = Underrun */
            0,  /* 6 = Full                 #define VP880_CID_STATE_FULL_D      0xC0 */
            0   /* 7 = Last 3 Byte - EOM    #define VP880_CID_STATE_L3BYTE_D    0xE0
                 * There should be no way to get here. It occurs when "FUll" state and
                 * write + EOM, which the VP-API-II does not do.
                 */
        };

#ifdef VP_CID_POLLING_ENABLED
        if ((pLineObj->tickBeginState[0] & VP880_CID_STATE_MASK) == VP880_CID_STATE_FULL_D) {
            pLineObj->delayConsumed = TRUE;
            stateRetry = VP_CID_NORMAL_MPI_CNT;
            do {
                stateRetry--;

                /* Check the Generator State until it is no longer full */
                VpMpiCmdWrapper(deviceId, ecVal, VP880_CID_PARAM_RD, VP880_CID_PARAM_LEN,
                    cidState);
                cidState[0] &= VP880_CID_STATE_MASK;

                VP_CID(VpLineCtxType, pLineCtx, ("CID State 0x%02X at time %d",
                                                 cidState[0], pDevObj->timeStamp));
                if (cidState[0] != VP880_CID_STATE_FULL_D) {
                    stateRetry = 0;
                }
            } while (stateRetry != 0);
        } else {
#endif
            cidState[0] = (pLineObj->tickBeginState[0] & VP880_CID_STATE_MASK);
#ifdef VP_CID_POLLING_ENABLED
        }
#endif
        stateIndex = ((cidState[0] >> 5) & 0x7);
        pLineObj->cidBytesRemain = numBytes[stateIndex];
    }
    VP_CID(VpLineCtxType, pLineCtx,
           ("Vp880FSKGeneratorReady() - return %s with cidBytes remain %d",
            ((pLineObj->cidBytesRemain == 0) ? "FALSE" : "TRUE"), pLineObj->cidBytesRemain));
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp880FSKGeneratorReady()"));
    return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
} /* Vp880FSKGeneratorReady() */

/**
 * Vp880CliGetEncodedByte()
 *  This function returns an encoded byte of data that is suitable for writing
 * the FSK generator (device dependent).
 *
 * Preconditions
 *  Must have a valid CLI packet in to work from.
 *
 * Postconditions
 *  The per-channel caller ID buffer will be updated with encoded data.
 *
 */
VpCliEncodedDataType
Vp880CliGetEncodedByte(
    VpLineCtxType *pLineCtx,
    uint8 *pByte)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpOptionEventMaskType *pLineEvents = &(pLineObj->lineEvents);
    VpCallerIdType *pCidStruct = &(pLineObj->callerId);

    uint8 checkSumIndex = VP_CID_PROFILE_FSK_PARAM_LEN +
        pLineObj->callerId.pCliProfile[VP_CID_PROFILE_FSK_PARAM_LEN] +
        VP_CID_PROFILE_CHECKSUM_OFFSET_LSB;

    if (pLineObj->suspendCid == TRUE) {
        *pByte = '\0';
        VP_CID(VpLineCtxType, pLineCtx, ("VE880 EOM In Progress..."));
        return VP_CLI_ENCODE_END;
    }

    return VpCSLACCliGetEncodedByte(pByte, pCidStruct, &pLineObj->processData,
                pLineEvents, checkSumIndex);
} /* Vp880CliGetEncodedByte() */
#endif  /* VP880_FXS_SUPPORT */
#endif  /* VP_CSLAC_SEQ_EN */
#endif  /* VP_CC_880_SERIES */
