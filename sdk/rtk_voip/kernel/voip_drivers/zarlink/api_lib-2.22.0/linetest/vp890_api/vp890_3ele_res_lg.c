/** file vp890_3ele_res.c
 *
 *  This file contains the 3 ele res primitive algorithm
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 7253 $
 * $LastChangedDate: 2011-03-10 15:41:29 -0600 (Thu, 10 Mar 2011) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_890_SERIES)
#if defined (VP890_INCLUDE_TESTLINE_CODE)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp890_api.h"
#include "vp890_api_int.h"
#include "vp890_testline_int.h"

#define FEED_CHANGE_TIME_CONSTANT       50      /* 50ms */
#define SETTLING_ERROR_THRESHOLD_RG     1       /* 1 adu err in ringing*/
#define SETTLING_ERROR_THRESHOLD        10      /* +/- 10 adu err in low power*/
#define IMT_NF_THRESHOLD                29000   /* Imt threshold to switch from LG to HG primitive */
#define IMT_NF_LP_THRESHOLD             19000   /* Imt threshold to switch from LG to HG primitive */
#define IMT_ZF_THRESHOLD                29000   /* Imt threshold to switch from LG to HG primitive */
#define VAB_NF_THRESHOLD                410     /* Vab threshold to switch from LG to HG primitive */
#define IMT_RMS_NF_THRESHOLD            400     /* IMT threshold to switch from 18ms intg to 100ms*/
#define IMT_ACTIVE_10MA                 5400    /* Metallic current threshold to detect off-hook*/
#define LG_TEST_THRESHOLD               5       /* Threshold for the first settling time:
                                                continue in low gain only OR ringing/low gain */
#define MAX_ITERATION_ADC               10      /* Max number ADC failures */
#define MAX_ITERATION_SET               80      /* Max number of measurement for settling */
#define MAX_ITERATION_SET_RG            30      /* Max number of measurement for settling */

#ifndef ABS
    #define ABS(a) (((a) < 0) ? -(a) : (a))
#endif

