
/***********************************************************************
 * Includes
 ***********************************************************************/
#include <linux/types.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/fs.h>

#include "snd_microsemi.h"

/***********************************************************************
 * Defines 
 ***********************************************************************/
/* enable this for support of interlaced ringing. Tested with single 
 * device with two lines. */
//#define INTER_RING  

/* Enable this macro first time to calibrate  and disable this macro for all next iterations */
/* Procedure

1.	Make sure that "#define CAL" is not commented in the snd_microsemi.c. Compiled the code, and flash it.
2.	Power up the board, wait till it boots completely (You may here ding once).
3.	Write this command at console. 
a.	echo 2 > /proc/msdumpreg

You will see output as below.

echo 2 > /proc/msdumpreg
==== LE89116 ====
Dumping coeff : 8133cb20
0x8 0xc 0x0 0x30 0x0 0x0 0xff 0xfe 0xff 0x13 0x0 0x6 0xff 0x14 0x0 0x84 0xff 0x5a 0x0 0x0 0x2b 0x52 0x35 0xd4 0x44 0xa2 0x55 0x59 0xff 0xb4 0xfe 0x67 0x7 0x5 0xff 0x10 0xff 0x10 0xff 0x21 0xff 0x22 0xff 0xc 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0
==== LE89116 ====
Dumping coeff : 8133df7c
0x8 0xc 0x0 0x30 0x0 0x0 0x0 0x60 0x0 0xbd 0xff 0x1e 0xff 0x48 0x0 0x64 0xff 0xc3 0x0 0x0 0x2c 0xcc 0x37 0x83 0x46 0x97 0x57 0x7f 0x0 0xbd 0xfe 0xf5 0x6 0x5 0xff 0x8b 0xff 0x8a 0xff 0x4a 0xff 0x4a 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0

4.	Copy first coefficients to CAL_COEFF1 array
5.	Copy second coefficients to CAL_COEFF2 array
6.	Make sure that "#define CAL" is commented in the snd_microsemi.c  Recompile your code and flash it.
7.	Reboot the board, wait till it boots completely. You won't hear ding sound now. 
8.	You can try step 7 multiple times.
*/ 

/* CALIBRATE THE LINE; don't use coefficients */
#define CAL 

//forward declaration
struct MsDeviceInfoType;

typedef struct {

    bool                    isReady;            /* 1: line is ready, 0: can not process 
                                                                                        *  any request. After line is initalized, 
                                                                                        *  set to 1. */
    uint16                  chanId;             /* Line# within a slic. usually 0 or 1 
                                                                                        * (for two device with two lines) */
    VpTermType              termType;           /* Termination type: 
                                                                                        * VP_TERM_FXS_GENERIC, 
                                                                                        * VP_TERM_FXS_LOW_PWR etc. */
    VpOptionCodecType       codecType;          /* codec type, initial value is 
                                                                                         * determined from kernel configuration */
    int                     txRelGain;          /* Transmit gain value */
    uint16                  rxRelGain;          /* Received gain value */
    VpLineCtxType           lineCtx;            /* Line context, VP API Lib initalizes context */
    MSVpLineObjType         lineObj;            /* Line object, VP API Lib initalizes line object*/
    uint8                   slotTx;             /* Transmit slot */
    uint8                   slotRx;             /* Receive slot */
    uint8                   hookStatus;         /* Hook status: 0(On Hook) or 1(Off Hook)*/ 
    uint16                  expectedEvent;      /* Event which is expected from the lib. 
                                                                                        * At present only used for interlaced ringing. */

    uint8                   calCoeff[256];           
                                                                                        
    struct MsDeviceInfoType *pDevInfo;          /* Pointer to parent device */

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
    LtTestCtxType           ltTestCtx;
    LtTestResultType        ltTestResults;
    LtTestTempType          ltTestTempData;
    struct semaphore        ltTestSem;
#endif       
    
} MsLineInfoType;

typedef struct MsDeviceInfoType{
    RTKDevType                  deviceType;     /* Provides info about device type 
                                                                                        *  i.e. DEV_FXSFXS, DEV_FXS etc. */
    VpDeviceIdType              deviceId;       /* Pointer to rtl_spi_dev_t. Passed to 
                                                                                         * VP API HAL to write through SPI */
    VpDevCtxType                devCtx;         /* Device context, VP API Lib initalizes context */
    MSVpDeviceObjType           devObj;         /* Device object, VP API Lib initalizes object*/
    uint16                      numLines;       /* Total number of lines: max 2, min 1 */
    VpDeviceType                VpDevType;      /* VP Device type: e.g. VP_DEV_886_SERIES, etc....*/
                /* profiles */
    VpProfilePtrType            pDevProfile;    /* Pointer to Device profile */
    VpProfilePtrType            pAcProfile;     /* Pointer to AC profile */
    VpProfilePtrType            pDcProfile;     /* Pointer to DC profile */
    VpProfilePtrType            pRingProfile;   /* Pointer to Ring profile */
    VpProfilePtrType            pFxoAcProfile;  /* Pointer to FXO AC Profile */
    VpProfilePtrType            pFxoDialProfile;/* Pointer to FXO config Profile*/
    VpProfilePtrType            pRingCadProfile;/* Pointer to Ring cadence Profile */
    uint16                      cadOnMs;        /* Cadence On time in Milliseconds */
    uint16                      cadOffMs;       /* Cadence Off time in Milliseconds */
    MS_SLIC_TYPE                msSlicType;     /* Slic type : MS_SLIC_TYPE_LE9662, etc. 
                                                                                        * SLIC type is obtained by reading config file at 
                                                                                        * initialization time. */
    uint32                      slicId;         /* Unique ID of this SLIC, generated 
                                                                                         * at initialization time. */
    voip_snd_t                  *pVoipSnd;      /* Pointer to snd device (speicific to Relatek). */
    MsLineInfoType              lineInfoType[VP_MAX_LINES_PER_DEVICE]; /* Line information related to this device */
}MSDeviceInfoType;


#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)

#define THRESHOLD 150
#define LINE_TEST_TIMEOUT (3000)
typedef union {
    LtLineVInputType lineVInp;
    LtRingerInputType ringersInp;
    LtRohInputType rohInp;
    LtResFltInputType resFltInp;
    /* ... Add other input structs here */
} TestInputType;


typedef struct {    
    const char* text;    
    unsigned int val;
} LutType;

#endif


/***********************************************************************
 * Globals 
 ***********************************************************************/

static uint16 GAINTABLE[]= {
    0xFEC9, 0xE314, 0xCA62, 0xB460, 0xA0C2, 0x8F47,         /* 12dB to   7dB */
        //0x7FB2, 0x71CF, 0x656E, 0x5A67, 0x5092, 0x47CF,        /*  6dB to   0dB */
    0x788D, 0x71CF, 0x656E, 0x5A67, 0x5092, 0x47CF,         /*  5.5dB to   0dB */
    0x4000,                                                 /*  0dB          */
    0x390A, 0x32D6, 0x2D4E, 0x2861, 0x23FD, 0x2013,         /* -1dB to - 6dB */
    0x1C96, 0x197A, 0x16B5, 0x143D, 0x1209, 0x1013          /* -7dB to -12dB */
};

/* Global which tells total number of devices connected. The value is calculated at kernel initialization
  * time. */
static uint16                          gNoOfDevices = 0;           

/* Work queue to receive events from Microsemi VP-API*/
static struct delayed_work             *pollWork;
static struct workqueue_struct         *msEventsWq;
static voip_snd_t                      msSnd[MICRO_SEMI_MAX_NUM_DEV];
static rtl_spi_dev_t                   rtlSpi[MICRO_SEMI_MAX_NUM_DEV];
//static 
static struct MsDeviceInfoType         msDevInfo[MICRO_SEMI_MAX_NUM_DEV];

#ifndef CAL
static uint8 CAL_COEFF1[52] = { 0x8, 0xc, 0x0, 0x30, 0x0, 0x0, 0xff, 0xfe, 0xff, 0x13, 0x0, 0x6, 0xff, 0x14, 0x0, 0x84, 0xff, 0x5a, 0x0, 0x0, 0x2b, 0x52, 0x35, 0xd4, 0x44, 0xa2, 0x55, 0x59, 0xff, 0xb4, 0xfe, 0x67, 0x7, 0x5, 0xff, 0x10, 0xff, 0x10, 0xff, 0x21, 0xff, 0x22, 0xff, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
static uint8 CAL_COEFF2[52] = { 0x8, 0xc, 0x0, 0x30, 0x0, 0x0, 0x0, 0x60, 0x0, 0xbd, 0xff, 0x1e, 0xff, 0x48, 0x0, 0x64, 0xff, 0xc3, 0x0, 0x0, 0x2c, 0xcc, 0x37, 0x83, 0x46, 0x97, 0x57, 0x7f, 0x0, 0xbd, 0xfe, 0xf5, 0x6, 0x5, 0xff, 0x8b, 0xff, 0x8a, 0xff, 0x4a, 0xff, 0x4a, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
#endif

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
VpLineCtxType *pLineCtxUnderTest = NULL;

static LutType LtRingerLut[] = {
    {"LT_RINGER_REGULAR_PHNE_TEST"          ,LT_RINGER_REGULAR_PHNE_TEST},
    {"LT_RINGER_ELECTRONIC_PHNE_TEST"       ,LT_RINGER_ELECTRONIC_PHNE_TEST},
    {"LT_RINGER_REGULAR_PHNE_TEST_3_ELE"    ,LT_RINGER_REGULAR_PHNE_TEST_3_ELE},
    {"LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE" ,LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE},
    {NULL                                   ,(int) NULL}
};


static LutType LtImpedanceLut[] = {
    {"LT_IMPEDANCE_NOT_MEASURED"    ,LT_IMPEDANCE_NOT_MEASURED},
    {"LT_IMPEDANCE_SHORT_CKT"       ,LT_IMPEDANCE_SHORT_CKT},
    {"LT_IMPEDANCE_OPEN_CKT"        ,LT_IMPEDANCE_OPEN_CKT},
    {NULL                           ,(int) NULL}
};

static LutType LtMeasureStatusLut[] = {
    {"LT_MSRMNT_STATUS_PASSED"             ,LT_MSRMNT_STATUS_PASSED},
    {"LT_MSRMNT_STATUS_EXCESSIVE_ILG"      ,LT_MSRMNT_STATUS_EXCESSIVE_ILG},
    {"LT_MSRMNT_STATUS_DEGRADED_ACCURACY"  ,LT_MSRMNT_STATUS_DEGRADED_ACCURACY},
    {NULL                                  ,(int) NULL}
};

static LutType LtLineVMaskLut[] = {
    {"LT_LVM_HEMF_DC_TIP"           ,LT_LVM_HEMF_DC_TIP},
    {"LT_LVM_HEMF_DC_RING"          ,LT_LVM_HEMF_DC_RING},
    {"LT_LVM_HEMF_DC_DIFF"          ,LT_LVM_HEMF_DC_DIFF},
    {"LT_LVM_HEMF_AC_TIP"           ,LT_LVM_HEMF_AC_TIP},
    {"LT_LVM_HEMF_AC_RING"          ,LT_LVM_HEMF_AC_RING},
    {"LT_LVM_HEMF_AC_DIFF"          ,LT_LVM_HEMF_AC_DIFF},
    {"LT_LVM_FEMF_DC_TIP"           ,LT_LVM_FEMF_DC_TIP},
    {"LT_LVM_FEMF_DC_RING"          ,LT_LVM_FEMF_DC_RING},
    {"LT_LVM_FEMF_DC_DIFF"          ,LT_LVM_FEMF_DC_DIFF},
    {"LT_LVM_FEMF_AC_TIP"           ,LT_LVM_FEMF_AC_TIP},
    {"LT_LVM_FEMF_AC_RING"          ,LT_LVM_FEMF_AC_RING},
    {"LT_LVM_FEMF_AC_DIFF"          ,LT_LVM_FEMF_AC_DIFF},
    {"LT_TEST_MEASUREMENT_ERROR"    ,LT_TEST_MEASUREMENT_ERROR},
    {NULL                           ,(int) NULL}

};

static LutType LtRohMaskLut[] = {
    {"LT_ROHM_OFF_HOOK"             ,LT_ROHM_OFF_HOOK},
    {"LT_ROHM_RES_LOOP"             ,LT_ROHM_RES_LOOP},
    {"LT_ROHM_OUT_OF_RANGE_LOOP"    ,LT_ROHM_OUT_OF_RANGE_LOOP},
    {"LT_TEST_MEASUREMENT_ERROR"    ,LT_TEST_MEASUREMENT_ERROR},
    {NULL                           ,(int) NULL}
};

static LutType LtRenMaskLut[] = {
    {"LT_RNGM_REN_LOW"              ,LT_RNGM_REN_LOW},
    {"LT_RNGM_REN_HIGH"             ,LT_RNGM_REN_HIGH},
    {"LT_TEST_MEASUREMENT_ERROR"    ,LT_TEST_MEASUREMENT_ERROR},
    {NULL                           ,(int) NULL}
};

static LutType LtResFltMaskLut[] = {
    {"LT_RESFM_TIP"                 ,LT_RESFM_TIP},
    {"LT_RESFM_RING"                ,LT_RESFM_RING},
    {"LT_RESFM_DIFF"                ,LT_RESFM_DIFF},
    {"LT_TEST_MEASUREMENT_ERROR"    ,LT_TEST_MEASUREMENT_ERROR},
    {NULL                           ,(int) NULL}
};

static LutType LtCapMaskLut[] = {
    {"LT_CAP_TEST_PASSED"           ,LT_CAP_TEST_PASSED},
    {"LT_CAP_TEST_FAILED"           ,LT_CAP_TEST_FAILED},
    {"LT_TEST_MEASUREMENT_ERROR"    ,LT_TEST_MEASUREMENT_ERROR},
    {NULL                           ,(int) NULL}
};


#endif


/***********************************************************************
 * Function Prototypes 
 ***********************************************************************/

static int Snd_Ms_Slic_Init(
    int pcmMode,
    int initOnly);

static unsigned int Snd_Ms_Get_Number_Of_Slics(
    MS_SLIC_TYPE msSlicType);

static void Snd_Ms_Get_Slic_Info(
    MS_SLIC_TYPE msSlicType,
    MsSlicInfo   *pMsSlicInfo);

BOOL Snd_Ms_Init_Device(
    struct MsDeviceInfoType *pMsDevInfoType);

static void Snd_Ms_Setup_Work(
    void);

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
static void Snd_Ms_LineTest_Update_Results(
        LtTestIdType testId,    
        LtTestResultType *pResults,    
        unsigned int *result);

static void Snd_Ms_Process_Line_Test_Event(
        struct MsDeviceInfoType *pMsDevInfoType,
        const VpEventType *pEvent);
#endif


/***********************************************************************
 * Function 
 ***********************************************************************/

/***********************************************************************
* Conv2MicrosemiGain()-  This function converts input gain value to Microsemi gain value.
*
* inputs:
*  adj - input gain
*
* outputs:
*  return converted gain
***********************************************************************/
static int Conv2MicrosemiGain(
    int adj)
{
    int nGainVal;

    nGainVal = adj;

    if (adj > 12) {
        nGainVal = 12;
    }
    
    if (adj < -12) {
        nGainVal = -12;
    }

    return (GAINTABLE[12 - nGainVal]);
}

/***********************************************************************
* Enable_Microsemi()-  This function will switch between wideband and narrow band as well  
*   as modify the AC profile for a particular line
*
* inputs:
*  enable = 1 (enable), 0 (disable), 2(enable with wideband)
*
* outputs:
*  return -1 (line not ready)
*  return 0 (enabled)
***********************************************************************/
int Enable_Microsemi(
    voip_snd_t      *this,
    int             enable) 
{
    VpStatusType                status;
    VpOptionPcmTxRxCntrlType    pcmCtrl;
    MsLineInfoType              *pLineInfo;
    VpLineCtxType               *pLineCtx;
    VpOptionCodecType           codec;

    pLineInfo   = (MsLineInfoType *)this->priv;
    pLineCtx    = &pLineInfo->lineCtx;
    codec       = pLineInfo->codecType;
    
    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return -1;
    }
    
    if ((enable == 1) && (codec == VP_OPTION_ALAW_WIDEBAND)) {
        codec = VP_OPTION_ALAW;
    } else if ((enable == 1) && (codec == VP_OPTION_MLAW_WIDEBAND)) {
        codec = VP_OPTION_MLAW;
    } else if ((enable == 1) && (codec == VP_OPTION_LINEAR_WIDEBAND)) {
        codec = VP_OPTION_LINEAR;
    } else if ((enable == 2) && (codec == VP_OPTION_ALAW)) {
        codec = VP_OPTION_ALAW_WIDEBAND;
    } else if ((enable == 2) && (codec == VP_OPTION_MLAW)) {
        codec = VP_OPTION_MLAW_WIDEBAND;
    } else if ((enable == 2) && (codec == VP_OPTION_LINEAR)) {
        codec = VP_OPTION_LINEAR_WIDEBAND;
    } else if (enable >= 3) {
        PRINT_R("%s() pcm_mode %d error\n", __FUNCTION__, enable);
        return -1;
    }

    /* if the new mode and old mode do not match, make the change */
    if (codec != pLineInfo->codecType) {
        /* change the codec mode */
        status = VpSetOption(pLineCtx, VP_NULL, VP_OPTION_ID_CODEC, &codec);
        ASSERT_microsemi(status,"VpSetOption(VP_OPTION_ID_CODEC) ");
    }

    if (enable == 0) {
        pcmCtrl = VP_OPTION_PCM_OFF;
    } else {
        pcmCtrl = VP_OPTION_PCM_BOTH;
    }

    status = VpSetOption(pLineCtx, VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &pcmCtrl);
    PRINT_MSG("Setting codec %d pcm control %d status %d \n", codec, pcmCtrl, status);
    ASSERT_microsemi(status,"VpSetOption(VP_OPTION_ID_PCM_TXRX_CNTRL) ");

    return 0;
}

/***********************************************************************
 * FXS_Ring_Microsemi()-  Enables/Disables Ringing
 *
 * inputs:
 *  ringset = 0 (disable)
 *  ringset = 1 (enable)
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void FXS_Ring_Microsemi(
    voip_snd_t      *this,
    unsigned char   ringset)
{
    VpStatusType            status;
    bool                    offHook = FALSE;
    MsLineInfoType *const   pLineInfo = (MsLineInfoType *)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

#ifdef INTER_RING    
    MsLineInfoType          *pOtherLineInfo = NULL;
    VpLineStateType         lineState;
#endif

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    PRINT_MSG("ctx %p line %d slotRx %d \n", &pLineInfo->lineCtx, pLineInfo->chanId, pLineInfo->slotRx);

    if (ringset) {
        
#ifdef INTER_RING
        //get pointer to another line
        
        pOtherLineInfo = Snd_Ms_Get_Other_Line(pLineInfo);

        //if another line is ringing
        status = VpGetLineState(&pOtherLineInfo->lineCtx, &lineState);

        //if other line is in ringing state then this line needs to wait for CAD_BREAK event
        //so returning without setting this line to ringing state.
        if (lineState == VP_LINE_RINGING || lineState == VP_LINE_RINGING_POLREV) {
            pOtherLineInfo->expectedEvent = VP_RING_CAD_BREAK;
            //wait for CAD_BREAK event on another line before setting line to RINGING

            PRINT_MSG("MARK(%d): VP_RING_CAD_BREAK\n", pOtherLineInfo->chanId);
            return;
        }
#endif

        status = VpSetLineState(pLineCtx, VP_LINE_RINGING);
        ASSERT_microsemi(status, "VpSetLineState(RINGING) ");
        PRINT_MSG("VP_LINE_RINGING\n");
    } else {
        status = VpGetLineStatus(pLineCtx, VP_INPUT_HOOK, &offHook);
        ASSERT_microsemi(status, "VpGetLineStatus(INPUT_HOOK) ");

        if (offHook == TRUE) {
            status = VpSetLineState(pLineCtx, VP_LINE_OHT);
            ASSERT_microsemi(status, "VpSetLineState(OHT) ");
            PRINT_MSG("VP_LINE_OHT\n");
        } else {
            status = VpSetLineState(pLineCtx, VP_LINE_STANDBY);
            ASSERT_microsemi(status, "VpSetLineState(STANDBY) ");
            PRINT_MSG("VP_LINE_STANDBY\n");
        }
    }

    return;
}

/***********************************************************************
 * FXS_Check_Ring_Microsemi()
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  return 0 (not ringing)
 *  return 1 (ringing)
 ***********************************************************************/
static unsigned char FXS_Check_Ring_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    unsigned char           ringer = 0; //0: ring off, 1: ring on
    VpLineStateType         lineState;
    MsLineInfoType *const   pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    if (!pLineInfo->isReady) {
        return 0;
    }

    status = VpGetLineState(pLineCtx, &lineState);
    ASSERT_microsemi(status, "VpGetLineState() ");

    if (lineState == VP_LINE_RINGING || lineState == VP_LINE_RINGING_POLREV) {
        ringer = 1;
    }
    
    return ringer;
}

