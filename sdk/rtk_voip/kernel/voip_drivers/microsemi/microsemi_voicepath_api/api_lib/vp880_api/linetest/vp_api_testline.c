/** file apitestline.c
 *
 *  This file contains the functions used in the Vp880 Test Line.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 11505 $
 * $LastChangedDate: 2014-07-24 12:38:31 -0500 (Thu, 24 Jul 2014) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_880_SERIES)
#if defined (VP880_INCLUDE_TESTLINE_CODE)

/* Project Includes */
#include "vp_api_types.h"
#include "sys_service.h"
#include "vp_hal.h"
#include "vp_api_int.h"
#include "vp880_api.h"
#include "vp880_api_int.h"
#include "vp_api_testline_int.h"

#ifdef VP880_EZ_MPI_PCM_COLLECT
#include "vp_pcm_compute.h"
#endif

static VpTestStatusType
VpIsLineReadyForTest (
    VpLineCtxType *pLineCtx,
    VpTestIdType test);

static VpStatusType
Vp880TestPrepare(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

static VpStatusType
Vp880TestPrepareExt(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

static void
    Vp880InitTestHeap(
    Vp880TestHeapType *pTestHeap);

/*v-------------------------EXTERN functions----------------------------------v*/

/*------------------------------------------------------------------------------
 * Vp880TestLine()
 *  Initiates a line test.
 * This function will perfrom numerous checks to ensure that the test specified
 * by the testId argument can legally run. Once all error checks have been
 * completed the function relies on the Vp880TestLineInt function to actually
 * figure out which test primative to execute.
 *
 * Refer to the VoicePath API User's Guide for the rest of the
 * details about this function.
 *----------------------------------------------------------------------------*/
VpStatusType
Vp880TestLine (
    VpLineCtxType *pLineCtx,
    VpTestIdType test,
    const void *pArgsUntyped,
    uint16 handle)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    VpTestStatusType errorCode = VP_TEST_STATUS_SUCCESS;
    VpStatusType rtnval = VP_STATUS_SUCCESS;

    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLine(tid:%i)+",test));

    /* Proceed if device state is either in progress or complete */
    if (pDevObj->state & (VP_DEV_INIT_CMP | VP_DEV_INIT_IN_PROGRESS)) {
    } else {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

    /*
     * Do not proceed if the device calibration is in progress. This could
     * damage the device.
     */
    if (pDevObj->state & VP_DEV_IN_CAL) {
        return VP_STATUS_DEV_NOT_INITIALIZED;
    }

#ifdef VP880_EZ_MPI_PCM_COLLECT
    {
        /* if in wideband and tick rate is greater than 6ms then error out*/
        if ((pLineObj->codec == VP_OPTION_WIDEBAND) &&
            (pDevObj->devProfileData.tickRate > 0x600)) {

            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestLine(%i:0x%02X:0x%04X): " \
                "VP_STATUS_FAILURE wideband not supported at this tick rate",
                pLineObj->codec, pLineObj->ecVal, pDevObj->devProfileData.tickRate ));

            return VP_STATUS_FAILURE;
        }
    }
#endif

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    if (!(pDevObj->stateInt & VP880_IS_TEST_CAPABLE)) {
        /* Check that the device is a high voltage device */
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestLine(): VP_STATUS_FUNC_NOT_SUPPORTED"));
        VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLine()-"));
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_FUNC_NOT_SUPPORTED;
    }

    /* Check that the line is not an FXO VTD*/
    if (pLineObj->status & VP880_IS_FXO) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestLine(): VP_STATUS_ERR_VTD_CODE"));
        VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLine()-"));
        VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);
        return VP_STATUS_ERR_VTD_CODE;
    }

    errorCode = VpIsLineReadyForTest(pLineCtx, test);

    pDevObj->testResults.errorCode = errorCode;
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    rtnval = Vp880TestLineInt(pLineCtx, test, pArgsUntyped, handle, FALSE);

    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLine()-"));
    return rtnval;
}

/*------------------------------------------------------------------------------
 * Vp880TestLineInt()
 *
 * This function is basically just a wrapper to each of the test primatives and
 * is called by one of two locations, either the Vp880TestLine function or by
 * the Vp880ServiceTimers function when the VP_DEV_TIMER_TESTLINE expires.
 *
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp880TestLineInt(
    VpLineCtxType *pLineCtx,
    VpTestIdType test,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback)
{
    VpStatusType rtnval;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp880TestHeapType *pTestHeap = pDevObj->currentTest.pTestHeap;

    if ((TRUE == callback) &&
        (test != pDevObj->currentTest.testId)) {
            VpGenerateTestEvent(pLineCtx, test, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestLineInt(%i!=%i): tid mismatch",
                test, pDevObj->currentTest.testId));
            return VP_STATUS_FAILURE;
    }

    switch (test) {
        case VP_TEST_ID_PREPARE: {
            const VpTestPrepareType *pPrepareArgs = (VpTestPrepareType *)pArgsUntyped;

            if (
                (pPrepareArgs == VP_NULL) ||
                (pPrepareArgs->intrusive == TRUE)
            ) {
                pDevObj->currentTest.nonIntrusiveTest = FALSE;
                rtnval = Vp880TestPrepare(pLineCtx, handle, callback, test);
            } else {
                pDevObj->currentTest.nonIntrusiveTest = TRUE;
                rtnval = Vp880TestPrepareExt(pLineCtx, handle, callback, test);
            }
            break;
        }

        case VP_TEST_ID_CONCLUDE:
            rtnval = Vp880TestConclude(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_USE_LINE_CAL:
            pDevObj->calOffsets[pLineObj->channelId].nullOffset = 0;
            if ((pTestHeap->slacState & VP880_SS_POLARITY_MASK) == 0) {
                pDevObj->calOffsets[pLineObj->channelId].vabOffset = \
                  pDevObj->vp880SysCalData.vocOffset[pLineObj->channelId][VP880_NORM_POLARITY] * -1;
            } else {
                pDevObj->calOffsets[pLineObj->channelId].vabOffset = \
                  pDevObj->vp880SysCalData.vocOffset[pLineObj->channelId][VP880_REV_POLARITY] * -1;
            }

            if ((pTestHeap->slacState & VP880_SS_POLARITY_MASK) == 0) {
                pDevObj->calOffsets[pLineObj->channelId].vahOffset = \
                    pDevObj->vp880SysCalData.vagOffsetNorm[pLineObj->channelId];
            } else {
                pDevObj->calOffsets[pLineObj->channelId].vahOffset = \
                    pDevObj->vp880SysCalData.vagOffsetRev[pLineObj->channelId];
            }

            if ((pTestHeap->slacState & VP880_SS_POLARITY_MASK) == 0) {
                pDevObj->calOffsets[pLineObj->channelId].vbhOffset = \
                    pDevObj->vp880SysCalData.vbgOffsetNorm[pLineObj->channelId];
            } else {
                pDevObj->calOffsets[pLineObj->channelId].vbhOffset = \
                    pDevObj->vp880SysCalData.vbgOffsetRev[pLineObj->channelId];
            }

            pDevObj->calOffsets[pLineObj->channelId].valOffset = 0;
            pDevObj->calOffsets[pLineObj->channelId].vblOffset = 0;

            pDevObj->calOffsets[pLineObj->channelId].imtOffset = \
                pDevObj->vp880SysCalData.ilaOffsetNorm[pLineObj->channelId];

            pDevObj->calOffsets[pLineObj->channelId].ilgOffset = \
                pDevObj->vp880SysCalData.ilgOffsetNorm[pLineObj->channelId];

            rtnval = VP_STATUS_SUCCESS;
            break;

        case VP_TEST_ID_OPEN_VXC:
            rtnval = Vp880TestOpenVxc(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_AC_RLOOP:
            rtnval = Vp880TestAcRloop(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_DC_RLOOP:
            rtnval = Vp880TestDcRloop(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_3ELE_RES_LG:
            rtnval = Vp880Test3EleResLG(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_DELAY:
            rtnval = Vp880TestTimer(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

#if (VP880_INCLUDE_LINE_TEST_PACKAGE == VP880_LINE_TEST_PROFESSIONAL)

        case VP_TEST_ID_CALIBRATE:
            rtnval = Vp880TestCalibrate(pLineCtx, handle, callback, test);
            break;

        case VP_TEST_ID_3ELE_RES_HG:
            rtnval = Vp880Test3EleResHG(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_LOOP_CONDITIONS:
            rtnval = Vp880TestGetLoopCond(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_LOOPBACK:
            rtnval = Vp880TestLoopback(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_RAMP_INIT:
            rtnval = Vp880TestInitMetRamp(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_RAMP:
            rtnval = Vp880TestMetRamp(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_FLT_DSCRM:
            rtnval = Vp880TestFltDscrm(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_3ELE_CAP_CSLAC:
            rtnval = Vp880Test3EleCap(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_MSOCKET_TYPE2:
            rtnval = Vp880TestMSocket(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

        case VP_TEST_ID_XCONNECT:
            rtnval = Vp880TestGetXConnect(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;
        case VP_TEST_ID_VP_MPI_CMD: {
            const VpTestVpMpiCmdType *pMpiCmdArgs = (VpTestVpMpiCmdType *)pArgsUntyped;

            if (pMpiCmdArgs == NULL) {
                VP_ERROR(VpLineCtxType, pLineCtx, ("Vp880TestVpMpiCmd NULL input struct"));
                rtnval = VP_STATUS_INVALID_ARG;
            } else {

                VpMemCpy(pDevObj->testResults.result.mpiCmd.buffer, pMpiCmdArgs->buffer,
                    sizeof(pDevObj->testResults.result.mpiCmd.buffer));
                    
                VpMpiCmdWrapper(pDevObj->deviceId, pLineObj->ecVal, pMpiCmdArgs->cmd,
                    pMpiCmdArgs->cmdLen, pDevObj->testResults.result.mpiCmd.buffer);

                pDevObj->testResults.result.mpiCmd.cmd = pMpiCmdArgs->cmd;
                pDevObj->testResults.result.mpiCmd.cmdLen = pMpiCmdArgs->cmdLen;

                VpGenerateTestEvent(pLineCtx, test, handle, TRUE, FALSE);
                VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestVpMpiCmd(): complete"));

                rtnval = VP_STATUS_SUCCESS;
            }
            break;
        }

#endif

        default:
            rtnval = VP_STATUS_INVALID_ARG;
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestLineInt(%i): invalid tid", test));
            break;
    }
    return rtnval;
}

/*------------------------------------------------------------------------------
 * Vp880TestLineCallback()
 *
 * This function is normally called by an outside application after collecting
 * and processing PCM data requested by the previous PcmCollect routine. If
 * the api is operating in EZ mode this function is actaull called from within
 * the api by the VpEzPcmCallback() function.
 *
 * The results structure pointed to by the pResults argument are copied into
 * the TestHeap for later use.
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  pResults    - pointer to results from the pcmCollect system service layer
 *----------------------------------------------------------------------------*/
