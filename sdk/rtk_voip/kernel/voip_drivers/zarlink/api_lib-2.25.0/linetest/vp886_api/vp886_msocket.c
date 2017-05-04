/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10457 $
 * $LastChangedDate: 2012-09-10 16:55:28 -0500 (Mon, 10 Sep 2012) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

#include "vp886_testline_int.h"
#define SETTLING_RANGE                  2      /* +/- 5 adu err*/
#define SETTLING_ITERATIONS             100     /* Max settling iterations */
#define SETTLING_DELAY                  (100*8) /* 100ms */
#define MEASUREMENT_DELAY_1             (200*8) /* 200ms */
#define MEASUREMENT_DELAY_2             (600*8) /* 600ms */
#define MEASUREMENT_DELAY_3             (600*8) /* 600ms */
#define INTEGRATION_TIME                (5*8)   /* 5ms */

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

typedef enum
{
    MSOCKET_CHECK_RELAY                 = 0,
    MSOCKET_CAL_MODE_SETUP              = 1,

    MSOCKET_SETUP                       = 2,

    MSOCKET_SET_NORMAL_FEED             = 3,
    MSOCKET_START_SETTLING_MEASUREMENT  = 4,
    MSOCKET_CHECK_SETTLING              = 5,

    MSOCKET_SET_REVERSE_FEED            = 6,
    MSOCKET_START_MEASUREMENT_1         = 7,
    MSOCKET_GET_MEASUREMENT_1           = 8,
    MSOCKET_START_MEASUREMENT_2         = 9,
    MSOCKET_GET_MEASUREMENT_2           = 10,
    MSOCKET_START_MEASUREMENT_3         = 11,
    MSOCKET_GET_MEASUREMENT_3           = 12,

    MSOCKET_RESTORE                     = 13,
    MSOCKET_GEN_EVENT                   = 14,

    MSOCKET_MAX                         = 14,
    MSOCKET_ENUM_SIZE                   = FORCE_STANDARD_C_ENUM_SIZE
}  Vp886MSocketStateTypes;

bool Vp886TestMSocketSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestMSocketType *pMSocketArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap);


