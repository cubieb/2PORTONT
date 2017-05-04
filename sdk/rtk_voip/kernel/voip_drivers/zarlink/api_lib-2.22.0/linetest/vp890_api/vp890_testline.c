/** \file vp890_testline.c
 * vp890_testline.c
 *
 *  This file contains the implementation of the VP-API 890 Series
 *  Test Functions.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2011-03-23 17:18:34 -0500 (Wed, 23 Mar 2011) $
 */

/* INCLUDES */
#include <linux/delay.h>
#include    "vp_api.h"

#if defined (VP_CC_890_SERIES)  /* Compile only if required */
#if defined (VP890_INCLUDE_TESTLINE_CODE)

#include    "vp_api_int.h"
#include    "vp890_api_int.h"
#include    "vp890_testline_int.h"
#include    "sys_service.h"

/* =================================
    Prototypes for Static Functions
   ================================= */
static VpTestStatusType
Vp890IsLineReadyForTest (
    VpLineCtxType *pLineCtx,
    VpTestIdType test);

static VpStatusType
Vp890TestPrepare (
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback);

static VpStatusType
Vp890TestPrepareExt (
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback);

static void
Vp890InitTestHeap(
    Vp890TestHeapType *pTestHeap);

/*------------------------------------------------------------------------------
 * Vp890TestLine()
 *  Initiates a line test.
 * This function will perfrom numerous checks to ensure that the test specified
 * by the testId argument can legally run. Once all error checks have been
 * completed the function relies on the Vp890TestLineInt function to actaully
 * figure out which test primative to execute.
 *
 * Refer to the VoicePath API User's Guide for the rest of the
 * details about this function.
 *----------------------------------------------------------------------------*/
VpStatusType
Vp890TestLine (
    VpLineCtxType *pLineCtx,
    VpTestIdType test,
    const void *pArgsUntyped,
    uint16 handle)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    VpTestStatusType errorCode = VP_TEST_STATUS_SUCCESS;
    VpStatusType rtnval = VP_STATUS_SUCCESS;

    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp890TestLine(tid:%i)+",test));

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

#ifdef VP890_EZ_MPI_PCM_COLLECT
    {
        Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;

        /* if in wideband and tick rate is greater than 6ms then error out*/
        if ((pLineObj->codec == VP_OPTION_WIDEBAND) &&
            (pDevObj->devProfileData.tickRate > 0x600)) {
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp890TestLine(%i:0x%02x:0x%04x): " \
                "VP_STATUS_FAILURE wideband not supported at this tick rate",
                pLineObj->codec, pLineObj->ecVal, pDevObj->devProfileData.tickRate ));

            return VP_STATUS_FAILURE;
        }
    }
#endif

    VpSysEnterCritical(deviceId, VP_CODE_CRITICAL_SEC);

    errorCode = Vp890IsLineReadyForTest(pLineCtx, test);

    pDevObj->testResults.errorCode = errorCode;
    VpSysExitCritical(deviceId, VP_CODE_CRITICAL_SEC);

    rtnval = Vp890TestLineInt(pLineCtx, test, pArgsUntyped, handle, FALSE);

    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp890TestLine()-"));
    return rtnval;
} /* Vp890TestLine() */

/*------------------------------------------------------------------------------
 * Vp890TestLineInt()
 *
 * This function is basically just a wrapper to each of the test primatives and
 * is called by one of two locations, either the Vp890TestLine function or by
 * the Vp890ServiceTimers function when the VP_DEV_TIMER_TESTLINE expires.
 *
 *----------------------------------------------------------------------------*/
