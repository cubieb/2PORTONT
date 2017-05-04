/* obcif.h: A RealCom ADSL driver for uClinux. */
/*
	This is for the RTL8670 ADSL router.
*/

#ifndef OBCIF_H
#define OBCIF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OBCIF_ARG {
  int argsize;
  int arg;
}obcif_arg;

// ioctl command called by protocols (system-independent)
#define RLCM_IOC_MAGIC	(('R'+'L'+'C'+'M'+'a'+'d'+'s'+'l') << 8)  //0x2d200
#define RLCM_PHY_ENABLE_MODEM			(RLCM_IOC_MAGIC + 1)
#define RLCM_PHY_DISABLE_MODEM			(RLCM_IOC_MAGIC + 2)
#define RLCM_GET_DRIVER_VERSION			(RLCM_IOC_MAGIC + 3)
#define RLCM_GET_DRIVER_BUILD			(RLCM_IOC_MAGIC + 4)
#define RLCM_MODEM_RETRAIN			(RLCM_IOC_MAGIC + 5)
#define RLCM_GET_REHS_COUNT			(RLCM_IOC_MAGIC + 6)
#define RLCM_GET_CHANNEL_SNR			(RLCM_IOC_MAGIC + 7)
#define RLCM_GET_AVERAGE_SNR			(RLCM_IOC_MAGIC + 8)
#define RLCM_GET_SNR_MARGIN			(RLCM_IOC_MAGIC + 9)
#define RLCM_REPORT_MODEM_STATE			(RLCM_IOC_MAGIC +10)
#define RLCM_REPORT_PM_DATA			(RLCM_IOC_MAGIC +11)
#define RLCM_MODEM_NEAR_END_ID_REQ		(RLCM_IOC_MAGIC +12)
#define RLCM_MODEM_FAR_END_ID_REQ		(RLCM_IOC_MAGIC +13)
#define RLCM_MODEM_NEAR_END_LINE_DATA_REQ	(RLCM_IOC_MAGIC +14)
#define RLCM_MODEM_FAR_END_LINE_DATA_REQ	(RLCM_IOC_MAGIC +15)
#define RLCM_MODEM_NEAR_END_FAST_CH_DATA_REQ	(RLCM_IOC_MAGIC +16)
#define RLCM_MODEM_NEAR_END_INT_CH_DATA_REQ	(RLCM_IOC_MAGIC +17)
#define RLCM_MODEM_FAR_END_FAST_CH_DATA_REQ	(RLCM_IOC_MAGIC +18)
#define RLCM_MODEM_FAR_END_INT_CH_DATA_REQ	(RLCM_IOC_MAGIC +19)
#define RLCM_SET_ADSL_MODE			(RLCM_IOC_MAGIC +20)
#define RLCM_GET_ADSL_MODE			(RLCM_IOC_MAGIC +21)
#define RLCM_GET_LOSS_DATA			(RLCM_IOC_MAGIC +22)
#define RLCM_GET_LINK_SPEED			(RLCM_IOC_MAGIC +23)
#define RLCM_GET_CHANNEL_MODE			(RLCM_IOC_MAGIC +24)
#define RLCM_GET_LOOP_ATT			(RLCM_IOC_MAGIC +25)
#define RLCM_INC_TX_POWER			(RLCM_IOC_MAGIC +26)
#define RLCM_TUNE_PERF				(RLCM_IOC_MAGIC +27)
#define RLCM_ENABLE_BIT_SWAP			(RLCM_IOC_MAGIC +28)
#define RLCM_DISABLE_BIT_SWAP			(RLCM_IOC_MAGIC +29)
#define RLCM_ENABLE_PILOT_RELOCATION		(RLCM_IOC_MAGIC +30)
#define RLCM_DISABLE_PILOT_RELOCATION		(RLCM_IOC_MAGIC +31)
#define RLCM_ENABLE_TRELLIS			(RLCM_IOC_MAGIC +32)
#define RLCM_DISABLE_TRELLIS			(RLCM_IOC_MAGIC +33)
#define RLCM_SET_VENDOR_ID			(RLCM_IOC_MAGIC +34)
#define RLCM_MODEM_READ_CONFIG                  (RLCM_IOC_MAGIC +35)
#define RLCM_MODEM_WRITE_CONFIG                 (RLCM_IOC_MAGIC +36)
#define RLCM_DEBUG_MODE				(RLCM_IOC_MAGIC +37)
#define RLCM_TEST_PSD				(RLCM_IOC_MAGIC +38)
#define RLCM_GET_ADSL_TIME			(RLCM_IOC_MAGIC +39)
#define RLCM_PHY_START_MODEM			(RLCM_IOC_MAGIC +40)
#define RLCM_ENABLE_ADSL_LOG			(RLCM_IOC_MAGIC +41)
#define RLCM_DISABLE_ADSL_LOG			(RLCM_IOC_MAGIC +42)
//added command
#define RLCM_GET_VENDOR_ID			(RLCM_IOC_MAGIC +43)
#define RLCM_GET_TX_POWER			(RLCM_IOC_MAGIC +44)
#define RLCM_GET_PERF_VALUE			(RLCM_IOC_MAGIC +45)
#define RLCM_GET_15MIN_LOSS_DATA		(RLCM_IOC_MAGIC +46)
#define RLCM_GET_1DAY_LOSS_DATA			(RLCM_IOC_MAGIC +47)
#define RLCM_GET_CHANNEL_BITLOAD		(RLCM_IOC_MAGIC +48)
//for MIB TRAP set
#define RLCM_GET_TRAP_THRESHOLD			(RLCM_IOC_MAGIC +49)
#define RLCM_SET_TRAP_THRESHOLD			(RLCM_IOC_MAGIC +50)
#define RLCM_15MIN_WAIT_TRAP			(RLCM_IOC_MAGIC +51)
//for ATM test
#define RLCM_ENABLE_ATM_LOOPBACK		(RLCM_IOC_MAGIC +52)
#define RLCM_DISABLE_ATM_LOOPBACK		(RLCM_IOC_MAGIC +53)
#define RLCM_MSGMODE				(RLCM_IOC_MAGIC +54)
#define RLCM_CMD_API				(RLCM_IOC_MAGIC +55)
#define RLCM_GET_CURRENT_LOSS_DATA		(RLCM_IOC_MAGIC +56)
#define RLCM_GET_CHANNEL_BH			(RLCM_IOC_MAGIC +57)
#define RLCM_GET_TRAP_15MIN_LOSS_DATA		(RLCM_IOC_MAGIC +58)
#define RLCM_SEND_DYING_GASP			(RLCM_IOC_MAGIC +59)
#define RLCM_TEST_HW				(RLCM_IOC_MAGIC +62)
#define RLCM_SET_LOOPBACK			(RLCM_IOC_MAGIC +63)
#define RLCM_INIT_ADSL_MODE			(RLCM_IOC_MAGIC +64)
#define RLCM_ENABLE_POM_ACCESS			(RLCM_IOC_MAGIC +65)
#define RLCM_DISABLE_POM_ACCESS			(RLCM_IOC_MAGIC +66)
#define RLCM_MASK_TONE				(RLCM_IOC_MAGIC +67)
#define RLCM_GET_CAPABILITY			(RLCM_IOC_MAGIC +68)
#define RLCM_VERIFY_HW				(RLCM_IOC_MAGIC +69)
#define RLCM_TRIG_OLR_TYPE			(RLCM_IOC_MAGIC +70)
#define RLCM_TRIG_OLR_TYPE1			(RLCM_IOC_MAGIC +71)
#define RLCM_ENABLE_AEQ				(RLCM_IOC_MAGIC +72)
#define RLCM_ENABLE_HPF				(RLCM_IOC_MAGIC +73)
#define RLCM_SET_HPF_FC				(RLCM_IOC_MAGIC +74)

