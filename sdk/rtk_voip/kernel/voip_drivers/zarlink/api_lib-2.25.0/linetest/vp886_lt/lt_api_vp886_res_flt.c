/** file lt_api_vp886_res_flt.c
 *
 *  This file contains the VP886 resistive faults test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11004 $
 * $LastChangedDate: 2013-06-12 15:30:24 -0500 (Wed, 12 Jun 2013) $
 */
#include "lt_api.h"

#ifdef LT_RES_FLT_886

#include "lt_api_internal.h"

typedef enum {
    RES_FLT_INIT            = LT_GLOBAL_FIRST_STATE,
    RES_FLT_REQUEST,
    RES_FLT_GET_RESULTS,
    RES_FLT_CALCULATE,
    RES_FLT_CONCLUDE,
    RES_FLT_COMPLETE,
    RES_FLT_ABORT           = LT_GLOBAL_ABORTED_STATE,
    RES_FLT_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886ResFltStates;


static bool
ResFltHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx,
    bool *pNeedToRunHighGain);

static void
CalculateLowGainResults(
    LtTestCtxType *pTestCtx);

static void
CalculateHighGainResults(
    LtTestCtxType *pTestCtx);

static bool
LongitudinalShortIsPresent(
    LtTestCtxType *pTestCtx);

extern bool
LtVp886ResFltAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtResFltCriteriaType *ppCriteria =
        (LtResFltCriteriaType*)pTemp886->attributes.pCriteria;

    LtResFltInputType *ppInputs =
        (LtResFltInputType*)pTemp886->attributes.pInputs;

    LtVp886ResFltTempType *pTempResFlt = &pTemp886->tempData.resFlt;

    /* If the inputs are not defined then save defaults into temp structure */
    if (VP_NULL == ppInputs) {
        pTempResFlt->input.startHighGain =  FALSE;
    } else {
        /* Save input data into temp structure */
        pTempResFlt->input.startHighGain = ppInputs->startHighGain;
    }

    /* critera check */
    if (VP_NULL == ppCriteria) {
        pTempResFlt->criteria.resFltLowLimit = LT_RES_FLT_LOW_LIMIT;
    } else {
        pTempResFlt->criteria.resFltLowLimit = ppCriteria->resFltLowLimit;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = ResFltHandlerSM;

    return TRUE;
}