/***********************************************************************
 * FXS_Line_Check_Microsemi()
 *
 * inputs:
 *  n/a
 *
 * outputs:
 * return 0 (Phone dis-connect)
 * return 1 (Phone connect)
 * return 2 (Phone off-hook)
 ***********************************************************************/
static unsigned int FXS_Line_Check_Microsemi(
    voip_snd_t *this )
{
    VpStatusType            status;
    unsigned int            ret = 0;    /* 0: phone disconnect, 1: phone connect, 
                                                                          * 2: phone off-hook */
    VpLineStateType         lineState;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return 0;
    }

    status = VpGetLineState(pLineCtx, &lineState);
    ASSERT_microsemi(status, "VpGetLineState() ");

    if (lineState == VP_LINE_DISCONNECT) {
        ret = 0;
    } else if((lineState == VP_LINE_STANDBY) || 
            (lineState == VP_LINE_STANDBY_POLREV) ||
            (lineState == VP_LINE_ACTIVE) ||
            (lineState == VP_LINE_ACTIVE_POLREV)) {
        ret = 1;
    } else if((lineState == VP_LINE_OHT) || (lineState == VP_LINE_OHT_POLREV)) {
        ret = 2;
    }

    return ret;
}

/***********************************************************************
 * SLIC_Set_PCM_state_Microsemi()
 *
 * inputs:
 *  enable 0 : Turn off both PCM, 1: Turn on both PCM
 *
 * outputs:
 * none
 ***********************************************************************/
static void SLIC_Set_PCM_state_Microsemi(
    voip_snd_t *this,
    int         enable)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;
    VpOptionPcmTxRxCntrlType pcmCtrl;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    if (enable == 1) {
        pcmCtrl = VP_OPTION_PCM_BOTH;
    } else if (enable == 0) {
        pcmCtrl = VP_OPTION_PCM_OFF;
    } else {
        PRINT_R("%s :Error config %d!\n",__FUNCTION__, enable);
        return;
    }

    status = VpSetOption(pLineCtx, VP_NULL, VP_OPTION_ID_PCM_TXRX_CNTRL, &pcmCtrl);
    ASSERT_microsemi(status, "VpSetOption(PCM_TXRX_CNTRL) ");

    return;
}

/***********************************************************************
 * SLIC_Get_Hook_Status_Microsemi()
 *
 * inputs:
 *  directly is supposed to be a polling timer but the original code never used it
 *
 * outputs:
 * return 0 (on-hook)
 * return 1 (off-hook)
 ***********************************************************************/
static unsigned char SLIC_Get_Hook_Status_Microsemi(
    voip_snd_t *this,
    int         directly)
{
    VpStatusType            status;
    bool                    offHook = FALSE;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    if (!pLineInfo->isReady) {
        return 0;
    }

    status = VpGetLineStatus(pLineCtx, VP_INPUT_HOOK, &offHook);
    ASSERT_microsemi(status, "VpGetLineStatus(INPUT_HOOK) ");

    if (offHook == TRUE) {
        return 1;
    } else {
        return 0;
    }
}

/***********************************************************************
 * SLIC_Set_Power_Save_Mode_Microsemi()
 *
 * inputs:
 *  none
 *
 * outputs:
 *  none
 ***********************************************************************/
static void SLIC_Set_Power_Save_Mode_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();
    
    if (!pLineInfo->isReady) {
        return;
    }

    status = VpSetLineState(pLineCtx, VP_LINE_STANDBY);
    ASSERT_microsemi(status, "VpSetLineState(STANDBY) ");

    return;
}

/***********************************************************************
 * SLIC_Set_FXS_Line_State_Microsemi()
 *
 * inputs:
 * state:
    0: line in-active state
    1: line active state
    2: power save state
    3: OHT
    4: OHT polrev
    5: Ring
 *
 * outputs:
 *  none
 ***********************************************************************/
static void SLIC_Set_FXS_Line_State_Microsemi(
    voip_snd_t *this,
    int         state)
{
    VpStatusType            status;
    VpLineStateType         lineState;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    switch(state) {
        case 0: {
            lineState = VP_LINE_TIP_OPEN;
            break;
        }
        case 1: {
            lineState = VP_LINE_ACTIVE;
            break;
        }
        case 2: {
            lineState = VP_LINE_STANDBY;
            break;
        }
        case 3: {
            lineState = VP_LINE_OHT;
            break;
        }
        case 4: {
            lineState = VP_LINE_OHT_POLREV;
            break;
        }
        case 5: {
            lineState = VP_LINE_RINGING;
            break;
        }
        default:
            PRINT_R("Warnning! Error case (%d), set to OHT state. in %s, line%d\n", 
                    state, __FUNCTION__, __LINE__);
            lineState = VP_LINE_OHT;
            break;
    }

    status = VpSetLineState(pLineCtx, lineState);
    ASSERT_microsemi(status, "VpSetLineState() ");
    PRINT_MSG("VpSetLineState(%d) \n", lineState); 
    return;

}

/***********************************************************************
 * Adjust_SLIC_Tx_Gain_Microsemi()
 *
 * inputs:
 *  txGain => -12 to +12 dB
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void Adjust_SLIC_Tx_Gain_Microsemi(
    voip_snd_t *this,
    int         txGain)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    status = VpSetRelGain(pLineCtx, 
                            Conv2MicrosemiGain(txGain), 
                            Conv2MicrosemiGain(pLineInfo->rxRelGain), 
                            pLineInfo->chanId);
    ASSERT_microsemi(status, "VpSetRelGain() ");

    if (status == VP_STATUS_SUCCESS) {
        pLineInfo->txRelGain = txGain;
    }

    return;

}

/***********************************************************************
 * Adjust_SLIC_Rx_Gain_Microsemi()
 *
 * inputs:
 *  rxGain => -12 to +12 dB
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void Adjust_SLIC_Rx_Gain_Microsemi(
    voip_snd_t *this,
    int         rxGain)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    status = VpSetRelGain(pLineCtx, 
                            Conv2MicrosemiGain(pLineInfo->txRelGain), 
                            Conv2MicrosemiGain(rxGain), 
                            pLineInfo->chanId);
    
    ASSERT_microsemi(status, "VpSetRelGain() ");

    if (status == VP_STATUS_SUCCESS) {
        pLineInfo->txRelGain = rxGain;
    }

    return;
}

/***********************************************************************
 * SLIC_Set_Ring_Cadence_Microsemi()
 *
 * inputs:
 *  onMsec => ring ontime (ms)
 *  offMsec => ring offtime (ms)
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Set_Ring_Cadence_Microsemi(
    voip_snd_t      *this,
    unsigned short  onMsec,
    unsigned short  offMsec)
{
    VpStatusType            status;
    uint8                   *pRingCad;
    uint8                   tData[4];
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    pRingCad = (uint8 *)pLineInfo->pDevInfo->pRingCadProfile;

    /* write cadence timer coeff. */
    tData[0] = 0x20 + ((onMsec/5)>>8);
    tData[1] = (onMsec/5);
    tData[2] = 0x20 + ((offMsec/5)>>8);
    tData[3] = (offMsec/5);

    pRingCad[RING_PROFILE_CAD_ON_H_IDX]  = tData[0];
    pRingCad[RING_PROFILE_CAD_ON_L_IDX]  = tData[1];
    pRingCad[RING_PROFILE_CAD_OFF_H_IDX] = tData[2];
    pRingCad[RING_PROFILE_CAD_OFF_L_IDX] = tData[3];

    status = VpInitRing(pLineCtx, pRingCad, VP_PTABLE_NULL);
    ASSERT_microsemi(status, "VpInitRing() ");

    if (status == VP_STATUS_SUCCESS) {
        pLineInfo->pDevInfo->pRingCadProfile = pRingCad;
        pLineInfo->pDevInfo->cadOnMs   = onMsec;
        pLineInfo->pDevInfo->cadOffMs  = offMsec;
    }

    return;
}

/******************************************************************************
 * SLIC_Set_Multi_Ring_Cadence_Microsemi() - Place Holder for
 *  multi cadence ringing
 *
 * inputs:
 *  onMsecX => ring ontime (ms)
 *  offnMsecX => ring offtime (ms)
 *
 * outputs:
 *  n/a
 *****************************************************************************/
static void SLIC_Set_Multi_Ring_Cadence_Microsemi(
    voip_snd_t      *this,
    unsigned short  onMsec1,
    unsigned short  offMsec1,
    unsigned short  onMsec2,
    unsigned short  offMsec2,
    unsigned short  onMsec3,
    unsigned short  offMsec3,
    unsigned short  onMsec4,
    unsigned short  offMsec4)
{
    unsigned short          maxOn, maxOff;
    uint32                  cch;
    MsLineInfoType *const   pLineInfo = (MsLineInfoType*)this->priv;
    uint32 const            chid = (this->con_ptr)->cch;
    
    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }
    
    maxOn = maxOff = 60000;
    SLIC_Set_Ring_Cadence_Microsemi(this, maxOn, maxOff);

    MultiRingCadenceEnable(chid, 1, onMsec1, offMsec1, onMsec2, offMsec2, onMsec3, offMsec3, onMsec4, offMsec4);

    PRINT_MSG("Enable multi ring cadence\n");
    
    for (cch = 0; cch < con_ch_num; cch++) {   
        
        //used to check if any channel has not been set by multi ring
        const voip_con_t* con_ptr = get_const_con_ptr(cch);
        
        if (MultiRingCadenceEnableCheck_con(con_ptr) == 0) {
            
            unsigned short resetOnMsec = 2000;
            unsigned short resetOffMsec = 4000;   
            
            MsLineInfoType * const pResetLine = 
                (MsLineInfoType * )(con_ptr->snd_ptr->priv);           

            if (pResetLine->pDevInfo->cadOnMs > 0) {
                resetOnMsec = pResetLine->pDevInfo->cadOnMs;
            }
            
            if (pResetLine->pDevInfo->cadOffMs > 0) {
                resetOffMsec = pResetLine->pDevInfo->cadOffMs;
            }

            MultiRingCadenceEnable(cch, 1, resetOnMsec, resetOffMsec, 0, 0, 0, 0, 0, 0);
        }
    }
    return;
}

/***********************************************************************
 * SLIC_Set_Ring_Freq_Amp_Microsemi() - Adjust ringing frequency
 *
 * inputs:
 *  preset => 
 *  
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Set_Ring_Freq_Amp_Microsemi(
    voip_snd_t *this,
    char        preset)
{
    PRINT_R("%s is not implemented\n", __FUNCTION__);
    return;
}

/***********************************************************************
 * GetCountryAcProfile() - Get AC profile for a country and for particular series
 *
 * inputs:
 *  pLineInfo => Line on which AC profile needs to be set
 *  country => country for which AC profile needs to be set
 *
 * outputs:
 *  return AC profile
 ***********************************************************************/
VpProfilePtrType Snd_Ms_Get_Country_AC_Profile(
    MsLineInfoType * const      pLineInfo,
    unsigned short              country)
{
    VpDeviceType deviceType = pLineInfo->pDevInfo->VpDevType;

    switch (deviceType) {
        case VP_DEV_880_SERIES:

            PRINT_MSG("line#%d Using NARROWBAND AC profile\n", pLineInfo->chanId);

            switch (country) {
                case COUNTRY_AUSTRALIA: 
                    return LE880_AC_FXS_RF14_AU;
                case COUNTRY_BE:    /* Belgium*/
                    return LE880_AC_FXS_RF14_BE;
                case COUNTRY_CN:    /* China  */
                    return LE880_AC_FXS_RF14_CN;
                case COUNTRY_GR:    /* German */
                    return LE880_AC_FXS_RF14_DE;
                case COUNTRY_FL:    /* Finland*/
                    return LE880_AC_FXS_RF14_FI;
                case COUNTRY_FR:    /* France */
                    return LE880_AC_FXS_RF14_FR;
                case COUNTRY_IT:    /* Italy  */
                    return LE880_AC_FXS_RF14_IT;
                case COUNTRY_JP:    /* Japan  */
                    return LE880_AC_FXS_RF14_JP;
                case COUNTRY_SE:    /* Sweden */
                    return LE880_AC_FXS_RF14_SE;
                case COUNTRY_HK:
                case COUNTRY_TW:    
                case COUNTRY_UK:
                case COUNTRY_USA:
                    PRINT_MSG("Set SLIC impedance to 600 ohm.\n");
                    return DEF_LE880_AC_PROFILE;

                default:
                    printk("country wasn't defined. \
                            Set to default SLIC impedance 600 ohm.\n");
                    return DEF_LE880_AC_PROFILE;
            }

        case VP_DEV_890_SERIES:
            
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_WIDEBAND
            
            PRINT_MSG("line#%d Using WIDEBAND AC profile\n", pLineInfo->chanId);

            switch (country) {
                case COUNTRY_AUSTRALIA:
                    return LE890_BB_AC_FXS_RF50_WB_AU;
                case COUNTRY_BE:    /* Belgium*/
                    return LE890_BB_AC_FXS_RF50_WB_BE;
                case COUNTRY_CN:    /* China  */
                    return LE890_BB_AC_FXS_RF50_WB_CN;
                case COUNTRY_GR:    /* German */
                    return LE890_BB_AC_FXS_RF50_WB_DE;
                case COUNTRY_FL:    /* Finland*/
                    return LE890_BB_AC_FXS_RF50_WB_FI;
                case COUNTRY_FR:    /* France */
                    return LE890_BB_AC_FXS_RF50_WB_FR;
                case COUNTRY_IT:    /* Italy  */
                    return LE890_BB_AC_FXS_RF50_WB_IT;
                case COUNTRY_JP:    /* Japan  */
                    return LE890_BB_AC_FXS_RF50_WB_JP;
                case COUNTRY_SE:    /* Sweden */
                    return LE890_BB_AC_FXS_RF50_WB_SE;
                case COUNTRY_HK:
                case COUNTRY_TW:
                case COUNTRY_UK:
                case COUNTRY_USA:
                    PRINT_MSG("Set SLIC impedance to 600 ohm.\n");
                    return LE890_BB_AC_FXS_RF50_WB_600R_DEF;

                default:
                    printk("country wasn't defined. \
                            Set to default SLIC impedance 600 ohm.\n");
                    return LE890_BB_AC_FXS_RF50_WB_600R_DEF;
            }
#else

            PRINT_MSG("line#%d Using NARROWBAND AC profile\n", pLineInfo->chanId);

            switch (country) {
                case COUNTRY_AUSTRALIA:
                    return LE890_BB_AC_FXS_RF50_AU;
                case COUNTRY_BE:    /* Belgium*/
                    return LE890_BB_AC_FXS_RF50_BE;
                case COUNTRY_CN:    /* China  */
                    return LE890_BB_AC_FXS_RF50_CN;
                case COUNTRY_GR:    /* German */
                    return LE890_BB_AC_FXS_RF50_DE;
                case COUNTRY_FL:    /* Finland*/
                    return LE890_BB_AC_FXS_RF50_FI;
                case COUNTRY_FR:    /* France */
                    return LE890_BB_AC_FXS_RF50_FR;
                case COUNTRY_IT:    /* Italy  */
                    return LE890_BB_AC_FXS_RF50_IT;
                case COUNTRY_JP:    /* Japan  */
                    return LE890_BB_AC_FXS_RF50_JP;
                case COUNTRY_SE:    /* Sweden */
                    return LE890_BB_AC_FXS_RF50_SE;
                case COUNTRY_HK:
                case COUNTRY_TW:
                case COUNTRY_UK:
                case COUNTRY_USA:
                    PRINT_MSG("Set SLIC impedance to 600 ohm.\n");
                    return DEF_LE890_AC_PROFILE;

                default:
                    printk("country wasn't defined. \
                            Set to default SLIC impedance 600 ohm.\n");
                    return DEF_LE890_AC_PROFILE;
            }
#endif  /* CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_WIDEBAND */
        break;

        case VP_DEV_886_SERIES:
        case VP_DEV_887_SERIES:
            
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_WIDEBAND
            
            PRINT_MSG("line#%d Using WIDEBAND AC profile\n", pLineInfo->chanId);

            switch (country) {
                case COUNTRY_AUSTRALIA: 
                    return ZLR88621L_AC_FXS_RF14_WB_AU;
                case COUNTRY_BE:    /* Belgium*/
                    return ZLR88621L_AC_FXS_RF14_WB_BE;
                case COUNTRY_CN:    /* China  */
                    return ZLR88621L_AC_FXS_RF14_WB_CN;
                case COUNTRY_GR:    /* German */
                    return ZLR88621L_AC_FXS_RF14_WB_DE;
                case COUNTRY_FL:    /* Finland*/
                    return ZLR88621L_AC_FXS_RF14_WB_FI;
                case COUNTRY_FR:    /* France */
                    return ZLR88621L_AC_FXS_RF14_WB_FR;
                case COUNTRY_IT:    /* Italy  */
                    return ZLR88621L_AC_FXS_RF14_WB_IT;
                case COUNTRY_JP:    /* Japan  */
                    return ZLR88621L_AC_FXS_RF14_WB_JP;
                case COUNTRY_SE:    /* Sweden */
                    return ZLR88621L_AC_FXS_RF14_WB_SE;
                case COUNTRY_HK:
                case COUNTRY_TW:    
                case COUNTRY_UK:
                case COUNTRY_USA:
                    PRINT_MSG("Set SLIC impedance to 600 ohm.\n");
                    return ZLR88621L_AC_FXS_RF14_WB_600R_DEF;

                default:
                    PRINT_R("country wasn't defined. \
                                Set to default SLIC impedance 600 ohm.\n");
                    return ZLR88621L_AC_FXS_RF14_WB_600R_DEF;
            }
#else

            PRINT_MSG("line#%d Using NARROWBAND AC profile\n", pLineInfo->chanId);

            switch (country) {
                case COUNTRY_AUSTRALIA: 
                    return ZLR88621L_AC_FXS_RF14_AU;
                case COUNTRY_BE:    /* Belgium*/
                    return ZLR88621L_AC_FXS_RF14_BE;
                case COUNTRY_CN:    /* China  */
                    return ZLR88621L_AC_FXS_RF14_CN;
                case COUNTRY_GR:    /* German */
                    return ZLR88621L_AC_FXS_RF14_DE;
                case COUNTRY_FL:    /* Finland*/
                    return ZLR88621L_AC_FXS_RF14_FI;
                case COUNTRY_FR:    /* France */
                    return ZLR88621L_AC_FXS_RF14_FR;
                case COUNTRY_IT:    /* Italy  */
                    return ZLR88621L_AC_FXS_RF14_IT;
                case COUNTRY_JP:    /* Japan  */
                    return ZLR88621L_AC_FXS_RF14_JP;
                case COUNTRY_SE:    /* Sweden */
                    return ZLR88621L_AC_FXS_RF14_SE;
                case COUNTRY_HK:
                case COUNTRY_TW:    
                case COUNTRY_UK:
                case COUNTRY_USA:
                    PRINT_MSG("Set SLIC impedance to 600 ohm.\n");
                    return ZLR88621L_AC_FXS_RF14_600R_DEF;

                default:
                    PRINT_R("country wasn't defined. \
                            Set to default SLIC impedance 600 ohm.\n");
                    return ZLR88621L_AC_FXS_RF14_600R_DEF;
            }
#endif /* CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_WIDEBAND */

        break;

        default:
            PRINT_R("%s: Unknown device type %d\n", __FUNCTION__, deviceType);
            return NULL;
    }

    PRINT_R("%s: Unknown error\n", __FUNCTION__);

    return NULL;
}

