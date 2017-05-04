/*
 * Copyright (C) 2011 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 *
 * $Revision: 51194 $
 * $Date: 2014-09-10 16:36:56 +0800 (Wed, 10 Sep 2014) $
 *
 * Purpose : EPON kernel drivers
 *
 * Feature : This module install EPON kernel callbacks and other 
 * kernel functions
 *
 */

#ifdef CONFIG_EPON_FEATURE
/*
 * Include Files
 */
#include <linux/module.h> 
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <net/sock.h> 

/* For RTK APIs */
#include <common/rt_error.h>
#include <rtk/epon.h>
#include <rtk/port.h>
#include <rtk/switch.h>
#include <rtk/trap.h>

#include <pkt_redirect.h>

#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif


#define VTAG2VLANTCI(v) (( (((v) & 0xff00)>>8) | (((v) & 0x00ff)<<8) ) + 1) 
void skb_push_qtag(struct sk_buff *pSkb, unsigned short usVid, unsigned char ucPriority)
{
    if(usVid){
        /*push switch header*/
        skb_push(pSkb, 4); /*pSkb->data -= 4; pSkb->len += 4*/
        memmove(pSkb->data, 
                pSkb->data + 4, 
                (2 * 6 /* MAC_ADDR_LEN */));
        *(pSkb->data + (2 * 6 /* MAC_ADDR_LEN */)) = 0x81;
        *(pSkb->data + (2 * 6 /* MAC_ADDR_LEN */) + 1) = 0x00;
        *(pSkb->data + (2 * 6 /* MAC_ADDR_LEN */) + 2) = (unsigned char)(((usVid >> 8) & 0xF) | (ucPriority << 5));
        *(pSkb->data + (2 * 6 /* MAC_ADDR_LEN */) + 3) = usVid & 0xFF;
    }    
}

static unsigned int rtl960x_pon_port_mask;

void epon_oam_pkt_tx(unsigned short dataLen, unsigned char *data)
{
    unsigned char llidIdx;
    uint32 chipId, rev, subtype;
    struct tx_info txInfo;
    extern int re8686_tx_with_Info(unsigned char *pPayload, unsigned short length, void *pInfo);

    llidIdx = *((unsigned char *) (data + (dataLen - sizeof(unsigned char))));

    memset(&txInfo,0x0,sizeof(struct tx_info));

    /*keep format, switch do not modify packet format ex:vlan tag*/
    txInfo.opts1.bit.keep = 1;
    
    txInfo.opts2.bit.cputag = 1;
    txInfo.opts3.bit.tx_portmask = rtl960x_pon_port_mask;

    txInfo.opts2.bit.aspri = 1;
    txInfo.opts2.bit.cputag_pri = 7;

    /* Specified the output queue according to the LLId index */
    txInfo.opts1.bit.cputag_psel = 1;

    rtk_switch_version_get(&chipId, &rev, &subtype);
    
    switch(chipId)
    {    
        case RTL9601B_CHIP_ID:
            txInfo.opts3.bit.tx_dst_stream_id = 0x08 + (llidIdx * 16);
            break;

        case RTL9602C_CHIP_ID:
            switch(llidIdx)
            {
                case 0:
                    txInfo.opts3.bit.tx_dst_stream_id = 8;
                    break;
                case 1:
                    txInfo.opts3.bit.tx_dst_stream_id = 10;
                    break;
                case 2:
                    txInfo.opts3.bit.tx_dst_stream_id = 24;
                    break;
                case 3:
                    txInfo.opts3.bit.tx_dst_stream_id = 26;
                    break;
                case 4:
                    txInfo.opts3.bit.tx_dst_stream_id = 40;
                    break;
                case 5:
                    txInfo.opts3.bit.tx_dst_stream_id = 42;
                    break;
                case 6:
                    txInfo.opts3.bit.tx_dst_stream_id = 56;
                    break;
                case 7:
                    txInfo.opts3.bit.tx_dst_stream_id = 58;
                    break;
                        
            }
            
            break;
        case APOLLOMP_CHIP_ID:
        default:
            txInfo.opts3.bit.tx_dst_stream_id = 0x07;
            break;
        
    }    
    re8686_tx_with_Info(data, dataLen - sizeof(unsigned char), &txInfo);
}

