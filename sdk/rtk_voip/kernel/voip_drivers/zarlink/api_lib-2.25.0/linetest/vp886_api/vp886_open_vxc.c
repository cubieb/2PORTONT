/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11074 $
 * $LastChangedDate: 2013-07-18 14:41:08 -0500 (Thu, 18 Jul 2013) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

#define VAB_ERR_SET_THRES       4       /* +/- 4 adu err*/
#define VAG_ERR_SET_THRES       4       /* +/- 4 adu err*/
#define VBG_ERR_SET_THRES       4       /* +/- 4 adu err*/

typedef enum {
    OPEN_VXC_DISABLE_FEED,
    OPEN_VXC_WAIT_FOR_DISABLE_FEED,

    OPEN_VXC_COLLAPSE_FEED,
    OPEN_VXC_WAIT_FOR_COLLAPSE_FEED,

    OPEN_VXC_RELAX_FEED,

    OPEN_VXC_GET_TIMESTAMP,

    OPEN_VXC_START_SADC_MEASURE,
    OPEN_VXC_GET_SADC_VALUES,

    OPEN_VXC_CLEAN_UP,

    OPEN_VXC_GEN_EVENT,
    OPEN_VXC_MAX,
    OPEN_VXC_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886OpenVxcStateTypes;

bool Vp886TestOpenVxcSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestOpenVType *pOpenVxcArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static bool
VxcCollapseFeed(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj);

static bool
VxcRelaxFeed(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    bool calMode);

static bool
VxcStartSadcMeasure(
    VpLineCtxType *pLineCtx,
    uint16 settlingTime,
    uint16 integrateTime);

static bool
VxcHasLineSettled(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap,
    bool *pSettled);


/*------------------------------------------------------------------------------
 * Vp886TestOpenVxc
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestOpenVxc(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTestOpenVType *pOpenVxcArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTestOpenVType *pInputArgs = (VpTestOpenVType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxc NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results values */
        pLineObj->testResults.result.vxc.vdc = 0;
        pLineObj->testResults.result.vxc.vac = 0;

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.openV, pInputArgs, sizeof(VpTestOpenVType));

        /* we will be using the generic previous value info */
        pTestHeap->prevVal_1 = 0;
        pTestHeap->prevVal_2 = 0;
        pTestHeap->prevVal_3 = 0;
        pTestHeap->timeStamp = 0;

        /* make sure the requested lead is valid */
        if ((pInputArgs->tip != VP_TEST_TIP) &&
            (pInputArgs->tip != VP_TEST_RING) &
            (pInputArgs->tip != VP_TEST_TIP_RING) ) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxc invalid lead: %i", pInputArgs->tip));
            return VP_STATUS_INVALID_ARG;
        }

        /* don't need to collapse the feed over and over */
        if (pInputArgs->calMode == TRUE) {
            pTestHeap->nextState = OPEN_VXC_DISABLE_FEED;
        } else {
            pTestHeap->nextState = OPEN_VXC_RELAX_FEED;
        }
    }

    pOpenVxcArgs = &pTestHeap->testArgs.openV;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestOpenVxcSM(pLineCtx, pLineObj, pOpenVxcArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestOpenVxcSM
 *----------------------------------------------------------------------------*/
bool Vp886TestOpenVxcSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestOpenVType *pOpenVxcArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {

        case OPEN_VXC_DISABLE_FEED:
            /* Go to the disconnect state with codec activated.  Setting the polrev
               bit gives extra range, (-225 to 240) instead of (-240 to 190) */
            pLineObj->registers.sysState[0] = (VP886_R_STATE_CODEC | VP886_R_STATE_SS_DISCONNECT | VP886_R_STATE_POL);
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM: Cannot go to disconnect"));
                break;
            }

            /* get into a non-low power state and disable the SLIC bias' */
            pLineObj->registers.icr1[0] = 0xFF;
            pLineObj->registers.icr1[1] = 0x00;
            pLineObj->registers.icr1[2] = 0xCF;
            pLineObj->registers.icr1[3] = 0x40;
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("VxcCollapseFeed() failed VP886_R_ICR1_WRT"));
                return FALSE;
            }

            pTestHeap->nextState = OPEN_VXC_WAIT_FOR_DISABLE_FEED;
            requestNextState = TRUE;
            break;

        case OPEN_VXC_WAIT_FOR_DISABLE_FEED:
            /* ensure the line gets into disconnect */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 20, 0, OPEN_VXC_WAIT_FOR_DISABLE_FEED) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM: Cannot add to timer que."));
                break;
            }
            pTestHeap->nextState = OPEN_VXC_COLLAPSE_FEED;
            break;

        case OPEN_VXC_COLLAPSE_FEED:
            /* Set the line to drive Tip/Ring towards 0V */
            if (VxcCollapseFeed(pLineCtx, pLineObj) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM: Cannot add to timer que."));
                break;
            }
            pTestHeap->nextState = OPEN_VXC_WAIT_FOR_COLLAPSE_FEED;
            requestNextState = TRUE;
            break;

        case OPEN_VXC_WAIT_FOR_COLLAPSE_FEED:
            /* Wait for Tip/Ring to get near 0V */
            if (Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 400, 0, OPEN_VXC_WAIT_FOR_COLLAPSE_FEED) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM: Cannot add to timer que."));
                break;
            }
            pTestHeap->nextState = OPEN_VXC_RELAX_FEED;
            break;

        case OPEN_VXC_RELAX_FEED:
            /* Stop Driving the Feed to ground and prep for measurements */
            if (VxcRelaxFeed(pLineCtx, pLineObj, pOpenVxcArgs->calMode) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM: Cannot relax feed"));
                break;
            }
            pTestHeap->nextState = OPEN_VXC_GET_TIMESTAMP;
            requestNextState = TRUE;
            break;

        case OPEN_VXC_GET_TIMESTAMP:
            pTestHeap->timeStamp = Vp886GetTimestamp32(pLineCtx->pDevCtx);
            pTestHeap->nextState = OPEN_VXC_START_SADC_MEASURE;
            requestNextState = TRUE;
            break;


        case OPEN_VXC_START_SADC_MEASURE:
            if (VxcStartSadcMeasure(pLineCtx, pOpenVxcArgs->settlingTime, pOpenVxcArgs->integrateTime) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM unable to setup SADC for settle"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                break;
            }
            pTestHeap->nextState = OPEN_VXC_GET_SADC_VALUES;
            break;

        case OPEN_VXC_GET_SADC_VALUES: {

            bool settled = FALSE;
            if (VxcHasLineSettled(pLineCtx, pLineObj, pTestHeap, &settled) == FALSE) {
                /* VxcHasLineSettled() will generate an event if something goes wrong */
                break;
            }

            if (settled == FALSE) {
                /* line has not settled need to try again */
                pTestHeap->nextState = OPEN_VXC_START_SADC_MEASURE;
            } else {
                /* line has settled */
                pTestHeap->nextState = OPEN_VXC_CLEAN_UP;
            }

            requestNextState = TRUE;
            break;
        }

        case OPEN_VXC_CLEAN_UP :
            /* Disable the line control circuits */
            pLineObj->registers.icr3[0] = 0x01;
            pLineObj->registers.icr3[1] = 0x01;
            pLineObj->registers.icr3[2] = 0x00;
            pLineObj->registers.icr3[3] = 0x00;
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM() failed VP886_R_ICR3_WRT"));
            }

            /* restore icr 1 */
            VpMemCpy(pLineObj->registers.icr1, pTestHeap->scratch.icr1, VP886_R_ICR1_LEN);
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM() failed VP886_R_ICR1_WRT"));
            }
            
            /* restore icr 2 */
            VpMemCpy(pLineObj->registers.icr2, pTestHeap->scratch.icr2, VP886_R_ICR2_LEN);
            if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM() failed VP886_R_ICR2_WRT"));
            }

            pTestHeap->nextState = OPEN_VXC_GEN_EVENT;
            requestNextState = TRUE;

            break;

        case OPEN_VXC_GEN_EVENT:
            /* generate the final event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestOpenVxcSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

/*------------------------------------------------------------------------------
 * VxcCollapseFeed
 * This function is used to collapes the tip/ring feed by using the
 * ringing generator to driver towards 0V.
 *----------------------------------------------------------------------------*/
