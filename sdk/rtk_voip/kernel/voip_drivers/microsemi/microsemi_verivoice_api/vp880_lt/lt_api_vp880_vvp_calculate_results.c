/** file vp_test_lib_calculate_results.c
 *
 *  This file contains the test limit calculations
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 11500 $
 * $LastChangedDate: 2014-07-18 11:28:36 -0500 (Fri, 18 Jul 2014) $
 */

#include "lt_api.h"

#if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE) && defined(LT_VP880_VVP_PACKAGE)
#ifdef LT_VP880_VVP_PACKAGE

#include "lt_api_internal.h"
#include "lt_api_vp880.h"

#ifdef LT_LINE_V
static void Vp880LineVCalResult(LtTestCtxType *pTestCtx);
static bool Vp880LineVAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_LINE_V */

#ifdef LT_ROH
static void Vp880RohCalResult(LtTestCtxType *pTestCtx);
static bool Vp880RohAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_ROH */

#ifdef LT_RINGERS
static void Vp880RingerCalResult(LtTestCtxType *pTestCtx);
static bool Vp880RingerAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_RINGERS */

#ifdef LT_RES_FLT
static void Vp880ResFltLGCalResult(LtTestCtxType *pTestCtx);
static void Vp880ResFltHGCalResult(LtTestCtxType *pTestCtx);
static bool Vp880ResFltHGLongitudinalShortFound(LtTestCtxType *pTestCtx);
static bool Vp880ResFltAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_RES_FLT */

#ifdef LT_MSOCKET
static void Vp880MSocketCalResultType1(LtTestCtxType *pTestCtx);
static void Vp880MSocketCalResultType2(LtTestCtxType *pTestCtx);
static bool Vp880MSocketAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_MSOCKET */

#ifdef LT_XCONNECT
static void Vp880XConnectCalResult(LtTestCtxType *pTestCtx);
static bool Vp880XConnectAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_XCONNECT */

#ifdef LT_CAP
static void Vp880CapCalResult(LtTestCtxType *pTestCtx);
static bool Vp880CapAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_CAP */

#if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)
static bool Vp880909AllAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_ALL_GR_909 */

#ifdef LT_LOOPBACK
static bool Vp880LoopbackAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_LOOPBACK */

#ifdef LT_DC_FEED_ST
static void Vp880DcFeedCalResult(LtTestCtxType *pTestCtx);
static bool Vp880DcFeedAttributeCheck(LtTestCtxType *pTestCtx);
#endif /* LT_DC_FEED_ST */

#ifdef LT_RD_LOOP_COND
static void Vp880RdLoopCondCalResult(LtTestCtxType *pTestCtx);
static bool Vp880RdLoopCondAttributeCheck(LtTestCtxType *pTestCtx);
#endif /* LT_RD_LOOP_COND */

#ifdef LT_DC_VOLTAGE
static void Vp880DcVoltageCalResult(LtTestCtxType *pTestCtx);
static bool Vp880DcVoltageAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_DC_VOLTAGE */

#ifdef LT_RINGING_ST
static void Vp880RingingStCalResult(LtTestCtxType *pTestCtx);
static bool Vp880RingingStAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_RINGING_ST */

#ifdef LT_ON_OFF_HOOK_ST
static bool Vp880OnOffHookStAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_ON_OFF_HOOK_ST */

#ifdef LT_RD_BAT_COND
static void Vp880RdBatCondCalResult(LtTestCtxType *pTestCtx);
static bool Vp880RdBatCondAttributesCheck(LtTestCtxType *pTestCtx);
#endif /* LT_RD_BAT_COND */

#ifdef LT_FLT_DSCRM
static void Vp880FltDscrmResult(LtTestCtxType *pTestCtx);
#endif /* LT_FLT_DSCRM */


bool
LtVp880CalculateResults(
    LtTestCtxType *pTestCtx,
    LtTestIdType tid)
{
    bool result = FALSE;

    switch (tid) {
        #ifdef LT_LINE_V
        case LT_TID_LINE_V:
            Vp880LineVCalResult(pTestCtx);
            break;
        #endif /* LT_LINE_V */

        #ifdef LT_ROH
        case LT_TID_ROH:
            Vp880RohCalResult(pTestCtx);
            break;
        #endif /* LT_ROH */

        #ifdef LT_RINGERS
        case LT_TID_RINGERS:
            Vp880RingerCalResult(pTestCtx);
            break;
        #endif /* LT_RINGERS */

        #ifdef LT_RES_FLT
        case LT_TID_RES_FLT:
            if (TRUE == pTestCtx->pTemp->vp880Temp.tempData.resFlt.highGainMode) {
                if (FALSE == Vp880ResFltHGLongitudinalShortFound(pTestCtx)) {
                    Vp880ResFltHGCalResult(pTestCtx);
                }
            } else {
                Vp880ResFltLGCalResult(pTestCtx);
            }
            break;
        #endif /* LT_RES_FLT */

        #ifdef LT_MSOCKET
        case LT_TID_MSOCKET:
            switch (pTestCtx->pTemp->vp880Temp.tempData.mSocket.input.mSocket) {
                case LT_MSOCKET_TYPE_1:
                    LT_DOUT(LT_DBG_INFO, ("LT_MSOCKET_TYPE_1: call Vp880MSocketCalResultType1"));
                    Vp880MSocketCalResultType1(pTestCtx);
                    break;

                case LT_MSOCKET_TYPE_2:
                    LT_DOUT(LT_DBG_INFO, ("LT_MSOCKET_TYPE_2: call Vp880MSocketCalResultType2"));
                    Vp880MSocketCalResultType2(pTestCtx);
                    break;

                default:
                    LT_DOUT(LT_DBG_ERRR, ("Unknown master socket type"));
                    break;
            }
            break;
        #endif /* LT_MSOCKET */

        #ifdef LT_XCONNECT
        case LT_TID_XCONNECT:
            Vp880XConnectCalResult(pTestCtx);
            break;
        #endif /* LT_XCONNECT */

        #ifdef LT_CAP
        case LT_TID_CAP:
            Vp880CapCalResult(pTestCtx);
            break;
        #endif /* LT_CAP */

        #ifdef LT_DC_FEED_ST
        case LT_TID_DC_FEED_ST:
            Vp880DcFeedCalResult(pTestCtx);
            break;
        #endif /* LT_DC_FEED_ST */

        #ifdef LT_RD_LOOP_COND
        case LT_TID_RD_LOOP_COND:
            Vp880RdLoopCondCalResult(pTestCtx);
            break;
        #endif /* LT_RD_LOOP_COND */

        #ifdef LT_DC_VOLTAGE
        case LT_TID_DC_VOLTAGE:
            Vp880DcVoltageCalResult(pTestCtx);
            break;
        #endif /* LT_DC_VOLTAGE */

        #ifdef LT_RINGING_ST
        case LT_TID_RINGING_ST:
            Vp880RingingStCalResult(pTestCtx);
            break;
        #endif /* LT_RINGING_ST */

        #ifdef LT_RD_BAT_COND
        case LT_TID_RD_BAT_COND:
            Vp880RdBatCondCalResult(pTestCtx);
            break;
        #endif /* LT_RD_BAT_COND */

        #ifdef LT_FLT_DSCRM
        case LT_TID_FLT_DSCRM:
            Vp880FltDscrmResult(pTestCtx);
            break;
        #endif /* LT_FLT_DSCRM */

        default:
            LT_DOUT(LT_DBG_FUNC, ("LtVp880CalculateResults(ltTestId:%i) : invalid",tid));

            result = FALSE;
            break;
    }
    LT_DOUT(LT_DBG_FUNC, ("LtVp880CalculateResults(ltTestId:%i)",tid));
    return result;
}

