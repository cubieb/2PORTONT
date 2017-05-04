/** \file vp580_seq.c
 * vp580_seq.c
 *
 *  This file contains the VP580 functions called by the API-II Caller ID or
 * sequencer. It is seperated from "normal" API functions for users that want
 * to remove this section of code from the API-II.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_580_SERIES)
#ifdef VP_CSLAC_SEQ_EN

/* INCLUDES */
#include "vp_api_types.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp580_api.h"
#include "vp580_api_int.h"
#include "sys_service.h"

/**< Function called by Send Signal only. Implements FXO digit generation */
static VpStatusType
Vp580SendDigit(
    VpLineCtxType *pLineCtx,
    VpDigitGenerationType digitType,
    VpDigitType digit);

/**
 * Vp580CommandInstruction()
 *  This function implements the Sequencer Command instruction for the Vp580
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
Vp580CommandInstruction(
    VpLineCtxType *pLineCtx,
    VpProfilePtrType pSeqData)
{
    Vp580LineObjectType *pLineObj = pLineCtx->pLineObj;

    /*
     * We know the current value "pSeqData[0]" is 0, now we need to determine if
     * the next command is generator control operator followed by time, or a
     * Line state command -- No other options supported
     */
    switch (pSeqData[0] & VP_SEQ_SUBTYPE_MASK) {
        case VP_SEQ_SUBCMD_LINE_STATE:
            Vp580SetLineStateInt(pLineCtx,
                ConvertPrfWizState2ApiState(pSeqData[1]));
            break;

        default:
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
        switch(pLineObj->cadence.pActiveCadence[VP_PROFILE_TYPE_LSB]) {
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

            default:
                break;

        }
        pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;
        pLineObj->cadence.pActiveCadence = VP_PTABLE_NULL;
    }

    return VP_STATUS_SUCCESS;
}

/**
 * Vp580InitRing()
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
Vp580InitRing(
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
    Vp580LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp580DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpProfilePtrType *pProfileTable;

    int cadIndex = VpGetProfileIndex(pCadProfile);

    /*
     * If this line is a custom termination type and has not been configured
     * with a line state map, return error.
     */
    if ((pLineObj->termType == VP_TERM_FXS_CUSTOM)
     && (pLineObj->lineStateInit == FALSE)) {
        return VP_STATUS_CUSTOM_TERM_NOT_CFG;
    }

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pLineObj->status & VP580_IS_FXO) {
        return VP_STATUS_INVALID_ARG;
    }

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
        return VP_STATUS_ERR_PROFILE;
    } else {
        /* This is a pointer. Set it if it's the correct type */
        if(VpVerifyProfileType(VP_PROFILE_RINGCAD, pCadProfile) == TRUE) {
            pLineObj->pRingingCadence = pCadProfile;
        } else {
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            return VP_STATUS_ERR_PROFILE;
        }
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;
} /* Vp580InitRing */

/**
 * Vp580SendSignal()
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
Vp580SendSignal(
    VpLineCtxType *pLineCtx,
    VpSendSignalType type,
    void *pStruct)
{
    VpDigitType *pDigit;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp580DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpStatusType status;
    Vp580LineObjectType *pLineObj = pLineCtx->pLineObj;

    /*
     * If this line is a custom termination type and has not been configured
     * with a line state map, return error.
     */
    if ((pLineObj->termType == VP_TERM_FXS_CUSTOM)
     && (pLineObj->lineStateInit == FALSE)) {
        return VP_STATUS_CUSTOM_TERM_NOT_CFG;
    }

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->status.state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    switch(type) {
        case VP_SENDSIG_PULSE_DIGIT:
            pDigit = pStruct;
            status = Vp580SendDigit(pLineCtx, VP_DIGIT_GENERATION_DIAL_PULSE,
                *pDigit);
            break;

        case VP_SENDSIG_HOOK_FLASH:
            /* prevent case of *pDigit when user passes VP_NULL */
            status = Vp580SendDigit(pLineCtx, VP_DIGIT_GENERATION_DIAL_HOOK_FLASH,
                VP_DIG_NONE);
            break;

        default:
            status = VP_STATUS_INVALID_ARG;
            break;
    }

    return status;
}

