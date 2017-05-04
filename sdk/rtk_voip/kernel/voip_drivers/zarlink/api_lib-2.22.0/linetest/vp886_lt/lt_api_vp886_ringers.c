/** file lt_api_vp886_ringers.c
 *
 *  This file contains the VP886 ren and capacitance ren test routines.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10837 $
 * $LastChangedDate: 2013-03-01 15:42:35 -0600 (Fri, 01 Mar 2013) $
 */
#include <linux/delay.h>
#include "lt_api.h"

#ifdef LT_RINGERS_886

#include "lt_api_internal.h"
#undef LT_API_DISABLE_EMPIRICAL_REN_CORRECTIONS

typedef enum {
    REG_RINGER_INIT            = LT_GLOBAL_FIRST_STATE,
    REG_RINGER_REQUEST_DIFF,
    REG_RINGER_STORE_DIFF,
    REG_RINGER_REQUEST_RING,
    REG_RINGER_STORE_RING,
    REG_RINGER_REQUEST_TIP,
    REG_RINGER_STORE_TIP,
    REG_RINGER_CONCLUDE,
    REG_RINGER_COMPLETE,
    REG_RINGER_ABORT           = LT_GLOBAL_ABORTED_STATE,
    REG_RINGER_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886RegRingerStates;

typedef enum {
    ELEC_RINGER_INIT                = LT_GLOBAL_FIRST_STATE,
    ELEC_RINGER_GO_ACTIVE,
    ELEC_RINGER_LINE_STATE_SETTLE,
    ELEC_RINGER_CHECK_FOR_ABORT_GNDKEY,
    ELEC_RINGER_CHECK_FOR_OFF_HOOK,

    ELEC_RINGER_REQUEST_VAB,
    ELEC_RINGER_STORE_VAB,
    ELEC_RINGER_RAMP_DIFF_INIT,
    ELEC_RINGER_RAMP_DIFF_UP,
    ELEC_RINGER_RAMP_DIFF_UP_STORE,
    ELEC_RINGER_RAMP_DIFF_DOWN,
    ELEC_RINGER_RAMP_DIFF_DOWN_STORE,
    ELEC_RINGER_RAMP_DIFF_TO_FEED,

    ELEC_RINGER_REQUEST_VBG,
    ELEC_RINGER_STORE_VBG,
    ELEC_RINGER_RAMP_RING_INIT,
    ELEC_RINGER_RAMP_RING_UP,
    ELEC_RINGER_RAMP_RING_UP_STORE,
    ELEC_RINGER_RAMP_RING_DOWN,
    ELEC_RINGER_RAMP_RING_DOWN_STORE,
    ELEC_RINGER_RAMP_RING_TO_FEED,

    ELEC_RINGER_RAMP_TIP_INIT,
    ELEC_RINGER_RAMP_TIP_UP,
    ELEC_RINGER_RAMP_TIP_UP_STORE,
    ELEC_RINGER_RAMP_TIP_DOWN,
    ELEC_RINGER_RAMP_TIP_DOWN_STORE,
    ELEC_RINGER_RAMP_TIP_TO_FEED,
 
    ELEC_RINGER_CONCLUDE,
    ELEC_RINGER_COMPLETE,
    ELEC_RINGER_ABORT               = LT_GLOBAL_ABORTED_STATE,
    ELEC_RINGER_ENUM_SIZE           = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886CapRegRingerStates;


static bool
RegRingersHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static bool
ElecRingersHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static LtTestStatusType
ElecRingersRampInit(
    LtTestCtxType *pTestCtx,
    VpTestTipSelectType lead);

static LtTestStatusType
ElecRingersRamp(
    LtTestCtxType *pTestCtx,
    VpTestTipSelectType lead,
    int16 voltage);

static void
CalculateRegResults(
    LtTestCtxType *pTestCtx);

static void
CalculateElecResults(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886RingersAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtRingersCriteriaType *ppCriteria =
        (LtRingersCriteriaType*)pTemp886->attributes.pCriteria;

    LtRingerInputType *ppInputs =
        (LtRingerInputType*)pTemp886->attributes.pInputs;

    LtVp886RingersTempType *pTempRingers = &pTemp886->tempData.ringers;

    if (VP_NULL == ppInputs) {
        pTempRingers->input.ringerTestType = LT_RINGERS_DFLTI_TEST_TYPE;
        pTempRingers->input.vRingerTest = LT_RINGERS_DFLTI_TEST_VOLTAGE;
        pTempRingers->input.freq = LT_RINGERS_DFLTI_TEST_FREQUENCY;
        pTempRingers->input.renFactor = LT_RINGERS_DFLTI_TEST_RENFACTOR;
    } else {
        int32 maxV = 0;
        /* make sure the test type is legal */
        if (ppInputs->ringerTestType >= LT_RINGER_TEST_NUM_TYPES) {
            LT_DOUT(LT_DBG_ERRR, ("LtVp886RingersAttributeCheck(ringerTestType:%i) : "\
                "invalid", ppInputs->ringerTestType));
            return FALSE;
        }
        pTempRingers->input.ringerTestType = ppInputs->ringerTestType;

        /* make sure the input test v is legal */
        if (pTempRingers->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST) {
            maxV = LT_VP886_REG_RINGERS_MAX_INPUTV * 2; /* Twice the max bias voltage */
        } else if (pTempRingers->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST_3_ELE) {
            maxV = LT_VP886_REG_RINGERS_MAX_INPUTV; /* No more than max bias voltage */
        } else {
            maxV = LT_VP886_REG_RINGERS_MAX_INPUTV * 2; /* Twice the max bias voltage */
        }

        if (maxV < ppInputs->vRingerTest) {
            LT_DOUT(LT_DBG_ERRR, ("LtVp886RingersAttributeCheck(vRingerTest:%li) : "\
                "invalid", ppInputs->vRingerTest));
            return FALSE;
        }
        pTempRingers->input.vRingerTest = ppInputs->vRingerTest;


        /* neg freq is for special output */
        if (ppInputs->freq < 0) {
            pTempRingers->negative = TRUE;
        } else {
            pTempRingers->negative = FALSE;
        }

        if ( (pTempRingers->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST) ||
             (pTempRingers->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST_3_ELE) ) {

            int32 freq = ABS(ppInputs->freq);

            if ((freq > LT_VP886_MAX_FREQ) || (freq < LT_VP886_MIN_FREQ)) {
                LT_DOUT(LT_DBG_ERRR, ("LtVp886RingersAttributeCheck(freq:%li) : "\
                    "invalid", ppInputs->freq));
                return FALSE;
            }
        }
        pTempRingers->input.freq = ABS(ppInputs->freq);

        pTempRingers->input.renFactor = ABS(ppInputs->renFactor);
    }

    /* critera check */
    if (VP_NULL == ppCriteria) {
        pTempRingers->criteria.renHigh = LT_RINGERS_CRT_REN_HIGH;
        pTempRingers->criteria.renLow = LT_RINGERS_CRT_REN_LOW;
    } else {
        pTempRingers->criteria.renHigh = ppCriteria->renHigh;
        pTempRingers->criteria.renLow = ppCriteria->renLow;
    }

    /* initialize internal flags */
    pTempRingers->quickStop = FALSE;

    /* setup function Handler */
    if ((pTempRingers->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST) ||
        (pTempRingers->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST_3_ELE)) {
        pTestCtx->Lt886TestEventHandlerFunc = RegRingersHandlerSM;
    } else {
        pTestCtx->Lt886TestEventHandlerFunc = ElecRingersHandlerSM;
    }
    return TRUE;
}

static bool
RegRingersHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingersTempType *pTempRingers = &pTemp886->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("RegRingersHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case REG_RINGER_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_RINGERS);
            pResults->ringerTestType = pTempRingers->input.ringerTestType;

            /* setup the ac rloop struct for multiple measurements */
            pTempRingers->acRloopInput.tip = VP_TEST_TIP_RING;
            pTempRingers->acRloopInput.calMode = TRUE;
            pTempRingers->acRloopInput.vBias = LT_VP886_REG_RINGERS_TEST_BIAS;
            pTempRingers->acRloopInput.freq = pTempRingers->input.freq / 1000;
            pTempRingers->acRloopInput.settlingTime = LT_VP886_REG_RINGERS_SETTLE_TIME;
            pTempRingers->acRloopInput.vTestLevel = (int16)VP886_UNIT_CONVERT(
            pTempRingers->input.vRingerTest, VP886_UNIT_MV, VP886_UNIT_DAC_RING);
            pTempRingers->acRloopInput.integrateTime = (uint16)
                ((LT_VP886_REG_RINGERS_RING_CYCLES * 8000l) / pTempRingers->acRloopInput.freq);

            pTestCtx->state = REG_RINGER_REQUEST_DIFF;
            requestNextState = TRUE;
            break;

        case REG_RINGER_REQUEST_DIFF:
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP, &pTempRingers->acRloopInput);
            pTestCtx->state = REG_RINGER_STORE_DIFF;
            break;

        case REG_RINGER_STORE_DIFF:
            pTempRingers->vabDiff = pTestPrimRslts->result.acimt.vab;
            pTempRingers->imtDiff = pTestPrimRslts->result.acimt.imt;

            if (pTempRingers->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST_3_ELE) {
                pTestCtx->state = REG_RINGER_REQUEST_RING;
            } else {
                pTestCtx->state = REG_RINGER_CONCLUDE;
            }
            requestNextState = TRUE;
            break;


        case REG_RINGER_REQUEST_RING:
            pTempRingers->acRloopInput.tip = VP_TEST_RING;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP, &pTempRingers->acRloopInput);
            pTestCtx->state = REG_RINGER_STORE_RING;
            break;

        case REG_RINGER_STORE_RING:
            pTempRingers->vabRing = pTestPrimRslts->result.acimt.vab;
            pTempRingers->ilgRing = pTestPrimRslts->result.acimt.ilg;

            pTestCtx->state = REG_RINGER_REQUEST_TIP;
            requestNextState = TRUE;
            break;


        case REG_RINGER_REQUEST_TIP:
            pTempRingers->acRloopInput.tip = VP_TEST_TIP;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_AC_RLOOP, &pTempRingers->acRloopInput);
            pTestCtx->state = REG_RINGER_STORE_TIP;
            break;

        case REG_RINGER_STORE_TIP:
            pTempRingers->vabTip = pTestPrimRslts->result.acimt.vab;
            pTempRingers->ilgTip = pTestPrimRslts->result.acimt.ilg;

            pTestCtx->state = REG_RINGER_CONCLUDE;
            requestNextState = TRUE;
            break;


        /* Start the conclude primitive if this test not a part of an internal test */
        case REG_RINGER_CONCLUDE: {
            if (!pTestCtx->pTemp->vp886Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            } else {
                requestNextState = TRUE;
            }
            pTestCtx->state = REG_RINGER_COMPLETE;
            break;
        }

        case REG_RINGER_COMPLETE:
            /* Calculate results and end the test */
            CalculateRegResults(pTestCtx);
            *pRetval = LT_STATUS_DONE;
            break;

        case REG_RINGER_ABORT:
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

