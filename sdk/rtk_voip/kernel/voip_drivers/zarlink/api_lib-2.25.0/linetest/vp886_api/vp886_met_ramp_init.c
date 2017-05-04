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


typedef enum
{
    MET_RAMP_INIT_SETUP1             = 0,
    MET_RAMP_INIT_SETUP2             = 1,
    MET_RAMP_INIT_START              = 2,
    MET_RAMP_INIT_GEN_EVENT          = 3,
    MET_RAMP_INIT_MAX                = 3,
    MET_RAMP_INIT_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886MetRampStateTypes;

bool Vp886TestMetRampInitSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestRampInitType *pMetRampInitArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static bool
RampSetup1 (
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
RampSetup2 (
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
RampStart (
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp886TestMetRamp
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestMetRampInit(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{
    VpStatusType status = VP_STATUS_SUCCESS;
    VpTestRampInitType *pMetRampInitArgs = NULL;

    if (!callback) {
        const VpTestRampInitType *pInputArgs = (VpTestRampInitType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMetRamp NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.rampInit, pInputArgs, sizeof(VpTestRampInitType));

        pTestHeap->nextState = MET_RAMP_INIT_SETUP1;
    }
    pMetRampInitArgs = &pTestHeap->testArgs.rampInit;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestMetRampInitSM(pLineCtx, pLineObj, pMetRampInitArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestMetRampSM
 *----------------------------------------------------------------------------*/
bool Vp886TestMetRampInitSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestRampInitType *pMetRampInitArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMetRampInitSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {

        case MET_RAMP_INIT_SETUP1:

            if (RampSetup1(pLineCtx, pTestHeap) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1 failed"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /* wait for sig gen setup to complete */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 35, 0, MET_RAMP_INIT_SETUP1) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMetRampInitSM: Cannot add to timer queue."));
                return FALSE;
            }
            pTestHeap->nextState = MET_RAMP_INIT_SETUP2;
            break;

        case MET_RAMP_INIT_SETUP2:

            if (RampSetup2(pLineCtx, pTestHeap) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup2 failed"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /* wait for backdoor ringing setup to complete */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 20, 0, MET_RAMP_INIT_SETUP2) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMetRampInitSM: Cannot add to timer queue."));
                return FALSE;
            }
            pTestHeap->nextState = MET_RAMP_INIT_START;
            break;

        case MET_RAMP_INIT_START:
            if (RampStart(pLineCtx, pTestHeap) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("RampStart failed"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /* wait for ramp to complete */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 200, 0, MET_RAMP_INIT_START) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMetRampInitSM: Cannot add to timer queue."));
                return FALSE;
            }
            pTestHeap->nextState = MET_RAMP_INIT_GEN_EVENT;
            break;

        case MET_RAMP_INIT_GEN_EVENT:
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMetRampInitSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

/*------------------------------------------------------------------------------
 * RampSetup()
 * This function is called by the Vp886TestMetRampInitSM() state machine during
 * the MET_RAMP_INIT_SETUP1 state to prepare for entering backdoor ringing for
 * the initial ramp.
 *
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *
 * Returns:TRUE if no error, FALSE otherwise
 * --
 *
 *----------------------------------------------------------------------------*/
static bool
RampSetup1 (
   VpLineCtxType *pLineCtx,
   Vp886TestHeapType *pTestHeap)
{
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    VpTestTipSelectType tip = pTestHeap->testArgs.rampInit.tip;
    uint8 sigGenAB[VP886_R_SIGAB_LEN] =
        {0x01, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 sigGenCtrl = VP886_R_SIGCTRL_EN_SIGA;
    uint8 sysSt  = VP886_R_STATE_CODEC;
    uint8 disn   = 0x00;
    uint8 vpgain = VP886_R_VPGAIN_AR_GAIN;
    uint8 opCond = (VP886_R_OPCOND_CUT_TX | VP886_R_OPCOND_CUT_RX | VP886_R_OPCOND_HIGHPASS_DIS | 0x02);
    uint8 opFun  = VP886_R_OPFUNC_CODEC_LINEAR;
    uint8 ringCal[VP886_R_RINGCAL_LEN];
    uint8 extBatCor;
    uint8 vocCor;
    uint8 vasCor;
    uint8 ilrCor;
    uint8 activeCal[VP886_R_NORMCAL_LEN];
    /* Bytes initialized to 0x00 are set based on Tracker/ABS and ramp lead */
    uint8 icr1[VP886_R_ICR1_LEN] = {0xFF, 0xC8, 0xFF, 0x00};
    uint8 icr4[VP886_R_ICR4_LEN] = {0xFF, 0x03, 0xFF, 0x28};


    /* Read the ringing cal register so that we can apply its values to the
       normal or reverse cal register. */
    VpSlacRegRead(NULL, pLineCtx, VP886_R_RINGCAL_RD, VP886_R_RINGCAL_LEN, ringCal);
    extBatCor = (ringCal[0] & VP886_R_RINGCAL_EXTBATCOR) >> 4;
    vocCor = (ringCal[0] & VP886_R_RINGCAL_VOCCOR);
    vasCor = (ringCal[1] & VP886_R_RINGCAL_VASCOR) >> 4;
    ilrCor = (ringCal[1] & VP886_R_RINGCAL_ILRCOR);
    VpMemSet(activeCal, 0, VP886_R_NORMCAL_LEN);
    activeCal[0] |= extBatCor;
    activeCal[1] |= (vocCor << 4);
    activeCal[1] |= ilrCor;
    if (VP886_IS_TRACKER(pDevObj)) {
        activeCal[2] |= vasCor;
    }

    /* Undo the backdoor ringing state, in case it was applied earlier */
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pTestHeap->scratch.icr2);
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pTestHeap->scratch.icr3);

    if (VP886_IS_TRACKER(pDevObj)) {
        sysSt |= VP886_R_STATE_SS_ACTIVE;
        icr1[3] = 0x4C;
    } else {
        /* need to be in VBH */
        sysSt |= VP886_R_STATE_SS_ACTIVE_HIGH;
        icr1[3] = 0x6C;
    }
    
    if ((tip == VP_TEST_TIP_RING && pTestHeap->testArgs.rampInit.dcVstart > 0) ||
        tip == VP_TEST_TIP)
    {
        sysSt |= VP886_R_STATE_POL;
    }

    /* start in the active state */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, &sysSt) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_STATE_WRT"));
        return FALSE;
    }

    if (VP886_IS_TRACKER(pDevObj)) {
        /* if we are tracking then go to fixed mode with an 80V battery to prevent ring trip */
        uint8 swrp[VP886_R_SWPARAM_LEN];
        swrp[0] = (pTestHeap->scratch.swParam[0] & ~VP886_R_SWPARAM_FLOOR_V) | 0x0F /* -80V */;
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, swrp) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_SWPARAM_WRT"));
            return FALSE;
        }
    }

    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_DISN_WRT, VP886_R_DISN_LEN, &disn) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_DISN_WRT"));
        return FALSE;
    }
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_VPGAIN_WRT, VP886_R_VPGAIN_LEN, &vpgain) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_VPGAIN_WRT"));
        return FALSE;
    }
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, &opCond) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_OPCOND_WRT"));
        return FALSE;
    }
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPFUNC_WRT, VP886_R_OPFUNC_LEN, &opFun) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_DISN_WRT"));
        return FALSE;
    }

    /* Set SigGenA to 0V */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenAB) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_SIGAB_WRT"));
        return FALSE;
    }
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_SIGCTRL_WRT"));
        return FALSE;
    }

    /* For non-differential ramps, set up siggen A to the starting voltage */
    if (tip != VP_TEST_TIP_RING) {
        int16 sigGenAAmp;

        sigGenCtrl = 0x00;
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_SIGCTRL_WRT"));
            return FALSE;
        }

        sigGenAAmp = (-1) * (int16)VP886_UNIT_CONVERT(pTestHeap->testArgs.rampInit.dcVstart,
            VP886_UNIT_ADC_VAB, VP886_UNIT_DAC_RING);
        if (tip == VP_TEST_TIP) {
            sigGenAB[0] = 0x03;
        } else {
            sigGenAB[0] = 0x07;
        }
        sigGenAB[5] = (uint8)((sigGenAAmp >> 8) & 0xFF);
        sigGenAB[6] = (uint8)(sigGenAAmp & 0x00FF);
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenAB) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_SIGAB_WRT"));
            return FALSE;
        }
    }

    if (tip == VP_TEST_TIP) {
        /* Apply ringing calibration to the reverse active cal register */
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_REVCAL_WRT, VP886_R_REVCAL_LEN, activeCal);
    } else {
        /* Apply ringing calibration to the normal active cal register */
        VpSlacRegWrite(NULL, pLineCtx, VP886_R_NORMCAL_WRT, VP886_R_NORMCAL_LEN, activeCal);
    }

    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    if (tip != VP_TEST_TIP_RING) {
        sigGenCtrl = VP886_R_SIGCTRL_EN_SIGA;
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup1() failed VP886_R_SIGCTRL_WRT"));
            return FALSE;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * RampSetup2()
 * This function is called by the Vp886TestMetRampInitSM() state machine during
 * the MET_RAMP_INIT_SETUP2 state to enter backdoor ringing from active.
 *
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *
 * Returns:TRUE if no error, FALSE otherwise
 * --
 *
 *----------------------------------------------------------------------------*/
static bool
RampSetup2 (
   VpLineCtxType *pLineCtx,
   Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    VpTestTipSelectType tip = pTestHeap->testArgs.rampInit.tip;

    /* Bytes initialized to 0x00 are set based on Tracker/ABS and ramp lead */
    uint8 icr2[VP886_R_ICR2_LEN] = {0xFF, 0x00, 0xFF, 0x00};
    uint8 icr3[VP886_R_ICR3_LEN] = {0xFF, 0x00, 0xFF, 0x00};

    if (VP886_IS_TRACKER(pDevObj)) {
        icr2[3] = 0x69;
        if (tip == VP_TEST_TIP_RING) {
            icr2[1] = 0xFC;
            icr3[1] = 0x31;
            if (pLineObj->channelId == 0) {
                icr3[3] = 0xDE;
            } else {
                icr3[3] = 0xF6;
            }
        } else if (tip == VP_TEST_RING) {
            icr2[1] = 0xDC;
            icr3[1] = 0x3D;
            icr3[3] = 0x97;
        } else { /* VP_TEST_TIP */
            icr2[1] = 0xDC;
            icr3[1] = 0x39;
            icr3[3] = 0x97;
        }
    } else {
        if (tip == VP_TEST_TIP_RING) {
            icr2[1] = 0xFC;
            icr2[3] = 0xB9;
            icr3[1] = 0x21;
            icr3[3] = 0xF6;
        } else if (tip == VP_TEST_RING) {
            icr2[1] = 0xDC;
            icr2[3] = 0x39;
            icr3[1] = 0x2D;
            icr3[3] = 0x97;
        } else { /* VP_TEST_TIP */
            icr2[1] = 0xDC;
            icr2[3] = 0x39;
            icr3[1] = 0x29;
            icr3[3] = 0x97;
        }
    }
    
    /* back door ringing from the active state */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup2() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("RampSetup2() failed VP886_R_ICR2_WRT"));
        return FALSE;
    }


    return TRUE;
}

