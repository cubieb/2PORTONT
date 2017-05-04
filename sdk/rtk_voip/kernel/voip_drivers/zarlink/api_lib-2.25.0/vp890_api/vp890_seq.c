/** \file vp890_seq.c
 * vp890_seq.c
 *
 *  This file contains the implementation of the VP-API 890 Series
 *  Sequencer Functions.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10011 $
 * $LastChangedDate: 2012-05-16 16:41:01 -0500 (Wed, 16 May 2012) $
 */

/* INCLUDES */
#include    "vp_api.h"

#if defined (VP_CC_890_SERIES)  /* Compile only if required */

#include    "vp_api_int.h"
#include    "vp890_api_int.h"
#include    "sys_service.h"

#ifdef VP_CSLAC_SEQ_EN
#ifdef VP890_FXS_SUPPORT
static VpStatusType
SendMsgWaitPulse(
    VpLineCtxType           *pLineCtx,
    VpSendMsgWaitType       *pMsgWait);

static VpStatusType
SendFwdDisc(
    VpLineCtxType           *pLineCtx,
    uint16                  timeInMs);

static VpStatusType
SendPolRevPulse(
    VpLineCtxType           *pLineCtx,
    uint16                  timeInMs);
#endif

#ifdef VP890_FXO_SUPPORT
static VpStatusType
MomentaryLoopOpen(
    VpLineCtxType           *pLineCtx);

static VpStatusType
SendDigit(
    VpLineCtxType           *pLineCtx,
    VpDigitGenerationType   digitType,
    VpDigitType             digit);
#endif

/*******************************************************************************
 * Vp890SendSignal()
 *  This function sends a signal on the line. The type of signal is specified
 * by the type parameter passed. The structure passed specifies the parameters
 * associated with the signal.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The signal specified is applied to the line.
 ******************************************************************************/
VpStatusType
Vp890SendSignal(
    VpLineCtxType       *pLineCtx,
    VpSendSignalType    type,
    void                *pStruct)
{
    VpDigitType           *pDigit;
    VpDigitType           digit    = VP_DIG_NONE;
    VpDevCtxType          *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpStatusType          status;

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->state, TRUE)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pStruct == VP_NULL) {
        pDigit = &digit;
    } else {
        pDigit = pStruct;
    }

    switch(type) {
#ifdef VP890_FXO_SUPPORT
        case VP_SENDSIG_DTMF_DIGIT:
            status = SendDigit(pLineCtx, VP_DIGIT_GENERATION_DTMF, *pDigit);
            break;

        case VP_SENDSIG_PULSE_DIGIT:
            pDigit = (VpDigitType *)pStruct;
            status = SendDigit(pLineCtx, VP_DIGIT_GENERATION_DIAL_PULSE,
                *pDigit);
            break;

        case VP_SENDSIG_HOOK_FLASH:
            /* prevent case of *pDigit when user passes VP_NULL */
            status = SendDigit(pLineCtx, VP_DIGIT_GENERATION_DIAL_HOOK_FLASH,
                VP_DIG_NONE);
            break;

        case VP_SENDSIG_MOMENTARY_LOOP_OPEN:
            status = MomentaryLoopOpen(pLineCtx);
            break;
#endif

#ifdef VP890_FXS_SUPPORT
        case VP_SENDSIG_MSG_WAIT_PULSE:
            status = SendMsgWaitPulse(pLineCtx, pStruct);
            break;

        case VP_SENDSIG_FWD_DISCONNECT:
            if (pStruct != VP_NULL) {
                status = SendFwdDisc(pLineCtx, *((uint16 *)pStruct));
            } else {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SendSignal() - VP_NULL invalid for FWD_DISCONNECT"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_SENDSIG_POLREV_PULSE:
            if (pStruct != VP_NULL) {
                status = SendPolRevPulse(pLineCtx, *((uint16 *)pStruct));
            } else {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SendSignal() - VP_NULL invalid for POLREV_PULSE"));
                return VP_STATUS_INVALID_ARG;
            }
            break;
#endif

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SendSignal() - Invalid signal type"));
            status = VP_STATUS_INVALID_ARG;
            break;
    }

    return status;
} /* Vp890SendSignal() */

#ifdef VP890_FXS_SUPPORT
/******************************************************************************
 * SendMsgWaitPulse()
 *  This function sends a message waiting pulse to the line specified by the
 * by the pMsgWait parameter passed. The structure specifies a voltage, on-time,
 * off-time, and number of pulses.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  The message waiting signal specified is applied to the line.
 ******************************************************************************/
static VpStatusType
SendMsgWaitPulse(
    VpLineCtxType           *pLineCtx,
    VpSendMsgWaitType       *pMsgWait)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint8                   ecVal       = pLineObj->ecVal;
    uint16                  tickRate    = pDevObj->devProfileData.tickRate;
    VpLineStateType         currentState = pLineObj->lineState.usrCurrent;
    uint8                   addStep     = 0;
    uint32                  aVolt;
    int32                   userVolt;
    uint8                   branchCount;
    uint16                  tempTime, firstTimer, secondTimer;
    uint8                   cmdLen      = 0x08; /* Min Cadence with infinite on */

    /*
     * Set the signal generator parameters to set the A amplitude and frequency
     * "very low". We'll adjust the bias to the user defined MsgWait voltage
     */
    uint8 sigGenBytes[VP890_SIGA_PARAMS_LEN] = {
        0x00, 0x29, 0x73, 0x04, 0x44, 0x00, 0x15, 0x7F, 0xFD, 0x00, 0x00};

    if (pLineObj->status & VP890_IS_FXO) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SendMsgWaitPulse() - Function invalid for FXO"));
        return VP_STATUS_INVALID_ARG;
    }

    /*
     * If we're already in Ringing, return a failure since we're using a
     * shared resource to accomplish this function.
     */
    if ((currentState == VP_LINE_RINGING) || (currentState == VP_LINE_RINGING_POLREV)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SendMsgWaitPulse() - Shared resource in use - ringing"));
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
        VP_ERROR(VpLineCtxType, pLineCtx, ("SendMsgWaitPulse() - 0 voltage not supported"));
        return VP_STATUS_INVALID_ARG;
    }

    /* All parameters passed are good -- proceed */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP890_INT_SEQ_LEN);

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
        return VP_STATUS_SUCCESS;
    }

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
    aVolt = userVolt * (uint32)VP890_RINGING_BIAS_FACTOR;

    /* Scale down by the bit resolution of the device */
    aVolt /= (uint32)VP890_RINGING_BIAS_SCALE;

    sigGenBytes[VP890_SIGA_BIAS_MSB] = (aVolt >> 8) & 0xFF;
    sigGenBytes[VP890_SIGA_BIAS_LSB] = (aVolt & 0xFF);

    /* Write the new signal generator parameters */
    VpMpiCmdWrapper(deviceId, ecVal, VP890_RINGER_PARAMS_WRT,
       VP890_RINGER_PARAMS_LEN, sigGenBytes);

    /* Clear flag to indicate the generators are NOT in a Ringing Mode */
    pLineObj->status &= ~(VP890_RING_GEN_NORM | VP890_RING_GEN_REV);

    /*
     * Build the rest of the cadence defined by the user input (message state
     * set above). Start by setting the type of profile to an API Message Wait
     * Pulse type
     */
    pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
        VP_PRFWZ_PROFILE_MSG_WAIT_PULSE_INT;

    /*
     * Set the timers for on/off pulse duration, scale from mS to tickRate
     * and prevent rounding down to 0
     */
    if (pMsgWait->onTime < (tickRate >> 8)) {
        firstTimer = 3;
    } else {
        firstTimer = MS_TO_TICKRATE(pMsgWait->onTime, tickRate);

        /* Prevent 0 for time (because that means "forever") */
        if (firstTimer <= 2) {
            firstTimer = 3;
        }
    }

    branchCount = 0;
    if (firstTimer > 8192) {
        /* Special Handling for using 16-bit time in 14-bit data fields */
        for (; firstTimer > 8192; branchCount++) {
            firstTimer = ((firstTimer >> 1) & 0x3FFF);
        }
        cmdLen+=2;
    }

    pLineObj->intSequence[10] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
    tempTime = ((firstTimer - 2) >> 8) & 0x1F;
    pLineObj->intSequence[10] |= tempTime;

    tempTime = ((firstTimer - 2) & 0x00FF);
    pLineObj->intSequence[11] = tempTime;

    if (branchCount) {
        pLineObj->intSequence[12] = VP_SEQ_SPRCMD_BRANCH_INSTRUCTION;
        pLineObj->intSequence[12] |= 0x01;  /* On-Time is the step 1 (0 base) */
        pLineObj->intSequence[13] = branchCount;
        addStep+=2;
    }

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
         * Set the timers for on/off pulse duration, scale from mS to tickRate
         * and prevent rounding down to 0
         */
        if (pMsgWait->offTime < (tickRate >> 8)) {
            secondTimer = 3;
        } else {
            secondTimer = MS_TO_TICKRATE(pMsgWait->offTime, tickRate);

            /* Prevent 0 for time (because that means "forever") */
            if (secondTimer <= 2) {
                secondTimer = 3;
            }
        }

        branchCount = 0;
        if (secondTimer > 8192) {
            cmdLen+=2;   /* Add two for the off-time branch loop */
            /* Special Handling for using 16-bit time in 14-bit data fields */
            for (; secondTimer > 8192; branchCount++) {
                secondTimer = ((secondTimer >> 1) & 0x3FFF);
            }
        }

        pLineObj->intSequence[14+addStep] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
        tempTime = ((secondTimer - 2) >> 8) & 0x1F;
        pLineObj->intSequence[14+addStep] |= tempTime;

        tempTime = ((secondTimer - 2) & 0x00FF);
        pLineObj->intSequence[15+addStep] = tempTime;

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
    }

    /*
     * Set the line object cadence variables to this sequence and activate the
     * sequencer
     */
    pLineObj->intSequence[VP_PROFILE_LENGTH] = cmdLen;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = cmdLen - 4;

    pLineObj->cadence.index = VP_PROFILE_TYPE_SEQUENCER_START;
    pLineObj->cadence.length = pLineObj->intSequence[VP_PROFILE_LENGTH];

    pLineObj->cadence.pActiveCadence = &pLineObj->intSequence[0];
    pLineObj->cadence.pCurrentPos = &pLineObj->intSequence[8];

    pLineObj->cadence.status |= VP_CADENCE_STATUS_ACTIVE;
    pLineObj->cadence.status |= VP_CADENCE_STATUS_SENDSIG;

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    return VP_STATUS_SUCCESS;
} /* SendMsgWaitPulse() */