EXTERN VpStatusType
Vp890TestLineInt(
    VpLineCtxType *pLineCtx,
    VpTestIdType test,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback)
{
    VpStatusType rtnval;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp890TestHeapType *pTestHeap = pDevObj->currentTest.pTestHeap;

    if ((TRUE == callback) &&
        (test != pDevObj->currentTest.testId)) {
            Vp890GenerateTestEvent(pLineCtx, test, handle, TRUE, TRUE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestLineInt(%i!=%i): tid mismatch",
                test, pDevObj->currentTest.testId));
            return VP_TEST_STATUS_INTERNAL_ERROR;
    }

    if (pLineObj->status & VP890_IS_FXO) {
        switch (test) {
            case VP_TEST_ID_PREPARE:
                pDevObj->currentTest.nonIntrusiveTest = FALSE;
                rtnval = Vp890TestPrepare(pLineCtx, handle, callback);
                break;

            case VP_TEST_ID_CONCLUDE:
                rtnval = Vp890TestConclude(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            case VP_TEST_ID_HYBRID_LOSS:
                rtnval = Vp890TestHybridLoss(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            default:
                rtnval = VP_STATUS_INVALID_ARG;
                VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestLineInt(%i): invalid tid", test));
                break;
        }
    } else {
        switch (test) {
            case VP_TEST_ID_PREPARE:
				msleep(1);
                pDevObj->currentTest.nonIntrusiveTest = FALSE;
                rtnval = Vp890TestPrepare(pLineCtx, handle, callback);
                break;

            case VP_TEST_ID_PREPARE_EXT:
				msleep(1);
                pDevObj->currentTest.nonIntrusiveTest = TRUE;
                rtnval = Vp890TestPrepareExt(pLineCtx, handle, callback);
                break;

            case VP_TEST_ID_CONCLUDE:
				msleep(1);
                rtnval = Vp890TestConclude(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            case VP_TEST_ID_CALIBRATE:
				msleep(1);
                rtnval = Vp890TestCalibrate(pLineCtx, handle, callback, test);
                break;

            case VP_TEST_ID_USE_LINE_CAL:
				msleep(1);
                pDevObj->calOffsets[pLineObj->channelId].nullOffset = 0;

                if ((pTestHeap->slacState & VP890_SS_POLARITY_MASK) == 0) {
                    pDevObj->calOffsets[pLineObj->channelId].vabOffset = \
                      pDevObj->vp890SysCalData.vocOffset[pLineObj->channelId][VP890_NORM_POLARITY] *
                      -1;
                } else {
                    pDevObj->calOffsets[pLineObj->channelId].vabOffset = \
                      pDevObj->vp890SysCalData.vocOffset[pLineObj->channelId][VP890_REV_POLARITY] *
                      -1;
                }

                if ((pTestHeap->slacState & VP890_SS_POLARITY_MASK) == 0) {
                    pDevObj->calOffsets[pLineObj->channelId].vahOffset = \
                        pDevObj->vp890SysCalData.vagOffsetNorm[pLineObj->channelId];
                } else {
                    pDevObj->calOffsets[pLineObj->channelId].vahOffset = \
                        pDevObj->vp890SysCalData.vagOffsetRev[pLineObj->channelId];
                }

                if ((pTestHeap->slacState & VP890_SS_POLARITY_MASK) == 0) {
                    pDevObj->calOffsets[pLineObj->channelId].vbhOffset = \
                        pDevObj->vp890SysCalData.vbgOffsetNorm[pLineObj->channelId];
                } else {
                    pDevObj->calOffsets[pLineObj->channelId].vbhOffset = \
                        pDevObj->vp890SysCalData.vbgOffsetRev[pLineObj->channelId];
                }

                pDevObj->calOffsets[pLineObj->channelId].valOffset = 0;
                pDevObj->calOffsets[pLineObj->channelId].vblOffset = 0;
                pDevObj->calOffsets[pLineObj->channelId].imtOffset = \
                    pDevObj->vp890SysCalData.ilaOffsetNorm[pLineObj->channelId];

                pDevObj->calOffsets[pLineObj->channelId].ilgOffset = 0;
                /*
                pDevObj->calOffsets[pLineObj->channelId].ilgOffset = \
                    pDevObj->vp890SysCalData.ilgOffsetNorm[pLineObj->channelId];
                */
                pDevObj->calOffsets[pLineObj->channelId].batOffset = \
                    pDevObj->vp890SysCalData.swyOffset[0];

                rtnval = VP_STATUS_SUCCESS;
                break;

            case VP_TEST_ID_OPEN_VXC:
				msleep(10);
                rtnval = Vp890TestOpenVxc(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            case VP_TEST_ID_AC_RLOOP:
				msleep(1);
                rtnval = Vp890TestAcRloop(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            case VP_TEST_ID_DC_RLOOP:
				msleep(1);				
                rtnval = Vp890TestDcRloop(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            case VP_TEST_ID_3ELE_RES_LG:
				msleep(1);			
                rtnval = Vp890Test3EleResLG(pLineCtx, pArgsUntyped, handle,
                        callback, test);
                break;

            case VP_TEST_ID_DELAY:
				msleep(1);
                rtnval = Vp890TestTimer(pLineCtx, pArgsUntyped, handle,
                        callback, test);
                break;

    #if (VP890_INCLUDE_LINE_TEST_PACKAGE == VP890_LINE_TEST_PROFESSIONAL)

            case VP_TEST_ID_3ELE_RES_HG:
				msleep(1);
                rtnval = Vp890Test3EleResHG(pLineCtx, pArgsUntyped, handle,
                        callback, test);
                break;

            case VP_TEST_ID_LOOP_CONDITIONS:
				msleep(1);
                rtnval = Vp890TestGetLoopCond(pLineCtx, pArgsUntyped, handle,
                        callback, test);
                break;

            case VP_TEST_ID_LOOPBACK:
				msleep(1);
                rtnval = Vp890TestLoopback(pLineCtx, pArgsUntyped, handle,
                        callback, test);
                break;

            case VP_TEST_ID_RAMP_INIT:
				msleep(1);
                rtnval = Vp890TestInitMetRamp(pLineCtx, pArgsUntyped, handle,
                        callback, test);
                break;

            case VP_TEST_ID_RAMP:
				msleep(1);
                rtnval = Vp890TestMetRamp(pLineCtx, pArgsUntyped, handle,
                        callback, test);
                break;

            case VP_TEST_ID_FLT_DSCRM:
				msleep(1);
                rtnval = Vp890TestFltDscrm(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            case VP_TEST_ID_3ELE_CAP_CSLAC:
				msleep(1);
                rtnval = Vp890Test3EleCap(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

            case VP_TEST_ID_MSOCKET_TYPE2:
				msleep(1);
                rtnval = Vp890TestMSocket(pLineCtx, pArgsUntyped, handle,
                    callback, test);
                break;

        case VP_TEST_ID_XCONNECT:
			msleep(1);
            rtnval = Vp890TestGetXConnect(pLineCtx, pArgsUntyped, handle,
                callback, test);
            break;

    #endif

            default:
                rtnval = VP_STATUS_INVALID_ARG;
                VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestLineInt(%i): invalid tid", test));
                break;
        }
    }

    return rtnval;
} /* Vp890TestLineInt() */

/*------------------------------------------------------------------------------
 * Vp890TestLineCallback()
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
Vp890TestLineCallback(
    VpLineCtxType *pLineCtx,
    VpPcmOperationResultsType *pResults)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890TestHeapType *pTestHeap = pDevObj->currentTest.pTestHeap;

    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp890TestLineCallback()+"));

    /* If the current test heap is null, there is no running test */
    if (NULL == pTestHeap) {
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestLineCallback(): no running tests"));
        VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp890TestLineCallback()-"));
        return VP_STATUS_FAILURE;
    }

    /*
     * If the PCMCollect routine was unsuccessful change the errorcode
     * to VP_TEST_STATUS_RESOURCE_NA and the device timer to expire on the
     * next tick
     */
    if (TRUE == pResults->error) {
        pDevObj->testResults.errorCode = VP_TEST_STATUS_INTERNAL_ERROR;
        Vp890SetTestStateAndTimer(pDevObj, &pDevObj->currentTest.testState,
            pDevObj->currentTest.testState, NEXT_TICK);
        pTestHeap->pcmRequest = FALSE;

        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestLineCallback(): VP_TEST_STATUS_INTERNAL_ERROR"));
        pTestHeap->pcmResults.error = TRUE;
        Vp890SetTestStateAndTimer(pDevObj, &pDevObj->currentTest.testState,
            VP890_TESTLINE_GLB_QUIT_STATE, NEXT_TICK);

        VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp890TestLineCallback()-"));
        return VP_STATUS_FAILURE;
    }
    /*
     * If the code gets here, hypotheticaly the current pcm data is for the
     * currently running test on the current line so save off the results
     * and set up the next test state.
     */
    VpMemCpy(&pTestHeap->pcmResults, pResults, (uint16)sizeof(VpPcmOperationResultsType));
    Vp890SetTestStateAndTimer(pDevObj, &pDevObj->currentTest.testState,
        pTestHeap->nextState, NEXT_TICK);
    VP_TEST_FUNC(VpLineCtxType, pLineCtx, ("Vp890TestLineCallback()-"));
    return VP_STATUS_SUCCESS;
} /* Vp890TestLineCallback() */

/*------------------------------------------------------------------------------
 * Vp890GenerateTestEvent()
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
Vp890GenerateTestEvent (
    VpLineCtxType *pLineCtx,
    VpTestIdType testId,
    uint16 handle,
    bool cleanUp,
    bool goToDisconnect)
{
    Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;

    pDevObj->testResults.testId = testId;
    pLineObj->lineEventHandle = handle;
    pLineObj->lineEvents.test |= VP_LINE_EVID_TEST_CMP;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890GenerateTestEvent(tid:%i,ec:%i,handle:%i)",
        pDevObj->testResults.testId, pDevObj->testResults.errorCode,
        pLineObj->lineEventHandle));

    /*
     * if a test needs to generate an event but does not want to disturb
     * a test already in progress, then these steps are skipped
     */
    if (TRUE == cleanUp) {
        VpDeviceIdType deviceId = pDevObj->deviceId;
        uint8 slacState = VP890_SS_DISCONNECT;

        VP_TEST(VpLineCtxType, pLineCtx, ("Vp890GenerateTestEvent(): cleanup"));

        Vp890InitTestHeap(pTestHeap);

        pDevObj->currentTest.testState = -1;
        pDevObj->currentTest.testId = VP_NUM_TEST_IDS;

        /* make sure that the device is in a non lethal feed state if requested */
        if (TRUE == goToDisconnect) {
            VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SYS_STATE_WRT,
                VP890_SYS_STATE_LEN, &slacState);
            pLineObj->slicValueCache = slacState;
        }
    }
    return;
} /* Vp890GenerateTestEvent() */

/*------------------------------------------------------------------------------
 * Vp890SetTestStateAndTimer
 * This function is sets up the test timer for the given testTime (ticks).
 * This function also changes the value of pState to testState. Almost
 * all of the Vp890 test primitives call this function at least once.
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
Vp890SetTestStateAndTimer(
    Vp890DeviceObjectType *pDevObj,
    int16 *pState,
    int16 testState,
    uint16 testTime)
{
    *pState = testState;
    pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] = testTime | VP_ACTIVATE_TIMER;
    return;
} /* Vp890SetTestStateAndTimer() */

/*------------------------------------------------------------------------------
 * Vp890StartPcmCollect
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
 * The alternate mode defined by VP890_EZ_MPI_PCM_COLLECT allows the API tick to do
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
EXTERN bool
Vp890StartPcmCollect (
    VpLineCtxType *pLineCtx,
    VpPcmOperationMaskType operationMask,
    int16 nextState,
    uint16 settlingTime,
    uint16 integrateTime)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890TestHeapType *pTestHeap = pDevObj->currentTest.pTestHeap;

#ifndef VP890_EZ_MPI_PCM_COLLECT
    Vp890LineObjectType *pLineObj = pLineCtx->pLineObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;

    uint8 chanId = pLineObj->channelId;
    int16 *ptestState = &(pDevObj->currentTest).testState;
    int16 timeoutTime = VP890_TOTAL_PCM_COLLECTION_TIME(integrateTime, settlingTime);
    uint8 startTimeSlot;

    /* get the current transmit timeslot */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_TX_TS_RD,
        VP890_TX_TS_LEN, &startTimeSlot);

    startTimeSlot &= VP890_TX_TS_MASK;

    /* make sure timeoutTime is atleast one ms */
    if (0 > timeoutTime) {
        timeoutTime = 1;
    }

    /* Set the error code incase the timeout happens */
    pDevObj->testResults.errorCode = VP_TEST_STATUS_TIMEOUT;

    /* Set the next state to be the quit/timeout state*/
    *ptestState = VP890_TESTLINE_GLB_QUIT_STATE;

    /* Setup the callback next state */
    pTestHeap->nextState = nextState;

    VpSysPcmCollectAndProcess(pLineCtx, deviceId, chanId, startTimeSlot, integrateTime,
        settlingTime, operationMask);
    /*
     * Setup the device timer to cause a test to quit if call back does not
     * happen in the calculated time frame
     */
    Vp890SetTestStateAndTimer(pDevObj, ptestState, VP890_TESTLINE_GLB_QUIT_STATE,
        MS_TO_TICKRATE(timeoutTime, pDevObj->devProfileData.tickRate));
    VP_TEST(VpLineCtxType, pLineCtx, ("VpStartPcmCollect(it:%i,st:%i,om:%i,to:%i): request",
        integrateTime, settlingTime, operationMask, timeoutTime));
    return TRUE;

#else
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    uint8 numSamples = 1;
    int16 newPcmSamples[8]; /* maximum of 7 16bit samples in preparation of
                             * the revision 3 VoicePort devices */
    uint8 *pTxBuffer = pDevObj->testDataBuffer;
    int i = 2;
    int j = 0;

    if (FALSE == pTestHeap->pcmRequest) {
        uint16 downSampleRate;
        pTestHeap->pcmRequest = TRUE;

        if (pLineObj->codec == VP_OPTION_WIDEBAND) {
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

    /* get the num samples from the first nibble of the buffer */
    numSamples = (pTxBuffer[0] >> 4) & 0x07;
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
            /* underflows */
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

    /* used RunningPcmCalculate() until ready to do callback */
    if (PCM_CAL_DONE == VpPcmCompute(newPcmSamples, numSamples,
        &pTestHeap->ezPcmTemp, &pTestHeap->pcmResults)){

        pTestHeap->pcmRequest = FALSE;
        pTestHeap->nextState = nextState;

        /* perform call back when data is ready */
        Vp890TestLineCallback(pLineCtx, &pTestHeap->pcmResults);

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
        pDevObj->devTimer[VP_DEV_TIMER_TESTLINE] = 1 | VP_ACTIVATE_TIMER;
        return FALSE;
    }
#endif
} /* Vp890StartPcmCollect() */


/*------------------------------------------------------------------------------
 * Vp890CommonTestSetup
 * This functions puts the current channel under test into a known state in order
 * to accomplish any of the test Primatives. Excluding Vp890TestPrepare and
 * Vp890TestConclude, this function should be run before attempting to take a
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
Vp890CommonTestSetup (
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId)
{
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    uint8 adcDefault = VP890_NO_CONNECT;
    uint8 sigGenAB[VP890_SIGA_PARAMS_LEN]
          = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8 sigGenCtrl = VP890_GEN_ALLOFF;
    uint8 opCond = (VP890_RXPATH_DIS | VP890_HIGH_PASS_DIS);
    uint8 opFunction = VP890_LINEAR_CODEC;
    uint8 disn = 0x00;
    uint8 gain = VP890_DEFAULT_VP_GAIN;

    uint8 ssCfg = (VP890_ACFS_MASK | VP890_AUTO_SSC_DIS);

    uint8 mpiBuffer[VP890_CONV_CFG_LEN+1 +
                     VP890_SIGA_PARAMS_LEN+1 +
                     VP890_GEN_CTRL_LEN+1 +
                     VP890_OP_COND_LEN+1 +
                     VP890_OP_FUNC_LEN+1 +
                     VP890_VP_GAIN_LEN+1 +
                     VP890_DISN_LEN+1 +
                     VP890_SS_CONFIG_LEN+1 +
                     VP890_ICR1_LEN+1 +
                     VP890_ICR2_LEN+1 +
                     VP890_ICR3_LEN+1 +
                     VP890_ICR4_LEN+1];
    uint8 mpiIndex = 0;

    pLineObj->icr1Values[0] = 0x00;
    pLineObj->icr1Values[1] = 0x00;
    pLineObj->icr1Values[2] &= 0xC0;
    pLineObj->icr1Values[3] &= 0xC0;

    pLineObj->icr2Values[0] = 0x0C;
    pLineObj->icr2Values[1] = 0x0C;
    pLineObj->icr2Values[2] &= 0x0C;
    pLineObj->icr2Values[3] &= 0x0C;

    pLineObj->icr3Values[0] = 0x00;
    pLineObj->icr3Values[1] = 0x00;
    pLineObj->icr3Values[2] = 0x00;
    pLineObj->icr3Values[3] = 0x00;

    pLineObj->icr4Values[0] = 0x91;
    pLineObj->icr4Values[1] = 0x01;
    pLineObj->icr4Values[2] = 0x00;
    pLineObj->icr4Values[3] = 0x00;

    /* Get the adc reg setting */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD, VP890_CONV_CFG_LEN, &adcDefault);
    adcDefault &= ~VP890_CONV_CONNECT_BITS;
    adcDefault |= VP890_NO_CONNECT;

    /*
     * FORCE ADC to metalic AC:
     * THIS is a required workaround. If the device is in idle and
     * the ADC is forced on in ICR4, the data from the ADC will be
     * railed. However, if the converter configuration register is
     * set to NO_CONNECT first, then this issue does not exist.
    */
    /* set ADC to NULL state */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_CONV_CFG_WRT,
        VP890_CONV_CFG_LEN, &adcDefault);

    /* blank out siggen A */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SIGA_PARAMS_WRT,
        VP890_SIGA_PARAMS_LEN, sigGenAB);

    /* Turn off sig gens */
    if (pLineObj->sigGenCtrl[0] != sigGenCtrl) {
        pLineObj->sigGenCtrl[0] = sigGenCtrl;
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer,
            VP890_GEN_CTRL_WRT, VP890_GEN_CTRL_LEN, pLineObj->sigGenCtrl);
    }

    /* Disable hi-pass filter and cutoff receive path */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_COND_WRT,
        VP890_OP_COND_LEN, &opCond);
    pLineObj->opCond[0] = opCond;

    /* Set to read linear data */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_OP_FUNC_WRT,
        VP890_OP_FUNC_LEN, &opFunction);

    /* Disable automatic state changes */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_VP_GAIN_WRT,
        VP890_VP_GAIN_LEN, &gain);

    /* Disable DISN */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_DISN_WRT,
        VP890_DISN_LEN, &disn);

    /* Disable automatic state changes */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_SS_CONFIG_WRT,
        VP890_SS_CONFIG_LEN, &ssCfg);

    /* Clear ICR1 mask bits */
    if (pLineObj->internalTestTermApplied == FALSE) {
        mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR1_WRT,
            VP890_ICR1_LEN, pLineObj->icr1Values);
    }

    /* Turn off feed voltage */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR2_WRT,
        VP890_ICR2_LEN, pLineObj->icr2Values);

    /* Modify longitudinal bias */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR3_WRT,
        VP890_ICR3_LEN, pLineObj->icr3Values);

    /* AISN & DAC met drive off, ADC on */
    mpiIndex = VpCSLACBuildMpiBuffer(mpiIndex, mpiBuffer, VP890_ICR4_WRT,
        VP890_ICR4_LEN, pLineObj->icr4Values);

    /* send down the mpi commands */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, mpiBuffer[0], mpiIndex-1, &mpiBuffer[1]);

    return;
}

