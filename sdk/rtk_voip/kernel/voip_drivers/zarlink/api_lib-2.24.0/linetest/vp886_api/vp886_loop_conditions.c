/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10988 $
 * $LastChangedDate: 2013-05-30 10:00:23 -0500 (Thu, 30 May 2013) $
 */


#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

#define LOOP_COND_VADC_EXTRA_TIME 5 /* 5ms */

#define VP886_ILA_SCALE_1MA     (568)
#define VP886_ILA_SCALE_2MA     (2 * VP886_ILA_SCALE_1MA)
#define VP886_ILA_SCALE_18MA    (9 * VP886_ILA_SCALE_2MA)

typedef enum
{
    LOOP_COND_SETUP         = 0,
    LOOP_COND_SEL_ADC       = 1,
    LOOP_COND_START_SADC    = 2,
    LOOP_COND_START_VADC    = 3,
    LOOP_COND_GET_DATA      = 4,
    LOOP_COND_MAX           = 4,
    LOOP_COND_ENUM_SIZE     = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886GetLoopCondStateTypes;

static bool 
Vp886TestGetLoopCondSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestLoopCondType *pLoopCondArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp886TestGetLoopCond
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestGetLoopCond(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{
    VpTestLoopCondType *pLoopCondArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
        Vp886DeviceObjectType *pDevObj = pDevCtx->pDevObj;

        uint8 slicState = (pLineObj->registers.sysState[0] & VP886_R_STATE_SS);

        const VpTestLoopCondType *pInputArgs = (VpTestLoopCondType *)pArgsUntyped;
        uint8 io2Use = pDevObj->devProfileData.io2Use >> (4 * pLineObj->channelId);
        io2Use &= VP886_DEV_PROFILE_IO2_USE_BITS;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCond NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* make sure requested loop condition is valid */
        if ((pInputArgs->loopCond != VP_LOOP_COND_TEST_VSAB) &&
            (pInputArgs->loopCond != VP_LOOP_COND_TEST_VSAG) &&
            (pInputArgs->loopCond != VP_LOOP_COND_TEST_VSBG) &&
            (pInputArgs->loopCond != VP_LOOP_COND_TEST_IMT)  &&
            (pInputArgs->loopCond != VP_LOOP_COND_TEST_ILG)  &&
            (pInputArgs->loopCond != VP_LOOP_COND_TEST_BAT1) &&
            (pInputArgs->loopCond != VP_LOOP_COND_TEST_BAT2) &&
            (pInputArgs->loopCond != VP_LOOP_COND_TEST_BAT3) ) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCond invalid loopCond: %i", pInputArgs->loopCond));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results to nonmeaningful values */
        pLineObj->testResults.result.loopCond.data = VP_INT16_MAX;
        pLineObj->testResults.result.loopCond.condition = pInputArgs->loopCond;
        pLineObj->testResults.result.loopCond.calibrated = TRUE;
        pLineObj->testResults.result.loopCond.limit = 0;

        /*
         * No measurements can be made in shutdown. This would happen if 
         * a battery fault occured and when to line state disabled.
         */
        if (slicState == VP886_R_STATE_SS_SHUTDOWN) {
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            return VP_STATUS_SUCCESS;
        }

        /*
         * vbat 3 should not be measured if it is not setup as a voltage monitor
         * tracking devices should not return battery value of opposite channel
         */
        if ( ( (pInputArgs->loopCond == VP_LOOP_COND_TEST_BAT3) && ( io2Use != VP886_DEV_PROFILE_IO2_USE_VMM) ) ||
             ( !VP886_IS_ABS(pDevObj) && (pInputArgs->loopCond == VP_LOOP_COND_TEST_BAT2) && (pLineObj->channelId == 0) ) ||
             ( !VP886_IS_ABS(pDevObj) && (pInputArgs->loopCond == VP_LOOP_COND_TEST_BAT1) && (pLineObj->channelId == 1) ) )
        {
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            return VP_STATUS_SUCCESS;
        }

        /* 
         * Current measurements should not be made in the disconnect or low power SLIC states
         */
        if ( ((slicState == VP886_R_STATE_SS_DISCONNECT) || (slicState == VP886_R_STATE_SS_LOWPOWER)) &&  
             ((pInputArgs->loopCond == VP_LOOP_COND_TEST_IMT) || (pInputArgs->loopCond == VP_LOOP_COND_TEST_ILG)) ) 
        {
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            return VP_STATUS_SUCCESS;
        }

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.loopCond, pInputArgs, sizeof(VpTestLoopCondType));

        /* Fill in the limit value result */
        if (pInputArgs->loopCond == VP_LOOP_COND_TEST_IMT) {
            int32 targetIla;

            /* Retrieve the DC Profile target ILA value */
            targetIla = (pLineObj->registers.dcFeed[1] & VP886_R_DCFEED_ILA);

            /* Convert to PCM scale */
            targetIla *= ((VP886_ILA_SCALE_2MA / 2) * 10); /* 5682; */
            targetIla += (VP886_ILA_SCALE_18MA * 10); /* Base of 18ma 102272; */
            targetIla /= 10;

            pLineObj->testResults.result.loopCond.limit = (int16)targetIla;
        }

        /* set the state machine starting point based on cal flag */
        if (pInputArgs->calMode) {
            pTestHeap->nextState = LOOP_COND_SETUP;
        } else {
            pTestHeap->nextState = LOOP_COND_SEL_ADC;
        }
    }

    pLoopCondArgs = &pTestHeap->testArgs.loopCond;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestGetLoopCondSM(pLineCtx, pLineObj, pLoopCondArgs, pTestInfo, pTestHeap));

    return status;
}

