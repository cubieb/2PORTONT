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

//#define ACL_CID_TEST

//#include "CidSampleTbl.h"

typedef struct 
{
	unsigned int 					command;
	Tac49xRxPacketParams 		RxPacketParams;
} AC494_DSP_Receive_STCT;


/* define the driver structure holding the dsp device and channels status */
typedef struct dsp_dev_stc 
{
	devfs_handle_t 		handle;
	Tac49xSetupChannelAttr 	SetupChannelAttr[ACMW_MAX_NUM_CH]; 	/* the channels status */
	Tac49xSetupDeviceAttr	SetupDeviceAttr; 			/* the device status */
	int			downloadStatus;  			/* download flag, 0=no download */
	int			initStatus;                        	/* init flag, 0=no init */
	int			numOfChannels;
} DSP_DEV;


static devfs_handle_t devfs_dir_handle = NULL;
static DSP_DEV  ac_dsp_dev;
static DSP_DEV *dsp_dev = &ac_dsp_dev;



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
#define FXS_CHANNEL0 0
#define FXO_CHANNEL1 1
#define NUM_CHANNELS 2

extern Tac49xTxPacketParams TxPacketParams;
static Tac49xTxPacketParams *pTxPacketParams = &TxPacketParams;

#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF 			0x0	/* FOR DTMF BASED CID */
#define CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF 	0x1	/* FOR FSK BASED CID*/

Tac49xCallerIdAttr 		callerIdAttr;
Tac49xCallerIdAsciMessage 	callerIdAsciMessage;
Tac49xCallerIdAttr 		*pCallerIdAttr;
Tac49xCallerIdAsciMessage 	*pCallerIdAsciMessage;

static void ACMWPacketCB(void){printk(".");};

int play_idx_ivr =0;
static void fxs2fxo_detector(void)
{
	int channel, runtick;
	Tac49xIbsStringAttr IbsAttr  = {{0}};
	Tac49xPacket 		ac49xPacket = {{0}};
	AC494_DSP_Receive_STCT Receive;
	long ret, i, j, err_fsk=0;

	pcm_disableChan(0);
	pcm_disableChan(1);
	
	dsp_dev->numOfChannels = 2;
	
	/* get device defaults */
	Ac49xSetDefaultSetupDeviceAttr(&(dsp_dev->SetupDeviceAttr));

	dsp_dev->SetupDeviceAttr.Open.NumberOfChannels = dsp_dev->numOfChannels;


	/* set channels defaults */
	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++)
		Ac49xSetDefaultSetupChannelAttr(&(dsp_dev->SetupChannelAttr[channel]));

	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++) 
	{
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectCallProgress = CONTROL__ENABLE;
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectDTMFRegister = CONTROL__ENABLE;
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.IbsDetectionRedirection = IBS_DETECTION_REDIRECTION__ON_INPUT_STREAM_ENCODER_INPUT;

#ifdef ACL_CID_TEST	
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CallerIdOnHookService= CONTROL__ENABLE;
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CallerIdOffHookService= CONTROL__ENABLE;
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.CallerIdStandard= CALLER_ID_STANDARD__ETSI;
#endif
	}

	pTxPacketParams->Channel = 0;
	pTxPacketParams->Device = 0;
	
	Ac49xInitDriver(CONTROL__ENABLE);
	Ac49xResetDeviceBufferDescriptors(0);
	Ac49xInitializePacketReceive(ACMWPacketCB); 
	Ac49xSetupDevice(pTxPacketParams->Device, &dsp_dev->SetupDeviceAttr);
	Ac49xOpenDeviceConfiguration(pTxPacketParams->Device, &dsp_dev->SetupDeviceAttr);	
	Ac49xCallProgressDeviceConfiguration(pTxPacketParams->Device, &dsp_dev->SetupDeviceAttr);	
	
	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++) 
	{	
		
		PCM_restart(channel);
		Ac49xOpenChannelConfiguration(pTxPacketParams->Device, channel, &dsp_dev->SetupChannelAttr[channel]);
	}


	printk("<<<<<<<<<<<<<<<< FXS2FXO TST >>>>>>>>>>>>>>>>>>>>>>\n");
	
