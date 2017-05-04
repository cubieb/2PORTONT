/*
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef TESTLINE_INT_H
#define TESTLINE_INT_H

/*
 * Indicates a max number of samples a test
 * primative will perform calculations on per tick
 * during its calculation state.
 */

#include "vp_api_cfg.h"
#include "vp880_api_int.h"

#define VP880_STATE_CHANGE_SETTLING_TIME 100  /* number is in ms */
#define VP880_STATE_CHANGE_SETTLING_TIME_SHORT 50 /* number is in ms */
#define VP880_FEED_DISCHARGE_TIME 10          /* number is in ms */
#define VP880_ADC_CHANGE_SETTLING_TIME 5      /* number is in ms */
#define VP880_TESTLINE_GLB_IVLD_STATE -1
#define VP880_TESTLINE_GLB_STRT_STATE 0
#define VP880_TESTLINE_GLB_END_STATE 127
#define VP880_TESTLINE_GLB_QUIT_STATE 128
#ifndef NEXT_TICK
#define NEXT_TICK 1                     /* number is in ticks */
#endif


#define VP880_LEGAL_A2D_CVRT_VALS ( (1 << VP880_SWITCHER_Y)| \
    (1 << VP880_SWITCHER_Z)| \
    (1 << VP880_XBR)| \
    (1 << VP880_TIP_TO_GND_V)| \
    (1 << VP880_RING_TO_GND_V)| \
    (1 << VP880_METALLIC_DC_V)| \
    (1 << VP880_METALLIC_DC_I)| \
    (1 << VP880_LONGITUDINAL_DC_I)| \
    (1 << VP880_NO_CONNECT)| \
    (1 << VP880_LOW_TIP_TO_GND_V)| \
    (1 << VP880_LOW_RING_TO_GND_V) )

/* Converts ms to units of API_TICKRATE */
#define VP880_TOTAL_PCM_COLLECTION_TIME(INTEGRATE_TIME, SETTLE_TIME)  \
    (uint16)((((uint32)INTEGRATE_TIME*125)+((uint32)SETTLE_TIME*125)) / 1000) \
        + (uint16)VP880_PCM_CALCULATION_TIME

/* Converts pcm time to samples at the current api tick rate*/
#define PCM_TIME_TO_API_TICKS(TIME, TICK_RATE) \
    (uint16)( ((uint32)TIME*125) / ((uint32)TICK_RATE/256 * 1000))

EXTERN VpStatusType
Vp880TestConclude (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestCalibrate (
    VpLineCtxType *pLineCtx,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestOpenVxc (
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestAcRloop(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestDcRloop(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880Test3EleResHG(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880Test3EleResLG(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestMSocket(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestGetXConnect(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880Test3EleCap(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestGetLoopCond(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestLoopback(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestTimer(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestInitMetRamp(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestMetRamp(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN VpStatusType
Vp880TestFltDscrm(
    VpLineCtxType *pLineCtx,
    const void *pArgsUntyped,
    uint16 handle,
    bool callback,
    VpTestIdType testId);

EXTERN void
VpGenerateTestEvent (
    VpLineCtxType *pLineCtx,
    VpTestIdType testId,
    uint16 handle,
    bool cleanUp,
    bool goToDisconnect);

EXTERN void
Vp880CommonTestSetup (
    VpLineCtxType *pLineCtx,
    VpDeviceIdType deviceId);

EXTERN bool
Vp880AdcSetup (
    VpLineCtxType *pLineCtx,
    uint8 adRoute,
    bool txAnalogGainEnable);


EXTERN bool
VpStartPcmCollect (
    VpLineCtxType *pLineCtx,
    VpPcmOperationMaskType mask,
    int16 nextState,
    uint16 settlingTime,
    uint16 integrateTime);
bool
VpResultReady(
    Vp880DeviceObjectType *pDevObj,
    int16 *pResult);

EXTERN void
VpSetTestStateAndTimer(
    Vp880DeviceObjectType *pDevObj,
    int16 *pState,
    int16 testState,
    int16 testTime);

EXTERN void
VpGetPcmResult(
    Vp880DeviceObjectType *pDevObj,
    VpPcmOperationBitType pcmOperation,
    bool  txAnalogGainEnable,
    bool  removeOffset,
    uint8 chanId,
    uint8 aToDRoute,
    void *pResult);

EXTERN uint16
VpReturnElapsedTime (
    Vp880DeviceObjectType *pDevObj,
    uint16 timeStamp);

#endif  /* TESTLINE_INT_H */
