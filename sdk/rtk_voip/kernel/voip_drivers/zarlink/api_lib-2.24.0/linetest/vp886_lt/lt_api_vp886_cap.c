/** file lt_api_vp886_cap.c
 *
 *  This file contains the VP886 capacitance test routine.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10371 $
 * $LastChangedDate: 2012-08-07 16:18:56 -0500 (Tue, 07 Aug 2012) $
 */
#include "lt_api.h"

#ifdef LT_CAP_886

#include "lt_api_internal.h"

#define LT_CAP_THRESHOLD    50000   /* 50nF */
#define LT_CAP_LOW_VOLTAGE  600     /* 600mV */

typedef enum {
    CAP_INIT            = LT_GLOBAL_FIRST_STATE,
    CAP_MEASURE_TIP,
    CAP_GET_RESULTS_TIP,
    CAP_MEASURE_RING,
    CAP_GET_RESULTS_RING,
    CAP_CALCULTATE,
    CAP_CONCLUDE,
    CAP_COMPLETE,
    CAP_ABORT           = LT_GLOBAL_ABORTED_STATE,
    CAP_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886CapStates;


static bool
CapHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

static void
CalculateResults(
    LtTestCtxType *pTestCtx);

static bool
IsCapBelowThreshold(
    LtTestCtxType *pTestCtx,
    int32 threshold);

extern bool 
LtVp886CapAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtCapInputType *ppInputs = (LtCapInputType*)pTemp886->attributes.pInputs;
    LtVp886CapTempType *pTempCap = &pTemp886->tempData.cap;

    /* If the inputs are not defined then save defaults into temp structure */
    if (VP_NULL == ppInputs) {
        pTempCap->input.testAmp = LT_CAP_DFLT_TEST_AMPL;
        pTempCap->input.testFreq = LT_CAP_DFLT_TEST_FREQ;
        pTempCap->input.tipCapCalValue = LT_CAP_DFLT_TIP_CAL;
        pTempCap->input.ringCapCalValue = LT_CAP_DFLT_RING_CAL;
    } else {
        /* Check for valid test freqs */
        if (ppInputs->testFreq >= LT_CAP_FREQ_ENUM_SIZE) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886CapAttributesCheck(testFreq:%i) : "
                "invalid", ppInputs->testFreq));
            return FALSE;
        }

        /* check validity of input test amplitude */
        if ((ppInputs->testAmp < LT_VP886_CAP_MIN_AMP) ||
            (ppInputs->testAmp > LT_VP886_CAP_MAX_AMP)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886CapAttributesCheck(testAmp:%li) : "
                "invalid", ppInputs->testAmp));
            return FALSE;
        }

        /* check validity of input tip->ground calibration value */
        if ((ppInputs->tipCapCalValue != LT_CAP_MANUFACTURING_CAL) &&
            ( (ppInputs->tipCapCalValue < LT_VP886_CAP_MIN_CAL) ||
              (ppInputs->tipCapCalValue > LT_VP886_CAP_MAX_CAL)) ) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886CapAttributesCheck(tipCapCalValue:%li) : "
                "invalid", ppInputs->tipCapCalValue));
            return FALSE;
        }

        /* check validity of input ring->ground calibration value */
        if ((ppInputs->tipCapCalValue != LT_CAP_MANUFACTURING_CAL) &&
            ( (ppInputs->ringCapCalValue < LT_VP886_CAP_MIN_CAL) ||
              (ppInputs->ringCapCalValue > LT_VP886_CAP_MAX_CAL)) ) {
            LT_DOUT(LT_DBG_ERRR, ("Vp886CapAttributesCheck(ringCapCalValue:%li) : "
                "invalid", ppInputs->ringCapCalValue));
            return FALSE;
        }

        /* store inputs */
        pTempCap->input.testAmp = ppInputs->testAmp;
        pTempCap->input.testFreq = ppInputs->testFreq;
        pTempCap->input.tipCapCalValue = ppInputs->tipCapCalValue;
        pTempCap->input.ringCapCalValue = ppInputs->ringCapCalValue;
    }

    /* setup function Handler */
    pTestCtx->Lt886TestEventHandlerFunc = CapHandlerSM;
    return TRUE;
}

