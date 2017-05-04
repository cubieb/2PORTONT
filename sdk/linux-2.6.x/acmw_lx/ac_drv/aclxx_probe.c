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


/*for udp packet recording */
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/route.h>
#include <asm/checksum.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h> /* for struct sk_buff */
/*end for udp packet recording */




/*********************** Local  Includes *************************************/

#include "userdef.h"
#include "si3210init.h"
#include "type.h"
#include "voip_params.h"
#include "voip_control.h"


/******************************************************************************
*
* 	Description:
*
******************************************************************************/
#define PACKET_RECORDING_SRC_IP		IP2LONG(10,16,2,65)
#define PACKET_RECORDING_DST_IP		IP2LONG(10,16,2,45)
#define PACKET_SRC_PORT		55500
#define PACKET_DST_PORT		55500
__ac49xdrv static void send_pkt_record_probe(void *ptr_data, int data_len)
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



void ACUdpProbe(unsigned long channel ,unsigned long opcode, char *pPacket, int PacketSize)
{
#if 0
	static unsigned long packet[256+2];

	if (opcode != 0x7)
		return;

	packet[0] = channel;
	packet[1] = opcode;	
	memcpy(&packet[2], pPacket, PacketSize);
		
	send_pkt_record_probe((void *)packet, PacketSize+8);
#endif	
}