VpStatusType
Vp880TestLineCallback(
    VpLineCtxType *pLineCtx,
    VpPcmOperationResultsType *pResults)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880TestHeapType *pTestHeap = pDevObj->currentTest.pTestHeap;

    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLineCallback()+"));

    /* If the current test heap is null, there is no running test */
    if (NULL == pTestHeap) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestLineCallback(): no running tests"));
        VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLineCallback()-"));
        return VP_STATUS_FAILURE;
    }

    /*
     * If the PCMCollect routine was unsuccessful change the errorcode
     * to VP_TEST_STATUS_RESOURCE_NA
     */
    if (TRUE == pResults->error) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_INTERNAL_ERROR;
        VpSetTestStateAndTimer(pDevObj, &pDevObj->currentTest.testState,
            pDevObj->currentTest.testState, NEXT_TICK);
        pTestHeap->pcmRequest = FALSE;

        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestLineCallback(): VP_TEST_STATUS_INTERNAL_ERROR"));
        pTestHeap->pcmResults.error = TRUE;
        VpSetTestStateAndTimer(pDevObj, &pDevObj->currentTest.testState,
            VP880_TESTLINE_GLB_QUIT_STATE, NEXT_TICK);

        VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLineCallback()-"));
        return VP_STATUS_FAILURE;
    }
    /*
     * If the code gets here, hypotheticaly the current pcm data is for the
     * currently running test on the current line so save off the results
     * and set up the next test state.
     */
    VpMemCpy(&pTestHeap->pcmResults, pResults, (uint16)sizeof(VpPcmOperationResultsType));

    VpSetTestStateAndTimer(pDevObj, &pDevObj->currentTest.testState,
        pTestHeap->nextState, NEXT_TICK);
    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp880TestLineCallback()-"));
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * VpSetTestStateAndTimer
 * This function is sets up the test timer for the given testTime (ticks).
 * This function also changes the value of pState to testState. Almost
 * all of the Vp880 test primitives call this function at least once.
 *
 * Parameters:
 *  pDevObj     - pointer to the device object
 *  pState      - pointer to the current test state
 *  testState   - value of the next state once the timer expires
 *  testTime    - how long (in ticks) until the timer expires.
 *
 * Returns:
 *  --
 *----------------------------------------------------------------------------*/
EXTERN void
VpSetTestStateAndTimer(
    Vp880DeviceObjectType *pDevObj,
    int16 *pState,
    int16 testState,
    int16 testTime)
{
    *pState = testState;
    pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] = testTime | VP_ACTIVATE_TIMER;
    return;
}

/*------------------------------------------------------------------------------
 * VpGenerateTestEvent()
 *
 * This function is called by any test primative that needs to generate
 * an event.
 *
 *
 * pDevCtx:
 * pLineCtx - pointer to the device context
 * test     - test Id of the test being requested to run.
 * handle   - unique test handle
 * cleanUp  - flag indicating if the function should reset the test specific
 *            deviceObj->currentTest test members.
 *
 * Returns:
 *----------------------------------------------------------------------------*/
EXTERN void
VpGenerateTestEvent(
    VpLineCtxType *pLineCtx,
    VpTestIdType testId,
    uint16 handle,
    bool cleanUp,
    bool goToDisconnect)
{
    Vp880LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = pTestInfo->pTestHeap;

    pDevObj->testResults.testId = testId;
    pLineObj->lineEventHandle = handle;
    pLineObj->lineEvents.test |= VP_LINE_EVID_TEST_CMP;

    VP_TEST(VpLineCtxType, pLineCtx, ("VpGenerateTestEvent(tid:%i,ec:%i,handle:%i)",
        pDevObj->testResults.testId, pDevObj->testResults.errorCode,
        pLineObj->lineEventHandle));

    /*
     * if a test needs to generate an event but does not want to disturb
     * a test already in progress, then these steps are skipped
     */
    if (TRUE == cleanUp) {
        VpDeviceIdType deviceId = pDevObj->deviceId;
        uint8 slacState = VP880_SS_DISCONNECT;

        VP_TEST(VpLineCtxType, pLineCtx, ("VpGenerateTestEvent(): cleanup"));

        Vp880InitTestHeap(pTestHeap);

        pDevObj->currentTest.testState = -1;
        pDevObj->currentTest.testId = VP_NUM_TEST_IDS;

        /* make sure that the device is in a non lethal feed state if requested */
        if (TRUE == goToDisconnect) {
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SYS_STATE_WRT,
                VP880_SYS_STATE_LEN, &slacState);
            pLineObj->slicValueCache = slacState;
        }
    }
    return;
}

/*------------------------------------------------------------------------------
 * Vp880CommonTestSetup
 * This functions puts the current channel under test into a known state in order
 * to accomplish any of the test Primatives. Excluding Vp880TestPrepare and
 * Vp880TestConclude, this function should be run before attempting to take a
 * measurment using the ADC. Once the channel is setup the function will advance
 * the currently running primative to the next state and then set up the device
 * timer to expire once the feed has collapsed.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  nextTestState   - what test state to move to once the setup is complete
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN void
Vp880CommonTestSetup(
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId)
{
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    uint8 adcDefault;
    uint8 sigGenAB[VP880_SIGA_PARAMS_LEN]
          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 sigGenCtrl = VP880_GEN_ALLOFF;
    uint8 opCond = (VP880_RXPATH_DIS | VP880_HIGH_PASS_DIS);
    uint8 opFunction = VP880_LINEAR_CODEC;
    uint8 disn = 0x00;
    uint8 gain = VP880_DEFAULT_VP_GAIN;

    uint8 ssCfg = (VP880_ACFS_MASK | VP880_AUTO_SSC_DIS);

    uint8 mpiBuffer[VP880_CONV_CFG_LEN+1 +
                     VP880_SIGA_PARAMS_LEN+1 +
                     VP880_GEN_CTRL_LEN+1 +
                     VP880_OP_COND_LEN+1 +
                     VP880_OP_FUNC_LEN+1 +
                     VP880_VP_GAIN_LEN+1 +
                     VP880_DISN_LEN+1 +
                     VP880_SS_CONFIG_LEN+1 +
                     VP880_ICR1_LEN+1 +
                     VP880_ICR2_LEN+1 +
                     VP880_ICR3_LEN+1 +
                     VP880_ICR4_LEN+1];
    uint8 mpiIndex = 0;

    pLineObj->icr1Values[0] = 0x00;
    pLineObj->icr1Values[1] = 0x00;
    pLineObj->icr1Values[2] &= 0xC0;
    pLineObj->icr1Values[3] &= 0xC0;

    pLineObj->icr2Values[0] = 0x0C;
    pLineObj->icr2Values[1] = 0x0C;
    pLineObj->icr2Values[2] &= 0x0C;
    pLineObj->icr2Values[2] |= 0x01;
    pLineObj->icr2Values[3] &= 0x0C;
    pLineObj->icr2Values[3] |= 0x01;

    pLineObj->icr3Values[0] = 0x00;
    pLineObj->icr3Values[1] = 0x00;
    pLineObj->icr3Values[2] = 0x00;
    pLineObj->icr3Values[3] = 0x00;

    pLineObj->icr4Values[0] = 0x91;
    pLineObj->icr4Values[1] = 0x01;
    pLineObj->icr4Values[2] = 0x00;
    pLineObj->icr4Values[3] = 0x00;


    /* Get the adc reg setting */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_RD, VP880_CONV_CFG_LEN, &adcDefault);
    adcDefault &= ~VP880_CONV_CONNECT_BITS;
    adcDefault |= VP880_NO_CONNECT;

    /*
     * FORCE ADC to metalic AC:
     * THIS is a required workaround. If the device is in idle and
     * the ADC is forced on in ICR4, the data from the ADC will be
     * railed. However, if the converter configuration register is
     * set to NO_CONNECT first, then this issue does not exist.
    */
    /* set ADC to NULL state */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_CONV_CFG_WRT,
        VP880_CONV_CFG_LEN, &adcDefault);

    /* blank out siggen A */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SIGA_PARAMS_WRT,
        VP880_SIGA_PARAMS_LEN, sigGenAB);

    /* Turn off sig gens */
    pLineObj->sigGenCtrl[0] = sigGenCtrl;
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_GEN_CTRL_WRT,
        VP880_GEN_CTRL_LEN, pLineObj->sigGenCtrl);

    /* Disable hi-pass filter and cutoff receive path */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_COND_WRT,
        VP880_OP_COND_LEN, &opCond);
    pLineObj->opCond[0] = opCond;

    /* Set to read linear data */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_OP_FUNC_WRT,
        VP880_OP_FUNC_LEN, &opFunction);

    /* Disable automatic state changes */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_VP_GAIN_WRT,
        VP880_VP_GAIN_LEN, &gain);

    /* Disable DISN */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_DISN_WRT,
        VP880_DISN_LEN, &disn);

    /* Disable automatic state changes */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_SS_CONFIG_WRT,
        VP880_SS_CONFIG_LEN, &ssCfg);

    /* Clear ICR1 mask bits */
    if (pLineObj->internalTestTermApplied == FALSE) {
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR1_WRT,
            VP880_ICR1_LEN, pLineObj->icr1Values);
    }

    /* Turn off feed voltage */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR2_WRT,
        VP880_ICR2_LEN, pLineObj->icr2Values);

    /* Modify longitudinal bias */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR3_WRT,
        VP880_ICR3_LEN, pLineObj->icr3Values);

    /* AISN & DAC met drive off, ADC on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP880_ICR4_WRT,
        VP880_ICR4_LEN, pLineObj->icr4Values);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * Vp880AdcSetup()
 * This function connects the ADC, sets the AX bits and set the Voice Driver
 * in motion to start collecting PCM data.
 *
 * Parameters:
 *  pLineCtx    - pointer to the line context
 *  adRoute     - index indicating how to setup the ADC routing
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
EXTERN bool
Vp880AdcSetup(
    VpLineCtxType *pLineCtx,
    uint8 adRoute,
    bool txAnalogGainEnable)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;

    uint8 chanId = pLineObj->channelId;
    uint8 tempRxAngLoss[VP880_VP_GAIN_LEN];
    uint8 rxAngLoss = 0;
    uint8 txBufferDataRate = pDevObj->txBufferDataRate;
    uint8 deviceRcn = (uint8)(pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]);

    /* error check the A to D routing value*/
    if ((adRoute > 15) || (0 == ((1 << adRoute) && VP880_LEGAL_A2D_CVRT_VALS))) {
        return FALSE;
    }
    adRoute |= txBufferDataRate;

    /* select the proper gain setting */
    if (TRUE == txAnalogGainEnable) {
        rxAngLoss = VP880_AX_MASK;
    }

    /* must redirect the pcm data from proper channel to pcmBuffer */
    if (VP880_REV_VC < deviceRcn) {
        uint8 chanSel[] = {VP880_DEV_MODE_CHAN0_SEL, VP880_DEV_MODE_CHAN1_SEL};
        pDevObj->devMode[0] = (chanSel[chanId] | VP880_DEV_MODE_TEST_DATA);
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_DEV_MODE_WRT,
            VP880_DEV_MODE_LEN, pDevObj->devMode);
    }

    /*
     * ADC workaround
     * 1) set adc to NULL
     * 2) wait for 5 noop transactions
     * 3) set adc to requested connection
     */

     /* no reason to do the work around if null was selected */
    if (VP880_NO_CONNECT != adRoute) {
        uint8 adcWorkAround[6] = {VP880_NO_CONNECT, VP880_NO_OP_WRT, VP880_NO_OP_WRT,
                VP880_NO_OP_WRT, VP880_NO_OP_WRT, VP880_NO_OP_WRT};

        /* make sure the no connect state has the correct data rate */
        adcWorkAround[0] |= txBufferDataRate;

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_WRT,
            6, adcWorkAround);
    }

    /* write the requested converter configuration adRouting */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_CONV_CFG_WRT,
        VP880_CONV_CFG_LEN, &adRoute);

    /* read modify write the Voice Path Gain register with rxAngLoss input arg*/
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_VP_GAIN_RD,
        VP880_VP_GAIN_LEN, tempRxAngLoss);

    /* some tests modify the AR bits so don't touch them */
    tempRxAngLoss[0] &= (uint8)(~(VP880_DR_LOSS_MASK | VP880_AX_MASK));
    tempRxAngLoss[0] |= rxAngLoss;

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_VP_GAIN_WRT,
        VP880_VP_GAIN_LEN, tempRxAngLoss);

    return TRUE;
}

/*------------------------------------------------------------------------------
 * VpGetPcmResult()
 * This function will retreive the data from the Test Heap and place it into the
 * variable indicated by pResults.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object.
 *  pcmOperation    - specifies which value to get from heap.
 *  aToDRoute       - indication as to which measurment was taken.
 *  removeOffset    - indication if calibration offsets should be removed.
 *  chanId          - specifies which channels offsets to remove (only used
 *                    if removeOffset bool is TRUE)
 *  pdata           - pointer to the data that needs to be altered.
 *
 * Returns:
 *
 *----------------------------------------------------------------------------*/
EXTERN void
VpGetPcmResult(
    Vp880DeviceObjectType *pDevObj,
    VpPcmOperationBitType pcmOperation,
    bool  txAnalogGainEnable,
    bool  removeOffset,
    uint8 chanId,
    uint8 aToDRoute,
    void *pResult)
{
    int16 *pResult16 = (int16*)pResult;
    int32 *pResult32 = (int32*)pResult;
    uint8 deviceRcn = (uint8)(pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]);
    int32 temp;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    uint8 channelId = pTestInfo->channelId;

    switch (pcmOperation) {
        case VP_PCM_OPERATION_AVERAGE:
            *pResult16 = pDevObj->currentTest.pTestHeap->pcmResults.average;
            break;
        case VP_PCM_OPERATION_RANGE:
            *pResult16 = pDevObj->currentTest.pTestHeap->pcmResults.range;
            break;
        case VP_PCM_OPERATION_RMS:
            *pResult16 = pDevObj->currentTest.pTestHeap->pcmResults.rms;
            break;
        case VP_PCM_OPERATION_MIN:
            *pResult16 = pDevObj->currentTest.pTestHeap->pcmResults.min;
            break;
        case VP_PCM_OPERATION_MAX:
            *pResult16 = pDevObj->currentTest.pTestHeap->pcmResults.max;
            break;
        case VP_PCM_OPERATION_APP_SPECIFIC:
            pResult = pDevObj->currentTest.pTestHeap->pcmResults.pApplicationInfo;
            break;
        case VP_PCM_OPERATION_FREQ:
            *pResult32 = pDevObj->currentTest.pTestHeap->pcmResults.freq;
            break;
        default:
            VP_TEST(None, NULL, ("Invalid VpPcmOperationBitType VpGetPcmResult(0x%04x)", pcmOperation));
            pResult16 = 0;
            return;
    }

    /*
     * If the AX bit was enabled the measurment was doubled
     */
    if (TRUE == txAnalogGainEnable) {
        *pResult16 /= 2 ;
    }

    /* HARDWARE BUG: METALLIC VOLTAGE is inverted for devices greater than REVC */
    if ((VP880_REV_VC < deviceRcn) && (VP880_METALLIC_DC_V == aToDRoute)) {
        if ((pcmOperation & VP_PCM_OPERATION_AVERAGE) ||
          (pcmOperation & VP_PCM_OPERATION_MIN) ||
          (pcmOperation & VP_PCM_OPERATION_MAX)) {
            VP_TEST(None, NULL, ("VpGetPcmResult() INVERTING_SIGN"));
            *pResult16 = ((*pResult16 == VP_INT16_MIN) ? VP_INT16_MAX : -(*pResult16));
        }
    }

    /* remove calibration offset if requested */
    if (TRUE == removeOffset) {
        Vp880CalOffCoeffs *pCalOffsets = &pDevObj->calOffsets[chanId];
        int16 offset = 0;
        int16 result = *pResult16;

        switch (aToDRoute) {
            case VP880_METALLIC_DC_V:
                offset = (pCalOffsets->vabOffset) ? pCalOffsets->vabOffset :
                    (-1 * pDevObj->vp880SysCalData.vocOffset[chanId][VP880_NORM_POLARITY]);
                break;
            case VP880_TIP_TO_GND_V:
                offset = pCalOffsets->vahOffset;
                break;
            case VP880_RING_TO_GND_V:
                offset = pCalOffsets->vbhOffset;
                break;
            case VP880_METALLIC_DC_I:
                offset = pCalOffsets->imtOffset;
                break;
            case VP880_LONGITUDINAL_DC_I:
                offset = pCalOffsets->ilgOffset;
                break;
            case VP880_SWITCHER_Y:
                offset = pDevObj->vp880SysCalData.swyOffset[chanId];
                break;
            case VP880_SWITCHER_Z:
                offset = pDevObj->vp880SysCalData.swzOffset[chanId];
                break;
            case VP880_XBR:
                offset = pDevObj->vp880SysCalData.swxbOffset[chanId];
                break;
            case VP880_NO_CONNECT:
                offset = pCalOffsets->nullOffset;
                break;
            default:
                break;
        }

        if ((result == VP_INT16_MIN) || (result == VP_INT16_MAX)) {
            temp = (int32)result;
        } else {
            VP_TEST(None, NULL, ("Vp880GetPcmResult(): ADCRoute: %d, chanId: %d, REMOVING_OFFSET: %d, FROM: %d",
                aToDRoute, chanId, offset, result));
            temp = (int32)result - (int32)offset;
        }

        /* make sure that the offset does not cause over/underflow on 16 bits */
        if ((int32)VP_INT16_MAX <= temp) {
            *pResult16 = VP_INT16_MAX;
        } else if ((int32)VP_INT16_MIN >= temp) {
            *pResult16 = VP_INT16_MIN;
        } else {
            *pResult16 = (int16)temp;
        }

        /* VBAT3 only need to be measured on ABS devices, otherwise store a marker value */
        if ((aToDRoute == VP880_XBR) && ((pDevObj->stateInt & VP880_IS_ABS) == FALSE)) {
            /* Tracker: VP_INT16_MIN->vbat1=VP880_SWITCHER_Y / VP_INT16_MAX->vbat1=VP880_SWITCHER_Z */
            if (channelId == 1) {
                *pResult16 = VP_INT16_MAX;
            } else {
                *pResult16 = VP_INT16_MIN;
            }
        }
    }

    return;
}

/*v-------------------------local functions----------------------------------v*/


/*------------------------------------------------------------------------------
 * VpIsLineReadyForTest()
 *
 * This function is only called by the Vp880TestLine function and
 * is determining that if the test specified by the test argument
 * can be run at this point in time.
 *
 *
 * Parameters:
 *  pLineCtx - pointer to the line context
 *  handle   - unique test handle
 *  test     - test Id of the test being requested to run.
 *
 * Returns:
 * An test status type error code indicating if the test Id can be run at
 * this time. If for any reason the test can not be run, then the function
 * will return something other than VP_TEST_STATUS_SUCCESS.
 *----------------------------------------------------------------------------*/
