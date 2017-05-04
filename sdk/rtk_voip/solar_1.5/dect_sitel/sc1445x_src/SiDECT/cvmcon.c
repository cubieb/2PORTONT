/****************************************************************************
*  Program/file: cvmcon.cpp
*
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LJA
*
*  MODULE:
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*
*  DESCRIPTION:
*
****************************************************************************/
/****************************************************************************
* Compilation switches
****************************************************************************/
#include "../common/operation_mode_defs.h"
#ifndef OLD_CC
#ifdef CONFIG_LMX4180_DECT_SUPPORT // only natalie
	#define DECT_NATALIE
#else
	 #define CVM480
#endif

#if ((defined CONFIG_SC1445x_DECT_SUPPORT) || (!(defined CONFIG_SC1445x_DECT_HEADSET_SUPPORT))) //Any dect
	#define DECT_DESIGN
#endif

#if ((defined CONFIG_SC1445x_LEGERITY_890_SUPPORT) && (defined CONFIG_SC1445x_DECT_SUPPORT))
	#define ATA_DECT_DESIGN
#endif

#ifdef DECT_HEADSET_ENABLED 
	#define DECT_HEADSET_DESIGN
#endif

#ifdef CONFIG_SC1445x_LEGERITY_890_SUPPORT
	#define ATA_DESIGN
#endif

#endif


#define full_mmi
#ifndef DECT_HEADSET_DESIGN
	#define REGISTRATION_BUTTON //Enable the use of a button for setting registration mode on / deleting handset registrations
#endif
#define FIX_NATALIE_BUG
#define USE_DSP_TRANSCODING 
//#define DYNAMIC_CHANNEL_ALLOCATION //allocates dynamically pcm channels and puts limit to max allocated pcm channels
#ifdef DECT_NATALIE
	#define LED_GPIO_SUPPORT //adds support for controlling LEDs using GPIOs
#endif
#define CLIP_SUPPORT 
//#define CALLING_NAME //causes problems to RTX and SIEMENS s60 handsets
/****************************************************************************
* Include files
****************************************************************************/
#include <time.h>
#include <stdio.h>
//#include <conio.h>
#include <stdlib.h>
#include <unistd.h>	// close 
#include <signal.h>	// signal
#include <sys/signal.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
//vm
//#include <windows.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<stdbool.h>
#include <termios.h>
#ifndef ATA_DECT_DESIGN
#include "std-def.h"
#endif
#ifdef CVM480
	#include "primitiv.h"
	#include "api-intf.h"
	#include "hdlc.h"
#endif

#ifndef mips
#include "regs_sc14450.h"
#endif
//#include <asm/config.h>
#include "si_linux_autoconf.h"
#ifdef full_mmi
#ifdef OLD_CC
	#include "../common/si_callcntrl_api.h"
#else
  #include "cvmcon.h"
  #include "si_dect.h"
  #include "si_dect_callcontrol.h"
#endif
#endif

#ifdef DECT_NATALIE
	#include "ISip450Api.h"
	#include "natalie_app.h"
	#include "MailLog.h"
#endif //DECT_NATALIE

#include "si_dect_manager.h"

/****************************************************************************
* Defines
****************************************************************************/
/*General*/
#define FALSE 0
#define TRUE 1
#define INVALID_HANDLE_VALUE -1
#define ERROR_SUCCESS 0

//Registration mode tmouts
#define REGISTRATION_MODE_KEY_PRESS_TMOUT	2 	//Secs that the reg. key should be pressed for entering REGISTRATION_MODE
#define DELETE_ALL_MODE_KEY_PRESS_TMOUT		8 	//Secs that the reg. key should be pressed for entering DELETE_ALL_MODE
#define FWU_UPGRADE_MODE_KEY_PRESS_TMOUT	15	//Secs that the reg. key should be pressed for entering FWU_UPGRADE_MODE

#define REGISTRATION_MODE_TMOUT				90	//Secs after the reg. key press where the mode returns from REGISTRATION_MODE to NORMAL_MODE
#define DELETE_MODE_TMOUT					30	//Secs waiting in REGISTRATION_MODE for deleting a handset
#define FWU_UPGRADE_MODE_TMOUT				150 //Secs waiting in FWU_UPGRADE_MODE for upgrading firmware
//Firmware upgrade file
//#define FWU_FILENAME "/mnt/flash/cvm.fwu\0"
#define FWU_FILENAME "/mnt/nfs/dect/Cvm480Fp_v0113.fwu"
#define FWU_FILENAME_SEC "/mnt/nfs/dect/Cvm480Fp_v0112.fwu"


//Progress bar
#define PROGRESS_BAR_SIZE 40

//Max allowed registered handsets 
#ifdef DECT_HEADSET_DESIGN
	#define MAX_REGISTERED_HS	1 //The max handsets we want registered in our application
#else
	#define MAX_REGISTERED_HS	6 //The max handsets we want registered in our application
	//#define MAX_REGISTERED_HS	CONFIG_RTK_VOIP_DECT_SITEL_HS_NR
#endif
#define SUPPORTED_HS		6 //DO NOT CHANGE THIS !!!! (This is the max registered handsets supported by the hw

/*Handset mapping to users*/
#define BROADCAST_HS 255
#ifdef ATA_DECT_DESIGN
	#define HS_APP_OFFSET ATA_DEV_CNT//0  //the offset of the handset representation for the application (e.g 2 means that line_id = handset id - 1 +2)
#else
	#define HS_APP_OFFSET 0//-1  //the offset of the handset representation for the application (e.g 2 means that line_id = handset id - 1 +2)
#endif
#define LINE_TO_HS(X) (X==BROADCAST_HS)?(BROADCAST_HS):(X + 1 - HS_APP_OFFSET) //the transformation of line_id to handset_id
#define HS_TO_LINE(X) (X==BROADCAST_HS)?(BROADCAST_HS):(X - 1 + HS_APP_OFFSET) //the transformation of handset_id to line_id
#ifndef CVM480
#define ADPCM_TO_APP(X) (X + HS_APP_OFFSET) //the transformation of ADPCM channel to the representation of the adpcm channel by the application
#else
#define ADPCM_TO_APP(X) (X) //the transformation of ADPCM channel to the representation of the adpcm channel by the application
#endif/*Handset tones*/
#define HS_RING_TONE ASV_ALERTING_ON_PATTERN2 //ASV_ALERTING_OFF //ASV_ALERTING_ON_PATTERN1 //ASV_ALERTING_OFF

/*Star codes. Replace * with .  and # with null*/
#define STAR_CODE_HOLD ".80" //AP_CALLCNTRL_CALL_HOLD_REQ

/*EEPROM map*/
#ifdef DECT_NATALIE
#define RFPI_EEPROM_ADDR 0
#else
#define RFPI_EEPROM_ADDR 0x10
#endif

/*Led gpios*/
#ifdef LED_GPIO_SUPPORT
	#ifdef CONFIG_SC14452
		#ifndef CONFIG_SC14452_ES2 //this is for ES3 version
			#define LED_REG_ON	*(volatile unsigned short*)0xFF48B0 = 0x300;\
								*(volatile unsigned short*)0xFF4892 = 0x100;
			#define LED_REG_OFF	*(volatile unsigned short*)0xFF48B0 = 0;
		
			#define LED_DEL_ON *(volatile unsigned short*)0xFF48B0 = 0x300;\
								*(volatile unsigned short*)0xFF4894 = 0x100;
			#define LED_DEL_OFF	*(volatile unsigned short*)0xFF48B0 = 0;
		
			#define LED_DEL_MODE_SET 
			#define LED_REG_MODE_SET 
		#else //not ES3 version
			#define LED_REG_ON	
			#define LED_REG_OFF	
			#define LED_DEL_ON 
			#define LED_DEL_OFF	
			#define LED_DEL_MODE_SET 
			#define LED_REG_MODE_SET 
		#endif
	#else
		#define LED_REG_ON *(volatile unsigned short*)0xFF4834 = 0x80;
		#define LED_REG_OFF	*(volatile unsigned short*)0xFF4832 = 0x80;
		#define LED_DEL_ON *(volatile unsigned short*)0xFF4864 = 0x2000;
		#define LED_DEL_OFF *(volatile unsigned short*)0xFF4862 = 0x2000;
		#define LED_DEL_MODE_SET *(volatile unsigned short*)0xFF488A = 0x300;
		#define LED_REG_MODE_SET *(volatile unsigned short*)0xFF484E = 0x300;
	#endif
#endif
/****************************************************************************
* Macro Definitions
****************************************************************************/
#define NO_HS 6
#define USER_TASK      0x0F // for standard API calls
#define CRC16_INIT_VALUE 0xFFFF

#define BUSM_SendMailP0(pid,tid,prim_h)          rsx_SendMailP0(tid,prim_h)
#define BUSM_SendMailP1(pid,tid,prim_h,p1)       rsx_SendMailP1(tid,prim_h,p1)
#define BUSM_SendMailP2(pid,tid,prim_h,p1,p2)    rsx_SendMailP2(tid,prim_h,p1,p2)
#define BUSM_SendMailP3(pid,tid,prim_h,p1,p2,p3) rsx_SendMailP3(tid,prim_h,p1,p2,p3)

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define RemoveEntryList(Entry) {\
    pListEntryType _EX_Blink;\
    pListEntryType _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

#define InsertTailList(ListHead,Entry) {\
    pListEntryType _EX_Blink;\
    pListEntryType _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

/****************************************************************************
* Typedefinitions
****************************************************************************/
#ifdef DECT_NATALIE
	/*Typedefs*/
	#define CvmApiMailType Sip450ApiMailType
	#define u16 unsigned short
#endif //DECT_NATALIE

/****************************************************************************
* Global Variables
****************************************************************************/
//uint16               Comport = 9;  // Default COM1:
int32                debug_ProgramId;
uint8                *bConPtr;
uint8                ConTask;
uint16               ConLength_h;
PcmCfgModeType       pcmcfgmode = PCM_CFG_FALSE;
FILE                 *pcmcfgfile;
unsigned char key_seq[40];
unsigned char key_seq_index = 0;
#if 0
PcmConfigType        PcmConfig = {APE_ENABLED,APM_MASTER,AP_FSC_FREQ_16KHZ,AP_FSC_LENGTH_16,APF_START_ALIGNED,AP_PCM_CLK_1152,AP_CLK_MODE_RISING};
CallStateType        CallState[NO_HS] = {HS_NOT_REGISTERED,HS_NOT_REGISTERED,HS_NOT_REGISTERED,HS_NOT_REGISTERED,HS_NOT_REGISTERED,HS_NOT_REGISTERED};
ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_0,AP_CHANNEL_1,AP_CHANNEL_2,AP_CHANNEL_3,AP_CHANNEL_3,AP_CHANNEL_3};
ApiPcmDataFormatType PcmDataFormat[NO_HS] = {AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz};
#endif

#if 1
	#if ((defined ATA_DECT_DESIGN) && !(defined DECT_NATALIE))
		//When using ata, FSYNC only in 8Khz
		#define WIDEBAND_PCM_CODEC		AP_DATA_FORMAT_CWB_ULAW//AP_DATA_FORMAT_CWB_ALAW 		//wideband in 8Khz fsync
		#define NARROWBAND_PCM_CODEC	AP_DATA_FORMAT_LINEAR_8kHz		//narrowband in 8Khz fsync

		PcmConfigType        PcmConfig = {APE_ENABLED,APM_SLAVE,AP_FSC_FREQ_8KHZ,AP_FSC_LENGTH_1,APF_START_1_BIT_BEFORE,AP_PCM_CLK_1152,AP_CLK_MODE_RISING};
		ApiPcmDataFormatType PcmDataFormat[NO_HS] = {AP_DATA_FORMAT_LINEAR_16kHz,AP_DATA_FORMAT_LINEAR_16kHz,AP_DATA_FORMAT_LINEAR_16kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz};
	#else
		//When not using ata, FSYNC only in 16Khz
		//#define WIDEBAND_PCM_CODEC		AP_DATA_FORMAT_LINEAR_16kHz		//wideband in 16Khz fsync
		#define WIDEBAND_PCM_CODEC		AP_DATA_FORMAT_CWB_ALAW		//wideband in 8Khz fsync
		//#define NARROWBAND_PCM_CODEC	AP_DATA_FORMAT_LINEAR_8kHz		//narrowband in 16Khz fsync
		#define NARROWBAND_PCM_CODEC	AP_DATA_FORMAT_G711A		//narrowband in 8Khz fsync

		//PcmConfigType        PcmConfig = {APE_ENABLED,APM_SLAVE,AP_FSC_FREQ_16KHZ,AP_FSC_LENGTH_1,APF_START_1_BIT_BEFORE,AP_PCM_CLK_1152,AP_CLK_MODE_RISING};
		PcmConfigType        PcmConfig = {APE_ENABLED,APM_SLAVE,AP_FSC_FREQ_8KHZ,AP_FSC_LENGTH_1,APF_START_1_BIT_BEFORE,AP_PCM_CLK_1152,AP_CLK_MODE_RISING};
		ApiPcmDataFormatType PcmDataFormat[NO_HS] = {AP_DATA_FORMAT_LINEAR_16kHz,AP_DATA_FORMAT_LINEAR_16kHz,AP_DATA_FORMAT_LINEAR_16kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz,AP_DATA_FORMAT_LINEAR_8kHz};
	#endif
#endif
		
/*PCM CHANNEL Configuration*/
#ifdef 	DYNAMIC_CHANNEL_ALLOCATION		
	#if (defined CONFIG_SC14452 && defined DECT_NATALIE)//4 channels		
		#define MAX_PCM_CHANNEL AP_CHANNEL_3 //the max pcm channel that can be allocated
	#else
		#define MAX_PCM_CHANNEL AP_CHANNEL_2 //the max pcm channel that can be allocated		
	#endif	
		
	#if (defined ATA_DECT_DESIGN && !(defined DECT_NATALIE))
		#if (ATA_DEV_CNT == 1) //1 ATA
			uint8 PcmChannelAllocation = 1; //this is the bitwise representation of PcmChannel allocation
		#elif 	(ATA_DEV_CNT == 2) //2 ATAs
			uint8 PcmChannelAllocation = 3; //this is the bitwise representation of PcmChannel allocation
		#endif
	#else 		
		uint8 PcmChannelAllocation = 0; //this is the bitwise representation of PcmChannel allocation
	#endif	
	//#define free_pcm_channel(pcm_channel_id)  {(PcmChannelAllocation&= ~(1<<pcm_channel_id));PRINT_MESSAGE("Releasing pcm channel %d, PcmChannelAllocation: %d \n", pcm_channel_id, PcmChannelAllocation);}
	#define try_pcm_channel_allocation  (PcmChannelAllocation<((1<<(MAX_PCM_CHANNEL+1))-1))
	#define bind_pcm_channel(pcm_channel_id)  (PcmChannelAllocation |= (1<<pcm_channel_id))
	//	allocate_pcm_channel is used for binding the first available pcm channel. Since it is more complex and is defined as function 
	ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_INVALID,AP_CHANNEL_INVALID,AP_CHANNEL_INVALID,AP_CHANNEL_INVALID,AP_CHANNEL_INVALID,AP_CHANNEL_INVALID};				
#else		
	//When using DECT HANDSET, support only 1 handset in PCM channel 2 
	#ifdef DECT_HEADSET_DESIGN
		ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_0,AP_CHANNEL_1,AP_CHANNEL_2,AP_CHANNEL_3,AP_CHANNEL_3,AP_CHANNEL_3};
	#elif defined DECT_NATALIE
		//ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_0,AP_CHANNEL_0,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_0};
		ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_0,AP_CHANNEL_0,AP_CHANNEL_0,AP_CHANNEL_0,AP_CHANNEL_0,AP_CHANNEL_0};
	#elif defined ATA_DECT_DESIGN 
		#if (ATA_DEV_CNT == 1) //1 ATA
			ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_1,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2};
		#elif 	(ATA_DEV_CNT == 2) //2 ATAs
			ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2};
		#endif
	#else //DECT_DESIGN
		//ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_0,AP_CHANNEL_1,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2,AP_CHANNEL_2};
		ApiPcmChannelIdType  HandsetPcmChannel[NO_HS] = {AP_CHANNEL_0,AP_CHANNEL_1,AP_CHANNEL_2,AP_CHANNEL_3,AP_CHANNEL_3,AP_CHANNEL_3};
	#endif
	const uint8 PcmChannelAllocation = 0;
#endif		


/*Call control variables*/
CallStateInfoType	CallState[NO_HS];
static ListEntryType ContextList;
static FileDataType FwuFileInfo_Buf;	// pkshih: keep it to be mail type 
static FileDataType *FwuFileInfo = NULL;
static char FWU_FILENAME_buffer[256] = { FWU_FILENAME };	// pkshih: we move out this buffer from FwuFileInfo_Buf
int FwuFileOpen = 0;
//volatile uint8 line_connect = 0;
volatile uint8 line_codec = 0;
volatile uint8 new_codec = 0;
volatile uint8 go_on_with_ata = 0;

/*Registration button variables*/
uint8 RegisteredHS = 0;
pthread_t check_registration_button_thread;
bool stop_check_registration_button_thread;
volatile unsigned registration_key_press_tmouts = 0;
volatile unsigned registration_mode = NORMAL_MODE;
volatile unsigned registration_in_progress_tmouts = 0;

/*Variables and definitions for setting the RFPI*/
#ifdef SET_RFPI
	unsigned char RFPI[5];//current rfpi of the FP
	unsigned char CC_RFPI[5]={0x00, 0x00, 0x00, 0x00, 0x00};//requested rfpi by the CC

//	#define check_rfpi_change (memcmp(RFPI, CC_RFPI, 5)!=0)
	#define check_rfpi_emc_fpn_change ( ((CC_RFPI[0]& 0x0f) != (RFPI[0]& 0x0f)) || (CC_RFPI[1]!= RFPI[1]) || (CC_RFPI[2]!= RFPI[2]) || (CC_RFPI[3]!= RFPI[3]) || ((CC_RFPI[4] & 0xf8)!= (RFPI[4] & 0xf8)))//check if emc or fpn fields differ
#endif

/* global variables for reference */
unsigned long ref_VersionHex = 0;
unsigned char ref_AccessCode[ 2 ] = { 0, 0 };	// BCD code 


/****************************************************************************
* Constant vars
****************************************************************************/
const ApiCodecInfoType CodecG722 = {ACT_G722, AMDS_1_MD, ACPR_CS, ASS_LS640};
const ApiCodecInfoType CodecG726 = {ACT_G726, AMDS_1_MD, ACPR_CS, ASS_FS};

static const uint16 Crc16Tbl[256] =
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
  0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
  0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
  0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
  0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
  0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
  0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
  0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
  0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
  0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
  0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
  0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
  0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
  0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
  0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
  0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
  0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
  0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
  0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
  0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
  0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
  0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
  0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
  0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
  0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
  0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
  0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
  0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
  0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

//#ifdef DECT_NATALIE
const char* ApiPcmEnableIdName[]= {
  "APE_DISABLE",
  "APE_ENABLED"
};

const char* ApiPcmMasterSlaveIdName[] = {
  "APM_SLAVE",
  "APM_MASTER",
  "APM_SLAVE_NO_CLOCK_SYNC"
};

const char* ApiPcmFscFreqIdName[] = {
  "AP_FSC_FREQ_8KHZ",
  "AP_FSC_FREQ_16KHZ"
};

const char* ApiPcmFscLengthIdName[] = {
  "AP_FSC_LENGTH_1",
  "AP_FSC_LENGTH_8",
  "AP_FSC_LENGTH_16",
  "AP_FSC_LENGTH_32"
};

const char* ApiPcmFscStartIdName[] = {
  "APF_START_1_BIT_BEFORE",
  "APF_START_ALIGNED"
};

const char* ApiPcmClkIdName[] = {
  "AP_PCM_CLK_1152",
  "AP_PCM_CLK_2304",
  "AP_PCM_CLK_4608"
};

const char* ApiPcmClkModeIdName[] = {
  "AP_CLK_MODE_RISING",
  "AP_CLK_MODE_FALLING"
};

const char* ApiCodecTypeName[] = {
  "ACT_NONE",
  "ACT_USER_SPECIFIC_32",
  "ACT_G726",
  "ACT_G722",
  "ACT_G711A",
  "ACT_G711U",
  "ACT_G7291",
  "ACT_MP4_32",
  "ACT_MP4_64",
  "ACT_USER_SPECIFIC_64",
  "ACT_INVALID"
};


//#endif //DECT_NATALIE
/****************************************************************************
* Function prototypes
****************************************************************************/
void FwuCloseFile(void);
void FwuOpenFile(void);
static void *check_registration_button_thread_function( void* unused );
void ModifyCodec(unsigned char handsetId, unsigned char req_codec);
#ifdef OLD_CC
void HandleCodecInfoInd(callcntrl_ap_msg_u_type* msg_rx );
void HandleOutgoingCallAnsweredInd(callcntrl_ap_msg_u_type* msg_rx );
#endif
void print_progress_bar(unsigned char *title_string, unsigned int current, unsigned int total, unsigned int bar_size);
#ifdef DYNAMIC_CHANNEL_ALLOCATION
	uint8 allocate_pcm_channel();
	void free_pcm_channel(uint8 pcm_channel_id);
	void display_pcm_channel_status();	
#endif
#ifdef SET_RFPI
	void SetRfpi(void);
#endif

extern void _sc1445x_dect_terminal_outgoing_req(ccfsm_attached_entity attachedentity, unsigned char *dialnumber, int portid, int accountid, ccfsm_codec_type codec);
extern void _sc1445x_dect_terminal_audio_peripheral_change(ccfsm_attached_entity attachedentity, int port, int accountid, ccfsm_audio_peripheral_status hookstatus, int codec, unsigned char appliedtocall);
extern int _sc1445x_dect_terminal_get_key_seq(ccfsm_attached_entity attachedentity, char *key_seq, int portid, int accountid, ccfsm_codec_type codec);

/****************************************************************************
* Exported (not nice)
****************************************************************************/
void mail_switch(uint16 Length, uint8 *MailPtr);

/****************************************************************************
* Externals (not nice)
****************************************************************************/

#ifdef CVM480
extern void hdlc_Exit(int dummy);
extern void hdlc_Init(void);
extern void InitTestBus(void);
extern void hdlc_SendPacketToUnit(int iTaskId, int iLength, unsigned char *bInputDataPtr);
//extern void sendmail_ReceivePacket(uint16 Length, uint8 *bDataPtr);
extern bool hdlc_Busy(void);

/****************************************************************************
* Implementation (subfunctions)
****************************************************************************/
extern void rsx_SendMail(uint32 iTaskId, uint32 iLength_h, uint8 *bDataPtr)
{
  hdlc_SendPacketToUnit(iTaskId, iLength_h, bDataPtr);
}

extern void rsx_SendMailP0(uint32 iTaskId, PrimitiveType Primitive_h)
{
  PrimitiveType Primitive_m = H2M_16( Primitive_h );
  
  hdlc_SendPacketToUnit(iTaskId, sizeof(PrimitiveType), (uint8*) &Primitive_m);
}

extern void rsx_SendMailP1(uint32 iTaskId, PrimitiveType Primitive_h, uint8 bParm1)
{
   recSendMailP1Type tmp;
   tmp.PrimitiveIdentifier = H2M_16( Primitive_h );
   tmp.bParm1 = bParm1;

   hdlc_SendPacketToUnit(iTaskId, sizeof(recSendMailP1Type), (uint8*) &tmp);
}

extern void rsx_SendMailP2(uint32 iTaskId, PrimitiveType Primitive_h, uint8 bParm1, uint8 bParm2)
{
   recSendMailP2Type tmp;
   tmp.PrimitiveIdentifier = H2M_16( Primitive_h );
   tmp.bParm1 = bParm1;
   tmp.bParm2 = bParm2;

   hdlc_SendPacketToUnit(iTaskId, sizeof(recSendMailP2Type), (uint8*) &tmp);
}

extern void rsx_SendMailP3(uint32 iTaskId, PrimitiveType Primitive, uint8 bParm1, uint8 bParm2, uint8 bParm3)
{
   recSendMailP3Type tmp;
   tmp.PrimitiveIdentifier = H2M_16( Primitive );
   tmp.bParm1 = bParm1;
   tmp.bParm2 = bParm2;
   tmp.bParm3 = bParm3;

   hdlc_SendPacketToUnit(iTaskId, sizeof(recSendMailP3Type), (uint8*) &tmp);
}
#endif //CVM480
// vm

// int mygetch( ) {
	// struct termios oldt,
	// newt;
	// int ch;
	// tcgetattr( STDIN_FILENO, &oldt );
	// newt = oldt;
	// newt.c_lflag &= ~( ICANON | ECHO );
	// tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	// ch = getchar();
	// tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	// return ch;
// }

uint8 *BUSM_AllocateMail(uint8 uProgramId, uint8 uTaskId, uint16 u16Size_h)
{
   ConTask = uTaskId;
   ConLength_h = u16Size_h;
   bConPtr = (uint8 *)malloc(u16Size_h);
   return(bConPtr);
}

void BUSM_DeliverMail(uint8 *Mail)
{
   rsx_SendMail(ConTask, ConLength_h, bConPtr);
   free(bConPtr);
}


/****************************************************************************
*                                Implementation
****************************************************************************/
void CloseFileMap(FileMapDataType* p)
/****************************************************************************
* DESCRIPTION:
****************************************************************************/
{
	//vm
#if 1
	if (p->pMap != NULL)
	{
//		munmap(p->pMap, p->Size);
		munmap(p->pMap, 0);
		//    UnmapViewOfFile(p->pMap);
		p->pMap = NULL;
	}
	p->hMap = NULL;
	//  if (p->hMap != NULL)
	//  {
	//    CloseHandle(p->hMap);
	//    p->hMap = NULL;
	//  }
	if (p->hFile != INVALID_HANDLE_VALUE)
	{
		//    CloseHandle(p->hFile);
		close(p->hFile);
		p->hFile = INVALID_HANDLE_VALUE;
	}
#endif //vm
}

uint32 OpenFileMap(const char* FileName, FileMapDataType* p)
/****************************************************************************
* DESCRIPTION:
****************************************************************************/
{//vm
#if 1
  uint32 res = ERROR_SUCCESS;
  //uint32 n;
  struct stat info;
  p->hFile = INVALID_HANDLE_VALUE;
  p->hMap = NULL;
  p->pMap = NULL;
//  p->hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  p->hFile = open(FileName, O_RDWR);	//|O_CREAT
  if (p->hFile == INVALID_HANDLE_VALUE)
  {
//    res = GetLastError();
//    PRINT_MESSAGE("Cannot open %s: 0x%X\n", FileName, res);
//    CloseFileMap(p);
//    return res;
	  perror("Open: ");
	  return(1);
  }

  if (fstat(p->hFile, &info) == -1) {
	  perror("fstat: ");
	  return 2;
  }
  
  //GetFileSize(p->hFile, &n);
  p->Size = info.st_size;
  
  //p->hMap = CreateFileMapping(p->hFile, NULL, PAGE_READONLY, n, p->Size, NULL);
  p->hMap = mmap(NULL, info.st_size, PROT_READ, MAP_PRIVATE, p->hFile, 0);
  if (p->hMap == NULL)
  {
//    res = GetLastError();
//    PRINT_MESSAGE("Cannot create map for %s: 0x%X\n", FileName, res);
	  CloseFileMap(p);
//    return res;
	  perror("mmap: ");
	  return 3;
  }
//  DEBUG_PRINT("mmap returns %x \n", p->hMap);  
//  DEBUG_PRINT("First bytes of fw file \n");
//  int i;
//  for(i=0; i<40;i++){
//	  DEBUG_PRINT("byte[%d]=0x%x \n", i, p->hMap[i]);
//  }
  p->pMap = p->hMap; // no mapview function is needed 
//  p->pMap = MapViewOfFile(p->hMap, FILE_MAP_READ, 0, 0, 0);
//  if (p->pMap == NULL)
//  {
//    res = GetLastError();
//    PRINT_MESSAGE("Cannot map view of %s: 0x%X\n", FileName, res);
//    CloseFileMap(p);
//    return res;
//  }
  return res;
#endif //vm
  return 0;
}



