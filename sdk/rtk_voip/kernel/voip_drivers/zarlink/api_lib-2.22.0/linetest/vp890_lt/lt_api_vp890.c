/** \lt_api_vp890.c
 * lt_api_vp890.c
 *
 * This file contains the implementation top level LT-API application interface
 * for Vp890 device family.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#include "lt_api.h"

//#if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE)

#include "lt_api_internal.h"
#include "lt_api_vp890.h"
#include "lt_api_vp890_int.h"
/* Internal Function Declaration */
LtTestStatusType
LtVp890AbortTest(
    LtTestCtxType *pTestCtx);

static LtTestStatusType
LtVp890TestInit(
    LtTestCtxType *pTestCtx,
    const LtTestIdType testId);

static bool
CheckDevAndChanId(
    VpLineCtxType *pLineCtx,
    LtEventType *pEvent);

static bool
IsTestSupported(
    LtTestCtxType *pTestCtx,
    VpDeviceInfoType *pDeviceInfo,
    VpTermType termType,
    LtTestIdType testId);

static LtTestStatusType
CleanUpAfterTestFailure(
    LtTestCtxType *pTestCtx);

static void HouseKeeping(LtTestCtxType *pTestCtx);

LtTestStatusType
LtVp890StartTest(
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

    LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()+, test id = %d", testId));

    /* pTemp cannot be NULL */
    if (pTemp == NULL) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest() : pTemp is NULL"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    /* Checking for legal test id */
    if ((testId < 0) || (testId >= LT_NUM_TID_CODES)) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest(testId:%i) : invalid", testId));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : Initializing TestCtx struct"));
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

    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : Initializing Temp struct"));
    pTestCtx->pTemp->vp890Temp.internalTest = FALSE;
    pTestCtx->pTemp->vp890Temp.resources.testNotConcluded = TRUE;
    pTestCtx->pTemp->vp890Temp.resources.testAborted = FALSE;
    pTestCtx->pTemp->vp890Temp.attributes.pInputs = NULL;
    pTestCtx->pTemp->vp890Temp.attributes.pCriteria = NULL;
    pTestCtx->pTemp->vp890Temp.revCode = 0;
    pTestCtx->pTemp->vp890Temp.productCode = 0;

    /* default topology */
    pTestCtx->pTemp->vp890Temp.attributes.topology.rTestLoad =
        VP890_TOP_RES_TEST_LOAD;
    pTestCtx->pTemp->vp890Temp.attributes.topology.rSenseA =
        VP890_TOP_R_SENSE;
    pTestCtx->pTemp->vp890Temp.attributes.topology.rSenseB =
        VP890_TOP_R_SENSE;
    pTestCtx->pTemp->vp890Temp.attributes.topology.rLeakageA =
        VP890_TOP_R_LEAKAGE;
    pTestCtx->pTemp->vp890Temp.attributes.topology.rLeakageB =
        VP890_TOP_R_LEAKAGE;

    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : Check for Null attributes"));
    if (VP_NULL != pTestCtx->pAttributes) {
        /* Checking for any NULL pointer in the appropriate unions */
        pTestCtx->pTemp->vp890Temp.attributes.pInputs =
            pTestCtx->pAttributes->inputs.pLineVInp;

        pTestCtx->pTemp->vp890Temp.attributes.pCriteria =
            pTestCtx->pAttributes->criteria.pLintVCrt;

        if (VP_NULL != pTestCtx->pAttributes->topology.pVp890Topology) {
            LtMemCpy(&pTestCtx->pTemp->vp890Temp.attributes.topology,
                pTestCtx->pAttributes->topology.pVp890Topology,
                sizeof(LtVp890TestTopologyType));
        }
    }

    lineInfo.pLineCtx = pLineCtx;
    lineInfo.pDevCtx = NULL;
    lineInfo.channelId = 0;
    lineInfo.termType = 0;

    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : GetLineInfo"));
    vpStatus = VpGetLineInfo(&lineInfo);
    pTestCtx->pTemp->vp890Temp.termType = lineInfo.termType;

    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : Is top NULL"));
    if ( ((VP_NULL == pTestCtx->pAttributes) ||
          (VP_NULL == pTestCtx->pAttributes->topology.pVp890Topology)) &&

         ((lineInfo.termType == VP_TERM_FXS_LOW_PWR) ||
          (lineInfo.termType == VP_TERM_FXS_ISOLATE_LP) ||
          (lineInfo.termType == VP_TERM_FXS_SPLITTER_LP)) ) {
        pTestCtx->pTemp->vp890Temp.attributes.topology.rLeakageB = VP890_TOP_LOW_PWR_RNG_LEAK;
    }

    if(VP_STATUS_SUCCESS != vpStatus) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest() : VpGetLineInfo() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    vpStatus = VpGetLineState(pTestCtx->pLineCtx,
        &pTestCtx->pTemp->vp890Temp.resources.initialLineState);

    if(VP_STATUS_SUCCESS != vpStatus) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest() : VpGetLineState() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    pTestCtx->pTemp->vp890Temp.resources.relayStRestore = TRUE;
    vpStatus = VpGetRelayState(pTestCtx->pLineCtx,
        &pTestCtx->pTemp->vp890Temp.resources.initialRelayState);

    if(VP_STATUS_SUCCESS != vpStatus) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest() : VpGetRelayState() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    /* Only support testing on the following terminations */
    if ((lineInfo.termType != VP_TERM_FXS_GENERIC) &&
        (lineInfo.termType != VP_TERM_FXS_LOW_PWR) &&
        (lineInfo.termType != VP_TERM_FXS_SPLITTER_LP) &&
        (lineInfo.termType != VP_TERM_FXS_SPLITTER) &&
        (lineInfo.termType != VP_TERM_FXS_ISOLATE_LP)) {
        pTestCtx->pResult->vpGeneralErrorCode = VP_STATUS_ERR_VTD_CODE;
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest(%i) : invalid termType",
            lineInfo.termType));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : GetDeviceInfo"));
    devInfo.pLineCtx = pLineCtx;
    devInfo.pDevCtx = NULL;
    devInfo.deviceId = 0;
    devInfo.deviceType = 0;
    devInfo.numLines =0;
    vpStatus = VpGetDeviceInfo(&devInfo);
    if (VP_STATUS_SUCCESS != vpStatus) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest() : VpGetDeviceInfo() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    /* Store the rev code and product code */
    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : revcode"));
    pTestCtx->pTemp->vp890Temp.revCode = devInfo.revCode;
    pTestCtx->pTemp->vp890Temp.productCode = devInfo.productCode;

    LT_DOUT(LT_DBG_INFO, ("LtVp890StartTest() : is test supported"));
    if (IsTestSupported(pTestCtx, &devInfo, lineInfo.termType, testId) == FALSE) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp890StartTest() : IsTestSupported() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));
        return LT_STATUS_TEST_NOT_SUPPORTED;
    }
    pTestCtx->termType = lineInfo.termType;

    /* Run the generic initialization function */
    returnVal = LtVp890TestInit(pTestCtx, testId);
    LT_DOUT(LT_DBG_FUNC, ("LtVp890StartTest()-"));

    return returnVal;
} /* LtVp890StartTest() */

