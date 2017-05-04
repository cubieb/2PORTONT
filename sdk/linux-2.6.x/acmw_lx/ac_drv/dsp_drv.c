/****************************************************************************
*
*	Company:			Audiocodes Ltd.
*
*   Project:			VoPP SoC
*
*   Hardware Module: 	AC494-EVM
*
*   File Name: 			dsp_drv.c
*
*
******************************************************************************
*
* 	DESCRIPTION:		This file wrap the AC49X driver to a char device
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
#include "dsp_drv.h"
#include "Ac49xDrv_Ethernet_Definitions.h"
#include "RTL8186_UserDefinedFunctions.h"
/*
#include "acltypedef.h"
#include "pcm_interface.h"
#include "si3210init.h"
#include "rtk_system.h"
*/
extern void Ac49xSetSegB(int segB);

int acdspDev_setDefaults(void);
extern void dspStart(void);

extern unsigned long  Ac49xUserDef_DisableInterrupts(void);
extern void Ac49xUserDef_RestoreInterrupts(unsigned long flags);


/* define the driver structure holding the dsp device and channels status */
typedef struct dsp_dev_stc {

	devfs_handle_t 			handle;
	Tac49xSetupChannelAttr 	SetupChannelAttr[ACMW_MAX_NUM_CH]; 	/* the channels status */
	Tac49xSetupDeviceAttr		SetupDeviceAttr; 					/* the device status */
	int						downloadStatus;  					/* download flag, 0=no download */
	int						initStatus;                        				/* init flag, 0=no init */
//	int 						dspCashSegment;   					/* derived from the dinamic allocation for the dsp memory */
	int						numOfChannels;
//	struct semaphore 		sem;

} DSP_DEV;


static devfs_handle_t devfs_dir_handle 	= NULL;
static DSP_DEV  *dsp_dev 				= NULL;

#if 0
static unsigned int 	baseAddr_acdspDev;
static int			size_acdspDev;

static unsigned int baseAddr_setupDevice;
static unsigned int baseAddr_setupChannels;
#endif


/****************************************************************************
*
*	Name:		acdspDev_read
*----------------------------------------------------------------------------
*	Abstract:	read routine from the device
*----------------------------------------------------------------------------
*	Input:		file 	- file decriptor
*				buf		- buf to fill
*				count	- size of buf
*				ppos	- flags
*----------------------------------------------------------------------------
*  	Output:		buf	- read data
*----------------------------------------------------------------------------
*	Returns: 	success = AC494_DSP_OK_E, error = AC494_DSP_ERROR_E,
*				in case of RECEIVE_CMD_E check Ac49xReceivePacket() return value
******************************************************************************/
static ssize_t acdspDev_read( struct file * file, char * buf, size_t count, loff_t *ppos )
{

	int 								ret = AC494_DSP_ERROR_E;
	unsigned int 						flag;
	AC494_DSP_SetupChannel_STCT		*pSetupChannel;
	AC494_DSP_SetupDevice_STCT		*pSetupDevice;
	AC494_DSP_Receive_STCT			*pReceive;
	AC494_DSP_Receive_STCT			Receive;
//	char 							buffer[AC494_DSP_RECEIVER_PACKET_SIZE];
	char 							*user_pInPacket;
	int								Channel = 0;
	Tac49xPacket 						ac49xPacket = {{0}};


	flag = *(unsigned int *)buf;	/* get the command flag and switch to the command */

	switch(flag) {

		/* get channel command - returns the channel status */
		case GET_CHANNEL_CMD_E:

			pSetupChannel = (AC494_DSP_SetupChannel_STCT *)buf;   /* cast the pointer */
			Channel = pSetupChannel->Channel;                     /* copy the channel data */

			if(copy_to_user(&(pSetupChannel->SetupChannelAttr),(char *)&(dsp_dev->SetupChannelAttr[Channel]),sizeof(Tac49xSetupChannelAttr)))
				ret = -EFAULT;
			else
				ret = AC494_DSP_OK_E;

		break;

		case GET_DEVICE_CMD_E:

			pSetupDevice = (AC494_DSP_SetupDevice_STCT *)buf;   	/* cast the pointer */
			if(copy_to_user(&(pSetupDevice->SetupDeviceAttr),(char *)&(dsp_dev->SetupDeviceAttr),sizeof(Tac49xSetupDeviceAttr)))
				ret = -EFAULT;
			else
				ret = AC494_DSP_OK_E;

		break;


		/* get receive command - read the buffers from the dsp */
		case RECEIVE_CMD_E:

			pReceive = (AC494_DSP_Receive_STCT *)buf;    		/* cast the pointer */
			if(copy_from_user(	&(Receive.RxPacketParams),      /* copy the receive structure to kernel */
								&(pReceive->RxPacketParams),
								sizeof(Tac49xRxPacketParams))) {
				ret = -EFAULT;
				break;
			}
			user_pInPacket = Receive.RxPacketParams.pInPacket; 			/* save the user buffer pointer */
			Receive.RxPacketParams.pInPacket = (U8 *) &ac49xPacket;         /* point to the local buffer */

			ret = Ac49xReceivePacket(&(Receive.RxPacketParams));	/* get the dsp buffer */

			/* copy the receive structure to user */
			if(copy_to_user(user_pInPacket, &ac49xPacket, Receive.RxPacketParams.PacketSize)) {
				ret = -EFAULT;
				break;
			}
			/* copy back to the user buffer */
			Receive.RxPacketParams.pInPacket = user_pInPacket;
			if(copy_to_user(&(pReceive->RxPacketParams), &(Receive.RxPacketParams), sizeof(Tac49xRxPacketParams))) {
				ret = -EFAULT;
				break;
			}
			ret = (ret==0) ? 1 : AC494_DSP_OK_E;
		break;

		default:

			ret = AC494_DSP_ERROR_E;
			break;

	}

	return ret;

} /* end of acdspDev_read() */




/******************************************************************************/


/****************************************************************************
*
*	Name:		acdspDev_write
*----------------------------------------------------------------------------
*	Abstract:	write routine to the device
*----------------------------------------------------------------------------
*	Input:		file 	- file decriptor
*				buf		- buf to fill
*				count	- size of buf
*				ppos	- flags
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of error = AC494_DSP_ERROR_E
******************************************************************************/
static ssize_t acdspDev_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{


	char 		*temp_buf;
	int 			order;
	int 			ret= AC494_DSP_ERROR_E;
	unsigned int 	flag;
 	int			Device = 0;
	int			Channel = 0;
	unsigned long saved_flags;

	/* the next structures use to wrap the AC49X routines */

	AC494_DSP_SetupDevice_STCT								*pSetupDevice;
	AC494_DSP_SetupChannel_STCT								*pSetupChannel;
//	AC494_DSP_CodecConfg_STCT								*pCodecConfg;
	AC494_DSP_AdvanceChannelConfg_STCT						*pAdvanceChannelConfg;
	AC494_DSP_OpenUpdateChannelConfg_STCT 					*pOpenChannelConfg;
	AC494_DSP_ActivateUpdateRegularRtpChannelConfg_STCT 		*pActivateRegularRtp;
	AC494_DSP_SendIbsString_STCT								sendIbsString;
	AC494_DSP_SendIbsStop_STCT								*pSendIbsStop;
	AC494_DSP_SendExtendedIbsString_STCT						sendExtendedIbsString;
	AC494_DSP_Transmit_STCT									TransmitPacket;
	AC494_DSP_Transmit_STCT									*pTransmitPacket;
	AC494_DSP_ActivateOrDeactivate3WayConf_STCT				conf3way;
	AC494_DSP_CloseChannel_STCT								closeChannel;
//	AC494_DSP_CODEC_DIR_STCT								codecDir;
//	AC494_DSP_AcousticEchoCancelerConfiguration_STCT			*pAchousticEchoCancelerConfg;
	AC494_DSP_DeviceReport_STCT 								*pDeviceReport;
	AC494_DSP_DeviceReport_STCT  							deviceReport;

//	AC494_MEM_MODIFY_STCT			mm;
//	AC494_MEM_DISPLAY_STCT			md;
//	AC494_MEM_CHECKSUM_STCT		mc;

	int temp;
	int size;


	unsigned int flags;

	flag = *(unsigned int *)buf; 	/* get the command flag and switch to the command */

	switch(flag) {

//EXTENDED CMD for RTL8186 :: add by miki

/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_HOOK_STATUS:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			Device 	= TransmitPacket.TxPacketParams.Device;
			Channel	= TransmitPacket.TxPacketParams.Channel;

			if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
				ret = -EFAULT;
				break;
			}
			//save_flags(flags); cli();
			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			Rtl8186_HookStatus(Device, Channel);
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;
			//restore_flags(flags);

			break;

/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_PCM_DISABLE:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			Device 	= TransmitPacket.TxPacketParams.Device;
			Channel	= TransmitPacket.TxPacketParams.Channel;

			if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
				ret = -EFAULT;
				break;
			}
			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			ret = Rtl8186_PcmDisable(Device, Channel);
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;


			break;

/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_PCM_ENABLE:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			Device 	= TransmitPacket.TxPacketParams.Device;
			Channel	= TransmitPacket.TxPacketParams.Channel;

			if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
				ret = -EFAULT;
				break;
			}
			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			ret = Rtl8186_PcmEnable(Device, Channel);
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;

			break;

/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_PCM_RESET:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			Device 	= TransmitPacket.TxPacketParams.Device;
			Channel	= TransmitPacket.TxPacketParams.Channel;

			if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
				ret = -EFAULT;
				break;
			}
			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			ret = Rtl8186_PcmReset(Device, Channel);
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;

			break;

/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_MEDIA_ACTIVATE:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			ret = Rtl8186_ActivateMediaSession(&(TransmitPacket.TxPacketParams));
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);

			break;

/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_DEVICE_POLL_TIME100MSEC:
			pDeviceReport = (AC494_DSP_DeviceReport_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&deviceReport,
							pDeviceReport,
							sizeof(AC494_DSP_DeviceReport_STCT))) {
				ret = -EFAULT;
				break;
			}

			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			Ac49xInitializeDevicePollingTime(deviceReport.pollTime100Msec);
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;

			break;
/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_MEDIA_CLOSE:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			ret = Rtl8186_UnSetMediaSession(&(TransmitPacket.TxPacketParams));
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;

			break;
/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_MEDIA_ROUTE:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			temp_buf = kmalloc(pTransmitPacket->TxPacketParams.PacketSize,GFP_KERNEL);

			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			if(copy_from_user(		temp_buf,
								pTransmitPacket->TxPacketParams.pOutPacket,
								pTransmitPacket->TxPacketParams.PacketSize)) {
				ret = -EFAULT;
			 	break;
            		}

			// points to the kernel buffer
			TransmitPacket.TxPacketParams.pOutPacket = temp_buf;

			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);;
			ret = Rtl8186_UnSetMediaSession(&(TransmitPacket.TxPacketParams));
			ret = Rtl8186_SetMediaSession(&(TransmitPacket.TxPacketParams));
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;

			kfree(temp_buf);

			break;

/////////////////////////////////////////////////////////////////////////////////////
		case RTL8186_RING_CHx:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			Device 	= TransmitPacket.TxPacketParams.Device;
			Channel	= TransmitPacket.TxPacketParams.Channel;

			if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
				ret = -EFAULT;
				break;
			}
			//save_flags(flags); cli();
			saved_flags = Ac49xUserDef_DisableInterrupts();//Ac49xUserDef_CriticalSection_EnterCommand(0);
		//	ret = Rtl8186_Ring(Device, Channel);
			Ac49xUserDef_RestoreInterrupts(saved_flags);//Ac49xUserDef_CriticalSection_LeaveCommand(0);;
			//restore_flags(flags);


			break;


////////////////////////////////////////////////////////////////////////////////////
		case 0xAC49:

			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			temp_buf = kmalloc(pTransmitPacket->TxPacketParams.PacketSize,GFP_KERNEL);

			// copy the user tx structure to kernel
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}

			if(copy_from_user(		temp_buf,
								pTransmitPacket->TxPacketParams.pOutPacket,
								pTransmitPacket->TxPacketParams.PacketSize)) {
				ret = -EFAULT;
			 	break;
            		}

			// points to the kernel buffer
			TransmitPacket.TxPacketParams.pOutPacket = temp_buf;

			//RTL8186_Ac49xCommandParser(&TransmitPacket.TxPacketParams);
			Ac49xTransmitPacket(&(TransmitPacket.TxPacketParams));

			kfree(temp_buf);
			ret = AC494_DSP_OK_E;

			break;


//END EXTENDED CMD for RTL8186 :: add by miki



#if 0

/////////////////////////////////////////////////////////////////////////////////////
/* by itzik - for testing */
		case 0x55555555:

			for(temp=0 ; temp < 2 ; temp++)
				printk("<1> got the number %d\r\n",(*(unsigned int *)(buf+4)));

			ret = AC494_DSP_OK_E;

			break;


		case 0xAAAAAAAA:

			if(down_interruptible(&(dsp_dev->sem))) {
        		ret =  -ERESTARTSYS;
				break;
			}

			for(temp=0 ; temp < 100 ; temp++)
				printk("<1> got the number %d\r\n",(*(unsigned int *)(buf+4)));

			up(&(dsp_dev->sem));

			ret = AC494_DSP_OK_E;

			break;

#endif

		/* download command - download a program to the dsp */
		case DOWNLOAD_CMD_E:
#if 0
			/* allocate a page for the program */
			order = (19 - PAGE_SHIFT > 0) ? 19 - PAGE_SHIFT : 0;
			temp_buf = (char *)__get_free_pages(GFP_KERNEL,order);

			/* copy the program */
			if(copy_from_user(temp_buf,buf + SIZE_OF_FLAG, count - SIZE_OF_FLAG))
				ret = -EFAULT;
			else  {
        		/* ret = Ac49xBoot(0, temp_buf, 0, 0);	*//* boot the program to the dsp */
        		ret = Ac49xUserDef_Boot(0, temp_buf, 0, 0);	/* boot the program to the dsp */
               	//dspStart();
			dsp_dev->numOfChannels = buf[26+4] - 48;
			}

			dsp_dev->downloadStatus = 1; 	/* update the download flag to true */

			free_pages((unsigned long)temp_buf,order); 	/* free the page */
#else
			dsp_dev->numOfChannels = ACMW_MAX_NUM_CH;
			dsp_dev->downloadStatus = 1;
			ret = PROGRAM_DOWNLOAD_STATUS__BOOT_STATUS_REGISTER_SUCCESS;
#endif
			break;

		/* reset buffers descriptor command */
		case RESET_BUF_DES_E:
#if 0
			/* check for init flag */
			if(!(dsp_dev->initStatus)) {
				ret = AC494_DSP_NO_INIT_E;
				break;
			}

			/* reset the buffer descriptors */
			Ac49xResetBufferDescriptors();
#endif
			ret = AC494_DSP_OK_E;

			break;

		/* transmit command  - write buffer to the dsp buffer */
		case TRANSMIT_CMD_E:
			/* check for init flag */
			if(!(dsp_dev->initStatus)) {
				ret = AC494_DSP_NO_INIT_E;
				break;
			}

			/* cast the pointer */
			pTransmitPacket = (AC494_DSP_Transmit_STCT *)buf;
			/* allocate kernel buffer */
			temp_buf = kmalloc(pTransmitPacket->TxPacketParams.PacketSize,GFP_KERNEL);
			/* copy the user tx structure to kernel */
			if(copy_from_user(	&(TransmitPacket.TxPacketParams),      /* copy the receive structure to kernel */
								&(pTransmitPacket->TxPacketParams),
								sizeof(Tac49xTxPacketParams))) {
				ret = -EFAULT;
				break;
			}
			/* copy the user buffer to kernel */
			if(copy_from_user(temp_buf, pTransmitPacket->TxPacketParams.pOutPacket, pTransmitPacket->TxPacketParams.PacketSize)) {
				ret = -EFAULT;
				break;
			}
			/* points to the kernel buffer */
			TransmitPacket.TxPacketParams.pOutPacket = temp_buf;
			ret = Ac49xTransmitPacket( &(TransmitPacket.TxPacketParams) );
			/* free the kernel buffer */
			kfree(temp_buf);

			break;

		/* setup device command */
		case SETUP_DEVICE_CMD_E:

			/* cast the pointer */
			pSetupDevice = (AC494_DSP_SetupDevice_STCT *)buf;

			/* copy the user structure to kernel */
			if(copy_from_user(	&(dsp_dev->SetupDeviceAttr),
								&(pSetupDevice->SetupDeviceAttr),
								sizeof(Tac49xSetupDeviceAttr))) {
				ret = -EFAULT;
				break;
			}
			/* setup the device */
				Device = pSetupDevice->Device;
				ret = Ac49xSetupDevice(Device, &(dsp_dev->SetupDeviceAttr));

				if(ret == AC494_DSP_OK_E)
					dsp_dev->initStatus = 1; 	/* update init flag to true */

#if 0
				if((*(volatile unsigned int *)(EMIF_4M_DSP_ALIGEND + 0x380000)) == 0)  {  /* (EMIF_4M_DSP_ALIGEND + 0x380000) = DSP start address at SDRAM */
				/* check that the firmware is ok (silicon version) */
				printk("<1>Incorrect silicon version\r\n");
				ret = -EFAULT;
				}
#endif
		break;

		case SET_DEVICE_CMD_E:
			/* cast the pointer */
			pSetupDevice = (AC494_DSP_SetupDevice_STCT *)buf;
			/* copy the user structure to kernel */
			if(copy_from_user(	&(dsp_dev->SetupDeviceAttr),
								&(pSetupDevice->SetupDeviceAttr),
								sizeof(Tac49xSetupDeviceAttr))) {
				ret = -EFAULT;
			}
			else
				ret = AC494_DSP_OK_E;

			break;


		case SET_CHANNEL_CMD_E:
	            /* copy the user structure to kernel */
			pSetupChannel	= (AC494_DSP_SetupChannel_STCT *)buf;
			Channel 			= pSetupChannel->Channel;
			size 				= sizeof(Tac49xSetupChannelAttr);

			if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
				ret = -EFAULT;
				break;
			}

			/* copy the user structure to kernel */
			if((temp = copy_from_user(		&(dsp_dev->SetupChannelAttr[Channel]),
										&(pSetupChannel->SetupChannelAttr),
										sizeof(Tac49xSetupChannelAttr)))) {
				ret = -EFAULT;
			}
			else
				ret = AC494_DSP_OK_E;

			break;


		case DEFAULTS_DSP_CMD_E:
			/* check for download flag */
			if(!(dsp_dev->downloadStatus)) {
				ret = AC494_DSP_NO_DOWNLOAD_E;
				break;
			}

			/* set to run mode and defaults values */
			ret = acdspDev_setDefaults();

		break;


		/* codec configuration command */
		case CODEC_CNFG_CMD_E:
#if 0
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}
			/* copy the user structure to kernel */
			pCodecConfg = (AC494_DSP_CodecConfg_STCT *)buf;
			Device = pCodecConfg->Device;
			Channel = pCodecConfg->Channel;
			if(copy_from_user(	&(dsp_dev->SetupChannelAttr[Channel].CodecConfiguration),
								&(pCodecConfg->Attr),
								sizeof(Tac49xCodecConfigurationAttr))) {
					ret = -EFAULT;
					break;
				}
				ret = Ac49xCodecConfiguration(Device, Channel, &(dsp_dev->SetupChannelAttr[Channel].CodecConfiguration));
#else
				ret = AC494_DSP_OK_E;
#endif
		break;

		/* advance channel configuration command */
		case ADVANCE_CHANNEL_CNFG_CMD_E:
#if 0
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}
#endif
				/* copy the user structure to kernel */
				pAdvanceChannelConfg = (AC494_DSP_AdvanceChannelConfg_STCT *)buf;
				Device 				= pAdvanceChannelConfg->Device;
				Channel 				= pAdvanceChannelConfg->Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				if(copy_from_user(	&(dsp_dev->SetupChannelAttr[Channel].Advanced),
									&(pAdvanceChannelConfg->Attr),
									sizeof(Tac49xAdvancedChannelConfigurationAttr))) {
					ret = -EFAULT;
					break;
				}

				/* set advance channel configuration */
				ret = Ac49xAdvancedChannelConfiguration(Device, Channel, &(dsp_dev->SetupChannelAttr[Channel].Advanced));

				break;

		case ECHO_CANCELER_CNFG_CMD_E:
#if 0
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				pAchousticEchoCancelerConfg = (AC494_DSP_AcousticEchoCancelerConfiguration_STCT *)buf;
				Device = pAchousticEchoCancelerConfg->Device;
				Channel = pAchousticEchoCancelerConfg->Channel;
				if(copy_from_user(	&(dsp_dev->SetupChannelAttr[Channel].AcousticEchoCancelerConfiguration),
								&(pAchousticEchoCancelerConfg->Attr),
								sizeof(Tac49xAcousticEchoCancelerConfigurationAttr))) {
					ret = -EFAULT;
					break;
				}

				/* set advance channel configuration */

				ret = Ac49xAcousticEchoCancelerConfiguration(Device, Channel, &(dsp_dev->SetupChannelAttr[Channel].AcousticEchoCancelerConfiguration));
#else
				ret = AC494_DSP_OK_E;
#endif
				break;

	        /* open channel command */
		case OPEN_CHANNEL_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				pOpenChannelConfg	= (AC494_DSP_OpenUpdateChannelConfg_STCT *)buf;
				Device 				= pOpenChannelConfg->Device;
				Channel				= pOpenChannelConfg->Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				/* copy the user structure to kernel */
				if(copy_from_user(	&(dsp_dev->SetupChannelAttr[Channel].OpenOrUpdateChannel),
									&(pOpenChannelConfg->Attr),
									sizeof(Tac49xOpenOrUpdateChannelConfigurationAttr))) {
					ret = -EFAULT;
					break;
				}

				/* set open channel configuration */
				ret = Ac49xOpenChannelConfiguration(Device, Channel, &(dsp_dev->SetupChannelAttr[Channel].OpenOrUpdateChannel));

				break;

	        /* update channel command */
		case UPDATE_CHANNEL_CMD_E:

				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				pOpenChannelConfg 	= (AC494_DSP_OpenUpdateChannelConfg_STCT *)buf;
				Device 				= pOpenChannelConfg->Device;
				Channel 				= pOpenChannelConfg->Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				if(copy_from_user(	&(dsp_dev->SetupChannelAttr[Channel].OpenOrUpdateChannel),
									&(pOpenChannelConfg->Attr),
									sizeof(Tac49xOpenOrUpdateChannelConfigurationAttr))) {
					ret = -EFAULT;
					break;
				}

				/* set update channel configuration */
				ret = Ac49xUpdateChannelConfiguration(Device, Channel, &(dsp_dev->SetupChannelAttr[Channel].OpenOrUpdateChannel));

				break;

		/* active regular channel command */
		case ACTIVE_REGULAR_CHANNEL_CMD_E:

				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				pActivateRegularRtp 	= (AC494_DSP_ActivateUpdateRegularRtpChannelConfg_STCT *)buf;
				Device 				= pActivateRegularRtp->Device;
				Channel 				= pActivateRegularRtp->Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				if(copy_from_user(	&(dsp_dev->SetupChannelAttr[Channel].ActivateOrUpdateRtp),
									&(pActivateRegularRtp->Attr),
									sizeof(Tac49xActivateOrUpdateRtpChannelConfigurationAttr))) {
					ret = -EFAULT;
					break;
				}

				/* set active regular channel configuration */
				ret = Ac49xActivateRegularRtpChannelConfiguration(Device, Channel, &(dsp_dev->SetupChannelAttr[Channel].ActivateOrUpdateRtp[0]));

				break;

		/* update regular channel command */
		case UPDATE_REGULAR_CHANNEL_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				pActivateRegularRtp 	= (AC494_DSP_ActivateUpdateRegularRtpChannelConfg_STCT *)buf;
				Device 				= pActivateRegularRtp->Device;
				Channel 				= pActivateRegularRtp->Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				if(copy_from_user(	&(dsp_dev->SetupChannelAttr[Channel].ActivateOrUpdateRtp),
									&(pActivateRegularRtp->Attr),
									sizeof(Tac49xActivateOrUpdateRtpChannelConfigurationAttr))) {
					ret = -EFAULT;
					break;
				}

				/* set update regular channel configuration */
				ret = Ac49xUpdateRegularRtpChannelConfiguration(Device, Channel, &(dsp_dev->SetupChannelAttr[Channel].ActivateOrUpdateRtp[0]));

				break;

		/* close regular channel command */
		case CLOSE_REGULAR_CHANNEL_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				if(copy_from_user(&closeChannel, buf, sizeof(AC494_DSP_CloseChannel_STCT))) {
					ret = -EFAULT;
					break;
				}

				Device 	= closeChannel.Device;
				Channel	= closeChannel.Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				ret = Ac49xCloseRegularRtpChannelConfiguration(Device, Channel, NULL);

				break;


		/* send ibs string command */
		case SEND_IBS_STRING_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				if(copy_from_user(&sendIbsString, buf, sizeof(AC494_DSP_SendIbsString_STCT))) {
					ret = -EFAULT;
					break;
				}

				Device 	= sendIbsString.Device;
				Channel	= sendIbsString.Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				/* send ibs string */
				ret = Ac49xSendIbsString(Device, Channel, &(sendIbsString.Attr));

				break;

		/* send ibs string command */
		case SEND_IBS_STOP_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}
				/* copy the user structure to kernel */
				if(copy_from_user(&sendIbsString, buf, sizeof(AC494_DSP_SendExtendedIbsString_STCT))) {
					ret = -EFAULT;
					break;
				}

				Device	= sendIbsString.Device;
				Channel	= sendIbsString.Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				ret = Ac49xSendIbsStop(Device, Channel);

				break;

		/* send ibs string command */
		case SEND_EXTENDED_IBS_STRING_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}
				/* copy the user structure to kernel */
				if(copy_from_user(&sendExtendedIbsString, buf, sizeof(AC494_DSP_SendExtendedIbsString_STCT))) {
					ret = -EFAULT;
					break;
				}

				Device 	= sendExtendedIbsString.Device;
				Channel	= sendExtendedIbsString.Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				/* send ibs string */
				ret = Ac49xSendExtendedIbsString(Device, Channel, &(sendExtendedIbsString.Attr));

				break;

		case SET_CODEC_CMD_E:
