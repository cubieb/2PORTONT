/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10733 $
 * $LastChangedDate: 2013-01-28 10:44:52 -0600 (Mon, 28 Jan 2013) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"
#include "vp_api_int.h"


#define FCT_MAX                         100     /* max factorization factor */
#ifndef ABS
    #define ABS(a) ( ((a) < 0) ? -(a) : (a))
#endif

typedef enum
{
    CAP_3ELE_SETUP              = 0,
    CAP_3ELE_MEASURE            = 1,
    CAP_3ELE_GET_DATA           = 2,
    CAP_3ELE_RESTORE            = 3,
    CAP_3ELE_GEN_EVENT          = 4,
    CAP_3ELE_MAX                = 4,
    CAP_3ELE_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
} Vp886Cap3EleTypes;

typedef enum
{
    ELE3_CAP_VA,
    ELE3_CAP_VB,
    ELE3_CAP_IMT,
    ELE3_CAP_ILG,
    ELE3_CAP_MAX_ADC,
    ELE3_CAP_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vp3EleCapAdcOrderTypes;


bool Vp886Test3EleCapSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTest3EleCapAltResType *p3EleCapArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static bool
CapSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);


static void
AnalyzeSamples(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 numSamples,
    int16 *data);

/*------------------------------------------------------------------------------
 * Vp886Test3EleCap
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886Test3EleCap(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTest3EleCapAltResType *p3EleCapArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTest3EleCapAltResType *pInputArgs = (VpTest3EleCapAltResType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886Test3EleCap NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }
        pTestHeap->prevVal_1 = 0; /*used to store numSamples Count*/
        /* reset the results to nonmeaningful values */
        VpMemSet(&pLineObj->testResults.result.capAlt, 0, sizeof(VpTestResultAltCapType));

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.capAlt, pInputArgs, sizeof(VpTest3EleCapAltResType));

        /* set for first state */
        pTestHeap->nextState = CAP_3ELE_SETUP;
    }

    p3EleCapArgs = &pTestHeap->testArgs.capAlt;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886Test3EleCapSM(pLineCtx, pLineObj, p3EleCapArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886Test3EleCapSM
 *----------------------------------------------------------------------------*/
bool Vp886Test3EleCapSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTest3EleCapAltResType *p3EleCapArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886Test3EleCapSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {
        case CAP_3ELE_SETUP: {
            pTestHeap->adcState = ELE3_CAP_VA;
            if (CapSetup(pLineCtx, pTestHeap)==FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886Test3EleCapSM failed setup"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            }
            /* wait some time to settle */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 100, 0, CAP_3ELE_SETUP)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("CAP_3ELE_SETUP: Cannot add to timer queue."));
            }
            pTestHeap->nextState = CAP_3ELE_MEASURE;
            break;
        }
        case CAP_3ELE_MEASURE: {
            Vp886SadcRateType rate;
            Vp886SadcSignalType sig;
            uint16 collectFrames;
            uint16 skipFrames;

            VP_TEST(VpLineCtxType, pLineCtx, ("CAP_3ELE_MEASURE: freq=%u", p3EleCapArgs->testFreq));

            if (p3EleCapArgs->testFreq == 4) {
                /* At 280Hz, collect 50 samples at 2kHz, for a total of 7
                   cycles over 25ms. */
                collectFrames = 50 * 4;
                rate = VP886_SADC_RATE_QUAR;
                /* Remember the number of collected samples */
                pTestHeap->prevVal_1 = 50;
            } else {
                /* At other frequencies (333, 667, 1333, 2667Hz), collect 48
                   samples at 8kHz, for a total of 2, 4, 8, or 16 cycles over
                   6ms. */
                collectFrames = 48;
                rate = VP886_SADC_RATE_FULL;
                /* Remember the number of collected samples */
                pTestHeap->prevVal_1 = 48;
            }

            skipFrames = 16;
            switch (pTestHeap->adcState) {
                case ELE3_CAP_VA:
                    sig = VP886_SADC_SEL_TIP;
                    skipFrames = 160;
                    break;
                case ELE3_CAP_VB:
                    sig = VP886_SADC_SEL_RING;
                    break;
                case ELE3_CAP_IMT:
                    sig = VP886_SADC_SEL_MET_I;
                    break;
                case ELE3_CAP_ILG:
                    sig = VP886_SADC_SEL_LONG_I;
                    break;
                default:
                    sig = VP886_SADC_SEL_TIP;
                    VP_ERROR(VpLineCtxType, pLineCtx, ("CAP_3ELE_MEASURE bad adcState %d", pTestHeap->adcState));
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                    break;
            }

            Vp886SadcSetup(pLineCtx, VP886_SADC_RAW_SINGLE, rate,
                sig,
                VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN,
                skipFrames,
                collectFrames,
                VP886_LINE_TEST);

            pTestHeap->nextState = CAP_3ELE_GET_DATA;
            break;
        }
        case CAP_3ELE_GET_DATA: {
            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
            Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
            int16 offset = pCalData->tipSense.offset;
            int16 gain = pCalData->tipSense.gain;
            int16 testDataBuf[60];
            uint8 numSamples;

            switch (pTestHeap->adcState) {
                case ELE3_CAP_VA:
                    offset = pCalData->tipSense.offset;
                    gain = pCalData->tipSense.gain;
                    break;
                case ELE3_CAP_VB:
                    offset = pCalData->ringSense.offset;
                    gain = pCalData->ringSense.gain;
                case ELE3_CAP_IMT:
                case ELE3_CAP_ILG:
                    offset = pCalData->sadc.offset;
                    gain = pCalData->sadc.gain;
                    break;
                default:
                    VP_ERROR(VpLineCtxType, pLineCtx, ("CAP_3ELE_GET_DATA Invalid adcState"));
                    break;
            }

            /*collect the samples*/
            numSamples = Vp886SadcGetRawData(pLineCtx, testDataBuf, pTestHeap->prevVal_1, offset, gain);
            VP_TEST(VpLineCtxType, pLineCtx, ("number of collected samples =%u", numSamples));

            if (numSamples < pTestHeap->prevVal_1) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("CAP_3ELE_GET_DATA Wrong number of samples collected, %d", numSamples));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /*Calculate the rms value of VA, VB, IM, or IL*/
            AnalyzeSamples(pLineCtx, pTestHeap, numSamples, testDataBuf);

            if (++(pTestHeap->adcState) < ELE3_CAP_MAX_ADC) {
                pTestHeap->nextState = CAP_3ELE_MEASURE;
            } else {
                pTestHeap->nextState = CAP_3ELE_GEN_EVENT;
            }
            requestNextState = TRUE;
            break;
        }

        case CAP_3ELE_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886Test3EleCapSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