VpTestStatusType
VpIsLineReadyForTest(
    VpLineCtxType *pLineCtx,
    VpTestIdType test)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;

    if (!pDevObj->currentTest.prepared) {
        if (VP_TEST_ID_PREPARE == test) {
            /*
             * VocicePort object does NOT have prepared line for testing,
             * and the test id requested is test prepare.
             */
            return VP_TEST_STATUS_SUCCESS;
        } else {
            /*
             * VocicePort object does NOT have prepared line for testing and
             * the current test id is not a test prepare so return not ready
             */
            VP_WARNING(VpLineCtxType, pLineCtx, ("VpIsLineReadyForTest(): VP_TEST_STATUS_LINE_NOT_READY"));
            return VP_TEST_STATUS_LINE_NOT_READY;
        }
    } else {
        if ((VP_TEST_ID_PREPARE == test) || (pDevObj->currentTest.preparing == TRUE)) {
            /*
             * VocicePort object has a prepared line but the current test id
             * is to run prepare again so return busy prevents user from
             * accidently storing testing coeffs.
             */
            VP_WARNING(VpLineCtxType, pLineCtx, ("VpIsLineReadyForTest(1): VP_TEST_STATUS_RESOURCE_NA"))
            return VP_TEST_STATUS_RESOURCE_NA;
        }
    }

    /*
     * This section of code will only be reached under one condition:
     * If the object is prepared to run a test and the test is not prepare
     */
    if (pDevObj->currentTest.channelId != pLineObj->channelId) {
        /*
         * the test request is NOT on the prepared channel
         */
        VP_WARNING(VpLineCtxType, pLineCtx, ("VpIsLineReadyForTest(2): VP_TEST_STATUS_RESOURCE_NA"))
        return VP_TEST_STATUS_RESOURCE_NA;
    } else {
        if ((VP_TEST_ID_CONCLUDE != test) && (0 <= pDevObj->currentTest.testState)) {
            /*
             * The requested test is not conclude and the device is busy
             * with another test.
             */
            VP_WARNING(VpLineCtxType, pLineCtx, ("VpIsLineReadyForTest(): VP_TEST_STATUS_TESTING"))
            return VP_TEST_STATUS_TESTING;
        }
    }
    return VP_TEST_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp880TestPrepare()
 *
 * If there is a pending errorCode then the function will generate an event
 * indicating that there was trouble and bail out without saving any coeffs.
 *
 * If the function does not bail, all registers that may be modified by any
 * of the other test primatives are save to the device object. All registers
 * will be saved off in one tick so that the device will not have to
 * deal with being aborted during the middle of a PREPARE. Once all registers
 * are saved the prepared flag will be set to TRUE, and and event will be
 * generated indicating all went well.
 *
 * Parameters:
 *  pLineCtx - pointer to the line context
 *  handle   - unique test handle
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
VpStatusType
Vp880TestPrepare(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = NULL;
    int16 *pTestState = &pTestInfo->testState;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 chanId = pLineObj->channelId;

    if (callback == FALSE) {
        #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
            VpSysServiceToggleLed(10);
        #endif

        if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
            /*
             * if erroCode = VP_TEST_STATUS_RESOURCE_NA then someone tried to
             * call VP_TESTI_ID_PREPARE more than once without calling conclude
             * in which case we do not want to change the prepared or preparing
             * flags.
             */
            if (pDevObj->testResults.errorCode != VP_TEST_STATUS_RESOURCE_NA) {
                pTestInfo->prepared = FALSE;
                pTestInfo->preparing = FALSE;
            }
            VpGenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestPrepare(ec:%i): bailed",
                pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;
        }

        /* try to get a test heap */
        pTestInfo->pTestHeap =
            (Vp880TestHeapType*)VpSysTestHeapAcquire(&pTestInfo->testHeapId);

        VP_TEST(VpLineCtxType, pLineCtx, ("VpSysTestHeapAcquire()"));
        if (pTestInfo->pTestHeap == VP_NULL) {
            /* if a heap is not available then generate an event saying prepare failed. */
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880AcquireTestHeap(heap:NULL): bailed"));
            pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
            VpGenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, FALSE, FALSE);
            return VP_STATUS_SUCCESS;
        }

        /* setup the test heap */
        Vp880InitTestHeap(pTestInfo->pTestHeap);

        /* set the necessary flags to indicate the readiness of the test code */
        pTestInfo->testId = testId;
        pTestInfo->handle = handle;
        pTestInfo->prepared = TRUE;
        pDevObj->currentTest.preparing = TRUE;
        pDevObj->currentTest.channelId = chanId;

        pTestInfo->pTestHeap->testArgs.prepare.intrusive = TRUE;

        /*
         * if this is a low power device then we have to wait before
         * storing all of the data to the scratch pads
         */
        if ((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
            (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
            (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) {

            VP_INFO(VpLineCtxType, pLineCtx, ("Initialize low power mode"));

            /* As the flag pDevObj->currentTest.preparing is set to true, the device will be */
            /* take out of low power mode during the next tick */

            VpSetTestStateAndTimer(pDevObj, pTestState, VP880_TESTLINE_GLB_STRT_STATE, NEXT_TICK*2);

            return VP_STATUS_SUCCESS;
        }
    }

    /*
     * If this is a low power device, we must wait until we are completely
     * out of low power mode before we start storing register values.
     * This conditional continues to read the low power change timer
     * until it is no longer active.
     */
    if ((*pTestState == VP880_TESTLINE_GLB_STRT_STATE) &&
        ((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
        (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
        (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) ) {

        if (pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] & VP_ACTIVATE_TIMER) {
            *pTestState = VP880_TESTLINE_GLB_STRT_STATE;
        } else {
            *pTestState = VP880_TESTLINE_GLB_END_STATE;
        }
        VpSetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK*2);
        return VP_STATUS_SUCCESS;
    }

    pTestHeap = pTestInfo->pTestHeap;

    /* save all registers that may be modified by any test primative */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_OP_FUNC_RD,
        VP880_OP_FUNC_LEN, &pTestHeap->opFunction);

    pTestHeap->opCond = pLineObj->opCond[0];

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SS_CONFIG_RD,
        VP880_SS_CONFIG_LEN, &pTestHeap->sysConfig);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_VP_GAIN_RD,
        VP880_VP_GAIN_LEN, &pTestHeap->vpGain);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_DISN_RD,
        VP880_DISN_LEN, &pTestHeap->disn);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_DC_FEED_RD,
        VP880_DC_FEED_LEN, pTestHeap->dcFeed);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_LOOP_SUP_RD,
        VP880_LOOP_SUP_LEN, pTestHeap->lpSuper);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SIGA_PARAMS_RD,
        VP880_SIGA_PARAMS_LEN, pTestHeap->sigGenAB);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_REGULATOR_CTRL_RD,
        VP880_REGULATOR_CTRL_LEN, &pTestHeap->SwRegCtrl);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_B1_FILTER_RD,
        VP880_B1_FILTER_LEN, pTestHeap->b1Filter);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_B2_FILTER_RD,
        VP880_B2_FILTER_LEN, pTestHeap->b2Filter);

    VpMemCpy(pTestHeap->icr1, pLineObj->icr1Values, VP880_ICR1_LEN);
    VpMemCpy(pTestHeap->icr2, pLineObj->icr2Values, VP880_ICR2_LEN);
    VpMemCpy(pTestHeap->icr3, pLineObj->icr3Values, VP880_ICR3_LEN);
    VpMemCpy(pTestHeap->icr4, pLineObj->icr4Values, VP880_ICR4_LEN);
    VpMemCpy(pTestHeap->icr6, pLineObj->icr6Values, VP880_ICR6_LEN);
    pTestHeap->sigCtrl = pLineObj->sigGenCtrl[0];

    VpMemCpy(pTestHeap->switchReg, pDevObj->swParamsCache,
        VP880_REGULATOR_PARAM_LEN);

    pTestHeap->slacState = pLineObj->slicValueCache;

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_BAT_CALIBRATION_RD,
        VP880_BAT_CALIBRATION_LEN, pTestHeap->batCal);

    /* Enable the line control block circuitry to have accurate voltage measurments in disconnect */
    pLineObj->icr3Values[0] = pTestHeap->icr3[0] | 0x20;
    pLineObj->icr3Values[1] = pTestHeap->icr3[1] | 0x20;
    pLineObj->icr3Values[2] = pTestHeap->icr3[2] | 0x00;
    pLineObj->icr3Values[3] = pTestHeap->icr3[3] | 0x00;
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_ICR3_WRT, VP880_ICR3_LEN, pLineObj->icr3Values);

    /* Force High battery on ABS */
    if ((VP880_IS_ABS & pDevObj->stateInt) == VP880_IS_ABS) {
        uint8 SwRegCtrl = pTestHeap->SwRegCtrl | 0x0F;

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_REGULATOR_CTRL_WRT,
            VP880_REGULATOR_CTRL_LEN, &SwRegCtrl);
    }

    /*
     * the following code is a workaround preventing the storing of
     * the opFunction when the slic is in the ringing state.
     */
    pTestHeap->opFunction |= VP880_ENABLE_LOADED_COEFFICIENTS;
    /* Enable the desired CODEC mode */
    switch (pLineObj->codec) {
        case VP_OPTION_LINEAR:      /* 16 bit linear PCM */
        case VP_OPTION_WIDEBAND:    /* Wideband asumes Linear PCM */
            pTestHeap->opFunction |= VP880_LINEAR_CODEC;
            pTestHeap->opFunction &= ~(VP880_ULAW_CODEC);
            break;

        case VP_OPTION_ALAW:                /* A-law PCM */
            pTestHeap->opFunction &= (uint8)(~(VP880_LINEAR_CODEC | VP880_ULAW_CODEC));
            break;

        case VP_OPTION_MLAW:                /* u-law PCM */
            pTestHeap->opFunction |= VP880_ULAW_CODEC;
            pTestHeap->opFunction &= ~(VP880_LINEAR_CODEC);
            break;

        default:
            /* Cannot reach here.  Error checking at top */
            break;
    } /* Switch */

    /* save off current event mask */
    pTestHeap->preTestEventMask = pLineObj->lineEventsMask;

    /* mask off all events (except VP_LINE_EVID_TEST_CMP) until test conclude */
    pLineObj->lineEventsMask.faults = 0xFFFF;
    pLineObj->lineEventsMask.signaling = 0xFFFF;
    pLineObj->lineEventsMask.response = 0xFFFF;
    pLineObj->lineEventsMask.test = ~(VP_LINE_EVID_TEST_CMP | VP_LINE_EVID_ABORT);
    pLineObj->lineEventsMask.process = 0xFFFF;
    pLineObj->lineEventsMask.fxo = 0xFFFF;
    pLineObj->lineEventsMask.packet = 0xFFFF;

    pDevObj->testResults.errorCode = VP_TEST_STATUS_SUCCESS;
    VpGenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, TRUE, FALSE);

    /* indication that we are no longer preparing the device */
    pDevObj->currentTest.preparing = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestPrepare(): complete"));
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp880TestPrepareExt()
 *
 * If there is a pending errorCode then the function will generate an event
 * indicating that there was trouble and bail out without saving any coeffs.
 *
 * If the function does not bail, all registers that may be modified by any
 * of the other test primatives are save to the device object. All registers
 * will be saved off in one tick so that the device will not have to
 * deal with being aborted during the middle of a PREPARE. Once all registers
 * are saved the prepared flag will be set to TRUE, and and event will be
 * generated indicating all went well.
 *
 * Parameters:
 *  pLineCtx - pointer to the line context
 *  handle   - unique test handle
 *
 * Returns:
 * --
 *----------------------------------------------------------------------------*/
