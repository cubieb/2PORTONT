/*
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2010-07-27 16:29:14 -0500 (Tue, 27 Jul 2010) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_890_SERIES)
#ifdef VP890_INCLUDE_TESTLINE_CODE
#if (VP890_INCLUDE_LINE_TEST_PACKAGE == VP890_LINE_TEST_PROFESSIONAL)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp890_api.h"
#include "vp890_api_int.h"
#include "vp890_testline_int.h"


#define VP_SHORT_SETTLE_MS 20         /* 20ms */
#define VP_ADC_SETTLE 80             /* 10ms */
#define VP_ADC_COLLECT 80            /* 10ms */
#define VP_ADC_UNBAL_COLLECT 160     /* 20ms */
typedef enum
{
    FLT_DSCRM_SETUP                 = VP890_TESTLINE_GLB_STRT_STATE,

    FLT_HOOK_GNK_DET,
    FLT_PREPARE_FOR_ADC_DATA,

    FLT_TIP_GND_FEED_ADC,
    FLT_TIP_GND_FEED_COLLECT,
    FLT_TIP_GND_FEED_GET_DATA,

    FLT_RING_GND_FEED_ADC,
    FLT_RING_GND_FEED_COLLECT,
    FLT_RING_GND_FEED_GET_DATA,

    FLT_TIP_RING_FEED_ADC,
    FLT_TIP_RING_FEED_COLLECT,
    FLT_TIP_RING_FEED_GET_DATA,

    FLT_TIP_RING_1KHZ_ADC,
    FLT_TIP_RING_1KHZ_COLLECT,
    FLT_TIP_RING_1KHZ_GET_DATA,

    FLT_TIP_OPEN_SETUP,
    FLT_TIP_OPEN_1KHZ_ADC,
    FLT_TIP_OPEN_1KHZ_COLLECT,
    FLT_TIP_OPEN_1KHZ_GET_DATA,

    FLT_SETUP_UNBAL_RING_ON_RING,
    FLT_TIP_GND_UNBAL_ADC,
    FLT_TIP_GND_UNBAL_COLLECT,
    FLT_TIP_GND_UNBAL_GET_DATA,

    FLT_SETUP_UNBAL_RING_ON_TIP,
    FLT_RING_GND_UNBAL_ADC,
    FLT_RING_GND_UNBAL_COLLECT,
    FLT_RING_GND_UNBAL_GET_DATA,
    FLT_STOP_UNBAL_RING_ON_TIP,

    FLT_RING_OPEN_SETUP,
    FLT_RING_OPEN_1KHZ_ADC,
    FLT_RING_OPEN_1KHZ_COLLECT,
    FLT_RING_OPEN_1KHZ_GET_DATA,

    FLT_DSCRM_END                   = VP890_TESTLINE_GLB_END_STATE,
    FLT_DSCRM_QUIT                  = VP890_TESTLINE_GLB_QUIT_STATE,
    FLT_DSCRM_TEST_STATE_ENUM_SIZE  = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpFltDscrmTestStateTypes;


static VpStatusType
VpFltDscrmArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestFltDscrm(
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
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;

    int16 *pTestState = &pTestInfo->testState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    bool adcFlag = TRUE;
    uint8 chan = pLineObj->ecVal;

    if (FALSE == callback) {
        return VpFltDscrmArgCheck(pLineCtx, pArgsUntyped, handle, testId);
    }
    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestFltDscrm(%i): pTestState", *pTestState));
    switch (*pTestState) {

        /***********************************************************************
         * Put the device into a feed state with the CODEC activated
         **********************************************************************/
        case FLT_DSCRM_SETUP: {
            uint8 slacState = (VP890_SS_ACTIVE | VP890_SS_ACTIVATE_MASK);
            uint8 sysCfg = 0x03;
            uint8 dcFeed[VP890_DC_FEED_LEN] = {0x12, 0x08};
            uint8 loopSup[VP890_LOOP_SUP_LEN] = {0x1B, 0x84, 0xB4, 0x1F};
            uint8 disn = 0x00;
            uint8 voicePathFGain = 0x00;

            uint8 mpiBuffer[6 + VP890_SS_CONFIG_LEN + VP890_DC_FEED_LEN + VP890_LOOP_SUP_LEN
                            + VP890_DISN_LEN + VP890_VP_GAIN_LEN + VP890_SLIC_STATE_WRT];
            uint8 mpiIndex = 0;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SS_CONFIG_WRT, VP890_SS_CONFIG_LEN, &sysCfg);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_DC_FEED_WRT, VP890_DC_FEED_LEN, dcFeed);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_LOOP_SUP_WRT, VP890_LOOP_SUP_LEN, loopSup);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_DISN_WRT, VP890_DISN_LEN, &disn);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_VP_GAIN_WRT, VP890_VP_GAIN_LEN, &voicePathFGain);

            /* go to active */
            pLineObj->slicValueCache = slacState;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SLIC_STATE_WRT, VP890_SLIC_STATE_LEN, &slacState);

            /* Index = buffer size (set to last filled value + 1). First byte
             * is the command, so MPI length after that = index - 1.
             * --------------------------------------------------*/
            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* allow time for feed to settle */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_HOOK_GNK_DET,
                MS_TO_TICKRATE(VP_SHORT_SETTLE_MS, tick));
            break;
        }

        /***********************************************************************
         * Get the Raw hook and ground key status
         **********************************************************************/
        case FLT_HOOK_GNK_DET:
            pDevObj->testResults.result.fltDscrm.hookDet =
                (pLineObj->lineState.condition & VP_CSLAC_HOOK);
            pDevObj->testResults.result.fltDscrm.gnkDet  =
                (pLineObj->lineState.condition & VP_CSLAC_GKEY);

            *pTestState = FLT_PREPARE_FOR_ADC_DATA;
            /* no break */

        /***********************************************************************
         * Prep the device to take linear measurements
         **********************************************************************/
        case FLT_PREPARE_FOR_ADC_DATA: {
            uint8 opCond = (VP890_TXPATH_EN | VP890_RXPATH_DIS | VP890_HIGH_PASS_DIS);
            uint8 opFunction = VP890_LINEAR_CODEC;
            if (pLineObj->opCond[0] != opCond) {
                VpMpiCmdWrapper(deviceId, chan, VP890_OP_COND_WRT, VP890_OP_COND_LEN, &opCond);
                pLineObj->opCond[0] = opCond;
            }
            VpMpiCmdWrapper(deviceId, chan, VP890_OP_FUNC_WRT, VP890_OP_FUNC_LEN, &opFunction);

            *pTestState = FLT_TIP_GND_FEED_ADC;
            /* no break */
        }

