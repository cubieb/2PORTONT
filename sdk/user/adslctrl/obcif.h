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
#define RLCM_GET_ADSL_CHANNEL_SNR			(RLCM_IOC_MAGIC + 7)
//#define RLCM_GET_AVERAGE_SNR			(RLCM_IOC_MAGIC + 8)
#define RLCM_GET_ADSL_SNR_MARGIN			(RLCM_IOC_MAGIC + 9)
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
#define RLCM_GET_ADSL_LOOP_ATT			(RLCM_IOC_MAGIC +25)
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
#define RLCM_GET_XDSL_TIME			(RLCM_IOC_MAGIC +39)
#define RLCM_PHY_START_MODEM			(RLCM_IOC_MAGIC +40)
#define RLCM_ENABLE_ADSL_LOG			(RLCM_IOC_MAGIC +41)
#define RLCM_DISABLE_ADSL_LOG			(RLCM_IOC_MAGIC +42)
//added command
#define RLCM_GET_VENDOR_ID			(RLCM_IOC_MAGIC +43)
#define RLCM_GET_TX_POWER			(RLCM_IOC_MAGIC +44)
#define RLCM_GET_PERF_VALUE			(RLCM_IOC_MAGIC +45)
#define RLCM_GET_15MIN_LOSS_DATA		(RLCM_IOC_MAGIC +46)
#define RLCM_GET_1DAY_LOSS_DATA			(RLCM_IOC_MAGIC +47)
#define RLCM_GET_ADSL_CHANNEL_BITLOAD		(RLCM_IOC_MAGIC +48)
//for MIB TRAP set
#define RLCM_GET_TRAP_THRESHOLD			(RLCM_IOC_MAGIC +49)
#define RLCM_SET_TRAP_THRESHOLD			(RLCM_IOC_MAGIC +50)
#define RLCM_15MIN_WAIT_TRAP			(RLCM_IOC_MAGIC +51)
//for ATM test
#define RLCM_ENABLE_ATM_LOOPBACK		(RLCM_IOC_MAGIC +52)
#define RLCM_DISABLE_ATM_LOOPBACK		(RLCM_IOC_MAGIC +53)
//new command 9/19/03'
#define RLCM_MSGMODE				(RLCM_IOC_MAGIC +54)
//new command 1/30/04' for API list
#define RLCM_CMD_API				(RLCM_IOC_MAGIC +55)
#define RLCM_GET_CURRENT_LOSS_DATA		(RLCM_IOC_MAGIC +56)
#define RLCM_GET_CHANNEL_BH			(RLCM_IOC_MAGIC +57)
//new command 4/20/04', for MIB trap information get
#define RLCM_GET_TRAP_15MIN_LOSS_DATA		(RLCM_IOC_MAGIC +58)
//tylo, 8671 new command
#define RLCM_SEND_DYING_GASP				(RLCM_IOC_MAGIC +59)
//#define RLCM_READ_D						(RLCM_IOC_MAGIC +60)
//#define RLCM_WRITE_D						(RLCM_IOC_MAGIC +61)
#define RLCM_TEST_HW						(RLCM_IOC_MAGIC +62)
#define RLCM_SET_LOOPBACK					(RLCM_IOC_MAGIC +63)
#define RLCM_INIT_ADSL_MODE				(RLCM_IOC_MAGIC +64)
#define RLCM_ENABLE_POM_ACCESS			(RLCM_IOC_MAGIC +65)
#define RLCM_DISABLE_POM_ACCESS			(RLCM_IOC_MAGIC +66)
#define RLCM_MASK_TONE						(RLCM_IOC_MAGIC +67)
#define RLCM_GET_CAPABILITY				(RLCM_IOC_MAGIC +68)
#define RLCM_VERIFY_HW						(RLCM_IOC_MAGIC +69)
//#define RLCM_TRIG_OLR_TYPE					(RLCM_IOC_MAGIC +70)
#define RLCM_TRIG_OLR_TYPE1				(RLCM_IOC_MAGIC +71)
//#define RLCM_ENABLE_AEQ					(RLCM_IOC_MAGIC +72)
#define RLCM_ENABLE_HPF					(RLCM_IOC_MAGIC +73)
//#define RLCM_SET_HPF_FC					(RLCM_IOC_MAGIC +74)
#define RLCM_SET_HYBRID					(RLCM_IOC_MAGIC +75)
#define RLCM_SET_RX_GAIN					(RLCM_IOC_MAGIC +76)
#define RLCM_SET_AFE_REG					(RLCM_IOC_MAGIC +77)
//#define RLCM_SET_FOBASE					(RLCM_IOC_MAGIC +78)
//yaru + for webdata
#define RLCM_SET_XDSL_MODE				(RLCM_IOC_MAGIC +79)
#define RLCM_GET_SHOWTIME_XDSL_MODE		(RLCM_IOC_MAGIC +80)
#define RLCM_GET_XDSL_MODE				(RLCM_IOC_MAGIC +81)
#define RLCM_SET_OLR_TYPE					(RLCM_IOC_MAGIC +82)
#define RLCM_GET_OLR_TYPE 					(RLCM_IOC_MAGIC +83)
#define RLCM_GET_LINE_RATE					(RLCM_IOC_MAGIC +84)
#define RLCM_GET_DS_ERROR_COUNT			(RLCM_IOC_MAGIC +85)
#define RLCM_GET_US_ERROR_COUNT			(RLCM_IOC_MAGIC +86)

#define RLCM_GET_ADSL_DIAG_QLN					(RLCM_IOC_MAGIC +87)
#define RLCM_GET_ADSL_DIAG_HLOG				(RLCM_IOC_MAGIC +88)
#define RLCM_GET_ADSL_DIAG_SNR					(RLCM_IOC_MAGIC +89)

#define RLCM_GET_XDSL_DS_PMS_PARAM1			(RLCM_IOC_MAGIC +90)
#define RLCM_GET_XDSL_US_PMS_PARAM1			(RLCM_IOC_MAGIC +91)
#define RLCM_SET_ANNEX_L					(RLCM_IOC_MAGIC +92)
#define RLCM_GET_ANNEX_L					(RLCM_IOC_MAGIC +93)
#define RLCM_GET_LINK_POWER_STATE		(RLCM_IOC_MAGIC +94)
#define RLCM_GET_ATT_RATE				(RLCM_IOC_MAGIC +95)
#define RLCM_LOADCARRIERMASK				(RLCM_IOC_MAGIC +96)
#define RLCM_SET_ANNEX_M					(RLCM_IOC_MAGIC +97)
#define RLCM_GET_ANNEX_M					(RLCM_IOC_MAGIC +98)
#define RLCM_SET_8671_REV					(RLCM_IOC_MAGIC +99)
#define RLCM_GET_8671_REV					(RLCM_IOC_MAGIC +100)
#define RLCM_SET_HIGH_INP					(RLCM_IOC_MAGIC +101)
#define RLCM_GET_HIGH_INP					(RLCM_IOC_MAGIC +102)
#define RLCM_GET_LD_STATE					(RLCM_IOC_MAGIC +103)
#define RLCM_SET_ANNEX_B					(RLCM_IOC_MAGIC +104)
#define RLCM_GET_ANNEX_B					(RLCM_IOC_MAGIC +105)


//for TR069
#define RLCM_GET_DSL_STAT_SHOWTIME		(RLCM_IOC_MAGIC +106)
#define RLCM_GET_DSL_STAT_TOTAL			(RLCM_IOC_MAGIC +107)
#define RLCM_GET_ADSL_PSD					(RLCM_IOC_MAGIC +108)
#define RLCM_GET_DSL_ORHERS				(RLCM_IOC_MAGIC +109)
#define RLCM_GET_DSL_GI					(RLCM_IOC_MAGIC +110)
//end for TR069
#define RLCM_SET_ADSL_LAST_OPMode		(RLCM_IOC_MAGIC +111)
#define RLCM_SET_ADSL_PMS_CONFIG			(RLCM_IOC_MAGIC +112)