/*------------------------------------------------------------------------------
 * Vp890AdcSetup
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
Vp890AdcSetup (
    VpLineCtxType *pLineCtx,
    uint8 adRoute,
    bool txAnalogGainEnable)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    VpDeviceIdType deviceId = pDevObj->deviceId;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;

    uint8 chanId = pLineObj->channelId;
    uint8 tempRxAngLoss[VP890_VP_GAIN_LEN];
    uint8 rxAngLoss = 0;
    uint8 txBufferDataRate = pDevObj->txBufferDataRate;

    uint8 chanSel[] = {VP890_DEV_MODE_CHAN0_SEL, VP890_DEV_MODE_CHAN1_SEL};

    /* error check the A to D routing value*/
    if ((adRoute > 15) || (0 == ((1 << adRoute) && VP890_LEGAL_A2D_CVRT_VALS))) {
        return FALSE;
    }
    adRoute |= txBufferDataRate;

    /* select the proper gain setting */
    if (TRUE == txAnalogGainEnable) {
        rxAngLoss = VP890_AX_MASK;
    }

    /* must redirect the pcm data from proper channel to pcmBuffer */
    pDevObj->devMode[0] = (chanSel[chanId] | VP890_DEV_MODE_TEST_DATA);
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DEV_MODE_WRT,
        VP890_DEV_MODE_LEN, pDevObj->devMode);

    /*
     * ADC workaround
     * 1) set adc to NULL
     * 2) wait for 5 noop transactions
     * 3) set adc to requested connection
     */

     /* no reason to do the work around if null was selected */
    if (VP890_NO_CONNECT != adRoute) {
        uint8 adcWorkAround[6] = {VP890_NO_CONNECT, VP890_NO_OP_WRT, VP890_NO_OP_WRT,
                VP890_NO_OP_WRT, VP890_NO_OP_WRT, VP890_NO_OP_WRT};

        adcWorkAround[0] |= txBufferDataRate;

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_WRT,
            6, adcWorkAround);
    }

    /* write the requested converter configuration adRouting */
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_WRT,
        VP890_CONV_CFG_LEN, &adRoute);

    /* read modify write the Voice Path Gain register with rxAngLoss input arg*/
    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_VP_GAIN_RD,
        VP890_VP_GAIN_LEN, tempRxAngLoss);

    /* some tests modify the AR bits so don't touch them */
    tempRxAngLoss[0] &= (uint8)(~(VP890_DR_LOSS_MASK | VP890_AX_MASK));
    tempRxAngLoss[0] |= rxAngLoss;

    VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_VP_GAIN_WRT,
        VP890_VP_GAIN_LEN, tempRxAngLoss);

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Vp890GetPcmResult()
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
Vp890GetPcmResult(
    Vp890DeviceObjectType *pDevObj,
    VpPcmOperationBitType pcmOperation,
    bool  txAnalogGainEnable,
    bool  removeOffset,
    uint8 chanId,
    uint8 aToDRoute,
    void *pResult)
{
    int16 *pResult16 = (int16*)pResult;
    int32 *pResult32 = (int32*)pResult;
    int32 temp;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = pTestInfo->pTestHeap;
    bool LI;

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
            VP_TEST(None, NULL, ("Invalid VpPcmOperationBitType Vp890GetPcmResult(0x%04x)", pcmOperation));
            pResult16 = 0;
            return;
    }

    /*
     * If the AX bit was enabled the measurment was doubled
     */
    if (TRUE == txAnalogGainEnable) {
        *pResult16 /= 2 ;
    }

    /* HARDWARE BUG: METALLIC VOLTAGE is inverted for devices after 890 REVC */
    if (VP890_METALLIC_DC_V == aToDRoute) {
        if ((pcmOperation & VP_PCM_OPERATION_AVERAGE) ||
          (pcmOperation & VP_PCM_OPERATION_MIN) ||
          (pcmOperation & VP_PCM_OPERATION_MAX)) {
            VP_TEST(None, NULL, ("Vp890GetPcmResult() INVERTING_SIGN"));
            *pResult16 = ((*pResult16 == VP_INT16_MIN) ? VP_INT16_MAX : -(*pResult16));
        }
    }

    /* remove calibration offset if requested */
    if (TRUE == removeOffset) {
        Vp890CalOffCoeffs *pCalOffsets = &pDevObj->calOffsets[chanId];
        int16 offset = 0;
        int16 result = *pResult16;

        switch (aToDRoute) {
            case VP890_METALLIC_DC_V:
                offset = (pCalOffsets->vabOffset) ? pCalOffsets->vabOffset :
                    (-1 * pDevObj->vp890SysCalData.vocOffset[chanId][VP890_NORM_POLARITY]);
                break;
            case VP890_TIP_TO_GND_V:
                offset = pCalOffsets->vahOffset;
                break;
            case VP890_RING_TO_GND_V:
                offset = pCalOffsets->vbhOffset;
                break;
            case VP890_METALLIC_DC_I:
                offset = pCalOffsets->imtOffset;
                break;
            case VP890_LONGITUDINAL_DC_I:
                offset = pCalOffsets->ilgOffset;
                break;
            case VP890_SWITCHER_Y:
                offset = pCalOffsets->batOffset;
                break;
            case VP890_NO_CONNECT:
                offset = pCalOffsets->nullOffset;
                break;
            default:
                break;
        }

        /* usefull to compute ILG */
        LI = pTestHeap->dcFeed[0] & VP890_LONG_IMP_MASK;

        if ((result == VP_INT16_MIN) || (result == VP_INT16_MAX)) {
            temp = (int32)result;
        } else {
            VP_TEST(None, NULL, ("Vp890GetPcmResult(): ADCRoute: %d, chanId: %d, REMOVING_OFFSET: %d, FROM: %d",
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
    }

    return;
}

/*------------------------------------------------------------------------------
 * Vp890IsLineReadyForTest()
 *
 * This function is only called by the Vp890TestLine function and
 * is determining that if the test specified by the test argument
 * can be run at this point in time.
 *
 * Parameters:
 *  pLineCtx - pointer to the line context
 *  test     - test Id of the test being requested to run.
 *
 * Returns:
 * An test status type error code indicating if the test Id can be run at
 * this time. If for any reason the test can not be run, then the function
 * will return something other than VP_TEST_STATUS_SUCCESS.
 *----------------------------------------------------------------------------*/
static VpTestStatusType
Vp890IsLineReadyForTest (
    VpLineCtxType *pLineCtx,
    VpTestIdType test)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;

    if (!pDevObj->currentTest.prepared) {
        if ((VP_TEST_ID_PREPARE == test) || (VP_TEST_ID_PREPARE_EXT == test)) {
            /*
             * VocicePort object does NOT have prepared line for testing,
             * and the test id requested is test prepare.
             */
            return VP_TEST_STATUS_SUCCESS;
        } else {
            /*
             * VoicePort object does NOT have prepared line for testing and
             * the current test id is not a test prepare so return not ready
             */
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890IsLineReadyForTest(): VP_TEST_STATUS_LINE_NOT_READY"));
            return VP_TEST_STATUS_LINE_NOT_READY;
        }
    } else {
        if ((VP_TEST_ID_PREPARE == test) || (VP_TEST_ID_PREPARE_EXT == test) || (pDevObj->currentTest.preparing == TRUE)) {
            /*
             * VocicePort object has a prepared line but the current test id
             * is to run prepare again so return busy prevents user from
             * accidently storing testing coeffs.
             */
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890IsLineReadyForTest(1): VP_TEST_STATUS_RESOURCE_NA"));
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
        VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890IsLineReadyForTest(2): VP_TEST_STATUS_RESOURCE_NA"));
        return VP_TEST_STATUS_RESOURCE_NA;
    } else {
        if ((VP_TEST_ID_CONCLUDE != test) && (0 <= pDevObj->currentTest.testState)) {
            /*
             * The requested test is not conclude and the device is busy
             * with another test.
             */
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890IsLineReadyForTest(): VP_TEST_STATUS_TESTING"));
            return VP_TEST_STATUS_TESTING;
        }
    }
    return VP_TEST_STATUS_SUCCESS;
} /* Vp890IsLineReadyForTest() */

/*------------------------------------------------------------------------------
 * Vp890TestPrepare
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
static VpStatusType
Vp890TestPrepare (
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = NULL;
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
            Vp890GenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestPrepare(ec:%i): bailed",
                pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;
        }

        /* try to get a test heap */
        pTestInfo->pTestHeap =
            (Vp890TestHeapType*)VpSysTestHeapAcquire(&pTestInfo->testHeapId);

        VP_TEST(VpLineCtxType, pLineCtx, ("VpSysTestHeapAcquire()"));

        if (VP_NULL == pTestInfo->pTestHeap) {
            /* if a heap is not available then generate an event */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
            Vp890GenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestPrepare(heap:NULL): bailed"));
            return VP_STATUS_SUCCESS;
        }

        /* setup the test heap */
        Vp890InitTestHeap(pTestInfo->pTestHeap);

        /* set the necessary flags to indicate the readiness of the test code */
        pTestInfo->testId = VP_TEST_ID_PREPARE;
        pTestInfo->handle = handle;
        pTestInfo->prepared = TRUE;
        pDevObj->currentTest.preparing = TRUE;
        pDevObj->currentTest.channelId = chanId;

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

            Vp890SetTestStateAndTimer(pDevObj, pTestState, VP890_TESTLINE_GLB_STRT_STATE, NEXT_TICK*2);

            return VP_STATUS_SUCCESS;
        }
    }

    /*
     * If this is a low power device, we must wait until we are completely
     * out of low power mode before we start storing register values.
     * This conditional continues to read the low power change timer
     * until it is no longer active.
     */

    if ((*pTestState == VP890_TESTLINE_GLB_STRT_STATE) &&
        ((VP_TERM_FXS_LOW_PWR == pLineObj->termType) ||
        (VP_TERM_FXS_ISOLATE_LP == pLineObj->termType) ||
        (VP_TERM_FXS_SPLITTER_LP == pLineObj->termType)) ) {

        if (pDevObj->devTimer[VP_DEV_TIMER_LP_CHANGE] & VP_ACTIVATE_TIMER) {
            *pTestState = VP890_TESTLINE_GLB_STRT_STATE;
        } else {
            *pTestState = VP890_TESTLINE_GLB_END_STATE;
        }
        Vp890SetTestStateAndTimer(pDevObj, pTestState, *pTestState, NEXT_TICK*2);
        return VP_STATUS_SUCCESS;
    }



    pTestHeap = pTestInfo->pTestHeap;

    /* save all registers that may be modified by any test primative */
    if (!(pLineObj->status & VP890_IS_FXO)) {
        /* Line is FXS */
        /* Save registers as necessary for FXS tests */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_FUNC_RD,
            VP890_OP_FUNC_LEN, &pTestHeap->opFunction);

        pTestHeap->opCond = pLineObj->opCond[0];

        pTestHeap->sigCtrl = pLineObj->sigGenCtrl[0];

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SS_CONFIG_RD,
            VP890_SS_CONFIG_LEN, &pTestHeap->sysConfig);

        pTestHeap->slacState = pLineObj->slicValueCache;

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_VP_GAIN_RD,
            VP890_VP_GAIN_LEN, &pTestHeap->vpGain);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DISN_RD,
            VP890_DISN_LEN, &pTestHeap->disn);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DC_FEED_RD,
            VP890_DC_FEED_LEN, pTestHeap->dcFeed);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_B1_FILTER_RD,
            VP890_B1_FILTER_LEN, pTestHeap->b1Filter);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_B2_FILTER_RD,
            VP890_B2_FILTER_LEN, pTestHeap->b2Filter);

        VpMemCpy(pTestHeap->icr1, pLineObj->icr1Values, VP890_ICR1_LEN);
        VpMemCpy(pTestHeap->icr2, pLineObj->icr2Values, VP890_ICR2_LEN);
        VpMemCpy(pTestHeap->icr3, pLineObj->icr3Values, VP890_ICR3_LEN);
        VpMemCpy(pTestHeap->icr4, pLineObj->icr4Values, VP890_ICR4_LEN);
        VpMemCpy(pTestHeap->icr6, pLineObj->dcCalValues, VP890_DC_CAL_REG_LEN);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_LOOP_SUP_RD,
            VP890_LOOP_SUP_LEN, pTestHeap->lpSuper);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SIGA_PARAMS_RD,
            VP890_SIGA_PARAMS_LEN, pTestHeap->sigGenAB);

        VpMemCpy(pTestHeap->switchReg, pDevObj->swParamsCache,
            VP890_REGULATOR_PARAM_LEN);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_REGULATOR_TIMING_RD,
            VP890_REGULATOR_TIMING_LEN, pTestHeap->switchTimes);

        /* Enable the line control block circuitry to have accurate voltage measurments in disconnect */
        pLineObj->icr3Values[0] = pTestHeap->icr3[0] | 0x20;
        pLineObj->icr3Values[1] = pTestHeap->icr3[1] | 0x20;
        pLineObj->icr3Values[2] = pTestHeap->icr3[2] | 0x00;
        pLineObj->icr3Values[3] = pTestHeap->icr3[3] | 0x00;
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);

        /*
         * the following code is a workaround preventing the storing of
         * the opFunction when the slic is in the ringing state.
         */
        pTestHeap->opFunction |= VP890_ENABLE_LOADED_COEFFICIENTS;
        /* Enable the desired CODEC mode */
        switch (pLineObj->codec) {
            case VP_OPTION_LINEAR:      /* 16 bit linear PCM */
            case VP_OPTION_WIDEBAND:    /* Wideband asumes Linear PCM */
                pTestHeap->opFunction |= VP890_LINEAR_CODEC;
                pTestHeap->opFunction &= ~(VP890_ULAW_CODEC);
                break;

            case VP_OPTION_ALAW:                /* A-law PCM */
                pTestHeap->opFunction &= (uint8)(~(VP890_LINEAR_CODEC | VP890_ULAW_CODEC));
                break;

            case VP_OPTION_MLAW:                /* u-law PCM */
                pTestHeap->opFunction |= VP890_ULAW_CODEC;
                pTestHeap->opFunction &= ~(VP890_LINEAR_CODEC);
                break;

            default:
                /* Cannot reach here.  Error checking at top */
                break;
        } /* Switch */

    } else {
        /* Line is FXO */
        /* Save registers as necessary for FXO tests */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_FUNC_RD,
            VP890_OP_FUNC_LEN, &pTestHeap->opFunction);

        pTestHeap->opCond = pLineObj->opCond[0];

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD,
            VP890_CONV_CFG_LEN, &pTestHeap->convCfg);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_MODE_RD,
            VP890_OP_MODE_LEN, &pTestHeap->opMode);
    }

    /* save off current event mask */
    pTestHeap->preTestEventMask = pLineObj->lineEventsMask;

    /* mask off all events (except VP_LINE_EVID_TEST_CMP) until test conclude */
    pLineObj->lineEventsMask.faults = 0xFFFF;
    pLineObj->lineEventsMask.signaling = 0xFFFF;
    pLineObj->lineEventsMask.response = 0xFFFF;
    pLineObj->lineEventsMask.test = ~(VP_LINE_EVID_TEST_CMP | VP_LINE_EVID_ABORT);
    pLineObj->lineEventsMask.process = 0xFFFF;
    pLineObj->lineEventsMask.fxo = 0xFFFF;

    pDevObj->testResults.errorCode = VP_TEST_STATUS_SUCCESS;
    Vp890GenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE, handle, TRUE, FALSE);

    /* indication that we are no longer preparing the device */
    pDevObj->currentTest.preparing = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestPrepare(): complete"));
    return VP_STATUS_SUCCESS;
} /* Vp890TestPrepare() */