typedef enum
{
    COMMON_TEST_SETUP           = VP890_TESTLINE_GLB_STRT_STATE,
    ELE3_RES_LG_SET_ADC_CURRENT = 5,
    ELE3_RES_LG_MEASURE_CURRENT = 10,
    ELE3_RES_LG_CHECK_CURRENT   = 15,
    ELE3_RES_LG_SETUP           = 20,
    ELE3_RES_LG_CHANGE_FEED     = 25,
    ELE3_RES_LG_ADC_SETUP       = 30,
    ELE3_RES_LG_COLLECT_DATA    = 35,
    ELE3_RES_LG_GET_RESULT      = 40,
    ELE3_RES_LG_RESTORE         = 45,
    ELE3_RES_LG_END             = VP890_TESTLINE_GLB_END_STATE,
    ELE3_RES_LG_QUIT            = VP890_TESTLINE_GLB_QUIT_STATE,
    ELE3_RES_LG_ENUM_SIZE       = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vp3EleResTestStateTypes;

typedef enum
{
    ELE3_RES_LG_IMT_NORMAL_FEED_RG,
    ELE3_RES_LG_VAB_NORMAL_FEED_RG,
    ELE3_RES_LG_IMT_NORMAL_FEED,
    ELE3_RES_LG_ILG_NORMAL_FEED,
    ELE3_RES_LG_VAL_NORMAL_FEED,
    ELE3_RES_LG_VBL_NORMAL_FEED, /* 5 */
    ELE3_RES_LG_IMT_ZERO_FEED_RG,
    ELE3_RES_LG_VAB_ZERO_FEED_RG,
    ELE3_RES_LG_IMT_ZERO_FEED,
    ELE3_RES_LG_ILG_ZERO_FEED,
    ELE3_RES_LG_VAL_ZERO_FEED, /* 10 */
    ELE3_RES_LG_VBL_ZERO_FEED,
    ELE3_RES_LG_IMT_REVERSE_FEED_RG,
    ELE3_RES_LG_VAB_REVERSE_FEED_RG,
    ELE3_RES_LG_IMT_REVERSE_FEED,
    ELE3_RES_LG_ILG_REVERSE_FEED, /* 15 */
    ELE3_RES_LG_VAL_REVERSE_FEED,
    ELE3_RES_LG_VBL_REVERSE_FEED,
    ELE3_RES_LG_MAX_ADC,
    ELE3_RES_LG_ADC_ORDER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vp3EleResAdcOrderTypes;

static VpStatusType
Vp3EleResLGArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
Vp3EleResLGSetup(
    VpLineCtxType *pLineCtx,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static uint16
Vp3EleResLGChangeFeed(
    VpLineCtxType *pLineCtx,
    uint8 *pAdcState);

static void
Vp3EleResLGAdcSetup(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static void
Vp3EleResLGCollectData(
    VpLineCtxType *pLineCtx,
    uint8 adcState);

static void
Vp3EleResLGGetResult(
    VpLineCtxType *pLineCtx,
    int16 *pTestState,
    uint8 *pAdcState,
    uint8 chanId);

static bool
Vp3EleResLGStoreResult(
    VpLineCtxType *pLineCtx,
    uint8 chanId,
    uint8 adcState);

static bool
Vp3EleResLGCheckADC(
    VpLineCtxType *pLineCtx,
    uint8 *loopCnt,
    uint8 adcState);

static int16
Vp3EleResLGStoreMeasurement(
    Vp890DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState);

static bool
Vp3EleResLGCheckSettling(
    Vp890TestHeapType *pTestHeap,
    int16 currentValue,
    uint8 adcState);

static bool
Vp3EleResLGCheckHighGain(
    VpLineCtxType *pLineCtx,
    int16 *pTestState,
    uint8 adcState);

/*------------------------------------------------------------------------------
 * Vp890Test3EleResLG()
 * This functions implements the 3 Ele Res Test primitive ...
 *
 * Parameters:
 *  pLineCtx     - pointer to the line context
 *  pArgsUntyped - pointer to the test specific inputs.
 *  handle       - unique test handle
 *  callback     - indicates if this function was called via a callback
 *  testId       - test identifier
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp890Test3EleResLG(
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
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    VpTest3EleResAltResType *p3EleResData = &(pTestHeap->testArgs).resFltAlt;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint16 *settlingCnt = &pTestHeap->speedupTime;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint16 time;


    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890Test3EleResLG(ts:%i)", *pTestState));
    if (FALSE == callback) {
        return Vp3EleResLGArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    switch (*pTestState) {
        case COMMON_TEST_SETUP:
            Vp890CommonTestSetup(pLineCtx, deviceId);
            /* No break */

        case ELE3_RES_LG_SET_ADC_CURRENT:
            Vp3EleResLGAdcSetup(pLineCtx, ELE3_RES_LG_IMT_NORMAL_FEED_RG);
            *settlingCnt = 0;
            *loopCnt = 0;

            /* Only VVP can run the high gain primitive */
            if ((p3EleResData->settlingTime & 0x8000) == 0x0000) {
                Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_MEASURE_CURRENT,
                    MS_TO_TICKRATE(10, tick));
            } else {
                Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_SETUP,
                    MS_TO_TICKRATE(10, tick));
            }
            break;

        case ELE3_RES_LG_MEASURE_CURRENT: {
            uint8 adcConf;

            /* If the adc setup is not set as expected, set it and measure again */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD, VP890_CONV_CFG_LEN,
                &adcConf);
            if ((adcConf & 0x0F) != VP890_METALLIC_DC_I) {
                if ((*loopCnt)++ >= MAX_ITERATION_ADC) {
                    /* Metalic current too high (close to hook threshold) -> run high gain */
                    VP_TEST(VpLineCtxType, pLineCtx,
                        ("The ADC can't settle! Carry on in high gain"));
                    pDevObj->testResults.result.resAlt.imnf = VP_INT16_MAX;
                    *pTestState = ELE3_RES_LG_END;
                    Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState,
                        MS_TO_TICKRATE(10, tick));
                    break;
                }
                Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_I, FALSE);
                Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_MEASURE_CURRENT,
                    MS_TO_TICKRATE(10, tick));
                break;
            }

            /* Measure imt 0 settling / 18.25ms integration */
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE, ELE3_RES_LG_CHECK_CURRENT,
                0, 146);
            break;
        }

        case ELE3_RES_LG_CHECK_CURRENT: {
            int16 imtActive;
            bool loop;

            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId,
                VP890_METALLIC_DC_I, &imtActive);
            /* Check if metalic current is stable */
            loop = Vp3EleResLGCheckSettling(pTestHeap, imtActive, ELE3_RES_LG_IMT_NORMAL_FEED_RG);

            if ((*settlingCnt < MAX_ITERATION_SET_RG) && (loop == TRUE)) {
                /* imt is not stable, need more time to settle */
                Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_MEASURE_CURRENT,
                    MS_TO_TICKRATE(10, tick));
                break;
            } else {
                if (ABS(imtActive) > IMT_ACTIVE_10MA) {
                    /* Metalic current too high -> run high gain */
                    pDevObj->testResults.result.resAlt.imnf = VP_INT16_MAX;
                    *pTestState = ELE3_RES_LG_END;
                    Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState,
                        MS_TO_TICKRATE(10, tick));
                    break;
                }
                /* Otherwise continue in low gain */
            }
        }

        case ELE3_RES_LG_SETUP: {
            Vp3EleResLGSetup(pLineCtx, pTestHeap, deviceId);

            Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_CHANGE_FEED, NEXT_TICK);
            break;
        }

        case ELE3_RES_LG_CHANGE_FEED:
            /* Change the current feed condtion and power if needeed */
            time = Vp3EleResLGChangeFeed(pLineCtx, pAdcState);

            if (time > 0) {
                Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_ADC_SETUP,
                    MS_TO_TICKRATE(time, tick));
                break;
            }
            /* No break */

        case ELE3_RES_LG_ADC_SETUP:
            Vp3EleResLGAdcSetup(pLineCtx, *pAdcState);

            Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_COLLECT_DATA, NEXT_TICK);
            break;

        case ELE3_RES_LG_COLLECT_DATA:
            Vp3EleResLGCollectData(pLineCtx, *pAdcState);
            break;

        case ELE3_RES_LG_GET_RESULT:
            /* Store the PCM data in the results structure */
            Vp3EleResLGGetResult(pLineCtx, pTestState, pAdcState, chanId);

            Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;

        case ELE3_RES_LG_RESTORE: {
            uint8 mpiBuffer[8 + VP890_SYS_STATE_LEN + VP890_LOOP_SUP_LEN +
                            VP890_DC_FEED_LEN + VP890_REGULATOR_PARAM_LEN +
                            VP890_ICR1_LEN + VP890_ICR2_LEN + VP890_ICR3_LEN +
                            VP890_ICR4_LEN ];
            uint8 mpiIndex = 0;
            uint8 slacState =  VP890_SS_IDLE | VP890_SS_ACTIVATE_MASK;

            /* Restore registers */
            /* Must take the slic out of low gain state before restoring */
            /* registers otherwise risk huge transients. */

            pLineObj->slicValueCache = slacState;
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SYS_STATE_WRT,
                VP890_SYS_STATE_LEN, &slacState);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_LOOP_SUP_WRT,
                VP890_LOOP_SUP_LEN, pTestHeap->lpSuper);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
                VP890_DC_FEED_LEN, pTestHeap->dcFeed);

            VpMemCpy(pDevObj->swParamsCache, pTestHeap->switchReg,
                VP890_REGULATOR_PARAM_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_REGULATOR_PARAM_WRT,
                VP890_REGULATOR_PARAM_LEN, pTestHeap->switchReg);

            VpMemCpy(pLineObj->icr1Values, pTestHeap->icr1, VP890_ICR1_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR1_WRT,
                VP890_ICR1_LEN, pTestHeap->icr1);

            VpMemCpy(pLineObj->icr2Values, pTestHeap->icr2, VP890_ICR2_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
                VP890_ICR2_LEN, pTestHeap->icr2);

            VpMemCpy(pLineObj->icr3Values, pTestHeap->icr3, VP890_ICR3_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
                VP890_ICR3_LEN, pTestHeap->icr3);

            VpMemCpy(pLineObj->icr4Values, pTestHeap->icr4, VP890_ICR4_LEN);
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
                VP890_ICR4_LEN, pTestHeap->icr4);

            /* Send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

            Vp890SetTestStateAndTimer(pDevObj, pTestState, ELE3_RES_LG_END,
                MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME, tick));

            break;
        }

        case ELE3_RES_LG_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890Test3EleResLG(): complete"));
            break;

        case ELE3_RES_LG_QUIT:
            /* The test has eneded early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890Test3EleResLG(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890Test3EleResLG(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGArgCheck()
 * This functions is called by the Vp890Test3EleResLG() function if
 * Vp890Test3EleResLG() was not called via a callback.
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
 *  testId          - test identifier
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
Vp3EleResLGArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTest3EleResAltResType *p3EleResData = &(pTestInfo->pTestHeap->testArgs).resFltAlt;
    const VpTest3EleResAltResType *pTestInput = pArgsUntyped;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* Make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResHGArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */
    *pAdcState = ELE3_RES_LG_IMT_NORMAL_FEED_RG;
    pDevObj->testResults.result.resAlt.highGain = FALSE;
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

    /* This test cannot run if the isolate relay is open */
    if (VP_RELAY_RESET == pLineObj->relayState) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp3EleResLGArgCheck(): bailed openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* TODO: check input arguments before assigining them to test struct!!! */
    p3EleResData->feedBias = pTestInput->feedBias;
    p3EleResData->shiftZeroFeed = pTestInput->shiftZeroFeed;
    p3EleResData->integrateTime = pTestInput->integrateTime;
    p3EleResData->settlingTime = pTestInput->settlingTime; /* The highest bit is set to 1 for VVA */
    

    /*
     * Force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE, NEXT_TICK);

    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGSetup()
 * This function is called by the Vp890Test3EleResLG() state machine during
 * the ELE3_RES_LG_SETUP state.
 *
 * This function is used to configure the line for the Low Gain Res Flt test.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestHeap       - pointer to the test heap.
 *  deviceId        - device ID
 *
 * Returns:
 * --
 * Result:
 *
 *----------------------------------------------------------------------------*/
