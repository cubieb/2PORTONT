/** file vp_3ele_res.c
 *
 *  This file contains the 3 ele res primitive algorithm
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#include "vp_api_cfg.h"

#if defined(VP_CC_880_SERIES) && defined(VP880_INCLUDE_TESTLINE_CODE)
#if (VP880_INCLUDE_LINE_TEST_PACKAGE == VP880_LINE_TEST_PROFESSIONAL)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_api_testline_int.h"

#define SETTLING_ERROR_THRESHOLD        2       /* +/- 2 adu err*/
#define MAX_ITERATION_ADC               10      /* Max number ADC failures */
#define MAX_ITERATION_SET_18MS          20      /* Max number of measurement for settling */
#define MAX_ITERATION_SET_100MS         10      /* Max number of measurement for settling */
#define IMT_RMS_THRESHOLD               400     /* IMT threshold to switch from 18ms intg to 100ms*/

typedef enum
{
    COMMON_TEST_SETUP           = VP880_TESTLINE_GLB_STRT_STATE,
    ELE3_RES_HG_SETUP           = 3,
    ELE3_RES_HG_CHANGE_FEED     = 5,
    ELE3_RES_HG_AD_SETUP        = 10,
    ELE3_RES_HG_COLLECT_DATA    = 15,
    ELE3_RES_HG_GET_RESULT      = 20,
    ELE3_RES_HG_RESTORE         = 25,
    ELE3_RES_HG_END             = VP880_TESTLINE_GLB_END_STATE,
    ELE3_RES_HG_QUIT            = VP880_TESTLINE_GLB_QUIT_STATE,
    ELE3_RES_HG_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} vp3EleResHGTestStateTypes;