/*------------------------------------------------------------------------------
 * RampStart()
 * This function is called by the Vp886TestMetRampInitSM() state machine during
 * the MET_RAMP_INIT_START state to set sig gen A to ramp towards the requested
 * input voltage.
 *
 * Notice that we never program the bias value for this device and we
 * do NOT have to program the signal generator to match the starting voltage.
 * This is due to the fact that the line starts at the current VOC value.
 * We do however use the starting voltage to determine how we need to set 
 * the amplitude.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *
 * Returns:
 * TRUE if no error, FALSE otherwise
 * --
 *
 *----------------------------------------------------------------------------*/
static bool
RampStart (
   VpLineCtxType *pLineCtx,
   Vp886TestHeapType *pTestHeap)
{
    int16 sigGenAAmp, ampADC, sigGenAFreq;
    uint8 sigGenAB[VP886_R_SIGAB_LEN] =
        {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    ampADC = pTestHeap->testArgs.rampInit.dcVend - pTestHeap->testArgs.rampInit.dcVstart;

    sigGenAFreq = VP886_SLOPE_TO_FREQ((ampADC ? ampADC : 0),pTestHeap->testArgs.rampInit.vRate);

    /* ensure value is non-negative integer */
    sigGenAFreq = sigGenAFreq ? ABS(sigGenAFreq) : 1;
 
    sigGenAB[3] = (uint8)((sigGenAFreq >> 8) & 0xFF);
    sigGenAB[4] = (uint8)(sigGenAFreq & 0x00FF);

    /* the amplitude is simple the voltage we need to travel */
    sigGenAAmp = (int16)VP886_UNIT_CONVERT(ampADC, VP886_UNIT_ADC_VAB, VP886_UNIT_DAC_RING);
    if (pTestHeap->testArgs.rampInit.tip == VP_TEST_RING) {
        sigGenAAmp *= -1;
    }

    sigGenAB[5] = (uint8)((sigGenAAmp >> 8) & 0xFF);
    sigGenAB[6] = (uint8)(sigGenAAmp & 0x00FF);

    /* Program SigGenAB */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenAB) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886InitStartRamp() failed VP886_R_SIGAB_WRT"));
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
