/** file lt_api_vp886_msocket.c
 *
 *  This file contains the VP886 master socket test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10344 $
 * $LastChangedDate: 2012-07-26 11:34:08 -0500 (Thu, 26 Jul 2012) $
 */
#include "lt_api.h"

#ifdef LT_MSOCKET_886

#include "lt_api_internal.h"

typedef enum {
    MSOCKET_INIT            = LT_GLOBAL_FIRST_STATE,
    MSOCKET_GO_TO_ACTIVE,
    MSOCKET_LINE_STATE_SETTLE,
    MSOCKET_START,
    MSOCKET_GET_RESULTS,
    MSOCKET_CONCLUDE,
    MSOCKET_COMPLETE,
    MSOCKET_ABORT           = LT_GLOBAL_ABORTED_STATE,
    MSOCKET_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886MSocketStates;


static bool
MSocketHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResultsType1(
    LtTestCtxType *pTestCtx);

static void
CalculateResultsType2(
    LtTestCtxType *pTestCtx);

extern bool 
LtVp886MSocketAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;

    LtMSocketInputType *ppInputs =
        (LtMSocketInputType*)pTemp886->attributes.pInputs;

    LtVp886MSocketTempType *pTempMSocket = &pTemp886->tempData.mSocket;

    /* If the inputs are not defined then save defaults into temp structure */
    if (VP_NULL == ppInputs) {
        pTempMSocket->input.mSocket = LT_MSOCKET_DFLTI_TEST_TYPE;
    } else {
        /* Save input data into temp structure */
        pTempMSocket->input.mSocket = ppInputs->mSocket;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = MSocketHandlerSM;
    return TRUE;
}

static bool
MSocketHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886MSocketTempType *pTempMSocket = &pTemp886->tempData.mSocket;
    LtMSocketType mSocketType = pTempMSocket->input.mSocket;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("MSocketHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case MSOCKET_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_MSOCKET);
            pTestCtx->state = MSOCKET_GO_TO_ACTIVE;
            requestNextState = TRUE;
            break;

        case MSOCKET_GO_TO_ACTIVE:
            *pRetval = Vp886SetLineStateWrapper(pTestCtx, VP_LINE_ACTIVE);
            pTestCtx->state = MSOCKET_LINE_STATE_SETTLE;
            requestNextState = TRUE;
            break;

        case MSOCKET_LINE_STATE_SETTLE:
            pTemp886->testTimer.timerVal = LT_VP886_STATE_CHANGE_SETTLE;
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_DELAY, &pTemp886->testTimer);
            pTestCtx->state = MSOCKET_START;
            break;

        case MSOCKET_START:
            switch (mSocketType) {
                case LT_MSOCKET_TYPE_1: {
                    VpTest3EleResAltResType lowGainInput;

                    lowGainInput.feedBias = 0x09F3;         /* +/- 6V of feed */
                    lowGainInput.shiftZeroFeed = FALSE;     /* constant longitudinal voltage */
                    lowGainInput.integrateTime = 18;        /* 18ms */
                    lowGainInput.settlingTime = 2;          /* 2ms */

                    *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_RES_LG, &lowGainInput);
                    break;
                }

                case LT_MSOCKET_TYPE_2: {
                    VpTestMSocketType mSInput;

                    mSInput.dummy = 2;

                    *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_MSOCKET_TYPE2, &mSInput);
                    break;
                }

                default:
                    LT_DOUT(LT_DBG_ERRR, ("Unknown master socket type"));
                    break;
            }
            pTestCtx->state = MSOCKET_GET_RESULTS;
            break;

        case MSOCKET_GET_RESULTS:
            /* Calculate results and end the test */
            switch (mSocketType) {
                case LT_MSOCKET_TYPE_1:
                    LtMemCpy(&pTempMSocket->vpApiMSocketType1Results, &pTestPrimRslts->result.resAlt,
                        sizeof(VpTestResultAltResType));
                    break;

                case LT_MSOCKET_TYPE_2:
                    LtMemCpy(&pTempMSocket->vpApiMSocketType2Results, &pTestPrimRslts->result.mSock,
                        sizeof(VpTestResultMSockType));
                    break;

                default:
                    LT_DOUT(LT_DBG_ERRR, ("Unknown master socket type %d", mSocketType));
                    break;
            }
            requestNextState = TRUE;
            pTestCtx->state = MSOCKET_CONCLUDE;
            break;

        case MSOCKET_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = MSOCKET_COMPLETE;
            break;
        }

        case MSOCKET_COMPLETE:
            /* Calculate results and end the test */
            switch (mSocketType) {
                case LT_MSOCKET_TYPE_1:
                    CalculateResultsType1(pTestCtx);
                    break;

                case LT_MSOCKET_TYPE_2:
                    CalculateResultsType2(pTestCtx);
                    break;

                default:
                    LT_DOUT(LT_DBG_ERRR, ("Unknown master socket type %d", mSocketType));
                    break;
            }
            *pRetval = LT_STATUS_DONE;
            break;

        case MSOCKET_ABORT:
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
CalculateResultsType1(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886MSocketTempType *pTempMSocket = &pTemp886->tempData.mSocket;
    LtMSocketResultType *pResults = &pTestCtx->pResult->result.mSocket;
    int32 cIMnf, cIMzf, cIMrf;
    int32 denominator, numerator, fwd;
    int32 res;
    bool polarity = TRUE;

    const int32 vanf = pTempMSocket->vpApiMSocketType1Results.vanf;
    const int32 vbnf = pTempMSocket->vpApiMSocketType1Results.vbnf;
    const int32 imnf = pTempMSocket->vpApiMSocketType1Results.imnf;
    const int32 vazf = pTempMSocket->vpApiMSocketType1Results.vazf;
    const int32 vbzf = pTempMSocket->vpApiMSocketType1Results.vbzf;
    const int32 imzf = pTempMSocket->vpApiMSocketType1Results.imzf;
    const int32 varf = pTempMSocket->vpApiMSocketType1Results.varf;
    const int32 vbrf = pTempMSocket->vpApiMSocketType1Results.vbrf;
    const int32 imrf = pTempMSocket->vpApiMSocketType1Results.imrf;

    if ((ABS(vanf) >= VP_INT16_MAX) || (ABS(vbnf) >= VP_INT16_MAX) ||
        (ABS(imnf) >= VP_INT16_MAX) || (ABS(varf) >= VP_INT16_MAX) ||
        (ABS(vbrf) >= VP_INT16_MAX) || (ABS(imrf) >= VP_INT16_MAX) ||
        (ABS(vazf) >= VP_INT16_MAX) || (ABS(vbzf) >= VP_INT16_MAX) ||
        (ABS(imzf) >= VP_INT16_MAX)) {
        /* Something interferes on the line, impossible to check for a master socket */
        LT_DOUT(LT_DBG_ERRR, ("Vp886MSocketCalResultType1(): Impossible to compute"));
        return;
    }

    pResults->fltMask = LT_TEST_PASSED;

    cIMnf = (-2028 * imnf - 852 * (vanf - vbnf) + 8192) / 16384;
    cIMzf = (-2028 * imzf - 852 * (vazf - vbzf) + 8192) / 16384;
    cIMrf = (-2028 * imrf - 852 * (varf - vbrf) + 8192) / 16384;

    denominator = ((varf - vbrf) - (vazf - vbzf)) * (cIMnf - cIMzf) -
        ((vanf - vbnf) - (vazf - vbzf)) * (cIMrf - cIMzf);

    if (denominator > 0) {
        numerator = (varf - vbrf - vazf + vbzf) * (vanf - vbnf + 75 - vazf + vbzf);
    } else {
        polarity = FALSE;
        numerator = -(vanf - vbnf - vazf + vbzf) * (varf - vbrf - 75 - vazf + vbzf);
    }

    res = numerator * 32 / denominator * -3125;

    /* LT_MSKT_TWO_OPPOSITE calculation  (average of both feed) */
    fwd = (vanf - vbnf + 75 - (vazf - vbzf)) * 50000 / (cIMnf - cIMzf) +
        (varf - vbrf - 75 - (vazf - vbzf)) * 50000 / (cIMrf - cIMzf);

    pResults->fltMask = LT_MSKT_NOT_PRESENT;
    if ((res > 400000) && (res < 940000)) {             /* 1 MSocket */
        /* Allow 40% of error */
        if (polarity == TRUE) {
            /* Cathode on A lead */
            pResults->fltMask = LT_MSKT_TEST_PASSED;
        } else {
            pResults->fltMask = LT_MSKT_REVERSE;
        }
    } else if ((res > 141000) && (res < 400001)) {      /* 2 MSockets */
        /* Allow 40% of error */
        if (polarity == TRUE) {
            /* Cathode on A lead */
            pResults->fltMask = LT_MSKT_TWO_PARALLEL;
        } else {
            pResults->fltMask = LT_MSKT_TWO_REVERSE;
        }
    } else if ((fwd > 376000) && (fwd < 564000)) {      /* 2 MSockets in opposition */
        pResults->fltMask = LT_MSKT_TWO_OPPOSITE;
    }

    LT_DOUT(LT_DBG_INFO, ("vanf: %li", vanf));
    LT_DOUT(LT_DBG_INFO, ("vbnf: %li", vbnf));
    LT_DOUT(LT_DBG_INFO, ("imnf: %li", imnf));
    LT_DOUT(LT_DBG_INFO, ("vazf: %li", vazf));
    LT_DOUT(LT_DBG_INFO, ("vbzf: %li", vbzf));
    LT_DOUT(LT_DBG_INFO, ("imzf: %li", imzf));
    LT_DOUT(LT_DBG_INFO, ("varf: %li", varf));
    LT_DOUT(LT_DBG_INFO, ("vbrf: %li", vbrf));
    LT_DOUT(LT_DBG_INFO, ("imrf: %li\n", imrf));
    LT_DOUT(LT_DBG_INFO, ("cIMnf: %li", cIMnf));
    LT_DOUT(LT_DBG_INFO, ("cIMzf: %li", cIMzf));
    LT_DOUT(LT_DBG_INFO, ("cIMrf: %li\n", cIMrf));

    LT_DOUT(LT_DBG_INFO, ("numerator: %li, denominator: %li", numerator, denominator));
    LT_DOUT(LT_DBG_INFO, ("Master socket signature: %li", res));
    LT_DOUT(LT_DBG_INFO, ("Opposite socket signature: %li\n", fwd));

    return;
}