#if 0
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

	            /* copy the user structure to kernel */
				if(copy_from_user(&codecDir, buf, sizeof(AC494_DSP_CODEC_DIR_STCT))) {
					ret = -EFAULT;
	                break;
				}

				ret = AC494_DSP_OK_E;

				switch(codecDir.direction) {

					case AC494_CODEC_INPUT:

						switch(codecDir.user) {

							case AC494_CODEC_HANDSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HandsetInputSelect = CONTROL__ENABLE;
	        					break;
							case AC494_CODEC_HEADSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HeadsetInputSelect = CONTROL__ENABLE;
	        					break;
							case AC494_CODEC_MIC:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.MicrophoneInputSelect = CONTROL__ENABLE;
	        					break;
							case AC494_CODEC_LINE:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.LineInputSelect = CONTROL__ENABLE;
	        					break;
							default:
								ret = AC494_DSP_ERROR_E;
								break;
						}
						break;

					case AC494_CODEC_OUTPUT:

						switch(codecDir.user) {

							case AC494_CODEC_HANDSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HandsetOutputSelect = CONTROL__ENABLE;
	        					break;
							case AC494_CODEC_HEADSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HeadsetOutputSelect = CONTROL__ENABLE;
	        					break;
							case AC494_CODEC_SPEAKER:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.SpeakerSelect = CONTROL__ENABLE;
	        					break;
							case AC494_CODEC_LINE:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.LineOutputSelect = CONTROL__ENABLE;
	        					break;
							default:
								ret = AC494_DSP_ERROR_E;
								break;
						}
						break;

					case AC494_CODEC_LOOP_ON:

						switch(codecDir.user) {

							case AC494_CODEC_HANDSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HandsetInputToOutput = CONTROL__ENABLE;
	        					break;
							case AC494_CODEC_HEADSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HeadsetInputToOutput = CONTROL__ENABLE;
	        					break;
							default:
								ret = AC494_DSP_ERROR_E;
								break;
						}
						break;

					case AC494_CODEC_LOOP_OFF:

						switch(codecDir.user) {

							case AC494_CODEC_HANDSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HandsetInputToOutput = CONTROL__DISABLE;
	        					break;
							case AC494_CODEC_HEADSET:
								dsp_dev->SetupChannelAttr[codecDir.Channel].CodecConfiguration.HeadsetInputToOutput = CONTROL__DISABLE;
	        					break;
							default:
								ret = AC494_DSP_ERROR_E;
								break;
						}
						break;

					default:
						ret = AC494_DSP_ERROR_E;
						break;

				} /* end switch */
#else
				ret = AC494_DSP_OK_E;
#endif
			break;


		case SET_3WAYCONF_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				if(copy_from_user(&conf3way, buf, sizeof(AC494_DSP_ActivateOrDeactivate3WayConf_STCT))) {
					ret = -EFAULT;
					break;
				}

				Device 	= conf3way.Device;
				Channel	= conf3way.Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}
				/* send 3WayConference */
				ret = Ac49xActivateOrDeactivate3WayConferenceConfiguration(Device, Channel, &(conf3way.Attr));

				break;


		case CLOSE_CHANNEL_CMD_E:
				/* check for init flag */
				if(!(dsp_dev->initStatus)) {
					ret = AC494_DSP_NO_INIT_E;
					break;
				}

				/* copy the user structure to kernel */
				if(copy_from_user(&closeChannel, buf, sizeof(AC494_DSP_CloseChannel_STCT))) {
					ret = -EFAULT;
					break;
				}

				Device 	= closeChannel.Device;
				Channel	= closeChannel.Channel;

				if (Channel<0 || Channel>= ACMW_MAX_NUM_CH) {
					ret = -EFAULT;
					break;
				}

				ret = Ac49xCloseChannelConfiguration(Device, Channel);
				break;


		case MEM_MODIFY_CMD_E:
#if 0
			/* copy the user structure to kernel */
			if(copy_from_user(&mm, buf, sizeof(AC494_MEM_MODIFY_STCT))) {
				ret = -EFAULT;
				break;
			}

			while((mm.count--)>0) {
				*((unsigned int *)mm.addr) = mm.data;
				mm.addr+=4;
			}
#endif
			ret = AC494_DSP_OK_E;
			break;

		case MEM_DISPLAY_CMD_E:
#if 0
			/* copy the user structure to kernel */
			if(copy_from_user(&md, buf, sizeof(AC494_MEM_DISPLAY_STCT))) {
			ret = -EFAULT;
			break;
			}

			printk("<1>\r\n");
			while((md.count--)>0) {
			printk("<1> %x : %x \r\n",md.addr ,*(unsigned int *)md.addr);
			md.addr+=4;
			}
#endif
			ret = AC494_DSP_OK_E;

			break;

		case MEM_CHECKSUM_CMD_E:
#if 0
			/* copy the user structure to kernel */
			if(copy_from_user(&mc, buf, sizeof(AC494_MEM_CHECKSUM_STCT))) {
				ret = -EFAULT;
				break;
			}

			mc.checksum = 0;
			while((mc.count--)>0) {
				mc.checksum += *(unsigned int *)md.addr;
			}
			printk("<1>checksum = %x\r\n",mc.checksum);
#endif
			ret = AC494_DSP_OK_E;

			break;


		default:
			ret = AC494_DSP_ERROR_E;
			break;

	} 	/* end switch */

	return ret;

} /* end of acdspDev_write() */
/******************************************************************************/