static bool
VpMSocketSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpMSocketSetNormalFeed(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpMSocketSetReverseFeed(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

static bool
VpMSocketGetResult(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    Vp886SadcBufferType sadcBuf,
    int16 offset,
    int16 gain,
    int16 *pResult);

static bool
VpMSocketNeedMoreSettlingTime(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int16 currentValue);

static bool
VpMSocketRestore(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp886TestMSocket
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp886TestMSocket(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback)
{

    VpTestMSocketType *pMSocketArgs = NULL;
    VpStatusType status = VP_STATUS_SUCCESS;

    if (!callback) {
        const VpTestMSocketType *pInputArgs = (VpTestMSocketType *)pArgsUntyped;

        if (pInputArgs == NULL) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMSocket NULL input struct"));
            return VP_STATUS_INVALID_ARG;
        }

        /* reset the results to nonmeaningful values */
        VpMemSet(&pLineObj->testResults.result.mSock, 0, sizeof(VpTestResultMSockType));

        /* store the input args into the test heap for later use */
        VpMemCpy(&pTestHeap->testArgs.mSocket, pInputArgs, sizeof(VpTestMSocketType));

        /* set for first state */
        pTestHeap->nextState = MSOCKET_CHECK_RELAY;
    }

    pMSocketArgs = &pTestHeap->testArgs.mSocket;

    /* Continue calling the state machine until it wants to stop */
    while (Vp886TestMSocketSM(pLineCtx, pLineObj, pMSocketArgs, pTestInfo, pTestHeap));

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestMSocketSM
 *----------------------------------------------------------------------------*/
bool Vp886TestMSocketSM(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    VpTestMSocketType *pMSocketArgs,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap)
{
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;

    bool requestNextState = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMSocketSM state: %i", pTestHeap->nextState));

    switch (pTestHeap->nextState) {
        case MSOCKET_CHECK_RELAY: {
            /* This test cannot run if the isolate relay is open*/
            if (pLineObj->relayState == VP_RELAY_RESET) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMSocketSM: Relay open."));
                break;
            }
            pTestHeap->nextState = MSOCKET_CAL_MODE_SETUP;
            requestNextState = TRUE;
            break;
        }
        case MSOCKET_CAL_MODE_SETUP: {
            if (Vp886CalmodeTestSetup(pLineCtx, pTestHeap) == FALSE) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestMSocketSM failed Vp886CalmodeTestSetup"));
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = MSOCKET_SETUP;
            requestNextState = TRUE;
            break;
        }
        case MSOCKET_SETUP: {
            if(VpMSocketSetup(pLineCtx, pTestHeap)== FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = MSOCKET_SET_NORMAL_FEED;
            requestNextState = TRUE;
            break;
        }

        case MSOCKET_SET_NORMAL_FEED: {
            if(VpMSocketSetNormalFeed(pLineCtx, pTestHeap)== FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->settlingCount = 0;
            pTestHeap->previousAvg = 0;
            pTestHeap->nextState = MSOCKET_START_SETTLING_MEASUREMENT;
            requestNextState = TRUE;
            break;
        }

        case MSOCKET_START_SETTLING_MEASUREMENT: {
            Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                VP886_SADC_SEL_LONG_I,
                VP886_SADC_SEL_TIP,
                VP886_SADC_SEL_RING,
                VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN,
                SETTLING_DELAY,
                INTEGRATION_TIME,
                VP886_LINE_TEST);
            pTestHeap->nextState = MSOCKET_CHECK_SETTLING;
            break;
        }
        case MSOCKET_CHECK_SETTLING: {
            int16 imt;
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_1,
                pCalData->sadc.offset, pCalData->sadc.gain, &imt);

            if (VpMSocketNeedMoreSettlingTime(pLineCtx, pTestHeap, imt))
            {
                /* Need more time for the current to settle */
                pTestHeap->nextState = MSOCKET_START_SETTLING_MEASUREMENT;
            } else {
                /* Measurement settled.  Store the IMT, VA, and VB results
                   then move on. */
                pLineObj->testResults.result.mSock.imnf = imt;
                VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_2,
                    pCalData->tipSense.offset, pCalData->tipSense.gain,
                    &pLineObj->testResults.result.mSock.vanf);
                VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_3,
                    pCalData->ringSense.offset, pCalData->ringSense.gain,
                    &pLineObj->testResults.result.mSock.vbnf);

                pTestHeap->nextState = MSOCKET_SET_REVERSE_FEED;
            }
            requestNextState = TRUE;
            break;
        }

        case MSOCKET_SET_REVERSE_FEED: {
            if(VpMSocketSetReverseFeed(pLineCtx, pTestHeap)== FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
                break;
            }
            pTestHeap->nextState = MSOCKET_START_MEASUREMENT_1;
            requestNextState = TRUE;
            break;
        }

        case MSOCKET_START_MEASUREMENT_1: {
            Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                VP886_SADC_SEL_LONG_I,
                VP886_SADC_SEL_TIP,
                VP886_SADC_SEL_RING,
                VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN,
                MEASUREMENT_DELAY_1,
                INTEGRATION_TIME,
                VP886_LINE_TEST);
            pTestHeap->nextState = MSOCKET_GET_MEASUREMENT_1;
            break;
        }
        case MSOCKET_GET_MEASUREMENT_1: {
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_1,
                pCalData->tipSense.offset, pCalData->tipSense.gain,
                &pLineObj->testResults.result.mSock.imrf1);
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_2,
                pCalData->tipSense.offset, pCalData->tipSense.gain,
                &pLineObj->testResults.result.mSock.varf1);
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_3,
                pCalData->ringSense.offset, pCalData->ringSense.gain,
                &pLineObj->testResults.result.mSock.vbrf1);
            pTestHeap->nextState = MSOCKET_START_MEASUREMENT_2;
            requestNextState = TRUE;
            break;
        }

        case MSOCKET_START_MEASUREMENT_2: {
            Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                VP886_SADC_SEL_LONG_I,
                VP886_SADC_SEL_TIP,
                VP886_SADC_SEL_RING,
                VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN,
                MEASUREMENT_DELAY_2,
                INTEGRATION_TIME,
                VP886_LINE_TEST);
            pTestHeap->nextState = MSOCKET_GET_MEASUREMENT_2;
            break;
        }
        case MSOCKET_GET_MEASUREMENT_2: {
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_1,
                pCalData->tipSense.offset, pCalData->tipSense.gain,
                &pLineObj->testResults.result.mSock.imrf2);
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_2,
                pCalData->tipSense.offset, pCalData->tipSense.gain,
                &pLineObj->testResults.result.mSock.varf2);
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_3,
                pCalData->ringSense.offset, pCalData->ringSense.gain,
                &pLineObj->testResults.result.mSock.vbrf2);
            pTestHeap->nextState = MSOCKET_START_MEASUREMENT_3;
            requestNextState = TRUE;
            break;
        }

        case MSOCKET_START_MEASUREMENT_3: {
            Vp886SadcSetup(pLineCtx, VP886_SADC_MATH_GROUP, VP886_SADC_RATE_FULL,
                VP886_SADC_SEL_LONG_I,
                VP886_SADC_SEL_TIP,
                VP886_SADC_SEL_RING,
                VP886_SADC_SEL_NO_CONN,
                VP886_SADC_SEL_NO_CONN,
                MEASUREMENT_DELAY_3,
                INTEGRATION_TIME,
                VP886_LINE_TEST);
            pTestHeap->nextState = MSOCKET_GET_MEASUREMENT_3;
            break;
        }
        case MSOCKET_GET_MEASUREMENT_3: {
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_1,
                pCalData->tipSense.offset, pCalData->tipSense.gain,
                &pLineObj->testResults.result.mSock.imrf3);
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_2,
                pCalData->tipSense.offset, pCalData->tipSense.gain,
                &pLineObj->testResults.result.mSock.varf3);
            VpMSocketGetResult(pLineCtx, pTestHeap, VP886_SADC_BUF_3,
                pCalData->ringSense.offset, pCalData->ringSense.gain,
                &pLineObj->testResults.result.mSock.vbrf3);
            pTestHeap->nextState = MSOCKET_RESTORE;
            requestNextState = TRUE;
            break;
        }

        case MSOCKET_RESTORE:{
            if(VpMSocketRestore(pLineCtx, pTestHeap) == FALSE) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            }
            /* wait for the requested amount of time */
            if (! Vp886AddTimerMs(NULL, pLineCtx, VP886_TIMERID_LINE_TEST, 50, 0, MSOCKET_RESTORE)) {
                Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMSocketSM: Cannot add to timer queue."));
            }
            pTestHeap->nextState = MSOCKET_GEN_EVENT;
            break;
        }
        case MSOCKET_GEN_EVENT: {
            /* generate the event */
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_SUCCESS);
            break;
        }

        default:
            /* some how the next state variable was corrupted */
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestMSocketSM invalid state: %i",
                pTestHeap->nextState));
            Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
            break;
    }

    return requestNextState;
}
/*------------------------------------------------------------------------------
 * VpMSocketSetup()
 * This function is called by the Vp886TestMSocket() state machine during
 * the MSOCKET_SETUP state.
 *
 * This function is used to configure the line in low gain mode. Also forcing
 * the the battery voltage if possible and the VOC voltage to +6V.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *
 * Returns:  TRUE if no error, FALSE otherwise
 * --
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
VpMSocketSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;

    uint8 sigGenCtrl = VP886_R_SIGCTRL_EN_BIAS | VP886_R_SIGCTRL_EN_SIGA;
    uint8 lpSuper[VP886_R_LOOPSUP_LEN];

    VP_TEST(VpLineCtxType, pLineCtx, ("VpMSocketSetup()"));

    /* need to be in the active state */
    pLineObj->registers.sysState[0] = VP886_R_STATE_CODEC | VP886_R_STATE_SS_ACTIVE;

    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_STATE_WRT, VP886_R_STATE_LEN, pLineObj->registers.sysState) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetup failed VP886_R_STATE_WRT"));
        return FALSE;
    }

    /* Masking the hook detection by maxing out TSH */
    if (VpSlacRegRead(NULL, pLineCtx, VP886_R_LOOPSUP_RD, VP886_R_LOOPSUP_LEN, pLineObj->registers.loopSup) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetup failed VP886_R_LOOPSUP_RD"));
        return FALSE;
    }

    lpSuper[0] = (pLineObj->registers.loopSup[0]) | 0x07;
    lpSuper[1] = 0xFF;
    lpSuper[2] = pLineObj->registers.loopSup[2];
    lpSuper[3] = pLineObj->registers.loopSup[3];
    lpSuper[4] = pLineObj->registers.loopSup[4];

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, lpSuper) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetup failed VP886_R_LOOPSUP_WRT"));
        return FALSE;
    }

    if (VP886_IS_TRACKER(pDevObj)) {
        uint8 switchRegParam[VP886_R_SWPARAM_LEN] = {0x44, 0x04, 0x04}; /* battery 25V fixed */

        if (VpSlacRegRead(NULL, pLineCtx, VP886_R_SWPARAM_RD, VP886_R_SWPARAM_LEN, pLineObj->registers.swParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetup failed VP886_R_SWPARAM_RD"));
            return FALSE;
        }

        switchRegParam[0] = (pLineObj->registers.swParam[0] & 0xA0) | switchRegParam[0];  /*RTM=1, SWF[4:0] = 00100*/
        switchRegParam[1] = (pLineObj->registers.swParam[1] & 0xE0) | switchRegParam[1];  /*SWRV = 00100; */
        switchRegParam[2] = (pLineObj->registers.swParam[2] & 0xE0) | switchRegParam[2];  /*SWLV = 00100*/

        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, switchRegParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetup() failed VP886_R_SWPARAM_WRT"));
            return FALSE;
        }
    }

    pLineObj->registers.icr4[0] = 0xFF;
    pLineObj->registers.icr4[1] = 0x03;
    pLineObj->registers.icr4[2] = 0xFF;
    pLineObj->registers.icr4[3] = 0x2C;

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetup() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGCTRL_WRT, VP886_R_SIGCTRL_LEN, &sigGenCtrl) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetup() failed VP886_R_SIGCTRL_WRT"));
        return FALSE;
    }

    return TRUE;
}