static bool
ElecRingersHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingersTempType *pTempRingers = &pTemp886->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("ElecRingersHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case ELEC_RINGER_INIT:
			msleep(10);
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_RINGERS);
            pResults->ringerTestType = pTempRingers->input.ringerTestType;

            /* Setting up a loop condition struct to be used a later states */
            pTemp886->loopCond.calMode = FALSE;
            pTemp886->loopCond.integrateTime =  LT_VP886_ELEC_RINGERS_INTEGRATE_TIME;
            pTemp886->loopCond.settlingTime = LT_VP886_ELEC_RINGERS_SETTLE_TIME;
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_VSAB;

            pTestCtx->state = ELEC_RINGER_GO_ACTIVE;
            requestNextState = TRUE;
            break;

        case ELEC_RINGER_GO_ACTIVE:
			msleep(10);
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            pTestCtx->state = ELEC_RINGER_LINE_STATE_SETTLE;
            requestNextState = TRUE;
            break;

        case ELEC_RINGER_LINE_STATE_SETTLE:
			msleep(10);
            /* wait for the line to settle */
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = ELEC_RINGER_CHECK_FOR_ABORT_GNDKEY;
            break;

        case ELEC_RINGER_CHECK_FOR_ABORT_GNDKEY: {
			msleep(10);
            bool gndkey = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_GKEY, &gndkey);

            if (gndkey) {
                pTempRingers->quickStop = TRUE;
                pResults->fltMask = LT_TEST_MEASUREMENT_ERROR;
                pResults->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
                pTestCtx->state = ELEC_RINGER_CONCLUDE;
            } else {
                pTestCtx->state = ELEC_RINGER_CHECK_FOR_OFF_HOOK;
            }
            requestNextState = TRUE;
            break;
        }

        case ELEC_RINGER_CHECK_FOR_OFF_HOOK: {
			msleep(10);
            bool rawHook = FALSE;
            *pRetval = Vp886GetLineStatusWrapper(pTestCtx, VP_INPUT_RAW_HOOK, &rawHook);

            /* If we are off-hook then max out the REN and pass the test */
            if (rawHook) {
                pTempRingers->quickStop = TRUE;
                pResults->fltMask = LT_RNGM_REN_HIGH;
                pResults->measStatus = LT_MSRMNT_STATUS_PASSED;
                pResults->ren = LT_MAX_REN;
                pResults->rentg = LT_MAX_REN;
                pResults->renrg = LT_MAX_REN;
                pTestCtx->state = ELEC_RINGER_CONCLUDE;
            } else {
                pTestCtx->state = ELEC_RINGER_REQUEST_VAB;
            }
            requestNextState = TRUE;
            break;
        }

        /* 
         * Measuring and ramping differentially 
         */
        case ELEC_RINGER_REQUEST_VAB:
			msleep(10);
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = ELEC_RINGER_STORE_VAB;
            break;

        case ELEC_RINGER_STORE_VAB:
			msleep(10);
            pTempRingers->feedVADC = (uint16)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = ELEC_RINGER_RAMP_DIFF_INIT;
            requestNextState = TRUE;
            break;

        case ELEC_RINGER_RAMP_DIFF_INIT:
			msleep(10);
            *pRetval = ElecRingersRampInit(pTestCtx, VP_TEST_TIP_RING);
            pTestCtx->state = ELEC_RINGER_RAMP_DIFF_UP;
            break;

        case ELEC_RINGER_RAMP_DIFF_UP:
			msleep(10);
            /* Ramp up (positive rampSizeVADC) */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_TIP_RING, pTempRingers->rampSizeVADC);
            pTestCtx->state = ELEC_RINGER_RAMP_DIFF_UP_STORE;
            break;

        case ELEC_RINGER_RAMP_DIFF_UP_STORE:
			msleep(10);
            LtMemCpy(&pTempRingers->vpApiDiffRamp1,&pTestPrimRslts->result.ramp, sizeof(VpTestResultRampType));
            requestNextState = TRUE;
            pTestCtx->state = ELEC_RINGER_RAMP_DIFF_DOWN;
            break;

        case ELEC_RINGER_RAMP_DIFF_DOWN:
			msleep(10);
            /* Ramp down (negative rampSizeVADC) */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_TIP_RING, -pTempRingers->rampSizeVADC);
            pTestCtx->state = ELEC_RINGER_RAMP_DIFF_DOWN_STORE;
            break;

        case ELEC_RINGER_RAMP_DIFF_DOWN_STORE:
			msleep(10);
            LtMemCpy(&pTempRingers->vpApiDiffRamp2,&pTestPrimRslts->result.ramp, sizeof(VpTestResultRampType));
            requestNextState = TRUE;
            pTestCtx->state = ELEC_RINGER_RAMP_DIFF_TO_FEED;
            break;

        case ELEC_RINGER_RAMP_DIFF_TO_FEED:
			msleep(10);
            /* Ramp back to feed voltage */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_TIP_RING, pTempRingers->rampToFeedVADC);
            /* Continue to the ring lead for 3ELE test, otherwise conclude */
            if (pTempRingers->input.ringerTestType == LT_RINGER_ELECTRONIC_PHNE_TEST) {
                pTestCtx->state = ELEC_RINGER_CONCLUDE;
            } else {
                pTestCtx->state = ELEC_RINGER_REQUEST_VBG;
            }
            break;

        /* 
         * Measuring and ramping on the ring lead
         */
        case ELEC_RINGER_REQUEST_VBG:
			msleep(10);
            pTemp886->loopCond.loopCond = VP_LOOP_COND_TEST_VSBG;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_LOOP_CONDITIONS, &pTemp886->loopCond);
            pTestCtx->state = ELEC_RINGER_STORE_VBG;
            break;

        case ELEC_RINGER_STORE_VBG:
			msleep(10);
            pTempRingers->feedVADC = (uint16)pTestPrimRslts->result.loopCond.data;
            pTestCtx->state = ELEC_RINGER_RAMP_RING_INIT;
            requestNextState = TRUE;
            break;

        case ELEC_RINGER_RAMP_RING_INIT:
			msleep(10);
            *pRetval = ElecRingersRampInit(pTestCtx, VP_TEST_RING);
            pTestCtx->state = ELEC_RINGER_RAMP_RING_UP;
            break;

        case ELEC_RINGER_RAMP_RING_UP:
			msleep(10);
            /* Ramp up toward zero (negative rampSizeVADC) */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_RING, -pTempRingers->rampSizeVADC);
            pTestCtx->state = ELEC_RINGER_RAMP_RING_UP_STORE;
            break;

        case ELEC_RINGER_RAMP_RING_UP_STORE:
			msleep(10);
            LtMemCpy(&pTempRingers->vpApiRingRamp1,&pTestPrimRslts->result.ramp, sizeof(VpTestResultRampType));
            requestNextState = TRUE;
            pTestCtx->state = ELEC_RINGER_RAMP_RING_DOWN;
            break;

        case ELEC_RINGER_RAMP_RING_DOWN:
			msleep(10);
            /* Ramp down away from zero (positive rampSizeVADC) */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_RING, pTempRingers->rampSizeVADC);
            pTestCtx->state = ELEC_RINGER_RAMP_RING_DOWN_STORE;
            break;

        case ELEC_RINGER_RAMP_RING_DOWN_STORE:
			msleep(10);
            LtMemCpy(&pTempRingers->vpApiRingRamp2,&pTestPrimRslts->result.ramp, sizeof(VpTestResultRampType));
            requestNextState = TRUE;
            pTestCtx->state = ELEC_RINGER_RAMP_RING_TO_FEED;
            break;

        case ELEC_RINGER_RAMP_RING_TO_FEED:
			msleep(10);
            /* Ramp back to feed voltage */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_RING, pTempRingers->rampToFeedVADC);
            pTestCtx->state = ELEC_RINGER_RAMP_TIP_INIT;
            break;

        /* 
         * Ramping on the ring lead
         * Use the same measured feedVADC from the ring lead
         */
        case ELEC_RINGER_RAMP_TIP_INIT:
			msleep(10);
            *pRetval = ElecRingersRampInit(pTestCtx, VP_TEST_TIP);
            pTestCtx->state = ELEC_RINGER_RAMP_TIP_UP;
            break;

        case ELEC_RINGER_RAMP_TIP_UP:
			msleep(10);
            /* Ramp up toward zero (negative rampSizeVADC) */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_TIP, -pTempRingers->rampSizeVADC);
            pTestCtx->state = ELEC_RINGER_RAMP_TIP_UP_STORE;
            break;

        case ELEC_RINGER_RAMP_TIP_UP_STORE:
			msleep(10);
            LtMemCpy(&pTempRingers->vpApiTipRamp1,&pTestPrimRslts->result.ramp, sizeof(VpTestResultRampType));
            requestNextState = TRUE;
            pTestCtx->state = ELEC_RINGER_RAMP_TIP_DOWN;
            break;

        case ELEC_RINGER_RAMP_TIP_DOWN:
			msleep(10);
            /* Ramp down away from zero (positive rampSizeVADC) */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_TIP, pTempRingers->rampSizeVADC);
            pTestCtx->state = ELEC_RINGER_RAMP_TIP_DOWN_STORE;
            break;

        case ELEC_RINGER_RAMP_TIP_DOWN_STORE:
			msleep(10);
            LtMemCpy(&pTempRingers->vpApiTipRamp2,&pTestPrimRslts->result.ramp, sizeof(VpTestResultRampType));
            requestNextState = TRUE;
            pTestCtx->state = ELEC_RINGER_RAMP_TIP_TO_FEED;
            break;

        case ELEC_RINGER_RAMP_TIP_TO_FEED:
			msleep(10);
            /* Ramp back to feed voltage */
            *pRetval = ElecRingersRamp(pTestCtx, VP_TEST_TIP, pTempRingers->rampToFeedVADC);
            pTestCtx->state = ELEC_RINGER_CONCLUDE;
            break;

        /* Start the conclude primitive if this test not a part of an internal test */
        case ELEC_RINGER_CONCLUDE: {
			msleep(10);
            if (!pTestCtx->pTemp->vp886Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            } else {
                requestNextState = TRUE;
            }
            pTestCtx->state = ELEC_RINGER_COMPLETE;
            break;
        }

        case ELEC_RINGER_COMPLETE:
			msleep(10);
            /* Calculate results and end the test */
            if (!pTempRingers->quickStop) {
                CalculateElecResults(pTestCtx);
            }
            *pRetval = LT_STATUS_DONE;
            break;

        case ELEC_RINGER_ABORT:
			msleep(10);
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

