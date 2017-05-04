/** \file vp790_seq.c
 * vp790_seq.c
 *
 *  This file contains the VP790 functions called by the API-II Caller ID or
 * sequencer. It is seperated from "normal" API functions for users that want
 * to remove this section of code from the API-II.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_790_SERIES)
#ifdef VP_CSLAC_SEQ_EN

/* INCLUDES */
#include "vp_api_types.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp790_api.h"
#include "vp790_api_int.h"
#include "sys_service.h"

/* Function called by API-II functions only to implement Caller ID */
static uint8
Vp790MirrorByte(
    char byte);

/**
 * Vp790CommandInstruction()
 *  This function implements the Sequencer Command instruction for the Vp790
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
Vp790CommandInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData)
{
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp790DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal[] = {VP790_EC_CH1, VP790_EC_CH2, VP790_EC_CH3, VP790_EC_CH4};
    uint8 channelId = pLineObj->channelId;
    uint8 sigGenCtrl, lineState;
    uint8 telTax[VP790_CCR5_LEN];

    switch (pSeqData[0] & VP_SEQ_SUBTYPE_MASK) {
        case VP_SEQ_SUBCMD_SIGGEN:
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_GEN_CTRL_RD,
                VP790_GEN_CTRL_LEN, &sigGenCtrl);
            sigGenCtrl &= ~(VP790_GENB_EN);

            /* Get the signal generator bits and set. Only B Control possible */
            sigGenCtrl |= ((pSeqData[1] & 0x01) ?  VP790_GENB_EN : 0);
            sigGenCtrl |= ((pSeqData[1] & 0x02) ?  VP790_GENB_EN : 0);
            VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_GEN_CTRL_WRT,
                VP790_GEN_CTRL_LEN, &sigGenCtrl);
            break;

        case VP_SEQ_SUBCMD_LINE_STATE:
            Vp790SetLineStateInt(pLineCtx,
                ConvertPrfWizState2ApiState(pSeqData[1]));
            break;

        case VP_SEQ_SUBCMD_START_CID:
        case VP_SEQ_SUBCMD_WAIT_ON:
            if (pLineObj->pCidProfileType1 != VP_PTABLE_NULL) {
                pLineObj->callerId.pCliProfile = pLineObj->pCidProfileType1;
                VpCSLACInitCidStruct(&pLineObj->callerId, pSeqData[0]);

                /*
                 * Three byte offset in the primary buffer because an "init"
                 * CID function would have processed the first three bytes and
                 * moved the converted data to the converted buffer
                 */
                pLineObj->callerId.cliMPIndex = 3;
            }
            break;

        case VP_SEQ_SUBCMD_METERING:
            if (pSeqData[1]) {  /* Metering On */
                /*
                 * Signal Generator A must be disabled before this point
                 * But, that shouldn't be a problem since this API only uses
                 * Generator A for Ringing, so by changing the SLIC state the
                 * generator will automatically be disabled.
                 * Just keep it in mind.
                 */

                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_CCR5_RD,
                    VP790_CCR5_LEN, telTax);
                if (telTax[0] & VP790_TELATAX_POLREV) {
                    pLineObj->lineTimers.timers.timer[VP_LINE_HOOK_FREEZE] =
                        MS_TO_TICKRATE(VP_POLREV_DEBOUNCE_TIME,
                            pDevObj->devProfileData.tickRate);
                    pLineObj->lineTimers.timers.timer[VP_LINE_HOOK_FREEZE]
                        |= VP_ACTIVATE_TIMER;
                }

                lineState = (VP790_SLIC_ST_TELETAX | VP790_SLIC_ST_HIGH_BATT);
                VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_SLIC_STATE_WRT,
                    VP790_SLIC_STATE_LEN, &lineState);
                pLineObj->cadence.meteringBurst++;
            } else {    /* Metering Off */
                Vp790SetLineStateInt(pLineCtx, pLineObj->lineState.currentState);

                if (pLineObj->cadence.meterPendingAbort) {
                    Vp790SetLineState(pLineCtx, pLineObj->cadence.meterAbortLineState);
                }
            }
            break;

        default:
            return VP_STATUS_INVALID_ARG;
    }

    pLineObj->cadence.index+=2;
    /*
     * Check to see if there is more sequence data, and if so, move the
     * sequence pointer to the next command. Otherwise, end this cadence
     */

    if (pLineObj->cadence.index <
       (pLineObj->cadence.length + VP_PROFILE_LENGTH + 1)) {
        pSeqData+=2;
        pLineObj->cadence.pCurrentPos = pSeqData;
    } else {
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

            default:
                break;

        }
        pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
        pLineObj->cadence.pActiveCadence = VP_PTABLE_NULL;
    }
    return VP_STATUS_SUCCESS;
}