static void
Vp3EleResLGSetup(
    VpLineCtxType *pLineCtx,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    uint8 dcfeed[VP890_DC_FEED_LEN] = {0x6F, 0x1F};
    uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* +6V */
    uint8 sigGenCtrl = VP890_GEN_CTRL_EN_BIAS | VP890_GEN_CTRL_EN_A;
    uint8 switchRegParam[VP890_REGULATOR_PARAM_LEN] = {0x0A, 0x04, 0x04}; /* battery 25V fixed */
    uint8 lpSuper[VP890_LOOP_SUP_LEN];
    uint8 slacState =  VP890_SS_ACTIVE;     /* Active state and codec activated */
    uint8 chanId = pLineObj->channelId;
    int16 biasErr;

    uint8 mpiBuffer[VP890_LOOP_SUP_LEN+1 +
                    VP890_SYS_STATE_LEN+1 +
                    VP890_REGULATOR_PARAM_LEN+1 +
                    VP890_GEN_CTRL_LEN+1 +
                    VP890_DC_FEED_LEN+1 +
                    VP890_ICR1_LEN+1 +
                    VP890_ICR4_LEN+1 +
                    VP890_ICR2_LEN+1 +
                    VP890_ICR3_LEN+1 +
                    VP890_SIGA_PARAMS_LEN+1];
    uint8 mpiIndex = 0;

    switchRegParam[0] = (pTestHeap->switchReg[0] & 0xF5) | switchRegParam[0];
    switchRegParam[1] = (pTestHeap->switchReg[1] & 0xE0) | switchRegParam[1];
    switchRegParam[2] = (pTestHeap->switchReg[2] & 0xE0) | switchRegParam[2];

    pLineObj->icr1Values[0] = 0xFF;
    pLineObj->icr1Values[1] = 0xC8;
    pLineObj->icr1Values[2] = 0xFF;
    pLineObj->icr1Values[3] = 0x0C;

    pLineObj->icr2Values[0] = 0xFF;
    pLineObj->icr2Values[1] = 0xDC;
    pLineObj->icr2Values[2] &= 0x0C;
    pLineObj->icr2Values[2] |= 0xF3;
    pLineObj->icr2Values[3] &= 0x0C;
    pLineObj->icr2Values[3] |= 0x61;

    pLineObj->icr3Values[0] = 0xFF;
    pLineObj->icr3Values[1] = 0x21;
    pLineObj->icr3Values[2] = 0xFF;
    pLineObj->icr3Values[3] = 0xCE;

    pLineObj->icr4Values[0] = 0xFF;
    pLineObj->icr4Values[1] = 0x03;
    pLineObj->icr4Values[2] = 0xFF;
    pLineObj->icr4Values[3] = 0x2C;

    /* Masking the hook detection by maxing out TSH */
    lpSuper[0] = pTestHeap->lpSuper[0] | 0x07;
    lpSuper[1] = 0xFF;
    lpSuper[2] = pTestHeap->lpSuper[2];
    lpSuper[3] = pTestHeap->lpSuper[3];

    pTestHeap->lowGain = FALSE;

    /* Set a calibrated feed */
    biasErr = -pTestHeap->testArgs.resFltAlt.feedBias;
    biasErr += 2 * ((pDevObj->vp890SysCalData.sigGenAError[chanId][0] -
        pDevObj->vp890SysCalData.vocOffset[chanId][VP890_NORM_POLARITY]) * 15 / 10);
    sigGenA[1] = (uint8)((biasErr >> 8) & 0x00FF);
    sigGenA[2] = (uint8)(biasErr & 0x00FF);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SYS_STATE_WRT,
        VP890_SYS_STATE_LEN, &slacState);
    pLineObj->slicValueCache = slacState;

    /* Regular tracker */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_REGULATOR_PARAM_WRT,
        VP890_REGULATOR_PARAM_LEN, switchRegParam);
    VpMemCpy(pDevObj->swParamsCache, switchRegParam, VP890_REGULATOR_PARAM_LEN);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_LOOP_SUP_WRT,
        VP890_LOOP_SUP_LEN, lpSuper);

    pLineObj->sigGenCtrl[0] = sigGenCtrl;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_GEN_CTRL_WRT,
        VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);

    /* LI=1, min VOC; max curr lim */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
        VP890_DC_FEED_LEN, dcfeed);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR1_WRT,
        VP890_ICR1_LEN, pLineObj->icr1Values);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
        VP890_ICR4_LEN, pLineObj->icr4Values);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
        VP890_ICR2_LEN, pLineObj->icr2Values);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
        VP890_ICR3_LEN, pLineObj->icr3Values);

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SIGA_PARAMS_WRT,
        VP890_SIGA_PARAMS_LEN, sigGenA);

    /* Send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);
    return;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGChangeFeed()
 * This function is called by the Vp890Test3EleResLG() state machine during
 * the ELE3_RES_LG_CHANGE_FEED state.
 *
 * This function is used to change the the feed (VOC +6V -> 0V -> -6V).
 * It is also used to initialize some variables
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pAdcState       - pointer to the current value of the adc setup state.
 *
 * Returns:
 *  Duration of the next timer.
 * Result:
 * If the incoming adc state is not a feed condition change then nothing
 * happens and the next timer duration is set to 0. Other wise the device
 * is changed according to the feed state and the time argument is set to
 * state change settling time.
 *----------------------------------------------------------------------------*/