/*** Tip to Gnd ***/
        /***********************************************************************
         * Setup, collect and get Tip to Gnd Samples in feed state
         **********************************************************************/
        case FLT_TIP_GND_FEED_ADC:
            /* setup adc to measure the tip to ground feed voltage */
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_TIP_TO_GND_V, FALSE);
            if ( !adcFlag ) {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_DSCRM_QUIT, NEXT_TICK);
                break;
            }

            *pTestState = FLT_TIP_GND_FEED_COLLECT;
            /* no break */

        case FLT_TIP_GND_FEED_COLLECT:
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                FLT_TIP_GND_FEED_GET_DATA, VP_ADC_SETTLE, VP_ADC_COLLECT);
            break;

        case FLT_TIP_GND_FEED_GET_DATA:
            /* collect the tip to gnd measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_TIP_TO_GND_V, &pDevObj->testResults.result.fltDscrm.vtgFeed);

            *pTestState = FLT_RING_GND_FEED_ADC;
            /* no break */

/*** Ring to Gnd ***/
        /***********************************************************************
         * Setup, collect and get Ring to Gnd Samples in feed state
         **********************************************************************/
        case FLT_RING_GND_FEED_ADC:
            /* setup adc to measure the ring to ground feed voltage */
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_RING_TO_GND_V, FALSE);
            if ( !adcFlag ) {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_DSCRM_QUIT, NEXT_TICK);
                break;
            }

            *pTestState = FLT_RING_GND_FEED_COLLECT;
            /* no break */

        case FLT_RING_GND_FEED_COLLECT:
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                FLT_RING_GND_FEED_GET_DATA, VP_ADC_SETTLE, VP_ADC_COLLECT);
            break;

        case FLT_RING_GND_FEED_GET_DATA:
            /* collect the ring to gnd measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_RING_TO_GND_V, &pDevObj->testResults.result.fltDscrm.vrgFeed);

            *pTestState = FLT_TIP_RING_FEED_ADC;
            /* no break */

/*** Tip to Ring ***/
        /***********************************************************************
         * Setup, collect and get Tip to Ring Samples in feed state
         **********************************************************************/
        case FLT_TIP_RING_FEED_ADC:
            /* setup adc to measure the tip to ring feed voltage */
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_V, FALSE);
            if ( !adcFlag ) {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_DSCRM_QUIT, NEXT_TICK);
                break;
            }
            *pTestState = FLT_TIP_RING_FEED_COLLECT;
            /* no break */

        case FLT_TIP_RING_FEED_COLLECT:
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE,
                 FLT_TIP_RING_FEED_GET_DATA, VP_ADC_SETTLE, VP_ADC_COLLECT);
            break;

        case FLT_TIP_RING_FEED_GET_DATA:
            /* collect the tip to ring measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_METALLIC_DC_V, &pDevObj->testResults.result.fltDscrm.vtrFeed);

            *pTestState = FLT_TIP_RING_1KHZ_ADC;
            /* no break */