static bool
ResFltHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886ResFltTempType *pTempResFlt = &pTemp886->tempData.resFlt;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("ResFltHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case RES_FLT_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_RES_FLT);
            if (pTempResFlt->input.startHighGain) {
                pTempResFlt->highGainMode = TRUE;
            } else {
                pTempResFlt->highGainMode = FALSE;
            }
            pTestCtx->state = RES_FLT_REQUEST;
            requestNextState = TRUE;
            break;

        case RES_FLT_REQUEST: {
            VpTest3EleResAltResType resFltInput;
            resFltInput.feedBias = 0x09F3;
            resFltInput.shiftZeroFeed = TRUE;
            resFltInput.integrateTime = 18;
            resFltInput.settlingTime = 2;

            if (pTempResFlt->highGainMode) {
                *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_RES_HG, &resFltInput);
            } else {
                *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_RES_LG, &resFltInput);
            }
            pTestCtx->state = RES_FLT_GET_RESULTS;
            break;
        }

        case RES_FLT_GET_RESULTS:
            LtMemCpy(&pTempResFlt->vpApiResFltResults, &pTestPrimRslts->result.resAlt,
                sizeof(VpTestResultAltResType));
            pTestCtx->state = RES_FLT_CALCULATE;
            requestNextState = TRUE;
            break;

        case RES_FLT_CALCULATE: {
            bool needToRunHighGain = FALSE;
            CalculateResults(pTestCtx, &needToRunHighGain);

            if (needToRunHighGain) {
                /* 
                 * This only occurs if the low gain measurements was 
                 * unable to to determine the condition on the line.
                 * Resart the test using the high gain measurements.
                 */
                LtInitOutputStruct(pTestCtx->pResult, LT_TID_RES_FLT);
                pTestCtx->state = RES_FLT_REQUEST;
                pTempResFlt->highGainMode = TRUE;
            } else {
                pTestCtx->state = RES_FLT_CONCLUDE;
            }

            requestNextState = TRUE;
            break;
        }

        /* Start the conclude primitive if this test not a part of an internal test */
        case RES_FLT_CONCLUDE: {
            if (!pTestCtx->pTemp->vp886Temp.internalTest) {
                VpTestConcludeType conclude = { FALSE };
                *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            } else {
                requestNextState = TRUE;
            }
            pTestCtx->state = RES_FLT_COMPLETE;
            break;
        }

        case RES_FLT_COMPLETE:
            /* End the test */
            *pRetval = LT_STATUS_DONE;
            break;

        case RES_FLT_ABORT:
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
    LtTestCtxType *pTestCtx,
    bool *pNeedToRunHighGain)
{

    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886ResFltTempType *pTempResFlt = &pTemp886->tempData.resFlt;

    LT_DOUT(LT_DBG_INFO, ("highGain: %i", pTempResFlt->vpApiResFltResults.highGain));
    LT_DOUT(LT_DBG_INFO, ("vanf: %i", pTempResFlt->vpApiResFltResults.vanf));
    LT_DOUT(LT_DBG_INFO, ("vbnf: %i", pTempResFlt->vpApiResFltResults.vbnf));
    LT_DOUT(LT_DBG_INFO, ("imnf: %i", pTempResFlt->vpApiResFltResults.imnf));
    LT_DOUT(LT_DBG_INFO, ("ilnf: %i", pTempResFlt->vpApiResFltResults.ilnf));

    LT_DOUT(LT_DBG_INFO, ("varf: %i", pTempResFlt->vpApiResFltResults.varf));
    LT_DOUT(LT_DBG_INFO, ("vbrf: %i", pTempResFlt->vpApiResFltResults.vbrf));
    LT_DOUT(LT_DBG_INFO, ("imrf: %i", pTempResFlt->vpApiResFltResults.imrf));
    LT_DOUT(LT_DBG_INFO, ("ilrf: %i", pTempResFlt->vpApiResFltResults.ilrf));

    LT_DOUT(LT_DBG_INFO, ("vazf: %i", pTempResFlt->vpApiResFltResults.vazf));
    LT_DOUT(LT_DBG_INFO, ("vbzf: %i", pTempResFlt->vpApiResFltResults.vbzf));
    LT_DOUT(LT_DBG_INFO, ("imzf: %i", pTempResFlt->vpApiResFltResults.imzf));
    LT_DOUT(LT_DBG_INFO, ("ilzf: %i", pTempResFlt->vpApiResFltResults.ilzf));

    *pNeedToRunHighGain = FALSE;

    if (pTempResFlt->vpApiResFltResults.highGain) {
        if (!LongitudinalShortIsPresent(pTestCtx)) {
            CalculateHighGainResults(pTestCtx);
        }
    } else {
        CalculateLowGainResults(pTestCtx);
        /* 
         * if the low gain calculation sets the high gain mode flag 
         * then we need to let the upper level know to rerun in high gain.
         */
        if (pTempResFlt->highGainMode) {
            *pNeedToRunHighGain = TRUE;
        }
    }

    return;
}

