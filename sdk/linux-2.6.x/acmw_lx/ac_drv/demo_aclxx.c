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
#include "demo_aclxx.h"

/* for kmalloc */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
/** end for kmalloc */

#if (PACKET_RECORDING_ENABLE==1)
/*for udp packet recording */
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/route.h>
#include <asm/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h> /* for struct sk_buff */
/*end for udp packet recording */
#endif



/*********************** Local  Includes *************************************/

#include "userdef.h"
#include "si3210init.h"
#include "type.h"
#include "voip_params.h"
#include "voip_control.h"


static int aclxxDemo_setDefaults(void);
 



/* define the driver structure holding the dsp device and channels status */
typedef struct aclxx_demo_dev_stc {

	devfs_handle_t 			handle;
	Tac49xSetupChannelAttr 	SetupChannelAttr[ACMW_MAX_NUM_CH]; 	/* the channels status */
	Tac49xSetupDeviceAttr		SetupDeviceAttr; 					/* the device status */
	int						numOfChannels;

} ACLXX_DEMO_DEV;


static devfs_handle_t devfs_dir_handle 	= NULL;
static ACLXX_DEMO_DEV  *aclxx_demo_dev 				= NULL;
static int enable_routing = 0;


#if (PACKET_RECORDING_ENABLE==1)
/******************************************************************************
*
* 	Description:
*
******************************************************************************/
#define PACKET_RECORDING_SRC_IP		IP2LONG(10,16,2,65)
#define PACKET_RECORDING_DST_IP		IP2LONG(10,16,2,17)
#define PACKET_SRC_PORT		50000
#define PACKET_DST_PORT		50000
__ac49xdrv static void send_pkt_record(void *ptr_data, int data_len)
{
	unsigned char *tmp;
	int tos, rst;
	struct sk_buff *skb;
	struct rtable *rt = NULL;
	struct udphdr *udphdr;
	struct iphdr *iphdr;

	tos = 0;

	if((rst = ip_route_output(&rt, PACKET_RECORDING_DST_IP, PACKET_RECORDING_SRC_IP,(unsigned long)(RT_TOS(tos)), 0)))
		return;

	skb = alloc_skb(data_len+4+16 + 20 + 8, GFP_ATOMIC);
	if (skb == NULL){
		return ;
	}
	skb_reserve(skb,16);

	tmp = skb_put(skb, data_len + 20 + 8); 

	iphdr = (struct iphdr *)tmp;
	udphdr = (struct udphdr *)(tmp+20);

/* ip */
	iphdr->version 	= 4;
	iphdr->ihl 		= 5;
	iphdr->tos 		= tos;	
	iphdr->tot_len 	= htons(data_len + 8 + 20);
	iphdr->id 		= 0;
	iphdr->frag_off 	= 0;
	iphdr->ttl 		= 0x40;
	iphdr->protocol 	= 0x11;
	iphdr->check = 0;
	iphdr->saddr = PACKET_RECORDING_SRC_IP;
	iphdr->daddr = PACKET_RECORDING_DST_IP;
	iphdr->check = ip_fast_csum((unsigned char *)(iphdr), 5);


/* udp */
	udphdr->source 	= PACKET_SRC_PORT;
	udphdr->dest 	= PACKET_DST_PORT;
	udphdr->len 		= htons(data_len + 8);
	udphdr->check 	= 0;

/* analysis packet payload */	
	memcpy(tmp+28, ptr_data, data_len);

	skb->dst 	= dst_clone(&rt->u.dst);
	skb->dev 	= (skb->dst)->dev;
	skb->nh.iph 	= skb->data;
	skb->priority 	= 0xabc;

	NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, skb, NULL, rt->u.dst.dev, skb->dst->output);	
}


static  void PacketRecordingCB(int Device, char *pPacket, int PacketSize, Tac49xPacketDirection PacketDirection)
{
	static Tac49xAnalysisPacket ac49xAnalysisPacket;
	Tac49xAnalysisPacket 			*pac49xAnalysisPacket = &ac49xAnalysisPacket;		
	int ac49xAnalysisPacketLen =  sizeof(Tac49xAnalysisPacketHeader);
	Tac49xAnalysisPacketHeader		*pac49xAnalysisPacketHeader = &ac49xAnalysisPacket.Header;

	
	pac49xAnalysisPacketHeader->Device = Device;
	pac49xAnalysisPacketHeader->Direction = PacketDirection;
	pac49xAnalysisPacketHeader->Ver = 108;
	pac49xAnalysisPacketHeader->SubVersion = 9;

	memcpy(&pac49xAnalysisPacket->Payload, pPacket, PacketSize);
	ac49xAnalysisPacketLen += PacketSize;

	send_pkt_record((void *)pac49xAnalysisPacket, ac49xAnalysisPacketLen);



}
#endif /*PACKET_RECORDING_ENABLE*/

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
static int32 Ac49xTxPacketCB( uint32 channel, uint32 mid, void* packet, uint32 pktLen, uint32 flags )
{
	Tac49xTxPacketParams 	TxPacketParams;
	__attribute__ ((aligned (8))) static Tac49xPacket		ac49xPacket;	
	Tac49xTxPacketParams 	*pTxPacketParams = &TxPacketParams;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
	
	if ((packet == NULL) || pktLen <0)
		return -1;

	memcpy(ac49xPacket.u.Payload, (char *)packet, pktLen);

	pTxPacketParams->TransferMedium 			= TRANSFER_MEDIUM__HOST;
	pTxPacketParams->UdpChecksumIncluded 	= CONTROL__DISABLE;
	pTxPacketParams->FavoriteStream 			= CONTROL__DISABLE;
	pTxPacketParams->Device	 				= mid;
	pTxPacketParams->Channel 				= channel;
	pTxPacketParams->pOutPacket 				= (char *)&ac49xPacket;
	pTxPacketParams->PacketSize 				= pktLen+sizeof(Tac49xHpiPacketHeader);
	pTxPacketParams->Protocol = mid;

	
	Ac49xTransmitPacket(pTxPacketParams);

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

/******************************************************************************
*
* 	Description:
*
******************************************************************************/
#define VOIP_SRC_IP		IP2LONG(192, 168, 1, 159) //IP2LONG(10,16,2,65)
#define VOIP_DST_IP		IP2LONG(192, 168, 1, 158) //IP2LONG(10,16,2,68)
#define VOIP_SRC_DST_PORT		6000



static int ActivateMediaSession(Tac49xTxPacketParams	*pTxPacketParams)
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
	stat += rtk_trap_register(&voipMgrSession, channel, PROTOCOL__RTCP, s_id,  Ac49xTxPacketCB);
	printk("ok!\n");
	
#if (PACKET_RECORDING_ENABLE==1)
	Ac49xInitializePacketRecording(PacketRecordingCB);
#endif

 	Ac49xInitializePacketReceive(ACMWPacketCB);
	printk("1...\n");
	Ac49xSetupDevice(device, &aclxx_demo_dev->SetupDeviceAttr);
	printk("2...\n");
	Ac49xAdvancedChannelConfiguration(device, channel, &aclxx_demo_dev->SetupChannelAttr[channel].Advanced);
	printk("3...\n");
	Ac49xOpenChannelConfiguration(device, channel, &aclxx_demo_dev->SetupChannelAttr[channel].OpenOrUpdateChannel);
	printk("4...\n");
	PCM_restart(channel);
	printk("5...\n");
	Ac49xActivateRegularRtpChannelConfiguration(device,channel, &aclxx_demo_dev->SetupChannelAttr[channel].ActivateOrUpdateRtp);
	printk("6...\n");

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

	return 0;
}





/****************************************************************************
*
*	Name:		aclxxDemo_read
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
static ssize_t aclxxDemo_read( struct file * file, char * buf, size_t count, loff_t *ppos )
{
	return 0;

} /* end of aclxxDemo_read() */


/******************************************************************************/


/****************************************************************************
*
*	Name:		aclxxDemo_write
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
static ssize_t aclxxDemo_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	Tac49xTxPacketParams TxPacketParams;
	TxPacketParams.Channel = 0;
	TxPacketParams.Device = 0;

	if (enable_routing == 0)
	{
		printk("<<<<<<<<<<<<<<<< WRITE ACLXX DEMO->start >>>>>>>>>>>>>>>>>>>>>>\n");
		enable_routing = 1;
		ActivateMediaSession(&TxPacketParams);
	}

	return 0;

} /* end of aclxxDemo_write() */
/******************************************************************************/



/****************************************************************************
*
*	Name:		aclxxDemo_setDefaults
*----------------------------------------------------------------------------
*	Abstract:	set the dsp device and channels to defaults values
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = AC494_DSP_OK_E
******************************************************************************/