//for TR069
#define RLCM_GET_ADSL2WAN_IFCFG			(RLCM_IOC_MAGIC +113)
#define RLCM_ENABLE_DIAGNOSTIC	(RLCM_IOC_MAGIC +114)
//end for TR069
//for FIELD_TRY_SAFE_MODE web control
#define RLCM_GET_SAFEMODE_CTRL		(RLCM_IOC_MAGIC +115)
#define RLCM_SET_SAFEMODE_CTRL		(RLCM_IOC_MAGIC +116)
//- end FIELD_TRY_SAFE_MODE web control

#define RLCM_GET_DSL_STAT_15MIN		(RLCM_IOC_MAGIC +117)
#define RLCM_GET_DSL_STAT_1DAY			(RLCM_IOC_MAGIC +118)

#define RLCM_WEB_SET_USPSD			(RLCM_IOC_MAGIC +154)
#ifdef TR105_SUPPORT
#define RLCM_GET_INIT_COUNT_LAST_LINK_RATE			(RLCM_IOC_MAGIC +155)
#define RLCM_ENABLE_NODROPLINEFLAG			(RLCM_IOC_MAGIC +156)
#define RLCM_DISABLE_NODROPLINEFLAG			(RLCM_IOC_MAGIC +157)
#endif

//yaru TR069 //yaru 20100521
#define RLCM_GET_DSL_STAT_LASTSHOWTIME		(RLCM_IOC_MAGIC +159)

//TR069_REV1P4_SUPPORT
#define RLCM_GET_DSL_STAT_SHOWTIME_REV1P4		(RLCM_IOC_MAGIC +160)
#define RLCM_GET_DSL_STAT_TOTAL_REV1P4			(RLCM_IOC_MAGIC +161)
#define RLCM_GET_DSL_STAT_LAST_SHOWTIME_REV1P4		(RLCM_IOC_MAGIC +162)
#define RLCM_GET_DSL_STAT_15MIN_REV1P4		(RLCM_IOC_MAGIC +163)
#define RLCM_GET_DSL_STAT_1DAY_REV1P4			(RLCM_IOC_MAGIC +164)
#define RLCM_GET_DSL_STAT_FLAG_LOF			(RLCM_IOC_MAGIC +165)
#define RLCM_GET_FRAME_COUNT			(RLCM_IOC_MAGIC +166)
#define RLCM_DBG_DATA (RLCM_IOC_MAGIC +167)
//---end TR069_REV1P4_SUPPORT
#define RLCM_GET_DSL_STAT_PRE_15MIN		(RLCM_IOC_MAGIC +180)

// Ben add 20140619 for manual set following info
#define RLCM_SET_ATUR_VENDORID		(RLCM_IOC_MAGIC+181)
#define RLCM_SET_ATUR_VERSIONNUM		(RLCM_IOC_MAGIC+182)
#define RLCM_SET_ATUR_SERIALNUM		(RLCM_IOC_MAGIC+183)


//VDSL specifics
//PY 20090114
#define RLCM_DEBUG_DATA				(RLCM_IOC_MAGIC +111+1000)
//PY 20090114 END


#define RLCM_GET_XDSL_WAN_IFCFG			(RLCM_IOC_MAGIC +113+1000)
#define RLCM_GET_VDSL2_MREFPSD_DS			(RLCM_IOC_MAGIC +114+1000)
#define RLCM_GET_VDSL2_TXREFVNPSD_DS			(RLCM_IOC_MAGIC +115+1000)
#define RLCM_GET_VDSL2_MREFPSD_US			(RLCM_IOC_MAGIC +116+1000)
#define RLCM_GET_VDSL2_TXREFVNPSD_US			(RLCM_IOC_MAGIC +117+1000)