/*** 1kHz Tip to Ring ***/
        /***********************************************************************
         * Setup, the device for 1kHz tone and the ADC for 8 kHz sampling of the
         * metallic AC Voltage
         **********************************************************************/

        case FLT_TIP_RING_1KHZ_ADC : {
            uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
                {0x00, 0x00, 0x00, 0x0E, 0x39, 0x5A, 0x82, 0x00, 0x00, 0x00, 0x00};  /* 1333.33 kHz */

            uint8 cvtrCfg = {VP890_CC_8KHZ_RATE | VP890_METALLIC_AC_V};
            uint8 opCond = (VP890_TXPATH_EN | VP890_RXPATH_DIS | VP890_HIGH_PASS_EN);

            uint8 mpiBuffer[4 + VP890_OP_COND_LEN + VP890_CONV_CFG_LEN +
                            VP890_SIGA_PARAMS_LEN + VP890_GEN_CTRL_LEN];
            uint8 mpiIndex = 0;

            if (pLineObj->opCond[0] != opCond) {
                pLineObj->opCond[0] = opCond;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_OP_COND_WRT, VP890_OP_COND_LEN, &opCond);
            }

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_CONV_CFG_WRT, VP890_CONV_CFG_LEN, &cvtrCfg);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SIGA_PARAMS_WRT, VP890_SIGA_PARAMS_LEN, sigGenA);

            if (pLineObj->sigGenCtrl[0] != VP890_GENA_EN) {
                pLineObj->sigGenCtrl[0] = VP890_GENA_EN;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }

            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* testing has shown that we must wait atleast 10ms after changing the adc */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_TIP_RING_1KHZ_COLLECT,
                MS_TO_TICKRATE(20, tick)  );

            break;
        }

        /***********************************************************************
         * Sample the PCM buffer
         **********************************************************************/
        case FLT_TIP_RING_1KHZ_COLLECT: {

            uint8 tmpTxBufRate = pDevObj->txBufferDataRate;
            uint8 txBuf[VP890_TEST_DATA_LEN];

            VpMpiCmdWrapper(deviceId, chan, VP890_TEST_DATA_RD,
                VP890_TEST_DATA_LEN, txBuf);

            /* change the ADC tx rate back to orginal setting */
            VpMpiCmdWrapper(deviceId, chan, VP890_CONV_CFG_WRT,
                VP890_CONV_CFG_LEN, &pDevObj->txBufferDataRate);

            /* fool PCM collect into thinking the rate is at 4 kHz */
            pDevObj->txBufferDataRate = VP890_CC_8KHZ_RATE;

            VpMemCpy(pDevObj->testDataBuffer, txBuf, VP890_TEST_DATA_LEN);
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_RMS,
                FLT_RING_OPEN_1KHZ_COLLECT, 0, 6);

            /* set the txBufferRate back */
            pDevObj->txBufferDataRate = tmpTxBufRate;

            *pTestState = FLT_TIP_RING_1KHZ_GET_DATA;
            /* NO BREAK */
        }

        case FLT_TIP_RING_1KHZ_GET_DATA: {
            /* collect the tip to ring measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, chanId,
                VP890_METALLIC_AC_V, &pDevObj->testResults.result.fltDscrm.vtrAC);

            *pTestState = FLT_TIP_OPEN_SETUP;
            /* no break */
        }

