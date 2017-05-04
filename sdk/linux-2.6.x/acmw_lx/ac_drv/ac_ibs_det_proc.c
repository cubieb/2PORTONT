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
#define CHANNEL 0
//Tac49xTxPacketParams TxPacketParams;
//Tac49xTxPacketParams *pTxPacketParams = &TxPacketParams;
extern Tac49xTxPacketParams *pTxPacketParams;

extern int ACMWPacketCB(char *buff,  U32 PacketSize);

static void ibs_detector(void)
{

	int channel;
	Tac49xIbsStringAttr IbsAttr  = {{0}};
	Tac49xPacket 		ac49xPacket = {{0}};
	AC494_DSP_Receive_STCT Receive;
	long ret, i;

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
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.IbsDetectionRedirection = IBS_DETECTION_REDIRECTION__ON_OUTPUT_STREAM_DECODER_OUTPUT;
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


	printk("<<<<<<<<<<<<<<<< IBS DET TST >>>>>>>>>>>>>>>>>>>>>>");
	
	
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

	Ac49xSendIbsString(pTxPacketParams->Device, CHANNEL, &IbsAttr);

	for(i=0; i<(IbsAttr.OnDuration*100); i++)
	{
		memset(&ac49xPacket, '\0', sizeof(ac49xPacket));
		Receive.RxPacketParams.pInPacket = (char *)&ac49xPacket;
		ret = Ac49xReceivePacket(&(Receive.RxPacketParams));

		if(ret>0)
		{
			if( ac49xPacket.u.ProprietaryPayload.ProprietaryHeader.Opcode == STATUS_OR_EVENT_PACKET_OP_CODE__IBS_EVENT  )
			{
				if( ac49xPacket.u.ProprietaryPayload.ProprietaryHeader.PacketCategory == PACKET_CATEGORY__COMMAND_OR_STATUS_PACKET )
				{
					//unsigned long saved_flags;					
					//saved_flags = Ac49xUserDef_DisableInterrupts();

					long digit = ac49xPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.Digit;
					long system = ac49xPacket.u.ProprietaryPayload.u.StatusOrEvent.u.IbsEvent.System;	

					printk("\n \n Digit-%d, System-%d \n \n",digit ,system);

					if( (digit == 0) && (system== IBS_SYSTEM_CODE__CALL_PROGRESS_0_TO_15))
						break;

					//Ac49xUserDef_RestoreInterrupts(saved_flags);
				}
			}
		}
		
	}

	printk("<<<<<<<<<<<<<<<< IBS DET TST END >>>>>>>>>>>>>>>>>>>>>>");


	return 0;

} /* end of dspDev_setDefaults() */
/******************************************************************************/



/*****************************************************************************
* Function Name: cp3_proc_read
* Description  :
******************************************************************************
* Input:
* Output:
******************************************************************************/

int cp3_proc_read( char *buf,  char **start, off_t off,  int count,  int *eof,  void *data )
{
	ibs_detector();
	return 0;
}


/*****************************************************************************
* Function Name: cp3proc_init
* Description  :
******************************************************************************
* Input:
* Output:
******************************************************************************/
int __init cp3proc_init( void )
 {
#ifndef ACL_SIMPLE_KERNEL
	create_proc_read_entry( "ibsdet",0, NULL, cp3_proc_read,NULL );
#endif
	printk("\n<<<<<<<<<<	INIT IBSDET PROC	>>>>>>>>>\n");
	return  0;
 }

/*****************************************************************************
* Function Name: cp3proc_exit
* Description  :
******************************************************************************
* Input:
* Output:
******************************************************************************/
void __exit cp3proc_exit( void )
 {
#ifndef ACL_SIMPLE_KERNEL

	remove_proc_entry( "ibsdet", NULL );
#endif
 }


#ifndef ACL_SIMPLE_KERNEL
module_init(cp3proc_init);
module_exit(cp3proc_exit);
#endif