static uint16
Vp3EleResLGChangeFeed(
    VpLineCtxType *pLineCtx,
    uint8 *pAdcState)
{
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    int16 *previousValue = &pTestHeap->previousAvg;
    int16 *vabComputed = &pTestHeap->vabComputed;
    uint16 *settlingCnt = &pTestHeap->speedupTime;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    uint8 sigGenA[VP890_SIGA_PARAMS_LEN] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 dcfeed[VP890_DC_FEED_LEN] = {0x6F, 0x1F};
    uint16 nextTimer;
    int16 feedBias;
    int16 biasErr;

    /* Initialize the 'previous' measurement used in he settling algorithm */
    *previousValue = 0;

    /* Initialize the settling counter and the ADC retry counter */
    *settlingCnt = 0;
    *loopCnt = 0;

    VP_TEST(VpLineCtxType, pLineCtx, (" Vp3EleResLGChangeFeed(%i)", *pAdcState));

    switch (*pAdcState) {
        case ELE3_RES_LG_IMT_NORMAL_FEED:
        case ELE3_RES_LG_IMT_ZERO_FEED:
        case ELE3_RES_LG_IMT_REVERSE_FEED: {
            /* switch to low gain */
            uint8 mpiBuffer[VP890_ICR1_LEN+1 +
                            VP890_ICR2_LEN+1 +
                            VP890_ICR3_LEN+1 +
                            VP890_DC_FEED_LEN+1 +
                            VP890_SIGA_PARAMS_LEN+1];
            uint8 mpiIndex = 0;

            pLineObj->icr1Values[0] = 0xFF;
            pLineObj->icr1Values[1] = 0x01;
            pLineObj->icr1Values[2] = 0xCF;
            pLineObj->icr1Values[3] = 0x00;

            pLineObj->icr2Values[0] = 0xFF;
            pLineObj->icr2Values[1] = 0xFD;
            pLineObj->icr2Values[2] &= 0x0C;
            pLineObj->icr2Values[2] |= 0xF3;
            pLineObj->icr2Values[3] &= 0x0C;
            pLineObj->icr2Values[3] |= 0x71;

            pLineObj->icr3Values[0] = 0xFF;
            pLineObj->icr3Values[1] = 0xE9;
            pLineObj->icr3Values[2] = 0xFF;
            pLineObj->icr3Values[3] = 0xC0;

            /* Use the VAB measured in ringing to set the VAB in low gain */
            *vabComputed = (*vabComputed * -2) * 15 / 10;
            sigGenA[1] = (uint8)((*vabComputed & 0xFF00) >> 8);
            sigGenA[2] = (uint8)(*vabComputed & 0x00FF);
            *vabComputed = 0;

            /* Shift the longitudinal voltage if needed */
            if ((*pAdcState == ELE3_RES_LG_IMT_ZERO_FEED) &&
                (pTestHeap->testArgs.resFltAlt.shiftZeroFeed == TRUE)) {
                dcfeed[0] = 0x27;
            }

            /* Go to low gain */
            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
                VP890_DC_FEED_LEN, dcfeed);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR1_WRT,
                VP890_ICR1_LEN, pLineObj->icr1Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
                VP890_ICR2_LEN, pLineObj->icr2Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
                VP890_ICR3_LEN, pLineObj->icr3Values);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SIGA_PARAMS_WRT,
                VP890_SIGA_PARAMS_LEN, sigGenA);

            /* Send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

            nextTimer = VP890_STATE_CHANGE_SETTLING_TIME_SHORT;  /* 50ms */
            break;
        }

        case ELE3_RES_LG_IMT_ZERO_FEED_RG:
        case ELE3_RES_LG_IMT_REVERSE_FEED_RG: {
            uint8 mpiBuffer[VP890_ICR3_LEN+1 +
                            VP890_ICR2_LEN+1 +
                            VP890_ICR1_LEN+1 +
                            VP890_DC_FEED_LEN+1 +
                            VP890_SIGA_PARAMS_LEN+1];
            uint8 mpiIndex = 0;

            if ((pTestHeap->lowGain == TRUE) && (*pAdcState == ELE3_RES_LG_IMT_ZERO_FEED_RG)) {
                /* Change the feed: zero feed low gain */
                *pAdcState = ELE3_RES_LG_IMT_ZERO_FEED;
                if (pTestHeap->testArgs.resFltAlt.shiftZeroFeed == TRUE) {
                    dcfeed[0] = 0x27;
                }
            } else if ((pTestHeap->lowGain == TRUE) &&
                       (*pAdcState == ELE3_RES_LG_IMT_REVERSE_FEED_RG)) {
                /* Change the feed: reverse feed low gain */
                *pAdcState = ELE3_RES_LG_IMT_REVERSE_FEED;
                feedBias = -pTestHeap->testArgs.resFltAlt.feedBias;
                sigGenA[1] = (uint8)((feedBias & 0xFF00) >> 8);
                sigGenA[2] = (uint8)(feedBias & 0x00FF);
            } else {
                /* Go to high gain and change the feed */
                pLineObj->icr3Values[0] = 0xFF;
                pLineObj->icr3Values[1] = 0x21;
                pLineObj->icr3Values[2] = 0xFF;
                pLineObj->icr3Values[3] = 0xCE;

                pLineObj->icr2Values[0] = 0xFF;
                pLineObj->icr2Values[1] = 0xDC;
                pLineObj->icr2Values[2] &= 0x0C;
                pLineObj->icr2Values[2] |= 0xF3;
                pLineObj->icr2Values[3] &= 0x0C;
                pLineObj->icr2Values[3] |= 0x61;

                pLineObj->icr1Values[0] = 0xFF;
                pLineObj->icr1Values[1] = 0xC8;
                pLineObj->icr1Values[2] = 0xCF;
                pLineObj->icr1Values[3] = 0x0C;

                if (*pAdcState == ELE3_RES_LG_IMT_ZERO_FEED_RG) {
                    /* Change the feed: zero feed ringing */
                    feedBias = 0;
                    if (pTestHeap->testArgs.resFltAlt.shiftZeroFeed == TRUE) {
                        dcfeed[0] = 0x27;
                    }
                } else {
                    /* Change the feed: reverse feed ringing */
                    feedBias = pTestHeap->testArgs.resFltAlt.feedBias;
                }
                biasErr = feedBias;
                biasErr += 2 * ((pDevObj->vp890SysCalData.sigGenAError[pLineObj->channelId][0] -
                    pDevObj->vp890SysCalData.vocOffset[pLineObj->channelId][VP890_NORM_POLARITY]) * 15 / 10);
                sigGenA[1] = (uint8)((biasErr >> 8) & 0x00FF);
                sigGenA[2] = (uint8)(biasErr & 0x00FF);

                /* Connect the signal generator (backdoor ringing) */
                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
                    VP890_ICR3_LEN, pLineObj->icr3Values);

                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
                    VP890_ICR2_LEN, pLineObj->icr2Values);

                mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR1_WRT,
                    VP890_ICR1_LEN, pLineObj->icr1Values);
            }

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
                VP890_DC_FEED_LEN, dcfeed);

            mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SIGA_PARAMS_WRT,
                VP890_SIGA_PARAMS_LEN, sigGenA);

            /* Send down the mpi commands */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

            nextTimer = VP890_STATE_CHANGE_SETTLING_TIME_SHORT;  /* 50ms */
            break;
        }

        default:
            /* No change in feed, so no need to set a timer */
            nextTimer = 0;
            break;
    }

    return nextTimer;
}

