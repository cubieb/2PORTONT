/****************************************************************************
*
*   Company:			Audiocodes Ltd.
*
*   Project:			VoPP SoC
*
*   Hardware Module: 		AC494-EVM
*
*   File Name: 			ac_dsp_init.c
*
*
******************************************************************************
*
* 	DESCRIPTION:		This file init the AC49X
*
******************************************************************************/


#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/devfs_fs_kernel.h>
#include <linux/spinlock.h>
#include <asm/semaphore.h>
#include "AC49xDrv_Drv.h"
#include "demo_aclxx.h"

#include "userdef.h"
#include "si3210init.h"
#include "type.h"
#include "voip_params.h"
#include "voip_control.h"
extern void Ac49xSetSegB(int segB);

static int dspDev_setDefaults(void);
extern void dspStart(void);



/* define the driver structure holding the dsp device and channels status */
typedef struct dsp_dev_stc {

	devfs_handle_t 		handle;
	Tac49xSetupChannelAttr 	SetupChannelAttr[ACMW_MAX_NUM_CH]; 	/* the channels status */
	Tac49xSetupDeviceAttr	SetupDeviceAttr; 			/* the device status */
	int			downloadStatus;  			/* download flag, 0=no download */
	int			initStatus;                        	/* init flag, 0=no init */
//	int 			dspCashSegment;   			/* derived from the dinamic allocation for the dsp memory */
	int			numOfChannels;
//	struct semaphore 	sem;

} DSP_DEV;


static devfs_handle_t devfs_dir_handle = NULL;
static DSP_DEV  ac_dsp_dev;
static DSP_DEV *dsp_dev = &ac_dsp_dev;

#if 0
static unsigned int 	baseAddr_dspDev;
static int			size_dspDev;

static unsigned int baseAddr_setupDevice;
static unsigned int baseAddr_setupChannels;
#endif

#define VOIP_SRC_IP		IP2LONG(192, 168, 1, 158) //IP2LONG(10,16,2,65)
#define VOIP_DST_IP		IP2LONG(192, 168, 1, 159) //IP2LONG(10,16,2,68)
#define VOIP_SRC_DST_PORT		6000




/******************************************************************************
*
* 	Description:
*
******************************************************************************/
static void SendToNetwork(int channel, char *vpNITxBuff, S16 NetPayloadSize, Tac49xProtocol portProtocol)
{

	unsigned int 	 		sid;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return;
			

	if ((vpNITxBuff != NULL) && (NetPayloadSize>0))
	{
		sid = (2*channel+portProtocol-1);
		system_process_rtp_tx(channel, sid, vpNITxBuff, NetPayloadSize);
	}


}

/******************************************************************************
*
* 	Description:
*
******************************************************************************/
int32 Ac49xTxPacketCB( uint32 channel, uint32 mid, void* packet, uint32 pktLen, uint32 flags )
{
	Tac49xTxPacketParams 	TxPacketParams2;
	__attribute__ ((aligned (8))) static Tac49xPacket		ac49xPacket;	
	Tac49xTxPacketParams 	*pTxPacketParams2 = &TxPacketParams2;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
	
	if ((packet == NULL) || pktLen <0)
		return -1;

	memcpy(ac49xPacket.u.Payload, (char *)packet, pktLen);

	pTxPacketParams2->TransferMedium 			= TRANSFER_MEDIUM__HOST;
	pTxPacketParams2->UdpChecksumIncluded 	= CONTROL__DISABLE;
	pTxPacketParams2->FavoriteStream 			= CONTROL__DISABLE;
	pTxPacketParams2->Device	 				= mid;
	pTxPacketParams2->Channel 				= channel;
	pTxPacketParams2->pOutPacket 				= (char *)&ac49xPacket;
	pTxPacketParams2->PacketSize 				= pktLen+sizeof(Tac49xHpiPacketHeader);
	pTxPacketParams2->Protocol = mid;

	
	Ac49xTransmitPacket(pTxPacketParams2);

	return 0;
}