#ifdef LT_LINE_V
static void
Vp880LineVCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880LineVTempType *pTempLineV = &pTemp->tempData.lineV;
    LtLineVResultType *pResults = &pTestCtx->pResult->result.lineV;
    LtVoltageType dcHemfMax = ABS(pTempLineV->criteria.dcHemf);
    LtVoltageType acHemfMax = ABS(pTempLineV->criteria.acHemf);
    LtVoltageType dcFemfMax = ABS(pTempLineV->criteria.dcFemf);
    LtVoltageType acFemfMax = ABS(pTempLineV->criteria.acFemf);

    pResults->vDcTip = VP880_UNIT_CONVERT(pTempLineV->vpApiTipResults.vdc,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    pResults->vAcTip = VP880_UNIT_CONVERT(-pTempLineV->vpApiTipResults.vac,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    pResults->vDcRing = VP880_UNIT_CONVERT(pTempLineV->vpApiRingResults.vdc,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    pResults->vAcRing = VP880_UNIT_CONVERT(-pTempLineV->vpApiRingResults.vac,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    if ((pTemp->revCode >= 3) || 
        (pTemp->productCode == 0xFF) ||
        (pTemp->productCode == 0xF3)) {
        pResults->vDcDiff = VP880_UNIT_CONVERT(pTempLineV->vpApiDiffResults.vdc,
            VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    } else {
        pResults->vDcDiff = pResults->vDcTip - pResults->vDcRing;
    }

    pResults->vAcDiff = VP880_UNIT_CONVERT(-pTempLineV->vpApiDiffResults.vac,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    pResults->vAcTip = LtLineVAcDecompression(pResults->vAcTip);
    pResults->vAcRing = LtLineVAcDecompression(pResults->vAcRing);
    pResults->vAcDiff = LtLineVAcDecompression(pResults->vAcDiff);

    pResults->fltMask = LT_TEST_PASSED;

    if ( ABS(pResults->vDcTip) > dcHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_DC_TIP;
    }
    if ( ABS(pResults->vDcRing) > dcHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_DC_RING;
    }
    if ( ABS(pResults->vDcDiff) > dcHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_DC_DIFF;
    }

    if (ABS(pResults->vAcTip) > acHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_AC_TIP;
    }
    if (ABS(pResults->vAcRing) > acHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_AC_RING;
    }
    if (ABS(pResults->vAcDiff) > acHemfMax) {
        pResults->fltMask |= LT_LVM_HEMF_AC_DIFF;
    }

    if ( ABS(pResults->vDcTip) > dcFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_DC_TIP;
    }
    if ( ABS(pResults->vDcRing) > dcFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_DC_RING;
    }
    if ( ABS(pResults->vDcDiff) > dcFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_DC_DIFF;
    }

    if (ABS(pResults->vAcTip) > acFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_AC_TIP;
    }
    if (ABS(pResults->vAcRing) > acFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_AC_RING;
    }
    if (ABS(pResults->vAcDiff) > acFemfMax) {
        pResults->fltMask |= LT_LVM_FEMF_AC_DIFF;
    }

    return;
}
#endif /* LT_LINE_V */

#ifdef LT_ROH
static void
Vp880RohCalResult(
    LtTestCtxType *pTestCtx)
{
    LtRohResultType *pRoh = &pTestCtx->pResult->result.roh;
    LtVp880RohTempType *pTempRoh = &pTestCtx->pTemp->vp880Temp.tempData.roh;
    int32 percentErrorLimit = 15;
    int32 percentError = 0;

    const int32 sFactor = ((VP880_UNIT_ADC_VAB * 10000L) /VP880_UNIT_MV);

    const int32 res1 = ((pTempRoh->vab2 - pTempRoh->vab1) * sFactor) /
        (pTempRoh->imt2 - pTempRoh->imt1);

    const int32 res2 = ((pTempRoh->vab3 - pTempRoh->vab2) * sFactor) /
        (pTempRoh->imt3 - pTempRoh->imt2);

    if (0 != res2) {
        percentError = (((res2 - res1) * 10000) / res2);
    }
    if ((ABS(res2) < 500) && (ABS(res1) < 500)) {
        /* values are so small it cannot be a phone off-hook */
        pRoh->fltMask = LT_ROHM_RES_LOOP;
    } else if (ABS(percentError) > (percentErrorLimit * 100)) {
        pRoh->fltMask = LT_ROHM_OFF_HOOK;
    } else {
        pRoh->fltMask = LT_ROHM_RES_LOOP;
    }

    pRoh->rLoop1 = ABS(res1);
    pRoh->rLoop2 = ABS(res2);
    return;
}
#endif /* LT_ROH */

#ifdef LT_RINGERS
static void
Vp880RingerCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880RingersTempType *pTempRingers = &pTemp->tempData.ringers;
    LtRingersResultType *pResults = &pTestCtx->pResult->result.ringers;
    const int16 maxRes = pTempRingers->criteria.renHigh;
    const int16 minRes = pTempRingers->criteria.renLow;

    if ((LT_RINGER_REGULAR_PHNE_TEST == pTempRingers->input.ringerTestType) ||
        (LT_RINGER_REGULAR_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType)) {
        int32 imt, vab;

        const int32 renFactor = pTempRingers->input.renFactor / 10;
        const int32 freq = pTempRingers->input.freq / 1000;

        int32 cFactA, cFactB;
        int32 senseA = (int32)(pTemp->attributes.topology.rSenseA / (10 * 1000));
        int32 senseB = (int32)(pTemp->attributes.topology.rSenseB / (10 * 1000));
        int32 leakA = (int32)(pTemp->attributes.topology.rLeakageA / (10 * 1000));
        int32 leakB = (int32)(pTemp->attributes.topology.rLeakageB / (10 * 1000));

        int32 capImp = 1768389 / (freq * 1000); /* 68nF || 22nF to ground impedance 1 / (90e-9 * 2pi * freq)*/

        /* Find impedance of sense resistors and leakage resistors */
        cFactA = (leakA) ? (senseA * leakA) / (senseA + leakA) : senseA;
        cFactB = (leakB) ? (senseB * leakB) / (senseB + leakB) : senseB;

        /* Factor in the cap's impedance */
        cFactA = (cFactA * capImp) / (cFactA + capImp);
        cFactB = (cFactB * capImp) / (cFactB + capImp);

        /* convert to mREN and account for offsets */
        cFactA = (renFactor / cFactA);
        cFactB = (renFactor / cFactB);

        /*
         * The AC_RLOOP primitive uses the balanced ringing SLIC state for
         * creating the differential AC signal and the unbalanced ringing
         * SLIC state for creating the longitudinal AC signals.
         *
         * The differential calculation uses a full scale REN value that
         * consists of the full scale metallic current in ringing, divided by the
         * full scale voltage in ringing. Unfortunately the full scale metallic
         * current is slightly different between revisions.
         *
         * The AC_RLOOP primitive sets the LI bit in the DC Feed register
         * to 0. This means that the ADC value for ilg has a full scale
         * value of 21mA. However, the longitudinal measurement is taken
         * with the AX bit set to one, effectively doubling the full-scale
         * range to 42mA. So the full scale longitudinal REN value is
         * full scale longitudinal current, multiplied by 2 (AX=1), divided by
         * the full scale voltage.
         */

        /* calculate tip to ring ren */
        imt = pTempRingers->vpApiAcR1Results1.imt;
        vab = pTempRingers->vpApiAcR1Results1.vab;

        /* Due to an offset in JA silicon the renFactor is adjusted differently */
        if (vab == 0) {
            pResults->ren = LT_REN_NOT_MEASURED;
            pResults->renrg = LT_REN_NOT_MEASURED;
            pResults->rentg = LT_REN_NOT_MEASURED;
            return;
        } else if ((pTemp->revCode >= 3) || 
                   (pTemp->productCode == 0xFF) ||
                   (pTemp->productCode == 0xF3)) {
            /* 115.4 / 240.0 = full scale*/
            pResults->ren = (( ((renFactor * 1154) / 2400) * imt) / vab);
        } else {
            /* 119.0 / 240.0 = full scale */
            pResults->ren = (( ((renFactor * 1190) / 2400) * imt) / vab);
        }

        LT_DOUT(LT_DBG_INFO, ("renFactor: %li", renFactor));
        LT_DOUT(LT_DBG_INFO, ("cFactA: %li leakA %li, senseA %li, capImp %li", cFactA, leakA, senseA, capImp));
        LT_DOUT(LT_DBG_INFO, ("cFactB: %li leakB %li, senseB %li, capImp %li", cFactB, leakB, senseB, capImp));
        LT_DOUT(LT_DBG_INFO, ("renAB before correction: %li", pResults->ren));
        LT_DOUT(LT_DBG_INFO, ("imt: %li", imt));
        LT_DOUT(LT_DBG_INFO, ("vab: %li", vab));

        if (LT_RINGER_REGULAR_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType) {
            /* calculate tip to ground ren */
            int32  rgRen, tgRen, trRen, ilg, rgoff, tgoff, troff;

            /* calculate tip to ground ren */
            ilg = pTempRingers->vpApiAcR1Results3.ilg;
            vab = pTempRingers->vpApiAcR1Results3.vab;
            tgRen = (( ((renFactor * 420) / 2400) * ilg) / vab);
            pResults->rentg = tgRen - cFactA;

            /* calculate ring to ground ren */
            ilg = pTempRingers->vpApiAcR1Results2.ilg;
            vab = pTempRingers->vpApiAcR1Results2.vab;
            rgRen = (( ((renFactor * 420) / 2400) * ilg) / vab);
            pResults->renrg = rgRen - cFactB;

            /* remove unmodified ring/gnd and tip/gnd result from tip/ring result */
            pResults->ren -= ((rgRen + tgRen) / 4);


            LT_DOUT(LT_DBG_INFO, ("renA before correction: %li", tgRen));
            LT_DOUT(LT_DBG_INFO, ("ilg: %i", pTempRingers->vpApiAcR1Results3.ilg));
            LT_DOUT(LT_DBG_INFO, ("vab: %i", pTempRingers->vpApiAcR1Results3.vab));

            LT_DOUT(LT_DBG_INFO, ("renB before correction: %li", rgRen));
            LT_DOUT(LT_DBG_INFO, ("ilg: %i", pTempRingers->vpApiAcR1Results2.ilg));
            LT_DOUT(LT_DBG_INFO, ("vab: %i", pTempRingers->vpApiAcR1Results2.vab));

            /*
             * Based on empirical validation data, all leads need offset
             * and feedthrough corrections.
             */
            if ((pTemp->revCode >= 3) || 
                (pTemp->productCode == 0xFF) ||
                (pTemp->productCode == 0xF3)) {

                tgoff = -4;
                rgoff = +15;
                troff = -16;
                LT_DOUT(LT_DBG_INFO, ("renA : %li", pResults->rentg));
                LT_DOUT(LT_DBG_INFO, ("renB : %li", pResults->renrg));
                LT_DOUT(LT_DBG_INFO, ("renAB: %li", pResults->ren));

                /* feedthrough adjustments based on validation results */
                tgRen = pResults->rentg - (pResults->renrg * 0)   - (pResults->ren   / 270);
                rgRen = pResults->renrg - (pResults->rentg / 400) - (pResults->ren   / 200);
                trRen = pResults->ren   - (pResults->rentg / 200) - (pResults->renrg / 200);

            } else {
                rgoff = tgoff = troff = 0;
                tgRen = pResults->rentg;
                rgRen = pResults->renrg;
                trRen = pResults->ren;

            }

            /* offset correction */
            pResults->rentg = tgRen + tgoff;
            pResults->renrg = rgRen + rgoff;
            pResults->ren   = trRen + troff;

        } else {
            pResults->ren -= (cFactA + cFactB) / 4;
        }
    } else {
        int32 i1, i2, slope1, slope2, iavg, slope, cap, kCapIm, kCapIl;
        int32 renFactor = ABS(pTempRingers->input.renFactor / 1000);
        int32 fccCap_nF = 1210;

        /*
         * The METALIC_RAMP primitive sets the LI bit  in the DC Feed register
         * to 0. This means that the ADC value for ilg has a full scale value
         * of 21mA. However, since we are doing all calculations in fixed
         * point, we double the value to 42mA and do not divide the average
         * current by 2. This prevents us from truncating some of the smaller
         * measured ilg currents and gives us the room to calculate the final
         * value in milliREN. When we multiply the calculated REN Scale
         * factor by the average current it all works out.
         *
         * Similarly, the METALIC_RAMP sets a back door bit that forces the
         * DAC to use DCFeed Output range. The measured result will be
         * half of the full-scale value. Rather than dividing the
         * measured results by 2 and loosing some of the low order bits
         * we put the factor of 2 in the metallic REN scale factor where
         * there is plenty of room.
         *
         * Lastly, both sets of REN Scale factors have been blown out
         * by 1,000,000 and 1000 to give us even more room to do the
         * fixed point math. The factor of 1000 is present in order
         * to report the final value in mREN. The second factor of 1,000,000
         * is present to compensate for what was left of the input REN factors units.
         * The REN factor is in units of picoF we cut it down to nanoF and now down
         * to mF.
         */

        /* Longitudinal REN Scale Factor (1000s * 42mA * 1,000,000 * 1000) / (240V) */
        kCapIl = 175000000;
        if ((pTemp->revCode >= 3) || 
            (pTemp->productCode == 0xFF) ||
            (pTemp->productCode == 0xF3)) {
            /* Metallic REN Scale Factor (1000s * (115.4mA / 2) * 1,000,000 * 1000) / (240V) */
            kCapIm = 240416667;
        } else {
            /* Metallic REN Scale Factor (1000s * (119.0mA / 2) * 1,000,000 * 1000) / (240V) */
            kCapIm = 247916667;
        }

        if (pTempRingers->inputVADC <= 0) {
            kCapIm *= -1;
            kCapIl *= -1;
        }

        /* calculate tip to ring REN */
        i1      = (int32)pTempRingers->vpApiDiffRamp1.imt;
        slope1  = (int32)pTempRingers->vpApiDiffRamp1.vRate;
        i2      = (int32)pTempRingers->vpApiDiffRamp2.imt;
        slope2  = (int32)pTempRingers->vpApiDiffRamp2.vRate;
        iavg    = ABS(i2 - i1);
        slope   = 10 * (slope1 - slope2);
        cap     = ( ((-kCapIm / slope) * iavg) / renFactor);
        pResults->ren = cap/10;

        LT_DOUT(LT_DBG_INFO, ("renFactor: %li", renFactor));
        LT_DOUT(LT_DBG_INFO, ("inputVADC: %i", pTempRingers->inputVADC));
        LT_DOUT(LT_DBG_INFO, ("feedVADC: %i", pTempRingers->feedVADC));
        LT_DOUT(LT_DBG_INFO, ("kCapIm: %li", kCapIm));

        LT_DOUT(LT_DBG_INFO, ("imt1: %li", i1));
        LT_DOUT(LT_DBG_INFO, ("vRate1: %li", slope1));
        LT_DOUT(LT_DBG_INFO, ("imt2: %li", i2));
        LT_DOUT(LT_DBG_INFO, ("vRate2: %li", slope2));
        LT_DOUT(LT_DBG_INFO, ("ren: %li", pResults->ren));

        if (LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE == pTempRingers->input.ringerTestType) {

            int32 renrg, rentg, rentr, rgoff, tgoff, troff;

            if ((pTemp->revCode >= 3) || 
                (pTemp->productCode == 0xFF) ||
                (pTemp->productCode == 0xF3)) {
                rgoff = (75 * fccCap_nF) / renFactor;
                tgoff = (75 * fccCap_nF) / renFactor;
                troff = (40 * fccCap_nF) / renFactor;
/*                  rgoff = 0; tgoff = 0; troff = 0; */
            } else {
                rgoff = (100 * fccCap_nF) / renFactor;
                tgoff = (75 * fccCap_nF) / renFactor;
                troff = (40 * fccCap_nF) / renFactor;
                  /*rgoff = 0; tgoff = 0; troff = 0; */
            }

            /* calculate ring to gnd REN */
            i1      = (int32)pTempRingers->vpApiRingRamp1.imt;
            slope1  = (int32)pTempRingers->vpApiRingRamp1.vRate;
            i2      = (int32)pTempRingers->vpApiRingRamp2.imt;
            slope2  = (int32)pTempRingers->vpApiRingRamp2.vRate;
            iavg    = i2 - i1;
            slope   = 10 * (slope1 - slope2);
            renrg   = ( ((ABS(kCapIl) / slope) * iavg) / (renFactor * 10));

            pResults->renrg = renrg - rgoff;

            /* calculate tip to gnd REN */
            i1      = (int32)pTempRingers->vpApiTipRamp1.imt;
            slope1  = (int32)pTempRingers->vpApiTipRamp1.vRate;
            i2      = (int32)pTempRingers->vpApiTipRamp2.imt;
            slope2  = (int32)pTempRingers->vpApiTipRamp2.vRate;
            iavg    = i2 - i1;
            slope   = 10 * (slope1 - slope2);
            rentg   = (-1 * ((kCapIl / slope) * iavg) / (renFactor * 10));

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

            LT_DOUT(LT_DBG_INFO, ("renA before correction: %li", rentg));
            LT_DOUT(LT_DBG_INFO, ("renB before correction: %li", renrg));

            /*
             * Based on empirical validation data, all leads need offset
             * and feedthrough corrections.
             */
            if ((pTemp->revCode >= 3) || 
                (pTemp->productCode == 0xFF) ||
                (pTemp->productCode == 0xF3)) {
                tgoff = -6;
                rgoff = -10;
                troff = -1;
                LT_DOUT(LT_DBG_INFO, ("renA : %li", pResults->rentg));
                LT_DOUT(LT_DBG_INFO, ("renB : %li", pResults->renrg));
                LT_DOUT(LT_DBG_INFO, ("renAB: %li", pResults->ren));

                /* feedthrough adjustments based on validation results */
                rentg = pResults->rentg - (pResults->renrg *  0)   - (pResults->ren  / 500);
                renrg = pResults->renrg - (pResults->rentg / 100) - (pResults->ren   / 500);
                rentr = pResults->ren   - (pResults->rentg / 125) + (pResults->renrg / 303);

            } else {
                rgoff = tgoff = troff = 0;
                rentg = pResults->rentg;
                renrg = pResults->renrg;
                rentr = pResults->ren;
            }

            /* offset correction */
            pResults->rentg = rentg + tgoff;
            pResults->renrg = renrg + rgoff;
            pResults->ren   = rentr + troff;


        } else {
            /*
             * If the test was differential only then we need to subtract
             * the theoretical ren to ground that appears on each lead.
             * (22nF + 68nF) * 10000
            */
            int32 cFact = ((22 + 68) * 10000L) / (renFactor);
            pResults->ren -= (cFact / 20) + ((40 * fccCap_nF) / renFactor);
        }
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
#endif /* LT_RINGERS */

#ifdef LT_RES_FLT
static void
Vp880ResFltLGCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtResFltResultType *pResFlt = &pTestCtx->pResult->result.resFlt;
    LtVp880ResFltTempType *pTempResFlt = &pTestCtx->pTemp->vp880Temp.tempData.resFlt;
    LtImpedanceType senseA = (pTemp->attributes.topology.rSenseA / (10 * 1000));
    LtImpedanceType senseB = (pTemp->attributes.topology.rSenseB / (10 * 1000));
    LtImpedanceType leakA = (pTemp->attributes.topology.rLeakageA / (10 * 1000));
    LtImpedanceType leakB = (pTemp->attributes.topology.rLeakageB / (10 * 1000));

    int32 dMicM, dMicL, dLicL, numAB, numAG, numBG, denAB, denAG, denBG;
    LtImpedanceType worst;

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
    const int32 imzf = pTempResFlt->vpApiResFltResults.imzf;
    const int32 ilzf = pTempResFlt->vpApiResFltResults.ilzf;

    /* Calculate the output voltages from the chip */
    const int32 dMvcA =  varf - vanf;
    const int32 dMvcB =  vbrf - vbnf;
    const int32 dLvcA =  (-varf + 2 * vazf - vanf) >> 1;
    const int32 dLvcB =  (-vbrf + 2 * vbzf - vbnf) >> 1;

    /* adjust gdc value based on leakage resistance in the sense paths*/
    int32 gdc = 2120;
    int32 gdcA = gdc + ((gdc * senseA) / leakA);
    int32 gdcB = gdc + ((gdc * senseB) / leakB);

    /* debugging */
    LT_DOUT(LT_DBG_INFO, ("leakA: %li", leakA));
    LT_DOUT(LT_DBG_INFO, ("leakB: %li", leakB));

    LT_DOUT(LT_DBG_INFO, ("vanf: %li", vanf));
    LT_DOUT(LT_DBG_INFO, ("vbnf: %li", vbnf));
    LT_DOUT(LT_DBG_INFO, ("imnf: %li", imnf));
    LT_DOUT(LT_DBG_INFO, ("ilnf: %li", ilnf));

    LT_DOUT(LT_DBG_INFO, ("varf: %li", varf));
    LT_DOUT(LT_DBG_INFO, ("vbrf: %li", vbrf));
    LT_DOUT(LT_DBG_INFO, ("imrf: %li", imrf));
    LT_DOUT(LT_DBG_INFO, ("ilrf: %li", ilrf));

    LT_DOUT(LT_DBG_INFO, ("vazf: %li", vazf));
    LT_DOUT(LT_DBG_INFO, ("vbzf: %li", vbzf));
    LT_DOUT(LT_DBG_INFO, ("imzf: %li", imzf));
    LT_DOUT(LT_DBG_INFO, ("ilzf: %li", ilzf));

    /* If any of the measurements were saturated then go to high gain mode */
    if ((ABS(vanf) >= VP_INT16_MAX) || (ABS(vbnf) >= VP_INT16_MAX) ||
        (ABS(imnf) >= VP_INT16_MAX) || (ABS(ilnf) >= VP_INT16_MAX) ||
        (ABS(varf) >= VP_INT16_MAX) || (ABS(vbrf) >= VP_INT16_MAX) ||
        (ABS(imrf) >= VP_INT16_MAX) || (ABS(ilrf) >= VP_INT16_MAX) ||
        (ABS(vazf) >= VP_INT16_MAX) || (ABS(vbzf) >= VP_INT16_MAX) ||
        (ABS(imzf) >= VP_INT16_MAX) || (ABS(ilzf) >= VP_INT16_MAX)) {
        pTempResFlt->highGainMode = TRUE;
        return;
    }

    /* Calculate and ddjust the currents to get the ext. currents */
    dMicM = (2 * -717 * (imnf - imrf)) + gdcA * dMvcA - gdcB * dMvcB;
    dMicL = (2 * -2048 * (ilnf - ilrf)) - gdcA * dMvcA - gdcB * dMvcB;
    dLicL = (-2048 * (ilnf - 2 * ilzf + ilrf)) - gdcA * dLvcA - gdcB * dLvcB;

    /* Shift the current results into 16 bit words */
    dMicM = (dMicM + 8192) >> 14;
    dMicL = (dMicL + 8192) >> 14;
    dLicL = (dLicL + 8192) >> 14;

    /* calculate Tip to Ring Res to closest 10th of an Ohm */
    numAB = ((dLvcB*dMvcB - dLvcB*dMvcA + dLvcA*dMvcB - dLvcA*dMvcA + 8) >> 4) * 3125;
    denAB = ((dLvcB*dMicM + dLvcA*dMicM - dMvcB*dLicL + dMvcA*dLicL + 256) >> 9);
    denAB = (denAB/10 ? denAB : (denAB < 0 ? -10 : 10));
    pResFlt->rtr = 10 * (numAB/denAB);
    if ((pResFlt->rtr < 0) || (pResFlt->rtr > VVP_RES_FLT_OPEN_CIRCUIT)) {
        pResFlt->rtr = LT_IMPEDANCE_OPEN_CKT;
    }

    /* calculate Tip to Gnd Res to closest 10th of an Ohm */
    numAG = ((dLvcA*dMvcA + 2) >> 2) * 3125;
    denAG = ((dMvcA*dLicL + dLvcA*dMicL + 64) >> 7);
    denAG = (denAG/10 ? denAG : (denAG < 0 ? -10 : 10));
    pResFlt->rtg = 10 * (numAG/denAG);
    if ((pResFlt->rtg < 0) || (pResFlt->rtg > VVP_RES_FLT_OPEN_CIRCUIT)) {
        pResFlt->rtg = LT_IMPEDANCE_OPEN_CKT;
    }

    /* calculate Ring to Gnd Res to closest 10th of an Ohm */
    numBG = ((dLvcB*dMvcB + 2) >> 2) * 3125;
    denBG = ((dMvcB*dLicL + dLvcB*dMicL + 64) >> 7);
    denBG = (denBG/10 ? denBG : (denBG < 0 ? -10 : 10));
    pResFlt->rrg = 10 * (numBG/denBG);
    if ((pResFlt->rrg < 0) || (pResFlt->rrg > VVP_RES_FLT_OPEN_CIRCUIT)) {
        pResFlt->rrg = LT_IMPEDANCE_OPEN_CKT;
    }

    /* If longitudinal shift was not adequate go to high gain mode */
    if (ABS((dLvcA + dLvcB)) < 410) {
        pTempResFlt->highGainMode = TRUE;
        return;
    }

    /* Find worst data point and determine if we need high gain mode */
    pResFlt->fltMask = LT_TEST_PASSED;
    worst = pTempResFlt->criteria.resFltLowLimit;
    if ((pResFlt->rtr < worst) ||
        (pResFlt->rtg < worst) ||
        (pResFlt->rrg < worst)) {

        LtImpedanceType rtr = pResFlt->rtr;
        LtImpedanceType rtg = pResFlt->rtg;
        LtImpedanceType rrg = pResFlt->rrg;

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
    }
    return;
}

static bool
Vp880ResFltHGLongitudinalShortFound(
    LtTestCtxType *pTestCtx)
{
    LtResFltResultType *pResFlt = &pTestCtx->pResult->result.resFlt;
    LtVp880ResFltTempType *pTempResFlt =
        &pTestCtx->pTemp->vp880Temp.tempData.resFlt;
    const int16 ilzf = pTempResFlt->vpApiResFltResults.ilzf;
    const int16 ilnf = pTempResFlt->vpApiResFltResults.ilnf;
    const int16 ilrf = pTempResFlt->vpApiResFltResults.ilrf;
    int16 worstILG = 0;
    bool shortFnd = FALSE;

    int16 hgMinIlg = (int16)VP880_UNIT_CONVERT(
            LT_VP880_RES_FLT_HG_MIN_ILG, VP880_UNIT_UA, VP880_UNIT_ADC_ILG);

    int16 hgMaxIlg = (int16)VP880_UNIT_CONVERT(
            LT_VP880_RES_FLT_DEFAULT_HG_MAX_ILG, VP880_UNIT_UA, VP880_UNIT_ADC_ILG);

    LT_DOUT(LT_DBG_INFO, ("Checking for Longitudinal Short"));
    LT_DOUT(LT_DBG_INFO, ("hgMinIlg: %i", hgMinIlg));
    LT_DOUT(LT_DBG_INFO, ("hgMaxIlg: %i", hgMaxIlg));
    LT_DOUT(LT_DBG_INFO, ("ilnf: %i", ilnf));
    LT_DOUT(LT_DBG_INFO, ("ilrf: %i", ilrf));
    LT_DOUT(LT_DBG_INFO, ("ilzf: %i", ilzf));

    pResFlt->fltMask = LT_TEST_PASSED;

    /* looking for hard short to ground */
    if (hgMinIlg >= ilzf) {
        pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->fltMask = LT_TEST_MEASUREMENT_ERROR;
        pResFlt->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
        return TRUE;
    }

    /* looking for a foreign current source */
    if (hgMaxIlg <= ilzf) {
        pResFlt->rtr = LT_IMPEDANCE_SHORT_CKT;
        pResFlt->rtg = LT_IMPEDANCE_SHORT_CKT;
        pResFlt->rrg = LT_IMPEDANCE_SHORT_CKT;
        pResFlt->fltMask = LT_TEST_MEASUREMENT_ERROR;
        pResFlt->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
        return TRUE;
    }

    /* looking for a ring to ground short */
    if (hgMaxIlg <= ilnf) {
        pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->rrg = LT_IMPEDANCE_SHORT_CKT;
        pResFlt->fltMask = LT_RESFM_RING;
        pResFlt->fltMask |= LT_TEST_MEASUREMENT_ERROR;
        pResFlt->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
        worstILG = ilnf;
        shortFnd = TRUE;
    }

    /* looking for a worse tip to ground short */
    if ((hgMaxIlg <= ilrf) && (ilrf > worstILG)) {
        pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->rtg = LT_IMPEDANCE_SHORT_CKT;
        pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
        pResFlt->fltMask = LT_RESFM_TIP;
        pResFlt->fltMask |= LT_TEST_MEASUREMENT_ERROR;
        pResFlt->measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
        shortFnd = TRUE;
    }

    return shortFnd;
}

static void
Vp880ResFltHGCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtResFltResultType *pResFlt = &pTestCtx->pResult->result.resFlt;
    LtVp880ResFltTempType *pTempResFlt = &pTestCtx->pTemp->vp880Temp.tempData.resFlt;
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

    int32 gfud = 1200;

    /* adjust gdc value based on leakage resistance in the sense paths*/
    int32 gdc = 255;
    int32 gdcA = gdc + ((gdc * senseA) / leakA);
    int32 gdcB = gdc + ((gdc * senseB) / leakB);

     /* adjust fudge factor based on rev code */
    if ((pTemp->revCode >= 3) || 
        (pTemp->productCode == 0xFF) ||
        (pTemp->productCode == 0xF3)) {
        /*  was 133 not sure why... merle has 130.. might be for JA */
        gfud = 130;
    }

    /* debugging */
    LT_DOUT(LT_DBG_INFO, ("vanf: %li", vanf));
    LT_DOUT(LT_DBG_INFO, ("vbnf: %li", vbnf));
    LT_DOUT(LT_DBG_INFO, ("imnf: %li", imnf));
    LT_DOUT(LT_DBG_INFO, ("ilnf: %li", ilnf));

    LT_DOUT(LT_DBG_INFO, ("varf: %li", varf));
    LT_DOUT(LT_DBG_INFO, ("vbrf: %li", vbrf));
    LT_DOUT(LT_DBG_INFO, ("imrf: %li", imrf));
    LT_DOUT(LT_DBG_INFO, ("ilrf: %li", ilrf));

    LT_DOUT(LT_DBG_INFO, ("vazf: %li", vazf));
    LT_DOUT(LT_DBG_INFO, ("vbzf: %li", vbzf));
    LT_DOUT(LT_DBG_INFO, ("ilzf: %li", ilzf));

    /*
     * 1) Calculate the output current changes from the chip
     * 2) Adjust the longitudinal current measurements for the metallic voltage
     * 3) Adjust for the currents in the DC sense resistors to get the external currents
     */
    dMicM = (2 * -25600L * (imrf - imnf))          /* 1 */
        + (gdcA * dMvcA - gdcB * dMvcB);          /* 3 */
    dMicL = (2 * -8960 * (ilrf - ilnf))           /* 1 */
        + (gfud * (dMvcA - dMvcB))                /* 2 */
        - (gdcA * dMvcA + gdcB * dMvcB);          /* 3 */
    dLicL = (-8960 * (ilrf - 2 * ilzf + ilnf))    /* 1 */
        + (gfud * (dLvcA - dLvcB))                /* 2 */
        + (-gdcA * dLvcA - gdcB * dLvcB);         /* 3 */


    /* Shift the current results into 16 bit words */
    dMicM = (dMicM + 8192) >> 14;
    dMicL = (dMicL + 8192) >> 14;
    dLicL = (dLicL + 8192) >> 14;

    /* Calculate Tip to Ring Res. to closest 10th of an Ohm. */
    numAB =
        ((dLvcB*dMvcB - dLvcB*dMvcA + dLvcA*dMvcB - dLvcA*dMvcA + 512) >> 10) * 3125;
    denAB =
        ((dLvcB*dMicM + dLvcA*dMicM - dMvcB*dLicL + dMvcA*dLicL + 2048) >> 12);
    denAB = (denAB/10 ? denAB : (denAB < 0 ? -10 : 10));
    rtr = 10 * (numAB/denAB);

    /* calculate Tip to Gnd Res to closest 10th of an Ohm */
    numAG = ((dLvcA*dMvcA + 128) >> 8) * 3125;
    denAG = ((dMvcA*dLicL + dLvcA*dMicL + 512) >> 10);
    denAG = (denAG/10 ? denAG : (denAG < 0 ? -10 : 10));
    rtg = 10 * (numAG/denAG);

    /* calculate Ring to Gnd Res to closest 10th of an Ohm */
    numBG = ((dLvcB*dMvcB + 128) >> 8) * 3125;
    denBG = ((dMvcB*dLicL + dLvcB*dMicL + 512) >> 10);
    denBG = (denBG/10 ? denBG : (denBG < 0 ? -10 : 10));
    rrg = 10 * (numBG/denBG);

    /* What is the High Gain Mode tip to ring resistance */
    if ((0 > rtr) || (VVP_RES_FLT_OPEN_CIRCUIT < rtr )) {
        /* is this an open circuit? */
        rtr = LT_IMPEDANCE_OPEN_CKT;
    }

     /* What is the High Gain Mode tip to Gnd resistance */
    if ((0 > rtg) || (VVP_RES_FLT_OPEN_CIRCUIT < rtg )) {
        /* is this an open circuit? */
        rtg = LT_IMPEDANCE_OPEN_CKT;
    }

    /* What is the High Gain Mode ring to Gnd resistance */
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
            pResFlt->rtg = (rtg <= VVP_RES_FLT_SHORT_TO_GND) ? LT_IMPEDANCE_SHORT_CKT : rtg;
            pResFlt->rrg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->fltMask = LT_RESFM_TIP;
            worst = rtg;
        }

        /* is this condiditon worse than the previous? */
        if (rrg < worst) {
            pResFlt->rtr = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rtg = LT_IMPEDANCE_NOT_MEASURED;
            pResFlt->rrg = (rrg <= VVP_RES_FLT_SHORT_TO_GND) ? LT_IMPEDANCE_SHORT_CKT : rrg;
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
#endif /* LT_RES_FLT */

#ifdef LT_MSOCKET
static void
Vp880MSocketCalResultType1(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880MSocketTempType *pTempMSocket = &pTestCtx->pTemp->vp880Temp.tempData.mSocket;
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
        LT_DOUT(LT_DBG_ERRR, ("Vp880MSocketCalResultType1(): Impossible to compute"));
        return;
    }

    pResults->fltMask = LT_TEST_PASSED;

    cIMnf = (-1434 * imnf - 2120 * (vanf - vbnf) + 8192) / 16384;
    cIMzf = (-1434 * imzf - 2120 * (vazf - vbzf) + 8192) / 16384;
    cIMrf = (-1434 * imrf - 2120 * (varf - vbrf) + 8192) / 16384;

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

    /* LP modules have 150kOhm to battery, looks like 600kOhm in parallel  */
    if ((pTemp880->termType == VP_TERM_FXS_LOW_PWR) ||
        (pTemp880->termType == VP_TERM_FXS_ISOLATE_LP) ||
        (pTemp880->termType == VP_TERM_FXS_SPLITTER_LP)) {
        /* Convert to 1kOhm resolution to prevent overflow */
        fwd /= 1000 ;
        /* Remove the effect of a 600kOhm in parallel */
        fwd = fwd * 600 / (600 - fwd);
        /* Re-scale to 1Ohm resolution */
        fwd *= 1000 ;
    }

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
Vp880MSocketCalResultType2(
    LtTestCtxType *pTestCtx)
{
    LtVp880MSocketTempType *pTempMSocket = &pTestCtx->pTemp->vp880Temp.tempData.mSocket;
    LtMSocketResultType *pResults = &pTestCtx->pResult->result.mSocket;
    int32 decay, iM0est, iM1est, vStepC, rSigCest;

    const int32 vanf = pTempMSocket->vpApiMSocketType2Results.vanf;
    const int32 vbnf = pTempMSocket->vpApiMSocketType2Results.vbnf;
    const int32 imrf1 = pTempMSocket->vpApiMSocketType2Results.imrf1;
    const int32 varf2 = pTempMSocket->vpApiMSocketType2Results.varf2;
    const int32 vbrf2 = pTempMSocket->vpApiMSocketType2Results.vbrf2;
    const int32 imrf2 = pTempMSocket->vpApiMSocketType2Results.imrf2;
    const int32 imrf3 = pTempMSocket->vpApiMSocketType2Results.imrf3;

    if ((imrf1 == imrf2) || (imrf1 == imrf3) || (imrf2 == imrf3) ||
        ((imrf1 - 2 * imrf2 + imrf3) == 0)) {
        /* Something interferes on the line, impossible to check for a master socket */
        LT_DOUT(LT_DBG_ERRR, ("Vp880MSocketCalResultType2(): Impossible to compute"));
        return;
    }

    decay = (imrf2 - imrf3) * 1000 / (imrf1 - imrf2);
    iM1est = (imrf1 - imrf2) * (imrf1 - imrf2) / (imrf1 - 2 * imrf2 + imrf3);
    iM0est = iM1est * 1125 / 1000;

    if (iM0est == 0) {
        /* Something interferes on the line, impossible to check for a master socket */
        LT_DOUT(LT_DBG_ERRR, ("Vp880MSocketCalResultType2(): Impossible to compute"));
        return;
    }

    vStepC = (vanf - vbnf) - (varf2 - vbrf2);
    rSigCest = (vStepC * 4096 / iM0est) * 279;

    pResults->fltMask = LT_MSKT_NOT_PRESENT;

    /* Allow 12% tolerance */
    if ((ABS(decay - 550) < 66) && (ABS(rSigCest - 470000) < 60000)) {
        pResults->fltMask = LT_MSKT_TEST_PASSED;
    }

    LT_DOUT(LT_DBG_INFO, ("vanf: %li", vanf));
    LT_DOUT(LT_DBG_INFO, ("vbnf: %li", vbnf));
    LT_DOUT(LT_DBG_INFO, ("imrf1: %li", imrf1));
    LT_DOUT(LT_DBG_INFO, ("varf2: %li", varf2));
    LT_DOUT(LT_DBG_INFO, ("vbrf2: %li", vbrf2));
    LT_DOUT(LT_DBG_INFO, ("imrf2: %li", imrf2));
    LT_DOUT(LT_DBG_INFO, ("imrf3: %li", imrf3));

    LT_DOUT(LT_DBG_INFO, ("iM1est = %li, iM0est = %li, vStepC = %li,", iM1est, iM0est, vStepC));
    LT_DOUT(LT_DBG_INFO, ("decay = %li, rSigCest = %li", decay, rSigCest));

    return;
}
#endif /* LT_MSOCKET */

#ifdef LT_XCONNECT
static void
Vp880XConnectCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880XConnectTempType *pTempXConnect = &pTestCtx->pTemp->vp880Temp.tempData.xConnect;
    LtXConnectResultType *pResults = &pTestCtx->pResult->result.xConnect;

    const int32 data = (int32)pTempXConnect->vpApiXConnectResults.data;

    pResults->fltMask = LT_XCON_TEST_PASSED;
    pResults->measStatus = LT_MSRMNT_STATUS_PASSED;

    switch (pTempXConnect->phase) {
        case VP880_XCONNECT_PHASE1:
            /* Measure the metalic voltage in disconnect, should be less than 35V */
            LT_DOUT(LT_DBG_INFO, ("VP880_XCONNECT_PHASE1: (data=%li | %liV)", data, (data * 732 + 50000) / 100000));
            if (ABS(data) < 4782) {
                pResults->fltMask = LT_XCON_TEST_PASSED;
                pTempXConnect->phase = VP880_XCONNECT_DONE;
            } else {
                pTempXConnect->phase = VP880_XCONNECT_PHASE2;
            }

            /* Store the metalic voltage sign for the next phase */
            if (data > 0) {
                pTempXConnect->isEMFPositive = TRUE;
            } else {
                pTempXConnect->isEMFPositive = FALSE;
            }
            break;

        case VP880_XCONNECT_PHASE2:
            /* Measure the metalic current in active with DCFeed=12V, should be less than 15mA */
            LT_DOUT(LT_DBG_INFO, ("VP880_XCONNECT_PHASE2: (data=%li | %limA)", data, (data * 176 + 50000) / 100000));
            if (ABS(data) < 8522) {
                pResults->fltMask = LT_XCON_TEST_PASSED;
                pTempXConnect->phase = VP880_XCONNECT_DONE;
            } else {
                pTempXConnect->phase = VP880_XCONNECT_PHASE3;
            }
            break;

        case VP880_XCONNECT_PHASE3:
            /* Measure the metalic voltage in disconnect, should be less than 35V */
            LT_DOUT(LT_DBG_INFO, ("VP880_XCONNECT_PHASE3: (data=%li | %liV)", data, (data * 732 + 50000) / 100000));
            if (ABS(data) < 4782) {
                pResults->fltMask = LT_XCON_TEST_PASSED;
            } else {
                if (pTempXConnect->isEMFPositive == TRUE) {
                    pResults->fltMask = LT_XCON_REVERSE_POLARITY;
                } else {
                    pResults->fltMask = LT_XCON_NORMAL_POLARITY;
                }
            }
            pTempXConnect->phase = VP880_XCONNECT_DONE;
            break;

        default:
            pResults->fltMask = LT_XCON_FAULT;
            break;
    }

    return;
}
#endif /* LT_XCONNECT */

#ifdef LT_CAP
static void
Vp880CapCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880CapTempType *pTempCap = &pTemp->tempData.cap;
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

    if (((pTempCap->vpApiCapResults1.freq >> 4) & 0x0F) == 4) {
        /* 280Hz */
        /* mConst = 109266 / (pTempCap->vpApiCapResults1.freq & 0x0F); */
        mConst = 54633;
        /* lConst = 19884 / (pTempCap->vpApiCapResults1.freq & 0x0F); */
        lConst = 9942;
    } else {
        /* 166Hz -> 2666Hz */
        mConst = 11482 << ((pTempCap->vpApiCapResults1.freq >> 4) & 0x0F);
        /* mConst /= pTempCap->vpApiCapResults1.freq & 0x0F; */
        mConst /= 2;
        lConst = 2090 << ((pTempCap->vpApiCapResults1.freq >> 4) & 0x0F);
        /* lConst /= pTempCap->vpApiCapResults1.freq & 0x0F; */
        lConst /= 2;
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

    return;
}
#endif /* LT_CAP */

#ifdef LT_DC_FEED_ST
static void
Vp880DcFeedCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880DcFeedSTTempType *pTempDcFeedSt = &pTemp->tempData.dcFeedST;
    LtDcFeedSTResultType *pResults = &pTestCtx->pResult->result.dcFeedST;

    /* knocking down the % error granularity to Ohms*/
    LtPercentType rloadErr = (pTempDcFeedSt->criteria.rloadErr / 1000);
    LtImpedanceType testSwitch = pTemp->attributes.topology.rTestLoad;

    pResults->vTestLoad = VP880_UNIT_CONVERT(pTempDcFeedSt->vsab,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    pResults->iTestLoad = VpConvertImt(pTestCtx, pTempDcFeedSt->imt);

    pResults->fltMask = LT_TEST_PASSED;

    if (pTempDcFeedSt->internalTestTerm == FALSE) {
        /* Test switch impedance minus test switch voltage drop */
        pResults->rTestLoad  = (
         ((ABS(pResults->vTestLoad) - VVP_INTERNAL_TEST_SWITCH_VDC_DROP) * 10000l) /
            ABS(pResults->iTestLoad));

        /* remove internal test switch impedance */
        pResults->rTestLoad -= VVP_INTERNAL_TEST_SWITCH_IMPED;

        /* determine if the measured value is within criteria spec */
        if ( ABS(testSwitch - ABS(pResults->rTestLoad) ) >
            ((testSwitch * rloadErr) / 100)) {
            pResults->fltMask |= LT_DC_FEED_ST_TEST_FAILED;
        }
    } else {
        LtCurrentType targetCurrent;
        LtPercentType percentErr;

        /* Different calculations needed for rTestLoad if the internal test
         * termination is in use. */
        pResults->rTestLoad = ABS((pResults->vTestLoad * 10000l) /
                                pResults->iTestLoad);

        /* Determine if the measured loop current is within a defined % of the
         * ILA value specified from the DC profile */
        targetCurrent = VP880_UNIT_CONVERT(pTempDcFeedSt->ila,
            VP880_UNIT_ADC_IMET_NOR_JA, VP880_UNIT_UA);

        percentErr = (LT_DC_FEED_ST_CRT_ILA_PERCENT / 1000);
        percentErr = (targetCurrent * percentErr) / 100;
        if ( ABS(targetCurrent - ABS(pResults->iTestLoad) ) > percentErr) {
            pResults->fltMask |= LT_DC_FEED_ST_TEST_FAILED;
        }
    }

    return;
}
#endif /* LT_DC_FEED_ST */

#ifdef LT_RD_LOOP_COND
static void
Vp880RdLoopCondCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880RdLoopCondTempType *pTempRdLoopCond = &pTemp->tempData.rdLoopCond;
    LtRdLoopCondResultType *pResults = &pTestCtx->pResult->result.rdLoopCond;

    pResults->fltMask = LT_TEST_PASSED;

    if (LT_VOLTAGE_NOT_MEASURED != pTempRdLoopCond->vab) {
        pResults->vab = VP880_UNIT_CONVERT(pTempRdLoopCond->vab,
            VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    if (LT_VOLTAGE_NOT_MEASURED != pTempRdLoopCond->vag) {
        pResults->vag = VP880_UNIT_CONVERT(pTempRdLoopCond->vag,
            VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    if (LT_VOLTAGE_NOT_MEASURED != pTempRdLoopCond->vbg) {
        pResults->vbg = VP880_UNIT_CONVERT(pTempRdLoopCond->vbg,
            VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    if (LT_CURRENT_NOT_MEASURED != pTempRdLoopCond->imt) {
        if ((pTemp->resources.initialLineState == VP_LINE_RINGING) ||
            (pTemp->resources.initialLineState == VP_LINE_RINGING_POLREV)) {
            /* Ringing state */
            pResults->imt = VP880_UNIT_CONVERT(pTempRdLoopCond->imt,
                    VP880_UNIT_ADC_IMET_RING, VP880_UNIT_UA);
        } else {
            /* Convert IMT according to the revision code */
            pResults->imt = VpConvertImt(pTestCtx, pTempRdLoopCond->imt);
        }
    }

    if (LT_CURRENT_NOT_MEASURED != pTempRdLoopCond->ilg) {
        if ((pTempRdLoopCond->ilg != VP_INT16_MIN) && (pTempRdLoopCond->ilg != VP_INT16_MAX)) {
            pResults->ilg = VP880_UNIT_CONVERT(pTempRdLoopCond->ilg,
                VP880_UNIT_ADC_ILG, VP880_UNIT_UA);
        } else {
            pResults->ilg = LT_MAX_CURRENT;
        }
    }

    if (LT_VOLTAGE_NOT_MEASURED != pTempRdLoopCond->vbat1) {
        pResults->vbat1 = VP880_UNIT_CONVERT(pTempRdLoopCond->vbat1,
            VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    if (LT_VOLTAGE_NOT_MEASURED != pTempRdLoopCond->vbat2) {
        pResults->vbat2 = VP880_UNIT_CONVERT(pTempRdLoopCond->vbat2,
            VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    if (LT_VOLTAGE_NOT_MEASURED != pTempRdLoopCond->vbat3) {
        pResults->vbat3 = VP880_UNIT_CONVERT(pTempRdLoopCond->vbat3,
            VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    /* Compute rloop */
    if (LT_CURRENT_NOT_MEASURED != pTempRdLoopCond->imt) {
        if (pTempRdLoopCond->imt == 0) {
            pResults->rloop = LT_IMPEDANCE_OPEN_CKT;
        } else {
            pResults->rloop  = ((pResults->vab * 10000L) / pResults->imt);
            if (pResults->rloop < 0) {
                pResults->rloop *= -1;
            }
            if (pResults->rloop > 250000L) {
                pResults->rloop = LT_IMPEDANCE_OPEN_CKT;
            }
        }
    }

    return;
}
#endif /* LT_RD_LOOP_COND */

#ifdef LT_DC_VOLTAGE
static void
Vp880DcVoltageCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVoltageType diffV;
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880DcVoltageTempType *pTempDcVoltage = &pTemp->tempData.dcVoltage;
    LtDcVoltageResultType *pResults = &pTestCtx->pResult->result.dcVoltage;

    LtVoltageType reqstV = ABS(pTempDcVoltage->input.testVoltage);
    /* knocking down the % error granularity to Volts*/
    LtPercentType testVoltageErr = (pTempDcVoltage->criteria.voltageErr / 1000);
    testVoltageErr = ((reqstV * testVoltageErr) / 100) + LT_DC_VOLTAGE_DFLTC_VOLTAGE_OFF;

    pResults->measuredVoltage1 = VP880_UNIT_CONVERT(pTempDcVoltage->adcVsab1,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    pResults->measuredVoltage2 = VP880_UNIT_CONVERT(pTempDcVoltage->adcVsab2,
        VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    pResults->fltMask = LT_TEST_PASSED;

    diffV = ABS(reqstV - ABS(pResults->measuredVoltage1));
    if (diffV > testVoltageErr) {
        pResults->fltMask |= LT_DC_VOLTAGE_TEST_FAILED;
    }

    diffV = ABS(reqstV - ABS(pResults->measuredVoltage2));
    if (diffV > testVoltageErr) {
        pResults->fltMask |= LT_DC_VOLTAGE_TEST_FAILED;
    }
    return;
}
#endif /* LT_DC_VOLTAGE */

#ifdef LT_RINGING_ST
static void
Vp880RingingStCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880RingingStTempType *pTempRingingSt = &pTemp->tempData.ringingSt;
    LtRingingSTResultType *pResults = &pTestCtx->pResult->result.ringingST;
    LtRingingSTInputType *pInput = &pTempRingingSt->input;
    LtRingingSTCriteriaType *pCriteria = &pTempRingingSt->criteria;
    VpTestResultAcRlType *pAcRloop1Rslt = &pTempRingingSt->vpApiAcRloop1Rslt;
    VpTestResultAcRlType *pAcRloop2Rslt = &pTempRingingSt->vpApiAcRloop2Rslt;
    LtImpedanceType testSwitch = pTemp->attributes.topology.rTestLoad;
    LtFreqType requestedFreq;
    LtVoltageType requestedV;
    LtPercentType percentErr;

    /* if an off-hook phone was detected then no measurements were taken */
    if (pTempRingingSt->offhook) {
        pResults->fltMask = LT_RINGING_STM_OFF_HOOK;
        return;
    }

    pResults->fltMask = LT_TEST_PASSED;

    /* evaluate and save measured open circuit voltage */
    requestedV = pInput->vRinging;
    pResults->openCktRingVol = VP880_UNIT_CONVERT(pAcRloop2Rslt->vab,
            VP880_UNIT_ADC_VAB_RMS, VP880_UNIT_MVRMS);
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
    pResults->iRload = VP880_UNIT_CONVERT(pAcRloop1Rslt->imt,
            VP880_UNIT_ADC_IMET_RING, VP880_UNIT_UA);

    /* evaluate and save the ac load resistance */
    /* knocking down the % error granularity to Ohms*/
    percentErr = (pCriteria->rLoadErr / 1000);
    percentErr = (testSwitch * percentErr) / 100;
    if (pTempRingingSt->internalTestTerm == FALSE) {
        pResults->acRload =  (((requestedV - VVP_INTERNAL_TEST_SWITCH_VAVG_DROP)
            * 10000L) / pResults->iRload) - 2000 - VVP_INTERNAL_TEST_SWITCH_IMPED;
        if ((ABS(testSwitch - ABS(pResults->acRload))) > percentErr) {
            pResults->fltMask |= LT_RINGING_STM_ACRLOAD_OC;
        }
    } else {
        LtCurrentType currentThreshold;
        LtVoltageType voltageThreshold;

        /* No test switch parameters to subtract if using the internal test
         * termination */
        pResults->acRload = ABS((requestedV * 10000L) / pResults->iRload) - 2000;

        /* No tolerance check on load resistance when not using a load */

        /* Check that the measured rms current is close to what we expect. */
        /* The AC Rloop test uses a hard-coded ILR value.  ILR defines the
         * maximum peak current.  First divide by sqrt(2) to get the rms value
         * of a sine wave with a peak of ILR, the largest that will not clip. */
        currentThreshold = (LT_VP880_RINGING_ST_ACRLOOP_ILR * 10000) / 14142;

        /* Compute the rms voltage of the maximum clean sine wave, assuming
         * a defined source impedance as the only loop resistance */
        voltageThreshold = (currentThreshold * LT_VP880_RINGING_ST_SOURCE_IMPEDANCE)
                                / 10000;

        if (requestedV <= voltageThreshold) {
            /* If the requested rms voltage is less than the rms voltage of the
             * maximum clean sine wave, then we expect the measured rms current
             * to match up */
            LtCurrentType currentTarget;

            currentTarget = (requestedV * 10000) / LT_VP880_RINGING_ST_SOURCE_IMPEDANCE;

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

            upperLimit = LT_VP880_RINGING_ST_ACRLOOP_ILR;
            upperLimit = upperLimit + ((upperLimit * percentErr) / 100);

            if (pResults->iRload < lowerLimit || pResults->iRload > upperLimit) {
                pResults->fltMask |= LT_RINGING_STM_IRLOAD_OC;
            }
        }
    }

    if (FALSE == pTempRingingSt->ringTrip) {
        pResults->fltMask |= LT_RINGING_STM_NO_RINGTRIP;
    }
    return;
}
#endif /* LT_RINGING_ST */

#ifdef LT_RD_BAT_COND
static void
Vp880RdBatCondCalResult(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880RdBatCondTempType *pTempRdBatCond = &pTemp->tempData.rdBatCond;
    LtReadBatCondResultType *pResults = &pTestCtx->pResult->result.readBatCond;
    int16 data;

    pResults->fltMask = LT_TEST_PASSED;

    data = pTempRdBatCond->vpApiLpCnd1Rslt.data;
    if (VP_INT16_MAX != data) {
        pResults->bat1 =
            VP880_UNIT_CONVERT(data, VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    data = pTempRdBatCond->vpApiLpCnd2Rslt.data;
    if (VP_INT16_MAX != data) {
        pResults->bat2 =
            VP880_UNIT_CONVERT(data, VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    data = pTempRdBatCond->vpApiLpCnd3Rslt.data;
    if (VP_INT16_MAX != data) {
        pResults->bat3 =
            VP880_UNIT_CONVERT(data, VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    }

    return;
}
#endif /* LT_RD_BAT_COND */

#ifdef LT_FLT_DSCRM

static void
Vp880FltDscrmResult(
    LtTestCtxType *pTestCtx)
{

    LtVp880TestTempType *pTemp      = &pTestCtx->pTemp->vp880Temp;
    LtFltDscrmResultType *pResults  = &pTestCtx->pResult->result.fltDscrm;
    VpTestResultFltDscrmType *pVpApiFltDscrm = &pTemp->tempData.fltDscrm.vpApiFltDscrmRslt;

    /* local vars to hold the converted data */
    int32 vtuBalAC, vruBalAC, vtrAC, vtoAC, vroAC, vtgFeed, vrgFeed, vtrFeed;

    /* convert all measured data to real mV and mVrms values */
    vtuBalAC = VP880_UNIT_CONVERT(pVpApiFltDscrm->vtuBalAC, VP880_UNIT_ADC_AC_VAB_RMS, VP880_UNIT_MVRMS);
    vruBalAC = VP880_UNIT_CONVERT(pVpApiFltDscrm->vruBalAC, VP880_UNIT_ADC_AC_VAB_RMS, VP880_UNIT_MVRMS);
    vtrAC    = VP880_UNIT_CONVERT(pVpApiFltDscrm->vtrAC, VP880_UNIT_ADC_AC_VAB_RMS, VP880_UNIT_MVRMS);
    vtoAC    = VP880_UNIT_CONVERT(pVpApiFltDscrm->vtoAC, VP880_UNIT_ADC_AC_VAB_RMS, VP880_UNIT_MVRMS);
    vroAC    = VP880_UNIT_CONVERT(pVpApiFltDscrm->vroAC, VP880_UNIT_ADC_AC_VAB_RMS, VP880_UNIT_MVRMS);
    vtgFeed  = VP880_UNIT_CONVERT(pVpApiFltDscrm->vtgFeed, VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    vrgFeed  = VP880_UNIT_CONVERT(pVpApiFltDscrm->vrgFeed, VP880_UNIT_ADC_VAB, VP880_UNIT_MV);
    vtrFeed  = VP880_UNIT_CONVERT(pVpApiFltDscrm->vtrFeed, VP880_UNIT_ADC_VAB, VP880_UNIT_MV);

    /* debug info */
    LT_DOUT(LT_DBG_INFO, ("hookDet=%i", pVpApiFltDscrm->hookDet));
    LT_DOUT(LT_DBG_INFO, ("gnkDet=%i", pVpApiFltDscrm->gnkDet));

    LT_DOUT(LT_DBG_INFO, ("vtrFeed %i = %li mV", pVpApiFltDscrm->vtrFeed, vtrFeed ));
    LT_DOUT(LT_DBG_INFO, ("vtgFeed %i = %li mV", pVpApiFltDscrm->vtgFeed, vtgFeed ));
    LT_DOUT(LT_DBG_INFO, ("vrgFeed %i = %li mV", pVpApiFltDscrm->vrgFeed, vrgFeed ));

    LT_DOUT(LT_DBG_INFO, ("vtrAC %i = %li mVrms", pVpApiFltDscrm->vtrAC, vtrAC ));
    LT_DOUT(LT_DBG_INFO, ("vtoAC %i = %li mVrms", pVpApiFltDscrm->vtoAC, vtoAC ));
    LT_DOUT(LT_DBG_INFO, ("vroAC %i = %li mVrms", pVpApiFltDscrm->vroAC, vroAC ));

    LT_DOUT(LT_DBG_INFO, ("vtuBalAC %i = %li mVrms", pVpApiFltDscrm->vtuBalAC, vtuBalAC ));
    LT_DOUT(LT_DBG_INFO, ("vruBalAC %i = %li mVrms", pVpApiFltDscrm->vtuBalAC, vruBalAC ));


    pResults->fltMask = LT_TEST_PASSED;

    /* determine what faults are present */
    /* determine if TDC and/or RDC are open */
    if ( (vtgFeed > 1400) && (vtgFeed < 1600) ) {
        pResults->fltMask |= LT_FDM_TDC_OPEN;
    }
    if ( (vrgFeed > 1400) && (vrgFeed < 1600) ) {
        pResults->fltMask |= LT_FDM_RDC_OPEN;
    }

    /* determine if TIPD and or RINGD are shorted */
    if ( pResults->fltMask == LT_TEST_PASSED ) {
        if ( (vtgFeed > -5000) && (vtuBalAC < 3) ) {
            pResults->fltMask |= LT_FDM_TIPD_SHORT;
        }
        if ( (vrgFeed > -5000) && (vruBalAC < 3) ) {
            pResults->fltMask |= LT_FDM_RINGD_SHORT;
        }
    }

    /* determine if (TAC and/or RAC) and/or (Tip short and/or Ring Short) are present */
    if ( pResults->fltMask == LT_TEST_PASSED ) {
        if ( (vrgFeed >- 500) && (vtgFeed < -48000) ) {
            pResults->fltMask |= LT_FDM_RING_SHORT;
            if (vtrAC < 200 ) {
                pResults->fltMask |= LT_FDM_TAC_OPEN;
            }

        }
        if ( (vtgFeed > -500) && (vrgFeed < -48000) ) {
            pResults->fltMask |= LT_FDM_TIP_SHORT;
            if ( vtrAC < 200 ) {
                pResults->fltMask |= LT_FDM_RAC_OPEN;
            }
        }
    }

    /*  determine if Tip/Ring are shorted and if TAC/RAC are open */
    if ( pResults->fltMask == LT_TEST_PASSED ) {
        if ( ABS(vtgFeed - vrgFeed) < 1000 ) {

            if (vtrAC < 10) {
                pResults->fltMask |= LT_FDM_TIPD_RINGD_SHORT;
            } else {
                pResults->fltMask |= LT_FDM_TIP_RING_SHORT;
                if ( vtoAC >= 100 ) {
                    pResults->fltMask |= (LT_FDM_RAC_OPEN | LT_FDM_TAC_OPEN);
                }
             }
        }
    }

    /* determine if TAC and/or RAC are open due to and AC path issue */
    if ( pResults->fltMask == LT_TEST_PASSED ) {
        if ( vruBalAC < 300 ) {
            pResults->fltMask |= LT_FDM_RAC_OPEN;
        }

        if ( vtuBalAC < 300 ) {
            pResults->fltMask |= LT_FDM_TAC_OPEN;
        }
    }

    /* if we get all the way here without a fault then report gndky if need be */
    if ( (pResults->fltMask == LT_TEST_PASSED) && pVpApiFltDscrm->gnkDet ) {
        pResults->fltMask |= LT_FDM_UNKNOWN_LINE_FAULT;
    }

    /*if we get all the way here without and hook bit is set then report the hook */
    if ( (pResults->fltMask == LT_TEST_PASSED) && pVpApiFltDscrm->hookDet ) {
        pResults->fltMask |= LT_FDM_OFF_HOOK;
    }

    /* if any device faults are present then remove line faults from list */
    if (pResults->fltMask & 0xFFE0) {
        pResults->fltMask &= 0xFFE0;
    }

    (void)vtrFeed;   /* prevent compiler warning when debug is not enabled */
    (void)vroAC;     /* prevent compiler warning when debug is not enabled */
    return;
}

#endif /* LT_FLT_DSCRM */

#if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)
extern bool
LtVp880AllGr909RunRoh(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880AllGr909TestTempType *pTempAllGr909 = &pTemp->allGr909;
    LtAllGr909ResultType *pTempResults = &pTempAllGr909->results;

    /* if the resflt does not have an ilg issue then run the roh test */
    if (pTempResults->resFlt.measStatus != LT_MSRMNT_STATUS_EXCESSIVE_ILG) {
        return TRUE;
    }

    /* indicate we ran the roh test and it failed */
    pTempResults->roh.fltMask = LT_TEST_MEASUREMENT_ERROR;
    pTempResults->roh.measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
    pTempResults->fltMask |= LT_A909TM_ROH_FAILED;
    pTempResults->subTestMask |= LT_A909STM_ROH;

    LtVp880AllGr909RunRen(pTestCtx);

    return FALSE;
}

/*
 * This function is determining if the REN portion of the 909 All test needs to
 * run in order to reduce test execution time for common test conditions.
 *
 * If possible the function will calculate the REN value from the tip to ring
 * resistance as measured by the resistive faults test.
 *
 * The calculation will only be made if the rtr value is valid and the ROH
 * test fails.
 */
extern bool
LtVp880AllGr909RunRen(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880RingersTempType *pTempRingers = &pTemp->tempData.ringers;
    LtVp880AllGr909TestTempType *pTempAllGr909 = &pTemp->allGr909;
    LtAllGr909ResultType *pTempResults = &pTempAllGr909->results;

    const int16 maxRes = pTempRingers->criteria.renHigh;
    const int16 minRes = pTempRingers->criteria.renLow;
    int32 rtr;


    /* if the roh had an ilg issue then dont run  */
    if (pTempResults->roh.measStatus == LT_MSRMNT_STATUS_EXCESSIVE_ILG) {
        pTempResults->ringers.fltMask = LT_TEST_MEASUREMENT_ERROR;
        pTempResults->ringers.measStatus = LT_MSRMNT_STATUS_EXCESSIVE_ILG;
        pTempResults->fltMask |= LT_A909TM_RINGERS_FAILED;
        pTempResults->subTestMask |= LT_A909STM_RINGERS;
        return FALSE;
    }

    /* if roh detects an off-hook phone then dont run REN */
    if (pTempResults->ringers.fltMask == LT_ROHM_OFF_HOOK) {
        return FALSE;
    }

    /*
     * Note: LT_IMPEDANCE_SHORT_CKT is assumed to be 0 Ohms and
     * resulting calculated ren will be 7000 REN, approx same value
     * that the actual ren test measures.
     */
    if ((pTempResults->resFlt.rtr == LT_IMPEDANCE_NOT_MEASURED) ||
        (pTempResults->resFlt.rtr == LT_IMPEDANCE_OPEN_CKT) ) {
        /* cannot calculate REN from rtr value */
        return TRUE;
    }

    /*
     * if the ROH test completes then run ren and
     * tip/ring is greater than 1400 ohms
     */
    if ((pTempResults->roh.measStatus != LT_MSRMNT_STATUS_EXCESSIVE_ILG) &&
        (pTempResults->resFlt.rtr > 14000)) {
        return TRUE;
    }

    /* we can calculate REN from rtr */
    pTempResults->ringers.ringerTestType = LT_RINGER_REGULAR_PHNE_TEST;

    /*
     * REN = 7000 Ohms / measured resistance
     * ensure we dont divide by 0
     */
    rtr = (pTempResults->resFlt.rtr <= 10) ? 10 : pTempResults->resFlt.rtr;
    pTempResults->ringers.ren = ((70000L * 1000) / rtr);

    /* check 909 limits */
    if (maxRes < pTempResults->ringers.ren) {
        pTempResults->ringers.fltMask = LT_RNGM_REN_HIGH;
        pTempResults->fltMask |= LT_A909TM_RINGERS_FAILED;
    } else if (minRes > pTempResults->ringers.ren) {
        pTempResults->ringers.fltMask = LT_RNGM_REN_LOW;
        pTempResults->fltMask |= LT_A909TM_RINGERS_FAILED;
    } else {
        pTempResults->ringers.fltMask = LT_TEST_PASSED;
    }

    /* indicate we ran the ren test */
    pTempResults->subTestMask |= LT_A909STM_RINGERS;
    return FALSE;
}
#endif

/*
 * This function returns TRUE if a given test is supported for a given
 * type of line termination type else it returns false;
 */
extern bool
LtVp880IsTestSupported(
    LtTestCtxType *pTestCtx,
    VpDeviceInfoType *pDeviceInfo,
    LtTestIdType testId)
{
    switch (testId) {
        #ifdef LT_PRE_LINE_V
        case LT_TID_PRE_LINE_V:
            return TRUE;
        #endif

        #ifdef LT_LINE_V
        case LT_TID_LINE_V:
            return TRUE;
        #endif

        #ifdef LT_ROH
        case LT_TID_ROH:
            return TRUE;
        #endif

        #ifdef LT_RINGERS
        case LT_TID_RINGERS:
            return TRUE;
        #endif

        #ifdef LT_RES_FLT
        case LT_TID_RES_FLT:
            return TRUE;
        #endif

        #ifdef LT_MSOCKET
        case LT_TID_MSOCKET:
            return TRUE;
        #endif

        #ifdef LT_XCONNECT
        case LT_TID_XCONNECT:
            return TRUE;
        #endif

        #ifdef LT_CAP
        case LT_TID_CAP: {
            if ((pDeviceInfo->revCode > 3) || 
                (pDeviceInfo->productCode == 0xFF) ||
                (pDeviceInfo->productCode == 0xF3)) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
        #endif

        #if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)
        case LT_TID_ALL_GR_909:
            return TRUE;
        #endif

        #ifdef LT_LOOPBACK
        case LT_TID_LOOPBACK: {
            LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
            LtVp880LoopbackTempType *pTempLoopback = &pTemp880->tempData.loopback;

            /* Loopback test must be run from an active state (except in TIMESLOT mode) */
            if ((pTempLoopback->vpApiInput.loopback != VP_LOOPBACK_TEST_TIMESLOT) &&
                (pTemp880->resources.initialLineState != VP_LINE_ACTIVE) &&
                (pTemp880->resources.initialLineState != VP_LINE_ACTIVE_POLREV) &&
                (pTemp880->resources.initialLineState != VP_LINE_TALK) &&
                (pTemp880->resources.initialLineState != VP_LINE_TALK_POLREV) &&
                (pTemp880->resources.initialLineState != VP_LINE_OHT) &&
                (pTemp880->resources.initialLineState != VP_LINE_OHT_POLREV)) {

                return FALSE;
            } else {
                return TRUE;
            }
        }
        #endif

        #ifdef LT_DC_FEED_ST
        case LT_TID_DC_FEED_ST:
            /* return (pCode == 0x00BC) ? FALSE : TRUE; */
            return TRUE;
        #endif

        #ifdef LT_RD_LOOP_COND
        case LT_TID_RD_LOOP_COND:
            return TRUE;
        #endif

        #ifdef LT_DC_VOLTAGE
        case LT_TID_DC_VOLTAGE:
            return TRUE;
        #endif

        #ifdef LT_RINGING_ST
        case LT_TID_RINGING_ST:
            return TRUE;
        #endif

        #ifdef LT_ON_OFF_HOOK_ST
        case LT_TID_ON_OFF_HOOK_ST:
            return TRUE;
        #endif

        #ifdef LT_RD_BAT_COND
        case LT_TID_RD_BAT_COND:
            return TRUE;
        #endif

        #ifdef LT_FLT_DSCRM
        case LT_TID_FLT_DSCRM:
            return TRUE;
        #endif

       default:
            return FALSE;
    }

} /* LtVp880IsTestSupported() */

/*
 * This function returns TRUE if no issues were found with the input
 * arguments for the specfied testId;
 */
extern bool
LtVp880AttributeCheck(
    LtTestCtxType *pTestCtx,
    const LtTestIdType testId)
{
    bool result = FALSE;

    LT_DOUT(LT_DBG_FUNC, ("LtVp880AttributeCheck(ltTestId:%i)+",testId));

    switch (testId) {
        #ifdef LT_PRE_LINE_V
        case LT_TID_PRE_LINE_V:
            pTestCtx->TestEventHandlerFunc = LtVp880PreLineVHandler;
            result = TRUE;
            break;
        #endif

        #ifdef LT_LINE_V
        case LT_TID_LINE_V:
            result = Vp880LineVAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_ROH
        case LT_TID_ROH:
            result = Vp880RohAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RINGERS
        case LT_TID_RINGERS:
            result = Vp880RingerAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RES_FLT
        case LT_TID_RES_FLT:
            result = Vp880ResFltAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_MSOCKET
        case LT_TID_MSOCKET:
            result = Vp880MSocketAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_XCONNECT
        case LT_TID_XCONNECT:
            result = Vp880XConnectAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_CAP
        case LT_TID_CAP:
            result = Vp880CapAttributesCheck(pTestCtx);
            break;
        #endif

        #if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)
        case LT_TID_ALL_GR_909:
            result = Vp880909AllAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_LOOPBACK
        case LT_TID_LOOPBACK:
            result = Vp880LoopbackAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_DC_FEED_ST
        case LT_TID_DC_FEED_ST:
            result = Vp880DcFeedAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RD_LOOP_COND
        case LT_TID_RD_LOOP_COND:
            result = Vp880RdLoopCondAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_DC_VOLTAGE
        case LT_TID_DC_VOLTAGE:
            result = Vp880DcVoltageAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RINGING_ST
        case LT_TID_RINGING_ST:
            result = Vp880RingingStAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_ON_OFF_HOOK_ST
        case LT_TID_ON_OFF_HOOK_ST:
            result = Vp880OnOffHookStAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RD_BAT_COND
        case LT_TID_RD_BAT_COND:
            result = Vp880RdBatCondAttributesCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_FLT_DSCRM
        case LT_TID_FLT_DSCRM:
            pTestCtx->TestEventHandlerFunc = LtVp880FltDscrmHandler;
            result = TRUE;
            break;
        #endif


       default:
            LT_DOUT(LT_DBG_ERRR, ("LtVp880AttributeCheck(ltTestId:%i) : "\
                "invalid", testId));
            result = FALSE;
            break;
    }

    LT_DOUT(LT_DBG_FUNC, ("LtVp880AttributeCheck(ltTestId:%i,result:%i)-",testId,result));
    return result;
}

#ifdef LT_LINE_V
static bool
Vp880LineVAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtLineVCriteriaType *ppCriteria =
        (LtLineVCriteriaType*)pTemp->attributes.pCriteria;
    LtVp880LineVTempType *pTempLineV = &pTemp->tempData.lineV;

    /* Check criteria args*/
    if (VP_NULL == ppCriteria) {
        pTempLineV->criteria.dcHemf = LT_LINEV_CRT_DC_HEMF;
        pTempLineV->criteria.acHemf = LT_LINEV_CRT_AC_HEMF;
        pTempLineV->criteria.dcFemf = LT_LINEV_CRT_DC_FEMF;
        pTempLineV->criteria.acFemf = LT_LINEV_CRT_AC_FEMF;
    } else {
        pTempLineV->criteria.dcHemf = ppCriteria->dcHemf;
        pTempLineV->criteria.acHemf = ppCriteria->acHemf;
        pTempLineV->criteria.dcFemf = ppCriteria->dcFemf;
        pTempLineV->criteria.acFemf = ppCriteria->acFemf;
    }
    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880LineVHandler;
    return TRUE;
}
#endif /* LT_LINE_V */

#ifdef LT_ROH
static bool
Vp880RohAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880RohHandler;
    return TRUE;
}
#endif /* LT_ROH */

#ifdef LT_RINGERS
static bool
Vp880RingerAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtRingerInputType *ppInputs =
        (LtRingerInputType*)pTemp->attributes.pInputs;
    LtRingersCriteriaType *ppCriteria =
        (LtRingersCriteriaType*)pTemp->attributes.pCriteria;
    LtVp880RingersTempType *pTempRinger = &pTemp->tempData.ringers;

    if (VP_NULL == ppInputs) {
        pTempRinger->input.ringerTestType = LT_RINGERS_DFLTI_TEST_TYPE;
        pTempRinger->input.vRingerTest = LT_RINGERS_DFLTI_TEST_VOLTAGE;
        pTempRinger->input.freq = LT_RINGERS_DFLTI_TEST_FREQUENCY;
        pTempRinger->input.renFactor = LT_RINGERS_DFLTI_TEST_RENFACTOR;
    } else {
        int32 maxV = 0;
        /* make sure the test type is legal */
        if (LT_RINGER_TEST_NUM_TYPES <= ppInputs->ringerTestType) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880RingerAttributesCheck(ringerTestType:%i) : "\
                "invalid", ppInputs->ringerTestType));
            return FALSE;
        }
        pTempRinger->input.ringerTestType = ppInputs->ringerTestType;

        /* make sure the input test v is legal */
        if (pTempRinger->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST) {
            maxV = LT_VP880_RINGERS_MAX_INPUTV * 2; /* Twice the max bias voltage */
        } else if (pTempRinger->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST_3_ELE) {
            maxV = LT_VP880_RINGERS_MAX_INPUTV; /* No more than max bias voltage */
        } else {
            maxV = LT_VP880_RINGERS_MAX_INPUTV * 2; /* Twice the max bias voltage */
        }

        if (maxV < ppInputs->vRingerTest) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880RingerAttributesCheck(vRingerTest:%li) : "\
                "invalid", ppInputs->vRingerTest));
            return FALSE;
        }
        pTempRinger->input.vRingerTest = ppInputs->vRingerTest;


        /* neg freq is for special output */
        if (ppInputs->freq < 0) {
            pTempRinger->negative = TRUE;
        } else {
            pTempRinger->negative = FALSE;
        }

        if ( (pTempRinger->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST) ||
             (pTempRinger->input.ringerTestType == LT_RINGER_REGULAR_PHNE_TEST_3_ELE) ) {

            int32 freq = ABS(ppInputs->freq);

            if ((freq > LT_VP880_MAX_FREQ) || (freq < LT_VP880_MIN_FREQ)) {
                LT_DOUT(LT_DBG_ERRR, ("Vp880RingerAttributesCheck(freq:%li) : "\
                    "invalid", ppInputs->freq));
                return FALSE;
            }
        }
        pTempRinger->input.freq = ABS(ppInputs->freq);

        pTempRinger->input.renFactor = ABS(ppInputs->renFactor);
    }

    /* critera check */
    if (VP_NULL == ppCriteria) {
        pTempRinger->criteria.renHigh = LT_RINGERS_CRT_REN_HIGH;
        pTempRinger->criteria.renLow = LT_RINGERS_CRT_REN_LOW;
    } else {
        pTempRinger->criteria.renHigh = ppCriteria->renHigh;
        pTempRinger->criteria.renLow = ppCriteria->renLow;
    }

    /* select the proper function handler */
    if ((LT_RINGER_REGULAR_PHNE_TEST == pTempRinger->input.ringerTestType) ||
        (LT_RINGER_REGULAR_PHNE_TEST_3_ELE == pTempRinger->input.ringerTestType)) {
        pTestCtx->TestEventHandlerFunc = LtVp880RingerHandler;
    } else {
        pTestCtx->TestEventHandlerFunc = LtVp880CapRingerHandler;

    }
    return TRUE;
}
#endif /* LT_RINGERS */

#ifdef LT_RES_FLT
static bool
Vp880ResFltAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtResFltCriteriaType *ppCriteria =
        (LtResFltCriteriaType*)pTemp->attributes.pCriteria;
    LtVp880ResFltTempType *pTempResFlt = &pTemp->tempData.resFlt;
    LtResFltInputType *ppInputs = (LtResFltInputType*)pTemp->attributes.pInputs;

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
    pTestCtx->TestEventHandlerFunc = LtVp880ResFltHandler;
    return TRUE;
}
#endif /* LT_RES_FLT */

#ifdef LT_MSOCKET
static bool
Vp880MSocketAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880MSocketTempType *pTempMSocket = &pTemp->tempData.mSocket;
    LtMSocketInputType *ppInputs = (LtMSocketInputType*)pTemp->attributes.pInputs;

    /* If the inputs are not defined then save defaults into temp structure */
    if (VP_NULL == ppInputs) {
        LT_DOUT(LT_DBG_ERRR, ("Vp880MSocketAttributesCheck(): Socket type is mandatory"));
        return FALSE;
    } else {
        /* Save input data into temp structure */
        pTempMSocket->input.mSocket = ppInputs->mSocket;
    }

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880MSocketHandler;
    return TRUE;
}
#endif /* LT_MSOCKET */

#ifdef LT_XCONNECT
static bool
Vp880XConnectAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880XConnectTempType *pTempXConnect = &pTemp880->tempData.xConnect;

    pTempXConnect->input.dummy = 0;
    pTempXConnect->criteria.dummy = 0;

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880XConnectHandler;
    return TRUE;
}
#endif /* LT_XCONNECT */

#ifdef LT_CAP
static bool
Vp880CapAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880CapTempType *pTempCap = &pTemp->tempData.cap;
    LtCapInputType *ppInputs = (LtCapInputType*)pTemp->attributes.pInputs;

    /* If the inputs are not defined then save defaults into temp structure */
    if (VP_NULL == ppInputs) {
        pTempCap->input.testAmp = LT_CAP_DFLT_TEST_AMPL;
        pTempCap->input.testFreq = LT_CAP_DFLT_TEST_FREQ;
        pTempCap->input.tipCapCalValue = LT_CAP_DFLT_TIP_CAL;
        pTempCap->input.ringCapCalValue = LT_CAP_DFLT_RING_CAL;
    } else {
        /* Check for valid test freqs */
        if (ppInputs->testFreq >= LT_CAP_FREQ_ENUM_SIZE) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880CapAttributesCheck(testFreq:%i) : "\
                "invalid", ppInputs->testFreq));
            return FALSE;
        }

        /* check validity of input test amplitude */
        if ((ppInputs->testAmp < LT_VP880_CAP_MIN_AMP) ||
            (ppInputs->testAmp > LT_VP880_CAP_MAX_AMP)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880CapAttributesCheck(testAmp:%li) : "\
                "invalid", ppInputs->testAmp));
            return FALSE;
        }

        /* check validity of input tip->ground calibration value */
        if ((ppInputs->tipCapCalValue != LT_CAP_MANUFACTURING_CAL) &&
            ( (ppInputs->tipCapCalValue < LT_VP880_CAP_MIN_CAL) ||
              (ppInputs->tipCapCalValue > LT_VP880_CAP_MAX_CAL)) ) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880CapAttributesCheck(tipCapCalValue:%li) : "\
                "invalid", ppInputs->tipCapCalValue));
            return FALSE;
        }

        /* check validity of input ring->ground calibration value */
        if ((ppInputs->ringCapCalValue != LT_CAP_MANUFACTURING_CAL) &&
            ( (ppInputs->ringCapCalValue < LT_VP880_CAP_MIN_CAL) ||
              (ppInputs->ringCapCalValue > LT_VP880_CAP_MAX_CAL)) ) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880CapAttributesCheck(ringCapCalValue:%li) : "\
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
    pTestCtx->TestEventHandlerFunc = LtVp880CapHandler;
    return TRUE;
}
#endif /* LT_CAP */

#if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)
static bool
Vp880909AllAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtVp880AllGr909TestTempType *pTempAllGr909 = &pTemp->allGr909;
    LtAllGr909InputType gr909Input;
    LtAllGr909CriteriaType gr909Criteria;

    /* is this really a pointer */
    void *pOrigInputs = pTemp->attributes.pInputs;
    void *pOrigCriteria = pTemp->attributes.pCriteria;

    if (VP_NULL != pOrigInputs) {
        LtMemCpy(&gr909Input, pOrigInputs, sizeof(LtAllGr909InputType));
    }
    if (VP_NULL != pOrigCriteria) {
        LtMemCpy(&gr909Criteria, pOrigCriteria, sizeof(LtAllGr909CriteriaType));
    }

    /* ResFlts argument checking */
    if (VP_NULL != pOrigInputs) {
        pTemp->attributes.pInputs = &gr909Input.resFltInp;
    }
    if (VP_NULL != pOrigCriteria) {
        pTemp->attributes.pCriteria = &gr909Criteria.resFltCrt;
    }
    if (FALSE == LtVp880AttributeCheck(pTestCtx, LT_TID_RES_FLT)) {
        return FALSE;
    }
    LtMemCpy(&pTempAllGr909->input.resFltInp, &pTemp->tempData.resFlt.input,
        sizeof(LtResFltInputType));

    LtMemCpy(&pTempAllGr909->criteria.resFltCrt, &pTemp->tempData.resFlt.criteria,
        sizeof(LtResFltCriteriaType));

    pTempAllGr909->ResFltHandlerFnPtr = pTestCtx->TestEventHandlerFunc;

    /* Ren argument checking */
    if (VP_NULL != pOrigInputs) {
        pTemp->attributes.pInputs = &gr909Input.ringersInp;
    }
    if (VP_NULL != pOrigCriteria) {
        pTemp->attributes.pCriteria = &gr909Criteria.ringersCrt;
    }
    if (FALSE == LtVp880AttributeCheck(pTestCtx, LT_TID_RINGERS)) {
        return FALSE;
    }
    LtMemCpy(&pTempAllGr909->input.ringersInp, &pTemp->tempData.ringers.input,
        sizeof(LtRingerInputType));

    LtMemCpy(&pTempAllGr909->criteria.ringersCrt, &pTemp->tempData.ringers.criteria,
        sizeof(LtRingersCriteriaType));

    pTempAllGr909->RingerHandlerFnPtr = pTestCtx->TestEventHandlerFunc;

    /* ROH argument checking */
    if (VP_NULL != pOrigInputs) {
        pTemp->attributes.pInputs = &gr909Input.rohInp;
    }
    if (VP_NULL != pOrigCriteria) {
        pTemp->attributes.pCriteria = &gr909Criteria.rohCrt;
    }
    if (FALSE == LtVp880AttributeCheck(pTestCtx, LT_TID_ROH)) {
        return FALSE;
    }
    LtMemCpy(&pTempAllGr909->input.rohInp, &pTemp->tempData.roh.input,
        sizeof(LtRohInputType));

    LtMemCpy(&pTempAllGr909->criteria.rohCrt, &pTemp->tempData.roh.criteria,
        sizeof(LtRohCriteriaType));

    pTempAllGr909->RohHandlerFnPtr = pTestCtx->TestEventHandlerFunc;

    /* Line V argument checking */
    if (VP_NULL != pOrigInputs) {
        pTemp->attributes.pInputs = &gr909Input.lineVInp;
    }
    if (VP_NULL != pOrigCriteria) {
        pTemp->attributes.pCriteria = &gr909Criteria.lintVCrt;
    }
    if (FALSE == LtVp880AttributeCheck(pTestCtx, LT_TID_LINE_V)) {
        return FALSE;
    }

    LtMemCpy(&pTempAllGr909->input.lineVInp, &pTemp->tempData.lineV.input,
        sizeof(LtLineVInputType));

    LtMemCpy(&pTempAllGr909->criteria.lintVCrt, &pTemp->tempData.lineV.criteria,
        sizeof(LtLineVCriteriaType));

    pTempAllGr909->LineVHandlerFnPtr = pTestCtx->TestEventHandlerFunc;

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880AllGr909Handler;
    return TRUE;
}
#endif /* LT_ALL_GR_909 */

#ifdef LT_LOOPBACK
static bool
Vp880LoopbackAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp = &pTestCtx->pTemp->vp880Temp;
    LtLoopbackInputType *ppInputs =
        (LtLoopbackInputType*)pTemp->attributes.pInputs;
    LtVp880LoopbackTempType *pTempLoopback = &pTemp->tempData.loopback;
    uint32 time;

    /* If the inputs are not defined then save defaults into temp structure */
    if (VP_NULL == ppInputs) {
        time = LT_LOOPBACK_DFLT_WAIT_TIME/125;
        pTempLoopback->vpApiInput.waitTime =  (uint16)time;

        time = LT_LOOPBACK_DFLT_TEST_TIME/125;
        pTempLoopback->vpApiInput.loopbackTime =  (uint16)time;

        if (LT_LOOPBACK_CODEC == LT_LOOPBACK_DFLT_TEST_TYPE) {
            pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_CODEC;
        } else {
            pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_ANALOG;
        }
    } else {

        /* Check for valid loopbacks */
        if (LT_LOOPBACK_TEST_NUM_TYPES <= ppInputs->loopbackType) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880LoopbackAttributesCheck(loopbackType:%i) : "\
                "invalid", ppInputs->loopbackType));
            return FALSE;
        }

        /* Ensure that the max wait and loopback times are not exceeded */
        if ((LT_LOOPBACK_MAX_TIME < ppInputs->waitTime) ||
            (LT_LOOPBACK_MAX_TIME < ppInputs->loopbackTime) ) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880LoopbackAttributesCheck(waitTime:%li||"\
                "loopbackTime:%li) : invalid", ppInputs->waitTime,
                ppInputs->loopbackTime));
            return FALSE;
        }

        /* Save input data into temp structure */
        time = ppInputs->waitTime/125;
        pTempLoopback->vpApiInput.waitTime =  (uint16)time;

        time = ppInputs->loopbackTime/125;
        pTempLoopback->vpApiInput.loopbackTime =  (uint16)time;

        switch (ppInputs->loopbackType) {
            case LT_LOOPBACK_CODEC:
                pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_CODEC;
                break;

            case LT_LOOPBACK_ANALOG:
                pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_ANALOG;
                break;

            case LT_LOOPBACK_BFILTER:
                pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_BFILTER;
                break;

            case LT_LOOPBACK_TIMESLOT:
                pTempLoopback->vpApiInput.loopback = VP_LOOPBACK_TEST_TIMESLOT;
                break;

            default:
                break;
        }
    }

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880LoopbackHandler;
    return TRUE;
}
#endif /* LT_LOOPBACK */

#ifdef LT_DC_FEED_ST
static bool
Vp880DcFeedAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtDcFeedSTInputType *ppInputs  =
        (LtDcFeedSTInputType*)pTemp880->attributes.pInputs;
    LtDcFeedSTCriteriaType *ppCriteria =
        (LtDcFeedSTCriteriaType*)pTemp880->attributes.pCriteria;
    LtVp880DcFeedSTTempType *pTempDcFeedSt = &pTemp880->tempData.dcFeedST;

    /* Check input args */
    if (VP_NULL == ppInputs) {
        pTempDcFeedSt->input.lineState = LT_DC_FEED_ST_DFLTI_LINE_STATE;
    } else {
        pTempDcFeedSt->input.lineState = ppInputs->lineState;
    }

    if (pTempDcFeedSt->input.lineState == VP_LINE_DISCONNECT ||
        pTempDcFeedSt->input.lineState == VP_LINE_RINGING)
    {
        return FALSE;
    }

    /* Check criteria args*/
    if (VP_NULL == ppCriteria) {
        pTempDcFeedSt->criteria.rloadErr = LT_DC_FEED_ST_CRT_IMP_PERCENT;
    } else {
        if ((LT_0_PERCENT >= ppCriteria->rloadErr) ||
            (LT_100_PERCENT < ppCriteria->rloadErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880DcFeedAttributeCheck(rloadErr:%li) : "\
                "invalid", ppCriteria->rloadErr));
            return FALSE;
        }
        pTempDcFeedSt->criteria.rloadErr = ppCriteria->rloadErr;
    }

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880DcFeedHandler;
    return TRUE;
}
#endif /* LT_DC_FEED_ST */

#ifdef LT_RD_LOOP_COND
static bool
Vp880RdLoopCondAttributeCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtVp880RdLoopCondTempType *pTempRdLoopCond = &pTemp880->tempData.rdLoopCond;

    pTempRdLoopCond->input.lineState = LT_RD_LOOP_COND_DFLTI_LINE_STATE;
    pTempRdLoopCond->criteria.rloadErr = LT_RD_LOOP_COND_CRT_IMP_PERCENT;

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880RdLoopCondHandler;
    return TRUE;
}
#endif /* LT_RD_LOOP_COND */

#ifdef LT_DC_VOLTAGE
static bool
Vp880DcVoltageAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtDcVoltageInputType *ppInputs  =
        (LtDcVoltageInputType*)pTemp880->attributes.pInputs;
    LtDcVoltageCriteriaType *ppCriteria  =
        (LtDcVoltageCriteriaType*)pTemp880->attributes.pCriteria;
    LtVp880DcVoltageTempType *pTempDcVolt = &pTemp880->tempData.dcVoltage;

    /* Check input args*/
    if (VP_NULL == ppInputs) {
        pTempDcVolt->input.testVoltage = LT_DC_VOLTAGE_DFLT_SIGNAL;
    } else {
        /* Check for valid input */
        if (LT_VP880_MAX_VPK_VOLTAGE < ppInputs->testVoltage) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880DcVoltageAttributesCheck(testVoltage:%li) : "\
                "invalid", ppInputs->testVoltage));
            return FALSE;
        }
        pTempDcVolt->input.testVoltage = ppInputs->testVoltage;
    }

    /* Input voltage must be positive */
    pTempDcVolt->input.testVoltage = ABS(pTempDcVolt->input.testVoltage);

    /* Check criteria args*/
    if (VP_NULL == ppCriteria) {
        pTempDcVolt->criteria.voltageErr = LT_DC_VOLTAGE_DFLTC_VOLTAGE_TOL;
    } else {
        /* Check for valid criteria >=0% and <100% */
        if ((LT_0_PERCENT >= ppCriteria->voltageErr) ||
            (LT_100_PERCENT < ppCriteria->voltageErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880DcVoltageAttributesCheck(voltageErr:%li) : "\
                "invalid", ppCriteria->voltageErr));
            return FALSE;
        }
        pTempDcVolt->criteria.voltageErr = ppCriteria->voltageErr;
    }

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880DcVoltageHandler;
    return TRUE;
}
#endif /* LT_DC_VOLTAGE */

#ifdef LT_RINGING_ST
static bool
Vp880RingingStAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtRingingSTInputType *ppInputs  =
        (LtRingingSTInputType*)pTemp880->attributes.pInputs;
    LtRingingSTCriteriaType *ppCriteria  =
        (LtRingingSTCriteriaType*)pTemp880->attributes.pCriteria;
    LtVp880RingingStTempType *pTempRingingSt = &pTemp880->tempData.ringingSt;

    /* Check input args*/
    if (VP_NULL == ppInputs) {
        pTempRingingSt->input.freq = LT_RINGING_ST_DFLTI_FREQ;
        pTempRingingSt->input.vRinging = LT_RINGING_ST_DFLTI_VRING;
        pTempRingingSt->input.pRingProfileForTest = LT_RINGING_ST_DFLTI_PROFILE;
        pTempRingingSt->input.pDcProfileForTest = LT_RINGING_ST_DFLTI_PROFILE;
    } else {
        /* make sure the input test v is legal */
        LtVoltageType maxRms = VP880_UNIT_CONVERT(LT_VP880_RINGING_ST_MAX_INPUTV,
                VP880_UNIT_MV, VP880_UNIT_MVRMS);

        /* make sure the ring freq is legal */
        if ((ppInputs->freq > LT_VP880_MAX_FREQ) || (ppInputs->freq < LT_VP880_MIN_FREQ)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880RingingStAttributesCheck(freq:%li) : "\
                "invalid", ppInputs->freq));
            return FALSE;
        }
        pTempRingingSt->input.freq = ABS(ppInputs->freq);

        /* make sure the input test v is legal */
        if (maxRms < ppInputs->vRinging) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880RingingStAttributesCheck(vRinging:%li) : "\
                "invalid", ppInputs->vRinging));
            return FALSE;
        }
        pTempRingingSt->input.vRinging = ppInputs->vRinging;

        /* store the profile pointers */
        pTempRingingSt->input.pRingProfileForTest = ppInputs->pRingProfileForTest;
        pTempRingingSt->input.pDcProfileForTest = ppInputs->pDcProfileForTest;
    }

    /* Check Criteria args */
    if (VP_NULL == ppCriteria) {
        pTempRingingSt->criteria.openVoltageErr = LT_RINGING_ST_DFLTC_VRING;
        pTempRingingSt->criteria.freqErr = LT_RINGING_ST_DFLTC_FREQ;
        pTempRingingSt->criteria.rLoadErr = LT_RINGING_ST_DFLTC_RLOAD;
    } else {
        /* make sure the open circuit voltage error is legal */
        if ((LT_0_PERCENT >= ppCriteria->openVoltageErr) ||
            (LT_100_PERCENT < ppCriteria->openVoltageErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880RingingStAttributesCheck(openVoltageErr:%li) : "\
                "invalid", ppCriteria->openVoltageErr));
            return FALSE;
        }
        pTempRingingSt->criteria.openVoltageErr = ppCriteria->openVoltageErr;

        /* make sure the ring freq error is legal */
        if ((LT_0_PERCENT >= ppCriteria->freqErr) ||
            (LT_100_PERCENT < ppCriteria->freqErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880RingingStAttributesCheck(freqErr:%li) : "\
                "invalid", ppCriteria->freqErr));
            return FALSE;
        }
        pTempRingingSt->criteria.freqErr = ppCriteria->freqErr;

        /* make sure the r load error is legal */
        if ((LT_0_PERCENT >= ppCriteria->rLoadErr) ||
            (LT_100_PERCENT < ppCriteria->rLoadErr)) {
            LT_DOUT(LT_DBG_ERRR, ("Vp880RingingStAttributesCheck(rLoadErr:%li) : "\
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
        (int16)VP880_UNIT_CONVERT(pTempRingingSt->input.vRinging,
            VP880_UNIT_MVRMS, VP880_UNIT_DAC_RING);
    pTempRingingSt->vpApiAcRloopInput.settlingTime = LT_VP880_RINGING_ST_SETTLE_TIME;
    pTempRingingSt->vpApiAcRloopInput.integrateTime = (uint16)
        ((LT_VP880_RINGING_ST_RING_CYCLES * 8000000L) / pTempRingingSt->input.freq);

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880RingingStHandler;
    return TRUE;
}
#endif /* LT_RINGING_ST */

#ifdef LT_ON_OFF_HOOK_ST
static bool
Vp880OnOffHookStAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    LtVp880TestTempType *pTemp880 = &pTestCtx->pTemp->vp880Temp;
    LtOnOffHookSTInputType *ppInputs  =
        (LtOnOffHookSTInputType*)pTemp880->attributes.pInputs;
    LtVp880OnOffHookStTempType *pTempHookSt = &pTemp880->tempData.onOffHookSt;

    /* Check input args*/
    if (VP_NULL == ppInputs) {
        pTempHookSt->input.overrideOffHook = LT_ON_OFF_HOOK_ST_DFLT_OVRRD_FLG;
    } else {
        /*
         * Check the override bit. Currently there are no VP880 termination
         * types that support this bit so error out.
         */
        if (TRUE != ppInputs->overrideOffHook) {
            pTempHookSt->input.overrideOffHook = ppInputs->overrideOffHook;
        } else {
            LT_DOUT(LT_DBG_ERRR, ("Vp880OnOffHookStAttributesCheck(overrideOffHook:%i) : "\
                "invalid", ppInputs->overrideOffHook));
            return FALSE;
        }
    }

    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880OnOffHookStHandler;
    return TRUE;
}
#endif /* LT_ON_OFF_HOOK_ST */

#ifdef LT_RD_BAT_COND
static bool
Vp880RdBatCondAttributesCheck(
    LtTestCtxType *pTestCtx)
{
    /* setup function Handler */
    pTestCtx->TestEventHandlerFunc = LtVp880RdBatCondHandler;
    return TRUE;
}
#endif /* LT_RD_BAT_COND */

#endif /* LT_VP880_VVP_PACKAGE */
#endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */
