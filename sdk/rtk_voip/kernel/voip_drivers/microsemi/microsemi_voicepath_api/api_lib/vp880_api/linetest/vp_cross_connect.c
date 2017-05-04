/** file vp_cross_connect.c
 *
 *  This file contains the cross connect detection primitive algorithm
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 10000 $
 * $LastChangedDate: 2012-05-14 11:39:28 -0500 (Mon, 14 May 2012) $
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

#define MAX_ITERATION                   10      /* Max settling loop iterations per measurements */

typedef enum
{
    XCONNECT_SETUP          = VP880_TESTLINE_GLB_STRT_STATE,
    XCONNECT_STATE_SETUP    = 15,
    XCONNECT_COLLECT_DATA   = 20,
    XCONNECT_GET_RESULT     = 25,
    XCONNECT_END            = VP880_TESTLINE_GLB_END_STATE,
    XCONNECT_QUIT           = VP880_TESTLINE_GLB_QUIT_STATE,
    XCONNECT_ENUM_SIZE      = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  vpOpenVdcTestStateTypes;

static VpStatusType
VpXConnectArgCheck(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    uint8 *pAdcState,
    VpTestIdType testId);

static void
VpXConnectSetup(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static bool
VpXConnectSetState(
    VpLineCtxType *pLineCtx,
    VpTestXConnectType *pXConnectData,
    VpDeviceIdType deviceId);

/*------------------------------------------------------------------------------
 * Vp880TestGetXConnect()
 * This functions implements the LOOP CONDITIONS Test primitive ...
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  pArgsUntyped- pointer to the test specific inputs.
 *  handle      - unique test handle
 *  callbak     - indicates if this function was called via a callback
 *  testId      - test identifier
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp880TestGetXConnect(
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
    VpTestXConnectType *pXConnectData = &(pTestHeap->testArgs).xConnect;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint8 aToDRoute = VP880_NO_CONNECT;
    uint8 *loopCnt = &pTestHeap->loopCnt;

    if (FALSE == callback) {
        return VpXConnectArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestGetXConnect(ts:%i)", *pTestState));
    switch (*pTestState) {

        case XCONNECT_SETUP:
            /* The initialization is only required  for the first phase */
            if (pXConnectData->phase == 0) {
                VpXConnectSetup(pDevObj, pLineObj, pTestHeap, deviceId);
                VpSetTestStateAndTimer(pDevObj, pTestState, XCONNECT_STATE_SETUP,
                    MS_TO_TICKRATE(VP880_STATE_CHANGE_SETTLING_TIME, tick));
                break;
            }

        case XCONNECT_STATE_SETUP:
            if (VpXConnectSetState(pLineCtx, pXConnectData, deviceId) == TRUE) {
                *pTestState = XCONNECT_COLLECT_DATA;
            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                *pTestState = XCONNECT_QUIT;
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestGetXConnect(): ADC issue"));
            }

            /* Reset the ADC settling counter */
            *loopCnt = 0;
            VpSetTestStateAndTimer(pDevObj, pTestState, *pTestState,
                MS_TO_TICKRATE(VP880_ADC_CHANGE_SETTLING_TIME, tick));
            break;

        case XCONNECT_COLLECT_DATA:
            VpStartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE, XCONNECT_GET_RESULT,
                pXConnectData->settlingTime,pXConnectData->integrateTime);
            break;

        case XCONNECT_GET_RESULT: {
            uint8 adcConf;

            /* If the adc setup is not set as expected, set it and measure again */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN, &adcConf);

            switch(pXConnectData->measReq) {
                case VP_XCONNECT_DISC_V:
                    if ((adcConf & 0x0F) != VP880_METALLIC_DC_V) {
                        Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_V, FALSE);
                        (*loopCnt)++;
                        *pTestState = XCONNECT_COLLECT_DATA;
                    } else {
                        *pTestState = XCONNECT_END;
                    }
                    aToDRoute = VP880_METALLIC_DC_V;
                    break;

                case VP_XCONNECT_12VVOC_I:
                    if ((adcConf & 0x0F) != VP880_METALLIC_DC_I) {
                        Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_I, FALSE);
                        (*loopCnt)++;
                        *pTestState = XCONNECT_COLLECT_DATA;
                    } else {
                        *pTestState = XCONNECT_END;
                    }
                    aToDRoute = VP880_METALLIC_DC_I;
                    break;

                default:
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestGetXConnect(): Unknown test input"));
                    *pTestState = XCONNECT_QUIT;
                    break;
            }

            /* Prevent an infinite loop in case of ADC failure */
            if (*loopCnt >= MAX_ITERATION) {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                *pTestState = XCONNECT_QUIT;
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestGetXConnect(): The ADC can't settle"));
            }

            VpGetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId, aToDRoute,
                &pDevObj->testResults.result.xConnect.data);
            VpSetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;
        }

        case XCONNECT_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestGetXConnect(): complete"));
            break;

        case XCONNECT_QUIT:
            /* The test has eneded early due to the current errorCode */
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestGetXConnect(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestGetXConnect(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpXConnectArgCheck()
 * This functions is called by the Vp880TestGetXConnect() function if
 * Vp880TestGetXConnect() was not called via a callback.
 *
 * This function will check that no errorCode is pending and that the input
 * arguments for the Get Loop Conditions primitive are legal. The function also
 * sets up the primitives state machine for the first state and initializes
 * the results from this fucntion to 0.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  pArgsUntyped    - pointer to the struct which contains test specfic info.
 *  handle          - unique test handle
 *  pAdcState       - pointer to the value of the current ADC measurement state
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
VpStatusType
VpXConnectArgCheck(
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
    VpTestXConnectType *pTestInput = (VpTestXConnectType*)pArgsUntyped;
    VpTestXConnectType *pXConnectData = &(pTestInfo->pTestHeap->testArgs).xConnect;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        VpGenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpXConnectArgCheck(ec:%i): bailed",
            pDevObj->testResults.errorCode));
        return VP_STATUS_SUCCESS;
    }

    /*
     * Setup current Test members and test pTestInfo data.
     * This portion is called only one time once the primitive is started.
     */

    pDevObj->testResults.result.xConnect.data = VP_INT16_MAX;
    pDevObj->testResults.result.xConnect.condition = pTestInput->measReq;

    pTestInfo->testId = testId;
    pTestInfo->handle = handle;

    /* This test cannot run if the isolate relay is open */
    if (((pLineObj->termType == VP_TERM_FXS_ISOLATE) ||
         (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP)) &&
        (VP_RELAY_RESET == pLineObj->relayState)) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
        VpGenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpXConnectArgCheck(): bailed openRelay"));
        return VP_STATUS_SUCCESS;
    }

    /* Check input arguments before assigining them to test struct */
    if ((pTestInput->measReq != VP_XCONNECT_DISC_V) &&
        (pTestInput->measReq != VP_XCONNECT_12VVOC_I)) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpXConnectArgCheck(): bailed invalid request"));
        return VP_STATUS_INVALID_ARG;
    }

    pXConnectData->integrateTime = pTestInput->integrateTime;
    pXConnectData->settlingTime = pTestInput->settlingTime;
    pXConnectData->measReq = pTestInput->measReq;
    pXConnectData->isFeedPositive = pTestInput->isFeedPositive;
    pXConnectData->phase = pTestInput->phase;

    /*
     * force the function to start running state maching on next tick only if
     * all arguments checkout ok and no events were generated.
     */
    VpSetTestStateAndTimer(pDevObj, pTestState, XCONNECT_SETUP, NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpXConnectSetup()
 * This function is called by the Vp880TestGetXConnect() state machine during
 * the XCONNECT_SETUP state.
 *
 * This function is used to configure the line for the condition test.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  pLineObj        - pointer to the line object.
 *  pTestHeap       - pointer to the test eap.
 *  deviceId        - device identifier
 *
 * Returns:
 * --
 * Result:
 * --
 *----------------------------------------------------------------------------*/
static void
VpXConnectSetup(
    Vp880DeviceObjectType *pDevObj,
    Vp880LineObjectType *pLineObj,
    Vp880TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    uint8 disn = 0x00;
    uint8 opCond = ( VP880_RXPATH_DIS | VP880_HIGH_PASS_DIS );
    uint8 opFunction = VP880_LINEAR_CODEC;
    uint8 adc;
    uint8 ssCfg = ( VP880_ZXR_DIS | VP880_AUTO_SSC_DIS);
    uint8 dcfeed[VP880_DC_FEED_LEN] = {0x62, 0x02};
    uint8 switchReg[VP880_REGULATOR_PARAM_LEN];

    uint8 mpiBuffer[VP880_CONV_CFG_LEN+1 +
                     VP880_SLIC_STATE_LEN+1 +
                     VP880_DISN_LEN+1 +
                     VP880_OP_COND_LEN+1 +
                     VP880_OP_FUNC_LEN+1 +
                     VP880_ICR2_LEN+1 +
                     VP880_ICR3_LEN+1 +
                     VP880_ICR4_LEN+1 +
                     VP880_SS_CONFIG_LEN+1 +
                     VP880_REGULATOR_PARAM_LEN+1 +
                     VP880_DC_FEED_LEN+1];
    uint8 mpiIndex = 0;

    pLineObj->icr2Values[0] = 0x0C;
    pLineObj->icr2Values[1] = 0x0C;
    if ((pLineObj->lineState.currentState == VP_LINE_DISCONNECT) &&
        ((pLineObj->termType == VP_TERM_FXS_LOW_PWR) ||
        (pLineObj->termType == VP_TERM_FXS_SPLITTER_LP) ||
        (pLineObj->termType == VP_TERM_FXS_ISOLATE_LP))) {
        pLineObj->icr2Values[2] &= 0x0C;
        pLineObj->icr2Values[2] |= 0x20;
        pLineObj->icr2Values[3] &= 0x0C;
    } else {
        pLineObj->icr2Values[2] &= 0x0C;
        pLineObj->icr2Values[3] &= 0x0C;
    }

    pLineObj->icr3Values[0] |= 0x21;
    pLineObj->icr3Values[1] |= 0x21;

    pLineObj->icr4Values[0] |= 0x01;
    pLineObj->icr4Values[1] |= 0x01;

    /* Get Slac State */
    pLineObj->slicValueCache |= VP880_SS_ACTIVATE_MASK;

    /* Set the floor voltage to -60V (tracker only) */
    if ((VP880_IS_ABS & pDevObj->stateInt) == FALSE) {
        VpMemCpy(switchReg, pTestHeap->switchReg, VP880_REGULATOR_PARAM_LEN);
        switchReg[1] &= 0xE0;
        switchReg[1] |= 0x0B;
    }

    /* Get the adc reg setting */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN, &adc);
    adc &= ~VP880_CONV_CONNECT_BITS;
    adc |= VP880_NO_CONNECT;


    /*
     * FORCE ADC to metalic AC:
     * THIS is a required workaround. If the device is in idle and
     * the ADC is forced on in ICR4, the data from the ADC will be
     * railed. However, if the converter configuration register is
     * set to NO_CONNECT first, then this issue does not exist.
    */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_CONV_CFG_WRT,
        VP880_CONV_CFG_LEN, &adc);


    /* Remove disn */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DISN_WRT,
        VP880_DISN_LEN, &disn);

    /* Disable hi-pass filter and cutoff receive path */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_COND_WRT,
        VP880_OP_COND_LEN, &opCond);
    pLineObj->opCond[0] = opCond;

    /* Set to read linear data */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_FUNC_WRT,
        VP880_OP_FUNC_LEN, &opFunction);

    /* VOC on, A/B sns on, incr ADC rg */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
        VP880_ICR2_LEN, pLineObj->icr2Values);

    /* Force line control circuits on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR3_WRT,
        VP880_ICR3_LEN, pLineObj->icr3Values);

    /* Force ADC on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR4_WRT,
        VP880_ICR4_LEN, pLineObj->icr4Values);

    /* Prevent ADC from changing state setup during testing */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SS_CONFIG_WRT,
        VP880_SS_CONFIG_LEN, &ssCfg);

    /* State Change */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SLIC_STATE_WRT,
        VP880_SLIC_STATE_LEN, &pLineObj->slicValueCache);

    /* Set the floor voltage to -60V (tracker only) */
    if ((VP880_IS_ABS & pDevObj->stateInt) == FALSE) {
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_REGULATOR_PARAM_WRT,
            VP880_REGULATOR_PARAM_LEN, switchReg);
    }

    /* VOC = 12V / ILA = 20mA*/
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DC_FEED_WRT,
        VP880_DC_FEED_LEN, dcfeed);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * VpXConnectSetState()
 * This function is called by the Vp880TestGetXConnect() state machine during
 * the XCONNECT_STATE_SETUP state.
 *
 * This function is used to set the requested line state and call the Vp880AdcSetup().
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context.
 *  pXConnectData   - input data from the ltAPI.
 *  deviceId        - device identifier.
 *
 * Returns:
 *  FALSE if something goes wrong during the adc setup.
 *  TRUE if everything goes ok.
 *
 * Result:
 *  The ADC routing is changed
 *----------------------------------------------------------------------------*/
static bool
VpXConnectSetState(
    VpLineCtxType *pLineCtx,
    VpTestXConnectType *pXConnectData,
    VpDeviceIdType deviceId)
{
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    bool adcFlag = FALSE;

    switch(pXConnectData->measReq) {
        case VP_XCONNECT_DISC_V:
            pLineObj->slicValueCache = VP880_SS_DISCONNECT | VP880_SS_ACTIVATE_MASK;
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SLIC_STATE_WRT, VP880_SLIC_STATE_LEN,
                &pLineObj->slicValueCache);
            adcFlag = Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_V, FALSE);
            break;

        case VP_XCONNECT_12VVOC_I:
            if (pXConnectData->isFeedPositive == TRUE) {
                pLineObj->slicValueCache = VP880_SS_ACTIVE;
            } else {
                pLineObj->slicValueCache = VP880_SS_ACTIVE_POLREV;
            }
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SLIC_STATE_WRT, VP880_SLIC_STATE_LEN,
                &pLineObj->slicValueCache);

            adcFlag = Vp880AdcSetup(pLineCtx, VP880_METALLIC_DC_I, FALSE);
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetState(): invalid teststate"));
            break;
    }

    return adcFlag;
}

#endif /* VP880_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_880_SERIES */