typedef enum
{
    ELE3_RES_HG_IMT_ZERO_FEED,
    ELE3_RES_HG_ILG_ZERO_FEED,
    ELE3_RES_HG_VAH_ZERO_FEED,
    ELE3_RES_HG_VBH_ZERO_FEED,
    ELE3_RES_HG_IMT_REVERSE_FEED,
    ELE3_RES_HG_ILG_REVERSE_FEED,
    ELE3_RES_HG_VAH_REVERSE_FEED,
    ELE3_RES_HG_VBH_REVERSE_FEED,
    ELE3_RES_HG_IMT_NORMAL_FEED,
    ELE3_RES_HG_ILG_NORMAL_FEED,
    ELE3_RES_HG_VAH_NORMAL_FEED,
    ELE3_RES_HG_VBH_NORMAL_FEED,
    ELE3_RES_HG_MAX_ADC,
    ELE3_RES_HG_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vp3EleResAdcOrderTypes;


static VpStatusType
Vp3EleResHGArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
Vp3EleResHGSetup(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static void
Vp3EleResHGChangeFeed(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    uint8 adcState,
    VpDeviceIdType deviceId,
    uint16 *pTime);

static void
Vp3EleResHGAdcSetup(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static void
Vp3EleResHGGetResult(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    int16 *pTestState,
    uint8 *pAdcState,
    uint8 chanId);

static bool
Vp3EleResHGStoreResult(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState);

EXTERN VpStatusType
Vp880Test3EleResHG(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    int16 *pPreviousMeas = &pTestHeap->vabComputed;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880Test3EleResHG(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return Vp3EleResHGArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    switch (*pTestState) {
        case COMMON_TEST_SETUP:
            Vp880CommonTestSetup(pLineCtx, deviceId);
            *pPreviousMeas = 0;
            /* no break */

        case ELE3_RES_HG_SETUP:
            /* setup the device for the test */
            Vp3EleResHGSetup(pDevObj, pLineObj, pTestHeap, deviceId);
            /* no break */

        case ELE3_RES_HG_CHANGE_FEED: {
            uint16 time;

            /* change the current feed condtion if need be */
            Vp3EleResHGChangeFeed(pLineCtx, pDevObj, *pAdcState, deviceId, &time);
            if (time > 0) {
                VpSetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_HG_AD_SETUP,
                    MS_TO_TICKRATE(time, tick));
                break;
            }
        }

        case ELE3_RES_HG_AD_SETUP:
            /* change the adc setting */
            Vp3EleResHGAdcSetup(pLineCtx, *pAdcState);
            VpSetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_HG_COLLECT_DATA, NEXT_TICK);
            break;

        case ELE3_RES_HG_COLLECT_DATA:
            /* Start the PCM collect process */
            VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE | VP_PCM_OPERATION_RMS,
                ELE3_RES_HG_GET_RESULT, pTestHeap->testArgs.resFltAlt.settlingTime,
                pTestHeap->testArgs.resFltAlt.integrateTime);
            break;

        case ELE3_RES_HG_GET_RESULT:
            /* Store the PCM data in the results structure */
            Vp3EleResHGGetResult(pLineCtx, pDevObj, pTestState, pAdcState, chanId);
            VpSetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;

        case ELE3_RES_HG_RESTORE: {
                /* Restore registers */
                uint8 mpiBuffer[7 + VP880_LOOP_SUP_LEN + VP880_DC_FEED_LEN +
                                VP880_REGULATOR_PARAM_LEN + VP880_ICR1_LEN +
                                VP880_ICR2_LEN + VP880_ICR3_LEN +
                                VP880_ICR4_LEN];
                uint8 mpiIndex = 0;

                /* Restore registers */
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP880_LOOP_SUP_WRT, VP880_LOOP_SUP_LEN, pTestHeap->lpSuper);

                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP880_DC_FEED_WRT, VP880_DC_FEED_LEN, pTestHeap->dcFeed);

                VpMemCpy(pDevObj->swParamsCache, pTestHeap->switchReg,
                    VP880_REGULATOR_PARAM_LEN);
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP880_REGULATOR_PARAM_WRT, VP880_REGULATOR_PARAM_LEN, pTestHeap->switchReg);

                VpMemCpy(pLineObj->icr1Values, pTestHeap->icr1, VP880_ICR1_LEN);
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP880_ICR1_WRT, VP880_ICR1_LEN, pTestHeap->icr1);

                VpMemCpy(pLineObj->icr2Values, pTestHeap->icr2, VP880_ICR2_LEN);
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP880_ICR2_WRT, VP880_ICR2_LEN, pTestHeap->icr2);

                VpMemCpy(pLineObj->icr3Values, pTestHeap->icr3, VP880_ICR3_LEN);
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP880_ICR3_WRT, VP880_ICR3_LEN, pTestHeap->icr3);

                VpMemCpy(pLineObj->icr4Values, pTestHeap->icr4, VP880_ICR4_LEN);
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                    VP880_ICR4_WRT, VP880_ICR4_LEN, pTestHeap->icr4);


                /* send down the mpi commands */
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                    mpiIndex-1, &mpiBuffer[1]);

                /* check for special error code */
                if (*pErrorCode == VP_TEST_STATUS_NO_CONVERGENCE) {
                    VpSetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_HG_QUIT, NEXT_TICK);
                } else {
                    VpSetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_HG_END, NEXT_TICK);
                }
            }
            break;

        case ELE3_RES_HG_END:
            /* The test has completed */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880Test3EleResHG(): complete"));
            break;

        case ELE3_RES_HG_QUIT:
            /* The test has eneded early due to the current errorCode */
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880Test3EleResHG(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880Test3EleResHG(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp3EleResHGArgCheck()
 * This functions is called by the Vp880Test3EleResHG() function if
 * Vp880Test3EleResHG() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the AC Rloop primitive are legal. The function also sets up
 * the AC Rloop state machine for the first state and initializes the results
 * from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  handle          - unique test handle
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *  testId          - current Test Id
 *
 * Returns:
 * VpStatusType
 *
 * Result:
 *  This function initializes all relevent result values to 0. Stores the
 *  TestId and handle into the device Object as well as modifes the current
 *  ADC state to the first measurement.
 *
 *----------------------------------------------------------------------------*/
static VpStatusType
Vp3EleResHGArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTest3EleResAltResType *p3EleResHGData =
        &(pTestInfo->pTestHeap->testArgs).resFltAlt;
    const VpTest3EleResAltResType *pTestInput = pArgsUntyped;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResHGArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */
    *pAdcState = ELE3_RES_HG_IMT_ZERO_FEED;
    pDevObj->testResults.result.resAlt.highGain = TRUE;
    pDevObj->testResults.result.resAlt.vanf = 0;
    pDevObj->testResults.result.resAlt.vbnf = 0;
    pDevObj->testResults.result.resAlt.imnf = 0;
    pDevObj->testResults.result.resAlt.ilnf = 0;
    pDevObj->testResults.result.resAlt.varf = 0;
    pDevObj->testResults.result.resAlt.vbrf = 0;
    pDevObj->testResults.result.resAlt.imrf = 0;
    pDevObj->testResults.result.resAlt.ilrf = 0;
    pDevObj->testResults.result.resAlt.vazf = 0;
    pDevObj->testResults.result.resAlt.vbzf = 0;
    pDevObj->testResults.result.resAlt.imzf = 0;
    pDevObj->testResults.result.resAlt.ilzf = 0;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* This test cannot run if the isolate relay is open*/
    if (VP_RELAY_RESET == pLineObj->relayState) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResHGArgCheck(): bailed openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    p3EleResHGData->integrateTime = pTestInput->integrateTime * 8;
    p3EleResHGData->settlingTime = pTestInput->settlingTime * 8;

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    VpSetTestStateAndTimer(pDevObj, pTestState, VP880_TESTLINE_GLB_STRT_STATE,
        NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp3EleResSetup()
 * This function is called by the Vp880Test3EleRes() state machine during
 * the ELE3_RES_SETUP state.
 *
 * This function is used to configure the line for the dc rloop test.
 *
 * Parameters:
 *  pLineObj        - pointer to the line object.
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *
 * Returns:
 * --
 * Result:
 *  Disable AISN
 *----------------------------------------------------------------------------*/
static void
Vp3EleResHGSetup(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    uint8 dcfeed[VP880_DC_FEED_LEN] = {0x42, 0x08};
    uint8 switchRegParam[VP880_REGULATOR_PARAM_LEN] = {0x0A, 0x04, 0x04}; /* battery 25V fixed */
    uint8 lpSuper[VP880_LOOP_SUP_LEN];
    uint8 mpiBuffer[VP880_LOOP_SUP_LEN+1 +
                    VP880_REGULATOR_PARAM_LEN+1 +
                    VP880_GEN_CTRL_LEN+1 +
                    VP880_DC_FEED_LEN+1 +
                    VP880_ICR1_LEN+1];
    uint8 mpiIndex = 0;

    switchRegParam[0] = (pTestHeap->switchReg[0] & 0xF5) | switchRegParam[0];
    switchRegParam[1] = (pTestHeap->switchReg[1] & 0xE0) | switchRegParam[1];
    switchRegParam[2] = (pTestHeap->switchReg[2] & 0xE0) | switchRegParam[2];

    pLineObj->icr1Values[0] = 0xFF;
    pLineObj->icr1Values[1] = 0xA8;
    pLineObj->icr1Values[2] = 0xCF;
    pLineObj->icr1Values[3] = 0x0A;
    

    /* Masking the hook detection by maxing out TSH */
    lpSuper[0] = pTestHeap->lpSuper[0] | 0x07;
    lpSuper[1] = 0xFF;
    lpSuper[2] = pTestHeap->lpSuper[2];
    lpSuper[3] = pTestHeap->lpSuper[3];

    /* Limit the floor voltage */
    if (!(VP880_IS_ABS & pDevObj->stateInt)) {
        /* Tracker */
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_REGULATOR_PARAM_WRT,
            VP880_REGULATOR_PARAM_LEN, switchRegParam);
        VpMemCpy(pDevObj->swParamsCache, switchRegParam, VP880_REGULATOR_PARAM_LEN);
    }

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_LOOP_SUP_WRT,
        VP880_LOOP_SUP_LEN, lpSuper);

    /* Turn on DC bias and Sig Gen A */
    pLineObj->sigGenCtrl[0] = VP880_GEN_CTRL_EN_BIAS;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_GEN_CTRL_WRT,
        VP880_GEN_CTRL_LEN, pLineObj->sigGenCtrl);

    /* LI=0, min VOC; max curr lim */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DC_FEED_WRT,
        VP880_DC_FEED_LEN, dcfeed);

    /* VOC on, A/B sns on, incr ADC rg */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR1_WRT,
        VP880_ICR1_LEN, pLineObj->icr1Values);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * Vp3EleResHGChangeFeed()
 * This function is called by the Vp880Test3EleResHG() state machine during
 * the ELE3_RES_HG_ADC_SETUP state.
 *
 * This function is used to set up the device based on the current adc state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pDevObj         - pointer to the device object.
 *  adcState        - current value of the adc setup state.
 *  deviceId        - device ID
 *  ecVAl           - current enable channel value.
 *
 * Returns:
 *  --
 * Result:
 * If the incoming adc state is not a feed condition change then nothing
 * happens and the time argument is set to 0. Other wise the device is changed
 * according to the feed state and the time argument is set to state change
 * settling time.
 *----------------------------------------------------------------------------*/