// new_hibrid
#define RLCM_SET_HYBRID				(RLCM_IOC_MAGIC +75)
#define RLCM_SET_RX_GAIN			(RLCM_IOC_MAGIC +76)
#define RLCM_SET_AFE_REG			(RLCM_IOC_MAGIC +77)
#define RLCM_SET_FOBASE				(RLCM_IOC_MAGIC +78)

//yaru + for webdata
#define RLCM_SET_XDSL_MODE			(RLCM_IOC_MAGIC +79)
#define RLCM_GET_SHOWTIME_XDSL_MODE		(RLCM_IOC_MAGIC +80)
#define RLCM_GET_XDSL_MODE			(RLCM_IOC_MAGIC +81)
#define RLCM_SET_OLR_TYPE			(RLCM_IOC_MAGIC +82)
#define RLCM_GET_OLR_TYPE 			(RLCM_IOC_MAGIC +83)
#define RLCM_GET_LINE_RATE			(RLCM_IOC_MAGIC +84)
#define RLCM_GET_DS_ERROR_COUNT			(RLCM_IOC_MAGIC +85)
#define RLCM_GET_US_ERROR_COUNT			(RLCM_IOC_MAGIC +86)
#define RLCM_GET_DIAG_QLN			(RLCM_IOC_MAGIC +87)
#define RLCM_GET_DIAG_HLOG			(RLCM_IOC_MAGIC +88)
#define RLCM_GET_DIAG_SNR			(RLCM_IOC_MAGIC +89)
#define RLCM_GET_DS_PMS_PARAM1			(RLCM_IOC_MAGIC +90)
#define RLCM_GET_US_PMS_PARAM1			(RLCM_IOC_MAGIC +91)
#define RLCM_SET_ANNEX_L			(RLCM_IOC_MAGIC +92)
#define RLCM_GET_ANNEX_L			(RLCM_IOC_MAGIC +93)
#define RLCM_GET_LINK_POWER_STATE		(RLCM_IOC_MAGIC +94)
#define RLCM_GET_ATT_RATE			(RLCM_IOC_MAGIC +95)
#define RLCM_LOADCARRIERMASK			(RLCM_IOC_MAGIC +96)
#define RLCM_SET_ANNEX_M			(RLCM_IOC_MAGIC +97)
#define RLCM_GET_ANNEX_M			(RLCM_IOC_MAGIC +98)
#define RLCM_SET_8671_REV			(RLCM_IOC_MAGIC +99)
#define RLCM_GET_8671_REV			(RLCM_IOC_MAGIC +100)
#define RLCM_SET_HIGH_INP			(RLCM_IOC_MAGIC +101)
#define RLCM_GET_HIGH_INP			(RLCM_IOC_MAGIC +102)
#define RLCM_GET_LD_STATE			(RLCM_IOC_MAGIC +103)
#define RLCM_SET_ANNEX_B			(RLCM_IOC_MAGIC +104)
#define RLCM_GET_ANNEX_B			(RLCM_IOC_MAGIC +105)
//for TR069
#define RLCM_GET_DSL_STAT_SHOWTIME		(RLCM_IOC_MAGIC +106)
#define RLCM_GET_DSL_STAT_TOTAL			(RLCM_IOC_MAGIC +107)
#define RLCM_GET_DSL_PSD			(RLCM_IOC_MAGIC +108)
#define RLCM_GET_DSL_ORHERS			(RLCM_IOC_MAGIC +109)
#define RLCM_GET_DSL_GI				(RLCM_IOC_MAGIC +110)
// for Telefonica
#define RLCM_SET_ADSL_LAST_OPMode		(RLCM_IOC_MAGIC +111)
#define RLCM_SET_ADSL_PMS_CONFIG		(RLCM_IOC_MAGIC +112)
//Lupin, for TR069
#define RLCM_GET_ADSL2WAN_IFCFG			(RLCM_IOC_MAGIC +113)
#define RLCM_ENABLE_DIAGNOSTIC			(RLCM_IOC_MAGIC +114)



#ifdef FIELD_TRY_SAFE_MODE
#define RLCM_GET_SAFEMODE_CTRL			(RLCM_IOC_MAGIC +115)
#define RLCM_SET_SAFEMODE_CTRL			(RLCM_IOC_MAGIC +116)   

typedef struct {
	int FieldTrySafeMode;
	int FieldTryTestPSDTimes;
	int FieldTryCtrlIn;
	char SafeModeNote[20];
} SafeModeData;
#define SAFEMODE_DATA_SIZE sizeof(SafeModeData)
#endif

// Mason Yu. For CurrentDay and QuarterHour.
#define RLCM_GET_DSL_STAT_15MIN			(RLCM_IOC_MAGIC +117)
#define RLCM_GET_DSL_STAT_1DAY			(RLCM_IOC_MAGIC +118)

#define RLCM_GET_LOOP_LENGTH_METER  (RLCM_IOC_MAGIC +119)

#define RLCM_WEB_SET_USPSD			(RLCM_IOC_MAGIC +154)
 
typedef struct {
	unsigned short breakFreq_array_us[8];
	float MIB_PSD_us[8];
} UsPSDData;
#define USPSDDATA_DATA_SIZE sizeof(UsPSDData)

#define RLCM_GET_INIT_COUNT_LAST_LINK_RATE	(RLCM_IOC_MAGIC +155)
#define RLCM_ENABLE_NODROPLINEFLAG		(RLCM_IOC_MAGIC +156)
#define RLCM_DISABLE_NODROPLINEFLAG		(RLCM_IOC_MAGIC +157)

#define RLCM_SET_D_CONFIG			(RLCM_IOC_MAGIC +158)
#define RLCM_GET_DSL_STAT_LAST_SHOWTIME		(RLCM_IOC_MAGIC +159)

#define RLCM_GET_DSL_STAT_SHOWTIME_REV1P4	(RLCM_IOC_MAGIC +160)
#define RLCM_GET_DSL_STAT_TOTAL_REV1P4		(RLCM_IOC_MAGIC +161)
#define RLCM_GET_DSL_STAT_LAST_SHOWTIME_REV1P4	(RLCM_IOC_MAGIC +162)
#define RLCM_GET_DSL_STAT_15MIN_REV1P4		(RLCM_IOC_MAGIC +163)
#define RLCM_GET_DSL_STAT_1DAY_REV1P4		(RLCM_IOC_MAGIC +164)
#define RLCM_GET_DSL_STAT_FLAG_LOF		(RLCM_IOC_MAGIC +165)
#define RLCM_GET_FRAME_COUNT			(RLCM_IOC_MAGIC +166)
#define RLCM_DBG_DATA				(RLCM_IOC_MAGIC +167)
#define RLCM_SET_DSL_FUNC			(RLCM_IOC_MAGIC +168)
#define RLCM_GET_DSL_STAT_PRE_15MIN		(RLCM_IOC_MAGIC +180)
#define	RLCM_GET_CHANNEL_TABLE	 (RLCM_IOC_MAGIC +190)		// Mason Yu.  20130207
#define RLCM_GET_PREV1DAY_LOSS_DATA  (RLCM_IOC_MAGIC +200)
#define RLCM_GET_ANY_15MIN_LOSS_DATA (RLCM_IOC_MAGIC +201)
#define RLCM_GET_ADSLLINE_ENABLED  (RLCM_IOC_MAGIC +202)
#define RLCM_SET_ADSLLINE_ENABLED (RLCM_IOC_MAGIC +203)
#define RLCM_GET_ADSLLINE_CAP_ACT  (RLCM_IOC_MAGIC +204)
#define RLCM_GET_ADSLLINE_MODE_CAP (RLCM_IOC_MAGIC +205)
#define RLCM_GET_ADSLLINE_MODE_ACT (RLCM_IOC_MAGIC +206) 

#ifdef CONFIG_VDSL
#define RLCM_DEBUG_DATA						(RLCM_IOC_MAGIC +111+1000)
#define RLCM_GET_VDSL2_MREFPSD_DS			(RLCM_IOC_MAGIC +114+1000)
#define RLCM_GET_VDSL2_TXREFVNPSD_DS		(RLCM_IOC_MAGIC +115+1000)
#define RLCM_GET_VDSL2_MREFPSD_US			(RLCM_IOC_MAGIC +116+1000)
#define RLCM_GET_VDSL2_TXREFVNPSD_US		(RLCM_IOC_MAGIC +117+1000)
#define RLCM_GET_ADSL_DIAG_HLIN				(RLCM_IOC_MAGIC +118+1000)
#define RLCM_GET_ADSL_DIAG_GI				(RLCM_IOC_MAGIC +119+1000)
#define RLCM_GET_ADSL_DIAG_BI				(RLCM_IOC_MAGIC +120+1000)
#define RLCM_GET_ADSL_DIAG_OTHER			(RLCM_IOC_MAGIC +121+1000)
#define RLCM_GET_VDSL2_DIAG_QLN				(RLCM_IOC_MAGIC +122+1000)
#define RLCM_GET_VDSL2_DIAG_HLOG			(RLCM_IOC_MAGIC +123+1000)
#define RLCM_GET_VDSL2_DIAG_SNR				(RLCM_IOC_MAGIC +124+1000)
#define RLCM_GET_VDSL2_DIAG_HLIN			(RLCM_IOC_MAGIC +125+1000)
#define RLCM_GET_VDSL2_DIAG_HLIN_SCALE		(RLCM_IOC_MAGIC +126+1000)
#define RLCM_GET_VDSL2_DIAG_OTHER			(RLCM_IOC_MAGIC +127+1000)
//#define RLCM_GET_VDSL2_DS_PMS_PARAM1		(RLCM_IOC_MAGIC +128+1000)
//#define RLCM_GET_VDSL2_US_PMS_PARAM1		(RLCM_IOC_MAGIC +129+1000)
#define RLCM_GET_VDSL2_SNR_MARGIN			(RLCM_IOC_MAGIC +130+1000)
#define RLCM_GET_VDSL2_LOOP_ATT				(RLCM_IOC_MAGIC +131+1000)
#define RLCM_GET_VDSL2_CHANNEL_SNR			(RLCM_IOC_MAGIC +132+1000)
#define RLCM_GET_VDSL2_CHANNEL_BITLOAD		(RLCM_IOC_MAGIC +133+1000)
#define RLCM_WANBonderDSLConfig_Enable  	(RLCM_IOC_MAGIC +137+1000)
#define RLCM_WANBonderDSLConfig_Status   	(RLCM_IOC_MAGIC +138+1000)
#define RLCM_WANBonderDSLConfig_GroupBondScheme   		(RLCM_IOC_MAGIC +139+1000)
#define RLCM_WANBonderDSLConfig_GroupCapacity   		(RLCM_IOC_MAGIC +140+1000)
#define RLCM_WANBonderDSLConfig_aGroupTargetUpRate   	(RLCM_IOC_MAGIC +141+1000)
#define RLCM_WANBonderDSLConfig_aGroupTargetDownRate   	(RLCM_IOC_MAGIC +142+1000)
#define RLCM_WANConfig_LineNumber 						(RLCM_IOC_MAGIC +143+1000)
#define RLCM_UserGetDslData 				(RLCM_IOC_MAGIC +144+1000)
#define RLCM_UserSetDslData 				(RLCM_IOC_MAGIC +145+1000)
//VDSL specifics END
#endif /*CONFIG_VDSL*/

#if defined(CONFIG_DSL_ON_SLAVE)
#define RLCM_INIT_ATMSAR                    (RLCM_IOC_MAGIC +146+1000)
#define RLCM_INIT_PTMMAC                    (RLCM_IOC_MAGIC +147+1000)
#endif

#define TONE_RANGE	512
#define GET_LOADCARRIERMASK_SIZE (sizeof(unsigned char)*64)
/*ioctl argument size or struct*/
//for RLCM_GET_DRIVER_VERSION
#define RLCM_DRIVER_VERSION_SIZE	10

//for RLCM_GET_DRIVER_BUILD
#define RLCM_DRIVER_BUILD_SIZE		14

//for RLCM_GET_REHS_COUNT
#define RLCM_GET_REHS_COUNT_SIZE	4  //(sizeof(reHandshakeCount))

//for RLCM_GET_CHANNEL_BITLOAD
#define RLCM_GET_CHANNEL_BITLOAD_SIZE	(sizeof(short)*TONE_RANGE)  //(sizeof(gChannelBitLoad))

//for RLCM_GET_CHANNEL_SNR
#define RLCM_GET_CHANNEL_SNR_SIZE	(sizeof(short)*TONE_RANGE)  //(sizeof(gChannelSNR))

//for RLCM_GET_AVERAGE_SNR
#define RLCM_GET_AVERAGE_SNR_SIZE	4  //(sizeof(gAverageSNR))

//for RLCM_GET_SNR_MARGIN
#define RLCM_GET_SNR_MARGIN_SIZE	8  //(sizeof(gitex_EstimatedSNRMargin))

//for RLCM_REPORT_MODEM_STATE
#define MODEM_STATE_IDLE                0
#define MODEM_STATE_L3                  1
#define MODEM_STATE_LISTENING           2
#define MODEM_STATE_ACTIVATING          3
#define MODEM_STATE_Ghs_HANDSHAKING     4
#define MODEM_STATE_ANSI_HANDSHAKING    5
#define MODEM_STATE_INITIALIZING        6
#define MODEM_STATE_RESTARTING          7
#define MODEM_STATE_FAST_RETRAIN        8
#define MODEM_STATE_SHOWTIME_L0         9
#define MODEM_STATE_SHOWTIME_LQ         10
#define MODEM_STATE_SHOWTIME_L1         11
#define MODEM_STATE_EXCHANGE            12

