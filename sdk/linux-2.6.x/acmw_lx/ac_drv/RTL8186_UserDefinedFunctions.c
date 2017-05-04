/*****************************************************************************
*   Company:			Audiocodes Ltd.
*   File Name: 		RTL8186 Client API.c
******************************************************************************
*
* 	Description:
*
******************************************************************************/

/*********************** Extern Includes *************************************/


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

#include "RTL8186_UserDefinedFunctions.h"

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
#include "dsp_drv.h"
#include "userdef.h"
#include "si3210init.h"
#include "type.h"
#include "voip_params.h"
#include "voip_control.h"

/*********************** Local Variables ************************************/


__ac49xdrvdata static Trtl8186MediaRoute	_rtl8186MediaRoute[PCM_CH_NUM] ={{0}};



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


__ac49xdrv void Rtl8186_PacketRecordingCB(int Device, char *pPacket, int PacketSize, Tac49xPacketDirection PacketDirection)
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
__ac49xdrv int32 Rtl8186_Ac49xTxPacketCB( uint32 channel, uint32 mid, void* packet, uint32 pktLen, uint32 flags )
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
__ac49xdrv int Rtl8186_SetMediaSession(Tac49xTxPacketParams	*pTxPacketParams)
{
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	Trtl8186MediaRoute *pRtl8186MediaRoute;

//	printk("Rtl8186_SetMediaSession chid-%d,device-%d\n",channel,device);


	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;

	pRtl8186MediaRoute = &_rtl8186MediaRoute[channel];

	memset(pRtl8186MediaRoute, 0 ,sizeof(Trtl8186MediaRoute));
	memcpy(&pRtl8186MediaRoute->MediaRouting, pTxPacketParams->pOutPacket, sizeof(Tacmw_MediaRoutingConfigurationPayload));
	pRtl8186MediaRoute->ActiveMedia = FALSE;

	return 0;
}


/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_UnSetMediaSession(Tac49xTxPacketParams	*pTxPacketParams)
{
	Trtl8186MediaRoute *pRtl8186MediaRoute;
	unsigned int device 	= pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	unsigned int s_id, stat;

	//printk("Rtl8186_UnSetMediaSession chid-%d,device-%d\n",channel,device);

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;

	s_id = (2*channel+PROTOCOL__RTP-1);
	stat = rtk_trap_unregister(s_id);

	s_id = (2*channel+PROTOCOL__RTCP-1);
	stat += rtk_trap_unregister(s_id);

	if(!stat)
	{
		pRtl8186MediaRoute = &_rtl8186MediaRoute[channel];
		pRtl8186MediaRoute->ActiveMedia = FALSE;
	}

	return 0;
}


/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int ACMWPacketCB(char *buff,  U32 PacketSize)
{
	U16 channel;
	Tac49xPacket * pPacket = (Tac49xPacket *)buff;
	Trtl8186MediaRoute *pRtl8186MediaRoute;

	channel = pPacket->HpiHeader.Channel;

	switch(pPacket->HpiHeader.Protocol)
	{
		case PROTOCOL__RTP:
		 	Rtl8186_SendToNetwork(channel, &pPacket->u, PacketSize-sizeof(Tac49xHpiPacketHeader), PROTOCOL__RTP);
			break;

		case PROTOCOL__RTCP:
		 	Rtl8186_SendToNetwork(channel, &pPacket->u, PacketSize-sizeof(Tac49xHpiPacketHeader), PROTOCOL__RTCP);
		 	break;

		}
}

