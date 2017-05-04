/** \lt_api_Vp886.c
 * lt_api_Vp886.c
 *
 * This file contains the implementation top level LT-API application interface
 * for VCP device family.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11110 $
 * $LastChangedDate: 2013-08-01 17:59:58 -0500 (Thu, 01 Aug 2013) $
 */

#include "lt_api.h"

#if defined(LT_VP886_PACKAGE)

#include "lt_api_internal.h"

/* Internal Function Declaration */

static bool
Vp886CheckDevAndChanId(
    VpLineCtxType *pLineCtx,
    LtEventType *pEvent);

static LtTestStatusType
Vp886CleanUpAfterTestFailure(
    LtTestCtxType *pTestCtx);

static void
Vp886HouseKeeping(
    LtTestCtxType *pTestCtx);

static LtTestStatusType
Vp886TestInit(
    LtTestCtxType *pTestCtx,
    LtTestIdType testId);

static bool
Vp886IsTestSupported(
    LtTestCtxType *pTestCtx,
    VpDeviceInfoType *pDeviceInfo,
    VpTermType termType,
    LtTestIdType testId);

static bool
Vp886AttributeCheck(
    LtTestCtxType *pTestCtx,
    const LtTestIdType testId);


extern LtTestStatusType
LtVp886StartTest(
    VpLineCtxType *pLineCtx,              /* VP-API line context */
    LtTestIdType testId,                  /* LT TestId that is requested */
    uint16 handle,                        /* Handle to be used for the
                                           * VP-API */
    LtTestAttributesType *pAttributes,     /* Test attributes */
    LtTestTempType *pTemp,                /* Pointer to where temporary test
                                           * data is stored */
    LtTestResultType *pResult,            /* Pointer to where results need to
                                           * be stored */
    LtTestCtxType *pTestCtx)              /* LT test context pointer */
{

    VpDeviceInfoType devInfo;
    VpLineInfoType lineInfo;
    VpStatusType vpStatus;
    LtTestStatusType returnVal = LT_STATUS_ERROR_UNKNOWN;

    LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()+"));

    /* pTemp cannot be NULL */
    if (pTemp == NULL) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp886StartTest() : pTemp is NULL"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    /* Checking for legal test id */
    if ( (testId < 0) || (testId >= LT_NUM_TID_CODES) ) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp886StartTest(testId:%i) : invalid", testId));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : Initializing TestCtx struct"));
    /* Initialize the test context. */
    pTestCtx->testId = testId;
    pTestCtx->state = LT_GLOBAL_FIRST_STATE;
    pTestCtx->handle = handle;
    pTestCtx->pLineCtx = pLineCtx;
    pTestCtx->pAttributes = pAttributes;
    pTestCtx->pTemp = pTemp;
    pTestCtx->pResult = pResult;
    pTestCtx->pResult->testId = testId;
    pTestCtx->testFailed = FALSE;
    pTestCtx->tempStatus = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : Initializing Temp struct"));
    pTestCtx->pTemp->vp886Temp.internalTest = FALSE;
    pTestCtx->pTemp->vp886Temp.resources.testNotConcluded = TRUE;
    pTestCtx->pTemp->vp886Temp.resources.testAborted = FALSE;
    pTestCtx->pTemp->vp886Temp.attributes.pInputs = NULL;
    pTestCtx->pTemp->vp886Temp.attributes.pCriteria = NULL;
    pTestCtx->pTemp->vp886Temp.revCode = 0;
    pTestCtx->pTemp->vp886Temp.productCode = 0;

    /* default topology */
    pTestCtx->pTemp->vp886Temp.attributes.topology.rSenseA =  VP886_TOP_R_SENSE;
    pTestCtx->pTemp->vp886Temp.attributes.topology.rSenseB =  VP886_TOP_R_SENSE;
    pTestCtx->pTemp->vp886Temp.attributes.topology.rLeakageA =  VP886_TOP_R_LEAKAGE;
    pTestCtx->pTemp->vp886Temp.attributes.topology.rLeakageB =  VP886_TOP_R_LEAKAGE;

    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : Check for Null attributes"));
    if (VP_NULL != pTestCtx->pAttributes) {
        /* Checking for any NULL pointer in the appropriate unions */
        pTestCtx->pTemp->vp886Temp.attributes.pInputs =
            pTestCtx->pAttributes->inputs.pLineVInp;

        pTestCtx->pTemp->vp886Temp.attributes.pCriteria =
            pTestCtx->pAttributes->criteria.pLintVCrt;

        if (pTestCtx->pAttributes->topology.pVp886Topology != VP_NULL) {
            LtMemCpy(&pTestCtx->pTemp->vp886Temp.attributes.topology,
                pTestCtx->pAttributes->topology.pVp886Topology,
                sizeof(LtVp886TestTopologyType));
        }
    }

    lineInfo.pLineCtx = pLineCtx;
    lineInfo.pDevCtx = NULL;
    lineInfo.channelId = 0;
    lineInfo.termType = 0;

    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : GetLineInfo"));
    vpStatus = VpGetLineInfo(&lineInfo);
    pTestCtx->pTemp->vp886Temp.termType = lineInfo.termType;
    pTestCtx->pTemp->vp886Temp.channelId = lineInfo.channelId;

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtVp886StartTest() : VpGetLineInfo() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : GetLineState"));
    vpStatus = VpGetLineState(pTestCtx->pLineCtx,
        &pTestCtx->pTemp->vp886Temp.resources.initialLineState);

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtVp886StartTest() : VpGetLineState() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    /* Only support testing on the following terminations */
    if ((lineInfo.termType != VP_TERM_FXS_GENERIC) &&
        (lineInfo.termType != VP_TERM_FXS_LOW_PWR)) {

        pTestCtx->pResult->vpGeneralErrorCode = VP_STATUS_ERR_VTD_CODE;
        LT_DOUT(LT_DBG_ERRR, ("LtVp886StartTest(%i) : invalid termType",
            lineInfo.termType));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : GetDeviceInfo"));
    devInfo.pLineCtx = pLineCtx;
    devInfo.pDevCtx = NULL;
    devInfo.deviceId = 0;
    devInfo.deviceType = 0;
    devInfo.numLines =0;
    vpStatus = VpGetDeviceInfo(&devInfo);
    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtVp886StartTest() : VpGetDeviceInfo() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    /* Store the rev code and product code */
    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : revcode"));
    pTestCtx->pTemp->vp886Temp.revCode = devInfo.revCode;
    pTestCtx->pTemp->vp886Temp.productCode = devInfo.productCode;
    pTestCtx->pTemp->vp886Temp.deviceType = devInfo.deviceType;

    LT_DOUT(LT_DBG_INFO, ("LtVp886StartTest() : is test supported"));
    if (Vp886IsTestSupported(pTestCtx, &devInfo, lineInfo.termType, testId) == FALSE) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp886StartTest() : IsTestSupported() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));
        return LT_STATUS_TEST_NOT_SUPPORTED;
    }
    pTestCtx->termType = lineInfo.termType;

    /* Run the generic initialization function */
    returnVal = Vp886TestInit(pTestCtx, testId);
    LT_DOUT(LT_DBG_FUNC, ("LtVp886StartTest()-"));

    return returnVal;
} /* LtVp886StartTest() */