//for RLCM_REPORT_PM_DATA
// Mason Yu.  20130207
typedef struct {
	unsigned long AdslAtucChanInterleaveDelay;
	unsigned long AdslAtucChanCurrRate;
	unsigned long AdslAtucChanPrevRate;
	unsigned long AdslAtucChanCrcBlockLength;	
	unsigned long AdslAturChanInterleaveDelay;
	unsigned long AdslAturChanCurrRate;
	unsigned long AdslAturChanPrevRate;
	unsigned long AdslAturChanCrcBlockLength;
 } Modem_AdslChannelTable;
#define	CHANNEL_TABLE_SIZE	sizeof(Modem_AdslChannelTable)

//for RLCM_GET_ATT_RATE
typedef struct {
	unsigned short	upstreamRate;
	unsigned short	downstreamRate;
} Modem_AttRate;
#define GET_LINE_RATE_SIZE		sizeof(Modem_AttRate)

//for RLCM_GET_DS_ERROR_COUNT & RLCM_GET_US_ERROR_COUNT
typedef struct {
	unsigned long	crc;
	unsigned long	fec;
	unsigned long	es;
	unsigned long	ses;
	unsigned long	uas;
	#if SUPPORT_TR105
	unsigned long	los;	
	unsigned long	reserved[4]; // make total size 40 bytes
	#endif
} Modem_MgmtCounter;

///for RLCM_GET_DS_PMS_PARAM1 & RLCM_GET_US_PMS_PARAM1
typedef struct {
	unsigned short	K;
	unsigned short	R;
	unsigned short	D;
	unsigned short	N_fec;
	unsigned short	Lp;
	unsigned short	S;
	unsigned short	Delay;
} Modem_PMSParm;
#define GET_PMS_SIZE	sizeof(Modem_PMSParm)
// Mason Yu.  20130207. End

typedef struct {
  unsigned short  FecNotInterleaved;
  unsigned short  FecInterleaved;
  unsigned short  CrcNotInterleaved;
  unsigned short  CrcInterleaved;
  unsigned short  HecNotInterleaved;
  unsigned short  HecInterleaved;
  unsigned short  TotalCellCountInterleaved;
  unsigned short  TotalCellCountNotInterleaved;
  unsigned short  ActiveCellCountInterleaved;
  unsigned short  ActiveCellCountNotInterleaved;
  unsigned short  BERInterleaved;
  unsigned short  BERNotInterleaved;
} Modem_def_counters;

typedef struct {
  Modem_def_counters  near_end;
  Modem_def_counters   far_end;
} Modem_def_counter_set;
#define RLCM_REPORT_PM_DATA_SIZE	sizeof(Modem_def_counter_set) 


/*for 	RLCM_MODEM_NEAR_END_ID_REQ
	RLCM_MODEM_FAR_END_ID_REQ
*/
typedef struct   { 
  unsigned char   countryCode; 
  unsigned char   reserved; 
  unsigned long   vendorCode; 
  unsigned short  vendorSpecific; 
} RLCM_ITU_VendorId; 

typedef struct { 
  RLCM_ITU_VendorId    ITU_VendorId; 
  unsigned char   ITU_StandardRevisionNbr; 
  unsigned short  ANSI_ETSI_VendorId; 
  unsigned char   ANSI_ETSI_VendorRevisionNbr; 
  unsigned char   ANSI_ETSI_StandardRevisionNbr; 
  unsigned long   ALC_ManagementInfo; 
} Modem_Identification; 
#define RLCM_MODEM_ID_REQ_SIZE	sizeof(Modem_Identification) 

//for RLCM_MODEM_NEAR_END_LINE_DATA_REQ
typedef struct {
  unsigned short relCapacityOccupationDnstr;
  // Mason Yu Output Power Error
  //signed char	 noiseMarginDnstr;
  signed short	 noiseMarginDnstr;
  signed char    outputPowerUpstr;
  unsigned char  attenuationDnstr;
  unsigned long operationalMode;
} Modem_NearEndLineOperData;
#define RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE sizeof(Modem_NearEndLineOperData)

//for RLCM_MODEM_FAR_END_LINE_DATA_REQ
typedef struct {
  unsigned short relCapacityOccupationUpstr;
  // Mason Yu Output Power Error
  //signed char	 noiseMarginUpstr;
  signed short	 noiseMarginUpstr;
  signed char	 outputPowerDnstr;
  unsigned char	 attenuationUpstr;
  unsigned char  carrierLoad[TONE_RANGE>>1];
} Modem_FarEndLineOperData;
#define RLCM_MODEM_FAR_END_LINE_DATA_REQ_SIZE sizeof(Modem_FarEndLineOperData)

/*for 	RLCM_MODEM_NEAR_END_FAST_CH_DATA_REQ
	RLCM_MODEM_NEAR_END_INT_CH_DATA_REQ
	RLCM_MODEM_FAR_END_FAST_CH_DATA_REQ
	RLCM_MODEM_FAR_END_INT_CH_DATA_REQ
*/
#define RLCM_MODEM_CH_DATA_REQ_SIZE	sizeof(int)

/*for RLCM_SET_ADSL_MODE & RLCM_GET_ADSL_MODE & RLCM_PHY_START_MODEM
If you select multi-mode value (that is, ANSI | G_DMT | G_LITE or ANSI | G_DMT
or ANSI | G_LITE or G_DMT | G_LITE), the handshaking priority will be G_DMT first,
then G_LITE, finally ANSI.
*/
#define ADSL_MODE_ANSI		1	//ANSI T1.413 issue 2 mode
#define ADSL_MODE_G_DMT		2	//ITU G.dmt mode
#define ADSL_MODE_G_LITE	8	//ITU G.lite mode

//for RLCM_GET_LOSS_DATA, RLCM_GET_15MIN_LOSS_DATA, RLCM_GET_1DAY_LOSS_DATA
//
typedef struct {
	unsigned long	LossOfFrame_NE;		//NELOF
	unsigned long	LossOfFrame_FE;		//FELOF
	unsigned long	LossOfSignal_NE;	//NELOS
	unsigned long	LossOfSignal_FE;	//FELOS
	unsigned long	LossOfPower_NE;		//NELPR
	unsigned long	LossOfPower_FE;		//FELPR
	unsigned long	LCD_Fast_NE;		//NELCD_f
	unsigned long	LCD_Fast_FE;		//FELCD_f
	unsigned long	LCD_Interleaved_NE;	//NELCD_f
	unsigned long	LCD_Interleaved_FE;	//FELCD_f
	unsigned long   CrcCounter_Fast_NE;	//CRC error count
	unsigned long   CrcCounter_Interleaved_NE; //CRC error count
	unsigned long   ESs_NE; //Error second count
} Modem_LossData;
#define RLCM_GET_LOSS_DATA_SIZE 	sizeof(Modem_LossData)

//for RLCM_GET_LINK_SPEED
typedef struct {
	unsigned long	upstreamRate;
	unsigned long	downstreamRate;
} Modem_LinkSpeed;
#define RLCM_GET_LINK_SPEED_SIZE 	sizeof(Modem_LinkSpeed)