static bool
VpMSocketSetNormalFeed(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
    uint8 sigGenA[VP886_R_SIGAB_LEN] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 dcfeed[VP886_R_DCFEED_LEN] = {0xEF, 0x3F};
    int32 metallic_mV;
    int32 buffErr;
    int16 feedBias;

    /* Force the line into Low gain mode to
     * that we can take the measurements we need
     */
    pLineObj->registers.icr1[0] = 0xFF;
    pLineObj->registers.icr1[1] = 0x01;
    if (VP886_IS_ABS(pDevObj)) {
        pLineObj->registers.icr1[2] = 0xFF;
    } else  {
        pLineObj->registers.icr1[2] = 0xCF;
    }
    pLineObj->registers.icr1[3] = 0x40;

    pLineObj->registers.icr2[0] = 0xFF;
    pLineObj->registers.icr2[1] = 0xFD;
    pLineObj->registers.icr2[2] &= 0x0C;
    pLineObj->registers.icr2[2] |= 0xF3;
    pLineObj->registers.icr2[3] &= 0x0C;
    if (VP886_IS_ABS(pDevObj)) {
        pLineObj->registers.icr2[3] |= 0x31;
    } else  {
        pLineObj->registers.icr2[3] |= 0x71;
    }

    pLineObj->registers.icr3[0] = 0xFF;
    pLineObj->registers.icr3[1] = 0xE9;
    pLineObj->registers.icr3[2] = 0xFF;
    pLineObj->registers.icr3[3] = 0xC0;

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetNormalFeed() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetNormalFeed() failed VP886_R_ICR2_WRT"));
        return FALSE;
    }

    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetNormalFeed() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }

    /* Set the Metallic Voltage to 6V, calibrated */
    metallic_mV = 6000 * 2;

    metallic_mV *= pCalData->vocSenseNormal.gain;
    metallic_mV = VpRoundedDivide(metallic_mV, 1000);

    buffErr = pCalData->ringingBuffer.offset * 1000;
    buffErr = VpRoundedDivide(buffErr, (int32)pCalData->ringingBuffer.gain * 40);
    metallic_mV -= buffErr;

    metallic_mV -= pCalData->ringingGenerator.offset;

    metallic_mV *= 1000;
    metallic_mV = VpRoundedDivide(metallic_mV, pCalData->ringingGenerator.gain);

    feedBias = VP886_UNIT_CONVERT(metallic_mV, VP886_UNIT_MV, VP886_UNIT_DAC_RING);
    
    sigGenA[1] = (uint8)((feedBias >> 8) & 0x00FF);
    sigGenA[2] = (uint8)(feedBias & 0x00FF);
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenA) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetNormalFeed() failed VP886_R_SIGAB_WRTT"));
        return FALSE;
    }

    /* Set the Longitudinal Voltage */
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, dcfeed) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetNormalFeed() failed VP886_R_DCFEED_WRT"));
        return FALSE;
    }

    return TRUE;
}