static bool
CapHandlerSM(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886CapTempType *pTempCap = &pTemp886->tempData.cap;
    LtCapResultType *pResults = &pTestCtx->pResult->result.cap;
    LtCapTestFreqType *pTestFreq = &pTempCap->input.testFreq;
    LtVoltageType *pTestAmp = &pTempCap->input.testAmp;

    bool requestNextState = FALSE;
    *pRetval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("CapHandlerSM(testState:%i)", pTestCtx->state));

    switch (pTestCtx->state) {
        case CAP_INIT:
            LtInitOutputStruct(pTestCtx->pResult, LT_TID_CAP);
            pTempCap->ctg = LT_IMPEDANCE_NOT_MEASURED;
            pTempCap->crg = LT_IMPEDANCE_NOT_MEASURED;
            pTempCap->ctr = LT_IMPEDANCE_NOT_MEASURED;

            pTestCtx->state = CAP_MEASURE_TIP;
            requestNextState = TRUE;
            break;

        case CAP_MEASURE_TIP: {
            VpTest3EleCapAltResType capInput;

            capInput.calMode = FALSE;
            capInput.testFreq = (uint8)(*pTestFreq);
            capInput.testAmp = (uint16)(*pTestAmp);
            capInput.bias = -2100;  /* -10V */

            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_CAP_CSLAC, &capInput);

            pTestCtx->state = CAP_GET_RESULTS_TIP;
            break;
        }

        case CAP_GET_RESULTS_TIP:
            /* store all api cap results into the temp structure */
            if (pTestPrimRslts->result.capAlt.accuracyFlag == TRUE) {
                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            LtMemCpy(&pTempCap->vpApiCapResults1, &pTestPrimRslts->result.capAlt,
                sizeof(VpTestResultAltCapType));

            pTestCtx->state = CAP_MEASURE_RING;
            requestNextState = TRUE;
            break;

        case CAP_MEASURE_RING: {
            VpTest3EleCapAltResType capInput;

            capInput.calMode = FALSE;
            capInput.testFreq = (uint8)(*pTestFreq);
            capInput.testAmp = (uint16)(*pTestAmp);
            capInput.bias = 2100;  /* 10V */

            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_3ELE_CAP_CSLAC, &capInput);

            pTestCtx->state = CAP_GET_RESULTS_RING;
            break;
        }

        case CAP_GET_RESULTS_RING:
            /* store all api cap results into the temp structure */
            if (pTestPrimRslts->result.capAlt.accuracyFlag == TRUE) {
                pResults->measStatus = LT_MSRMNT_STATUS_DEGRADED_ACCURACY;
            }
            LtMemCpy(&pTempCap->vpApiCapResults2, &pTestPrimRslts->result.capAlt,
                sizeof(VpTestResultAltCapType));

            pTestCtx->state = CAP_CALCULTATE;
            requestNextState = TRUE;
            break;

        case CAP_CALCULTATE:
            /* Calculate results */
            CalculateResults(pTestCtx);
            if ((pResults->measStatus == LT_MSRMNT_STATUS_DEGRADED_ACCURACY) &&
                (*pTestAmp > LT_CAP_LOW_VOLTAGE))
            {
                *pTestAmp = LT_CAP_LOW_VOLTAGE;

                pResults->measStatus = LT_MSRMNT_STATUS_PASSED;
                pTestPrimRslts->result.capAlt.accuracyFlag = FALSE;
                pResults->ctg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->crg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->ctr = LT_IMPEDANCE_NOT_MEASURED;

                pTestCtx->state = CAP_MEASURE_TIP;
            } else if (TRUE == IsCapBelowThreshold(pTestCtx, (int32)LT_CAP_THRESHOLD)) {
                /* Run again at a higher frequency for better accuracy with low
                   capacitances. */
                *pTestFreq = LT_CAP_1333_HZ;

                pResults->ctg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->crg = LT_IMPEDANCE_NOT_MEASURED;
                pResults->ctr = LT_IMPEDANCE_NOT_MEASURED;

                pTestCtx->state = CAP_MEASURE_TIP;
            } else {
                /* Update the result structure with results that were already
                   valid in the first pass */
                if (pTempCap->ctg != LT_IMPEDANCE_NOT_MEASURED) {
                    LT_DOUT(LT_DBG_INFO, ("Update ctg = %li", pTempCap->ctg));
                    pResults->ctg = pTempCap->ctg;
                }
                if (pTempCap->crg != LT_IMPEDANCE_NOT_MEASURED) {
                    LT_DOUT(LT_DBG_INFO, ("Update crg = %li", pTempCap->crg));
                    pResults->crg = pTempCap->crg;
                }
                if (pTempCap->ctr != LT_IMPEDANCE_NOT_MEASURED) {
                    LT_DOUT(LT_DBG_INFO, ("Update ctr = %li", pTempCap->ctr));
                    pResults->ctr = pTempCap->ctr;
                }
                pTestCtx->state = CAP_CONCLUDE;
            }
            requestNextState = TRUE;
            break;

        /* Start the conclude primitive if this test not a part of an internal test */
        case CAP_CONCLUDE: {
            VpTestConcludeType conclude = { FALSE };
            *pRetval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &conclude);
            pTestCtx->state = CAP_COMPLETE;
            break;
        }

        case CAP_COMPLETE:
            /* End the test */
            *pRetval = LT_STATUS_DONE;
            break;

        case CAP_ABORT:
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
    LtVp886CapTempType *pTempCap = &pTemp886->tempData.cap;
    LtCapResultType *pResults = &pTestCtx->pResult->result.cap;
    LtCapacitanceType tipCapCalValue = pTempCap->input.tipCapCalValue;
    LtCapacitanceType ringCapCalValue = pTempCap->input.ringCapCalValue;
    uint32 cag, ca, cbg, cb;
    uint32 mConst, lConst;
    int32 cab;

    /* Check for a DC fault */
    if ((pTempCap->vpApiCapResults1.va == 0) || (pTempCap->vpApiCapResults2.vb == 0)) {
        pResults->ctg = LT_IMPEDANCE_NOT_MEASURED;
        pResults->crg = LT_IMPEDANCE_NOT_MEASURED;
        pResults->ctr = LT_IMPEDANCE_NOT_MEASURED;
        return;
    }

    /* Folding a bit of math into two constants, mConst and lConst.
       Tcl script to generate these numbers:
        set TestFreq 280.151                            ; # Test frequency in Hz
        set IIMmx 119.06e-3                             ; # Max ringing mode IM (amp)
        set IILmx 119.06e-3                             ; # Max measurable total IL (amp)
        set VDCmx 239.3                                 ; # Max measurable line voltage (V)
        set CAPstep 10e-12                              ; # Capacitance step in output (F)
        set pi 3.14159265358793                         ; # Pi
        set tau [expr 2*$pi*$TestFreq*$CAPstep]         ; # Radian frequency
        set Mconst [expr round(2*($IIMmx/$VDCmx)/$tau)] ; # Calculated met.  cap. constant
        set Lconst [expr round(  ($IILmx/$VDCmx)/$tau)] ; # Calculated long. cap. constant
    */
    switch (pTempCap->vpApiCapResults1.freq) {
        case 0:
            /* TestFreq = 2666.741Hz */
            mConst = 5939;
            lConst = 2969;
            break;
        case 1:
            /* TestFreq = 1333.371Hz */
            mConst = 11877;
            lConst = 5939;
            break;
        case 2:
            /* TestFreq = 666.502Hz */
            mConst = 23761;
            lConst = 11881;
            break;
        case 3:
            /* TestFreq = 333.251Hz */
            mConst = 47523;
            lConst = 23761;
            break;
        case 4:
            /* TestFreq = 280.151Hz */
            mConst = 56530;
            lConst = 28265;
            break;
        default:
            return;
    }

    cag = (lConst * (uint32)(pTempCap->vpApiCapResults1.il)) /
        (uint32)(pTempCap->vpApiCapResults1.va);
    ca = (mConst * (uint32)(pTempCap->vpApiCapResults1.im)) /
        (uint32)(pTempCap->vpApiCapResults1.va);
    cbg = (lConst * (uint32)(pTempCap->vpApiCapResults2.il)) /
        (uint32)(pTempCap->vpApiCapResults2.vb);
    cb = (mConst * (uint32)(pTempCap->vpApiCapResults2.im)) /
        (uint32)(pTempCap->vpApiCapResults2.vb);

    if (cag < cbg) {
        cab = ((int32)ca - (int32)cag) / 2;
    } else {
        cab = ((int32)cb - (int32)cbg) / 2;
    }

    LT_DOUT(LT_DBG_INFO, ("va1 = %d, vb1 = %d, im1 = %d, il1 = %d", pTempCap->vpApiCapResults1.va,
        pTempCap->vpApiCapResults1.vb, pTempCap->vpApiCapResults1.im,
        pTempCap->vpApiCapResults1.il));
    LT_DOUT(LT_DBG_INFO, ("va2 = %d, vb2 = %d, im2 = %d, il2 = %d", pTempCap->vpApiCapResults2.va,
        pTempCap->vpApiCapResults2.vb, pTempCap->vpApiCapResults2.im,
        pTempCap->vpApiCapResults2.il));
    LT_DOUT(LT_DBG_INFO, ("cag = %li, ca = %li, cbg = %li, cb = %li, cab = %li",
        cag, ca, cbg, cb, cab));
    LT_DOUT(LT_DBG_INFO, ("testAmp = %li, freq = 0x%02X, mConst = %li, lConst = %li",
        pTempCap->input.testAmp, pTempCap->vpApiCapResults1.freq, mConst, lConst));

    if (cab < 0) {
        cab = 0;
    }

    /* with -1 as a user input (default), use the pre-calibrated values (from device object
        calibration structure) */
    if (tipCapCalValue == LT_CAP_MANUFACTURING_CAL) {
        tipCapCalValue = pTempCap->vpApiCapResults1.tipCapCal;
    }
    if (ringCapCalValue == LT_CAP_MANUFACTURING_CAL) {
        ringCapCalValue = pTempCap->vpApiCapResults1.ringCapCal;
    }
    LT_DOUT(LT_DBG_INFO, ("tipCapCalValue = %li, ringCapCalValue = %li",
        tipCapCalValue, ringCapCalValue));

    /* limit the returned value to 1.2uF */
    pResults->fltMask = LT_TEST_PASSED;
    if ((cag * 10) > 1200000) {
        pResults->ctg = LT_MAX_CAPACITANCE;
    } else {
        pResults->ctg = (cag * 10); /* return value in picofarads */
        pResults->ctg -= tipCapCalValue; /* apply the calibration */
        if (pResults->ctg < 0) {
            pResults->ctg = 0;
        }
    }
    if ((cbg * 10) > 1200000) {
        pResults->crg = LT_MAX_CAPACITANCE;
    } else {
        pResults->crg = (cbg * 10); /* return value in picofarads */
        pResults->crg -= ringCapCalValue; /* apply the calibration */
        if (pResults->crg < 0) {
            pResults->crg = 0;
        }
    }
    if ((cab * 10) > 1200000) {
        pResults->ctr = LT_MAX_CAPACITANCE;
    } else {
        pResults->ctr = (cab * 10); /* return value in picofarads */
    }

    LT_DOUT(LT_DBG_INFO, ("Results: ctg = %li, crg = %li, ctr = %li",
        pResults->ctg, pResults->crg, pResults->ctr));

    return;
}

static bool
IsCapBelowThreshold(
    LtTestCtxType *pTestCtx,
    int32 threshold)
{
    LtCapResultType *pResults = &pTestCtx->pResult->result.cap;
    LtVp886TestTempType *pTemp886 = &pTestCtx->pTemp->vp886Temp;
    LtVp886CapTempType *pTempCap = &pTemp886->tempData.cap;
    bool retval = FALSE;

    /* we do not need to re-run if the test frequency is already high [0,1] */
    if (pTempCap->input.testFreq <= 1) {
        return FALSE;
    }

    /* save the acceptable values before the potential re-run */
    if (pResults->ctg < threshold) {
        retval = TRUE;
    } else {
        pTempCap->ctg = pResults->ctg;
    }
    if (pResults->crg < threshold) {
        retval = TRUE;
    } else {
        pTempCap->crg = pResults->crg;
    }
    if (pResults->ctr < threshold) {
        retval = TRUE;
    } else {
        pTempCap->ctr = pResults->ctr;
    }

    return retval;
}

#endif /* LT_CAP_886 */
