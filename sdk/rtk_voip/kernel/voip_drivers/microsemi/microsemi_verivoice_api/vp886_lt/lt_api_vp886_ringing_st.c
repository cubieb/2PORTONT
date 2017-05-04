/** file lt_api_vp886_ringing_st.c
 *
 *  This file contains the VP886 ringing self test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10693 $
 * $LastChangedDate: 2012-12-25 11:06:36 -0600 (Tue, 25 Dec 2012) $
 */
#include "lt_api.h"

#ifdef LT_RINGING_ST_886

#include "lt_api_internal.h"

typedef enum {
    RINGING_ST_INIT                     = LT_GLOBAL_FIRST_STATE,
    RINGING_ST_GO_TO_ACTIVE,
    RINGING_ST_GO_TO_ACTIVE_SETTLE,
    RINGING_ST_ABORT_IF_OFFHOOK,
    RINGING_ST_DC_RING_LOOPSUP,
    RINGING_ST_DC_RING_RINGGEN,
    RINGING_ST_DC_RING_STATE,
    RINGING_ST_DC_RING_WAIT,
    RINGING_ST_APPLY_TEST_SWITCH,
    RINGING_ST_APPLY_TEST_SWITCH_SETTLE,
    RINGING_ST_CHECK_RING_TRIP,
    RINGING_ST_EXIT_RINGING,
    RINGING_ST_OFFHOOK_AC_RLOOP,
    RINGING_ST_STORE_AC_LOOP_1_RESULTS,
    RINGING_ST_STOP_AC_RLOOP,
    RINGING_ST_STOP_AC_RLOOP_SETTLE,
    RINGING_ST_REMOVE_TEST_SWITCH,
    RINGING_ST_OPEN_CIRCUIT_AC_RLOOP,
    RINGING_ST_STORE_AC_LOOP_2_RESULTS,
    RINGING_ST_CONCLUDE,
    RINGING_ST_COMPLETE,
    RINGING_ST_ABORT                = LT_GLOBAL_ABORTED_STATE,
    RINGING_ST_ENUM_SIZE            = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886RingingStStates;


static bool
RingingStHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886RingingStAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtRingingSTInputType *ppInputs = 
        (LtRingingSTInputType*)pTemp886->attributes.pInputs;

    LtRingingSTCriteriaType *ppCriteria =
        (LtRingingSTCriteriaType*)pTemp886->attributes.pCriteria;

    LtVp886RingingStTempType *pTempRingingSt = &pTemp886->tempData.ringingSt;

    /* Check input args */
    if (ppInputs == VP_NULL ) {
        pTempRingingSt->input.freq = LT_RINGING_ST_DFLTI_FREQ;
        pTempRingingSt->input.vRinging = LT_RINGING_ST_DFLTI_VRING;
        pTempRingingSt->input.pRingProfileForTest = LT_RINGING_ST_DFLTI_PROFILE;
        pTempRingingSt->input.pDcProfileForTest = LT_RINGING_ST_DFLTI_PROFILE;
    } else {
        /* make sure the input test v is legal */
        LtVoltageType maxRms = VP886_UNIT_CONVERT(LT_VP886_RINGING_ST_MAX_INPUTV,
                VP886_UNIT_MV, VP886_UNIT_MVRMS);

        /* make sure the ring freq is legal */
        if ((ppInputs->freq > LT_VP886_MAX_FREQ) || (ppInputs->freq < LT_VP886_MIN_FREQ)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886RingingStAttributesCheck(freq:%li) : "
                "invalid", ppInputs->freq));
            return FALSE;
        }
        pTempRingingSt->input.freq = ABS(ppInputs->freq);

        /* make sure the input test v is legal */
        if (maxRms < ppInputs->vRinging) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886RingingStAttributesCheck(vRinging:%li) : "
                "invalid", ppInputs->vRinging));
            return FALSE;
        }
        pTempRingingSt->input.vRinging = ppInputs->vRinging;

        /* store the profile pointers */
        pTempRingingSt->input.pRingProfileForTest = ppInputs->pRingProfileForTest;
        pTempRingingSt->input.pDcProfileForTest = ppInputs->pDcProfileForTest;
    }

    /* Check Criteria args */
    if (ppCriteria == VP_NULL) {
        pTempRingingSt->criteria.openVoltageErr = LT_RINGING_ST_DFLTC_VRING;
        pTempRingingSt->criteria.freqErr = LT_RINGING_ST_DFLTC_FREQ;
        pTempRingingSt->criteria.rLoadErr = LT_RINGING_ST_DFLTC_RLOAD;
    } else {
        /* make sure the open circuit voltage error is legal */
        if ((LT_0_PERCENT >= ppCriteria->openVoltageErr) ||
            (LT_100_PERCENT < ppCriteria->openVoltageErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886RingingStAttributesCheck(openVoltageErr:%li) : "
                "invalid", ppCriteria->openVoltageErr));
            return FALSE;
        }
        pTempRingingSt->criteria.openVoltageErr = ppCriteria->openVoltageErr;

        /* make sure the ring freq error is legal */
        if ((LT_0_PERCENT >= ppCriteria->freqErr) ||
            (LT_100_PERCENT < ppCriteria->freqErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886RingingStAttributesCheck(freqErr:%li) : "
                "invalid", ppCriteria->freqErr));
            return FALSE;
        }
        pTempRingingSt->criteria.freqErr = ppCriteria->freqErr;

        /* make sure the r load error is legal */
        if ((LT_0_PERCENT >= ppCriteria->rLoadErr) ||
            (LT_100_PERCENT < ppCriteria->rLoadErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886RingingStAttributesCheck(rLoadErr:%li) : "
                "invalid", ppCriteria->rLoadErr));
            return FALSE;
        }
        pTempRingingSt->criteria.rLoadErr = ppCriteria->rLoadErr;
    }

    /* load up the api ac rloop testline struct */
    pTempRingingSt->vpApiAcRloopInput.tip = VP_TEST_TIP_RING;
    pTempRingingSt->vpApiAcRloopInput.calMode = FALSE;
    pTempRingingSt->vpApiAcRloopInput.freq =
        (uint16)(ABS(pTempRingingSt->input.freq)/1000);
    pTempRingingSt->vpApiAcRloopInput.vBias = 0;
    pTempRingingSt->vpApiAcRloopInput.vTestLevel =
        (int16)VP886_UNIT_CONVERT(pTempRingingSt->input.vRinging,
            VP886_UNIT_MVRMS, VP886_UNIT_DAC_RING);
    pTempRingingSt->vpApiAcRloopInput.settlingTime = LT_VP886_RINGING_ST_SETTLE_TIME;
    pTempRingingSt->vpApiAcRloopInput.integrateTime = (uint16)
        ((LT_VP886_RINGING_ST_RING_CYCLES * 8000000l) / pTempRingingSt->input.freq);
    pTempRingingSt->offhook = FALSE;

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = RingingStHandlerSM;
    return TRUE;
}