//for RLCM_GET_CHANNEL_MODE
#define	CH_MODE_FAST		1
#define	CH_MODE_INTERLEAVE	2

//for RLCM_GET_LOOP_ATT
typedef struct {
	unsigned short	upstreamAtt;
	unsigned short	downstreamAtt;
} Modem_AvgLoopAttenuation;
#define RLCM_GET_LOOP_ATT_SIZE 	sizeof(Modem_AvgLoopAttenuation)

//for RLCM_MODEM_READ_CONFIG, RLCM_MODEM_WRITE_CONFIG
typedef struct {
	int HandshakeMode;
        int TxPower;
        int PerfTuning;
        int AtuC_VendorID;
        int BitSwapEnable;
        int PilotRelocationEnable;
        int TrellisEnable;
} Modem_Config;
#define RLCM_MODEM_CONFIG_SIZE 	sizeof(Modem_Config)


//for RLCM_TEST_PSD, other value is normal operation
#define TEST_SEND_QUIET		1	//send Quiet
#define TEST_SEND_REVERB1	2	//send Reverb1

//for RLCM_GET_TRAP_THRESHOLD
#define RLCM_TRAP_THRESHOLD_SIZE 	sizeof(Modem_LossData)

//for RLCM_15MIN_WAIT_TRAP
#define RLCM_WAIT_TRAP_ES_NE	0	//wait 15min ATUR Error second trap


#ifdef CONFIG_VDSL
#define TR069_SUPPORT
#define TR069_REV1P4_SUPPORT
#define ATandT_WT157
//yaru for TR069
typedef struct {
	unsigned int	ReceiveBlocks;
	unsigned int	TransmitBlocks;
	unsigned int	CellDelin;
	unsigned int	LinkRetain;
	unsigned int	InitErrors;
	#ifdef TR069_REV1P4_SUPPORT
	unsigned int	LInit;//bool LInit;
	#endif
	unsigned int	InitTimeouts;
	unsigned int	LossOfFraming;
	#ifdef TR069_REV1P4_SUPPORT
	unsigned int	LOF;//bool	LOF;
	#endif
	unsigned int	ES;
	#ifdef TR069_REV1P4_SUPPORT
	unsigned int	AtucES;
	#endif
	unsigned int	SES;
	#ifdef TR069_REV1P4_SUPPORT
	unsigned int	AtucSES;
	#endif
	unsigned int	FEC;
	unsigned int	AtucFEC;
	unsigned int	HEC;
	unsigned int	AtucHEC;
	unsigned int	CRC;
	unsigned int	AtucCRC;
	#ifdef ATandT_WT157
	unsigned long	LossOfMargin;	//LOM
	unsigned long	LossOfPower;	//LPR
	unsigned long	LossOfSignal;	//LOS
	unsigned long	UnavailableSec; //unavailable second count
	#endif
}Modem_DSLConfigStatus;	//yaru TR069, copy T_DSLConfigStatus
#else
//yaru for TR069
typedef struct {
	unsigned int	ReceiveBlocks;
	unsigned int	TransmitBlocks;
	unsigned int	CellDelin;
	unsigned int	LinkRetain;
	unsigned int	InitErrors;
	unsigned int	InitTimeouts;
	unsigned int	LOF;
	unsigned int	ES;
	unsigned int	SES;
	unsigned int	FEC;
	unsigned int	AtucFEC;
	unsigned int	HEC;
	unsigned int	AtucHEC;
	unsigned int	CRC;
	unsigned int	AtucCRC;
} Modem_DSLConfigStatus;	//yaru TR069, copy T_DSLConfigStatus
#endif /*CONFIG_VDSL*/


//Magician: TR069_REV1P4_SUPPORT
typedef struct {
 unsigned int AtucES;  //Rev1.4
 unsigned int AtucSES; //Rev1.4
} Modem_DSLConfigStatusRev1p4Add; //yaru TR069
#define TR069_STAT_SIZE_REV1P4 sizeof(Modem_DSLConfigStatusRev1p4Add)
//---- end TR069_REV1P4_SUPPORT


//Modem_ADSL2WANConfig.LinkEncapSupported/LinkEncapRequested/LinkEncapUsed:
enum
{
	LE_G_992_3_ANNEX_K_ATM=0,
	LE_G_992_3_ANNEX_K_PTM,
	LE_G_993_2_ANNEX_K_ATM,
	LE_G_993_2_ANNEX_K_PTM,
	LE_G_994_1,
	
	LE_END /*the last one*/
};
//Modem_ADSL2WANConfig.StandardsSuported/StandardUsed:
enum
{
	STD_G_992_1_Annex_A=0,
	STD_G_992_1_Annex_B,
	STD_G_992_1_Annex_C,
	STD_T1_413,
	STD_T1_413i2,
	STD_ETSI_101_388,
	STD_G_992_2,
	STD_G_992_3_Annex_A,
	STD_G_992_3_Annex_B,
	STD_G_992_3_Annex_C,
	
	STD_G_992_3_Annex_I,
	STD_G_992_3_Annex_J,
	STD_G_992_3_Annex_L,
	STD_G_992_3_Annex_M,
	STD_G_992_4,
	STD_G_992_5_Annex_A,
	STD_G_992_5_Annex_B,
	STD_G_992_5_Annex_C,
	STD_G_992_5_Annex_I,
	STD_G_992_5_Annex_J,
	
	STD_G_992_5_Annex_M,
	STD_G_993_1,
	STD_G_993_1_Annex_A,
	STD_G_993_2_Annex_A,
	STD_G_993_2_Annex_B,
	STD_G_993_1_Annex_C,
	
	STD_END /*the last one*/
};

typedef struct {
	unsigned int LinkEncapSupported;
	unsigned int LinkEncapRequested;
	unsigned int LinkEncapUsed;
	unsigned int StandardsSuported;
	unsigned int StandardUsed;		
} Modem_ADSL2WANConfig; //Lupin, tr069

#define TR069_ADSL2WANCFG_SIZE sizeof(Modem_ADSL2WANConfig)
#define TR069_STAT_SIZE sizeof(Modem_DSLConfigStatus)
#define TR069_DIAG_GI_SIZE (sizeof(unsigned short)*(TONE_RANGE+1)))
#define TR069_DSL_PSD_SIZE (sizeof(int)*2)
#define TR069_DSL_OTHER_SIZE (sizeof(unsigned int)*3)
//end for TR069

//additional size for RLCM_GET_DIAG_HLOG ioctl, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE)
#define HLOG_ADDITIONAL_SIZE	13

//use RLCM_ENABLE_DIAGNOSTIC to start dsl diag. (instead of RLCM_DEBUG_MODE ioctl (mode=41))
#define _USE_NEW_IOCTL_FOR_DSLDIAG_	1


#define SUPPORT_TR105 0 /* support tr105, default off */
// DSL function mask for RLCM_SET_DSL_FUNC
#ifdef ENABLE_ADSL_MODE_GINP
#define DSL_FUNC_GINP			0x00000001
#endif



#ifdef CONFIG_VDSL
typedef struct
{
	int message;
	int* intVal;
} MSGTODSL;