static void
Vp3EleResHGChangeFeed(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    uint8 adcState,
    VpDeviceIdType deviceId,
    uint16 *pTime)
{
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    uint8 sigGenA[VP880_SIGA_PARAMS_LEN] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 icr2[VP880_ICR2_LEN] = {0x00, 0x00, 0x80, 0x00};     /* Enable speed-up capacitors */
    uint8 icr3[VP880_ICR3_LEN] = {0x00, 0x00, 0x00, 0x00};
    uint8 icr4[VP880_ICR4_LEN] = {0x00, 0x00, 0x00, 0x00};
    uint8 slacState = VP880_SS_ACTIVE;
    bool *firstSettling = &pTestHeap->compensate;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    uint16 *settlingCnt = &pTestHeap->speedupTime;

    /* Reset the settling counter */
    *settlingCnt = 0;

    if (adcState == ELE3_RES_HG_IMT_ZERO_FEED) {
        /* icr2[VP880_ICR2_LEN] = {0x20, 0x00, 0x80, 0x00}*/
        /* icr3[VP880_ICR3_LEN] = {0x08, 0x08, 0x58, 0x40} (abs)*/
        /* icr3[VP880_ICR3_LEN] = {0x08, 0x08, 0x40, 0x40} (tracker)*/
        icr2[0] = 0x20;

        icr3[0] = 0x08;
        icr3[1] = 0x08;
        if (VP880_IS_ABS & pDevObj->stateInt) {
            icr3[2] = 0x58;
        } else {
            icr3[2] = 0x40;
        }
        icr3[3] = 0x40;

        sigGenA[1] = 0x0C;

        icr4[0] = pLineObj->icr4Values[0];
        icr4[1] = pLineObj->icr4Values[1];
        icr4[2] = pLineObj->icr4Values[2] | 0x05;
        icr4[3] = pLineObj->icr4Values[3] | 0x05;
    } else {
        /* icr3[VP880_ICR3_LEN] = {0x08, 0x08, 0x18, 0x08} (abs)*/
        /* icr3[VP880_ICR3_LEN] = {0x00, 0x00, 0x00, 0x00} (tracker)*/

        if (VP880_IS_ABS & pDevObj->stateInt) {
            icr3[0] = 0x08;
            icr3[1] = 0x08;
            icr3[2] = 0x18;
            icr3[3] = 0x08;
        }

        icr4[0] = pLineObj->icr4Values[0];
        icr4[1] = pLineObj->icr4Values[1];
        icr4[2] = (pLineObj->icr4Values[2] & ~0x01);
        icr4[3] = (pLineObj->icr4Values[3] & ~0x01);
    }

    switch (adcState) {
        case ELE3_RES_HG_IMT_ZERO_FEED:
        case ELE3_RES_HG_IMT_NORMAL_FEED:
        case ELE3_RES_HG_IMT_REVERSE_FEED: {
            uint8 mpiBuffer[5 + VP880_SIGA_PARAMS_LEN + VP880_ICR2_LEN + VP880_ICR3_LEN +
                VP880_ICR4_LEN + VP880_SLIC_STATE_LEN];
            uint8 mpiIndex = 0;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP880_SIGA_PARAMS_WRT, VP880_SIGA_PARAMS_LEN, sigGenA);

            VpMemCpy(pLineObj->icr2Values, icr2, VP880_ICR2_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP880_ICR2_WRT, VP880_ICR2_LEN, icr2);

            VpMemCpy(pLineObj->icr3Values, icr3, VP880_ICR3_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP880_ICR3_WRT, VP880_ICR3_LEN, icr3);

            VpMemCpy(pLineObj->icr4Values, icr4, VP880_ICR4_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP880_ICR4_WRT, VP880_ICR4_LEN, icr4);

            if (ELE3_RES_HG_IMT_REVERSE_FEED == adcState) {
                slacState = VP880_SS_ACTIVE_POLREV;
            }
            pLineObj->slicValueCache = slacState;

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
                VP880_SLIC_STATE_WRT, VP880_SLIC_STATE_LEN, &pLineObj->slicValueCache);

            /* send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0],
                mpiIndex-1, &mpiBuffer[1]);

            *firstSettling = TRUE;
            *loopCnt = 0;
            *pTime = VP880_STATE_CHANGE_SETTLING_TIME_SHORT;
            break;
        }

        default:
            *pTime = 0; /* no reason to wait if not changing feed states */
            break;
    }
    return;
}