static bool
VxcCollapseFeed(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj)
{

    /* Ensure the tip and ring sense signals are on and ringing time constant is on */
    pLineObj->registers.icr2[0] = 0xFF;
    pLineObj->registers.icr2[1] = 0xDC;
    pLineObj->registers.icr2[2] = 0xFF;
    pLineObj->registers.icr2[3] = 0x29;
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcCollapseFeed() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }

    /* Ensure the line control circuits are on */
    pLineObj->registers.icr3[0] = 0xFF;
    pLineObj->registers.icr3[1] = 0x35;
    pLineObj->registers.icr3[2] = 0xFF;
    pLineObj->registers.icr3[3] = 0x94;
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcCollapseFeed() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }

    /* Connect External Ctrl Network to AC Feedback Loop */
    pLineObj->registers.icr4[0] = 0xFF;
    pLineObj->registers.icr4[1] = 0x00;
    pLineObj->registers.icr4[0] = 0xFF;
    pLineObj->registers.icr4[1] = 0x00;
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcCollapseFeed() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    /* enabling SLIC driver bias to actually collapse the feed */
    pLineObj->registers.icr1[0] = 0xFF;
    pLineObj->registers.icr1[1] = 0xFF;
    pLineObj->registers.icr1[2] = 0xCF;
    pLineObj->registers.icr1[3] = 0x4F;
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcCollapseFeed() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }

    return TRUE;
}