/**
 * Vp580SendDigit()
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
Vp580SendDigit(
    VpLineCtxType *pLineCtx,            /**< Line to send a digit on */
    VpDigitGenerationType digitType,    /**< Type of digit to send. May indicate
                                         * DTMF, Dial Pulse, or Hook Flash
                                         */
    VpDigitType digit)                  /**< The digit to send. Used if type of
                                         * digit is DTMF or Dial Pulse
                                         */
{
    Vp580LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp580DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint16 tempTime, firstTimer, secondTimer;

    uint8 seqByte;

    VpDeviceIdType deviceId = pDevObj->deviceId;

    if (!(pLineObj->status & VP580_IS_FXO)) {
        return VP_STATUS_FUNC_NOT_SUPPORTED;
    }

    if (VpIsDigit(digit) == FALSE) {
        return VP_STATUS_INVALID_ARG;
    }

    switch(digitType) {
        case VP_DIGIT_GENERATION_DIAL_PULSE:
        case VP_DIGIT_GENERATION_DIAL_HOOK_FLASH:
            break;

        default:
            return VP_STATUS_INVALID_ARG;
    }

    /* Parameters passed are good -- proceed */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    for (seqByte = 0; seqByte < VP580_INT_SEQ_LEN; seqByte++) {
        pLineObj->intSequence[seqByte] = 0x00;
    }

    switch(digitType) {
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
            pLineObj->intSequence[10] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = ((firstTimer - 2) >> 8) & 0x03;
            pLineObj->intSequence[10] |= tempTime;

            tempTime = ((firstTimer - 2) & 0x00FF);
            pLineObj->intSequence[11] |= tempTime;

            secondTimer = pLineObj->digitGenStruct.makeTime;
            pLineObj->intSequence[14] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = ((secondTimer - 2) >> 8) & 0x03;
            pLineObj->intSequence[14] |= tempTime;

            tempTime = ((secondTimer - 2) & 0x00FF);
            pLineObj->intSequence[15] |= tempTime;

            firstTimer = pLineObj->digitGenStruct.dpInterDigitTime;
            if (digit > 1) {
                pLineObj->intSequence[16] = VP_SEQ_SPRCMD_BRANCH_INSTRUCTION;
                pLineObj->intSequence[17] = digit - 1;

                pLineObj->intSequence[18] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                tempTime = ((firstTimer - 2) >> 8) & 0x03;
                pLineObj->intSequence[18] |= tempTime;
                tempTime = ((firstTimer - 2) & 0x00FF);
                pLineObj->intSequence[19] |= tempTime;
            } else {
                pLineObj->intSequence[16] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                tempTime = ((firstTimer - 2) >> 8) & 0x03;
                pLineObj->intSequence[16] |= tempTime;
                tempTime = ((firstTimer - 2) & 0x00FF);
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
            pLineObj->intSequence[VP_PROFILE_LENGTH] = 0x0C;
            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = 0x08;

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
            pLineObj->intSequence[10] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = ((firstTimer - 2) >> 8) & 0x03;
            pLineObj->intSequence[10] |= tempTime;

            tempTime = ((firstTimer - 2) & 0x00FF);
            pLineObj->intSequence[11] |= tempTime;

            secondTimer = pLineObj->digitGenStruct.dpInterDigitTime;
            pLineObj->intSequence[14] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            tempTime = ((secondTimer - 2) >> 8) & 0x03;
            pLineObj->intSequence[14] |= tempTime;

            tempTime = ((secondTimer - 2) & 0x00FF);
            pLineObj->intSequence[15] |= tempTime;

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
    return VP_STATUS_SUCCESS;
}
#endif
#endif