/*------------------------------------------------------------------------------
 * Vp3EleResHGAdcSetup()
 * This function is called by the Vp880Test3EleResHG() state machine during
 * the ELE3_RES_HG_ADC_SETUP state.
 *
 * This function is used to call the Vp880AdcSetup() depending on the
 * current  adc state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  adcState        - current value of the adc setup state.
 *
 * Returns:
 *  --
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static void
Vp3EleResHGAdcSetup(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    switch (adcState) {
        case ELE3_RES_HG_IMT_NORMAL_FEED:
        case ELE3_RES_HG_IMT_REVERSE_FEED:
        case ELE3_RES_HG_IMT_ZERO_FEED:
            Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_I, FALSE);
            break;
        case ELE3_RES_HG_ILG_NORMAL_FEED:
        case ELE3_RES_HG_ILG_REVERSE_FEED:
        case ELE3_RES_HG_ILG_ZERO_FEED:
            Vp880AdcSetup(pLineCtx, VP880_LONGITUDINAL_DC_I, FALSE);
            break;
        case ELE3_RES_HG_VAH_NORMAL_FEED:
        case ELE3_RES_HG_VAH_REVERSE_FEED:
        case ELE3_RES_HG_VAH_ZERO_FEED:
            Vp880AdcSetup(pLineCtx, VP880_TIP_TO_GND_V, FALSE);
            break;
        case ELE3_RES_HG_VBH_NORMAL_FEED:
        case ELE3_RES_HG_VBH_REVERSE_FEED:
        case ELE3_RES_HG_VBH_ZERO_FEED:
            Vp880AdcSetup(pLineCtx, VP880_RING_TO_GND_V, FALSE);
            break;
        default:
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResHGAdcSetup(): invalid ADC state"));
            break;
    }
    return;
}

/*------------------------------------------------------------------------------
 * Vp3EleResHGGetResult()
 * This function is called by the Vp880Test3EleResHG() state machine during
 * the ELE3_RES_HG_GET_RESULT state.
 *
 * This function is used to collect the result form the PCM integrator. And load
 * the result into the results structure.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  pTestState      - pointer to the device current test state.
 *  pErrorCode      - pointer to the device results errorCode member
 *  pAdcState       - pointer to the value of the current ADC measurement state
 *
 * Returns:
 *  Three possible results of running this function are:
 *  1) The measured data is not valid, request a new one jump to ELE3_RES_HG_COLLECT_DATA.
 *  2) not all adc connections have been made so advance the pAdcState and set
 *     the testState to DC_RLOOP_AD_SETUP.
 *  3) all adc connections have been made jump to ELE3_RES_HG_RESTORE.
 *----------------------------------------------------------------------------*/
