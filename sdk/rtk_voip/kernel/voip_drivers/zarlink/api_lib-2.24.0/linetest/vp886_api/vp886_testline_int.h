/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 11170 $
 * $LastChangedDate: 2013-09-03 17:42:14 -0500 (Tue, 03 Sep 2013) $
 */

#ifndef VP886_TESTLINE_INT_H
#define VP886_TESTLINE_INT_H

#include "vp_api.h"
#include "vp_api_int.h"
#include "vp886_api_int.h"
#include "vp886_testline.h"
#include "vp_debug.h"

#define VP886_HOOKFREEZE_TEST_CONCLUDE 10

typedef enum {
    VP886_VADC_RAW          = 0,
    VP886_VADC_MATH         = 1,
    VP886_VADC_MODE_MAX     = 1,
    VP886_VADC_MODE_SIZE    = FORCE_STANDARD_C_ENUM_SIZE
} Vp886VadcModeType;

typedef enum {
    VP886_SADC_RAW_SINGLE   = 0,
    VP886_SADC_RAW_GROUP    = 1,
    VP886_SADC_MATH_SINGLE  = 2,
    VP886_SADC_MATH_GROUP   = 3,
    VP886_SADC_MODE_MAX     = 3,
    VP886_SADC_MODE_SIZE    = FORCE_STANDARD_C_ENUM_SIZE
} Vp886SadcModeType;

typedef enum {
    VP886_VADC_RATE_FULL    = 0,
    VP886_VADC_RATE_HALF    = 1,
    VP886_VADC_RATE_QUAR    = 2,
    VP886_VADC_RATE_MAX     = 2,
    VP886_VADC_RATE_SIZE    = FORCE_STANDARD_C_ENUM_SIZE
} Vp886VadcRateType;

typedef enum {
    VP886_SADC_RATE_QUAR    = 0,
    VP886_SADC_RATE_HALF    = 1,
    VP886_SADC_RATE_FULL    = 2,
    VP886_SADC_RATE_MAX     = 2,
    VP886_SADC_RATE_SIZE    = FORCE_STANDARD_C_ENUM_SIZE
} Vp886SadcRateType;

typedef enum {
    VP886_SADC_BUF_1        = 0,
    VP886_SADC_BUF_2        = 1,
    VP886_SADC_BUF_3        = 2,
    VP886_SADC_BUF_4        = 3,
    VP886_SADC_BUF_5        = 4,
    VP886_SADC_BUF_MAX      = 4,
    VP886_SADC_BUF_SIZE     = FORCE_STANDARD_C_ENUM_SIZE
} Vp886SadcBufferType;

typedef enum {
    VP886_SADC_SEL_MET_VAC          = VP886_R_SADC_SEL_TIP_RING_AC_V,
    VP886_SADC_SEL_SWY              = VP886_R_SADC_SEL_SWY,
    VP886_SADC_SEL_SWZ              = VP886_R_SADC_SEL_SWZ,
    VP886_SADC_SEL_IO2              = VP886_R_SADC_SEL_IO2X_V,
    VP886_SADC_SEL_TIP              = VP886_R_SADC_SEL_TIP_GROUND_V,
    VP886_SADC_SEL_RING             = VP886_R_SADC_SEL_RING_GROUND_V,
    VP886_SADC_SEL_MET_VDC          = VP886_R_SADC_SEL_TIP_RING_DC_V,
    VP886_SADC_SEL_MET_I            = VP886_R_SADC_SEL_METALLIC_CUR,
    VP886_SADC_SEL_LONG_I           = VP886_R_SADC_SEL_LONG_CUR,
    VP886_SADC_SEL_ICAL_H           = VP886_R_SADC_SEL_HIGH_CAL_CUR,
    VP886_SADC_SEL_LONG_V           = VP886_R_SADC_SEL_TIP_PLUS_RING,
    VP886_SADC_SEL_NO_CONN          = VP886_R_SADC_SEL_ADC_OFFSET,
    VP886_SADC_SEL_ICAL_L           = VP886_R_SADC_SEL_LOW_CAL_CUR,
    VP886_SADC_SEL_TEMP             = VP886_R_SADC_SEL_TEMP,
    VP886_SADC_SEL_IM_PLUS_IL       = VP886_R_SADC_SEL_IM_PLUS_IL,
    VP886_SADC_SEL_IM_MINUS_IL      = VP886_R_SADC_SEL_IM_MINUS_IL,
    VP886_SADC_SEL_VDDSW            = VP886_R_SADC_SEL_VDDSW,
    VP886_SADC_SEL_VMODE_OFF        = VP886_R_SADC_SEL_VMODE_OFFSET,
    VP886_SADC_SEL_VMODE_REF        = VP886_R_SADC_SEL_VMODE_REF,
    VP886_SADC_SEL_SWY_CAL_ERR      = VP886_R_SADC_SEL_SWY_ERR,
    VP886_SADC_SEL_SWZ_CAL_ERR      = VP886_R_SADC_SEL_SWZ_ERR,
    VP886_SADC_SEL_HK_DET_CAL_I     = VP886_R_SADC_SEL_HOOK_DET_CUR,
    VP886_SADC_SEL_GKY_DET_CAL_I    = VP886_R_SADC_SEL_GNDKEY_CUR,
    VP886_SADC_SEL_LONG_CAL_OUT     = VP886_R_SADC_SEL_LONG_CAL_OUT,
    VP886_SADC_SEL_LONG_CAL_IN      = VP886_R_SADC_SEL_LONG_CAL_IN,
    VP886_SADC_SEL_VOC_CAL_OUT      = VP886_R_SADC_SEL_METTALIC_CAL_OUT,
    VP886_SADC_SEL_VOC_CAL_IN       = VP886_R_SADC_SEL_METTALIC_CAL_IN,
    VP886_SADC_SEL_BAT_SW_HIGH      = VP886_R_SADC_SEL_SW_CAL_CMP_HIGH,
    VP886_SADC_SEL_BAT_SW_LOW       = VP886_R_SADC_SEL_SW_CAL_CMP_LOW,
    VP886_SADC_SEL_BAT_SAT_DET      = VP886_R_SADC_SEL_BAT_SAT_DET_CMP,
    VP886_SADC_SEL_MAX              = VP886_R_SADC_SEL_BAT_SAT_DET_CMP,
    VP886_SADC_SEL_SIZE   = FORCE_STANDARD_C_ENUM_SIZE
} Vp886SadcSignalType;