/******************************************************************************
 * SendFwdDisc()
 *  This function sends a forward disconnect to the line specified for a duration
 * given in mS.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  A disconnect has been applied to the line, the line state is restored to what
 * it was prior to this function being called. No events are generated while a
 * disconnect is occuring -- the application should know that it is not possible
 * to detect a line condition while no feed is being presented.
 ******************************************************************************/
static VpStatusType
SendFwdDisc(
    VpLineCtxType               *pLineCtx,
    uint16                      timeInMs)
{
    Vp890LineObjectType         *pLineObj     = pLineCtx->pLineObj;
    VpLineStateType             currentState  = pLineObj->lineState.usrCurrent;
    VpProfileCadencerStateTypes cadenceState;

    VpDevCtxType                *pDevCtx      = pLineCtx->pDevCtx;
    Vp890DeviceObjectType       *pDevObj      = pDevCtx->pDevObj;
    VpDeviceIdType              deviceId      = pDevObj->deviceId;

    uint16                      timeIn5mS     = 0;
    uint8                       index, tickTime;

    if (pLineObj->status & VP890_IS_FXO) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SendFwdDisc() - Function invalid for FXO"));
        return VP_STATUS_INVALID_ARG;
    }

    cadenceState = ConvertApiState2PrfWizState(currentState);

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP890_INT_SEQ_LEN);

    /* Set the cadence type */
    pLineObj->intSequence[VP_PROFILE_TYPE_LSB] = VP_PRFWZ_PROFILE_FWD_DISC_INT;

    /* First step is to go to disconnect */
    index = 0;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_PROFILE_CADENCE_STATE_DISCONNECT;

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

    return VP_STATUS_SUCCESS;
} /* SendFwdDisc() */

/******************************************************************************
 * SendPolRevPulse()
 *  This function sends a polarity reversal pulse to the line specified for a
 * duration given in ms.
 *
 * Preconditions:
 *  The line must first be initialized.
 *
 * Postconditions:
 *  A polarity reversal has been applied to the line, the line state is restored
 * to what it was prior to this function being called. A hook event may be
 * generated while the polarity reversal is occuring. The application should
 * ignore as appropriate based on expected loop conditions.
 ******************************************************************************/
static VpStatusType
SendPolRevPulse(
    VpLineCtxType               *pLineCtx,
    uint16                      timeInMs)
{
    Vp890LineObjectType         *pLineObj     = pLineCtx->pLineObj;
    VpLineStateType             currentState  = pLineObj->lineState.usrCurrent;
    VpProfileCadencerStateTypes cadenceState, polRevState;

    VpDevCtxType                *pDevCtx      = pLineCtx->pDevCtx;
    Vp890DeviceObjectType       *pDevObj      = pDevCtx->pDevObj;
    VpDeviceIdType              deviceId      = pDevObj->deviceId;

    uint16                      timeIn5mS     = 0;
    uint8                       index, tickTime;

    if (pLineObj->status & VP890_IS_FXO) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("SendPolRevPulse() - Function invalid for FXO"));
        return VP_STATUS_INVALID_ARG;
    }

    if (currentState == VP_LINE_DISCONNECT) {
        return VP_STATUS_INVALID_ARG;
    }

    cadenceState = ConvertApiState2PrfWizState(currentState);
    polRevState = ConvertApiState2PrfWizState(VpGetReverseState(currentState));

    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("PolRevPulse: Current API State %d Reverse API State %d - Cadence State %d Cadence Reverse State %d",
        currentState, VpGetReverseState(currentState), cadenceState, polRevState));

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP890_INT_SEQ_LEN);

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

    return VP_STATUS_SUCCESS;
} /* SendPolRevPulse() */
#endif

#ifdef VP890_FXO_SUPPORT
/******************************************************************************
 * MomentaryLoopOpen()
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
 ******************************************************************************/
static VpStatusType
MomentaryLoopOpen(
    VpLineCtxType               *pLineCtx)
{
    Vp890LineObjectType         *pLineObj    = pLineCtx->pLineObj;
    VpLineStateType             currentState = pLineObj->lineState.usrCurrent;
    VpProfileCadencerStateTypes cadenceState;

    VpDevCtxType                *pDevCtx     = pLineCtx->pDevCtx;
    Vp890DeviceObjectType       *pDevObj     = pDevCtx->pDevObj;
    VpDeviceIdType              deviceId     = pDevObj->deviceId;

    uint16                      timeIn5mS    = 0;
    uint8                       index;

    if (!(pLineObj->status & VP890_IS_FXO)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("MomentaryLoopOpen() - Function invalid for FXS"));
        return VP_STATUS_INVALID_ARG;
    }

    cadenceState = ConvertApiState2PrfWizState(currentState);

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP890_INT_SEQ_LEN);

    /* Set the cadence type */
    pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
        VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT;

    /* First step is to go to Loop Open */
    index = 0;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);

    index++;
    pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START + index]
        = VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN;

    /* Then wait for at least 320ms. The time is higher by 2*ApiTick value
       Afterward, when the cadence ends, CommandInstruction will read the LIU
       bit and generate an event if necessary */
    timeIn5mS = 64;  /* Cadencer Tick is 5ms increments */

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

    return VP_STATUS_SUCCESS;
} /* MomentaryLoopOpen() */