void epon_oam_dyingGasp_tx(unsigned short dataLen, unsigned char *data)
{
    unsigned char llidIdx;
    uint32 chipId, rev, subtype;
    struct tx_info txInfo;
    extern int re8686_tx_with_Info(unsigned char *pPayload, unsigned short length, void *pInfo);

    llidIdx = *((unsigned char *) (data + (dataLen - sizeof(unsigned char))));

    memset(&txInfo,0x0,sizeof(struct tx_info));
    txInfo.opts1.bit.cputag_psel = 1;
    txInfo.opts2.bit.cputag = 1;

    rtk_switch_version_get(&chipId, &rev, &subtype);
 
    switch(chipId)
    {    
        case RTL9601B_CHIP_ID:
        case RTL9602C_CHIP_ID:
            txInfo.opts3.bit.tx_dst_stream_id = 0x1f;
            break;
        case APOLLOMP_CHIP_ID:
        default:
            txInfo.opts3.bit.tx_dst_stream_id = 0x7f;
            break;
    }   
    
    txInfo.opts3.bit.tx_portmask = rtl960x_pon_port_mask;
    re8686_tx_with_Info(data, dataLen - sizeof(unsigned char), &txInfo);
}


int epon_oam_pkt_rx(
    struct re_private *cp,
    struct sk_buff *skb,
    struct rx_info *pRxInfo)
{
    int ret;
    unsigned char chLlidIdx; 

    /* Filter only spa = PON and OAM frames */
    if((skb->data[12] == 0x88) && (skb->data[13] == 0x09) && (skb->data[14] == 0x03))
    {
        /* concate the LLID index to the end of the packet buffer */
        chLlidIdx = pRxInfo->opts2.bit.pon_stream_id & 0xf;
        skb_put(skb, sizeof(chLlidIdx));
        *((unsigned char *)(skb->data + (skb->len - sizeof(chLlidIdx)))) = chLlidIdx;
        ret = pkt_redirect_kernelApp_sendPkt(PR_USER_UID_EPONOAM, 1, skb->len, skb->data);
        if(ret)
        {
            printk("send to user app (%d) fail (%d)\n", PR_USER_UID_EPONOAM, ret);
        }
        /* Left the skb to be free by caller */
        return RE8670_RX_STOP;
    }

    return RE8670_RX_CONTINUE;
}

#if defined(CONFIG_RTK_OAM_V2)
int epon_muticast_pkt_rx(
    struct re_private *cp,
    struct sk_buff *skb,
    struct rx_info *pRxInfo)
{
    int ret;

    /* Filter only spa = All Ports and Muticast frames */
    if((skb->data[0] == 0x01) && (skb->data[1] == 0x00) && (skb->data[2] == 0x5e))
    {
    	skb->vlan_tci = (pRxInfo->opts2.bit.ctagva) ? VTAG2VLANTCI(pRxInfo->opts2.bit.cvlan_tag) : 0;
    	skb_push_qtag(skb, skb->vlan_tci & 0xfff /* VLAN_VID_MASK */, 0);
    	skb_push(skb, 3);    					
		skb->data[0] = pRxInfo->opts3.bit.src_port_num + 1;
		skb->data[1] = ((skb->vlan_tci & 0xfff /* VLAN_VID_MASK */) >> 8) & 0xF;
		skb->data[2] = (skb->vlan_tci & 0xfff /* VLAN_VID_MASK */) & 0xFF;
		//printk("%s %d 0x%x 0x%x 0x%x %d\n", __FUNCTION__, __LINE__, skb->data[0], skb->data[1], skb->data[2], skb->len);
        ret = pkt_redirect_kernelApp_sendPkt(PR_USER_UID_EPONOAM, 1, skb->len, skb->data);
        if(ret)
        {
            //printk("send to user app (%d) for muticast frames fail (%d)\n", PR_USER_UID_EPONOAM, ret);
        }
        /* Left the skb to be free by caller */
        return RE8670_RX_STOP;
    }

    return RE8670_RX_CONTINUE;
}
#elif defined(CONFIG_RTK_OAM_V1)
typedef struct ipv4hdr_s 
{
#if defined(_LITTLE_ENDIAN)
    uint8   ihl:4,
            version:4;
#else
    uint8   version:4,
            ihl:4;
#endif            
	uint8	typeOfService;			
	uint16	length;			/* total length */
	uint16	identification;	/* identification */
	uint16	offset;			
	uint8	ttl;			/* time to live */
	uint8	protocol;			
	uint16	checksum;			
	uint32 	sip;
	uint32 	dip;
}ipv4hdr_t;

typedef struct ipv6hdr_s
{
	uint32  vtf;            /*version(4bits),  traffic class(8bits), and flow label(20bits)*/
	uint16	payloadLenth;	/* payload length */
	uint8	nextHeader;		/* next header */
	uint8	hopLimit;		/* hop limit*/
	uint32  sip[4];	    	/*source address*/
	uint32  dip[4];			/* destination address */
} ipv6hdr_t;

typedef struct igmpv3_record_s
{
	uint8	type;				/* Record Type */
	uint8	auxLen;			    /* auxiliary data length, in uints of 32 bit words*/
	uint16	numOfSrc;			/* number of sources */
	uint32	groupAddr;			/* group address being reported */
	uint32	srcList[0];			/* first entry of source list */	
} igmpv3_record_t;

typedef struct igmpv3_report_s
{
	uint8	type;				/* Report Type */
	uint8  reserved1;             
	uint16 checkSum;			/*IGMP checksum*/
	uint16 reserved2;
	uint16	numOfRecords;		/* number of Group records */
	igmpv3_record_t recordList[0];  /*first entry of group record list*/
} igmpv3_report_t;


#ifndef IGMP_PROTOCOL
#define IGMP_PROTOCOL     2
#endif

#define IGMPV3_TYPE_MEMBERSHIP_REPORT 0x22

static int epon_is_igmp_packet(uint8 * data)
{
	uint8 *ptr;
	ipv4hdr_t *iph;
	
	ptr = data;
	ptr += 12;

	if ((ptr[0] == 0x88) && (ptr[1] == 0xA8))
	{
		ptr += 4;
	}
	if ((ptr[0] == 0x81) && (ptr[1] == 0x00))
	{
		ptr += 4;
	}
	if ((ptr[0] == 0x08) && (ptr[1] == 0x00)) /* ipv4 */
	{
		ptr += 2;
	}
	else
	{
		return 0;
	}

	iph = (ipv4hdr_t *)(ptr);
	if(iph->protocol == IGMP_PROTOCOL)
	{
		igmpv3_report_t *report;

		/*239.255.255.250 igmpv2 packet must return to protocol stack*/
		if(ntohl(iph->dip) == 0xEFFFFFFA)
		{
			return 0;
		}

		report = (igmpv3_report_t*)((char*)iph + (iph->ihl<<2));
		if(report->type == IGMPV3_TYPE_MEMBERSHIP_REPORT)
		{
			igmpv3_record_t *grec, *ptr;
			uint32 size1, size2;
			int i;
			grec = report->recordList;
			size1 = sizeof(igmpv3_record_t);
			
			for (i = 0; i < report->numOfRecords; i++)
			{
				/*239.255.255.250 igmpv3 packet must return to protocol stack*/
				if(ntohl(grec->groupAddr) == 0xEFFFFFFA)
				{
					return 0;
				}
				size2 = grec->numOfSrc * 4;
				ptr = (igmpv3_record_t*)((char*) grec + size1 + size2);
				grec = ptr;
			}
		}
		return 1;
	}
	return 0;
}