void SetCallState(uint8 HandsetId, CallStateType State)
{
  CallState[HandsetId].State = State;
  if (State == F00_NULL)
  {
    CallState[HandsetId].CallClass = 0;
    CallState[HandsetId].Connected2Line = FALSE;
//    LineSimRelease(CallState[HandsetId].LineId);
    if (CallState[HandsetId].ProposedCodecListLength > 0)
    {
      free(CallState[HandsetId].ProposedCodecListPtr);
      CallState[HandsetId].ProposedCodecListLength = 0;
    }
	/*Added for SC1445x*/
	CallState[HandsetId].RequestedCodec = DECT_NARROWBAND_CODEC;
	CallState[HandsetId].LineId = 0;
#ifdef	DYNAMIC_CHANNEL_ALLOCATION	
	free_pcm_channel(HandsetPcmChannel[HandsetId]);
	HandsetPcmChannel[HandsetId]=AP_CHANNEL_INVALID;
	display_pcm_channel_status();
#endif	
  }
//  ShowCallState(HandsetId);
}

int CheckIfCallStateIsBusy( uint8 HandsetId )
{
	//printf( "CheckIfCallStateIsBusy:%d=%d\n", HandsetId, CallState[HandsetId].State );

	switch( CallState[HandsetId].State ) {
	case F00_NULL:
		return 0;
	default:
		;
	}
	
	return 1;
}

int CheckIfCallStateIsRinging( uint8 HandsetId )
{
	switch( CallState[HandsetId].State ) {
	case F06_CALL_PRESENT:
	case F07_CALL_RECEIVED:
		return 1;
	default:
		;
	}
	
	return 0;	
}

/****************************************************************************
* FUNCTION:      SendApiFpFwuGetCrcRes
* DESCRIPTION:   MPS function for sending mails with primitive:
*                API_FP_FWU_GET_CRC_RES
****************************************************************************/
void SendApiFpFwuGetCrcRes(uint8 DeviceNr,uint32 PrvContext,uint32 Address,uint32 Size,uint16 Crc)
{
  ApiFpFwuGetCrcResType * NewMailPtr;
  NewMailPtr = (ApiFpFwuGetCrcResType*)BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpFwuGetCrcResType));
  NewMailPtr->Primitive = H2M_16( API_FP_FWU_GET_CRC_RES );
  NewMailPtr->DeviceNr = DeviceNr;
  NewMailPtr->PrvContext = PrvContext;
  NewMailPtr->Address = Address;
  NewMailPtr->Size = Size;
  NewMailPtr->Crc = Crc;
  BUSM_DeliverMail((uint8*) NewMailPtr);
  PRINT_MESSAGE("<-API_FP_FWU_GET_CRC_RES - DeviceNr: %X, PrvContext: %lX, Address: %lX, Size: %lX, CRC: %X\n",DeviceNr,PrvContext,M2H_32(Address),M2H_32(Size),M2H_16(Crc));
  
  // start to sync RX DLE 
  if( Address == 0 && Size == 0 )
    EnableSyncRxDLE( 1 );
}

/****************************************************************************
* FUNCTION:      SendApiFpFwuGetBlockRes
* DESCRIPTION:   MPS function for sending mails with primitive:
*                API_FP_FWU_GET_BLOCK_RES
****************************************************************************/
void SendApiFpFwuGetBlockRes(uint8 DeviceNr,uint32 PrvContext,uint32 Address_m,uint32 DataLength_m,uint8 Data_h[1])
{
  const uint32 DataLength_h = M2H_32( DataLength_m );
  ApiFpFwuGetBlockResType * NewMailPtr;
  NewMailPtr = (ApiFpFwuGetBlockResType*)BUSM_AllocateMail(0, USER_TASK, (uint16)(sizeof(ApiFpFwuGetBlockResType) - 1 + DataLength_h));
  NewMailPtr->Primitive  = H2M_16( API_FP_FWU_GET_BLOCK_RES );
  NewMailPtr->DeviceNr   = DeviceNr;
  NewMailPtr->PrvContext = PrvContext;
  NewMailPtr->Address    = Address_m;
  NewMailPtr->DataLength = DataLength_m;
  if (Data_h)
  {
    memcpy(NewMailPtr->Data,Data_h,DataLength_h);
  }
  BUSM_DeliverMail((uint8*) NewMailPtr);
  PRINT_MESSAGE("<-API_FP_FWU_GET_BLOCK_RES - DeviceNr: %X, PrvContext: %lX, Address: %lX, DataLength: %lX\n", DeviceNr,PrvContext,M2H_32(Address_m),DataLength_h);
}

/****************************************************************************
* FUNCTION:      SendApiFpFwuUpdateRes
* DESCRIPTION:   MPS function for sending mails with primitive:
*                API_FP_FWU_UPDATE_RES
****************************************************************************/
void SendApiFpFwuUpdateRes(uint8 DeviceNr,uint8 LinkDate[5],uint32 PrvContext,uint32 RangesCount_h,ApiFwuRangeType Ranges[1])
{
  ApiFpFwuUpdateResType * NewMailPtr;
  NewMailPtr = (ApiFpFwuUpdateResType*)BUSM_AllocateMail(0, USER_TASK, (uint16)(sizeof(ApiFpFwuUpdateResType) + sizeof(ApiFwuRangeType)*RangesCount_h));
  NewMailPtr->Primitive   = H2M_16( API_FP_FWU_UPDATE_RES );
  NewMailPtr->DeviceNr    = DeviceNr;
  memcpy(NewMailPtr->LinkDate,LinkDate,5*sizeof(uint8));
  NewMailPtr->PrvContext  = PrvContext;
  NewMailPtr->RangesCount = H2M_32( RangesCount_h );
  memcpy(NewMailPtr->Ranges,Ranges,sizeof(ApiFwuRangeType)*RangesCount_h);
  BUSM_DeliverMail((uint8*) NewMailPtr);
  PRINT_MESSAGE("<-API_FP_FWU_UPDATE_RES - DeviceNr: %X, LinkDate: %02X.%02X.%02X.%02X.%02X, PrvContext: %lX, RangesCount: %lX, Ranges: %lX(%lX)\n", DeviceNr, LinkDate[0], LinkDate[1], LinkDate[2], LinkDate[3], LinkDate[4], PrvContext, RangesCount_h, ( Ranges ? M2H_32( Ranges[0].StartAddr ) : 0 ), ( Ranges ? M2H_32( Ranges[0].Size ) : 0 ));
}

uint16 Crc16(uint8* DataPtr_h, uint32 Size_m)
{
  uint16 crc = CRC16_INIT_VALUE;
  uint32 Size_h = M2H_32( Size_m );
  while (Size_h--)
  {
    crc = Crc16Tbl[((crc)^(*DataPtr_h))&0xff]^((crc)>>8);
    DataPtr_h++;
  }
  return crc;
}

static PrvContextType* FindContext(uint32 PrvContext)
{
  PrvContextType* pe = (PrvContextType*)ContextList.Flink;
  while (pe != (PrvContextType*)&ContextList.Flink)
  {
    if ((uint32)pe == PrvContext)
    {
      return pe;
    }
    pe = (PrvContextType*)pe->Link.Flink;
  }
  return NULL;
}

static bool FindFwuFile(FwuDeviceInfoType* pi, FwuFileHeaderType** pFileHeader, FwuAreaInfoType** pArea)
{
	//vm
#if 1
	
  *pFileHeader = NULL;
  *pArea = NULL;
//  DEBUG_PRINT("Inside %s . Point 1 \n", __FUNCTION__);
  if (FwuFileInfo != NULL && FwuFileInfo->Map.pMap)
  {
	//DEBUG_PRINT("Inside %s . Point 2. FwuFileInfo->Map.pMap = %x \n", __FUNCTION__, FwuFileInfo->Map.pMap);
    FwuFileHeaderType* pf = (FwuFileHeaderType*)FwuFileInfo->Map.pMap;
    if (pf->Magic[0] == 'F' && pf->Magic[1] == 'W' && pf->Magic[2] == 'U')
    {
        FwuAreaInfoType* pa = NULL;
//		DEBUG_PRINT("Inside %s . Point 3 \n", __FUNCTION__);
//      DEBUG_PRINT("pf->High.DeviceId = 0x%x, pf->Low.DeviceId = 0x%x, pi->DeviceId = 0x%x, \n pf->Low.Address = 0x%x, pf->High.Address = 0x%x, pi->Area.StartAdr = 0x%x,\n pf->High.Size = 0x%x, pf->Low.Size = 0x%x, pi->Area.Size = 0x%x \n", pf->High.DeviceId, pf->Low.DeviceId, pi->DeviceId, pf->High.Address, pf->Low.Address, pi->Area.StartAdr, pf->High.Size, pf->Low.Size,pi->Area.Size);      
//      DEBUG_PRINT("pf->Low.InfoOffset = 0x%x \n", pf->Low.InfoOffset);
//      DEBUG_PRINT("pf->Low.Address = 0x%x \n", pf->Low.Address);
//      DEBUG_PRINT("pf->Low.Size = 0x%x \n", pf->Low.Size);
//      DEBUG_PRINT("pf->Low.DeviceId = 0x%x \n", pf->Low.DeviceId);
//      DEBUG_PRINT("pf->Low.LinkDate[0] = 0x%x \n", pf->Low.LinkDate[0]);
//      DEBUG_PRINT("pf->Low.LinkDate[1] = 0x%x \n", pf->Low.LinkDate[1]);  
      PRINT_INFO("Firmware file link date %02X/%02X/%02X, %02X:%02X \n", pf->Low.LinkDate[2], pf->Low.LinkDate[1],pf->Low.LinkDate[0], pf->Low.LinkDate[3], pf->Low.LinkDate[4]);
      if (pf->Low.DeviceId == pi->DeviceId && pf->Low.Address == pi->Area.StartAdr && pf->Low.Size == pi->Area.Size)
      {
        pa = &pf->Low;
      }
      if (pf->High.DeviceId == pi->DeviceId && pf->High.Address == pi->Area.StartAdr && pf->High.Size == pi->Area.Size)
      {
        pa = &pf->High;
      }
      if (pa)
      {
        *pFileHeader = pf;
        *pArea = pa;
        return true;
      }
    }
  }
  return false;
#endif //vm
  return true;
}

static void GetBlock(DataBlockType* db_m, PrvContextType* pe_m, uint32 Address_m, uint32 Size_m)
/****************************************************************************
* DESCRIPTION:
****************************************************************************/
{
  FwuImageBlockType* pb_m = pe_m->Blocks;
  uint32 i, n_h = M2H_32( pe_m->Area->ImageBlocks );
  uint32 Address_h = M2H_32( Address_m );
  uint32 Size_h = M2H_32( Size_m );
  
  for (i=0; i < n_h; i++)
  {
    uint32 high_h;
    if (Address_h <= M2H_32( pb_m->Address ))
    {
      db_m->Address = pb_m->Address;
      db_m->Size = H2M_32( MIN(Size_h, M2H_32( pb_m->Size ) ) );
      db_m->Data = pb_m->Data;
      return;
    }
    high_h = M2H_32( pb_m->Address ) + M2H_32( pb_m->Size );
    if (Address_h > M2H_32( pb_m->Address ) && Address_h < high_h)
    {
      db_m->Address = Address_m;
      db_m->Size = H2M_32( MIN(Size_h, (high_h-Address_h)) );
      db_m->Data = pb_m->Data + (Address_h - M2H_32( pb_m->Address ) );
      return;
    }
    pb_m = (FwuImageBlockType*)(pb_m->Data + M2H_32( pb_m->Size ) + sizeof(uint16));
  }
}

void ApiFpFwuEnableCfm(CvmApiMailType *pApi)
{
  PRINT_MESSAGE("->API_FP_FWU_ENABLE_CFM\n");
}

void ApiFpFwuDeviceNotifyInd(CvmApiMailType *pApi)
{
  ApiFpFwuDeviceNotifyIndType* pm = (ApiFpFwuDeviceNotifyIndType*)pApi;
  #define M pApi->ApiFpFwuDeviceNotifyInd
  PRINT_MESSAGE("->API_FP_FWU_DEVICE_NOTIFY_IND - Present: %X, DeviceNr: %X, DeviceId: %lX, TargetData: %lX, FwuMode: %X, LinkDate: %02X.%02X.%02X.%02X.%02X, Range: %lX(%lX)\n", M.Present, M.DeviceNr, M.DeviceId, M2H_32(M.TargetData), M.FwuMode, M.LinkDate[0], M.LinkDate[1], M.LinkDate[2], M.LinkDate[3], M.LinkDate[4], M2H_32(M.Range.StartAddr), M2H_32(M.Range.Size));
  #undef M
  if (pm->Present && pm->FwuMode == 0)
  {
    // The CVM module is ruinning the FWU program.
    // We must request it to start update of the main program.
    FwuOpenFile();
    if (FwuFileInfo != NULL)
    {
      BUSM_SendMailP2(0, USER_TASK, API_FP_FWU_UPDATE_REQ, 0, 1);
      PRINT_MESSAGE("<-API_FP_FWU_UPDATE_REQ - DeviceNr: 0, FwuMode: 1\n");
    }
  }
}

void ApiFpFwuUpdateCfm(CvmApiMailType *pApi)
{
  PRINT_MESSAGE("->API_FP_FWU_UPDATE_CFM - DeviceNr: %X, Status: %X\n", pApi->ApiFpFwuUpdateCfm.DeviceNr, pApi->ApiFpFwuUpdateCfm.Status);
}

void ApiFpFwuUpdateInd(CvmApiMailType *pApi_m)
{
  ApiFpFwuUpdateIndType* pm_m = (ApiFpFwuUpdateIndType*)pApi_m;
  PrvContextType* pe_m = NULL;
  FwuFileHeaderType* pf_m;
  FwuAreaInfoType* pa_m;
  FwuDeviceInfoType info_m;

  #define M pApi_m->ApiFpFwuUpdateInd
  PRINT_MESSAGE("->API_FP_FWU_UPDATE_IND - DeviceNr: %X, DeviceId: %lX, TargetData: %lX, FwuMode: %X, LinkDate: %02X.%02X.%02X.%02X.%02X, Range: %lX(%lX)\n",M.DeviceNr,M.DeviceId,M2H_32(M.TargetData),M.FwuMode,M.LinkDate[0],M.LinkDate[1],M.LinkDate[2],M.LinkDate[3],M.LinkDate[4],M2H_32(M.Range.StartAddr),M2H_32(M.Range.Size));
  PRINT_INFO("Current CVM firmware file link date %02X/%02X/%02X, %02X:%02X \n", M.LinkDate[2], M.LinkDate[1],M.LinkDate[0], M.LinkDate[3], M.LinkDate[4]);
  #undef M

  info_m.DeviceId = pm_m->DeviceId;
  info_m.FwuTargetData = pm_m->TargetData;
  memcpy(info_m.LinkDate, pm_m->LinkDate, 5);
  info_m.Mode = pm_m->FwuMode;
  info_m.Area.Size = pm_m->Range.Size;
  info_m.Area.StartAdr = pm_m->Range.StartAddr;

#if 1
  if (FwuFileInfo != NULL && FwuFileInfo->Map.pMap)
  	;
  else if( memcmp( pApi_m->ApiFpFwuUpdateInd.LinkDate, 
  					"\xFF\xFF\xFF\xFF\xFF", 5 ) == 0 )
  {
  	/* 
  	 * To deal with: 
  	 *   78/128  [done for FIRMWARE UPGRADE ]
  	 *   Message 4f0b received.
  	 *   ((((There are some CRC error....))))
  	 *   ->API_FP_FWU_COMPLETE_IND
  	 *   Firmware upgrade process completed ... 
  	 *   Entering normal mode ...
  	 *   Message 4f05 received. RES - DeviceNr: 0, PrvContext: 48A038, Address: F1078, DataLength: 78
  	 *   ->API_FP_FWU_UPDATE_IND - DeviceNr: 0, DeviceId: 37009A0D, TargetData: 200, FwuMode: 1, Link
  	 *   Date: FF.FF.FF.FF.FF, Range: F1000(5F000)
  	 *   Current CVM firmware file link date FF/FF/FF, FF:FF
  	 * 
  	 * This is to deal with upgrade fail in progress, and it resumes 
  	 * to upgrade automatically and announce API_FP_FWU_UPDATE_IND. 
  	 *
  	 * IMPORTANT!! 
  	 * In newer design of HDLC, it will not occur. 
  	 */
  	FwuOpenFile();
  }
#endif

  if (FindFwuFile(&info_m, &pf_m, &pa_m))
  {
    FwuImageBlockType* pb_m = NULL;
    if (pa_m == &pf_m->Low)
    {
      pb_m = (FwuImageBlockType*)&pf_m[1];
    }
    if (pa_m == &pf_m->High)
    {
      // Layout of image blocks:
      //   Address
      //   Size
      //   Data (variable)
      //   Padding (variable, 0-3 bytes).
      //   CRC
      //   Padding (fixed, 2 bytes).
      uint32 i, n_h = M2H_32( pf_m->Low.ImageBlocks );
      pa_m = &pf_m->High;
      pb_m = (FwuImageBlockType*)&pf_m[1];
      for (i=0; i < n_h; i++)
      {
        uint32 padsize_h = M2H_32( pb_m->Size ) & 0x3;
        pb_m = (FwuImageBlockType*)(pb_m->Data + M2H_32( pb_m->Size ) + padsize_h + 4);
      }
    }
    pe_m = (PrvContextType*)malloc(sizeof(PrvContextType));
    InitializeListHead(&pe_m->Link);
    pe_m->Area = pa_m;
    pe_m->Blocks = pb_m;
    InsertTailList(&ContextList, &pe_m->Link);
    // Send res back to the CVM module
    {
      // build the "ranges array"
      FwuImageBlockType* pb_m = pe_m->Blocks;
      ApiFwuRangeType* pr_m = (ApiFwuRangeType*)malloc(M2H_32(pe_m->Area->ImageBlocks) * sizeof(ApiFwuRangeType));
//vm
//	  for (uint16 i=0; i < pe->Area->ImageBlocks; i++)
	  uint16 i;
	  const uint16 ImageBlocks_h = M2H_32(pe_m->Area->ImageBlocks);
	  for (i=0; i < ImageBlocks_h; i++)
      {
        pr_m[i].StartAddr = pb_m->Address;
        pr_m[i].Size = pb_m->Size;
        pb_m = (FwuImageBlockType*)(pb_m->Data + M2H_32( pb_m->Size ) + sizeof(uint16));
      }
      SendApiFpFwuUpdateRes(pm_m->DeviceNr, pe_m->Area->LinkDate, (uint32)pe_m, M2H_32(pe_m->Area->ImageBlocks), pr_m);
      free(pr_m);
    }
  }
  else
  {
	PRINT_MESSAGE("FindFwuFile returned error !!! \n");
    SendApiFpFwuUpdateRes(pm_m->DeviceNr, pm_m->LinkDate, 0, 0, NULL);
  }
}

void ApiFpFwuGetBlockInd(CvmApiMailType *pApi)
{
  ApiFpFwuGetBlockIndType* pm = (ApiFpFwuGetBlockIndType*)pApi;
  PrvContextType* pe = FindContext(pm->PrvContext);
  DataBlockType db_m;

  PRINT_MESSAGE("->API_FP_FWU_GET_BLOCK_IND\n");
  memset(&db_m, 0, sizeof(db_m));
  if (pe)
  {
    GetBlock(&db_m, pe, pm->Address, pm->Size);
  }
  SendApiFpFwuGetBlockRes(pm->DeviceNr, (uint32)pe, db_m.Address, db_m.Size, db_m.Data);
  print_progress_bar("[done for FIRMWARE UPGRADE ]",  M2H_32(pm->Address)-M2H_32(((FwuImageBlockType*)(pe->Blocks))->Address), M2H_32(((FwuImageBlockType*)(pe->Blocks))->Size), PROGRESS_BAR_SIZE);  
}

void ApiFpFwuGetCrcInd(CvmApiMailType *pApi)
{
  ApiFpFwuGetCrcIndType* pm = (ApiFpFwuGetCrcIndType*)pApi;
  PrvContextType* pe = FindContext(pm->PrvContext);
  DataBlockType db_m;
  uint16 crc_h = 0;

  #define M pApi->ApiFpFwuGetCrcInd
  PRINT_MESSAGE("->API_FP_FWU_GET_CRC_IND - DeviceNr: %X, PrvContext: %lX, Address: %lX, Size: %lX\n",M.DeviceNr,M.PrvContext,M2H_32(M.Address),M2H_32(M.Size));
  #undef M
  memset(&db_m, 0, sizeof(db_m));
  if (pe)
  {
    GetBlock(&db_m, pe, pm->Address, pm->Size);
    crc_h = Crc16(db_m.Data, db_m.Size);
  }
  SendApiFpFwuGetCrcRes(pm->DeviceNr, (uint32)pe, db_m.Address, db_m.Size, H2M_16(crc_h));
}

void ApiFpFwuCompleteInd(CvmApiMailType *pApi)
{
  ApiFpFwuCompleteIndType* pm = (ApiFpFwuCompleteIndType*)pApi;
  PrvContextType* pe = FindContext(pm->PrvContext);
  PRINT_MESSAGE("->API_FP_FWU_COMPLETE_IND\n");
  if (pe)
  {
    RemoveEntryList(&pe->Link);
    free(pe);
  }
  FwuCloseFile();
  PRINT_INFO("Firmware upgrade process completed ...\n");  
  //return to normal mode
  SetRegistrationMode(NORMAL_MODE);  
  // stop to sync RX DLE 
  EnableSyncRxDLE( 0 );
}

void ApiFpFwuStatusInd(CvmApiMailType *pApi)
{
  uint8 i;
  
  PRINT_MESSAGE("->API_FP_FWU_STATUS_IND - BusyDeviceNr: %X, StatusLength: %X, Status: ", pApi->ApiFpFwuStatusInd.BusyDeviceNr, pApi->ApiFpFwuStatusInd.StatusLength);
  
  for( i = 0; i < pApi->ApiFpFwuStatusInd.StatusLength; i ++ )
  	PRINT_MESSAGE("%X.", pApi->ApiFpFwuStatusInd.Status[i]);
  PRINT_MESSAGE("\n");
}

void FwuInit(void)
{
  InitializeListHead(&ContextList);
  FwuOpenFile(); // Check that we can open the FWU file.
  FwuCloseFile();
}

void FwuStart(void)
{
  FwuOpenFile();
  if (FwuFileInfo != NULL)
  {
	BUSM_SendMailP1(0, USER_TASK, API_FP_FWU_ENABLE_REQ, 1);//giag
	PRINT_MESSAGE("<-API_FP_FWU_ENABLE_REQ - FwuMode: 1\n");//giag		  
    BUSM_SendMailP2(0, USER_TASK, API_FP_FWU_UPDATE_REQ, 0, 1);
    PRINT_MESSAGE("<-API_FP_FWU_UPDATE_REQ - DeviceNr: 0, FwuMode: 1\n");
  }
}

int FwuSetFile( const char *filename )
{
	size_t len = ( filename ? strlen( filename ) : 0 );
	
	if( len >= sizeof( FWU_FILENAME_buffer ) ) {
		PRINT_MESSAGE( "Firmware filename '%s' is too long\n", filename );
		return 0;	// filename is too long 
	} else if( len == 0 ) {
		PRINT_MESSAGE( "Firmware filename is not given\n" );
		return 0;
	}
	
	memcpy( FWU_FILENAME_buffer, filename, len + 1 );
	
	return 1;
}

void FwuOpenFile(void)
{
	//vm
#if 1
  if (FwuFileInfo != NULL)
  {
    //delete(FwuFileInfo);
    //FwuFileInfo = NULL;
    FwuCloseFile();//giag
  }
  //FwuFileInfo = new(FileDataType);
  FwuFileInfo = &FwuFileInfo_Buf;
  //memcpy((void*)FwuFileInfo->Filename, FWU_FILENAME, strlen(FWU_FILENAME));//20
  //strcpy( FwuFileInfo->Filename, FWU_FILENAME );
  FwuFileInfo->Filename = FWU_FILENAME_buffer;
  uint32 ErrorCode = OpenFileMap(FwuFileInfo->Filename, &FwuFileInfo->Map);
  if (ErrorCode != ERROR_SUCCESS)
  {
    PRINT_INFO("Cannot open %s\n",FwuFileInfo->Filename);
    //delete(FwuFileInfo);
    FwuFileInfo = NULL;
    return;
  }
  PRINT_INFO("Opened firmware upgrade file: %s succesfully\n", FwuFileInfo->Filename);
  
  FwuFileOpen = 1;
#endif //vm
}

void FwuCloseFile(void)
{
	//vm
  if (FwuFileInfo != NULL)
  {
 //   delete(FwuFileInfo);
//    FwuFileInfo = NULL;
		CloseFileMap(&FwuFileInfo->Map);
  }
  
  FwuFileOpen = 0;
}

/****************************************************************************
*  FUNCTION: 	check_registration_button_thread_function( void* unused )
*   INPUTS  :
*  RETURNS :
*  DESCRIPTION: Checks the status of the registration button.
*			If it is pressed for
****************************************************************************/
void SetRegistrationMode(unsigned char mode)
{
	/*if requested mode other than current mode, end current process first*/
	if(mode != registration_mode){
		switch (registration_mode){//current mode
			case REGISTER_MODE:
				//End registration process
				PRINT_MESSAGE("Ending registration process ...\n");
				BUSM_SendMailP2(0, USER_TASK, API_FP_SET_REGISTRATION_MODE_REQ, 0, 0x01);//disable registration mode
				PRINT_MESSAGE("<-API_FP_SET_REGISTRATION_MODE_REQ %s\n","Disable");
				#ifdef DECT_HEADSET_DESIGN
					fnScreenDectRegistrationCompletionInd(CCFSM_REGISTRATION_OK);
				#endif				
				break;
			case DELETE_ALL_MODE:
				//End delete process
				#ifdef DECT_HEADSET_DESIGN
					fnScreenDectDeleteCompletionInd(CCFSM_DELETE_DECT_END);
				#endif
				#ifdef SET_RFPI
					if(check_rfpi_emc_fpn_change)
						SetRfpi();
				#endif					
				PRINT_INFO("Ending recursive delete process ...\n");
				break;
			case NORMAL_MODE:
				//End normal mode
				PRINT_INFO("Ending normal mode ...\n");
				break;
#ifdef CVM480				
			case FWU_UPGRADE_MODE:
				//End FWU mode
				PRINT_INFO("Ending firmware upgrade mode ...\n");
				break;
#endif				
			default:
				break;
		}
		registration_in_progress_tmouts = 0;
	}

	/*now switch to requested mode*/
	switch(mode){
#ifdef CVM480		
		case FWU_UPGRADE_MODE:
			//Start firmware upgrade process
			PRINT_INFO("Starting firmware upgrade process ...\n");
			FwuStart();
			break;
#endif			
		case REGISTER_MODE:
			//Check if more handsets can be registered
			PRINT_INFO("\n\nMax Handsets allowed are %d Current Registered HS are %d \n\n", MAX_REGISTERED_HS, RegisteredHS);
			if(RegisteredHS>=MAX_REGISTERED_HS){
				PRINT_INFO("No more handsets can be registered ...Deleting last handset(s) ...\n");
				#ifdef DECT_HEADSET_DESIGN
					fnScreenDectRegisterStartInd(CCFSM_DELETE_HANDSETS);
				#endif
				//Start delete process
				registration_in_progress_tmouts = DELETE_MODE_TMOUT;
				#ifdef LED_GPIO_SUPPORT
					LED_REG_OFF;
					LED_DEL_ON;
				#endif				
				PRINT_INFO("Starting recursive delete handset process ...\n");
				BUSM_SendMailP1(0, USER_TASK, API_FP_DELETE_REGISTRATION_REQ, RegisteredHS);
				PRINT_MESSAGE("<-API_FP_DELETE_REGISTRATION_REQ Id = %X\n",RegisteredHS);
			}
			else{
				//Start registration process
				#ifdef DECT_HEADSET_DESIGN
					fnScreenDectRegisterStartInd(CCFSM_REG_HANDSET);
				#endif
				#ifdef LED_GPIO_SUPPORT
					LED_DEL_OFF;
					LED_REG_ON;
				#endif
				registration_in_progress_tmouts = REGISTRATION_MODE_TMOUT;
				PRINT_INFO("Starting registration process ...\n");
				BUSM_SendMailP2(0, USER_TASK, API_FP_SET_REGISTRATION_MODE_REQ, 1, 0x01);//delete last handset if max number of handsets are registered
				PRINT_MESSAGE("<-API_FP_SET_REGISTRATION_MODE_REQ %s\n","Enable");
			}
			break;		
		case DELETE_ALL_MODE:
			if(RegisteredHS){
				//Start delete process
				registration_in_progress_tmouts = DELETE_MODE_TMOUT;
				#ifdef DECT_HEADSET_DESIGN
					fnScreenDectDeleteCompletionInd(CCFSM_DELETE_DECT_START);
				#endif
				#ifdef LED_GPIO_SUPPORT
					LED_REG_OFF;
					LED_DEL_ON;
				#endif
				PRINT_INFO("Starting recursive delete handset process ...\n");
				BUSM_SendMailP1(0, USER_TASK, API_FP_DELETE_REGISTRATION_REQ, RegisteredHS);
				PRINT_MESSAGE("<-API_FP_DELETE_REGISTRATION_REQ Id = %X\n",RegisteredHS);
			}
			else{
				#ifdef DECT_HEADSET_DESIGN
					fnScreenDectDeleteCompletionInd(CCFSM_DELETE_DECT_NO_DEVICE);
				#endif
				#ifdef SET_RFPI
					if(check_rfpi_emc_fpn_change)
						SetRfpi();
				#endif						
				PRINT_INFO("No registered handsets to delete ...Return to normal mode ...\n");
				mode=NORMAL_MODE;
				registration_key_press_tmouts=0;
				break;
			}
			break;
		case NORMAL_MODE:
			//Switch to normal mode
			PRINT_INFO("Entering normal mode ...\n");
			#ifdef LED_GPIO_SUPPORT
				LED_REG_OFF;
				LED_DEL_OFF;
			#endif			
			registration_key_press_tmouts=0;
			
			break;
		default:
			break;
	}
	registration_mode=mode;
}