typedef enum {
    VP886_VADC_SEL_MET_VAC          = VP886_R_VADC_SEL_TIP_RING_AC_V,
    VP886_VADC_SEL_SWY              = VP886_R_VADC_SEL_SWY,
    VP886_VADC_SEL_SWZ              = VP886_R_VADC_SEL_SWZ,
    VP886_VADC_SEL_TIP              = VP886_R_VADC_SEL_TIP_GROUND_V,
    VP886_VADC_SEL_RING             = VP886_R_VADC_SEL_RING_GROUND_V,
    VP886_VADC_SEL_MET_VDC          = VP886_R_VADC_SEL_TIP_RING_DC_V,
    VP886_VADC_SEL_MET_I            = VP886_R_VADC_SEL_METALLIC_CUR,
    VP886_VADC_SEL_LG_LONG_I        = VP886_R_VADC_SEL_LOWGAIN_LONG_CUR,
    VP886_VADC_SEL_LONG_I           = VP886_R_VADC_SEL_LONG_CUR,
    VP886_VADC_SEL_LG_MET_I         = VP886_R_VADC_SEL_LOWGAIN_METALLIC_CUR,
    VP886_VADC_SEL_ICAL_H           = VP886_R_VADC_SEL_HIGH_CAL_CUR,
    VP886_VADC_SEL_LOOPBACK         = VP886_R_VADC_SEL_VDAC_LOOPBACK,
    VP886_VADC_SEL_OFFSET           = VP886_R_VADC_SEL_ADC_OFFSET,
    VP886_VADC_SEL_ICAL_L           = VP886_R_VADC_SEL_LOW_CAL_CUR,
    VP886_VADC_SEL_MAX              = VP886_R_VADC_SEL_LOW_CAL_CUR,
    VP886_VADC_SEL_SIZE             = FORCE_STANDARD_C_ENUM_SIZE
} Vp886VadcSignalType;

typedef struct {
    int16   lastSample;
    uint16  numSamples;
    int16   minVal;
    int16   maxVal;
    int16   average;
    uint32  avgOfSqrs;
    uint16  rms;
} Vp886AdcMathDataType;

EXTERN bool
Vp886VadcSetup(
    VpLineCtxType *pLineCtx,
    Vp886VadcModeType mode,
    Vp886VadcRateType rate,
    bool genInterrupt,
    bool oRideSm,
    Vp886VadcSignalType sig,
    uint16 skipSamps,
    uint16 collectSamps,
    Vp886LineBusyFlagsType busyFlag);

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
    uint16 skipSamps,
    uint16 collectSamps,
    Vp886LineBusyFlagsType busyFlag);

EXTERN bool
Vp886VadcGetMathData(
    VpLineCtxType *pLineCtx,
    Vp886AdcMathDataType *pMathData,
    int16 offset,
    int16 gain);

EXTERN bool
Vp886SadcGetMathData(
    VpLineCtxType *pLineCtx,
    Vp886SadcBufferType sadcBuf,
    Vp886AdcMathDataType *pMathData,
    int16 offset,
    int16 gain);

EXTERN uint8
Vp886SadcGetRawData(
    VpLineCtxType *pLineCtx,
    int16 *pDataSamples,
    uint8 numSamples,
    int16 offset,
    int16 gain);

EXTERN uint8
Vp886VadcGetRawData(
    VpLineCtxType *pLineCtx,
    int16 *pDataSamples,
    uint8 numSamples,
    int16 offset,
    int16 gain);

EXTERN void
Vp886PushTestEvent(
    VpLineCtxType *pLineCtx,
    VpTestStatusType errorCode);

EXTERN VpStatusType
Vp886TestPrepare (
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    bool callback);

EXTERN VpStatusType
Vp886TestConclude (
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

#ifndef VP886_VVMT_REMOVE
EXTERN VpStatusType
Vp886TestTimer(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestGetLoopCond(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestLoopback(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestOpenVxc(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestAcRloop(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestDcRloop(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestResFlt(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

#ifdef OLD_HG_RESFT
EXTERN VpStatusType
Vp886TestResFltHG(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);
#endif

EXTERN VpStatusType
Vp886TestXConnect(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestMSocket(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886Test3EleCap(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestVpMpiCmd(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestMetRamp(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN VpStatusType
Vp886TestMetRampInit(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);
#endif

EXTERN VpStatusType
Vp886TestGainTlbx(
    VpLineCtxType *pLineCtx,
    Vp886LineObjectType *pLineObj,
    const void *pArgsUntyped,
    Vp886TestInfoType *pTestInfo,
    Vp886TestHeapType *pTestHeap,
    bool callback);

EXTERN bool
Vp886CalmodeTestSetup(
    VpLineCtxType *pLineCtx,
    Vp886TestHeapType *pTestHeap);

#endif  /* VP886_TESTLINE_INT_H */