/*
 * ElecRingersRampInit() -
 *
 * This function is used to prepare the line to switch from DC-Feed voltage
 * to a ramping voltage.
 */
static LtTestStatusType
ElecRingersRampInit(
    LtTestCtxType *pTestCtx,
    VpTestTipSelectType lead)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingersTempType *pTempRingers = &pTemp886->tempData.ringers;

    VpTestRampInitType rampInitInput;
    LtVoltageType inputV = ABS(pTempRingers->input.vRingerTest);

    /* Set up ramp parameters for both the init and future ramps */
    if (lead == VP_TEST_TIP_RING) {
        /* Start at the input voltage, ramp 2x the size of the input voltage to
           go to the same magnitude in the opposite polarity. */
        pTempRingers->startVADC = (int16)VP886_UNIT_CONVERT(inputV, VP886_UNIT_MV, VP886_UNIT_ADC_VAB);
        pTempRingers->rampSizeVADC = ABS(2 * pTempRingers->startVADC);
        pTempRingers->rampToFeedVADC = pTempRingers->feedVADC - pTempRingers->startVADC;
    } else {
        /* Start at least 7V beyond the input voltage to allow some headroom to
           avoid zero-cross. */
        pTempRingers->startVADC = ABS((int16)VP886_UNIT_CONVERT(inputV + 7000, VP886_UNIT_MV, VP886_UNIT_ADC_VAB));
        if (pTempRingers->feedVADC > pTempRingers->startVADC) {
            pTempRingers->startVADC = pTempRingers->feedVADC;
        }
        pTempRingers->rampSizeVADC = ABS((int16)VP886_UNIT_CONVERT(inputV, VP886_UNIT_MV, VP886_UNIT_ADC_VAB));
        pTempRingers->rampToFeedVADC = pTempRingers->feedVADC - pTempRingers->startVADC;
    }

    LT_DOUT(LT_DBG_INFO, ("Lead %d, inputV %ld, feedVADC %d,",
        lead, inputV, pTempRingers->feedVADC));
    LT_DOUT(LT_DBG_INFO, ("startVADC %d, rampSizeVADC %d, rampToFeedVADC %d",
        pTempRingers->startVADC, pTempRingers->rampSizeVADC, pTempRingers->rampToFeedVADC));

    rampInitInput.vRate = LT_VP886_ELEC_RINGERS_RAMP_SLOPE;
    rampInitInput.dcVstart = pTempRingers->feedVADC;
    rampInitInput.dcVend = pTempRingers->startVADC;
    rampInitInput.bias = 0;
    rampInitInput.tip = lead;

    return Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP_INIT, &rampInitInput);
}