static void
CalculateResultsType2(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886MSocketTempType *pTempMSocket = &pTemp886->tempData.mSocket;
    LtMSocketResultType *pResults = &pTestCtx->pResult->result.mSocket;
    int32 decay, iM0est, iM1est, vStepC, rSigCest;

    int32 vanf;
    int32 vbnf;
    int32 imrf1;
    int32 varf2;
    int32 vbrf2;
    int32 imrf2;
    int32 imrf3;

    LT_DOUT(LT_DBG_INFO, ("vanf: %i", pTempMSocket->vpApiMSocketType2Results.vanf));
    LT_DOUT(LT_DBG_INFO, ("vbnf: %i", pTempMSocket->vpApiMSocketType2Results.vbnf));
    LT_DOUT(LT_DBG_INFO, ("imrf1: %i", pTempMSocket->vpApiMSocketType2Results.imrf1));
    LT_DOUT(LT_DBG_INFO, ("varf2: %i", pTempMSocket->vpApiMSocketType2Results.varf2));
    LT_DOUT(LT_DBG_INFO, ("vbrf2: %i", pTempMSocket->vpApiMSocketType2Results.vbrf2));
    LT_DOUT(LT_DBG_INFO, ("imrf2: %i", pTempMSocket->vpApiMSocketType2Results.imrf2));
    LT_DOUT(LT_DBG_INFO, ("imrf3: %i", pTempMSocket->vpApiMSocketType2Results.imrf3));

    /* Convert units to mV and nA */
    vanf = VP886_UNIT_CONVERT(pTempMSocket->vpApiMSocketType2Results.vanf, VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    vbnf = VP886_UNIT_CONVERT(pTempMSocket->vpApiMSocketType2Results.vbnf, VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    imrf1 = VP_ROUNDED_DIVIDE((pTempMSocket->vpApiMSocketType2Results.imrf1 * VP886_STEP_ADC_MET_LOW_GAIN_PA), 1000);

    varf2 = VP886_UNIT_CONVERT(pTempMSocket->vpApiMSocketType2Results.varf2, VP886_UNIT_ADC_VAB, VP886_UNIT_MV);
    vbrf2 = VP886_UNIT_CONVERT(pTempMSocket->vpApiMSocketType2Results.vbrf2, VP886_UNIT_ADC_VAB, VP886_UNIT_MV);

    imrf2 = VP_ROUNDED_DIVIDE((pTempMSocket->vpApiMSocketType2Results.imrf2 * VP886_STEP_ADC_MET_LOW_GAIN_PA), 1000);
    imrf3 = VP_ROUNDED_DIVIDE((pTempMSocket->vpApiMSocketType2Results.imrf3 * VP886_STEP_ADC_MET_LOW_GAIN_PA), 1000);

    LT_DOUT(LT_DBG_INFO, ("vanf (mV): %li", vanf));
    LT_DOUT(LT_DBG_INFO, ("vbnf (mV): %li", vbnf));
    LT_DOUT(LT_DBG_INFO, ("imrf1 (pA): %li", imrf1));
    LT_DOUT(LT_DBG_INFO, ("varf2 (mV): %li", varf2));
    LT_DOUT(LT_DBG_INFO, ("vbrf2 (mV): %li", vbrf2));
    LT_DOUT(LT_DBG_INFO, ("imrf2 (pA): %li", imrf2));
    LT_DOUT(LT_DBG_INFO, ("imrf3 (pA): %li", imrf3));

    if ((imrf1 == imrf2) || (imrf1 == imrf3) || (imrf2 == imrf3) ||
        ((imrf1 - 2 * imrf2 + imrf3) == 0)) {
        /* Something interferes on the line, impossible to check for a master socket */
        LT_DOUT(LT_DBG_ERRR, ("Vp886MSocketCalResultType2(): Impossible to compute"));
        return;
    }

    /* Calculate decay as the ratio of the current changes between T2-T3 and
     * T1-T2, where T1 = 200ms, T2 = 800ms, T3 = 1400ms
     * decay = (IM3 - IM2)/(IM1 - IM2), scaled up by 1000 for integer math */
    decay = VP_ROUNDED_DIVIDE((imrf2 - imrf3) * 1000, (imrf1 - imrf2));
    
    /* iM1est is an estimate of the capacitor current at T1, given by
     * (IM1 - IM2)/(1 - decay).  In this case we have substituted in the
     * expression for decay due to integer math. */
    iM1est = VP_ROUNDED_DIVIDE((imrf1 - imrf2) * (imrf1 - imrf2), (imrf1 - 2 * imrf2 + imrf3));
    
    /* iM0est is an estimate of the capacitor current immediately after the
     * voltage change, given by iM1est * e ^ (T1/tauest), where tauest is
     * Rsig*Csig (470kOhm * 1.8uF).  With T1 = 200ms, this comes out to
     * iM0est = iM1est * 1.267 */
    iM0est = VP_ROUNDED_DIVIDE(iM1est * 1267, 1000);

    if (iM0est == 0) {
        /* Something interferes on the line, impossible to check for a master socket */
        LT_DOUT(LT_DBG_ERRR, ("Vp886MSocketCalResultType2(): Impossible to compute"));
        return;
    }

    /* Calculate the voltage across the signature resistance immediately after
     * the voltage step */
    vStepC = (varf2 - vbrf2) - (vanf - vbnf);
    
    /* The signature resistance is calculated as vStepC / iM0est
     * For integer math it's easier to do vStepC * (1/iM0est), or
     * vStepC * (1000000 / iM0est) to account for the units of mV and nA */
    rSigCest = vStepC * VP_ROUNDED_DIVIDE(1000000, iM0est);

    pResults->fltMask = LT_MSKT_NOT_PRESENT;

    /* Allow ~12% tolerance from the expected values of 0.492 decay factor 
     * and 470kOhm signature resistance */
    if ((ABS(decay - 492) < 59) && (ABS(rSigCest - 470000) < 60000)) {
        pResults->fltMask = LT_MSKT_TEST_PASSED;
    }

    LT_DOUT(LT_DBG_INFO, ("iM1est = %li nA, iM0est = %li nA, vStepC = %li mV,", iM1est, iM0est, vStepC));
    LT_DOUT(LT_DBG_INFO, ("decay = %li/1000, rSigCest = %li Ohms", decay, rSigCest));

    return;
}

#endif /* LT_MSOCKET_886 */