/*------------------------------------------------------------------------------
 * CapSetup()
 *
 * This function is used to configure the line for the Low Gain Res Flt test.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object
 *  pLineObj        - pointer to the line object
 *  pTestHeap       - pointer to the test heap
 *  deviceId        - device ID
 *  ecVal           - current enable channel value
 *
 * Returns:
 * --
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
CapSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpTest3EleCapAltResType *pCapData = &(pTestHeap->testArgs).capAlt;
    uint8 sigGenA[VP886_R_RINGGEN_LEN] =  /* set 0V */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 switchRegParam[VP886_R_SWPARAM_LEN];
    uint8 sigGenCtrl = VP886_R_SIGCTRL_EN_BIAS | VP886_R_SIGCTRL_EN_SIGA;
    uint8 vpGain = 0x40;                            /* AX = 1 */
    uint8 vadcCtrl[VP886_R_VADC_LEN];
    uint16 audioAmp;
    int16 bias;
    uint16 freq = 0;
    uint8 dcfeed[VP886_R_DCFEED_LEN];
    pTestHeap->compensate = FALSE;

    if (Vp886CalmodeTestSetup(pLineCtx, pTestHeap)==FALSE) {
        return FALSE;
    }

    /* Enable metallic drive speed-up */
    pLineObj->registers.icr2[0] = 0x20;
    pLineObj->registers.icr2[1] = 0x20;
    pLineObj->registers.icr2[2] &= 0x0C;
    pLineObj->registers.icr2[2] |= 0x80;
    pLineObj->registers.icr2[3] &= 0x0C;
    pLineObj->registers.icr2[3] |= 0x80;

    if (pCapData->bias > 0) {               /* Measuring ring->ground */
        pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC |
                                           VP886_R_STATE_POL |
                                           VP886_R_STATE_SS_UNBAL_RING);
        pLineObj->registers.icr3[0] = 0x04;
        pLineObj->registers.icr3[1] = 0x04;     /* Reverse longitudinal drive */
    } else {                                /* Measuring tip->ground */
        pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC |
                                           VP886_R_STATE_SS_UNBAL_RING);
        pLineObj->registers.icr3[0] = 0x04;
        pLineObj->registers.icr3[1] = 0x00;
    }

    /* Audio amplitude of the test signal */
    /* audioAmp = Vscale(120680) * RmsAudioAmp / (1000 * Rsense(402)) */
    audioAmp = pCapData->testAmp * 3 / 10;
    sigGenA[5] = (uint8)((audioAmp >> 8) & 0x00FF);
    sigGenA[6] = (uint8)(audioAmp & 0x00FF);

    /* offset of the test signal */
    bias = (ABS(pCapData->bias) + audioAmp) * -1;
    sigGenA[1] = (uint8)((bias >> 8) & 0x00FF);
    sigGenA[2] = (uint8)(bias & 0x00FF);

    /* test frequency */
    if (pCapData->testFreq < 4) {
        freq = 0x1C72 >> pCapData->testFreq; /* 333.252Hz - 2666.74Hz */
    } else {
        freq = 0x02FD; /* 280.151Hz */
        pCapData->testFreq = 4;
    }
    VP_TEST(VpLineCtxType, pLineCtx, ("amp %u, freq=%u, freqi=%u", audioAmp, freq, pCapData->testFreq));

    sigGenA[3] = (uint8)((freq >> 8) & 0x00FF);
    sigGenA[4] = (uint8)(freq & 0x00FF);

    /* LI = 0 (100 Ohms / leg), VOC = 12V */
    dcfeed[0] = 0x21;
    dcfeed[1] = 0xC2;

    /* Take control of the VADC and disable it */
    VpMemSet(vadcCtrl, 0, VP886_R_VADC_LEN);
    vadcCtrl[0] |= VP886_R_VADC_SM_OVERRIDE;
    vadcCtrl[1] |= VP886_R_VADC_SEL_ADC_OFFSET;

    if (VP886_IS_TRACKER(pDevObj)) {
        /* Regular tracker - force a fixed voltage -60V */
        VpSlacRegRead(NULL, pLineCtx, VP886_R_SWPARAM_RD, VP886_R_SWPARAM_LEN, pLineObj->registers.swParam);

        switchRegParam[0] = pLineObj->registers.swParam[0] | 0x40 | 0x04;
        switchRegParam[1] = 0x0B;
        switchRegParam[2] = pLineObj->registers.swParam[2];
        VP_TEST(VpLineCtxType, pLineCtx, ("CapSetup(): switchRegParam[] = (%i, %i, %i),", switchRegParam[0], switchRegParam[1], switchRegParam[2]));

        VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, switchRegParam);
        VpMemCpy(pLineObj->registers.swParam, switchRegParam, VP886_R_SWPARAM_LEN);
    }

    VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_RINGGEN_WRT, VP886_R_RINGGEN_LEN, sigGenA);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_VPGAIN_WRT, VP886_R_VPGAIN_LEN, &vpGain);

    if (VP886_IS_ABS(pDevObj)) {
        /* Force Low voltage on ABS */
        pLineObj->registers.icr1[2] |= 0x30;
        pLineObj->registers.icr1[3] &= 0xCF;
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1);
    }

    VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_VADC_WRT, VP886_R_VADC_LEN, vadcCtrl);

    pLineObj->testResults.result.capAlt.freq = pCapData->testFreq;
    return TRUE;
}