/*** Tip Open ***/
        /***********************************************************************
         * Setup, the device for Tip Open with the CODEC activated.
         **********************************************************************/
        case FLT_TIP_OPEN_SETUP: {
            uint8 slacState = (VP890_SS_TIP_OPEN | VP890_SS_ACTIVATE_MASK);

            VpMpiCmdWrapper(deviceId, chan, VP890_SLIC_STATE_WRT, VP890_SLIC_STATE_LEN, &slacState);
            pLineObj->slicValueCache = slacState;

            /* allow time for feed to settle */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_TIP_OPEN_1KHZ_ADC,
                MS_TO_TICKRATE(VP_SHORT_SETTLE_MS, tick));
            break;
        }

        /***********************************************************************
         * Setup, the ADC for 8 kHz sampling of the metallic AC Voltage
         **********************************************************************/
        case FLT_TIP_OPEN_1KHZ_ADC: {
            uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
                {0x00, 0x00, 0x00, 0x0E, 0x39, 0x5A, 0x82, 0x00, 0x00, 0x00, 0x00};  /* 1333.33 Hz */

            uint8 opCond = (VP890_TXPATH_EN | VP890_RXPATH_DIS | VP890_HIGH_PASS_EN);
            uint8 cvtrCfg = (VP890_CC_8KHZ_RATE | VP890_METALLIC_AC_V);
            uint8 mpiBuffer[7 + VP890_ICR1_LEN + VP890_ICR3_LEN + VP890_ICR4_LEN +
                            VP890_CONV_CFG_LEN + VP890_SIGA_PARAMS_LEN + VP890_GEN_CTRL_LEN +
                            VP890_OP_COND_LEN];
            uint8 mpiIndex = 0;

            pLineObj->icr1Values[0] = 0x00;   /* Increase ibr SLIC drive */
            pLineObj->icr1Values[1] = 0x08;
            pLineObj->icr1Values[2] = 0xCF;
            pLineObj->icr1Values[3] = 0x07;

            pLineObj->icr3Values[0] = 0x00;   /* disable se_ring bit to ground the opposite lead */
            pLineObj->icr3Values[1] = 0x00;
            pLineObj->icr3Values[2] = 0x00;
            pLineObj->icr3Values[3] = 0x00;

            pLineObj->icr4Values[0] = 0x03;   /* enable voice DAC override */
            pLineObj->icr4Values[1] = 0x03;
            pLineObj->icr4Values[2] = 0x00;
            pLineObj->icr4Values[3] = 0x00;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR1_WRT, VP890_ICR1_LEN, pLineObj->icr1Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR4_WRT, VP890_ICR4_LEN, pLineObj->icr4Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_CONV_CFG_WRT, VP890_CONV_CFG_LEN, &cvtrCfg);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SIGA_PARAMS_WRT, VP890_SIGA_PARAMS_LEN, sigGenA);

            if (pLineObj->sigGenCtrl[0] != VP890_GENA_EN) {
                pLineObj->sigGenCtrl[0] = VP890_GENA_EN;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }

            if (pLineObj->opCond[0] != opCond) {
                pLineObj->opCond[0] = opCond;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_OP_COND_WRT, VP890_OP_COND_LEN, pLineObj->opCond);
            }

            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* testing has shown that we must wait atleast 20ms after changing the adc */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_TIP_OPEN_1KHZ_COLLECT,
                MS_TO_TICKRATE(20, tick)  );
            break;
        }

        /***********************************************************************
         * Sample the PCM buffer
         **********************************************************************/
        case FLT_TIP_OPEN_1KHZ_COLLECT: {

            uint8 tmpTxBufRate = pDevObj->txBufferDataRate;
            uint8 txBuf[VP890_TEST_DATA_LEN];

            /* read the current tx buffer */
            VpMpiCmdWrapper(deviceId, chan, VP890_TEST_DATA_RD, VP890_TEST_DATA_LEN, txBuf);

            /* change the ADC tx rate back to orginal setting */
            VpMpiCmdWrapper(deviceId, chan, VP890_CONV_CFG_WRT,  VP890_CONV_CFG_LEN, &pDevObj->txBufferDataRate);

            /* fool PCM collect into thinking the rate is at 4 kHz */
            pDevObj->txBufferDataRate = VP890_CC_8KHZ_RATE;

            VpMemCpy(pDevObj->testDataBuffer, txBuf, VP890_TEST_DATA_LEN);
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_RMS, FLT_TIP_OPEN_1KHZ_COLLECT, 0, 6);

            /* set the txBufferRate back */
            pDevObj->txBufferDataRate = tmpTxBufRate;

            *pTestState = FLT_TIP_OPEN_1KHZ_GET_DATA;
            /* NO BREAK */
        }

        case FLT_TIP_OPEN_1KHZ_GET_DATA:
            /* collect the tip to ring measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, chanId,
                VP890_METALLIC_AC_V, &pDevObj->testResults.result.fltDscrm.vtoAC);

            *pTestState = FLT_SETUP_UNBAL_RING_ON_RING;
            /* no break */