static bool
VpMSocketSetReverseFeed(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886CmnCalDeviceDataType *pCalData = &pDevObj->calData[pLineObj->channelId].cmn;
    uint8 sigGenA[VP886_R_SIGAB_LEN] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    int32 metallic_mV;
    int32 buffErr;
    int16 feedBias;

    /* Set the Metallic Voltage to -6V, calibrated */
    metallic_mV = -6000 * 2;

    metallic_mV *= pCalData->vocSenseNormal.gain;
    metallic_mV = VpRoundedDivide(metallic_mV, 1000);

    buffErr = pCalData->ringingBuffer.offset * 1000;
    buffErr = VpRoundedDivide(buffErr, (int32)pCalData->ringingBuffer.gain * 40);
    metallic_mV -= buffErr;

    metallic_mV -= pCalData->ringingGenerator.offset;

    metallic_mV *= 1000;
    metallic_mV = VpRoundedDivide(metallic_mV, pCalData->ringingGenerator.gain);

    feedBias = VP886_UNIT_CONVERT(metallic_mV, VP886_UNIT_MV, VP886_UNIT_DAC_RING);
    
    sigGenA[1] = (uint8)((feedBias >> 8) & 0x00FF);
    sigGenA[2] = (uint8)(feedBias & 0x00FF);
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenA) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketSetReverseFeed() failed VP886_R_SIGAB_WRTT"));
        return FALSE;
    }

    return TRUE;
}