/****************************************************************************
*  FUNCTION: 	check_registration_button_thread_function( void* unused )
*   INPUTS  :
*  RETURNS :
*  DESCRIPTION: Checks the status of the registration button. every second.
*			Counts how many seconds it is pressed and alters the operating mode (to registration / delete).
****************************************************************************/
static void *check_registration_button_thread_function( void* unused )
{
	extern int rtk_gpio(unsigned long action, unsigned long pid, unsigned long value, unsigned long *ret_value);
	
	PRINT_MESSAGE("Enter check_registration_button_thread_function (pid=%d) \n", getpid());

	stop_check_registration_button_thread = FALSE;
	//static long long b = 0;
#if !defined( SA_CVM_NO_MAIN ) && defined (CONFIG_SC14452)
	unsigned long gpio_val;
#endif

	while(stop_check_registration_button_thread == FALSE)
	{
		sleep(1);

		/*Check if key is pressed*/
#ifdef SA_CVM_NO_MAIN
		if( !rtk_Si_GetDectPage() )
#elif defined (CONFIG_SC14452)
		#define REG_PIN_ID	( ( ( 'A' - 'A' ) << 16 ) | ( 2 & 0xFFFF ) ) // A2
		//#define REG_PIN_ID	( ( ( 'F' - 'A' ) << 16 ) | ( 1 & 0xFFFF ) ) // F1

		rtk_gpio( 0, 	// init
				REG_PIN_ID, 
				( 0 | ( 0 << 2 ) | ( 0 << 3 ) ), // GPIO, IN, NO_INT
				&gpio_val );
		
		rtk_gpio( 1, 	// read
				REG_PIN_ID,
				( 0 | ( 0 << 2 ) | ( 0 << 3 ) ), // (ignore) GPIO, IN, NO_INT
				&gpio_val );
		
		//printf( "------gpio_val:%d\n", gpio_val );
		
		if( !gpio_val )
		//if ( P2_DATA_REG & 0x400 )
#else
		if ( P2_DATA_REG & 8 )
#endif
			registration_key_press_tmouts++;
		else
			registration_key_press_tmouts=0;

		/*Check if a process is in progress*/
		if (registration_in_progress_tmouts){
			registration_in_progress_tmouts--;
			if(!registration_in_progress_tmouts)
				SetRegistrationMode(NORMAL_MODE);
#ifdef DECT_HEADSET_DESIGN
				if(registration_mode==REGISTER_MODE)
					fnScreenDectRegistrationCompletionInd(CCFSM_REGISTRATION_FAILED);
#endif
		}
		else{
			switch(registration_mode){
				case NORMAL_MODE:
					if (registration_key_press_tmouts > REGISTRATION_MODE_KEY_PRESS_TMOUT){
						registration_mode = REGISTER_MODE;
						#ifdef LED_GPIO_SUPPORT
							LED_REG_ON;
						#endif
					}
					else if (registration_key_press_tmouts){
#ifdef LED_GPIO_SUPPORT						
						LED_REG_OFF;
#endif						
						PRINT_INFO("%d seconds left to enter registration mode ... \n", REGISTRATION_MODE_KEY_PRESS_TMOUT - registration_key_press_tmouts);	
						//print_progress_bar("[Release button to enter REGISTRATION mode ...]",  registration_key_press_tmouts, REGISTRATION_MODE_KEY_PRESS_TMOUT, PROGRESS_BAR_SIZE);				
#if 0
						BUSM_SendMailP3(0, USER_TASK, API_FP_PROPRIETARY_REQ, 2, 0x00, 0);
						PRINT_MESSAGE("<-API_FP_PROPRIETARY_REQ \n");				
#endif						
					}
					break;
				case REGISTER_MODE:
					if(!registration_key_press_tmouts){//key released after at least REGISTRATION_MODE_KEY_PRESS_TMOUT secs
						//Start registration process
						SetRegistrationMode(REGISTER_MODE);
					}
					else if (registration_key_press_tmouts > DELETE_ALL_MODE_KEY_PRESS_TMOUT){
						registration_mode = DELETE_ALL_MODE;
#ifndef CVM480	//If it is not CVM480 compilation, then no firmware upgrade option is needed		
						//Start delete process
						SetRegistrationMode(DELETE_ALL_MODE);
#endif						
					}
					else{
						PRINT_INFO("%d seconds left to enter delete mode ... \n", DELETE_ALL_MODE_KEY_PRESS_TMOUT - registration_key_press_tmouts);
						//print_progress_bar("[Release button to enter DELETE mode ...]",  registration_key_press_tmouts-REGISTRATION_MODE_KEY_PRESS_TMOUT, DELETE_ALL_MODE_KEY_PRESS_TMOUT-REGISTRATION_MODE_KEY_PRESS_TMOUT, PROGRESS_BAR_SIZE);
					}
					break;
				case DELETE_ALL_MODE:
					if(!registration_key_press_tmouts){//key released after at least REGISTRATION_MODE_KEY_PRESS_TMOUT secs
						//Start registration process
						SetRegistrationMode(DELETE_ALL_MODE);
					}
					else if (registration_key_press_tmouts > FWU_UPGRADE_MODE_KEY_PRESS_TMOUT){
						registration_mode = FWU_UPGRADE_MODE;
						//Start delete process
						SetRegistrationMode(FWU_UPGRADE_MODE);
					}
					else{
						PRINT_INFO("%d seconds left to enter firmware upgrade mode ... \n", FWU_UPGRADE_MODE_KEY_PRESS_TMOUT - registration_key_press_tmouts);		
						//print_progress_bar("[Release button to enter FIRMWARE UPGRADE mode ...]",  registration_key_press_tmouts-DELETE_ALL_MODE_KEY_PRESS_TMOUT, FWU_UPGRADE_MODE_KEY_PRESS_TMOUT-DELETE_ALL_MODE_KEY_PRESS_TMOUT, PROGRESS_BAR_SIZE);
					}
					break;					
				default:
					break;
			}
		}
	}

	PRINT_MESSAGE("Exit check_registration_button_thread_function \n");

	stop_check_registration_button_thread = FALSE;
	pthread_exit(0);

}

/****************************************************************************
*  FUNCTION: 	CvmPowerReset
*   INPUTS  :
*  RETURNS :
*  DESCRIPTION: Performs power reset to the CVM480 module.
****************************************************************************/
void CvmPowerReset(void)
{
#ifdef SA_CVM_NO_MAIN
	struct timeval tv ;
	unsigned msec = 500 ;
	
	rtk_Si_SetDectPower( 1 );	// power off
	{
		tv.tv_sec  = ( msec / 1000 ) ;
		tv.tv_usec = ( msec % 1000 ) * 1000 ;
		select( 0, NULL, NULL, NULL, &tv ) ;
	}
	
	rtk_Si_SetDectPower( 0 );	// power on
	{
		msec = 500;
		tv.tv_sec  = ( msec / 1000 ) ;
		tv.tv_usec = ( msec % 1000 ) * 1000 ;
		select( 0, NULL, NULL, NULL, &tv ) ;
	}
#endif

#ifdef CONFIG_F_G2_BOARDS

	struct timeval tv ;
	unsigned msec = 500 ;


	P2_10_MODE_REG = 0x300;

	P2_RESET_DATA_REG = 0x0400;
	{
		tv.tv_sec  = ( msec / 1000 ) ;
		tv.tv_usec = ( msec % 1000 ) * 1000 ;
		select( 0, NULL, NULL, NULL, &tv ) ;
	}
	P2_SET_DATA_REG = 0x0400;
#endif
}

/****************************************************************************
*  FUNCTION: main
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: main function
****************************************************************************/
void dect_main(void)
{
  uint16 Primitiv;
  uint16 tempLength = 1;
  uint8  tempDataPtr[5];
  //uint8  Key = 'h'; // Start with print of help text
  uint8 i;
  //struct sigaction saio;           /* definition of signal action */
  
#ifdef LED_GPIO_SUPPORT  
	LED_DEL_MODE_SET;
	LED_REG_MODE_SET;
	LED_DEL_OFF;
	LED_REG_OFF;
#endif	
#if defined (CONFIG_SC14452)	
        //SetPort(P1_14_MODE_REG, GPIO_PUPD_OUT_NONE,  GPIO_PID_PCM_FSC);       /* P1_14 is PCM_FSC */
        //SetPort(P2_07_MODE_REG, GPIO_PUPD_IN_NONE,  GPIO_PID_PCM_DI);         /* P2_07 is PCM_DI */
        //SetPort(P1_00_MODE_REG, GPIO_PUPD_OUT_NONE,  GPIO_PID_PCM_DO);         /* P1_00 is PCM_DO */
        //SetPort(P2_09_MODE_REG, GPIO_PUPD_OUT_NONE,  GPIO_PID_PCM_CLK);        /* P2_09 is PCM_CLK */
#else
        SetPort(P0_02_MODE_REG, GPIO_PUPD_OUT_NONE,  GPIO_PID_PCM_FSC);       /* P0_02 is PCM_FSC */
        SetPort(P0_03_MODE_REG, GPIO_PUPD_IN_NONE,  GPIO_PID_PCM_DI);         /* P0_03 is PCM_DI */
        SetPort(P0_04_MODE_REG, GPIO_PUPD_OUT_NONE,  GPIO_PID_PCM_DO);         /* P0_04 is PCM_DO */
        SetPort(P0_05_MODE_REG, GPIO_PUPD_OUT_NONE,  GPIO_PID_PCM_CLK);        /* P0_05 is PCM_CLK */
#endif

	for (i=0;i<NO_HS;i++) {
		CallState[i].State =HS_NOT_REGISTERED;
		CallState[i].CallClass = 0;
		CallState[i].Connected2Line = FALSE;
		CallState[i].ProposedCodecListLength = 0;
		CallState[i].ProposedCodecListPtr = NULL;
		/*Added for SC1445x*/
		CallState[i].RequestedCodec = DECT_NARROWBAND_CODEC;
	}

#ifdef REGISTRATION_BUTTON

#if defined (CONFIG_SC14452)
	//P2_10_MODE_REG = 0x200;
#endif

	/* Create a  threads for testing registration button press */
	i = pthread_create( &check_registration_button_thread, NULL, check_registration_button_thread_function,
						"Check Registration Button Thread");
	if( i )
	{
		PRINT_INFO("Check Registration Button Thread creation failed\n");
	} 
	else
		pthread_detach( check_registration_button_thread ) ;
#endif

  //install signal hanlders for safe exit
#ifdef CVM480
	signal(SIGINT, hdlc_Exit);
	signal(SIGTERM, hdlc_Exit);
	signal(SIGABRT, hdlc_Exit);

	CvmPowerReset();
	hdlc_Init();
#if 0
	BUSM_SendMailP0(0, USER_TASK, API_FP_RESET_REQ);
#endif
	
  // initializing com port with cmd line args
  // PRINT_MESSAGE("CVM480 Communication Console\n");
  // if((argc == 2) && ((*argv[1] >= '1') && (*argv[1] <= '9')))
  // {
    // Comport = ((*argv[1]) - '0'); // set com port
  // }
  // else
  // {
    // PRINT_MESSAGE("No cmdline arguments\n");
  // }
   //PRINT_MESSAGE("Connecting to COM%d:\n",Comport);
#endif
/*
  pcmcfgfile = fopen("pcmcfg.cfg", "rb+");
  if(pcmcfgfile == NULL)
  { // file did not exist
    pcmcfgfile = fopen("pcmcfg.cfg", "wb");
  }
  else
  {
    fread((void*)&PcmConfig, 1, sizeof(PcmConfig), pcmcfgfile);
  }

  FwuInit();
*/
//  BUSM_SendMailP0(0, USER_TASK, API_FP_RESET_REQ);
  // initialize state handler

/**/
#ifdef DECT_NATALIE
	natalie_main();
#endif
#ifdef CVM480
	FwuInit();
  {
    while(hdlc_Busy()) usleep (10000); // wait for rs232 handshake
    Primitiv = INITTASK;
    tempDataPtr[0] = (uint8) (Primitiv&0x00ff);
    tempDataPtr[1] = (uint8) (Primitiv>>8);
    tempLength = 0;
    sendmail_ReceivePacket(tempLength, tempDataPtr);
  }
#endif

  while (go_on_with_ata == 0) usleep (10000);

// while (1);

/*
  rewind(pcmcfgfile);
  fwrite((void*)&PcmConfig, 1, sizeof(PcmConfig), pcmcfgfile);
  fclose(pcmcfgfile);
*/

	PRINT_MESSAGE("\n-Leaving %s..\n", __FUNCTION__);
}

/****************************************************************************
*  FUNCTION: DisplayCodecList
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Display the codec list
****************************************************************************/
void DisplayCodecList(uint8 CodecListLength, uint8 *Data)
{
  // pkshih: If we want to uncomment followings, please review argument Data of callers. 
  //ApiCodecListType *ApiCodecListPtr;
  //ApiCodecInfoType *ApiCodecInfoPtr;
  //int i;
#if 0
  if(CodecListLength > 0)
  {
    uint8 i;
    PRINT_MESSAGE("  CodecList: ");
    for(i=0;i<CodecListLength;i++)
    {
      PRINT_MESSAGE("%02X ", Data[i]);
    }
    PRINT_MESSAGE("\n");

	ApiCodecListPtr = (ApiCodecListType*)Data;
	ApiCodecInfoPtr = &ApiCodecListPtr->Codec;
	switch (ApiCodecListPtr->NegotiationIndicator){
		case ANI_NOT_POSSIBLE:
			PRINT_MESSAGE("\t ApiNegotiationIndicator = %s \n", "ANI_NOT_POSSIBLE");
			break;
		case ANI_POSSIBLE:
			PRINT_MESSAGE("\t ApiNegotiationIndicator = %s \n", "ANI_POSSIBLE");
			break;
		case ANI_INVALID:
			PRINT_MESSAGE("\t ApiNegotiationIndicator = %s \n", "ANI_INVALID");
			break;
	}

	PRINT_MESSAGE("\t NoOfCodecs = %d \n", ApiCodecListPtr->NoOfCodecs);

	for (i=0; i<ApiCodecListPtr->NoOfCodecs; i++){
		PRINT_MESSAGE("\t\t Codec = %s \n", ApiCodecTypeName[ApiCodecInfoPtr->Codec]);
		PRINT_MESSAGE("\t\t MacDlcService = %02X \n", ApiCodecInfoPtr->MacDlcService);
		PRINT_MESSAGE("\t\t CplaneRouting = %02X \n", ApiCodecInfoPtr->CplaneRouting);
		PRINT_MESSAGE("\t\t SlotSize = %02X \n\n", ApiCodecInfoPtr->SlotSize);
		ApiCodecInfoPtr++;
	}
  }
#endif
}

/****************************************************************************
*  FUNCTION: HandlePcmCfgMode
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: PCM interface configuration
****************************************************************************/
void HandlePcmCfgMode(uint8 *MailPtr)
{
	//vm
	ApiFpInitPcmReqType * NewMailPtr;
  switch(((recDataType*) MailPtr)->bData[0])
  {
    case '1':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
          pcmcfgmode = PCM_CFG_ENABLE;
          PRINT_MESSAGE("\n1 - Enable PCM interface\n");
          PRINT_MESSAGE("2 - Disable PCM interface\n");
          break;
        case PCM_CFG_ENABLE:
          PRINT_MESSAGE("PCM interface enabled\n");
          PcmConfig.PcmEnable = APE_ENABLED;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_MODE:
          PRINT_MESSAGE("PCM slave mode\n");
          PcmConfig.PcmMasterSlaveId = APM_SLAVE;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_FSC_FREQ:
          PRINT_MESSAGE("FSC frequency 8 KHz\n");
          PcmConfig.PcmFscFreq = AP_FSC_FREQ_8KHZ;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_FSC_LEN:
          PRINT_MESSAGE("FSC length 1 bit\n");
          PcmConfig.PcmFscLength = AP_FSC_LENGTH_1;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_FSC_START:
          PRINT_MESSAGE("FSC start position 1 bit before\n");
          PcmConfig.PcmFscStart = APF_START_1_BIT_BEFORE;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_CLK_FREQ:
          PRINT_MESSAGE("PCM interface clock 1152\n");
          PcmConfig.PcmClk = AP_PCM_CLK_1152;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_INTF_CLK:
          PRINT_MESSAGE("PCM clock mode rising\n");
          PcmConfig.PcmClkMode = AP_CLK_MODE_RISING;
          pcmcfgmode = PCM_CFG;
          break;
        default:
          break;
      }
      break;
    case '2':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
          pcmcfgmode = PCM_CFG_MODE;
          PRINT_MESSAGE("\n1 - PCM slave mode\n");
          PRINT_MESSAGE("2 - PCM master mode\n");
          PRINT_MESSAGE("3 - PCM slave no clock sync mode\n");
          break;
        case PCM_CFG_ENABLE:
          PRINT_MESSAGE("PCM interface disabled\n");
          PcmConfig.PcmEnable = APE_DISABLE;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_MODE:
          PRINT_MESSAGE("PCM master mode\n");
          PcmConfig.PcmMasterSlaveId = APM_MASTER;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_FSC_FREQ:
          PRINT_MESSAGE("FSC frequency 16 KHz\n");
          PcmConfig.PcmFscFreq = AP_FSC_FREQ_16KHZ;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_FSC_LEN:
          PRINT_MESSAGE("FSC length 8 bit\n");
          PcmConfig.PcmFscLength = AP_FSC_LENGTH_8;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_FSC_START:
          PRINT_MESSAGE("FSC start position aligned\n");
          PcmConfig.PcmFscStart = APF_START_ALIGNED;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_CLK_FREQ:
          PRINT_MESSAGE("PCM interface clock 2304\n");
          PcmConfig.PcmClk = AP_PCM_CLK_2304;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_INTF_CLK:
          PRINT_MESSAGE("PCM clock mode falling\n");
          PcmConfig.PcmClkMode = AP_CLK_MODE_FALLING;
          pcmcfgmode = PCM_CFG;
          break;
        default:
          break;
      }
      break;
    case '3':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
          pcmcfgmode = PCM_CFG_FSC_FREQ;
          PRINT_MESSAGE("\n1 - FSC frequency 8 KHz\n");
          PRINT_MESSAGE("2 - FSC frequency 16 Khz\n");
          break;
        case PCM_CFG_MODE:
          PRINT_MESSAGE("PCM slave no clock sync mode\n");
          PcmConfig.PcmMasterSlaveId = APM_SLAVE_NO_CLOCK_SYNC;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_FSC_LEN:
          PRINT_MESSAGE("FSC length 16 bit\n");
          PcmConfig.PcmFscLength = AP_FSC_LENGTH_16;
          pcmcfgmode = PCM_CFG;
          break;
        case PCM_CFG_CLK_FREQ:
          PRINT_MESSAGE("PCM interface clock 4608\n");
          PcmConfig.PcmClk = AP_PCM_CLK_4608;
          pcmcfgmode = PCM_CFG;
          break;
        default:
          break;
      }
      break;
    case '4':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
          pcmcfgmode = PCM_CFG_FSC_LEN;
          PRINT_MESSAGE("\n1 - FSC length 1 bit\n");
          PRINT_MESSAGE("2 - FSC length 8 bit\n");
          PRINT_MESSAGE("3 - FSC length 16 bit\n");
          PRINT_MESSAGE("4 - FSC length 32 bit\n");
          break;
        case PCM_CFG_FSC_LEN:
          PRINT_MESSAGE("FSC length 32 bit\n");
          PcmConfig.PcmFscLength = AP_FSC_LENGTH_32;
          pcmcfgmode = PCM_CFG;
          break;
        default:
          break;
      }
      break;
    case '5':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
          pcmcfgmode = PCM_CFG_FSC_START;
          PRINT_MESSAGE("\n1 - FSC start position 1 bit before\n");
          PRINT_MESSAGE("2 - FSC start position aligned\n");
          break;
        default:
          break;
      }
      break;
    case '6':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
          pcmcfgmode = PCM_CFG_CLK_FREQ;
          PRINT_MESSAGE("\n1 - PCM interface clock 1152\n");
          PRINT_MESSAGE("2 - PCM interface clock 2304\n");
          PRINT_MESSAGE("3 - PCM interface clock 4608\n");
          break;
        default:
          break;
      }
      break;
    case '7':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
          pcmcfgmode = PCM_CFG_INTF_CLK;
          PRINT_MESSAGE("\n1 - PCM clock mode rising\n");
          PRINT_MESSAGE("2 - PCM clock mode falling\n");
          break;
        default:
          break;
      }
      break;
    case '8':
      PRINT_MESSAGE("\nPcmEnable      = %s\n",ApiPcmEnableIdName[PcmConfig.PcmEnable]);
      PRINT_MESSAGE("PcmMasterSlave = %s\n",ApiPcmMasterSlaveIdName[PcmConfig.PcmMasterSlaveId]);
      PRINT_MESSAGE("PcmFscFreq     = %s\n",ApiPcmFscFreqIdName[PcmConfig.PcmFscFreq]);
      PRINT_MESSAGE("PcmFscLength   = %s\n",ApiPcmFscLengthIdName[PcmConfig.PcmFscLength]);
      PRINT_MESSAGE("PcmFscStart    = %s\n",ApiPcmFscStartIdName[PcmConfig.PcmFscStart]);
      PRINT_MESSAGE("PcmClk         = %s\n",ApiPcmClkIdName[PcmConfig.PcmClk]);
      PRINT_MESSAGE("PcmClkMode     = %s\n",ApiPcmClkModeIdName[PcmConfig.PcmClkMode]);
      break;

    case '9':
      switch(pcmcfgmode)
      {
        case PCM_CFG:
			//vm
        //  ApiFpInitPcmReqType * NewMailPtr;

          NewMailPtr = (ApiFpInitPcmReqType*) BUSM_AllocateMail(0, USER_TASK, (sizeof(ApiFpInitPcmReqType)+7));
          if(NewMailPtr != NULL)
          {
            NewMailPtr->Primitive      = H2M_16( API_FP_INIT_PCM_REQ );
            NewMailPtr->PcmEnable      = PcmConfig.PcmEnable;
            NewMailPtr->PcmMasterSlave = PcmConfig.PcmMasterSlaveId;
            NewMailPtr->PcmFscFreq     = PcmConfig.PcmFscFreq;
            NewMailPtr->PcmFscLength   = PcmConfig.PcmFscLength;
            NewMailPtr->PcmFscStart    = PcmConfig.PcmFscStart;
            NewMailPtr->PcmClk         = PcmConfig.PcmClk;
            NewMailPtr->PcmClkMode     = PcmConfig.PcmClkMode;
            BUSM_DeliverMail((uint8*) NewMailPtr);
          }
          PRINT_MESSAGE("PCM configuration send to target\n");
          pcmcfgmode = PCM_CFG_FALSE;
          break;
        default:
          break;
      }
      break;
    case '0':
      PRINT_MESSAGE("Exit PCM config mode without sending config to target\n");
      pcmcfgmode = PCM_CFG_FALSE;
      break;

    case 'h':
    case 'H':
      PRINT_MESSAGE("\nPCM configuration mode:\n");
      PRINT_MESSAGE("1 - PCM interface enable\n");
      PRINT_MESSAGE("2 - PCM interface mode\n");
      PRINT_MESSAGE("3 - FSC frequency\n");
      PRINT_MESSAGE("4 - FSC pulse length\n");
      PRINT_MESSAGE("5 - FSC start position\n");
      PRINT_MESSAGE("6 - PCM interface clock freq.\n");
      PRINT_MESSAGE("7 - PCM interface clock mode\n");
      PRINT_MESSAGE("8 - Show PCM config\n");
      PRINT_MESSAGE("9 - Exit PCM config mode and send config to target\n");
      PRINT_MESSAGE("0 - Exit without sending config to target\n");
      break;

    default:
      break;
  }
}

