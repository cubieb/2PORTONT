/** \lt_api_vp890.h
 *
 * This file contains device specific LT-API interface enums and structures.
 *
 * Copyright (c) 2010, Zarlink Semiconductor, Inc.
 *
 * $Revision: 5264 $
 * $LastChangedDate: 2009-07-06 10:22:54 -0500 (Mon, 06 Jul 2009) $
 */

#ifndef _LT_API_VP890_DATA_H_
#define _LT_API_VP890_DATA_H_

#ifdef LT_VP890_PACKAGE

/* default VoicePort topology values */
#define VP890_TOP_RES_TEST_LOAD                 10000l      /* 1 kOhm */
#define VP890_TOP_LOW_PWR_RNG_LEAK              1550000l    /* 170 kOhm */
#define VP890_TOP_R_SENSE                       4020000l    /* 402 kOhm */
#define VP890_TOP_R_LEAKAGE                     (VP_INT32_MAX) /* 214.748 MOhms (open circuit) */

/* Topology for VoicePort (Vp890) device family */
typedef struct {
    LtImpedanceType rTestLoad;  /* Test Switch resistor */
    LtImpedanceType rSenseA;    /* Sense Resistor A */
    LtImpedanceType rSenseB;    /* Sense Resistor B */
    LtImpedanceType rLeakageA;  /* Leakage in Sence path A */
    LtImpedanceType rLeakageB;  /* Leakage in Sence path B */
} LtVp890TestTopologyType;


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
    VP890_LINE_V_TIP,
    VP890_LINE_V_RING,
    VP890_LINE_V_DIFF,
    VP890_LINE_V_NUM_INNER_STATES,
    VP890_LINE_V_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890LineVInnerStates;

typedef struct {
    LtLineVInputType input;
    LtLineVCriteriaType criteria;
    LtVp890LineVInnerStates innerState;
    VpTestResultVxcType vpApiTipResults;
    VpTestResultVxcType vpApiRingResults;
    VpTestResultVxcType vpApiDiffResults;
} LtVp890LineVTempType;