static LtTestStatusType
LtVp890TestInit(
    LtTestCtxType *pTestCtx,
    const LtTestIdType testId)
{
    LtTestStatusType retval = LT_STATUS_RUNNING;

    LT_DOUT(LT_DBG_FUNC, ("LtVp890TestInit(ltTestId:%i)+",testId));

    /* 909 all has special needs */
    if (testId == LT_TID_ALL_GR_909) {
        /* Indicate this will contain internal test */
        pTestCtx->pTemp->vp890Temp.internalTest = TRUE;
        pTestCtx->pTemp->vp890Temp.allGr909.innerState = VP890_ALL_GR_909_INIT;
    }

    /* Check attributes */
    if (FALSE == LtVp890AttributeCheck(pTestCtx, pTestCtx->testId)) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp890TestInit() : LtVp890AttributeCheck() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890TestInit()-"));
        return  LT_STATUS_ERROR_INVALID_ARG;
    }

    if ((testId == LT_TID_RD_LOOP_COND) || (testId == LT_TID_LOOPBACK)) {
        pTestCtx->state = LT_GLOBAL_FIRST_STATE;
        retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_PREPARE_EXT, NULL);
    } else if ((FALSE == pTestCtx->pTemp->vp890Temp.internalTest) ||
        testId == LT_TID_ALL_GR_909) {
        pTestCtx->state = LT_GLOBAL_FIRST_STATE;
        retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_PREPARE, NULL);
    }
    LT_DOUT(LT_DBG_FUNC, ("LtVp890TestInit()-"));
    return retval;
}