/****************************************************************************
*  FUNCTION: HandleKeyMessage
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: handle keypresses from the consol window
****************************************************************************/
void HandleKeyMessage(uint8 *MailPtr)
{
  if(pcmcfgmode != PCM_CFG_FALSE)
  {
    HandlePcmCfgMode(MailPtr);
  }
  else
  {
    switch(((recDataType*) MailPtr)->bData[0])
    {
#if 0
      case 'G':
      {
        extern volatile int bPrintRX;
        bPrintRX = 1;
      }
      	break;
      case 'g':
      {
        extern volatile int bPrintRX;
        bPrintRX = 0;
      }
      	break;
#endif
      case 'h':
        PRINT_MESSAGE("\nCVM_FP_API:\n");
        PRINT_MESSAGE("t   : Test communication to task (get target ID)\n");
        PRINT_MESSAGE("e/E : TestEeprom: Read/Write 8 bytes eep at adr 0x0A00\n");
        PRINT_MESSAGE("l/L : Set LED on/off (led 1)\n");
        PRINT_MESSAGE("P   : Goto PCM config mode\n");
        PRINT_MESSAGE("r   : Enable registration mode (delete last reg if no more reg's free)\n");
        PRINT_MESSAGE("i   : Simulate incoming call to all handsets - narrow/wide according to PCM\n");
        PRINT_MESSAGE("a/A : Switch PCM0 (handset 1) to narrowband (a)/wideband (A)\n");
        PRINT_MESSAGE("b/B : Switch PCM1 (handset 2) to narrowband (b)/wideband (B)\n");
        PRINT_MESSAGE("c/C : Switch PCM2 (handset 3) to narrowband (c)/wideband (C)\n");
        PRINT_MESSAGE("d/D : Switch PCM3 (handset 4) to narrowband (d)/wideband (D)\n");
        PRINT_MESSAGE("F   : Firmware update (filename: \"test.fwu\" in same directory as cvmcon.exe)\n\n");
        PRINT_MESSAGE("M/m : Set firmware name (filename: \"Cvm480Fp_v0113.fwu\" / \"Cvm480Fp_v0112.fwu\" )\n\n");
        break;

      case 't':
        BUSM_SendMailP2(0, USER_TASK, API_FP_GET_ID_REQ, (uint8) debug_ProgramId, 0);
        PRINT_MESSAGE("Testing communication to Usertask\n");
        PRINT_MESSAGE("<-API_FP_GET_ID_REQ\n");
        break;

      case 'e':
        BUSM_SendMailP3(0, USER_TASK, API_FP_GET_EEPROM_REQ, 0x00, 0x0A, 8); // read 8 bytes of eeprom at adr 0x0800
        PRINT_MESSAGE("Testing: eeprom read 8 bytes at adr 0x0A00\n");
        PRINT_MESSAGE("<-API_GET_EEPROM_REQ\n");
        break;

      case 'E':
        {
          ApiFpSetEepromReqType * NewMailPtr;

          NewMailPtr = (ApiFpSetEepromReqType*) BUSM_AllocateMail(0, USER_TASK, (sizeof(ApiFpSetEepromReqType)+7));
          if(NewMailPtr != NULL)
          {
            NewMailPtr->Primitive  = H2M_16( API_FP_SET_EEPROM_REQ );
            NewMailPtr->Address    = H2M_16( 0x0A00 );
            NewMailPtr->DataLength = 8;
            NewMailPtr->Data[0]    = 0x11;
            NewMailPtr->Data[1]    = 0x22;
            NewMailPtr->Data[2]    = 0x33;
            NewMailPtr->Data[3]    = 0x44;
            NewMailPtr->Data[4]    = 0x55;
            NewMailPtr->Data[5]    = 0x66;
            NewMailPtr->Data[6]    = 0x77;
            NewMailPtr->Data[7]    = 0x88;
            BUSM_DeliverMail((uint8*) NewMailPtr); // write 8 bytes in eeprom at adr 0x0A00
          }
          PRINT_MESSAGE("Testing: eeprom write 8 bytes at adr 0x0A00\n");
          PRINT_MESSAGE("<-API_FP_SET_EEPROM_REQ\n");
        }
        break;

      case 'l':
        {
          ApiFpSetLedReqType * NewMailPtr;

          NewMailPtr = (ApiFpSetLedReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpSetLedReqType));
          if(NewMailPtr != NULL)
          {
            NewMailPtr->Primitive          = H2M_16( API_FP_SET_LED_REQ );
            NewMailPtr->LED                = AL_LED1;
            NewMailPtr->Commands->Command  = ALC_LED_ON;
            NewMailPtr->Commands->Duration = H2M_16( 0 );
            NewMailPtr->CommandsCount      = 1;
            BUSM_DeliverMail((uint8*) NewMailPtr);
          }
          PRINT_MESSAGE("Turn on LED 1\n");
          PRINT_MESSAGE("<-API_FP_SET_LED_REQ\n");
        }
        break;

      case 'L':
        {
          ApiFpSetLedReqType * NewMailPtr;

          NewMailPtr = (ApiFpSetLedReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpSetLedReqType));
          if(NewMailPtr != NULL)
          {
            NewMailPtr->Primitive          = H2M_16( API_FP_SET_LED_REQ );
            NewMailPtr->LED                = AL_LED1;
            NewMailPtr->Commands->Command  = ALC_LED_OFF;
            NewMailPtr->Commands->Duration = H2M_16( 0 );
            NewMailPtr->CommandsCount      = 1;
            BUSM_DeliverMail((uint8*) NewMailPtr);
          }
          PRINT_MESSAGE("Turn off LED 1\n");
          PRINT_MESSAGE("<-API_FP_SET_LED_REQ\n");
        }
        break;

      case 'P':
        pcmcfgmode = PCM_CFG;
        PRINT_MESSAGE("\nPCM configuration mode:\n");
        PRINT_MESSAGE("1 - PCM interface enable\n");
        PRINT_MESSAGE("2 - PCM interface mode\n");
        PRINT_MESSAGE("3 - FSC frequency\n");
        PRINT_MESSAGE("4 - FSC pulse length\n");
        PRINT_MESSAGE("5 - FSC start position\n");
        PRINT_MESSAGE("6 - PCM interface clock freq.\n");
        PRINT_MESSAGE("7 - PCM interface clock mode\n");
        PRINT_MESSAGE("8 - Show PCM config\n");
        PRINT_MESSAGE("9 - Exit PCM config mode and send config to target\n");
        PRINT_MESSAGE("0 - Exit without sending config to target\n");
        break;

      case 'r':
        BUSM_SendMailP2(0, USER_TASK, API_FP_SET_REGISTRATION_MODE_REQ, 0x01, 0x01);
        PRINT_MESSAGE("Enable registration mode and last handset deleted if no free reg's\n");
        PRINT_MESSAGE("<-API_FP_SET_REGISTRATION_MODE_REQ\n");
        break;

      case 'i':
        {
          uint8 i;
          for(i=0;i<NO_HS;i++)
          {
            if(CallState[i].State == F00_NULL)
            {
              ApiFpSetupReqType * NewMailPtr;
              ApiCodecListType CodecList;

              NewMailPtr = (ApiFpSetupReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpSetupReqType) - 1 + sizeof(CodecList) + sizeof(CodecList.Codec));
              if(NewMailPtr != NULL)
              {

                NewMailPtr->Primitive           = H2M_16( API_FP_SETUP_REQ );
                NewMailPtr->HandsetId           = i+1;
                NewMailPtr->CallClass           = ACC_NORMAL;
                NewMailPtr->SourceId            = HandsetPcmChannel[i];
                NewMailPtr->Signal              = ASV_ALERTING_ON_PATTERN1;
                NewMailPtr->ClipLength          = 0;
                NewMailPtr->CallingNumberLength = 0;
                NewMailPtr->CallingNameLength   = 0;
                NewMailPtr->IwuLength           = 0;
                NewMailPtr->ProprietaryLength   = 0;
                switch(PcmDataFormat[i])
                {
                  case AP_DATA_FORMAT_LINEAR_8kHz:
                    NewMailPtr->BasicService       = ABS_BASIC_SPEECH;
                    NewMailPtr->CodecListLength    = sizeof(CodecList);

                    CodecList.NoOfCodecs           = 1;
                    CodecList.NegotiationIndicator = ANI_NOT_POSSIBLE;
                    memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));

                    memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));
                    break;
                  case AP_DATA_FORMAT_LINEAR_16kHz:
                    NewMailPtr->BasicService       = ABS_WIDEBAND_SPEECH;
                    NewMailPtr->CodecListLength    = sizeof(CodecList) + sizeof(CodecList.Codec);

                    CodecList.NoOfCodecs           = 2;
                    CodecList.NegotiationIndicator = ANI_POSSIBLE;
                    memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));

                    memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));
                    memcpy((void*)&NewMailPtr->Data[sizeof(CodecList)], (void*)&CodecG726, sizeof(CodecList.Codec));
                    break;
                }
                PRINT_MESSAGE("<-API_FP_SETUP_REQ - HandsetID: %X\n", i+1);
                DisplayCodecList(NewMailPtr->CodecListLength, NewMailPtr->Data);
                BUSM_DeliverMail((uint8*)NewMailPtr);
              }
			  SetCallState(i,F07_CALL_RECEIVED);
             // CallState[i].State = F07_CALL_RECEIVED;

            }
          }
        }
        break;
      case 'a':
      case 'b':
      case 'c':
      case 'd':
        PcmDataFormat[((recDataType*) MailPtr)->bData[0] - 'a'] = AP_DATA_FORMAT_LINEAR_8kHz;
        PRINT_MESSAGE("PCM%X, handset %X set to narrowband\n", ((recDataType*) MailPtr)->bData[0] - 'a', ((recDataType*) MailPtr)->bData[0] - 'a' + 1);
        break;

      case 'A':
      case 'B':
      case 'C':
      case 'D':
        PcmDataFormat[((recDataType*) MailPtr)->bData[0] - 'A'] = AP_DATA_FORMAT_LINEAR_16kHz;
        if(PcmConfig.PcmFscFreq == AP_FSC_FREQ_8KHZ)
        {
          PRINT_MESSAGE("Error: PCM setup needs to be 16 kHz to enable wideband calls\n");
        }
        else
        {
          PRINT_MESSAGE("PCM%X, handset %X set to wideband\n", ((recDataType*) MailPtr)->bData[0] - 'A', ((recDataType*) MailPtr)->bData[0] - 'A' + 1);
        }
        break;

      case 'F':
        FwuStart();
        break;
      case 'M':
        FwuSetFile( FWU_FILENAME );
        PRINT_MESSAGE("Set firmware " FWU_FILENAME "\n" );
        break;
      case 'm':
        FwuSetFile( FWU_FILENAME_SEC );
        PRINT_MESSAGE("Set firmware " FWU_FILENAME_SEC "\n" );
        break;

    }
  }
}

/****************************************************************************
*  FUNCTION: HandleGetFwuVersionCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION:
****************************************************************************/
void HandleGetFwuVersionCfm(CvmApiMailType *MailPtr)
{
  #define M MailPtr->ApiFpGetFwVersionCfm
	if(M.Status == AS_SUCCESS)
	{
		ref_VersionHex = M.VersionHex;
		
		PRINT_MESSAGE("->API_FP_GET_FW_VERSION_CFM - Status: %X, VersionHex: %lX, LinkDate: %02X.%02X.%02X.%02X.%02X, DectType: %X\n",
			M.Status,
			M.VersionHex,
			M.LinkDate[0],
			M.LinkDate[1],
			M.LinkDate[2],
			M.LinkDate[3],
			M.LinkDate[4],
			M.DectType);
	}
	else
	{
		PRINT_MESSAGE(" !!! API_FP_GET_FW_VERSION_REQ failed!!!\n");
	}

  #undef M
}

/****************************************************************************
*  FUNCTION: HandleSetupInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle incoming setup from handsets (API_FP_SETUP_IND)
*               Send API_FP_SETUP_RES to handset
*               Send API_FP_SET_PCM_CODEC_REQ to setup PCM CODEC (wide/narrow)
*               Send API_FP_CONNECT_REQ to establish call
*               - include codec list to support both narrow/wide-band calls
*
*	OUTGOING CALL
****************************************************************************/
void HandleSetupInd(CvmApiMailType *MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpSetupInd.HandsetId;
	//uint8 lineId;
	uint8 codecListLength = MailPtr->ApiFpSetupInd.CodecListLength;
	ApiCodecListType *codecListPtr =
		(ApiCodecListType*)&MailPtr->ApiFpSetupInd.Data[MailPtr->ApiFpSetupInd.KeypadLength +
                                                 MailPtr->ApiFpSetupInd.CalledNumberLength +
                                                 MailPtr->ApiFpSetupInd.CallingNumberLength];


      PRINT_MESSAGE("->API_FP_SETUP_IND\n");
      #define M ((ApiFpSetupIndType*)MailPtr)
      PRINT_MESSAGE("\tHandsetId = %X\n",M->HandsetId);
      PRINT_MESSAGE("\tDestinationId = %X\n",M->DestinationId);
      PRINT_MESSAGE("\tBasicService = %X\n",M->BasicService);
      PRINT_MESSAGE("\tCallClass = %X\n",M->CallClass);
      PRINT_MESSAGE("\tKeypadLength = %X\n",M->KeypadLength);
      PRINT_MESSAGE("\tCalledNumberLength = %X\n",M->CalledNumberLength);
      PRINT_MESSAGE("\tCallingNumberLength = %X\n",M->CallingNumberLength);
      PRINT_MESSAGE("\tCodecListLength = %X\n",M->CodecListLength);
      PRINT_MESSAGE("\tIwuLength = %X\n",M->IwuLength);
      PRINT_MESSAGE("\tProprietaryLength = %X\n",M->ProprietaryLength);

      #undef M


	  //line_connect = 0;

	switch(CallState[handsetId - 1].State)
	{
		case HS_OUT_OF_RANGE:
		case F00_NULL:
		if(MailPtr->ApiFpSetupInd.CallClass == ACC_NORMAL)
		{
//vm modify to check if there is available pcm channel
			PRINT_MESSAGE("->API_FP_SETUP_IND - HandsetID: %X ACC_NORMAL\n", handsetId);
			// Display codec list
			DisplayCodecList(codecListLength, (uint8 *)codecListPtr);
			if(MailPtr->ApiFpSetupInd.KeypadLength > 0)
			{
				uint8 i;
				PRINT_MESSAGE("  Multikeypad: ");
				for(i=0;i<MailPtr->ApiFpSetupInd.KeypadLength;i++)
				{
					PRINT_MESSAGE("%X ", MailPtr->ApiFpSetupInd.Data[i]);
				}
				PRINT_MESSAGE("\n");
			}
#ifdef DYNAMIC_CHANNEL_ALLOCATION
			if(!try_pcm_channel_allocation){//no pcm channel available
				BUSM_SendMailP3(0, USER_TASK, API_FP_SETUP_RES, handsetId, AS_REJECTED, HandsetPcmChannel[handsetId - 1]);
				PRINT_MESSAGE("<-API_FP_SETUP_RES - HandsetID: %X\n", handsetId);
				PRINT_INFO("Connection rejected. No PCM channel available. \n");
				display_pcm_channel_status();
				return;
			}
#endif	
#ifdef DECT_NATALIE	//in natalie we dont know the associated adpcm channel	before the connect_conf msg. We setup the pcm codec there.
			// Send API_FP_SETUP_RES as a result of API_FP_SETUP_IND
			BUSM_SendMailP3(0, USER_TASK, API_FP_SETUP_RES, handsetId, AS_SUCCESS, HandsetPcmChannel[handsetId - 1]);
			PRINT_MESSAGE("<-API_FP_SETUP_RES - HandsetID: %X\n", handsetId);

			// Setup PCM codec to correct data format
			// if((MailPtr->ApiFpSetupInd.BasicService == ABS_WIDEBAND_SPEECH) && (PcmConfig.PcmFscFreq == AP_FSC_FREQ_16KHZ))
			// {
				// BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_LINEAR_16kHz);
				// PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_LINEAR_16kHz);
			// }
			// else
			// {
				// BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_CWB_ALAW/*AP_DATA_FORMAT_LINEAR_8kHz*/);
				// PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_CWB_ALAW/*AP_DATA_FORMAT_LINEAR_8kHz*/);
			// }
#else
#ifdef DYNAMIC_CHANNEL_ALLOCATION
			HandsetPcmChannel[handsetId - 1] = allocate_pcm_channel();
			display_pcm_channel_status();
#endif	
			{
				BUSM_SendMailP3(0, USER_TASK, API_FP_SETUP_RES, handsetId, AS_SUCCESS, HandsetPcmChannel[handsetId - 1]);
				PRINT_MESSAGE("<-API_FP_SETUP_RES - HandsetID: %X\n", handsetId);
				if(MailPtr->ApiFpSetupInd.BasicService == ABS_WIDEBAND_SPEECH)
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_WIDEBAND_SPEECH");
				}
				else
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_NARROWBAND_SPEECH");
				}
			}
#endif
			// Send API_FP_CONNECT_REQ with correct codec list for the handset air-link
			{
			ApiFpConnectReqType *NewMailPtr;
			ApiCodecListType CodecList;

			NewMailPtr = (ApiFpConnectReqType *) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpConnectReqType ) - 1 + sizeof(CodecList));
			if(NewMailPtr != NULL)
			{
				NewMailPtr->Primitive       = H2M_16( API_FP_CONNECT_REQ );
				NewMailPtr->HandsetId       = handsetId;
				NewMailPtr->CodecListLength = sizeof(CodecList);

				CodecList.NoOfCodecs           = 1;
				CodecList.NegotiationIndicator = ANI_POSSIBLE;//ANI_NOT_POSSIBLE;
//#ifndef DECT_NATALIE
				if(MailPtr->ApiFpSetupInd.BasicService == ABS_WIDEBAND_SPEECH)
				{
					// if(PcmConfig.PcmFscFreq == AP_FSC_FREQ_16KHZ)
					// {
						memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));
						PRINT_MESSAGE("  Set air-codec to 722 \n");
					// }
					// else
					// {
						// PRINT_MESSAGE("  !!! handset requested wideband and PCM is set to 8 kHz !!!\n");
						// PRINT_MESSAGE("  !!! compression is used !!!\n");
						// memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));
					// }
				}
				else
//#endif
				{
					memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));
					PRINT_MESSAGE("  Set air-codec to 726 \n");
				}

				memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));

				PRINT_MESSAGE("<-API_FP_CONNECT_REQ - HandsetID: %X\n", MailPtr->ApiFpSetupInd.HandsetId);
				// Display codec list
				DisplayCodecList(NewMailPtr->CodecListLength, NewMailPtr->Data);

				BUSM_DeliverMail((uint8*)NewMailPtr);
			}
			}
			CallState[handsetId - 1].State = F01_CALL_INITIATED;
			CallState[handsetId - 1].CallClass = ACC_NORMAL;
			CallState[handsetId - 1].Connected2Line = TRUE;
			CallState[handsetId - 1].BasicService = MailPtr->ApiFpSetupInd.BasicService;//store codec, to use it on AP_CALLCNTRL_OUTGOING_CALL_REQ

			PRINT_MESSAGE(" CallState = F01_CALL_INITIATED\n");
		}
		else if(MailPtr->ApiFpSetupInd.CallClass == ACC_INTERNAL)
		{
			PRINT_MESSAGE("->API_FP_SETUP_IND - HandsetID: %X ACC_INTERNAL\n", MailPtr->ApiFpSetupInd.HandsetId);

			// Display codec list
			DisplayCodecList(codecListLength, (uint8 *)codecListPtr);


			CallState[handsetId - 1].CallClass = ACC_INTERNAL;
			CallState[handsetId - 1].BasicService = MailPtr->ApiFpSetupInd.BasicService;


			// Free old codec list (if any).
			if (CallState[handsetId - 1].ProposedCodecListLength > 0)
			{
				CallState[handsetId - 1].ProposedCodecListLength = 0;
				free(CallState[handsetId - 1].ProposedCodecListPtr);
			}
			// Store the propsed codec list (if any).
			if (codecListLength)
			{
				CallState[handsetId - 1].ProposedCodecListLength = codecListLength;
				CallState[handsetId - 1].ProposedCodecListPtr = (ApiCodecListType*)malloc(codecListLength);
				memcpy(CallState[handsetId-1].ProposedCodecListPtr,codecListPtr, codecListLength);

			}
			if (MailPtr->ApiFpSetupInd.KeypadLength == 0)
			{
				// if no dest is included then change state to "overlap sending"
				SetCallState(handsetId - 1, F02_OVERLAP_SENDING);
				//  SendApiFpSetupRes(CVMAPI_TASK, handsetId, AS_SUCCESS, 0);
				/*The following conditional check is used for fixing the wideband internal call issue with the Siemens handsets*/
				if( MailPtr->ApiFpSetupInd.BasicService == ABS_WIDEBAND_SPEECH ){
					BUSM_SendMailP3(0, USER_TASK, API_FP_SETUP_RES, handsetId, AS_SUCCESS,0);
					PRINT_MESSAGE("<-API_FP_SETUP_RES - HandsetID: %X\n", handsetId);
					//  SendApiFpSetupAckReq(CVMAPI_TASK, handsetId, ASV_DIAL_TONE_ON, AP_IN_BAND_NOT_AVAILABLE, 0, 0, 0, NULL);

					ApiFpCallProcReqType*
					m = (ApiFpCallProcReqType*)BUSM_AllocateMail(0,USER_TASK,(uint16)(sizeof(ApiFpCallProcReqType) - 1 +
							0));

					m->Primitive = H2M_16( API_FP_CALL_PROC_REQ );
					m->HandsetId = handsetId;
					m->Signal = ASV_DIAL_TONE_ON;
					m->ProgressInd = AP_IN_BAND_NOT_AVAILABLE;
					m->CodecListLength = 0;//no codec returned, until negotiation is performed

					BUSM_DeliverMail((uint8*)m);
					PRINT_MESSAGE("<-API_FP_CALL_PROC_REQ - HandsetID: %X\n", handsetId);				
				}
				else{
					ApiFpSetupAckReqType*
					m1 = (ApiFpSetupAckReqType*)BUSM_AllocateMail(0,USER_TASK,(uint16)(sizeof(ApiFpSetupAckReqType) - 1 +
							0 +
							0 +
							0));

					m1->Primitive = H2M_16( API_FP_SETUP_ACK_REQ );
					m1->HandsetId = handsetId;
					m1->Signal = ASV_DIAL_TONE_ON;
					m1->ProgressInd = AP_IN_BAND_NOT_AVAILABLE;
					m1->CodecListLength = 0;
					m1->IwuLength = 0;
					m1->ProprietaryLength = 0;

					BUSM_DeliverMail((uint8*)m1);
					PRINT_MESSAGE("<-API_FP_SETUP_ACK_REQ - HandsetID: %X\n", handsetId);
				}
			}
			else
			{
				uint8 destId = MailPtr->ApiFpSetupInd.Data[0] - '0'; // FIX ME
				if (destId > 0 && destId <= NO_HS &&
					(CallState[destId-1].State == F00_NULL || CallState[destId-1].State == HS_OUT_OF_RANGE))
				{
					// setup the call to the other handset...
					CallState[handsetId-1].DestinationId = destId;
					CallState[destId-1].CallClass = ACC_INTERNAL;
					CallState[destId-1].DestinationId = handsetId;

					ApiFpSetupReqType * NewMailPtr;

					NewMailPtr = (ApiFpSetupReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpSetupReqType) - 1 + MailPtr->ApiFpSetupInd.CodecListLength);
					if(NewMailPtr != NULL)
					{
						NewMailPtr->Primitive           = H2M_16( API_FP_SETUP_REQ );
						NewMailPtr->HandsetId           = destId;
						NewMailPtr->BasicService		= CallState[handsetId-1].BasicService;
						NewMailPtr->CallClass           = ACC_INTERNAL;
						NewMailPtr->SourceId            = handsetId;
						NewMailPtr->Signal              = ASV_ALERTING_ON_PATTERN0_INT;
						NewMailPtr->ClipLength          = 0;
						NewMailPtr->CallingNumberLength = 0;
						NewMailPtr->CallingNameLength   = 0;
						NewMailPtr->IwuLength           = 0;
						NewMailPtr->ProprietaryLength   = 0;
						NewMailPtr->CodecListLength		= MailPtr->ApiFpSetupInd.CodecListLength;


						#define M MailPtr->ApiFpSetupInd
						memcpy((void*)&NewMailPtr->Data[0], (void*)&M.Data[M.KeypadLength + M.CalledNumberLength + M.CallingNumberLength], M.CodecListLength);
						#undef M

						PRINT_MESSAGE("<-API_FP_SETUP_REQ - HandsetID: %X\n", handsetId);
						DisplayCodecList(NewMailPtr->CodecListLength, NewMailPtr->Data);
						BUSM_DeliverMail((uint8*)NewMailPtr);
					}
				}
				else
				{
					//SendApiFpSetupRes(CVMAPI_TASK, handsetId, AS_REJECTED, 0);
					BUSM_SendMailP3(0, USER_TASK, API_FP_SETUP_RES, handsetId, AS_REJECTED,0);
					PRINT_MESSAGE("<-API_FP_SETUP_RES - HandsetID: %X\n", handsetId);
				}
			}
		}
		else if (MailPtr->ApiFpSetupInd.CallClass == ACC_SERVICE)
		{
			//SendApiFpSetupRes(CVMAPI_TASK, handsetId, AS_REJECTED, 0);
			BUSM_SendMailP3(0, USER_TASK, API_FP_SETUP_RES, handsetId, AS_REJECTED,0);
			PRINT_MESSAGE("<-API_FP_SETUP_RES - HandsetID: %X\n", handsetId);
		}
		break;
	case F01_CALL_INITIATED:
	case F02_OVERLAP_SENDING:
	case F03_CALL_PROCEEDING:
	case F04_CALL_DELIVERED:
	case F06_CALL_PRESENT:
	case F07_CALL_RECEIVED:
	case F10_ACTIVE:
	case F19_RELEASE_PENDING:
		BUSM_SendMailP3(0, USER_TASK, API_FP_SETUP_RES, handsetId, AS_REJECTED,0);
		PRINT_MESSAGE("<-API_FP_SETUP_RES - HandsetID: %X\n", handsetId);
		break;
	default:
		break;

  }
}

/****************************************************************************
*  FUNCTION: HandleSetPcmCodecCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle result of PCM CODEC setup (API_FP_SET_PCM_CODEC_CFM)
****************************************************************************/
void HandleSetPcmCodecCfm(CvmApiMailType *MailPtr)
{
  if(MailPtr->ApiFpSetPcmCodecCfm.Status == AS_SUCCESS)
  {
    PRINT_MESSAGE("->API_FP_SET_PCM_CODEC_CFM\n");
  }
  else
  {
	  PRINT_MESSAGE("->API_FP_SET_PCM_CODEC_CFM  !!! REQUEST FAILED ....\n");
  }

}

/****************************************************************************
*  FUNCTION: HandleConnectCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle result of API_FP_CONNECT_REQ
*               Call is now active -> Set callstate of handset to CALL_PRESENT
****************************************************************************/
void HandleConnectCfm(CvmApiMailType *MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpConnectCfm.HandsetId;
#ifdef full_mmi
#ifdef OLD_CC
	ap_callcntrl_msg_u_type msg;
#endif
#endif
	switch(CallState[handsetId - 1].State)
	{
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F10_ACTIVE:
			if (MailPtr->ApiFpConnectCfm.Status != AS_SUCCESS)
			{
				PRINT_MESSAGE("->API_FP_CONNECT_CFM - HandsetID: %X !!!!! FAILED !!!!\n", handsetId);
			}
			else
			{
#ifdef DECT_NATALIE
				PRINT_MESSAGE("->API_FP_CONNECT_CFM - HandsetID: %X, AdpcmChannel : %d \n", handsetId, MailPtr->ApiFpConnectCfm.AdpcmChannel);
				HandsetPcmChannel[handsetId - 1] = MailPtr->ApiFpConnectCfm.AdpcmChannel;
#ifdef DYNAMIC_CHANNEL_ALLOCATION						
				bind_pcm_channel(HandsetPcmChannel[handsetId - 1]);
				display_pcm_channel_status();
#endif					
				if(CallState[handsetId - 1].BasicService == ABS_WIDEBAND_SPEECH)
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_WIDEBAND_SPEECH");
				}
				else
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_NARROWBAND_SPEECH");
				}

#else
				PRINT_MESSAGE("->API_FP_CONNECT_CFM - HandsetID: %X\n", handsetId);
#endif
				SetCallState(handsetId- 1, F10_ACTIVE);
				PRINT_MESSAGE(" CallState = F10_ACTIVE\n");
#ifdef full_mmi
#ifdef OLD_CC
				/*HOOK - OFF */
				PRINT_MESSAGE("================ hook off to host ==================\n");
				msg.ap_callcntrl_hook_off_ind.msg_id = AP_CALLCNTRL_HOOK_OFF_IND;
				msg.ap_callcntrl_hook_off_ind.line = HS_TO_LINE(handsetId);
				msg.ap_callcntrl_hook_off_ind.pcm_port = ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]);
				if(CallState[handsetId - 1].BasicService == ABS_BASIC_SPEECH)
					msg.ap_callcntrl_hook_off_ind.codec=DECT_NARROWBAND_CODEC ;				     
				else
					msg.ap_callcntrl_hook_off_ind.codec=DECT_WIDEBAND_CODEC;				
				  ap_send_msg_to_callcntrl( &msg );
#else
				  int codec_sent=0;
				  if(CallState[handsetId - 1].BasicService == ABS_BASIC_SPEECH)
					  codec_sent=CCFSM_NARROWBAND_CODEC ;				     
				  else
					  codec_sent=CCFSM_WIDEBAND_CODEC;

//				  sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
//						  (int)HS_TO_LINE(handsetId), CCFSM_HOOK_OFF, codec_sent);
				  sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
				  						  (int)HS_TO_LINE(handsetId), CCFSM_HOOK_OFF, codec_sent, CallState[handsetId - 1].LineId);				  
#endif
#endif
			}
			break;
		case F00_NULL:
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
		case F19_RELEASE_PENDING:
			PRINT_MESSAGE(" !!! Unexpected API_FP_CONNECT_CFM !!!\n");
			break;
		default:
			break;
  }
}

/****************************************************************************
*  FUNCTION: HandleReleaseInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_RELEASE_IND from handset
*               Send API_FP_RELEASE_RES to handset
*               Set callstate of handset to NULL
****************************************************************************/
void HandleReleaseInd(CvmApiMailType *MailPtr)
{


	uint8 handsetId = MailPtr->ApiFpReleaseInd.HandsetId;

#ifdef full_mmi
#ifdef OLD_CC
	ap_callcntrl_msg_u_type msg;
#endif
#endif

	switch(CallState[handsetId - 1].State)
	{
		case F00_NULL:
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
		case F10_ACTIVE:
		case F19_RELEASE_PENDING:

//    case F06_CALL_PRESENT:
			PRINT_MESSAGE("->API_FP_RELEASE_IND - HandsetID: %X\n", handsetId);
			PRINT_MESSAGE("<-API_FP_RELEASE_RES - handsetID: %X\n", handsetId);

			BUSM_SendMailP2(0, USER_TASK, API_FP_RELEASE_RES, handsetId, AS_SUCCESS);


			PRINT_MESSAGE("CallState.CallClass %X \n",CallState[handsetId - 1].CallClass);


			if (CallState[handsetId - 1].CallClass == ACC_INTERNAL)
			{
				if (CallState[handsetId-1].DestinationId > 0 && CallState[CallState[handsetId-1].DestinationId].State != F00_NULL)
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_RELEASE_REQ, CallState[handsetId-1].DestinationId, ARR_NORMAL);
					PRINT_MESSAGE("<-API_FP_RELEASE_REQ - handsetID: %X\n", CallState[handsetId-1].DestinationId);
				}
			}
			//vm check state machine
			else //if (CallState[handsetId-1].CallClass == ACC_NORMAL)
			{
				ApiFpSetPcmCodecReqType*
				m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));
				m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
				m2->PcmChannel =  HandsetPcmChannel[handsetId-1];
				m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
				BUSM_DeliverMail((uint8*)m2);
				PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[MailPtr->ApiFpReleaseInd.HandsetId - 1], AP_DATA_FORMAT_NONE);