static int aclxxDemo_setDefaults(void)
{

	int channel;
	int device = 0;
	int status;

	/* get device defaults */
	Ac49xSetDefaultSetupDeviceAttr(&(aclxx_demo_dev->SetupDeviceAttr));

	aclxx_demo_dev->SetupDeviceAttr.Open.NumberOfChannels = aclxx_demo_dev->numOfChannels;

	/* set the channels over the device */
	for(channel=0; channel< (aclxx_demo_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++)
	{
		aclxx_demo_dev->SetupDeviceAttr.Open.Channel[channel].Slot = channel;
		aclxx_demo_dev->SetupDeviceAttr.Open.Channel[channel].InterconnectedSlot = channel+4;
	}

	/* set channels defaults */
	for(channel=0; channel< (aclxx_demo_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++)
		Ac49xSetDefaultSetupChannelAttr(&(aclxx_demo_dev->SetupChannelAttr[channel]));

	for(channel=0; channel< (aclxx_demo_dev->SetupDeviceAttr.Open.NumberOfChannels) ; channel++) {
		aclxx_demo_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectCallProgress		= CONTROL__DISABLE;
		aclxx_demo_dev->SetupChannelAttr[channel].OpenOrUpdateChannel.DetectDTMFRegister	= CONTROL__ENABLE;
	}

	Ac49xInitDriver(CONTROL__ENABLE);
	Ac49xResetDeviceBufferDescriptors(0);


	return AC494_DSP_OK_E;

} /* end of aclxxDemo_setDefaults() */
/******************************************************************************/





/*********************************************************************************************/

static int aclxxDemo_ioctl( 	struct inode 	*inode,
							struct file 	*file,
							unsigned int 	cmd,
							unsigned long 	arg )
{
	return 0;
}

/*********************************************************************************************/

static int aclxxDemo_open( 	struct inode 	*inode,
							struct file 	*file )
{
	return 0;
}

/*********************************************************************************************/

static int aclxxDemo_release( 	struct inode 	*inode,
							struct file 	*file )
{
	return 0;
}
/*********************************************************************************************/

int aclxxDemo_fasync(	int 		fd,
					struct file *file,
					int 		mode)
{
	return 0;
}

/*********************************************************************************************/

static int aclxxDemo_flush(struct file *file)
{
	return 0;
}

/*********************************************************************************************/


/* set the device operations to the functions pointers */
struct file_operations aclxxDemo_fops = {
	owner	: THIS_MODULE,
	read		: aclxxDemo_read,
	write	: aclxxDemo_write,
	ioctl		: aclxxDemo_ioctl,
	fasync	: aclxxDemo_fasync,
	flush		: aclxxDemo_flush,
	open	: aclxxDemo_open,
	release	: aclxxDemo_release
};


/****************************************************************************
*
*	Name:		aclxxDemo_init
*----------------------------------------------------------------------------
*	Abstract:	initialize the device & activate rtp channel
*----------------------------------------------------------------------------
*	Input:		none
*----------------------------------------------------------------------------
*  	Output:		none
*----------------------------------------------------------------------------
*	Returns: 	case of success = 0, else -1
******************************************************************************/
int __init	aclxxDemo_init(void)
{
	char devname[8];

	printk("<<<<<<<<<<<<<<<< INIT ACLXX DEMO >>>>>>>>>>>>>>>>>>>>>>\n");

	// allocate space to the device structure
	aclxx_demo_dev = kmalloc(sizeof(ACLXX_DEMO_DEV), GFP_KERNEL);

#if 0
	printk("reg chr...");
	int res = register_chrdev(ACLXX_DEMO_MAJOR, DEV_NAME, &aclxxDemo_fops);

	if(res < 0){
		printk("Can't register AC dsp devices.");
		return res;	
	}
	printk("ok!\n");
#else

	devfs_dir_handle = devfs_mk_dir(NULL, "dsp0", NULL);
	sprintf(devname, "dsp0");

	devfs_register_chrdev(ACLXX_DEMO_MAJOR, devname, &aclxxDemo_fops);
	aclxx_demo_dev->handle=devfs_register(devfs_dir_handle, devname, DEVFS_FL_NONE, ACLXX_DEMO_MAJOR, 0,
		S_IFCHR | S_IRUGO | S_IWUGO, &aclxxDemo_fops,NULL);

#endif
	aclxx_demo_dev->numOfChannels = 2;

	aclxxDemo_setDefaults();
	

	return (0);

} /* end of aclxxDemo_init() */
/******************************************************************************/



void __exit aclxxDemo_exit(void)
{
	devfs_unregister(devfs_dir_handle);
	kfree(aclxx_demo_dev);
}




module_init(aclxxDemo_init);
module_exit(aclxxDemo_exit);
/*********************** Local Variables ************************************/