#define RLCM_GET_ADSL_DIAG_HLIN			(RLCM_IOC_MAGIC +118+1000)
#define RLCM_GET_ADSL_DIAG_GI				(RLCM_IOC_MAGIC +119+1000)
#define RLCM_GET_ADSL_DIAG_BI				(RLCM_IOC_MAGIC +120+1000)
#define RLCM_GET_ADSL_DIAG_OTHER			(RLCM_IOC_MAGIC +121+1000)

#define RLCM_GET_VDSL2_DIAG_QLN			(RLCM_IOC_MAGIC +122+1000)
#define RLCM_GET_VDSL2_DIAG_HLOG			(RLCM_IOC_MAGIC +123+1000)
#define RLCM_GET_VDSL2_DIAG_SNR			(RLCM_IOC_MAGIC +124+1000)
#define RLCM_GET_VDSL2_DIAG_HLIN			(RLCM_IOC_MAGIC +125+1000)
#define RLCM_GET_VDSL2_DIAG_HLIN_SCALE			(RLCM_IOC_MAGIC +126+1000)
#define RLCM_GET_VDSL2_DIAG_OTHER			(RLCM_IOC_MAGIC +127+1000)
                                                  	
//#define RLCM_GET_VDSL2_DS_PMS_PARAM1			(RLCM_IOC_MAGIC +128+1000)
//#define RLCM_GET_VDSL2_US_PMS_PARAM1			(RLCM_IOC_MAGIC +129+1000)


#define RLCM_GET_VDSL2_SNR_MARGIN			(RLCM_IOC_MAGIC + 130+1000)
#define RLCM_GET_VDSL2_LOOP_ATT			(RLCM_IOC_MAGIC +131+1000)

#define RLCM_GET_VDSL2_CHANNEL_SNR			(RLCM_IOC_MAGIC + 132+1000)
#define RLCM_GET_VDSL2_CHANNEL_BITLOAD			(RLCM_IOC_MAGIC +133+1000)

#define RLCM_WANBonderDSLConfig_Enable   		(RLCM_IOC_MAGIC +137+1000)
#define RLCM_WANBonderDSLConfig_Status   		(RLCM_IOC_MAGIC +138+1000)
#define RLCM_WANBonderDSLConfig_GroupBondScheme   	(RLCM_IOC_MAGIC +139+1000)
#define RLCM_WANBonderDSLConfig_GroupCapacity   		(RLCM_IOC_MAGIC +140+1000)
#define RLCM_WANBonderDSLConfig_aGroupTargetUpRate   	(RLCM_IOC_MAGIC +141+1000)
#define RLCM_WANBonderDSLConfig_aGroupTargetDownRate   	(RLCM_IOC_MAGIC +142+1000)
#define RLCM_WANConfig_LineNumber 			(RLCM_IOC_MAGIC +143+1000)

#define RLCM_UserGetDslData 				(RLCM_IOC_MAGIC +144+1000)
#define RLCM_UserSetDslData 				(RLCM_IOC_MAGIC +145+1000)

#ifdef ZTE_API
#define RLCM_ZTE_INIT				(RLCM_IOC_MAGIC +146+1000)
#define RLCM_ZTE_GET_STATUS            (RLCM_IOC_MAGIC +147+1000)
#define RLCM_ZTE_UNINIT                                (RLCM_IOC_MAGIC +148+1000)
#define RLCM_ZTE_CONNECT                             (RLCM_IOC_MAGIC +149+1000)
#define RLCM_ZTE_SET_PARAM        (RLCM_IOC_MAGIC +159+1000)
#endif

//VDSL specifics END