/***********************************************************************
 * SLIC_Set_Impedance_Country_Microsemi() - 
 *
 * inputs:
 *  country => country for which AC profile needs to be set
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Set_Impedance_Country_Microsemi(
    voip_snd_t      *this,
    unsigned short  country,
    unsigned short  impd)
{
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpProfilePtrType        acProfile;
    VpStatusType            status;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    PRINT_MSG(" %s Country %d \n",__FUNCTION__, country);

    acProfile = Snd_Ms_Get_Country_AC_Profile(pLineInfo, country);

    if (acProfile == NULL) {
        return;
    }
    
    status = VpConfigLine(&pLineInfo->lineCtx, 
                            acProfile,
                            VP_PTABLE_NULL,
                            VP_PTABLE_NULL);

}

/***********************************************************************
 * SLIC_Set_Impendance_Microsemi() - 
 *
 * inputs:
 *  
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Set_Impedance_Microsemi(
    voip_snd_t      *this,
    unsigned short  preset)
{
    MsLineInfoType * const      pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    PRINT_R("%s is not implemented.\n", __FUNCTION__);

    return;
}

/***********************************************************************
 * OnHookLineReversal_Microsemi() - Change line state to forward or reverse on hook 
 *
 * inputs:
 *  bReversal - //0: Forward On-Hook Transmission, 1: Reverse On-Hook Transmission
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void OnHookLineReversal_Microsemi(
    voip_snd_t      *this,
    unsigned char   bReversal)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    PRINT_MSG("reverse %d \n", bReversal);
    
    if (bReversal) {
        status = VpSetLineState(pLineCtx, VP_LINE_OHT_POLREV);
        ASSERT_microsemi(status, "VpSetLineState(OHT_POLREV) ");
    } else {
        status = VpSetLineState(pLineCtx, VP_LINE_OHT);
        ASSERT_microsemi(status, "VpSetLineState(OHT) ");
    }

    return;
}

/***********************************************************************
 * SLIC_Set_LineVoltageZero_Microsemi() - Disconnect a line.
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Set_LineVoltageZero_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    status = VpSetLineState(pLineCtx, VP_LINE_DISCONNECT);
    ASSERT_microsemi(status, "VpSetLineState(VP_LINE_DISCONNECT) ");

    return;
}

/***********************************************************************
 * SLIC_CPC_Gen_Microsemi() - Get a state before disconnecting a line.
 *
 * inputs:
 *  n/a
 *  
 * outputs:
 *  line state.
 ***********************************************************************/
static uint8 SLIC_CPC_Gen_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;
    VpLineStateType         lineState;
    uint8                   pre_linefeed;

    DEBUG_API_PRINT();

    // save current linefeed status
    status = VpGetLineState(pLineCtx, &lineState);
    ASSERT_microsemi(status, "VpGetLineState() ");
    
    pre_linefeed = (uint8)lineState;

    status = VpSetLineState(pLineCtx, VP_LINE_DISCONNECT);
    ASSERT_microsemi(status, "VpSetLineState(DISCONNECT) ");

    return pre_linefeed;

}

/***********************************************************************
 * SLIC_CPC_Check_Microsemi() - 
 *
 * inputs:
 *  n/a
 *  
 * outputs:
 *  line state.
 ***********************************************************************/
static void SLIC_CPC_Check_Microsemi(
    voip_snd_t *this,
    uint8       pre_linefeed)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }
    
    status = VpSetLineState(pLineCtx, (VpLineStateType)pre_linefeed);
    ASSERT_microsemi(status, "VpSetLineState() ");

    return;
}

/***********************************************************************
 * SendNTTCAR_Microsemi() - 
 *
 * inputs:
 *  n/a
 *  
 * outputs:
 *  line state.
 ***********************************************************************/
static void SendNTTCAR_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpProfilePtrType        ringCadence;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    /* Load short ring cadence */
    if (VP_DEV_880_SERIES == pLineInfo->pDevInfo->VpDevType) {
        ringCadence = LE880_RING_CAD_SHORT;
    } 

    if ((VP_DEV_886_SERIES == pLineInfo->pDevInfo->VpDevType) ||
        (VP_DEV_887_SERIES == pLineInfo->pDevInfo->VpDevType)) {
        ringCadence = Le9662_RING_CAD_SHORT;
    }

    if (VP_DEV_890_SERIES == pLineInfo->pDevInfo->VpDevType) {
        ringCadence = LE890_BB_RING_CAD_SHORT;
    }

    status = VpInitRing(&pLineInfo->lineCtx, 
                        ringCadence,
                        VP_PTABLE_NULL);
   
    /* Ring the FXS */
    status = VpSetLineState(&pLineInfo->lineCtx, VP_LINE_RINGING);

    return;
}

/***********************************************************************
 * SendNTTCAR_check_Microsemi() - 
 *
 * inputs:
 *  time_out - 
 *  
 * outputs:
 *  
 ***********************************************************************/
static unsigned int SendNTTCAR_check_Microsemi(
    voip_snd_t          *this,
    unsigned long       time_out)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    VpLineCtxType           *pLineCtx = &pLineInfo->lineCtx;
    unsigned int            chid;
    bool                    hook_status = FALSE;

    DEBUG_API_PRINT();

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
    chid = pLineInfo->chanId;

    if (chid > (SUPPORT_CH_NUM-1)) {
        PRINT_MSG("%s, line%d, chid %d is over range(%d).\n", 
                __FUNCTION__, __LINE__, chid, SUPPORT_CH_NUM);
    }

    if (NTTCar1stCheckFlag[chid] == 0) {
        NTTCar1stCheckFlag[chid] = 1;
        time_out_modify[chid] = timetick + 6000;
    }
#endif

    /* Check Phone Hook State */
    status = VpGetLineStatus(pLineCtx, VP_INPUT_HOOK, &hook_status);
    ASSERT_microsemi(status, "VpGetLineStatus(INPUT_HOOK) ");
  
    /* if phone on-hook */
    if (hook_status == FALSE) {
        
        /* time_after(a,b) returns true if the time a is after time b. */
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
        if (timetick_after(timetick,time_out_modify[chid])) {
#else
        if (timetick_after(timetick,time_out)) {
#endif
            /* don't return 0, return 1, report time out don't wait */
            PRINT_R("wait off-hook timeout...\n");
        } else {
            return 0;
        }
    }

    /* Set Reverse On-Hook Transmission */        
    
    /* if phone off-hook, set Reverse On-Hook Transmission */
    status = VpSetLineState(&pLineInfo->lineCtx, VP_LINE_OHT_POLREV);
    
    /* restore the register */
    SLIC_Set_Ring_Cadence_Microsemi(this, 
                                    pLineInfo->pDevInfo->cadOnMs, 
                                    pLineInfo->pDevInfo->cadOffMs);
        
#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
    NTTCar1stCheckFlag[chid] = 0;
#endif
    
    PRINT_MSG("Set normal ring\n");
    
    return 1;
}

/***********************************************************************
 * DisableOscillators_Microsemi() - 
 *
 * inputs:
 *  n/a 
 *  
 * outputs:
 *  n/a
 ***********************************************************************/
static void DisableOscillators_Microsemi(
    voip_snd_t *this)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

 /***********************************************************************
  * SetOnHookTransmissionAndBackupRegister_Microsemi() -  use for DTMF caller id
  *
  * inputs:
  *  n/a 
  *  
  * outputs:
  *  n/a
  ***********************************************************************/
static void SetOnHookTransmissionAndBackupRegister_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    status = VpSetLineState(&pLineInfo->lineCtx, VP_LINE_OHT);
    ASSERT_microsemi(status, "VpSetLineState(VP_LINE_OHT) ");
}

/***********************************************************************
 * RestoreBackupRegisterWhenSetOnHookTransmission_Microsemi() -  use for DTMF caller id
 *
 * inputs:
 *  n/a 
 *  
 * outputs:
 *  n/a
 ***********************************************************************/
static inline void RestoreBackupRegisterWhenSetOnHookTransmission_Microsemi(
    voip_snd_t *this) 
{
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    PRINT_R("%s is not implemented.\n", __FUNCTION__);

    return;
}

/***********************************************************************
 * FXS_FXO_DTx_DRx_Loopback_Microsemi() - 
 *
 * inputs:
 *  daa_snd -  
 *  enable -
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void FXS_FXO_DTx_DRx_Loopback_Microsemi(
    voip_snd_t  *this,
    voip_snd_t  *daa_snd,
    unsigned int enable)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

/***********************************************************************
 * SLIC_OnHookTrans_PCM_start_Microsemi() - Forward On-Hook Transmission
 *
 * inputs:
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_OnHookTrans_PCM_start_Microsemi(
    voip_snd_t *this)
{
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    SLIC_Set_PCM_state_Microsemi(this, SLIC_PCM_ON);
    OnHookLineReversal_Microsemi(this, 0);       //Forward On-Hook Transmission
    
    PRINT_MSG("SLIC_OnHookTrans_PCM_start, ch = %d\n", this ->sch);
}

/***********************************************************************
 * SLIC_Set_Param_Microsemi() - 
 *
 * inputs:
 * slic_type - 
 * param_type -
 * pParam -
 * param_size -
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Set_Param_Microsemi(
    voip_snd_t      *this,
    unsigned int    slic_type,
    unsigned int    param_type,
    unsigned char*  pParam,
    unsigned int    param_size)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

/***********************************************************************
 * SLIC_Read_Reg_Microsemi() - write the specific register of microsemi SLIC
 *
 * inputs:
 *   num - 
 *   len -
 *   val -
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Read_Reg_Microsemi(
    voip_snd_t      *this,
    unsigned int    num,
    unsigned char   *len,
    unsigned char   *val)
{
    MsLineInfoType * const pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    VpMpiCmd(pLineInfo->pDevInfo->deviceId, 1, num, *len, val);
    
    return;
}

/***********************************************************************
 * SLIC_Write_Reg_Microsemi() - write the specific register of microsemi SLIC
 *
 * inputs:
 *   num - 
 *   len -
 *   val -
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Write_Reg_Microsemi(
    voip_snd_t      *this,
    unsigned int    num,
    unsigned char   *len,
    unsigned char   *val)
{
    MsLineInfoType * const pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }

    VpMpiCmd(pLineInfo->pDevInfo->deviceId, 0, num, *len, val);
    
    return;
}

/***********************************************************************
 * SLIC_Read_Ram_Microsemi() - 
 *
 * inputs:
 *   num - 
 *   val -
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Read_Ram_Microsemi(
    voip_snd_t      *this,
    unsigned short  num,
    unsigned int    *val) 
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

/***********************************************************************
 * SLIC_Write_Ram_Microsemi() - 
 *
 * inputs:
 *   num - 
 *   val -
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Write_Ram_Microsemi(
    voip_snd_t      *this,
    unsigned short  num,
    unsigned int    val)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

/***********************************************************************
 * SLIC_Dump_Reg_Microsemi() - dump all the SLIC register values
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Dump_Reg_Microsemi(
    voip_snd_t *this)
{
    VpDevCtxType *pDevCtx = VP_NULL;
    MsLineInfoType * const pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    pDevCtx = &pLineInfo->pDevInfo->devCtx;    
    VpRegisterDump(pDevCtx);
}

/***********************************************************************
 * SLIC_Dump_Ram_Microsemi() - dump value of device and line objects
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  n/a
 ***********************************************************************/

static void SLIC_Dump_Ram_Microsemi(
    voip_snd_t *this)
{
    VpDevCtxType *pDevCtx = VP_NULL;

    MsLineInfoType * const pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();    

    pDevCtx = &pLineInfo->pDevInfo->devCtx;    
    VpObjectDump(VP_NULL, pDevCtx);
    VpObjectDump(&pLineInfo->lineCtx, VP_NULL);    
    return;
}

/***********************************************************************
 * SLIC_Reset_Microsemi() - Reset the microsemi device (calls makedevice object, makelineobject)
 *
 * inputs:
 *  *this - 
 *  codec_law - 
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Reset_Microsemi(
    voip_snd_t *this,
    int         codecLaw)
{
    int                     rtn, i;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();
        
    PRINT_MSG("Resetting SLIC %p \n", (void *)pLineInfo->pDevInfo->deviceId);

    /* Initialize API-2 device settings */
    
    rtn = Snd_Ms_Init_Device(pLineInfo->pDevInfo);
    
    if (rtn == FAILED) {
        PRINT_R("Error (%d:x) %s\n", i, __FUNCTION__);
    }

    return;
}

/***********************************************************************
 * SLIC_Show_ID_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void SLIC_Show_ID_Microsemi(
    voip_snd_t *this)
{
    uint8                   devicePcn, deviceRcn, res[2];
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    //send read command through SPI to read two bytes
    VpMpiCmd(pLineInfo->pDevInfo->deviceId, 1, VP886_R_RCNPCN_RD, VP886_R_RCNPCN_LEN, res);
    
    devicePcn = res[VP886_R_RCNPCN_PCN_IDX];
    deviceRcn = res[VP886_R_RCNPCN_RCN_IDX]; 

    PRINT_MSG("devicePcn = [%x], deviceRcn = [%x]\n", devicePcn, deviceRcn);

    /* MPI Failure if the PCN and RCN are both 0x00 or 0xFF */
    if (((devicePcn == 0xFF) && (deviceRcn == 0xFF))||
       ((devicePcn == 0x00) && (deviceRcn == 0x00))) {
        PRINT_MSG("Device %p Failed to Detect Revision/PCN Properly\n",
                (void *)pLineInfo->pDevInfo->deviceId);

        return;
    }
    return;
}

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
/***********************************************************************
 * SLIC_LineTest_Microsemi() - Run line test on the SLIC.
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  n/a
 ***********************************************************************/