/******************************************************************************
 * SendDigit()
 *  This function sends a DTMF or Dial Pulse digit on an FXO line. It creates
 * a sequencer compatible profile to control the FXO loop open, loop close, and
 * time operators.
 *
 * Arguments:
 *  *pLineCtx - Line to send a digit on
 *  digitType - Type of digit to send. May indicate DTMF, Dial Pulse,
 *               or Hook Flash
 *  digit     - The digit to send. Used if type of digit is DTMF or Dial Pulse
 *
 * Preconditions:
 *  The line must first be initialized and must be of FXO type.
 *
 * Postconditions:
 *  The digit specified is sent on the line in the form specified (DTMF or Dial
 * Pulse).  This function returns the success code if the line is an FXO type of
 * line, if the digit is between 0 - 9, and if the digit type is either DTMF or
 * Dial Pulse.
 ******************************************************************************/
static VpStatusType
SendDigit(
    VpLineCtxType           *pLineCtx,
    VpDigitGenerationType   digitType,
    VpDigitType             digit)
{
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint16                  tempTime, firstTimer, secondTimer;
    uint16                  tickAdjustment;

    switch(digitType) {
        case VP_DIGIT_GENERATION_DTMF:
        case VP_DIGIT_GENERATION_DIAL_PULSE:
            if (!(pLineObj->status & VP890_IS_FXO)) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Function invalid for FXS"));
                return VP_STATUS_INVALID_ARG;
            }
            if ((VpIsDigit(digit) == FALSE) || (digit == VP_DIG_NONE)) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Invalid digit"));
                return VP_STATUS_INVALID_ARG;
            }
            break;

        case VP_DIGIT_GENERATION_DIAL_HOOK_FLASH:
            if (!(pLineObj->status & VP890_IS_FXO)) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Function invalid for FXS"));
                return VP_STATUS_INVALID_ARG;
            }

            if ((pLineObj->lineState.currentState != VP_LINE_FXO_TALK)
             && (pLineObj->lineState.currentState != VP_LINE_FXO_LOOP_CLOSE)) {
                return VP_STATUS_INVALID_ARG;
            }
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Invalid digitType"));
            return VP_STATUS_INVALID_ARG;
    }

    /* Parameters passed are good -- proceed */
    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    /*
     * This is implemented with the cadencer so we have to stop all previous
     * sequences first
     */
    pLineObj->cadence.status = VP_CADENCE_RESET_VALUE;    /* No active status */

    VpMemSet(pLineObj->intSequence, 0, VP890_INT_SEQ_LEN);

    /* Tick adjustment in 5ms cadence time units
     * This is used to adjust delay commands for the time taken to execute
     * non-delay commands */
    tickAdjustment = TICKS_TO_MS(1, pDevObj->devProfileData.tickRate);
    if (tickAdjustment % 5 > 2) {
        tickAdjustment = (tickAdjustment / 5) + 1;
    } else {
        tickAdjustment = (tickAdjustment / 5);
    }

    switch(digitType) {
        case VP_DIGIT_GENERATION_DTMF:
            Vp890MuteChannel(pLineCtx, TRUE);
            Vp890SetDTMFGenerators(pLineCtx, VP_CID_NO_CHANGE, digit);

            /* Fixed total length and sequence length for FLASH generation */
            pLineObj->intSequence[VP_PROFILE_LENGTH] = 0x0C;
            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] = 0x08;

            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_START]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_SIGGEN);

            pLineObj->intSequence[12]
                = (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_SIGGEN);

            pLineObj->intSequence[9] =
                (VP_SEQ_SIGGEN_C_EN | VP_SEQ_SIGGEN_D_EN);

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

        case VP_DIGIT_GENERATION_DIAL_PULSE: {
            /* Sequence:
             *   Starting state: Make
             *   1. Break
             *   2.   Break delay
             *   3. Make
             *   4.   Make delay
             *   5. Break
             *   6.   Break Delay
             *   7. Loop to 3
             *   8. Make
             *   9.   Make Delay
             *   10.Interdigit Delay
             *
             *  The seemingly extraneous Break at the beginning and Make at the
             *  end are there to allow a shorter minimum time for Make than for
             *  Break by including the branch instruction in the Break duration.
             */
            uint8 index;
            uint8 branchTarget;

            index = VP_PROFILE_TYPE_SEQUENCER_START;
            branchTarget = 0;

            /* Start initial Break period. */
            pLineObj->intSequence[index] =
                (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);
            index++;
            if (pLineObj->lineState.currentState == VP_LINE_FXO_TALK) {
                pLineObj->intSequence[index] =
                    VP_PROFILE_CADENCE_STATE_FXO_OHT;
            } else {
                pLineObj->intSequence[index] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN;
            }
            index++;
            branchTarget++;

            /* Break duration after adjusting by 1 tick.  Require at least
             * 1 tick delay here to match the looping break duration which
             * includes a tick to execute the branch instruction */
            tempTime = pLineObj->digitGenStruct.breakTime;
            if (tempTime > tickAdjustment) {
                tempTime -= tickAdjustment;
            } else {
                tempTime = tickAdjustment;
            }
            pLineObj->intSequence[index] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
            pLineObj->intSequence[index] |= (tempTime & 0x1F00) >> 8;
            index++;
            pLineObj->intSequence[index] = (tempTime & 0x00FF);
            index++;
            branchTarget++;

            if (digit > 1) {
                /* Branch target is here */

                /* Start Looping Make period */
                pLineObj->intSequence[index] =
                    (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);
                index++;
                if (pLineObj->lineState.currentState == VP_LINE_FXO_TALK) {
                    pLineObj->intSequence[index] =
                        VP_PROFILE_CADENCE_STATE_FXO_LOOP_TALK;
                } else {
                    pLineObj->intSequence[index] =
                        VP_PROFILE_CADENCE_STATE_FXO_LOOP_CLOSE;
                }
                index++;

                /* Make duration, if needed after adjusting by 1 tick */
                tempTime = pLineObj->digitGenStruct.makeTime;
                if (tempTime > tickAdjustment) {
                    tempTime -= tickAdjustment;
                } else {
                    tempTime = 0;
                }
                if (tempTime > 0) {
                    pLineObj->intSequence[index] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                    pLineObj->intSequence[index] |= (tempTime & 0x1F00) >> 8;
                    index++;
                    pLineObj->intSequence[index] = (tempTime & 0x00FF);
                    index++;
                }

                /* Start looping Break period. */
                pLineObj->intSequence[index] =
                    (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);
                index++;
                if (pLineObj->lineState.currentState == VP_LINE_FXO_TALK) {
                    pLineObj->intSequence[index] =
                        VP_PROFILE_CADENCE_STATE_FXO_OHT;
                } else {
                    pLineObj->intSequence[index] =
                        VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN;
                }
                index++;

                /* Break duration, if needed after adjusting for ticks
                 * -2 ticks for digits > 2 to account for the branch
                 * -1 tick for a digit 2 because there will be no branch */
                tempTime = pLineObj->digitGenStruct.breakTime;
                if (digit > 2) {
                    if (tempTime > tickAdjustment * 2) {
                        tempTime -= tickAdjustment * 2;
                    } else {
                        tempTime = 0;
                    }
                } else {
                    if (tempTime > tickAdjustment) {
                        tempTime -= tickAdjustment;
                    } else {
                        tempTime = 0;
                    }
                }
                if (tempTime > 0) {
                    pLineObj->intSequence[index] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                    pLineObj->intSequence[index] |= (tempTime & 0x1F00) >> 8;
                    index++;
                    pLineObj->intSequence[index] = (tempTime & 0x00FF);
                    index++;
                }

                /* Branch to beginning of the loop if needed */
                if (digit > 2) {
                    pLineObj->intSequence[index] = VP_SEQ_SPRCMD_BRANCH_INSTRUCTION;
                    pLineObj->intSequence[index] |= branchTarget;
                    index++;
                    pLineObj->intSequence[index] = digit - 2;
                    index++;
                }
            }

            /* Return to Make state */
            pLineObj->intSequence[index] =
                (VP_SEQ_SPRCMD_COMMAND_INSTRUCTION | VP_SEQ_SUBCMD_LINE_STATE);
            index++;
            if (pLineObj->lineState.currentState == VP_LINE_FXO_TALK) {
                pLineObj->intSequence[index] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_TALK;
            } else {
                pLineObj->intSequence[index] =
                    VP_PROFILE_CADENCE_STATE_FXO_LOOP_CLOSE;
            }
            index++;

            /* Make duration, if needed after adjusting by 1 tick */
            tempTime = pLineObj->digitGenStruct.makeTime;
            if (tempTime > tickAdjustment) {
                tempTime -= tickAdjustment;
            } else {
                tempTime = 0;
            }
            if (tempTime > 0) {
                pLineObj->intSequence[index] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                pLineObj->intSequence[index] |= (tempTime & 0x1F00) >> 8;
                index++;
                pLineObj->intSequence[index] = (tempTime & 0x00FF);
                index++;
            }

            /* Interdigit time */
            tempTime = pLineObj->digitGenStruct.dpInterDigitTime;
            if (tempTime > 0) {
                pLineObj->intSequence[index] = VP_SEQ_SPRCMD_TIME_INSTRUCTION;
                pLineObj->intSequence[index] |= (tempTime & 0x1F00) >> 8;
                index++;
                pLineObj->intSequence[index] = (tempTime & 0x00FF);
                index++;
            }

            /* Fill in sequence length */
            pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] =
                index - VP_PROFILE_TYPE_SEQUENCER_START;

            /* Profile Length is always sequence lengh + 4 because of header
             * definition. */
            pLineObj->intSequence[VP_PROFILE_LENGTH] =
                pLineObj->intSequence[VP_PROFILE_TYPE_SEQUENCER_COUNT_LSB] + 4;

            /* Sequence type */
            pLineObj->intSequence[VP_PROFILE_TYPE_LSB] =
                VP_PRFWZ_PROFILE_DIAL_PULSE_DIG_GEN;
            break;
        }
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

            secondTimer = pLineObj->digitGenStruct.dpInterDigitTime;
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
                VP_PRFWZ_PROFILE_HOOK_FLASH_DIG_GEN;
            break;

        default:
            /*
             * This can only occur if there is an error in the error checking
             * above.
             */
            VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
            VP_ERROR(VpLineCtxType, pLineCtx, ("SendDigit() - Invalid digitType; should not get here"));
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
} /* SendDigit() */
#endif

