/** file apitestline.c
 *
 *  This file contains the functions used in the Vp886 Test Line.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 11451 $
 * $LastChangedDate: 2014-06-06 16:35:35 -0500 (Fri, 06 Jun 2014) $
 */

#include "vp_api_cfg.h"

#if defined (VP_CC_886_SERIES)
#if defined (VP886_INCLUDE_TESTLINE_CODE)

/* Project Includes */
#include "vp886_testline_int.h"
#include "vp_api_int.h"

typedef struct {
    int32 high32;
    uint32 low32;
} int64tl;


static bool VpIsTestIdLegal(
    VpLineCtxType *pLineCtx,
    VpTestIdType testId);

static bool GetMathData(
    VpLineCtxType *pLineCtx,
    Vp886AdcMathDataType *pMathData,
    uint8 *pData,
    int16 offset,
    int16 gain);

static uint16
Vp886SysFrames2VadcFrames(
    VpLineCtxType *pLineCtx,
    uint16 sysFrames,
    Vp886VadcModeType mode,
    Vp886VadcRateType rate);

static uint16
Vp886SysFrames2SadcFrames(
    uint16 sysFrames,
    Vp886SadcModeType mode,
    Vp886SadcRateType rate);

static int64tl int64_from_int32(
    int32 x);

static int int64_cmp_gt(
    int64tl x,
    int64tl y);

static int int64_cmp_ge(
    int64tl x,
    int64tl y);

static int64tl int64_neg(
    int64tl x);

static int64tl int64_add(
    int64tl x,
    int64tl y);

static int64tl int64_sub(
    int64tl x,
    int64tl y);

static int64tl int64_shift(
    int64tl x,
    int8 y);

static int64tl int64_mul_int32_int32(
    int32 x,
    int32 y);

static int64tl int64_mul_int64_int32(
    int64tl x,
    int32 y);

static int64tl int64_div_int64_int32(
    int64tl x,
    int32 y);

/*------------------------------------------------------------------------------
 * Vp886TestLine()
 *  Initiates a line test.
 * This function will perfrom numerous checks to ensure that the test specified
 * by the testId argument can legally run. Once all error checks have been
 * completed the function relies on the Vp886TestLineInt function to actually
 * figure out which test primative to execute.
 *
 * Refer to the VoicePath API User's Guide for the rest of the
 * details about this function.
 *----------------------------------------------------------------------------*/
VpStatusType
Vp886TestLine (
    VpLineCtxType *pLineCtx,
    VpTestIdType testId,
    const void *pArgsUntyped,
    uint16 handle)
{
    VpStatusType status = VP_STATUS_SUCCESS;
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

    Vp886EnterCritical(VP_NULL, pLineCtx, "Vp886TestLine");

    /* only fxs line testing is supported */
    if (!pLineObj->isFxs) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886TestLine");
        return VP_STATUS_FUNC_NOT_SUPPORTED;
    }

    /* ensure we are not initializing / calibrating the device or line */
    if (!Vp886ReadyStatus(VP_NULL, pLineCtx, &status)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886TestLine");
        return status;
    }

    /*
     * Ensure the testId is legal at this time. If it was not
     * then a test event will be pushed by the function.
     */
    if (!VpIsTestIdLegal(pLineCtx, testId)) {
        Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886TestLine");
        return VP_STATUS_SUCCESS;
    }

    if (pLineObj->gndFltProt.state != VP886_GNDFLTPROT_ST_INACTIVE) {
        Vp886GndFltProtHandler(pLineCtx, VP886_GNDFLTPROT_INP_STOP);
    }

    /* store some of the input args to the line object */
    pLineObj->testInfo.testId = testId;
    pLineObj->testInfo.handle = handle;

    status = Vp886TestLineInt(pLineCtx, pArgsUntyped, FALSE);

    Vp886ExitCritical(VP_NULL, pLineCtx, "Vp886TestLine");

    return status;
}

/*------------------------------------------------------------------------------
 * Vp886TestLineInt()
 *
 * This function is basically just a wrapper to each of the test primatives and
 * is called by one of two locations, either the Vp886TestLine function or by
 * the Vp886ServiceTimers function when the VP_DEV_TIMER_TESTLINE expires.
 *
 *----------------------------------------------------------------------------*/