static void
CalculateLowGainResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp = &pTestCtx->pTemp->vp886Temp;
    LtResFltResultType *pResFlt = &pTestCtx->pResult->result.resFlt;
    LtVp886ResFltTempType *pTempResFlt = &pTestCtx->pTemp->vp886Temp.tempData.resFlt;

    LtImpedanceType senseA = (pTemp->attributes.topology.rSenseA / (10 * 1000));
    LtImpedanceType senseB = (pTemp->attributes.topology.rSenseB / (10 * 1000));
    LtImpedanceType leakA = (pTemp->attributes.topology.rLeakageA / (10 * 1000));
    LtImpedanceType leakB = (pTemp->attributes.topology.rLeakageB / (10 * 1000));

    int32 dMicM, dMicL, dLicL, numAB, numAG, numBG, denAB, denAG, denBG;
    LtImpedanceType worst;

    const int32 vanf = pTempResFlt->vpApiResFltResults.vanf;
    const int32 vbnf = pTempResFlt->vpApiResFltResults.vbnf;
    const int32 imnf = pTempResFlt->vpApiResFltResults.imnf  ;
    const int32 ilnf = pTempResFlt->vpApiResFltResults.ilnf;

    const int32 varf = pTempResFlt->vpApiResFltResults.varf;
    const int32 vbrf = pTempResFlt->vpApiResFltResults.vbrf;
    const int32 imrf = pTempResFlt->vpApiResFltResults.imrf;
    const int32 ilrf = pTempResFlt->vpApiResFltResults.ilrf;

    const int32 vazf = pTempResFlt->vpApiResFltResults.vazf;
    const int32 vbzf = pTempResFlt->vpApiResFltResults.vbzf;
    const int32 imzf = pTempResFlt->vpApiResFltResults.imzf  ;
    const int32 ilzf = pTempResFlt->vpApiResFltResults.ilzf;

    /* Calculate the output voltages from the chip */
    const int32 dMvcA =  varf - vanf;
    const int32 dMvcB =  vbrf - vbnf;
    const int32 dLvcA =  (-varf + 2 * vazf - vanf) >> 1;
    const int32 dLvcB =  (-vbrf + 2 * vbzf - vbnf) >> 1;

    /* adjust gdc value based on leakage resistance in the sense paths */
    int32 gdc = 815;
    int32 gdcA = gdc + ((gdc * senseA) / leakA);
    int32 gdcB = gdc + ((gdc * senseB) / leakB);

    LT_DOUT(LT_DBG_INFO, ("Calculating Low Gain Results"));
    LT_DOUT(LT_DBG_INFO, ("leakA: %li", leakA));
    LT_DOUT(LT_DBG_INFO, ("leakB: %li", leakB));
    LT_DOUT(LT_DBG_INFO, ("senseA: %li", senseA));
    LT_DOUT(LT_DBG_INFO, ("senseB: %li", senseB));
    LT_DOUT(LT_DBG_INFO, ("gdc: %li", gdc));
    LT_DOUT(LT_DBG_INFO, ("gdcA: %li", gdcA));
    LT_DOUT(LT_DBG_INFO, ("gdcB: %li", gdcB));
    LT_DOUT(LT_DBG_INFO, ("dMvcA %li", dMvcA));
    LT_DOUT(LT_DBG_INFO, ("dMvcB %li", dMvcB));
    LT_DOUT(LT_DBG_INFO, ("dLvcA %li", dLvcA));
    LT_DOUT(LT_DBG_INFO, ("dLvcB %li", dLvcB));

    /* If any of the measurements were saturated then go to high gain mode */
    if ((ABS(vanf) >= VP_INT16_MAX) || (ABS(vbnf) >= VP_INT16_MAX) ||
        (ABS(imnf) >= VP_INT16_MAX) || (ABS(ilnf) >= VP_INT16_MAX) ||
        (ABS(varf) >= VP_INT16_MAX) || (ABS(vbrf) >= VP_INT16_MAX) ||
        (ABS(imrf) >= VP_INT16_MAX) || (ABS(ilrf) >= VP_INT16_MAX) ||
        (ABS(vazf) >= VP_INT16_MAX) || (ABS(vbzf) >= VP_INT16_MAX) ||
        (ABS(imzf) >= VP_INT16_MAX) || (ABS(ilzf) >= VP_INT16_MAX)) {
        LT_DOUT(LT_DBG_INFO, ("Low Gain Results Saturated must use High Gain"));
        pTempResFlt->highGainMode = TRUE;
        return;
    }

    /* Calculate and ddjust the currents to get the ext. currents */
    dMicM = (2 * -1015 * (imnf - imrf)) + gdcA * dMvcA - gdcB * dMvcB;
    dMicL = (2 * -2031 * (ilnf - ilrf)) - gdcA * dMvcA - gdcB * dMvcB;
    dLicL = (-2031 * (ilnf - 2 * ilzf + ilrf)) - gdcA * dLvcA - gdcB * dLvcB;

    /* Shift the current results into 16 bit words */
    dMicM = (dMicM + 8192) >> 14;
    dMicL = (dMicL + 8192) >> 14;
    dLicL = (dLicL + 8192) >> 14;
    LT_DOUT(LT_DBG_INFO, ("dMicM %li", dMicM));
    LT_DOUT(LT_DBG_INFO, ("dMicL %li", dMicL));
    LT_DOUT(LT_DBG_INFO, ("dLicL %li", dLicL));

    /* calculate Tip to Ring Res to closest 10th of an Ohm */
    numAB = VP_ROUNDED_DIVIDE(dLvcB*dMvcB - dLvcB*dMvcA + dLvcA*dMvcB - dLvcA*dMvcA, 16) * 3125;
    denAB = VP_ROUNDED_DIVIDE(dLvcB*dMicM + dLvcA*dMicM - dMvcB*dLicL + dMvcA*dLicL, 512);
    denAB = (denAB/10 ? denAB : (denAB < 0 ? -10 : 10));
    pResFlt->rtr = 10 * (numAB/denAB);
    if ((pResFlt->rtr < 0) || (pResFlt->rtr > VVP_RES_FLT_OPEN_CIRCUIT)) {
        pResFlt->rtr = LT_IMPEDANCE_OPEN_CKT;
    }
    LT_DOUT(LT_DBG_INFO, ("numAB %li", numAB));
    LT_DOUT(LT_DBG_INFO, ("denAB %li", denAB));

    /* calculate Tip to Gnd Res to closest 10th of an Ohm */
    numAG = VP_ROUNDED_DIVIDE(dLvcB * dMvcA - dLvcA * dMvcB, 8) * 3125;
    denAG = VP_ROUNDED_DIVIDE(dLvcB * dMicL - dMvcB * dLicL, 128);
    denAG = (denAG/10 ? denAG : (denAG < 0 ? -10 : 10));
    pResFlt->rtg = 10 * (numAG/denAG);
    if ((pResFlt->rtg < 0) || (pResFlt->rtg > VVP_RES_FLT_OPEN_CIRCUIT)) {
        pResFlt->rtg = LT_IMPEDANCE_OPEN_CKT;
    }
    LT_DOUT(LT_DBG_INFO, ("numAG %li", numAG));
    LT_DOUT(LT_DBG_INFO, ("denAG %li", denAG));

    /* calculate Ring to Gnd Res to closest 10th of an Ohm */
    numBG = VP_ROUNDED_DIVIDE(dLvcB * dMvcA - dLvcA * dMvcB, 8) * 3125;
    denBG = VP_ROUNDED_DIVIDE(dMvcA * dLicL - dLvcA * dMicL, 128);
    denBG = (denBG/10 ? denBG : (denBG < 0 ? -10 : 10));
    pResFlt->rrg = 10 * (numBG/denBG);
    if ((pResFlt->rrg < 0) || (pResFlt->rrg > VVP_RES_FLT_OPEN_CIRCUIT)) {
        pResFlt->rrg = LT_IMPEDANCE_OPEN_CKT;
    }
    LT_DOUT(LT_DBG_INFO, ("numBG %li", numBG));
    LT_DOUT(LT_DBG_INFO, ("denBG %li", denBG));

    /* If longitudinal shift was not adequate go to high gain mode */
    if (ABS((dLvcA + dLvcB)) < 410) {
        LT_DOUT(LT_DBG_INFO, ("Low Gain inadequate long shift must use High Gain %li < 410", ABS(dLvcA + dLvcB)));
        pTempResFlt->highGainMode = TRUE;
        return;
    }

    /* Find worst data point */
    pResFlt->fltMask = LT_TEST_PASSED;
    worst = pTempResFlt->criteria.resFltLowLimit;
    if ((pResFlt->rtr < worst) ||
        (pResFlt->rtg < worst) ||
        (pResFlt->rrg < worst)) {

        LtImpedanceType rtr = pResFlt->rtr;
        LtImpedanceType rtg = pResFlt->rtg;
        LtImpedanceType rrg = pResFlt->rrg;

#if 1
        /* Check if tip to ring is worse than criteria */
        if (rtr < worst) {
            pResFlt->rtr = rtr;
            pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->fltMask = LT_RESFM_DIFF;
            pResFlt->rtr++;
            worst = rtr;
        }
        /* Check if tip to gnd is worse than previous worst case */
        if (rtg < worst) {
            pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rtg = rtg;
            pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->fltMask = LT_RESFM_TIP;
            worst = rtg;
        }
        /* Check if ring to gnd is worse than previous worst case */
        if (rrg < worst) {
            pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rrg = rrg;
            pResFlt->fltMask = LT_RESFM_RING;
            worst = rrg;
        }
#else
        /* Check if tip to ring is worse than criteria */
        pResFlt->rtr = rtr;
        if (rtr < worst) {
            pResFlt->fltMask = LT_RESFM_DIFF;
            worst = rtr;
        }
        /* Check if tip to gnd is worse than previous worst case */
        pResFlt->rtg = rtg;
        if (rtg < worst) {
            pResFlt->fltMask = LT_RESFM_TIP;
            worst = rtg;
        }
        /* Check if ring to gnd is worse than previous worst case */
        pResFlt->rrg = rrg;
        if (rrg < worst) {
            pResFlt->fltMask = LT_RESFM_RING;
            worst = rrg;
        }
#endif
    }

    /* 
     * Work around - 
     * The resistive faults primitive does not always automatically jump to
     * high gain when a low resistive value is detected from tip to ground.
     * This causes the primitive to return a number that is close to the 
     * limit of the datasheet.
     *
     * So if the worst measurement was tip/gnd and the value is less than
     * 7k then rerun the primitive in high gain.
     */
    if ((pResFlt->rtg != LT_IMPEDANCE_NOT_MEASURED) && (pResFlt->rtg < 70000)) {
        pTempResFlt->highGainMode = TRUE;
    }
    return;
}

static void
CalculateHighGainResults(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp = &pTestCtx->pTemp->vp886Temp;
    LtResFltResultType *pResFlt = &pTestCtx->pResult->result.resFlt;
    LtVp886ResFltTempType *pTempResFlt = &pTestCtx->pTemp->vp886Temp.tempData.resFlt;
    LtImpedanceType senseA = (pTemp->attributes.topology.rSenseA / 10000);
    LtImpedanceType senseB = (pTemp->attributes.topology.rSenseB / 10000);
    LtImpedanceType leakA = (pTemp->attributes.topology.rLeakageA / 10000);
    LtImpedanceType leakB = (pTemp->attributes.topology.rLeakageB / 10000);

    int32 numAB, numAG, numBG, denAB, denAG, denBG, rtr, rtg, rrg, worst;
    int32 dMicM, dMicL, dLicL;

    const int32 vanf = pTempResFlt->vpApiResFltResults.vanf;
    const int32 vbnf = pTempResFlt->vpApiResFltResults.vbnf;
    const int32 imnf = pTempResFlt->vpApiResFltResults.imnf;
    const int32 ilnf = pTempResFlt->vpApiResFltResults.ilnf;

    const int32 varf = pTempResFlt->vpApiResFltResults.varf;
    const int32 vbrf = pTempResFlt->vpApiResFltResults.vbrf;
    const int32 imrf = pTempResFlt->vpApiResFltResults.imrf;
    const int32 ilrf = pTempResFlt->vpApiResFltResults.ilrf;

    const int32 vazf = pTempResFlt->vpApiResFltResults.vazf;
    const int32 vbzf = pTempResFlt->vpApiResFltResults.vbzf;
    const int32 ilzf = pTempResFlt->vpApiResFltResults.ilzf;

    /* Calculate the output voltage changes */
    const int32 dMvcA =  vanf - varf;
    const int32 dMvcB =  vbnf - vbrf;
    const int32 dLvcA =  (-vanf + 2 * vazf - varf) >> 1;
    const int32 dLvcB =  (-vbnf + 2 * vbzf - vbrf) >> 1;

    VpLineTopologyType topInfo;

    int32 gfud = 0;

    /* adjust gdc value based on leakage resistance in the sense paths*/
    int32 gdc = 102;
    int32 gdcA = gdc + ((gdc * senseA) / leakA);
    int32 gdcB = gdc + ((gdc * senseB) / leakB);
    LT_DOUT(LT_DBG_INFO, ("Calculating High Gain Results"));
    LT_DOUT(LT_DBG_INFO, ("leakA: %li", leakA));
    LT_DOUT(LT_DBG_INFO, ("leakB: %li", leakB));
    LT_DOUT(LT_DBG_INFO, ("senseA: %li", senseA));
    LT_DOUT(LT_DBG_INFO, ("senseB: %li", senseB));
    LT_DOUT(LT_DBG_INFO, ("gfud: %li", gfud));
    LT_DOUT(LT_DBG_INFO, ("gdc: %li", gdc));
    LT_DOUT(LT_DBG_INFO, ("gdcA: %li", gdcA));
    LT_DOUT(LT_DBG_INFO, ("gdcB: %li", gdcB));
    LT_DOUT(LT_DBG_INFO, ("dMvcA %li", dMvcA));
    LT_DOUT(LT_DBG_INFO, ("dMvcB %li", dMvcB));
    LT_DOUT(LT_DBG_INFO, ("dLvcA %li", dLvcA));
    LT_DOUT(LT_DBG_INFO, ("dLvcB %li", dLvcB));

    /* need topology info to correct for resistance in the loop */
    VpQueryImmediate(pTestCtx->pLineCtx, VP_QUERY_ID_LINE_TOPOLOGY, &topInfo);
    LT_DOUT(LT_DBG_INFO, ("rOutsideDcSense %i Ohms", topInfo.rOutsideDcSense));

    /*
     * 1) Calculate the output current changes from the chip
     * 2) Adjust the longitudinal current measurements for the metallic voltage
     * 3) Adjust for the currents in the DC sense resistors to get the external currents
     */
    dMicM = (2 * -25387L * (imrf - imnf))         /* 1 */
        + (gdcA * dMvcA - gdcB * dMvcB);          /* 3 */
    dMicL = (2 * -25387L * (ilrf - ilnf))         /* 1 */
        + (gfud * (dMvcA - dMvcB))                /* 2 */
        - (gdcA * dMvcA + gdcB * dMvcB);          /* 3 */
    dLicL = (-25387L * (ilrf - 2 * ilzf + ilnf))  /* 1 */
        + (gfud * (dLvcA - dLvcB))                /* 2 */
        - (gdcA * dLvcA + gdcB * dLvcB);          /* 3 */


    /* Shift the current results into 16 bit words */
    dMicM = (dMicM + 8192) >> 14;
    dMicL = (dMicL + 8192) >> 14;
    dLicL = (dLicL + 8192) >> 14;
    LT_DOUT(LT_DBG_INFO, ("dMicM %li", dMicM));
    LT_DOUT(LT_DBG_INFO, ("dMicL %li", dMicL));
    LT_DOUT(LT_DBG_INFO, ("dLicL %li", dLicL));

    /* Calculate Tip to Ring Res. to closest 10th of an Ohm. */
    numAB = VP_ROUNDED_DIVIDE(dLvcB*dMvcB - dLvcB*dMvcA + dLvcA*dMvcB - dLvcA*dMvcA, 1024) * 3125;
    denAB = VP_ROUNDED_DIVIDE(dLvcB*dMicM + dLvcA*dMicM - dMvcB*dLicL + dMvcA*dLicL, 4096);
    denAB = (denAB/10 ? denAB : (denAB < 0 ? -10 : 10));
    rtr = 10 * ((numAB/denAB) - (2 * topInfo.rOutsideDcSense));
    LT_DOUT(LT_DBG_INFO, ("numAB %li", numAB));
    LT_DOUT(LT_DBG_INFO, ("denAB %li", denAB));

    /* calculate Tip to Gnd Res to closest 10th of an Ohm */
    numAG = VP_ROUNDED_DIVIDE(dLvcB * dMvcA - dLvcA * dMvcB, 512) * 3125;
    denAG = VP_ROUNDED_DIVIDE(dLvcB * dMicL - dMvcB * dLicL, 1024);
    denAG = (denAG/10 ? denAG : (denAG < 0 ? -10 : 10));
    rtg = 10 * ((numAG/denAG) - topInfo.rOutsideDcSense);
    LT_DOUT(LT_DBG_INFO, ("numAG %li", numAG));
    LT_DOUT(LT_DBG_INFO, ("denAG %li", denAG));

    /* calculate Ring to Gnd Res to closest 10th of an Ohm */
    numBG = VP_ROUNDED_DIVIDE(dLvcB * dMvcA - dLvcA * dMvcB, 512) * 3125;
    denBG = VP_ROUNDED_DIVIDE(dMvcA * dLicL - dLvcA * dMicL, 1024);
    denBG = (denBG/10 ? denBG : (denBG < 0 ? -10 : 10));
    rrg = 10 * ((numBG/denBG) - topInfo.rOutsideDcSense);
    LT_DOUT(LT_DBG_INFO, ("numBG %li", numBG));
    LT_DOUT(LT_DBG_INFO, ("denBG %li", denBG));

    if ((0 > rtr) || (VVP_RES_FLT_OPEN_CIRCUIT < rtr )) {
        /* is this an open circuit? */
        rtr = LT_IMPEDANCE_OPEN_CKT;
    }

    if ((0 > rtg) || (VVP_RES_FLT_OPEN_CIRCUIT < rtg )) {
        /* is this an open circuit? */
        rtg = LT_IMPEDANCE_OPEN_CKT;
    }

    if ((0 > rrg) || (VVP_RES_FLT_OPEN_CIRCUIT < rrg )) {
        /* is this an open circuit? */
        rrg = LT_IMPEDANCE_OPEN_CKT;
    }

    /* ----------------------Find worst data point --------------------------*/

    pResFlt->fltMask = LT_TEST_PASSED;
    worst = pTempResFlt->criteria.resFltLowLimit;

    if ((rtr < worst) ||
        (rtg < worst) ||
        (rrg < worst)) {

        /* is this condiditon worse than the previous? */
        if (rtr < worst) {
            pResFlt->rtr = rtr;
            pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->fltMask = LT_RESFM_DIFF;
            worst = rtr;
        }

        /* is this condiditon worse than the previous? */
        if (rtg < worst) {
            pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rtg = (rtg <= LT_VP886_RES_FLT_SHORT_TO_GND) ? LT_IMPEDANCE_SHORT_CKT : rtg;
            pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->fltMask = LT_RESFM_TIP;
            worst = rtg;
        }

        /* is this condiditon worse than the previous? */
        if (rrg < worst) {
            pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rrg = (rrg <= LT_VP886_RES_FLT_SHORT_TO_GND) ? LT_IMPEDANCE_SHORT_CKT : rrg;
            pResFlt->fltMask = LT_RESFM_RING;
            worst = rrg;
        }
    } else {
        pResFlt->rtr = rtr;
        pResFlt->rtg = rtg;
        pResFlt->rrg = rrg;
    }

    return;
}