/*** Unbalanced Ringing Normal Polarity (Tip is Fixed)***/
        /***********************************************************************
         * Setup an unbalanced ringing signal on the ring lead
         **********************************************************************/
        case FLT_SETUP_UNBAL_RING_ON_RING: {
            /* Setup SigGen A for 1.5Vpk 1333.33 Hz AC on a -12V DC offset */
            uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
                {0x00, 0xF6, 0x0D, 0x0E, 0x39, 0x01, 0x3D, 0x00, 0x00, 0x00, 0x00};
            uint8 sigGenCtrl[] = {VP890_GEN_CTRL_EN_BIAS | VP890_GENA_EN};

            uint8 opCond = (VP890_TXPATH_EN | VP890_RXPATH_DIS | VP890_HIGH_PASS_EN);
            uint8 slacState = (VP890_SS_ACTIVE | VP890_SS_ACTIVATE_MASK);
            uint8 voicePathFGain = VP890_AR_6DB_GAIN;

            uint8 mpiBuffer[9 + VP890_SIGA_PARAMS_LEN + VP890_SYS_STATE_LEN +
                                VP890_GEN_CTRL_LEN + VP890_OP_COND_LEN +
                                VP890_ICR1_LEN + VP890_ICR2_LEN + VP890_ICR3_LEN +
                                VP890_ICR4_LEN + VP890_VP_GAIN_LEN];
            uint8 mpiIndex = 0;

             /* back door ringing */
            pLineObj->icr1Values[0] = 0xFF;
            pLineObj->icr1Values[1] = 0xC8;
            pLineObj->icr1Values[2] = 0xCF;
            pLineObj->icr1Values[3] = 0x0C;

            pLineObj->icr2Values[0] = 0xFF;
            pLineObj->icr2Values[1] = 0xDC;
            pLineObj->icr2Values[2] &= 0x0C;
            pLineObj->icr2Values[2] |= 0xF3;
            pLineObj->icr2Values[3] &= 0x0C;
            pLineObj->icr2Values[3] |= 0x61;

            pLineObj->icr3Values[0] = 0xFF;
            pLineObj->icr3Values[1] = 0x2D;
            pLineObj->icr3Values[2] = 0xFF;
            pLineObj->icr3Values[3] = 0x86;

            pLineObj->icr4Values[0] = 0xFF;
            pLineObj->icr4Values[1] = 0x03;
            pLineObj->icr4Values[2] = 0xFF;
            pLineObj->icr4Values[3] = 0x28;

            /* force the device into ringing throught the back door so that the high pass filter stays enabled */
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SIGA_PARAMS_WRT, VP890_SIGA_PARAMS_LEN, sigGenA);

            pLineObj->slicValueCache = slacState;
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN, &slacState);

            if (pLineObj->sigGenCtrl[0] != sigGenCtrl[0]) {
                pLineObj->sigGenCtrl[0] = sigGenCtrl[0];
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }

            if (pLineObj->opCond[0] != opCond) {
                pLineObj->opCond[0] = opCond;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_OP_COND_WRT, VP890_OP_COND_LEN, &opCond);
            }

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR1_WRT, VP890_ICR1_LEN, pLineObj->icr1Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR2_WRT, VP890_ICR2_LEN, pLineObj->icr2Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR4_WRT, VP890_ICR4_LEN, pLineObj->icr4Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_VP_GAIN_WRT, VP890_VP_GAIN_LEN, &voicePathFGain);

            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /*
             * allow the feed to settle since these steps are looking
             * for a REN load we need to give the feed as much time as possible
             * to settle out before trying to take a measurement so I have
             * doubled the normal 100ms of state change settling.
            */

            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_TIP_GND_UNBAL_ADC,
                MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME, tick));

            break;
        }

        /***********************************************************************
         * Prepare the ADC to take a differential RMS measurement.
         **********************************************************************/
        case FLT_TIP_GND_UNBAL_ADC: {

            uint8 cvtrCfg = (VP890_CC_8KHZ_RATE | VP890_METALLIC_AC_V);
            VpMpiCmdWrapper(deviceId, chan, VP890_CONV_CFG_WRT,  VP890_CONV_CFG_LEN, &cvtrCfg);

              /* testing has shown that we must wait atleast 20ms after changing the adc */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_TIP_GND_UNBAL_COLLECT,
                MS_TO_TICKRATE(20, tick)  );

            break;
        }


        /***********************************************************************
         * Collect PCM samples
         **********************************************************************/
        case FLT_TIP_GND_UNBAL_COLLECT: {
            uint8 tmpTxBufRate = pDevObj->txBufferDataRate;
            uint8 txBuf[VP890_TEST_DATA_LEN];

            /* read the current tx buffer */
            VpMpiCmdWrapper(deviceId, chan, VP890_TEST_DATA_RD, VP890_TEST_DATA_LEN, txBuf);

            /* change the ADC tx rate back to orginal setting */
            VpMpiCmdWrapper(deviceId, chan, VP890_CONV_CFG_WRT,  VP890_CONV_CFG_LEN, &pDevObj->txBufferDataRate);

            /* fool PCM collect into thinking the rate is at 4 kHz */
            pDevObj->txBufferDataRate = VP890_CC_8KHZ_RATE;

            VpMemCpy(pDevObj->testDataBuffer, txBuf, VP890_TEST_DATA_LEN);
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_RMS, FLT_TIP_GND_UNBAL_COLLECT, 0, 6);

            /* set the txBufferRate back */
            pDevObj->txBufferDataRate = tmpTxBufRate;

            *pTestState = FLT_TIP_GND_UNBAL_GET_DATA;
            /* NO BREAK */
        }

        /***********************************************************************
         * Store the data
         **********************************************************************/

        case FLT_TIP_GND_UNBAL_GET_DATA:
            /* collect the tip to ring measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, chanId,
                VP890_METALLIC_AC_V, &pDevObj->testResults.result.fltDscrm.vruBalAC);

            *pTestState = FLT_SETUP_UNBAL_RING_ON_TIP;
            /* no break */