extern LtTestStatusType
LtVp886EventHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestIdType testId;
    LtTestTempType *pTemp;
    LtTestStatusType retval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_FUNC, ("LtVp886EventHandler()+"));

    /* Very basic error checking */
    if((pTestCtx == NULL) || (pEvent == NULL)) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp886EventHandler(pTestCtx=NULL|pEvent=NULL) : "\
            "invalid args"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886EventHandler()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    testId = pTestCtx->testId;

    if ((0 > testId) || (LT_NUM_TID_CODES <= testId )) {
        /* invalid Test ID */
        LT_DOUT(LT_DBG_ERRR, ("LtVp886EventHandler(testId:%i) : invalid", testId));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886EventHandler()-"));
        retval = LT_STATUS_ERROR_INVALID_ARG;

    } else if(((!Vp886CheckDevAndChanId(pTestCtx->pLineCtx, pEvent) ) ||
         (pEvent->pVpEvent->eventCategory !=
            pTestCtx->pTemp->vp886Temp.expected.eventCat) ||
         (pEvent->pVpEvent->eventId != pTestCtx->pTemp->vp886Temp.expected.eventId))) {

        /* Ignore all events that are not related to the line under test or do
         * not match the expected event type.
         */
        LT_DOUT(LT_DBG_INFO, ("LtVp886EventHandler(cat:0x%04x,id:0x%04x,data:0x%04x) : "\
            "ignoring event", pEvent->pVpEvent->eventCategory,
            pEvent->pVpEvent->eventId,
            pEvent->pVpEvent->eventData));

        retval = LT_STATUS_RUNNING;

    } else if (pTestCtx->pTemp->vp886Temp.resources.testAborted == TRUE) {
        /* Checking if the abort test command was called */

        LT_DOUT(LT_DBG_WARN, ("LtVp886EventHandler(testAborted:TRUE,eventId:%i) : "\
            ": Test Aborted", pEvent->pVpEvent->eventId));

        if ( (pEvent->pVpEvent->eventId == VP_LINE_EVID_TEST_CMP) &&
             (pEvent->pVpEvent->eventData == VP_TEST_ID_CONCLUDE) ) {
            /* Checking if the event is a conclude */
            retval = LT_STATUS_ABORTED;
        } else {
            /* if it is not then we need to call conclude */
            retval = Vp886CleanUpAfterTestFailure(pTestCtx);
        }

    } else {
        if(pTestCtx->testFailed == FALSE) {
            /* Execute the appropriate test-specific event handler. */
            LT_DOUT(LT_DBG_INFO, ("LtVp886EventHandler(cat:0x%04x,id:0x%04x,data:%i)",
                pEvent->pVpEvent->eventCategory,
                pEvent->pVpEvent->eventId,
                pEvent->pVpEvent->eventData));

            /* Check for any VP-API or LT_API errors */
            if (Vp886EventErrorCheck(pTestCtx, pEvent, &retval) == FALSE) {

                /* continue calling the Lt886 test specific event handler until it indicates otherwise. */
                while (pTestCtx->Lt886TestEventHandlerFunc(pTestCtx, pEvent, &retval) && (retval == LT_STATUS_RUNNING));
            }
        } else {
            if(pTestCtx->pTemp->vp886Temp.resources.testNotConcluded == FALSE) {
                /* LT test had failed and the last action that was taken was
                 * test conclude. Return the reasons why test failed */
                retval = pTestCtx->tempStatus;
                LT_DOUT(LT_DBG_WARN, ("LtVp886EventHandler(testFailed:TRUE,testId:%i)"\
                    ": Normal Conclude", testId));
            } else {
                /* Clean up after a test concludes too early*/
                LT_DOUT(LT_DBG_WARN, ("LtVp886EventHandler(testFailed:TRUE,testId:%i)"\
                    ": Abnormally Concluding", testId));
                retval = Vp886CleanUpAfterTestFailure(pTestCtx);
            }
        }
    }

    /* Forcibly execute a Test Conclude if any error detected. */
    if(pTestCtx->testFailed == FALSE) {
        if ((retval != LT_STATUS_RUNNING) &&
            (retval != LT_STATUS_DONE) &&
            (retval != LT_STATUS_ABORTED)) {
            /*
             * Even though test failed
             * we can not return from the test since there is still one event
             * pending as consequence of the call to test conclude that
             * is about to happen.
             * The test now needs to wait all of actions result in appropriate
             * events; So store the reason why test failed temporarily and
             * clean up the test.
             */

            pTestCtx->testFailed = TRUE;
            pTestCtx->tempStatus = retval;

            /* Store a state for which none of the test does nothing */
            pTestCtx->state = LT_GLOBAL_ABORTED_STATE;
            LT_DOUT(LT_DBG_ERRR, ("LtVp886EventHandler(testFailed:FALSE,retval:%i)"\
                ": *** Forcing a Conclude ***", retval));

            /* Perform necessary cleanup */
            retval = Vp886CleanUpAfterTestFailure(pTestCtx);
        }
    }

    if (retval == LT_STATUS_DONE) {
        /* Test is done; Make sure test has not locked up any resource */
        pTemp = pTestCtx->pTemp;

        if (pTemp->vp886Temp.resources.testNotConcluded == TRUE) {
            /* This error is not expected to happen */
            LT_DOUT(LT_DBG_ERRR, ("LtVp886EventHandler(LT_STATUS_DONE) :"\
                "Resource NOT release"));

            retval = LT_STATUS_ERROR_UNKNOWN;
        }
    }

    /*
     * If the chanid and deviceId are correct and
     * If the eventId is expected and the eventCat is expected and
     * If the current eventData = VP_TEST_ID_CONCLUDE we need to any
     * last second clean up
     */
    if( Vp886CheckDevAndChanId(pTestCtx->pLineCtx, pEvent) &&
        (pEvent->pVpEvent->eventCategory ==  pTestCtx->pTemp->vp886Temp.expected.eventCat) &&
        (pEvent->pVpEvent->eventId == pTestCtx->pTemp->vp886Temp.expected.eventId) &&
        (pEvent->pVpEvent->eventData == VP_TEST_ID_CONCLUDE)) {
        Vp886HouseKeeping(pTestCtx);
    }

    LT_DOUT(LT_DBG_FUNC, ("LtVp886EventHandler()-"));
    return retval;
} /* LtVp886EventHandler() */