/*------------------------------------------------------------------------------
 * Vp3EleResAdcSetup()
 * This function is called by the Vp890Test3EleResLG() state machine during
 * the ELE3_RES_LG_ADC_SETUP state.
 *
 * This function is used to call the Vp890AdcSetup() depending on the
 * current ac rloop adc state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  adcState        - current value of the adc setup state.
 *
 * Returns:
 *  --
 *
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static void
Vp3EleResLGAdcSetup(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    switch (adcState) {
        case ELE3_RES_LG_IMT_NORMAL_FEED:   /* imt->ilg in low gain */
        case ELE3_RES_LG_IMT_REVERSE_FEED:
        case ELE3_RES_LG_IMT_ZERO_FEED:
            Vp890AdcSetup(pLineCtx, VP890_LONGITUDINAL_DC_I, FALSE);
            break;

        case ELE3_RES_LG_IMT_NORMAL_FEED_RG:
        case ELE3_RES_LG_IMT_REVERSE_FEED_RG:
        case ELE3_RES_LG_IMT_ZERO_FEED_RG:
        case ELE3_RES_LG_ILG_NORMAL_FEED:   /* ilg->imt in low gain */
        case ELE3_RES_LG_ILG_REVERSE_FEED:
        case ELE3_RES_LG_ILG_ZERO_FEED:
            Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_I, FALSE);
            break;

        case ELE3_RES_LG_VAL_NORMAL_FEED:
        case ELE3_RES_LG_VAL_REVERSE_FEED:
        case ELE3_RES_LG_VAL_ZERO_FEED:
            Vp890AdcSetup(pLineCtx, VP890_TIP_TO_GND_V, FALSE);
            break;

        case ELE3_RES_LG_VBL_NORMAL_FEED:
        case ELE3_RES_LG_VBL_REVERSE_FEED:
        case ELE3_RES_LG_VBL_ZERO_FEED:
            Vp890AdcSetup(pLineCtx, VP890_RING_TO_GND_V, FALSE);
            break;

        case ELE3_RES_LG_VAB_NORMAL_FEED_RG:
        case ELE3_RES_LG_VAB_REVERSE_FEED_RG:
        case ELE3_RES_LG_VAB_ZERO_FEED_RG:
            Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_V, FALSE);
            break;

        default:
            break;
    }

    return;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGCollectData()
 * This function is called by the Vp890Test3EleResLG() state machine during
 * the ELE3_RES_LG_COLLECT_DATA state.
 *
 * This function is used to set the proper settling and integration time
 * zaacording to the ADC state.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  adcState        - current value of the adc setup state.
 *
 * Result:
 * --
 *----------------------------------------------------------------------------*/
