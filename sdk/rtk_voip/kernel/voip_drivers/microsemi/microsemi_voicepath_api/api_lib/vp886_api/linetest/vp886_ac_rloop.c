/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10393 $
 * $LastChangedDate: 2012-08-16 15:29:50 -0500 (Thu, 16 Aug 2012) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

typedef enum
{
    AC_RLOOP_OFFSET_SETUP       = 0,
    AC_RLOOP_START_SADC_OFFSET  = 1,
    AC_RLOOP_GET_OFFSET         = 2,
    AC_RLOOP_SETUP              = 3,
    AC_RLOOP_START_SADC_MEASURE = 4,
    AC_RLOOP_GET_DATA           = 5,
    AC_RLOOP_RESTORE            = 6,
    AC_RLOOP_GEN_EVENT          = 7,
    AC_RLOOP_MAX                = 7,
    AC_RLOOP_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886AcRloopStateTypes;

bool Vp886TestAcRloopSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestAcRLoopType *pAcRloopArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

bool
Vp886AcRloopSetup(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap,
    bool measureOffset);

bool Vp886TrackerSetup(VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp886TestAcRloop
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestAcRloop(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTestAcRLoopType *pAcRloopArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTestAcRLoopType *pInputArgs = (VpTestAcRLoopType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloop NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results to nonmeaningful values */
        pLineObj->testResults.result.acimt.vab = 0;
        pLineObj->testResults.result.acimt.imt = 0;
        pLineObj->testResults.result.acimt.ilg = 0;
        pLineObj->testResults.result.acimt.freq = pInputArgs->freq * 1000;
        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.acRloop, pInputArgs, sizeof(VpTestAcRLoopType));

        /* make sure the requested lead is valid */
        if ((pInputArgs->tip != VP_TEST_TIP) &&
            (pInputArgs->tip != VP_TEST_RING) &
            (pInputArgs->tip != VP_TEST_TIP_RING) ) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloop invalid lead: %i", pInputArgs->tip));
            return VP_STATUS_INVALID_ARG;
        }

        pTestHeap->nextState = AC_RLOOP_OFFSET_SETUP;
    }

    pAcRloopArgs = &pTestHeap->testArgs.acRloop;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestAcRloopSM(pLineCtx, pLineObj, pAcRloopArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestAcRloopSM
 *----------------------------------------------------------------------------*/
bool Vp886TestAcRloopSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestAcRLoopType *pAcRloopArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM state: %i", pTestHeap->nextState));
    switch (pTestHeap->nextState) {

        case AC_RLOOP_OFFSET_SETUP: {
            if (Vp886TrackerSetup(pLineCtx, pTestHeap) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            if (Vp886CalmodeTestSetup(pLineCtx, pTestHeap) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /*
             * Bring both Tip and Ring to mid-rail, by performing a 100ms ramp
             * with signal generator A
             */
            if (Vp886AcRloopSetup(pLineCtx, pLineObj, pTestHeap, TRUE) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM failed offset measurement setup"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = AC_RLOOP_START_SADC_OFFSET;
            break;
        }

        case AC_RLOOP_START_SADC_OFFSET: {
            Vp886SadcSignalType sadcSigType = VP886_SADC_SEL_NO_CONN;

            if (pAcRloopArgs->tip == VP_TEST_TIP_RING) {
                sadcSigType = VP886_SADC_SEL_MET_VDC;
            } else if (pAcRloopArgs->tip == VP_TEST_TIP) {
                sadcSigType = VP886_SADC_SEL_TIP;
            } else if (pAcRloopArgs->tip == VP_TEST_RING) {
                sadcSigType = VP886_SADC_SEL_RING;
            } else {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM invalid lead"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }

            /* start the ADC to measure VSAB */
            if( Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                    sadcSigType,
                    VP886_SADC_SEL_NO_CONN,
                    VP886_SADC_SEL_NO_CONN,
                    VP886_SADC_SEL_NO_CONN,
                    VP886_SADC_SEL_NO_CONN,
                    (10 * 8),
                    (10 * 8),
                    VP886_LINE_TEST) == FALSE)

            {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM unable to setup SADC for offset meaurement"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                break;
            }

            pTestHeap->nextState = AC_RLOOP_GET_OFFSET;
            break;
        }

        case AC_RLOOP_GET_OFFSET: {
            int16 offset = 0;
            int16 gain = 0;
            int16 avgSign = -1;

            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
            Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
            Vp886AdcMathDataType mathData;

            if (pLineObj->registers.sysState[0] & VP886_R_STATE_POL) {
                offset = pCalData->vabSenseReverse.offset;
                gain = pCalData->vabSenseReverse.gain;
            } else {
                offset =pCalData->vabSenseNormal.offset;
                gain = pCalData->vabSenseNormal.gain;
            }

            /* get vsab */
            Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain);
            /* make sure we actually collected samples  */
            if (mathData.numSamples == 0) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM invalid numSamps collected 0"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                requestNextState = FALSE;
                break;
            }

            /* flip it if the lead is not differential*/
            if (pAcRloopArgs->tip != VP_TEST_TIP_RING) {
                avgSign *= -1;
            }

            pLineObj->testResults.result.acimt.vab = (int16)(mathData.average) * avgSign;;
            VP_TEST(None, NULL, ("VpAcRloopOffsetSetup(measuredBiasOffset)=%i",
                                      pLineObj->testResults.result.acimt.vab));

            pTestHeap->nextState = AC_RLOOP_SETUP;
            requestNextState = TRUE;
            break;
        }

        case AC_RLOOP_SETUP: {
            if (Vp886AcRloopSetup(pLineCtx, pLineObj, pTestHeap, FALSE)==FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM failed measurement setup"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = AC_RLOOP_START_SADC_MEASURE;
            break;
        }

        case AC_RLOOP_START_SADC_MEASURE: {
            /* start the ADC to measure VSAB */
            if( Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                    VP886_SADC_SEL_MET_VDC,
                    VP886_SADC_SEL_MET_I,
                    VP886_SADC_SEL_LONG_I,
                    VP886_SADC_SEL_NO_CONN,
                    VP886_SADC_SEL_NO_CONN,
                    pAcRloopArgs->settlingTime,
                    pAcRloopArgs->integrateTime,
                    VP886_LINE_TEST) == FALSE)

            {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM unable to setup SADC for offset measurement"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                break;
            }

            pTestHeap->nextState = AC_RLOOP_GET_DATA;
            break;

        }

        case AC_RLOOP_GET_DATA: {
            /* Read the rms value for vab, imt and ilg */
            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
            Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
            Vp886AdcMathDataType mathData;

            int16 offset = pCalData->vabSenseNormal.offset;
            int16 gain   = pCalData->vabSenseNormal.gain;

            /*vsab*/
            /* get the data and generate the event */
            if (Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM VP886_SADC_BUF_1 sadc calculations failed"));
                break;
            }
            pLineObj->testResults.result.acimt.vab = (int16)mathData.rms;

            /*imt*/
            offset = pCalData->sadc.offset;
            gain   = pCalData->sadc.gain;
            /* get the data and generate the event */
            if (Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_2, &mathData, offset, gain) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM VP886_SADC_BUF_2 sadc calculations failed"));
                break;
            }
            pLineObj->testResults.result.acimt.imt = (int16)mathData.rms;

            /*ilg*/
            /* get the data and generate the event */
            if (Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_3, &mathData, offset, gain) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM VP886_SADC_BUF_3 sadc calculations failed"));
                break;
            }
            pLineObj->testResults.result.acimt.ilg = (int16)mathData.rms;
            pTestHeap->nextState = AC_RLOOP_RESTORE;
            requestNextState = TRUE;
            break;
        }
        case AC_RLOOP_RESTORE: {
            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

            if (pTestHeap->testArgs.acRloop.calMode && VP886_IS_TRACKER(pDevObj)) {
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, pTestHeap->scratch.swParam);
            }

            /* 
             * When the requested test lead is tip or ring the linestate at this point is 
             * unbalanced. The device will burn itself up if we go from unbalanced ringing
             * to low power mode. So we have to transition through some other feed state
             * till ring exit is observed before we can allow any other code such as
             * conclude to write the slic state to low power. I am doing this for
             * no low power lines as well so that the timing of the tests remains
             * consistant for all line types.
             */
            if ( (pAcRloopArgs->tip == VP_TEST_RING) || (pAcRloopArgs->tip == VP_TEST_TIP)) {

                if (pAcRloopArgs->tip == VP_TEST_RING) {
                    pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC | VP886_R_STATE_SS_ACTIVE);
                } else {
                    pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC | VP886_R_STATE_SS_ACTIVE | VP886_R_STATE_POL);
                }
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState);

                /* wait for the requested amount of time */
                if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 50, 0, pTestHeap->nextState) == FALSE) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM Cannot add to timer queue."));
                    break;
                }
            } else {
                requestNextState = TRUE;
            }

            pTestHeap->nextState = AC_RLOOP_GEN_EVENT;
            break;
        }
        case AC_RLOOP_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