/**
 * Vp790InitRing()
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
Vp790InitRing(
    VpLineCtxType *pLineCtx,        /**< Line Context to modify Ringing
                                     * Parameters for
                                     */
    VpProfilePtrType pCadProfile,   /**< Pointer of a Ringing Cadence profile,
                                     * or the index into the Ringing Cadence
                                     * profile table.
                                     */
    VpProfilePtrType pCidProfile)   /**< Pointer of a Caller ID profile,
                                     * or the index into the Caller ID
                                     * profile table.
                                     */
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp790DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpProfilePtrType *pProfileTable;

    int cadIndex = VpGetProfileIndex(pCadProfile);
    int cidIndex = VpGetProfileIndex(pCidProfile);

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * If the profile passed is an index, make sure it's in the valid range
     * and if so, set the currently used profile to it.
     */
    if ((cadIndex >= 0) && (cadIndex < VP_CSLAC_RING_CADENCE_PROF_TABLE_SIZE)) {
        /* Valid Cadence index.  Set it if it's not an invalid table entry */
        if (!(pDevObj->profEntry.ringCadProfEntry & (0x01 << cadIndex))) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_ERR_PROFILE;
        }

        pProfileTable = pDevObj->devProfileTable.pRingingCadProfileTable;
        pLineObj->pRingingCadence = pProfileTable[cadIndex];
    } else if (cadIndex >= VP_CSLAC_RING_CADENCE_PROF_TABLE_SIZE) {
        /* It's an index, but it's out of range */
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_INVALID_ARG;
    } else {
        /* This is a pointer.  Set it */
        pLineObj->pRingingCadence = pCadProfile;
    }

    /* Setup Caller ID profile like Ringing Cadence. */
    if ((cidIndex >= 0) && (cidIndex < VP_CSLAC_CALLERID_PROF_TABLE_SIZE)) {
        /* Valid Caller ID index.  Set it if it's not an invalid table entry */
        if (!(pDevObj->profEntry.cidCadProfEntry & (0x01 << cidIndex))) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_ERR_PROFILE;
        }

        pProfileTable = pDevObj->devProfileTable.pCallerIdProfileTable;
        pLineObj->pCidProfileType1 = pProfileTable[cidIndex];
    } else {
        if (cidIndex > VP_CSLAC_CALLERID_PROF_TABLE_SIZE) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_INVALID_ARG;
        } else {
            /* This is a pointer.  Set it */
            pLineObj->pCidProfileType1 = pCidProfile;
        }
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
} /* Vp790InitRing */

/**
 * Vp790InitCid()
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
Vp790InitCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    uint8p pCidData)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp790DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 primaryByteCount, secondaryByteCount, remainder;

    if (length > (2 * VP_SIZEOF_CID_MSG_BUFFER)) {
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        return VP_STATUS_SUCCESS;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    pLineObj->callerId.cliIndex = 0;
    pLineObj->callerId.cliMPIndex = 0;
    pLineObj->callerId.cliMSIndex = 0;
    pLineObj->cidEncodeIndex = 0;

    /* Stop CID if it was in progress */
    pLineObj->callerId.cliTimer = 0;
    pLineObj->callerId.status = VP_CID_RESET_VALUE;
    pLineObj->callerId.dtmfStatus = VP_CID_DTMF_RESET_VALUE;
    pLineObj->callerId.cidCheckSum = 0;
    pLineObj->cidEncodeSize = 5;

    /*
     * Make sure the primary buffer will get at least 3 bytes, even if it has
     * only 3 bytes. This makes the initial indexing for buffer switching work
     */

    remainder = length - VP_SIZEOF_CID_MSG_BUFFER;
    if ((remainder < 3) && (length > VP_SIZEOF_CID_MSG_BUFFER)) {
        pLineObj->callerId.primaryMsgLen = 3;
        pLineObj->callerId.secondaryMsgLen = length - 3;
    } else {
        if (length <= VP_SIZEOF_CID_MSG_BUFFER) {
            pLineObj->callerId.primaryMsgLen = length;
            pLineObj->callerId.secondaryMsgLen = 0;
        } else {
            pLineObj->callerId.primaryMsgLen = length - VP_SIZEOF_CID_MSG_BUFFER;
            pLineObj->callerId.secondaryMsgLen = VP_SIZEOF_CID_MSG_BUFFER;
        }
    }

    /*
     * Copy the message data to the primary API buffer. If we're here, there's
     * at least 3 bytes of primary message data. So a check is not necessary
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

    /* Create the 5 byte buffer of caller ID data converted from user data */
    Vp790EncodeData(pLineCtx, pCidData);

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
}