static int Snd_Ms_SLIC_LineTest_Microsemi(
    voip_snd_t *this, LtTestIdType testId, unsigned int *val)
{
    LtTestAttributesType    testAtts;
    LtTestAttributesType    *pTestAtts = NULL;
    TestInputType           testInputs;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    unsigned int            runtime = 0;
    /* user handle value (can be any number)*/    
    uint16 ltParamHandle = 0xAAAA;
    LtTestStatusType ltStatus = LT_STATUS_RUNNING;        

    DEBUG_API_PRINT(); 

    /*      
         * if the LtTestAttributesType argument to LtStartTest is NULL     
         * then the LT-API will use a set of default inputs,     
         * criteria, and topology value     
         */    
    pTestAtts = &testAtts;    
    VpMemSet(pTestAtts, 0, sizeof(LtTestAttributesType));    


    /* is a line test already in progress */    
    if (pLineCtxUnderTest != NULL) {        
        PRINT_R("Line Test is already in progress\n");        
        return FAIL;    
    }    

    switch (testId) {            

        case LT_TID_RINGERS:            

TESTAGAIN:
            if (runtime == 0) {
                /* use default parameters */
                pTestAtts->inputs.pRingersInp = VP_NULL;
            } else if (runtime == 1) {
                /* configure the input params */            
                testInputs.ringersInp.ringerTestType = LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE;
                testInputs.ringersInp.vRingerTest = 56000;
                testInputs.ringersInp.freq = 20000;
                testInputs.ringersInp.renFactor = 1210000;
                pTestAtts->inputs.pRingersInp = &testInputs.ringersInp;
            }

            //TODO: Remove the code
            //pTestAtts->criteria.pRingersCrt = VP_NULL;
            //pTestAtts->topology.dummyPtr = VP_NULL;
            //pTestAtts->topology.pVp886Topology = VP_NULL;
            //pTestAtts->calFactors.dummyPtr = VP_NULL;
            break;        

        case LT_TID_ROH: 
            /* Settting pAttribues->criteria.pRohCrt to VP_NULL is default criterial and 
                        * is used to determine the outcome of the test. At the moment no test criteria 
                        * has been defined for this test. The applications must specify default
                        * criteria for this test using the mechanisms defined above to ensure 
                        * compatibility to future versions of the LT-API.
                        */
            pTestAtts->criteria.pRohCrt = VP_NULL;
            /*
                    pTestAtts->inputs.pRohInp = VP_NULL;            
                    pTestAtts->criteria.pRohCrt = VP_NULL;          
                    pTestAtts->topology.dummyPtr = VP_NULL;         
                    pTestAtts->topology.pVp890Topology = VP_NULL;           
                    pTestAtts->calFactors.dummyPtr = VP_NULL;
                    */
            break;

        case LT_TID_LINE_V: 
            pTestAtts->criteria.pLintVCrt = VP_NULL;
            break;

        case LT_TID_RES_FLT: 
            pTestAtts->criteria.pResFltCrt = VP_NULL;
            break;

        case LT_TID_CAP: 
            pTestAtts->criteria.pCapCrt = VP_NULL;
            break;

        default:             
            printk("Unsupported Line Test ID %i\n", testId);            
            return LT_STATUS_TEST_NOT_SUPPORTED;        
    }


    /* enable/disable runtime debug */
    pTestAtts->ltDbgFlag = LT_DBG_ALL;

    /* Start the line test */    
    ltStatus = LtStartTest(&pLineInfo->lineCtx, testId, ltParamHandle, pTestAtts,
                            &pLineInfo->ltTestTempData, &pLineInfo->ltTestResults, &pLineInfo->ltTestCtx);    
 
    if (ltStatus != LT_STATUS_RUNNING) {        
        printk("Unable to start testId %i: ltStatus = %i\n", testId, ltStatus);
        return FAIL;
    }        

    /* indicate that a test is in progress */    
    pLineCtxUnderTest = &pLineInfo->lineCtx;
    down(&pLineInfo->ltTestSem);

    /* Update results in the buffer provided by the application */
    Snd_Ms_LineTest_Update_Results(testId, &pLineInfo->ltTestResults, val);


    if (testId == LT_TID_RINGERS) {     

       /* Return the result to the calling function
              * In the first run, use the default setting to run ringer test.
              * If the first run fails, the ringerTestType would be assign to LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE
              * and run test again. This is done because some phones can not be detected correctly in default setting 
              * but in LT_RINGER_ELECTRONIC_PHNE_TEST_3_ELE.
              */
         
        if (pLineInfo->ltTestResults.result.ringers.ren >= THRESHOLD) {
            return PASS; 
        } else if ((pLineInfo->ltTestResults.result.ringers.ren < THRESHOLD) && (runtime == 0)) {
            runtime = 1;
            goto TESTAGAIN;
        } else if ((pLineInfo->ltTestResults.result.ringers.ren < THRESHOLD) && (runtime == 1)) {
            return FAIL;
        }
    }
    else {
        return PASS;
    }
}

static int SLIC_LineTest_Microsemi(
    voip_snd_t *this, uint16 tID, unsigned int *val)
{
    return Snd_Ms_SLIC_LineTest_Microsemi(this, tID, val);
}

static int SLIC_PortDetect_Microsemi(
    voip_snd_t *this, unsigned int *val)
{
    return Snd_Ms_SLIC_LineTest_Microsemi(this, LT_TID_RINGERS, val);
}

static int SLIC_LineROH_Microsemi(
    voip_snd_t *this, unsigned int *val)
{
    return Snd_Ms_SLIC_LineTest_Microsemi(this, LT_TID_ROH, val);
}
static int SLIC_LineVOLTAGE_Microsemi(
    voip_snd_t *this, unsigned int *val)
{
    return Snd_Ms_SLIC_LineTest_Microsemi(this, LT_TID_LINE_V, val);
}
static int SLIC_LineRESFLT_Microsemi(
    voip_snd_t *this, unsigned int *val)
{
    //return Snd_Ms_SLIC_LineTest_Microsemi(this, LT_TID_RES_FLT, val);
    return Snd_Ms_SLIC_LineTest_Microsemi(this, LT_TID_CAP, val);
}

static int SLIC_LineCap_Microsemi(
    voip_snd_t *this, unsigned int *val)
{
    return Snd_Ms_SLIC_LineTest_Microsemi(this, LT_TID_CAP, val);
}

#endif

/*
 *--------------------------------------------------------
 * FXO (DAA) specific function calls
 * --------------------------------------------------------
 */

/***********************************************************************
 * DAA_Set_Rx_Gain_Microsemi()
 *
 * inputs:
 *  rx_gain => -12 to +12 dB
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void DAA_Set_Rx_Gain_Microsemi(
    voip_snd_t      *this,
    unsigned char   rxGain)
{
    //obsolete API
#ifndef CONFIG_VOIP_SDK
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
#endif

    return;
}

/***********************************************************************
 * DAA_Set_Tx_Gain_Microsemi()
 *
 * inputs:
 *  tx_gain => -12 to +12 dB
 *
 * outputs:
 *  n/a
 ***********************************************************************/
static void DAA_Set_Tx_Gain_Microsemi(
    voip_snd_t      *this,
    unsigned char   txGain)
{
    //obsolete API
#ifndef CONFIG_VOIP_SDK
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
#endif

    return;
}

/***********************************************************************
 * DAA_Check_Line_State_Microsemi() - Check whether line is connected/disconnected/busy
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *   0: connect
 *   1: not connect
 *   2: busy
 ***********************************************************************/
static int DAA_Check_Line_State_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    VpLineStateType         lineState;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    int                     ret = 1;
    
    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return ret;
    }

    status = VpGetLineState(&pLineInfo->lineCtx, &lineState);
    ASSERT_microsemi(status, "VpGetLineState() ");
    PRINT_MSG("VpGetLineState(%d) \n", lineState); 

    if ((lineState == VP_LINE_STANDBY) || 
        (lineState == VP_LINE_STANDBY_POLREV) ||
        (lineState == VP_LINE_ACTIVE) ||
        (lineState == VP_LINE_ACTIVE_POLREV) ||
        (lineState == VP_LINE_FXO_OHT)) {
        ret = 0;
    } else if (lineState == VP_LINE_DISCONNECT) {
        ret = 1;
    } else if (lineState == VP_LINE_FXO_TALK) {
        ret = 2;
    }

    return ret;
}

/***********************************************************************
 * DAA_On_Hook_Microsemi() - Set line to on hook state.
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  n/a
 *   
 ***********************************************************************/
static void DAA_On_Hook_Microsemi(
    voip_snd_t *this)
{
    VpStatusType status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }
   
    status = VpSetLineState(&pLineInfo->lineCtx, VP_LINE_FXO_OHT);
    ASSERT_microsemi(status, "VpSetLineState(VP_LINE_OHT) ");

    if (VP_STATUS_SUCCESS == status) {
        pLineInfo->hookStatus = 0;
    }
    
    return;
}

/***********************************************************************
 * DAA_Off_Hook_Microsemi() - Set line to off hook state.
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  VP_STATUS_SUCCESS - if successful
 *   
 ***********************************************************************/
static int DAA_Off_Hook_Microsemi(
    voip_snd_t *this)
{
    VpStatusType status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return 0;
    }
    
    status = VpSetLineState(&pLineInfo->lineCtx, VP_LINE_FXO_TALK);
    ASSERT_microsemi(status, "VpSetLineState(VP_LINE_FXO_TALK) ");

    if (VP_STATUS_SUCCESS == status) {
        pLineInfo->hookStatus = 1;
    }
    
    return status;
}

/***********************************************************************
 * DAA_Hook_Status_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  1 - Off hook
 *  0 - On hook
 *   
 ***********************************************************************/
static unsigned char DAA_Hook_Status_Microsemi(
    voip_snd_t *this, int directly)
{
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    return pLineInfo->hookStatus;
}

/***********************************************************************
 * DAA_Polarity_Reversal_Det_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  1 - reverse polarity
 *  0 - normal polarity
 *   
 ***********************************************************************/
static unsigned char DAA_Polarity_Reversal_Det_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    bool                    polStatus;
    
    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return 0;
    }
    
    status = VpGetLineStatus(&pLineInfo->lineCtx, VP_INPUT_POLREV, &polStatus);
    ASSERT_microsemi(status, "VpGetLineStatus(VP_INPUT_POLREV) ");
    PRINT_MSG("VpGetLineStatus(%d) \n", polStatus); 

    if (TRUE == polStatus) {
        return 1;
    } else {
        return 0;
    }
}


/***********************************************************************
 * DAA_Bat_DropOut_Det_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  1 - battery dropped
 *  0 - battery connected
 *   
 ***********************************************************************/
static unsigned char DAA_Bat_DropOut_Det_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    bool                    feedStatus;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return 0;
    }

    status = VpGetLineStatus(&pLineInfo->lineCtx, VP_INPUT_FEED_DIS, &feedStatus);
    ASSERT_microsemi(status, "VpGetLineStatus(VP_INPUT_FEED_DIS) ");
    PRINT_MSG("VpGetLineStatus(%d) \n", feedStatus); 

    if (TRUE == feedStatus) {
        return 1;
    } else {
        return 0;
    }
    return 0;
}

/***********************************************************************
 * DAA_Ring_Detection_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  1 - ringing
 *  0 - not ringing
 *   
 ***********************************************************************/
static int DAA_Ring_Detection_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    bool                    ringStatus;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return 0;
    }

    status = VpGetLineStatus(&pLineInfo->lineCtx, VP_INPUT_RINGING, &ringStatus);
    ASSERT_microsemi(status, "VpGetLineStatus(VP_INPUT_RINGING) ");
    PRINT_MSG("VpGetLineStatus(%d) \n", ringStatus); 

    if (TRUE == ringStatus) {
        return 1;
    } else {
        return 0;
    }
}

/***********************************************************************
 * DAA_Positive_Negative_Ring_Detect_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  1 - ringing
 *  0 - not ringing
 *   
 ***********************************************************************/
static unsigned int DAA_Positive_Negative_Ring_Detect_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    bool                    ringStatus;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return 0;
    }

    status = VpGetLineStatus(&pLineInfo->lineCtx, VP_INPUT_RINGING, &ringStatus);
    ASSERT_microsemi(status, "VpGetLineStatus(VP_INPUT_RINGING) ");
    PRINT_MSG("VpGetLineStatus(%d) \n", ringStatus); 

    if (TRUE == ringStatus) {
        return 1;
    } else {
        return 0;
    }
    return 0;
}

/***********************************************************************
 * DAA_Get_Polarity_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  0: normal
 *  1: reverse
 *   
 ***********************************************************************/
static unsigned int DAA_Get_Polarity_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;
    bool                    polStatus;

    DEBUG_API_PRINT();
    
    if (!pLineInfo->isReady) {
        return 0;
    }

    status = VpGetLineStatus(&pLineInfo->lineCtx, VP_INPUT_POLREV, &polStatus);
    ASSERT_microsemi(status, "VpGetLineStatus(VP_INPUT_POLREV) ");
    PRINT_MSG("VpGetLineStatus(%d) \n", polStatus); 

    if (TRUE == polStatus) {
        return 1;
    } else {
        return 0;
    }
}

/***********************************************************************
 * DAA_Get_Line_Voltage_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  
 *   
 ***********************************************************************/
static unsigned short DAA_Get_Line_Voltage_Microsemi(
    voip_snd_t *this)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return 0;
}

/***********************************************************************
 * DAA_OnHook_Line_Monitor_Enable_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  
 *   
 ***********************************************************************/
static void DAA_OnHook_Line_Monitor_Enable_Microsemi(
    voip_snd_t *this)
{
    VpStatusType            status;
    MsLineInfoType * const  pLineInfo = (MsLineInfoType*)this->priv;

    DEBUG_API_PRINT();

    if (!pLineInfo->isReady) {
        return;
    }
    
    status = VpSetLineState(&pLineInfo->lineCtx, VP_LINE_FXO_OHT);
    ASSERT_microsemi(status, "VpSetLineState(VP_LINE_OHT) ");

    return;
}

/***********************************************************************
 * DAA_Set_PulseDial_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  
 *   
 ***********************************************************************/
static void DAA_Set_PulseDial_Microsemi(
    voip_snd_t *this,
    unsigned int pulse_enable)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

/***********************************************************************
 * DAA_Set_Country_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  
 *   
 ***********************************************************************/
static void DAA_Set_Country_Microsemi(
    voip_snd_t *this,
    unsigned int country)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

/***********************************************************************
 * DAA_Set_Hybrid_Microsemi() - 
 *
 * inputs:
 *  n/a
 *
 * outputs:
 *  
 *   
 ***********************************************************************/
static void DAA_Set_Hybrid_Microsemi(
    voip_snd_t *this,
    unsigned char index)
{
    PRINT_R("%s is not implemented.\n", __FUNCTION__);
    return;
}

__attribute__ ((section(".snd_desc_data")))
const snd_ops_fxs_t ms_snd_fxs_ops = {
    
    // common operation 
    .enable                                         = Enable_Microsemi,

    // for each snd_type 
    .FXS_Ring                                       = FXS_Ring_Microsemi,
    .FXS_Check_Ring                                 = FXS_Check_Ring_Microsemi,
    .FXS_Line_Check                                 = FXS_Line_Check_Microsemi,    
    .SLIC_Set_PCM_state                             = SLIC_Set_PCM_state_Microsemi,
    .SLIC_Get_Hook_Status                           = SLIC_Get_Hook_Status_Microsemi,
    .SLIC_Set_Power_Save_Mode                       = SLIC_Set_Power_Save_Mode_Microsemi,
    .SLIC_Set_FXS_Line_State                        = SLIC_Set_FXS_Line_State_Microsemi,

    .Set_SLIC_Tx_Gain                               = Adjust_SLIC_Tx_Gain_Microsemi,
    .Set_SLIC_Rx_Gain                               = Adjust_SLIC_Rx_Gain_Microsemi,
    .SLIC_Set_Ring_Cadence                          = SLIC_Set_Ring_Cadence_Microsemi,
    .SLIC_Set_Multi_Ring_Cadence                    = SLIC_Set_Multi_Ring_Cadence_Microsemi,
    .SLIC_Set_Ring_Freq_Amp                         = SLIC_Set_Ring_Freq_Amp_Microsemi,
    .SLIC_Set_Impendance_Country                    = SLIC_Set_Impedance_Country_Microsemi, 
    .SLIC_Set_Impendance                            = SLIC_Set_Impedance_Microsemi,
    .OnHookLineReversal                             = OnHookLineReversal_Microsemi,    
    .SLIC_Set_LineVoltageZero                       = SLIC_Set_LineVoltageZero_Microsemi,

    .SLIC_CPC_Gen                                   = SLIC_CPC_Gen_Microsemi,
    .SLIC_CPC_Check                                 = SLIC_CPC_Check_Microsemi,    

    .SendNTTCAR                                     = SendNTTCAR_Microsemi,
    .SendNTTCAR_check                               = SendNTTCAR_check_Microsemi,

    .disableOscillators                             = DisableOscillators_Microsemi,

    .SetOnHookTransmissionAndBackupRegister         = SetOnHookTransmissionAndBackupRegister_Microsemi,   
    .RestoreBackupRegisterWhenSetOnHookTransmission = RestoreBackupRegisterWhenSetOnHookTransmission_Microsemi,    

    .FXS_FXO_DTx_DRx_Loopback                       = FXS_FXO_DTx_DRx_Loopback_Microsemi,
    .SLIC_OnHookTrans_PCM_start                     = SLIC_OnHookTrans_PCM_start_Microsemi,
    .SLIC_set_param                                 = SLIC_Set_Param_Microsemi,

    /* read/write register/ram */
    .SLIC_read_reg                                  = SLIC_Read_Reg_Microsemi,
    .SLIC_write_reg                                 = SLIC_Write_Reg_Microsemi,
    .SLIC_read_ram                                  = SLIC_Read_Ram_Microsemi,
    .SLIC_write_ram                                 = SLIC_Write_Ram_Microsemi,
    .SLIC_dump_reg                                  = SLIC_Dump_Reg_Microsemi,
    .SLIC_dump_ram                                  = SLIC_Dump_Ram_Microsemi,

    .SLIC_show_ID                                   = SLIC_Show_ID_Microsemi,
    .SLIC_reset                                     = SLIC_Reset_Microsemi,
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)    
#if 0
    .PortDetect                                     = SLIC_PortDetect_Microsemi, 
    .LineROH                                        = SLIC_LineROH_Microsemi,
    .LineVOLTAGE                                    = SLIC_LineVOLTAGE_Microsemi,
    .LineRESFLT                                     = SLIC_LineRESFLT_Microsemi,
    .LineCap                                        = SLIC_LineCap_Microsemi 
#else
    .LineTest					     = SLIC_LineTest_Microsemi,
#endif
#endif    
};