/*------------------------------------------------------------------------------
 * AnalyzeSamples()
 *
 * This function will process the test data buffer to compute the RMS values
 * of: va, vb, im, il.
 * If the test data buffer is not full, this function will request the next
 * buffer and concatenate the samples.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pTestState      - pointer to the next test state
 *  pTimer          - number of ticks required after calling this function
 *
 * Returns:
 * nothing
 *
 * Result:
 *  This function stores the RMS values in the result structure.
 *
 *----------------------------------------------------------------------------*/
static void
AnalyzeSamples(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    uint8 numSamples,
    int16 *data)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpTest3EleCapAltResType *pCapData = &pTestHeap->testArgs.capAlt;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint8 smp;
    int16 adcSample;
    uint16 amplitude;
    int32 cGoertzel;
    int32 gm1 = 0;
    int32 gm2 = 0;
    int32 acc = 0;
    int32 scaleFactorM;
    int32 scaleFactorD;
    int32 factor1 = 1;
    int32 factor2 = 1;

    switch (pCapData->testFreq) {
        case 0:
            /* Fgenerator = 2666.74Hz, Fsample = 8000Hz */
            cGoertzel = -4096;
            break;
        case 1:
            /* Fgenerator = 1333.37Hz, Fsample = 8000Hz */
            cGoertzel = 4096;
            break;
        case 2:
            /* Fgenerator = 666.50Hz, Fsample = 8000Hz */
            cGoertzel = 7095;
            break;
        case 3:
            /* Fgenerator = 333.25Hz, Fsample = 8000Hz */
            cGoertzel = 7913;
            break;
        default:
            /* Fgenerator = 280.151Hz, Fsample = 2000Hz */
            cGoertzel = 5219;
            break;
    }

#if 0
    for (smp = 0 ; smp < numSamples; smp++) {
        VP_TEST(VpLineCtxType, pLineCtx, ("ADC(%d):testdata word_%u=%i",*pAdcState, smp, *(data+smp)));
    }