static void
Vp3EleResLGCollectData(
    VpLineCtxType *pLineCtx,
    uint8 adcState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTest3EleResAltResType *p3EleResData = &(pTestInfo->pTestHeap->testArgs).resFltAlt;
    uint16 settlingTime = 0;       /* PcmCollect settling time in milliseconds */
    uint16 integrationTime = 0;    /* PcmCollect integration time in milliseconds */

    switch (adcState) {
        case ELE3_RES_LG_IMT_NORMAL_FEED_RG:
        case ELE3_RES_LG_IMT_ZERO_FEED_RG:
        case ELE3_RES_LG_IMT_REVERSE_FEED_RG:
        case ELE3_RES_LG_IMT_NORMAL_FEED:
        case ELE3_RES_LG_IMT_ZERO_FEED:
        case ELE3_RES_LG_IMT_REVERSE_FEED:
        case ELE3_RES_LG_ILG_NORMAL_FEED:
        case ELE3_RES_LG_ILG_ZERO_FEED:
        case ELE3_RES_LG_ILG_REVERSE_FEED:
            /* These measurements need to be immune to the foreign voltage if any */
            settlingTime = (p3EleResData->settlingTime & 0x7FFF);
            integrationTime = p3EleResData->integrateTime;      /* 18ms or 100ms */
            break;

        case ELE3_RES_LG_VAB_NORMAL_FEED_RG:
        case ELE3_RES_LG_VAL_NORMAL_FEED:
        case ELE3_RES_LG_VBL_NORMAL_FEED:
        case ELE3_RES_LG_VAB_ZERO_FEED_RG:
        case ELE3_RES_LG_VAL_ZERO_FEED:
        case ELE3_RES_LG_VBL_ZERO_FEED:
        case ELE3_RES_LG_VAB_REVERSE_FEED_RG:
        case ELE3_RES_LG_VAL_REVERSE_FEED:
        case ELE3_RES_LG_VBL_REVERSE_FEED:
            /* These measurements can be impacted by a foreign voltage, no settling */
            settlingTime = (p3EleResData->settlingTime & 0x7FFF);
            integrationTime = 18;
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp3EleResLGCollectData(): invalid ADC state"));
            settlingTime = 0;
            integrationTime = 0;
            break;
    }

    /* Run the PCM collect with a variable settling time and integration time */
    Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE | VP_PCM_OPERATION_RMS,
        ELE3_RES_LG_GET_RESULT, settlingTime * 8, integrationTime * 8);

    return;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGGetResult()
 * This function is called by the Vp890Test3EleResLG() state machine during
 * the ELE3_RES_LG_GET_RESULT state. Another measurement will be requested if
 * the ADC was reseted or if a measurement needs more timr to settle.
 *
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestState      - pointer to the device current test state.
 *  adcState        - current value of the adc setup state.
 *  pAdcState       - pointer to the value of the current ADC measurement state.
 *  chanId          - channel identifier.
 *
 * Returns:
 *  Three possible results of running this function are:
 *  1) The measured data is not valid, request a new one jump to ELE3_RES_LG_COLLECT_DATA.
 *  2) The ADC can't stay in the requested setting, bail-out with an error.
 *  3) The measurement can't settle within the required time, issue a warning
 *     and go to the next measurement.
 *----------------------------------------------------------------------------*/
static void
Vp3EleResLGGetResult(
    VpLineCtxType *pLineCtx,
    int16 *pTestState,
    uint8 *pAdcState,
    uint8 chanId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    uint8 *loopCnt = &pTestHeap->loopCnt;
    uint16 *settlingCnt = &pTestHeap->speedupTime;
    int16 *previousValue = &pTestHeap->previousAvg;
    VpTestStatusType *pErrorCode = &pDevObj->testResults.errorCode;
    VpTest3EleResAltResType *p3EleResData = &(pTestHeap->testArgs).resFltAlt;
    uint16 maxIteration;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResLGGetResult(%d): settlingCnt = %d, loopCnt = %d",
        *pAdcState, *settlingCnt, *loopCnt));

    if (TRUE == Vp3EleResLGStoreResult(pLineCtx, chanId, *pAdcState)) {
        if ((*pAdcState == ELE3_RES_LG_IMT_NORMAL_FEED_RG) ||
            (*pAdcState == ELE3_RES_LG_IMT_ZERO_FEED_RG) ||
            (*pAdcState == ELE3_RES_LG_IMT_REVERSE_FEED_RG)) {
            maxIteration = MAX_ITERATION_SET_RG;
        } else {
            maxIteration = MAX_ITERATION_SET;
        }

        if ((*settlingCnt < maxIteration) && (*loopCnt < MAX_ITERATION_ADC)) {
            /* An extra measurement is requested with the same ADC setting */
            *pTestState = ELE3_RES_LG_COLLECT_DATA;
            return;
        } else if (*loopCnt >= MAX_ITERATION_ADC) {
            /* The ADC can't settle */
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            *pTestState = ELE3_RES_LG_QUIT;
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResLGGetResult(): The ADC can't settle!"));
            return;
        } else {
            *previousValue = 0;
            VP_TEST(VpLineCtxType, pLineCtx,
                ("Vp3EleResLGGetResult(): Took too long to settle, reduced accuracy"));
        }
    }

    /* Only VVP can run the high gain primitive */
    if ((p3EleResData->settlingTime & 0x8000) == 0x0000) {
        /* Check if the metallic current is too high, if so, bail-out for the high gain primitive */
        if (TRUE == Vp3EleResLGCheckHighGain(pLineCtx, pTestState, *pAdcState)) {
            return;
        }
    }

    /* Move to the next ADC measurment state */
    if (ELE3_RES_LG_MAX_ADC > ++(*pAdcState)) {
        *pTestState = ELE3_RES_LG_CHANGE_FEED;
    } else {
        *pTestState = ELE3_RES_LG_RESTORE;
    }
    return;
}


/*------------------------------------------------------------------------------
 * Vp3EleResLGStoreResult()
 * This function is called by the Vp3EleResLGGetResult() function, it is
 * used to collect the result form the PCM integrator, load the result
 * into the results structure and check the settling of a measurement
 * if needed.
 *
 * Parameters:
 *  pLineCtx       - pointer to the line context.
 *  chanId         - channel ID.
 *  adcState       - value of the current ADC state.
 *
 * Returns:
 * This function returns a boolean indicating if the measurement was successful
 * TRUE: need an extra measurement
 * FALSE: the measurement was successful, go to the next one.
 *
 * Result:
 *  --
 *----------------------------------------------------------------------------*/
