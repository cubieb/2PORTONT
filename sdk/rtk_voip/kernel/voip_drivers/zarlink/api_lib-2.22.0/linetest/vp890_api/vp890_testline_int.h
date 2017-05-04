/*
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 4942 $
 * $LastChangedDate: 2010-05-18 14:32:58 -0500 (Tue, 18 May 2010) $
 */

#ifndef TESTLINE_INT_H
#define TESTLINE_INT_H

/*
 * Indicates a max number of samples a test
 * primative will perform calculations on per tick
 * during its calculation state.
 */

#include "vp_api_cfg.h"
#include "vp890_api_int.h"

#define VP890_STATE_CHANGE_SETTLING_TIME 100        /* number is in ms */
#define VP890_STATE_CHANGE_SETTLING_TIME_SHORT 50   /* number is in ms */
#define VP890_STATE_CHANGE_SETTLING_TIME_FAST  10   /* number is in ms */
#define VP890_FEED_DISCHARGE_TIME 10                /* number is in ms */
#define VP890_ADC_CHANGE_SETTLING_TIME 5            /* number is in ms */
#define VP890_TESTLINE_GLB_IVLD_STATE -1
#define VP890_TESTLINE_GLB_STRT_STATE 0
#define VP890_TESTLINE_GLB_END_STATE 127
#define VP890_TESTLINE_GLB_QUIT_STATE 128
#ifndef NEXT_TICK
#define NEXT_TICK 1                                 /* number is in ticks */
#endif

#define VP890_LEGAL_A2D_CVRT_VALS ( (1 << VP890_SWITCHER_Y)| \
    (1 << VP890_TIP_TO_GND_V)| \
    (1 << VP890_RING_TO_GND_V)| \
    (1 << VP890_METALLIC_DC_V)| \
    (1 << VP890_METALLIC_DC_I)| \
    (1 << VP890_LONGITUDINAL_DC_I)| \
    (1 << VP890_NO_CONNECT)| \
    (1 << VP890_LOW_TIP_TO_GND_V)| \
    (1 << VP890_LOW_RING_TO_GND_V) )

/* Converts ms to units of API_TICKRATE */
#define VP890_TOTAL_PCM_COLLECTION_TIME(INTEGRATE_TIME, SETTLE_TIME)  \
    (uint16)((((uint32)INTEGRATE_TIME*125)+((uint32)SETTLE_TIME*125)) / 1000) \
        + (uint16)VP890_PCM_CALCULATION_TIME

/* Converts pcm time to samples at the current api tick rate*/
#define PCM_TIME_TO_API_TICKS(TIME, TICK_RATE) \
    (uint16)( ((uint32)TIME*125) / ((uint32)TICK_RATE/256 * 1000))

EXTERN VpStatusType
Vp890TestConclude (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestCalibrate(
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890Test3EleResHG(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890Test3EleResLG(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestMSocket(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestGetXConnect(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestAcRloop(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestHybridLoss(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890Test3EleCap(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestLoopback(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestGetLoopCond(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestDcRloop(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestMetRamp(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestInitMetRamp(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestFltDscrm(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestOpenVxc (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp890TestTimer (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN void
Vp890GenerateTestEvent (
    VpLineCtxType *pLineCtx,
    VpTestIdType testId,
    uint16 handle,
    bool cleanUp,
    bool goToDisconnect);

EXTERN bool
Vp890StartPcmCollect (
    VpLineCtxType *pLineCtx,
    VpPcmOperationMaskType mask,
    int16 nextState,
    uint16 settlingTime,
    uint16 integrateTime);

EXTERN void
Vp890CommonTestSetup (
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId);

EXTERN bool
Vp890AdcSetup (
    VpLineCtxType *pLineCtx,
    uint8 adRoute,
    bool txAnalogGainEnable);

EXTERN void
Vp890GetPcmResult(
    Vp890DeviceObjectType *pDevObj,
    VpPcmOperationBitType pcmOperation,
    bool  txAnalogGainEnable,
    bool  removeOffset,
    uint8 chanId,
    uint8 aToDRoute,
    void *pResult);

EXTERN void
Vp890SetTestStateAndTimer(
    Vp890DeviceObjectType *pDevObj,
    int16 *pState,
    int16 testState,
    uint16 testTime);

EXTERN uint16
Vp890ReturnElapsedTime (
    Vp890DeviceObjectType *pDevObj,
    uint16 timeStamp);

#endif  /* TESTLINE_INT_H */