static void
Vp3EleResHGGetResult(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    int16 *pTestState,
    uint8 *pAdcState,
    uint8 chanId)
{
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    uint16 *settlingCnt = &pTestHeap->speedupTime;
    int16 *pPreviousMeas = &pTestHeap->vabComputed;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    uint16 maxIterationSet;

    if (TRUE == Vp3EleResHGStoreResult(pLineCtx, pDevObj, chanId, *pAdcState)) {
        /* Allow less settling cycles with a 100ms integration time */
        if (pTestHeap->testArgs.resFltAlt.integrateTime == 800) {
            maxIterationSet = MAX_ITERATION_SET_100MS;
        } else {
            maxIterationSet = MAX_ITERATION_SET_18MS;
        }
        if ((*settlingCnt < maxIterationSet) && (*loopCnt < MAX_ITERATION_ADC)) {
            /* An extra measurement is requested with the same ADC setting */
            *pTestState = ELE3_RES_HG_COLLECT_DATA;
            return;
        } else if (*loopCnt >= MAX_ITERATION_ADC) {
            /* The ADC can't settle */

            /* 
             * During testing it was found that the ADC will not settle in the 
             * presents of some foreign voltages from ring to ground. A secondary
             * symptom of the condition will be a ground key. So if we end up here
             * and we have a ground key then we end the test early with
             * a special error code that will allow ltapi to properly display
             * the info.
             */
            if (pLineObj->lineState.condition & VP_CSLAC_GKEY) {
                *pErrorCode = VP_TEST_STATUS_NO_CONVERGENCE;
                *pTestState = ELE3_RES_HG_RESTORE;
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResHGGetResult(): The ADC can't settle " 
                    "due to line voltage return special error!!!"));
            } else {
                /* Originaly *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR */
                *pErrorCode = VP_TEST_STATUS_NO_CONVERGENCE;
                *pTestState = ELE3_RES_HG_RESTORE;
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResHGGetResult(): The ADC can't settle!"));
            }

            return;
        } else {
            *pPreviousMeas = 0;
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResHGGetResult(): High REN, reduced accuracy"));
        }
    }

    /* move to the next ADC measurment state */
    if (ELE3_RES_HG_MAX_ADC > ++(*pAdcState)) {
        *pTestState = ELE3_RES_HG_CHANGE_FEED;
    } else {
        *pTestState = ELE3_RES_HG_RESTORE;
    }
    return;
}