extern LtTestStatusType
LtVp886AbortTest(
    LtTestCtxType *pTestCtx)
{
    LtTestStatusType retval;

    LT_DOUT(LT_DBG_FUNC, ("LtVp886AbortTest()+"));
    /* Very basic error checking */
    if(pTestCtx == NULL) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp886AbortTest(pTestCtx=NULL) : invalid args"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp886AbortTest()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    if (pTestCtx->pTemp->vp886Temp.resources.testNotConcluded == FALSE) {
        /* no test is running so just return Abort indication */
        retval = LT_STATUS_DONE;
        LT_DOUT(LT_DBG_FUNC, ("LtVp886AbortTest() test was already done"));
    } else {
        /* a test is running so set the abort flag and return running */
        pTestCtx->pTemp->vp886Temp.resources.testAborted = TRUE;
        retval = LT_STATUS_RUNNING;
        LT_DOUT(LT_DBG_FUNC, ("LtVp886AbortTest() starting abort test"));
    }
    LT_DOUT(LT_DBG_FUNC, ("LtVp886AbortTest()-"));
    return retval;
} /* LtVp886AbortTest() */

extern LtTestStatusType
Vp886TestLineWrapper(
    LtTestCtxType *pTestCtx,
    VpTestIdType testId,
    const void * pArgs)
{
    VpStatusType vpStatus;

    LtTestTempType *pTemp = pTestCtx->pTemp;

    /* Save the currently executing test for later error checking. */
    pTemp->vp886Temp.expected.eventCat = VP_EVCAT_TEST;
    pTemp->vp886Temp.expected.eventId = VP_LINE_EVID_TEST_CMP;
    pTemp->vp886Temp.expected.thing.testId = testId;

    vpStatus = VpTestLine(pTestCtx->pLineCtx, testId, pArgs, pTestCtx->handle);

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("Vp886TestLineWrapper(vpTestId:%i) : 0x%04x", testId, vpStatus));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    /* Set various resource useage flags */
    if(testId == VP_TEST_ID_PREPARE) {
        pTemp->vp886Temp.resources.testNotConcluded = TRUE;

    } else if(testId == VP_TEST_ID_CONCLUDE) {
        pTemp->vp886Temp.resources.testNotConcluded = FALSE;

    }
    LT_DOUT(LT_DBG_INFO, ("Vp886TestLineWrapper(vpTestId:%i) : VP_STATUS_SUCCESS", testId));

    return LT_STATUS_RUNNING;
} /* Vp886TestLineWrapper() */