VpStatusType
Vp886TestLineInt(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    bool callback)
{
    VpStatusType status = VP_STATUS_SUCCESS;
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;
    Vp886TestInfoType *pTestInfo = &pLineObj->testInfo;
    Vp886TestHeapType *pTestHeap = pTestInfo->pTestHeap;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886TestLineInt() testId %d at time %u",
        pTestInfo->testId, Vp886GetTimestamp(pLineCtx->pDevCtx)));

    switch (pTestInfo->testId) {
        case VP_TEST_ID_PREPARE:
            #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
            VpSysServiceToggleLed(10);
            #endif
            status = Vp886TestPrepare(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, callback);
            break;

        case VP_TEST_ID_CONCLUDE:
            status = Vp886TestConclude(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
            VpSysServiceToggleLed(10);
            #endif
            break;

        #ifndef VP886_VVMT_REMOVE
        case VP_TEST_ID_DELAY:
            status = Vp886TestTimer(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_LOOP_CONDITIONS:
            status = Vp886TestGetLoopCond(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_LOOPBACK:
            status = Vp886TestLoopback(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_OPEN_VXC:
            status = Vp886TestOpenVxc(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_AC_RLOOP:
            status = Vp886TestAcRloop(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_DC_RLOOP:
            status = Vp886TestDcRloop(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_3ELE_RES_LG:
            status = Vp886TestResFlt(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_3ELE_RES_HG:
            status = Vp886TestResFlt(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_XCONNECT:
            status = Vp886TestXConnect(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_MSOCKET_TYPE2:
            status = Vp886TestMSocket(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_3ELE_CAP_CSLAC:
            status = Vp886Test3EleCap(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_VP_MPI_CMD:
            status = Vp886TestVpMpiCmd(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_RAMP:
            status = Vp886TestMetRamp(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        case VP_TEST_ID_RAMP_INIT:
            status = Vp886TestMetRampInit(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;
        #endif

        case VP_TEST_ID_GAIN_TLBX:
            status = Vp886TestGainTlbx(pLineCtx, pLineObj, pArgsUntyped, pTestInfo, pTestHeap, callback);
            break;

        default:
            status = VP_STATUS_INVALID_ARG;
            VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886TestLineInt invalid testId: %i", pTestInfo->testId));
    }

    return status;
}

/*------------------------------------------------------------------------------
 * VpIsTestIdLegal()
 *
 * This function is a wrapper around the Vp886PushEvent() function.
 *
 *----------------------------------------------------------------------------*/
bool VpIsTestIdLegal(
    VpLineCtxType *pLineCtx,
    VpTestIdType testId)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;

    if (pLineObj->testInfo.prepared && (testId == VP_TEST_ID_PREPARE) ) {
        /* don't allow test prepare to run more than once */
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_RESOURCE_NA);

        VP_ERROR(VpLineCtxType, pLineCtx,
            ("VpIsTestIdLegal: Cannot reprepare the line for testing"));
        return FALSE;

    } else if (!pLineObj->testInfo.prepared && (testId != VP_TEST_ID_PREPARE) ) {
        /* don't allow any primatives to run if not prepared */
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_LINE_NOT_READY);

        VP_ERROR(VpLineCtxType, pLineCtx,
            ("VpIsTestIdLegal: Cannot run testId %i unless the line is prepared", testId));
        return FALSE;

    } else if (pLineObj->testInfo.concluding) {
        /* don't allow any test to be called if conclude is in progress */
        Vp886PushTestEvent(pLineCtx, VP_TEST_STATUS_TESTING);
        VP_ERROR(VpLineCtxType, pLineCtx,
            ("VpIsTestIdLegal: Cannot run testId %i while line is concluding", testId));
        return FALSE;
    }
    return TRUE;
}

/*------------------------------------------------------------------------------
 * Vp886PushTestEvent()
 *
 * This function is a wrapper around the Vp886PushEvent() function.
 *
 *----------------------------------------------------------------------------*/
EXTERN void
Vp886PushTestEvent(
    VpLineCtxType *pLineCtx,
    VpTestStatusType errorCode)
{
    Vp886LineObjectType *pLineObj = pLineCtx->pLineObj;

    /* load up the results of the test */
    pLineObj->testResults.testId = pLineObj->testInfo.testId;
    pLineObj->testResults.errorCode = errorCode;

    /* push the event */
    Vp886PushEvent(pLineCtx->pDevCtx,
        pLineObj->channelId,
        VP_EVCAT_TEST,
        VP_LINE_EVID_TEST_CMP,
        pLineObj->testInfo.testId,
        pLineObj->testInfo.handle,
        TRUE);

    return;
}

/*------------------------------------------------------------------------------
 * Vp886CalmodeTestSetup
 * Function that should be called by all primitives to start the device
 * from a known starting point.
 *
 *
 * Parameters:
 *  pLineCtx         - pointer to the line context.
 *  pTestHeap        - Pointer to the stored data in the scratch pad
 *
 * Returns:
 *         TRUE if no error, FALSE otherwise
 * --
 * Result:
 *  - Set specific device registers appropriately so that low DC loop current <18mA
 *     can be generated using the Signal generator DC bias
 *----------------------------------------------------------------------------*/
EXTERN bool Vp886CalmodeTestSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    uint8 opCond = VP886_R_OPCOND_CUT_RX | VP886_R_OPCOND_HIGHPASS_DIS;
    uint8 opFunction = VP886_R_OPFUNC_CODEC_LINEAR;
    uint8 disn = 0x00;
    uint8 sigGenA[VP886_R_SIGAB_LEN];
    uint8 vadcConvConf[VP886_R_SADC_LEN];
    uint8 ssCfg[VP886_R_SSCFG_LEN];

    ssCfg[0] = pTestHeap->scratch.ssCfg[0] & (VP886_R_SSCFG_WBAND | VP886_R_SSCFG_AUTO_SYSSTATE);
    ssCfg[0] |= VP886_R_SSCFG_AUTO_SYSSTATE;
    ssCfg[1] = pTestHeap->scratch.ssCfg[1];

    /*backup icr1 and 2 into the lineObject*/
    if (VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR1_RD, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalmodeTestSetup failed VP886_R_ICR1_RD"));
        return FALSE;
    }

    if (VpSlacRegRead(NULL, pLineCtx, VP886_R_ICR2_RD, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalmodeTestSetup failed VP886_R_ICR2_RD"));
        return FALSE;
    }

    pLineObj->registers.icr1[0] = 0x00;
    pLineObj->registers.icr1[1] = 0x00;
    pLineObj->registers.icr1[2] &= 0xC0;
    pLineObj->registers.icr1[3] &= 0xC0;

    pLineObj->registers.icr4[0] = 0x91;
    pLineObj->registers.icr4[1] = 0x01;
    pLineObj->registers.icr4[2] = 0x00;
    pLineObj->registers.icr4[3] = 0x00;

    pLineObj->registers.icr2[0] = 0x0C;
    pLineObj->registers.icr2[1] = 0x0C;
    pLineObj->registers.icr2[2] &= 0x0C;
    pLineObj->registers.icr2[2] |= 0x01;
    pLineObj->registers.icr2[3] &= 0x0C;
    pLineObj->registers.icr2[3] |= 0x01;

    pLineObj->registers.icr3[0] = 0x00;
    pLineObj->registers.icr3[1] = 0x00;
    pLineObj->registers.icr3[2] = 0x00;
    pLineObj->registers.icr3[3] = 0x00;

    /*
     * Program the VADC so that it does not intefere with the SADC by:
     *   - forcing the VADC selection not to move if SLAC state is changed
     *   - Setting the selection to a non-intrusive selection
     */
    VpMemSet(&vadcConvConf, 0x00, VP886_R_VADC_LEN);
    vadcConvConf[0] = VP886_R_VADC_SM_OVERRIDE;
    vadcConvConf[1] = VP886_R_VADC_SEL_ADC_OFFSET;
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_VADC_WRT, VP886_R_VADC_LEN, vadcConvConf) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalmodeTestSetup() failed VP886_R_VADC_WRT"));
        return FALSE;
    }

    VpMemSet(&sigGenA, 0x00, VP886_R_SIGAB_LEN);
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_SIGAB_WRT, VP886_R_SIGAB_LEN, sigGenA) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalmodeTestSetup() failed VP886_R_SIGAB_WRT"));
        return FALSE;
    }

    /* Disable hi-pass filter and cutoff receive path */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPCOND_WRT, VP886_R_OPCOND_LEN, &opCond) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_SIGCTRL_WRT"));
        return FALSE;
    }
    /* Set to read linear data */
    if(VpSlacRegWrite(NULL, pLineCtx, VP886_R_OPFUNC_WRT, VP886_R_OPFUNC_LEN, &opFunction) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_SIGCTRL_WRT"));
        return FALSE;
    }
    /* Disable DISN */
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_DISN_WRT, VP886_R_DISN_LEN, &disn) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_DISN_WRT"));
        return FALSE;
    }
    /* Disable automatic state changes */
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_SSCFG_WRT, VP886_R_SSCFG_LEN , ssCfg) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_SSCFG_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR1_WRT, VP886_R_ICR1_LEN, pLineObj->registers.icr1) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_ICR1_WRT"));
        return FALSE;
    }
    /* Turn off feed voltage */
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR2_WRT, VP886_R_ICR2_LEN, pLineObj->registers.icr2) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_ICR2_WRT"));
        return FALSE;
    }
    /* Modify longitudinal bias */
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR3_WRT, VP886_R_ICR3_LEN, pLineObj->registers.icr3) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_ICR3_WRT"));
        return FALSE;
    }
    if (VpSlacRegWrite(NULL, pLineCtx, VP886_R_ICR4_WRT, VP886_R_ICR4_LEN, pLineObj->registers.icr4) == FALSE) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886CalModeTestSetup() failed VP886_R_ICR4_WRT"));
        return FALSE;
    }

    return TRUE;
}


EXTERN bool
Vp886VadcSetup(
    VpLineCtxType *pLineCtx,
    Vp886VadcModeType mode,
    Vp886VadcRateType rate,
    bool genInterrupt,
    bool oRideSm,
    Vp886VadcSignalType sig,
    uint16 skipSysFrames,
    uint16 collectSysFrames,
    Vp886LineBusyFlagsType busyFlag)
{

    uint16 skipVadcFrames;
    uint16 collectVadcFrames;
    uint16 origCollectSysFrames = collectSysFrames;
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

    uint8 convConf[VP886_R_VADC_LEN];

    /* argument checking */
    if (mode > VP886_VADC_MODE_MAX) {
        return FALSE;
    }

    if (rate > VP886_VADC_RATE_MAX) {
        return FALSE;
    }

    if (sig > VP886_VADC_SEL_MAX) {
        return FALSE;
    }

    convConf[0] = (genInterrupt ? VP886_R_VADC_TX_INTERRUPT : 0) |
        (oRideSm ? VP886_R_VADC_SM_OVERRIDE : 0) |
        (uint8)rate;

    /* manual override of the ADC such that the device does not change it behind our back */
    if (oRideSm) {
        convConf[0] |= VP886_R_VADC_SM_OVERRIDE;
    }

    switch (mode) {
        case VP886_VADC_RAW:
            break;
        case VP886_VADC_MATH:
            convConf[0] |= VP886_R_VADC_MATH;
            break;
        default:
            return FALSE;
            break;
    }

    convConf[1] = (uint8)sig;

    /*
     * The collectSamps and skipSamps inputs are specified int in 125us steps.
     * However, the VADC will collect and skip samples at the rate specified.
     * Also, the VADC rate has different meanings based on the mode. So
     * we have to determine how many samples to collect / skip based on
     * mode and rate.
     */

    /* TODO: Correct this equation for wideband */

    /* due to how the SADC works, skipSysFrames has to be a min of 2 */
    skipVadcFrames = Vp886SysFrames2VadcFrames(pLineCtx, skipSysFrames, mode, rate);
    if (skipVadcFrames < 2) {
        skipVadcFrames = 2;
    }

    /*
     * if collectSamps is 0 then the VADC runs in continious mode.
     * Unless collectSamps was explicitly set to 0 when the function
     * was called, we must force collectSamps to 1.
     */
    collectVadcFrames = Vp886SysFrames2VadcFrames(pLineCtx, collectSysFrames, mode, rate);
    if ((origCollectSysFrames != 0) && (collectVadcFrames == 0)) {
        collectVadcFrames = 1;
    }

    convConf[2] = (uint8)((collectVadcFrames & 0xFF00) >> 8);
    convConf[3] = (uint8)(collectVadcFrames & 0x00FF);

    convConf[4] = (uint8)((skipVadcFrames & 0xFF00) >> 8);
    convConf[5] = (uint8)(skipVadcFrames & 0x00FF);

    /* need to register the interrupt with the line */
    if (genInterrupt) {
        pLineObj->busyFlags |= busyFlag;
    }

    /* Send down the Voice Converter Configuration */
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_VADC_WRT, VP886_R_VADC_LEN, convConf);

    VP_TEST(VpLineCtxType, pLineCtx,
        ("Vp886VadcSetup: mode %i, rate %i, skip %i, coll %i s 0x%02x",
            mode, rate, skipVadcFrames, collectVadcFrames, sig));

    return TRUE;
}

/*
 * The VADC will collect and skip samples at the rate specified.
 * The VADC rate has different meanings based on the mode. So
 * we have to determine the actual number of VADC samples based on the
 * mode and rate.
 */
static uint16
Vp886SysFrames2VadcFrames(
    VpLineCtxType *pLineCtx,
    uint16 sysFrames,
    Vp886VadcModeType mode,
    Vp886VadcRateType rate)
{
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;
    uint32 vadcFrames = sysFrames;

    /* The VADC collects at twice the rate in wideband */
    switch (pLineObj->options.codec) {
        case VP_OPTION_LINEAR_WIDEBAND:
        case VP_OPTION_ALAW_WIDEBAND:
        case VP_OPTION_MLAW_WIDEBAND:
            vadcFrames = sysFrames * 2;
            break;
        default:
            vadcFrames = sysFrames;
            break;
    }

    if (rate == VP886_VADC_RATE_QUAR) {
        vadcFrames /= 4;
    } else if (rate == VP886_VADC_RATE_HALF) {
        vadcFrames /= 2;
    }

    return (uint16)vadcFrames;
}

EXTERN bool
Vp886SadcSetup(
    VpLineCtxType *pLineCtx,
    Vp886SadcModeType mode,
    Vp886SadcRateType rate,
    Vp886SadcSignalType sig1,
    Vp886SadcSignalType sig2,
    Vp886SadcSignalType sig3,
    Vp886SadcSignalType sig4,
    Vp886SadcSignalType sig5,
    uint16 skipSysFrames,
    uint16 collectSysFrames,
    Vp886LineBusyFlagsType busyFlag)
{
    uint16 skipSadcFrames;
    uint16 collectSadcFrames;
    uint16 origCollectSysFrames = collectSysFrames;
    Vp886LineObjectType *pLineObj =  pLineCtx->pLineObj;

    uint8 convConf[VP886_R_SADC_LEN];

    /* argument checking */
    if (mode > VP886_SADC_MODE_MAX) {
        return FALSE;
    }

    if (rate > VP886_SADC_RATE_MAX) {
        return FALSE;
    }

    if (sig1 > VP886_SADC_SEL_MAX) {
        return FALSE;
    }

    if (sig2 > VP886_SADC_SEL_MAX) {
        return FALSE;
    }

    if (sig3 > VP886_SADC_SEL_MAX) {
        return FALSE;
    }

    if (sig4 > VP886_SADC_SEL_MAX) {
        return FALSE;
    }

    if (sig5 > VP886_SADC_SEL_MAX) {
        return FALSE;
    }

    convConf[0] = VP886_R_SADC_TX_INTERRUPT | VP886_R_SADC_ENABLE | (uint8)rate;

    switch (mode) {
        case VP886_SADC_RAW_SINGLE:
            break;
        case VP886_SADC_RAW_GROUP:
            convConf[0] |= VP886_R_SADC_GROUP_MODE;
            break;
        case VP886_SADC_MATH_SINGLE:
            convConf[0] |= VP886_R_SADC_MATH;
            break;
        case VP886_SADC_MATH_GROUP:
            convConf[0] |= VP886_R_SADC_MATH;
            convConf[0] |= VP886_R_SADC_GROUP_MODE;
            break;
        default:
            return FALSE;
            break;
    }

    convConf[1] = (uint8)sig1;
    convConf[2] = (uint8)sig2;
    convConf[3] = (uint8)sig3;
    convConf[4] = (uint8)sig4;
    convConf[5] = (uint8)sig5;

    /* due to how the SADC works, skipSysFrames has to be a min of 2 */
    skipSadcFrames = Vp886SysFrames2SadcFrames(skipSysFrames, mode, rate);
    if (skipSadcFrames < 2) {
        skipSadcFrames = 2;
    }

    /*
     * if collectSamps is 0 then the SADC runs in continious mode.
     * Unless collectSamps was explicitly set to 0 when the function
     * was called, we must force collectSamps to 1.
     */
    collectSadcFrames = Vp886SysFrames2SadcFrames(collectSysFrames, mode, rate);
    if ((origCollectSysFrames != 0) && (collectSadcFrames == 0)) {
        collectSadcFrames = 1;
    }

    convConf[6] = (uint8)((collectSadcFrames & 0xFF00) >> 8);
    convConf[7] = (uint8)(collectSadcFrames & 0x00FF);

    convConf[8] = (uint8)((skipSadcFrames & 0xFF00) >> 8);
    convConf[9] = (uint8)(skipSadcFrames & 0x00FF);

    /* need to register the interrupt with the line */
    pLineObj->busyFlags |= busyFlag;

    /* Send down the Supervision Converter Configuration */
    VpSlacRegWrite(NULL, pLineCtx, VP886_R_SADC_WRT, VP886_R_SADC_LEN, convConf);

    VP_TEST(VpLineCtxType, pLineCtx,
        ("Vp886SadcSetup: mode %i, rate %i, skip %i, coll %i",
            mode, rate, skipSadcFrames, collectSadcFrames));
    VP_TEST(VpLineCtxType, pLineCtx,
        ("Vp886SadcSetup: s1 0x%02x, s2 0x%02x, s3 0x%02x, s4 0x%02x s5 0x%02x",
            sig1, sig2, sig3, sig4, sig5));

    #if defined(__KERNEL__) && defined(ZARLINK_CFG_INTERNAL)
        VpSysServiceToggleLed(9);
    #endif
    return TRUE;
}

/*
 * The SADC will collect and skip samples at the rate specified.
 * The SADC rate has different meanings based on the mode. So
 * we have to determine the actual number of SADC samples based on the
 * mode and rate.
 */
static uint16
Vp886SysFrames2SadcFrames(
    uint16 sysFrames,
    Vp886SadcModeType mode,
    Vp886SadcRateType rate)
{
    uint32 sadcFrames = sysFrames;

    if ((mode == VP886_SADC_RAW_GROUP) || (mode == VP886_SADC_MATH_GROUP) ) {
        sadcFrames /= 4;
    }

    if (rate == VP886_SADC_RATE_QUAR) {
        sadcFrames /= 4;
    } else if (rate == VP886_SADC_RATE_HALF) {
        sadcFrames /= 2;
    }

    return (uint16)sadcFrames;
}

EXTERN bool
Vp886VadcGetMathData(
    VpLineCtxType *pLineCtx,
    Vp886AdcMathDataType *pMathData,
    int16 offset,
    int16 gain)
{
    uint8 data[VP886_R_VBUFFER_LEN];

    if (pLineCtx == NULL) {
        return FALSE;
    }

    if (pMathData == NULL) {
        return FALSE;
    }

    VpMemSet(pMathData, 0, sizeof(pMathData));

    /* get the math result data from the requested buffer */
    VpSlacRegRead(NULL, pLineCtx, VP886_R_VBUFFER_RD, VP886_R_VBUFFER_LEN, data);

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886VadcGetMathData"));

    return GetMathData(pLineCtx, pMathData, data, offset, gain);
}

EXTERN bool
Vp886SadcGetMathData(
    VpLineCtxType *pLineCtx,
    Vp886SadcBufferType sadcBuf,
    Vp886AdcMathDataType *pMathData,
    int16 offset,
    int16 gain)
{
    uint8 bufReg = VP886_R_B1_RD;
    uint8 data[VP886_R_B1_LEN];

    if (pLineCtx == NULL) {
        return FALSE;
    }

    if (sadcBuf > VP886_SADC_BUF_MAX) {
        return FALSE;
    }

    if (pMathData == NULL) {
        return FALSE;
    }

    VpMemSet(pMathData, 0, sizeof(pMathData));

    switch (sadcBuf) {
        case VP886_SADC_BUF_1:
            bufReg = VP886_R_B1_RD;
            break;
        case VP886_SADC_BUF_2:
            bufReg = VP886_R_B2_RD;
            break;
        case VP886_SADC_BUF_3:
            bufReg = VP886_R_B3_RD;
            break;
        case VP886_SADC_BUF_4:
            bufReg = VP886_R_B4_RD;
            break;
        case VP886_SADC_BUF_5:
            bufReg = VP886_R_B5_RD;
            break;
        default:
            return FALSE;
            break;
    }

    /* get the math result data from the requested buffer */
    VpSlacRegRead(NULL, pLineCtx, bufReg, VP886_R_B1_LEN, data);

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886SadcGetMathData from buff %i", sadcBuf+1));

    /* parse the data bytes into math struct */
    return GetMathData(pLineCtx, pMathData, data, offset, gain);
}

EXTERN uint8
Vp886SadcGetRawData(
    VpLineCtxType *pLineCtx,
    int16 *pDataSamples,
    uint8 numSamples,
    int16 offset,
    int16 gain)
{
    uint8 bufNum = 0;
    uint8 sampleIdx = 0;
    uint8 maxSamp = 0;
    uint8 sampCnt = 0;
    uint8 bufReg = VP886_R_B1_RD;
    uint8 buff[VP886_R_B1_LEN];
    uint8 buffNumMax = (numSamples - 1) / ((VP886_R_B1_LEN - 1) / 2) + 1;
    int32 temp32 = 0;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886SadcGetRawData() numSamples %d, offset %d, gain %d", numSamples, offset, gain));

    if (pLineCtx == NULL) {
        VP_ERROR(None, VP_NULL, ("Vp886SadcGetRawData() NULL line context"));
        return sampCnt;
    }

    if (numSamples > 60) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SadcGetRawData() numSamples (%d) > 60", numSamples));
        return sampCnt;
    }

    if (pDataSamples == NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886SadcGetRawData() NULL pDataSamples"));
        return sampCnt;
    }

    VpMemSet(pDataSamples, 0, numSamples);

    for (bufNum = 0; bufNum < buffNumMax; bufNum++) {
        VpSlacRegRead(NULL, pLineCtx, bufReg, VP886_R_B1_LEN, buff);
        VP_TEST(VpLineCtxType, pLineCtx, ("buff%d %02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X",
            bufNum, buff[0],
            buff[1], buff[2],
            buff[3], buff[4],
            buff[5], buff[6],
            buff[7], buff[8],
            buff[9], buff[10],
            buff[11], buff[12],
            buff[13], buff[14],
            buff[15], buff[16],
            buff[17], buff[18],
            buff[19], buff[20],
            buff[21], buff[22],
            buff[23], buff[24]));

        for (sampleIdx = 1; sampleIdx < VP886_R_B1_LEN; sampleIdx+=2) {
            if ((bufNum == 0) && (sampleIdx == 1)) {
                maxSamp = buff[0] & 0x7F;
            }

            if (sampCnt > maxSamp || sampCnt >= numSamples) {
                return sampCnt;
            }

            temp32 = (int16)((buff[sampleIdx]<<8) | buff[sampleIdx+1]);
            temp32 += offset;
            temp32 *= gain;
            temp32 = VpRoundedDivide(temp32, 1000);
            temp32 = MIN(temp32, VP_INT16_MAX);
            temp32 = MAX(temp32, VP_INT16_MIN+1);
            pDataSamples[sampCnt] = (int16)temp32;
            VP_TEST(VpLineCtxType, pLineCtx, ("pDataSamples[%u]=%i",sampCnt, pDataSamples[sampCnt]));
            sampCnt++;
        }

        bufReg += 2;
    }

    return sampCnt;
}

EXTERN uint8
Vp886VadcGetRawData(
    VpLineCtxType *pLineCtx,
    int16 *pDataSamples,
    uint8 numSamples,
    int16 offset,
    int16 gain)
{
    uint8 bufNum = 0;
    uint8 sampleIdx = 0;
    uint8 maxSamp = 0;
    uint8 sampCnt = 0;
    uint8 bufReg = VP886_R_VBUFFER_RD;
    uint8 buff[VP886_R_VBUFFER_LEN];
    uint8 buffNumMax = (numSamples - 1) / ((VP886_R_B1_LEN - 1) / 2) + 1;
    int32 temp32 = 0;

    VP_TEST(VpLineCtxType, pLineCtx, ("Vp886VadcGetRawData() numSamples %d, offset %d, gain %d", numSamples, offset, gain));

    if (pLineCtx == NULL) {
        VP_ERROR(None, VP_NULL, ("Vp886VadcGetRawData() NULL line context"));
        return sampCnt;
    }

    if (numSamples > 60) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886VadcGetRawData() numSamples (%d) > 60", numSamples));
        return sampCnt;
    }

    if (pDataSamples == NULL) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("Vp886VadcGetRawData() NULL pDataSamples"));
        return sampCnt;
    }

    VpMemSet(pDataSamples, 0, numSamples);

    for (bufNum = 0; bufNum < buffNumMax; bufNum++) {
        VpSlacRegRead(NULL, pLineCtx, bufReg, VP886_R_VBUFFER_LEN, buff);
        VP_TEST(VpLineCtxType, pLineCtx, ("buff%d %02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X %02X%02X",
            bufNum, buff[0],
            buff[1], buff[2],
            buff[3], buff[4],
            buff[5], buff[6],
            buff[7], buff[8],
            buff[9], buff[10],
            buff[11], buff[12],
            buff[13], buff[14],
            buff[15], buff[16],
            buff[17], buff[18],
            buff[19], buff[20],
            buff[21], buff[22],
            buff[23], buff[24]));

        for (sampleIdx = 1; sampleIdx < VP886_R_VBUFFER_LEN; sampleIdx+=2) {
            if ((bufNum == 0) && (sampleIdx == 1)) {
                maxSamp = buff[0] & 0x7F;
            }

            if (sampCnt > maxSamp || sampCnt >= numSamples) {
                return sampCnt;
            }

            temp32 = (int16)((buff[sampleIdx]<<8) | buff[sampleIdx+1]);
            temp32 += offset;
            temp32 *= gain;
            temp32 = VpRoundedDivide(temp32, 1000);
            temp32 = MIN(temp32, VP_INT16_MAX);
            temp32 = MAX(temp32, VP_INT16_MIN+1);
            pDataSamples[sampCnt] = (int16)temp32;
            VP_TEST(VpLineCtxType, pLineCtx, ("pDataSamples[%u]=%i",sampCnt, pDataSamples[sampCnt]));
            sampCnt++;
        }

        bufReg += 2;
    }

    return sampCnt;
}


static bool GetMathData(
    VpLineCtxType *pLineCtx,
    Vp886AdcMathDataType *pMathData,
    uint8 *pData,
    int16 offset,
    int16 gain)
{
    uint8 data[VP886_R_B1_LEN];

    int32 temp32 = 0;
    uint32 sum32;
    int64tl sum64;
    int64tl sumSqr64;
    int64tl temp64;
    int32 avgSqr32;

    if (pMathData == NULL) {
        return FALSE;
    }

    VpMemCpy(data, pData, 25); /* <---- Fix this no reason to make a local copy S.H. */
    VpMemSet(pMathData, 0, sizeof(pMathData));

    /* parse the data */
    pMathData->lastSample = (((uint16)data[0]  << 8)  |  (uint16)data[1]);
    pMathData->numSamples = (((uint16)data[2]  << 8)  |  (uint16)data[3]);
    pMathData->minVal     = (((uint16)data[4]  << 8)  |  (uint16)data[5]);
    pMathData->maxVal     = (((uint16)data[6]  << 8)  |  (uint16)data[7]);
    sum32                 = (((uint32)data[8]  << 24) | ((uint32)data[9] << 16) |
                             ((uint32)data[10] << 8)  |  (uint32)data[11]);

    /* store the sum of squares as a 64 bit number for later */
    sumSqr64.high32       = (int32)(((uint16)data[12] << 8)  |  (uint16)data[13]);

    sumSqr64.low32        = (uint32)(((uint32)data[14] << 24) | ((uint32)data[15] << 16) |
                                     ((uint32)data[16] << 8)  |  (uint32)data[17]);

    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:offset: %i", offset));
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:gain: %i", gain));
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:numSamples %i", pMathData->numSamples));

    /* correct each of the measurements with the offset and gain info */
    temp32 = pMathData->lastSample;
    temp32 += offset;
    temp32 *= gain;
    temp32 /= 1000;
    temp32 = MIN(temp32, VP_INT16_MAX);
    temp32 = MAX(temp32, VP_INT16_MIN+1);
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:lastSample orig:%i corr:%i", pMathData->lastSample, (int16)temp32));
    pMathData->lastSample = (int16)temp32;

    temp32 = pMathData->minVal;
    temp32 += offset;
    temp32 *= gain;
    temp32 /= 1000;
    temp32 = MIN(temp32, VP_INT16_MAX);
    temp32 = MAX(temp32, VP_INT16_MIN+1);
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:minVal orig:%i corr:%i", pMathData->minVal, (int16)temp32));
    pMathData->minVal = (int16)temp32;

    temp32 = pMathData->maxVal;
    temp32 += offset;
    temp32 *= gain;
    temp32 /= 1000;
    temp32 = MIN(temp32, VP_INT16_MAX);
    temp32 = MAX(temp32, VP_INT16_MIN+1);
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:maxVal orig:%i corr:%i", pMathData->maxVal, (int16)temp32));
    pMathData->maxVal = (int16)temp32;

    /*
     * cannot calculate the average if no samples were collected
     */
    if (pMathData->numSamples == 0) {
        VP_ERROR(VpLineCtxType, pLineCtx, ("GetMathData invalid numSamps collected 0"));
        return FALSE;
    }

    /*
     * Calculate the calibrated sum from the circuit sum
     * Sum_cir = (Sum_cal/G - O*N)
     *   so
     * Sum_cal = (Sum_cir - (O*N)) * G
     */

    /* cast circuit sum into 64 bits */
    sum64 = int64_from_int32(sum32);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sum64: high %li low %li",
            sum64.high32, (uint32)sum64.low32));


    /* multiply the offset and number of samples, then add it to the circuit sum */
    temp64 = int64_mul_int32_int32((int32)offset, (int32)pMathData->numSamples);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:(O*N): high %li low %li",
            temp64.high32, temp64.low32));

    sum64 = int64_add(sum64, temp64);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sum64 + (O*N): high %li low %li",
            sum64.high32, sum64.low32));


    /* remove the circuit gain. */
    sum64 = int64_mul_int64_int32(sum64, (int32)gain);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sum64*G: high %li low %li",
            sum64.high32, sum64.low32));



    sum64 =  int64_div_int64_int32(sum64, 1000);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sum64/1000: high %li low %li",
            sum64.high32, sum64.low32));

    /*
     * Sum should only ever be in the low order 32
     * bits so we can cast it down to 32 bits
     */

    /*
     * calculate the average from the calibrated sum
     * each sample is only 16 bits so the average should
     * never need more than 16 bits.
     */
    temp32 = (int32)sum64.low32;
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:sum: orig: %li corr:%li", sum32, temp32));
    temp32 = (temp32 / pMathData->numSamples);
    temp32 = MIN(temp32, VP_INT16_MAX);
    temp32 = MAX(temp32, VP_INT16_MIN+1);
    pMathData->average = (int16)temp32;
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:average: %i", pMathData->average));

    /*
     * In order to preseve 2 decimal places for the average^2 calculation
     * I am going to multiply the temp32 by 100 before doing the divide
     * then divide by 100^2 after the performing the average^2.
     */
    temp32 = (int32)sum64.low32;
    temp64 = int64_mul_int32_int32(temp32, 100);
    temp64 = int64_div_int64_int32(temp64, pMathData->numSamples);
    if ((temp64.high32 < -1) || (temp64.high32 > 1)) {
        VP_TEST_CALC(VpLineCtxType, pLineCtx,
            ("GetMathData:(average)^2: OVERFLOW using 32 bit math instead"));
        avgSqr32 = (pMathData->average * pMathData->average);
    } else {
        temp32 = (int32)temp64.low32;
        temp64 = int64_mul_int32_int32(temp32, temp32);
        temp64 = int64_div_int64_int32(temp64, 100 * 100); /* rounding issue */
    }
    avgSqr32 = (int32)temp64.low32;
    VP_TEST_CALC(VpLineCtxType, pLineCtx, ("GetMathData:(average)^2: %li", avgSqr32));

    /*
     * Calculate the calibrated sum of squares from the circuit sum of squares
     * SumSq_cir = (Sum_cir)^2
     *           = [(Sum_cal/G - O*)]^2
     *           = (SumSq_cal) * (1/G)^2) - 2*Sum_cal*O/G + N(O)^2
     *  so
     * SumSq_cal * (1/G)^2 = SumSq_cir - N(O)^2 + 2*Sum_cal*O/G
     * SumSq_cal = (SumSq_cal * (1/G)^2) * G^2
     *
     */

    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:sumSqr: orig: 0x%04lx%08lx", sumSqr64.high32, sumSqr64.low32));
    /*
     * Remove N*(O)^2 from circuit sum of squares
     */
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqr64: high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    /* get N(O)^2 into 64 bits */
    temp32 = offset * offset;
    temp64 = int64_mul_int32_int32(temp32, pMathData->numSamples);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:N(O)^2: high %li low %li",
            temp64.high32, (uint32)temp64.low32));

    /* remove N(O)^2 from circuit sum of squares */
    sumSqr64 = int64_sub(sumSqr64, temp64);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqr64 - N(O)^2: high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    /*
     * Remove 2*Sum_cal*O/G from circuit sum of squares
     */
    temp64 = int64_mul_int64_int32(sum64, (2000 * offset));
    temp64 = int64_div_int64_int32(temp64, gain);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:(2*SumCorr*O/G): high %li low %li",
            temp64.high32, (uint32)temp64.low32));

    /* add (2*Sum_cal*O)/G from circuit sum of squares */
    sumSqr64 = int64_add(sumSqr64, temp64);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqr64 + (2*SumCorr*O/G): high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    /*
     * Remove (G)^2 from the circuit sum of squares
     *
     * to ensure I dont go over 64 bits or loose
     * bits to rounding I am doing this in several steps
     */
    sumSqr64 = int64_mul_int64_int32(sumSqr64, (int32)gain);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqr64: high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    sumSqr64 = int64_div_int64_int32(sumSqr64, 1000);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqr64: high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    sumSqr64 = int64_mul_int64_int32(sumSqr64, (int32)gain);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqr64: high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    sumSqr64 = int64_div_int64_int32(sumSqr64, 1000);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqr64: high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:sumSqr: corr: 0x%04lx%08lx", sumSqr64.high32, sumSqr64.low32));

    /*
     * need the average sum of squares to compute rms
     */
    sumSqr64 = int64_div_int64_int32(sumSqr64, pMathData->numSamples);
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:sumSqrAvg64: high %li low %li",
            sumSqr64.high32, (uint32)sumSqr64.low32));

    /*
     * Once again the average sum of squares should be in the
     * lower 32 bits only
     */
    pMathData->avgOfSqrs = (int32)sumSqr64.low32;
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:sumSqrAvg: %li", pMathData->avgOfSqrs));
    /*
     * Finally calculate rms = sqrt((average)^2 - avgOfSqrs)
    */
    avgSqr32 -= pMathData->avgOfSqrs;
    if (avgSqr32 < 0) {
        avgSqr32 *= -1;
    }
    VP_TEST_CALC(VpLineCtxType, pLineCtx,
        ("GetMathData:((average)^2 - avgOfSqrs): %li", avgSqr32));

    pMathData->rms =  VpComputeSquareRoot(avgSqr32);
    VP_TEST(VpLineCtxType, pLineCtx, ("GetMathData:rms: %i", pMathData->rms));

    return TRUE;
}


/*
 * Fixed point int32 cast into 64 bit number
 */
static int64tl int64_from_int32(
    int32 x)
{
    int64tl ret;
    ret.high32 = (x < 0 ? -1 : 0);
    ret.low32  =  x;

    return ret;
}

/*
 * Fixed point int64tl > compare
 */
static int int64_cmp_gt(
    int64tl x,
    int64tl y)
{
    return ((x.high32 > y.high32) || ((x.high32 == y.high32) && (x.low32 >  y.low32)));
}

/*
 * Fixed point int64tl >= compare
 */
static int int64_cmp_ge(
    int64tl x,
    int64tl y)
{
    return ((x.high32 > y.high32) || ((x.high32 == y.high32) && (x.low32 >= y.low32)));
}

/*
 * Fixed point negate 64 bit number
 */
static int64tl int64_neg(
    int64tl x)
{
    int64tl ret;
    ret.high32 = ~x.high32;
    ret.low32 = ~x.low32 + 1;
    if(ret.low32 == 0) {
        ret.high32++;
    }
    return ret;
}

/*
 * Fixed point int64tl addition into int64
 */
static int64tl int64_add(
    int64tl x,
    int64tl y)
{
    int64tl ret;
    ret.high32 = x.high32 + y.high32;
    ret.low32 = x.low32 + y.low32;

    /*
     * The lower 32 math is unsigned. Therefore if the resulting
     * addition of the lower numbers yields a number that is
     * less than either of the original numbers than a carry
     * was generated.
     */
    if ((ret.low32 < x.low32) || (ret.low32 < y.low32)) {
        ret.high32++;
    }

    return ret;
}

/*
 * Fixed point int64tl subtraction into int64
 */
static int64tl int64_sub(
    int64tl x,
    int64tl y)
{
    return int64_add(x, int64_neg(y));
}

/*
 * Fixed point 8 bit shift of a 64 bit number
 */
static int64tl int64_shift(
    int64tl x,
    int8 y)
{
    int64tl ret;
    ret.high32 = 0;
    ret.low32  = 0;

    if(y > 0) {
        if (y < 32) {
            ret.high32 = (x.high32 << y) | (x.low32 >> (32 - y));
            ret.low32 = (x.low32 << y);
        }
    } else {
        y = -y;
        if (y < 32) {
            ret.low32 = (x.low32 >> y) | (x.high32 << (32 - y));
            ret.high32 = (x.high32 >> y);
        }
    }
    return ret;
}

/*
 * Fixed point int32 multiplication into 64 bit number
 */
static int64tl int64_mul_int32_int32(
    int32 x,
    int32 y)
{
    uint16 _x[2];
    uint16 _y[2];
    uint32 _r[3];
    uint32 _t;
    int64tl ret;

    int neg = ((x ^ y) < 0);

    if (x < 0) {
        x = -x;
    }
    if (y < 0) {
        y = -y;
    }

    _x[0] = (x >> 16);
    _x[1] = (x & 0xFFFF);

    _y[0] = (y >> 16);
    _y[1] = (y & 0xFFFF);

    _r[0] = (_x[0] * _y[0]);
    _r[1] = (_x[1] * _y[0]) + (_x[0] * _y[1]);
    _r[2] = (_x[1] * _y[1]);

    ret.high32 = _r[0] + (_r[1] >> 16);
    _t = (_r[1] << 16);
    ret.low32  = _r[2] +  _t;

    /* check for a carry of the lower 32 */
    if ( (ret.low32 < _r[2]) || ( ret.low32 < _t) ) {
        ret.high32 += 1;
    }

    return (neg ? int64_neg(ret) : ret);
}

/*
 * Fixed point int64, int32 multiplication into 64 bit number
 */
static int64tl int64_mul_int64_int32(
    int64tl x,
    int32 y)
{
    uint16 _x[4];
    uint16 _y[2];
    uint32 _r[5];
    uint32 _t;
    int64tl ret;

    int neg = ((x.high32 ^ y) < 0);
    if (x.high32 < 0) {
        x = int64_neg(x);
    }
    if (y < 0) {
        y = -y;
    }

    _x[0] = (x.high32 >> 16);
    _x[1] = (x.high32 & 0xFFFF);
    _x[2] = (x.low32 >> 16);
    _x[3] = (x.low32 & 0xFFFF);

    _y[0] = (y >> 16);
    _y[1] = (y & 0xFFFF);

    /*
     * The upper most term _r[0] will always be unused as it would
     * overflow 64 bits. So there is no need to waste time
     * calculating the value. Might even want to reduce the
     * size of the array since its unused.
     * _r[0] = (_x[0] * _y[0]);
     */
    _r[1] = (_x[1] * _y[0]) + (_x[0] * _y[1]);
    _r[2] = (_x[1] * _y[1]) + (_x[2] * _y[0]);
    _r[3] = (_x[3] * _y[0]) + (_x[2] * _y[1]);
    _r[4] = (_x[3] * _y[1]);

    _t = (_r[3] << 16);
    ret.low32  = _r[4] + _t;
    ret.high32 = _r[2] + (_r[3] >> 16) + (_r[1] << 16);

    /* check for a carry of the lower 32 */
    if ( (ret.low32 < _r[4]) || ( ret.low32 < _t) ) {
        ret.high32 += 1;
    }


    return (neg ? int64_neg(ret) : ret);
}

/*
 * Fixed point int64, int32 divition into 64 bit number
 */
static int64tl int64_div_int64_int32(
    int64tl x,
    int32 y)
{
    uint32 y_u;
    int64tl ret;
    int64tl _y;
    int64tl _i;

    int neg = ((x.high32 ^ y) < 0);

    if (x.high32 < 0) {
        x = int64_neg(x);
    }

    /* Use a uint32 instead of simply "y = -y" to handle the case where
       y = 0x80000000 */
    if (y < 0) {
        y_u = -y;
    } else {
        y_u = y;
    }

    ret.high32 = (x.high32 / y_u);
    ret.low32 = (x.low32 / y_u);

    x.high32 = x.high32 % y_u;
    x.low32 = x.low32 % y_u;

    _y.high32 = 0;
    _y.low32 = y_u;

    for(_i = int64_from_int32(1); int64_cmp_gt(x, _y); _y = int64_shift(_y, 1), _i = int64_shift(_i, 1));

    while(x.high32) {
        _y = int64_shift(_y, -1);
        _i = int64_shift(_i, -1);
        if (int64_cmp_ge(x, _y)) {
            x = int64_sub(x, _y);
            ret = int64_add(ret, _i);
        }
    }

    ret = int64_add(ret, int64_from_int32(x.low32 / y_u));

    return (neg ? int64_neg(ret) : ret);
}

#endif /* VP886_INCLUDE_TESTLINE_CODE */
#endif /* VP_CC_886_SERIES */