/*******************************************************************************
 * Vp890CommandInstruction()
 *  This function implements the Sequencer Command instruction for the Vp890
 * device type.
 *
 * Preconditions:
 *  The line must first be initialized and the sequencer data must be valid.
 *
 * Postconditions:
 *  The command instruction currently being pointed to by the sequencer
 * instruction passed is acted upon.  The sequencer may or may not be advanced,
 * depending on the specific command instruction being executed.
 ******************************************************************************/
VpStatusType
Vp890CommandInstruction(
    VpLineCtxType           *pLineCtx,
    VpProfilePtrType        pSeqData)
{
    Vp890LineObjectType     *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType            *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId  = pDevObj->deviceId;

    uint8                   ecVal     = pLineObj->ecVal;
    uint8                   channelId = pLineObj->channelId;

#ifdef VP890_FXS_SUPPORT
    uint8                   lineState;
    uint8                   lsConfig[VP890_LOOP_SUP_LEN];
#endif

    /*
     * We know the current value "pSeqData[0]" is 0, now we need to determine if
     * the next command is generator control operator followed by time, or a
     * Line state command -- No other options supported
     */
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("890 Command 0x%02X", pSeqData[0]));

    switch (pSeqData[0] & VP_SEQ_SUBTYPE_MASK) {
        case VP_SEQ_SUBCMD_SIGGEN: {
            /* Disable everything. */
            uint8 sigGenCtrl[] = {0};
            VP_SEQUENCER(VpLineCtxType, pLineCtx,
                         ("VP_SEQ_SUBCMD_SIGGEN for generators 0x%02X", (pSeqData[1] & 0xF)));

            /* Get the signal generator bits and set. */
            sigGenCtrl[0] |= ((pSeqData[1] & 0x01) ?  VP890_GENA_EN : 0);
            sigGenCtrl[0] |= ((pSeqData[1] & 0x02) ?  VP890_GENB_EN : 0);
            sigGenCtrl[0] |= ((pSeqData[1] & 0x04) ?  VP890_GENC_EN : 0);
            sigGenCtrl[0] |= ((pSeqData[1] & 0x08) ?  VP890_GEND_EN : 0);

            if (pLineObj->sigGenCtrl[0] != sigGenCtrl[0]) {
                pLineObj->sigGenCtrl[0] = sigGenCtrl[0];
                VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT,
                    VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }
            }
            break;

        case VP_SEQ_SUBCMD_LINE_STATE:
            VP_SEQUENCER(VpLineCtxType, pLineCtx, ("VP_SEQ_SUBCMD_LINE_STATE %d", pSeqData[0]));

            switch(pSeqData[1]) {
#ifdef VP890_FXS_SUPPORT
                case VP_PROFILE_CADENCE_STATE_STANDBY:
                case VP_PROFILE_CADENCE_STATE_POLREV_STANDBY:
                case VP_PROFILE_CADENCE_STATE_TALK:
                case VP_PROFILE_CADENCE_STATE_ACTIVE:
                case VP_PROFILE_CADENCE_STATE_OHT:
                case VP_PROFILE_CADENCE_STATE_DISCONNECT:
                case VP_PROFILE_CADENCE_STATE_RINGING:
                case VP_PROFILE_CADENCE_STATE_POLREV_RINGING:
                case VP_PROFILE_CADENCE_STATE_POLREV_ACTIVE:
                case VP_PROFILE_CADENCE_STATE_POLREV_TALK:
                case VP_PROFILE_CADENCE_STATE_POLREV_OHT:
                    VP_LINE_STATE(VpLineCtxType, pLineCtx,
                        ("Channel %d Changing Line API State to %d at time %d for Cadencing",
                        pLineObj->channelId, ConvertPrfWizState2ApiState(pSeqData[1]), pDevObj->timeStamp));
                    Vp890SetFxsLineState(pLineCtx,
                        ConvertPrfWizState2ApiState(pSeqData[1]));
                    break;

                case VP_PROFILE_CADENCE_STATE_MSG_WAIT_NORM:
                case VP_PROFILE_CADENCE_STATE_MSG_WAIT_POLREV:
                    VpMemCpy(lsConfig, pLineObj->loopSup, VP890_LOOP_SUP_LEN);
                    if (lsConfig[VP890_LOOP_SUP_RT_MODE_BYTE]
                        & VP890_RING_TRIP_AC) {
                        if (!(pLineObj->status & VP890_BAD_LOOP_SUP)) {
                            pLineObj->status |= VP890_BAD_LOOP_SUP;
                        }

                        /* Force DC Trip */
                        lsConfig[VP890_LOOP_SUP_RT_MODE_BYTE] &=
                            ~VP890_RING_TRIP_AC;
                        /*
                         * Note that this does not update the cached values
                         * because the cached values hold the Init/Config
                         * values provided by the application. This method
                         * modifies the device directly when MSG Waiting is
                         * enabled, and restores from the cached values when
                         * MSG Waiting is disabled.
                         */
                        VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOP_SUP_WRT,
                            VP890_LOOP_SUP_LEN, lsConfig);
                    }

                    lineState =
                        (pSeqData[1] == VP_PROFILE_CADENCE_STATE_MSG_WAIT_NORM) ?
                        VP890_SS_BALANCED_RINGING :
                        VP890_SS_BALANCED_RINGING_PR;

                    Vp890LLSetSysState(deviceId, pLineCtx, lineState, TRUE);
                    break;
#endif

#ifdef VP890_FXO_SUPPORT
                case VP_PROFILE_CADENCE_STATE_FXO_LOOP_OPEN:
                case VP_PROFILE_CADENCE_STATE_FXO_OHT:
                case VP_PROFILE_CADENCE_STATE_FXO_LOOP_CLOSE:
                case VP_PROFILE_CADENCE_STATE_FXO_LOOP_TALK:
                    Vp890SetFxoLineState(pLineCtx, ConvertPrfWizState2ApiState(pSeqData[1]));
                    break;
#endif

                default:
                    VP_ERROR(VpLineCtxType, pLineCtx,
                             ("Vp890CommandInstruction() - Invalid sequencer state command"));
                    return VP_STATUS_INVALID_ARG;
            }
            break;