/**
 * Vp790SendCid()
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
Vp790SendCid(
    VpLineCtxType *pLineCtx,
    uint8 length,
    VpProfilePtrType pCidProfile,
    uint8p pCidData)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp790DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VpProfilePtrType *pProfileTable;
    uint8 primaryByteCount, secondaryByteCount, remainder;

    int cidIndex = VpGetProfileIndex(pCidProfile);

    /*
     * Proceed if device state is either in progress or complete and not
     * calibrating
     */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
        if (pDevObj->status.state & VP_DEV_IN_CAL) {
            return VP_STATUS_DEVICE_BUSY;
        }
    } else {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (length > (2 * VP_SIZEOF_CID_MSG_BUFFER)) {
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        return VP_STATUS_SUCCESS;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    pLineObj->cidEncodeSize = 5;

    /* Determine if it's an index or profile */
    if ((cidIndex >= 0) && (cidIndex < VP_CSLAC_CALLERID_PROF_TABLE_SIZE)) {
        /* It's an index. Set the line profile to the device indexed profile */
        pProfileTable = pDevObj->devProfileTable.pCallerIdProfileTable;
        pLineObj->pCidProfileType2 = pProfileTable[cidIndex];
    } else {
        if (cidIndex > VP_CSLAC_CALLERID_PROF_TABLE_SIZE) {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_INVALID_ARG;
        } else {
            /*
             * This is a pointer. Make sure it's the correct type, and if so,
             * set it to the line object
             */
            if(VpVerifyProfileType(VP_PROFILE_CID, pCidProfile) == TRUE) {
                pLineObj->pCidProfileType2 = pCidProfile;
            } else {
                VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
                return VP_STATUS_ERR_PROFILE;
            }
        }
    }

    if (pLineObj->pCidProfileType2 == VP_PTABLE_NULL) {
        pLineObj->callerId.status = VP_CID_RESET_VALUE;  /* Clear all status bits */
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_ERR_PROFILE;
    }

    VpCSLACInitCidStruct(&pLineObj->callerId, 0);
    pLineObj->callerId.pCliProfile = pLineObj->pCidProfileType2;

    pLineObj->callerId.status &= ~VP_CID_REPEAT_MSG;
    pLineObj->callerId.status &= ~VP_CID_END_OF_MSG;
    pLineObj->callerId.cidCheckSum = 0;

    /*
     * Offset by the first 3 bytes because we're going to use the first
     * 3 bytes immediately to create our 5/3 byte conversion (required by
     * VE790 CID)
     */
    pLineObj->callerId.cliMPIndex = 3;
    pLineObj->cidEncodeIndex = 0;

    /*
     * Make sure the primary buffer will get at least 3 bytes, even if it has
     * only 3 bytes. This makes the initial indexing for buffer switching work
     */

    remainder = length - VP_SIZEOF_CID_MSG_BUFFER;
    if ((remainder < 3) && (length > VP_SIZEOF_CID_MSG_BUFFER)) {
        pLineObj->callerId.primaryMsgLen = 3;
        pLineObj->callerId.secondaryMsgLen = length - 3;
    } else {
        if (length <= VP_SIZEOF_CID_MSG_BUFFER) {
            pLineObj->callerId.primaryMsgLen = length;
            pLineObj->callerId.secondaryMsgLen = 0;
        } else {
            pLineObj->callerId.primaryMsgLen = length - VP_SIZEOF_CID_MSG_BUFFER;
            pLineObj->callerId.secondaryMsgLen = VP_SIZEOF_CID_MSG_BUFFER;
        }
    }

    /*
     * Copy the message data to the primary API buffer. If we're here, there's
     * at least 3 bytes of primary message data. So a check is not necessary
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

    /* Create the 5 byte buffer of caller ID data converted from user data */
    Vp790EncodeData(pLineCtx, pCidData);

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
}

/**
 * Vp790ContinueCid()
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
Vp790ContinueCid(
    VpLineCtxType *pLineCtx,    /**< Line to continue CID on */
    uint8 length,               /**< Length of data passed not to exceed the
                                 * buffer length
                                 */
    uint8p pCidData)            /**< CID message data */
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp790DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 byteCount = 0;
    uint8 *pMsgLen;
    uint8 *pBuffer;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    pLineObj->callerId.status &= ~VP_CID_END_OF_MSG;

    /*
     * Proceed if device state is either in progress or complete and not
     * calibrating
     */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
        if (pDevObj->status.state & VP_DEV_IN_CAL) {
            return VP_STATUS_DEVICE_BUSY;
        }
    } else {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (length >  VP_SIZEOF_CID_MSG_BUFFER) {
        return VP_STATUS_INVALID_ARG;
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
        pLineObj->callerId.cliMPIndex = 0;
        pMsgLen = &(pLineObj->callerId.primaryMsgLen);
        pBuffer = &(pLineObj->callerId.primaryBuffer[0]);
    } else {
        /* Fill the secondary buffer */
        pLineObj->callerId.status |= VP_CID_SECONDARY_FULL;
        pMsgLen = &(pLineObj->callerId.secondaryMsgLen);
        pBuffer = &(pLineObj->callerId.secondaryBuffer[0]);
        pLineObj->callerId.cliMSIndex = 0;
    }

    /*
     * If the length is larger than what can fit in the CID buffer, max out
     * the primary message length value. Otherwise, set the primary message
     * length value to the length
     */
    if (length >= VP_SIZEOF_CID_MSG_BUFFER) {
        *pMsgLen = VP_SIZEOF_CID_MSG_BUFFER;
    } else {
        *pMsgLen = length;
    }

    /* Copy the message data to the API buffer */
    for (byteCount = 0; (byteCount < *pMsgLen); byteCount++) {
        pBuffer[byteCount] = pCidData[byteCount];

        pLineObj->callerId.cidCheckSum += pBuffer[byteCount];
        pLineObj->callerId.cidCheckSum = pLineObj->callerId.cidCheckSum % 256;
    }
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
} /* Vp790ContinueCid() */

