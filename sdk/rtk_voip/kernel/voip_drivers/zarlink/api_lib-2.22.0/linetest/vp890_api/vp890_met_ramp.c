/*
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2011-03-23 17:18:34 -0500 (Wed, 23 Mar 2011) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_890_SERIES)
#if defined (VP890_INCLUDE_TESTLINE_CODE)
#if (VP890_INCLUDE_LINE_TEST_PACKAGE == VP890_LINE_TEST_PROFESSIONAL)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp890_api.h"
#include "vp890_api_int.h"
#include "vp890_testline_int.h"

typedef enum
{
    RAMP_ADC_SETUP      = VP890_TESTLINE_GLB_STRT_STATE,
    RAMP_SETUP          = 5,
    RAMP_PCM_COLLECT    = 10,
    RAMP_GET_RESULTS    = 15,
    RAMP_END            = VP890_TESTLINE_GLB_END_STATE,
    RAMP_QUIT           = VP890_TESTLINE_GLB_QUIT_STATE,
    RAMP_TEST_STATE_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpInitRampTestStateTypes;


static VpStatusType
VpMetRampArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId);


EXTERN VpStatusType
Vp890TestMetRamp(
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
    VpTestRampType *pRampInput = &(pTestHeap->testArgs).ramp;

    int16 *pTestState = &pTestInfo->testState;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    int16 calculateSamples = 0;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return VpMetRampArgCheck(pLineCtx, pArgsUntyped, handle, testId);
    }

    switch (*pTestState) {
        case RAMP_ADC_SETUP: {
            bool txAnalogGainEnable = FALSE;
            uint8 measType = VP890_METALLIC_DC_I;
            uint8 dcfeed[VP890_DC_FEED_LEN];

            pLineObj->icr2Values[0] = 0x10;
            pLineObj->icr2Values[1] = 0x00;
            pLineObj->icr2Values[2] &= 0x0C;
            pLineObj->icr2Values[2] |= 0x80;
            pLineObj->icr2Values[3] &= 0x0C;
            pLineObj->icr2Values[3] |= 0x80;

            /* Force dcfeed to use 100 Ohm/leg longitudinal impedance*/
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DC_FEED_RD, VP890_DC_FEED_LEN,
                dcfeed);

            if (VP890_LONG_IMP_50 & dcfeed[0]) {
                dcfeed[0] &= ~VP890_LONG_IMP_50;
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DC_FEED_WRT, VP890_DC_FEED_LEN,
                    dcfeed);
            }

            if (pRampInput->tip == VP_TEST_TIP) {
                pLineObj->icr3Values[0] = 0x24;
                pLineObj->icr3Values[1] = 0x20;
                pLineObj->icr3Values[2] = 0x00;
                pLineObj->icr3Values[3] = 0x00;
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_ICR3_WRT,
                    VP890_ICR3_LEN, pLineObj->icr3Values);
            }

            /* Force normal current sensing */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_ICR2_WRT, VP890_ICR2_LEN, pLineObj->icr2Values);

            /* change the measurement to be longitudinal if not tip to ring */
            if (pRampInput->tip != VP_TEST_TIP_RING) {
                measType = VP890_LONGITUDINAL_DC_I;
                txAnalogGainEnable = TRUE;
            }

            /* setup ADC for a measurement */
            if ( TRUE == Vp890AdcSetup(pLineCtx, measType, txAnalogGainEnable) ) {
                Vp890SetTestStateAndTimer(pDevObj, pTestState,
                    RAMP_SETUP, MS_TO_TICKRATE(VP890_ADC_CHANGE_SETTLING_TIME, tick));
            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                Vp890SetTestStateAndTimer(pDevObj, pTestState, RAMP_QUIT, NEXT_TICK);
            }

            break;
        }
        case RAMP_SETUP: {
            uint8 chanId = pLineObj->channelId;
            uint8 byte1, byte2;
            uint16 sigGenAFreqAbs;
            uint8 sigGenAB[VP890_SIGA_PARAMS_LEN] =
                {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            /* convert input ADC amplitude to DAC amplitude */
            int16 endAmpDAC = (int16)VP890_UNIT_CONVERT(pRampInput->dcVoltage,
                VP890_UNIT_ADC_VAB, VP890_UNIT_DAC_RING);

            /* convert input slope to nearest Sig Gen A freq */
            int16 sigGenAFreq = VP890_SLOPE_TO_FREQ(pRampInput->dcVoltage,
                pRampInput->vRate);

            /* covert bias to DAC */
            int16 bias = (int16)VP890_UNIT_CONVERT(pRampInput->bias,
                VP890_UNIT_ADC_VAB, VP890_UNIT_DAC_RING);

            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(dcVoltage:%i)", pRampInput->dcVoltage));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(endAmpDAC:%i)", endAmpDAC));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(sigGenAFreq:%i)", sigGenAFreq));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(biasInput:%i)", pRampInput->bias));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(bias:%i)", bias));

            /* load calibrated ampl into siggen A*/
            if (pLineObj->slicValueCache & VP890_SS_POLARITY_MASK) {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(polrev)"));
                bias += (pDevObj->vp890SysCalData.sigGenAError[chanId][0] -
                    pDevObj->vp890SysCalData.vocOffset[chanId][VP890_REV_POLARITY]) * 16 / 10;
            } else {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(normal)"));
                bias -= (pDevObj->vp890SysCalData.sigGenAError[chanId][0] -
                    pDevObj->vp890SysCalData.vocOffset[chanId][VP890_NORM_POLARITY]) * 16 / 10;
            }

            byte1 = (uint8)((bias >> 8) & 0xFF);
            byte2 = (uint8)(bias & 0x00FF);
            sigGenAB[1] = byte1;
            sigGenAB[2] = byte2;

            /* ensure value is non-negative integer and not 0 */
            sigGenAFreqAbs = (sigGenAFreq < 0) ? -sigGenAFreq : sigGenAFreq;
            sigGenAFreq = (0 == sigGenAFreqAbs) ? 1 : sigGenAFreqAbs;

            /* rtn 10 times the actual slope that is programed into sig Gen AB */
            pDevObj->testResults.result.ramp.vRate =
                ((10 * sigGenAFreq * pRampInput->dcVoltage) +
                (VP890_UNIT_RAMP_TIME >> 1)) / VP890_UNIT_RAMP_TIME;

            /* calculate delay till sample collection*/
            calculateSamples = 32768 / (sigGenAFreq * 3);

            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(calculateSamples:%i)", calculateSamples));

            /* load freq into siggen A*/
            byte1 = (uint8)((sigGenAFreq >> 8) & 0xFF);
            byte2 = (uint8)(sigGenAFreq & 0x00FF);
            sigGenAB[3] = byte1;
            sigGenAB[4] = byte2;

            /* load ampl into siggen A*/
            byte1 = (uint8)((endAmpDAC >> 8) & 0xFF);
            byte2 = (uint8)(endAmpDAC & 0x00FF);
            sigGenAB[5] = byte1;
            sigGenAB[6] = byte2;

            /* Program SigGenAB */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SIGA_PARAMS_WRT,
                VP890_SIGA_PARAMS_LEN, sigGenAB);

            *pTestState = RAMP_PCM_COLLECT;
            /*no break */
        }

        case RAMP_PCM_COLLECT:
            /*
             * forcing a delay of 10ms to allow adc to catch up
             * without this delay a large imt or ilg value
             * at the begining of the ramp will be captured.
             */
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                RAMP_GET_RESULTS, 80, calculateSamples);
            break;

        case RAMP_GET_RESULTS:
            /* get results */
            if (pRampInput->tip != VP_TEST_TIP_RING) {
                Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, TRUE, FALSE, 0,
                    VP890_LONGITUDINAL_DC_I, &pDevObj->testResults.result.ramp.imt);
            } else {
                Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, FALSE, 0,
                    VP890_METALLIC_DC_I, &pDevObj->testResults.result.ramp.imt);
            }
            /* no break */

        case RAMP_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(): complete"));
            break;

        case RAMP_QUIT:
            /* The test has ended early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            /* The test has entered an unsuppoted state */
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            pDevObj->testResults.result.calFailed = TRUE;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestMetRamp(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
}

static VpStatusType
VpMetRampArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId)
{

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTestRampType *pTestInput = (VpTestRampType*)pArgsUntyped;
    VpTestRampType *pRampInput = &(pTestInfo->pTestHeap->testArgs).ramp;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpMetRampArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only once per test.
     */
    pDevObj->testResults.result.ramp.imt = 0;
    pDevObj->testResults.result.ramp.vRate = 0;
    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /*
     * ensure that requested final v is not greater than |154.368|
     * (154.368 / (240/32768))
     */
    if (VP890_UNIT_DAC_RING < ABS(pTestInput->dcVoltage)) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResHGArgCheck(dcv:%i>%li): bailed",
            ABS(pTestInput->dcVoltage), VP890_UNIT_DAC_RING));
        return VP_STATUS_INVALID_ARG;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    /* make sure the dcvoltage is not literaly 0 as it will cause divide by 0 errors */
    pRampInput->dcVoltage = (0 == pTestInput->dcVoltage) ? 1 : pTestInput->dcVoltage;
    pRampInput->vRate = pTestInput->vRate;
    pRampInput->bias = pTestInput->bias;
    pRampInput->tip = pTestInput->tip;

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;

}

#endif
#endif /* VP890_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_890_SERIES */