static bool
LongitudinalShortIsPresent(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886ResFltTempType *pTempResFlt = &pTemp886->tempData.resFlt;
    LtResFltResultType *pResFlt = &pTestCtx->pResult->result.resFlt;

    const int16 ilzf = pTempResFlt->vpApiResFltResults.ilzf;
    const int16 ilnf = pTempResFlt->vpApiResFltResults.ilnf;
    const int16 ilrf = pTempResFlt->vpApiResFltResults.ilrf;

    pResFlt->fltMask = LT_TEST_PASSED;

    LT_DOUT(LT_DBG_INFO, ("Checking for Longitudinal Short"));
    LT_DOUT(LT_DBG_INFO, ("ilnf: %i", ilnf));
    LT_DOUT(LT_DBG_INFO, ("ilrf: %i", ilrf));
    LT_DOUT(LT_DBG_INFO, ("ilzf: %i", ilzf));

    /* looking for a foreign current source */
    if (ilzf == VP_INT16_MIN) {
        pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->fltMask = LT_TEST_MEASUREMENT_ERROR;
        pResFlt->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
        LT_DOUT(LT_DBG_INFO, ("Foreign Current Source Detected"));
        return TRUE;
    }

    /* looking for hard short to ground */
    if (ilzf == VP_INT16_MAX) {
        pResFlt->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;

        if (ilnf == VP_INT16_MAX) {
            pResFlt->rrg = LT_IMPEDANCE_SHORT_CKT;
            pResFlt->fltMask |= LT_RESFM_RING;
            LT_DOUT(LT_DBG_INFO, ("Ring to Gnd Short Detected"));
        }

        if (ilrf == VP_INT16_MAX) {
            pResFlt->rtg = LT_IMPEDANCE_SHORT_CKT;
            pResFlt->fltMask |= LT_RESFM_TIP;
            LT_DOUT(LT_DBG_INFO, ("Tip to Gnd Short Detected"));
        }

        /* If both tip and ring shorted then we need to report rtr as a short also */
        if ((ilnf == VP_INT16_MAX) && (ilrf == VP_INT16_MAX)) {
            pResFlt->rtr = LT_IMPEDANCE_SHORT_CKT;
            pResFlt->fltMask |= LT_RESFM_DIFF;
            LT_DOUT(LT_DBG_INFO, ("Tip to Ring Short Detected"));
        }

        /* if the primitive reports long fault but did not 
           find it on tip/ring report an error */
        if (pResFlt->fltMask == LT_TEST_PASSED) {
            pResFlt->fltMask = LT_TEST_MEASUREMENT_ERROR;
            LT_DOUT(LT_DBG_INFO, ("Unable to determine the location of the short"));
        }

        return TRUE;
    }

    return FALSE;
}

#endif /* LT_RES_FLT_886 */
