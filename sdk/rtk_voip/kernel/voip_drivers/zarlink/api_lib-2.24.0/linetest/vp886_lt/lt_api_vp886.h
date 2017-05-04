/** \lt_api_vp886.h
 *
 * This file contains device specific LT-API interface enums and structures.
 *
 * Copyright (c) 2012, Microsemi Corporation
 *
 * $Revision: 10606 $
 * $LastChangedDate: 2012-11-12 17:10:35 -0600 (Mon, 12 Nov 2012) $
 */

#ifndef _LT_API_VP886_DATA_H_
#define _LT_API_VP886_DATA_H_

#ifdef LT_VP886_PACKAGE

#include "lt_api_vp886_test_sel.h"

/* compile time control of device specific implementations of test */

/* default VoicePort topology values */
#define VP886_TOP_R_SENSE                       10050000L       /* 1 Meg + 5k */
#define VP886_TOP_R_LEAKAGE                     (VP_INT32_MAX)  /* 214.748 MOhms (open circuit) */

/* Define a generic function pointer for the event handler */
typedef bool
    (*Lt886TestEventHandlerFnPtrType) (
        struct LtTestCtxType *pTestCtx,
        LtEventType *pEvent,
        LtTestStatusType *pResult);

/* Topology for VoicePort (Vp886) device family */
typedef struct {
    LtImpedanceType rSenseA;    /* Sense Resistor A */
    LtImpedanceType rSenseB;    /* Sense Resistor B */
    LtImpedanceType rLeakageA;  /* Leakage in Sence path A */
    LtImpedanceType rLeakageB;  /* Leakage in Sence path B */
} LtVp886TestTopologyType;

#ifdef LT_LINE_V_886
typedef struct {
    LtLineVInputType input;
    LtLineVCriteriaType criteria;

    VpTestResultVxcType vpApiTipResults;
    VpTestResultVxcType vpApiRingResults;
    VpTestResultVxcType vpApiDiffResults;
} LtVp886LineVTempType;
#endif /* LT_LINE_V_886 */

#ifdef LT_ROH_886
typedef struct {
    LtRohInputType input;
    LtRohCriteriaType criteria;
    bool quickStop;
    int16 vab1;
    int16 vab2;
    int16 vab3;
    int16 vab4;
    int16 imt1;
    int16 imt2;
    int16 imt3;
    int16 imt4;
} LtVp886RohTempType;
#endif /* LT_ROH_886 */

#ifdef LT_RINGERS_886
typedef struct {
    LtRingerInputType input;
    LtRingersCriteriaType criteria;

    /* input info for ac ringers*/
    VpTestAcRLoopType acRloopInput;

    /* input info for cap ringers*/
    int16 feedVADC;         /* Feed position for RAMP_INIT */
    int16 startVADC;        /* Target position for RAMP_INIT */
    int16 rampSizeVADC;     /* Size of main up/down ramps */
    int16 rampToFeedVADC;   /* Size of the ramp back to feed */

    /* results from reg ringers test */
    bool quickStop;
    bool negative;
    int16 vabDiff;
    int16 imtDiff;
    int16 vabRing;
    int16 ilgRing;
    int16 vabTip;
    int16 ilgTip;

    /* results from cap ringers test */
    VpTestResultRampType vpApiDiffRamp1;
    VpTestResultRampType vpApiDiffRamp2;
    VpTestResultRampType vpApiRingRamp1;
    VpTestResultRampType vpApiRingRamp2;
    VpTestResultRampType vpApiTipRamp1;
    VpTestResultRampType vpApiTipRamp2;
} LtVp886RingersTempType;
#endif /* LT_RINGERS_886 */

#ifdef LT_RES_FLT_886
typedef struct {
    bool highGainMode;
    LtResFltInputType input;
    LtResFltCriteriaType criteria;
    VpTestResultAltResType vpApiResFltResults;
} LtVp886ResFltTempType;
#endif /* LT_RES_FLT_886 */

#ifdef LT_MSOCKET_886
typedef struct {
    LtMSocketInputType input;
    VpTestResultAltResType vpApiMSocketType1Results;
    VpTestResultMSockType vpApiMSocketType2Results;
} LtVp886MSocketTempType;
#endif /* LT_MSOCKET_886 */