static bool
VpMSocketGetResult(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    Vp886SadcBufferType sadcBuf,
    int16 offset,
    int16 gain,
    int16 *pResult)
{
    Vp886AdcMathDataType mathData;

    if (!Vp886SadcGetMathData(pLineCtx, sadcBuf, &mathData, offset, gain)) {
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_INTERNAL_ERROR);
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SadcGetMathData calculations failed"));
        return FALSE;
    }
    *pResult = (int16)(-1 * mathData.average);
    
    return TRUE;
}

static bool
VpMSocketNeedMoreSettlingTime(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap,
    int16 currentValue)
{
    bool settleMore = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpMSocketNeedMoreSettlingTime() cVal %i, pVal %i",
        currentValue, pTestHeap->previousAvg));

    if (ABS(currentValue - pTestHeap->previousAvg) > SETTLING_RANGE) {
        /* Measurement is not stable, need more time to settle */
        pTestHeap->previousAvg = currentValue;
        pTestHeap->settlingCount++;
        settleMore = TRUE;
    } else if ((currentValue == VP_INT16_MIN) || (currentValue == VP_INT16_MAX)) {
        /* Measurement saturates, need some time to get out of saturation */
        pTestHeap->settlingCount++;
        settleMore = TRUE;
    }

    /* if the measurement is not stable then how long has it been not stable */
    VP_TEST(VpLineCtxType, pLineCtx, ("VpMSocketNeedMoreSettlingTime() settlingCount %i :settleMore %i",
            pTestHeap->settlingCount, settleMore));

    if (settleMore) {
        if (pTestHeap->settlingCount < SETTLING_ITERATIONS) {
            VP_TEST(VpLineCtxType, pLineCtx,
                ("VpMSocketNeedMoreSettlingTime(): Another measurement is required"));
            return TRUE;
        } else {
            settleMore = FALSE;
            VP_TEST(VpLineCtxType, pLineCtx,
                ("VpMSocketNeedMoreSettlingTime(): Took too long to settle, reduced accuracy"));
        }
    }

    return settleMore;
}

/*------------------------------------------------------------------------------
 * VpMSocketRestore()
 * This function is called after the measusrment is completed to restore
 * the registers that were
 * modified during the test to their original value.
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
VpMSocketRestore(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    /* Restore registers */
    /* Must take the slic out of low gain state before restoring */
    /* registers otherwise risk huge transients. */

    Vp886DeviceObjectType *pDevObj = pLineCtx->pDevCtx->pDevObj;
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

    pLineObj->registers.sysState[0] = pTestHeap->scratch.sysState[0];

    /*restore loop supervision*/
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_LOOPSUP_WRT, VP886_R_LOOPSUP_LEN, pTestHeap->scratch.loopSup) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketRestore() failed VP886_R_LOOPSUP_WRT"));
        return FALSE;
    }
    /*restore dcfeed*/
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_DCFEED_WRT, VP886_R_DCFEED_LEN, pTestHeap->scratch.dcFeed) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketRestore() failed VP886_R_DCFEED_WRT"));
        return FALSE;
    }
    if (VP886_IS_TRACKER(pDevObj)) {
        if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SWPARAM_WRT, VP886_R_SWPARAM_LEN, pLineObj->registers.swParam) == FALSE) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketRestore() failed VP886_R_SWPARAM_WRT"));
            return FALSE;
        }
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pTestHeap->scratch.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketRestore() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pTestHeap->scratch.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketRestore() failed VP886_R_ICR2_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pTestHeap->scratch.icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketRestore() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pTestHeap->scratch.icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("VpMSocketRestore() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    return TRUE;
}


#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
