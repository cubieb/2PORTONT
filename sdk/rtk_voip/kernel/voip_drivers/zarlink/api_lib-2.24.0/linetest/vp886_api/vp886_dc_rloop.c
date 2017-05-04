/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11380 $
 * $LastChangedDate: 2014-04-10 12:55:59 -0500 (Thu, 10 Apr 2014) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"

typedef enum
{
    DC_RLOOP_SETUP              = 0,
    DC_RLOOP_START_SADC_MEASURE = 1,
    DC_RLOOP_GET_DATA           = 2,
    DC_RLOOP_REMOVE_TARGET_I    = 3,
    DC_RLOOP_GEN_EVENT          = 4,
    DC_RLOOP_MAX                = 4,
    DC_RLOOP_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886DcRloopStateTypes;

bool Vp886TestDcRloopSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestDcRLoopType *pDcRloopArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);

static bool VpDcRloopSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpSetTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int32 iTarget);

static bool
VpRemoveTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);


/*------------------------------------------------------------------------------
 * Vp886TestDcRloop
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestDcRloop(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTestDcRLoopType *pDcRloopArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTestDcRLoopType *pInputArgs = (VpTestDcRLoopType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestDcRloop NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results to nonmeaningful values */
        pLineObj->testResults.result.dcvab.vab = 0;
        pLineObj->testResults.result.dcvab.ilg = 0;

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.dcRloop, pInputArgs, sizeof(VpTestDcRLoopType));

        if (pInputArgs->iTestLevel == 0) {
            /*
             * Special case to simply remove the targe current
             * that may have been set up by a previous call to
             * to this primitive.
             */
            pTestHeap->nextState = DC_RLOOP_REMOVE_TARGET_I;
        } else {
            pTestHeap->nextState = DC_RLOOP_SETUP;
        }
    }

    pDcRloopArgs = &pTestHeap->testArgs.dcRloop;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestDcRloopSM(pLineCtx, pLineObj, pDcRloopArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestDcRloopSM
 *----------------------------------------------------------------------------*/
bool Vp886TestDcRloopSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestDcRLoopType *pDcRloopArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestDcRloopSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {
        case DC_RLOOP_SETUP: {
            if (VpDcRloopSetup(pLineCtx, pTestHeap)==FALSE)
            {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestDcRloopSM failed measurement setup"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                requestNextState = TRUE;
                pTestHeap->nextState = DC_RLOOP_REMOVE_TARGET_I;
            }
            /* wait for the requested amount of time */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 100, 0, DC_RLOOP_SETUP)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("VpAcRloopOffsetSetup: Cannot add to timer queue."));
            }
            pTestHeap->nextState = DC_RLOOP_START_SADC_MEASURE;
            break;
        }
        case DC_RLOOP_START_SADC_MEASURE: {
            /* start the ADC to measure VSAB */
            if( Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                    VP886_SADC_SEL_LONG_I,
                    VP886_SADC_SEL_MET_VDC,
                    VP886_SADC_SEL_MET_I,
                    VP886_SADC_SEL_NO_CONN,
                    VP886_SADC_SEL_NO_CONN,
                    pDcRloopArgs->settlingTime,
                    pDcRloopArgs->integrateTime,
                    VP886_LINE_TEST) == FALSE)

            {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestAcRloopSM unable to setup SADC for offset measurement"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                requestNextState = TRUE;
                pTestHeap->nextState = DC_RLOOP_REMOVE_TARGET_I;
                break;
            }
            pTestHeap->nextState = DC_RLOOP_GET_DATA;
            break;
        }

        case DC_RLOOP_GET_DATA: {
            /* Read the sum of the squared 16 samples from the SADC buffer and
             * Calculate the rms value
             */
            Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
            Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
            Vp886AdcMathDataType mathData;

            int16 offset = 0;
            int16 gain   = 0;

            /*ilg*/
            offset = pCalData->sadc.offset;
            gain   = pCalData->sadc.gain;
            /* get the data and generate the event */
            if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_1, &mathData, offset, gain)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("DC_RLOOP_GET_DATA VP886_SADC_BUF_1 sadc calculations failed"));
            }
            pLineObj->testResults.result.dcvab.ilg = (int16)mathData.average;

            /*vsab*/
            offset = pCalData->vabSenseNormal.offset;
            gain = pCalData->vabSenseNormal.gain;
            /* get the data and generate the event */
            if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_2, &mathData, offset, gain)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("DC_RLOOP_GET_DATA VP886_SADC_BUF_2 sadc calculations failed"));
            }
            pLineObj->testResults.result.dcvab.vab = -1*(int16)mathData.average;

            /* imt: debug only */