extern LtTestStatusType
Vp886SetRelayStateWrapper(
    LtTestCtxType *pTestCtx,
    VpRelayControlType relayState)
{
    VpStatusType vpStatus;
    vpStatus = VpSetRelayState(pTestCtx->pLineCtx, relayState);

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("Vp886SetRelayStateWrapper(relayState:0x%04x) : 0x%04x", relayState, vpStatus));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    LT_DOUT(LT_DBG_INFO, ("Vp886SetRelayStateWrapper(relayState:0x%04x) : VP_STATUS_SUCCESS", relayState));

    return LT_STATUS_RUNNING;

} /* Vp886SetRelayStateWrapper */

extern LtTestStatusType
Vp886SetLineStateWrapper(
    LtTestCtxType *pTestCtx,
    VpLineStateType lineState)
{
    VpStatusType vpStatus;
    vpStatus = VpSetLineState(pTestCtx->pLineCtx, lineState);

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("Vp886SetLineStateWrapper(lineState:0x%04x) : 0x%04x",lineState, vpStatus));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    LT_DOUT(LT_DBG_INFO, ("Vp886SetLineStateWrapper(lineState:0x%04x) : VP_STATUS_SUCCESS", lineState));

    return LT_STATUS_RUNNING;

} /* Vp886SetLineStateWrapper */

