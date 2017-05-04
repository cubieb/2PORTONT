/** \lt_api_vp880.h
 *
 * This file contains device specific LT-API interface enums and structures.
 *
 * Copyright (c) 2011, Microsemi Corporation
 *
 * $Revision: 9115 $
 * $LastChangedDate: 2011-11-15 15:25:17 -0600 (Tue, 15 Nov 2011) $
 */

#ifndef _LT_API_VP880_DATA_H_
#define _LT_API_VP880_DATA_H_

#ifdef LT_VP880_PACKAGE

/* default VoicePort topology values */
#define VP880_TOP_RES_TEST_LOAD                 10000l      /* 1 kOhm */
#define VP880_TOP_LOW_PWR_RNG_LEAK              1550000l    /* 170 kOhm */
#define VP880_TOP_R_SENSE                       4020000l    /* 402 kOhm */
#define VP880_TOP_R_LEAKAGE                     (VP_INT32_MAX) /* 214.748 MOhms (open circuit) */

/* Topology for VoicePort (Vp880) device family */
typedef struct {
    LtImpedanceType rTestLoad;  /* Test Switch resistor */
    LtImpedanceType rSenseA;    /* Sense Resistor A */
    LtImpedanceType rSenseB;    /* Sense Resistor B */
    LtImpedanceType rLeakageA;  /* Leakage in Sence path A */
    LtImpedanceType rLeakageB;  /* Leakage in Sence path B */
} LtVp880TestTopologyType;


/*******************************************************************************
 * The LtTestTempType struct is used to save data between calls to the LT
 * event handler. Some information is retained for all test, e.g. the next
 * expected event category and event type. Some information is unique to the
 * test being executed and is overlaid in a union to save storage space.
 *
 * The following enumeration and structs make up the data that will be saved
 * between calls.
 ******************************************************************************/