static bool 
Vp886TestGetLoopCondSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestLoopCondType *pLoopCondArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {
        case LOOP_COND_SETUP: {
            uint8 slicState = (pLineObj->registers.sysState[0] & VP886_R_STATE_SS);
            /*
             * In order to save SPI transactions and time, this step can be
             * skipped if the input arg cal flag is FALSE.
             */

            /* small setup when if line state is disconnect so that voltages can be measured */
            if (slicState == VP886_R_STATE_SS_DISCONNECT) {

                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM measuring in disconnect"));

                /* Ensure the tip and ring sense signals are on */
                pLineObj->registers.icr2[0] = 0x0C;
                pLineObj->registers.icr2[1] = 0x0C;
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2);

                /* Ensure the line control circuits are on */
                pLineObj->registers.icr3[0] |= 0x21;
                pLineObj->registers.icr3[1] |= 0x21;
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3);

                /* Connect External Ctrl Network to AC Feedback Loop */
                pLineObj->registers.icr4[0] |= 0x01;
                pLineObj->registers.icr4[1] |= 0x01;
                VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4);
            }

            pTestHeap->nextState = LOOP_COND_SEL_ADC;
            requestNextState = TRUE;
            break;
        }

        case LOOP_COND_SEL_ADC: {
            uint8 slicState = (pLineObj->registers.sysState[0] & VP886_R_STATE_SS);

            if (((slicState == VP886_R_STATE_SS_BAL_RING) || (slicState == VP886_R_STATE_SS_UNBAL_RING)) && 
                 (pLoopCondArgs->loopCond == VP_LOOP_COND_TEST_IMT)) {
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM measuring imt with VADC"));
                pTestHeap->nextState = LOOP_COND_START_VADC;
            } else {
                pTestHeap->nextState = LOOP_COND_START_SADC;
            }
            requestNextState = TRUE;
            break;
        }

        case LOOP_COND_START_SADC: {
            bool sadc = TRUE;
            Vp886SadcSignalType sadcSel = VP886_SADC_SEL_NO_CONN;

            /* determine which signal to program into sadc based on input */
            switch (pLoopCondArgs->loopCond) {
                case VP_LOOP_COND_TEST_VSAB:
                    sadcSel = VP886_SADC_SEL_MET_VDC;
                    break;
                case VP_LOOP_COND_TEST_VSAG:
                    sadcSel = VP886_SADC_SEL_TIP;
                    break;
                case VP_LOOP_COND_TEST_VSBG:
                    sadcSel = VP886_SADC_SEL_RING;
                    break;
                case VP_LOOP_COND_TEST_IMT:
                    sadcSel = VP886_SADC_SEL_MET_I;
                    break;
                case VP_LOOP_COND_TEST_ILG:
                    sadcSel = VP886_SADC_SEL_LONG_I;
                    break;
                case VP_LOOP_COND_TEST_BAT1:
                    sadcSel = VP886_SADC_SEL_SWY;
                    break;
                case VP_LOOP_COND_TEST_BAT2:
                    sadcSel = VP886_SADC_SEL_SWZ;
                    break;
                case VP_LOOP_COND_TEST_BAT3:
                   /*
                    * mid bat is not supported by this device. However,
                    * we will be monitoring the new IO2 voltage sense 
                    * signal instead.
                    */
                    sadcSel = VP886_SADC_SEL_IO2;
                    break;
                default:
                    /* should never get here */
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM "
                        "invalid signal input: %i", pLoopCondArgs->loopCond));
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    break;
            }

            /* set the sadc to collect data */
            sadc = Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                sadcSel, VP886_SADC_SEL_NO_CONN, VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN, VP886_SADC_SEL_NO_CONN,
                pLoopCondArgs->settlingTime, pLoopCondArgs->integrateTime,
                VP886_LINE_TEST);

            if (!sadc) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM unable to setup SADC"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
            }

            pTestHeap->nextState = LOOP_COND_GET_DATA;
            break;
        }

        case LOOP_COND_START_VADC: {
            uint16 timerValMs = 0;

            /* set the vadc to collect data imt data while line is ringing */
            /* workaround:
             * not using VADC interrupt due to a device issue that
             * generates a superfluous interrupt immediately after programming 
             * the regsiter causing the get data step to occur before data is
             * actually ready.
             */
            bool vadc = Vp886VadcSetup(pLineCtx, VP886_VADC_MATH, VP886_VADC_RATE_FULL,
                FALSE, FALSE, VP886_VADC_SEL_MET_I, pLoopCondArgs->settlingTime, 
                pLoopCondArgs->integrateTime, VP886_LINE_TEST);

            if (!vadc) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM unable to setup VADC"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
            }

            timerValMs = (pLoopCondArgs->integrateTime / 8) +
                (pLoopCondArgs->settlingTime / 8) + 
                LOOP_COND_VADC_EXTRA_TIME;

            /* setup up the timer to hold the loop back for wait time + loopback time */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, timerValMs, 0, pTestInfo->handle)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx,
                    ("Vp886TestGetLoopbackSM: Cannot add to timer que."));
            } else {
                pTestHeap->nextState = LOOP_COND_GET_DATA;
            }
            break;
        }

        case LOOP_COND_GET_DATA: {
            bool usedVadc = FALSE;
            int32 average = 0;
            int16 avgSign = 1;
            int16 gain = 0;
            int16 offset = 0;
            Vp886AdcMathDataType mathData;
            VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
            Vp886DeviceObjectType *pDevObj = pDevCtx->pDevObj;
            Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
            uint8 slicState = (pLineObj->registers.sysState[0] & VP886_R_STATE_SS);

            /* determine how to adjust result with cal data based on input */
            switch (pLoopCondArgs->loopCond) {
                case VP_LOOP_COND_TEST_VSAB:
                    /* line state determines which sense offset and gain are used */
                    if ((slicState == VP886_R_STATE_SS_BAL_RING) || (slicState == VP886_R_STATE_SS_UNBAL_RING)) {
                        offset = pCalData->vabSenseRinging.offset;
                        gain = pCalData->vabSenseRinging.gain;
                    } else if (pLineObj->registers.sysState[0] & VP886_R_STATE_POL) {
                        offset = pCalData->vabSenseReverse.offset;
                        gain = pCalData->vabSenseReverse.gain;
                    } else {
                        offset = pCalData->vabSenseNormal.offset;
                        gain = pCalData->vabSenseNormal.gain;
                    }
                    avgSign = -1;
                    break;
                case VP_LOOP_COND_TEST_VSAG:
                    offset = pCalData->tipSense.offset;
                    gain = pCalData->tipSense.gain;
                    avgSign = -1;
                    break;
                case VP_LOOP_COND_TEST_VSBG:
                    offset = pCalData->ringSense.offset;
                    gain = pCalData->ringSense.gain;
                    avgSign = -1;
                    break;
                case VP_LOOP_COND_TEST_IMT:
                    if ((slicState == VP886_R_STATE_SS_BAL_RING) || (slicState == VP886_R_STATE_SS_UNBAL_RING)) {
                        offset = pCalData->vadcActive.offset;
                        gain = pCalData->vadcActive.gain;
                        avgSign = -1;
                        usedVadc = TRUE;
                        /* If low ILR is enabled, imt should be halved */
                        /* As the offset is applied before the gain and SHOUDN'T be halved, it
                           is temporarily multiplied by 2 */
                        if (pLineObj->lowIlr) {
                            gain /= 2;
                            offset *= 2;
                        }
                    } else {
                        offset = pCalData->sadc.offset;
                        gain = pCalData->sadc.gain;
                        avgSign = -1;
                    }
                    break;
                case VP_LOOP_COND_TEST_ILG:
                    /* current measurements only take sadc gain and offset into account */
                    offset = pCalData->sadc.offset;
                    gain = pCalData->sadc.gain;
                    avgSign = -1;
                    break;
                case VP_LOOP_COND_TEST_BAT1:
                    offset = pCalData->swySense.offset;
                    gain = pCalData->swySense.gain;
                    avgSign = -1;
                    break;
                case VP_LOOP_COND_TEST_BAT2:
                    offset = pCalData->swzSense.offset;
                    gain = pCalData->swzSense.gain;
                    avgSign = -1;
                    break;
                case VP_LOOP_COND_TEST_BAT3:
                   /*
                    * mid bat is not supported by this device. However,
                    * we will be monitoring the new IO2 voltage sense 
                    * signal instead. IO2 calibration is only relevant if IO2 is defined
                    * to be in "Voltage Monitor Mode" in the device profile.
                    */
                    offset = pCalData->io2Sense.offset;
                    gain = pCalData->io2Sense.gain;
                    avgSign = -1;
                    break;
                default:
                    /* should never get here */
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM invalid signal result"));
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                    break;
            }

            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM useVadc?: %i", usedVadc));
            if (usedVadc) {
                /* get the data and generate the event */
                if (!Vp886VadcGetMathData(pLineCtx, &mathData, offset, gain)) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                    VP_ERROR(VpLineCtxType, pLineCtx, ("LOOP_COND_GET_DATA vadc calculations failed"));
                    break;
                }
            } else {
                /* get the data and generate the event */
                if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain)) {
                    Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                    VP_ERROR(VpLineCtxType, pLineCtx, ("LOOP_COND_GET_DATA sadc calculations failed"));
                    break;
                }
            }

            average = mathData.average * avgSign;

            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM offset %d gain %d", offset, gain));
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM average: %li", average));

            /* place the calibrated data into the results struct */
            pLineObj->testResults.result.loopCond.data = (int16)average;

            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestGetLoopCondSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