#define IPV6_HEADER_LENGTH 40
#define HOP_BY_HOP_OPTIONS_HEADER 	 0
#define ROUTING_HEADER 				43
#define FRAGMENT_HEADER 			44
#define DESTINATION_OPTION_HEADER 	60
#define NO_NEXT_HEADER 				59
#define ICMPV6_PROTOCOL    58

static int epon_is_mld_packet(uint8 * data)
{
	uint8 *ptr;
	ipv6hdr_t *iph6;
	uint8  nextHeader=0;
	uint8 * playloadPtr;
	uint16 extensionHdrLen = 0;
	int ret = 0;
	
	ptr = data;
	ptr += 12;

	if ((ptr[0] == 0x88) && (ptr[1] == 0xA8))
	{
		ptr += 4;
	}
	if ((ptr[0] == 0x81) && (ptr[1] == 0x00))
	{
		ptr += 4;
	}
	if ((ptr[0] == 0x86) && (ptr[1] == 0xDD)) /* ipv6 */
	{
		ptr += 2;
	}
	else
	{
		return 0;
	}

	iph6 = (ipv6hdr_t *)(ptr);
	nextHeader = iph6->nextHeader;
	ptr = ptr + IPV6_HEADER_LENGTH;
	playloadPtr = ptr;

	while(((ptr - playloadPtr) < ntohs(iph6->payloadLenth)) 
		&& (NO_NEXT_HEADER != nextHeader))
	{
		switch(nextHeader) 
		{
			case HOP_BY_HOP_OPTIONS_HEADER:
			case ROUTING_HEADER:
			case DESTINATION_OPTION_HEADER:
				nextHeader=ptr[0];
				extensionHdrLen=((uint16)(ptr[1])+1)*8;
				ptr=ptr+extensionHdrLen;
				break;
			case FRAGMENT_HEADER:
				nextHeader=ptr[0];
				ptr=ptr+8;
				break;
			case ICMPV6_PROTOCOL:
				nextHeader=NO_NEXT_HEADER;
				ret = 1;
				break;
			default:
				nextHeader=NO_NEXT_HEADER;
				break;
		}
	}
	return ret;
}

int epon_muticast_pkt_rx(
    struct re_private *cp,
    struct sk_buff *skb,
    struct rx_info *pRxInfo)
{
    /* Filter only spa = All Ports and Muticast frames */
    if((skb->data[0] == 0x01) && (skb->data[1] == 0x00) && (skb->data[2] == 0x5e))
    {
		if(0 == epon_is_igmp_packet(skb->data))
		{
			return RE8670_RX_CONTINUE;
		}

		skb->vlan_tci = (pRxInfo->opts2.bit.ctagva) ? VTAG2VLANTCI(pRxInfo->opts2.bit.cvlan_tag) : 0;
    	skb_push_qtag(skb, skb->vlan_tci & 0xfff /* VLAN_VID_MASK */, 0);
    	skb_push(skb, 4); /* make sure data 4 bytes align */   					
		skb->data[0] = pRxInfo->opts3.bit.src_port_num+1;
		skb->data[1] = ((skb->vlan_tci & 0xfff /* VLAN_VID_MASK */) >> 8) & 0xF;
		skb->data[2] = (skb->vlan_tci & 0xfff /* VLAN_VID_MASK */) & 0xFF;
		//printk("%s %d 0x%x 0x%x 0x%x %d\n", __FUNCTION__, __LINE__, skb->data[0], skb->data[1], skb->data[2], skb->len);
		
		pkt_redirect_kernelApp_sendPkt(PR_USER_UID_IGMPMLD, 1, skb->len, skb->data);     
        /* Left the skb to be free by caller */
        return RE8670_RX_STOP;
    }
	else if((skb->data[0] == 0x33) && (skb->data[1] == 0x33) && (skb->data[2] != 0xff))
	{
		if(0 == epon_is_mld_packet(skb->data))
		{
			return RE8670_RX_CONTINUE;
		}
		skb->vlan_tci = (pRxInfo->opts2.bit.ctagva) ? VTAG2VLANTCI(pRxInfo->opts2.bit.cvlan_tag) : 0;
    	skb_push_qtag(skb, skb->vlan_tci & 0xfff /* VLAN_VID_MASK */, 0);
    	skb_push(skb, 4); /* make sure data 4 bytes align */   					
		skb->data[0] = pRxInfo->opts3.bit.src_port_num+1;
		skb->data[1] = ((skb->vlan_tci & 0xfff /* VLAN_VID_MASK */) >> 8) & 0xF;
		skb->data[2] = (skb->vlan_tci & 0xfff /* VLAN_VID_MASK */) & 0xFF;
		//printk("%s %d 0x%x 0x%x 0x%x %d\n", __FUNCTION__, __LINE__, skb->data[0], skb->data[1], skb->data[2], skb->len);
		
		pkt_redirect_kernelApp_sendPkt(PR_USER_UID_IGMPMLD, 1, skb->len, skb->data);     
        /* Left the skb to be free by caller */
        return RE8670_RX_STOP;
	}
    return RE8670_RX_CONTINUE;
}