#if defined(VP_DEBUG) && (VP_CC_DEBUG_SELECT & VP_DBG_TEST)

            offset = pCalData->sadc.offset;
            gain = pCalData->sadc.gain;
            if (!Vp886SadcGetMathData(pLineCtx, VP886_SADC_BUF_3, &mathData, offset, gain)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                VP_ERROR(VpLineCtxType, pLineCtx, ("DC_RLOOP_GET_DATA VP886_SADC_BUF_3 sadc calculations failed"));
            }
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestDcRloopSM imt: %i", -1*(int16)mathData.average));
#endif

            pTestHeap->nextState = DC_RLOOP_REMOVE_TARGET_I;
            requestNextState = TRUE;
            break;
        }

        case DC_RLOOP_REMOVE_TARGET_I: {

            if (VpRemoveTargetCurrent(pLineCtx, pTestHeap)==FALSE)
            {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestDcRloopSM failed VpRemoveTargetCurrent"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            }

            pTestHeap->nextState = DC_RLOOP_GEN_EVENT;
            requestNextState = TRUE;
            break;
        }

        case DC_RLOOP_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestDcRloopSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}

/*------------------------------------------------------------------------------
 * VpDcRloopSetup
 * This function is called by the Vp886TestDcRloopSM() state machine during
 * the DC_RLOOP_SETUP state.
 *
 * This function is used to configure the line for the dc rloop test.
 *
 * Parameters:
 *  pLineCtx         - pointer to the line context.
 *  pTestHeap        - Pointer to the stored data in the scratch pad
 *
 * Returns:
 *         TRUE if no error, FALSE otherwise
 * --
 * Result:
 *  -Verify that the user requested iTestLevel current level is within
 *      supported current range.
 *  -Configure the device to force a current of iTestLevel to the loop
 *----------------------------------------------------------------------------*/
static bool VpDcRloopSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpTestDcRLoopType *pDcRloopData = &pTestHeap->testArgs.dcRloop;
    /*if calMode is true, or if the requested test current is lower than the lowest
     * DC feed ILA current limit capability
     * then set registers appropriately to handle the low current
     */
    int32 iTestLevelUA = (int16)VP886_UNIT_CONVERT(pDcRloopData->iTestLevel,
                                VP886_UNIT_ADC_IMET_NOR, VP886_UNIT_UA);
    if (pDcRloopData->calMode || (ABS(iTestLevelUA)  < 18000)) {
        if (Vp886CalmodeTestSetup(pLineCtx, pTestHeap)==FALSE)
            {return FALSE;}
    }
    /* Disable AISN */
    pLineObj->registers.icr4[0] = 0x10;
    pLineObj->registers.icr4[1] = 0x00;
    pLineObj->registers.icr4[2] = 0x00;
    pLineObj->registers.icr4[3] = 0x00;

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4)==FALSE)
        {return FALSE;}

    return VpSetTargetCurrent(pLineCtx, pTestHeap, iTestLevelUA);
}