/*deference MSGTODSL*/
typedef struct
{
	int message;
	int intVal[4];
} DEF_MSGTODSL;

//for RLCM_UserGetDslData
enum 
{
	GetPmdMode=0,		//read enum RevisionMode
	GetVdslProfile=1,	//read enum Vdsl2Profile 
	GetTrellis=2,		//0:off, 1:on; val[0:1]={Ds, Us}
	GetPhyR=3,			// 0:off, 1:on; val[0:1]={Ds, Us}
	GetHskXdslMode=4,	//val[0:1]={pmdmode, vdslprofile}
	GetUPBOKLE=5,		//val[0:1]={UPBOKLE(O to R), UPBOKLE-R(R to O)}; 0.1dB; unsigned
	GetPmsPara_Nfec=6,	//val[0:3]={Lp0Ds, Lp0Us, Lp1Ds, Lp1Us}
	GetPmsPara_L=8,		//val[0:3]={Lp0Ds, Lp0Us, Lp1Ds, Lp1Us}
	GetACTSNRMODE=9,	//val[0:1]={Ds,Us}; unsigned
	GetACTUALCE=10,		//val[0:0]; unit: N/32; unsigned
	GetPmsPara_INP=11,	//val[0:3]={Lp0Ds, Lp0Us, Lp1Ds, Lp1Us}*1000
	GetTpsMode=12,		//val[0:0];0:NA 2:PTM, 4:ATM, 8:STM, 0:NA
	GetAttNDR=13,		//val[0:1];[Ds, Us]


//#ifdef CONFIG_DSL_VTUO
	GetLimitMask=14,	//0: D32, 1:D48, 2:D64, 3:D128
	GetUs0Mask=15,		//From 0 upwards: EU32,36,40,44,48,52,56,60,64,128
	GetSATN=16,			//[0:1]={DS,US};0.1dB;unsigned
	GetLATN=17,			//[0:1]={DS,US};0.1dB;unsigned
	GetDslStateLast=18,	//
	GetDslStateCurr=19,	//
	GetRxPwr=20,		//val[0:1]={DS US};0.1dBm;signed
	GetInpReport=21,	//
	GetPmsPara_I=22,	//val[0:3]={Lp0Ds, Lp0Us, Lp1Ds, Lp1Us};unsigned
	GetDataLpId=23,		//val[0:1]={DS US};unsigned
	GetPTMStatus=24,	//0: NoDefect
	GetRAMode=25,		//val[0:1]={DS US};signed; -1:NA(No showtime), RA_MANUAL=0, RA_AT_INIT=1,RA_DYNAMIC=2,RA_DYNAMICSOS=3
	GetReTxOhRate=26,	//val[0:1]={DS US};kbps;unsigned
	GetReTxFrmType=27,	//val[0:1]={DS US};unsigned
	GetReTxActInpRein=28,//val[0:1]={DS US};unsigned
	GetReTxH=29,		//val[0:1]={DS US};unsigned

	GetSNRpb=30,		//[0:9]={US0,US1,US2,US3,US4,DS1,DS2,DS3,DS4,DS5};0.1dB;signed; -512:out of range or band is disabled
	GetSATNpb=31,		//[0:9]={US0,US1,US2,US3,US4,DS1,DS2,DS3,DS4,DS5};0.1dB;unsigned; 1023:out of range or band is disabled
	GetLATNpb=32,		//[0:9]={US0,US1,US2,US3,US4,DS1,DS2,DS3,DS4,DS5};0.1dB;unsigned; 1023:out of range or band is disabled
	GetTxPwrpb=33,		//[0:9]={US0,US1,US2,US3,US4,DS1,DS2,DS3,DS4,DS5};0.1dmB;signed
	GetRxPwrpb=34,		//[0:9]={US0,US1,US2,US3,US4,DS1,DS2,DS3,DS4,DS5};0.1dBm;signed

	GetFECS=36,			//val[0:1]={DS US};unsigned
	GetLOSs=37,			//val[0:1]={DS US};unsigned
	GetLOFs=38,			//val[0:1]={DS US};unsigned
	GetRSCorr=39,		//val[0:1]={DS US};unsigned
	GetRSUnCorr=40,		//val[0:1]={DS US};unsigned
	GetReTxRtx=41,		//val[0:1]={DS US};unsigned
	GetReTxRtxCorr=42,	//val[0:1]={DS US};unsigned
	GetReTxRtxUnCorr=43,//val[0:1]={DS US};unsigned
	GetReTxLEFTRs=44,	//val[0:1]={DS US};unsigned
	GetReTxMinEFTR=45,	//val[0:1]={DS US};unsigned
	GetReTxErrFreeBits=46,//val[0:1]={DS US};unsigned
	GetLOL=47,			//val[0:1]={DS US};unsigned
	GetLOLs=48,			//val[0:1]={DS US};unsigned
	GetLPR=49,			//val[0:1]={DS US};unsigned
	GetLPRs=50,			//val[0:1]={DS US};unsigned

	GetPSD_MD_Ds=51,	//val[]={n, (x1,y1),(x2,y2},...,(xn,yn)}; n(# of points; unsigned), x(tone; unsigned), y(PSD; signed, 0.1dBm/Hz)
	GetPSD_MD_Us=52,	//val[]={n, (x1,y1),(x2,y2},...,(xn,yn)}; n(# of points; unsigned), x(tone; unsigned), y(PSD; signed, 0.1dBm/Hz)

	GetDeviceType=53,	//0:VTUO, 1:VTUR

	GetDsBand=54,		//{n,(toneStart1,toneEnd1),(toneStart2,toneEnd2)...(toneStartn,toneEndn)}, max n=5
	GetUsBand=55,		//{n,(toneStart1,toneEnd1),(toneStart2,toneEnd2)...(toneStartn,toneEndn)}, max n=5
	GetBitPerToneDs=56,	//val[0:4095]; input array; int[4096/4]=char[4096];unsigned; value=0~15
	GetBitPerToneUs=57,	//val[0:4095]; unsigned; int[4096/4]=char[4096]; value=0~15
	GetGainPerToneDs=58,//val[0:4095]; unsigned; int[4096/2]=short[4096]; value=0~682
	GetGainPerToneUs=59,//val[0:4095]; unsigned; int[4096/2]=short[4096]; value=0~682

	GetEvCntDs_e127=60,
	GetEvCntUs_e127=61,
	GetEvCnt15MinDs_e127=62,	//need input: Index=val[0]=0~95
	GetEvCnt15MinUs_e127=63,	//need input: Index=val[0]=0~95
	GetEvCnt1DayDs_e127=64,		//need input: Index=val[0]=0~6
	GetEvCnt1DayUs_e127=65,		//need input: Index=val[0]=0~6
		/*format: all unsigned int for GetEvCnt-related
		  val[ 0]=Index 
		  val[ 1]=Full Inits    
		  val[ 2]=Failed Full Inits 
		  val[ 3]=FECS           
		  val[ 4]=ES             
		  val[ 5]=SES            
		  val[ 6]=LOSs           
		  val[ 7]=LOFs           
		  val[ 8]=UAS            
		  val[ 9]=CodeViolation  
		  val[10]=Corrected      
		  val[11]=Uncorrected    
		  val[12]=Rtx            
		  val[13]=RtxCorrected   
		  val[14]=RtxUncorrected 
		  val[15]=LEFTRs         
		  val[16]=MinEFTR        
		  val[17]=ErrFreeBits    
		  val[18]=LOLs           
		  val[19]=LOL            
		  val[20]=LPRs           
		  val[21]=LPR            
		  val[22]=TimeElapsed    
		*/