#ifdef VP890_FXS_SUPPORT
        case VP_SEQ_SUBCMD_START_CID:
        case VP_SEQ_SUBCMD_WAIT_ON:
            VP_SEQUENCER(VpLineCtxType, pLineCtx,
                 ("%s", ((pSeqData[0] & VP_SEQ_SUBTYPE_MASK) == VP_SEQ_SUBCMD_START_CID
                         ? "VP_SEQ_SUBCMD_START_CID" : "VP_SEQ_SUBCMD_WAIT_ON")));

            if (pLineObj->pCidProfileType1 != VP_PTABLE_NULL) {
                pLineObj->callerId.pCliProfile = pLineObj->pCidProfileType1;
                VpCSLACInitCidStruct(&pLineObj->callerId, pSeqData[0]);
            }
            break;

        case VP_SEQ_SUBCMD_RAMP_GENERATORS:
            if(VpCSLACProcessRampGenerators(&pLineObj->cadence)) {
                VP_SEQUENCER(VpLineCtxType, pLineCtx,
                             ("New Signal Generator Values: 0x%02X 0x%02X 0x%02X 0x%02X at time %d",
                              pLineObj->cadence.regData[3], pLineObj->cadence.regData[4],
                              pLineObj->cadence.regData[5], pLineObj->cadence.regData[6],
                              pDevObj->timeStamp));

                VpMpiCmdWrapper(deviceId, ecVal, VP890_SIGAB_PARAMS_WRT,
                    VP890_SIGAB_PARAMS_LEN, pLineObj->cadence.regData);
                /* Clear flag to indicate the generators are NOT in a Ringing Mode */
                pLineObj->status &= ~(VP890_RING_GEN_NORM | VP890_RING_GEN_REV);
            }
            break;
#endif

        default:
            VP_ERROR(VpLineCtxType, pLineCtx,
                     ("Vp890CommandInstruction() - Invalid sequencer command"));
            return VP_STATUS_INVALID_ARG;
    }

    /*
     * Check to see if there is more sequence data, and if so, move the
     * sequence pointer to the next command. Otherwise, end this cadence
     */
    pLineObj->cadence.index+=2;
    VP_SEQUENCER(VpLineCtxType, pLineCtx, ("New Index %d Length %d",
        pLineObj->cadence.index, pLineObj->cadence.length));

    if (pLineObj->cadence.index <
       (pLineObj->cadence.length + VP_PROFILE_LENGTH + 1)) {
        pSeqData+=2;
        pLineObj->cadence.pCurrentPos = pSeqData;

        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("Continue Cadence..."));
    } else {
        VP_SEQUENCER(VpLineCtxType, pLineCtx, ("End Cadence"));

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

            case VP_PRFWZ_PROFILE_MOMENTARY_LOOP_OPEN_INT:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_MOMENTARY_LOOP_OPEN;
                if (pDevObj->intReg[channelId] & VP890_LIU_MASK) {
                    pLineObj->lineEventHandle = 1;
                } else {
                    pLineObj->lineEventHandle = 0;
                }
                VpMpiCmdWrapper(deviceId, ecVal, VP890_LOOP_SUP_WRT,
                    VP890_LOOP_SUP_LEN, pLineObj->loopSup);
                break;

            case VP_PRFWZ_PROFILE_DTMF_DIG_GEN:
                pLineObj->lineEvents.process |= VP_LINE_EVID_SIGNAL_CMP;
                pLineObj->processData = VP_SENDSIG_DTMF_DIGIT;
                Vp890MuteChannel(pLineCtx, FALSE);
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

    return VP_STATUS_SUCCESS;
} /* Vp890CommandInstruction() */

#ifdef VP890_FXS_SUPPORT
/*******************************************************************************
 * Vp890FSKGeneratorReady()
 *  This function is used for Caller ID to determine if the FSK generator is
 *  ready to accept another byte. It uses the device caller ID state machine
 *  and signaling (caller ID status) register. This function should be called
 *  from an API internal function only.
 *
 * Prototype is in vp_api_int.h
 *
 * Arguments:
 *
 * Preconditions:
 *
 * Returns:
 *  TRUE if the FSK generator for Caller ID can accept a byte, FALSE otherwise.
 ******************************************************************************/
bool
Vp890FSKGeneratorReady(
    VpLineCtxType         *pLineCtx)
{
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;
    uint8                 stateRetry, stateIndex;
    uint8                 cidState[VP890_CID_PARAM_LEN];

    uint8 numBytes[] = {
        3,  /* 0 = Idle                 #define VP890_CID_STATE_IDLE        0x00 */
        2,  /* 1 = Empty                #define VP890_CID_STATE_EMPTY_D     0x20 */
        1,  /* 2 = Half-Full            #define VP890_CID_STATE_HALF_FULL_D 0x40 */
        2,  /* 3 = Last Byte - EOM      #define VP890_CID_STATE_LBYTE_D     0x60 */
        1,  /* 4 = Last 2 Byte - EOM    #define VP890_CID_STATE_L2BYTE_D    0x80 */
        2,  /* 5 = Underrun */
        0,  /* 6 = Full                 #define VP890_CID_STATE_FULL_D      0xC0 */
        0   /* 7 = Last 3 Byte - EOM    #define VP890_CID_STATE_L3BYTE_D    0xE0
             * There should be no way to get here. It occurs when "FUll" state and
             * write + EOM, which the VP-API-II does not do.
             */
    };
    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890FSKGeneratorReady()"));

    if (pLineObj->tickBeginState[0] == VP890_CID_STATE_FULL_D) {
        pLineObj->delayConsumed = TRUE;

#ifdef VP_CID_POLLING_ENABLED
        stateRetry = VP_CID_NORMAL_MPI_CNT;
#else
        stateRetry = 1;
#endif
        do {
            stateRetry--;

            /* Check the Generator State until it is no longer full */
            VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_RD, VP890_CID_PARAM_LEN,
                cidState);
            cidState[0] &= VP890_CID_STATE_MASK;

            VP_CID(VpLineCtxType, pLineCtx, ("CID State 0x%02X", cidState[0]));
            if (cidState[0] != VP890_CID_STATE_FULL_D) {
                stateRetry = 0;
            }
        } while (stateRetry != 0);
    } else {
        cidState[0] = (pLineObj->tickBeginState[0] & VP890_CID_STATE_MASK);
    }

    stateIndex = ((cidState[0] >> 5) & 0x7);
    pLineObj->cidBytesRemain = numBytes[stateIndex];

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890FSKGeneratorReady()"));
    return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
} /* Vp890FSKGeneratorReady() */

/*******************************************************************************
 * Vp890CliGetEncodedByte()
 *  This function returns an encoded byte of data that is suitable for writing
 *  the FSK generator (device dependent).
 *
 * Preconditions
 *  Must have a valid CLI packet in to work from.
 *
 * Postconditions
 *  The per-channel caller ID buffer will be updated with encoded data.
 *
 ******************************************************************************/