/*------------------------------------------------------------------------------
 * Vp3EleResHGStoreResult()
 * This function is called by the Vp3EleResHGGetResult() function.
 *
 * Parameters:
 *  pLineCtx       - pointer to the line context.
 *  pDevObj        - pointer to the device object.
 *  chanId         - channel ID.
 *  adcState       - value of the current ADC state.
 *
 * Returns:
 * This function returns a boolean indicating if the measurement was successful
 *
 * Result:
 *  --
 *----------------------------------------------------------------------------*/
static bool
Vp3EleResHGStoreResult(
    VpLineCtxType *pLineCtx,
    Vp880DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState)
{
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 adcConf;
    int16 *pCurrentMeas;
    uint8 aToDRoute;
    int16 iRms = 0;
    bool *firstSettling = &pTestHeap->compensate;
    int16 *pPreviousMeas = &pTestHeap->vabComputed;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    uint16 *settlingCnt = &pTestHeap->speedupTime;

    /* If the adc setup is not set as expected, set it and measure again */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN, &adcConf);
    switch (adcState) {
        case ELE3_RES_HG_IMT_NORMAL_FEED:
        case ELE3_RES_HG_IMT_REVERSE_FEED:
        case ELE3_RES_HG_IMT_ZERO_FEED:
            if ((adcConf & 0x0F) != VP880_METALLIC_DC_I) {
                (*loopCnt)++;
                Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_I, FALSE);
                return TRUE;
            }
            break;
        case ELE3_RES_HG_ILG_NORMAL_FEED:
        case ELE3_RES_HG_ILG_REVERSE_FEED:
        case ELE3_RES_HG_ILG_ZERO_FEED:
            if ((adcConf & 0x0F) != VP880_LONGITUDINAL_DC_I) {
                (*loopCnt)++;
                Vp880AdcSetup(pLineCtx, VP880_LONGITUDINAL_DC_I, FALSE);
                return TRUE;
            }
            break;
        case ELE3_RES_HG_VAH_NORMAL_FEED:
        case ELE3_RES_HG_VAH_REVERSE_FEED:
        case ELE3_RES_HG_VAH_ZERO_FEED:
            if ((adcConf & 0x0F) != VP880_TIP_TO_GND_V) {
                (*loopCnt)++;
                Vp880AdcSetup(pLineCtx, VP880_TIP_TO_GND_V, FALSE);
                return TRUE;
            }
            break;
        case ELE3_RES_HG_VBH_NORMAL_FEED:
        case ELE3_RES_HG_VBH_REVERSE_FEED:
        case ELE3_RES_HG_VBH_ZERO_FEED:
            if ((adcConf & 0x0F) != VP880_RING_TO_GND_V) {
                (*loopCnt)++;
                Vp880AdcSetup(pLineCtx, VP880_RING_TO_GND_V, FALSE);
                return TRUE;
            }
            break;
        default:
            break;
    }

    switch (adcState) {
        case ELE3_RES_HG_IMT_NORMAL_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.imnf;
            aToDRoute = VP880_METALLIC_DC_I;
            break;

        case ELE3_RES_HG_ILG_NORMAL_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.ilnf;
            aToDRoute = VP880_LONGITUDINAL_DC_I;
            break;

        case ELE3_RES_HG_VAH_NORMAL_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.vanf;
            aToDRoute = VP880_TIP_TO_GND_V;
            break;

        case ELE3_RES_HG_VBH_NORMAL_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.vbnf;
            aToDRoute = VP880_RING_TO_GND_V;
            break;

        case ELE3_RES_HG_IMT_REVERSE_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.imrf;
            aToDRoute = VP880_METALLIC_DC_I;
            break;

        case ELE3_RES_HG_ILG_REVERSE_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.ilrf;
            aToDRoute = VP880_LONGITUDINAL_DC_I;
            break;

        case ELE3_RES_HG_VAH_REVERSE_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.varf;
            aToDRoute = VP880_TIP_TO_GND_V;
            break;

        case ELE3_RES_HG_VBH_REVERSE_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.vbrf;
            aToDRoute = VP880_RING_TO_GND_V;
            break;

        case ELE3_RES_HG_IMT_ZERO_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.imzf;
            aToDRoute = VP880_METALLIC_DC_I;
            break;

        case ELE3_RES_HG_ILG_ZERO_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.ilzf;
            aToDRoute = VP880_LONGITUDINAL_DC_I;
            break;

        case ELE3_RES_HG_VAH_ZERO_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.vazf;
            aToDRoute = VP880_TIP_TO_GND_V;
            break;

        case ELE3_RES_HG_VBH_ZERO_FEED:
            pCurrentMeas = &pDevObj->testResults.result.resAlt.vbzf;
            aToDRoute = VP880_RING_TO_GND_V;
            break;

        default:
            pCurrentMeas = NULL;
            aToDRoute = VP880_NO_CONNECT;
            break;
    }
    VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId, aToDRoute, pCurrentMeas);

    switch (adcState) {
        case ELE3_RES_HG_IMT_NORMAL_FEED:
        case ELE3_RES_HG_IMT_REVERSE_FEED:
        case ELE3_RES_HG_IMT_ZERO_FEED:
            if (*firstSettling == TRUE) {
                /* Disconnect the speed-up capacitors after the first cycle */
                pLineObj->icr2Values[2] |= 0x80;
                pLineObj->icr2Values[3] |= 0x80;   /* Disable speed-up capacitors */
                VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_ICR2_WRT,
                    VP880_ICR2_LEN, pLineObj->icr2Values);
                *firstSettling = FALSE;
                (*settlingCnt)++;
                return TRUE;
            }

            if (ABS(*pCurrentMeas - *pPreviousMeas) > SETTLING_ERROR_THRESHOLD) {
                /* Measurement is not stable, need more time to settle */
                *pPreviousMeas = *pCurrentMeas;
                VpGetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, TRUE, chanId,
                    VP880_METALLIC_DC_I, &iRms);
                if ((*settlingCnt == 5) && (iRms > IMT_RMS_THRESHOLD)) {
                    /* If the RMS measurment is still high at the 6th samples,
                       then there is a foreign voltage on the line */
                    pTestHeap->testArgs.resFltAlt.integrateTime = 100 * 8;
                }
                (*settlingCnt)++;
                return TRUE;
            }
            *pPreviousMeas = 0;
            break;

        case ELE3_RES_HG_VAH_NORMAL_FEED:
        case ELE3_RES_HG_VAH_REVERSE_FEED:
        case ELE3_RES_HG_VAH_ZERO_FEED:
            if (ABS(*pCurrentMeas - *pPreviousMeas) > SETTLING_ERROR_THRESHOLD) {
                /* Measurement is not stable, need more time to settle */
                *pPreviousMeas = *pCurrentMeas;
                (*settlingCnt)++;
                return TRUE;
            }
            *pPreviousMeas = 0;
            break;

        default:
            break;
    }

    return FALSE;
}
#endif /* VP880_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_880_SERIES */