static LtTestStatusType
ElecRingersRamp(
    LtTestCtxType *pTestCtx,
    VpTestTipSelectType lead,
    int16 voltage)
{
    VpTestRampType rampInput;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingersTempType *pTempRingers = &pTemp886->tempData.ringers;

    rampInput.vRate = LT_VP886_ELEC_RINGERS_VRATE(pTempRingers->input.renFactor);
    rampInput.dcVoltage = voltage;
    rampInput.bias = 0;
    rampInput.tip = lead;

    return Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_RAMP, &rampInput);
}

#define VP886_I_FULL_SCALE 1154 /* 115.4 */

static void
CalculateRegResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingersTempType *pTempRingers = &pTemp886->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;

    const int16 maxRes = pTempRingers->criteria.renHigh;
    const int16 minRes = pTempRingers->criteria.renLow;
    const int32 renFactor = pTempRingers->input.renFactor / 10;

    /* calculate tip to ring ren */
    int32 imt = pTempRingers->imtDiff;
    int32 vab = pTempRingers->vabDiff;

    VpLineTopologyType topInfo;

    /* need topology info to correct for resistance in the loop */
    VpQueryImmediate(pTestCtx->pLineCtx, VP_QUERY_ID_LINE_TOPOLOGY, &topInfo);

    LT_DOUT(LT_DBG_INFO, ("rOutsideDcSense %i Ohms", topInfo.rOutsideDcSense));

    if (vab == 0) {
        pResults->ren = LT_REN_NOT_MEASURED;
        pResults->renrg = LT_REN_NOT_MEASURED;
        pResults->rentg = LT_REN_NOT_MEASURED;
        return;
    } else {
        /*
         * I/V = 1/Ohms = REN
         * full scale = 115.4 / 240.0 = (1/Ohms)
         * (renFactor (Ohms) * full scale (1/Ohms)) * ((measure IMT) / (measure VAB))
         * renFactor * full scale cancels units leaving only I/V = REN
         */
        pResults->ren = (( ((renFactor * VP886_I_FULL_SCALE) / 2400) * imt) / vab);
    }
    LT_DOUT(LT_DBG_INFO, ("imtDiff: %li", imt));
    LT_DOUT(LT_DBG_INFO, ("vabDiff: %li", vab));
    LT_DOUT(LT_DBG_INFO, ("renAB before correction: %li", pResults->ren));

    if (LT_RINGER_REGULAR_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType) {
        const int32 freq = pTempRingers->input.freq / 1000;

        int32 cFactA, cFactB;
        int32 senseA = (int32)(pTemp886->attributes.topology.rSenseA / (10 * 1000));
        int32 senseB = (int32)(pTemp886->attributes.topology.rSenseB / (10 * 1000));
        int32 leakA = (int32)(pTemp886->attributes.topology.rLeakageA / (10 * 1000));
        int32 leakB = (int32)(pTemp886->attributes.topology.rLeakageB / (10 * 1000));

        int32 ilg, renrg, rentg, rgoff, tgoff, troff;

        /* 
         * 22nF (AC Sense + 10k?) || 22nF (CTD+CRD) to ground impedance:
         *     1 / (44e-9 * 2pi * freq) => 
         *     (1/44e-9 * 2pi) * (1 / freq) =>
         *     3617161 * (1 / freq)
         */
        int32 capImp = 3617161 / (freq * 1000); 

        /* Offsets empirically determined */
        if (pTemp886->deviceType == VP_DEV_886_SERIES) {
            if (pTemp886->channelId == 0) {
                troff = 12;
                rgoff = 4;
                tgoff = 6;
            } else {
                troff = 12;
                rgoff = 2;
                tgoff = 4;
            }
        } else { /* VP_DEV_887_SERIES */
            if (pTemp886->channelId == 0) {
                troff = 12;
                rgoff = 6;
                tgoff = 10;
            } else {
                troff = 3;
                rgoff = 2;
                tgoff = 4;
            }
        }
#ifdef LT_API_DISABLE_EMPIRICAL_REN_CORRECTIONS
        rgoff = tgoff = troff = 0;
#endif

        /* Find impedance of sense resistors and leakage resistors */
        cFactA = (leakA) ? (senseA * leakA) / (senseA + leakA) : senseA;
        cFactB = (leakB) ? (senseB * leakB) / (senseB + leakB) : senseB;

        /* Factor in the cap's impedance */
        cFactA = (cFactA * capImp) / (cFactA + capImp);
        cFactB = (cFactB * capImp) / (cFactB + capImp);

        /* convert to mREN and account for offsets */
        cFactA = (renFactor / cFactA);
        cFactB = (renFactor / cFactB);

        LT_DOUT(LT_DBG_INFO, ("renFactor: %li", renFactor));
        LT_DOUT(LT_DBG_INFO, ("cFactA: %li leakA %li, senseA %li, capImp %li", cFactA, leakA, senseA, capImp));
        LT_DOUT(LT_DBG_INFO, ("cFactB: %li leakB %li, senseB %li, capImp %li", cFactB, leakB, senseB, capImp));
        LT_DOUT(LT_DBG_INFO, ("ilgTip: %i vabTip %i", pTempRingers->ilgTip, pTempRingers->vabTip));
        LT_DOUT(LT_DBG_INFO, ("ilgRing: %i vabRing %i", pTempRingers->ilgRing, pTempRingers->vabRing));

        /* calculate tip to ground ren */
        ilg = pTempRingers->ilgTip;
        vab = pTempRingers->vabTip;
        rentg = (( ((renFactor * VP886_I_FULL_SCALE) / 2400) * ilg) / vab);
        pResults->rentg = rentg - cFactA - tgoff;
        LT_DOUT(LT_DBG_INFO, ("rentg: %li", pResults->rentg));

        /* calculate ring to ground ren */
        ilg = pTempRingers->ilgRing;
        vab = pTempRingers->vabRing;
        renrg = (( ((renFactor * VP886_I_FULL_SCALE) / 2400) * ilg) / vab);
        pResults->renrg = renrg - cFactB - rgoff;
        LT_DOUT(LT_DBG_INFO, ("renrg: %li", pResults->renrg));

        /* remove unmodified ring/gnd and tip/gnd result from tip/ring result */
        pResults->ren -= ((renrg + rentg) / 4) + troff;

    } else {
        int32 troff = 0;

        /* Offset empirically determined */
        if (pTemp886->deviceType == VP_DEV_886_SERIES) {
            if (pTemp886->channelId == 0) {
                troff = 38;
            } else {
                troff = 37;
            }
        } else { /* VP_DEV_887_SERIES */
            if (pTemp886->channelId == 0) {
                troff = 39;
            } else {
                troff = 35;
            }
        }
#ifdef LT_API_DISABLE_EMPIRICAL_REN_CORRECTIONS
        troff = 0;
#endif
        pResults->ren -= troff;
    }

    /* remove PTC resistance */
    if (topInfo.rOutsideDcSense > 0) {
        int32 temp = ((1000 * 1000 * 1000) / pResults->ren) - ((1000 * 1000) / (2 * topInfo.rOutsideDcSense));
        pResults->ren = (1000 * 1000 * 1000) / temp;
    }

    if (!pTempRingers->negative) {
        if ((pResults->ren <= 0) && (pResults->ren != LT_REN_NOT_MEASURED)) {
            pResults->ren = 0;
        }
        if ((pResults->renrg <= 0) && (pResults->renrg != LT_REN_NOT_MEASURED)) {
            pResults->renrg = 0;
        }
        if ((pResults->rentg <= 0) && (pResults->rentg != LT_REN_NOT_MEASURED)) {
            pResults->rentg = 0;
        }
    }

    /* check 909 limits */
    if (maxRes < pResults->ren) {
        pResults->fltMask = LT_RNGM_REN_HIGH;
    } else if (minRes > pResults->ren) {
        pResults->fltMask = LT_RNGM_REN_LOW;
    } else {
        pResults->fltMask = LT_TEST_PASSED;
    }

    return;
}