static bool
RingingStHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{

    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886RingingStTempType *pTempRingingSt = &pTemp886->tempData.ringingSt;
    LtRingingSTResultType *pResults = &pTestCtx->pResult->result.ringingST;
    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("RingingStHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case RINGING_ST_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_RINGING_ST);
            pTestCtx->state = RINGING_ST_GO_TO_ACTIVE;
            requestNextState = TRUE;
            break;

        case RINGING_ST_GO_TO_ACTIVE:
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            pTestCtx->state = RINGING_ST_GO_TO_ACTIVE_SETTLE;
            requestNextState = TRUE;
            break;

        case RINGING_ST_GO_TO_ACTIVE_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = RINGING_ST_ABORT_IF_OFFHOOK;
            break;

        case RINGING_ST_ABORT_IF_OFFHOOK: {
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            /* If the raw hook bit is true (off-hook) then test is done early*/
            if (rawHook) {
                LT_DOUT(LT_DBG_INFO, ("RingingStHandlerSM() aborted due to off-hook"));
                pTempRingingSt->offhook = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pTestCtx->state = RINGING_ST_CONCLUDE;
            } else {
                pTestCtx->state = RINGING_ST_DC_RING_LOOPSUP;
            }
            requestNextState = TRUE;
            break;
        }

        case RINGING_ST_DC_RING_LOOPSUP: {
            /* Set the ring trip mode to DC with a 13mA trip threshold. */
            uint8 mpiBuf[VP886_R_LOOPSUP_LEN] = {
                0x5C, 0x84, 0x1A, 0x8D, 0x5B
            };

            VpTestVpMpiCmdType regInput;
            regInput.cmd = VP886_R_LOOPSUP_WRT;
            regInput.cmdLen = VP886_R_LOOPSUP_LEN;
            VpMemCpy(&regInput.buffer, mpiBuf, VP886_R_LOOPSUP_LEN);

            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_VP_MPI_CMD, &regInput);

            pTestCtx->state = RINGING_ST_DC_RING_RINGGEN;
            break;
        }

        case RINGING_ST_DC_RING_RINGGEN: {
            /* Create a dc ringing signal so that we don't cause the phone to
             * ding */
            uint8 mpiBuf[VP886_R_RINGGEN_LEN] = {
                0x00, 0xEB, 0x46, 0x00, 0x37, 0x00, 0xD3, 0x00, 0x00, 0x00, 0x00
            };

            VpTestVpMpiCmdType regInput;
            regInput.cmd = VP886_R_RINGGEN_WRT;
            regInput.cmdLen = VP886_R_RINGGEN_LEN;
            VpMemCpy(&regInput.buffer, mpiBuf, VP886_R_RINGGEN_LEN);

            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_VP_MPI_CMD, &regInput);

            pTestCtx->state = RINGING_ST_DC_RING_STATE;
            break;
        }

        case RINGING_ST_DC_RING_STATE: {
            /* Go to the ringing line state.  Do this directly instead of
             * through VpSetLineState() to avoid using the customer's ring
             * cadence. */
            uint8 mpiBuf[VP886_R_STATE_LEN] = {
                VP886_R_STATE_SS_BAL_RING
            };

            VpTestVpMpiCmdType regInput;
            regInput.cmd = VP886_R_STATE_WRT;
            regInput.cmdLen = VP886_R_STATE_LEN;
            VpMemCpy(&regInput.buffer, mpiBuf, VP886_R_STATE_LEN);

            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_VP_MPI_CMD, &regInput);

            pTestCtx->state = RINGING_ST_DC_RING_WAIT;
            break;
        }

        case RINGING_ST_DC_RING_WAIT:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = RINGING_ST_APPLY_TEST_SWITCH;
            break;

        case RINGING_ST_APPLY_TEST_SWITCH:
            *pRetval = Vp886SetRelayStateWrapper(pTestCtx, VP_RELAY_BRIDGED_TEST);
            pTestCtx->state = RINGING_ST_APPLY_TEST_SWITCH_SETTLE;
            requestNextState = TRUE;
            break;

        case RINGING_ST_APPLY_TEST_SWITCH_SETTLE:
            /* Use the state change settling time to allow time for ring trip
             * detection. */
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = RINGING_ST_CHECK_RING_TRIP;
            break;

        case RINGING_ST_CHECK_RING_TRIP:
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &pTempRingingSt->ringTrip);
            pTestCtx->state = RINGING_ST_EXIT_RINGING;
            requestNextState = TRUE;
            break;

        case RINGING_ST_EXIT_RINGING:
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            pTestCtx->state = RINGING_ST_OFFHOOK_AC_RLOOP;
            requestNextState = TRUE;
            break;

        case RINGING_ST_OFFHOOK_AC_RLOOP:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP, &pTempRingingSt->vpApiAcRloopInput);
            pTestCtx->state = RINGING_ST_STORE_AC_LOOP_1_RESULTS;
            break;

        case RINGING_ST_STORE_AC_LOOP_1_RESULTS:
            LtMemCpy(&pTempRingingSt->vpApiAcRloop1Rslt, &pTestPrimRslts->result.acimt,
                sizeof(VpTestResultAcRlType));
            pTestCtx->state = RINGING_ST_STOP_AC_RLOOP;
            requestNextState = TRUE;
            break;

        case RINGING_ST_STOP_AC_RLOOP:
            /* Stop the AC_RLOOP signal so that we don't get a big glitch when
             * we remove the test switch. */
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            pTestCtx->state = RINGING_ST_STOP_AC_RLOOP_SETTLE;
            requestNextState = TRUE;
            break;

        case RINGING_ST_STOP_AC_RLOOP_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = RINGING_ST_REMOVE_TEST_SWITCH;
            break;

        case RINGING_ST_REMOVE_TEST_SWITCH:
            *pRetval = Vp886SetRelayStateWrapper(pTestCtx, VP_RELAY_NORMAL);
            pTestCtx->state = RINGING_ST_OPEN_CIRCUIT_AC_RLOOP;
            requestNextState = TRUE;
            break;

        case RINGING_ST_OPEN_CIRCUIT_AC_RLOOP:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP, &pTempRingingSt->vpApiAcRloopInput);
            pTestCtx->state = RINGING_ST_STORE_AC_LOOP_2_RESULTS;
            break;

        case RINGING_ST_STORE_AC_LOOP_2_RESULTS:
            LtMemCpy(&pTempRingingSt->vpApiAcRloop2Rslt, &pTestPrimRslts->result.acimt,
                sizeof(VpTestResultAcRlType));
            pTestCtx->state = RINGING_ST_CONCLUDE;
            requestNextState = TRUE;
            break;

        case RINGING_ST_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = RINGING_ST_COMPLETE;
            break;
        }

        case RINGING_ST_COMPLETE:
            /* Calculate results and end the test */
            CalculateResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case RINGING_ST_ABORT:
            /* Test aborted, results may indicate source of error. */
            *pRetval = LT_STATUS_ABORTED;
            break;

        default:
            /* This should never happen */
            LT_DOUT(LT_DBG_ERRR, ("This should never happen -> LT_STATUS_ERROR_UNKNOWN"));
            *pRetval = LT_STATUS_ERROR_UNKNOWN;
            break;
    }
    return requestNextState;
}

