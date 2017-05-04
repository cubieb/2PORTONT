/** file vp_dc_rloop.c
 *
 *  This file contains the dc rloop primitive algorithm
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2010-03-19 16:51:13 -0500 (Fri, 19 Mar 2010) $
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

typedef enum
{
    HYBRID_LOSS_START              = VP890_TESTLINE_GLB_STRT_STATE,
    HYBRID_LOSS_END                = VP890_TESTLINE_GLB_END_STATE,
    HYBRID_LOSS_QUIT               = VP890_TESTLINE_GLB_QUIT_STATE,
    HYBRID_LOSS_ENUM_SIZE          = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
}  VpHybridLossStateTypes;

static void
HybridLossSetup(
    VpDeviceIdType  deviceId,
    uint8           ecVal);

/*------------------------------------------------------------------------------
 * Vp890TestHybridLoss
 * This functions
 *
 * Parameters:
 *  pLineCtx     -
 *  pArgsUntyped -
 *  handle       -
 *  callback     -
 *  testId       -
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp890TestHybridLoss(
    VpLineCtxType           *pLineCtx,
    const void              *pArgsUntyped,
    uint16                  handle,
    bool                    callback,
    VpTestIdType            testId)
{
    VpDevCtxType            *pDevCtx    = pLineCtx->pDevCtx;
    Vp890DeviceObjectType   *pDevObj    = pDevCtx->pDevObj;
    Vp890LineObjectType     *pLineObj   = pLineCtx->pLineObj;
    Vp890CurrentTestType    *pTestInfo  = &pDevObj->currentTest;
    VpTestStatusType        *pErrorCode = &pDevObj->testResults.errorCode;

    int16                   *pTestState = &pTestInfo->testState;
    VpDeviceIdType          deviceId    = pDevObj->deviceId;
    uint8                   ecVal       = pLineObj->ecVal;
    uint16                  tick        =  pDevObj->devProfileData.tickRate;
    uint8                   convCfg     = VP890_ENH_B_FILTER_AVG_DET;
    uint8                   result[VP890_TX_PCM_DATA_LEN];

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestHybridLoss(ts:%i) time: %d", *pTestState, pDevObj->timeStamp));

    if(callback == FALSE){
        HybridLossSetup(deviceId, ecVal);
        pLineObj->opCond[0] = (VP890_TXPATH_DIS | VP890_NOISE_GEN_EN);
        pDevObj->currentTest.testId = testId;
        pDevObj->currentTest.testState = HYBRID_LOSS_START;
        pDevObj->currentTest.handle = handle;
        pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] =  1 | VP_ACTIVATE_TIMER;
        return VP_STATUS_SUCCESS;
    }

    switch (*pTestState) {
        case HYBRID_LOSS_START:
            VpMpiCmdWrapper(deviceId, ecVal, VP890_CONV_CFG_WRT, VP890_CONV_CFG_LEN, &convCfg);
            Vp890SetTestStateAndTimer(pDevObj, pTestState, HYBRID_LOSS_END,
                MS_TO_TICKS_ROUND_UP(35, tick) );
            break;

        case HYBRID_LOSS_END:
            /* After 32ms, the result will be in the Transmit PCM/Test Data
               register */
            VpMpiCmdWrapper(deviceId, ecVal, VP890_TX_PCM_DATA_RD,
                VP890_TX_PCM_DATA_LEN, result);
            pDevObj->testResults.result.vrms = ( result[0] << 8 ) | result[1];

            /* The test has completed with no issues and generates an event. */
            *pErrorCode = VP_TEST_STATUS_SUCCESS;
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            break;

        case HYBRID_LOSS_QUIT:
            /* The test has ended early due to the current errorCode */
            Vp890GenerateTestEvent(pLineCtx, testId, handle, TRUE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestHybridLoss(ec:%i): quit",
                 *pErrorCode));
            break;

        default:
            *pErrorCode = VP_TEST_STATUS_INTERNAL_ERROR;
            VP_ERROR(VpLineCtxType, pLineCtx, ("State VP_TEST_STATUS_INTERNAL_ERROR, invalid teststate"));
            break;

    }
    return VP_STATUS_SUCCESS;
} /* Vp890TestHybridLoss() */

/*******************************************************************************
 * HybridLossSetup()
 * This function sets up the device as needed to perform the test
 *
 * Arguments:
 *
 ******************************************************************************/
static void
HybridLossSetup(
    VpDeviceIdType  deviceId,
    uint8           ecVal)
{
    uint8 opFunctions = VP890_ENABLE_LOADED_COEFFICIENTS | VP890_LINEAR_CODEC;
    uint8 opCond = VP890_TXPATH_DIS | VP890_NOISE_GEN_EN;
    uint8 opMode = VP890_EC_CH2;
    uint8 mpiBuffer[3 + VP890_OP_FUNC_LEN + VP890_OP_COND_LEN + VP890_OP_MODE_LEN];
    uint8 mpiIndex = 0;

    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
        VP890_OP_FUNC_LEN, &opFunctions);
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
        VP890_OP_COND_LEN, &opCond);
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_MODE_WRT,
        VP890_OP_MODE_LEN, &opMode);

    VpMpiCmdWrapper(deviceId, ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

} /* HybridLossSetup() */

#endif
#endif