#ifdef LT_XCONNECT_886
typedef enum {
    VP886_XCONNECT_PHASE0,
    VP886_XCONNECT_PHASE1,
    VP886_XCONNECT_PHASE2,
    VP886_XCONNECT_DONE,
    VP886_XCONNECT_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886XConnectPhases;

typedef struct {
    VpTestResultXConnectType vpApiXConnectResults;
    LtVp886XConnectPhases phase;
    bool isEMFPositive;
} LtVp886XConnectTempType;
#endif /* LT_XCONNECT_886 */

#ifdef LT_CAP_886
typedef struct {
    LtCapInputType input;

    /* results from cap test primitive */
    VpTestResultAltCapType vpApiCapResults1;
    VpTestResultAltCapType vpApiCapResults2;

    /* Temporary data */
    LtCapacitanceType ctg;
    LtCapacitanceType crg;
    LtCapacitanceType ctr;
} LtVp886CapTempType;
#endif /* LT_CAP_886 */

#ifdef LT_LOOPBACK_886
typedef struct {
    VpTestLoopbackType vpApiInput;
    LtLoopbackCriteriaType criteria;
} LtVp886LoopbackTempType;
#endif /* LT_LOOPBACK_886 */

#ifdef LT_DC_FEED_ST_886
typedef struct {
    LtDcFeedSTInputType input;
    LtDcFeedSTCriteriaType criteria;
    int16 vsab;
    int16 imt;
    int16 ila;
    bool internalTestTerm;
} LtVp886DcFeedStTempType;
#endif /* LT_DC_FEED_ST_886 */

#ifdef LT_RD_LOOP_COND_886
typedef struct {
    LtRdLoopCondInputType input;
    LtRdLoopCondCriteriaType criteria;
    int32 vab;
    int32 vag;
    int32 vbg;
    int32 imt;
    int32 ilg;
    int32 vbat1;
    int32 vbat2;
    int32 vbat3;
} LtVp886RdLoopCondTempType;
#endif /* LT_RD_LOOP_COND_886 */

#ifdef LT_DC_VOLTAGE_886
typedef struct {
    LtDcVoltageInputType    input;
    LtDcVoltageCriteriaType criteria;

    /* inputs to the different test stages*/
    VpTestLoopCondType vpApiLoopCondInput;
    VpTestRampInitType vpApiRampInitInput;
    VpTestRampType vpApiRampInput;

    /* results from the different test stages */
    bool    normalPolarity;
    int16   feedV;
    int16   adcVsab1;
    int16   adcVsab2;
} LtVp886VoltageTempType;
#endif /* LT_DC_VOLTAGE_886 */

#ifdef LT_RINGING_ST_886
typedef struct {
    LtRingingSTInputType    input;
    LtRingingSTCriteriaType criteria;

    bool internalTestTerm;
    VpTestAcRLoopType vpApiAcRloopInput;
    VpTestResultAcRlType vpApiAcRloop1Rslt;
    VpTestResultAcRlType vpApiAcRloop2Rslt;
    bool ringTrip;
    bool offhook;
} LtVp886RingingStTempType;
#endif /* LT_RINGING_ST_886 */

#ifdef LT_ON_OFF_HOOK_ST_886
typedef struct {
    LtOnOffHookSTInputType input;
} LtVp886OnOffHookStTempType;
#endif /* LT_ON_OFF_HOOK_ST_886 */

#ifdef LT_RD_BAT_COND_886
typedef struct {
    int32 bat1;
    int32 bat2;
    int32 bat3;
} LtVp886RdBatCondTempType;
#endif /* LT_RD_BAT_COND_886 */

#ifdef LT_PRE_LINE_V_886
typedef struct {
    int8 polarity;
} LtVp886PreLineVTempType;
#endif /* LT_PRE_LINE_V_886 */

#ifdef LT_ALL_GR_909_886
typedef enum {
    VP886_ALL_GR_909_INIT,
    VP886_ALL_GR_909_LINE_V_INIT,
    VP886_ALL_GR_909_LINE_V,
    VP886_ALL_GR_909_RES_FLT_INIT,
    VP886_ALL_GR_909_RES_FLT,
    VP886_ALL_GR_909_OFF_HOOK_INIT,
    VP886_ALL_GR_909_OFF_HOOK,
    VP886_ALL_GR_909_RINGERS_INIT,
    VP886_ALL_GR_909_RINGERS,
    VP886_ALL_GR_909_CONCLUDE,
    VP886_ALL_GR_909_COMPLETE,
    VP886_ALL_GR_909_ABORT,
    VP886_ALL_GR_909_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp886AllGr909States;

typedef struct {
    LtVp886AllGr909States innerState;
    LtAllGr909InputType input;
    LtAllGr909CriteriaType criteria;
    LtAllGr909ResultType results;
    VpRelayControlType relayState;
    Lt886TestEventHandlerFnPtrType LineVHandlerFnPtr;
    Lt886TestEventHandlerFnPtrType RohHandlerFnPtr;
    Lt886TestEventHandlerFnPtrType RingerHandlerFnPtr;
    Lt886TestEventHandlerFnPtrType ResFltHandlerFnPtr;
} LtVp886AllGr909TestTempType;

#endif /* LT_ALL_GR_909_886 */

/*******************************************************************************
 * The LtTestTempType struct is used to save data between calls to the LT
 * event handler. Some information is retained for all test, e.g. the next
 * expected event category and event type. Some information is unique to the
 * test being executed and is overlaid in a union to save storage space.
 *
 * The following enumeration and structs make up the data that will be saved
 * between calls.
 ******************************************************************************/

typedef struct {
    struct {
        bool testNotConcluded;                  /* If = 1: test is complete */
        bool testAborted;                       /* If = 1: test is not aborted*/
        VpLineStateType initialLineState;       /* Line state before any test is run */
    } resources;

    struct {
        void *pInputs;
        void *pCriteria;
        LtVp886TestTopologyType topology;
    } attributes;

    struct {
        VpEventCategoryType eventCat;
        uint16 eventId;

        union {                             /* The "thing" union is used to */
            VpTestIdType testId;            /* save data associated with    */
        } thing;                            /* certain event types.         */
    } expected;

    VpTestTimerType testTimer;
    VpTestLoopCondType loopCond;            /* Since the get loop condition and
                                             * test timer primitives are used
                                             * again and again in tests, we have
                                             * added general loopCond and test
                                             * timer structs to the temp struct*/

    bool internalTest;                      /* Boolean indicating the current
                                             * test is calling other tests
                                             * internally */
    union {
        /* test-specific data */


        #ifdef LT_LINE_V_886
        LtVp886LineVTempType lineV;
        #endif

        #ifdef LT_ROH_886
        LtVp886RohTempType roh;
        #endif

        #ifdef LT_RINGERS_886
        LtVp886RingersTempType ringers;
        #endif

        #ifdef LT_RES_FLT_886
        LtVp886ResFltTempType resFlt;
        #endif

        #ifdef LT_MSOCKET_886
        LtVp886MSocketTempType mSocket;
        #endif

        #ifdef LT_XCONNECT_886
        LtVp886XConnectTempType xConnect;
        #endif

        #ifdef LT_CAP_886
        LtVp886CapTempType cap;
        #endif

        #ifdef LT_LOOPBACK_886
        LtVp886LoopbackTempType loopback;
        #endif

        #ifdef LT_DC_FEED_ST_886
        LtVp886DcFeedStTempType dcFeedSt;
        #endif

        #ifdef LT_RD_LOOP_COND_886
        LtVp886RdLoopCondTempType rdLoopCond;
        #endif

        #ifdef LT_DC_VOLTAGE_886
        LtVp886VoltageTempType dcVoltage;
        #endif

        #ifdef LT_RINGING_ST_886
        LtVp886RingingStTempType ringingSt;
        #endif

        #ifdef LT_ON_OFF_HOOK_ST_886
        LtVp886OnOffHookStTempType onOffHookSt;
        #endif

        #ifdef LT_RD_BAT_COND_886
        LtVp886RdBatCondTempType rdBatCond;
        #endif

        #ifdef LT_PRE_LINE_V_886
        LtVp886PreLineVTempType preLineV;
        #endif

        uint8 dummy;

    } tempData;

    #ifdef LT_ALL_GR_909_886
    /* This has been left out of the temp data because it uses the timeData */
    LtVp886AllGr909TestTempType allGr909;
    #endif


    VpTermType termType;                    /* Some tests need to know the
                                             * termination type */
    uint8 revCode;                          /* Some tests need to know the
                                             * device revision number */
    uint16 productCode;                     /* info about the features supported
                                             * by the device */
    VpDeviceType deviceType;                /* Some tests need to know which 
                                             * which device type is being tested*/
    uint8 channelId;                        /* Some tests need to know which 
                                             * which channel is being tested*/

} LtVp886TestTempType;


#endif /* LT_VP886_PACKAGE */

#endif /* _LT_API_VP886_DATA_H_ */