/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_ActivateMediaSession(Tac49xTxPacketParams	*pTxPacketParams)
{

	TstVoipMgrSession voipMgrSession;
	Trtl8186MediaRoute *pRtl8186MediaRoute;
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	unsigned int s_id, stat;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;

	if((pRtl8186MediaRoute = &_rtl8186MediaRoute[channel])== NULL)
		return -1;

	voipMgrSession.ch_id 			= channel;
	voipMgrSession.m_id 			= PROTOCOL__RTP;
	voipMgrSession.protocol 	  	= UDP_PROTOCOL;
	voipMgrSession.ip_dst_addr  	= MergeFieldsToLong(pRtl8186MediaRoute->MediaRouting.SourceIP); 	//ip of caller (ex: MP108)
	voipMgrSession.ip_src_addr  	= MergeFieldsToLong(pRtl8186MediaRoute->MediaRouting.MediaRoutingCfgPayload.DestinationIP); 	//ip of caller (ex: MP108)
	voipMgrSession.udp_src_port 	= MergeFieldsToShort(pRtl8186MediaRoute->MediaRouting.MediaRoutingCfgPayload.PortPair[PROTOCOL__RTP].DestinationPort);	 //port on caller (ex: for MP108 should be 4000)
	voipMgrSession.udp_dst_port 	= MergeFieldsToShort(pRtl8186MediaRoute->MediaRouting.MediaRoutingCfgPayload.PortPair[PROTOCOL__RTP].SourcePort); 		 //port on target

//	printk("RTP: Rtl8186_ActivateMediaSession chid-%d,device-%d, udp_src-%d, udp_dst-%d\n",channel,device,voipMgrSession.udp_src_port ,voipMgrSession.udp_dst_port );

	s_id = (2*channel+PROTOCOL__RTP-1);

	stat = rtk_trap_register(&voipMgrSession, channel, PROTOCOL__RTP, s_id,  Rtl8186_Ac49xTxPacketCB);

	voipMgrSession.ch_id 			= channel;
	voipMgrSession.m_id 			= PROTOCOL__RTCP;
	voipMgrSession.protocol 	  	= UDP_PROTOCOL;
	voipMgrSession.ip_dst_addr  	= MergeFieldsToLong(pRtl8186MediaRoute->MediaRouting.SourceIP); 	//ip of caller (ex: MP108)
	voipMgrSession.ip_src_addr  	= MergeFieldsToLong(pRtl8186MediaRoute->MediaRouting.MediaRoutingCfgPayload.DestinationIP); 	//ip of caller (ex: MP108)
	voipMgrSession.udp_src_port 	= MergeFieldsToShort(pRtl8186MediaRoute->MediaRouting.MediaRoutingCfgPayload.PortPair[PROTOCOL__RTCP].DestinationPort);// SourcePort);	 //port on caller (ex: for MP108 should be 4000)
	voipMgrSession.udp_dst_port 	= MergeFieldsToShort(pRtl8186MediaRoute->MediaRouting.MediaRoutingCfgPayload.PortPair[PROTOCOL__RTCP].SourcePort); //DestinationPort);	 //port on target
//	printk("RTCP: Rtl8186_ActivateMediaSession chid-%d,device-%d, udp_src-%d, udp_dst-%d\n",channel,device,voipMgrSession.udp_src_port ,voipMgrSession.udp_dst_port );

	s_id = (2*channel+PROTOCOL__RTCP-1);
	stat += rtk_trap_register(&voipMgrSession, channel, PROTOCOL__RTCP, s_id,  Rtl8186_Ac49xTxPacketCB);

	if(!stat)
		pRtl8186MediaRoute->ActiveMedia = TRUE;

	Ac49xInitializePacketReceive(ACMWPacketCB);

#if (PACKET_RECORDING_ENABLE==1)
	Ac49xInitializePacketRecording(Rtl8186_PacketRecordingCB);
#endif


	return 0;
}





/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_PcmReset(int device, int channel)/*(Tac49xTxPacketParams *pTxPacketParams)*/
{
#if 0
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	unsigned long flags;
  	//printk("Rtl8186_PcmReset chid-%d,device-%d\n",channel,device);
	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
#endif
	PCM_restart(channel);
	pcm_disableChan(channel);

	return 0;
}



/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_PcmEnable(int device, int channel)/*(Tac49xTxPacketParams *pTxPacketParams)*/
{
#if 0
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	unsigned long flags;
	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
#endif
	//printk("Rtl8186_PcmEnable chid-%d,device-%d\n",channel,device);

	PCM_restart(channel);
	return 0;
}



/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_PcmDisable(int device, int channel)
{
#if 0
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	unsigned long flags;
	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
	return -1;
#endif
	//printk("Rtl8186_PcmDisable, chid-%d,device-%d\n",channel,device);

	pcm_disableChan(channel);
	return 0;
}



/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_Ring(int device, int channel)/*(Tac49xTxPacketParams	*pTxPacketParams)*/
{
	static ring_struct ringer = {0};

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
	ringer.CH 		= channel;
	ringer.ring_set 	= 1;

//	Ring_FXS_Silicon(&ringer);

	return 0;
}

/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_RingOff(int device, int channel)/*(Tac49xTxPacketParams	*pTxPacketParams)*/
{
	static ring_struct ringer = {0};

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
	ringer.CH 		= channel;
	ringer.ring_set 	= 0;

//	Ring_FXS_Silicon(&ringer);

	return 0;
}