	GetInmCntDs_e127=66,		//VTUO's farend
	GetInmCntUs_e127=67,		//VTUO's nearend
	GetInmCnt15MinDs_e127=68,	//VTUO's farend; need input: Index=val[0]=0~95
	GetInmCnt15MinUs_e127=69,	//VTUO's nearend; need input: Index=val[0]=0~95
	GetInmCnt1DayDs_e127=70,	//VTUO's farend; need input: Index=val[0]=0~6
	GetInmCnt1DayUs_e127=71,	//VTUO's nearend; need input: Index=val[0]=0~6
		/*format: all unsigned int, GetInmCnt-related
		  val[ 0]=Index
		  val[ 1]=EqInp01
		  val[ 2]=EqInp02
		  val[ 3]=EqInp03
		  val[ 4]=EqInp04
		  val[ 5]=EqInp05
		  val[ 6]=EqInp06
		  val[ 7]=EqInp07
		  val[ 8]=EqInp08
		  val[ 9]=EqInp09
		  val[10]=EqInp10
		  val[11]=EqInp11
		  val[12]=EqInp12
		  val[13]=EqInp13
		  val[14]=EqInp14
		  val[15]=EqInp15
		  val[16]=EqInp16
		  val[17]=EqInp17
		  val[18]=IAT00  
		  val[19]=IAT01  
		  val[20]=IAT02  
		  val[21]=IAT03  
		  val[22]=IAT04  
		  val[23]=IAT05  
		  val[24]=IAT06  
		  val[25]=IAT07  
		  val[26]=Symbol 
		*/
//#endif /*CONFIG_DSL_VTUO*/
	GetVectorMode=73,		//val[0]={Vector_Enable};unsigned int
};

//RLCM_UserSetDslData
enum {
	SetPmdMode=0,//read enum RevisionMode
	SetVdslProfile=1,//read enum Vdsl2Profile 
	SetGInp=2,// 0:off, 1:on; BIT[0] for Ds; BIT[1] for Us
	SetOlr=3,//0:off, 1:on; BIT[0] for bitswap; BIT[1] for SRA
	SetSwApiDef=4,
	SetGVector=5,
//#ifdef CONFIG_DSL_VTUO
	SetChnProfUs=6,
	SetChnProfDs=7,
		/* int[6] for SetChnProfUs / SetChnProfDs
		 * 00  int netDataRateMax;// 1Kbps
		 * 01  int netDataRateMin;// 1Kbps
		 * 02  int delayMax;//ms
		 * 03  int inpMin;//0.1symbol
		 * 04  int inpMin8;//0.1symbol
		 * 05  int sosDataRateMin;// 1Kbps
		 */
	SetChnProfGinpUs=8,
	SetChnProfGinpDs=9,
		/* int[11] for SetChnProfGinpUs /SetChnProfGinpDs
		 * 00  gInpMode=pInt[0];//0: forbidden, 1: preferred, 2: forced, 3: Test mode
		 * 01  gInpEtrMax=pInt[1];// 1Kbps
		 * 02  gInpEtrMin=pInt[2];// 1Kbps
		 * 03  gInpDelayMax=pInt[3];//ms
		 * 04  gInpDelayMin=pInt[4];//ms
		 * 05  gInpInpMin=pInt[5];//0.1s ymbol
		 * 06  gInpReinInp=pInt[6];//0.1 symbol
		 * 07  gInpReinFreq=pInt[7];//0:100Hz, 1:120Hz
		 * 08  gInpShineRatio=pInt[8];//unit: 0.001*NDR
		 * 09  gInpLEFTRthrd=pInt[9];//unit: 0.01*NDR
		 * 10  gInpNDR=pInt[10];//unit: 1Kbps
		 */
	SetLineMarginUs=10,
	SetLineMarginDs=11,
		/* int[4] for SetLineMarginUs / SetLineMarginDs
		 * 00  short snrmMax;//0.1dB
		 * 01  short snrmTarget;//0.1dB
		 * 02  short snrmMin;//0.1dB
		 * 03  char snrNoLimit;//0: No, 1:Yes
		 */
	SetLinePowerUs=12,
	SetLinePowerDs=13,
		/* int[3] for SetLinePowerUs / SetLinePowerDs
		 * 00  short pwrRxMax;//0.1dBm
		 * 01  short pwrTxMax;//0.1dBm
		 * 02  char pwrRxLimit;//0:No, 1:Yes
		 */
	SetLineBSUs=14,
	SetLineBSDs=15,
		/* int[1] for SetLineBSUs / SetLineBSDs
		 * 00  char bitswap;//0:No, 1:Yes
		 */
	SetOHrateUs=16,
	SetOHrateDs=17,
		/* int[1] for SetOHrateUs / SetOHrateDs
		 * 00  int rateOhMin;// 1Kbps
		 */
	SetLineTxMode=18, //int[0]=  0:993.2AnnexA
	SetLineADSL=19, //int[0]= Use the old way to setup ATUR
	SetLineClassMask=20, //int[0]= 0:998, 1:997
	SetLineLimitMask=21, //int[0]= 0: D32, 1:D48, 2:D64, 3:D128
	SetLineUs0tMask=22, //int[0]= From 0 upwards: EU32,36,40,44,48,52,56,60,64,128
	SetLineUPBO=24,
		/* int [10] for SetLineUPBO
		 * 00  int upboEnable;//0:disable, 1:Auto, 2:override  
		 * 01  int upbokl;//0.1dB
		 * 02-05  int upboBandA[4];//0.01
		 * 06-09  int upboBandB[4];//0.01
		 */
	SetLineRTMode=25, //int[0]= 0:OFF, 1:ON
	SetLineUS0=26, //int[0]= 0:OFF, 1:ON
	SetLineRAUs=27,
	SetLineRADs=28,
		/* int [6] for SetLineRAUs/SetLineRAUs
		 * 00  char rateAdaptMode;//manaul=0, adaptInit=1, SRA=2, SOS=3
		 * 01  char dynamicD;//0: disable, 1: enable
		 * 02  short sraUpShiftSNRM;//0.1dB
		 * 03  short sraDnShiftSNRM;//0.1dB
		 * 04  short sraUpShiftTime;//s
		 * 05  short sraDnShiftTime;//s
		 */
	SetLineSOSUs=29,
	SetLineSOSDs=30,
		/* int [6] for SetLineSOSUs / SetLineSOSDs
		 * 00  char sosEnable;   //rateAdaptMode==SOS
		 * 01  short sosTime;//ms
		 * 02  short sosCRC;
		 * 03  char sosnTone;//0.01
		 * 04  char sosMax;
		 * 05  char sosMultiStepTone=0;
		 */
	SetLineROCUs=31,
	SetLineROCDs=32,
		/* int [3] for SetLineROCUs / SetLineROCDs
		 * 00  char rocEnable;
		 * 01  short rocSNRMOffset;//0.1dB
		 * 02  char rocInpMin;//symbol
		 */