VpCliEncodedDataType
Vp890CliGetEncodedByte(
    VpLineCtxType           *pLineCtx,
    uint8                   *pByte)
{
    Vp890LineObjectType     *pLineObj       = pLineCtx->pLineObj;
    VpOptionEventMaskType   *pLineEvents    = &(pLineObj->lineEvents);
    VpCallerIdType          *pCidStruct     = &(pLineObj->callerId);

    uint8 checkSumIndex = VP_CID_PROFILE_FSK_PARAM_LEN +
        pLineObj->callerId.pCliProfile[VP_CID_PROFILE_FSK_PARAM_LEN] +
        VP_CID_PROFILE_CHECKSUM_OFFSET_LSB;

    if (pLineObj->suspendCid == TRUE) {
        *pByte = '\0';
        VP_CID(VpLineCtxType, pLineCtx, ("VE890 EOM In Progress..."));
        return VP_CLI_ENCODE_END;
                }

    return VpCSLACCliGetEncodedByte(pByte, pCidStruct, &pLineObj->processData,
                pLineEvents, checkSumIndex);
} /* Vp890CliGetEncodedByte() */

/*******************************************************************************
 * Vp890CtrlSetCliTone()
 *  This function is called by the API internally to enable or disable the
 *  signal generator used for Caller ID.
 *
 * Preconditions:
 *  The line context must be valid (pointing to a Vp890 line object type
 *
 * Postconditions:
 *  The signal generator used for CID tones is enabled/disabled indicated by
 *  the mode parameter passed.
 *
 ******************************************************************************/
VpStatusType
Vp890CtrlSetCliTone(
    VpLineCtxType         *pLineCtx,
    bool                  mode)
{
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;
    uint8                 sigGenCtrl[] = {0};

    /*
     * This function should only be called when the Caller ID sequence is
     * generating an alerting tone. We're using the C/D generators, so disable
     * A/B and enable C/D only (if mode == TRUE).
     */
    if (mode == TRUE) {
        sigGenCtrl[0] = (VP890_GENC_EN | VP890_GEND_EN);
    }

    if (pLineObj->sigGenCtrl[0] != sigGenCtrl[0]) {
        pLineObj->sigGenCtrl[0] = sigGenCtrl[0];
        VpMpiCmdWrapper(deviceId, ecVal, VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN,
            pLineObj->sigGenCtrl);
    }

    return VP_STATUS_SUCCESS;
} /* Vp890CtrlSetCliTone() */

/*******************************************************************************
 * Vp890CtrlSetFSKGen()
 *  This function is called by the CID sequencer executed internally by the API
 *
 * Preconditions:
 *  The line context must be valid (pointing to a VP890 line object type
 *
 * Postconditions:
 *  The data indicated by mode and data is applied to the line. Mode is used
 * to indicate whether the data is "message", or a special character. The
 * special characters are "channel siezure" (alt. 1/0), "mark" (all 1), or
 * "end of transmission".
 *
 ******************************************************************************/
