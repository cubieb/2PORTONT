/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 9775 $
 * $LastChangedDate: 2012-04-11 18:15:57 -0400 (Wed, 11 Apr 2012) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

typedef enum {
    MET_RAMP_SETUP              = 0,
    MET_RAMP_START_AND_MEASURE  = 1,
    MET_RAMP_GET_RESULT         = 2,
    MET_RAMP_COMPLETE           = 3,
    MET_RAMP_GEN_EVENT          = 4,
    MET_RAMP_MAX                = 4,
    MET_RAMP_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886MetRampStateTypes;

bool Vp886TestMetRampSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestRampType *pMetRampArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static bool 
RampSetup(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap);

static bool 
RampStart(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp886TestMetRamp
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestMetRamp(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{
    VpStatusType status = VP_STATUS_SUCCESS;
    VpTestRampType *pMetRampArgs = NULL;

    if (!callback) {
        const VpTestRampType *pInputArgs = (VpTestRampType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMetRamp NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results to nonmeaningful values */
        VpMemSet(&pLineObj->testResults.result.ramp, 0, sizeof(VpTestResultRampType));

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.ramp, pInputArgs, sizeof(VpTestRampType));

        pTestHeap->nextState = MET_RAMP_SETUP;
    }
    pMetRampArgs = &pTestHeap->testArgs.ramp;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestMetRampSM(pLineCtx, pLineObj, pMetRampArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestMetRampSM
 *----------------------------------------------------------------------------*/
bool Vp886TestMetRampSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestRampType *pMetRampArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMetRampSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {

        case MET_RAMP_SETUP:
            if (RampSetup(pLineCtx, pLineObj, pTestHeap) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMetRampSM failed"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            pTestHeap->nextState = MET_RAMP_START_AND_MEASURE;
            requestNextState = TRUE;
            break;

        case MET_RAMP_START_AND_MEASURE:
            if (RampStart(pLineCtx, pLineObj, pTestHeap) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMetRampSM failed"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = MET_RAMP_GET_RESULT;
            break;

        case MET_RAMP_GET_RESULT: {
            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
            Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
            Vp886AdcMathDataType mathData;

            /* get the imt data and generate the event */
            if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, pCalData->sadc.offset, pCalData->sadc.gain)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SadcGetMathData VP886_SADC_BUF_2 sadc calculations failed"));
                break;
            }
            if ((mathData.average * -1) > VP_INT16_MAX) {
                pLineObj->testResults.result.ramp.imt = VP_INT16_MAX;
            } else if ((-1 * mathData.average) < VP_INT16_MIN) {
                pLineObj->testResults.result.ramp.imt = VP_INT16_MIN;
            } else {
                pLineObj->testResults.result.ramp.imt = (int16)((mathData.average * -1));
            }

            pTestHeap->nextState = MET_RAMP_COMPLETE;
            requestNextState = TRUE;
            break;
        }

        case MET_RAMP_COMPLETE:
            /* setup a timer to allow the ramp to complete */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 20, 0, MET_RAMP_SETUP)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMetRampSM: Cannot add to timer que."));
            }

            pTestHeap->nextState = MET_RAMP_GEN_EVENT;
            break;

        case MET_RAMP_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMetRampSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

static bool
RampSetup (
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap)
{
    uint8 dcfeed[VP886_R_DCFEED_LEN];

    /* Force normal current sensing */
    if (VpSlacRegRead(NULL, pLineCtx, VP886_R_DCFEED_RD, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886RampSetup failed VP886_R_DCFEED_RD"));
        return FALSE;
    }

    /* Is longitudinal impedance set to 50 Ohm? If yes, then set to 100 Ohm */
    if (VP886_R_DCFEED_LONG_IMPED & dcfeed[1]) {
        dcfeed[1] &= ~VP886_R_DCFEED_LONG_IMPED;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886RampSetup failed VP886_R_DCFEED_WRT"));
            return FALSE;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Vp886RampSetup()
 * This function is called by the Vp886TestMetRampSM() state machine during
 * the MET_RAMP_SETUP state to program the signal generator A in order to
 * perform a timed ramp.
 *
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *
 * Returns:TRUE if no error, FALSE otherwise
 * --
 *
 *----------------------------------------------------------------------------*/

static bool 
RampStart(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap)
{
    VpTestTipSelectType tip = pTestHeap->testArgs.ramp.tip;
    Vp886SadcSignalType sig;
    uint32 totalTimeInFrames = 0;
    uint16 numSamples, delaySamples;
    int16 sigGenAAmp, sigGenAFreq;
    uint8 sigGenAB[VP886_R_SIGAB_LEN] =
        {0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    /* convert input slope to nearest Sig Gen A freq */
    sigGenAFreq = VP886_SLOPE_TO_FREQ(pTestHeap->testArgs.ramp.dcVoltage, pTestHeap->testArgs.ramp.vRate);

    /* ensure value is non-negative integer */
    sigGenAFreq = sigGenAFreq ? ABS(sigGenAFreq) : 1;

    /* rtn 10 times the actual slope that is programed into sig Gen AB */
    pLineObj->testResults.result.ramp.vRate =
        ((10 * sigGenAFreq * pTestHeap->testArgs.ramp.dcVoltage) +
        (VP886_UNIT_RAMP_TIME >> 1)) / VP886_UNIT_RAMP_TIME;

    /* Total time is the unit ramp time (2.7307s / 2) divided by FRQA */
    totalTimeInFrames = (VP886_UNIT_RAMP_TIME * 8) / sigGenAFreq;
    VP_TEST(VpLineCtxType, pLineCtx, ("calculated totalTimeInFrames %li", totalTimeInFrames));
    
    /* force a min of 20ms of time */
    if (totalTimeInFrames < 160) {
        totalTimeInFrames = 160;
        VP_TEST(VpLineCtxType, pLineCtx, ("forced totalTimeInFrames %li", totalTimeInFrames));
    }

    /* measure the current for the duration of the ramp minus a 10ms delay at the begining */
    delaySamples = 80;
    numSamples = totalTimeInFrames - delaySamples;

    /* load freq into siggen A */
    sigGenAB[3] = (uint8)((sigGenAFreq >> 8) & 0xFF);
    sigGenAB[4] = (uint8)(sigGenAFreq & 0x00FF);

    /* convert input ADC amplitude to DAC amplitude */
    sigGenAAmp = (int16)VP886_UNIT_CONVERT(pTestHeap->testArgs.ramp.dcVoltage,
        VP886_UNIT_ADC_VAB, VP886_UNIT_DAC_RING);

    if (pTestHeap->testArgs.ramp.tip == VP_TEST_RING) {
        sigGenAAmp *= -1;
    }

    /* load ampl into siggen A */
    sigGenAB[5] = (uint8)((sigGenAAmp >> 8) & 0xFF);
    sigGenAB[6] = (uint8)(sigGenAAmp & 0x00FF);

    /* setup SADC for a measurement */
    if (tip == VP_TEST_TIP_RING) {
        sig = VP886_SADC_SEL_MET_I;
    } else {
        sig = VP886_SADC_SEL_LONG_I;
    }
    if (Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_SINGLE, VP886_SADC_RATE_FULL,
            sig,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            delaySamples,
            numSamples,
            VP886_LINE_TEST) == FALSE)

    {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampStart unable to setup SADC for imt measurement"));
        return FALSE;
    }

    VP_TEST(VpLineCtxType, pLineCtx, ("RampStart:dcVoltage: %i", pTestHeap->testArgs.ramp.dcVoltage));
    VP_TEST(VpLineCtxType, pLineCtx, ("RampStart:vRate in: %i", pTestHeap->testArgs.ramp.vRate));
    VP_TEST(VpLineCtxType, pLineCtx, ("RampStart:sigGenAFreq: %i", sigGenAFreq));
    VP_TEST(VpLineCtxType, pLineCtx, ("RampStart:sigGenAAmp: %i", sigGenAAmp));
    VP_TEST(VpLineCtxType, pLineCtx, ("RampStart:numSamples: %i", numSamples));
    VP_TEST(VpLineCtxType, pLineCtx, ("RampStart:delaySamples: %i", delaySamples));
    VP_TEST(VpLineCtxType, pLineCtx, ("RampStart:vRate out: %i", pLineObj->testResults.result.ramp.vRate));

    /* start the ramp */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenAB) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampStart failed VP886_R_SIGAB_WRT"));
        return FALSE;
    }

#if 0
    {
        int i = 0;
        VpSysDebugPrintf("\n");
        for (; i < VP886_R_SIGAB_LEN; i++) {
            VpSysDebugPrintf("%02x", sigGenAB[i]);
        }
        VpSysDebugPrintf("\n");
    }
#endif
    return TRUE;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
