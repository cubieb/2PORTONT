/****************************************************************************
*
*	Company:			Audiocodes Ltd.
*
*   Project:			VoPP SoC
*
*   Hardware Module: 	AC494-EVM
*
*   File Name: 			dsp_drv.h
*
*
******************************************************************************
*
* 	DESCRIPTION:		definitions for the ac49xdsp driver
*
******************************************************************************/
#ifndef 	_DSP_DRV_
#define	_DSP_DRV_

#define DSP_MAJOR						250 //for char device define in AP/mkimg
#define AC49X_DSP_DRV_DIR_NAME_STR		"dsp"
#define AC49X_DSP_DRV_DEVICE_NAME_STR	"0"
#define AC49X_DSP_DRV_FD_NAME_STR		"/dev/dsp0"

#ifndef ACMW_MAX_NUM_CH
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_SLIC
#define ACMW_MAX_NUM_CH 4
#elif defined (CONFIG_RTK_VOIP_DRIVERS_IP_PHONE)
#define ACMW_MAX_NUM_CH 2
#else
#error "ACMW_MAX_NUM_CH is not defined" 
#endif
#endif
//#define NUM_OF_CHANNELS 4
#define AC494_DSP_RECEIVER_PACKET_SIZE	1608
//#define DSP_PROGRAM_SIZE	0x80000
//#define DSP_PROGRAM_ORDER_SIZE	19 /* 0x80000 = 2^19 */


/* this is the list of the commands to the dsp driver */
typedef enum {

	DOWNLOAD_CMD_E,
	RESET_BUF_DES_E,
	TRANSMIT_CMD_E,
	RECEIVE_CMD_E,
	SETUP_DEVICE_CMD_E,
	CODEC_CNFG_CMD_E,
	ADVANCE_CHANNEL_CNFG_CMD_E,
	OPEN_CHANNEL_CMD_E,
	UPDATE_CHANNEL_CMD_E,
	ACTIVE_REGULAR_CHANNEL_CMD_E,
	UPDATE_REGULAR_CHANNEL_CMD_E,
	CLOSE_REGULAR_CHANNEL_CMD_E,
	INIT_DSP_CMD_E,
	DEFAULTS_DSP_CMD_E,
	SEND_IBS_STRING_CMD_E,
	SEND_IBS_STOP_CMD_E,
	SEND_EXTENDED_IBS_STRING_CMD_E,
	GET_CHANNEL_CMD_E,
	GET_DEVICE_CMD_E,
	SET_CHANNEL_CMD_E,
	SET_DEVICE_CMD_E,
	SET_3WAYCONF_CMD_E,
	SET_CODEC_CMD_E,
	CLOSE_CHANNEL_CMD_E,
	MEM_MODIFY_CMD_E,
	MEM_DISPLAY_CMD_E,
	MEM_CHECKSUM_CMD_E,
	ECHO_CANCELER_CNFG_CMD_E,
	//EXTENDED CMD for RTL8186 :: add by miki
	RTL8186_RING_CHx,
	RTL8186_MEDIA_ROUTE,
	RTL8186_MEDIA_ACTIVATE,
	RTL8186_MEDIA_CLOSE,
	RTL8186_PCM_RESET,
	RTL8186_PCM_ENABLE,
	RTL8186_PCM_DISABLE,
	RTL8186_HOOK_STATUS,
	RTL8186_DEVICE_POLL_TIME100MSEC
} DSP_DRIVER_COMMAND_ENT;




/* use this list to get the status of each routine */
typedef enum {

	AC494_DSP_ERROR_E = 	-1,
	AC494_DSP_OK_E	=	0,
	AC494_DSP_NO_MODULE_E = -2,
	AC494_DSP_NO_DOWNLOAD_E = -3,
	AC494_DSP_NO_INIT_E = -4

} AC494_DSP_STATUS_ENT;


typedef enum {

	AC494_CODEC_HANDSET,
	AC494_CODEC_HEADSET,
	AC494_CODEC_MIC,
	AC494_CODEC_SPEAKER,
	AC494_CODEC_LINE

} AC494_CODEC_USER_ENT;

typedef enum {

	AC494_CODEC_INPUT,
	AC494_CODEC_OUTPUT,
	AC494_CODEC_LOOP_ON,
	AC494_CODEC_LOOP_OFF

} AC494_CODEC_DIRECTION_ENT;