VpStatusType
Vp880TestPrepareExt(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback,
    VpTestIdType testId)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp880TestHeapType *pTestHeap = NULL;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    uint8 chanId = pLineObj->channelId;

    if (callback == FALSE) {
        #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
            VpSysServiceToggleLed(10);
        #endif

        if (VP_TEST_STATUS_SUCCESS != pDevObj->testResults.errorCode) {
            /*
             * if erroCode = VP_TEST_STATUS_RESOURCE_NA then someone tried to
             * call VP_TESTI_ID_PREPARE more than once without calling conclude
             * in which case we do not want to change the prepared or preparing
             * flags.
             */
            if (pDevObj->testResults.errorCode != VP_TEST_STATUS_RESOURCE_NA) {
                pTestInfo->prepared = FALSE;
                pTestInfo->preparing = FALSE;
            }
            VpGenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880TestPrepareExt(ec:%i): bailed",
                pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;
        }

        /* try to get a test heap */
        pTestInfo->pTestHeap =
            (Vp880TestHeapType*)VpSysTestHeapAcquire(&pTestInfo->testHeapId);

        VP_TEST(VpLineCtxType, pLineCtx, ("VpSysTestHeapAcquire()"));
        if (pTestInfo->pTestHeap == VP_NULL) {
            /* if a heap is not available then generate an event saying prepare failed. */
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp880AcquireTestHeap(heap:NULL): bailed"));
            pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
            VpGenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, FALSE, FALSE);
            return VP_STATUS_SUCCESS;
        }

        /* setup the test heap */
        Vp880InitTestHeap(pTestInfo->pTestHeap);

        /* set the necessary flags to indicate the readiness of the test code */
        pTestInfo->testId = testId;
        pTestInfo->handle = handle;
        pTestInfo->prepared = TRUE;
        pDevObj->currentTest.preparing = TRUE;
        pDevObj->currentTest.channelId = chanId;

        pTestInfo->pTestHeap->testArgs.prepare.intrusive = FALSE;
    }
    pTestHeap = pTestInfo->pTestHeap;

    /* save all registers that may be modified by any test primative */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_OP_FUNC_RD,
        VP880_OP_FUNC_LEN, &pTestHeap->opFunction);

    pTestHeap->opCond = pLineObj->opCond[0];

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SS_CONFIG_RD,
        VP880_SS_CONFIG_LEN, &pTestHeap->sysConfig);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_VP_GAIN_RD,
        VP880_VP_GAIN_LEN, &pTestHeap->vpGain);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_DISN_RD,
        VP880_DISN_LEN, &pTestHeap->disn);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_DC_FEED_RD,
        VP880_DC_FEED_LEN, pTestHeap->dcFeed);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_LOOP_SUP_RD,
        VP880_LOOP_SUP_LEN, pTestHeap->lpSuper);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_SIGA_PARAMS_RD,
        VP880_SIGA_PARAMS_LEN, pTestHeap->sigGenAB);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_REGULATOR_CTRL_RD,
        VP880_REGULATOR_CTRL_LEN, &pTestHeap->SwRegCtrl);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_B1_FILTER_RD,
        VP880_B1_FILTER_LEN, pTestHeap->b1Filter);

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_B2_FILTER_RD,
        VP880_B2_FILTER_LEN, pTestHeap->b2Filter);

    VpMemCpy(pTestHeap->icr1, pLineObj->icr1Values, VP880_ICR1_LEN);
    VpMemCpy(pTestHeap->icr2, pLineObj->icr2Values, VP880_ICR2_LEN);
    VpMemCpy(pTestHeap->icr3, pLineObj->icr3Values, VP880_ICR3_LEN);
    VpMemCpy(pTestHeap->icr4, pLineObj->icr4Values, VP880_ICR4_LEN);
    VpMemCpy(pTestHeap->icr6, pLineObj->icr6Values, VP880_ICR6_LEN);
    pTestHeap->sigCtrl = pLineObj->sigGenCtrl[0];

    VpMemCpy(pTestHeap->switchReg, pDevObj->swParamsCache,
        VP880_REGULATOR_PARAM_LEN);

    pTestHeap->slacState = pLineObj->slicValueCache;

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_BAT_CALIBRATION_RD,
        VP880_BAT_CALIBRATION_LEN, pTestHeap->batCal);


    /* Enable the line control block circuitry to have accurate voltage measurments in disconnect */
    pLineObj->icr3Values[0] = pTestHeap->icr3[0] | 0x20;
    pLineObj->icr3Values[1] = pTestHeap->icr3[1] | 0x20;
    pLineObj->icr3Values[2] = pTestHeap->icr3[2] | 0x00;
    pLineObj->icr3Values[3] = pTestHeap->icr3[3] | 0x00;
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_ICR3_WRT, VP880_ICR3_LEN, pLineObj->icr3Values);

    /*
     * the following code is a workaround preventing the storing of
     * the opFunction when the slic is in the ringing state.
     */
    pTestHeap->opFunction |= VP880_ENABLE_LOADED_COEFFICIENTS;
    /* Enable the desired CODEC mode */
    switch (pLineObj->codec) {
        case VP_OPTION_LINEAR:      /* 16 bit linear PCM */
        case VP_OPTION_WIDEBAND:    /* Wideband asumes Linear PCM */
            pTestHeap->opFunction |= VP880_LINEAR_CODEC;
            pTestHeap->opFunction &= ~(VP880_ULAW_CODEC);
            break;

        case VP_OPTION_ALAW:                /* A-law PCM */
            pTestHeap->opFunction &= (uint8)(~(VP880_LINEAR_CODEC | VP880_ULAW_CODEC));
            break;

        case VP_OPTION_MLAW:                /* u-law PCM */
            pTestHeap->opFunction |= VP880_ULAW_CODEC;
            pTestHeap->opFunction &= ~(VP880_LINEAR_CODEC);
            break;

        default:
            /* Cannot reach here.  Error checking at top */
            break;
    } /* Switch */

    /* save off current event mask */
    pTestHeap->preTestEventMask = pLineObj->lineEventsMask;

    /* mask off all events (except VP_LINE_EVID_TEST_CMP) until test conclude */
    pLineObj->lineEventsMask.faults = 0xFFFF;
    pLineObj->lineEventsMask.signaling = 0xFFFF;
    pLineObj->lineEventsMask.response = 0xFFFF;
    pLineObj->lineEventsMask.test = ~(VP_LINE_EVID_TEST_CMP | VP_LINE_EVID_ABORT);
    pLineObj->lineEventsMask.process = 0xFFFF;
    pLineObj->lineEventsMask.fxo = 0xFFFF;
    pLineObj->lineEventsMask.packet = 0xFFFF;

    pDevObj->testResults.errorCode = VP_TEST_STATUS_SUCCESS;
    VpGenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, TRUE, FALSE);

    /* indication that we are no longer preparing the device */
    pDevObj->currentTest.preparing = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp880TestPrepareExt(): complete"));
    return VP_STATUS_SUCCESS;
}