#ifdef full_mmi
#ifdef OLD_CC
				/*HOOK ON*/
				msg.ap_callcntrl_hook_on_ind.msg_id = AP_CALLCNTRL_HOOK_ON_IND;
				msg.ap_callcntrl_hook_on_ind.line = HS_TO_LINE(handsetId);
				ap_send_msg_to_callcntrl( &msg );
#else
//        sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)HandsetPcmChannel[handsetId - 1], 
//          (int)HS_TO_LINE(handsetId), CCFSM_HOOK_ON, 0);//codec does not matter
				sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
						(int)HS_TO_LINE(handsetId), CCFSM_HOOK_ON, 0, CallState[handsetId - 1].LineId);//codec does not matter				        
#endif
#endif
				key_seq_index = 0;
			}
			SetCallState(handsetId - 1, F00_NULL);
			PRINT_MESSAGE(" CallState = F00_NULL\n");
		break;
		default:
			break;
	}
}

/****************************************************************************
*  FUNCTION: HandleResetInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_RESET_IND from CVM480 module
*               Send active PCM interface configuration to module
*               Request number of registrations
****************************************************************************/
void HandleResetInd(CvmApiMailType *MailPtr)
{
  uint8 i;
  ApiFpInitPcmReqType * NewMailPtr;


  for(i=0;i<NO_HS;i++)
  {
    SetCallState(i,HS_NOT_REGISTERED);
  }

  PRINT_MESSAGE("->API_FP_RESET_IND\n");
  
  // stop to sync RX DLE 
  EnableSyncRxDLE( 0 );
  
#ifdef CVM480
#if 0
  {
  ApiFpSetRegisterReqType*
  m = (ApiFpSetRegisterReqType*)BUSM_AllocateMail(0, USER_TASK,sizeof(ApiFpSetRegisterReqType));

  m->Primitive = H2M_16( API_FP_SET_REGISTER_REQ );
  m->RegisterAddress = H2M_32( 0xff4940 );
  m->Data = H2M_16( 0x8000 );
  m->Mask = H2M_16( 0x8020 );
  BUSM_DeliverMail((uint8 *)m);

  PRINT_MESSAGE("<-API_FP_SET_REGISTER_REQ\n");

}
#endif

{
  ApiFpGetRegisterReqType*
  m1 = (ApiFpGetRegisterReqType*)BUSM_AllocateMail(0, USER_TASK,sizeof(ApiFpGetRegisterReqType));

  m1->Primitive = H2M_16( API_FP_GET_REGISTER_REQ );
  m1->RegisterAddress = H2M_32( 0xff4940 );
  BUSM_DeliverMail((uint8 *)m1);

  PRINT_MESSAGE("<-API_FP_GET_REGISTER_REQ\n");
}

#endif

  BUSM_SendMailP0(0, USER_TASK, API_FP_GET_REGISTRATION_COUNT_REQ);
  PRINT_MESSAGE("<-API_FP_GET_REGISTRATION_COUNT_REQ\n");

  BUSM_SendMailP0(0, USER_TASK, API_FP_GET_FW_VERSION_REQ);
  PRINT_MESSAGE("<-API_FP_GET_FW_VERSION_REQ\n");

  BUSM_SendMailP0(0, USER_TASK, API_FP_GET_MODEL_REQ);
  PRINT_MESSAGE("<-API_FP_GET_MODEL_REQ\n");

  BUSM_SendMailP0(0, USER_TASK, API_FP_GET_ID_REQ);
  PRINT_MESSAGE("<-API_FP_GET_ID_REQ\n");

  BUSM_SendMailP0(0, USER_TASK, API_FP_GET_ACCESS_CODE_REQ);
  PRINT_MESSAGE("<-API_FP_GET_ACCESS_CODE_REQ\n");




  PRINT_MESSAGE("\nPcmEnable      = %s\n",ApiPcmEnableIdName[PcmConfig.PcmEnable]);
  PRINT_MESSAGE("PcmMasterSlave = %s\n",ApiPcmMasterSlaveIdName[PcmConfig.PcmMasterSlaveId]);
  PRINT_MESSAGE("PcmFscFreq     = %s\n",ApiPcmFscFreqIdName[PcmConfig.PcmFscFreq]);
  PRINT_MESSAGE("PcmFscLength   = %s\n",ApiPcmFscLengthIdName[PcmConfig.PcmFscLength]);
  PRINT_MESSAGE("PcmFscStart    = %s\n",ApiPcmFscStartIdName[PcmConfig.PcmFscStart]);
  PRINT_MESSAGE("PcmClk         = %s\n",ApiPcmClkIdName[PcmConfig.PcmClk]);
  PRINT_MESSAGE("PcmClkMode     = %s\n",ApiPcmClkModeIdName[PcmConfig.PcmClkMode]);

  NewMailPtr = (ApiFpInitPcmReqType*) BUSM_AllocateMail(0, USER_TASK, (sizeof(ApiFpInitPcmReqType)+7));
  if(NewMailPtr != NULL)
  {
    NewMailPtr->Primitive      = H2M_16( API_FP_INIT_PCM_REQ );
    NewMailPtr->PcmEnable      = PcmConfig.PcmEnable;
    NewMailPtr->PcmMasterSlave = PcmConfig.PcmMasterSlaveId;
    NewMailPtr->PcmFscFreq     = PcmConfig.PcmFscFreq;
    NewMailPtr->PcmFscLength   = PcmConfig.PcmFscLength;
    NewMailPtr->PcmFscStart    = PcmConfig.PcmFscStart;
    NewMailPtr->PcmClk         = PcmConfig.PcmClk;
    NewMailPtr->PcmClkMode     = PcmConfig.PcmClkMode;
    BUSM_DeliverMail((uint8*) NewMailPtr);
  }
  PRINT_MESSAGE("PCM configuration send to target\n");
  PRINT_MESSAGE("<-API_FP_INIT_PCM_REQ\n");

  go_on_with_ata = 1;
#if 0
	BUSM_SendMailP2(0, USER_TASK, API_FP_SET_REGISTRATION_MODE_REQ, 0x01, 0x01);
	PRINT_MESSAGE("Enable registration mode and last handset deleted if no free reg's\n");
	PRINT_MESSAGE("<-API_FP_SET_REGISTRATION_MODE_REQ\n");
#endif

//  StartRegistrationTimer(TRUE);


}

/****************************************************************************
*  FUNCTION: HandleHandsetPresentInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_HANDSET_PRESENT_IND
*               Set correct callstate of handset
****************************************************************************/
void HandleHandsetPresentInd(CvmApiMailType *MailPtr)
{
  PRINT_MESSAGE("->API_FP_HANDSET_PRESENT_IND\n");
  PRINT_MESSAGE("Handset ID        : %X\n", MailPtr->ApiFpHandsetPresentInd.HandsetId);
  PRINT_MESSAGE("Model ID          : %X_%X\n", MailPtr->ApiFpHandsetPresentInd.ModelId.MANIC, MailPtr->ApiFpHandsetPresentInd.ModelId.MODIC );
  PRINT_MESSAGE("Codec list length : %X\n", MailPtr->ApiFpHandsetPresentInd.CodecListLength);
  if(MailPtr->ApiFpHandsetPresentInd.CodecListLength > 0)
  {
    uint8 i;

    for(i=0;i<MailPtr->ApiFpHandsetPresentInd.CodecListLength;i++)
    {
      PRINT_MESSAGE("%02X ",MailPtr->ApiFpHandsetPresentInd.Data[i]);
    }
    PRINT_MESSAGE("\n");
  }
  SetCallState(MailPtr->ApiFpHandsetPresentInd.HandsetId - 1,F00_NULL);
  PRINT_MESSAGE(" CallState = F00_NULL\n");

}

/****************************************************************************
*  FUNCTION: HandleMultiKeyPadInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_MULTIKEYPAD_IND from handset
****************************************************************************/
void HandleMultiKeyPadInd(CvmApiMailType *MailPtr)
{
#ifdef full_mmi
#ifdef OLD_CC
	ap_callcntrl_msg_u_type msg;
#endif
#endif

	unsigned char i;
	//unsigned char line_id =1;
	//unsigned char key_seq[40];

	uint8 handsetId = MailPtr->ApiFpMultikeypadInd.HandsetId;

	PRINT_MESSAGE("->API_FP_MULTIKEYPAD_IND - HandsetID: %X KeypadLength:%X Data:", MailPtr->ApiFpMultikeypadInd.HandsetId,MailPtr->ApiFpMultikeypadInd.KeypadLength);

	for(i=0;i<MailPtr->ApiFpMultikeypadInd.KeypadLength;i++)
	{
		PRINT_MESSAGE("(%02X)%c", MailPtr->ApiFpMultikeypadInd.Data[i],MailPtr->ApiFpMultikeypadInd.Data[i]);

	}
	PRINT_MESSAGE("\n");

	switch(CallState[MailPtr->ApiFpMultikeypadInd.HandsetId - 1].State)
	{
		case F02_OVERLAP_SENDING:
		if (CallState[handsetId-1].CallClass == ACC_NORMAL)
		{
		//	LineSimDialDigit(CallState[handsetId].LineId, pApi->ApiFpMultikeypadInd.KeypadLength, pApi->ApiFpMultikeypadInd.Data);
			PRINT_MESSAGE("...............Test Point 1..........\n");

		}
		else if (CallState[handsetId-1].CallClass == ACC_INTERNAL &&
			MailPtr->ApiFpMultikeypadInd.KeypadLength == 1)
		{
			uint8 destId = MailPtr->ApiFpMultikeypadInd.Data[0] - '0';

			DisplayCodecList(CallState[handsetId-1].ProposedCodecListLength, (uint8 *)CallState[handsetId-1].ProposedCodecListPtr);

			if (destId >= 1 && destId <= NO_HS)
			{
				if (destId > 0 && destId <= NO_HS && CallState[destId-1].State == F00_NULL)
				{
					// setup the call to the other handset...
					CallState[handsetId-1].DestinationId = destId;
					CallState[destId-1].CallClass = ACC_INTERNAL;
					CallState[destId-1].DestinationId = handsetId;

					ApiFpSetupReqType * NewMailPtr;

					NewMailPtr = (ApiFpSetupReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpSetupReqType) - 1 + MailPtr->ApiFpSetupInd.CodecListLength);
					if(NewMailPtr != NULL)
					{
						NewMailPtr->Primitive           = H2M_16( API_FP_SETUP_REQ );
						NewMailPtr->HandsetId           = destId;
						NewMailPtr->BasicService		= CallState[handsetId-1].BasicService;
						NewMailPtr->CallClass           = ACC_INTERNAL;
						NewMailPtr->SourceId            = handsetId;
						NewMailPtr->Signal              = ASV_ALERTING_ON_PATTERN0_INT;
						NewMailPtr->ClipLength          = 0;
						NewMailPtr->CallingNumberLength = 0;
						NewMailPtr->CallingNameLength   = 0;
						NewMailPtr->IwuLength           = 0;
						NewMailPtr->ProprietaryLength   = 0;
						NewMailPtr->CodecListLength		= CallState[handsetId-1].ProposedCodecListLength;

						memcpy((void*)&NewMailPtr->Data[0], (void*)CallState[handsetId-1].ProposedCodecListPtr, CallState[handsetId-1].ProposedCodecListLength);

						PRINT_MESSAGE("<-API_FP_SETUP_REQ - HandsetID: %X DestinationId: %X\n", handsetId,destId);
						DisplayCodecList(NewMailPtr->CodecListLength, NewMailPtr->Data);
						BUSM_DeliverMail((uint8*)NewMailPtr);
					}
				}
				else
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_RELEASE_REQ, handsetId, CallState[destId-1].State == HS_OUT_OF_RANGE ? ARR_USER_OUT_RANGE : ARR_USER_BUSY);
					PRINT_MESSAGE("<-API_FP_RELEASE_REQ - HandsetID: %X\n", handsetId);
				}
			}
		}
		break;

	case F01_CALL_INITIATED:
	case F06_CALL_PRESENT:
	case F10_ACTIVE:
		if(MailPtr->ApiFpMultikeypadInd.KeypadLength > 0)
		{
			PRINT_MESSAGE("->API_FP_MULTIKEYPAD_IND - HandsetID: %X\n", MailPtr->ApiFpMultikeypadInd.HandsetId);
			uint8 i;
#if 1
			for(i=0;i<MailPtr->ApiFpMultikeypadInd.KeypadLength;i++)
			{
				PRINT_MESSAGE("(%02X)%c", MailPtr->ApiFpMultikeypadInd.Data[i],MailPtr->ApiFpMultikeypadInd.Data[i]);
				key_seq[i]=MailPtr->ApiFpMultikeypadInd.Data[i];
			}
			PRINT_MESSAGE("\n");	
			key_seq[i]=0;//terminate string
			CallState[handsetId-1].CallClass = ACC_NORMAL;
			if (CallState[handsetId - 1].BasicService == ABS_WIDEBAND_SPEECH)
				sc1445x_dect_terminal_get_key_seq(CCFSM_ATTACHED_DECT, key_seq, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
						(unsigned short)HS_TO_LINE(handsetId), CCFSM_WIDEBAND_CODEC);
			else sc1445x_dect_terminal_get_key_seq(CCFSM_ATTACHED_DECT, key_seq, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
					(unsigned short)HS_TO_LINE(handsetId), CCFSM_NARROWBAND_CODEC);			
#else			
			for(i=0;i<MailPtr->ApiFpMultikeypadInd.KeypadLength;i++)
			{
				PRINT_MESSAGE("(%02X)%c", MailPtr->ApiFpMultikeypadInd.Data[i],MailPtr->ApiFpMultikeypadInd.Data[i]);
				if(MailPtr->ApiFpMultikeypadInd.Data[i] == '*')
					key_seq[key_seq_index+i]='.';
				else
					key_seq[key_seq_index+i]=MailPtr->ApiFpMultikeypadInd.Data[i];
			}
			PRINT_MESSAGE("\n");

			if (key_seq[key_seq_index+i-1] != '#')
			{

				key_seq_index+=MailPtr->ApiFpMultikeypadInd.KeypadLength;
				if (key_seq_index > 40)
					key_seq_index=0;
			}
			else
			{
				key_seq[key_seq_index+i-1]=0;

				for (i=0;i<key_seq_index;i++)
					PRINT_MESSAGE("%c", key_seq[i]);

				PRINT_MESSAGE("\n");

				key_seq_index = 0;

				CallState[handsetId-1].CallClass = ACC_NORMAL;
#ifdef full_mmi
				/*PLACE CALL*/
				//msg.ap_callcntrl_outgoing_call_req.msg_id =  AP_CALLCNTRL_KEY_PRESS_IND;
				//ap_send_msg_to_callcntrl( &msg );
				//if (line_connect == 0) {
				if(strncmp(key_seq, STAR_CODE_HOLD, sizeof(STAR_CODE_HOLD)) == 0){
#ifdef OLD_CC
					PRINT_MESSAGE("AP_CALLCNTRL_CALL_HOLD_REQ\n");
					msg.ap_callcntrl_call_hold_req.msg_id =  AP_CALLCNTRL_CALL_HOLD_REQ;
					msg.ap_callcntrl_call_hold_req.line  = HS_TO_LINE(handsetId);
					ap_send_msg_to_callcntrl( &msg );
#else
          sc1445x_dect_terminal_hold(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]));
#endif

				}
	else{

					PRINT_MESSAGE("AP_CALLCNTRL_OUTGOING_CALL_REQ\n");
#ifdef OLD_CC
					msg.ap_callcntrl_outgoing_call_req.msg_id =  AP_CALLCNTRL_OUTGOING_CALL_REQ;
					strcpy( msg.ap_callcntrl_outgoing_call_req.dialled_num, key_seq);
					msg.ap_callcntrl_outgoing_call_req.call_type = AP_CALL_DIRECT_IP;//ata_line[line_id]->call_type;
#ifdef DECT_DESIGN
					if (CallState[handsetId - 1].BasicService == ABS_WIDEBAND_SPEECH)
						msg.ap_callcntrl_outgoing_call_req.codec  = DECT_WIDEBAND_CODEC;
					else
						msg.ap_callcntrl_outgoing_call_req.codec  = DECT_NARROWBAND_CODEC;
#endif
					msg.ap_callcntrl_outgoing_call_req.line  = HS_TO_LINE(handsetId);
					ap_send_msg_to_callcntrl( &msg );
#else
          if (CallState[handsetId - 1].BasicService == ABS_WIDEBAND_SPEECH)
            sc1445x_dect_terminal_outgoing_req(CCFSM_ATTACHED_DECT, key_seq, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
             (unsigned short)HS_TO_LINE(handsetId), CCFSM_WIDEBAND_CODEC);
          else sc1445x_dect_terminal_outgoing_req(CCFSM_ATTACHED_DECT, key_seq, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
             (unsigned short)HS_TO_LINE(handsetId), CCFSM_NARROWBAND_CODEC);
#endif
			
				}
			//line_connect++;

			//}
#if 0
			else
			{
				PRINT_MESSAGE("............. key pressed while in call..........\n");
				{
     			ApiFpSetPcmCodecReqType* m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));
				m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
				m2->PcmChannel =  HandsetPcmChannel[handsetId-1];
				m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
				BUSM_DeliverMail((uint8*)m2);
				PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_NONE);
				 }

				{

				ApiCodecListType CodecList;
				ApiFpModifyCodecReqType*
					m = (ApiFpModifyCodecReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpModifyCodecReqType) - 1 + sizeof(CodecList));

				m->Primitive = H2M_16( API_FP_MODIFY_CODEC_REQ );
				m->HandsetId = handsetId;
				m->CodecListLength = sizeof(CodecList);
				CodecList.NoOfCodecs           = 1;
				CodecList.NegotiationIndicator = ANI_POSSIBLE;//ANI_NOT_POSSIBLE;
				if(line_connect & 1)
				{
					memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));

				}
				else
				{
					memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));

				}

 				memcpy((void*)&m->Data[0], (void*)&CodecList, sizeof(CodecList));

				PRINT_MESSAGE("<-API_FP_MODIFY_CODEC_REQ - HandsetID: %X\n", MailPtr->ApiFpMultikeypadInd.HandsetId);
				// Display codec list
				DisplayCodecList(m->CodecListLength, m->Data);

				BUSM_DeliverMail((uint8*)m);
				}
				//line_connect++;
			}
#endif
#endif
			}
#endif	//#if 1				
		}
		break;
	default:
		break;
   }
}

/****************************************************************************
*  FUNCTION: HandleGetRegistrationCoundCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_GET_REGISTRATION_COUNT_CFM
*               Set correct callstate of handsets
****************************************************************************/
void HandleGetRegistrationCoundCfm(CvmApiMailType *MailPtr)
{
	if(MailPtr->ApiFpGetRegistrationCountCfm.Status == AS_SUCCESS)
	{
		uint8 i;
		PRINT_MESSAGE("->API_FP_GET_REGISTRATION_COUNT_CFM Num=%X\n",MailPtr->ApiFpGetRegistrationCountCfm.HandsetIdLength);

		RegisteredHS = MailPtr->ApiFpGetRegistrationCountCfm.HandsetIdLength;

		for(i=0;i<MailPtr->ApiFpGetRegistrationCountCfm.HandsetIdLength;i++)
		{
			if(MailPtr->ApiFpGetRegistrationCountCfm.HandsetId[i] < NO_HS)
			{
				BUSM_SendMailP1(0, USER_TASK, API_FP_GET_HANDSET_IPUI_REQ,MailPtr->ApiFpGetRegistrationCountCfm.HandsetId[i]);
				PRINT_MESSAGE("<-API_FP_GET_HANDSET_IPUI_REQ HandsetId = %X \n", MailPtr->ApiFpGetRegistrationCountCfm.HandsetId[i]);
				if (CallState[MailPtr->ApiFpGetRegistrationCountCfm.HandsetId[i]].State == HS_NOT_REGISTERED)
				{
					MailPtr->ApiFpGetRegistrationCountCfm.HandsetId[i] = HS_OUT_OF_RANGE;
					PRINT_MESSAGE("Handset %X is registered\n", i+1);
#if 0
#ifdef LED_GPIO_SUPPORT          
					sleep(1);
					LED_REG_ON;
					sleep(1);
					LED_REG_OFF;
#endif					
#endif
					SetCallState(i, HS_OUT_OF_RANGE);
				}
			}
		}
	}
}

/****************************************************************************
*  FUNCTION: HandleCalledNumberInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_CALLED_NUMBER_IND from handset
****************************************************************************/
void HandleCalledNumberInd(CvmApiMailType *MailPtr)
{
	PRINT_MESSAGE("->API_FP_CALLED_NUMBER_IND\n");
	if(MailPtr->ApiFpCalledNumberInd.CalledNumberLength > 0)
	{
		uint8 i;

		for(i=0;i<MailPtr->ApiFpCalledNumberInd.CalledNumberLength;i++)
		{
			PRINT_MESSAGE("%X ",MailPtr->ApiFpCalledNumberInd.Data[i]);
		}
		PRINT_MESSAGE("\n");
	}
}

/****************************************************************************
*  FUNCTION: HandleRegistrationCompleteInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_REGISTRATION_COMPLETE_IND
****************************************************************************/
void HandleRegistrationCompleteInd(CvmApiMailType *MailPtr)
{
	if(MailPtr->ApiFpRegistrationCompleteInd.Status == AS_SUCCESS)
	{
		PRINT_MESSAGE("->API_FP_REGISTRATION_COMPLETE_IND\n");
		PRINT_MESSAGE("Handset ID        : %X\n", MailPtr->ApiFpRegistrationCompleteInd.HandsetId);
		PRINT_MESSAGE("Model ID          : %X_%X\n", MailPtr->ApiFpRegistrationCompleteInd.ModelId.MANIC, MailPtr->ApiFpRegistrationCompleteInd.ModelId.MODIC);
		PRINT_MESSAGE("Codec list length : %X\n", MailPtr->ApiFpRegistrationCompleteInd.CodecListLength);
		if(MailPtr->ApiFpRegistrationCompleteInd.CodecListLength > 0)
		{
			uint8 i;
			PRINT_MESSAGE("\n");
			for(i=0;i<MailPtr->ApiFpRegistrationCompleteInd.CodecListLength;i++)
			{
				PRINT_MESSAGE("%X ",MailPtr->ApiFpRegistrationCompleteInd.Data[i]);
			}
			PRINT_MESSAGE("\n");

		}
#ifdef DECT_HEADSET_DESIGN
	fnScreenDectRegistrationCompletionInd(CCFSM_REGISTRATION_OK);
#endif

		/* disable registration mode */
		SetRegistrationMode(NORMAL_MODE);
		if(MailPtr->ApiFpRegistrationCompleteInd.HandsetId > RegisteredHS)
			RegisteredHS++;
	}
}

/****************************************************************************
*  FUNCTION: HandleSetRegistrationModeCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_SET_REGISTRATION_MODE_CFM
****************************************************************************/
void HandleSetRegistrationModeCfm(CvmApiMailType *MailPtr)
{
  if(MailPtr->ApiFpSetRegistrationModeCfm.Status == AS_SUCCESS)
  {
    PRINT_MESSAGE("->API_FP_SET_REGISTRATION_MODE_CFM\n");
#if 0
	if (registration_mode)
	{
		registration_mode = 0;
	}
	else
	{
		registration_in_progress_tmouts = 0;
		registration_mode = 1;
	}
#endif

  }
}

/****************************************************************************
*  FUNCTION: HandleInitPcmCfg
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_INIT_PCM_CFG
****************************************************************************/
void HandleInitPcmCfg(CvmApiMailType *MailPtr)
{
  if(MailPtr->ApiFpInitPcmCfm.Status == AS_SUCCESS)
  {
    PRINT_MESSAGE("->API_FP_INIT_PCM_CFM\n");
  }
}

/****************************************************************************
*  FUNCTION: HandleSetLedCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_SET_LED_CFM
****************************************************************************/
void HandleSetLedCfm(CvmApiMailType *MailPtr)
{
  if(MailPtr->ApiFpSetLedCfm.Status == AS_SUCCESS)
  {
    PRINT_MESSAGE("->API_FP_SET_LED_CFM\n");
  }
}

/****************************************************************************
*  FUNCTION: HandleSetEepromCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_SET_EEPROM_CFM
****************************************************************************/
void HandleSetEepromCfm(CvmApiMailType *MailPtr)
{
  PRINT_MESSAGE("->API_FP_SET_EEPROM_CFM\n");
  if(MailPtr->ApiFpSetEepromCfm.Status == AS_SUCCESS)
  {
#ifdef SET_RFPI	  
    if(MailPtr->ApiFpSetEepromCfm.Address == H2M_16( RFPI_EEPROM_ADDR ))
    {
      if(MailPtr->ApiFpSetEepromCfm.DataLength == 5)
      {
    	if(check_rfpi_emc_fpn_change){
			PRINT_INFO("Rfpi changed. Reseting ..\n");
#ifdef DECT_NATALIE
			dect_nvs_init();
#else			
			BUSM_SendMailP0(0, USER_TASK, API_FP_RESET_REQ);
#endif			
    	}
      }
    }
#endif    
  }
}

/****************************************************************************
*  FUNCTION: HandleGetEepromCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_GET_EEPROM_CFM
****************************************************************************/
void HandleGetEepromCfm(CvmApiMailType *MailPtr)
{
  if(MailPtr->ApiFpGetEepromCfm.Status == AS_SUCCESS)
  {
    uint8 i;
    PRINT_MESSAGE("->API_FP_GET_EEPROM_CFM: (%X) ", MailPtr->ApiFpGetEepromCfm.DataLength);
    for(i=0;i<MailPtr->ApiFpGetEepromCfm.DataLength; i++)
    {
      PRINT_MESSAGE("%X ", MailPtr->ApiFpGetEepromCfm.Data[i]);
    }
    PRINT_MESSAGE("\n");
  }
}

/****************************************************************************
*  FUNCTION: HandleGetIdCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_GET_ID_CFM
****************************************************************************/
void HandleGetIdCfm(CvmApiMailType *MailPtr)
{
  if (MailPtr->ApiFpGetIdCfm.Status == AS_SUCCESS)
  {
    PRINT_MESSAGE("->API_FP_GET_ID_CFM\n");
    PRINT_MESSAGE("CVM ID:\t%02X %02X %02X %02X %02X\n",
         MailPtr->ApiFpGetIdCfm.Id[0],
         MailPtr->ApiFpGetIdCfm.Id[1],
         MailPtr->ApiFpGetIdCfm.Id[2],
         MailPtr->ApiFpGetIdCfm.Id[3],
         MailPtr->ApiFpGetIdCfm.Id[4]);
    PRINT_MESSAGE("Communication OK\n");    
#ifdef SET_RFPI
    memcpy(RFPI, MailPtr->ApiFpGetIdCfm.Id, 5);//store the current rfpi  
#if 0    
    SET_CC_RFPI_EMC(CC_RFPI, 0x0eab);
    SET_CC_RFPI_FPN(CC_RFPI, 0x008b8>>3);
#endif    
#if 0    
	unsigned short emc_rfpi=0;
	unsigned short fpn_rfpi=0;
	unsigned char MACAddress[4];
	MACAddress[0]=0x12;MACAddress[1]=0x34;MACAddress[2]=0x56;MACAddress[3]=0x79;
	memcpy (&emc_rfpi, (unsigned short *) &MACAddress[0], 2);
	memcpy (&fpn_rfpi, (unsigned short *) &MACAddress[2], 2);

	printf("\n\n emc_rfpi %x, fpn_rfpi %x \n\n\n", emc_rfpi, fpn_rfpi);

  SET_CC_RFPI_EMC(CC_RFPI, emc_rfpi);
  SET_CC_RFPI_FPN(CC_RFPI, fpn_rfpi<<1);
#endif  
    if(check_rfpi_emc_fpn_change)
    {
    	SetRegistrationMode(DELETE_ALL_MODE);
    }
#endif
  }
}