static bool
VpSetTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int32 iTarget)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

    /*Supported current range for this test is iTestLow = -10 to 10mA
     * iTestHigh = 18 to 49mA
     */

    #define VP886_UNIT_DC_BIAS_CURRENT 3494L  /*3601L*/
    if ((iTarget >= -10000) && (iTarget <= 10000)) {
        uint8 gain = VP886_R_VPGAIN_AR_GAIN;
        int32 dcBias = 0;
        uint8 ringGen[VP886_R_RINGGEN_LEN] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        uint8 sigGenCtrl=0x00;

        /*
         *  Using signal generator A bias to produce target currents between
         * -10 mA and 10 mA.
         */

        /* disable the signal generator */
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl)==FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent failed VP886_R_SIGCTRL_WRT"));
            return FALSE;
        }

        /* To get the +/-10mA range we have to gain up the voice path */

        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_VPGAIN_WRT, VP886_R_VPGAIN_LEN, &gain)==FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent failed VP886_R_VPGAIN_WRT"));
            return FALSE;
        }

        /* Convert target to a DAC (sig gen a bias) */
        dcBias = VP886_UNIT_CONVERT(iTarget, VP886_UNIT_UA,
            VP886_UNIT_DC_BIAS_CURRENT);

        /* Contain value to dc bias to parameter's bits */
        dcBias = (dcBias < -32768) ? (-32678) : (dcBias);
        dcBias = (dcBias > 32767) ? (32677) : (dcBias);

        /* write bias to the device */
        ringGen[1] = (uint8)((dcBias >> 8) & 0xFF);
        ringGen[2] = (uint8)(dcBias & 0xFF);

        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, ringGen)==FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent failed VP886_R_RINGGEN_WRT"));
            return FALSE;
        }

        /* enable the signal generator */
        sigGenCtrl = VP886_R_SIGCTRL_EN_BIAS;
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl)==FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent failed VP886_R_DCFEED_RD"));
            return FALSE;
        }

        /* Disable the Feed Circuit from the IHL pin. Disable DC Feed ILA.
           Enable metallic speedup. */
        pLineObj->registers.icr2[0] = 0xC0;
        pLineObj->registers.icr2[1] = 0x00;
        pLineObj->registers.icr2[2] |= 0x80;
        pLineObj->registers.icr2[3] |= 0x80;

    } else if ( (iTarget >= 18000) && (iTarget <= 49000) ) {
        uint8 dcFeed[VP886_R_DCFEED_LEN];
        int32 ilaFeed = 0;

        /*
         * Using the dcfeed registers ILA parameter to set target
         * currents between 18mA and 49mA. However, the accuracy of
         * of the ILA parameter needs to be adjusted by the ILA calibration
         * data that was taken by Cal Line.
         */
        /* Masking the hook detection by maxing out TSH */
        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_DCFEED_RD, VP886_R_DCFEED_LEN, pLineObj->registers.dcFeed) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent failed VP886_R_DCFEED_RD"));
            return FALSE;
        }

         /* clear out the LI and ila parameters from dc feed register */
        dcFeed[0] = (pLineObj->registers.dcFeed[0] & ~(VP886_R_DCFEED_LONG_IMPED));
        dcFeed[1] = (pLineObj->registers.dcFeed[1] & ~(VP886_R_DCFEED_ILA));
        ilaFeed = iTarget;

        /* convert it to ILA dc feed register format*/
        ilaFeed = (ilaFeed-18000) / 1000;


        /* Contain value to ILA parameter's bits */
        ilaFeed = (ilaFeed < 0) ? (0) : (ilaFeed);
        ilaFeed = (ilaFeed > 31) ? (31) : (ilaFeed);

        /* OR in the adjusted target ILA parameter and write to the device */
        dcFeed[1] |= (uint8)ilaFeed;

        /* write to the device */
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcFeed)==FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886AcRloopSetup failed VP886_R_DCFEED_WRT"));
            return FALSE;
        }

        pLineObj->registers.icr2[0] = 0x00;
        pLineObj->registers.icr2[1] = 0x00;
        pLineObj->registers.icr2[2] |= 0x80;
        pLineObj->registers.icr2[3] |= 0x80;

    } else {
        /* This is a current region unsupported by the primitive */
        VP_TEST(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent() invalid target current %li uA",
            iTarget));
        return FALSE;
    }

    /* Put device into active */
    pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_SS_ACTIVE;

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState)== FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpSetTargetCurrent() failed VP886_R_STATE_WRT"));
        return FALSE;
    }

    return VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2);
}

/*------------------------------------------------------------------------------
 * VpRemoveTargetCurrent
 * This function is called by the Vp886TestDcRloopSM() state machine
 * after the measusrment is completed to restore the registers that were
 * modified during the test to their original value.
 * This is important since this test is called by the ROH test twice without
 * an intermediary conclude in between.
 *
 *
 * Parameters:
 *  pLineCtx         - pointer to the line context.
 *  pTestHeap        - Pointer to the stored data in the scratch pad
 *
 * Returns:
 *         TRUE if no error, FALSE otherwise
 * --
 * Result:
 *  -restore the device registers that are modified during the test to their
 *     pre-test value
 *----------------------------------------------------------------------------*/

static bool
VpRemoveTargetCurrent (
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{

    /*restore the voice path gain*/
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_VPGAIN_WRT, VP886_R_VPGAIN_LEN, pTestHeap->scratch.vpGain)==FALSE)
    {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpRemoveTargetCurrent() failed VP886_R_VPGAIN_WRT"));
        return FALSE;
    }
    /*restore dcfeed*/
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, pTestHeap->scratch.dcFeed)==FALSE)
    {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpRemoveTargetCurrent() failed VP886_R_DCFEED_WRT"));
        return FALSE;
    }
    /*restore the System state*/
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pTestHeap->scratch.sysState)==FALSE)
    {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpRemoveTargetCurrent() failed VP886_R_STATE_WRT"));
        return FALSE;
    }
    /*restore the Signal Generator Control*/
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, pTestHeap->scratch.sigGenCtrl)==FALSE)
    {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpRemoveTargetCurrent() failed VP886_R_SIGCTRL_WRT"));
        return FALSE;
    }

    return TRUE;
}


#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