bool
Vp890CtrlSetFSKGen(
    VpLineCtxType *pLineCtx,        /**< Line affected by the mode and data */
    VpCidGeneratorControlType mode, /**< Indicates the type of data being sent.
                                     * Affects the start and stop bit used
                                     */
    uint8 data)                     /**< 8-bit message data */
{
    Vp890LineObjectType   *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType          *pDevCtx  = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj  = pDevCtx->pDevObj;
    VpDeviceIdType        deviceId  = pDevObj->deviceId;
    uint8                 ecVal     = pLineObj->ecVal;
    bool                  moreData  = TRUE;
    bool                  initFsk   = FALSE;
    uint8                 fskParam[VP890_CID_PARAM_LEN];
    bool                  returnStatus = FALSE;

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("+Vp890CtrlSetFSKGen()"));

    fskParam[0] = pLineObj->tickBeginState[0];
    fskParam[0] &= ~(VP890_CID_FRAME_BITS);
    if (fskParam[0] & VP890_CID_DIS) {
        initFsk = TRUE;
    }

    switch(mode) {
        case VP_CID_SIGGEN_EOT:
            VP_CID(VpLineCtxType, pLineCtx,
                ("Vp890CtrlSetFSKGen(): VP_CID_SIGGEN_EOT %d at time %d with tickBegin 0x%02X",
                data, pDevObj->timeStamp, pLineObj->tickBeginState[0]));
            /*
             * If CID State is in IDLE, can't write more CID Data. Otherwise, the line will
             * show 1 more data byte. Check this and prevent CID Data Write
             */
            if ((fskParam[0] & VP890_CID_STATE_MASK) == VP890_CID_STATE_IDLE) {
                /* Prevent writing the CID Data Register. */
                pLineObj->cidBytesRemain = 0;
                data = 0;
                initFsk = FALSE;
            } else {
                pLineObj->cidBytesRemain = 1;   /* Force count to 0 after wrting the current byte */
            }

            if (data == 0) {
                VP_CID(VpLineCtxType, pLineCtx,
                    ("Vp890CtrlSetFSKGen(): Disabling FSK at time %d", pDevObj->timeStamp));

                /* Stop Transmission Immediately */
                Vp890MuteChannel(pLineCtx, FALSE);
                fskParam[0] |= VP890_CID_DIS;
                moreData = FALSE;
            } else {
                if (pLineObj->suspendCid == FALSE) {
                   /* Wait until the device is complete */
                    VP_CID(VpLineCtxType, pLineCtx,
                        ("Vp890CtrlSetFSKGen(): Delaying FSK termination at time %d",
                        pDevObj->timeStamp));
                    pLineObj->suspendCid = TRUE;
                    /*
                     * Make sure the last byte is defined to be a "safe" mark
                     * signal and tell the silicon this is the last byte.
                     */
                    fskParam[0] &= ~VP890_CID_DIS;
                    fskParam[0] |=
                        (VP890_CID_FB_START_1 | VP890_CID_FB_STOP_1 | VP890_CID_EOM);
                    data = 0xFF;
                } else {
                    VP_CID(VpLineCtxType, pLineCtx,
                        ("Vp890CtrlSetFSKGen(): FSK Termination already in progress at time %d",
                        pDevObj->timeStamp));
                    return returnStatus;
                }
            }
            break;

        case VP_CID_GENERATOR_DATA:
            VP_CID(VpLineCtxType, pLineCtx,
                ("Vp890CtrlSetFSKGen(): VP_CID_GENERATOR_DATA 0x%02X at time %d with prior CID State: 0x%02X",
                data, pDevObj->timeStamp, pLineObj->tickBeginState[0]));

            pLineObj->callerId.status |= VP_CID_FSK_ACTIVE;
            Vp890MuteChannel(pLineCtx, TRUE);

            fskParam[0] |= (VP890_CID_FB_START_0 | VP890_CID_FB_STOP_1);
            fskParam[0] &= ~(VP890_CID_DIS);

            if (pLineObj->callerId.status & VP_CID_END_OF_MSG) {
                VP_CID(VpLineCtxType, pLineCtx,
                    ("Vp890CtrlSetFSKGen(): Sending EOM Signal to Silicon at time %d",
                    pDevObj->timeStamp));
                fskParam[0] |= VP890_CID_EOM;
            } else {
                fskParam[0] &= ~(VP890_CID_EOM);
            }
            break;

        case VP_CID_GENERATOR_KEYED_CHAR:
            VP_CID(VpLineCtxType, pLineCtx,
                ("Vp890CtrlSetFSKGen(): VP_CID_GENERATOR_KEYED_CHAR 0x%02X at time %d with prior CID State: 0x%02X",
                data, pDevObj->timeStamp, pLineObj->tickBeginState[0]));

            pLineObj->callerId.status |= VP_CID_FSK_ACTIVE;
            Vp890MuteChannel(pLineCtx, TRUE);
            fskParam[0] &= (uint8)(~(VP890_CID_EOM | VP890_CID_DIS));

            switch(data) {
                case VP_FSK_CHAN_SEIZURE:
                    fskParam[0] |=
                        (VP890_CID_FB_START_0 | VP890_CID_FB_STOP_1);
                    break;

                case VP_FSK_MARK_SIGNAL:
                    fskParam[0] |=
                        (VP890_CID_FB_START_1 | VP890_CID_FB_STOP_1);
                    if (pLineObj->callerId.markOutByteCount > 0) {
                        if (pLineObj->callerId.markOutByteRemain > 0) {
                            pLineObj->callerId.markOutByteRemain--;
                            VP_CID(VpLineCtxType, pLineCtx,
                                ("Vp890CtrlSetFSKGen(): Set markOutByteRemain to (%d) on ch (%d) at time %d",
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
                                ("Vp890CtrlSetFSKGen(): Last Mark-Out - Sending EOM Signal to Silicon at time %d",
                                pDevObj->timeStamp));
                            fskParam[0] |= VP890_CID_EOM;
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
      && (fskParam[0] & VP890_CID_DIS)) {
        VP_CID(VpLineCtxType, pLineCtx,
               ("Vp890CtrlSetFSKGen(): Redundant Disable Operation at time %d - Skipping Write",
                pDevObj->timeStamp));
        pLineObj->tickBeginState[0] = fskParam[0];
        return returnStatus;
    }

    if (fskParam[0] & VP890_CID_DIS) {
        pLineObj->callerId.status &= ~VP_CID_FSK_ACTIVE;
    }
    if ((pLineObj->tickBeginState[0] & VP890_CID_EOM) == VP890_CID_EOM) {
        VP_CID(VpLineCtxType, pLineCtx,
               ("Vp890CtrlSetFSKGen(): Identified EOM at time %d", pDevObj->timeStamp));
        return FALSE;
    }
    if (fskParam[0] != pLineObj->tickBeginState[0]) {
        pLineObj->tickBeginState[0] = fskParam[0];
        VP_CID(VpLineCtxType, pLineCtx, ("Vp890CtrlSetFSKGen(): Writing 0x%02X to CID Params",
            fskParam[0]));
        VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_WRT, VP890_CID_PARAM_LEN,
            fskParam);
    }
    if (moreData == TRUE) {
        VP_CID(VpLineCtxType, pLineCtx,
               ("Vp890CtrlSetFSKGen(): Writing 0x%02X to CID Data at Time %d",
                data, pDevObj->timeStamp));
        pLineObj->cidBytesRemain--;
        VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_DATA_WRT, VP890_CID_DATA_LEN,
            &data);

        if (initFsk == TRUE) {
            /*
             * Special Init case. The State machine updates in ~18us, so we
             * can't allow another byte write until we observe a change. It
             * will change to Empty, so check for that state as successfull
             * initialization.
             */
            uint8 safeFail = VP_CID_NORMAL_INIT_CNT;
            uint8 stateToExit = VP890_CID_STATE_EMPTY_D;

            while (safeFail > 0) {
                VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_RD, VP890_CID_PARAM_LEN,
                    pLineObj->tickBeginState);

                if ((pLineObj->tickBeginState[0] & VP890_CID_STATE_MASK) == stateToExit) {
                    VP_CID(VpLineCtxType, pLineCtx,
                           ("Vp890CtrlSetFSKGen(): State = 0x%02X at Count %d Time %d - Return Now",
                            stateToExit, safeFail, pDevObj->timeStamp));
                    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890CtrlSetFSKGen()"));
                    return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
                } else {
                    VP_CID(VpLineCtxType, pLineCtx,
                           ("Vp890CtrlSetFSKGen(): Not Empty (state 0x%02X) on Count %d Time %d",
                           (pLineObj->tickBeginState[0] & VP890_CID_STATE_MASK), safeFail,
                            pDevObj->timeStamp));
                }
                safeFail--;
            }
            /* If we're here it's because the initialization check failed. */
            VP_CID(VpLineCtxType, pLineCtx,
                   ("Vp890CtrlSetFSKGen(): Did not find Good Transition During Init Time %d",
                    pDevObj->timeStamp));
            VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890CtrlSetFSKGen()"));
            return FALSE;
        } else {
#ifdef VP_CID_POLLING_ENABLED
            if (mode != VP_CID_SIGGEN_EOT) {
                uint8 tickEndState[VP890_CID_PARAM_LEN];
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
                    VpMpiCmdWrapper(deviceId, ecVal, VP890_CID_PARAM_RD, VP890_CID_PARAM_LEN,
                        tickEndState);
                    VP_CID(VpLineCtxType, pLineCtx,
                           ("Vp890CtrlSetFSKGen(): CID Param Read 0x%02X Time %d",
                             tickEndState[0], pDevObj->timeStamp));
                    tickEndState[0] &= VP890_CID_STATE_MASK;

                    /*
                     * We've seen a transition toward more empty if these are the same since
                     * we wrote one byte earlier in this function. It means we can write more bytes.
                     */
                    if ((pLineObj->tickBeginState[0] & VP890_CID_STATE_MASK) == tickEndState[0]) {
                        pLineObj->cidBytesRemain++; /* Correct for state transition just observed */
                        VP_CID(VpLineCtxType, pLineCtx,
                               ("Vp890CtrlSetFSKGen(): Observed State Change to 0x%02X Time %d",
                                 tickEndState[0], pDevObj->timeStamp));
                        return TRUE;
                    }
                }
                /* If we're here, means we timed out and cannot write more bytes */
            }
            return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
#else
            return ((pLineObj->cidBytesRemain == 0) ? FALSE : TRUE);
#endif  /* VP_CID_POLLING_ENABLED */
        }
    }

    VP_API_FUNC_INT(VpLineCtxType, pLineCtx, ("-Vp890CtrlSetFSKGen()"));
    return returnStatus;
} /* Vp890CtrlSetFSKGen() */

/*******************************************************************************
 * Vp890LoadCidBuffers()
 *  This function loads cid data into the line objects cid buffers
 *  If length is within the size of just the primary buffer size, then only
 *  fill the primary buffer. Otherwise (the length exceeds the size of the
 *  primary buffer size) "low fill" the primary buffer and max fill the
 *  secondary buffer. This has the affect of causing a CID Data event
 *  quickly and giving the application a maximum amount of time to refill
 *  the message buffer
 *
 * Arguments:
 *   length     -
 *   *pCid      -
 *   pCidData   -
 *
 * Preconditions:
 *  none are needed
 *
 * Postconditions:
 * Caller ID information is saved to a line objects primary and secondary
 * CID buffers.
 *
 ******************************************************************************/
void
Vp890LoadCidBuffers(
    uint8           length,
    VpCallerIdType  *pCid,
    uint8p          pCidData)
{
    uint8 byteCnt1, byteCnt2;

    if (length <= VP_SIZEOF_CID_MSG_BUFFER) {
        pCid->primaryMsgLen = length;
        pCid->secondaryMsgLen = 0;
    } else {
        pCid->primaryMsgLen = (length - VP_SIZEOF_CID_MSG_BUFFER);
        pCid->secondaryMsgLen = VP_SIZEOF_CID_MSG_BUFFER;
    }

    /*
     * Copy the message data to the primary API buffer. If we're here, there's
     * at least one byte of primary message data. So a check is not necessary
     */
    pCid->status |= VP_CID_PRIMARY_FULL;
    for (byteCnt1 = 0; byteCnt1 < pCid->primaryMsgLen; byteCnt1++) {
        pCid->primaryBuffer[byteCnt1] = pCidData[byteCnt1];
        pCid->cidCheckSum += pCidData[byteCnt1];
        pCid->cidCheckSum = pCid->cidCheckSum % 256;
    }

    /* Copy the message data to the secondary API buffer if there is any */
    if (pCid->secondaryMsgLen > 0) {
        pCid->status |= VP_CID_SECONDARY_FULL;
        for (byteCnt2 = 0; (byteCnt2 < pCid->secondaryMsgLen); byteCnt2++) {
            pCid->secondaryBuffer[byteCnt2] = pCidData[byteCnt2 + byteCnt1];
            pCid->cidCheckSum += pCidData[byteCnt2 + byteCnt1];
            pCid->cidCheckSum =  pCid->cidCheckSum % 256;
        }
    }
    return;
} /* Vp890LoadCidBuffers() */