/****************************************************************************
*  FUNCTION: HandleSetupCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_SETUP_CFM
****************************************************************************/
void HandleSetupCfm(CvmApiMailType *MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpSetupCfm.HandsetId;
  switch(CallState[handsetId - 1].State)
  {
	case F06_CALL_PRESENT:
    case HS_OUT_OF_RANGE:
    case F00_NULL:
	  if (MailPtr->ApiFpSetupCfm.Status == AS_SUCCESS)
      {
		  PRINT_MESSAGE("->API_FP_SETUP_CFM - HandsetID: %X, BasicService: %X\n", MailPtr->ApiFpSetupCfm.HandsetId, MailPtr->ApiFpSetupCfm.BasicService);
        SetCallState(handsetId-1, F06_CALL_PRESENT);

      }
      else
      {
        PRINT_MESSAGE(" !!!Call setup failed!!!");
      }
      break;

    case F01_CALL_INITIATED:
    case F02_OVERLAP_SENDING:
    case F03_CALL_PROCEEDING:
    case F04_CALL_DELIVERED:
//    case F06_CALL_PRESENT:
    case F07_CALL_RECEIVED:
    case F10_ACTIVE:
    case F19_RELEASE_PENDING:
      PRINT_MESSAGE("!!! Unexpected API_FP_SETUP_CFM!!!\n");
      break;
    default:
      break;
  }
}

/****************************************************************************
*  FUNCTION: HandleAlertInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_ALERT_IND
****************************************************************************/
void HandleAlertInd(CvmApiMailType *MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpAlertInd.HandsetId;
	PRINT_MESSAGE("->API_FP_ALERT_IND - HandsetID: %X, CodecListLength: %X\n", handsetId, MailPtr->ApiFpAlertInd.CodecListLength);
	DisplayCodecList(MailPtr->ApiFpAlertInd.CodecListLength, MailPtr->ApiFpAlertInd.Data);
	switch (CallState[handsetId-1].State)
	{
		case F06_CALL_PRESENT:
		SetCallState(handsetId-1, F07_CALL_RECEIVED);
		PRINT_MESSAGE("CallState = F07_CALL_RECEIVED\n");

		if (CallState[handsetId-1].CallClass == ACC_INTERNAL)
		{
			// Check the state here?

			ApiFpAlertReqType*
			m = (ApiFpAlertReqType*)BUSM_AllocateMail(0,USER_TASK,sizeof(ApiFpAlertReqType) - 1 + MailPtr->ApiFpAlertInd.CodecListLength);
			m->Primitive = H2M_16( API_FP_ALERT_REQ );
			m->HandsetId = CallState[handsetId-1].DestinationId;
			m->ProgressInd = AP_IN_BAND_NOT_AVAILABLE;
			m->Signal = ASV_DIAL_TONE_ON;
			m->CodecListLength = MailPtr->ApiFpAlertInd.CodecListLength;

			memcpy((void*)&m->Data[0], MailPtr->ApiFpAlertInd.Data, MailPtr->ApiFpAlertInd.CodecListLength);

			BUSM_DeliverMail((uint8*)m);
			PRINT_MESSAGE("<-API_FP_ALERT_REQ - HandsetID: %X DestinationID: %X\n", handsetId,CallState[handsetId-1].DestinationId);
		}
		else
		{
			CallState[handsetId-1].ProposedCodecListLength = MailPtr->ApiFpAlertInd.CodecListLength;
			CallState[handsetId-1].ProposedCodecListPtr = (ApiCodecListType*)malloc(MailPtr->ApiFpAlertInd.CodecListLength);
			memcpy(CallState[handsetId-1].ProposedCodecListPtr, MailPtr->ApiFpAlertInd.Data, MailPtr->ApiFpAlertInd.CodecListLength);
		//	LineSimAlert(CallState[handsetId].LineId, 0, NULL);
		}
		break;
	case F01_CALL_INITIATED:
	case F02_OVERLAP_SENDING:
	case F03_CALL_PROCEEDING:
	case F00_NULL:
	case F04_CALL_DELIVERED:
	case F07_CALL_RECEIVED:
	case F10_ACTIVE:
	case F19_RELEASE_PENDING:
		PRINT_MESSAGE("!!! UNEXPECTED MESSAGE !!!\n");
		break;
	default:
		break;
  }

}
void HandleAlertCfm(CvmApiMailType *MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpAlertInd.HandsetId;
	PRINT_MESSAGE("->API_FP_ALERT_CFM - HandsetID: %X \n", handsetId);
	switch (CallState[handsetId-1].State)
	{
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
			SetCallState(handsetId-1, F04_CALL_DELIVERED);
			break;
		case F04_CALL_DELIVERED:
			break;
		case F00_NULL:
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
		case F10_ACTIVE:
		case F19_RELEASE_PENDING:
		PRINT_MESSAGE("!!! UNEXPECTED MESSAGE !!!\n");
			break;
		default:
			break;
	}
}



/****************************************************************************
*  FUNCTION: HandleConnectInd
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_CONNECT_IND
*               Send API_FP_SET_PCM_CODEC_REQ to setup PCM CODEC
*               Send API_FP_CONNECT_RES to handset
*               Set correct callstate for connecting handset
*               Release all other handsets
*
*	CONNECTION IN INCOMING CALL
****************************************************************************/
void HandleConnectInd(CvmApiMailType *MailPtr)
{
	uint8 i, codec_supported = 0;
	uint8 handsetId = MailPtr->ApiFpConnectInd.HandsetId;
	ApiCodecListType *CodecList;
#ifdef full_mmi
#ifdef OLD_CC
	ap_callcntrl_msg_u_type msg;
#endif
#endif
	
	PRINT_MESSAGE("->API_FP_CONNECT_IND - HandsetID: %X\n", handsetId);

	switch(CallState[handsetId - 1].State)
	{
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
			SetCallState(handsetId - 1, F10_ACTIVE);
			PRINT_MESSAGE(" CallState = F10_ACTIVE\n");
			PRINT_MESSAGE(" CallState.CallClass = %X\n",CallState[handsetId-1].CallClass);

			if (CallState[handsetId-1].CallClass == ACC_NORMAL)
			{
#ifdef DECT_NATALIE
	#ifdef FIX_NATALIE_BUG
				if (MailPtr->ApiFpConnectInd.AdpcmChannel == 0xff){
					PRINT_MESSAGE("<-API_FP_CONNECT_RES - BUG FIX !!!! HandsetID: %X\n", handsetId);
					BUSM_SendMailP2(0, USER_TASK, API_FP_CONNECT_RES, handsetId, AS_REJECTED);
					return;
				}
	#endif //FIX_NATALIE_BUG
				HandsetPcmChannel[handsetId - 1] = MailPtr->ApiFpConnectInd.AdpcmChannel;
				PRINT_MESSAGE("Connect HandsetID: %d to AdpcmChannel: %d \n", handsetId, HandsetPcmChannel[handsetId - 1]);
#ifdef DYNAMIC_CHANNEL_ALLOCATION
				bind_pcm_channel(HandsetPcmChannel[handsetId - 1]);
				display_pcm_channel_status();
#endif	
#endif
			
				//???????????????

				// if (line_codec)
				// {
					// BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1],/* AP_DATA_FORMAT_CWB_ALAW*/AP_DATA_FORMAT_LINEAR_16kHz);
						// PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_LINEAR_16kHz);
				// }

				// if(line_codec == DECT_WIDEBAND_CODEC)
				// {
					// BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1],WIDEBAND_PCM_CODEC);
					// PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
				// }
				//Check if a PCM channel is allocated to the handset
				if(HandsetPcmChannel[handsetId - 1] != AP_CHANNEL_INVALID){
#if 1
					//If FP returns the selected codec, use this code selection				
					if(MailPtr->ApiFpConnectInd.CodecListLength > 0)
					{
						PRINT_MESSAGE(" Codec list returned in API_FP_CONNECT_IND\n");
						CodecList = (ApiCodecListType*)&MailPtr->ApiFpConnectInd.Data[0];
						DisplayCodecList(MailPtr->ApiFpConnectInd.CodecListLength, MailPtr->ApiFpConnectInd.Data);
					}
					//If FP does NOT return the selected codec, use the ProposedCodecList which was updated in ALERT_IND
					else if (CallState[handsetId-1].ProposedCodecListPtr)
					{
						PRINT_MESSAGE(" No Codec list returned in API_FP_CONNECT_IND. Codeclist of API_ALERT_IND will be used\n");
						CodecList = CallState[handsetId-1].ProposedCodecListPtr;
						// pkshih: Cast type to avoid compiler warning, but I think pass this argument is wrong. 
						DisplayCodecList(CallState[handsetId-1].ProposedCodecListLength, (uint8 *)CallState[handsetId-1].ProposedCodecListPtr);
					}
					else						
					{						
						PRINT_MESSAGE(" No Codec list returned in API_FP_CONNECT_IND or API_ALERT_IND. G726 will be used\n");
						CodecList = 0;
						
					}

					if(CodecList && (CodecList->Codec->Codec == ACT_G722))
					{
						if(CallState[handsetId-1].RequestedCodec == DECT_WIDEBAND_CODEC){
							codec_supported = 1;
						}
						BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1],WIDEBAND_PCM_CODEC);
						PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
						CallState[handsetId - 1].BasicService = ABS_WIDEBAND_SPEECH;

					}
					else
					{
						if(CallState[handsetId-1].RequestedCodec == DECT_NARROWBAND_CODEC){
							codec_supported = 1;
						}
						BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
						PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
						CallState[handsetId - 1].BasicService = ABS_BASIC_SPEECH;
					}

#ifdef full_mmi
#ifdef OLD_CC
#ifdef ATA_DESIGN
					ap_callcntrl_msg_u_type msg;
					/*HOOK - OFF */
					msg.ap_callcntrl_dect_codec_ind.msg_id = AP_CALLCNTRL_DECT_CODEC_IND;
					msg.ap_callcntrl_dect_codec_ind.line = HS_TO_LINE(handsetId);
					msg.ap_callcntrl_dect_codec_ind.pcm_port = ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]);	
					if(codec_supported){
						PRINT_MESSAGE (" ============== respond codec supported ========== \n");					
						msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_SUPPORTED;
					}
					else{
						PRINT_MESSAGE (" ============== respond codec NOT supported ========== \n");					
						msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_NOT_SUPPORTED;
					}
					ap_send_msg_to_callcntrl( &msg );
#endif
#endif
#endif					


#endif
					CallState[handsetId-1].Connected2Line = TRUE;
				}
				else{				
					CallState[handsetId-1].Connected2Line = FALSE;
					PRINT_INFO("Connection rejected. No PCM channel available. PcmChannelAllocation: %d \n ", PcmChannelAllocation);
//					PRINT_MESSAGE("<-API_FP_CONNECT_RES - HandsetID: %X\n", handsetId);
//					BUSM_SendMailP2(0, USER_TASK, API_FP_CONNECT_RES, handsetId, AS_REJECTED);
				}

//				BUSM_SendMailP2(0, USER_TASK, API_FP_CONNECT_RES, handsetId, AS_SUCCESS);

				for(i=0;i<NO_HS;i++)
				{
					if( (i != (handsetId - 1)) && (CallState[i].LineId == BROADCAST_HS))
					{
						//if(CallState[i].State == F07_CALL_RECEIVED)
						if (CallState[i].CallClass == ACC_NORMAL && CallState[i].State != F10_ACTIVE)
						{
#ifdef DYNAMIC_CHANNEL_ALLOCATION							
							/*Set pcm channel to invalid so that it is not freed on release indication (needed for broadcast call)*/
							HandsetPcmChannel[i]=AP_CHANNEL_INVALID;
#endif							
							BUSM_SendMailP2(0, USER_TASK, API_FP_RELEASE_REQ, (i+1), ARR_NORMAL);
							PRINT_MESSAGE("<-API_FP_RELEASE_REQ - HandsetID: %X\n", (i+1));
						}
					}
				}
				
				/*Update CC state*/
				if (CallState[handsetId-1].Connected2Line == TRUE){
					//	SendApiFpConnectRes(CVMAPI_TASK, handsetId, AS_SUCCESS);
					/*Bind pcm channel, for the case that it was a broadcast call, and on of the ringing handsets has cancelled the call*/
#ifdef DYNAMIC_CHANNEL_ALLOCATION						
					bind_pcm_channel(HandsetPcmChannel[handsetId - 1]);
#endif					
					PRINT_MESSAGE("<-API_FP_CONNECT_RES - HandsetID: %X\n", handsetId);
					BUSM_SendMailP2(0, USER_TASK, API_FP_CONNECT_RES, handsetId, AS_SUCCESS);				
#ifdef full_mmi
#ifdef OLD_CC
					/*HOOK - OFF */
					PRINT_MESSAGE (" ============== off hook to sipua ========== \n");
					msg.ap_callcntrl_hook_off_ind.msg_id = AP_CALLCNTRL_HOOK_OFF_IND;				
					msg.ap_callcntrl_hook_off_ind.line = HS_TO_LINE(handsetId);
					msg.ap_callcntrl_hook_off_ind.pcm_port = ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]);	
					if(CallState[handsetId - 1].BasicService == ABS_BASIC_SPEECH)
						msg.ap_callcntrl_hook_off_ind.codec=DECT_NARROWBAND_CODEC ;				     
					else
						msg.ap_callcntrl_hook_off_ind.codec=DECT_WIDEBAND_CODEC;
					//msg.ap_callcntrl_hook_off_ind.line = HandsetPcmChannel[handsetId - 1];//2;//0;//1;//line

					ap_send_msg_to_callcntrl( &msg );
#else
					int codec_sent=0;
					if(CallState[handsetId - 1].BasicService == ABS_BASIC_SPEECH)
						codec_sent=CCFSM_NARROWBAND_CODEC ;				     
					else
						codec_sent=CCFSM_WIDEBAND_CODEC;

//					sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
//							(int)HS_TO_LINE(handsetId), CCFSM_HOOK_OFF, codec_sent);
					sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
												(int)HS_TO_LINE(handsetId), CCFSM_HOOK_OFF, codec_sent, CallState[handsetId - 1].LineId);
#endif
#endif		
				}
				else{
					BUSM_SendMailP2(0, USER_TASK, API_FP_RELEASE_REQ, handsetId, ARR_NORMAL);
					PRINT_MESSAGE("<-API_FP_RELEASE_REQ - HandsetID: %X\n", handsetId);					
				}			
#ifdef DYNAMIC_CHANNEL_ALLOCATION				
				display_pcm_channel_status();
#endif					
			}
			else if (CallState[handsetId-1].CallClass == ACC_INTERNAL)
			{
				// Check the state here?
			//	SendApiFpConnectReq(CVMAPI_TASK, CallState[handsetId].DestinationId, pApi->ApiFpConnectInd.CodecListLength, pApi->ApiFpConnectInd.Data);

				ApiFpConnectReqType *NewMailPtr;

				NewMailPtr = (ApiFpConnectReqType *) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpConnectReqType ) - 1 + MailPtr->ApiFpConnectInd.CodecListLength);
				if(NewMailPtr != NULL)
				{
					NewMailPtr->Primitive       = H2M_16( API_FP_CONNECT_REQ );
					NewMailPtr->HandsetId       = CallState[handsetId-1].DestinationId;
					NewMailPtr->CodecListLength = MailPtr->ApiFpConnectInd.CodecListLength;

					memcpy((void*)&NewMailPtr->Data[0], (void*)&MailPtr->ApiFpConnectInd.Data[0], MailPtr->ApiFpConnectInd.CodecListLength);

					PRINT_MESSAGE("<-API_FP_CONNECT_REQ - HandsetID: %X\n", CallState[handsetId-1].DestinationId);
					// Display codec list
					DisplayCodecList(NewMailPtr->CodecListLength, NewMailPtr->Data);

					BUSM_DeliverMail((uint8*)NewMailPtr);
				}
				//	SendApiFpConnectRes(CVMAPI_TASK, handsetId, AS_SUCCESS);
					PRINT_MESSAGE("<-API_FP_CONNECT_RES - HandsetID: %X\n", handsetId);
					BUSM_SendMailP2(0, USER_TASK, API_FP_CONNECT_RES, handsetId, AS_SUCCESS);
			}
			break;
		case F00_NULL:
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F10_ACTIVE:
		case F19_RELEASE_PENDING:
			PRINT_MESSAGE("<-API_FP_CONNECT_RES - HandsetID: %X\n", handsetId);
			BUSM_SendMailP2(0, USER_TASK, API_FP_CONNECT_RES, handsetId, AS_REJECTED);
			PRINT_MESSAGE("!!!!! API_FP_CONNECT_IND !!!! - CallState: %X\n",CallState[handsetId-1].State);

		//	SendApiFpConnectRes(CVMAPI_TASK, handsetId, AS_REJECTED);
		//	UnexpectedMessage("API_FP_CONNECT_IND", CallState[handsetId].State);
			break;
		default:
			break;
	}
}

/****************************************************************************
*  FUNCTION: HandleReleaseCfm
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle API_FP_RELEASE_CFM
*               Set callstate correct
****************************************************************************/
void HandleReleaseCfm(CvmApiMailType *MailPtr)
{
	PRINT_MESSAGE("->API_FP_RELEASE_CFM - HandsetID: %X\n", MailPtr->ApiFpReleaseCfm.HandsetId);
	switch(CallState[MailPtr->ApiFpReleaseCfm.HandsetId - 1].State)
	{
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
		case F10_ACTIVE:
		case F19_RELEASE_PENDING:
			SetCallState(MailPtr->ApiFpReleaseCfm.HandsetId - 1,F00_NULL);
			PRINT_MESSAGE(" CallState = F00_NULL\n");
			break;
		case F00_NULL:
		//	PRINT_MESSAGE("!!! UNEXPECTED MESSAGE !!!\n");
			break;
		default:
			break;
	}
}

void HandleRejectInd(CvmApiMailType *MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpRejectInd.HandsetId;
	PRINT_MESSAGE("->API_FP_REJECT_IND - HandsetID: %X\n", handsetId);

#ifdef full_mmi
// sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
//        (int)HS_TO_LINE(handsetId), CCFSM_HOOK_ON, 0);
	 sc1445x_dect_terminal_audio_peripheral_change(CCFSM_ATTACHED_DECT, (int)ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]), 
	        (int)HS_TO_LINE(handsetId), CCFSM_HOOK_ON, 0, CallState[handsetId - 1].LineId); 
#endif
 
	switch (CallState[handsetId-1].State)
	{
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
			if (CallState[handsetId-1].CallClass == ACC_INTERNAL)
			{

				ApiFpReleaseReqType*
					m1 = (ApiFpReleaseReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpReleaseReqType)));

				m1->Primitive = H2M_16( API_FP_RELEASE_REQ );
				m1->HandsetId = CallState[handsetId-1].DestinationId;
				m1->Reason = ARR_USER_REJECTION;
				BUSM_DeliverMail((uint8*)m1);
				PRINT_MESSAGE("<-API_FP_RELEASE_REQ - HandsetID: %X DestinationId: %X\n", handsetId,CallState[handsetId-1].DestinationId);
			}
			else if (CallState[handsetId-1].CallClass == ACC_NORMAL)
			{
			//	LineSimReject(CallState[handsetId].LineId);
			}
			SetCallState(handsetId-1, F00_NULL);
			PRINT_MESSAGE(" CallState = F00_NULL\n");
			break;
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F10_ACTIVE:
		case F19_RELEASE_PENDING:
			SetCallState(handsetId-1, F00_NULL);
			PRINT_MESSAGE(" CallState = F00_NULL\n");
			break;
		case F00_NULL:
			PRINT_MESSAGE("!!! UNEXPECTED MESSAGE !!!\n");
			break;
        default:
          break;
	}
}


void HandleSetAccessCodeCfm(CvmApiMailType *MailPtr)
{
	PRINT_MESSAGE("->API_FP_SET_ACCESS_CODE_CFM\n");
  if (MailPtr->ApiFpSetAccessCodeCfm.Status != AS_SUCCESS)
  {
    PRINT_MESSAGE("!!!API_FP_SET_ACCESS_CODE_REQ failed!!!\n");
  }
}

void HandleGetAccessCodeCfm(CvmApiMailType *MailPtr)
{
	PRINT_MESSAGE("->API_FP_GET_ACCESS_CODE_CFM\n");
  if (MailPtr->ApiFpGetAccessCodeCfm.Status == AS_SUCCESS)
  {
	PRINT_MESSAGE("%02X%02X\n",MailPtr->ApiFpGetAccessCodeCfm.Ac[0],MailPtr->ApiFpGetAccessCodeCfm.Ac[1]);
	ref_AccessCode[ 0 ] = MailPtr->ApiFpGetAccessCodeCfm.Ac[0];
	ref_AccessCode[ 1 ] = MailPtr->ApiFpGetAccessCodeCfm.Ac[1];
  }
  else
  {
    PRINT_MESSAGE("!!!API_FP_GET_ACCESS_CODE_REQ failed!!!\n");
  }
}

void HandleGetHandsetIpuiCfm(CvmApiMailType* MailPtr)
{
	if (MailPtr->ApiFpGetHandsetIpuiCfm.Status == AS_SUCCESS)
	{
		#define M ((ApiFpGetHandsetIpuiCfmType*)MailPtr)
		PRINT_MESSAGE("->API_FP_GET_HANDSET_IPUI_CFM: HandsetId = %02X, IPUI = %02X.%02X.%02X.%02X.%02X\n",M->HandsetId,M->IPUI[0],M->IPUI[1],M->IPUI[2],M->IPUI[3],M->IPUI[4]);
      #undef M
	}
	else
  {
    PRINT_MESSAGE("!!!API_FP_GET_HANDSET_IPUI_CFM failed!!!\n");
  }
}


void HandleSetupAckCfm(CvmApiMailType* MailPtr)
{
	PRINT_MESSAGE("->API_FP_SETUP_ACK_CFM\n");
}
void HandleProprietaryCfm(CvmApiMailType* MailPtr)
{
	PRINT_MESSAGE("->API_FP_PROPRIETARY_CFM, HandsetId %d, Status %d \n", (( ApiFpProprietaryCfmType *)MailPtr)->HandsetId, ((ApiFpProprietaryCfmType*)MailPtr)->Status);	
}
void HandleCallProcCfm(CvmApiMailType* MailPtr)
{
	PRINT_MESSAGE("->API_FP_CALL_PROC_CFM, HandsetId %d, Status %d \n", (( ApiFpCallProcCfmType *)MailPtr)->HandsetId, ((ApiFpCallProcCfmType*)MailPtr)->Status);	
}
void HandleProprietaryInd(CvmApiMailType* MailPtr)
{
	PRINT_MESSAGE("->API_FP_PROPRIETARY_IND\n");
}

void HandleFpModifyCodecCfm(CvmApiMailType* MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpModifyCodecCfm.HandsetId;
	PRINT_MESSAGE("->API_FP_MODIFY_CODEC_CFM\n");

	switch (CallState[handsetId-1].State)
	{
		case F10_ACTIVE:
			if (MailPtr->ApiFpModifyCodecCfm.Status != AS_SUCCESS)
			{
				PRINT_MESSAGE("!!! Failed to change the CODEC.\n");				
			}

#ifdef full_mmi
#ifdef OLD_CC
#ifdef ATA_DESIGN
			ap_callcntrl_msg_u_type msg;
			/*HOOK - OFF */
			msg.ap_callcntrl_dect_codec_ind.msg_id = AP_CALLCNTRL_DECT_CODEC_IND;
			msg.ap_callcntrl_dect_codec_ind.line = HS_TO_LINE(handsetId);
			msg.ap_callcntrl_dect_codec_ind.pcm_port = ADPCM_TO_APP(HandsetPcmChannel[handsetId - 1]);	
			if(MailPtr->ApiFpModifyCodecCfm.Status == AS_SUCCESS){
				PRINT_MESSAGE (" ============== respond codec supported ========== \n");					
				msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_SUPPORTED;
			}
			else{
				PRINT_MESSAGE (" ============== respond codec NOT supported ========== \n");					
				msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_NOT_SUPPORTED;
				
				/*Restore Codec - PCM codec*/
				if(CallState[handsetId - 1].BasicService == ABS_WIDEBAND_SPEECH)
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_WIDEBAND_SPEECH");
				}
				else
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_NARROWBAND_SPEECH");
				}				
			}
			ap_send_msg_to_callcntrl( &msg );
#endif
#endif
#endif						

			break;
		case F00_NULL:
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
		case F19_RELEASE_PENDING:
			PRINT_MESSAGE("UnexpectedMessage \n");
			break;
        default:
          break;
	}
}

void HandleFpModifyCodecInd(CvmApiMailType* MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpModifyCodecInd.HandsetId;
	PRINT_MESSAGE("->API_FP_MODIFY_CODEC_IND\n");
	DisplayCodecList(MailPtr->ApiFpModifyCodecInd.CodecListLength, MailPtr->ApiFpModifyCodecInd.Data);

	switch (CallState[handsetId-1].State)
	{
	case F10_ACTIVE:
		{
     		ApiFpSetPcmCodecReqType* m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));
			m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
			m2->PcmChannel =  HandsetPcmChannel[handsetId-1];
			m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
			BUSM_DeliverMail((uint8*)m2);
			PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_NONE);
		 }
			break;
		case F00_NULL:
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
		case F19_RELEASE_PENDING:
		  PRINT_MESSAGE("!!! UnexpectedMessage \n");
		  break;
		default:
			break;
	}
}

void HandleFpCodecInfoInd(CvmApiMailType* MailPtr)
{
	uint8 handsetId = MailPtr->ApiFpCodecInfoInd.HandsetId;
	PRINT_MESSAGE("->API_FP_CODEC_INFO_IND\n");
	DisplayCodecList(MailPtr->ApiFpCodecInfoInd.CodecListLength, MailPtr->ApiFpCodecInfoInd.Data);

	switch (CallState[handsetId-1].State)
	{
		case F10_ACTIVE:
			if (((ApiCodecListType*)MailPtr->ApiFpCodecInfoInd.Data)->NoOfCodecs == 1)
			{
#if 0
				if (((ApiCodecListType*)MailPtr->ApiFpCodecInfoInd.Data)->Codec[0].Codec == LineSimGetSelectedCodec(CallState[handsetId].LineId))
				{
					// Open the PCM
					ApiPcmDataFormatType pcmDataFormat = LineSimGetPcmDataFormat(CallState[handsetId].LineId);
					if (pcmDataFormat != AP_DATA_FORMAT_INVALID)
					{
						SendApiFpSetPcmCodecReq(CVMAPI_TASK,CallState[handsetId].LineId,pcmDataFormat);
					}


				}
				else
				{
					// request a codec change on the remote device
					if (AS_SUCCESS ==  LineSimModifyCodec(CallState[handsetId].LineId,
						pApi->ApiFpModifyCodecInd.CodecListLength,
						(ApiCodecListType*)pApi->ApiFpModifyCodecInd.Data))
					{
						// Mute the PCM and wait for response from remote part
						SendApiFpSetPcmCodecReq(CVMAPI_TASK,CallState[handsetId].LineId, AP_DATA_FORMAT_NONE);
					}
				}
#endif
				/*Modify Codec - PCM codec*/
				if(CallState[handsetId - 1].RequestedCodec == DECT_WIDEBAND_CODEC)
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], WIDEBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_WIDEBAND_SPEECH");
				}
				else
				{
					BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], NARROWBAND_PCM_CODEC);
					PRINT_MESSAGE("  BasicService: %s\n", "ABS_NARROWBAND_SPEECH");
				}
			}
			else
			{
				PRINT_MESSAGE("!!! UnexpectedMessage \n");
			}
			break;
		case F00_NULL:
		case F01_CALL_INITIATED:
		case F02_OVERLAP_SENDING:
		case F03_CALL_PROCEEDING:
		case F04_CALL_DELIVERED:
		case F06_CALL_PRESENT:
		case F07_CALL_RECEIVED:
		case F19_RELEASE_PENDING:
			PRINT_MESSAGE("!!! UnexpectedMessage \n");
			break;
		default:
			break;
	}
}