/**
 * Vp790CtrlSetCliTone()
 *  This function is called by the API internally to enable or disabel the
 * signal generator used for Caller ID.
 *
 * Preconditions:
 *  The line context must be valid (pointing to a Vp790 line object type
 *
 * Postconditions:
 *  The signal generator used for CID tones is enabled/disabled indicated by
 * the mode parameter passed.
 */
VpStatusType
Vp790CtrlSetCliTone(
    VpLineCtxType *pLineCtx,    /**< Line affected by the CLI tones */
    bool mode)                  /**< TRUE = enabled, FALSE = disable tones */
{
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp790DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 ecVal[] = {VP790_EC_CH1, VP790_EC_CH2, VP790_EC_CH3, VP790_EC_CH4};
    uint8 channelId = pLineObj->channelId;
    uint8 sigGenCtrl, sigGenCtrlPre;

    VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_GEN_CTRL_RD, VP790_GEN_CTRL_LEN,
        &sigGenCtrl);
    sigGenCtrlPre = sigGenCtrl;

    if (mode == TRUE) {
        sigGenCtrl |= VP790_GENB_EN;
    } else {
        sigGenCtrl &= ~(VP790_GENB_EN);
    }

    if (sigGenCtrlPre != sigGenCtrl) {
        VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_GEN_CTRL_WRT, VP790_GEN_CTRL_LEN,
            &sigGenCtrl);
    }
    return VP_STATUS_SUCCESS;
}

/**
 * Vp790CtrlSetFSKGen()
 *  This function is called by the CID sequencer executed internally by the API
 *
 * Preconditions:
 *  The line context must be valid (pointing to a Vp790 line object type
 *
 * Postconditions:
 *  The data indicated by mode and data is applied to the line. Mode is used
 * to indicate whether the data is "message", or a special character. The
 * special characters are "channel siezure" (alt. 1/0), "mark" (all 1), or
 * "end of transmission".
 */