void epon_muticast_pkt_tx(unsigned short dataLen, unsigned char *data)
{
	unsigned char llidIdx;
    uint32 chipId, rev, subtype;
    struct tx_info txInfo;
	uint32 fwdPortMask;
	
    extern int re8686_tx_with_Info(unsigned char *pPayload, unsigned short length, void *pInfo);

    llidIdx = *((unsigned char *) (data + (dataLen - 2)));
	fwdPortMask = *((uint8*)(data + (dataLen - 1)));

	//printk("%s %d fwd[0x%x] llidIdx[%d],len[%d]\n", __FUNCTION__, __LINE__, fwdPortMask, llidIdx,dataLen);
    memset(&txInfo,0x0,sizeof(struct tx_info));

    /*keep format, switch do not modify packet format ex:vlan tag*/
    txInfo.opts1.bit.keep = 1;

	/*disable switch l2 learning for this packet*/
    txInfo.opts1.bit.dislrn = 1;
	
    txInfo.opts2.bit.cputag = 1;
    txInfo.opts3.bit.tx_portmask = fwdPortMask;

    txInfo.opts2.bit.aspri = 1;
    txInfo.opts2.bit.cputag_pri = 7;

    /* Specified the output queue according to the LLId index */
    txInfo.opts1.bit.cputag_psel = 1;

    rtk_switch_version_get(&chipId, &rev, &subtype);
    
    switch(chipId)
    {    
        case RTL9601B_CHIP_ID:
            txInfo.opts3.bit.tx_dst_stream_id = 0x08 + (llidIdx * 16);
            break;

        case RTL9602C_CHIP_ID:
            switch(llidIdx)
            {
                case 0:
                    txInfo.opts3.bit.tx_dst_stream_id = 8;
                    break;
                case 1:
                    txInfo.opts3.bit.tx_dst_stream_id = 10;
                    break;
                case 2:
                    txInfo.opts3.bit.tx_dst_stream_id = 24;
                    break;
                case 3:
                    txInfo.opts3.bit.tx_dst_stream_id = 26;
                    break;
                case 4:
                    txInfo.opts3.bit.tx_dst_stream_id = 40;
                    break;
                case 5:
                    txInfo.opts3.bit.tx_dst_stream_id = 42;
                    break;
                case 6:
                    txInfo.opts3.bit.tx_dst_stream_id = 56;
                    break;
                case 7:
                    txInfo.opts3.bit.tx_dst_stream_id = 58;
                    break;
                        
            }
            
            break;
        case APOLLOMP_CHIP_ID:
        default:
            txInfo.opts3.bit.tx_dst_stream_id = 0x07;
            break;
        
    }    
    re8686_tx_with_Info(data, dataLen - 2, &txInfo);
}
#endif