void HandleFpIwuToIwuInd(CvmApiMailType* MailPtr)
{
	//uint8 handsetId = MailPtr->ApiFpIwuToIwuInd.HandsetId;
	PRINT_MESSAGE ("->API_FP_IWU_TO_IWU_IND\n");
	if (MailPtr->ApiFpIwuToIwuInd.IwuLength == 0x0a)
	{

#if 0
			ApiFpReleaseReqType*
			m1 = (ApiFpReleaseReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpReleaseReqType)));
			m1->Primitive = H2M_16( API_FP_RELEASE_REQ );
			m1->HandsetId = handsetId;
			m1->Reason = ARR_NORMAL;
			BUSM_DeliverMail((uint8*)m1);
			PRINT_MESSAGE("<-API_FP_RELEASE_REQ - HandsetID: %X\n", handsetId);

#if 1
			ApiFpSetPcmCodecReqType*
			m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));

			m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
			m2->PcmChannel =  handsetId;
			m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
			BUSM_DeliverMail((uint8*)m2);

			PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId-1], AP_DATA_FORMAT_NONE);

#endif

			SetCallState(handsetId-1,F00_NULL);
					    //vm====================
			PRINT_MESSAGE("CallState = F00_NULL\n");
#endif
	}
	else
	{
		switch (MailPtr->ApiFpIwuToIwuInd.Data[9])
		{
		case 1:
			if (MailPtr->ApiFpIwuToIwuInd.Data[10] == 0)
			{
				PRINT_MESSAGE("hook pressed \n");
			}
			if (MailPtr->ApiFpIwuToIwuInd.Data[10] == 1)
			{
				PRINT_MESSAGE("hook released \n");
			}
			break;
		case 2:
			break;
		case 3:
			if (MailPtr->ApiFpIwuToIwuInd.Data[10] == 0)
			{
				PRINT_MESSAGE("volume up pressed \n");
			}
			if (MailPtr->ApiFpIwuToIwuInd.Data[10] == 1)
			{
				PRINT_MESSAGE("volume up released \n");
			}

			break;
		case 4:
			if (MailPtr->ApiFpIwuToIwuInd.Data[10] == 0)
			{
				PRINT_MESSAGE("volume down pressed \n");
			}
			if (MailPtr->ApiFpIwuToIwuInd.Data[10] == 1)
			{
				PRINT_MESSAGE("volume down released \n");
			}

			break;

		}

	}
}


void HandleFpIwuToIwuCfm(CvmApiMailType* MailPtr)
{
	//uint8 handsetId = MailPtr->ApiFpIwuToIwuCfm.HandsetId;
	PRINT_MESSAGE ("->API_FP_IWU_TO_IWU_CFM\n");
}

void HandleFpGetRegisterCfm(CvmApiMailType* MailPtr)
{

	PRINT_MESSAGE ("->API_FP_GET_REGISTER_CFM Addr: %lX Data: %X\n",M2H_32(MailPtr->ApiFpGetRegisterCfm.RegisterAddress),M2H_16(MailPtr->ApiFpGetRegisterCfm.Data));
}



void HandleDeleteRegistrationCfm(CvmApiMailType* MailPtr)
{
	//PRINT_MESSAGE ("->API_FP_DELETE_REGISTRATION_CFM handsetID: %X \n",MailPtr->ApiFpDeleteRegistrationCfm.HandsetId);
}


void HandleDeleteRegistrationInd( CvmApiMailType* MailPtr)
{
//	PRINT_MESSAGE ("->API_FP_REGISTRATION_DELETED_IND handsetID: %X \n",MailPtr->ApiFpRegistrationDeletedInd.HandsetId);
	RegisteredHS--;
	/*If we are in a DELETE registration mode, continue deleting the remaining handsets*/
	PRINT_MESSAGE ("registration_mode: %X \n",registration_mode);
	if( (registration_mode == DELETE_ALL_MODE)){
		if (RegisteredHS) //if not all deleted, restart delete mode
			SetRegistrationMode(DELETE_ALL_MODE);
		else//return to normal mode
			SetRegistrationMode(NORMAL_MODE);
	}
	else if( (registration_mode == REGISTER_MODE)){
		SetRegistrationMode(REGISTER_MODE);
	}

}


#ifdef CVM480
/****************************************************************************
*  FUNCTION: sendmail_ReceivePacket
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle incoming messages and keypresses
****************************************************************************/
void sendmail_ReceivePacket(uint16 Length, uint8 *MailPtr)
{

  switch(M2H_16(((recDataType*) MailPtr)->PrimitiveIdentifier))
	{
	// case API_FP_HANDSET_PRESENT_IND:
	 // ApiFpHandsetPresentInd((CvmApiMailType *)MailPtr, Length);
	 // break;
	  //Application
    case INITTASK:
      break;
    case KEY_MESSAGE:
      HandleKeyMessage(MailPtr);
      break;
	default:
		mail_switch(Length, MailPtr);
		break;
	}
}
#endif

void ConsoleEnterKeyMessage( unsigned char ch )
{
	uint8 tempDataPtr[ 3 ];

    tempDataPtr[0] = (uint8) (KEY_MESSAGE&0x00ff);
    tempDataPtr[1] = (uint8) (KEY_MESSAGE>>8);
    tempDataPtr[2] = ch;
    
    sendmail_ReceivePacket(0, tempDataPtr);
}

 /****************************************************************************
*  FUNCTION: mail_switch
*
*  INPUTS  : mailpointer
*  OUTPUTS : none
*  RETURNS : none
*
*  DESCRIPTION: Handle incoming messages and keypresses
****************************************************************************/
void mail_switch(uint16 Length, uint8 *MailPtr)
{
  DEBUG_PRINT("Message %x received. \n", M2H_16( *(PrimitiveType*)MailPtr ));
  switch( M2H_16( *(PrimitiveType*)MailPtr ) )
  {
    case API_FP_GET_ID_CFM:
      HandleGetIdCfm((CvmApiMailType*)MailPtr);
       break;

    case API_FP_GET_EEPROM_CFM:
      HandleGetEepromCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_SET_EEPROM_CFM:
      HandleSetEepromCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_SET_LED_CFM:
      HandleSetLedCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_INIT_PCM_CFM:
      HandleInitPcmCfg((CvmApiMailType*)MailPtr);
      break;

    case API_FP_SET_REGISTRATION_MODE_CFM:
      HandleSetRegistrationModeCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_REGISTRATION_COMPLETE_IND:
      HandleRegistrationCompleteInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_HANDSET_PRESENT_IND:
      HandleHandsetPresentInd((CvmApiMailType*)MailPtr);
      break;

	case API_FP_GET_HANDSET_IPUI_CFM:
		HandleGetHandsetIpuiCfm((CvmApiMailType*)MailPtr);
		break;



    case API_FP_DELETE_REGISTRATION_CFM:
      HandleDeleteRegistrationCfm((CvmApiMailType*)MailPtr);
      break;

	case API_FP_REGISTRATION_DELETED_IND:
		HandleDeleteRegistrationInd((CvmApiMailType*)MailPtr);
		break;
    case API_FP_GET_REGISTRATION_COUNT_CFM:
      HandleGetRegistrationCoundCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_SETUP_IND:
      HandleSetupInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_CONNECT_CFM:
      HandleConnectCfm((CvmApiMailType*)MailPtr);
   //   PRINT_MESSAGE("->API_FP_CONNECT_CFM\n");
      break;

    case API_FP_SET_PCM_CODEC_CFM:
      HandleSetPcmCodecCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_RELEASE_IND:
      HandleReleaseInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_RESET_IND:
      HandleResetInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_MULTIKEYPAD_IND:
      HandleMultiKeyPadInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_CALLED_NUMBER_IND:
      HandleCalledNumberInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_SETUP_CFM:
      HandleSetupCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_ALERT_IND:
      HandleAlertInd((CvmApiMailType*)MailPtr);
      break;

	case API_FP_ALERT_CFM:
      HandleAlertCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_CONNECT_IND:
      HandleConnectInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_RELEASE_CFM:
      HandleReleaseCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_GET_FW_VERSION_CFM:
      HandleGetFwuVersionCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_SET_ACCESS_CODE_CFM:
      HandleSetAccessCodeCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_GET_ACCESS_CODE_CFM:
      HandleGetAccessCodeCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_SETUP_ACK_CFM:
      HandleSetupAckCfm((CvmApiMailType*)MailPtr);
      break;
    case API_FP_PROPRIETARY_REQ:
      HandleProprietaryCfm((CvmApiMailType*)MailPtr);
      break;
	case API_FP_PROPRIETARY_IND:
		HandleProprietaryInd((CvmApiMailType*)MailPtr);
		break;
	case API_FP_REJECT_IND:
		HandleRejectInd((CvmApiMailType*)MailPtr);
		break;


    case API_FP_MODIFY_CODEC_CFM:
      HandleFpModifyCodecCfm((CvmApiMailType*)MailPtr);
      break;
	case API_FP_MODIFY_CODEC_IND:
		HandleFpModifyCodecInd((CvmApiMailType*)MailPtr);
		break;
	case API_FP_CODEC_INFO_IND:
		HandleFpCodecInfoInd((CvmApiMailType*)MailPtr);
		break;


	case API_FP_IWU_TO_IWU_IND:
		HandleFpIwuToIwuInd((CvmApiMailType*)MailPtr);
		break;
	case API_FP_IWU_TO_IWU_CFM:
		HandleFpIwuToIwuCfm((CvmApiMailType*)MailPtr);
		break;


	case API_FP_GET_REGISTER_CFM:
		HandleFpGetRegisterCfm((CvmApiMailType*)MailPtr);
		break;




    case API_FP_FWU_ENABLE_CFM:
      ApiFpFwuEnableCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_FWU_DEVICE_NOTIFY_IND:
      ApiFpFwuDeviceNotifyInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_FWU_UPDATE_CFM:
      ApiFpFwuUpdateCfm((CvmApiMailType*)MailPtr);
      break;

    case API_FP_FWU_UPDATE_IND:
      ApiFpFwuUpdateInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_FWU_GET_BLOCK_IND:
      ApiFpFwuGetBlockInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_FWU_GET_CRC_IND:
      ApiFpFwuGetCrcInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_FWU_COMPLETE_IND:
      ApiFpFwuCompleteInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_FWU_STATUS_IND:
      ApiFpFwuStatusInd((CvmApiMailType*)MailPtr);
      break;

    case API_FP_PROPRIETARY_CFM:
    	HandleProprietaryCfm((CvmApiMailType*)MailPtr);
      break;
    case API_FP_CALL_PROC_CFM:
    	HandleCallProcCfm((CvmApiMailType*)MailPtr);
      break;
    default:
    	PRINT_MESSAGE("MESSAGE NOT HANDLED !!! : 0x%x \n", M2H_16( *(PrimitiveType*)MailPtr ));
    	break;

  }
}
#ifdef OLD_CC
/****************************************************************************
*			 SIP USER AGENT MSG HANDLING SECTION
*****************************************************************************/
#if 0
/****************************************************************************
* FUNCTION:	HandleSendConnectReq
* DESCRIPTION: First function to call on incoming call request. Sends API_FP_SETUP_REQ msg for
*		-Alerting handsets
*		-Configuring codec
*		-Assigning pcmchannels
*Next call state: F06_CALL_PRESENT
* INCOMING CALL
****************************************************************************/
void HandleSendConnectReq (callcntrl_ap_msg_u_type* msg_rx)
{
	uint8 i=0;
	uint8 call_codec = msg_rx->callcntrl_ap_incoming_call_ind.codec;
	uint8 line_id = msg_rx->callcntrl_ap_incoming_call_ind.line;
	PRINT_INFO("incomming codec = %X, line = %d \n",call_codec, line_id);
	ApiCallingNumberType *ClipNumber;
	ApiClipDataType *ClipData;
	//for testing
	uint8 *CallingNumber = "123456789";
	uint8 CallingNumberLen = strlen(CallingNumber);//+1 on ly if AUC_XCHAR8 is used
	// ApiCodecListType RequestedCodecList;
	// uint8 RequestedCodecListLength;

	//line_codec = call_codec;
	/*Translate requested codec to a valid represantation*/
	// RequestedCodecList.NegotiationIndicator = ANI_NOT_POSSIBLE;
	// RequestedCodecList.NoOfCodecs = 1;
	// RequestedCodecListLength = sizeof(CodecList);
	// switch(call_codec)
	// {
		// case DECT_NARROWBAND_CODEC:
			// RequestedCodecList.Codec[0].Codec = ACT_G726;
			// break;
		// case DECT_WIDEBAND_CODEC:
			// RequestedCodecList.Codec[0].Codec = ACT_G722;
			// break;
		// default:
			// RequestedCodecList.Codec[0].Codec = ACT_INVALID;
			// break;
	// }
// #ifdef full_mmi
	// #ifdef ATA_DESIGN
		// ap_callcntrl_msg_u_type msg;
		// /*HOOK - OFF */
		// PRINT_MESSAGE (" ============== respond codec supported ========== \n");
		// msg.ap_callcntrl_dect_codec_ind.msg_id = AP_CALLCNTRL_DECT_CODEC_IND;
		// msg.ap_callcntrl_dect_codec_ind.line = line_id;//2;//0;//1;//line
		// msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_SUPPORTED;
		// ap_send_msg_to_callcntrl( &msg );
	// #endif
// #endif


	for(i=0;i<NO_HS;i++)
	{
		if( (line_id == BROADCAST_HS) || (line_id == i) ){
			switch (CallState[i].State)
			{
//		case F10_ACTIVE:
				case HS_OUT_OF_RANGE:
				case F00_NULL:
				{
					ApiFpSetupReqType * NewMailPtr;
					ApiCodecListType CodecList;

					NewMailPtr = (ApiFpSetupReqType*) BUSM_AllocateMail(0, USER_TASK,
					sizeof(ApiFpSetupReqType) - 1 +
					sizeof (ApiClipDataType) -1 +
					sizeof (ApiCallingNumberType) - 1 + CallingNumberLen +
					sizeof(CodecList) + sizeof(CodecList.Codec));//this is space for max 2 codecs

					ClipData = (ApiClipDataType*)&NewMailPtr->Data[0];
					ClipNumber = (ApiCallingNumberType*)&ClipData->Data[0];
					ApiCodecListType *CodecListPtr = (ApiCodecListType*)(ClipNumber+sizeof (ApiCallingNumberType) - 1 + CallingNumberLen);

					/*Set clip fields*/
					ClipData->ReasonCode = ACRT_NO_REASON;        /* Reason code */
					ClipData->ReasonName = ACRT_NO_REASON;        /* Reason code */
					ClipData->UsedChar = AUC_DEFAULT;//AUC_IA5CHAR;//AUC_DEFAULT; //  ,/* This describes the encoding of the strings in  name and number. */
					ClipData->NumberLen = sizeof (ApiCallingNumberType) - 1 + CallingNumberLen;                     /* Number length */
					ClipData->NameLen = 0;                       /* Name Length */
					ClipData->TimeDateLen = 0;                   /* Length of time date */
					ClipData->VmwiLen = 0;                       /* Length of message waiting indicator */
					ClipData->NmssLen = 0;                       /* Lengt of SS element. */

					ClipNumber->NumberType = ANT_UNKNOWN;        /* Number type */
					ClipNumber->Npi = ANPI_UNKNOWN;//ANPI_TCP_IP;//ANPI_SIP;//ANPI_UNKNOWN;                      /* Numbering Plan Identification */
					ClipNumber->PresentationInd = API_PRESENTATION_ALLOWED; /* Presentation indicator-> */
					ClipNumber->NumberLength = CallingNumberLen;                  /* Number length in bytes */
					memcpy(ClipNumber->Number, CallingNumber, CallingNumberLen);
					if(ClipData->UsedChar == AUC_XCHAR8)
						ClipNumber->Number[CallingNumberLen]=0;//terminating char


					if(NewMailPtr != NULL)
					{
						NewMailPtr->Primitive           = H2M_16( API_FP_SETUP_REQ );
						NewMailPtr->HandsetId           = i+1;
						NewMailPtr->CallClass           = ACC_NORMAL;
	#ifdef DECT_NATALIE
						NewMailPtr->SourceId            = 0;//is not used anyway
	#else
						NewMailPtr->SourceId            = HandsetPcmChannel[i];
	#endif
						NewMailPtr->Signal              = HS_RING_TONE;
						NewMailPtr->ClipLength          = sizeof (ApiClipDataType) -1;
						NewMailPtr->CallingNumberLength = sizeof (ApiCallingNumberType) - 1 + CallingNumberLen;
						NewMailPtr->CallingNameLength   = 0;
						NewMailPtr->IwuLength           = 0;
						NewMailPtr->ProprietaryLength   = 0;
						//switch(PcmDataFormat[i])

						switch(call_codec)
						{
							//case AP_DATA_FORMAT_LINEAR_8kHz:
							case DECT_NARROWBAND_CODEC:
								NewMailPtr->BasicService       = ABS_BASIC_SPEECH;
								NewMailPtr->CodecListLength    = sizeof(CodecList);

								CodecList.NoOfCodecs           = 1;
								CodecList.NegotiationIndicator = ANI_NOT_POSSIBLE;
								memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));

								// memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));
								memcpy(CodecListPtr, (void*)&CodecList, sizeof(CodecList));

								break;
							//case AP_DATA_FORMAT_LINEAR_16kHz:
							case DECT_WIDEBAND_CODEC:
								NewMailPtr->BasicService       = ABS_WIDEBAND_SPEECH;
								NewMailPtr->CodecListLength    = sizeof(CodecList) + sizeof(CodecList.Codec);

								CodecList.NoOfCodecs           = 2;
								CodecList.NegotiationIndicator = ANI_POSSIBLE;
								memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));

								// memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));
								// memcpy((void*)&NewMailPtr->Data[sizeof(CodecList)], (void*)&CodecG726, sizeof(CodecList.Codec));
								memcpy(CodecListPtr, (void*)&CodecList, sizeof(CodecList));
								memcpy(CodecListPtr + sizeof(CodecList), (void*)&CodecG726, sizeof(CodecList.Codec));
								break;
						}
#if 1
						/* Store the propsed codec list */
						// Free old requested codec list (if any).
						if (CallState[i].RequestedCodecListLength > 0)
						{
							CallState[i].RequestedCodecListLength = 0;
							free(CallState[i].RequestedCodecListPtr);
						}
						if ( (call_codec == DECT_NARROWBAND_CODEC) || (call_codec == DECT_WIDEBAND_CODEC)  ){
							CallState[i].RequestedCodecListLength = NewMailPtr->CodecListLength;
							CallState[i].RequestedCodecListPtr = (ApiCodecListType*)malloc(CallState[i].RequestedCodecListLength);
							memcpy(CallState[i].RequestedCodecListPtr,(void*)&CodecList, CallState[i].RequestedCodecListLength);
						}
#endif
						PRINT_MESSAGE("<-API_FP_SETUP_REQ - HandsetID: %X\n", i+1);
						DisplayCodecList(NewMailPtr->CodecListLength, CodecListPtr);
						BUSM_DeliverMail((uint8*)NewMailPtr);
					}
				//	SetCallState(i, F07_CALL_RECEIVED);
				//	PRINT_MESSAGE("CallState = F07_CALL_RECEIVED\n");
					CallState[i].CallClass = ACC_NORMAL;
					SetCallState(i, F06_CALL_PRESENT);
					PRINT_MESSAGE("CallState = F06_CALL_PRESENT\n");

				}
				break;
			}
		}
	}
}
#else
/****************************************************************************
* FUNCTION:	HandleSendConnectReq
* DESCRIPTION: First function to call on incoming call request. Sends API_FP_SETUP_REQ msg for
*		-Alerting handsets
*		-Configuring codec
*		-Assigning pcmchannels
*Next call state: F06_CALL_PRESENT
* INCOMING CALL
****************************************************************************/
void HandleSendConnectReq (callcntrl_ap_msg_u_type* msg_rx)
{
	uint8 i=0;
	uint8 call_codec = msg_rx->callcntrl_ap_incoming_call_ind.codec;
	uint8 handset_id = LINE_TO_HS(msg_rx->callcntrl_ap_incoming_call_ind.line);
	PRINT_INFO("incomming codec = %X, line = %d \n",call_codec, msg_rx->callcntrl_ap_incoming_call_ind.line);

	// ApiCodecListType RequestedCodecList;
	// uint8 RequestedCodecListLength;

	//line_codec = call_codec;
	/*Translate requested codec to a valid represantation*/
	// RequestedCodecList.NegotiationIndicator = ANI_NOT_POSSIBLE;
	// RequestedCodecList.NoOfCodecs = 1;
	// RequestedCodecListLength = sizeof(CodecList);
	// switch(call_codec)
	// {
		// case DECT_NARROWBAND_CODEC:
			// RequestedCodecList.Codec[0].Codec = ACT_G726;
			// break;
		// case DECT_WIDEBAND_CODEC:
			// RequestedCodecList.Codec[0].Codec = ACT_G722;
			// break;
		// default:
			// RequestedCodecList.Codec[0].Codec = ACT_INVALID;
			// break;
	// }
// #ifdef full_mmi
	// #ifdef ATA_DESIGN
		// ap_callcntrl_msg_u_type msg;
		// /*HOOK - OFF */
		// PRINT_MESSAGE (" ============== respond codec supported ========== \n");
		// msg.ap_callcntrl_dect_codec_ind.msg_id = AP_CALLCNTRL_DECT_CODEC_IND;
		// msg.ap_callcntrl_dect_codec_ind.line = line_id;//2;//0;//1;//line
		// msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_SUPPORTED;
		// ap_send_msg_to_callcntrl( &msg );
	// #endif
// #endif


	for(i=0;i<NO_HS;i++)
	{
		if( (handset_id == BROADCAST_HS) || (handset_id == i+1) ){
			switch (CallState[i].State)
			{
//		case F10_ACTIVE:
				case HS_OUT_OF_RANGE:
				case F00_NULL:
				{
					ApiFpSetupReqType * NewMailPtr;
					ApiCodecListType CodecList;
					NewMailPtr = (ApiFpSetupReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpSetupReqType) - 1 + sizeof(CodecList) + sizeof(CodecList.Codec));
					if(NewMailPtr != NULL)
					{
						NewMailPtr->Primitive           = H2M_16( API_FP_SETUP_REQ );
						NewMailPtr->HandsetId           = i+1;
						NewMailPtr->CallClass           = ACC_NORMAL;
	#ifdef DECT_NATALIE
						NewMailPtr->SourceId            = 0;//is not used anyway
	#else
						NewMailPtr->SourceId            = HandsetPcmChannel[i];
	#endif
						NewMailPtr->Signal              = HS_RING_TONE;
						NewMailPtr->ClipLength          = 0;
						NewMailPtr->CallingNumberLength = 0;
						NewMailPtr->CallingNameLength   = 0;
						NewMailPtr->IwuLength           = 0;
						NewMailPtr->ProprietaryLength   = 0;
						//switch(PcmDataFormat[i])

						switch(call_codec)
						{
							//case AP_DATA_FORMAT_LINEAR_8kHz:
							case DECT_NARROWBAND_CODEC:
#ifndef USE_DSP_TRANSCODING								
								NewMailPtr->BasicService       = ABS_BASIC_SPEECH;
								NewMailPtr->CodecListLength    = sizeof(CodecList);

								CodecList.NoOfCodecs           = 1;
								CodecList.NegotiationIndicator = ANI_POSSIBLE;//ANI_NOT_POSSIBLE;
								memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));

								memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));


								break;
							//case AP_DATA_FORMAT_LINEAR_16kHz:
#endif								
							case DECT_WIDEBAND_CODEC:
								NewMailPtr->BasicService       = ABS_WIDEBAND_SPEECH;
								NewMailPtr->CodecListLength    = sizeof(CodecList) + sizeof(CodecList.Codec);

								CodecList.NoOfCodecs           = 2;
								CodecList.NegotiationIndicator = ANI_POSSIBLE;
								memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));

								memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));
								memcpy((void*)&NewMailPtr->Data[sizeof(CodecList)], (void*)&CodecG726, sizeof(CodecList.Codec));


								break;
						}
#if 1
						/* Store the requested codec*/
						//CallState[i].BasicService = NewMailPtr->BasicService;
						CallState[i].RequestedCodec = call_codec;
#endif
						PRINT_MESSAGE("<-API_FP_SETUP_REQ - HandsetID: %X\n", i+1);
						DisplayCodecList(NewMailPtr->CodecListLength, NewMailPtr->Data);
						BUSM_DeliverMail((uint8*)NewMailPtr);
					}
				//	SetCallState(i, F07_CALL_RECEIVED);
				//	PRINT_MESSAGE("CallState = F07_CALL_RECEIVED\n");
					CallState[i].CallClass = ACC_NORMAL;
					SetCallState(i, F06_CALL_PRESENT);
					PRINT_MESSAGE("CallState = F06_CALL_PRESENT\n");

				}
				break;
			}
		}
	}
}
#endif

/****************************************************************************
* FUNCTION:	HandleSendReleaseReq
* DESCRIPTION: Sends the following msgs to handsets:
*		-API_FP_RELEASE_REQ, to terminate call
*		-API_FP_SET_PCM_CODEC_REQ, to reset pcm codec
*		-Assigning pcmchannels
*Next call state: F06_CALL_PRESENT
* CALL TERMINATING FROM THE PEER
****************************************************************************/
void HandleSendReleaseReq(callcntrl_ap_msg_u_type* msg_rx)
{
	uint8 handset_id = LINE_TO_HS(msg_rx->callcntrl_ap_call_terminated_ind.line);	
	PRINT_INFO("Releasing line = %d \n", msg_rx->callcntrl_ap_call_terminated_ind.line);

	uint8 i=0;
	for(i=0;i<NO_HS;i++)
	{
		if( (handset_id == BROADCAST_HS) || (handset_id == i+1) ){
			switch (CallState[i].State)
			{
				case F06_CALL_PRESENT:
				case F07_CALL_RECEIVED:
				case F10_ACTIVE:
			//	if(CallState[i].State == F06_CALL_PRESENT)//F07_CALL_RECEIVED
				{
					ApiFpReleaseReqType*
					m1 = (ApiFpReleaseReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpReleaseReqType)));
					m1->Primitive = H2M_16( API_FP_RELEASE_REQ );
					m1->HandsetId = i+1;
					m1->Reason = ARR_NORMAL;
					BUSM_DeliverMail((uint8*)m1);
					PRINT_MESSAGE("<-API_FP_RELEASE_REQ - HandsetID: %X\n", (i+1));

					/*Reset pcm codec */
#ifdef DECT_NATALIE //in natalie no pcm port has been assigned until state has become F10_ACTIVE
					if (CallState[i].State == F10_ACTIVE){
						ApiFpSetPcmCodecReqType*
						m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));

						m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
						m2->PcmChannel =  HandsetPcmChannel[i];//i;
						m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
						BUSM_DeliverMail((uint8*)m2);

						PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[i], AP_DATA_FORMAT_NONE);

					}
#else
					ApiFpSetPcmCodecReqType*
					m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));

					m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
					m2->PcmChannel =  HandsetPcmChannel[i];//i;
					m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
					BUSM_DeliverMail((uint8*)m2);

					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[i], AP_DATA_FORMAT_NONE);
#endif //DECT_NATALIE
				}
				break;
			}
			SetCallState(i,F00_NULL);
			//vm====================
			PRINT_MESSAGE("CallState = F00_NULL\n");
		}
	}
 }

/****************************************************************************
* FUNCTION:	HandleOutgoingCallAnsweredInd
* DESCRIPTION:
* CALL TERMINATING FROM THE PEER
****************************************************************************/
 void HandleOutgoingCallAnsweredInd(callcntrl_ap_msg_u_type* msg_rx )
 {
	 uint8 req_codec = msg_rx->callcntrl_ap_codec_info_ind.codec;
	 //uint8 line_id = msg_rx->callcntrl_ap_codec_info_ind.line;
	 uint8 handset_id = LINE_TO_HS(msg_rx->callcntrl_ap_codec_info_ind.line);

	 PRINT_MESSAGE("Outgoing call answered codec:%X , line: %d\n", req_codec, msg_rx->callcntrl_ap_codec_info_ind.line);

 }
 
