/** \lt_api_vp890_int.h
 *
 * This file is internal to Line Test API. It contains definitions/declarations
 * used LT-API.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#ifndef _LT_API_VP890_INT_H_
#define _LT_API_VP890_INT_H_

#include "lt_api.h"

#ifdef LT_VP890_PACKAGE

/* commonly used values */
#define LT_VP890_NEXT_TICK                      1           /* next tick */
#define LT_VP890_STATE_CHANGE_SETTLE            800         /* 100ms     */
#define LT_VP890_TEST_SWITCH_SETTLE             320         /* 40 ms     */
#define LT_VP890_RINGTRIP_DELAY                 1600        /* 200 ms    */
#define LT_VP890_LOW_POWER_SETTLE               320         /* 40 ms     */

#define LT_VP890_MAX_VPK_VOLTAGE                (154 * 1000) /* 154 V   */
#define LT_VP890_MIN_FREQ                       (1 * 1000)   /* 1 Hz    */
#define LT_VP890_MAX_FREQ                       (3000 * 1000)/* 3000 Hz */

/* #defines used in lineV (hemf and femf) test */
#define LT_VP890_LINE_V_SETTLE_TIME             40          /* 5ms      */
#define LT_VP890_LINE_V_INTEGRATE_TIME          800         /* 100ms    */

/* #defines used in roh  test */
#define LT_VP890_ROH_QUICK_V_CHECK              5460        /* 40 V     */
#define LT_VP890_ROH_QUICK_I_CHECK              7801        /* 10 mA    */
#define LT_VP890_ROH_DRIVE_CURRENT_1            (25* 1000)  /* 25 mA    */
#define LT_VP890_ROH_DRIVE_CURRENT_2            ( 5 * 1000) /*  5 mA    */
#define LT_VP890_ROH_DRIVE_CURRENT_3            (-5* 1000)  /* -5 mA    */

#define LT_VP890_ROH_INTEGRATE_TIME             160         /* 20 ms    */
#define LT_VP890_ROH_SETTLE_TIME                80          /* 10 ms    */

/* #defines used in ringers (ren) test */
#define LT_VP890_RINGERS_MAX_INPUTV             VP890_AC_RLOOP_MAX_TEST_LVL
#define LT_VP890_RINGERS_TEST_BIAS              0           /*   0 V    */
#define LT_VP890_RINGERS_SETTLE_TIME            800         /*   100 ms */
#define LT_VP890_RINGERS_RING_CYCLES            2           /*   ringing cycles */

/* #defines used in resflt test */
#define LT_VP890_RES_FLT_INTEGRATE_TIME         146         /* 18.25ms  */
#define LT_VP890_RES_FLT_SETTLE_TIME            40          /* 5ms      */
#define LT_VP890_RES_FLT_HG_MIN_ILG             -3901        /* -5000 uA in adc*/
#define LT_VP890_RES_FLT_DEFAULT_HG_MAX_ILG     24966       /* 32000 uA in adc*/

/* #defines used in DC Feed Self Test */
#define LT_VP890_DC_FEED_SETTLE_TIME            80          /* 10 ms      */
#define LT_VP890_DC_FEED_INTEGRATE_TIME         800         /* 100ms    */

/* #defines used in DC Voltage Test */
#define LT_VP890_DC_VOLTAGE_RAMP_SLOPE          -41         /* 322 V/s  */
#define LT_VP890_DC_VOLTAGE_INTEGRATE_TIME      146         /* 18.25ms ms*/
#define LT_VP890_DC_VOLTAGE_SETTLE_TIME         80          /* 10 ms     */

/* #defines used in DC Voltage Test */
#define LT_VP890_RD_BAT_COND_INTEGRATE_TIME     146         /* 18.25ms ms*/
#define LT_VP890_RD_BAT_COND_SETTLE_TIME        80          /* 10 ms     */

/* #defines used in the capacitive REN Test */

#define LT_VP890_CAP_RINGERS_VRATE(renFactor)  \
    ((renFactor/1210000) > 1) ? \
    (LT_VP890_CAP_RINGERS_RAMP_SLOPE / ((renFactor/1210000) + 1)) : \
    LT_VP890_CAP_RINGERS_RAMP_SLOPE

#define LT_VP890_CAP_RINGERS_RAMP_SLOPE    -41              /* ~322 V/s */
/*#define LT_VP890_CAP_RINGERS_RAMP_SLOPE    -120 */             /* ~942 V/s */

#define LT_VP890_CAP_RINGERS_INTEGRATE_TIME     146         /* 18.25ms ms*/
#define LT_VP890_CAP_RINGERS_SETTLE_TIME        40          /* 5 ms */

/* #defines used in the ringing self test Test */
#define LT_VP890_RINGING_ST_SETTLE_TIME         80          /* 10 ms */
#define LT_VP890_RINGING_ST_RING_CYCLES         3           /* 3 ringing cycles */
#define LT_VP890_RINGING_ST_ACRLOOP_ILR         50000       /* 50mA peak */
#define LT_VP890_RINGING_ST_SOURCE_IMPEDANCE    2000        /* 200 Ohms */

