/*
 * Copyright (c) 2012, Microsemi Corporation
 *
 *  Header file that defines the VP-API testline interface for 886 device type
 *
 * $Revision: 11158 $
 * $LastChangedDate: 2013-08-30 16:18:21 -0500 (Fri, 30 Aug 2013) $
 */

#ifndef VP886_TESTLINE_H
#define VP886_TESTLINE_H
#if 0
#include "vp_api.h"
#include "vp886_registers.h"

/*
 * The following defines are the step sizes of a particular device aspect :
 *
 *   WARNING: These values should never be used with with the VP886_UNIT_CONVERT()
 *            macro because they have are scaled to the internal V, I or HZ
 *            units. These only represent step sizes of a particular measurement
 *            or input attribute of the device.
 */

#define VP886_STEP_VADC_AC_UV                       105L    /* non-scaled VADC Metallic AC Voltage uV step */
#define VP886_STEP_SADC_AC_UV                       206L    /* non-scaled SADC Metallic AC Voltage uV step */
#define VP886_STEP_ADC_DC_UV                       7303L    /* non-scaled ADC Metallic DC Voltage uV step */
#define VP886_STEP_ADC_MET_NA                      1817L    /* non-scaled ADC Metallic Current nA step */
#define VP886_STEP_ADC_MET_RINGING_NA              3633L    /* non-scaled ADC Metallic Ringing Current nA step */
#define VP886_STEP_ADC_MET_LOW_GAIN_PA             9082L    /* non-scaled ADC Metallic Low Gain Current pA step */
#define VP886_STEP_ADC_LONG_NA                     1817L    /* non-scaled ADC Total Longitudinal Current nA step */
#define VP886_STEP_ADC_LONG_LOW_GAIN_PA            9082L    /* non-scaled ADC Total Longitudinal Low Gain Current pA step */
#define VP886_STEP_DAC_DC_UV                       4730L    /* non-scaled DAC Metallic DC uV step */
#define VP886_STEP_DAC_FREQ_MILI_HZ                 366L    /* non-scaled DAC Frequency mH step */
#define VP886_STEP_SWP_SWITCHER_DC_UV           5000000L    /* non-scaled SRP Switcher uV step */

/*
 * The following defines are scaled to the following 32 bit max values:
 *     Max Voltage 480000 mV
 *     Max Current 240000 uA
 *     Max Frequency 16000000 Hz
 */

#define VP886_UNIT_MV                              4474L    /* scaled mV unit */
#define VP886_UNIT_MVRMS                           6327L    /* scaled mVrms unit */
#define VP886_UNIT_UA                              8948L    /* scaled uA unit */
#define VP886_UNIT_HZ                               134L    /* scaled HZ unit */

#define VP886_UNIT_VADC_AC_VAB_RMS                  922L    /* scaled Metallic AC VADC mVrms step */
#define VP886_UNIT_SADC_AC_VAB_RMS                  922L    /* scaled Metallic AC SADC mVrms step */
#define VP886_UNIT_ADC_VAB                       -32674L    /* scaled Metallic DC ADC mV step */
#define VP886_UNIT_ADC_VAB_RMS                    46208L    /* scaled Metallic DC ADC mVrms step */
#define VP886_UNIT_ADC_IMET_NOR                   16259L    /* scaled Metallic ADC mA step */
#define VP886_UNIT_ADC_IMET_RING                  32508L    /* scaled Metallic Ringing ADC mA step */
#define VP886_UNIT_ADC_ILG                        32517L    /* scaled Total Longitudinal ADC mA step <- review!! S.H. */

#define VP886_UNIT_DAC_RING                       21162L    /* scaled DC DAC mV step */
#define VP886_UNIT_DAC_FREQ                       49044L    /* scaled Frequency DAC mHz step */

#define VP886_UNIT_TRACK_SWZ                   22370000L    /* scaled DC SRP mV step (no offset) */




#define VP886_UNIT_RAMP_TIME                   1365L    /* Ramp time (-1/3) in ms*/

#define VP886_UNIT_CONVERT(DATA, I_UNIT, O_UNIT)    (int32)(VP_ROUNDED_DIVIDE((int32)(DATA) * I_UNIT, O_UNIT))

/* converts requested slope into SigGen Freq */
#define VP886_SLOPE_TO_FREQ(AMP,SLOPE)  (int16)(((int32)(SLOPE) * (int32)VP886_UNIT_RAMP_TIME) / (int32)((AMP) == 0 ? 1 : (AMP)))