#ifndef ACL_CID_TEST	
	IbsAttr.GeneratorRedirection = IBS_STRING_GENERATOR_REDIRECTION__INTO_DECODER_OUTPUT;
	IbsAttr.NumberOfDigits = 2;
	IbsAttr.OnDuration = 4000;
	IbsAttr.OffDuration = 0;
	IbsAttr.PauseDuration = 0;		

	IbsAttr.Signal[0].Pause = CONTROL__DISABLE;
	IbsAttr.Signal[0].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15/*IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15*/;
	IbsAttr.Signal[0].Digit = 1;
	
	IbsAttr.Signal[1].Pause = CONTROL__DISABLE;
	IbsAttr.Signal[1].SystemCode = IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15 /*IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15*/;
	IbsAttr.Signal[1].Digit = 0;

	// send ibs on fxs channell
	Ac49xSendIbsString(pTxPacketParams->Device, FXS_CHANNEL0, &IbsAttr);

	runtick = IbsAttr.OnDuration*100;
#else
	pCallerIdAttr = &callerIdAttr;
	//CID STANDARD
	//CALLER_ID_STANDARD__TELCORDIA_BELLCORE, CALLER_ID_STANDARD__ETSI, CALLER_ID_STANDARD__NTT
	pCallerIdAttr->Standard 			= CALLER_ID_STANDARD__ETSI;
	//CID SERVICE TYPE
	//CALLER_ID_SERVICE_TYPE__1_ON_HOOK, CALLER_ID_SERVICE_TYPE__2_OFF_HOOK
	pCallerIdAttr->ServiceType 		= CALLER_ID_SERVICE_TYPE__1_ON_HOOK;
	//for ETSI TYPE1 
	//ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL, ETSI_ON_HOOK_METHOD__WITH_PRECEDING_DUAL_TONE_ALERT_SIGNAL
	pCallerIdAttr->EtsiOnhookMethod 	= ETSI_ON_HOOK_METHOD__NO_PRECEDING_DUAL_TONE_ALERT_SIGNAL;
	
	pCallerIdAttr->ChecksumByteAbsent = CONTROL__DISABLE;	//defualt Value for command
	pCallerIdAttr->Event 	= CALLER_ID_EVENT__NONE_EVENT;		//defualt Value for command

	///////////	SET CALLER ID MESSAGE //////////		
	pCallerIdAsciMessage = &callerIdAsciMessage;
	//DTMF FSK based CID
	//CALLER_ID_SEGMENTATION_MESSAGE_TYPE__DTMF, CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF
	pCallerIdAsciMessage->Title.Type.Value = (char)CALLER_ID_SEGMENTATION_MESSAGE_TYPE__NONE_DTMF;
	//CALLER ID MESSAGE LENGTH (in BYTE)
	pCallerIdAsciMessage->Title.Length.Value = (char)CID_MDMF_LEN;
	memcpy(&pCallerIdAsciMessage->Data[0], (char *)&_CID_MDMF1[0], pCallerIdAsciMessage->Title.Length.Value);

	//SEND CID API
	Ac49xSendCallerIdMessage(pTxPacketParams->Device, FXS_CHANNEL0, pCallerIdAttr, pCallerIdAsciMessage);

	runtick = 8000*100;
