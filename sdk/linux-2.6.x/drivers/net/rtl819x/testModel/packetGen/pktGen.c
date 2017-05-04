/*
* Copyright c                  Realtek Semiconductor Corporation, 2002
* All rights reserved.
*
* Program : Code for packet generation
* Abstract :
* Author : 
* $Id: pktGen.c,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
*/

#include "protoForm.h"
#include "pktForm.h"
#include "utility.h"
#include "pktGen.h"

//#define _MSG

/*
 *  pktGen() - Generate a packet as you want
 *
 *  The returned value is the packet length.
 *  The length does not included L2 CRC, however, the L2 CRC is followed after the tail of packet.
 */
uint32 pktGen(rtl8651_PktConf_t *Pktptr, int8 *buff){
	rtl8651_PktConf_t pkt, *ptr = &pkt;
	uint32 pktLen;
	int8 dummy[6]={0};
	int8* PayloadPtr = dummy;
	uint32 l2Flag;

	// copy pkt conf to fake pkt
	memcpy(ptr, Pktptr, sizeof(rtl8651_PktConf_t));

	// End of test case
	if (ptr->pktType == _PKT_STOP){
	//	ptr->pktType=_PKT_TYPE_ETHER;
		return 0;
	}

	/*
			Ether-Header
	*/
	if (memcmp(ptr->conf_smac, PayloadPtr, 6) == 0){
		// default smac
		set_mac (ptr->conf_smac, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03);
	}
	if (memcmp(ptr->conf_dmac, PayloadPtr, 6) == 0){
		// default dmac (unicast)
		set_mac (ptr->conf_dmac, 0x00, 0x00, 0x00, 0x0a, 0x0b, 0x0c);
	}
	/*
			IP
	*/
	if (ptr->conf_sip == 0){
		// default sip
		ptr->conf_sip = 0xc0a80101;	// 192.168.1.1
	}
	if (ptr->conf_dip == 0){
		// default dip (unicast)
		ptr->conf_dip = 0x8c710101;	// 140.113.1.1
	}

	if (ptr->ip.version == 0){
		ptr->ip.version = 4;
	}
	if (ptr->ip.headerLen == 0){
		ptr->ip.headerLen = 20;
	}
	if (ptr->ip.ttl == 0){
		ptr->ip.ttl = 255;
	}

	/*
		Payload
	*/
	if ((ptr->payload.length == 0) || (ptr->payload.content == NULL)){
		pktLen = 0;
	}else{
		pktLen = ptr->payload.length;
		PayloadPtr = ptr->payload.content;
	}
	#ifdef _MSG
	rtlglue_printf("\t* Packet Payload Length\t\t[%d]\n", pktLen);
	#endif

	/*
		l2Flag
	*/
	l2Flag = ptr->l2Flag;
//	rtlglue_printf("the l2flag is  :[%d]\n", l2Flag);
//	rtlglue_printf("the pktType is: [%d]\n", ptr->pktType );
	// for 50B,(50 is also ok), when AUTOADD, we don't change ethertype
	#if 0
	if (ptr->conf_Flags & pkt_PKTHDR_PPPOE_AUTOADD){
		l2Flag &= ~L2_PPPoE;
	}
	if (ptr->conf_Flags & pkt_PKTHDR_VLAN_AUTOADD){
		l2Flag &= ~L2_VLAN;
	}
	#endif
//	rtlglue_printf("before switch prt->pktType\n");
	switch (ptr->pktType){
	//	rtlglue_printf("just in switch pktType\n");
		case _PKT_TYPE_ETHER:
			#ifdef _MSG
			rtlglue_printf("\t* Generate ETHER packet...");
			#endif
			protoForm_vsp_ether(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
						ptr->llc.dsap, ptr->llc.ssap,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->conf_ethtype,
						PayloadPtr, &pktLen);
			break;
		case _PKT_TYPE_IPX:
			#ifdef _MSG
			rtlglue_printf("\t* Generate IPX packet...");
			#endif
			protoForm_vsp_ipx(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
						ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
						//ptr->llc.dsap, ptr->llc.ssap,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ipx.type,
						ptr->ipx.dnet, ptr->ipx.dnode, ptr->ipx.dsock,
						ptr->ipx.snet, ptr->ipx.snode, ptr->ipx.ssock,
						PayloadPtr, &pktLen);
			break;
		case _PKT_TYPE_ARP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate ARP packet...");
			#endif
			protoForm_vs_arp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
                                                ptr->arp.mac_src, ptr->arp.ip_src,
                                                ptr->arp.mac_dst, ptr->arp.ip_dst,
                                                ptr->arp.op, &pktLen);
			break;
		case _PKT_TYPE_IP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate IP packet...");
			#endif
			protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, ptr->ip.protocol,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_PPTP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate PPTP packet...");
			#endif
			protoForm_vsp_pptp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->pptp.callid, ptr->pptp.seqno, ptr->pptp.ackno,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_ICMP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate ICMP packet...");
			#endif
			if (ptr->ip.offset){
				protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                	                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 1,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
			protoForm_vsp_icmp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->icmp.type, ptr->icmp.code, ptr->icmp.id, ptr->icmp.seq,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_IGMP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate IGMP packet...");
			#endif
			{
				int8 private_buff[2048];
				int8 *buffptr = private_buff;
				uint32 grlen = 0;

				if (ptr->igmp.ver == 3)
				{
					if ((ptr->igmp.type != IGMP_V3_MEMBERSHIP_REPORT) && (ptr->igmp.type != IGMP_MEMBERSHIP_QUERY))
						return 0;
				}

				if ((ptr->igmp.ver == 3) && (ptr->igmp.type == IGMP_V3_MEMBERSHIP_REPORT))
				{	// generate group record
					if ((grlen = grGen(ptr->igmp.grCfg, ptr->igmp.rnofg, private_buff)) == 0)
						return 0;	// group record generate error
						buffptr = (int8 *)((uint32)private_buff + grlen);
				}

				if (pktLen && PayloadPtr)
					memcpy(buffptr, PayloadPtr, pktLen);
				pktLen += grlen;

				protoForm_vsp_igmp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
	                                                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
							ptr->igmp.ver, ptr->igmp.type, ptr->igmp.respTime, ptr->igmp.gaddr,
							ptr->igmp.qrsq, ptr->igmp.qqic, ptr->igmp.qnofs, ptr->igmp.qslist, ptr->igmp.rnofg,
	                                        	private_buff, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_TCP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate TCP packet...");
			#endif
			if (ptr->ip.offset){
				protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                	                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 6,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
				if (ptr->conf_tcp_flag & TH_SYN)
				{
					int8 tcp_option[4];
					int8 *option_ptr = tcp_option;

					if (ptr->conf_tcp_mss == 0)
					{
						ptr->conf_tcp_mss = 1460;			// default mss
					}
					*((uint8*)option_ptr) = 2;			// kind
					*((uint8*)(option_ptr+1)) = 4;		// length
					*((uint16*)(option_ptr+2)) = htons(ptr->conf_tcp_mss);	//mss
					protoForm_vsp_tcp_option(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
							ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
							ptr->conf_sport, ptr->conf_dport,
							ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
							tcp_option, 4, PayloadPtr, ptr->ErrFlag, &pktLen);
				}
				else
				{
					protoForm_vsp_tcp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
							ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
							ptr->conf_sport, ptr->conf_dport,
							ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
							PayloadPtr, ptr->ErrFlag, &pktLen);
				}
			}
			break;
		case _PKT_TYPE_UDP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate UDP packet...");
			#endif
			if (ptr->ip.offset){
				protoForm_vsp_ip(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                	                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 17,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
			protoForm_vsp_udp(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_UDP_OPTION:
			#ifdef _MSG
			rtlglue_printf("\t* Generate UDP packet with option...");
			#endif
			if (ptr->ip.offset)
			{
				protoForm_vsp_ip_opt(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                	                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
							ptr->pppoe.type, ptr->pppoe.session,
							ptr->ip.version, ptr->ip.headerLen,
							ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
							ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip, 17,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			}else{
			protoForm_vsp_udp_opt(buff, l2Flag, ptr->conf_dmac, ptr->conf_smac,
                                                ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio,
						ptr->pppoe.type, ptr->pppoe.session,
						ptr->ip.version, ptr->ip.headerLen,
						ptr->ip.tos, ptr->ip.id, ptr->ip.diffserv, ptr->ip.mf, ptr->ip.offset,
						ptr->ip.ttl, ptr->conf_sip, ptr->conf_dip,
						ptr->conf_sport, ptr->conf_dport,
						PayloadPtr, ptr->ErrFlag, &pktLen);
			}
			break;
		case _PKT_TYPE_IPV6:
		//	rtlglue_printf("in IPV6");
			#ifdef _MSG
			rtlglue_printf("\t* Generate IPV6 packet!");
			#endif
			protoForm_vsp_ipv6(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio, 
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							PayloadPtr, ptr->ErrFlag, &pktLen);   
			break;
		case _PKT_TYPE_V6ICMP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate V6ICMP packet!");
			#endif
			protoForm_vsp_v6icmp(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio, 
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->icmp.type, ptr->icmp.code, ptr->icmp.id, ptr->icmp.seq,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break; 
		case _PKT_TYPE_V6TCP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate V6TCP packet!");
			#endif
			protoForm_vsp_v6tcp(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio, 
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->conf_sport, ptr->conf_dport,
							ptr->conf_tcp_seq, ptr->conf_tcp_ack, ptr->conf_tcp_flag, ptr->conf_tcp_win,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
		case _PKT_TYPE_V6UDP:
			#ifdef _MSG
			rtlglue_printf("\t* Generate V6UDP packet!");
			#endif
			protoForm_vsp_v6udp(buff, l2Flag, ptr->extHdr, ptr->conf_dmac, ptr->conf_smac, 
							ptr->vlan.vid, ptr->vlan.cfi, ptr->vlan.prio, 
							ptr->pppoe.type, ptr->pppoe.session, 
							ptr->ipv6.version, ptr->ipv6.priority, ptr->ipv6.flowlbl, ptr->ipv6.hoplmt, 
							ptr->conf_v6sip, ptr->conf_v6dip,
							ptr->rhdr.hdrlen, ptr->rhdr.segments_left, ptr->rhdr.dip,
							ptr->conf_sport, ptr->conf_dport,
							PayloadPtr, ptr->ErrFlag, &pktLen);
			break;
			
		default:
			rtlglue_printf("\t* Unknown pkt type[%d], Abort!\n", ptr->pktType);
	}

	/* for 50B,(50 is also ok), when AUTOADD, we don't change ethertype */
	#if 0
	if (ptr->conf_Flags & pkt_PKTHDR_PPPOE_AUTOADD){
		l2FormLayer3Type(l2Flag, buff, 0x8864);
	}
	#endif

	/* If packet is smaller than 60 bytes, add tailer padding. */
	if (pktLen < 60)
	{
#if 0 /* Padding has been done in l2FormCrc(). */
		memset(&buff[pktLen], 0x20/*Ethernet standard*/, (60 - pktLen));
#endif
		pktLen = 60;
	}

	#ifdef _MSG
	rtlglue_printf("\tSUCCESS!!\n");
	rtlglue_printf ("\t* Packet length\t\t\t[%d]\n", pktLen);
	#endif
	return pktLen;
}


uint32 grGen(rtl8651_IgmpGrConf_t *grCfg, uint32 grCnt, int8 *buff)
{
	rtl8651_IgmpGrConf_t *cfgptr;
	igmpGr_t* ptr;
	uint32 grLen = 0;
	uint32 cnt;

	if (!buff || !grCfg)
		goto out;

	ptr = (igmpGr_t*)buff;
	cfgptr = grCfg;

	for (cnt = 0 ; cnt < grCnt ; cnt ++, cfgptr ++)
	{	// generate
		uint32 i;
		uint32 *ss, *sd;
	
		ptr->igmp_gr_rt		= cfgptr->type;
		ptr->igmp_gr_auxlen	= cfgptr->auxlen;
		ptr->igmp_gr_nofs		= htons(cfgptr->nofs);
		ptr->igmp_gr_group	= htonl(cfgptr->gaddr);
		sd = &(ptr->src_list);
		ss = cfgptr->slist;
		for ( i = 0 ; i < cfgptr->nofs ; i ++)
		{
			*sd = htonl(*ss);
			sd ++;
			ss ++;
		}
		if (cfgptr->auxlen && cfgptr->auxdata)
			memcpy((void*)sd, cfgptr->auxdata, cfgptr->auxlen);

		grLen += (8 + (4 * cfgptr->nofs) + cfgptr->auxlen);
		ptr = (igmpGr_t *)((uint32)ptr + (uint32)(8 + (4 * cfgptr->nofs) + cfgptr->auxlen));
	}
out:
	return grLen;
}



