/** file vp_cross_connect.c
 *
 *  This file contains the cross connect detection primitive algorithm
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 6732 $
 * $LastChangedDate: 2010-03-22 19:09:05 -0500 (Mon, 22 Mar 2010) $
 */


#include "vp_api_cfg.h"

#if defined(VP_CC_890_SERIES) && defined(VP890_INCLUDE_TESTLINE_CODE)
#if (VP890_INCLUDE_LINE_TEST_PACKAGE == VP890_LINE_TEST_PROFESSIONAL)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp890_api.h"
#include "vp890_api_int.h"
#include "vp890_testline_int.h"

#define MAX_ITERATION                   10      /* Max settling loop iterations per measurements */

typedef enum
{
    XCONNECT_SETUP          = VP890_TESTLINE_GLB_STRT_STATE,
    XCONNECT_STATE_SETUP    = 15,
    XCONNECT_COLLECT_DATA   = 20,
    XCONNECT_GET_RESULT     = 25,
    XCONNECT_END            = VP890_TESTLINE_GLB_END_STATE,
    XCONNECT_QUIT           = VP890_TESTLINE_GLB_QUIT_STATE,
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
    Vp890DeviceObjectType *pDevObj,
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId);

static bool
VpXConnectSetState(
    VpLineCtxType *pLineCtx,
    VpTestXConnectType *pXConnectData,
    VpDeviceIdType deviceId);