/*** Unbalanced Ring Reverse Polarity (Ring is Fixed) ***/
        /***********************************************************************
         * Setup an unbalanced ringing signal on the tip lead
         **********************************************************************/
        case FLT_SETUP_UNBAL_RING_ON_TIP: {

            /* Setup SigGen A for 1.5Vpk 100Hz AC on a 12V DC offset */
            uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
                {0x00, 0x09, 0xF3, 0x0E, 0x39, 0x01, 0x3D, 0x00, 0x00, 0x00, 0x00};
            uint8 sigGenCtrl[] = {VP890_GEN_CTRL_EN_BIAS | VP890_GENA_EN};

            uint8 opCond = (VP890_TXPATH_EN | VP890_RXPATH_DIS | VP890_HIGH_PASS_EN);
            uint8 slacState = (VP890_SS_ACTIVE | VP890_SS_ACTIVATE_MASK);
            uint8 voicePathFGain = VP890_AR_6DB_GAIN;

            uint8 mpiBuffer[9 + VP890_SIGA_PARAMS_LEN + VP890_SYS_STATE_LEN +
                                VP890_GEN_CTRL_LEN + VP890_OP_COND_LEN +
                                VP890_ICR1_LEN + VP890_ICR2_LEN + VP890_ICR3_LEN +
                                VP890_ICR4_LEN + VP890_VP_GAIN_LEN];
            uint8 mpiIndex = 0;

             /* back door ringing */
            pLineObj->icr1Values[0] = 0xFF;
            pLineObj->icr1Values[1] = 0xC8;
            pLineObj->icr1Values[2] = 0xCF;
            pLineObj->icr1Values[3] = 0x0C;

            pLineObj->icr2Values[0] = 0xFF;
            pLineObj->icr2Values[1] = 0xDC;
            pLineObj->icr2Values[2] &= 0x0C;
            pLineObj->icr2Values[2] |= 0xF3;
            pLineObj->icr2Values[3] &= 0x0C;
            pLineObj->icr2Values[3] |= 0x61;

            pLineObj->icr3Values[0] = 0xFF;
            pLineObj->icr3Values[1] = 0x2D;
            pLineObj->icr3Values[2] = 0xFF;
            pLineObj->icr3Values[3] = 0x86;

            pLineObj->icr4Values[0] = 0xFF;
            pLineObj->icr4Values[1] = 0x03;
            pLineObj->icr4Values[2] = 0xFF;
            pLineObj->icr4Values[3] = 0x28;

            /* force the device into ringing throught the back door so that the high pass filter stays enabled */
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SIGA_PARAMS_WRT, VP890_SIGA_PARAMS_LEN, sigGenA);

                pLineObj->slicValueCache = slacState;
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SYS_STATE_WRT, VP890_SYS_STATE_LEN, &slacState);

            if (pLineObj->sigGenCtrl[0] != sigGenCtrl[0]) {
                pLineObj->sigGenCtrl[0] = sigGenCtrl[0];
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }

            if (pLineObj->opCond[0] != opCond) {
                pLineObj->opCond[0] = opCond;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_OP_COND_WRT, VP890_OP_COND_LEN, &opCond);
            }

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR1_WRT, VP890_ICR1_LEN, pLineObj->icr1Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR2_WRT, VP890_ICR2_LEN, pLineObj->icr2Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR4_WRT, VP890_ICR4_LEN, pLineObj->icr4Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_VP_GAIN_WRT, VP890_VP_GAIN_LEN, &voicePathFGain);

            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /*
             * allow the feed to settle since these steps are looking
             * for a REN load we need to give the feed as much time as possible
             * to settle out before trying to take a measurement so I have
             * doubled the normal 100ms of state change settling.
            */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_RING_GND_UNBAL_ADC,
                MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME, tick));

            break;
        }

        /***********************************************************************
         * Prepare the ADC to take a differential RMS measurement.
         **********************************************************************/
        case FLT_RING_GND_UNBAL_ADC: {
            uint8 cvtrCfg = (VP890_CC_8KHZ_RATE | VP890_METALLIC_AC_V);

            VpMpiCmdWrapper(deviceId, chan, VP890_CONV_CFG_WRT,  VP890_CONV_CFG_LEN, &cvtrCfg);

            /* testing has shown that we must wait atleast 20ms after changing the adc */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_RING_GND_UNBAL_COLLECT,
                MS_TO_TICKRATE(20, tick) );

            break;
        }

        /***********************************************************************
         * Collect PCM samples
         **********************************************************************/
        case FLT_RING_GND_UNBAL_COLLECT: {
            uint8 tmpTxBufRate = pDevObj->txBufferDataRate;
            uint8 txBuf[VP890_TEST_DATA_LEN];

            /* read the current tx buffer */
            VpMpiCmdWrapper(deviceId, chan, VP890_TEST_DATA_RD, VP890_TEST_DATA_LEN, txBuf);

            /* change the ADC tx rate back to orginal setting */
            VpMpiCmdWrapper(deviceId, chan, VP890_CONV_CFG_WRT,  VP890_CONV_CFG_LEN, &pDevObj->txBufferDataRate);

            /* fool PCM collect into thinking the rate is at 4 kHz */
            pDevObj->txBufferDataRate = VP890_CC_8KHZ_RATE;

            VpMemCpy(pDevObj->testDataBuffer, txBuf, VP890_TEST_DATA_LEN);
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_RMS, FLT_RING_GND_UNBAL_COLLECT, 0, 6);

            /* set the txBufferRate back */
            pDevObj->txBufferDataRate = tmpTxBufRate;

            *pTestState = FLT_RING_GND_UNBAL_GET_DATA;
            /* NO BREAK */
         }
        /***********************************************************************
         * Store the data
         **********************************************************************/

        case FLT_RING_GND_UNBAL_GET_DATA:
            /* collect the tip to ring measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, chanId,
                VP890_METALLIC_AC_V, &pDevObj->testResults.result.fltDscrm.vtuBalAC);

            *pTestState = FLT_STOP_UNBAL_RING_ON_TIP;
            /* no break */

        /***********************************************************************
         * Disable Ringing
         **********************************************************************/
        case FLT_STOP_UNBAL_RING_ON_TIP: {
            uint8 slacState = (VP890_SS_ACTIVE_POLREV);
            uint8 mpiBuffer[5 + VP890_ICR1_LEN + VP890_ICR2_LEN + VP890_ICR3_LEN +
                                VP890_ICR4_LEN + VP890_SLIC_STATE_LEN];
            uint8 mpiIndex = 0;

            pLineObj->icr1Values[0] = 0x00;
            pLineObj->icr1Values[1] = 0x00;
            pLineObj->icr1Values[2] = 0x00;
            pLineObj->icr1Values[3] = 0x00;

            pLineObj->icr2Values[0] = 0x00;
            pLineObj->icr2Values[1] = 0x00;
            pLineObj->icr2Values[2] &= 0x0C;
            pLineObj->icr2Values[3] &= 0x0C;

            pLineObj->icr3Values[0] = 0x00;
            pLineObj->icr3Values[1] = 0x00;
            pLineObj->icr3Values[2] = 0x00;
            pLineObj->icr3Values[3] = 0x00;

            pLineObj->icr4Values[0] = 0x00;
            pLineObj->icr4Values[1] = 0x00;
            pLineObj->icr4Values[2] = 0x00;
            pLineObj->icr4Values[3] = 0x00;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR1_WRT, VP890_ICR1_LEN, pLineObj->icr1Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR2_WRT, VP890_ICR2_LEN, pLineObj->icr2Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR4_WRT, VP890_ICR4_LEN, pLineObj->icr4Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SLIC_STATE_WRT, VP890_SLIC_STATE_LEN, &slacState);
            pLineObj->slicValueCache = slacState;

            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* allow time for feed to settle */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_RING_OPEN_SETUP,
                MS_TO_TICKRATE(VP_SHORT_SETTLE_MS, tick));

            break;
        }