const snd_ops_daa_t ms_snd_daa_ops = {
    
    /* common operation */
    .enable                             = Enable_Microsemi,

    /* for each snd_type */
    .DAA_Set_Rx_Gain                    = DAA_Set_Rx_Gain_Microsemi,
    .DAA_Set_Tx_Gain                    = DAA_Set_Tx_Gain_Microsemi,
    .DAA_Check_Line_State               = DAA_Check_Line_State_Microsemi,
    .DAA_On_Hook                        = DAA_On_Hook_Microsemi,
    .DAA_Off_Hook                       = DAA_Off_Hook_Microsemi,

    .DAA_Hook_Status                    = DAA_Hook_Status_Microsemi,
    .DAA_Polarity_Reversal_Det          = DAA_Polarity_Reversal_Det_Microsemi,
    .DAA_Bat_DropOut_Det                = DAA_Bat_DropOut_Det_Microsemi,
    .DAA_Ring_Detection                 = DAA_Ring_Detection_Microsemi,
    .DAA_Positive_Negative_Ring_Detect  = DAA_Positive_Negative_Ring_Detect_Microsemi,
    .DAA_Get_Polarity                   = DAA_Get_Polarity_Microsemi,
    .DAA_Get_Line_Voltage               = DAA_Get_Line_Voltage_Microsemi,
    .DAA_OnHook_Line_Monitor_Enable     = DAA_OnHook_Line_Monitor_Enable_Microsemi,
    .DAA_Set_PulseDial                  = DAA_Set_PulseDial_Microsemi,
    .DAA_Set_Country                    = DAA_Set_Country_Microsemi,
    .DAA_Set_Hybrid                     = DAA_Set_Hybrid_Microsemi,

    /* read/write register/ram */
    .DAA_read_reg                       = SLIC_Read_Reg_Microsemi,
    .DAA_write_reg                      = SLIC_Write_Reg_Microsemi,
    .DAA_read_ram                       = SLIC_Read_Ram_Microsemi,
    .DAA_write_ram                      = SLIC_Write_Ram_Microsemi,
    .DAA_dump_reg                       = SLIC_Dump_Reg_Microsemi,
    .DAA_dump_ram                       = SLIC_Dump_Ram_Microsemi,
};


/**********************************************************************
 * Functions
 ***********************************************************************/


 /*
************************************************************************
** FUNCTION:   Snd_Ms_Get_Number_Of_Slics
**
** PURPOSE:    Get total number of SLICs of a type.
**
** PARAMETERS: 
**             MS_SLIC_TYPE : type of SLIC
**
** RETURNS:    
**             Number of SLIC of MS_SLIC_TYPE
**
***********************************************************************/
static unsigned int Snd_Ms_Get_Number_Of_Slics(
    MS_SLIC_TYPE msSlicType)
{
    unsigned int noOfSlics = 0;
        
    switch (msSlicType) {
        
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI   

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116) 
        case MS_SLIC_TYPE_LE89116: {
            noOfSlics = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116_NR;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116      

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156) 
        case MS_SLIC_TYPE_LE89156: {
            noOfSlics = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156_NR;
            break;            
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156    

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9641)    
        case MS_SLIC_TYPE_LE9641: {
            noOfSlics = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9641_NR;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9641

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE88601)    
        case MS_SLIC_TYPE_LE88601: {
            noOfSlics = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE88601_NR;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE88601

#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI  
                                                                                                                            
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9662)    
        case MS_SLIC_TYPE_LE9662: {
            noOfSlics = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9662_NR;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9662

#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI

        default:
            PRINT_R( "Invalid SLIC type (%d) passed !! (%s)\n", msSlicType, __FUNCTION__);
            noOfSlics = 0;
            break;
    }
    return noOfSlics;
}


 /*
************************************************************************
** FUNCTION:   Snd_Ms_Get_Pin_Cs
**
** PURPOSE:    Gets PIN CS number of a particular SLIC type.
**
** PARAMETERS: 
**             MS_SLIC_TYPE : type of SLIC
**
** RETURNS:    
**             PIN CS of MS_SLIC_TYPE
**
***********************************************************************/
static unsigned int Snd_Ms_Get_Pin_Cs(
    MS_SLIC_TYPE msSlicType)
{
    unsigned int pinCs = 0;
    
    switch (msSlicType) {
#ifndef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI  

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116)
        case MS_SLIC_TYPE_LE89116: {
            pinCs = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116_PIN_CS;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156) 
        case MS_SLIC_TYPE_LE89156: {
            pinCs = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156_PIN_CS;
            break;            
        }        
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9641)
        case MS_SLIC_TYPE_LE9641: {
            pinCs = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9641_PIN_CS;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9641

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE88601)
        case MS_SLIC_TYPE_LE88601: {
            pinCs = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE88601_PIN_CS;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156


#endif  //CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI   

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI 

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9662)
        case MS_SLIC_TYPE_LE9662: {
            pinCs = CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9662_PIN_CS;
            break;
        }
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9662