extern LtTestStatusType
LtVp890EventHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestIdType testId;
    LtTestTempType *pTemp;
    LtTestStatusType retval;
    LtVp890AllGr909States innerState = VP890_ALL_GR_909_INNER_ENUM_SIZE;

    LT_DOUT(LT_DBG_FUNC, ("LtVp890EventHandler()+"));

    /* Very basic error checking */
    if((pTestCtx == NULL) || (pEvent == NULL)) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp890EventHandler(pTestCtx=NULL|pEvent=NULL) : "\
            "invalid args"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890EventHandler()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    testId = pTestCtx->testId;

    /* innerState is the "subtest id" during the GR909 test */
    if (testId == LT_TID_ALL_GR_909) {
        innerState = pTestCtx->pTemp->vp890Temp.allGr909.innerState;
    }

    if ( (testId < 0) || (testId >= LT_NUM_TID_CODES) ) {
        /* invalid Test ID */
        LT_DOUT(LT_DBG_ERRR, ("LtVp890EventHandler(testId:%i) : invalid", testId));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890EventHandler()-"));
        retval = LT_STATUS_ERROR_INVALID_ARG;

    } else if (!CheckDevAndChanId(pTestCtx->pLineCtx, pEvent)) {
        retval = LT_STATUS_RUNNING;

    } else if (pEvent->pVpEvent->eventCategory != pTestCtx->pTemp->vp890Temp.expected.eventCat) {
        retval = LT_STATUS_RUNNING;

    } else if ((pEvent->pVpEvent->eventCategory == pTestCtx->pTemp->vp890Temp.expected.eventCat) &&
               (pEvent->pVpEvent->eventId == VP_LINE_EVID_ABORT) && (testId != LT_TID_LINE_V) &&
               (innerState != VP890_ALL_GR_909_INIT) && (innerState != VP890_ALL_GR_909_LINE_V) &&
               (pTestCtx->pTemp->vp890Temp.resources.testAborted == FALSE) ) {
        LT_DOUT(LT_DBG_WARN, ("LtVp890EventHandler(): The test is aborting"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890EventHandler()-"));
        return LtVp890AbortTest(pTestCtx);

    } else if (pTestCtx->pTemp->vp890Temp.resources.testAborted == TRUE) {
        /* Checking if the abort test command was called */

        LT_DOUT(LT_DBG_WARN, ("LtVp890EventHandler(testAborted:TRUE,eventCat:%i,eventId:%i) : "\
            ": Test Aborted", pEvent->pVpEvent->eventCategory, pEvent->pVpEvent->eventId));

        if ( (pEvent->pVpEvent->eventId == VP_LINE_EVID_TEST_CMP) &&
             (pEvent->pVpEvent->eventData == VP_TEST_ID_CONCLUDE) ) {
            /* Checking if the event is a conclude */
            retval = LT_STATUS_ABORTED;
        } else {
            /* if it is not then we need to call conclude */
            retval = CleanUpAfterTestFailure(pTestCtx);
        }

    } else if (pEvent->pVpEvent->eventId != pTestCtx->pTemp->vp890Temp.expected.eventId) {
        retval = LT_STATUS_RUNNING;

    } else {
        if(pTestCtx->testFailed == FALSE) {
            /* Execute the appropriate test-specific event handler. */
            LT_DOUT(LT_DBG_INFO, ("LtVp890EventHandler(cat:0x%04x,id:0x%04x,data:%i)",
                pEvent->pVpEvent->eventCategory,
                pEvent->pVpEvent->eventId,
                pEvent->pVpEvent->eventData));

            retval = pTestCtx->TestEventHandlerFunc(pTestCtx, pEvent);
        } else {
            if(pTestCtx->pTemp->vp890Temp.resources.testNotConcluded == FALSE) {
                /* LT test had failed and the last action that was taken was
                 * test conclude. Return the reasons why test failed */
                retval = pTestCtx->tempStatus;
                LT_DOUT(LT_DBG_WARN, ("LtVp890EventHandler(testFailed:TRUE,testId:%i)"\
                    ": Normal Conclude", testId));
            } else {
                /* Clean up after a test concludes too early*/
                LT_DOUT(LT_DBG_WARN, ("LtVp890EventHandler(testFailed:TRUE,testId:%i)"\
                    ": Abnormally Concluding", testId));
                retval = CleanUpAfterTestFailure(pTestCtx);
            }
        }
    }

    /* Forcibly execute a Test Conclude if any error detected. */
    if(pTestCtx->testFailed == FALSE) {
        if ((retval != LT_STATUS_RUNNING) && (retval != LT_STATUS_DONE)
            && (retval != LT_STATUS_ABORTED)) {
            /* Even though test failed
             * we can not return from the test since there is still one event
             * pending as consequence of the test conclude call. Thus test
             * library needs to wait all of actions result in appropriate
             * events; So store the reason why test failed temporarily */

            pTestCtx->testFailed = TRUE;
            pTestCtx->tempStatus = retval;

            /* Store a state for which none of the test does nothing */
            pTestCtx->state = LT_GLOBAL_ABORTED_STATE;
            LT_DOUT(LT_DBG_ERRR, ("LtVp890EventHandler(testFailed:FALSE,retval:%i)"\
                ": *** Forcing a Conclude ***", retval));

            /* Perform necessary cleanup */
            retval = CleanUpAfterTestFailure(pTestCtx);
        }
    }

    if(retval == LT_STATUS_DONE) {
        /* Test is done; Make sure test has not locked up any resource */
        pTemp = pTestCtx->pTemp;

        if (pTemp->vp890Temp.resources.testNotConcluded == TRUE) {
            /* This error is not expected to happen */
            LT_DOUT(LT_DBG_ERRR, ("LtVp890EventHandler(LT_STATUS_DONE) :"\
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
    if( CheckDevAndChanId(pTestCtx->pLineCtx, pEvent) &&
        (pEvent->pVpEvent->eventCategory ==  pTestCtx->pTemp->vp890Temp.expected.eventCat) &&
        (pEvent->pVpEvent->eventId == pTestCtx->pTemp->vp890Temp.expected.eventId) &&
        (pEvent->pVpEvent->eventData == VP_TEST_ID_CONCLUDE)) {
        HouseKeeping(pTestCtx);
    }

    LT_DOUT(LT_DBG_FUNC, ("LtVp890EventHandler()-"));
    return retval;
} /* LtVp890EventHandler() */

LtTestStatusType
LtVp890AbortTest(
    LtTestCtxType *pTestCtx)
{
    LtTestStatusType retval;

    LT_DOUT(LT_DBG_FUNC, ("LtVp890AbortTest()+"));
    /* Very basic error checking */
    if(pTestCtx == NULL) {
        LT_DOUT(LT_DBG_ERRR, ("LtVp890AbortTest(pTestCtx=NULL) : invalid args"));
        LT_DOUT(LT_DBG_FUNC, ("LtVp890AbortTest()-"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    if (pTestCtx->pTemp->vp890Temp.resources.testNotConcluded == FALSE) {
        /* no test is running so just return Abort indication */
        retval = LT_STATUS_DONE;
    } else {
        /* a test is running so set the abort flag and return running */
        pTestCtx->pTemp->vp890Temp.resources.testAborted = TRUE;
        retval = LT_STATUS_RUNNING;
    }
    LT_DOUT(LT_DBG_FUNC, ("LtVp890AbortTest()-"));
    return retval;
} /* LtVp890AbortTest() */

static bool
CheckDevAndChanId(
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

/*
 * This function returns TRUE if a given test is supported for a given
 * type of line termination type else it returns false;
 */
static bool
IsTestSupported(
    LtTestCtxType *pTestCtx,
    VpDeviceInfoType *pDeviceInfo,
    VpTermType termType,
    LtTestIdType testId)
{
    /* Make sure the device object is really an 890 */
    if (VP_DEV_890_SERIES != pDeviceInfo->deviceType) {
        LT_DOUT(LT_DBG_ERRR, ("IsTestSupported(devType:%i) : invalid", pDeviceInfo->deviceType));
        return FALSE;
    }

    LT_DOUT(LT_DBG_FUNC, ("IsTestSupported() : TRUE"));
    return LtVp890IsTestSupported(pTestCtx, pDeviceInfo, testId);

} /* IsTestSupported() */

/*
 * This function is used to perform cleanup after a test fails.
 */
static LtTestStatusType
CleanUpAfterTestFailure(
    LtTestCtxType *pTestCtx)
{
    LtTestStatusType retval = LT_STATUS_RUNNING;
    VpTestConcludeType forceConclude;
    LtTestTempType *pTemp = pTestCtx->pTemp;

    LT_DOUT(LT_DBG_FUNC, ("CleanUpAfterTestFailure()+"));
    /* Cleanup the resources that were tied up */
    if(pTemp->vp890Temp.resources.testNotConcluded == TRUE) {
        /* Forcibly conclude the test */
        forceConclude.abortTest = TRUE;
        LT_DOUT(LT_DBG_INFO, ("CleanUpAfterTestFailure() : Forcing VP_TEST_ID_CONCLUDE"));
        retval = Vp890TestLineWrapper(pTestCtx, VP_TEST_ID_CONCLUDE,
                                        &forceConclude);
    }
    LT_DOUT(LT_DBG_FUNC, ("CleanUpAfterTestFailure()-"));
    return retval;
} /* CleanUpAfterTestFailure() */

static void
HouseKeeping(
    LtTestCtxType *pTestCtx)
{
    LtTestIdType testId = pTestCtx->testId;
    /*
     * This is performing any last minute cleanup.
     * If any of the tests had changed the line state
     * but then aborted early, this will restore it.
     */
    LT_DOUT(LT_DBG_FUNC, ("HouseKeeping()+"));
    VpSetLineState(pTestCtx->pLineCtx, pTestCtx->pTemp->vp890Temp.resources.initialLineState);

    if (pTestCtx->pTemp->vp890Temp.resources.relayStRestore) {
        VpSetRelayState(pTestCtx->pLineCtx,
            pTestCtx->pTemp->vp890Temp.resources.initialRelayState);
    }

    if (testId == LT_TID_ALL_GR_909) {
        LtVp890AllCopyResults(pTestCtx);
    }

    LT_DOUT(LT_DBG_FUNC, ("HouseKeeping()-"));
    return;
} /* HouseKeeping() */


extern LtTestStatusType
Vp890TestLineWrapper(
    LtTestCtxType *pTestCtx,
    VpTestIdType test,
    const void * pArgs)
{
    VpStatusType vpStatus;

    LtTestTempType *pTemp = pTestCtx->pTemp;

    /* Save the currently executing test for later error checking. */
    pTemp->vp890Temp.expected.eventCat = VP_EVCAT_TEST;
    pTemp->vp890Temp.expected.eventId = VP_LINE_EVID_TEST_CMP;
    pTemp->vp890Temp.expected.thing.testId = test;

    vpStatus = VpTestLine(pTestCtx->pLineCtx, test, pArgs, pTestCtx->handle);

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("Vp890TestLineWrapper(vpTestId:%i,vpStatus%i)",
            test, vpStatus));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    /* Set various resource useage flags */
    if ((test == VP_TEST_ID_PREPARE) || (test == VP_TEST_ID_PREPARE_EXT)) {
        pTemp->vp890Temp.resources.testNotConcluded = TRUE;

    } else if (test == VP_TEST_ID_CONCLUDE) {
        pTemp->vp890Temp.resources.testNotConcluded = FALSE;

    }

    LT_DOUT(LT_DBG_INFO, ("Vp890TestLineWrapper(vpTestId:%i)", test));

    return LT_STATUS_RUNNING;
} /* Vp890TestLineWrapper() */

extern LtTestStatusType
Vp890SetRelayStateWrapper(
    LtTestCtxType *pTestCtx,
    VpRelayControlType relayState)
{
    VpStatusType vpStatus;
    vpStatus = VpSetRelayState(pTestCtx->pLineCtx, relayState);

    if (vpStatus != VP_STATUS_SUCCESS) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("Vp890SetRelayStateWrapper(relayState:0x%04x"\
            ",vpStatus:0x%04x)",relayState, vpStatus));
        return LT_STATUS_ERROR_VP_GENERAL;
    }

    LT_DOUT(LT_DBG_INFO, ("Vp890SetRelayStateWrapper() : VP_STATUS_SUCCESS"));

    return LT_STATUS_RUNNING;

} /* Vp88SetRelayStateWrapper */


extern bool
Vp890EventErrorCheck(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval)
{
    if (pEvent == VP_NULL) {
        LT_DOUT(LT_DBG_ERRR, ("Vp890EventErrorCheck(pEvent=NULL)"));
        return FALSE;
    }

    if (pEvent->pVpEvent->eventCategory == VP_EVCAT_TEST) {
        VpTestResultType *pVpTestResult = (VpTestResultType*)pEvent->pResult;

        if(pVpTestResult->testId != pTestCtx->pTemp->vp890Temp.expected.thing.testId) {
            /* test id does not match the saved test id */
            *pRetval = LT_STATUS_ERROR_UNEXPECTED_EVENT;
            LT_DOUT(LT_DBG_ERRR, ("Vp890EventErrorCheck(testIdRcv:%i," \
                "testIdExp:%i) : Unexpected event",
                pVpTestResult->testId,
                pTestCtx->pTemp->vp890Temp.expected.thing.testId));
            return TRUE;
        }

        if(VP_TEST_STATUS_SUCCESS != pVpTestResult->errorCode) {
            /* has a primative caused an errorCode */
            pTestCtx->pResult->vpTestErrorCode = pVpTestResult->errorCode;
            *pRetval = LT_STATUS_ERROR_VP_TEST;
            LT_DOUT(LT_DBG_ERRR, ("Vp890EventErrorCheck(errorCode:0x%04x)",
                pVpTestResult->errorCode));
            return TRUE;
        }
    }
    return FALSE;
}
//#endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */
