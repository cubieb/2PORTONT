/** \lt_api_vp886_int.h
 *
 * This file is internal to Line Test API. It contains definitions/declarations
 * used LT-API.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11004 $
 * $LastChangedDate: 2013-06-12 15:30:24 -0500 (Wed, 12 Jun 2013) $
 */

#ifndef _LT_API_VP886_INT_H_
#define _LT_API_VP886_INT_H_

#include "lt_api.h"

#ifdef LT_VP886_PACKAGE

/* commonly used values */
#define LT_VP886_STATE_CHANGE_SETTLE            960             /* 120ms  */
#define LT_VP886_TEST_SWITCH_SETTLE             320             /* 40 ms     */
#define LT_VP886_MAX_VPK_VOLTAGE                (154 * 1000)    /* 154 V   */
#define LT_VP886_MIN_FREQ                       (1 * 1000)      /* 1 Hz    */
#define LT_VP886_MAX_FREQ                       (3000 * 1000)   /* 3000 Hz */

/* #defines used in lineV (hemf and femf) test */
#define LT_VP886_LINE_V_INTEGRATE_TIME          800             /* 100 ms */
#define LT_VP886_LINE_V_SETTLE_TIME             40              /* 5ms      */

/* #defines used in resflt test */
#define LT_VP886_RES_FLT_INTEGRATE_TIME         146             /* 18.25 ms */
#define LT_VP886_RES_FLT_SETTLE_TIME            40              /* 5ms      */
#define LT_VP886_RES_FLT_HG_MIN_ILG             -5000           /* -5000 uA */
#define LT_VP886_RES_FLT_DEFAULT_HG_MAX_ILG     32000           /* 32000 uA */
#define LT_VP886_RES_FLT_SHORT_TO_GND           1000            /* 10 Ohms */

/* #defines used in roh test */
#define LT_VP886_ROH_DRIVE_CURRENT_1            (30* 1000)      /* 30 mA    */
#define LT_VP886_ROH_DRIVE_CURRENT_2            (20* 1000)      /* 20 mA    */
#define LT_VP886_ROH_DRIVE_CURRENT_3            ( 5 * 1000)     /*  5 mA    */
#define LT_VP886_ROH_DRIVE_CURRENT_4            (-5* 1000)      /* -5 mA    */
#define LT_VP886_ROH_QUICK_V_CHECK              5187            /* 38 V     */
#define LT_VP886_ROH_QUICK_I_CHECK              7801            /* 10 mA    */

/* #defines used in ringers (regular ren) test */
#define LT_VP886_REG_RINGERS_MAX_INPUTV         VP886_AC_RLOOP_MAX_TEST_LVL
#define LT_VP886_REG_RINGERS_TEST_BIAS          0               /*   0 V    */
#define LT_VP886_REG_RINGERS_SETTLE_TIME        800             /*   100 ms */
#define LT_VP886_REG_RINGERS_RING_CYCLES        2               /*   cycles */

/* #defines used in ringers (electronic ren) test */
#define LT_VP886_ELEC_RINGERS_INTEGRATE_TIME    146             /* 18.25  ms*/
#define LT_VP886_ELEC_RINGERS_SETTLE_TIME       40              /* 5 ms */
#define LT_VP886_ELEC_RINGERS_RAMP_SLOPE        -41             /* ~322 V/s */
#define LT_VP886_ELEC_RINGERS_VRATE(renFactor)  \
    ((renFactor/1210000) > 1) ? \
    (LT_VP886_ELEC_RINGERS_RAMP_SLOPE / ((renFactor/1210000) + 1)) : \
    LT_VP886_ELEC_RINGERS_RAMP_SLOPE

/* #defines used in DC Feed Self Test */
#define LT_VP886_DC_FEED_SETTLE_TIME            80              /* 10 ms    */
#define LT_VP886_DC_FEED_INTEGRATE_TIME         800             /* 100ms    */

/* #defines used in the ringing self test Test */
#define LT_VP886_RINGING_ST_MAX_INPUTV          VP886_AC_RLOOP_MAX_BAT_LVL
#define LT_VP886_RINGING_ST_SETTLE_TIME         80          /* 10 ms */
#define LT_VP886_RINGING_ST_RING_CYCLES         3           /* 3 ringing cycles */
#define LT_VP886_RINGING_ST_ACRLOOP_ILR         50000       /* 50mA peak */
#define LT_VP886_RINGING_ST_SOURCE_IMPEDANCE    2000        /* 200 Ohms */

/* #defines used in 3 element cap test */
#define LT_VP886_CAP_MIN_AMP          100                 /* Amplitude min RMS in mV */
#define LT_VP886_CAP_MAX_AMP          2000                /* Amplitude max RMS in mV */
#define LT_VP886_CAP_MIN_CAL          0                   /* min calibration cap */
#define LT_VP886_CAP_MAX_CAL          (VP_INT32_MAX / 2)  /* max calibration cap */

/* #defines used in Battery Voltage Test */
#define LT_VP886_RD_BAT_COND_INTEGRATE_TIME     80          /* 10 ms    */
#define LT_VP886_RD_BAT_COND_SETTLE_TIME        80          /* 10 ms    */

/* #defines used in DC Voltage Test */
#define LT_VP886_DC_VOLTAGE_RAMP_SLOPE          -41         /* 322 V/s  */
#define LT_VP886_DC_VOLTAGE_INTEGRATE_TIME      146         /* 18.25ms ms*/
#define LT_VP886_DC_VOLTAGE_SETTLE_TIME         80          /* 10 ms     */
#define LT_VP886_DC_VOLTAGE_MAX_RAMP_SIZE       135000      /* 135V */

/* Internal LT-API */
extern LtTestStatusType
LtVp886StartTest(
    VpLineCtxType           *pLineCtx,
    LtTestIdType            testId,
    uint16                  handle,
    LtTestAttributesType    *pAttributes,
    LtTestTempType          *pTemp,
    LtTestResultType        *pResult,
    LtTestCtxType           *pTestCtx);

extern LtTestStatusType
LtVp886EventHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);

extern LtTestStatusType
LtVp886AbortTest(
    LtTestCtxType *pTestCtx);

/* Internal VP-API Test Wrappers */
extern LtTestStatusType
Vp886TestLineWrapper(
    LtTestCtxType *pTestCtx,
    VpTestIdType test,
    const void * pArgs);

extern LtTestStatusType
Vp886SetRelayStateWrapper(
    LtTestCtxType *pTestCtx,
    VpRelayControlType relayState);

extern LtTestStatusType
Vp886SetLineStateWrapper(
    LtTestCtxType *pTestCtx,
    VpLineStateType lineState);

extern LtTestStatusType
Vp886GetLineStatusWrapper(
    LtTestCtxType *pTestCtx,
    VpInputType input,
    bool *pLineStatus);

extern bool
Vp886EventErrorCheck(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

/*
 * Argument Checking and Test Handle Functions
 * Note that all Handler Functions are declared
 * extern so that they could be used by a "group test"
 * such as the 909 all test.
 */

#ifdef LT_LINE_V_886
extern bool
LtVp886LineVAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_LINE_V_886 */

#ifdef LT_ROH_886
extern bool
LtVp886RohAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_ROH_886 */

#ifdef LT_RINGERS_886
extern bool
LtVp886RingersAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_RINGERS_886 */

#ifdef LT_RES_FLT_886
extern bool
LtVp886ResFltAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_RES_FLT_886 */

#ifdef LT_MSOCKET_886
extern bool
LtVp886MSocketAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_MSOCKET_886 */

#ifdef LT_XCONNECT_886
extern bool
LtVp886XConnectAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_XCONNECT_886 */

#ifdef LT_CAP_886
extern bool
LtVp886CapAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_CAP_886 */

#ifdef LT_LOOPBACK_886
extern bool
LtVp886LoopbackAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_LOOPBACK_886 */

#ifdef LT_DC_FEED_ST_886
extern bool
LtVp886DcFeedStAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_DC_FEED_ST_886 */

#ifdef LT_RD_LOOP_COND_886
extern bool
LtVp886RdLoopCondAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_RD_LOOP_COND_886 */

#ifdef LT_DC_VOLTAGE_886
extern bool
LtVp886DcVoltageAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_DC_VOLTAGE_886 */

#ifdef LT_RINGING_ST_886
extern bool
LtVp886RingingStAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_RINGING_ST_886 */

#ifdef LT_ON_OFF_HOOK_ST_886
extern bool
LtVp886OnOffHookStAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_ON_OFF_HOOK_ST_886 */

#ifdef LT_RD_BAT_COND_886
extern bool
LtVp886RdBatCondAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_RD_BAT_COND_886 */

#ifdef LT_PRE_LINE_V_886
extern bool
LtVp886PreLineVAttributeCheck(
    LtTestCtxType *pTestCtx);
#endif /* LT_PRE_LINE_V_886 */

/*
 * Group Test
 * Argument Checking and Test Handle Functions
 */
#if defined(LT_ALL_GR_909_886)
extern bool
LtVp886AllGR909AttributeCheck(
    LtTestCtxType *pTestCtx);

#endif /* LT_ALL_GR_909_886 */


#endif /* LT_VP886_PACKAGE */

#endif /* !_LT_API_VP886_INT_H_ */