#endif


	for(i=0; i<(runtick); i++)
	{
		memset(&ac49xPacket, '\0', sizeof(ac49xPacket));
		Receive.RxPacketParams.pInPacket = (char *)&ac49xPacket;
		ret = Ac49xReceivePacket(&(Receive.RxPacketParams));

		if(ret>0)
		{
#ifndef ACL_CID_TEST		
			if( ac49xPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory == PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET )
			{
				if( ac49xPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode == STATUS_OR_EVENT_PACKET_OP_CODE__IBS_EVENT  )
				{
					long digit = ac49xPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit;
					long system = ac49xPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.System;	
					long channel = ac49xPacket.HpiHeader.Channel;

					if(channel==FXS_CHANNEL0)
						printk("\n Digit-%d, System-%d on FXS channel  <<<ERROR try me when Phone C off hook>>>\n",digit ,system);

					if(channel==FXO_CHANNEL1)
						printk("\n Digit-%d, System-%d on FXO channel  <<<OK>>>\n",digit ,system);
					
					if( (digit == 0) && (system== IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15))
						break;
				}
			}
#else
			if( ac49xPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory == PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET )
			{
				if( ac49xPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode == STATUS_OR_EVENT_PACKET_OP_CODE__CALLER_ID)
				{

					long channel = ac49xPacket.HpiHeader.Channel;

					Tac49xCallerIdPayload * pRxCallerId = &ac49xPacket.u.ProprietaryPayload.u.StatusOrEvent.u.CallerId;

					if(	(pRxCallerId->ChecksumByteAbsent == CONTROL__DISABLE) &&
						(pRxCallerId->Error == CONTROL__DISABLE) )
					{					
							char *msg = &pRxCallerId->Message[2];
							long msglen = pRxCallerId->Message[1];
							long nonedtmfcid = pRxCallerId->Message[0];
							long Event = pRxCallerId->Event;/* ==CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_VALID*/

							if(Event == CALLER_ID_EVENT__GENERATION_COMPLETED_SUCCESSFULLY)
								printk("cid send::\n");

							if(Event == CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_VALID)
								printk("cid recived ok::\n");

							printk("cid Standard = %d, nonedtmfcid=%d, Event=%d\n",pRxCallerId->Standard, nonedtmfcid, Event);

							for(j=0; j<msglen; j++)
							{
								printk("%c", msg[j]);
							}

								printk("\n FSK End \n");

							if(Event == CALLER_ID_EVENT__RECEIVE_PROCESS_COMPLETED_CHECKSUM_CRC_IS_VALID)
								break;
								
					}
					else
					{	
						printk("\n Error FSK End \n");					
					}
					
				}
					
			}
#endif
		}
		
	}
	printk("<<<<<<<<<<<<<<<< IBS FXS2FXO TST END >>>>>>>>>>>>>>>>>>>>>>\n");

} /* end of dspDev_setDefaults() */
/******************************************************************************/



/*****************************************************************************
* Function Name: cp3_proc_read
* Description  :
******************************************************************************
* Input:
* Output:
******************************************************************************/

int fxs2fxo_proc_read( char *buf,  char **start, off_t off,  int count,  int *eof,  void *data )
{
	fxs2fxo_detector();
	return -1;
}


/*****************************************************************************
* Function Name: cp3proc_init
* Description  :
******************************************************************************
* Input:
* Output:
******************************************************************************/
int __init fxs2fxo_init( void )
 {
#ifndef ACL_SIMPLE_KERNEL
	create_proc_read_entry( "fxs2fxo",0, NULL, fxs2fxo_proc_read,NULL );
#endif
	printk("\n<<<<<<<<<<	INIT FXS2FXO PROC	>>>>>>>>>\n");
	return  0;
 }

/*****************************************************************************
* Function Name: cp3proc_exit
* Description  :
******************************************************************************
* Input:
* Output:
******************************************************************************/
void __exit fxs2fxo_exit( void )
 {
#ifndef ACL_SIMPLE_KERNEL

	remove_proc_entry( "fxs2fxo", NULL );
#endif
 }


#ifndef ACL_SIMPLE_KERNEL
module_init(fxs2fxo_init);
module_exit(fxs2fxo_exit);
#endif

