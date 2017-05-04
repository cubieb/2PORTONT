/** \lt_api.c
 * lt_api.c
 *
 * This file contains the implementation top level LT application interface.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9542 $
 * $LastChangedDate: 2012-02-13 16:03:22 -0600 (Mon, 13 Feb 2012) $
 */

#include "lt_api.h"
#include "lt_api_internal.h"
#if defined (CONFIG_RTK_VOIP_SLIC_ZARLINK_890_SERIES) 
extern LtTestStatusType
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
    LtTestCtxType *pTestCtx);              /* LT test context pointer */
#endif	
/*
 * This function is the top level application interface to start a Line test.
 * Please refer the documentation for more details regarding mechanisms of
 * invoking this function.
 */
EXTERN LtTestStatusType
LtStartTest(
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
    VpStatusType vpStatus;
    LtTestStatusType returnVal = LT_STATUS_ERROR_UNKNOWN;

    /* Perform the least amount of error checking here and then pass the
     * control individual test library */


    LT_DOUT(LT_DBG_FUNC, ("LtStartTest(handle:0x%04x)+", handle));

    /* Basic error checking */
    if((pLineCtx == VP_NULL) || (pTestCtx == VP_NULL) || (pResult == VP_NULL)) {
        LT_DOUT(LT_DBG_ERRR, ("LtStartTest() : VP_NULL issue"));
        LT_DOUT(LT_DBG_FUNC, ("LtStartTest(0x%04x)-\n", handle));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    devInfo.pLineCtx = pLineCtx;
    devInfo.pDevCtx = NULL;
    devInfo.deviceId = 0;
    devInfo.deviceType = 0;
    devInfo.numLines =0;
    vpStatus = VpGetDeviceInfo(&devInfo);
    if(VP_STATUS_SUCCESS != vpStatus) {
        /* Give the user some indication why the test failed. */
        pTestCtx->pResult->vpGeneralErrorCode = vpStatus;
        LT_DOUT(LT_DBG_ERRR, ("LtStartTest() : VpGetDeviceInfo() Failed"));
        LT_DOUT(LT_DBG_FUNC, ("LtStartTest()-\n"));
        return LT_STATUS_ERROR_VP_GENERAL;
    }
    /* Store the info for future use */
    pTestCtx->devType = devInfo.deviceType;

    /* Initialize test default outputs */
    LtInitOutputStruct(pResult, testId);

    switch (pTestCtx->devType) {
        #if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE)
        case VP_DEV_880_SERIES: {
            /* Call the start handler for this device family */
            returnVal = LtVp880StartTest(pLineCtx, testId, handle, pAttributes,
                                            pTemp, pResult, pTestCtx);
            break;
        }
        #endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE*/

        #if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE)
        case VP_DEV_890_SERIES: {
            /* Call the start handler for this device family */
            returnVal = LtVp890StartTest(pLineCtx, testId, handle, pAttributes,
                                            pTemp, pResult, pTestCtx);
            break;
        }
        #endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE*/

        #if defined(VP886_INCLUDE_TESTLINE_CODE) && defined(LT_VP886_PACKAGE)
        case VP_DEV_886_SERIES: 
        case VP_DEV_887_SERIES: {
            /* Call the start handler for this device family */
            returnVal = LtVp886StartTest(pLineCtx, testId, handle, pAttributes,
                                            pTemp, pResult, pTestCtx);
            break;
        }
        #endif /* LT_VP886_PACKAGE */

        default: {
            LT_DOUT(LT_DBG_WARN, ("LtStartTest(devType:%i) : invalid",
                pTestCtx->devType));
            returnVal = LT_STATUS_TEST_NOT_SUPPORTED;
            break;
        }
    }
    LT_DOUT(LT_DBG_FUNC, ("LtStartTest()-\n"));
    return returnVal;

} /* LtStartTest() */

/*
 * This function is the top level application interface to pass on the events
 * and the results that are generated as a result of running the test.
 */
EXTERN LtTestStatusType
LtEventHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent)
{
    LtTestStatusType returnVal = LT_STATUS_ERROR_UNKNOWN;

    if (pTestCtx == VP_NULL) {
        LT_DOUT(LT_DBG_FUNC, ("LtEventHandler(Invalid args)"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }
    LT_DOUT(LT_DBG_FUNC, ("LtEventHandler(eventCat:%i,eventId:%i, pTestCtx->handle:0x%04x)+",
        pEvent->pVpEvent->eventCategory, pEvent->pVpEvent->eventId, pTestCtx->handle));


    switch (pTestCtx->devType) {
        #if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE)
        case VP_DEV_880_SERIES: {
            /* Call the test handler for this device family */
            returnVal = LtVp880EventHandler(pTestCtx, pEvent);
            break;
        }
        #endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE */

        #if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE)
        case VP_DEV_890_SERIES: {
            /* Call the test handler for this device family */
            returnVal = LtVp890EventHandler(pTestCtx, pEvent);
            break;
        }
        #endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE */

        #if defined(VP886_INCLUDE_TESTLINE_CODE) && defined(LT_VP886_PACKAGE)
        case VP_DEV_886_SERIES: 
        case VP_DEV_887_SERIES: {
            /* Call the test handler for this device family */
            returnVal = LtVp886EventHandler(pTestCtx, pEvent);
            break;
        }
        #endif /* LT_VP886_PACKAGE */

        default: {
            LT_DOUT(LT_DBG_WARN, ("LtEventHandler(devType:%i) : invalid",
                pTestCtx->devType));
            /* Nothing to, must be an error of some sort */
            break;
        }
    }
    LT_DOUT(LT_DBG_FUNC, ("LtEventHandler()-\n"));
    return returnVal;

} /* LtEventHandler() */


/*
 * This function is the top level application interface to abort any running
 * test.
 */
EXTERN LtTestStatusType                   /* LT Test return type */
LtAbortTest(
    LtTestCtxType *pTestCtx)
{
    LtTestStatusType returnVal = LT_STATUS_ERROR_UNKNOWN;
    if (pTestCtx == VP_NULL) {
        LT_DOUT(LT_DBG_FUNC, ("LtAbortTest(Invalid args)"));
        return LT_STATUS_ERROR_INVALID_ARG;
    }

    LT_DOUT(LT_DBG_FUNC, ("LtAbortTest(hndl:0x%04x)+", pTestCtx->handle));

    switch (pTestCtx->devType) {
        #if defined(VP880_INCLUDE_TESTLINE_CODE) && defined(LT_VP880_PACKAGE)
        case VP_DEV_880_SERIES: {
            /* Call the abort handler for this device family */
            returnVal = LtVp880AbortTest(pTestCtx);
            break;
        }
        #endif /* LT_VP880_PACKAGE && VP880_INCLUDE_TESTLINE_CODE*/

        #if defined(VP890_INCLUDE_TESTLINE_CODE) && defined(LT_VP890_PACKAGE)
        case VP_DEV_890_SERIES: {
            /* Call the abort handler for this device family */
            returnVal = LtVp890AbortTest(pTestCtx);
            break;
        }
        #endif /* LT_VP890_PACKAGE && VP890_INCLUDE_TESTLINE_CODE*/

        #if defined(VP886_INCLUDE_TESTLINE_CODE) && defined(LT_VP886_PACKAGE)
        case VP_DEV_886_SERIES: 
        case VP_DEV_887_SERIES: {
            /* Call the abort handler for this device family */
            returnVal = LtVp886AbortTest(pTestCtx);
            break;
        }
        #endif /* LT_VP886_PACKAGE */

        default: {
            /* Nothing to, must be an error of some sort */
            LT_DOUT(LT_DBG_WARN, ("LtAbortTest(devType:%i) : invalid",
                pTestCtx->devType));
            break;
        }
    }
    LT_DOUT(LT_DBG_FUNC, ("LtAbortTest()-\n"));
    return returnVal;
}

#ifdef LT_LINE_V
/**
 * LtLineVAcDecompression - Return the actual voltage in non-linear area
 *
 */
EXTERN int32
LtLineVAcDecompression(
    int32 AcVoltage)
{
    uint8 idx = 0;
    int32 lowValueDiff, highValueDiff;
    int32 decompressionArray[21][21] = \
        {{173288, 176356, 179456, 182586, 185745, 187656, 189291, 190952, 192638, 194349, 196084, \
            197422, 197912, 198412, 198922, 199441, 199968, 200504, 201050, 201605, 202168}, \
        {175000, 180000, 185000, 190000, 195000, 200000, 205000, 210000, 215000, 220000, 225000, \
            230000, 235000, 240000, 245000, 250000, 255000, 260000, 265000, 270000, 275000}};

    /* Do not modify the AC voltage if it's in the linear area */
    if (AcVoltage < 171645) {
        return AcVoltage;
    }

    /* Saturate the AC voltage measurement to 250V */
    if (AcVoltage > 202168) {
        return 275000;
    }

    /* Find the closest entry to the measured voltage in decompressionArray */
    while (AcVoltage > decompressionArray[0][idx++]);

    if (idx == 1) {
        return 175000;
    }

    lowValueDiff = ABS(AcVoltage - decompressionArray[0][idx - 2]);
    highValueDiff = ABS(AcVoltage - decompressionArray[0][idx - 1]);
    if (lowValueDiff < highValueDiff) {
        /* return the low value */
        return decompressionArray[1][idx - 2];
    } else {
        /* return the high value */
        return decompressionArray[1][idx - 1];
    }
}
#endif

/**
 * memcpy - Copy one area of memory to another
 * @dest: Where to copy to
 * @src: Where to copy from
 * @count: The size of the area.
 *
 */
EXTERN void *
LtMemCpy(
    void * dest,
    const void *src,
    uint16 count)
{
    char *tmp = (char *) dest, *s = (char *) src;

    while (count--)
        *tmp++ = *s++;

    return dest;
}

/**
 * LtInitOutputStruct()
 * This function will fill the test output stucture with the default value
 *
 */
EXTERN void
LtInitOutputStruct(
    LtTestResultType *pResult,
    LtTestIdType testId)
{
    switch (testId) {
        #ifdef LT_PRE_LINE_V
        case LT_TID_PRE_LINE_V:
            pResult->result.preLineV.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.preLineV.measStatus = LT_MSRMNT_STATUS_PASSED;
            break;
        #endif

        #ifdef LT_LINE_V
        case LT_TID_LINE_V:
            pResult->result.lineV.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.lineV.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.lineV.vDcTip = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.lineV.vDcRing = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.lineV.vDcDiff = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.lineV.vAcTip = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.lineV.vAcRing = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.lineV.vAcDiff = LT_VOLTAGE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_ROH
        case LT_TID_ROH:
            pResult->result.roh.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.roh.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.roh.rLoop1 = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.roh.rLoop2 = LT_IMPEDANCE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_RINGERS
        case LT_TID_RINGERS:
            pResult->result.ringers.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.ringers.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.ringers.ren = LT_REN_NOT_MEASURED;
            pResult->result.ringers.rentg = LT_REN_NOT_MEASURED;
            pResult->result.ringers.renrg = LT_REN_NOT_MEASURED;
            pResult->result.ringers.ringerTestType = LT_RINGER_TEST_NUM_TYPES;
            break;
        #endif

        #ifdef LT_RES_FLT
        case LT_TID_RES_FLT:
            pResult->result.resFlt.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.resFlt.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.resFlt.rtg = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.resFlt.rrg = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.resFlt.rtr = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.resFlt.rGnd = LT_IMPEDANCE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_MSOCKET
        case LT_TID_MSOCKET:
            pResult->result.mSocket.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.mSocket.measStatus = LT_MSRMNT_STATUS_PASSED;
            break;
        #endif

        #ifdef LT_XCONNECT
        case LT_TID_XCONNECT:
            pResult->result.xConnect.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.xConnect.measStatus = LT_MSRMNT_STATUS_PASSED;
            break;
        #endif

        #ifdef LT_CAP
        case LT_TID_CAP:
            pResult->result.cap.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.cap.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.cap.ctg = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.cap.crg = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.cap.ctr = LT_IMPEDANCE_NOT_MEASURED;
            break;
        #endif

        #if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)
        case LT_TID_ALL_GR_909:
            pResult->result.allGr909.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.allGr909.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.allGr909.subTestMask = LT_A909STM_NO_PF_TESTED;

            pResult->result.allGr909.resFlt.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.allGr909.resFlt.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.allGr909.resFlt.rtg = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.allGr909.resFlt.rrg = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.allGr909.resFlt.rtr = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.allGr909.resFlt.rGnd = LT_IMPEDANCE_NOT_MEASURED;

            pResult->result.allGr909.ringers.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.allGr909.ringers.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.allGr909.ringers.ren = LT_REN_NOT_MEASURED;
            pResult->result.allGr909.ringers.rentg = LT_REN_NOT_MEASURED;
            pResult->result.allGr909.ringers.renrg = LT_REN_NOT_MEASURED;
            pResult->result.allGr909.ringers.ringerTestType = LT_RINGER_TEST_NUM_TYPES;

            pResult->result.allGr909.roh.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.allGr909.roh.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.allGr909.roh.rLoop1 = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.allGr909.roh.rLoop2 = LT_IMPEDANCE_NOT_MEASURED;

            pResult->result.allGr909.lineV.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.allGr909.lineV.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.allGr909.lineV.vDcTip = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.allGr909.lineV.vDcRing = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.allGr909.lineV.vDcDiff = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.allGr909.lineV.vAcTip = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.allGr909.lineV.vAcRing = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.allGr909.lineV.vAcDiff = LT_VOLTAGE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_LOOPBACK
        case LT_TID_LOOPBACK:
            pResult->result.loopback.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.loopback.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.loopback.pApplicationInfo = VP_NULL;
            break;
        #endif

        #ifdef LT_DC_FEED_ST
        case LT_TID_DC_FEED_ST:
            pResult->result.dcFeedST.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.dcFeedST.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.dcFeedST.vTestLoad = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.dcFeedST.iTestLoad = LT_CURRENT_NOT_MEASURED;
            pResult->result.dcFeedST.rTestLoad = LT_IMPEDANCE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_RD_LOOP_COND
        case LT_TID_RD_LOOP_COND:
            pResult->result.rdLoopCond.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.rdLoopCond.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.rdLoopCond.vab = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.rdLoopCond.vag = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.rdLoopCond.vbg = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.rdLoopCond.imt = LT_CURRENT_NOT_MEASURED;
            pResult->result.rdLoopCond.ilg = LT_CURRENT_NOT_MEASURED;
            pResult->result.rdLoopCond.rloop = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.rdLoopCond.vbat1 = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.rdLoopCond.vbat2 = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.rdLoopCond.vbat3 = LT_VOLTAGE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_DC_VOLTAGE
        case LT_TID_DC_VOLTAGE:
            pResult->result.dcVoltage.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.dcVoltage.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.dcVoltage.measuredVoltage1 = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.dcVoltage.measuredVoltage2 = LT_VOLTAGE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_RINGING_ST
        case LT_TID_RINGING_ST:
            pResult->result.ringingST.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.ringingST.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.ringingST.openCktRingVol = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.ringingST.freq = LT_FREQ_NOT_MEASURED;
            pResult->result.ringingST.acRload = LT_IMPEDANCE_NOT_MEASURED;
            pResult->result.ringingST.iRload = LT_CURRENT_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_ON_OFF_HOOK_ST
        case LT_TID_ON_OFF_HOOK_ST:
            pResult->result.onOffHookST.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.onOffHookST.measStatus = LT_MSRMNT_STATUS_PASSED;
            break;
        #endif

        #ifdef LT_RD_BAT_COND
        case LT_TID_RD_BAT_COND:
            pResult->result.readBatCond.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.readBatCond.measStatus = LT_MSRMNT_STATUS_PASSED;
            pResult->result.readBatCond.bat1 = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.readBatCond.bat2 = LT_VOLTAGE_NOT_MEASURED;
            pResult->result.readBatCond.bat3 = LT_VOLTAGE_NOT_MEASURED;
            break;
        #endif

        #ifdef LT_FLT_DSCRM
        case LT_TID_FLT_DSCRM:
            pResult->result.fltDscrm.fltMask = LT_TEST_MEASUREMENT_ERROR;
            pResult->result.fltDscrm.measStatus = LT_MSRMNT_STATUS_PASSED;
            break;
        #endif

       default:
            break;
    }
}
