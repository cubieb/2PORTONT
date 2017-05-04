/** file vp_3ele_cap.c
 *
 *  This file contains the 3 ele capacitance primitive algorithm
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5608 $
 * $LastChangedDate: 2009-10-05 16:59:39 -0500 (Mon, 05 Oct 2009) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_890_SERIES)
#if defined (VP890_INCLUDE_TESTLINE_CODE)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp890_api.h"
#include "vp890_api_int.h"
#include "vp890_testline_int.h"

#define FEED_CHANGE_TIME_CONSTANT_SHORT 25      /* 25ms */
#define FCT_MAX                         100     /* max factorization factor */

#ifndef ABS
    #define ABS(a) ( ((a) < 0) ? -(a) : (a))
#endif

typedef enum
{
    COMMON_TEST_SETUP        = VP890_TESTLINE_GLB_STRT_STATE,
    ELE3_CAP_SETUP           = 5,
    ELE3_CAP_SETUP_ADC       = 10,
    ELE3_CAP_GET_DATA        = 15,
    ELE3_CAP_RESTORE         = 35,
    ELE3_CAP_END             = VP890_TESTLINE_GLB_END_STATE,
    ELE3_CAP_QUIT            = VP890_TESTLINE_GLB_QUIT_STATE,
    ELE3_CAP_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vp3EleCapTestStateTypes;

typedef enum
{
    ELE3_CAP_VAL,
    ELE3_CAP_VBL,
    ELE3_CAP_IMT,
    ELE3_CAP_ILG,
    ELE3_CAP_MAX_ADC,
    ELE3_CAP_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vp3EleCapAdcOrderTypes;

static void
Vp3EleCapGetData(
    VpLineCtxType *pLineCtx,
    int16 *pTestState,
    uint8 *pTimer);

static VpStatusType
Vp3EleCapArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
Vp3EleCapSetup(
    Vp890DeviceObjectType *pDevObj,
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static uint16
VpPcmComputeSquareRoot(
    uint32 number);

/*------------------------------------------------------------------------------
 * Vp890Test3EleCap
 * This functions implements the 3 Ele Res Test primitive ...
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  pArgsUntyped- pointer to the test specific inputs
 *  handle      - unique test handle
 *  callbak     - indicates if this function was called via a callback
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp890Test3EleCap(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    VpTest3EleCapAltResType *pCapData = &(pTestHeap->testArgs).capAlt;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint16 tick =  pDevObj->devProfileData.tickRate;

    /* VP_TEST(VpLineCtxType, pLineCtx, ("Vp890Test3EleCap(ts:%i)", *pTestState)); */
    if (FALSE == callback) {
        return Vp3EleCapArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    /* *pTestState = ELE3_CAP_QUIT; */

    switch (*pTestState) {
        case COMMON_TEST_SETUP:
            Vp890CommonTestSetup(pLineCtx, deviceId);
            /* no break */

        case ELE3_CAP_SETUP:
            Vp3EleCapSetup(pDevObj, pLineObj, pTestHeap, deviceId);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_CAP_SETUP_ADC,
                MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME, tick) );
            break;

        case ELE3_CAP_SETUP_ADC:{
            uint8 convConfig;

            switch (*pAdcState) {
                case ELE3_CAP_VAL:
                    convConfig = ((pCapData->testFreq << 4) & 0xF0) + 0x04;
                    break;

                case ELE3_CAP_VBL:
                    convConfig = ((pCapData->testFreq << 4) & 0xF0) + 0x05;
                    break;

                case ELE3_CAP_IMT:
                    convConfig = ((pCapData->testFreq << 4) & 0xF0) + 0x07;
                    break;

                case ELE3_CAP_ILG:
                    convConfig = ((pCapData->testFreq << 4) & 0xF0) + 0x08;
                    break;

                default:
                    break;
            }

            /* Program the converter configuration */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_WRT,
                VP890_CONV_CFG_LEN, &convConfig);

            /* Need to inform the API of the new sampling rate */
            pDevObj->txBufferDataRate = (pCapData->testFreq << 4);

            /* Wait 50ms for the ADC to settle */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_CAP_GET_DATA,
                MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME_SHORT, tick));
            break;
        }

        case ELE3_CAP_GET_DATA:{
            uint8 nbTick;

            Vp3EleCapGetData(pLineCtx, pTestState, &nbTick);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState, nbTick);
            break;
        }

        case ELE3_CAP_RESTORE:
            /* Restore registers */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_WRT,
                VP890_CONV_CFG_LEN, pTestHeap->saveConvConfig);
            pDevObj->txBufferDataRate = pTestHeap->saveConvConfig[0] & 0xF0;

            Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_CAP_END, NEXT_TICK);
            break;

        case ELE3_CAP_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890Test3EleCap(): complete"));
            break;

        case ELE3_CAP_QUIT:
            /* The test has eneded early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890Test3EleCap(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890Test3EleCap(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp3EleCapGetData
 * This functions is called by the Vp890Test3EleCap()
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
Vp3EleCapGetData(
    VpLineCtxType *pLineCtx,
    int16 *pTestState,
    uint8 *pTimer)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpTest3EleCapAltResType *pCapData = &(pTestHeap->testArgs).capAlt;
    uint8 *pAdcState = &pTestHeap->adcState;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    uint8 nbSamples;
    uint8 nbNewSamples;
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

    /* cGoertzel = 4096 * 2 * cos(2 * Pi * Fgenerator / Fsample) */
    if ((pDevObj->ecVal & VP890_WIDEBAND_MODE) == VP890_WIDEBAND_MODE) {
        if (pCapData->testFreq == 4) {
            /* signal: 280.151Hz / sampling: 1000Hz  */
            cGoertzel = -1542;
        } else {
            /* signal: 333.252Hz * N / sampling: 2000Hz * N  */
            cGoertzel = 4098;
        }
    } else {
        if (pCapData->testFreq == 4) {
            /* signal: 280.151Hz / sampling: 500Hz  */
            cGoertzel = -7611;
        } else {
            /* signal: 333.252Hz * N / sampling: 1000Hz * N  */
            cGoertzel = -4092;
        }
    }

    if (pTestHeap->xtraBuffer == TRUE) {
        /* Read the number of samples in the local buffer */
        nbSamples = (uint8)pTestHeap->adcSampleBuffer[0];

        /* Compute the number of samples to add */
        nbNewSamples = (pDevObj->testDataBuffer[0] >> 4) & 0x0F;
        if (nbNewSamples == 7) {
            VP_TEST(VpLineCtxType, pLineCtx, ("Impossible to collect back to back buffers"));
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            *pTestState = ELE3_CAP_QUIT;
            *pTimer = 1;
            return;
        }
        if ((pTestHeap->requestedSamples - nbSamples) <= nbNewSamples) {
            nbNewSamples = pTestHeap->requestedSamples - nbSamples;
        }

        /* Append the current data at the end of the local buffer */
        for (smp = 0 ; smp < (nbNewSamples * 2) ; smp += 2) {
            pTestHeap->adcSampleBuffer[1 + nbSamples + (smp / 2)] =
            (((int16)(pDevObj->testDataBuffer[2 + smp])) << 8) | (int16)(pDevObj->testDataBuffer[2 + smp + 1]);
        }

        /* Update the valid sample field in the local buffer */
        pTestHeap->adcSampleBuffer[0] = (int16)nbNewSamples + (int16)nbSamples;
        pTestHeap->xtraBuffer = FALSE;
    } else {
        /* Copy the test data buffer in the local buffer */
        nbSamples = (pDevObj->testDataBuffer[0] >> 4) & 0x0F;
        if (nbSamples == 7) {
            nbSamples = 6;
        }
        pTestHeap->adcSampleBuffer[0] = (int16)nbSamples;
        for (smp = 0 ; smp < (nbSamples * 2) ; smp += 2) {
            pTestHeap->adcSampleBuffer[1 + (smp / 2)] =
            (((int16)(pDevObj->testDataBuffer[2 + smp])) << 8) | (int16)(pDevObj->testDataBuffer[2 + smp + 1]);
        }
    }

    nbSamples = (uint8)pTestHeap->adcSampleBuffer[0];
    if (nbSamples < pTestHeap->requestedSamples) {
        /* We need more samples */
        if (nbSamples == 0) {
            VP_TEST(VpLineCtxType, pLineCtx, ("Data buffer empty, exit"));
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            *pTestState = ELE3_CAP_QUIT;
        } else {
            pTestHeap->xtraBuffer = TRUE;
            *pTestState = ELE3_CAP_GET_DATA;
        }
        *pTimer = 1;
        return;
    }

    /* scaleFactor = 2^x, scaleFactorM for x>0 and scaleFactorD for x<0 */
    scaleFactorD = 1;
    scaleFactorM = 16;

    /* Use a Goertzel filter to calculate the signal amplitude VP_INT32_MAX */
    for (smp = 1 ; smp < (1 + nbSamples) ; smp++) {
        adcSample = pTestHeap->adcSampleBuffer[smp];
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
        pDevObj->testResults.result.capAlt.accuracyFlag = TRUE;
    }

    if (factor1 == FCT_MAX) {
        /* the maximum is reached -> saturate this measurement */
        acc = VP_INT32_MAX / 2;
        pDevObj->testResults.result.capAlt.accuracyFlag = TRUE;
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
        pDevObj->testResults.result.capAlt.accuracyFlag = TRUE;
    }

    acc *= 2;
    amplitude = factor1 * (VpPcmComputeSquareRoot((uint32)acc) / (uint16)nbSamples);

    switch (*pAdcState) {
        case ELE3_CAP_VAL:
            pDevObj->testResults.result.capAlt.va = amplitude;
            break;

        case ELE3_CAP_VBL:
            pDevObj->testResults.result.capAlt.vb = amplitude;
            break;

        case ELE3_CAP_IMT:
            pDevObj->testResults.result.capAlt.im = amplitude;
            break;

        case ELE3_CAP_ILG:
            pDevObj->testResults.result.capAlt.il = amplitude;
            break;

        default:
            break;
    }

    if (++(*pAdcState) < ELE3_CAP_MAX_ADC) {
        *pTestState = ELE3_CAP_SETUP_ADC;
    } else {
        *pTestState = ELE3_CAP_RESTORE;
    }

    *pTimer = 2;
    return;
}
/*------------------------------------------------------------------------------
 * Vp3EleCapArgCheck
 * This functions is called by the Vp890Test3EleCap() function if
 * Vp890Test3EleCap() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the AC Rloop primitive are legal. The function also sets up
 * the AC Rloop state machine for the first state and initializes the results
 * from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info
 *  handle          - unique test handle
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *  testId          - test identifier
 *
 * Returns:
 * VpStatusType
 *
 * Result:
 *  This function initializes all relevent result values to 0. Stores the
 *  TestId and handle into the device Object as well as modifes the current
 *  ADC state to the first measurement.
 *
 *----------------------------------------------------------------------------*/
static VpStatusType
Vp3EleCapArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTest3EleCapAltResType *p3EleCapData = &(pTestInfo->pTestHeap->testArgs).capAlt;
    const VpTest3EleCapAltResType *pTestInput = pArgsUntyped;
    int16 *pTestState = &pTestInfo->testState;
    uint8 chanId = pLineObj->channelId;
    uint8 band = (pLineObj->codec == VP_OPTION_WIDEBAND) ? 1 : 2; /* Wideband = 1, Narrowband = 2 */

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleCapArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */
    *pAdcState = ELE3_CAP_VAL;
    pDevObj->testResults.result.capAlt.va = 0;
    pDevObj->testResults.result.capAlt.vb = 0;
    pDevObj->testResults.result.capAlt.im = 0;
    pDevObj->testResults.result.capAlt.il = 0;
    pDevObj->testResults.result.capAlt.accuracyFlag = FALSE;
    pDevObj->testResults.result.capAlt.tipCapCal = pDevObj->vp890SysCalData.tipCapCal[chanId];
    pDevObj->testResults.result.capAlt.ringCapCal = pDevObj->vp890SysCalData.ringCapCal[chanId];

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* This test cannot run if the isolate relay is open */
    if (VP_RELAY_RESET == pLineObj->relayState) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleCapArgCheck(): bailed openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    p3EleCapData->calMode = pTestInput->calMode;
    p3EleCapData->testAmp = pTestInput->testAmp;
    p3EleCapData->bias = pTestInput->bias;
    p3EleCapData->testFreq = pTestInput->testFreq;
    /* return the actual test frequency and the band to the ltApi */
    pDevObj->testResults.result.capAlt.freq = (p3EleCapData->testFreq << 4) & 0xF0;
    pDevObj->testResults.result.capAlt.freq |= band & 0x0F;

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp3EleCapSetup
 * This function is called by the Vp890Test3EleCap() state machine during
 * the ELE3_CAP_SETUP state.
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
static void
Vp3EleCapSetup(
    Vp890DeviceObjectType *pDevObj,
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    VpTest3EleCapAltResType *pCapData = &(pTestHeap->testArgs).capAlt;
    uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =  /* set 0V */
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 switchRegParam[VP890_REGULATOR_PARAM_LEN];
    uint8 sigGenCtrl = VP890_GEN_CTRL_EN_BIAS | VP890_GEN_CTRL_EN_A;
    uint8 slacState =  VP890_SS_BALANCED_RINGING;
    uint8 vpGain = 0x40;                            /* AX = 1 */
    uint16 audioAmp;
    int16 bias;
    uint16 freq;
    uint8 dcfeed[VP890_DC_FEED_LEN];

    uint8 mpiBuffer[VP890_REGULATOR_PARAM_LEN+1 +
                    VP890_SYS_STATE_LEN+1 +
                    VP890_SIGA_PARAMS_LEN+1 +
                    VP890_GEN_CTRL_LEN+1 +
                    VP890_VP_GAIN_LEN+1 +
                    VP890_ICR2_LEN+1 +
                    VP890_ICR3_LEN+1 +
                    VP890_DEV_MODE_LEN+1 +
                    VP890_DC_FEED_LEN+1];
    uint8 mpiIndex = 0;

    /* Save the converter configuration sampling frequency */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD,
        VP890_CONV_CFG_LEN, pTestHeap->saveConvConfig);

    /* Specify the number of samples to collect */
    if (pTestHeap->testArgs.capAlt.testFreq == 4) {
        pTestHeap->requestedSamples = 50;   /* 100 ms at 500Hz / 28 periodes at 280Hz */
    } else {
        pTestHeap->requestedSamples = 6;
    }
    pTestHeap->xtraBuffer = FALSE;

    /* Enable metallic drive speed-up */
    pLineObj->icr2Values[0] = 0x20;
    pLineObj->icr2Values[1] = 0x20;
    pLineObj->icr2Values[2] &= 0x0C;
    pLineObj->icr2Values[2] |= 0x80;
    pLineObj->icr2Values[3] &= 0x0C;
    pLineObj->icr2Values[3] |= 0x80;

    if (pCapData->bias > 0) {               /* Measuring ring->ground */
        slacState = VP890_SS_UNBALANCED_RINGING_PR;
        pLineObj->icr3Values[0] = 0x04;
        pLineObj->icr3Values[1] = 0x04;     /* Reverse longitudinal drive */
        pCapData->bias -= ((pDevObj->vp890SysCalData.sigGenAError[pLineObj->channelId][0] -
           pDevObj->vp890SysCalData.vocOffset[pLineObj->channelId][VP890_REV_POLARITY]) * 16 / 10);
    } else {                                /* Measuring tip->ground */
        slacState = VP890_SS_UNBALANCED_RINGING;
        pLineObj->icr3Values[0] = 0x04;
        pLineObj->icr3Values[1] = 0x00;
        pCapData->bias -= ((pDevObj->vp890SysCalData.sigGenAError[pLineObj->channelId][0] -
           pDevObj->vp890SysCalData.vocOffset[pLineObj->channelId][VP890_NORM_POLARITY]) * 16 / 10);
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
    if (pCapData->testFreq == 4) {
        freq = 0x02FD;
    } else {
        freq = 0x1C70 >> pCapData->testFreq; /* 333.252Hz -> 2666.016Hz*/
    }
    sigGenA[3] = (uint8)((freq >> 8) & 0x00FF);
    sigGenA[4] = (uint8)(freq & 0x00FF);

    /* LI = 0 (100 Ohms / leg), VOC = 12V */
    dcfeed[0] = pTestHeap->dcFeed[0] & 0x03;
    dcfeed[0] = dcfeed[0] | 0x40;
    dcfeed[1] = pTestHeap->dcFeed[1];

    /* force a fixed voltage -60V */
    switchRegParam[0] = pTestHeap->switchReg[0] | 0x0A;
    switchRegParam[1] = pTestHeap->switchReg[1];
    switchRegParam[2] = pTestHeap->switchReg[2] & 0xE0;
    switchRegParam[2] = switchRegParam[2] | 0x0B;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_REGULATOR_PARAM_WRT,
        VP890_REGULATOR_PARAM_LEN, switchRegParam);
    VpMemCpy(pDevObj->swParamsCache, switchRegParam, VP890_REGULATOR_PARAM_LEN);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SYS_STATE_WRT,
        VP890_SYS_STATE_LEN, &slacState);
    pLineObj->slicValueCache = slacState;

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SIGA_PARAMS_WRT,
        VP890_SIGA_PARAMS_LEN, sigGenA);

    if (pLineObj->sigGenCtrl[0] != sigGenCtrl) {
        pLineObj->sigGenCtrl[0] = sigGenCtrl;
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
            VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
    }

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_VP_GAIN_WRT,
        VP890_VP_GAIN_LEN, &vpGain);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
        VP890_ICR2_LEN, pLineObj->icr2Values);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
        VP890_ICR3_LEN, pLineObj->icr3Values);

    pDevObj->devMode[0] = pLineObj->channelId * 2 + 1;    /* setup to use the test data buffer */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DEV_MODE_WRT,
        VP890_DEV_MODE_LEN, pDevObj->devMode);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
        VP890_DC_FEED_LEN, dcfeed);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

static uint16
VpPcmComputeSquareRoot(
    uint32 number)
{
    uint8 itteration;
    int32 sqrtEst = 2;
    const int32 sqrtShift = number / 2;
    const uint8 newtonItt = 3;

    /* Find an estimate of the result in the correct octave approximately 1.5 bits of accuracy) */
    while (((sqrtEst * sqrtEst) < sqrtShift) && ((sqrtEst * sqrtEst) > 0)) {
        sqrtEst *= 2;
    }

    /* Use Newton's iteration to improve the estimate of the square root If the accuracy is N bits,
       on Newton's iteration increase the accuracy to 2N+1 bits. */
     for (itteration = 0; itteration < newtonItt; itteration++) {
        if (0 == sqrtEst) {
            break;
        } else {
            sqrtEst = (sqrtEst +  (number / sqrtEst)) / 2 ;
        }
     }

    return (uint16)sqrtEst;
}
#endif /* VP890_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_890_SERIES */