/*------------------------------------------------------------------------------
 * Vp890TestGetXConnect()
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
Vp890TestGetXConnect(
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
    VpTestXConnectType *pXConnectData = &(pTestHeap->testArgs).xConnect;

    int16 *pTestState = &pTestInfo->testState;
    uint8 *pAdcState = &pTestHeap->adcState;
    uint8 chanId = pLineObj->channelId;
    uint16 tick =  pDevObj->devProfileData.tickRate;
    uint8 aToDRoute = VP890_NO_CONNECT;
    uint8 *loopCnt = &pTestHeap->loopCnt;

    if (FALSE == callback) {
        return VpXConnectArgCheck(pLineCtx, pArgsUntyped, handle, pAdcState, testId);
    }

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestGetXConnect(ts:%i)", *pTestState));
    switch (*pTestState) {

        case XCONNECT_SETUP:
            /* The initialization is only required  for the first phase */
            if (pXConnectData->phase == 0) {
                VpXConnectSetup(pDevObj, pLineObj, pTestHeap, deviceId);
                Vp890SetTestStateAndTimer(pDevObj, pTestState, XCONNECT_STATE_SETUP,
                    MS_TO_TICKRATE(VP890_STATE_CHANGE_SETTLING_TIME, tick));
                break;
            }

        case XCONNECT_STATE_SETUP:
            if (VpXConnectSetState(pLineCtx, pXConnectData, deviceId) == TRUE) {
                *pTestState = XCONNECT_COLLECT_DATA;
            } else {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                *pTestState = XCONNECT_QUIT;
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestGetXConnect(): ADC issue"));
            }

            /* Reset the ADC settling counter */
            *loopCnt = 0;
            Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState,
                MS_TO_TICKRATE(VP890_ADC_CHANGE_SETTLING_TIME, tick));
            break;

        case XCONNECT_COLLECT_DATA:
            Vp890StartPcmCollect(pLineCtx, VP_PCM_OPERATION_AVERAGE, XCONNECT_GET_RESULT,
                pXConnectData->settlingTime,pXConnectData->integrateTime);
            break;

        case XCONNECT_GET_RESULT: {
            uint8 adcConf;

            /* If the adc setup is not set as expected, set it and measure again */
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD, VP890_CONV_CFG_LEN, &adcConf);

            switch(pXConnectData->measReq) {
                case VP_XCONNECT_DISC_V:
                    if ((adcConf & 0x0F) != VP890_METALLIC_DC_V) {
                        Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_V, FALSE);
                        (*loopCnt)++;
                        *pTestState = XCONNECT_COLLECT_DATA;
                    } else {
                        *pTestState = XCONNECT_END;
                    }
                    aToDRoute = VP890_METALLIC_DC_V;
                    break;

                case VP_XCONNECT_12VVOC_I:
                    if ((adcConf & 0x0F) != VP890_METALLIC_DC_I) {
                        Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_I, FALSE);
                        (*loopCnt)++;
                        *pTestState = XCONNECT_COLLECT_DATA;
                    } else {
                        *pTestState = XCONNECT_END;
                    }
                    aToDRoute = VP890_METALLIC_DC_I;
                    break;

                default:
                    VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestGetXConnect(): Unknown test input"));
                    *pTestState = XCONNECT_QUIT;
                    break;
            }

            /* Prevent an infinite loop in case of ADC failure */
            if (*loopCnt >= MAX_ITERATION) {
                *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
                *pTestState = XCONNECT_QUIT;
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestGetXConnect(): The ADC can't settle"));
            }

            Vp890GetPcmResult(pDevObj, VP_PCM_OPERATION_AVERAGE, FALSE, TRUE, chanId, aToDRoute,
                &pDevObj->testResults.result.xConnect.data);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK);
            break;
        }

        case XCONNECT_END:
            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestGetXConnect(): complete"));
            break;

        case XCONNECT_QUIT:
            /* The test has eneded early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestGetXConnect(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestGetXConnect(): invalid teststate"));
            break;
    }
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpXConnectArgCheck()
 * This functions is called by the Vp890TestGetXConnect() function if
 * Vp890TestGetXConnect() was not called via a callback.
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
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &(pDevObj->currentTest);
    VpTestXConnectType *pTestInput = (VpTestXConnectType*)pArgsUntyped;
    VpTestXConnectType *pXConnectData = &(pTestInfo->pTestHeap->testArgs).xConnect;
    int16 *pTestState = &pTestInfo->testState;

    if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
        /* make sure that starting test did not generate any errorCodes */
        Vp890GenerateTestEvent(pLineCtx, testId, handle, FALSE, FALSE);
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
        Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, TRUE);
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
    Vp890SetTestStateAndTimer(pDevObj, pTestState, XCONNECT_SETUP, NEXT_TICK);
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpXConnectSetup()
 * This function is called by the Vp890TestGetXConnect() state machine during
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
    Vp890DeviceObjectType *pDevObj,
    Vp890LineObjectType *pLineObj,
    Vp890TestHeapType *pTestHeap,
    VpDeviceIdType deviceId)
{
    uint8 disn = 0x00;
    uint8 opCond = ( VP890_RXPATH_DIS | VP890_HIGH_PASS_DIS );
    uint8 opFunction = VP890_LINEAR_CODEC;
    uint8 adc;
    uint8 ssCfg = ( VP890_ZXR_DIS | VP890_AUTO_SSC_DIS);
    uint8 dcfeed[VP890_DC_FEED_LEN] = {0x62, 0x02};
    uint8 switchReg[VP890_REGULATOR_PARAM_LEN];

    uint8 mpiBuffer[VP890_CONV_CFG_LEN+1 +
                     VP890_SLIC_STATE_LEN+1 +
                     VP890_DISN_LEN+1 +
                     VP890_OP_COND_LEN+1 +
                     VP890_OP_FUNC_LEN+1 +
                     VP890_ICR2_LEN+1 +
                     VP890_ICR3_LEN+1 +
                     VP890_ICR4_LEN+1 +
                     VP890_SS_CONFIG_LEN+1 +
                     VP890_REGULATOR_PARAM_LEN+1 +
                     VP890_DC_FEED_LEN+1];
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
    pLineObj->slicValueCache |= VP890_SS_ACTIVATE_MASK;

    /* Set the floor voltage to -60V */
    VpMemCpy(switchReg, pTestHeap->switchReg, VP890_REGULATOR_PARAM_LEN);
    switchReg[1] &= 0xE0;
    switchReg[1] |= 0x0B;

    /* Get the adc reg setting */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD, VP890_CONV_CFG_LEN, &adc);
    adc &= ~VP890_CONV_CONNECT_BITS;
    adc |= VP890_NO_CONNECT;


    /*
     * FORCE ADC to metalic AC:
     * THIS is a required workaround. If the device is in idle and
     * the ADC is forced on in ICR4, the data from the ADC will be
     * railed. However, if the converter configuration register is
     * set to NO_CONNECT first, then this issue does not exist.
    */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_CONV_CFG_WRT,
        VP890_CONV_CFG_LEN, &adc);


    /* Remove disn */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DISN_WRT,
        VP890_DISN_LEN, &disn);

    /* Disable hi-pass filter and cutoff receive path */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
        VP890_OP_COND_LEN, &opCond);
    pLineObj->opCond[0] = opCond;

    /* Set to read linear data */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
        VP890_OP_FUNC_LEN, &opFunction);

    /* VOC on, A/B sns on, incr ADC rg */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
        VP890_ICR2_LEN, pLineObj->icr2Values);

    /* Force line control circuits on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
        VP890_ICR3_LEN, pLineObj->icr3Values);

    /* Force ADC on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
        VP890_ICR4_LEN, pLineObj->icr4Values);

    /* Prevent ADC from changing state setup during testing */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SS_CONFIG_WRT,
        VP890_SS_CONFIG_LEN, &ssCfg);

    /* State Change */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SLIC_STATE_WRT,
        VP890_SLIC_STATE_LEN, &pLineObj->slicValueCache);

    /* Set the floor voltage to -60V */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_REGULATOR_PARAM_WRT,
        VP890_REGULATOR_PARAM_LEN, switchReg);

    /* VOC = 12V / ILA = 20mA*/
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DC_FEED_WRT,
        VP890_DC_FEED_LEN, dcfeed);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * VpXConnectSetState()
 * This function is called by the Vp890TestGetXConnect() state machine during
 * the XCONNECT_STATE_SETUP state.
 *
 * This function is used to set the requested line state and call the Vp890AdcSetup().
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
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    bool adcFlag = FALSE;

    switch(pXConnectData->measReq) {
        case VP_XCONNECT_DISC_V:
            pLineObj->slicValueCache = VP890_SS_DISCONNECT | VP890_SS_ACTIVATE_MASK;
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SLIC_STATE_WRT, VP890_SLIC_STATE_LEN,
                &pLineObj->slicValueCache);
            adcFlag = Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_V, FALSE);
            break;

        case VP_XCONNECT_12VVOC_I:
            if (pXConnectData->isFeedPositive == TRUE) {
                pLineObj->slicValueCache = VP890_SS_ACTIVE;
            } else {
                pLineObj->slicValueCache = VP890_SS_ACTIVE_POLREV;
            }
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SLIC_STATE_WRT, VP890_SLIC_STATE_LEN,
                &pLineObj->slicValueCache);

            adcFlag = Vp890AdcSetup(pLineCtx, VP890_METALLIC_DC_I, FALSE);
            break;

        default:
            VP_ERROR(VpLineCtxType, pLineCtx, ("VpXConnectSetState(): invalid teststate"));
            break;
    }

    return adcFlag;
}

#endif /* VP890_INCLUDE_TESTLINE_CODE */

#endif /* VP_CC_890_SERIES */