/*******************************************************************************
 * Vp890InitRing()
 *  This function is used to initialize the ringing profile and caller ID
 * cadence on a given line.
 *
 * Arguments:
 *  *pLineCtx       - Line Context to modify Ringing Parameters for
 *  pCadProfile     - Pointer of a Ringing Cadence profile, or the index into
 *                    the Ringing Cadence profile table.
 *  pCidProfile     - Pointer of a Caller ID profile, or the index into the
 *                    Caller ID profile table.
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
 ******************************************************************************/
VpStatusType
Vp890InitRing(
    VpLineCtxType           *pLineCtx,
    VpProfilePtrType        pCadProfile,
    VpProfilePtrType        pCidProfile)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp890InitRing()"));

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->state, TRUE)) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitRing()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pLineObj->status & VP890_IS_FXO) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitRing()"));
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
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitRing()"));
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
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitRing()"));
        return VP_STATUS_ERR_PROFILE;
    }

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitRing()"));
    return VP_STATUS_SUCCESS;
} /* Vp890InitRing() */

/*******************************************************************************
 * Vp890InitCid()
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
 ******************************************************************************/
VpStatusType
Vp890InitCid(
    VpLineCtxType           *pLineCtx,
    uint8                   length,
    uint8p                  pCidData)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    VpCallerIdType          *pCid       = &pLineObj->callerId;

    VP_API_FUNC(VpLineCtxType, pLineCtx, ("+Vp890InitCid()"));

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->state, TRUE)) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitCid()"));
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pLineObj->status & VP890_IS_FXO) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890InitCid() - FXO does not support CID"));
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitCid()"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length > (2 * VP_SIZEOF_CID_MSG_BUFFER)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890InitCid() - length exceeds internal msg buffer"));
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitCid()"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitCid()"));
        return VP_STATUS_SUCCESS;
    }

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    pCid->cliIndex = 0;
    pCid->cliMPIndex = 0;
    pCid->cliMSIndex = 0;

    /* Stop CID if it was in progress */
    pCid->cliTimer = 0;
    pCid->status = VP_CID_RESET_VALUE;
    pLineObj->suspendCid = FALSE;
    pCid->dtmfStatus = VP_CID_DTMF_RESET_VALUE;
    pCid->cidCheckSum = 0;
    pCid->messageDataRemain = length;

    /* load up the internal CID buffers */
    Vp890LoadCidBuffers(length, pCid, pCidData);

    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
    VP_API_FUNC(VpLineCtxType, pLineCtx, ("-Vp890InitCid()"));
    return VP_STATUS_SUCCESS;
} /* Vp890InitCid() */

/*******************************************************************************
 * Vp890SendCid()
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
 ******************************************************************************/
VpStatusType
Vp890SendCid(
    VpLineCtxType           *pLineCtx,
    uint8                   length,
    VpProfilePtrType        pCidProfile,
    uint8p                  pCidData)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpCallerIdType          *pCid       = &pLineObj->callerId;

    VpProfilePtrType        pCidProfileLocal;

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->state, TRUE)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pLineObj->status & VP890_IS_FXO) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SendCid() - FXO does not support CID"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length > (2 * VP_SIZEOF_CID_MSG_BUFFER)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SendCid() - length exceeds internal msg buffer"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        return VP_STATUS_SUCCESS;
    }

    /* Check the legality of the Ring CID profile */
    if (!VpCSLACIsProfileValid(VP_PROFILE_CID,
        VP_CSLAC_CALLERID_PROF_TABLE_SIZE,
        pDevObj->profEntry.cidCadProfEntry,
        pDevObj->devProfileTable.pCallerIdProfileTable,
        pCidProfile, &pCidProfileLocal))
    {
        return VP_STATUS_ERR_PROFILE;
    }

    /* Can't send a null profile */
    if (pCidProfileLocal == VP_PTABLE_NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890SendCid() - NULL CID profile is not allowed"));
        return VP_STATUS_ERR_PROFILE;
    }

    /* If we're here, all parameters passed are valid */
    VpSysEnterCritical(pDevObj->deviceId, VP_CODE_CRITICAL_SEC);

    VpCSLACInitCidStruct(&pLineObj->callerId, 0);

    pLineObj->callerId.pCliProfile = pCidProfileLocal;
    pLineObj->callerId.status &= ~VP_CID_REPEAT_MSG;
    pLineObj->callerId.status &= ~VP_CID_END_OF_MSG;
    pLineObj->callerId.cidCheckSum = 0;
    pLineObj->callerId.messageDataRemain = length;

    /* load up the internal CID buffers */
    Vp890LoadCidBuffers(length, pCid, pCidData);

    VpSysExitCritical(pDevObj->deviceId, VP_CODE_CRITICAL_SEC);
    return VP_STATUS_SUCCESS;

} /* Vp890SendCid() */

/*******************************************************************************
 * Vp890ContinueCid()
 *  This function is called to provide more caller ID data (in response to
 *  Caller ID data event from the VP-API). See VP-API-II  documentation
 *  for more information about this function.
 *
 *  When this function is called, the buffer that is in use is flagged
 *  by the VpCliGetEncodeByte() function in vp_api_common.c file. That
 *  function implements the logic of when to switch between the primary
 *  and secondary buffer. This function just needs to fill the bufffer that
 *  is not currently in use, starting with the primary (because the primary
 *  buffer is also used first for the first part of the message).
 *
 * Arguments:
 *  *pLineCtx   -
 *  length      -
 *  pCidData    -
 *
 * Preconditions:
 *  Device/Line context should be created and initialized. For applicable
 *  devices bootload should be performed before calling the function.
 *
 * Postconditions:
 *  Continues to transmit Caller ID information on the line.
 ******************************************************************************/
VpStatusType
Vp890ContinueCid(
    VpLineCtxType           *pLineCtx,
    uint8                   length,
    uint8p                  pCidData)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    VpCallerIdType          *pCid       = &pLineObj->callerId;

    uint8                   byteCount   = 0;
    uint8                   *pBuffer;

    /* Get out if device state is not ready */
    if (!Vp890IsDevReady(pDevObj->state, TRUE)) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    if (pLineObj->status & VP890_IS_FXO) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890ContinueCid() - FXO does not support CID"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length > (VP_SIZEOF_CID_MSG_BUFFER)) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890ContinueCid() - length exceeds internal msg buffer"));
        return VP_STATUS_INVALID_ARG;
    }

    if (length == 0) {
        return VP_STATUS_SUCCESS;
    }

    VpSysEnterCritical(pDevObj->deviceId, VP_CODE_CRITICAL_SEC);

    pCid->messageDataRemain += length;

    if (!(pLineObj->callerId.status & VP_CID_PRIMARY_IN_USE)) {
        /* Fill the primary buffer */
        pCid->status |= VP_CID_PRIMARY_FULL;
        pCid->primaryMsgLen = length;
        pBuffer = &(pCid->primaryBuffer[0]);
    } else {
        /* Fill the secondary buffer */
        pCid->status |= VP_CID_SECONDARY_FULL;
        pCid->secondaryMsgLen = length;
        pBuffer = &(pCid->secondaryBuffer[0]);
    }

    /* Copy the message data to the API buffer */
    for (byteCount = 0; (byteCount < length); byteCount++) {
        pBuffer[byteCount] = pCidData[byteCount];

        pCid->cidCheckSum += pBuffer[byteCount];
        pCid->cidCheckSum = pCid->cidCheckSum % 256;
    }
    VpSysExitCritical(pDevObj->deviceId, VP_CODE_CRITICAL_SEC);

    return VP_STATUS_SUCCESS;
} /* Vp890ContinueCid() */
#endif  /* VP890_FXS_SUPPORT */
#endif  /* VP_CSLAC_SEQ_EN  */
#endif  /* VP_CC_890_SERIES */