/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv int Rtl8186_HookStatus(int device, int channel)/*(Tac49xTxPacketParams	*pTxPacketParams)*/
{
	hook_struck hook;
#if 0
	unsigned int device = pTxPacketParams->Device;
	unsigned int channel 	= pTxPacketParams->Channel;
	//printk("Rtl8186_HookStatus: chid-%d,device-%d\n",channel,device);
	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;
#endif

	hook.CH 	= channel;
/*	//hook polling can cause annoying Tx Noise :: miki
	Hook_Polling(&hook);

	if (hook.hook_status)
		//printk("\n HOOK STATUS :: 	OFF	HOOK \n");
	else
		//printk("\n HOOK STATUS :: 	ON 	HOOK \n");

	return 0;
*/


}


/******************************************************************************
*
* 	Description:
*
******************************************************************************/
__ac49xdrv void Rtl8186_SendToNetwork(int channel, char *vpNITxBuff, S16 NetPayloadSize, Tac49xProtocol portProtocol)
{

	unsigned int 	 		ip_dst_addr;
	unsigned int 	 		ip_src_addr;
	unsigned short 		udp_dst_port;
	unsigned short 		udp_src_port;
	Trtl8186MediaRoute	*pMediaRoute;
	unsigned int 	 		sid;

	if ((channel < 0) || (channel >= ACMW_MAX_NUM_CH))
		return -1;

	if(!_rtl8186MediaRoute[channel].ActiveMedia)
		return -1;

	if ((vpNITxBuff != NULL) && (NetPayloadSize>0))
	{
		sid = (2*channel+portProtocol-1);
		system_process_rtp_tx(channel, sid, vpNITxBuff, NetPayloadSize);
	}


}


#if 0
/******************************************************************************
*
* 	Description:
*
******************************************************************************/
int  RTL8186_Ac49xCommandParser(Tac49xTxPacketParams *pTxPacketParams)
{
	Word16 channel;
	Word16 opcode;
	Word16 pcat;
	Word16 encoder;
	Word16 decoder;
	Word16 protocol;
	Tac49xPacket *pPacket;
	static int lastPageSizeMsec = 0;

	pPacket = (Tac49xPacket *) pTxPacketParams->pOutPacket;
	channel = pTxPacketParams->Channel;
	protocol = pTxPacketParams->Protocol;

	switch(protocol)
	{
		case PROTOCOL__PROPRIETARY:
			pcat 	= pPacket->u.ProprietaryPayload.ProprietaryHeader.PacketCategory;
			opcode 	= pPacket->u.ProprietaryPayload.ProprietaryHeader.Opcode;

			switch (pcat)
			{
				case PACKET_CATEGORY__CHANNEL_CONFIGURATION_PACKET:
					switch ( opcode )
					{
						case  CHANNEL_CONFIGURATION_OP_CODE__ACTIVATE_REGULAR_RTP:
						case  CHANNEL_CONFIGURATION_OP_CODE__UPDATE_REGULAR_RTP:

                                                 decoder =( (Tac49xActivateOrUpdateRtpChannelConfigurationPacket *) pPacket)->Decoder;
                                                 encoder =((Tac49xActivateOrUpdateRtpChannelConfigurationPacket *) pPacket)->Encoder;

							if ((((decoder == CODER__G711ALAW) && (encoder == CODER__G711ALAW)) ||
	 						     ((decoder == CODER__G711MULAW) && (encoder == CODER__G711MULAW)) ||
							     ((decoder == CODER__G729) && (encoder == CODER__G729)))&& (lastPageSizeMsec !=10))
							{

								//pcm_disableChan(channel);
								//pcm_set_page_size_msec(channel, 10);
								//PCM_restart(channel);
								lastPageSizeMsec = 10;
							}
 						       else if ((((decoder == CODER__G723HIGH) && (encoder == CODER__G723HIGH)) ||
	 						     ((decoder == CODER__G723LOW) && (encoder == CODER__G723LOW)))&& (lastPageSizeMsec !=30))
 						       {
								//pcm_disableChan(channel);
								//pcm_set_page_size_msec(channel, 30);
								//PCM_restart(channel);
								lastPageSizeMsec = 30;
							}
						break;
					}
				break;
			}
		break;
	}
}
#endif

void alaw2linear(void) {};
void linear2alaw(void) {};
void ulaw2linear(void) {};
void linear2ulaw(void) {};