#define VP886_AC_RLOOP_MAX_BAT_LVL              75000L /* mV */
#define VP886_AC_RLOOP_MAX_TEST_LVL         ((VP886_AC_RLOOP_MAX_BAT_LVL - 5000) / 2)


typedef union Vp886TestArgsType{
    VpTestPrepareType           prepare;
    VpTestConcludeType          conclude;
    VpTestOpenVType             openV;
    VpTestDcRLoopType           dcRloop;
    VpTestAcRLoopType           acRloop;
    VpTest3EleResAltResType     resFltAlt;
    VpTestMSocketType           mSocket;
    VpTestXConnectType          xConnect;
    VpTest3EleCapAltResType     capAlt;
    VpTestLoopCondType          loopCond;
    VpTestLoopbackType          loopback;
    VpTestRampType              ramp;
    VpTestRampInitType          rampInit;
    VpTestGainTlbxType          gainTlbx;
} Vp886TestArgsType;

typedef struct _Vp886TestHeapType{

    /* next state of the running primitive */
    int16 nextState;

    /* Saved input arguments of the currently running primitive */
    Vp886TestArgsType testArgs;

    /* Used for storing line event mask data */
    VpOptionEventMaskType preTestEventMask;

    /* Stored pulse mode option */
    VpOptionPulseModeType preTestPulseMode;

    /* Stored line state and status (hook, gkey, etc.) info */
    VpApiIntLineStateType lineState;

    /* Stored termination type */
    VpTermType termType;

    /* */
    struct {
        uint8 sysState[VP886_R_STATE_LEN];
        uint8 loopSup[VP886_R_LOOPSUP_LEN];
        uint8 disn[VP886_R_DISN_LEN];
        uint8 opCond[VP886_R_OPCOND_LEN];
        uint8 opFunc[VP886_R_OPFUNC_LEN];
        uint8 swParam[VP886_R_SWPARAM_LEN];
        uint8 icr1[VP886_R_ICR1_LEN];
        uint8 icr2[VP886_R_ICR2_LEN];
        uint8 icr3[VP886_R_ICR3_LEN];
        uint8 icr4[VP886_R_ICR4_LEN];
        uint8 dcFeed[VP886_R_DCFEED_LEN];
        uint8 bfir[VP886_R_B_FIR_FILT_LEN];
        uint8 biir[VP886_R_B_IIR_FILT_LEN];
        uint8 ringGen[VP886_R_RINGGEN_LEN];
        uint8 vadc[VP886_R_VADC_LEN];
        uint8 ssCfg[VP886_R_SSCFG_LEN];
        uint8 vpGain[VP886_R_VPGAIN_LEN];
        uint8 sigGenCtrl[VP886_R_SIGCTRL_LEN];
        uint8 normCal[VP886_R_NORMCAL_LEN];
        uint8 revCal[VP886_R_REVCAL_LEN];
    } scratch;

    /* generic variables that can be used between primtive states */
    int32 prevVal_1;
    int32 prevVal_2;
    int32 prevVal_3;
    int32 prevVal_4;
    int32 prevVal_5;

   /* Used for resflt primitive */
    uint8 adcState;
    int16 previousAvg;
    uint16 settlingCount;
    int16 vabComputed;
    uint8 loopCnt;
    bool lowGain;
    bool compensate;

    uint16 speedupTime;

    /* generice varaible timestamp that can be used between primtive states */
    uint32 timeStamp;

} Vp886TestHeapType;

typedef struct _Vp886TestInfoType{
    /*
     * 886 Test Heap Pointer. The memory for this struct is acquired by
     * VP_TEST_ID_PREPARE using the VpSysTestHeapAcquire(). The memory is
     * released by VpSysTestHeapRelease() VP_TEST_ID_CONCLUDE.
     */
    Vp886TestHeapType *pTestHeap;

    /* VpSysTestHeapAcquire() returns an id and is passed to VpSysTestHeapRelease()*/
    uint8 testHeapId;

    /* handle passed into VpTestLine() when the primitive was started */
    uint16 handle;

    /*
     * Indication that VP_TEST_ID_PREPARE completed successfully.
     * If this flag is false then no other primitives are allowed to run.
     */
    bool prepared;

    /*
     * Indication that VP_TEST_ID_CONCLUDE is running.
     * If this flag is true then a conclude is in progress
     */
    bool concluding;

    /* currently running primitive */
    VpTestIdType testId;

    /* indicates a "stealth" test */
    bool nonIntrusiveTest;

} Vp886TestInfoType;
#endif
#endif /* VP886_TESTLINE_H */