extern LtTestStatusType
Vp886GetLineStatusWrapper(
    LtTestCtxType *pTestCtx,
    VpInputType input,
    bool *pLineStatus)
{
    VpStatusType vpStatus;
    vpStatus = VpGetLineStatus(pTestCtx->pLineCtx, input, pLineStatus);

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("Vp886GetLineStatusWrapper(input:0x%04x) : 0x%04x",input, vpStatus));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    LT_DOUT(LT_DBG_INFO, ("Vp886GetLineStatusWrapper(input:0x%04x) : VP_STATUS_SUCCESS", input));

    return LT_STATUS_RUNNING;

} /* Vp886GetLineStatusWrapper */

extern bool
Vp886EventErrorCheck(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    if (pEvent == VP_NULL) {
        return FALSE;
    }

    if (pEvent->pVpEvent->eventCategory == VP_EVCAT_TEST) {
        VpTestResultType *pTestPrimRslts = (VpTestResultType*)pEvent->pResult;

        if(pTestPrimRslts->testId != pTestCtx->pTemp->vp886Temp.expected.thing.testId) {
            /* test id does not match the saved test id */
            *pRetval = LT_STATUS_ERROR_UNEXPECTED_EVENT;
            LT_DOUT(LT_DBG_ERRR, ("Vp886EventErrorCheck(testIdRcv:%i," \
                "testIdExp:%i) : Unexpected event",
                pTestPrimRslts->testId,
                pTestCtx->pTemp->vp886Temp.expected.thing.testId));
            return TRUE;
        }

        if(pTestPrimRslts->errorCode != VP_TEST_STATUS_SUCCESS) {
            /* has a primative caused an errorCode */
            pTestCtx->pResult->vpTestErrorCode = pTestPrimRslts->errorCode;
            *pRetval = LT_STATUS_ERROR_VP_TEST;
            LT_DOUT(LT_DBG_ERRR, ("Vp886EventErrorCheck(errorCode:0x%04x)",
                pTestPrimRslts->errorCode));
            return TRUE;
        }
    }
    return FALSE;
}


static bool
Vp886CheckDevAndChanId(
    VpLineCtxType *pLineCtx,
    LtEventType *pEvent)
{
    VpLineInfoType lineInfo;
    VpDeviceInfoType devInfo;

    /* Determine pLineCtx's chanId */
    lineInfo.pDevCtx = NULL;
    lineInfo.channelId = 0;
    lineInfo.pLineCtx = pLineCtx;
    VpGetLineInfo(&lineInfo);

    /* Determine pLineCtx's deviceId */
    devInfo.pLineCtx = pLineCtx;
    devInfo.pDevCtx = NULL;
    VpGetDeviceInfo(&devInfo);

    /* Compare vpEvent's chanId and deviceId to pLineCtx's */
    return ( (pEvent->pVpEvent->channelId == lineInfo.channelId) &&
             (pEvent->pVpEvent->deviceId == devInfo.deviceId) );

} /* CheckDevAndChanId() */

static LtTestStatusType
Vp886CleanUpAfterTestFailure(
    LtTestCtxType *pTestCtx)
{
    LtTestStatusType retval = LT_STATUS_RUNNING;
    VpTestConcludeType forceConclude;
    LtTestTempType *pTemp = pTestCtx->pTemp;

    LT_DOUT(LT_DBG_FUNC, ("Vp886CleanUpAfterTestFailure()+"));

    /* Cleanup the resources that were tied up */
    if(pTemp->vp886Temp.resources.testNotConcluded == TRUE) {

        /* Forcibly conclude the test */
        forceConclude.abortTest = TRUE;
        LT_DOUT(LT_DBG_INFO, ("Vp886CleanUpAfterTestFailure() : Forcing VP_TEST_ID_CONCLUDE"));
        retval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE, &forceConclude);
    }

    LT_DOUT(LT_DBG_FUNC, ("Vp886CleanUpAfterTestFailure()-"));
    return retval;
} /* Vp886CleanUpAfterTestFailure() */