static void
CalculateElecResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886RingersTempType *pTempRingers = &pTemp886->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;
    const int16 maxRes = pTempRingers->criteria.renHigh;
    const int16 minRes = pTempRingers->criteria.renLow;

    int32 cFact;
    int32 i1, i2, slope1, slope2, iavg, slope, cap, kCapIm, kCapIl;
    int32 renFactor = VP_ROUNDED_DIVIDE(pTempRingers->input.renFactor, 1000);
    /*
     * C to ground correction factor
     * accounts for 22nF and 22 nF caps to gnd
     */
    renFactor = ABS(renFactor);

    /* Longitudinal REN Scale Factor (1000s * 0.11906A * 1,000,000 * 1000) / (239.3V) */
    kCapIl = 497534476;

    /* Metallic REN Scale Factor (1000s * 0.11906A * 1,000,000 * 1000) / (239.3V) */
    kCapIm = 497534476;

    /* calculate tip to ring REN */
    i1      = (int32)pTempRingers->vpApiDiffRamp1.imt;
    slope1  = (int32)pTempRingers->vpApiDiffRamp1.vRate;
    i2      = (int32)pTempRingers->vpApiDiffRamp2.imt;
    slope2  = (int32)pTempRingers->vpApiDiffRamp2.vRate;
    iavg    = i2 - i1;
    slope   = 10 * (slope1 - slope2);
    cap     = VP_ROUNDED_DIVIDE(((kCapIm / slope) * iavg), renFactor);
    pResults->ren = VP_ROUNDED_DIVIDE(cap, 10);

    LT_DOUT(LT_DBG_INFO, ("renFactor: %li", renFactor));
    LT_DOUT(LT_DBG_INFO, ("kCapIm: %li", kCapIm));

    LT_DOUT(LT_DBG_INFO, ("imt1: %li", i1));
    LT_DOUT(LT_DBG_INFO, ("vRate1: %li", slope1));
    LT_DOUT(LT_DBG_INFO, ("imt2: %li", i2));
    LT_DOUT(LT_DBG_INFO, ("vRate2: %li", slope2));
    LT_DOUT(LT_DBG_INFO, ("ren: %li", pResults->ren));


    if (LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType) {

        int32 renrg, rentg, rgoff, tgoff, troff;

        /* Offsets empirically determined */
        if (pTemp886->deviceType == VP_DEV_886_SERIES) {
            if (pTemp886->channelId == 0) {
                troff = 33;
                rgoff = 56;
                tgoff = 54;
            } else {
                troff = 34;
                rgoff = 56;
                tgoff = 55;
            }
        } else { /* VP_DEV_887_SERIES */
            if (pTemp886->channelId == 0) {
                troff = 39;
                rgoff = 54;
                tgoff = 56;
            } else {
                troff = 20;
                rgoff = 78;
                tgoff = 51;
            }
        }
#ifdef LT_API_DISABLE_EMPIRICAL_REN_CORRECTIONS
        rgoff = tgoff = troff = 0;
#endif

        /* calculate ring to gnd REN */
        i1      = (int32)pTempRingers->vpApiRingRamp1.imt;
        slope1  = (int32)pTempRingers->vpApiRingRamp1.vRate;
        i2      = (int32)pTempRingers->vpApiRingRamp2.imt;
        slope2  = (int32)pTempRingers->vpApiRingRamp2.vRate;
        iavg    = i2 - i1;
        slope   = 10 * (slope2 - slope1);
        renrg   = VP_ROUNDED_DIVIDE(((ABS(kCapIl) / slope) * iavg), (renFactor * 10));

        pResults->renrg = renrg - rgoff;

        /* calculate tip to gnd REN */
        i1      = (int32)pTempRingers->vpApiTipRamp1.imt;
        slope1  = (int32)pTempRingers->vpApiTipRamp1.vRate;
        i2      = (int32)pTempRingers->vpApiTipRamp2.imt;
        slope2  = (int32)pTempRingers->vpApiTipRamp2.vRate;
        iavg    = i2 - i1;
        slope   = 10 * (slope2 - slope1);
        rentg   = VP_ROUNDED_DIVIDE(((kCapIl / slope) * iavg), (renFactor * 10));

        pResults->rentg = rentg - tgoff;

        /* remove unmodified ring/gnd and tip/gnd result from tip/ring result */
        pResults->ren -= ((renrg + rentg) / 4) + troff;

        LT_DOUT(LT_DBG_INFO, ("imt1_Ring: %li", (int32)pTempRingers->vpApiRingRamp1.imt));
        LT_DOUT(LT_DBG_INFO, ("vRate1_Ring: %li", (int32)pTempRingers->vpApiRingRamp1.vRate));
        LT_DOUT(LT_DBG_INFO, ("imt2_Ring: %li", (int32)pTempRingers->vpApiRingRamp2.imt));
        LT_DOUT(LT_DBG_INFO, ("vRate2_Ring: %li", (int32)pTempRingers->vpApiRingRamp2.vRate));

        LT_DOUT(LT_DBG_INFO, ("imt1_Tip: %li", (int32)pTempRingers->vpApiTipRamp1.imt));
        LT_DOUT(LT_DBG_INFO, ("vRate1_Tip: %li", (int32)pTempRingers->vpApiTipRamp1.vRate));
        LT_DOUT(LT_DBG_INFO, ("imt2_Tip: %li", (int32)pTempRingers->vpApiTipRamp2.imt));
        LT_DOUT(LT_DBG_INFO, ("vRate2_Tip: %li", (int32)pTempRingers->vpApiTipRamp2.vRate));

        LT_DOUT(LT_DBG_INFO, ("renA: %li", pResults->rentg));
        LT_DOUT(LT_DBG_INFO, ("renB: %li", pResults->renrg));
        LT_DOUT(LT_DBG_INFO, ("renAB: %li", pResults->ren));

    } else {
        int32 troff = 0;
        /*
         * If the test was differential only then we need to subtract
         * the theoretical ren to ground that appears on each lead.
         * (22nF + 22nF) * 1000 => 44000pF * 10 (make room for math)
         * renFactor is in nF at this point
        */
        cFact = VP_ROUNDED_DIVIDE(((22 + 22) * 1000 * 10), renFactor);

        /* divide by two then remove the 10 from the math */
        pResults->ren -= VP_ROUNDED_DIVIDE(VP_ROUNDED_DIVIDE(cFact, 2), 10);

        /* Offset empirically determined */
        if (pTemp886->deviceType == VP_DEV_886_SERIES) {
            if (pTemp886->channelId == 0) {
                troff = 42;
            } else {
                troff = 39;
            }
        } else { /* VP_DEV_887_SERIES */
            if (pTemp886->channelId == 0) {
                troff = 44;
            } else {
                troff = 34;
            }
        }
#ifdef LT_API_DISABLE_EMPIRICAL_REN_CORRECTIONS
        troff = 0;
#endif
        pResults->ren -= troff;
    }

    if (!pTempRingers->negative) {
        if ((pResults->ren <= 0) && (pResults->ren != LT_REN_NOT_MEASURED)) {
            pResults->ren = 0;
        }
        if ((pResults->renrg <= 0) && (pResults->renrg != LT_REN_NOT_MEASURED)) {
            pResults->renrg = 0;
        }
        if ((pResults->rentg <= 0) && (pResults->rentg != LT_REN_NOT_MEASURED)) {
            pResults->rentg = 0;
        }
    }

    /* check 909 limits */
    if (maxRes < pResults->ren) {
        pResults->fltMask = LT_RNGM_REN_HIGH;
    } else if (minRes > pResults->ren) {
        pResults->fltMask = LT_RNGM_REN_LOW;
    } else {
        pResults->fltMask = LT_TEST_PASSED;
    }

    return;
}


#endif /* LT_RINGERS_886 */