/*------------------------------------------------------------------------------
 * Vp880InitTestHeap()
 * This function is used to reset the some of the more commonly used members
 * of the test heap structure. This function is called durring test prepare
 * and again each time a test event is generated.
 *
 * Parameters:
 *  pTestHeap       - pointer to current test heap
 *
 * Returns:
 *  --
 *----------------------------------------------------------------------------*/
void
Vp880InitTestHeap(
    Vp880TestHeapType *pTestHeap)
{
    pTestHeap->adcState = 0;
    pTestHeap->nextState = VP880_TESTLINE_GLB_IVLD_STATE;
    pTestHeap->pcmRequest = FALSE;
    return;
}

/*------------------------------------------------------------------------------
 * VpStartPcmCollect()
 * This function will set up the device timer to expire in 'timeoutTime'
 * milliseconds. The timer will only expire VpPcmCallback() is not called in
 * the specified time. The function sets the current test State to the
 * global quit state so that if the timer does expirer an event will
 * be generated indicating the timeout in the errorCode.
 *
 * Next the function will request that the system service layer (ssl) function
 * begin collecting PCM data. If the ssl does not have an available buffer
 * for PCM data, the device timer is changed to expire on the next tick and
 * the errorCode is changed to indicate an internal error when the next event
 * is generated.
 *
 * If the ssl was able to create a buffer for PCM data the id of the buffer
 * is stored in the device object and the state that the VpPcmCallback()
 * function will need to call to continue the test is stored into the
 * device object.
 *
 * The alternate mode defined by VP880_EZ_MPI_PCM_COLLECT allows the API tick to do
 * the PCM collection.
 *
 * Parameters:
 *  pLineCtx        - pointer to the line context
 *  timeoutTime     - Amount of time (ms) to wait before calling timeout State
 *  nextState       - test state to run if VpPcmCallback() is called.
 *  skipSamples     - number of samples to skip once pcm buffer is ready
 *  pcmSamples      - number of samples to collect once skipped samples are done
 *
 * Returns:
 * TRUE if not in EZ mode or if all samples have been collected in EZ mode
 * FALSE in EZ mode if not all samples have been collected.
 *----------------------------------------------------------------------------*/
bool
VpStartPcmCollect(
    VpLineCtxType *pLineCtx,
    VpPcmOperationMaskType operationMask,
    int16 nextState,
    uint16 settlingTime,
    uint16 integrateTime)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp880LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp880DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp880TestHeapType *pTestHeap = pDevObj->currentTest.pTestHeap;
    VpDeviceIdType deviceId = pDevObj->deviceId;

#ifndef VP880_EZ_MPI_PCM_COLLECT
    uint8 chanId = pLineObj->channelId;
    int16 *ptestState = &(pDevObj->currentTest).testState;
    int16 timeoutTime = VP880_TOTAL_PCM_COLLECTION_TIME(integrateTime, settlingTime);
    uint8 startTimeSlot;

    /* get the current transmit timeslot */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_TX_TS_RD,
        VP880_TX_TS_LEN, &startTimeSlot);

    startTimeSlot &= VP880_TX_TS_MASK;

    /* make sure timeoutTime is atleast one ms */
    if (0 > timeoutTime) {
        timeoutTime = 1;
    }

    /* Set the error code incase the timeout happens */
    pDevObj->testResults.errorCode = VP_TEST_STATUS_TIMEOUT;

    /* Set the next state to be the quit/timeout state*/
    *ptestState = VP880_TESTLINE_GLB_QUIT_STATE;

    /* Setup the callback next state */
    pTestHeap->nextState = nextState;

    VpSysPcmCollectAndProcess(pLineCtx, deviceId, chanId, startTimeSlot, integrateTime,
        settlingTime, operationMask);
    /*
     * Setup the device timer to cause a test to quit if call back does not
     * happen in the calculated time frame
     */
    VpSetTestStateAndTimer(pDevObj, ptestState, VP880_TESTLINE_GLB_QUIT_STATE,
        MS_TO_TICKRATE(timeoutTime, pDevObj->devProfileData.tickRate));
    VP_TEST(VpLineCtxType, pLineCtx, ("VpStartPcmCollect(it:%i,st:%i,om:%i,to:%i): request",
        integrateTime, settlingTime, operationMask, timeoutTime));
    return TRUE;