/* #defines used in 3 element cap test */
#define LT_VP890_CAP_MAX_AMP          2000                /* Amplitude RMS in mV */
#define LT_VP890_CAP_MIN_CAL          0                   /* min calibration cap */
#define LT_VP890_CAP_MAX_CAL          (VP_INT32_MAX / 2)  /* max calibration cap */

extern LtTestStatusType
LtVp890StartTest(
    VpLineCtxType           *pLineCtx,
    LtTestIdType            testId,
    uint16                  handle,
    LtTestAttributesType    *pAttributes,
    LtTestTempType          *pTemp,
    LtTestResultType        *pResult,
    LtTestCtxType           *pTestCtx);

extern LtTestStatusType
LtVp890EventHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);

extern LtTestStatusType
LtVp890AbortTest(
    LtTestCtxType *pTestCtx);

/* Test Handle Function Prototypes */

#ifdef LT_PRE_LINE_V
extern LtTestStatusType
LtVp890PreLineVHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_PRE_LINE_V */

#ifdef LT_LINE_V
extern LtTestStatusType
LtVp890LineVHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_LINE_V */

#ifdef LT_ROH
extern LtTestStatusType
LtVp890RohHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_ROH */

#ifdef LT_RINGERS
extern LtTestStatusType
LtVp890RingerHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_RINGERS */

#ifdef LT_RES_FLT
extern LtTestStatusType
LtVp890ResFltHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_RES_FLT */

#if defined(LT_ALL_GR_909) && defined(LT_RES_FLT) && defined(LT_RINGERS) && defined(LT_ROH) && defined(LT_LINE_V)
extern LtTestStatusType
LtVp890AllGr909Handler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);

extern bool
LtVp890AllGr909RunRoh(
    LtTestCtxType *pTestCtx);

extern bool
LtVp890AllGr909RunRen(
    LtTestCtxType *pTestCtx);
#endif /* LT_ALL_GR_909 */



#ifdef LT_VP890_VVP_PACKAGE

#ifdef LT_RINGERS
extern LtTestStatusType
LtVp890CapRingerHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_RINGERS */

#ifdef LT_LOOPBACK
extern LtTestStatusType
LtVp890LoopbackHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_LOOPBACK */

#ifdef LT_DC_FEED_ST
extern LtTestStatusType
LtVp890DcFeedHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_DC_FEED_ST */

#ifdef LT_RD_LOOP_COND
extern LtTestStatusType
LtVp890RdLoopCondHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_RD_LOOP_COND */

#ifdef LT_MSOCKET
extern LtTestStatusType
LtVp890MSocketHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_MSOCKET */

#ifdef LT_XCONNECT
extern LtTestStatusType
LtVp890XConnectHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_XCONNECT */

#ifdef LT_CAP
extern LtTestStatusType
LtVp890CapHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_CAP */

#ifdef LT_DC_VOLTAGE
extern LtTestStatusType
LtVp890DcVoltageHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_DC_VOLTAGE */

#ifdef LT_RINGING_ST
extern LtTestStatusType
LtVp890RingingStHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_RINGING_ST */

#ifdef LT_ON_OFF_HOOK_ST
extern LtTestStatusType
LtVp890OnOffHookStHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_ON_OFF_HOOK_ST */

#ifdef LT_RD_BAT_COND
extern LtTestStatusType
LtVp890RdBatCondHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_RD_BAT_COND */

#ifdef LT_FLT_DSCRM
extern LtTestStatusType
LtVp890FltDscrmHandler(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent);
#endif /* LT_FLT_DSCRM */


#endif /* LT_VP890_VVP_PACKAGE */


/* test wrappers */
extern LtTestStatusType
Vp890TestLineWrapper(
    LtTestCtxType *pTestCtx,
    VpTestIdType test,
    const void * pArgs);

extern bool
LtVp890CalculateResults(
    LtTestCtxType *pTestCtx,
    LtTestIdType tid);

extern bool
LtVp890IsTestSupported(
    LtTestCtxType *pTestCtx,
    VpDeviceInfoType *pDeviceInfo,
    LtTestIdType testId);

extern bool
LtVp890AttributeCheck(
    LtTestCtxType *pTestCtx,
    const LtTestIdType testId);

extern bool
Vp890EventErrorCheck(
    LtTestCtxType *pTestCtx,
    LtEventType *pEvent,
    LtTestStatusType *pRetval);

extern LtTestStatusType
Vp890SetRelayStateWrapper(
    LtTestCtxType *pTestCtx,
    VpRelayControlType relayState);

extern void
LtVp890AllCopyResults(
    LtTestCtxType *pTestCtx);

#endif /* LT_VP890_PACKAGE */

#endif /* !_LT_API_VP890_INT_H_ */