/*------------------------------------------------------------------------------
 * Vp890TestPrepareExt
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
static VpStatusType
Vp890TestPrepareExt (
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback)
{
    VpDevCtxType *pDevCtx = pLineCtx->pDevCtx;
    Vp890DeviceObjectType *pDevObj = pDevCtx->pDevObj;
    Vp890LineObjectType *pLineObj =  pLineCtx->pLineObj;
    Vp890CurrentTestType *pTestInfo = &pDevObj->currentTest;
    Vp890TestHeapType *pTestHeap = NULL;
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
            Vp890GenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE_EXT, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestPrepareExt(ec:%i): bailed",
                pDevObj->testResults.errorCode));
            return VP_STATUS_SUCCESS;
        }

        /* try to get a test heap */
        pTestInfo->pTestHeap =
            (Vp890TestHeapType*)VpSysTestHeapAcquire(&pTestInfo->testHeapId);

        VP_TEST(VpLineCtxType, pLineCtx, ("VpSysTestHeapAcquire()"));

        if (VP_NULL == pTestInfo->pTestHeap) {
            /* if a heap is not available then generate an event */
            pDevObj->testResults.errorCode = VP_TEST_STATUS_RESOURCE_NA;
            Vp890GenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE_EXT, handle, FALSE, FALSE);
            VP_WARNING(VpLineCtxType, pLineCtx, ("Vp890TestPrepareExt(heap:NULL): bailed"));
            return VP_STATUS_SUCCESS;
        }

        /* setup the test heap */
        Vp890InitTestHeap(pTestInfo->pTestHeap);

        /* set the necessary flags to indicate the readiness of the test code */
        pTestInfo->testId = VP_TEST_ID_PREPARE_EXT;
        pTestInfo->handle = handle;
        pTestInfo->prepared = TRUE;
        pDevObj->currentTest.preparing = TRUE;
        pDevObj->currentTest.channelId = chanId;

    }
    pTestHeap = pTestInfo->pTestHeap;

    /* save all registers that may be modified by any test primative */
    if (!(pLineObj->status & VP890_IS_FXO)) {
        /* Line is FXS */
        /* Save registers as necessary for FXS tests */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_FUNC_RD,
            VP890_OP_FUNC_LEN, &pTestHeap->opFunction);

        pTestHeap->opCond = pLineObj->opCond[0];
        pTestHeap->sigCtrl = pLineObj->sigGenCtrl[0];

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SS_CONFIG_RD, VP890_SS_CONFIG_LEN,
            &pTestHeap->sysConfig);

        pTestHeap->slacState = pLineObj->slicValueCache;

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_VP_GAIN_RD, VP890_VP_GAIN_LEN,
            &pTestHeap->vpGain);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DISN_RD, VP890_DISN_LEN,
            &pTestHeap->disn);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_DC_FEED_RD, VP890_DC_FEED_LEN,
            pTestHeap->dcFeed);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_B1_FILTER_RD,
            VP890_B1_FILTER_LEN, pTestHeap->b1Filter);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_B2_FILTER_RD,
            VP890_B2_FILTER_LEN, pTestHeap->b2Filter);

        VpMemCpy(pTestHeap->icr1, pLineObj->icr1Values, VP890_ICR1_LEN);
        VpMemCpy(pTestHeap->icr2, pLineObj->icr2Values, VP890_ICR2_LEN);
        VpMemCpy(pTestHeap->icr3, pLineObj->icr3Values, VP890_ICR3_LEN);
        VpMemCpy(pTestHeap->icr4, pLineObj->icr4Values, VP890_ICR4_LEN);
        VpMemCpy(pTestHeap->icr6, pLineObj->dcCalValues, VP890_DC_CAL_REG_LEN);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_LOOP_SUP_RD,
            VP890_LOOP_SUP_LEN, pTestHeap->lpSuper);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_SIGA_PARAMS_RD,
            VP890_SIGA_PARAMS_LEN, pTestHeap->sigGenAB);

        VpMemCpy(pTestHeap->switchReg, pDevObj->swParamsCache,
            VP890_REGULATOR_PARAM_LEN);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_REGULATOR_TIMING_RD,
            VP890_REGULATOR_TIMING_LEN, pTestHeap->switchTimes);

        /* Enable the line control block circuitry to have accurate voltage measurments in disconnect */
        pLineObj->icr3Values[0] = pTestHeap->icr3[0] | 0x20;
        pLineObj->icr3Values[1] = pTestHeap->icr3[1] | 0x20;
        pLineObj->icr3Values[2] = pTestHeap->icr3[2] | 0x00;
        pLineObj->icr3Values[3] = pTestHeap->icr3[3] | 0x00;
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_ICR3_WRT, VP890_ICR3_LEN, pLineObj->icr3Values);

        /*
         * the following code is a workaround preventing the storing of
         * the opFunction when the slic is in the ringing state.
         */
        pTestHeap->opFunction |= VP890_ENABLE_LOADED_COEFFICIENTS;
        /* Enable the desired CODEC mode */
        switch (pLineObj->codec) {
            case VP_OPTION_LINEAR:      /* 16 bit linear PCM */
            case VP_OPTION_WIDEBAND:    /* Wideband asumes Linear PCM */
                pTestHeap->opFunction |= VP890_LINEAR_CODEC;
                pTestHeap->opFunction &= ~(VP890_ULAW_CODEC);
                break;

            case VP_OPTION_ALAW:                /* A-law PCM */
                pTestHeap->opFunction &= (uint8)(~(VP890_LINEAR_CODEC | VP890_ULAW_CODEC));
                break;

            case VP_OPTION_MLAW:                /* u-law PCM */
                pTestHeap->opFunction |= VP890_ULAW_CODEC;
                pTestHeap->opFunction &= ~(VP890_LINEAR_CODEC);
                break;

            default:
                /* Cannot reach here.  Error checking at top */
                break;
        } /* Switch */

    } else {
        /* Line is FXO */
        /* Save registers as necessary for FXO tests */
        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_FUNC_RD,
            VP890_OP_FUNC_LEN, &pTestHeap->opFunction);

        pTestHeap->opCond = pLineObj->opCond[0];

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_CONV_CFG_RD,
            VP890_CONV_CFG_LEN, &pTestHeap->convCfg);

        VpMpiCmdWrapper(deviceId, pLineObj->ecVal, VP890_OP_MODE_RD,
            VP890_OP_MODE_LEN, &pTestHeap->opMode);
    }

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
    Vp890GenerateTestEvent(pLineCtx, VP_TEST_ID_PREPARE_EXT, handle, TRUE, FALSE);

    /* indication that we are no longer preparing the device */
    pDevObj->currentTest.preparing = FALSE;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp890TestPrepareExt(): complete"));
    return VP_STATUS_SUCCESS;
} /* Vp890TestPrepareExt() */

/*------------------------------------------------------------------------------
 * Vp890InitTestHeap
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
static void
Vp890InitTestHeap(
    Vp890TestHeapType *pTestHeap)
{
    pTestHeap->adcState = 0;
    pTestHeap->nextState = VP890_TESTLINE_GLB_IVLD_STATE;
    pTestHeap->pcmRequest = FALSE;

    return;
} /* Vp890InitTestHeap() */

/*------------------------------------------------------------------------------
 * Vp890ReturnElapsedTime
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
Vp890ReturnElapsedTime (
    Vp890DeviceObjectType *pDevObj,
    uint16 timeStamp)
{
    uint16 timeStampCounter = pDevObj->timeStamp / 2;

    if (timeStamp > timeStampCounter) {
        return (0x7FFF - timeStamp + timeStampCounter);
    } else {
        return (timeStampCounter - timeStamp);
    }
}

#endif /* VP890_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_890_SERIES */