#else
    uint8 numSamples = 1;
    int16 newPcmSamples[8]; /* maximum of 7 16bit samples in preparation of
                             * the revision 3 VoicePort devices */
    uint8 deviceRcn = (uint8)(pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]);

    if (FALSE == pTestHeap->pcmRequest) {
        uint16 downSampleRate;
        pTestHeap->pcmRequest = TRUE;

        /* get the down sample rate based on the device revision */
        if (VP880_REV_VC >= deviceRcn) {
            uint16 tickInMs = TICKS_TO_MS(1, pDevObj->devProfileData.tickRate);
            /* down sample rate is based on the tick rate */
            downSampleRate = ((tickInMs * 1000) / 125);
        } else if (pLineObj->codec == VP_OPTION_WIDEBAND) {
            /* down sample rate is based on the Conv. Confg. register in wideband mode */
            uint8 downSampleArry[] = {1, 1, 2, 4, 8, 16, 16, 16};
            downSampleRate = downSampleArry[(pDevObj->txBufferDataRate >> 4) & 0x07];
        } else {
            /* down sample rate is based on the Conv. Confg. register in narrowband mode */
            uint8 downSampleArry[] = {1, 2, 4, 8, 16, 16, 16, 16};
            downSampleRate = downSampleArry[(pDevObj->txBufferDataRate >> 4) & 0x07];
        }

        /* reset the pcmcollect util */
        VpPcmComputeReset(operationMask, integrateTime, settlingTime,
            downSampleRate, TRUE, &pTestHeap->ezPcmTemp,
            &pTestHeap->pcmResults);

        #ifdef VP_DEBUG
        {
            uint16 rateArry[] = {8000, 4000, 2000, 1000, 500, 500, 500, 500};
            int32 underflowValue = ((pDevObj->devProfileData.tickRate *
                rateArry[(pDevObj->txBufferDataRate >> 4) & 0x07]) /
                ((pLineObj->codec == VP_OPTION_WIDEBAND) ? 500000 : 250000)) -2;

            /* reset the under and over flow counters */
            pTestHeap->underCnt = 0;
            pTestHeap->overCnt = 0;
            pTestHeap->underFlowValue = (underflowValue < 0) ? 0 :(int8)underflowValue;
        }
        #endif

        VP_TEST(VpLineCtxType, pLineCtx, ("VpStartPcmCollect(): EZrequest Reset"));
        VP_TEST(VpLineCtxType, pLineCtx, ("VpStartPcmCollect(it:%i,st:%i,om:%i,ds%i): EZrequest",
        integrateTime, settlingTime, operationMask, downSampleRate));

        /* Hardware debug method to check the PCM collection rate, can be removed */
        #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
            VpSysServiceToggleLed(9);
        #endif
    }

    /* need to implement this when the devices are ready */
    /*3 == pDevObj->staticInfo.rcnPcn[VP880_RCN_LOCATION]*/
    if (VP880_REV_VC >= deviceRcn) {
        uint8 tempBuff[VP880_TX_PCM_DATA_LEN];
        /*
         * Devices that rev 2 and lower are able to collect only
         * one 16bit linear sample each tick.
         */
        /* collect the single sample */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP880_TX_PCM_DATA_RD,
            VP880_TX_PCM_DATA_LEN, tempBuff);

        /* store the sample in 16bit format */
        newPcmSamples[0] = ( (tempBuff[0] << 8) | tempBuff[1]);
    } else {

        uint8 *pTxBuffer = pDevObj->txBuffer;
        int i = 2;
        int j = 0;

        /* get the num samples from the first nibble of the buffer */
        numSamples = (pTxBuffer[0] >> 4) & 0x07;

        /*
         * count under and or overflows
         * skip the first (reset) request
         */
        #ifdef VP_DEBUG
        if (FALSE != pTestHeap->pcmRequest) {
            if (numSamples >= 7) {
                /*
                 * overflow - we must set the numSamples to 6 so that we dont
                 * read off the end of our buffer array
                 */
                pTestHeap->overCnt++;
                numSamples = 6;
                /* Toggle LED11 each time a buffer overflow occurs */
                #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
                    VpSysServiceToggleLed(11);
                #endif
            } else if ((numSamples <= pTestHeap->underFlowValue) || (numSamples == 0)) {
                pTestHeap->underCnt++;
                #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
                    VpSysServiceToggleLed(11);
                #endif
            }
        }
        #else
            numSamples = (numSamples >= 7) ? 6 : numSamples;
        #endif

        /*
         * store all 16 bit linear samples into the newPcmSamples buffer
         * Pcm buffer data starts with the third nibble in the tempBuffer
         */
        for (i = 2; i <= (numSamples*2); i+=2) {
            newPcmSamples[j++] = ((uint16)pTxBuffer[i] << 8) | (uint16)pTxBuffer[i+1];
        }

        /* VP_TEST(VpLineCtxType, pLineCtx, ("VpStartPcmCollect2(numSamples:%i, timeStamp %d)" ,numSamples, pDevObj->timeStamp)); */
    }

    /* use VpPcmCompute() until ready to do callback */
    if (PCM_CAL_DONE == VpPcmCompute(newPcmSamples, numSamples,
        &pTestHeap->ezPcmTemp, &pTestHeap->pcmResults)) {

        pTestHeap->pcmRequest = FALSE;
        pTestHeap->nextState = nextState;
        Vp880TestLineCallback(pLineCtx, &pTestHeap->pcmResults);
        #ifdef VP_DEBUG
        if (pTestHeap->overCnt > 0) {
            VP_TEST_PCM(VpLineCtxType, pLineCtx, ("PcmCollect() overflows  >  7 %li",
                pTestHeap->overCnt));
        }
        if (pTestHeap->underCnt > 0) {
            VP_TEST_PCM(VpLineCtxType, pLineCtx, ("PcmCollect() underflows <= %i : %li",
                pTestHeap->underFlowValue, pTestHeap->underCnt));
        }
        #endif

        VP_TEST(VpLineCtxType, pLineCtx, ("VpStartPcmCollect(): complete"));

        /* Hardware debug method to check the PCM collection rate, can be removed */
        #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
            VpSysServiceToggleLed(9);
        #endif

        return TRUE;
    } else {
        pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] = (1 | VP_ACTIVATE_TIMER);
        return FALSE;
    }
#endif
}

/*------------------------------------------------------------------------------
 * VpReturnElapsedTime()
 * This function will return the elapsed time between the time-Stamp in argument
 * (ms) and the current value of the time stamp counter. This function will take
 * the rollover into account.
 *
 * Parameters:
 *  pDevObj         - pointer to the device object
 *  timeStamp       - Value of the last time stamp (ms).
 *
 * Returns:
 * The elapsed time in milliseconds.
 *----------------------------------------------------------------------------*/
uint16
VpReturnElapsedTime(
    Vp880DeviceObjectType *pDevObj,
    uint16 timeStamp)
{
    uint16 timeStampCounter = pDevObj->timeStamp / 2;

    if (timeStamp > timeStampCounter) {
        return (0x7FFF - timeStamp + timeStampCounter);
    } else {
        return (timeStampCounter - timeStamp);
    }
}

#endif /* VP880_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_880_SERIES */