/*------------------------------------------------------------------------------
 * Vp886AcRloopSetup
 * This function is called by the Vp886TestAcRloopSM() state machine,
 * to perform a controlled ramp using the signal genrator and the programmed vBias
 * in order to bring both leads to the half battery common mode point.
 * Then program the Signal generator to apply a voltage of amplitude VtestLevel
 * to the line
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pLineObj        - pointer to the line object.
 *  pTestHeap       - pointer to the test heap.
 *
 * Returns:
 * --
 * Result:
 *  Signal Generator A will be setup and slic will be put into
 *  balanced ringing. The loop super vision ringtrip threshold and current
 *  will be lowered in order not to cause a ring tirp during the test.
 *----------------------------------------------------------------------------*/
bool
Vp886AcRloopSetup(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap,
    bool measureOffset)
{
    VpTestAcRLoopType *pAcRloopData = &pTestHeap->testArgs.acRloop;
    int16 bias = pAcRloopData->vBias;
    /* Min amp of 2 must be set or the feed will not collapse */
    uint8 ringGen[VP886_R_RINGGEN_LEN] = {
        0x00, 0x00, 0x00, 0x0A, 0xAB, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00
    };

    /* convert voltage bias into sig gen bias */
    if (pAcRloopData->tip != VP_TEST_TIP_RING) {
        bias = (int16)VP886_UNIT_CONVERT(VP886_AC_RLOOP_MAX_TEST_LVL, VP886_UNIT_MV, VP886_UNIT_DAC_RING);
    } else {
        bias = (int16)VP886_UNIT_CONVERT(pAcRloopData->vBias, VP886_UNIT_MV, VP886_UNIT_DAC_RING);
    }
    /*if this function is called for the bias offset measurement, then
     * perform these steps
     */
    if (measureOffset) {
        Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

        uint8 lpSuper[VP886_R_LOOPSUP_LEN] = {0x1B, 0x84, 0x10, 0x00, 0x00};
        uint8 dcfeed[VP886_R_DCFEED_LEN] = {0x12, 0x08};


        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, lpSuper) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886AcRloopSetup failed VP886_R_LOOPSUP_WRT"));
            return FALSE;
        }

        /* Make sure we dont overwrite VAS */
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_DCFEED_RD, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886AcRloopSetup failed VP886_R_DCFEED_RD"));
            return FALSE;
        }
        VP_TEST(None, NULL, ("dcfeed=0x%02x, 0x%02x", dcfeed[0], dcfeed[1]));

        if (VP886_IS_TRACKER(pDevObj)) {
            dcfeed[0] = (dcfeed[0] & 0x03) | 0x12;
        } else {
            dcfeed[0] = (dcfeed[0] & 0x03) | 0x08;
        }
        dcfeed[1] = (dcfeed[1] & 0xC0) | 0x08;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886AcRloopSetup failed VP886_R_DCFEED_WRT"));
            return FALSE;
        }
    } else {
        uint16 freq  = pAcRloopData->freq;
        if (pLineObj->testResults.result.acimt.vab != 0) {
            int16 biasOffset = (int16)VP886_UNIT_CONVERT(pLineObj->testResults.result.acimt.vab, VP886_UNIT_ADC_VAB, VP886_UNIT_DAC_RING);
            VP_TEST(None, NULL, ("vab(bias_offset)=%i", biasOffset));
            bias -= (biasOffset - bias);
        }
        /* read siggen A */
        VpSlacRegRead(NULL, pLineCtx, VP886_R_RINGGEN_RD, VP886_R_RINGGEN_LEN,
                        ringGen);

        /* load freq into siggen */
        freq = (uint16)(((10000ul * freq) / 3662ul) + 1);
        ringGen[3] = (uint8)((freq >> 8) & 0xFF);
        ringGen[4] = (uint8)(freq & 0x00FF);

        /* load amplitude into siggen */
        ringGen[5] = (uint8)((pAcRloopData->vTestLevel >> 8) & 0xFF);
        ringGen[6] = (uint8)(pAcRloopData->vTestLevel & 0x00FF);
    }
    ringGen[1] = (uint8)((bias >> 8) & 0xFF);
    ringGen[2] = (uint8)(bias & 0x00FF);
    /*
     * load a very small amplitude into siggen
     * cannot use 0 because it causes issues with the
     * ring enter/exit algorithim
     */
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_RINGGEN_WRT, VP886_R_RINGGEN_LEN, ringGen) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886AcRloopSetup failed VP886_R_RINGGEN_WRT"));
        return FALSE;
    }
    if (measureOffset) {
        if (VP_TEST_TIP_RING == pAcRloopData->tip) {
            pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC | VP886_R_STATE_SS_BAL_RING);
        } else if (VP_TEST_TIP == pAcRloopData->tip) {
            pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC | VP886_R_STATE_POL | VP886_R_STATE_SS_UNBAL_RING);
            pLineObj->registers.icr3[0]  = 0x24;
            pLineObj->registers.icr3[1]  = 0x20;
            pLineObj->registers.icr3[2]  = 0x00;
            pLineObj->registers.icr3[3]  = 0x00;
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
                return FALSE;
            }
        } else {
            pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC | VP886_R_STATE_SS_UNBAL_RING);
        }
        /* enable ringing */

        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886AcRloopSetup failed VP886_R_STATE_WRT"));
            return FALSE;
        }

    }
    /* wait for the requested amount of time */
    if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 100, 0, pTestHeap->nextState) == FALSE) {
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
        VP_TEST(VpLineCtxType, pLineCtx, ("VpAcRloopOffsetSetup: Cannot add to timer queue."));
    }


    return TRUE;
}