#endif

    /* scaleFactor = 2^x, scaleFactorM for x>0 and scaleFactorD for x<0 */
    scaleFactorD = 1;
    scaleFactorM = 16;

    /* Use a Goertzel filter to calculate the signal amplitude VP_INT32_MAX */
    for (smp = 0 ; smp < numSamples ; smp++) {
        adcSample = *(data+smp);
        /* Goertzel differential equation */
        acc = scaleFactorM * (int32)adcSample / scaleFactorD + (cGoertzel * gm1 + 2048) / 4096 - gm2;
        if (ABS(acc) >= (int32)VP_INT16_MAX) {    /* acc greater than 2^15, reduce the scale factor */
            if (scaleFactorM == 1) {
                scaleFactorD *= 2;
            } else {
                scaleFactorM /= 2;
            }
            gm1 = (gm1 + 1) / 2;
            acc = (acc + 1) / 2;
        }
        gm2 = gm1;
        gm1 = acc;
    }
    gm2 = scaleFactorD * (gm2 + 8) / scaleFactorM;
    gm1 = scaleFactorD * (gm1 + 8) / scaleFactorM;

    /* Calculate filter power (check for a saturation) */
    if (gm1 != 0) {
        while (((ABS(gm1) / (factor1 * factor1)) > (VP_INT32_MAX / ABS(gm1))) && (factor1 < FCT_MAX)) {
            factor1++;
        }
    }
    if (gm2 != 0) {
        while (((ABS(gm2) / (factor2 * factor2)) > (VP_INT32_MAX / ABS(gm2))) && (factor2 < FCT_MAX)) {
            factor2++;
        }
    }
    if (factor2 > factor1) {
        factor1 = factor2;
    }

    factor2 = factor1;
    if (gm1 != 0) {
        while (((ABS(cGoertzel * (gm2 / (factor2 * factor2))) / 4096) > (VP_INT32_MAX / ABS(gm1))) && (factor2 < FCT_MAX)) {
            factor2++;
        }
        if (factor2 > factor1) {
            factor1 = factor2;
        }
    }

    if (ABS(gm2 / factor1) > (VP_INT32_MAX / ABS(cGoertzel))) {
        /* It will overflow in the equation -> saturates gm2*/
        if (gm2 > 0) {
            gm2 = VP_INT32_MAX / ABS(cGoertzel);
        } else {
            gm2 = VP_INT32_MIN / ABS(cGoertzel);
        }
        pLineObj->testResults.result.capAlt.accuracyFlag = TRUE;
    }

    if (factor1 == FCT_MAX) {
        /* the maximum is reached -> saturate this measurement */
        acc = VP_INT32_MAX / 2;
        pLineObj->testResults.result.capAlt.accuracyFlag = TRUE;
    } else {
        if ((scaleFactorD >= 16) && (((cGoertzel * (gm2 / factor1) / 4096) * (gm1 / factor1)) < 0)) {
            /* Each individual terms won't saturate, but the sum can -> increase the factor */
            factor1 *= 2;
        }
        acc = (gm2 / (factor1 * factor1)) * gm2 - (cGoertzel * (gm2 / factor1) / 4096) *
            (gm1 / factor1) + (gm1 / (factor1 * factor1)) * gm1;
    }

    /* first stage saturation check */
    while ((factor1 < FCT_MAX) && ((acc < 0) || (acc > (VP_INT32_MAX / 2)))) {
        factor1++;
        acc = (gm2 / (factor1 * factor1)) * gm2 - (cGoertzel * (gm2 / factor1) / 4096) *
            (gm1 / factor1) + (gm1 / (factor1 * factor1)) * gm1;
    }
    if (factor1 == FCT_MAX) {
        /* the maximum is reached -> saturate this measurement */
        acc = VP_INT32_MAX / 2;
        pLineObj->testResults.result.capAlt.accuracyFlag = TRUE;
    }

    acc *= 2;
    amplitude = factor1 * (VpComputeSquareRoot((uint32)acc) / (uint16)numSamples);
    VP_TEST(VpLineCtxType, pLineCtx, ("ADC(%d):number of samples in the local buffer nbSamples=%u",*pAdcState, numSamples));

    switch (*pAdcState) {
        case ELE3_CAP_VA:
            pLineObj->testResults.result.capAlt.va = amplitude;
            break;

        case ELE3_CAP_VB:
            pLineObj->testResults.result.capAlt.vb = amplitude;
            break;

        case ELE3_CAP_IMT:
            pLineObj->testResults.result.capAlt.im = amplitude;
            break;

        case ELE3_CAP_ILG:
            pLineObj->testResults.result.capAlt.il = amplitude;
            break;

        default:
            break;
    }


}



#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