static bool
VxcRelaxFeed(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    bool calMode)
{
    /* Stop driving the feed and let the device take over but ensure that we can measure in disconnect */

    /*
     * 0x0C - Ensure the tip and ring sense signals are on.
     * 0x10 - Ensure the ila_fdrng bit is set so that we can measure up to 150V.
     */
    pLineObj->registers.icr2[0] = 0x1C;
    pLineObj->registers.icr2[1] = 0x1C;
    pLineObj->registers.icr2[2] = 0x00;
    pLineObj->registers.icr2[3] = 0x00;
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcRelaxFeed() failed VP886_R_ICR2_WRT"));
        return FALSE;
    }

    /* Ensure the line control circuits are on */
    pLineObj->registers.icr3[0] = 0x21;
    pLineObj->registers.icr3[1] = 0x21;
    pLineObj->registers.icr3[2] = 0x00;
    pLineObj->registers.icr3[3] = 0x00;
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcRelaxFeed() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }

    /* Connect External Ctrl Network to AC Feedback Loop */
    pLineObj->registers.icr4[0] = 0x01;
    pLineObj->registers.icr4[1] = 0x01;
    pLineObj->registers.icr4[2] = 0x00;
    pLineObj->registers.icr4[3] = 0x00;
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcRelaxFeed() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    /* stop driving the feed */
    if (calMode) {
        pLineObj->registers.icr1[0] = 0xFF;
        pLineObj->registers.icr1[1] = 0x00;
        pLineObj->registers.icr1[2] = 0xCF;
        pLineObj->registers.icr1[3] = 0x40;

        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VxcRelaxFeed() failed VP886_R_ICR1_WRT"));
            return FALSE;
        }
    }
    return TRUE;
}

static bool
VxcStartSadcMeasure(
    VpLineCtxType *pLineCtx,
    uint16 settlingTime,
    uint16 integrateTime)
{
    /* start the ADC to measure VAB, VA and VB */
    return Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
            VP886_SADC_SEL_MET_VDC,
            VP886_SADC_SEL_TIP,
            VP886_SADC_SEL_RING,
            VP886_SADC_SEL_NO_CONN,
            VP886_SADC_SEL_NO_CONN,
            settlingTime,
            integrateTime,
            VP886_LINE_TEST);
}