static void
CalculateResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingingStTempType *pTempRingingSt = &pTemp886->tempData.ringingSt;
    LtRingingSTResultType *pResults = &pTestCtx->pResult->result.ringingST;
    LtRingingSTInputType *pInput = &pTempRingingSt->input;
    LtRingingSTCriteriaType *pCriteria = &pTempRingingSt->criteria;
    VpTestResultAcRlType *pAcRloop1Rslt = &pTempRingingSt->vpApiAcRloop1Rslt;
    VpTestResultAcRlType *pAcRloop2Rslt = &pTempRingingSt->vpApiAcRloop2Rslt;
    LtFreqType requestedFreq;
    LtVoltageType requestedV;
    LtPercentType percentErr;
    LtCurrentType currentThreshold;
    LtVoltageType voltageThreshold;

    /* if an off-hook phone was detected then no measurements were taken */
    if (pTempRingingSt->offhook) {
        pResults->fltMask = LT_RINGING_STM_OFF_HOOK;
        return;
    }

    pResults->fltMask = LT_TEST_PASSED;

    /* evaluate and save measured open circuit voltage */
    requestedV = pInput->vRinging;
    pResults->openCktRingVol = VP886_UNIT_CONVERT(pAcRloop2Rslt->vab,
            VP886_UNIT_ADC_VAB_RMS, VP886_UNIT_MVRMS);
    /* knocking down the % error granularity to Volts*/
    percentErr = (pCriteria->openVoltageErr / 1000);
    percentErr = (requestedV * percentErr) / 100;
    if ((ABS(requestedV - ABS(pResults->openCktRingVol))) > percentErr) {
        pResults->fltMask |= LT_RINGING_STM_OPENV_OC;
    }

    /* evaluate and save measured ringing frequency.  Use the second AcRloop
     * result because the first one won't reliably measure frequency when using
     * the internal test termination */
    requestedFreq = pInput->freq;
    pResults->freq = pAcRloop2Rslt->freq;
    /* knocking down the % error granularity to Hz*/
    percentErr = (pCriteria->freqErr / 1000);
    percentErr = (requestedFreq * percentErr) / 100;
    if ((ABS(requestedFreq - ABS(pResults->freq))) > percentErr) {
        pResults->fltMask |= LT_RINGING_STM_FREQ_OC;
    }

    /* evaluate and save the ac current */
    pResults->iRload = VP886_UNIT_CONVERT(pAcRloop1Rslt->imt,
            VP886_UNIT_ADC_IMET_RING, VP886_UNIT_UA);

    /* evaluate and save the ac load resistance */
    pResults->acRload = ABS((requestedV * 10000) / pResults->iRload) - 2000;

    /* Check that the measured rms current is close to what we expect. */
    /* The AC Rloop test uses a hard-coded ILR value.  ILR defines the
     * maximum peak current.  First divide by sqrt(2) to get the rms value
     * of a sine wave with a peak of ILR, the largest that will not clip. */
    currentThreshold = (LT_VP886_RINGING_ST_ACRLOOP_ILR * 10000L) / 14142;

    /* Compute the rms voltage of the maximum clean sine wave, assuming
     * a defined source impedance as the only loop resistance */
    voltageThreshold = (currentThreshold * LT_VP886_RINGING_ST_SOURCE_IMPEDANCE) / 10000;

    if (requestedV <= voltageThreshold) {
        /* If the requested rms voltage is less than the rms voltage of the
         * maximum clean sine wave, then we expect the measured rms current
         * to match up */
        LtCurrentType currentTarget;

        currentTarget = (requestedV * 10000) / LT_VP886_RINGING_ST_SOURCE_IMPEDANCE;

        percentErr = (LT_RINGING_ST_DFLTC_IRLOAD / 1000);
        percentErr = (currentTarget * percentErr) / 100;
        if ((ABS(currentTarget - ABS(pResults->iRload))) > percentErr) {
            pResults->fltMask |= LT_RINGING_STM_IRLOAD_OC;
        }
    } else {
        /* If the requested rms voltage exceeds the rms voltage of the
         * maximum clean sine wave, then the current wave will be clipped
         * at the ILR value.  The rms value should be somewhere between a
         * sine wave and a square wave of amplitude ILR. */
        LtCurrentType lowerLimit;
        LtCurrentType upperLimit;

        percentErr = (LT_RINGING_ST_DFLTC_IRLOAD / 1000);

        lowerLimit = currentThreshold;
        lowerLimit = lowerLimit - ((lowerLimit * percentErr) / 100);

        upperLimit = LT_VP886_RINGING_ST_ACRLOOP_ILR;
        upperLimit = upperLimit + ((upperLimit * percentErr) / 100);

        if (pResults->iRload < lowerLimit || pResults->iRload > upperLimit) {
            pResults->fltMask |= LT_RINGING_STM_IRLOAD_OC;
        }
    }

    if (FALSE == pTempRingingSt->ringTrip) {
        pResults->fltMask |= LT_RINGING_STM_NO_RINGTRIP;
    }
    return;
}

#endif /* LT_RINGING_ST_886 */