/* next are the command structure wrap mmAC494the AC49X parameters */

#define SIZE_OF_FLAG	4 /* sizeof(unsigned int) */

typedef struct {

	unsigned int	command;
	char			*params;

} DSP_CHAR_WRITE_COMMAND_STC;

typedef struct {

	unsigned int 					command;
	int 							Device;
	Tac49xSetupDeviceAttr 			SetupDeviceAttr;

} AC494_DSP_SetupDevice_STCT;

typedef struct {

	unsigned int 					command;
	int 							Channel;
	Tac49xSetupChannelAttr 			SetupChannelAttr;

} AC494_DSP_SetupChannel_STCT;

#if (AC49X_DEVICE_TYPE != ACLX5280_DEVICE)     
typedef struct {

	unsigned int 					command;
	int 							Device;
	int 							Channel;
	Tac49xCodecConfigurationAttr	Attr;

} AC494_DSP_CodecConfg_STCT;
#endif

typedef struct {

	unsigned int 							command;
	int 									Device;
	int 									Channel;
	Tac49xAdvancedChannelConfigurationAttr	Attr;

} AC494_DSP_AdvanceChannelConfg_STCT;

typedef struct {

	unsigned int 							command;
	int 									Device;
	int 									Channel;
	Tac49xOpenOrUpdateChannelConfigurationAttr Attr;

} AC494_DSP_OpenUpdateChannelConfg_STCT;

typedef struct {

	unsigned int 							command;
	int 									Device;
	int 									Channel;
	Tac49xActivateOrUpdateRtpChannelConfigurationAttr Attr;

} AC494_DSP_ActivateUpdateRegularRtpChannelConfg_STCT;

typedef struct {

	unsigned int 							command;
	int 									Device;
	int 									Channel;
	Tac49xIbsStringAttr 					Attr;

} AC494_DSP_SendIbsString_STCT;

typedef struct {

	unsigned int 							command;
	int 									Device;
	int 									Channel;

} AC494_DSP_SendIbsStop_STCT;

typedef struct {

	unsigned int 							command;
	int 									Device;
	int 									Channel;
	Tac49xExtendedIbsStringAttr				Attr;

} AC494_DSP_SendExtendedIbsString_STCT;

typedef struct {

	unsigned int 					command;
	Tac49xTxPacketParams 		TxPacketParams;

} AC494_DSP_Transmit_STCT;

typedef struct {

	unsigned int 					command;
	Tac49xRxPacketParams 			RxPacketParams;

} AC494_DSP_Receive_STCT;


typedef struct {

	unsigned int 												command;
	int 														Device;
	int 														Channel;
	Tac49xActivateOrDeactivate3WayConferenceConfigurationAttr 	Attr;

} AC494_DSP_ActivateOrDeactivate3WayConf_STCT;


typedef struct {

	unsigned int 				command;
	int							Device;
	int							Channel;
	AC494_CODEC_USER_ENT		user;
	AC494_CODEC_DIRECTION_ENT	direction;

} AC494_DSP_CODEC_DIR_STCT;


typedef struct {

	unsigned int 							command;
	int 									Device;
	int 									Channel;

} AC494_DSP_CloseChannel_STCT;


typedef struct {

	unsigned int					command;
	unsigned int 					addr;
	unsigned int 					data;
	int								count;

} AC494_MEM_MODIFY_STCT;

typedef struct {

	unsigned int 							command;
	unsigned int							addr;
	int										count;

} AC494_MEM_DISPLAY_STCT;

typedef struct {

	unsigned int 				command;
	unsigned int 				addr1;
	int							count;
	int							checksum;

} AC494_MEM_CHECKSUM_STCT;


#if (AC49X_DEVICE_TYPE != ACLX5280_DEVICE)     
typedef struct {

	unsigned int 									command;
	int 											Device;
	int 											Channel;
	Tac49xAcousticEchoCancelerConfigurationAttr		Attr;

} AC494_DSP_AcousticEchoCancelerConfiguration_STCT;
#endif

typedef struct {

	unsigned int 									command;
	int 											Device;
	int 											pollTime100Msec;
} AC494_DSP_DeviceReport_STCT;


#endif// 	_DSP_DRV_
