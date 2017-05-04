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
#define RLCM_IOC_MAGIC	(('R'+'L'+'C'+'M'+'a'+'d'+'s'+'l') << 8)  //0x2fe00
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

/*ioctl argument size or struct*/
//for RLCM_GET_DRIVER_VERSION
#define RLCM_DRIVER_VERSION_SIZE	6

//for RLCM_GET_DRIVER_BUILD
#define RLCM_DRIVER_BUILD_SIZE		14

//for RLCM_GET_REHS_COUNT
#define RLCM_GET_REHS_COUNT_SIZE	4  //(sizeof(reHandshakeCount))

//for RLCM_GET_CHANNEL_BITLOAD
#define RLCM_GET_CHANNEL_BITLOAD_SIZE	(sizeof(short)*256)  //(sizeof(gChannelBitLoad))

//for RLCM_GET_CHANNEL_SNR
#define RLCM_GET_CHANNEL_SNR_SIZE	(sizeof(short)*256)  //(sizeof(gChannelSNR))

//for RLCM_GET_AVERAGE_SNR
#define RLCM_GET_AVERAGE_SNR_SIZE	4  //(sizeof(gAverageSNR))

//for RLCM_GET_SNR_MARGIN
#define RLCM_GET_SNR_MARGIN_SIZE	8  //(sizeof(gitex_EstimatedSNRMargin))+FarEndLineOperData->noiseMarginUpstr

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
  signed char	 noiseMarginDnstr;
  signed char    outputPowerUpstr;
  unsigned char  attenuationDnstr;
  unsigned long operationalMode;
} Modem_NearEndLineOperData;
#define RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE sizeof(Modem_NearEndLineOperData)

//for RLCM_MODEM_FAR_END_LINE_DATA_REQ
typedef struct {
  unsigned short relCapacityOccupationUpstr;
  signed char	 noiseMarginUpstr;
  signed char	 outputPowerDnstr;
  unsigned char	 attenuationUpstr;
  unsigned char  carrierLoad[128];
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

#ifdef __cplusplus
}
#endif

#endif // OBCIF_H