#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI       
        default:
            //PRINT_R( "Invalid SLIC type passed !! (%s)\n", __FUNCTION__);
            pinCs = 0;
            break;
    }
    return pinCs;
}

 /*
************************************************************************
** FUNCTION:   Snd_Ms_Get_Slic_Info
**
** PURPOSE:    Gets SLIC textual form, number of FXS, and number of FXO from a particular 
**                     SLIC type.
**
** PARAMETERS: 
**             MS_SLIC_TYPE : type of SLIC
**
** RETURNS:    
**             SLIC name
**
************************************************************************/
static void Snd_Ms_Get_Slic_Info(
    MS_SLIC_TYPE msSlicType,
    MsSlicInfo   *pMsSlicInfo)
{
    char *le89116 = "LE89116";
    char *le9662 = "LE9662";
    char *le89316 = "LE89316";
    char *le89156 = "LE89156";
    char *le9641 = "LE9641";
    char *le88601 = "LE88601";
     
    switch (msSlicType) {
        case MS_SLIC_TYPE_LE89116: {
            pMsSlicInfo->slicName = kmemdup(le89116, strlen(le89116)+1, GFP_KERNEL);
            pMsSlicInfo->noOfFxs  = 1;
            pMsSlicInfo->noOfFxo  = 0;
            pMsSlicInfo->aDeviceType = VP_DEV_890_SERIES;
            break;
        }
        case MS_SLIC_TYPE_LE9662: {
            pMsSlicInfo->slicName = kmemdup(le9662,strlen(le9662)+1, GFP_KERNEL);
            pMsSlicInfo->noOfFxs  = 2;
            pMsSlicInfo->noOfFxo  = 0;
            pMsSlicInfo->aDeviceType = VP_DEV_886_SERIES;
            break;
        }
        case MS_SLIC_TYPE_LE89316: {
            pMsSlicInfo->slicName = kmemdup(le89316,strlen(le89316)+1, GFP_KERNEL);
            pMsSlicInfo->noOfFxs  = 1;
            pMsSlicInfo->noOfFxo  = 1;
            pMsSlicInfo->aDeviceType = VP_DEV_890_SERIES;
            break;
        }
        case MS_SLIC_TYPE_LE89156: {
            pMsSlicInfo->slicName = kmemdup(le89156,strlen(le89156)+1, GFP_KERNEL);
            pMsSlicInfo->noOfFxs  = 1;
            pMsSlicInfo->noOfFxo  = 0;
            pMsSlicInfo->aDeviceType = VP_DEV_890_SERIES;
            break;
        }        
        case MS_SLIC_TYPE_LE9641: {
            pMsSlicInfo->slicName = kmemdup(le9641,strlen(le9641)+1, GFP_KERNEL);
            pMsSlicInfo->noOfFxs  = 1;
            pMsSlicInfo->noOfFxo  = 0;
            pMsSlicInfo->aDeviceType = VP_DEV_887_SERIES;
            break;
        }    
        case MS_SLIC_TYPE_LE88601: {
            pMsSlicInfo->slicName = kmemdup(le88601,strlen(le88601)+1, GFP_KERNEL);
            pMsSlicInfo->noOfFxs  = 2;
            pMsSlicInfo->noOfFxo  = 0;
            pMsSlicInfo->aDeviceType = VP_DEV_886_SERIES;
            break;
        }        
        default:
            //PRINT_R( "Invalid SLIC type passed !! (%s)\n", __FUNCTION__);
            pMsSlicInfo->slicName = NULL;
            pMsSlicInfo->noOfFxs  = 0;
            pMsSlicInfo->noOfFxo  = 0;
            break;
    }

}

 /*
************************************************************************
** FUNCTION:   Snd_Ms_Get_Other_Line
**
** PURPOSE:    Get another line from same or different device. 
                
**
** PARAMETERS: 
**             MsLineInfoType* : current line
**
** RETURNS:    
**             Another line
**
************************************************************************/
MsLineInfoType * Snd_Ms_Get_Other_Line(
    MsLineInfoType *pCurrentLineInfo)
{
    uint32                      deviceIndex = 0;
    struct MsDeviceInfoType    *pMsDevInfoType;
    uint8                       lineNum = 0;
    MsLineInfoType              *pFoundLineInfo = NULL;

    for (deviceIndex = 0; deviceIndex < gNoOfDevices && !pFoundLineInfo; deviceIndex++) {
        pMsDevInfoType = &msDevInfo[deviceIndex]; 

        for (lineNum = 0; lineNum < pMsDevInfoType->numLines; lineNum++) {
            MsLineInfoType *pLineInfo = &pMsDevInfoType->lineInfoType[lineNum];

            if (pCurrentLineInfo != pLineInfo) {
                pFoundLineInfo = pLineInfo;
                break;
            }
        }
    }
    return pFoundLineInfo;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Get_Codec_Type
**
** PURPOSE:    Convert Realtek PCM mode to Microsemi Codec type.
**
** PARAMETERS: 
**             BUS_DATA_FORMAT : Realtek PCM mode
**
** RETURNS:    
**             Microsemi Codec type
**
************************************************************************/
static VpOptionCodecType Snd_Ms_Get_Codec_Type(
    BUS_DATA_FORMAT pcmDataFormat)
{
    VpOptionCodecType codecType;

    switch (pcmDataFormat) {
        case BUSDATFMT_PCM_LINEAR:
            codecType = VP_OPTION_LINEAR;
            break;

        case BUSDATFMT_PCM_ALAW:
            codecType = VP_OPTION_ALAW;
            break;

        case BUSDATFMT_PCM_ULAW:
            codecType = VP_OPTION_MLAW;
            break;

        case BUSDATFMT_PCM_WIDEBAND_LINEAR:
            codecType = VP_OPTION_WIDEBAND;
            break;

        case BUSDATFMT_PCM_WIDEBAND_ALAW:
            codecType = VP_OPTION_WIDEBAND;
            break;

        case BUSDATFMT_PCM_WIDEBAND_ULAW:
            codecType = VP_OPTION_WIDEBAND;
            break;

        default:
            codecType = VP_OPTION_LINEAR;
            break;
    }
    return (codecType);
}


 /*
************************************************************************
** FUNCTION:   Snd_Ms_Get_RTKDevType
**
** PURPOSE:    Gets RTK  device type
**
** PARAMETERS: 
**             MS_SLIC_TYPE : type of SLIC
**
** RETURNS:    
**             Device type.
**
************************************************************************/
static RTKDevType Snd_Ms_Get_RTKDevType(
    MS_SLIC_TYPE            msSlicType)
{
    RTKDevType aRtkType;
    
    switch (msSlicType) {
        case MS_SLIC_TYPE_LE9662: 
        case MS_SLIC_TYPE_LE88601: {
            aRtkType = DEV_FXSFXS;
            PRINT_MSG( "RTK Type is %d (%s)\n", aRtkType, __FUNCTION__);
            break;
        }
        case MS_SLIC_TYPE_LE9641:
        case MS_SLIC_TYPE_LE89116: 
        case MS_SLIC_TYPE_LE89156: {
            aRtkType = DEV_FXS;
            PRINT_MSG( "RTK Type is %d (%s)\n", aRtkType, __FUNCTION__);
            break;
        }
        default:
            PRINT_MSG( "Invalid type passed !! (%s)\n", __FUNCTION__);
            aRtkType = -1;
            break;
    }

    return aRtkType;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Init_Set_Masks
**
** PURPOSE:   This function initialize the masks to unmask all events except the rollover event.
**
** PARAMETERS: 
**             pDevCtx - device context
**
** RETURNS:    
**              n/a
**
************************************************************************
*/
static void Snd_Ms_Init_Set_Masks(
    VpDevCtxType *pDevCtx)
{
    VpOptionEventMaskType eventMask;

    /* mask everything */
    VpMemSet(&eventMask, 0xFF, sizeof(VpOptionEventMaskType));

    /* unmask only the events the application is interested in */
    eventMask.faults = VP_EVCAT_FAULT_UNMASK_ALL;
    eventMask.signaling = VP_EVCAT_SIGNALING_UNMASK_ALL | VP_DEV_EVID_TS_ROLLOVER;
    eventMask.response = VP_EVCAT_RESPONSE_UNMASK_ALL;
    eventMask.test = VP_EVCAT_TEST_UNMASK_ALL;
    eventMask.process = VP_EVCAT_PROCESS_UNMASK_ALL;

    /* inform the API of the mask */
    VpSetOption(VP_NULL, pDevCtx, VP_OPTION_ID_EVENT_MASK, &eventMask);
 
    return;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Enable_Debugging
**
** PURPOSE:    This function enables debugging in VP-API. This funciton shall be called after 
**                      calling InitDevice.
**
** PARAMETERS: 
**             pMsLineInfoType - Pointer to Lineinfotype
**
** RETURNS:    
**             VP_STATUS_SUCCESS  - Debugging enabled
**             VP_STATUS_FAIL          - Debugging failed
**
************************************************************************
*/
static VpStatusType Snd_Ms_Enable_Debugging(
    MsLineInfoType *pLineInfo)
{
    VpStatusType            status;

    uint32                  debugOption = ( VP_DBG_ERROR |
                                            VP_DBG_WARNING | 
                                            //VP_DBG_INFO  |
                                            //VP_DBG_API_FUNC |
                                            //VP_DBG_API_FUNC_INT |
                                            VP_DBG_EVENT 
                                            //VP_DBG_HOOK |
                                            //VP_DBG_LINE_STATE |
                                            //VP_DBG_CALIBRATION |
                                            //VP_DBG_TEST_PCM 
                                            //VP_DBG_INTERRUPT
                                            );

    DEBUG_API_PRINT();

    PRINT_MSG("Enable debugging options %d \n", debugOption);
    status = VpSetOption(&pLineInfo->lineCtx,
                            VP_NULL,
                            VP_OPTION_ID_DEBUG_SELECT,
                            &debugOption);
    return status;

}

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Print_Voltage
**
** PURPOSE:    
**                      
**
** PARAMETERS: 
**             pName - Pointer to char, parameter name 
**             value    - Line test voltage type
**             pUnits  - Pointer to char, string providing units info
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_Voltage(
    char* pName,
    LtVoltageType value,
    char* pUnits)
{
    if (LT_VOLTAGE_NOT_MEASURED == value) {
        printk("%s %s \n", pName, "LT_VOLTAGE_NOT_MEASURED");
    } else {
        printk("%s %d %s", pName, value, pUnits);
    }
    return;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Print_Ren
**
** PURPOSE:    
**                      
**
** PARAMETERS: 
**             pName - Pointer to char, parameter name 
**             value    - Line test Ringer Equivalence Number (REN) type
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_Ren(
    char* pName,
    LtRenType value)
{
    if (LT_REN_NOT_MEASURED == value) {
        printk("%s %s \n", pName, "LT_REN_NOT_MEASURED");
    } else if (LT_MAX_CURRENT == value) {
        printk("%s %s \n", pName, "LT_MAX_REN");
    } else {
        printk("%s %d mREN \n", pName, value);
    }

    return;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_GetText
**
** PURPOSE:    
**                      
**
** PARAMETERS: 
**             table            - Pointer to char, parameter name 
**             pString         - Line test Ringer Equivalence Number (REN) type
**             val               - mask
**             const char * - 
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static bool Snd_Ms_LineTest_GetText(
    const LutType table[],
    char **pString,
    unsigned int val,
    const char *pInvalidText)
{
    int i = 0;

    while (table[i].text != NULL) {
        if (table[i].val == val) {
            *pString = (char*)table[i].text;
            return TRUE;
        }

        i++;
    }
    *pString = (char*)pInvalidText;
    return FALSE;
} /* LutGetText() */


/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Print_FaultMask
**
** PURPOSE:    Print line fault mask
**                      
**
** PARAMETERS: 
**             fltMask       - fault mask
**             table          - look up table
**             pDes          - 
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_FaultMask(
    const int16 fltMask,
    const LutType table[],
    const char *pDes)
{
    char *pLutText;
    int mask;
    int i = 0;
    int j = 0;
    bool errFlg = FALSE;
    char buffer[250];
    char name[20];

    if (pDes == NULL) {
        sprintf(name, "fltMask");
    } else {
        sprintf(name, "%s", pDes);
    }

    /* For each bit in the fault mask record the name of the bit */
    for (i = 0; i < 16; i++) {
        mask = (1 << i);
        if (mask & fltMask) {
            bool found = Snd_Ms_LineTest_GetText(table, &pLutText, mask, NULL);
            if (errFlg) {
                j += sprintf(buffer + j, ",");
            }
            if (!found) {
                j += sprintf(buffer + j, "%i", mask);
            } else {
                j += sprintf(buffer + j, "%s", pLutText);
            }
            errFlg = TRUE;

        }
    }

    /* If no bits are found report Pass */
    if (FALSE == errFlg) {
        printk("%s %s \n", name, "LT_TEST_PASSED");
    } else {
        printk("%s %s \n", name, buffer);
    }
    return;
}


/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Print_MeasStatus
**
** PURPOSE:    Print line meaurement status 
**                      
**
** PARAMETERS: 
**             measStatus       - LT Measurment data
**             pDes                  - 
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_MeasStatus (
    const int16 measStatus,
    const char *pDes)
{
    char *pLutText;
    int mask;
    int i = 0;
    int j = 0;
    bool errFlg = FALSE;
    char buffer[80];
    char name[20];

    if (pDes == NULL) {
        sprintf(name, "measStatus");
    } else {
        sprintf(name, "%s", pDes);
    }

    /* For each bit in the fault mask record the name of the bit */
    for (i = 0; i < 16; i++) {
        mask = (1 << i);
        if (mask & measStatus) {
            if (!Snd_Ms_LineTest_GetText(LtMeasureStatusLut, &pLutText, mask, NULL)) {
                break;
            }

            if (errFlg) {
                j += sprintf(buffer + j, ",");
            }
            j += sprintf(buffer + j, "%s", pLutText);
            errFlg = TRUE;
        }
    }

    /* If no bits are found report Pass */
    if (FALSE == errFlg) {
        printk("%s %s \n", name, "LT_MSRMNT_STATUS_PASSED");
    } else {
        printk("%s %s \n", name, buffer);
    }
    return;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Print_RingerType
**
** PURPOSE:    Print line ringer type
**                      
**
** PARAMETERS: 
**             pName       - LT Ringer string
**             value          - LT Ringer value
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_RingerType(
    char* pName,
    LtRingerTestType value)
{
    char *pLutText;

    Snd_Ms_LineTest_GetText(LtRingerLut, &pLutText, value, NULL);
    printk("%s %s \n", pName, pLutText);

    return;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Print_Impedance
**
** PURPOSE:    Print line test impedance
**                      
**
** PARAMETERS: 
**             pName       - LT Impedance string
**             value          - LT impledance value
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_Impedance(
    char* pName,
    LtImpedanceType value)
{
    char *pLutText;

    if (Snd_Ms_LineTest_GetText(LtImpedanceLut, &pLutText, value, NULL)) {
        printk("%s %s \n", pName, pLutText);
    } else {
        printk("%s %d Ohms \n", pName, value/10);
    }
    return;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Print_Capacitance
**
** PURPOSE:    Print line test capacitance
**                      
**
** PARAMETERS: 
**             pName       - LT Capacitance string
**             value          - LT Capacitance value
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_Capacitance(
    char* pName,
    LtImpedanceType value)
{
    char *pLutText;

    if (Snd_Ms_LineTest_GetText(LtCapMaskLut, &pLutText, value, NULL)) {
        printk("%s %s \n", pName, pLutText);
    } else {
        printk("%s %d pF \n", pName, value);
    }
    return;
}


/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Update_Results
**
** PURPOSE:    Print Line test results 
**                      
**
** PARAMETERS: 
**             testId       - Test ID type
**             pResults   - Array having test results
**             result       - Int array to store output
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Print_Results(
        LtTestIdType testId,    
        LtTestResultType *pResults,    
        LtTestStatusType ltStatus) {

    switch (testId) {

        case LT_TID_RINGERS:
            printk("Ringer Equivalence Number Test \n");
            Snd_Ms_LineTest_Print_FaultMask(pResults->result.ringers.fltMask, LtRenMaskLut, NULL);
            Snd_Ms_LineTest_Print_MeasStatus(pResults->result.ringers.measStatus, NULL);
            Snd_Ms_LineTest_Print_Ren("ren", pResults->result.ringers.ren);
            Snd_Ms_LineTest_Print_Ren("rentg", pResults->result.ringers.rentg);
            Snd_Ms_LineTest_Print_Ren("renrg", pResults->result.ringers.renrg);
            Snd_Ms_LineTest_Print_RingerType("ringerTestType", pResults->result.ringers.ringerTestType);
            break;

        case LT_TID_LINE_V:
            printk("Line Voltage Test \n");
            Snd_Ms_LineTest_Print_FaultMask(pResults->result.lineV.fltMask, LtLineVMaskLut, NULL);
            Snd_Ms_LineTest_Print_MeasStatus(pResults->result.lineV.measStatus, NULL);
            Snd_Ms_LineTest_Print_Voltage("vAcTip", pResults->result.lineV.vAcTip, "mVrms");
            Snd_Ms_LineTest_Print_Voltage("vAcRing", pResults->result.lineV.vAcRing, "mVrms");
            Snd_Ms_LineTest_Print_Voltage("vAcDiff", pResults->result.lineV.vAcDiff, "mVrms");
            Snd_Ms_LineTest_Print_Voltage("vDcTip", pResults->result.lineV.vDcTip, "mV");
            Snd_Ms_LineTest_Print_Voltage("vDcRing", pResults->result.lineV.vDcRing, "mV");
            Snd_Ms_LineTest_Print_Voltage("vDcDiff", pResults->result.lineV.vDcDiff, "mV");
            break;        

        case LT_TID_ROH:
            printk("Receiver Off-Hook Test \n");
            Snd_Ms_LineTest_Print_FaultMask(pResults->result.roh.fltMask, LtRohMaskLut, NULL);
            Snd_Ms_LineTest_Print_MeasStatus(pResults->result.roh.measStatus, NULL);
            Snd_Ms_LineTest_Print_Impedance("rLoop1", pResults->result.roh.rLoop1);
            Snd_Ms_LineTest_Print_Impedance("rLoop2", pResults->result.roh.rLoop2);
            break;

        case LT_TID_RES_FLT:
            printk("Resistive Faults Test\n");
            Snd_Ms_LineTest_Print_FaultMask(pResults->result.resFlt.fltMask, LtResFltMaskLut, NULL);
            Snd_Ms_LineTest_Print_MeasStatus(pResults->result.resFlt.measStatus, NULL);
            Snd_Ms_LineTest_Print_Impedance("rtg", pResults->result.resFlt.rtg);
            Snd_Ms_LineTest_Print_Impedance("rrg", pResults->result.resFlt.rrg);
            Snd_Ms_LineTest_Print_Impedance("rtr", pResults->result.resFlt.rtr);
            break;    

        case LT_TID_CAP:
            printk("Capacitance Test\n");
            Snd_Ms_LineTest_Print_FaultMask(pResults->result.cap.fltMask, LtCapMaskLut, NULL);
            Snd_Ms_LineTest_Print_MeasStatus(pResults->result.cap.measStatus, NULL);
            Snd_Ms_LineTest_Print_Capacitance("ctg", pResults->result.cap.ctg);
            Snd_Ms_LineTest_Print_Capacitance("crg", pResults->result.cap.crg);
            Snd_Ms_LineTest_Print_Capacitance("ctr", pResults->result.cap.ctr);
            break;

        default:
            break;            

        }
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_LineTest_Update_Results
**
** PURPOSE:    
**                      
**
** PARAMETERS: 
**             testId       - Test ID type
**             pResults   - Array having test results
**             result       - Int array to store output
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_LineTest_Update_Results(
        LtTestIdType testId,    
        LtTestResultType *pResults,    
        unsigned int *result) {

    switch (testId) {

        case LT_TID_RINGERS:
            result[0] = pResults->result.ringers.ren;
            result[1] = pResults->result.ringers.rentg;
            result[2] = pResults->result.ringers.renrg;
            result[3] = pResults->result.ringers.ringerTestType;
            break;

        case LT_TID_LINE_V:
            result[0] = pResults->result.lineV.vAcDiff;
            result[1] = pResults->result.lineV.vAcRing;
            result[2] = pResults->result.lineV.vAcTip;
            result[3] = pResults->result.lineV.vDcDiff;
            result[4] = pResults->result.lineV.vDcRing;
            result[5] = pResults->result.lineV.vDcTip;
            result[6] = pResults->result.lineV.fltMask;
            result[7] = pResults->result.lineV.measStatus;            
            break;        

        case LT_TID_ROH:
            result[0] = pResults->result.roh.fltMask;            
            result[1] = pResults->result.roh.measStatus;         
            result[2] = pResults->result.roh.rLoop1;         
            result[3] = pResults->result.roh.rLoop2; 
            break;

        case LT_TID_RES_FLT:
            result[0] = pResults->result.resFlt.rGnd;
            result[1] = pResults->result.resFlt.rrg;
            result[2] = pResults->result.resFlt.rtg;
            result[3] = pResults->result.resFlt.rtr;
            result[4] = pResults->result.resFlt.fltMask;
            result[5] = pResults->result.resFlt.measStatus;
            break;    

        case LT_TID_CAP:
            result[0] = pResults->result.cap.ctg;
            result[1] = pResults->result.cap.crg;
            result[2] = pResults->result.cap.ctr;
            result[3] = pResults->result.cap.fltMask;
            result[4] = pResults->result.cap.measStatus;
            break; 

        default:
            break;            

        }
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Process_Line_Test_Event
**
** PURPOSE:    
**                      
**
** PARAMETERS: 
**             pMsDevInfoType - Pointer to DevInfoTYpe
**             pEvent                - Fault Event
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
static void Snd_Ms_Process_Line_Test_Event(
        struct MsDeviceInfoType *pMsDevInfoType,
        const VpEventType *pEvent) {
    
    VpStatusType        status = VP_STATUS_SUCCESS;
    LtTestStatusType    ltStatus = LT_STATUS_RUNNING;
    VpResultsType       vpResults;
    LtEventType         ltEvent;
    LtTestTempType      *pTestTempData;

    MsLineInfoType      *pLineInfo = NULL;
    
    PRINT_MSG("Line Event: event.eventData = %d event.channelId = %d \n", pEvent->eventData, pEvent->channelId);
    
    pLineInfo = &pMsDevInfoType->lineInfoType[pEvent->channelId];
            
    /* create LT-API event struct */
    /* casting done to avoid the warning */
    ltEvent.pVpEvent = (VpEventType *)pEvent;
    
    /* get vp-api result if available */
    if (pEvent->hasResults) {

        /* casting done to avoid the warning */
        status = VpGetResults((VpEventType *)pEvent, &vpResults);
        if (status != VP_STATUS_SUCCESS) {
            PRINT_R("VpGetResult() failed: %i\n", status);
            return;
        }
        ltEvent.pResult = &vpResults;
    } else {
        ltEvent.pResult = NULL;
    }

    /* process the event */
    ltStatus = LtEventHandler(&pLineInfo->ltTestCtx, &ltEvent);
    
    switch (ltStatus) {
        case LT_STATUS_DONE:

            /* Test has completed - parse the results */
            printk("LT-API test result status: %i\n", ltStatus);

            /* we are done with the current test*/
            pLineCtxUnderTest = NULL;
            up(&pLineInfo->ltTestSem);

            Snd_Ms_LineTest_Print_Results(pLineInfo->ltTestResults.testId, 
                &pLineInfo->ltTestResults, ltStatus);

            /* Dump the test results */
            pTestTempData = &pLineInfo->ltTestTempData;

            break;
            
        case LT_STATUS_RUNNING:
        case LT_STATUS_ABORTED:
            printk("LT-API test result status: %i\n", ltStatus);
             
            /* 
                        * If the line test is running or aborting the application must 
                        * continue to call the LT-API event handler on subsequent events.
                        */
            break;

        default:
            /* All other status indications are errors */
            break;
    }
 

    return;

}
#endif

/*
************************************************************************
** FUNCTION:   Snd_Ms_Process_Fault_Event
**
** PURPOSE:    Process fault events.
**                      
**
** PARAMETERS: 
**             pMsDevInfoType - Pointer to DevInfoTYpe
**             pEvent                - Fault Event
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
void Snd_Ms_Process_Fault_Event(
    struct MsDeviceInfoType *pMsDevInfoType,
    const VpEventType *pEvent) {
    /* No events to be observed so its empty */
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Process_Signaling_Event
**
** PURPOSE:    Process signalling events. 
**                      
**
** PARAMETERS: 
**             pMsDevInfoType - Pointer to DevInfoTYpe
**             pEvent                - Fault Event
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
void Snd_Ms_Process_Signaling_Event(
    struct MsDeviceInfoType *pMsDevInfoType,
    const VpEventType *pEvent) {

    switch (pEvent->eventId) {
        case VP_LINE_EVID_HOOK_OFF: {
            PRINT_MSG("Event: VP_LINE_EVID_HOOK_OFF \n");
            break;
        }
        case VP_LINE_EVID_HOOK_ON: {
            PRINT_MSG("Event: VP_LINE_EVID_HOOK_ON \n");
            break;
        }
        default:
            PRINT_R("Unrecongnized event Id : 0x%04X event category : 0x%04X", pEvent->eventId, pEvent->eventCategory);
            break;
    }
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Process_Response_Event
**
** PURPOSE:    Process response events. 
**                      
**
** PARAMETERS: 
**             pMsDevInfoType - Pointer to DevInfoTYpe
**             pEvent                - Fault Event
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
void Snd_Ms_Process_Response_Event(
    struct MsDeviceInfoType *pMsDevInfoType,
    const VpEventType        *pEvent) {

    static bool             deviceInitialized = FALSE;
    static uint8            cmpCounter = 0;

    VpStatusType            status;
    VpOptionTimeslotType    timeslot;
    VpProfilePtrType        ringCadence = Le9662_RING_CAD_SHORT;
    MsLineInfoType          *pLineInfo = NULL;
    static uint8            localDevId = 0;
                
    switch (pEvent->eventId) {
        case VP_DEV_EVID_DEV_INIT_CMP: {
            uint8 lineNum = 0;

            /* Unmask events to watch */
            Snd_Ms_Init_Set_Masks(&pMsDevInfoType->devCtx);

            PRINT_MSG("Device %p initialization completed\n",
                        (void *)pMsDevInfoType->deviceId);

            for (; lineNum < pMsDevInfoType->numLines; lineNum++) {
                
                pLineInfo = &pMsDevInfoType->lineInfoType[lineNum];

                status = Snd_Ms_Enable_Debugging(pLineInfo);
                ASSERT_microsemi(status,"VpSetOption(VP_OPTION_ID_DEBUG_SELECT)");

                timeslot.tx = (uint8)pLineInfo->slotTx;
                timeslot.rx = (uint8)pLineInfo->slotRx;
                                
                PRINT_MSG("FXS line TX/RX timeslot = %d/%d\n", 
                            pLineInfo->slotTx, pLineInfo->slotRx);

                status = VpSetOption(&pLineInfo->lineCtx, 
                                        VP_NULL, 
                                        VP_OPTION_ID_TIMESLOT, 
                                        (void*)&timeslot);
                
                ASSERT_microsemi(status,"VpSetOption(VP_OPTION_ID_TIMESLOT)");

                status = VpSetLineState(&pLineInfo->lineCtx, 
                                        VP_LINE_STANDBY);
                ASSERT_microsemi(status,"VpSetLineState");

#ifdef CAL 
                PRINT_MSG("Starting Calibration on lineNum %i of line %p\n", lineNum, pLineInfo);
                status = VpCalLine(&pLineInfo->lineCtx);
                ASSERT_microsemi(status,"VpCalLine");
#else

                
                if(localDevId == 0) {
                    PRINT_MSG("Calling VpCal line %p\n", pLineInfo);
                    status = VpCal(&pLineInfo->lineCtx, VP_CAL_APPLY_SYSTEM_COEFF, CAL_COEFF1);
                } else {
                    PRINT_MSG("Calling VpCal line %p\n", pLineInfo);
                    status = VpCal(&pLineInfo->lineCtx, VP_CAL_APPLY_SYSTEM_COEFF, CAL_COEFF2);
                }
                localDevId++;
                ASSERT_microsemi(status,"VpCal");
#endif                

            }
            break;
        }
        case VP_EVID_CAL_CMP: {
            
            PRINT_MSG("FXS event.channelId %d lineInfoType[event.channelId].chanId %d \n",
                    pEvent->channelId,  pMsDevInfoType->lineInfoType[pEvent->channelId].chanId);

#if 0//def CAL

            //1. Calling VpCal() with calType = VP_CAL_GET_LINE_COEFF.
            //2. Call VpGetEvent() until receive VP_EVID_CAL_CMP event.
            //3. Call VpGetResults(). to get the data. 

            pLineInfo = &pMsDevInfoType->lineInfoType[pEvent->channelId];

            if (pEvent->hasResults == FALSE) {
                status = VpCal(&pLineInfo->lineCtx, VP_CAL_GET_SYSTEM_COEFF, NULL);
                ASSERT_microsemi(status,"VpCal");
            } else if (pEvent->hasResults == TRUE) {
                PRINT_MSG("Get calibration coefficients : lineCtx ptr(%p) slotRx(%d) ", &pLineInfo->lineCtx, pLineInfo->slotRx);
                VpGetResults((VpEventType *)pEvent, pLineInfo->calCoeff);
            }
#endif        

            /* change the codec mode */
            status = VpSetOption(&pMsDevInfoType->lineInfoType[pEvent->channelId].lineCtx, VP_NULL, VP_OPTION_ID_CODEC, &pMsDevInfoType->lineInfoType[pEvent->channelId].codecType);
            ASSERT_microsemi(status,"VpSetOption(VP_OPTION_ID_CODEC) ");

            PRINT_MSG("FXS %d Line codec is set to %d\n",
                        pMsDevInfoType->lineInfoType[pEvent->channelId].chanId, 
                        pMsDevInfoType->lineInfoType[pEvent->channelId].codecType);

            pMsDevInfoType->lineInfoType[pEvent->channelId].isReady = TRUE;
            
            PRINT_MSG("FXS %d Line is initialized completely \n",
                        pMsDevInfoType->lineInfoType[pEvent->channelId].chanId);

            if (VP_DEV_880_SERIES == pMsDevInfoType->VpDevType) {
                ringCadence = LE880_RING_CAD_SHORT;
            } 
            
            if ((VP_DEV_886_SERIES == pMsDevInfoType->VpDevType) || 
                (VP_DEV_887_SERIES == pMsDevInfoType->VpDevType)) {
                ringCadence = Le9662_RING_CAD_SHORT;
            }
            
            if (VP_DEV_890_SERIES == pMsDevInfoType->VpDevType) {
                ringCadence = LE890_BB_RING_CAD_SHORT;
            }

            // Load standard ring cadence
            status = VpInitRing(&pMsDevInfoType->lineInfoType[pEvent->channelId].lineCtx, 
                                ringCadence,
                                VP_PTABLE_NULL);
                
            if (++cmpCounter == pMsDevInfoType->numLines) {
                deviceInitialized = TRUE;
                PRINT_MSG("Calibration completed on all lines of this device\n\n");

#ifdef CONFIG_RTL865X_WTDOG
                plat_enable_watchdog();
                PRINT_MSG("Enable watchdog\n");

#elif defined(CONFIG_RTL_WTDOG)
                bsp_enable_watchdog();
                PRINT_MSG("Enable watchdog\n");
#endif
            }
            break;
        }
        default:
            PRINT_R("Unrecongnized event Id : 0x%04X event category : 0x%04X", pEvent->eventId, pEvent->eventCategory);
            break;
    }
        
}



/*
************************************************************************
** FUNCTION:   Snd_Ms_Process_Process_Event
**
** PURPOSE:    Process process events. 
**                      
**
** PARAMETERS: 
**             pMsDevInfoType - Pointer to DevInfoTYpe
**             pEvent                - Fault Event
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
void Snd_Ms_Process_Process_Event(
    struct MsDeviceInfoType *pMsDevInfoType,
    const VpEventType *pEvent) {

    switch (pEvent->eventId) {
       
        case VP_LINE_EVID_RING_CAD: {
#ifdef INTER_RING //interleaved ringing              
            MsLineInfoType          *pLineInfo = NULL;
            MsLineInfoType          *pOtherLineInfo = NULL;
                    
            PRINT_MSG("Event: VP_LINE_EVID_RING_CAD event.eventData = %d event.channelId = %d \n", event.eventData, event.channelId);
            
            pLineInfo = &pMsDevInfoType->lineInfoType[pEvent->channelId];
            pOtherLineInfo = Snd_Ms_Get_Other_Line(pLineInfo);
        
            /* OFF period of ringing cadence begins */
            if (pEvent->eventData == VP_RING_CAD_BREAK) {
                PRINT_MSG("VPRING_CAD_BREAK \n");
    
                /* BREAK event on channel 1/0 and set line 0/1 to ringing */
                if (pLineInfo->expectedEvent == pEvent->eventData) {
                    //set channel 0 to ringing
                    status = VpSetLineState(&pOtherLineInfo->lineCtx, VP_LINE_RINGING);
                    PRINT_MSG("VP_LINE_RINGING (%d: INTERLEAVED RINGING)\n", pOtherLineInfo->chanId);                  
                    pLineInfo->expectedEvent = 0xFFFF;
                }
            }
    
            /* ON period of ringing cadence begins */
            else if (pEvent->eventData == VP_RING_CAD_MAKE) {
                PRINT_MSG("VP_RING_CAD_MAKE \n ");
            }
#endif
        }
            break;
        default:
            PRINT_R("Unrecongnized event Id : 0x%04X event category : 0x%04X", pEvent->eventId, pEvent->eventCategory);
            break;
    }
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Process_Fxo_Event
**
** PURPOSE:    Process FXO events. 
**                      
**
** PARAMETERS: 
**             pMsDevInfoType - Pointer to DevInfoTYpe
**             pEvent                - Fault Event
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
void Snd_Ms_Process_Fxo_Event(
    struct MsDeviceInfoType *pMsDevInfoType,
    const VpEventType *pEvent) {
    /* No events to be observed so its empty  */
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Process_Events
**
** PURPOSE:    
**                      
**
** PARAMETERS: 
**             pMsDevInfoType - Pointer to DevInfoTYpe
**             pEvent                - Fault Event
**
** RETURNS:    
**            n/a 
**             
**
************************************************************************
*/
void Snd_Ms_Process_Events(
    struct MsDeviceInfoType *pMsDevInfoType,
    const VpEventType *pEvent) {

    switch (pEvent->eventCategory) {
        /* sort fault events */
        case VP_EVCAT_FAULT:
            Snd_Ms_Process_Fault_Event(pMsDevInfoType, pEvent);
            break;

        /* sort signaling events */
        case VP_EVCAT_SIGNALING:
            Snd_Ms_Process_Signaling_Event(pMsDevInfoType, pEvent);
            break;

        /* sort response events */
        case VP_EVCAT_RESPONSE:
            Snd_Ms_Process_Response_Event(pMsDevInfoType, pEvent);
            break;
            
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
        /* sort test events */
        case VP_EVCAT_TEST:
            Snd_Ms_Process_Line_Test_Event(pMsDevInfoType, pEvent);
            break;
#endif

        /* sort process events */
        case VP_EVCAT_PROCESS:
            Snd_Ms_Process_Process_Event(pMsDevInfoType, pEvent);
            break;

        /* sort process events */
        case VP_EVCAT_FXO:
            Snd_Ms_Process_Fxo_Event(pMsDevInfoType, pEvent);
            break;

        default:
            PRINT_R("Unrecongnized event Id : 0x%04X event category : 0x%04X", pEvent->eventId, pEvent->eventCategory);
            break;
    }

    return;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_GetEvents_Func
**
** PURPOSE:    This function is called at a periodic rate to get specific event category and event 
**                      Id on device.
**
** PARAMETERS: 
**             work - work queue
**
** RETURNS:    
**             n/a
**              
**
************************************************************************
*/
static void Snd_Ms_GetEvents_Func(
    struct work_struct *work)
{

    bool                    deviceEventStatus = FALSE;
    VpEventType             event;
    uint32                  deviceIndex = 0;
    struct MsDeviceInfoType *pMsDevInfoType;

    /* 
     * always reschedule the tick before doing anything, this will 
     * prevent the VP-API code execution from introducing jitter. 
     */

    
    /* tick all devices */
    for (deviceIndex = 0; deviceIndex < gNoOfDevices; deviceIndex++) {
        pMsDevInfoType = &msDevInfo[deviceIndex]; 

        /*
                 * This loop will query the FXS device for events, and when an event is
                 * found (deviceEventStatus = TRUE), it will parse the event and perform
                 * further operations.
                 */
        VpApiTick(&pMsDevInfoType->devCtx, &deviceEventStatus);

        if (deviceEventStatus == FALSE) {
            /* skip this device if no event is present */
            continue;
        }
        
        while (VpGetEvent(&pMsDevInfoType->devCtx, &event)) {

            /*         
                        * A channel under test should process all line events without         
                        * passing the events over to the main event handling loop         
                        */        
             Snd_Ms_Process_Events(pMsDevInfoType, &event);
             
        }
    }

    Snd_Ms_Setup_Work();
    
    return;
}


/*
************************************************************************
** FUNCTION:   Snd_Ms_Setup_Work
**
** PURPOSE:    Create workqueue and initialize it to call at every 10 ms
**
** PARAMETERS: 
**      n/a
**
** RETURNS:    
**      n/a
**
************************************************************************
*/
static void Snd_Ms_Setup_Work(void)
{
    //create queue if not created already
    if (!pollWork) {
        /* Queue work to call APItick and getting the event */
        pollWork = (struct delayed_work *)kmalloc(sizeof(struct delayed_work), GFP_KERNEL);
    }
    
    if (pollWork) {
        INIT_DELAYED_WORK((struct delayed_work *)pollWork, Snd_Ms_GetEvents_Func);
        queue_delayed_work(msEventsWq,(struct delayed_work *)pollWork, msecs_to_jiffies(10));
    } else {
        PRINT_R("Error: can't create work queue %s\n", __FUNCTION__);
    }
    return;
}

/*
***********************************************************************
** FUNCTION:   Snd_Ms_Init_Device
**
** PURPOSE:    Initialize Microsemi SLIC device.
**
** PARAMETERS: 
**             MS_SLIC_TYPE : type of SLIC
**
** RETURNS:    
**             SUCCESS/FAIL
**
************************************************************************/
BOOL Snd_Ms_Init_Device(
    struct MsDeviceInfoType *pMsDevInfoType)
{
    int                 ch;
    VpProfilePtrType    devProfile;
    VpStatusType        status;

    PRINT_MSG("Initializing device %p VpDevType %d\n", 
                (rtl_spi_dev_t *)pMsDevInfoType->deviceId, 
                pMsDevInfoType->VpDevType);

    /* Create the API-2 device */
    status = VpMakeDeviceObject(pMsDevInfoType->VpDevType, 
                               (VpDeviceIdType) pMsDevInfoType->deviceId, 
                                &pMsDevInfoType->devCtx, 
                                &pMsDevInfoType->devObj);

    if (status != VP_STATUS_SUCCESS) {
        PRINT_R("Error: VpMakeDeviceObject (status %d) \n", status);
        return FAILED;
    }

    for (ch = 0; ch < pMsDevInfoType->numLines; ch++) {
        
        /* Create line objects */
        status = VpMakeLineObject(pMsDevInfoType->lineInfoType[ch].termType, 
                                 pMsDevInfoType->lineInfoType[ch].chanId,  
                                 &pMsDevInfoType->lineInfoType[ch].lineCtx,
                                 &pMsDevInfoType->lineInfoType[ch].lineObj,
                                 &pMsDevInfoType->devCtx);
       
        if (status != VP_STATUS_SUCCESS) {
            PRINT_R("Error: VpMakeLineObject ch %d term=%d(status %d) \n", 
               ch, pMsDevInfoType->lineInfoType[ch].termType, status);
            return FAILED;
        }

    }

    devProfile = pMsDevInfoType->pDevProfile;

    /* Avoid taking to long to init device */
#ifdef CONFIG_RTL865X_WTDOG
    plat_disable_watchdog();
    PRINT_MSG("Disable watchdog\n" );
#elif defined( CONFIG_RTL_WTDOG )
    bsp_disable_watchdog();
    PRINT_MSG("Disable watchdog\n" );
#endif  

    PRINT_MSG("calling VpInitDevice \n");

    /* Initialize the device */
    status = VpInitDevice(&pMsDevInfoType->devCtx,      
                         pMsDevInfoType->pDevProfile,
                         pMsDevInfoType->pAcProfile,
                         pMsDevInfoType->pDcProfile,
                         pMsDevInfoType->pRingProfile,
                         pMsDevInfoType->pFxoAcProfile,
                         pMsDevInfoType->pFxoDialProfile);

    if (status != VP_STATUS_SUCCESS) {
        PRINT_R("Error: VpInitDevice (status %d)\n", status);
        return FAILED;
    }

    if (status != VP_STATUS_SUCCESS) {
        return FAILED;
    }
    
    return SUCCESS;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Create_Device
**
** PURPOSE:    Create Microsemi SLIC device.
**
** PARAMETERS: 
**             MSDeviceInfoType, slicId, msSlicType
**
** RETURNS:    
**            Number of lines
**
************************************************************************/
int Snd_Ms_Create_Device(
    struct MsDeviceInfoType        *pMsDeviceInfoType,
    unsigned int                    slicId,
    MS_SLIC_TYPE                    msSlicType)
{

    MsSlicInfo msSlic = {0};

    /* assign spi structure pointer to deviceId. At time of writing deviceId will convert back to 
            rtl_spi_dev_t.  */
    pMsDeviceInfoType->deviceId     = (VpDeviceIdType) &rtlSpi[slicId]; 
    Snd_Ms_Get_Slic_Info(msSlicType, &msSlic);
    pMsDeviceInfoType->VpDevType    = msSlic.aDeviceType; 
    pMsDeviceInfoType->deviceType   = Snd_Ms_Get_RTKDevType(msSlicType); 
    
    PRINT_MSG("Creating dev 0x%08x, type=%d\n", 
            (unsigned int)slicId, pMsDeviceInfoType->deviceType);

    switch(pMsDeviceInfoType->deviceType) {
       case DEV_FXS: {
           pMsDeviceInfoType->numLines  = 1;
           pMsDeviceInfoType->lineInfoType[0].pDevInfo = pMsDeviceInfoType;
           pMsDeviceInfoType->lineInfoType[0].termType = VP_TERM_FXS_LOW_PWR;
           break;
       }        
       case DEV_FXO: {
           pMsDeviceInfoType->numLines  = 1;
           pMsDeviceInfoType->lineInfoType[0].pDevInfo = pMsDeviceInfoType;
           pMsDeviceInfoType->lineInfoType[0].termType = VP_TERM_FXO_GENERIC;
           break;
       }
       case DEV_FXSFXS: {
           pMsDeviceInfoType->numLines  = 2;
           pMsDeviceInfoType->lineInfoType[0].pDevInfo = pMsDeviceInfoType;
           pMsDeviceInfoType->lineInfoType[0].termType = VP_TERM_FXS_LOW_PWR;
           pMsDeviceInfoType->lineInfoType[1].pDevInfo = pMsDeviceInfoType;
           pMsDeviceInfoType->lineInfoType[1].termType = VP_TERM_FXS_LOW_PWR;
           break;
       }
       case DEV_FXSFXO: {
           pMsDeviceInfoType->numLines  = 2;
           pMsDeviceInfoType->lineInfoType[0].pDevInfo = pMsDeviceInfoType;
           pMsDeviceInfoType->lineInfoType[0].termType = VP_TERM_FXS_LOW_PWR;
           pMsDeviceInfoType->lineInfoType[1].pDevInfo = pMsDeviceInfoType;
           pMsDeviceInfoType->lineInfoType[1].termType = VP_TERM_FXO_GENERIC;
           break;
       }       
       default:
           pMsDeviceInfoType->numLines  = 0;
           PRINT_R("Error: Unknown line type.\n");
           return pMsDeviceInfoType->numLines;
           break;
   }


    if (VP_DEV_886_SERIES == pMsDeviceInfoType->VpDevType || 
        VP_DEV_887_SERIES == pMsDeviceInfoType->VpDevType) {
        #if defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9662)    
            pMsDeviceInfoType->pDevProfile       = DEF_LE886_9662_DEV_PROFILE;
            pMsDeviceInfoType->pAcProfile        = DEF_LE886_9662_AC_PROFILE;
            pMsDeviceInfoType->pDcProfile        = DEF_LE886_9662_DC_PROFILE;
            pMsDeviceInfoType->pRingProfile      = DEF_LE886_9662_RING_PROFILE;
            pMsDeviceInfoType->pFxoAcProfile   = DEF_LE886_9662_AC_FXO_LC_PROFILE;
            pMsDeviceInfoType->pFxoDialProfile   = DEF_LE886_9662_FXO_DIALING_PROFILE;
            pMsDeviceInfoType->pRingCadProfile  = DEF_LE886_9662_RING_CAD_PROFILE;
        #elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9661)
            pMsDeviceInfoType->pDevProfile       = DEF_LE886_9661_DEV_PROFILE;
            pMsDeviceInfoType->pAcProfile        = DEF_LE886_9661_AC_PROFILE;
            pMsDeviceInfoType->pDcProfile        = DEF_LE886_9661_DC_PROFILE;
            pMsDeviceInfoType->pRingProfile      = DEF_LE886_9661_RING_PROFILE;
            pMsDeviceInfoType->pFxoAcProfile   = DEF_LE886_9661_AC_FXO_LC_PROFILE;
            pMsDeviceInfoType->pFxoDialProfile   = DEF_LE886_9661_FXO_DIALING_PROFILE;
            pMsDeviceInfoType->pRingCadProfile  = DEF_LE886_9661_RING_CAD_PROFILE;
        #elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE88601) 
            pMsDeviceInfoType->pDevProfile       = DEF_LE886_88601_DEV_PROFILE;
            pMsDeviceInfoType->pAcProfile        = DEF_LE886_88601_AC_PROFILE;
            pMsDeviceInfoType->pDcProfile        = DEF_LE886_88601_DC_PROFILE;
            pMsDeviceInfoType->pRingProfile      = DEF_LE886_88601_RING_PROFILE;
            pMsDeviceInfoType->pFxoAcProfile   = DEF_LE886_88601_AC_FXO_LC_PROFILE;
            pMsDeviceInfoType->pFxoDialProfile   = DEF_LE886_88601_FXO_DIALING_PROFILE;
            pMsDeviceInfoType->pRingCadProfile  = DEF_LE886_88601_RING_CAD_PROFILE;
        #elif defined (CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE9641) 
            pMsDeviceInfoType->pDevProfile       = DEF_LE886_9641_DEV_PROFILE;
            pMsDeviceInfoType->pAcProfile        = DEF_LE886_9641_AC_PROFILE;
            pMsDeviceInfoType->pDcProfile        = DEF_LE886_9641_DC_PROFILE;
            pMsDeviceInfoType->pRingProfile      = DEF_LE886_9641_RING_PROFILE;
            pMsDeviceInfoType->pFxoAcProfile   = DEF_LE886_9641_AC_FXO_LC_PROFILE;
            pMsDeviceInfoType->pFxoDialProfile   = DEF_LE886_9641_FXO_DIALING_PROFILE;
            pMsDeviceInfoType->pRingCadProfile  = DEF_LE886_9641_RING_CAD_PROFILE;            
        #endif
        
        PRINT_MSG("setting the profile for device %d \n", 
                pMsDeviceInfoType->VpDevType);
    } else if (VP_DEV_890_SERIES == pMsDeviceInfoType->VpDevType)  {
        pMsDeviceInfoType->pDevProfile       = DEF_LE890_LE89156_DEV_PROFILE_LV;
        pMsDeviceInfoType->pAcProfile        = DEF_LE890_LE89156_AC_PROFILE;
        pMsDeviceInfoType->pDcProfile        = DEF_LE890_LE89156_DC_PROFILE;
        pMsDeviceInfoType->pRingProfile      = DEF_LE890_LE89156_RING_PROFILE;
        pMsDeviceInfoType->pRingCadProfile  = DEF_LE890_LE89156_RING_CAD_PROFILE;

        PRINT_MSG("setting the profile for device %d \n", 
               pMsDeviceInfoType->VpDevType);
    } else if (VP_DEV_880_SERIES == pMsDeviceInfoType->VpDevType) {
       pMsDeviceInfoType->pDevProfile       = DEF_LE880_DEV_PROFILE;
       pMsDeviceInfoType->pAcProfile        = DEF_LE880_AC_PROFILE;
       pMsDeviceInfoType->pDcProfile        = DEF_LE880_DC_PROFILE;
       pMsDeviceInfoType->pRingProfile      = DEF_LE880_RING_PROFILE;
       pMsDeviceInfoType->pFxoAcProfile   = DEF_LE880_AC_FXO_LC_PROFILE;
       pMsDeviceInfoType->pFxoDialProfile   = DEF_LE880_FXO_DIALING_PROFILE;
       pMsDeviceInfoType->pRingCadProfile  = DEF_LE880_RING_CAD_PROFILE;

       PRINT_MSG("setting the profile for device %d \n", 
               pMsDeviceInfoType->VpDevType);
    }

    if(msSlic.slicName) {
        kfree(msSlic.slicName);
    }

    return pMsDeviceInfoType->numLines;
}

/*
************************************************************************
** FUNCTION:   Snd_Ms_Create_Line
**
** PURPOSE:    Create line of a SLIC device.
**
** PARAMETERS: 
**             MSDeviceInfoType, RTKDevType, unsigned int, MS_SLIC_TYPE
**
** RETURNS:    
**            SUCCESS/FAILED
**
************************************************************************/
BOOL Snd_Ms_Create_Line(
    MsLineInfoType  *pMsLineInfoType,
    unsigned int    ch_id,
    int             law,
    unsigned int    slot)
{
    pMsLineInfoType->isReady = FALSE;
    pMsLineInfoType->chanId = ch_id;                    /* line# within a slic. usually 0 or 1 */
    pMsLineInfoType->codecType = Snd_Ms_Get_Codec_Type(law);
    printk(" ======== pMsLineInfoType->codecType %d law %d \n", pMsLineInfoType->codecType, law);
    pMsLineInfoType->txRelGain = 0;                     /* use default gain from AC profile. */
    pMsLineInfoType->rxRelGain = 0;                     /*use default gain from AC profile. */
    pMsLineInfoType->slotTx = slot;
    pMsLineInfoType->slotRx = slot;
    pMsLineInfoType->expectedEvent = 0xFFFF;

    
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LINETEST)
    sema_init(&pMsLineInfoType->ltTestSem, 0);
#endif  
   
    PRINT_MSG("Creating line 0x%08x \n", ch_id);
    return TRUE;
}

/*
***********************************************************************
** FUNCTION:   Snd_Ms_Slic_Init
**
** PURPOSE:    
**
** PARAMETERS: None
**             
**
** RETURNS:    
**             
**
************************************************************************/
static int Snd_Ms_Slic_Init(
    int                pcmMode, 
    int                notused)
{
    voip_snd_t                  *pMsSnd = NULL;
    struct MsDeviceInfoType     *pMsDevInfoType = NULL;
    rtl_spi_dev_t               *pSpiDev = NULL;
    MS_SLIC_TYPE                msSlicType = MS_SLIC_TYPE_NONE;
    int                         i;
    int                         rtn;
    unsigned int                slics = 0, pincs = 0;
    int                         slicId = 0;
    
#ifndef CONFIG_RTK_VOIP_8676_SHARED_SPI      
        extern void slicInternalCS(int cs);
#endif

    // setup pcm fs, slic reset
#ifdef CONFIG_RTK_VOIP_PLATFORM_8686
    extern void aipc_module_voip_set_pcm_fs(void);
    extern void aipc_module_voip_slic_reset(void);
    extern unsigned int soc_set_pcm_fs;

    if (soc_set_pcm_fs == 0) {
        aipc_module_voip_set_pcm_fs();
#ifdef CONFIG_RTL8686_SLIC_RESET 
        aipc_module_voip_slic_reset();
#endif
        soc_set_pcm_fs = 1;
    }
#endif

    PRINT_MSG("Zarlink API-II Lite Version %d.%d.%d\n", 
                  VP_API_VERSION_MAJOR_NUM, 
                  VP_API_VERSION_MINOR_NUM, 
                  VP_API_VERSION_MINI_NUM);

    for (msSlicType = MS_SLIC_TYPE_NONE+1; 
            msSlicType < MS_SLIC_TYPE_END; 
            msSlicType++) {
                
        MsSlicInfo msSlic;
        
        slics = Snd_Ms_Get_Number_Of_Slics(msSlicType);
        Snd_Ms_Get_Slic_Info(msSlicType, &msSlic);
        pincs = Snd_Ms_Get_Pin_Cs(msSlicType);

        PRINT_MSG( "%s Number of slics : %d Total Slics : %d\n", msSlic.slicName, slics, gNoOfDevices);
        
        //don't print if slic is not found
        if (slics == 0) {
            continue;
        }
        
        // Iterate for each slics of similar type
        for (i = 0; i < slics && slicId<gNoOfDevices; i++, slicId++) {
            pMsSnd = &msSnd[slicId];
            pMsDevInfoType = &msDevInfo[slicId];
            pSpiDev = &rtlSpi[slicId];

            if ((msSlicType == MS_SLIC_TYPE_LE9662) || (msSlicType == MS_SLIC_TYPE_LE9641) || (msSlicType == MS_SLIC_TYPE_LE88601)) {
#ifdef CONFIG_RTK_VOIP_8676_SHARED_SPI
                init_spi_pins(pSpiDev, pincs + i, PIN_CLK, PIN_DO, PIN_DI);
#else
                _init_rtl_spi_dev_type_hw(pSpiDev, pincs);
                slicInternalCS(pincs);
#endif
            } else if (msSlicType == MS_SLIC_TYPE_LE89116 || 
                        msSlicType == MS_SLIC_TYPE_LE89316 || 
                        msSlicType == MS_SLIC_TYPE_LE89156) {
#ifdef CONFIG_RTK_VOIP_8676_SHARED_SPI    
                PRINT_MSG( "%s[%d] CS=%08X\n", msSlic.slicName, i, pincs - 1 + i);
                init_spi_pins(pSpiDev, pincs -1 + i, PIN_CLK, PIN_DO, PIN_DI);
#else

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116_PIN_CS_USE_SW) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156_PIN_CS_USE_SW)
                PRINT_MSG( "%s[%d] CS=%08X\n", msSlic.slicName, i, pincs + i);
                init_spi_pins(pSpiDev, pin_cs_le89116[i], PIN_CLK, PIN_DO, PIN_DI);

#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116_PIN_CS_USE_HW) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156_PIN_CS_USE_HW)
                PRINT_MSG( "%s[%d] CS=%08X\n", msSlic.slicName, i, pincs + i);
                _init_rtl_spi_dev_type_hw(pSpiDev, pincs + i);

#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89116_PIN_CS_USE_HW3) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_MICROSEMI_LE89156_PIN_CS_USE_HW3)
                PRINT_MSG( "%s[%d] CS=%08X\n", msSlic.slicName, i, pincs);
                _init_rtl_spi_dev_type_hw3(pSpiDev, 
                                            pincs/* CS special mode use the same CS pin*/,
                                            PIN_CSEN, i);
#endif

#endif
            }
            
            PRINT_MSG( " Create Dev & Line Object for slics [%d] slicid[%d] === \n", slics, slicId);
            Snd_Ms_Create_Device(pMsDevInfoType, slicId, msSlicType);
            pMsDevInfoType->pVoipSnd = pMsSnd;
            
            /* FXS */
            Snd_Ms_Create_Line(&pMsDevInfoType->lineInfoType[0],
                                0,
                                pcmMode,
                                pMsSnd->TS1);

            if (pMsDevInfoType->numLines == 2) {
                Snd_Ms_Create_Line(&pMsDevInfoType->lineInfoType[1],
                                    1,
                                    pcmMode,
                                    (pMsSnd+1)->TS1);
            }

            /* Initialize API-2 device settings */

            rtn = Snd_Ms_Init_Device(pMsDevInfoType);
            if (rtn == FAILED) {
                PRINT_R("Error (%d:x) %s\n", i, __FUNCTION__);
                return FAILED;
            } 
        }

        if (msSlic.slicName) {
            kfree(msSlic.slicName);
        }
    }

    /* Initialize work queue for API tick */
    msEventsWq = create_workqueue("ms_events_queue");
    Snd_Ms_Setup_Work();

    return SUCCESS;
}


  /*
 ***********************************************************************
 ** FUNCTION:   Fill_Register_Info
 **
 ** PURPOSE:    Registers all Microsemi SLIC devices with the Realtek VOIP Stack.
 **
 ** PARAMETERS: None
 **             
 **
 ** RETURNS:    
 **             
 **
 ***********************************************************************/
 static void __init Fill_Register_Info( 
    voip_snd_t         aMsSnd[],
    int                noOfFxs, 
    int                noOfFxo, 
    unsigned short     tsBase,
    MsLineInfoType     msLineInfoType[],
    MS_SLIC_TYPE       msSlicType)
{
    int        index;
    int        daa = 0;
    MsSlicInfo msSlic = {0};

    for (index = 0; index < noOfFxs + noOfFxo; index ++) {
        if (index == noOfFxs) {
            daa = 1;
        }
        aMsSnd[index].sch = index;
        Snd_Ms_Get_Slic_Info(msSlicType, &msSlic);
        aMsSnd[index].name = msSlic.slicName;
        aMsSnd[index].snd_type = SND_TYPE_FXS;
        aMsSnd[index].bus_type_sup = BUS_TYPE_PCM;
        aMsSnd[index].TS1 = tsBase + index * 2;

#ifdef CONFIG_RTK_VOIP_WIDEBAND_SUPPORT
        aMsSnd[index].TS2 = 
        (daa || tsBase + index * 2 >= 16 ? 0 : tsBase + (index + 8) * 2);
        aMsSnd[index].band_mode_sup = 
        (daa ? BAND_MODE_8K : BAND_MODE_8K | BAND_MODE_16K);
#else
        aMsSnd[index].TS2 = 0;
        aMsSnd[index].band_mode_sup = BAND_MODE_8K;
#endif

        /* assign globally declared fxs_ops (same for all microsemi SLICs) */
        aMsSnd[index].snd_ops = (const snd_ops_t *)&ms_snd_fxs_ops;

        /* store reference of line type */
        aMsSnd[index].priv = &msLineInfoType[index]; 

        /* DAA port */
        if (daa) {
            aMsSnd[index].snd_type = SND_TYPE_DAA;
            aMsSnd[index].snd_ops = (const snd_ops_t *)&ms_snd_daa_ops;
        }       

        if (msSlic.slicName) {
            kfree(msSlic.slicName);
        }
     }
 }

 /*
************************************************************************
** FUNCTION:   Voip_Snd_Microsemi_Init
**
** PURPOSE:    Registers all Microsemi SLIC devices with the Realtek VOIP Stack.
**
** PARAMETERS: None
**             
**
** RETURNS:    
**             
**
************************************************************************/
static int  __init Voip_Snd_Microsemi_Init(void) 
{
    /* declare local variables */
    int                     tsBase, tsCur; //time slot
    unsigned int            slics;
    MS_SLIC_TYPE            msSlicType = MS_SLIC_TYPE_NONE;
    int                     i = 0;
    char*                   slicName;
    int                     rtn;
    extern int              law;
    
    /* This variable keeps track of slicId, its value changes with number of slics. */
    unsigned int            slicId = 0;     /* changes with number of slics.  */
    unsigned int            noOfFxs = 0;
    unsigned int            noOfFxo = 0;
    MsSlicInfo              msSlic = {0};
    unsigned int            soundId = 0;    /* changes with number of fxs in each slic. */
    
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
    static defer_init_t     defer_ms_init;
#endif  

    /* get free timeslot */
    tsBase = get_snd_free_timeslot();

    if (tsBase < 0) {    
        tsBase = 0;        
    }

    tsCur = tsBase;

    /* For each Microsemi SLIC type, determine total number of SLICs connected
     * suppose there are two different types of SLICs connected then for-loop will be running
     * two times.
     */
    
    for (msSlicType = MS_SLIC_TYPE_NONE + 1; 
         msSlicType < MS_SLIC_TYPE_END; 
         msSlicType++) {
            
        slics = Snd_Ms_Get_Number_Of_Slics(msSlicType);
        Snd_Ms_Get_Slic_Info(msSlicType, &msSlic);
        slicName = msSlic.slicName;
        noOfFxs = msSlic.noOfFxs;
        noOfFxo = msSlic.noOfFxo;
        
        /* don't print if slic is not found */
        if (slics) {
            PRINT_MSG("%s Slics are %d\n", slicName, slics);
        }
        
        /*  Iterate for each slics of similar type */
        for (i = 0; i < slics; i++) {
            PRINT_MSG("%s CS %d slicid %d\n", 
                        slicName, Snd_Ms_Get_Pin_Cs(msSlicType),slicId);
            
            Fill_Register_Info(&msSnd[soundId], 
                                noOfFxs /* fxs */, 
                                noOfFxo /* daa */, 
                                (unsigned short)tsCur,
                                &msDevInfo[slicId].lineInfoType[0],
                                msSlicType);

#ifdef CONFIG_RTK_VOIP_DRIVERS_MICROSEMI_OWNER_ALL_DSP

            if (fill_ipc_dsp_cpuid(&msSnd[soundId], 
                    noOfFxs /* fxs */, 
                    noOfFxo /* daa */, 
                    i, 
                    SND_PER_DSPCPU(slics, 
                                   CONFIG_RTK_VOIP_DSP_DEVICE_NR))) {
                tsCur = tsBase;
            } else
#endif
            {
                tsCur += 2;
            }

            msDevInfo[slicId].msSlicType = msSlicType;
            
            /* SLIC init use ops */
#ifdef CONFIG_RTK_VOIP_DEFER_SNDDEV_INIT
            defer_ms_init.fn_defer_func = (fn_defer_func_t) Snd_Ms_Slic_Init;
            /* At present the format is hard-coded, as the selection of the format happens in 
                        * Bus_pcm_law.c file. After modifying there set format to wideband or narrowband
                        * based on selection in kernel config.
                           defer_ms_init.p0 = law;
                        */
            defer_ms_init.p0 = law;
            defer_ms_init.p1 = slicId;

            add_defer_initialization(&defer_ms_init);
            rtn = 0; //success
#else
            rtn = Snd_Ms_Slic_Init(law, slicId);
#endif

            /* SLIC Init failed */
            if (rtn == FAILED) {
                return -1;
            }
            
            register_voip_snd(&msSnd[soundId], noOfFxs);  

            slicId++;
            soundId += noOfFxs;
            
        }
        
        if (msSlic.slicName) {
            kfree(msSlic.slicName);
        }
    }

    gNoOfDevices = slicId;

    return 0;
}
 
 /*
***********************************************************************
** FUNCTION:   Voip_Ioc_Microsemi_Init
**
** PURPOSE:    Registers all Microsemi SLIC devices with the Realtek VOIP Stack.
**
** PARAMETERS: None
**             
**
** RETURNS:    
**             
**
************************************************************************/
static int __init Voip_Ioc_Microsemi_Init(void) 
{
    return 0;
}

#define VDUMPREGNAME1 "msdumpreg"

 /*
***********************************************************************
** FUNCTION:   Voip_Dumpreg_Read
**
** PURPOSE:    
**
** PARAMETERS: None
**             
**
** RETURNS:    
**             
**
************************************************************************/
static int Voip_Dumpreg_Read(
    char    *page, 
    char    **start, 
    off_t   off,
    int     count, 
    int     *eof, 
    void    *data)
{
    return 0;
}

 /*
***********************************************************************
** FUNCTION:   Voip_Dumpreg_Write
**
** PURPOSE:    Dump SLICs found & Registers.
**
** PARAMETERS: None
**             
**
** RETURNS:    
**  n/a           
**
************************************************************************/
static int Voip_Dumpreg_Write(
    struct file     *file, 
    const char      *buffer, 
    unsigned long   count, 
    void            *data)
{
    uint8                   tmp[4];
    uint32                  t1 = 0;
    uint32                  slics;
    MS_SLIC_TYPE            msSlicType = MS_SLIC_TYPE_NONE;
    uint32                  slicId = 0;
    MsSlicInfo              msSlic = {0};

    if (count < 2) {
        return -EFAULT;
    }
    
    if (count > 4) {
        return -EFAULT;
    }
    
    if (buffer && !copy_from_user(tmp, buffer, count)) {
        sscanf(tmp, "%x", &t1);

        for (slicId = 0; slicId < gNoOfDevices; slicId++) {
            msSlicType = msDevInfo[slicId].msSlicType;
            
            slics = Snd_Ms_Get_Number_Of_Slics(msSlicType);
            Snd_Ms_Get_Slic_Info(msSlicType, &msSlic);
            
            PRINT_MSG("\n ==== %s ==== \n", msSlic.slicName);
            /* Dump All the register values */
            if (t1 == 0) {
                struct MsDeviceInfoType    *pDevInfo  = &msDevInfo[slicId];
                VpDevCtxType                *pDevCtx   = &pDevInfo->devCtx;    

                VpRegisterDump(pDevCtx);
                
            } else if (t1 == 1) { 
                /* Dump SLIC RCN, PCN */
                uint8 result[2] = {0xFF};
                VpMpiCmd(&rtlSpi[slicId], VP886_R_EC_EC1, VP886_R_RCNPCN_RD, VP886_R_RCNPCN_LEN, result);
                PRINT_MSG(" RCN: 0x%x PCN: 0x%x \n", result[0], result[1]);
                
            } else if (t1 == 2){ 
                /* Dump Cal Coefficients */
                uint32 coeffIndex = 0, maxCoeffIndex;
                uint8 ch;

                struct MsDeviceInfoType    *pDevInfo  = &msDevInfo[slicId];
                    
                PRINT_MSG(" Dumping coeff : %p\n", &pDevInfo->lineInfoType[0].lineCtx);

                maxCoeffIndex = pDevInfo->lineInfoType[0].calCoeff[3] + 4;

                while (coeffIndex < (maxCoeffIndex)) {            
                    ch = pDevInfo->lineInfoType[0].calCoeff[coeffIndex];
                    PRINT_MSG("0x%x ", ch);
                    coeffIndex++;
                }        
                PRINT_MSG(" \n ");
                
            } else {
                PRINT_MSG(" \n HELP: Write 0 to dump all registers, 1 to get PCN RCN, 2 to get calibration coefficients \n");
                
            }

            if (msSlic.slicName) {
                kfree(msSlic.slicName);
            }
        }
    }

    return count;
}

 /*
***********************************************************************
** FUNCTION:   Voip_Proc_Dumpreg_Init
**
** PURPOSE:    Create /proc/msdumpreigster entry.
**
** PARAMETERS: None
**             
**
** RETURNS:    
**  n/a           
**
************************************************************************/
static int __init Voip_Proc_Dumpreg_Init(void)
{
    struct proc_dir_entry *procDir;

    procDir = create_proc_entry(VDUMPREGNAME1, 0644, NULL);

    if (procDir == NULL) {
        remove_proc_entry(VDUMPREGNAME1, NULL);
        PRINT_R(KERN_ALERT "Error: Could not initialize /proc/%s\n",
                VDUMPREGNAME1);
        return -ENOMEM;
    }

    procDir->read_proc  = (read_proc_t *)Voip_Dumpreg_Read;
    procDir->write_proc  = (write_proc_t *)Voip_Dumpreg_Write;

    return 0;
}

 /*
***********************************************************************
** FUNCTION:   Voip_Proc_Dumpreg_Exit
**
** PURPOSE:    Remove /proc/msdumpreigster entry.
**
** PARAMETERS: None
**             
**
** RETURNS:    
**  n/a           
**
************************************************************************/
static void __exit Voip_Proc_Dumpreg_Exit(void)
{
    //DEBUG_API_PRINT();
    remove_proc_entry(VDUMPREGNAME1, NULL);
}


/* Register init calls with kernel */
voip_initcall_snd(Voip_Snd_Microsemi_Init);
voip_initcall_ioc(Voip_Ioc_Microsemi_Init);
module_init(Voip_Proc_Dumpreg_Init);
module_exit(Voip_Proc_Dumpreg_Exit);

MODULE_LICENSE("GPL");