/****************************************************************************
* FUNCTION:	HandleCodecInfoInd
* DESCRIPTION:
* CALL TERMINATING FROM THE PEER
****************************************************************************/
void HandleCodecInfoInd(callcntrl_ap_msg_u_type* msg_rx )
{
	uint8 req_codec = msg_rx->callcntrl_ap_codec_info_ind.codec;
	//uint8 line_id = msg_rx->callcntrl_ap_codec_info_ind.line;
	uint8 handset_id = LINE_TO_HS(msg_rx->callcntrl_ap_codec_info_ind.line);	

	PRINT_MESSAGE("Codec info indication. Codec: %X , line: %d\n",req_codec, msg_rx->callcntrl_ap_codec_info_ind.line);

#ifdef USE_DSP_TRANSCODING
	#ifdef full_mmi
	#ifdef ATA_DESIGN
		ap_callcntrl_msg_u_type msg;
		/*HOOK - OFF */
		msg.ap_callcntrl_dect_codec_ind.msg_id = AP_CALLCNTRL_DECT_CODEC_IND;
		msg.ap_callcntrl_dect_codec_ind.line = HS_TO_LINE(handset_id);
		msg.ap_callcntrl_dect_codec_ind.pcm_port = ADPCM_TO_APP(HandsetPcmChannel[handset_id - 1]);	
		if ( ((req_codec == DECT_NARROWBAND_CODEC) && (CallState[handset_id-1].BasicService == ABS_BASIC_SPEECH) ) ||
			 ((req_codec == DECT_WIDEBAND_CODEC) && (CallState[handset_id-1].BasicService == ABS_WIDEBAND_SPEECH) ) ){
			PRINT_MESSAGE (" ============== respond codec supported ========== \n");					
			msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_SUPPORTED;
		}
		else{
			PRINT_MESSAGE (" ============== respond codec NOT supported ========== \n");					
			msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_NOT_SUPPORTED;
		}
		ap_send_msg_to_callcntrl( &msg );
	#endif
	#endif		
#else	
	/*Check if codec already in use*/
	if ( ((req_codec == DECT_NARROWBAND_CODEC) && (CallState[handset_id].BasicService == ABS_WIDEBAND_SPEECH) ) ||
		 ((req_codec == DECT_WIDEBAND_CODEC) && (CallState[handset_id].BasicService == ABS_BASIC_SPEECH) ) ){
		ModifyCodec(handset_id, req_codec);	
	}
	else{
		#ifdef full_mmi
			#ifdef DECT_DESIGN
				ap_callcntrl_msg_u_type msg;
				/*HOOK - OFF */
				PRINT_MESSAGE (" ============== respond codec supported ========== \n");
				msg.ap_callcntrl_dect_codec_ind.msg_id = AP_CALLCNTRL_DECT_CODEC_IND;
				msg.ap_callcntrl_dect_codec_ind.line = msg_rx->callcntrl_ap_codec_info_ind.line;
				msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_SUPPORTED;
				msg.ap_callcntrl_dect_codec_ind.pcm_port = HandsetPcmChannel[handset_id - 1];
				ap_send_msg_to_callcntrl( &msg );
			#endif
		#endif		
	}
#endif	

#if 0
#ifdef DECT_NATALIE
	PRINT_MESSAGE ("req_codec=%d, CallState[handset_id].BasicService=%d \n", req_codec, CallState[handset_id].BasicService);

	if ((req_codec == DECT_NARROWBAND_CODEC) && (CallState[handset_id].BasicService == ABS_WIDEBAND_SPEECH) )
		ModifyCodec(handset_id, req_codec);
#endif
#if 0
	new_codec = req_codec;

	PRINT_MESSAGE("............. try to change codec..........\n");
	{
		ApiFpSetPcmCodecReqType* m1 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));
		m1->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
		m1->PcmChannel =  HandsetPcmChannel[0];//HandsetPcmChannel[handsetId-1];
		m1->PcmDataFormat = AP_DATA_FORMAT_NONE;
		BUSM_DeliverMail((uint8*)m1);
		PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n",/* HandsetPcmChannel[handsetId - 1]*/HandsetPcmChannel[0], AP_DATA_FORMAT_NONE);
	}

	{
		ApiCodecListType CodecList;
		ApiFpModifyCodecReqType*
		m2 = (ApiFpModifyCodecReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpModifyCodecReqType) - 1 + sizeof(CodecList));

		m2->Primitive = H2M_16( API_FP_MODIFY_CODEC_REQ );
		m2->HandsetId = 1;//handsetId;
		m2->CodecListLength = sizeof(CodecList);
		CodecList.NoOfCodecs           = 1;
		CodecList.NegotiationIndicator = ANI_POSSIBLE;//ANI_NOT_POSSIBLE;
		if(req_codec)
		{
			memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));

		}
		else
		{
			memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));

		}

		memcpy((void*)&m2->Data[0], (void*)&CodecList, sizeof(CodecList));

		PRINT_MESSAGE("<-API_FP_MODIFY_CODEC_REQ - \n");
		// Display codec list
		DisplayCodecList(m2->CodecListLength, m2->Data);

		BUSM_DeliverMail((uint8*)m2);
	}
#endif
#endif
}

/****************************************************************************
* FUNCTION:	callcntrl_send_msg_to_ap
* DESCRIPTION: The switch which handles the msgs received by the SIP user agent.
*   INPUTS  :        msg: the message sent by the user agent
****************************************************************************/
#ifdef full_mmi
void callcntrl_send_msg_to_ap_dect( callcntrl_ap_msg_u_type* msg )
{
	switch (msg->msg_id){
		case CALLCNTRL_AP_INCOMING_CALL_IND:
			/*Start ringing*/
			PRINT_MESSAGE("RECEIVED INCOMING CALL FROM SIPUA \n");
#ifdef DECT_DESIGN
			HandleSendConnectReq(msg);
#endif
			break;
		case CALLCNTRL_AP_CALL_TERMINATED_IND:
			/*Go to on - hook*/
			PRINT_MESSAGE("RECEIVED TERMINATING CALL FROM SIPUA \n");
			HandleSendReleaseReq(msg);
			break;
#ifdef DECT_DESIGN
		case CALLCNTRL_AP_CODEC_INFO_IND:
			PRINT_MESSAGE("RECEIVED CODEC INFO IND FROM SIPUA\n");
			HandleCodecInfoInd(msg);
			break;
		case CALLCNTRL_AP_OUTGOING_CALL_ANSWERED_IND:
			PRINT_MESSAGE("RECEIVED CALL_ANSWERED_IND FROM SIPUA\n");
			HandleOutgoingCallAnsweredInd(msg);
			break;
#endif
#ifdef DECT_HEADSET_DESIGN
		case CALLCNTRL_AP_REG_INIT_IND:
			if(msg->callcntrl_ap_reg_init_ind.reg_mode==DELETE_HANDSETS){
				SetRegistrationMode(DELETE_ALL_MODE);
			}else if (msg->callcntrl_ap_reg_init_ind.reg_mode==REG_HANDSET){
				SetRegistrationMode(REGISTER_MODE);
			}
			PRINT_MESSAGE("RECEIVED CALLCNTRL_AP_REG_INIT_IND\n");
			break;
#endif
		default:
			break;
	}
}
#endif

/****************************************************************************
* FUNCTION:	ModifyCodec
* DESCRIPTION:
* CALL TERMINATING FROM THE PEER
****************************************************************************/
void ModifyCodec(unsigned char handsetId, unsigned char req_codec){
	
	PRINT_MESSAGE("Trying codec changen. Codec: %X , handsetId: %d\n",req_codec, handsetId);
	CallState[handsetId - 1].RequestedCodec = req_codec;
	
	/*Modify Codec - Set PCM codec to AP_DATA_FORMAT_NONE*/
	BUSM_SendMailP2(0, USER_TASK, API_FP_SET_PCM_CODEC_REQ, HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_NONE);
	PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[handsetId - 1], AP_DATA_FORMAT_NONE);
	PRINT_MESSAGE("  BasicService: %s\n", "AP_DATA_FORMAT_NONE");
	
	/*Modify Codec - Air codec*/
	{
		ApiCodecListType CodecList;
		ApiFpModifyCodecReqType*
		m2 = (ApiFpModifyCodecReqType*) BUSM_AllocateMail(0, USER_TASK, sizeof(ApiFpModifyCodecReqType) - 1 + sizeof(CodecList));

		m2->Primitive = H2M_16( API_FP_MODIFY_CODEC_REQ );
		m2->HandsetId = handsetId;
		m2->CodecListLength = sizeof(CodecList);
		CodecList.NoOfCodecs           = 1;
		CodecList.NegotiationIndicator = ANI_POSSIBLE;//ANI_NOT_POSSIBLE;//;//;
		if(req_codec == DECT_WIDEBAND_CODEC)
		{
			memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));

		}
		else
		{
			memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));

		}

		memcpy((void*)&m2->Data[0], (void*)&CodecList, sizeof(CodecList));

		PRINT_MESSAGE("<-API_FP_MODIFY_CODEC_REQ - \n");
		// Display codec list
		DisplayCodecList(m2->CodecListLength, m2->Data);

		BUSM_DeliverMail((uint8*)m2);
	}

	/*Modify codec in HandleFpCodecInfoInd*/
	
}
#else

/****************************************************************************
* FUNCTION:	HandleSendConnectReq
* DESCRIPTION: First function to call on incoming call request. Sends API_FP_SETUP_REQ msg for
*		-Alerting handsets
*		-Configuring codec
*		-Assigning pcmchannels
*Next call state: F06_CALL_PRESENT
* INCOMING CALL
****************************************************************************/
int HandleSendConnectReq (int codec, uint8 accountid, const char* dialednum, const char* callerid)
{
	uint8 i=0;
	uint8 call_codec = codec;
	uint8 handset_id = LINE_TO_HS(accountid);
	uint8 *CodecListPtr;
	int result=-1;//error
#ifdef CLIP_SUPPORT	
	ApiCallingNumberType *ClipNumber;
	ApiCallingNameType *ClipName;
	ApiClipDataType *ClipData;
	const uint8 *CallingNumber = dialednum;//"test_number";//
	uint8 CallingNumberLen = strlen(CallingNumber);//+1 only if AUC_XCHAR8 is used
#ifdef CALLING_NAME	
	const uint8 *CallingName = callerid;//"test_name";//
	uint8 CallingNameLen = strlen(CallingName);//+1 only if AUC_XCHAR8 is used
#endif	
#endif
	PRINT_INFO("INCOMING CALL: codec = %X, line = %d handid = %d, dialednum = %s, callerid = %s \n", call_codec, accountid, handset_id, dialednum, callerid);

	// ApiCodecListType RequestedCodecList;
	// uint8 RequestedCodecListLength;

	//line_codec = call_codec;
	/*Translate requested codec to a valid represantation*/
	// RequestedCodecList.NegotiationIndicator = ANI_NOT_POSSIBLE;
	// RequestedCodecList.NoOfCodecs = 1;
	// RequestedCodecListLength = sizeof(CodecList);
	// switch(call_codec)
	// {
		// case DECT_NARROWBAND_CODEC:
			// RequestedCodecList.Codec[0].Codec = ACT_G726;
			// break;
		// case DECT_WIDEBAND_CODEC:
			// RequestedCodecList.Codec[0].Codec = ACT_G722;
			// break;
		// default:
			// RequestedCodecList.Codec[0].Codec = ACT_INVALID;
			// break;
	// }
// #ifdef full_mmi
	// #ifdef ATA_DESIGN
		// ap_callcntrl_msg_u_type msg;
		// /*HOOK - OFF */
		// PRINT_MESSAGE (" ============== respond codec supported ========== \n");
		// msg.ap_callcntrl_dect_codec_ind.msg_id = AP_CALLCNTRL_DECT_CODEC_IND;
		// msg.ap_callcntrl_dect_codec_ind.line = line_id;//2;//0;//1;//line
		// msg.ap_callcntrl_dect_codec_ind.codec_supported=CODEC_SUPPORTED;
		// ap_send_msg_to_callcntrl( &msg );
	// #endif
// #endif
		
#ifdef DYNAMIC_CHANNEL_ALLOCATION
	if(!try_pcm_channel_allocation){//no pcm channel available
		PRINT_INFO("Connection rejected. No PCM channel available. \n");
		display_pcm_channel_status();
		return -1;
	}			
	uint8 pcm_channel=AP_CHANNEL_INVALID, pcm_channel_used=0;
#ifdef DECT_NATALIE
	pcm_channel=0;//a valid pcm channel is needed allthough it is supposed that it is not taken into account !
#else	
	pcm_channel = allocate_pcm_channel();
#endif
#endif	

	
	for(i=0;i<NO_HS;i++)
	{
		if( (handset_id == BROADCAST_HS) || (handset_id == i+1) ){
			switch (CallState[i].State)
			{
//		case F10_ACTIVE:
				case HS_OUT_OF_RANGE:
				case F00_NULL:
				{		
					ApiFpSetupReqType * NewMailPtr;
					ApiCodecListType CodecList;
					NewMailPtr = (ApiFpSetupReqType*) BUSM_AllocateMail(0, USER_TASK, 				
							sizeof(ApiFpSetupReqType) - 1 +
#ifdef CLIP_SUPPORT									
							sizeof (ApiClipDataType) -1 +					
							sizeof (ApiCallingNumberType) - 1 + CallingNumberLen +					
	#ifdef CALLING_NAME							
							sizeof (ApiCallingNameType) - 1 + CallingNameLen + 
#endif				
#endif
							sizeof(CodecList) + sizeof(CodecList.Codec));//this is space for max 2 codecs
					if(NewMailPtr != NULL)
					{
						NewMailPtr->Primitive           = H2M_16( API_FP_SETUP_REQ );
						NewMailPtr->HandsetId           = i+1;
						NewMailPtr->CallClass           = ACC_NORMAL;		
#ifdef DYNAMIC_CHANNEL_ALLOCATION
						HandsetPcmChannel[i] = pcm_channel;	
#endif
						NewMailPtr->SourceId 			= HandsetPcmChannel[i];
						NewMailPtr->Signal              = HS_RING_TONE;		
#ifndef CLIP_SUPPORT							
						NewMailPtr->ClipLength          = 0;
						NewMailPtr->CallingNumberLength = 0;
						NewMailPtr->CallingNameLength   = 0;
						NewMailPtr->IwuLength           = 0;
						NewMailPtr->ProprietaryLength   = 0;
						
						CodecListPtr = (uint8 *)(ApiCodecListType*)&NewMailPtr->Data[0];
#else
						NewMailPtr->ClipLength          = sizeof (ApiClipDataType) -1;
						NewMailPtr->CallingNumberLength = sizeof (ApiCallingNumberType) - 1 + CallingNumberLen;						
#ifdef CALLING_NAME							
						NewMailPtr->CallingNameLength   = sizeof (ApiCallingNameType) - 1 + CallingNameLen;	
#else
						NewMailPtr->CallingNameLength   = 0;
#endif						
						NewMailPtr->IwuLength           = 0;
						NewMailPtr->ProprietaryLength   = 0;						
						/*Set clip data*/
						ClipData = (ApiClipDataType*)&NewMailPtr->Data[0];
						ClipNumber = (ApiCallingNumberType*)&ClipData->Data[0];
						ClipName = (ApiCallingNameType*)(&ClipNumber->Number[0]+CallingNumberLen);
#ifdef CALLING_NAME	      						
						CodecListPtr = (uint8 *)(ApiCodecListType*)(ClipName+sizeof (ApiCallingNameType) - 1 + CallingNameLen);
#else
						CodecListPtr = (uint8 *)(ApiCodecListType*)ClipName;//(ApiCodecListType*)(ClipName+sizeof (ApiCallingNameType) - 1 + CallingNameLen);
#endif
						/*Set clip fields*/
						ClipData->ReasonCode = ACRT_NO_REASON;        /* Reason code */
						ClipData->ReasonName = ACRT_NO_REASON;        /* Reason code */
						ClipData->UsedChar = AUC_IA5CHAR;//AUC_DEFAULT; //AUC_XCHAR8;//  ,/* This describes the encoding of the strings in  name and number. */
						ClipData->NumberLen = sizeof (ApiCallingNumberType) - 1 + CallingNumberLen;                     /* Number length */
#ifdef CALLING_NAME	                        
						ClipData->NameLen = sizeof (ApiCallingNameType) - 1 + CallingNameLen;//0;                       /* Name Length */
#else
						ClipData->NameLen = 0;
#endif						
						ClipData->TimeDateLen = 0;                   /* Length of time date */
						ClipData->VmwiLen = 0;                       /* Length of message waiting indicator */
						ClipData->NmssLen = 0;                       /* Lengt of SS element. */

						ClipNumber->NumberType = ANT_UNKNOWN;        /* Number type */
						ClipNumber->Npi = ANPI_UNKNOWN;//ANPI_SIP;//ANPI_UNKNOWN;//ANPI_TCP_IP;//ANPI_SIP;//ANPI_UNKNOWN;                      /* Numbering Plan Identification */
						ClipNumber->PresentationInd = API_PRESENTATION_ALLOWED; /* Presentation indicator-> */
						ClipNumber->NumberLength = CallingNumberLen;                  /* Number length in bytes */
						memcpy(ClipNumber->Number, CallingNumber, CallingNumberLen);
#ifdef CALLING_NAME	    
						ClipName->UsedAlphabet=AUA_DECT;    /* Specifies the encoding of the name. */
						ClipName->PresentationInd=API_PRESENTATION_ALLOWED; /* Presentation indicator. */
						ClipName->NameLength=CallingNameLen;                /* Name length in bytes */
						memcpy(ClipName->Name, CallingName, CallingNameLen);/* The name (NameLength number of bytes). */
#endif
						if(ClipData->UsedChar == AUC_XCHAR8){
							ClipNumber->Number[CallingNumberLen-1]=0;//terminating char
#ifdef CALLING_NAME	  							
							ClipName->Name[CallingNameLen-1]=0;//terminating char
#endif							
						}
#endif										
						//switch(PcmDataFormat[i])
						switch(call_codec)
						{
							//case AP_DATA_FORMAT_LINEAR_8kHz:
							case DECT_NARROWBAND_CODEC:
#ifndef USE_DSP_TRANSCODING								
								NewMailPtr->BasicService       = ABS_BASIC_SPEECH;
								NewMailPtr->CodecListLength    = sizeof(CodecList);

								CodecList.NoOfCodecs           = 1;
								CodecList.NegotiationIndicator = ANI_POSSIBLE;//ANI_NOT_POSSIBLE;
								memcpy((void*)&CodecList.Codec[0], (void*)&CodecG726, sizeof(CodecList.Codec));

								//memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));
								memcpy(CodecListPtr, (void*)&CodecList, sizeof(CodecList));

								break;
							//case AP_DATA_FORMAT_LINEAR_16kHz:
#endif								
							case DECT_WIDEBAND_CODEC:
								NewMailPtr->BasicService       = ABS_WIDEBAND_SPEECH;
								NewMailPtr->CodecListLength    = sizeof(CodecList) + sizeof(CodecList.Codec);

								CodecList.NoOfCodecs           = 2;
								CodecList.NegotiationIndicator = ANI_POSSIBLE;
								memcpy((void*)&CodecList.Codec[0], (void*)&CodecG722, sizeof(CodecList.Codec));

								// memcpy((void*)&NewMailPtr->Data[0], (void*)&CodecList, sizeof(CodecList));
								// memcpy((void*)&NewMailPtr->Data[sizeof(CodecList)], (void*)&CodecG726, sizeof(CodecList.Codec));
								memcpy(CodecListPtr, (void*)&CodecList, sizeof(CodecList));
								memcpy(CodecListPtr + sizeof(CodecList), (void*)&CodecG726, sizeof(CodecList.Codec));

								break;
						}
#if 1
						/* Store the requested codec*/
						//CallState[i].BasicService = NewMailPtr->BasicService;
						CallState[i].RequestedCodec = call_codec;
						CallState[i].LineId = handset_id;
#ifdef DYNAMIC_CHANNEL_ALLOCATION
						pcm_channel_used=1;//if the allocated pcm channel was not used, we must free it!!
#endif
#endif
						PRINT_MESSAGE("<-API_FP_SETUP_REQ - HandsetID: %X\n", i+1);
						//DisplayCodecList(NewMailPtr->CodecListLength, NewMailPtr->Data);
						DisplayCodecList(NewMailPtr->CodecListLength, CodecListPtr);
						BUSM_DeliverMail((uint8*)NewMailPtr);
						result=0;
					}
				//	SetCallState(i, F07_CALL_RECEIVED);
				//	PRINT_MESSAGE("CallState = F07_CALL_RECEIVED\n");
					CallState[i].CallClass = ACC_NORMAL;
					SetCallState(i, F06_CALL_PRESENT);
					PRINT_MESSAGE("CallState = F06_CALL_PRESENT\n");

				}
				break;
				
				default:
					;
			}
		}
	}
#ifdef DYNAMIC_CHANNEL_ALLOCATION	
	#ifndef DECT_NATALIE
	if(!pcm_channel_used){
		free_pcm_channel(pcm_channel);
	}
	display_pcm_channel_status();
	#endif	
#endif	
	return result;
}

/****************************************************************************
* FUNCTION:	HandleSendReleaseReq
* DESCRIPTION: Sends the following msgs to handsets:
*		-API_FP_RELEASE_REQ, to terminate call
*		-API_FP_SET_PCM_CODEC_REQ, to reset pcm codec
*		-Assigning pcmchannels
*Next call state: F06_CALL_PRESENT
* CALL TERMINATING FROM THE PEER
****************************************************************************/
void HandleSendReleaseReq(uint8 accountid)
{
  uint8 handset_id = LINE_TO_HS((uint8)accountid);	
	PRINT_INFO("DROP CALL: line = %d \n", accountid);

	uint8 i=0;
	for(i=0;i<NO_HS;i++)
	{
		if( ((handset_id == BROADCAST_HS) && (CallState[i].LineId == BROADCAST_HS) && CallState[i].State != F10_ACTIVE) || (handset_id == i+1) ){
			switch (CallState[i].State)
			{
				case F06_CALL_PRESENT:
				case F07_CALL_RECEIVED:
				case F10_ACTIVE:
			//	if(CallState[i].State == F06_CALL_PRESENT)//F07_CALL_RECEIVED
				{
					ApiFpReleaseReqType*
					m1 = (ApiFpReleaseReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpReleaseReqType)));
					m1->Primitive = H2M_16( API_FP_RELEASE_REQ );
					m1->HandsetId = i+1;
					m1->Reason = ARR_NORMAL;
					BUSM_DeliverMail((uint8*)m1);
					PRINT_MESSAGE("<-API_FP_RELEASE_REQ - HandsetID: %X\n", (i+1));

					/*Reset pcm codec */
#ifdef DECT_NATALIE //in natalie no pcm port has been assigned until state has become F10_ACTIVE
					if (CallState[i].State == F10_ACTIVE){
						ApiFpSetPcmCodecReqType*
						m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));

						m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
						m2->PcmChannel =  HandsetPcmChannel[i];//i;
						m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
						BUSM_DeliverMail((uint8*)m2);

						PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[i], AP_DATA_FORMAT_NONE);

					}
#else
					ApiFpSetPcmCodecReqType*
					m2 = (ApiFpSetPcmCodecReqType*)BUSM_AllocateMail(0,USER_TASK,(sizeof(ApiFpSetPcmCodecReqType)));

					m2->Primitive = H2M_16( API_FP_SET_PCM_CODEC_REQ );
					m2->PcmChannel =  HandsetPcmChannel[i];//i;
					m2->PcmDataFormat = AP_DATA_FORMAT_NONE;
					BUSM_DeliverMail((uint8*)m2);

					PRINT_MESSAGE("<-API_FP_SET_PCM_CODEC_REQ - PCM channel %X, PcmDataFormat %X\n", HandsetPcmChannel[i], AP_DATA_FORMAT_NONE);
#endif //DECT_NATALIE
				}
				break;
				
				default:
					;
			}
			SetCallState(i,F00_NULL);
			//vm====================
			PRINT_MESSAGE("CallState = F00_NULL\n");
		}
	}
 }

#endif

void print_progress_bar(unsigned char *title_string, unsigned int current, unsigned int total, unsigned int bar_size){

#if 1
	//const char stick_sign[] = { '-', '\\', '|', '//' };
	//static unsigned char stick_i = 0;
	//unsigned char ratio = ( current << 7 ) / total;
	#define ANSI_POSITION_HOME	"\x1b[0;0H"
	//fprintf(stderr, ANSI_POSITION_HOME "%d/128  %s\n", ratio, title_string);
	fprintf(stderr, ANSI_POSITION_HOME "%04X/%04X  %s\n", current >> 8, total >> 8, title_string);
#else
	int i =0;
	unsigned char percentage=0;

	percentage=current*100/total;//percentage
	for (i=0; i<bar_size*percentage/100;i++){
		fprintf(stderr, "|");
	}
	while(i++<bar_size)
		fprintf(stderr,".");
	fprintf(stderr, " %.3d %% %s\r", percentage, title_string);
#endif
}

#ifdef DYNAMIC_CHANNEL_ALLOCATION
uint8 allocate_pcm_channel(){
	uint8 i;
	for(i=AP_CHANNEL_0;i<=MAX_PCM_CHANNEL;i++){
		if(!(PcmChannelAllocation & (1<<i))){
			PcmChannelAllocation |= 1<<i;
			PRINT_MESSAGE("Allocated pcm channel %d, PcmChannelAllocation: %d \n", i, PcmChannelAllocation);			
			return i;
		}
	}
	
	return AP_CHANNEL_INVALID;
}

void free_pcm_channel(uint8 pcm_channel_id){
	if(/*pcm_channel_id>=0 &&*/ pcm_channel_id<=MAX_PCM_CHANNEL){
		PcmChannelAllocation&= ~(1<<pcm_channel_id);
		PRINT_MESSAGE("Releasing pcm channel %d, PcmChannelAllocation: %d \n", pcm_channel_id, PcmChannelAllocation);
	}
	else{
		PRINT_MESSAGE("Invalid call of releasing pcm channel %d, PcmChannelAllocation: %d \n", pcm_channel_id, PcmChannelAllocation);
	}
}
/****************************************************************************
* FUNCTION:	display_pcm_channel_status
* DESCRIPTION: Used for debugging of the DYNAMIC_CHANNEL_ALLOCATION support. 
* Displays array with following contents:
* -the HS id
* -the value of the HandsetPcmChannel variable for that handset
* -OK: if the value of the HandsetPcmChannel variable for that handset has been allocated
* -ERROR: if the value of the HandsetPcmChannel variable for that handset has NOT been allocated
* An error msg if there are pcm channels allocated but not stored to any HandsetPcmChannel variable  
****************************************************************************/
void display_pcm_channel_status(){
#if 0	
	uint i, j, pcm_map=PcmChannelAllocation;
	PRINT_MESSAGE("HS \t PCM_CHANNEL \t PCM_ALLOCATED ? \n");
	for(i=0;i<NO_HS;i++){
		j=1;
		if(HandsetPcmChannel[i] != AP_CHANNEL_INVALID) {
			if(pcm_map & (1<<HandsetPcmChannel[i])){
				pcm_map &= ~(1<<HandsetPcmChannel[i]);
				j=0;
			}
		}		
		PRINT_MESSAGE("%d \t %d \t\t %s \n", i+1, HandsetPcmChannel[i], (j==0)?"OK":"ERROR" );
	}
	if(pcm_map){
		for(i=0;i<=MAX_PCM_CHANNEL;i++){
			if(pcm_map & (1<<i)){
				PRINT_MESSAGE("ERROR: Pcm channel %d is allocated but not bound to any handset !\n", i);
			}
		}
	}
#endif
}
#endif //DYNAMIC_CHANNEL_ALLOCATION

/****************************************************************************
* FUNCTION:	SetRfpi
* DESCRIPTION: Sends a API_FP_SET_EEPROM_REQ msg for setting the content of the eeprom location 
* where the RFPI is stored, equal to the value of the CC_RFPI global variable
****************************************************************************/
#ifdef SET_RFPI
void SetRfpi(){
    	ApiFpSetEepromReqType * NewMailPtr;
    	NewMailPtr = (ApiFpSetEepromReqType*) BUSM_AllocateMail(0, USER_TASK, (sizeof(ApiFpSetEepromReqType)+7));
    	if(NewMailPtr != NULL)
    	{
    		NewMailPtr->Primitive  = H2M_16( API_FP_SET_EEPROM_REQ );
    		NewMailPtr->Address    = H2M_16( RFPI_EEPROM_ADDR );
    		NewMailPtr->DataLength = 5;
    		memcpy(NewMailPtr->Data, CC_RFPI, 5);
    		BUSM_DeliverMail((uint8*) NewMailPtr); 
    	}
    	PRINT_INFO("Setting RFPI to: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n\n",
    			CC_RFPI[0], CC_RFPI[1], CC_RFPI[2], CC_RFPI[3], CC_RFPI[4]);
    	PRINT_MESSAGE("<-API_FP_SET_EEPROM_REQ\n");   
}
#endif //SET_RFPI