/* Vp886TrackerSetup() - to configure the Switching Parameter Register
 * in accordance with the test.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *
 * Returns:
 * --
 * Result:
 *  Configure the Switching Parameter Register
 */
bool Vp886TrackerSetup(VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    if (pTestHeap->testArgs.acRloop.calMode &&
        VP886_IS_TRACKER(pDevObj) &&
        !(pTestHeap->scratch.swParam[0] & VP886_R_SWPARAM_RING_TRACKING))
    {
        Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

        uint32 swz = VP886_UNIT_CONVERT(VP886_AC_RLOOP_MAX_BAT_LVL,
            VP886_UNIT_MV, VP886_UNIT_TRACK_SWZ);

        uint8 switchParam[VP886_R_SWPARAM_LEN] = {0, 0, 0};
#if 0        
        uint8 swregCtl;
 
        /* Make sure we dont overwrite VAS */
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_SWCTRL_RD, VP886_R_SWCTRL_LEN, &swregCtl) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TrackerSetup failed VP886_R_DCFEED_RD"));
            return FALSE;
        }
       
        swregCtl &= ~VP886_R_SWCTRL_SWPOL;
        if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWCTRL_WRT, VP886_R_SWCTRL_LEN,  &swregCtl) == FALSE)
        {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TrackerSetup() failed VP886_R_DCFEED_RD"));
            return FALSE;
        }
#endif
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_SWPARAM_RD, VP886_R_SWPARAM_LEN, pLineObj->registers.swParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TrackerSetup failed VP886_R_SWPARAM_RD"));
            return FALSE;
        }

        /* ensure we have -25V floor for duration of the test */
        switchParam[0] = (uint8)(((pLineObj->registers.swParam[0]) & ~(VP886_R_SWPARAM_FLOOR_V)) | 0x04  | VP886_R_SWPARAM_RING_TRACKING);
        /* force the ringing battery to be 75V */
        switchParam[1] = (0xE0 & pLineObj->registers.swParam[1]) | (uint8)swz;
        /* ensure we are in fixed battery mode */
        switchParam[2] = pLineObj->registers.swParam[2];

        VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TrackerSetup() swy = 0x%02x%02x%02x , swz = 0x%lx",
            switchParam[0], switchParam[1], switchParam[2], swz));

        return VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, switchParam);
    }  else {
        return TRUE;
    }

}


#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