static void
Vp886HouseKeeping(
    LtTestCtxType *pTestCtx)
{
    LtTestIdType testId = pTestCtx->testId;
    /*
     * This is performing any last minute cleanup.
     * If any of the tests had changed the line state
     * but then aborted early, this will restore it.
     */

    LT_DOUT(LT_DBG_FUNC, ("Vp886HouseKeeping()+"));
    if ((testId != LT_TID_RD_LOOP_COND) &&
        (testId != LT_TID_RD_BAT_COND) &&
        (testId != LT_TID_LOOPBACK)) {
        VpSetLineState(pTestCtx->pLineCtx, pTestCtx->pTemp->vp886Temp.resources.initialLineState);
    }

    #ifdef LT_ALL_GR_909
    {
        if (testId == LT_TID_ALL_GR_909) {
            /* LtVp886AllCopyResults(pTestCtx); */
        }
    }
    #endif

    LT_DOUT(LT_DBG_FUNC, ("Vp886HouseKeeping()-"));
    return;
} /* Vp886HouseKeeping() */


/**
 * Vp886TestInit
 *
 * Determine if the input arguments are legal and to call the
 * appropriate test prepare primitive.
 *
 */
static LtTestStatusType
Vp886TestInit(
    LtTestCtxType *pTestCtx,
    LtTestIdType testId)
{
    LtTestStatusType retval = LT_STATUS_RUNNING;
    VpTestPrepareType prepareArgs = { FALSE };

    LT_DOUT(LT_DBG_FUNC, ("Vp886TestInit(ltTestId:%i)+",testId));

#if defined(LT_ALL_GR_909) && defined(LT_ALL_GR_909_886)
    /* 909 all has special needs */
    if (testId == LT_TID_ALL_GR_909) {
        /* Indicate this will contain internal test */
        pTestCtx->pTemp->vp886Temp.internalTest = TRUE;
        pTestCtx->pTemp->vp886Temp.allGr909.innerState = VP886_ALL_GR_909_INIT;
    }
#endif

    /* Check attributes */
    if (Vp886AttributeCheck(pTestCtx, pTestCtx->testId) == FALSE) {
        LT_DOUT(LT_DBG_ERRR, ("Vp886TestInit() : Vp886AttributeCheck() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("Vp886TestInit()-"));
        return  LT_STATUS_ERROR_INVALID_ARG;
    }

    if ((testId == LT_TID_RD_LOOP_COND) ||
        (testId == LT_TID_RD_BAT_COND) ||
        (testId == LT_TID_LOOPBACK) ||
        (testId == LT_TID_PRE_LINE_V)) {

        pTestCtx->state = LT_GLOBAL_FIRST_STATE;
        retval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_PREPARE, &prepareArgs);

    } else if ((FALSE == pTestCtx->pTemp->vp886Temp.internalTest) ||
        testId == LT_TID_ALL_GR_909) {

        pTestCtx->state = LT_GLOBAL_FIRST_STATE;
        retval = Vp886TestLineWrapper(pTestCtx, VP_TEST_ID_PREPARE, NULL);

    }
    LT_DOUT(LT_DBG_FUNC, ("Vp886TestInit()-"));
    return retval;
}

/**
 * Vp886IsTestSupported
 *
 * Simple switch to determine if the test has been compiled in.
 *
 */
static bool
Vp886IsTestSupported(
    LtTestCtxType *pTestCtx,
    VpDeviceInfoType *pDeviceInfo,
    VpTermType termType,
    LtTestIdType testId)
{
    bool retVal = FALSE;

    LT_DOUT(LT_DBG_FUNC, ("Vp886IsTestSupported()+"));

    /* Make sure the device object is really an Vp886 */
    if ((pDeviceInfo->deviceType != VP_DEV_886_SERIES) &&
        (pDeviceInfo->deviceType != VP_DEV_887_SERIES)) {
        LT_DOUT(LT_DBG_ERRR, ("Vp886IsTestSupported(devType:%i) : invalid", pDeviceInfo->deviceType));
        LT_DOUT(LT_DBG_FUNC, ("Vp886IsTestSupported()-"));
        pTestCtx->pResult->vpGeneralErrorCode = VP_STATUS_ERR_VTD_CODE;
        return FALSE;
    }

    switch (pTestCtx->testId) {
        #ifdef LT_LINE_V_886
        case LT_TID_LINE_V:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_ROH_886
        case LT_TID_ROH:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_RINGERS_886
        case LT_TID_RINGERS:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_RES_FLT_886
        case LT_TID_RES_FLT:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_MSOCKET_886
        case LT_TID_MSOCKET:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_XCONNECT_886
        case LT_TID_XCONNECT:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_CAP_886
        case LT_TID_CAP:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_LOOPBACK_886
        case LT_TID_LOOPBACK:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_DC_FEED_ST_886
        case LT_TID_DC_FEED_ST:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_RD_LOOP_COND_886
        case LT_TID_RD_LOOP_COND:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_DC_VOLTAGE_886
        case LT_TID_DC_VOLTAGE:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_RINGING_ST_886
        case LT_TID_RINGING_ST:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_ON_OFF_HOOK_ST_886
        case LT_TID_ON_OFF_HOOK_ST:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_RD_BAT_COND_886
        case LT_TID_RD_BAT_COND:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_PRE_LINE_V_886
        case LT_TID_PRE_LINE_V:
            retVal = TRUE;
            break;
        #endif

        #ifdef LT_ALL_GR_909_886
        case LT_TID_ALL_GR_909:
            retVal = TRUE;
            break;
        #endif

       default:
            LT_DOUT(LT_DBG_ERRR, ("Vp886IsTestSupported(testId:%i) : invalid", pTestCtx->testId));
            retVal = FALSE;
    }

    LT_DOUT(LT_DBG_FUNC, ("Vp886IsTestSupported()-"));
    return retVal;

} /* Vp886IsTestSupported() */

/*
 * This function returns TRUE if no issues were found with the input
 * arguments for the specfied testId;
 */
static bool
Vp886AttributeCheck(
    LtTestCtxType *pTestCtx,
    const LtTestIdType testId)
{
    bool result = FALSE;

    LT_DOUT(LT_DBG_FUNC, ("Vp886AttributeCheck(ltTestId:%i)+",testId));

    switch (testId) {

        #ifdef LT_LINE_V_886
        case LT_TID_LINE_V:
            result = LtVp886LineVAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_ROH_886
        case LT_TID_ROH:
            result = LtVp886RohAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RINGERS_886
        case LT_TID_RINGERS:
            result = LtVp886RingersAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RES_FLT_886
        case LT_TID_RES_FLT:
            result = LtVp886ResFltAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_MSOCKET_886
        case LT_TID_MSOCKET:
            result = LtVp886MSocketAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_XCONNECT_886
        case LT_TID_XCONNECT:
            result = LtVp886XConnectAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_CAP_886
        case LT_TID_CAP:
            result = LtVp886CapAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_LOOPBACK_886
        case LT_TID_LOOPBACK:
            result = LtVp886LoopbackAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_DC_FEED_ST_886
        case LT_TID_DC_FEED_ST:
            result = LtVp886DcFeedStAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RD_LOOP_COND_886
        case LT_TID_RD_LOOP_COND:
            result = LtVp886RdLoopCondAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_DC_VOLTAGE_886
        case LT_TID_DC_VOLTAGE:
            result = LtVp886DcVoltageAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RINGING_ST_886
        case LT_TID_RINGING_ST:
            result = LtVp886RingingStAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_ON_OFF_HOOK_ST_886
        case LT_TID_ON_OFF_HOOK_ST:
            result = LtVp886OnOffHookStAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_RD_BAT_COND_886
        case LT_TID_RD_BAT_COND:
            result = LtVp886RdBatCondAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_PRE_LINE_V_886
        case LT_TID_PRE_LINE_V:
            result = LtVp886PreLineVAttributeCheck(pTestCtx);
            break;
        #endif

        #ifdef LT_ALL_GR_909_886
        case LT_TID_ALL_GR_909:
            result = LtVp886AllGR909AttributeCheck(pTestCtx);
            break;
        #endif

       default:
            LT_DOUT(LT_DBG_ERRR, ("Vp886AttributeCheck(ltTestId:%i) : invalid", testId));
            result = FALSE;
            break;
    }

    LT_DOUT(LT_DBG_FUNC, ("Vp886AttributeCheck(ltTestId:%i,result:%i)-",testId,result));
    return result;
}

#endif /* LT_VP886_PACKAGE */