//RLCM_SET_XDSL_MODE		
//RLCM_GET_SHOWTIME_XDSL_MODE	
//RLCM_GET_XDSL_MODE	
#define XDSL_MODE_SIZE (sizeof(int))
//RLCM_SET_OLR_TYPE
#define SET_OLR_TYPE_SIZE (sizeof(int))		
//RLCM_GET_OLR_TYPE 		
#define GET_OLR_TYPE_SIZE (sizeof(int)*2)
//RLCM_GET_LINE_RATE		
#define GET_LINE_RATE_SIZE (sizeof(short)*2)
#define GET_LOADCARRIERMASK_SIZE (sizeof(unsigned char)*64)
//RLCM_GET_DS_ERROR_COUNT
#define GET_DS_ERROR_COUNT_SIZE (sizeof(unsigned long)*5)		
//RLCM_GET_US_ERROR_COUNT		
#ifdef TR105_SUPPORT
#define GET_US_ERROR_COUNT_SIZE (sizeof(unsigned short)*5)
#else
#define GET_US_ERROR_COUNT_SIZE (sizeof(unsigned short)*6)
#endif

#ifdef TR105_SUPPORT
#define RLCM_GET_INIT_COUNT_LAST_LINK_RATE_SIZE (sizeof(unsigned long)*4)		
#endif

//for TR069
typedef struct
{
short real;
short imag;
}Modem_ComplexShort;
#define GET_ADSL_DIAG_QLN_SIZE	512*sizeof(short)	
#define GET_ADSL_DIAG_HLOG_SIZE 512*sizeof(short)		
#define GET_ADSL_DIAG_SNR_SIZE 512*sizeof(short)	
#define GET_ADSL_DIAG_HLIN_SIZE 512*sizeof(Modem_ComplexShort)	
#define GET_ADSL_DIAG_HLINSCALE_SIZE 2*sizeof(short)
#define GET_ADSL_DIAG_BIT_SIZE  512*sizeof(char)
#define GET_ADSL_DIAG_GI_SIZE  512*sizeof(short)
#define GET_ADSL_DIAG_OTHER_SIZE  13*sizeof(short)



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
unsigned short  BandsTx_Start[9];
unsigned short  BandsTx_End[9];
unsigned short  BandsRx_Start[9];
unsigned short  BandsRx_End[9];
}Modem_VDSL2DiagOthers;

#define GET_VDSL2_DIAG_QLN_SIZE	1024*sizeof(short)	
#define GET_VDSL2_DIAG_HLOG_SIZE 1024*sizeof(short)		
#define GET_VDSL2_DIAG_SNR_SIZE 1024*sizeof(short)	
#define GET_VDSL2_DIAG_HLIN_SIZE 1024*sizeof(Modem_ComplexShort)	
#define GET_VDSL2_DIAG_HLIN_SCALE_SIZE 1024*sizeof(short)

#define GET_VDSL2_DIAG_OTHER_SIZE  sizeof(Modem_VDSL2DiagOthers)

#define GET_LD_STATE_SIZE (sizeof(int))
//end of TR069
	
//#define GET_ADSL_PMS_SIZE (sizeof(unsigned short)*14)

/*
typedef struct
{
	unsigned char B_pn;
	unsigned char T_p;
	unsigned char G_p;
	unsigned char F_p;
	unsigned char M_p;
	unsigned short L_p;
	unsigned char R_p;
	unsigned char I_p;
	unsigned short D_p;
}Modem_XDSL_PMS;
*/

//#define GET_XDSL_PMS_SIZE (sizeof(Modem_XDSL_PMS)*2)
#define GET_XDSL_PMS_SIZE (sizeof(unsigned short)*7)//follow ADSL.lib
#define TR069_DSL_OTHER_SIZE (sizeof(unsigned int)*3)

#define GET_ANNEX_L_SIZE (sizeof(int))
#define GET_LINK_POWER_STATE_SIZE (sizeof(unsigned char))
#define GET_ANNEX_M_SIZE (sizeof(int))
#define GET_HIGH_INP_SIZE (sizeof(int))
#define GET_ANNEX_B_SIZE (sizeof(int))

//-yaru 
/*ioctl argument size or struct*/
#define RLCM_DRIVER_VERSION_SIZE	8

//for RLCM_GET_DRIVER_BUILD
#define RLCM_DRIVER_BUILD_SIZE		14