void
Vp790CtrlSetFSKGen(
    VpLineCtxType *pLineCtx,        /**< Line affected by the mode and data */
    VpCidGeneratorControlType mode, /**< Indicates the type of data being sent.
                                     * Affects the start and stop bit used
                                     */
    uint8 data)                     /**< 8-bit message data */
{
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp790DeviceObjectType *pDevObj = pDevCtx->pDevObj;

    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 ecVal[] = {VP790_EC_CH1, VP790_EC_CH2, VP790_EC_CH3, VP790_EC_CH4};
    uint8 channelId = pLineObj->channelId;
    uint8 termData = VP790_SIGGEN_FSK_OFF;

    switch(mode) {
        case VP_CID_SIGGEN_EOT:
            data = VP790_DISABLE_CID_WHEN_CMPL;
            break;

        case VP_CID_GENERATOR_DATA:
            /*
             * Nothing special to do at the device level. All data is treated
             * equally. Checksum and End of Message are taken care of if the
             * data management itself.
             */
            break;

        case VP_CID_GENERATOR_KEYED_CHAR:
            switch(data) {
                case VP_FSK_CHAN_SEIZURE:
                    data = 0x95;
                    break;

                case VP_FSK_MARK_SIGNAL:
                    data = 0xBF;
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    if (pLineObj->callerId.status & VP_CID_IN_PROGRESS) {
        VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_CID_DATA_WRT,
            VP790_CID_DATA_LEN, &data);
    } else {
        VpMpiCmdWrapper(deviceId, ecVal[channelId], VP790_CID_DATA_WRT,
            VP790_CID_DATA_LEN, &termData);
    }

    return;
}

/**
 * Vp790MirrorByte()
 *  This function inverts the lower nibble of the byte passed. It is used as
 * part of the 5/3 byte CID message conversion for the VE790 device.
 *
 * Preconditions:
 *  None. Byte inversion only.
 *
 * Postconditions:
 *  None. Returns the inversion of the byte passed.
 */
uint8
Vp790MirrorByte(
    char byte)
{
    char luTable[] = {
        0x00, 0x08, 0x04, 0x0C, 0x02, 0x0A, 0x06, 0x0E, 0x01, 0x09, 0x05,
        0x0D, 0x03, 0x0B, 0x07, 0x0F
    };

    char lsNib;

    lsNib = luTable[byte>>4];
    byte = (luTable[byte & 0x0F] << 4) | lsNib;
    return byte;
}

/**
 * Vp790EncodeData()
 *  This function performs the 5/3 byte CID message conversion for the VE790
 * device. It operates on the first three bytes in the message data passed and
 * writes the five converted byte to the line object pointed to by the line
 * context passed.
 *
 * Preconditions:
 *  The line context passed must be pointing to an instantiated Vp790 type line
 * object.
 *
 * Postconditions:
 *  The line object is updated with 5 bytes of CID message data ready to be
 * written to the device.
 */
VpStatusType
Vp790EncodeData(
    VpLineCtxType *pLineCtx,
    uint8 *pMessageData)
{
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;
    uint8 *pCliData = pLineObj->cidEncodeBuff;
    uint8 cliIndex = 0;
    uint8 byte1 = pMessageData[0];
    uint8 byte2 = pMessageData[1];
    uint8 byte3 = pMessageData[2];
    uint8 done = 0;

    while (done == 0) {
        /*
         * Move data to API Buffer:
         *  Reverse the bit order,
         *  Add Start and Stop Bits,
         *  Break data up into 6bit chunks,
         *  Prepend bits 10 to the 6bit chunks for SLAC programming.
         *  If we've reached the end of the user buffer, then fill remaining
         *   API buffer with 1's.
         * Each 3 bytes of user buffer will equal 5 bytes of API buffer.
         */

        /* Encode User Byte 1 */
        pCliData[cliIndex++] = ((Vp790MirrorByte(byte1) >> 3) & 0x1F) | 0x80;
        pCliData[cliIndex] = (((Vp790MirrorByte(byte1) << 3) & 0x38) | 0x84);
        if (done == 1) {
            pCliData[cliIndex++] |= 0x03;
            pCliData[cliIndex++] = 0xC0;     /* End of Message Indicator */

        /* Encode User Byte 2 */
        } else {
            pCliData[cliIndex++] |= ((Vp790MirrorByte(byte2) >> 7) & 0x01);
            pCliData[cliIndex++] = ((Vp790MirrorByte(byte2) >> 1) & 0x3F) | 0x80;
            pCliData[cliIndex] = (((Vp790MirrorByte(byte2) << 5) & 0x20) | 0x90);
            if (done == 2) {
                pCliData[cliIndex++] |= 0x0F;
                pCliData[cliIndex++] = 0xC0; /*End of Message Indicator */

            /* Encode User Byte 3 */
            } else {
                pCliData[cliIndex++] |= ((Vp790MirrorByte(byte3) >> 5) & 0x07);
                pCliData[cliIndex++] =
                    ((Vp790MirrorByte(byte3) << 1) & 0x3E) | 0x81;
            }
            done = 3;
        }
    }
    if (done == 3) {
        pCliData[cliIndex] = 0xC0;
    }

    return VP_STATUS_SUCCESS;
}

/*
 * This function needs to force a 5mS timer on the API to implement CID
 * in the VE790 device. The device does not provide an interrupt or status
 * register, so the timer is enforced entirely in software dependant on
 * the device tickrate.
 */
bool
Vp790FSKGeneratorReady(
    VpLineCtxType *pLineCtx)
{
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;

    if (pLineObj->thisFskCid == TRUE) {
        return FALSE;
    }

    pLineObj->thisFskCid = TRUE;

    return TRUE;
}

VpCliEncodedDataType
Vp790CliGetEncodedByte(
    VpLineCtxType *pLineCtx,
    uint8 *pByte)
{
    Vp790LineObjectType *pLineObj = pLineCtx->pLineObj;

    VpCallerIdType *pCidStruct = &(pLineObj->callerId);
    VpOptionEventMaskType *pLineEvents = &(pLineObj->lineEvents);
    uint8 remainder;
    uint8 byteCount;
    uint8 tempBuffer[4] = {0xFF, 0xFF, 0xFF, 0xFF};

    uint8 checkSumIndex = VP_CID_PROFILE_FSK_PARAM_LEN +
        pLineObj->callerId.pCliProfile[VP_CID_PROFILE_FSK_PARAM_LEN] +
        VP_CID_PROFILE_CHECKSUM_OFFSET_LSB;

    bool addChecksum = pLineObj->callerId.pCliProfile[checkSumIndex];

    /*
     * If we have enough encoded data to send, then get the data from the
     * encoded buffer.
     */
    if (pLineObj->cidEncodeIndex < pLineObj->cidEncodeSize) {
         *pByte = pLineObj->cidEncodeBuff[pLineObj->cidEncodeIndex];
         pLineObj->cidEncodeIndex++;
         return VP_CLI_ENCODE_DATA;
    } else {
        if (pLineObj->callerId.status & VP_CID_MID_CHECKSUM) {
            pLineObj->callerId.status &= ~VP_CID_MID_CHECKSUM;
            return VP_CLI_ENCODE_END;
        }

        /* We need to create more encoded data and start over */
        pLineObj->cidEncodeIndex = 1;

        /* Get 3 user bytes to convert to the 5 encoded bytes */

        /* Check to determine if we are currently using the primary buffer */
        if (pCidStruct->status & VP_CID_PRIMARY_IN_USE) {
            /*
             * If the index is at the length of the buffer, we need to switch
             * buffers if there is more data
             */
            if (pCidStruct->cliMPIndex >= pCidStruct->primaryMsgLen) {
                /*
                 * At the end of the Primary Buffer. Flag an event and indicate
                 * to the API that this buffer is no longer being used and is
                 * empty
                 */
                pCidStruct->status &= ~VP_CID_PRIMARY_IN_USE;
                pCidStruct->status &= ~VP_CID_PRIMARY_FULL;

                if (pCidStruct->status & VP_CID_SECONDARY_FULL) {
                    pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                    pLineObj->processData = VP_CID_DATA_NEED_MORE_DATA;

                    pCidStruct->status |= VP_CID_SECONDARY_IN_USE;

                    Vp790EncodeData(pLineCtx, &pCidStruct->secondaryBuffer[0]);
                    *pByte = pLineObj->cidEncodeBuff[0];
                    pCidStruct->cliMSIndex = 3;
                    return VP_CLI_ENCODE_DATA;
                } else if (addChecksum) {
                    pCidStruct->status |= VP_CID_SECONDARY_IN_USE;
                    pCidStruct->cliMSIndex = 1;
                    pCidStruct->secondaryMsgLen = 1;
                    tempBuffer[0] =
                        (~pLineObj->callerId.cidCheckSum + 1);
                    if (pLineObj->cidEncodeSize > 3) {
                        pLineObj->cidEncodeSize = 5;
                    } else {
                        pLineObj->cidEncodeSize+=2;
                    }
                    Vp790EncodeData(pLineCtx, tempBuffer);
                    *pByte = pLineObj->cidEncodeBuff[0];
                    pLineObj->callerId.status |= VP_CID_MID_CHECKSUM;
                    return VP_CLI_ENCODE_DATA;
                } else {
                    /* There is no more data in either buffer */
                    *pByte = '\0';
                }
            } else {
                /*
                 * Determine if there are at least 3 bytes remaining in the
                 * buffer and if not, go to the secondary buffer for more
                 * data if it is full
                 */
                remainder =
                    (pCidStruct->primaryMsgLen - pCidStruct->cliMPIndex) % 3;

                if ((pCidStruct->cliMPIndex + 3) <= pCidStruct->primaryMsgLen) {
                    /* There is enough data in the primary buffer to continue */
                    Vp790EncodeData(pLineCtx,
                        &pCidStruct->primaryBuffer[pCidStruct->cliMPIndex]);
                    *pByte = pLineObj->cidEncodeBuff[0];
                    pCidStruct->cliMPIndex+=3;
                    return VP_CLI_ENCODE_DATA;
                } else {
                    /*
                     * There is not enough data in the primary buffer for the
                     * full 3 bytes. Check and use the secondary buffer data
                     * if it is available
                     */
                    for (byteCount = 0;
                         byteCount < remainder;
                         byteCount++) {
                        tempBuffer[byteCount] =
                            pCidStruct->primaryBuffer[pCidStruct->cliMPIndex];
                        pCidStruct->cliMPIndex++;
                    }

                    pCidStruct->status &= ~VP_CID_PRIMARY_IN_USE;
                    pCidStruct->status &= ~VP_CID_PRIMARY_FULL;

                    if (pCidStruct->status & VP_CID_SECONDARY_FULL) {
                        pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                        pLineObj->processData = VP_CID_DATA_NEED_MORE_DATA;

                        pCidStruct->status |= VP_CID_SECONDARY_IN_USE;

                        pCidStruct->cliMSIndex = 0;
                        for (; byteCount < 3;  byteCount++) {
                            tempBuffer[byteCount] =
                                pCidStruct->secondaryBuffer[pCidStruct->cliMSIndex];
                            pCidStruct->cliMSIndex++;
                        }
                    } else {
                        pLineObj->cidEncodeSize = 2 * remainder;
                        if (addChecksum) {
                            pCidStruct->status |= VP_CID_SECONDARY_IN_USE;
                            pCidStruct->cliMSIndex = 1;
                            pCidStruct->secondaryMsgLen = 1;
                            tempBuffer[remainder] =
                                (~pLineObj->callerId.cidCheckSum + 1);
                            if (pLineObj->cidEncodeSize > 3) {
                                pLineObj->cidEncodeSize = 5;
                            } else {
                                pLineObj->cidEncodeSize+=2;
                            }
                        }
                    }
                    Vp790EncodeData(pLineCtx, &tempBuffer[0]);
                    *pByte = pLineObj->cidEncodeBuff[0];
                    return VP_CLI_ENCODE_DATA;
                }
            }
        } else if (pCidStruct->status & VP_CID_SECONDARY_IN_USE) {
            /*
             * If the index is at the length of the buffer, we need to switch
             * buffers if there is more data
             */
            if (pCidStruct->cliMSIndex >= pCidStruct->secondaryMsgLen) {
                /*
                 * At the end of the Primary Buffer. Flag an event and indicate
                 * to the API that this buffer is no longer being used and is
                 * empty
                 */
                pCidStruct->status &= ~VP_CID_SECONDARY_IN_USE;
                pCidStruct->status &= ~VP_CID_SECONDARY_FULL;

                if (pCidStruct->status & VP_CID_PRIMARY_FULL) {
                    pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                    pLineObj->processData = VP_CID_DATA_NEED_MORE_DATA;

                    pCidStruct->status |= VP_CID_PRIMARY_IN_USE;

                    Vp790EncodeData(pLineCtx, &pCidStruct->primaryBuffer[0]);
                    pCidStruct->cliMPIndex = 3;
                    *pByte = pLineObj->cidEncodeBuff[0];
                    return VP_CLI_ENCODE_DATA;
                } else if (addChecksum) {
                    pCidStruct->status |= VP_CID_PRIMARY_IN_USE;
                    pCidStruct->cliMPIndex = 1;
                    pCidStruct->primaryMsgLen = 1;
                    tempBuffer[0] =
                        (~pLineObj->callerId.cidCheckSum + 1);
                    if (pLineObj->cidEncodeSize > 3) {
                        pLineObj->cidEncodeSize = 5;
                    } else {
                        pLineObj->cidEncodeSize+=2;
                    }
                    Vp790EncodeData(pLineCtx, tempBuffer);
                    *pByte = pLineObj->cidEncodeBuff[0];
                    pLineObj->callerId.status |= VP_CID_MID_CHECKSUM;
                    return VP_CLI_ENCODE_DATA;
                } else {
                    /* There is no more data in either buffer */
                    *pByte = '\0';
                }
            } else {
                /*
                 * Determine if there are at least 3 bytes remaining in the
                 * buffer and if not, go to the secondary buffer for more
                 * data if it is full
                 */
                remainder =
                    (pCidStruct->secondaryMsgLen - pCidStruct->cliMSIndex) % 3;
                if ((pCidStruct->cliMSIndex + 3) <= pCidStruct->secondaryMsgLen) {
                    /*
                     * There is enough data in the secondary buffer to
                     * continue
                     */
                    Vp790EncodeData(pLineCtx,
                        &pCidStruct->secondaryBuffer[pCidStruct->cliMSIndex]);
                    *pByte = pLineObj->cidEncodeBuff[0];
                    pCidStruct->cliMSIndex+=3;
                    return VP_CLI_ENCODE_DATA;
                } else {
                    /*
                     * There is not enough data in the secondary buffer for the
                     * full 3 bytes. Check and use the primary buffer data
                     * if it is available
                     */
                    for (byteCount = 0;
                         byteCount < remainder;
                         byteCount++) {
                        tempBuffer[byteCount] =
                            pCidStruct->secondaryBuffer[pCidStruct->cliMSIndex];
                        pCidStruct->cliMSIndex++;
                    }

                    pCidStruct->status &= ~VP_CID_SECONDARY_IN_USE;
                    pCidStruct->status &= ~VP_CID_SECONDARY_FULL;

                    if (pCidStruct->status & VP_CID_PRIMARY_FULL) {
                        pLineEvents->process |= VP_LINE_EVID_CID_DATA;
                        pLineObj->processData = VP_CID_DATA_NEED_MORE_DATA;

                        pCidStruct->status |= VP_CID_PRIMARY_IN_USE;

                        pCidStruct->cliMPIndex = 0;
                        for (; byteCount < 3;  byteCount++) {
                            tempBuffer[byteCount] =
                                pCidStruct->primaryBuffer[pCidStruct->cliMPIndex];
                            pCidStruct->cliMPIndex++;
                        }
                    } else {
                        pLineObj->cidEncodeSize = 2 * remainder;
                        if (addChecksum) {
                            pCidStruct->status |= VP_CID_PRIMARY_IN_USE;
                            pCidStruct->cliMPIndex = 1;
                            pCidStruct->primaryMsgLen = 1;
                            tempBuffer[remainder] =
                                (~pLineObj->callerId.cidCheckSum + 1);
                            if (pLineObj->cidEncodeSize > 3) {
                                pLineObj->cidEncodeSize = 5;
                            } else {
                                pLineObj->cidEncodeSize+=2;
                            }
                        }

                    }
                    Vp790EncodeData(pLineCtx, &tempBuffer[0]);
                    *pByte = pLineObj->cidEncodeBuff[0];
                    return VP_CLI_ENCODE_DATA;
                }
            }
        }
    }

    if ((!(pCidStruct->status & VP_CID_PRIMARY_IN_USE))
     && (!(pCidStruct->status & VP_CID_SECONDARY_IN_USE))) {
        return VP_CLI_ENCODE_END;
    }
    return VP_CLI_ENCODE_DATA;
}
#endif
#endif