/*------------------------------------------------------------------------------
 * VxcHasLineSettled
  *----------------------------------------------------------------------------*/
static bool
VxcHasLineSettled(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    Vp886TestHeapType *pTestHeap,
    bool *pSettled)
{
    int16 offset, gain;
    int32 avgVab, avgVag, avgVbg, rmsVab, rmsVag, rmsVbg;
    Vp886AdcMathDataType mathData;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
    VpTestOpenVType *pOpenVxcArgs = &pTestHeap->testArgs.openV;

    /* get Tip/Ring average */
    /* using vabSenseRinging values because ila_fdrng bit is set in icr2 */
    offset = pCalData->vabSenseRinging.offset;
    gain = pCalData->vabSenseRinging.gain;
    if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain)) {
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() vab calculations failed"));
        return FALSE;
    }
    avgVab = -1 * mathData.average;
    rmsVab = mathData.rms;

    /* get Tip/Gnd average */
    offset = pCalData->tipSense.offset;
    gain = pCalData->tipSense.gain;
    if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_2, &mathData, offset, gain)) {
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() vag calculations failed"));
        return FALSE;
    }
    avgVag = -1 * mathData.average;
    rmsVag = mathData.rms;


    /* get Ring/Gnd average */
    offset = pCalData->ringSense.offset;
    gain = pCalData->ringSense.gain;
    if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_3, &mathData, offset, gain)) {
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
        VP_ERROR(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() vbg calculations failed"));
        return FALSE;
    }
    avgVbg = -1 * mathData.average;
    rmsVbg = mathData.rms;

    VP_TEST(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() settled prev %li,%li,%li",
        pTestHeap->prevVal_1, pTestHeap->prevVal_2, pTestHeap->prevVal_3));

    VP_TEST(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() settled new %li,%li,%li",
        avgVab, avgVag, avgVbg));

    /* check if the measurments have settled */
    if ((ABS(pTestHeap->prevVal_1 - avgVab) > VAB_ERR_SET_THRES) ||
        (ABS(pTestHeap->prevVal_2 - avgVag) > VAG_ERR_SET_THRES) ||
        (ABS(pTestHeap->prevVal_3 - avgVbg) > VBG_ERR_SET_THRES) ) {

        /* if the cal mode is true then we are forcing the primitive to not worry about settling */
        VP_TEST(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() nope"));
        *pSettled = FALSE;
    } else {
        *pSettled = TRUE;
    }

    if (*pSettled == FALSE) {
        if (pOpenVxcArgs->calMode == FALSE) {
            /* the primitive caller assumes the line is already settled */
            VP_TEST(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() forced to stop"));
            *pSettled = TRUE;

        } else if ((Vp886GetTimestamp32(pLineCtx->pDevCtx) - pTestHeap->timeStamp) >= 4000) {
            /* if we have been setting for more than 2.0 sec its time to move on */
            VP_TEST(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() timed out"));
            *pSettled = TRUE;
        }
    }

    if (*pSettled == TRUE) {
        VP_TEST(VpLineCtxType, pLineCtx, ("VxcHasLineSettled() yep"));
        if ( pOpenVxcArgs->tip == VP_TEST_TIP_RING) {
            pLineObj->testResults.result.vxc.vdc = avgVab;
            pLineObj->testResults.result.vxc.vac = rmsVab;
        } else if (pOpenVxcArgs->tip == VP_TEST_TIP) {
            pLineObj->testResults.result.vxc.vdc = avgVag;
            pLineObj->testResults.result.vxc.vac = rmsVag;
        } else {
            pLineObj->testResults.result.vxc.vdc = avgVbg;
            pLineObj->testResults.result.vxc.vac = rmsVbg;
        }
    } else {
        /* save the previous values for the next loop */
        pTestHeap->prevVal_1 = avgVab;
        pTestHeap->prevVal_2 = avgVag;
        pTestHeap->prevVal_3 = avgVbg;
    }

    return TRUE;
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