/*** Ring Open ***/
        /***********************************************************************
         * Setup, the device for Ring Open with the CODEC activated.
         * unfortunatly going from ringing to tip open activate is not legal
         * so we had to do the previous step first
         **********************************************************************/
        case FLT_RING_OPEN_SETUP: {
            uint8 slacState = (VP890_SS_RING_OPEN | VP890_SS_ACTIVATE_MASK);
            VpMpiCmdWrapper(deviceId, chan, VP890_SLIC_STATE_WRT, VP890_SLIC_STATE_LEN, &slacState);
            pLineObj->slicValueCache = slacState;

            /* allow time for feed to settle */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_RING_OPEN_1KHZ_ADC,
                MS_TO_TICKRATE(VP_SHORT_SETTLE_MS, tick)  );
            break;
        }

        /***********************************************************************
         * Setup, the ADC for 8 kHz sampling of the metallic AC Voltage
         **********************************************************************/
        case FLT_RING_OPEN_1KHZ_ADC: {
            uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
                {0x00, 0x00, 0x00, 0x0E, 0x39, 0x5A, 0x82, 0x00, 0x00, 0x00, 0x00};  /* 1333.33 Hz */
            uint8 sigGenCtrl[] = {VP890_GENA_EN};

            uint8 opCond = (VP890_TXPATH_EN | VP890_RXPATH_DIS | VP890_HIGH_PASS_EN);
            uint8 cvtrCfg = {VP890_CC_8KHZ_RATE | VP890_METALLIC_AC_V};
            uint8 voicePathFGain = VP890_DEFAULT_VP_GAIN;

            uint8 mpiBuffer[8 + VP890_VP_GAIN_LEN + VP890_OP_COND_LEN + VP890_ICR1_LEN +
                                VP890_ICR3_LEN + VP890_ICR4_LEN + VP890_CONV_CFG_LEN +
                                VP890_SIGA_PARAMS_LEN + VP890_GEN_CTRL_LEN];

            uint8 mpiIndex = 0;

            pLineObj->icr1Values[0] = 0xF0;   /* Increase ibt SLIC drive */
            pLineObj->icr1Values[1] = 0x78;
            pLineObj->icr1Values[2] = 0xC0;
            pLineObj->icr1Values[3] = 0x00;

            pLineObj->icr3Values[0] = 0x00;   /* disable se_ring bit to ground the opposite lead */
            pLineObj->icr3Values[1] = 0x00;
            pLineObj->icr3Values[2] = 0x00;
            pLineObj->icr3Values[3] = 0x00;

            pLineObj->icr4Values[0] = 0x03;   /* enable voice DAC override */
            pLineObj->icr4Values[1] = 0x03;
            pLineObj->icr4Values[2] = 0x00;
            pLineObj->icr4Values[3] = 0x00;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_VP_GAIN_WRT, VP890_VP_GAIN_LEN, &voicePathFGain);

            if (pLineObj->opCond[0] != opCond) {
                pLineObj->opCond[0] = opCond;
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_OP_COND_WRT, VP890_OP_COND_LEN, &opCond);
            }

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR1_WRT, VP890_ICR1_LEN, pLineObj->icr1Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_ICR4_WRT, VP890_ICR4_LEN, pLineObj->icr4Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_CONV_CFG_WRT, VP890_CONV_CFG_LEN, &cvtrCfg);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP890_SIGA_PARAMS_WRT, VP890_SIGA_PARAMS_LEN, sigGenA);

            if (pLineObj->sigGenCtrl[0] != sigGenCtrl[0]) {
                pLineObj->sigGenCtrl[0] = sigGenCtrl[0];
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
            }

            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            /* testing has shown that we must wait atleast 20ms after changing the adc */
            Vp890SetTestStateAndTimer(pDevObj, pTestState, FLT_RING_OPEN_1KHZ_COLLECT,
                MS_TO_TICKRATE(20, tick)  );
            break;
        }

        /***********************************************************************
         * Sample the PCM buffer
         **********************************************************************/
        case FLT_RING_OPEN_1KHZ_COLLECT: {
            uint8 tmpTxBufRate = pDevObj->txBufferDataRate;
            uint8 txBuf[VP890_TEST_DATA_LEN];

            /* read the current tx buffer */
            VpMpiCmdWrapper(deviceId, chan, VP890_TEST_DATA_RD, VP890_TEST_DATA_LEN, txBuf);

            /* change the ADC tx rate back to orginal setting */
            VpMpiCmdWrapper(deviceId, chan, VP890_CONV_CFG_WRT,  VP890_CONV_CFG_LEN, &pDevObj->txBufferDataRate);

            /* fool PCM collect into thinking the rate is at 4 kHz */
            pDevObj->txBufferDataRate = VP890_CC_8KHZ_RATE;

            VpMemCpy(pDevObj->testDataBuffer, txBuf, VP890_TEST_DATA_LEN);
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_RMS, FLT_RING_OPEN_1KHZ_COLLECT, 0, 6);

            /* set the txBufferRate back */
            pDevObj->txBufferDataRate = tmpTxBufRate;


            *pTestState = FLT_RING_OPEN_1KHZ_GET_DATA;
            /* NO BREAK */

        }

        case FLT_RING_OPEN_1KHZ_GET_DATA:
            /* collect the tip to ring measurement */
            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, chanId,
                VP890_METALLIC_AC_V, &pDevObj->testResults.result.fltDscrm.vroAC);

            *pTestState = FLT_DSCRM_END;
            /* no break */


        case FLT_DSCRM_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestFltDscrm(): complete"));
            break;

        case FLT_DSCRM_QUIT:
            /* The test has eneded early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestFltDscrm(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestFltDscrm(): invalid teststate"));
            break;
    }

    return VP_STATUS_SUCCESS;
}

static VpStatusType
VpFltDscrmArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    VpTestIdType testId)
{

    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);

    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpFltDscrmArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only once per test.
     */
    pDevObj->testResults.result.fltDscrm.vtgFeed        = 0;
    pDevObj->testResults.result.fltDscrm.vrgFeed        = 0;
    pDevObj->testResults.result.fltDscrm.vtrFeed        = 0;

    pDevObj->testResults.result.fltDscrm.vtrAC          = 0;
    pDevObj->testResults.result.fltDscrm.vtoAC          = 0;
    pDevObj->testResults.result.fltDscrm.vroAC          = 0;

    pDevObj->testResults.result.fltDscrm.vtuBalAC       = 0;
    pDevObj->testResults.result.fltDscrm.vruBalAC       = 0;

    pDevObj->testResults.result.fltDscrm.hookDet        = FALSE;
    pDevObj->testResults.result.fltDscrm.gnkDet         = FALSE;
    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* TODO: check input arguments before assigining them to test struct!!! */

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;

}

#endif /* (VP890_INCLUDE_LINE_TEST_PACKAGE == VP890_LINE_TEST_PROFESSIONAL) */
#endif /* VP890_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_890_SERIES */