static bool
Vp3EleResLGStoreResult(
    VpLineCtxType *pLineCtx,
    uint8 chanId,
    uint8 adcState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    int16 currentValue;
    uint8 *loopCnt = &pTestHeap->loopCnt;

    /* Check the ADC state, if it was reseted, set it and try again */
    if (Vp3EleResLGCheckADC(pLineCtx, loopCnt, adcState) == TRUE) {
        return TRUE;
    }

    /* Get the PCM measurement and store it in the result structure */
    currentValue = Vp3EleResLGStoreMeasurement(pDevObj, chanId, adcState);

    /* Check if the current measurement has settled (if requested to) */
    if (Vp3EleResLGCheckSettling(pTestHeap, currentValue, adcState) == TRUE) {
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGCheckADC()
 * This function is called by Vp3EleResLGStoreResult() to check the ADC internal
 * switch.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  loopCnt         - pointer to the ADC failure counter.
 *  adcState        - ADC state.
 *
 * Returns:
 *  TRUE: the ADC was reseted.
 *  FALSE: the ADC was set as expected.
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
Vp3EleResLGCheckADC(
    VpLineCtxType *pLineCtx,
    uint8 *loopCnt,
    uint8 adcState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 adcConf;

    /* If the adc setup is not set as expected, set it and measure again */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD, VP890_CONV_CFG_LEN, &adcConf);
    switch (adcState) {
        case ELE3_RES_LG_IMT_NORMAL_FEED:   /* imt->ilg in low gain */
        case ELE3_RES_LG_IMT_REVERSE_FEED:
        case ELE3_RES_LG_IMT_ZERO_FEED:
            if ((adcConf & 0x0F) != VP890_LONGITUDINAL_DC_I) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_LONGITUDINAL_DC_I, FALSE);
                return TRUE;
            }
            break;

        case ELE3_RES_LG_IMT_NORMAL_FEED_RG:
        case ELE3_RES_LG_IMT_REVERSE_FEED_RG:
        case ELE3_RES_LG_IMT_ZERO_FEED_RG:
        case ELE3_RES_LG_ILG_NORMAL_FEED:   /* ilg->imt in low gain */
        case ELE3_RES_LG_ILG_REVERSE_FEED:
        case ELE3_RES_LG_ILG_ZERO_FEED:
            if ((adcConf & 0x0F) != VP890_METALLIC_DC_I) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_I, FALSE);
                return TRUE;
            }
            break;

        case ELE3_RES_LG_VAL_NORMAL_FEED:
        case ELE3_RES_LG_VAL_REVERSE_FEED:
        case ELE3_RES_LG_VAL_ZERO_FEED:
            if ((adcConf & 0x0F) != VP890_TIP_TO_GND_V) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_TIP_TO_GND_V, FALSE);
                return TRUE;
            }
            break;

        case ELE3_RES_LG_VBL_NORMAL_FEED:
        case ELE3_RES_LG_VBL_REVERSE_FEED:
        case ELE3_RES_LG_VBL_ZERO_FEED:
            if ((adcConf & 0x0F) != VP890_RING_TO_GND_V) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_RING_TO_GND_V, FALSE);
                return TRUE;
            }
            break;

        case ELE3_RES_LG_VAB_NORMAL_FEED_RG:
        case ELE3_RES_LG_VAB_REVERSE_FEED_RG:
        case ELE3_RES_LG_VAB_ZERO_FEED_RG:
            if ((adcConf & 0x0F) != VP890_METALLIC_DC_V) {
                (*loopCnt)++;
                Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_V, FALSE);
                return TRUE;
            }
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp3EleResLGCheckADC(): invalid ADC state"));
            return FALSE;
            break;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGStoreMeasurement()
 * This function is called by Vp3EleResLGStoreResult() to store the measurement
 * in the device object.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  chanId          - channel identifier.
 *  adcState        - ADC state.
 *
 * Returns:
 *  Measured value.
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static int16
Vp3EleResLGStoreMeasurement(
    Vp890DeviceObjectType *pDevObj,
    uint8 chanId,
    uint8 adcState)
{
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    VpTest3EleResAltResType *p3EleResData = &(pTestHeap->testArgs).resFltAlt;
    int16 *pResult;
    uint8 aToDRoute = VP890_NO_CONNECT;
    int16 result_RG;
    int16 imtRms;

    switch (adcState) {
        case ELE3_RES_LG_IMT_NORMAL_FEED:
            pResult = &pDevObj->testResults.result.resAlt.imnf;
            break;

        case ELE3_RES_LG_ILG_NORMAL_FEED:
            pResult = &pDevObj->testResults.result.resAlt.ilnf;
            break;

        case ELE3_RES_LG_VAL_NORMAL_FEED:
            pResult = &pDevObj->testResults.result.resAlt.vanf;
            break;

        case ELE3_RES_LG_VBL_NORMAL_FEED:
            pResult = &pDevObj->testResults.result.resAlt.vbnf;
            break;

        case ELE3_RES_LG_IMT_ZERO_FEED:
            pResult = &pDevObj->testResults.result.resAlt.imzf;
            break;

        case ELE3_RES_LG_ILG_ZERO_FEED:
            pResult = &pDevObj->testResults.result.resAlt.ilzf;
            break;

        case ELE3_RES_LG_VAL_ZERO_FEED:
            pResult = &pDevObj->testResults.result.resAlt.vazf;
            break;

        case ELE3_RES_LG_VBL_ZERO_FEED:
            pResult = &pDevObj->testResults.result.resAlt.vbzf;
            break;

        case ELE3_RES_LG_IMT_REVERSE_FEED:
            pResult = &pDevObj->testResults.result.resAlt.imrf;
            break;

        case ELE3_RES_LG_ILG_REVERSE_FEED:
            pResult = &pDevObj->testResults.result.resAlt.ilrf;
            break;

        case ELE3_RES_LG_VAL_REVERSE_FEED:
            pResult = &pDevObj->testResults.result.resAlt.varf;
            break;

        case ELE3_RES_LG_VBL_REVERSE_FEED:
            pResult = &pDevObj->testResults.result.resAlt.vbrf;
            break;

        case ELE3_RES_LG_VAB_NORMAL_FEED_RG:
        case ELE3_RES_LG_VAB_ZERO_FEED_RG:
        case ELE3_RES_LG_VAB_REVERSE_FEED_RG:
            pResult = &pTestHeap->vabComputed;
            aToDRoute = VP890_METALLIC_DC_V;
            break;

        default: /* Others ringing states */
            pResult = &result_RG;
            break;
    }

    Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId, aToDRoute, pResult);

    if (adcState == ELE3_RES_LG_IMT_NORMAL_FEED) {
        Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_RMS, FALSE, FALSE, chanId,
            VP890_NO_CONNECT, &imtRms);
        if ((pTestHeap->speedupTime == 5) && (imtRms > IMT_RMS_NF_THRESHOLD)) {
            /* If the RMS metalic current in low gain is still high at the 6th samples, */
            /* then there is a foreign voltage on the line */
            p3EleResData->integrateTime = 100;
        }
    }

    return *pResult;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGCheckSettling()
 * This function is called by Vp3EleResLGStoreResult() to check if the current
 * measurement has settled. Only the adcState in the case statement will be
 * checked.
 *
 * Parameters:
 *  pTestHeap       - pointer to the test heap.
 *  currentValue    - current measured value.
 *  adcState        - ADC state.
 *
 * Returns:
 *  TRUE: the measurement needs more time to settle.
 *  FALSE: the measurement has settled.
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
Vp3EleResLGCheckSettling(
    Vp890TestHeapType *pTestHeap,
    int16 currentValue,
    uint8 adcState)
{
    int16 *previousValue = &pTestHeap->previousAvg;
    uint16 *settlingCnt = &pTestHeap->speedupTime;

    switch (adcState) {
        case ELE3_RES_LG_IMT_NORMAL_FEED_RG:
        case ELE3_RES_LG_IMT_ZERO_FEED_RG:
        case ELE3_RES_LG_IMT_REVERSE_FEED_RG:
            if (ABS(currentValue - *previousValue) > SETTLING_ERROR_THRESHOLD_RG) {
                /* imt is not stable, need more time to settle */
                *previousValue = currentValue;
                (*settlingCnt)++;
                return TRUE;
            }
            *previousValue = 0;
            break;

        case ELE3_RES_LG_IMT_NORMAL_FEED:
        case ELE3_RES_LG_IMT_ZERO_FEED:
        case ELE3_RES_LG_IMT_REVERSE_FEED:
        case ELE3_RES_LG_ILG_NORMAL_FEED:
        case ELE3_RES_LG_ILG_ZERO_FEED:
        case ELE3_RES_LG_ILG_REVERSE_FEED:
        case ELE3_RES_LG_VAL_NORMAL_FEED:
        case ELE3_RES_LG_VAL_ZERO_FEED:
        case ELE3_RES_LG_VAL_REVERSE_FEED:
            if (ABS(currentValue - *previousValue) > SETTLING_ERROR_THRESHOLD) {
                /* Measurement is not stable, need more time to settle */
                *previousValue = currentValue;
                (*settlingCnt)++;
                return TRUE;
            } else if ((currentValue == VP_INT16_MIN) || (currentValue == VP_INT16_MAX)) {
                /* Measurement saturates, need some time to get out of saturation */
                (*settlingCnt)++;
                return TRUE;
            }
           *previousValue = 0;
            break;

        default:
            break;
    }

    /* A settling fast enough means no large REN between tip & ring. In this case it's useless */
    /* to switch back and force between low gain and ringing. So we stay in low gain. */
    /* Force this case for the master socket test */
    if ((adcState == ELE3_RES_LG_IMT_NORMAL_FEED_RG) &&
        ((*settlingCnt < LG_TEST_THRESHOLD) ||
         (pTestHeap->testArgs.resFltAlt.shiftZeroFeed == FALSE))) {
        pTestHeap->lowGain = TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------
 * Vp3EleResLGCheckHighGain()
 * This function is called by Vp3EleResLGGetResult() to check the metallic current
 * in both normal and reverse feed. If the current exceeds a threshold then the
 * low gain primitive is aborted to run the high gain one quickly.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pTestState      - pointer to the ADC failure counter.
 *  adcState        - ADC state.
 *
 * Returns:
 *  TRUE: need to abort this primitive and go to ELE3_RES_LG_RESTORE.
 *  FALSE: continue to the next step.
 *
 * Result:
 *
 *----------------------------------------------------------------------------*/
static bool
Vp3EleResLGCheckHighGain(
    VpLineCtxType *pLineCtx,
    int16 *pTestState,
    uint8 adcState)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    int16 *imnf = &pDevObj->testResults.result.resAlt.imnf;
    int16 *imzf = &pDevObj->testResults.result.resAlt.imzf;

    /* Check for the metallic current in normal feed */
    if ((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
        (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
        (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) {
        if ((adcState == ELE3_RES_LG_IMT_NORMAL_FEED) && (ABS(*imnf) > IMT_NF_LP_THRESHOLD)) {
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResLGCheckHighGain(imnf:%i) > %i", 
                ABS(*imnf), IMT_NF_LP_THRESHOLD));
            *imnf = VP_INT16_MAX;
            *pTestState = ELE3_RES_LG_RESTORE;
            return TRUE;
        }
    } else {
        if ((adcState == ELE3_RES_LG_IMT_NORMAL_FEED) && (ABS(*imnf) > IMT_NF_THRESHOLD)) {
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResLGCheckHighGain(imnf:%i) > %i", 
                ABS(*imnf), IMT_NF_THRESHOLD));
            *imnf = VP_INT16_MAX;
            *pTestState = ELE3_RES_LG_RESTORE;
            return TRUE;
        }
    }

    /* Check for the metallic current in zero feed */
    if ((adcState == ELE3_RES_LG_IMT_ZERO_FEED) && (ABS(*imzf) > IMT_ZF_THRESHOLD)) {
        VP_TEST(VpLineCtxType, pLineCtx, ("Vp3EleResLGCheckHighGain(imzf:%i) > %i", 
            ABS(*imzf), IMT_ZF_THRESHOLD));
        *imzf = VP_INT16_MAX;
        *pTestState = ELE3_RES_LG_RESTORE;
        return TRUE;
    }

    return FALSE;
}

#endif /* VP890_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_890_SERIES */