	SetLineDPBO=34,
		/*  int [8] for SetLineDPBO
		 * 00  char dpboEnable;
		 * 01  int dpboesel;//0.1dB
		 * 02  int dpboescma;//0.0001
		 * 03  int dpboescmb;//0.0001
		 * 04  int dpboescmc;//0.0001
		 * 05  int dpbomus;//0.1dB
		 * 06  int dpbofmin;//unit: 4.3125kHz 
		 * 07  int dpbofmax;//unit: 4.3125kHz
		 */
	SetLineDPBOPSD=35,
		/*
		 * 00  char Enable;//0:Off, 1:ON
		 * 01  char NumOfPoint;//# of active point
		 * 02  short tone[0] level[0] ... tone[NumOfPoint-1] level[NumOfPoint-1]
		 *     NOTE:
		 *     tone[X];//unit: 4.3125kHz
		 *     level[X];//0.1 dBm/Hz
		 */
	SetLineRFI=36,
		/*
		 * 00  char Enable;//0:Off, 1:ON
		 * 01  char NumOfPoint;//# of active point
		 * 02  short start_tone[0] stop_tone[0]... start_tone[NumOfPoint-1] stop_tone[NumOfPoint-1]
		        NOTE:
		        tone[X];//unit: 4.3125kHz
		 */

	SetInmFE=38,
	SetInmNE=39,
		/* int[5] for SetInmUs / SetInmDs
		 * 00  char inmInpEqMode;//0,1,2,3,...
		 * 01  char inmInmcc;//symbol
		 * 02  char inmIatOffset;//symbol
		 * 03  char inmIatSetup;//symbol
		 * 04  short inmIsddSen;//0.1dB
		 */
	SetLinePSDMIBUs=40,
	SetLinePSDMIBDs=41,
		/*
		 * 00  char Enable;//0:Off, 1:ON
		 * 01  char NumOfPoint;//# of active point
		 * 02  short tone[0] level[0] ... tone[NumOfPoint-1] level[NumOfPoint-1]
		        NOTE:
		        tone[X];//unit: 4.3125kHz
		        level[X];//0.1 dBm/Hz
		 */
	SetLineVNUs=42,
	SetLineVNDs=43,
		/*
		 * 00  char Enable;//0:Off, 1:ON
		 * 01  char NumOfPoint;//# of active point
		 * 02  short tone[0] level[0] ... tone[NumOfPoint-1] level[NumOfPoint-1]
		        NOTE:
		        tone[X];//unit: 4.3125kHz
		        level[X];//0.1 dBm/Hz
		 */
	SetDslBond=51,
	/*
	int enable;//0:Off, 1:ON
	int role;//0:master, 1,2,3,...:slave
	int numOfLineBonded;//1,2,3 or 4
	int tcType;//bit0: PTM; bit1:TDIM; bit2: ATM
	*/
//#endif /*CONFIG_DSL_VTUO*/
};

enum RevisionMode
{	
	//ANSI				    
	MODE_ANSI		= 0x01000,
	MODE_ETSI		= 0x02000,
	//ITU
	MODE_GDMT		= 0x04000,
	MODE_GLITE		= 0x08000,
	MODE_ADSL2		= 0x10000,
	MODE_ADSL2LITE	= 0x20000,
	MODE_ADSL2PLUS	= 0x40000,
	MODE_VDSL1		= 0x80000,
	MODE_VDSL2		= 0x100000,

	//compiler error: syntax error before numeric constant, use define instead
	//MODE_MASK		= MODE_ANSI|MODE_ETSI|MODE_GDMT|MODE_GLITE|MODE_ADSL2|MODE_ADSL2LITE|MODE_ADSL2PLUS|MODE_VDSL1|MODE_VDSL2,

	//Annex
	MODE_ANX_A		= 0x001,
	MODE_ANX_B		= 0x002,
	MODE_ANX_I		= 0x004,
	MODE_ANX_J		= 0x008,
	MODE_ANX_L		= 0x010,
	MODE_ANX_M		= 0x020,
//	MODE_ANX_A2P	= 0x040,
//	MODE_ANX_B2P	= 0x080,
//	MODE_ANX_I2P	= 0x100,
//	MODE_ANX_J2P	= 0x200,
//	MODE_ANX_M2P	= 0x400,
	MODE_T1_424		= 0x800
};
#define MODE_ADSL1 (MODE_ANSI|MODE_ETSI|MODE_GDMT|MODE_GLITE)
#define MODE_MASK (MODE_ANSI|MODE_ETSI|MODE_GDMT|MODE_GLITE|MODE_ADSL2|MODE_ADSL2LITE|MODE_ADSL2PLUS|MODE_VDSL1|MODE_VDSL2)

/*use VDSL2_PROFILE_xx definition*/
#if 0
enum Vdsl2Profile 
{ 
	PROFILE_8A_MASK = 0x01,
	PROFILE_8B_MASK = 0x02,
	PROFILE_8C_MASK = 0x04,
	PROFILE_8D_MASK = 0x08,
	PROFILE_12A_MASK = 0x10,
	PROFILE_12B_MASK = 0x20,
	PROFILE_17A_MASK = 0x40,
	PROFILE_30A_MASK = 0x80,
};
#endif

//RLCM_GET_VDSL2_DIAG_OTHER VDSL2 struct
#define	MAX_NUM_BANDS_US	9
#define	MAX_NUM_BANDS_DS	9
typedef struct
{
	int ATTNDRds;
	int ATTNDRus;
	int ACTATPds;
	int ACTATPus;
	unsigned int HLINGds;
	unsigned int HLINGus;
	unsigned int HLOGGds;
	unsigned int HLOGGus;
	unsigned int HLOGMTds;
	unsigned int HLOGMTus;
	unsigned int QLNGds;
	unsigned int QLNGus;
	unsigned int QLNMTds;
	unsigned int QLNMTus;
	unsigned int SNRGds;
	unsigned int SNRGus;
	unsigned int SNRMTds;
	unsigned int SNRMTus;
	short SNRMds;
	short SNRMus;
	short LATNpbds[5];
	short LATNpbus[5];
	short SATNpbds[5];
	short SATNpbus[5];
	short SNRMpbds[5];
	short SNRMpbus[5];
	unsigned short NumOfBandsRx;
	unsigned short NumOfBandsTx;
	unsigned short  BandsTx_Start[MAX_NUM_BANDS_US];
	unsigned short  BandsTx_End[MAX_NUM_BANDS_US];
	unsigned short  BandsRx_Start[MAX_NUM_BANDS_DS];
	unsigned short  BandsRx_End[MAX_NUM_BANDS_DS];
	short HlinScale_ds;
	short HlinScale_us;
}VDSL2DiagOthers;

#define MAX_CARRIER_GROUP_NUM    512
typedef struct{
	short real;
	short imag;
}ComplexShort;
#endif /*CONFIG_VDSL*/


#ifdef __cplusplus
}
#endif
#endif // OBCIF_H

