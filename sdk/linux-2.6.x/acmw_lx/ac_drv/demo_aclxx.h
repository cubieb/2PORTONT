/****************************************************************************
*
*	Company:			Audiocodes Ltd.
*
*   Project:			Demo ACLXx
*
*   File Name: 			demo_aclxx.h
*
*
******************************************************************************
*
* 	DESCRIPTION:		definitions for the demo app
*
******************************************************************************/
#ifndef 	_DEMO_ACLXX_
#define	_DEMO_ACLXX_

#define DEV_NAME			"acdev"
#define ACLXX_DEMO_MAJOR		250 //for char device define in AP/mkimg
#define ACLXX_DEMO_DIR_NAME_STR		"dsp"
#define ACLXX_DEMO_DEVICE_NAME_STR	"0"
#define ACLXX_DEMO_FD_NAME_STR		"/dev/dsp0"

#ifndef ACMW_MAX_NUM_CH
#define ACMW_MAX_NUM_CH 2
#endif
//#define NUM_OF_CHANNELS 4
#define AC494_DSP_RECEIVER_PACKET_SIZE	1608


/* use this list to get the status of each routine */
typedef enum {

	AC494_DSP_ERROR_E = 	-1,
	AC494_DSP_OK_E	=	0,
	AC494_DSP_NO_MODULE_E = -2,
	AC494_DSP_NO_DOWNLOAD_E = -3,
	AC494_DSP_NO_INIT_E = -4

} AC494_DSP_STATUS_ENT;

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

	unsigned int 									command;
	int 											Device;
	int 											pollTime100Msec;
} AC494_DSP_DeviceReport_STCT;
	

#endif// 	_DSP_DRV_