/******************************************************************************
*
* 	Description:
*
******************************************************************************/
static int ACMWPacketCB(char *buff,  U32 PacketSize)
{
	U16 channel;
	Tac49xPacket * pPacket = (Tac49xPacket *)buff;
	
	channel = pPacket->HpiHeader.Channel;
	
	switch(pPacket->HpiHeader.Protocol)
	{
		case PROTOCOL__RTP:
		 	SendToNetwork(channel, &pPacket->u, PacketSize-sizeof(Tac49xHpiPacketHeader), PROTOCOL__RTP);
			break;

		case PROTOCOL__RTCP:
		 	SendToNetwork(channel, &pPacket->u, PacketSize-sizeof(Tac49xHpiPacketHeader), PROTOCOL__RTCP);
		 	break;

		}
}




int ActivateMediaSession(Tac49xTxPacketParams	*pTxPacketParams)
{

	TstVoipMgrSession voipMgrSession;
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	unsigned int s_id, stat;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;

	voipMgrSession.ch_id 			= channel;
	voipMgrSession.m_id 			= PROTOCOL__RTP;
	voipMgrSession.protocol 	  	= UDP_PROTOCOL;
	voipMgrSession.ip_dst_addr  	= VOIP_SRC_IP; 	
	voipMgrSession.ip_src_addr  	= VOIP_DST_IP; 	
	voipMgrSession.udp_src_port 	= VOIP_SRC_DST_PORT; 
	voipMgrSession.udp_dst_port 	= VOIP_SRC_DST_PORT; 
	s_id = (2*channel+PROTOCOL__RTP-1);
	printk("reg RTP...");
	stat = rtk_trap_register(&voipMgrSession, channel, PROTOCOL__RTP, s_id,  Ac49xTxPacketCB);
	printk("ok!\n");
	voipMgrSession.ch_id 			= channel;
	voipMgrSession.m_id 			= PROTOCOL__RTCP;
	voipMgrSession.protocol 	  	= UDP_PROTOCOL;
	voipMgrSession.ip_dst_addr  	= VOIP_SRC_IP;  
	voipMgrSession.ip_src_addr  	= VOIP_DST_IP;  
	voipMgrSession.udp_src_port 	= VOIP_SRC_DST_PORT+1;
	voipMgrSession.udp_dst_port 	= VOIP_SRC_DST_PORT+1;


	s_id = (2*channel+PROTOCOL__RTCP-1);
	printk("reg RTCP...");
	stat += rtk_trap_register(&voipMgrSession, channel, PROTOCOL__RTCP, s_id,  Ac49xTxPacketCB);// register rx packets call back function
	printk("ok!\n");
	
#if (PACKET_RECORDING_ENABLE==1)
	Ac49xInitializePacketRecording(PacketRecordingCB);
#endif

#if 0
 	Ac49xInitializePacketReceive(ACMWPacketCB); 
	printk("1...\n");
	Ac49xSetupDevice(device, &dsp_dev->SetupDeviceAttr);
	printk("2...\n");
	Ac49xAdvancedChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].Advanced);
	printk("3...\n");
	Ac49xOpenChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel);
	printk("4...\n");
	PCM_restart(channel);
	printk("5...\n");
	Ac49xActivateRegularRtpChannelConfiguration(device,channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp);
	printk("6...\n");
#endif

#if 0
//test IBS generation
	{	Tac49xIbsStringAttr IbsAttr;
		IbsAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;
		IbsAttr.NumberOfDigits = 1;
		IbsAttr.OnDuration = 2000;
		IbsAttr.OffDuration = 0;
		IbsAttr.PauseDuration = 0;		
		IbsAttr.Signal[0].Pause = CONTROL__DISABLE;
		IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15;
		IbsAttr.Signal[0].Digit = 0;
		Ac49xSendIbsString(device, channel, &IbsAttr);
	}
#endif
	return 0;
}

/****************************************************************************
*
*	Name:		dspDev_setDefaults
*----------------------------------------------------------------------------
*	Abstract:	set the dsp device and channels to defaults values
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = AC494_DSP_OK_E
******************************************************************************/

Tac49xTxPacketParams TxPacketParams;
Tac49xTxPacketParams *pTxPacketParams = &TxPacketParams;

static int dspDev_setDefaults(void)
{

	int channel;

	/* get device defaults */
	Ac49xSetDefaultSetupDeviceAttr(&(dsp_dev->SetupDeviceAttr));

	dsp_dev->SetupDeviceAttr.Open.NumberOfChannels = dsp_dev->numOfChannels;

	/* set the channels over the device */
	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++)
	{
		dsp_dev->SetupDeviceAttr.Open.Channel[channel].Slot = channel;
		dsp_dev->SetupDeviceAttr.Open.Channel[channel].InterconnectedSlot = channel+4;
	}

	/* set channels defaults */
	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++)
		Ac49xSetDefaultSetupChannelAttr(&(dsp_dev->SetupChannelAttr[channel]));

	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++) 
	{
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectCallProgress = CONTROL__DISABLE;
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectDTMFRegister = CONTROL__ENABLE;
	}

#if 0
	/* by default connect channel 0 to the handset */
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetInputToOutput = CONTROL__ENABLE;
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetInputSelect = CONTROL__ENABLE;
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetOutputSelect = CONTROL__ENABLE;
#endif
	Ac49xInitDriver(CONTROL__ENABLE);
	Ac49xResetDeviceBufferDescriptors(0);
	
	pTxPacketParams->Channel = 0;
	pTxPacketParams->Device = 0;
	
	Ac49xInitializePacketReceive(ACMWPacketCB); 
	Ac49xSetupDevice(pTxPacketParams->Device, &dsp_dev->SetupDeviceAttr);


	return AC494_DSP_OK_E;

} /* end of dspDev_setDefaults() */
/******************************************************************************/



/****************************************************************************
*
*	Name:		dspDev_init
*----------------------------------------------------------------------------
*	Abstract:	initialize the device (call with insmod)
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = 0, else -1
******************************************************************************/
int __init	dspDev_init(void)
{

	dsp_dev->numOfChannels = AC49X_NUMBER_OF_CHANNELS;
	dsp_dev->downloadStatus = 0;
	dsp_dev->initStatus = 0;

	dspDev_setDefaults();

	printk("<<<<<<<<<<<<<<<< INIT DSP DRV >>>>>>>>>>>>>>>>>>>>>>");

	return (0);

} /* end of dspDev_init() */
/******************************************************************************/

void active_test(void)
{
	printk("<<<<<<<<<<<<<<<< active_test >>>>>>>>>>>>>>>>>>>>>>\n");
	ActivateMediaSession((Tac49xTxPacketParams*)pTxPacketParams);
}

int phone_off_hook_action(void)
{
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel = pTxPacketParams->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
		
	Ac49xAdvancedChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].Advanced);
	Ac49xOpenChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel);
	//ActivateMediaSession((Tac49xTxPacketParams*)pTxPacketParams);
	//Ac49xActivateRegularRtpChannelConfiguration(device,channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp);
	return 0;
}

int phone_on_hook_action(void)
{
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel = pTxPacketParams->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
		
	//Ac49xAdvancedChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].Advanced);
	//Ac49xCloseChannelConfiguration(device, channel, &dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel);
	return 0;
}

int ActivateRegularRtp(void)
{
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel = pTxPacketParams->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
		
	Ac49xActivateRegularRtpChannelConfiguration(device,channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp);
	return 0;
}

int InActivateRegularRtp(void)
{
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel = pTxPacketParams->Channel;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
		
	//Ac49xCloseRegularRtpChannelConfiguration(device,channel, &dsp_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp);
	return 0;
}

void __exit dspDev_exit(void)
{
	devfs_unregister(devfs_dir_handle);
	kfree(dsp_dev);
}




module_init(dspDev_init);
module_exit(dspDev_exit);