typedef enum {
    VP890_ROH_DC_RLOOP1,
    VP890_ROH_DC_RLOOP2,
    VP890_ROH_DC_RLOOP3,
    VP890_ROH_DC_RLOOP4,
    VP890_ROH_NUM_INNER_STATES,
    VP890_ROH_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RohInnerStates;

typedef struct {
    LtRohInputType input;
    LtRohCriteriaType criteria;
    LtVp890RohInnerStates innerState;
    bool quickStop;
    int16 vab1;
    int16 vab2;
    int16 vab3;
    int16 imt1;
    int16 imt2;
    int16 imt3;
} LtVp890RohTempType;

typedef enum {
    VP890_RINGERS_DIFF,
    VP890_RINGERS_RING,
    VP890_RINGERS_TIP,
    VP890_RINGERS_NUM_INNER_STATES,
    VP890_RINGERS_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890RingerInnerStates;

typedef struct {
    LtRingerInputType input;
    LtRingersCriteriaType criteria;
    LtVp890RingerInnerStates innerState;

    /* input info for ac ringers*/
    VpTestAcRLoopType acRloopInput;

    /* input info for cap ringers*/
    int16 inputVADC;
    int16 biasVADC;

    /* results from reg ringers test */
    bool negative;
    bool quickStop;
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

} LtVp890RingersTempType;

typedef struct {
    bool highGainMode;
    LtResFltInputType input;
    LtResFltCriteriaType criteria;
    VpTestResultAltResType vpApiResFltResults;
} LtVp890ResFltTempType;

typedef struct {
    LtMSocketInputType input;
    LtMSocketCriteriaType criteria;
    VpTestResultAltResType vpApiMSocketType1Results;
    VpTestResultMSockType vpApiMSocketType2Results;
} LtVp890MSocketTempType;

typedef enum {
    VP890_XCONNECT_PHASE1,
    VP890_XCONNECT_PHASE2,
    VP890_XCONNECT_PHASE3,
    VP890_XCONNECT_DONE,
    VP890_XCONNECT_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890XConnectPhases;

typedef struct {
    LtXConnectInputType input;
    LtXConnectCriteriaType criteria;
    VpTestResultXConnectType vpApiXConnectResults;

    /* Temporary data */
    LtVp890XConnectPhases phase;
    bool isEMFPositive;
} LtVp890XConnectTempType;

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
} LtVp890CapTempType;

typedef enum {
    VP890_ALL_GR_909_INIT,
    VP890_ALL_GR_909_LINE_V,
    VP890_ALL_GR_909_RES_FLT,
    VP890_ALL_GR_909_OFF_HOOK,
    VP890_ALL_GR_909_RINGERS,
    VP890_ALL_GR_909_COMPLETE,
    VP890_ALL_GR_909_INNER_ENUM_SIZE = FORCE_STANDARD_C_ENUM_SIZE /* Portability Req.*/
} LtVp890AllGr909States;

typedef struct {
    LtVp890AllGr909States innerState;
    LtAllGr909InputType input;
    LtAllGr909CriteriaType criteria;
    LtAllGr909ResultType results;
    VpRelayControlType relayState;
    TestEventHandlerFnPtrType LineVHandlerFnPtr;
    TestEventHandlerFnPtrType RohHandlerFnPtr;
    TestEventHandlerFnPtrType RingerHandlerFnPtr;
    TestEventHandlerFnPtrType ResFltHandlerFnPtr;
} LtVp890AllGr909TestTempType;

typedef struct {
    LtDcFeedSTInputType input;
    LtDcFeedSTCriteriaType criteria;
    int16 vsab;
    int16 imt;
    int16 ila;
} LtVp890DcFeedSTTempType;

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
} LtVp890RdLoopCondTempType;

typedef struct {
    LtOnOffHookSTInputType input;
    LtOnOffHookSTCriteriaType criteria;
} LtVp890OnOffHookStTempType;

typedef struct {
    VpTestLoopbackType vpApiInput;
    LtLoopbackCriteriaType criteria;
} LtVp890LoopbackTempType;

typedef struct {
    LtDcVoltageInputType    input;
    LtDcVoltageCriteriaType criteria;

    /* inputs to the different test stages*/
    VpTestLoopCondType vpApiLoopCondInput;
    VpTestRampInitType vpApiRampInitInput;
    VpTestRampType vpApiRampInput1;
    VpTestRampType vpApiRampInput2;

    /* results from the different test stages */
    int16   feedV;
    int16   adcVsab1;
    int16   adcVsab2;
} LtVp890DcVoltageTempType;

typedef struct {
    VpTestResultLoopCondType   vpApiLpCnd1Rslt;
    VpTestResultLoopCondType   vpApiLpCnd2Rslt;
    VpTestResultLoopCondType   vpApiLpCnd3Rslt;
} LtVp890RdBatCondTempType;

typedef struct {
    LtRingingSTInputType    input;
    LtRingingSTCriteriaType criteria;

    VpTestAcRLoopType vpApiAcRloopInput;
    VpTestResultAcRlType vpApiAcRloop1Rslt;
    VpTestResultAcRlType vpApiAcRloop2Rslt;
    bool ringTrip;
    bool offhook;
} LtVp890RingingStTempType;

typedef struct {
    VpTestResultFltDscrmType vpApiFltDscrmRslt;
} LtVp890FltDscrmtTempType;

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
        LtVp890TestTopologyType topology;
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

    LtVp890AllGr909TestTempType allGr909;       /* This has been left out of the
                                               tempData union on purpose */
    bool internalTest;                     /* Boolean indicating the current
                                            * test is calling other tests
                                            * internally                    */
    union {                                /* The "temps" union stores      */
        LtVp890LineVTempType lineV;        /* test-specific data.           */
        LtVp890RohTempType roh;
        LtVp890RingersTempType ringers;
        LtVp890ResFltTempType resFlt;
        LtVp890MSocketTempType mSocket;
        LtVp890XConnectTempType xConnect;
        LtVp890CapTempType cap;
        LtVp890LoopbackTempType loopback;
        LtVp890DcFeedSTTempType dcFeedST;
        LtVp890RdLoopCondTempType rdLoopCond;
        LtVp890OnOffHookStTempType onOffHookSt;
        LtVp890DcVoltageTempType dcVoltage;
        LtVp890RdBatCondTempType rdBatCond;
        LtVp890RingingStTempType ringingSt;
        LtVp890FltDscrmtTempType fltDscrm;
    } tempData;

    VpTermType termType;                    /* Some tests need to know the
                                             * termination type */
    uint8 revCode;                          /* Some tests need to know the
                                             * device revision number */
    uint16 productCode;                     /* info about the features supported
                                             * by the device */

} LtVp890TestTempType;
#endif /* LT_VP890_PACKAGE */

#endif /* _LT_API_VP890_DATA_H_ */
