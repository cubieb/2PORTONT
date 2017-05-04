/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11400 $
 * $LastChangedDate: 2014-05-06 16:45:24 -0500 (Tue, 06 May 2014) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

typedef enum
{
    LOOPBACK_CODEC_SETUP    = 0,
    LOOPBACK_BFILTER_SETUP  = 1,
    LOOPBACK_ANALOG_SETUP   = 2,
    LOOPBACK_TIMESLOT_SETUP = 3,
    LOOPBACK_ACTIVATE_CODEC = 4,
    LOOPBACK_HOLD           = 5,
    LOOPBACK_GEN_EVENT      = 6,
    LOOPBACK_MAX            = 6,
    LOOPBACK_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886LoopbackStateTypes;


bool Vp886TestGetLoopbackSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestLoopbackType *pLoopCondArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);


/*------------------------------------------------------------------------------
 * Vp886TestLoopback
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestLoopback(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTestLoopbackType *pLoopbackArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTestLoopbackType *pInputArgs = (VpTestLoopbackType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCond NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results to nonmeaningful values */
        pLineObj->testResults.result.loopback.pApplicationInfo = VP_NULL;
        pLineObj->testResults.result.loopback.loopback = pInputArgs->loopback;

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.loopback, pInputArgs, sizeof(VpTestLoopbackType));

        /* setup for the first state */
        if (pInputArgs->loopback == VP_LOOPBACK_TEST_CODEC) {
            pTestHeap->nextState = LOOPBACK_CODEC_SETUP;
        } else if (pInputArgs->loopback == VP_LOOPBACK_TEST_BFILTER) {
            pTestHeap->nextState = LOOPBACK_BFILTER_SETUP;
        } else if (pInputArgs->loopback == VP_LOOPBACK_TEST_ANALOG) {
            pTestHeap->nextState = LOOPBACK_ANALOG_SETUP;
        } else if (pInputArgs->loopback == VP_LOOPBACK_TEST_TIMESLOT) {
            pTestHeap->nextState = LOOPBACK_TIMESLOT_SETUP;
        } else {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestLoopback invalid loopback: %i", pInputArgs->loopback));
            return VP_STATUS_INVALID_ARG;
        }
    }

    pLoopbackArgs = &pTestHeap->testArgs.loopback;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestGetLoopbackSM(pLineCtx, pLineObj, pLoopbackArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestGetLoopbackSM
 *----------------------------------------------------------------------------*/
bool Vp886TestGetLoopbackSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestLoopbackType *pLoopbackArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopbackSM state: %i", pTestHeap->nextState));

      
    /* S.H. TODO:
     * Need to abort the test if a state change occurs or if the hook status changes !!!
     */
    switch (pTestHeap->nextState) {
        case LOOPBACK_TIMESLOT_SETUP: {

            /* Disable the DAC so that audio does not show up on Tip/Ring */
            pLineObj->registers.icr4[0] |= 0x02;
            pLineObj->registers.icr4[1] &= 0xFD;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4);

            /* Enable tsa loop back and disable high pass filter  */
            pLineObj->registers.opCond[0] = (VP886_R_OPCOND_TSA_LOOPBACK | VP886_R_OPCOND_HIGHPASS_DIS);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, pLineObj->registers.opCond);

            requestNextState = TRUE;
            pTestHeap->nextState = LOOPBACK_ACTIVATE_CODEC;
            break;
        }

        case LOOPBACK_BFILTER_SETUP: {
            uint8 vadc[VP886_R_VADC_LEN];
            uint8 opFunction[VP886_R_OPFUNC_LEN];

            /* special pass through bfilter values */
            uint8 bfir[VP886_R_B_FIR_FILT_LEN] =
                {0xBA, 0x3D, 0x21, 0xBA, 0x0B, 0xA2, 0x29, 0x72, 0x9F, 0x39, 0xF3, 0x9F, 0xC9, 0xF0};
            uint8 biir[VP886_R_B_IIR_FILT_LEN] = {0x2E, 0x01};

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_B_FIR_FILT_WRT, VP886_R_B_FIR_FILT_LEN, bfir);
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_B_IIR_FILT_WRT, VP886_R_B_IIR_FILT_LEN, biir);

            /* Disable the DAC so that audio does not show up on Tip/Ring */
            pLineObj->registers.icr4[0] |= 0x02;
            pLineObj->registers.icr4[1] &= 0xFD;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4);

            /* read the current state of the op func reg */
            VpSlacRegRead(NULL, pLineCtx, VP886_R_OPFUNC_RD, VP886_R_OPFUNC_LEN, opFunction);

            /* Enable only the B filter and leave companding mode alone */
            opFunction[0] &= ~VP886_R_OPFUNC_ALL_FILTERS;
            opFunction[0] |= VP886_R_OPFUNC_EB;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPFUNC_WRT, VP886_R_OPFUNC_LEN, opFunction);

            /* enable the tx rx path  */
            pLineObj->registers.opCond[0] = 0;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, pLineObj->registers.opCond);

            /* Prevent any signal from getting past the bfilter by disabling the VADC */
            VpMemSet(vadc, 0, sizeof(VP886_R_VADC_LEN));
            vadc[1] = VP886_R_VADC_SEL_ADC_OFFSET;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_VADC_WRT, VP886_R_VADC_LEN, vadc);

            requestNextState = TRUE;
            pTestHeap->nextState = LOOPBACK_ACTIVATE_CODEC;
            break;
        }

        case LOOPBACK_CODEC_SETUP: {
            uint8 vadc[VP886_R_VADC_LEN];
            uint8 disn[VP886_R_DISN_LEN] = {0x00};
            uint8 opFunction[VP886_R_OPFUNC_LEN];

            /* read the current state of the op func reg */
            VpSlacRegRead(NULL, pLineCtx, VP886_R_OPFUNC_RD, VP886_R_OPFUNC_LEN, opFunction);

            /* disable all of the filters */
            opFunction[0] &= ~VP886_R_OPFUNC_ALL_FILTERS;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPFUNC_WRT, VP886_R_OPFUNC_LEN, opFunction);

            /* disable disn */
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_DISN_WRT, VP886_R_DISN_LEN, disn);

            /* enable the tx rx path  */
            pLineObj->registers.opCond[0] = 0;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, pLineObj->registers.opCond);


            /* CODEC loopback actually occurs in the voice ADC */
            VpMemSet(vadc, 0, sizeof(VP886_R_VADC_LEN));
            vadc[1] = VP886_R_VADC_SEL_VDAC_LOOPBACK;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_VADC_WRT, VP886_R_VADC_LEN, vadc);

            requestNextState = TRUE;
            pTestHeap->nextState = LOOPBACK_ACTIVATE_CODEC;
            break;
        }

        case LOOPBACK_ANALOG_SETUP: {

            /* enable the tx rx path  */
            pLineObj->registers.opCond[0] = 0;
            VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, pLineObj->registers.opCond);

            requestNextState = TRUE;
            pTestHeap->nextState = LOOPBACK_ACTIVATE_CODEC;
            break;
        }

        case LOOPBACK_ACTIVATE_CODEC: {

            /* Need to ensure the codec is activated */
            pLineObj->registers.sysState[0] |= VP886_R_STATE_CODEC;

            VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN,  pLineObj->registers.sysState);
            pTestHeap->nextState = LOOPBACK_HOLD;
            requestNextState = TRUE;
            break;
        }

        case LOOPBACK_HOLD: {
            uint16 timerValMs = (pLoopbackArgs->waitTime / 8) + (pLoopbackArgs->loopbackTime / 8);

            /* setup up the timer to hold the loop back for wait time + loopback time */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, timerValMs, 0, pTestInfo->handle)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx,
                    ("Vp886TestGetLoopbackSM: Cannot add to timer que."));
            } else {
                pTestHeap->nextState = LOOPBACK_GEN_EVENT;
            }
            break;
        }

        case LOOPBACK_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopbackSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