int __init epon_drv_init(void)
{
    extern int drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx);
    uint32 chipId, rev, subtype;
    rtk_portmask_t portMask;

    rtk_switch_portMask_Clear(&portMask);
    rtk_switch_port2PortMask_set(&portMask,RTK_PORT_PON);
    rtl960x_pon_port_mask = portMask.bits[0];
    
    rtk_switch_version_get(&chipId, &rev, &subtype);
    switch(chipId)
    {
    case APOLLOMP_CHIP_ID:
    case RTL9601B_CHIP_ID:
    case RTL9602C_CHIP_ID:
    default:
        break;
    }

    printk("Register EPON for pkt_redirect module\n");
    /* For packet redirect to user space protocol */
    pkt_redirect_kernelApp_reg(PR_KERNEL_UID_GMAC, epon_oam_pkt_tx);
    pkt_redirect_kernelApp_reg(PR_KERNEL_UID_EPONDYINGGASP, epon_oam_dyingGasp_tx);
    /* Hook on PON port only */
    #if defined(FPGA_DEFINED)
    drv_nic_register_rxhook(0xFF, RE8686_RXPRI_OAM, epon_oam_pkt_rx);
    #else
    drv_nic_register_rxhook(rtl960x_pon_port_mask, RE8686_RXPRI_OAM, epon_oam_pkt_rx);
    #endif

#ifndef CONFIG_EPON_OAM_DUMMY_MODE
#if !defined(CONFIG_RTK_L34_ENABLE)
	/* This feature should be mutually exclusive with RG */
	#if defined(CONFIG_RTK_OAM_V2)
    drv_nic_register_rxhook(0x0F, RE8686_RXPRI_MUTICAST, epon_muticast_pkt_rx);
	#endif
#endif

/* epon oam handle igmp even we init RG */
#if defined(CONFIG_RTK_OAM_V1)
	rtk_trap_igmpCtrlPkt2CpuEnable_set(ENABLED);
	rtk_trap_mldCtrlPkt2CpuEnable_set(ENABLED);
	drv_nic_register_rxhook(0x0F, RE8686_RXPRI_MUTICAST, epon_muticast_pkt_rx);
	pkt_redirect_kernelApp_reg(PR_KERNEL_UID_IGMPMLD, epon_muticast_pkt_tx);
#endif
#endif

    return 0;
}

void __exit epon_drv_exit(void)
{
    extern int drv_nic_unregister_rxhook(int portmask,int priority,p2rfunc_t rx);

    pkt_redirect_kernelApp_dereg(PR_KERNEL_UID_GMAC);
    pkt_redirect_kernelApp_dereg(PR_KERNEL_UID_EPONDYINGGASP);
    #if defined(FPGA_DEFINED)
    drv_nic_unregister_rxhook(0xFF, RE8686_RXPRI_OAM, epon_oam_pkt_rx);
    #else
    drv_nic_unregister_rxhook(rtl960x_pon_port_mask, RE8686_RXPRI_OAM, epon_oam_pkt_rx);
    #endif

#if !defined(CONFIG_RTK_L34_ENABLE)
	#if defined(CONFIG_RTK_OAM_V2)
    /* This feature should be mutually exclusive with RG */
    drv_nic_unregister_rxhook(0x0F, RE8686_RXPRI_MUTICAST, epon_muticast_pkt_rx);
	#endif
#endif
/* epon oam handle igmp even we init RG */
#if defined(CONFIG_RTK_OAM_V1)
	drv_nic_unregister_rxhook(0x0F, RE8686_RXPRI_MUTICAST, epon_muticast_pkt_rx);
	pkt_redirect_kernelApp_dereg(PR_KERNEL_UID_IGMPMLD);
#endif
}


MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RealTek EPON drive module");
MODULE_AUTHOR("Realtek");
module_init(epon_drv_init);
module_exit(epon_drv_exit);
#endif