//for RLCM_GET_REHS_COUNT
#define RLCM_GET_REHS_COUNT_SIZE	4  //(sizeof(reHandshakeCount))

//for RLCM_GET_CHANNEL_BITLOAD
#define RLCM_GET_ADSL_CHANNEL_BITLOAD_SIZE	(sizeof(unsigned short)*512)  //(sizeof(gChannelBitLoad)) //yaru update
#define RLCM_GET_VDSL2_CHANNEL_BITLOAD_SIZE	(sizeof(unsigned short)*4096)  //(sizeof(gChannelBitLoad)) //yaru update

//for RLCM_GET_CHANNEL_SNR
#define RLCM_GET_ADSL_CHANNEL_SNR_SIZE	(sizeof(short)*512)  //(sizeof(gChannelSNR))
#define RLCM_GET_VDSL2_CHANNEL_SNR_SIZE	(sizeof(short)*4096)  //(sizeof(gChannelSNR))


//for RLCM_GET_AVERAGE_SNR
#define RLCM_GET_AVERAGE_SNR_SIZE	12*sizeof(short)  //(sizeof(gAverageSNR))

//for RLCM_GET_SNR_MARGIN
#define RLCM_GET_ADSL_SNR_MARGIN_SIZE	2*sizeof(short) //(sizeof(gitex_EstimatedSNRMargin))+FarEndLineOperData->noiseMarginUpstr

typedef struct {
short SNRMus;
short SNRMds;
short SNRM_PB_Us[5];
short SNRM_PB_Ds[5];
} Modem_Vdsl2SNRMpb;
#define RLCM_GET_VDSL2_SNR_MARGIN_SIZE	sizeof(Modem_Vdsl2SNRMpb) //(sizeof(gitex_EstimatedSNRMargin))+FarEndLineOperData->noiseMarginUpstr

//for RLCM_REPORT_MODEM_STATE
#define RLCM_REPORT_MODEM_STATE_SIZE	64 
/*#define MODEM_STATE_IDLE                0
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
#define MODEM_STATE_EXCHANGE            12*/

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
  signed short	 noiseMarginDnstr;
  signed char    outputPowerUpstr;
  unsigned char  attenuationDnstr;
  unsigned long operationalMode;
} Modem_NearEndLineOperData;
#define RLCM_MODEM_NEAR_END_LINE_DATA_REQ_SIZE sizeof(Modem_NearEndLineOperData)

//for RLCM_MODEM_FAR_END_LINE_DATA_REQ
//typedef struct {
//  unsigned short relCapacityOccupationUpstr;
//  signed short	 noiseMarginUpstr;
//  signed char	 outputPowerDnstr;
//  unsigned char	 attenuationUpstr;
//  unsigned char  carrierLoad[TONE_RANGE>>1];
//} Modem_FarEndLineOperData;
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
short upstreamAtt;
short downstreamAtt;
} Modem_AdslAvgLATN;
#define RLCM_GET_ADSL_LOOP_ATT_SIZE 	2*sizeof(short)

typedef struct {
short LTAN_PB_Us[5];
short LTAN_PB_Ds[5];
} Modem_Vdsl2LTANpb;
#define RLCM_GET_VDSL2_LOOP_ATT_SIZE 	sizeof(Modem_Vdsl2LTANpb)

#define RLCM_GET_CHANNEL_MODE_SIZE 	2*sizeof(char)

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