/****************************************************************************
*
*	Name:		acdspDev_setDefaults
*----------------------------------------------------------------------------
*	Abstract:	set the dsp device and channels to defaults values
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = AC494_DSP_OK_E
******************************************************************************/

int acdspDev_setDefaults(void)
{

	int channel;
	int device = 0;
	int status;

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

	for(channel=0; channel< (dsp_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++) {
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectCallProgress		= CONTROL__DISABLE;
		dsp_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectDTMFRegister	= CONTROL__ENABLE;
	}

#if 0
	/* by default connect channel 0 to the handset */
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetInputToOutput	= CONTROL__ENABLE;
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetInputSelect	= CONTROL__ENABLE;
	dsp_dev->SetupChannelAttr[0].CodecConfiguration.HandsetOutputSelect	= CONTROL__ENABLE;
#endif
	Ac49xInitDriver(CONTROL__ENABLE);
	Ac49xResetDeviceBufferDescriptors(0);


	return AC494_DSP_OK_E;

} /* end of acdspDev_setDefaults() */
/******************************************************************************/





/*********************************************************************************************/

static int acdspDev_ioctl( 	struct inode 	*inode,
							struct file 	*file,
							unsigned int 	cmd,
							unsigned long 	arg )
{
	return 0;
}

/*********************************************************************************************/

static int acdspDev_open( 	struct inode 	*inode,
							struct file 	*file )
{
	return 0;
}

/*********************************************************************************************/

static int acdspDev_release( 	struct inode 	*inode,
							struct file 	*file )
{
	return 0;
}
/*********************************************************************************************/

int acdspDev_fasync(	int 		fd,
					struct file *file,
					int 		mode)
{
	return 0;
}

/*********************************************************************************************/

static int acdspDev_flush(struct file *file)
{
	return 0;
}

/*********************************************************************************************/


/* set the device operations to the functions pointers */
struct file_operations acdspDev_fops = {
	owner	: THIS_MODULE,
	read		: acdspDev_read,
	write	: acdspDev_write,
	ioctl		: acdspDev_ioctl,
	fasync	: acdspDev_fasync,
	flush		: acdspDev_flush,
	open	: acdspDev_open,
	release	: acdspDev_release
};


/****************************************************************************
*
*	Name:		acdspDev_init
*----------------------------------------------------------------------------
*	Abstract:	initialize the device (call with insmod)
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = 0, else -1
******************************************************************************/
int __init	acdspDev_init(void)
{

	char devname[10];

	// allocate space to the device structure
	dsp_dev = kmalloc(sizeof(DSP_DEV), GFP_KERNEL);
#if 0
	baseAddr_acdspDev = (unsigned int)dsp_dev;
	size_acdspDev		= (int)sizeof(DSP_DEV);

	baseAddr_setupDevice = (unsigned int)&(dsp_dev->SetupDeviceAttr);
	baseAddr_setupChannels = (unsigned int)&(dsp_dev->SetupChannelAttr);

    	dsp_dev->dspCashSegment = CASH_SEG_B;
	(*(volatile int* )(0xA5080014)) = dsp_dev->dspCashSegment;
	dsp_dev->dspCashSegment = (dsp_dev->dspCashSegment << 22) & 0xFFFC0000;

	printk("<1>dsp memory at block %x\r\n",dsp_dev->dspCashSegment);
#endif

	devfs_dir_handle = devfs_mk_dir(NULL, "dsp0", NULL);
	sprintf(devname, "dsp0");

//	dsp_dev->handle=devfs_register(devfs_dir_handle, devname, DEVFS_FL_AUTO_DEVNUM, 0, 0,
//		S_IFCHR | S_IRUGO | S_IWUGO, &acdspDev_fops,NULL);

	devfs_register_chrdev(DSP_MAJOR, devname, &acdspDev_fops);
	dsp_dev->handle=devfs_register(devfs_dir_handle, devname, DEVFS_FL_NONE, DSP_MAJOR, 0,
		S_IFCHR | S_IRUGO | S_IWUGO, &acdspDev_fops,NULL);

	dsp_dev->numOfChannels = AC49X_NUMBER_OF_CHANNELS;
	dsp_dev->downloadStatus = 0;
	dsp_dev->initStatus = 0;

//	sema_init(&(dsp_dev->sem),1);

	acdspDev_setDefaults();

	printk("<<<<<<<<<<<<<<<< INIT DSP DRV >>>>>>>>>>>>>>>>>>>>>>");

	return (0);

} /* end of acdspDev_init() */
/******************************************************************************/



void __exit acdspDev_exit(void)
{
	devfs_unregister(devfs_dir_handle);
	kfree(dsp_dev);
}




module_init(acdspDev_init);
module_exit(acdspDev_exit);
