/*
 * Copyright (c) 2013, Microsemi Corporation
 *
 * $Revision: 10842 $
 * $LastChangedDate: 2013-03-04 11:28:51 -0600 (Mon, 04 Mar 2013) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

typedef enum
{
    GAIN_TLBX_SETUP              = 0,
    GAIN_TLBX_START_SADC_MEASURE = 1,
    GAIN_TLBX_GET_DATA           = 2,
    GAIN_TLBX_GEN_EVENT          = 3,
    GAIN_TLBX_MAX                = 4,
    GAIN_TLBX_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886GainTlbxStateTypes;

bool Vp886TestGainTlbxSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestGainTlbxType *pGainTlbxArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static void VpGainTlbxSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp886TestGainTlbx
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestGainTlbx(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTestGainTlbxType *pGainTlbxArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTestGainTlbxType *pInputArgs = (VpTestGainTlbxType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGainTlbx NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results to nonmeaningful values */
        pLineObj->testResults.result.gainTlbx.min = 0;
        pLineObj->testResults.result.gainTlbx.max = 0;
        pLineObj->testResults.result.gainTlbx.rms = 0;

        /* Compare the RMS result to the RMS reference level, which is 16141 for
         * the 0dBm0 signal.
         * (22826.4 * 10 ^ (0 / 20)) / sqrt(2) = 16140.7 */
        pLineObj->testResults.result.gainTlbx.dbRef = 16141;

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.gainTlbx, pInputArgs, sizeof(VpTestGainTlbxType));

        pTestHeap->nextState = GAIN_TLBX_SETUP;
    }

    pGainTlbxArgs = &pTestHeap->testArgs.gainTlbx;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestGainTlbxSM(pLineCtx, pLineObj, pGainTlbxArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestGainTlbxSM
 *----------------------------------------------------------------------------*/
bool Vp886TestGainTlbxSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestGainTlbxType *pGainTlbxArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGainTlbxSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {
        case GAIN_TLBX_SETUP: {
            VpGainTlbxSetup(pLineCtx, pTestHeap);

            pTestHeap->nextState = GAIN_TLBX_START_SADC_MEASURE;
            requestNextState = TRUE;
            break;
        }

        case GAIN_TLBX_START_SADC_MEASURE: {
            bool vadc = TRUE;
            uint16 timerValMs = 0;
            uint16 skipSysFrames = 10;
            uint16 collectSysFrames = 10;

            switch (pGainTlbxArgs->measType) {
                case VP_TEST_GAIN_MEAS_ECHO:
                    /* Capture 8 samples per cycles (8kHz sampling, 1kHz tone) */
                    skipSysFrames = 8 * pGainTlbxArgs->settlingMs;
                    collectSysFrames = 8 * pGainTlbxArgs->numCycle;
                    break;

                case VP_TEST_GAIN_MEAS_IDLE:
                    skipSysFrames = 8 * pGainTlbxArgs->settlingMs;
                    /* No tone, so no cycles... numCycle is just the number of requested frames  */
                    collectSysFrames = pGainTlbxArgs->numCycle;
                    break;

                default:
                    /* should never get here */
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGainTlbxSM "
                        "invalid input: %i", pGainTlbxArgs->measType));
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    break;
            }

            /* set the vadc to collect data, skip 10 samples to be sure */
            vadc = Vp886VadcSetup(pLineCtx, VP886_VADC_MATH, VP886_VADC_RATE_FULL,
                FALSE, FALSE, VP886_VADC_SEL_MET_VAC,
                skipSysFrames, collectSysFrames, VP886_LINE_TEST);

            if (!vadc) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGainTlbxSM unable to setup VADC"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
            }

            /* settling + integration + extra */
            timerValMs = (skipSysFrames / 8) + (collectSysFrames / 8) + 5;

            /* setup up the timer during the VADC measurement */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, timerValMs, 0,
                pTestInfo->handle)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx,
                    ("Vp886TestGainTlbxSM: Cannot add to timer que."));
            } else {
                pTestHeap->nextState = GAIN_TLBX_GET_DATA;
            }
            break;
        }

        case GAIN_TLBX_GET_DATA: {
            Vp886AdcMathDataType mathData;

            /* get the data and generate the event (no gain/offset -> trimmed in factory) */
            if (!Vp886VadcGetMathData(pLineCtx, &mathData, 0, 1000)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("GAIN_TLBX_GET_DATA vadc calculations failed"));
                break;
            }

            pLineObj->testResults.result.gainTlbx.min = mathData.minVal;
            pLineObj->testResults.result.gainTlbx.max = mathData.maxVal;
            pLineObj->testResults.result.gainTlbx.rms = mathData.rms;

            pTestHeap->nextState = GAIN_TLBX_GEN_EVENT;
            requestNextState = TRUE;
        }

        case GAIN_TLBX_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGainTlbxSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

/*------------------------------------------------------------------------------
 * VpGainTlbxSetup
 * This function is called by the Vp886TestGainTlbxSM() state machine during
 * the GAIN_TLBX_SETUP state.
 *
 *----------------------------------------------------------------------------*/
static void VpGainTlbxSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpTestGainTlbxType *pGainTlbxData = &pTestHeap->testArgs.gainTlbx;

    /* Use programmed filters in linear mode */
    pLineObj->registers.opFunc[0] = VP886_R_OPFUNC_CODEC_LINEAR | VP886_R_OPFUNC_ALL_FILTERS;
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPFUNC_WRT, VP886_R_OPFUNC_LEN,
        pLineObj->registers.opFunc);

    switch (pGainTlbxData->measType) {
        case VP_TEST_GAIN_MEAS_ECHO:
            /* Ensure that the receive path and HP filter are enabled and enable the 1kHz tone */
            pLineObj->registers.opCond[0] = 0x00 | VP886_R_OPCOND_1K_TONE;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN,
                pLineObj->registers.opCond);
            break;

        case VP_TEST_GAIN_MEAS_IDLE:
            /* Ensure that the receive path and HP filter are enabled */
            pLineObj->registers.opCond[0] = 0x00;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN,
                pLineObj->registers.opCond);
            break;

        default:
            /* should never get here */
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGainTlbxSM invalid input: %i",
                pGainTlbxData->measType));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
            break;
    }

    return;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