//yaru for TR069
typedef struct {
	char Enable;
	int Status;
	unsigned int LinkEncapSupported;
	unsigned int LinkEncapRequested;
	unsigned int LinkEncapUsed;
//	unsigned int ModulationType;
	unsigned int StandardsSuported;
	unsigned int StandardUsed;	
	unsigned int LineEncoding;
	unsigned int AllowedProfiles;
	unsigned int CurrentProfile;
	short PowerManagementState;
	unsigned int SuccessFailureCause;
	unsigned int LastStateTransmittedDownstream;
	unsigned int LastStateTransmittedUpstream;
	unsigned int UPBOKLE;
	#if 0
	gPSDDescriptorTable_t MREFPSDds;
	gPSDDescriptorTable_t MREFPSDus;
	#endif
	unsigned int LIMITMASK;
	unsigned int US0MASK;
	unsigned int DataPath;
	unsigned int InterDepth;
	unsigned int LPATH;
	int INTLVDEPTH;
	int INTLVBLOCK;
	unsigned int ActInterDelay;
	int ACTINP;
	unsigned int INPREPORT;
	int NFEC;
	int RFEC;
	int LSYMB;
	int TRELLISds;
	int TRELLISus;
	unsigned int ACTSNRMODEds;
	unsigned int ACTSNRMODEus;
	#if 0
	gPSDDescriptorTable_t VirtualNoisePSDds;
	gPSDDescriptorTable_t VirtualNoisePSDus;
	#endif
	
	unsigned int ACTUALCE;
	unsigned int LineNumber;
	unsigned int UpstreamCurrRate;
	unsigned int DownstreamCurrRate;
	unsigned int UpstreamMaxRate;
	unsigned int DownstreamMaxRate;
	int UpstreamNoiseMargin;
	int DownstreamNoiseMargin;
	int SNRMpbus[5];
	int SNRMpbds[5];
	unsigned int INMIATOds;
	unsigned int INMIATSds;
	unsigned int INMCCds;
	unsigned int INMINPEQMODEds;
	int UpstreamAttenuation;
	int DownstreamAttenuation;
	int UpstreamPower;
	int DownstreamPower;
	unsigned long  ATURVendor;
	unsigned char ATURCountry;
	unsigned int ATURANSIStd;
	unsigned int ATURANSIRev;
	unsigned long ATUCVendor;
	unsigned char ATUCCountry;
	unsigned int ATUCANSIStd;
	unsigned int ATUCANSIRev;
	#if 1
	unsigned int TotalStart;
	unsigned int ShowtimeStart;
	unsigned int LastShowtimeStart;
	unsigned int CurrentDayStart;
	unsigned int QuarterHourStart;
	#endif
} Modem_DSLWANConfig;

#define TR069_XDSL_WANCFG_SIZE sizeof(Modem_DSLWANConfig)


#define TR069_ADSL2WANCFG_SIZE sizeof(T_ADSL2WANConfig)


typedef struct
{
	unsigned short ToneIndex;
	short	PSD_level;
}Modem_PSDRecord;

typedef struct
{
	unsigned short	NumberOfTone;
	Modem_PSDRecord	PSDRecord[64];
} Modem_PSDDescriptorTable;

#define TR069_VDSL2_PSD_SIZE	sizeof(Modem_PSDDescriptorTable)
#define TR069_ADSL_PSD_SIZE	2*sizeof(char)

#define TR069_SUPPORT
#define TR069_REV1P4_SUPPORT
#define ATandT_WT157

typedef struct {
	unsigned int	ReceiveBlocks;
	unsigned int	TransmitBlocks;
	unsigned int	CellDelin;
	unsigned int	LinkRetain;
	unsigned int	InitErrors;
	#ifdef TR069_REV1P4_SUPPORT
	unsigned int LInit;  
	#endif
	unsigned int	InitTimeouts;
	unsigned int   LossOfFraming;
	#ifdef TR069_REV1P4_SUPPORT
	unsigned int	LOF;
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
	unsigned long	LossOfMargin;		//LOM
	unsigned long	LossOfPower;		//LPR
	unsigned long	LossOfSignal;	//LOS
	unsigned long UnavailableSec; //unavailable second count
	#endif
}Modem_DSLConfigStatus;	//yaru TR069, copy T_DSLConfigStatus
#define TR069_STAT_SIZE sizeof(Modem_DSLConfigStatus)
//end for TR069


#define USPSDDATA_DATA_SIZE sizeof(UsPSDData)

#ifdef __cplusplus
}
#endif

#endif // OBCIF_H