typedef enum {
    VP880_LINE_V_TIP,
    VP880_LINE_V_RING,
    VP880_LINE_V_DIFF,
    VP880_LINE_V_NUM_INNER_STATES,
    VP880_LINE_V_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880LineVInnerStates;

typedef struct {
    LtLineVInputType input;
    LtLineVCriteriaType criteria;
    LtVp880LineVInnerStates innerState;
    VpTestResultVxcType vpApiTipResults;
    VpTestResultVxcType vpApiRingResults;
    VpTestResultVxcType vpApiDiffResults;
} LtVp880LineVTempType;

typedef enum {
    VP880_ROH_DC_RLOOP1,
    VP880_ROH_DC_RLOOP2,
    VP880_ROH_DC_RLOOP3,
    VP880_ROH_DC_RLOOP4,
    VP880_ROH_NUM_INNER_STATES,
    VP880_ROH_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880RohInnerStates;

typedef struct {
    LtRohInputType input;
    LtRohCriteriaType criteria;
    LtVp880RohInnerStates innerState;
    bool quickStop;
    int16 vab1;
    int16 vab2;
    int16 vab3;
    int16 imt1;
    int16 imt2;
    int16 imt3;
} LtVp880RohTempType;

typedef enum {
    VP880_RINGERS_DIFF,
    VP880_RINGERS_RING,
    VP880_RINGERS_TIP,
    VP880_RINGERS_NUM_INNER_STATES,
    VP880_RINGERS_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880RingerInnerStates;

typedef struct {
    LtRingerInputType input;
    LtRingersCriteriaType criteria;
    LtVp880RingerInnerStates innerState;

    /* input info for ac ringers*/
    VpTestAcRLoopType acRloopInput;

    /* input info for cap ringers*/
    int16 inputVADC;
    int16 biasVADC;

    /* results from reg ringers test */
    bool quickStop;
    bool negative;
    VpTestResultAcRlType vpApiAcR1Results1;
    VpTestResultAcRlType vpApiAcR1Results2;
    VpTestResultAcRlType vpApiAcR1Results3;

    /* results from cap ringers test */
    int16 feedVADC;
    VpTestResultRampType vpApiDiffRamp1;
    VpTestResultRampType vpApiDiffRamp2;
    VpTestResultRampType vpApiRingRamp1;
    VpTestResultRampType vpApiRingRamp2;
    VpTestResultRampType vpApiTipRamp1;
    VpTestResultRampType vpApiTipRamp2;

} LtVp880RingersTempType;

typedef struct {
    bool highGainMode;
    LtResFltInputType input;
    LtResFltCriteriaType criteria;
    VpTestResultAltResType vpApiResFltResults;
} LtVp880ResFltTempType;

typedef struct {
    LtMSocketInputType input;
    LtMSocketCriteriaType criteria;
    VpTestResultAltResType vpApiMSocketType1Results;
    VpTestResultMSockType vpApiMSocketType2Results;
} LtVp880MSocketTempType;

typedef enum {
    VP880_XCONNECT_PHASE1,
    VP880_XCONNECT_PHASE2,
    VP880_XCONNECT_PHASE3,
    VP880_XCONNECT_DONE,
    VP880_XCONNECT_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880XConnectPhases;

typedef struct {
    LtXConnectInputType input;
    LtXConnectCriteriaType criteria;
    VpTestResultXConnectType vpApiXConnectResults;

    /* Temporary data */
    LtVp880XConnectPhases phase;
    bool isEMFPositive;
} LtVp880XConnectTempType;

typedef struct {
    LtCapInputType input;
    LtCapCriteriaType criteria;

    /* results from cap test primitive */
    VpTestResultAltCapType vpApiCapResults1;
    VpTestResultAltCapType vpApiCapResults2;

    /* Temporary data */
    LtCapacitanceType ctg;
    LtCapacitanceType crg;
    LtCapacitanceType ctr;
} LtVp880CapTempType;

typedef enum {
    VP880_ALL_GR_909_INIT,
    VP880_ALL_GR_909_LINE_V,
    VP880_ALL_GR_909_RES_FLT,
    VP880_ALL_GR_909_OFF_HOOK,
    VP880_ALL_GR_909_RINGERS,
    VP880_ALL_GR_909_COMPLETE,
    VP880_ALL_GR_909_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp880AllGr909States;

typedef struct {
    LtVp880AllGr909States innerState;
    LtAllGr909InputType input;
    LtAllGr909CriteriaType criteria;
    LtAllGr909ResultType results;
    VpRelayControlType relayState;
    TestEventHandlerFnPtrType LineVHandlerFnPtr;
    TestEventHandlerFnPtrType RohHandlerFnPtr;
    TestEventHandlerFnPtrType RingerHandlerFnPtr;
    TestEventHandlerFnPtrType ResFltHandlerFnPtr;
} LtVp880AllGr909TestTempType;

typedef struct {
    LtDcFeedSTInputType input;
    LtDcFeedSTCriteriaType criteria;
    int16 vsab;
    int16 imt;
    int16 ila;
    bool internalTestTerm;
} LtVp880DcFeedSTTempType;

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
} LtVp880RdLoopCondTempType;

typedef struct {
    LtOnOffHookSTInputType input;
    LtOnOffHookSTCriteriaType criteria;
} LtVp880OnOffHookStTempType;

typedef struct {
    VpTestLoopbackType vpApiInput;
    LtLoopbackCriteriaType criteria;
} LtVp880LoopbackTempType;

typedef struct {
    LtDcVoltageInputType    input;
    LtDcVoltageCriteriaType criteria;

    /* inputs to the different test stages*/
    VpTestLoopCondType vpApiLoopCondInput;
    VpTestRampInitType vpApiRampInitInput;
    VpTestRampType vpApiRampInput1;
    VpTestRampType vpApiRampInput2;

    /* results from the different test stages */
    bool    normalPolarity;
    int16   feedV;
    int16   adcVsab1;
    int16   adcVsab2;
} LtVp880DcVoltageTempType;

typedef struct {
    VpTestResultLoopCondType   vpApiLpCnd1Rslt;
    VpTestResultLoopCondType   vpApiLpCnd2Rslt;
    VpTestResultLoopCondType   vpApiLpCnd3Rslt;
} LtVp880RdBatCondTempType;

typedef struct {
    LtRingingSTInputType    input;
    LtRingingSTCriteriaType criteria;

    bool internalTestTerm;
    VpTestAcRLoopType vpApiAcRloopInput;
    VpTestResultAcRlType vpApiAcRloop1Rslt;
    VpTestResultAcRlType vpApiAcRloop2Rslt;
    bool ringTrip;
    bool offhook;
} LtVp880RingingStTempType;

typedef struct {
    VpTestResultFltDscrmType vpApiFltDscrmRslt;
} LtVp880FltDscrmtTempType;

typedef struct {
    struct {
        bool testNotConcluded;  /* If = 1 => test is complete */
        bool relayStRestore;    /* If = 1 => test will restore relay state */
        bool testAborted;       /* If = 1 => test is not aborted*/
        VpLineStateType initialLineState; /* Line state before any test is run */
        VpRelayControlType initialRelayState; /* Relay state before any test is run */
    } resources;

    struct {
        void *pInputs;
        void *pCriteria;
        LtVp880TestTopologyType topology;
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

    LtVp880AllGr909TestTempType allGr909;       /* This has been left out of the
                                               tempData union on purpose */
    bool internalTest;                     /* Boolean indicating the current
                                            * test is calling other tests
                                            * internally                    */
    union {                                /* The "temps" union stores      */
        LtVp880LineVTempType lineV;        /* test-specific data.           */
        LtVp880RohTempType roh;
        LtVp880RingersTempType ringers;
        LtVp880ResFltTempType resFlt;
        LtVp880MSocketTempType mSocket;
        LtVp880XConnectTempType xConnect;
        LtVp880CapTempType cap;
        LtVp880LoopbackTempType loopback;
        LtVp880DcFeedSTTempType dcFeedST;
        LtVp880RdLoopCondTempType rdLoopCond;
        LtVp880OnOffHookStTempType onOffHookSt;
        LtVp880DcVoltageTempType dcVoltage;
        LtVp880RdBatCondTempType rdBatCond;
        LtVp880RingingStTempType ringingSt;
        LtVp880FltDscrmtTempType fltDscrm;
    } tempData;

    VpTermType termType;                    /* Some tests need to know the
                                             * termination type */
    uint8 revCode;                          /* Some tests need to know the
                                             * device revision number */
    uint16 productCode;                     /* info about the features supported
                                             * by the device */

} LtVp880TestTempType;
#endif /* LT_VP880_PACKAGE */

#endif /* _LT_API_VP880_DATA_H_ */
