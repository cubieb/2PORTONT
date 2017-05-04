/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : dump packet
* Abstract : dumpEther, dumpVlan, dumpLlc...
*
* Original Creator: Edward Jin-Ru Chen
* $Name:  $
* $Revision: 1.1 $
* $Id: pktDump.c,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
* $Log: pktDump.c,v $
* Revision 1.1  2012/10/24 04:18:57  ikevin362
* initial version
*
* Revision 1.3  2005/09/21 02:33:02  yjlou
* *: re-order variables position to make ANSI C compile pass.
*
* Revision 1.2  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
*
* Revision 1.1  2004/02/25 14:27:24  chhuang
* *** empty log message ***
*
* Revision 1.3  2004/02/18 13:12:21  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.10  2003/10/06 10:16:24  waynelee
* add pptp gre support
*
* Revision 1.9  2002/10/02 15:12:32  waynelee
* ipx support debug
*
* Revision 1.8  2002/10/01 07:47:34  waynelee
* support user-defined ipid, df, mf, ipoff
*
* Revision 1.7  2002/09/06 04:30:02  waynelee
* support and dump RARP packets
*
* Revision 1.6  2002/09/02 08:51:40  waynelee
* dump more arp info (like: sender mac,ip and target mac,ip)
*
* Revision 1.5  2002/08/28 09:54:35  waynelee
* add: print igmp group address
*
* Revision 1.4  2002/08/23 14:17:07  waynelee
* modify display message (da, sa)
*
* Revision 1.3  2002/08/22 09:43:17  waynelee
* debug: ntohs(tcp->th_ack) -> ntohl(tcp->th_ack)
*
* Revision 1.2  2002/08/21 08:59:46  waynelee
* debug: udp sport and dport display mismatch
*
*
*/

#include <net/rtl/rtl_types.h>
#include "pktDump.h"
#include "utility.h"
#include "flagDef.h"
#include "pktParse.h"



void dumpEther(_etherHdr_t * ether) {

	if(ether == NULL)
		return;

	rtlglue_printf("DMAC: ");
	rtlglue_printf("%02x-%02x-%02x-%02x-%02x-%02x ",ether->da[0],ether->da[1],ether->da[2],ether->da[3],ether->da[4],ether->da[5]);
	rtlglue_printf("SMAC: ");
	rtlglue_printf("%02x-%02x-%02x-%02x-%02x-%02x ",ether->sa[0],ether->sa[1],ether->sa[2],ether->sa[3],ether->sa[4],ether->sa[5]);
	if(ntohs(ether->etherType)<0x0600)
		rtlglue_printf("Length: %04x (%u bytes)\n", ntohs(ether->etherType), ntohs(ether->etherType));
	else {
		rtlglue_printf("EtherType: %04x ", ntohs(ether->etherType));
		switch(ntohs(ether->etherType)) {
		case 0x8137:
			rtlglue_printf("(IPX)\n");
			break;
		case 0x0800:
			rtlglue_printf("(IP)\n");
			break;
		case 0x0806:
			rtlglue_printf("(ARP)\n");
			break;
		case 0x8863:
			rtlglue_printf("(PPPoE Discovery)\n");
			break;
		case 0x8864:
			rtlglue_printf("(PPPoE Session)\n");
			break;
		case 0x8035:
			rtlglue_printf("(RARP)\n");
			break;
		case 0x8100:
			rtlglue_printf("(VLAN)\n");
			break;
		default:
			rtlglue_printf("\n");
			break;
		}
	}
}

void dumpVlan(_vlanHdr_t * vlan) {
	if(vlan==NULL)
		return;
	rtlglue_printf("Vlan priority %d CFI:%d VID:%d\n",
		(ntohs(vlan->tag)>>13)&0x7, (ntohs(vlan->tag)>>12)&0x1,
		ntohs(vlan->tag)&0xfff);
	if(ntohs(vlan->etherType)<0x0600)
		rtlglue_printf("Length: %04x (%u bytes)\n", ntohs(vlan->etherType), ntohs(vlan->etherType));
	else {
		rtlglue_printf("EtherType: %04x ", ntohs(vlan->etherType));
		switch(ntohs(vlan->etherType)) {
		case 0x0800:
			rtlglue_printf("(IP)\n");
			break;
		case 0x0806:
			rtlglue_printf("(ARP)\n");
			break;
		case 0x8863:
			rtlglue_printf("(PPPoE Discovery)\n");
			break;
		case 0x8864:
			rtlglue_printf("(PPPoE Session)\n");
			break;
		case 0x8035:
			rtlglue_printf("(RARP)\n");
			break;
		case 0x8100:
			rtlglue_printf("(VLAN)\n");
			break;
		default:
			rtlglue_printf("\n");
			break;
		}
	}
}

void dumpLlc(_llcHdr_t * llc) {
	uint32 i;

	if(llc == NULL)
		return;
	rtlglue_printf("DSAP:%02x SSAP:%02x CTL:%x\n",llc->llc_dsap, llc->llc_ssap, llc->ctrl);
	rtlglue_printf("Org Code: ");
	for(i=0;i<3; i++)
		rtlglue_printf("%02x:",llc->org_code[i]);
	rtlglue_printf(" EtherType: %04x ", ntohs(llc->etherType));
	switch(ntohs(llc->etherType)) {
	case 0x0800:
		rtlglue_printf("(IP)\n");
		break;
	case 0x0806:
		rtlglue_printf("(ARP)\n");
		break;
	case 0x8863:
		rtlglue_printf("(PPPoE Discovery)\n");
		break;
	case 0x8864:
		rtlglue_printf("(PPPoE Session)\n");
		break;
	case 0x8035:
		rtlglue_printf("(RARP)\n");
		break;
	case 0x8100:
		rtlglue_printf("(VLAN)\n");
		break;
	default:
		rtlglue_printf("\n");
		break;
	}
}

void dumpPppoe(_pppoeHdr_t * pppoe) {
	if(pppoe == NULL)
		return;

	rtlglue_printf("Ver: %x Type: %x Code:%x\n",pppoe->ver, pppoe->type, pppoe->code);
	rtlglue_printf("Session:%x Length:%x PPP Protocol:%04x ",
			ntohs(pppoe->sessionId), ntohs(pppoe->length), ntohs(pppoe->proto));
	switch(ntohs(pppoe->proto)) {
	case 0x0021:
		rtlglue_printf("(IP)\n");
		break;
	default:
		rtlglue_printf("\n");
		break;
	}
}
void dumpIpx(ipxHdr_t * ipx) {

	if(ipx == NULL)
		return;
	rtlglue_printf("IPX Checksum:0x%04x, Length:%d, TCtrl:0x%02x, Type:0x%02x\n",
		ntohs(ipx->ipx_sum), ntohs(ipx->ipx_len), ipx->ipx_tctrl, ipx->ipx_type);

	rtlglue_printf("dest net:%d, node:%02x-%02x-%02x-%02x-%02x-%02x, sock: 0x%04x\n",
		ntohl(ipx->ipx_dnet),ipx->ipx_dnode[0],ipx->ipx_dnode[1],ipx->ipx_dnode[2],ipx->ipx_dnode[3],ipx->ipx_dnode[4],ipx->ipx_dnode[5],
		ntohs(ipx->ipx_dsock));

	rtlglue_printf("sour net:%d, node:%02x-%02x-%02x-%02x-%02x-%02x, sock: 0x%04x\n",
		ntohl(ipx->ipx_snet),ipx->ipx_snode[0],ipx->ipx_snode[1],ipx->ipx_snode[2],ipx->ipx_snode[3],ipx->ipx_snode[4],ipx->ipx_snode[5],
		ntohs(ipx->ipx_ssock));

	rtlglue_printf("\n");
}

void dumpArp(arpHdr_t * arp) {

	if(arp == NULL)
		return;
	rtlglue_printf("Hardware:%x Protocol:%x HardwareLen:%x ProtocolLen:%x ",
		ntohs(arp->ar_hrd), ntohs(arp->ar_pro), arp->ar_hln, arp->ar_pln);
	switch(ntohs(arp->ar_op)) {
	case ARPOP_REQUEST:
		rtlglue_printf("ARP request\n");
		break;
	case ARPOP_REPLY:
		rtlglue_printf("ARP reply\n");
		break;
	case ARPOP_REVREQUEST:
		rtlglue_printf("RARP Request\n");
		break;
	case ARPOP_REVREPLY:
		rtlglue_printf("RARP Reply\n");
		break;
	}
	rtlglue_printf("Sender MAC: ");
	rtlglue_printf("%02x-%02x-%02x-%02x-%02x-%02x ",arp->ar_sha[0],arp->ar_sha[1],arp->ar_sha[2],arp->ar_sha[3],arp->ar_sha[4],arp->ar_sha[5]);
	rtlglue_printf("Sender IP: ");
	rtlglue_printf("%d.%d.%d.%d",arp->ar_spa[0],arp->ar_spa[1],arp->ar_spa[2],arp->ar_spa[3]);
	rtlglue_printf("\n");

	rtlglue_printf("Target MAC: ");
	rtlglue_printf("%02x-%02x-%02x-%02x-%02x-%02x ",arp->ar_tha[0],arp->ar_tha[1],arp->ar_tha[2],arp->ar_tha[3],arp->ar_tha[4],arp->ar_tha[5]);
	rtlglue_printf("Target IP: ");
	rtlglue_printf("%d.%d.%d.%d",arp->ar_tpa[0],arp->ar_tpa[1],arp->ar_tpa[2],arp->ar_tpa[3]);
	rtlglue_printf("\n");
}

void dumpIp(ipHdr_t * ip, uint32 l3Flag, uint16 l3Cksum) {
	struct in_addr ina;

	if(ip == NULL)
		return;
	rtlglue_printf("Ver:%d Header Len:%d TOS:%02x Total Length:%d\n",
		ip->ip_v, ip->ip_hl*4, ip->ip_tos, ntohs(ip->ip_len));
	rtlglue_printf("ID:0x%x flag: ", ntohs(ip->ip_id));
	if(ntohs(ip->ip_off) & IP_DF)
		rtlglue_printf("Don't fragment ");
	if(ntohs(ip->ip_off) & IP_MF)
		rtlglue_printf("More fragment ");
	rtlglue_printf(" Offset: %d bytes\n",(uint16)(ntohs(ip->ip_off)&0x1fff)*8);
	rtlglue_printf("TTL:%d Layer4 Protocol:",ip->ip_ttl);
	switch(ip->ip_p) {
	case IP_PROTO_PPTP:
		rtlglue_printf("PPTP ");
		break;
	case IP_PROTO_ICMP:
		rtlglue_printf("ICMP ");
		break;
	case IP_PROTO_IGMP:
		rtlglue_printf("IGMP ");
		break;
	case IP_PROTO_TCP:
		rtlglue_printf("TCP ");
		break;
	case IP_PROTO_UDP:
		rtlglue_printf("UDP ");
		break;
	default:
		rtlglue_printf("%d",ip->ip_p);
		break;
	}
	if(l3Flag & PKT_PARSE_L3_CKSUM_ERR)
		rtlglue_printf("Checksum:%04x Error! (%04x)\n", ntohs(ip->ip_sum), l3Cksum);
	else if(l3Flag & PKT_PARSE_L3_CKSUM_OK)
		rtlglue_printf("Checksum:%04x Correct!\n", ntohs(ip->ip_sum));
	else
		rtlglue_printf("Cecksum:%04x Unchecked!\n", ntohs(ip->ip_sum));
	ina.s_addr = ip->ip_src;
	rtlglue_printf("Source IP: %s ",inet_ntoa(ina));
	ina.s_addr = ip->ip_dst;
	rtlglue_printf("Destination IP: %s\n",inet_ntoa(ina));
}

void dumpPptp(greHdr_t * pptp) {
	if(pptp == NULL)
		return;

	rtlglue_printf("PPTP GRE ");
	rtlglue_printf("Key(HW)Payload Length: %d, Key(LW)Call ID: %d\n",ntohs(pptp->gre_length),ntohs(pptp->gre_callid));
	rtlglue_printf("seq. no: %u, ack no: %u\n",ntohl(pptp->gre_seqno),ntohl(pptp->gre_ackno));

}

void dumpIcmp(icmpHdr_t * icmp, uint32 l4Flag, uint16 l4Cksum) {
	if(icmp == NULL)
		return;
	switch(icmp->icmp_type) {
	case ICMP_ECHOREPLY:
		rtlglue_printf("Echo reply ");
		if(icmp->icmp_code != 0)
			rtlglue_printf("Error code value! (%d) ", icmp->icmp_code);
		if(l4Flag & PKT_PARSE_L4_CKSUM_ERR)
			rtlglue_printf("Checksum %04x Error! (%04x)", ntohs(icmp->icmp_cksum), l4Cksum);
		else if(l4Flag & PKT_PARSE_L4_CKSUM_OK)
			rtlglue_printf("Checksum %04x Correct!", ntohs(icmp->icmp_cksum));
		else
			rtlglue_printf("Checksum %04x Unchecked!", ntohs(icmp->icmp_cksum));
		rtlglue_printf("ID:%d Seq:%d\n", ntohs(icmp->content.echo_s.icd_id),
			ntohs(icmp->content.echo_s.icd_seq));
		break;
	case ICMP_ECHO:
		rtlglue_printf("Echo request ");
		if(icmp->icmp_code != 0)
			rtlglue_printf("Error code value! (%d) ", icmp->icmp_code);
		if(l4Flag & PKT_PARSE_L4_CKSUM_ERR)
			rtlglue_printf("Checksum %04x Error! (%04x)", ntohs(icmp->icmp_cksum), l4Cksum);
		else if(l4Flag & PKT_PARSE_L4_CKSUM_OK)
			rtlglue_printf("Checksum %04x Correct!", ntohs(icmp->icmp_cksum));
		else
			rtlglue_printf("Checksum %04x Unchecked!", ntohs(icmp->icmp_cksum));
		rtlglue_printf("ID:%d Seq:%d\n", ntohs(icmp->content.echo_s.icd_id),
			ntohs(icmp->content.echo_s.icd_seq));
		break;
	default:
		if(l4Flag & PKT_PARSE_L4_CKSUM_ERR)
			rtlglue_printf("Type:%d code:%d Checksum:%04x Error! (%04x)\n",
				icmp->icmp_type, icmp->icmp_code, ntohs(icmp->icmp_cksum), l4Cksum);
		else if(l4Flag & PKT_PARSE_L4_CKSUM_OK)
			rtlglue_printf("Type:%d code:%d Checksum:%04x Correct!\n",
				icmp->icmp_type, icmp->icmp_code, ntohs(icmp->icmp_cksum));
		else
			rtlglue_printf("Type:%d code:%d Checksum:%04x Unchecked!\n",
				icmp->icmp_type, icmp->icmp_code, ntohs(icmp->icmp_cksum));
		break;
	}
}

void dumpIgmp(igmpHdr_t * igmp, uint32 l4Flag, uint16 l4Cksum) {
	struct in_addr ina;

	if(igmp == NULL)
		return;
	ina.s_addr = igmp->igmp_group ;

	rtlglue_printf("IGMP Group Address: %s\n",inet_ntoa(ina));
	switch(igmp->igmp_type) {
	case IGMP_MEMBERSHIP_QUERY:
		rtlglue_printf("Member query Max response time %d ",igmp->igmp_respTime);
		break;
	case IGMP_V1_MEMBERSHIP_REPORT:
		rtlglue_printf("Version1 Member report ");
		if(igmp->igmp_respTime != 0)
			rtlglue_printf("Error Max Response Time value (%d) ", igmp->igmp_respTime);
		break;
	case IGMP_V2_MEMBERSHIP_REPORT:
		rtlglue_printf("Version2 Member report ");
		if(igmp->igmp_respTime != 0)
			rtlglue_printf("Error Max Response Time value (%d) ", igmp->igmp_respTime);
		break;
	case IGMP_V2_LEAVE_GROUP:
		rtlglue_printf("Leave report ");
		if(igmp->igmp_respTime != 0)
			rtlglue_printf("Error Max Response Time value (%d) ", igmp->igmp_respTime);
		break;
	case IGMP_DVMRP:
		rtlglue_printf("DVMRP ");
		if(igmp->igmp_respTime != 0)
			rtlglue_printf("Error Max Response Time value (%d) ", igmp->igmp_respTime);
		break;
	case IGMP_PIM:
		rtlglue_printf("PIM ");
		if(igmp->igmp_respTime != 0)
			rtlglue_printf("Error Max Response Time value (%d) ", igmp->igmp_respTime);
		break;
	}
	if(l4Flag & PKT_PARSE_L4_CKSUM_ERR)
		rtlglue_printf("Checksum %04x Error! (%04x)\n", ntohs(igmp->igmp_cksum), l4Cksum);
	else if(l4Flag & PKT_PARSE_L4_CKSUM_OK)
		rtlglue_printf("Checksum %04x Correct! \n", ntohs(igmp->igmp_cksum));
	else
		rtlglue_printf("Checksum %04x Unchecked! \n", ntohs(igmp->igmp_cksum));
}

void dumpTcp(tcpHdr_t * tcp, uint32 l4Flag, uint16 l4Cksum) {
	if(tcp == NULL)
		return;
	rtlglue_printf("Source port:%u Destination port:%u Header Len:%u\n", ntohs(tcp->th_sport), ntohs(tcp->th_dport), tcp->th_off*4);
	rtlglue_printf("Sequence number:%u Acknowledge number:%u\n",ntohl(tcp->th_seq), ntohl(tcp->th_ack));
	rtlglue_printf("Flag:");
	if(tcp->th_flags & TH_URG)
		rtlglue_printf("Urgent ");
	if(tcp->th_flags & TH_ACK)
		rtlglue_printf("Ack ");
	if(tcp->th_flags & TH_PUSH)
		rtlglue_printf("Push ");
	if(tcp->th_flags & TH_RST)
		rtlglue_printf("Reset ");
	if(tcp->th_flags & TH_SYN)
		rtlglue_printf("Sync ");
	if(tcp->th_flags & TH_FIN)
		rtlglue_printf("Fin ");
	rtlglue_printf("Window size %u\n", ntohs(tcp->th_win));
	if(l4Flag & PKT_PARSE_L4_CKSUM_ERR)
		rtlglue_printf("Checksum:%04x Error! (%04x) Urgent pointer:%u\n",ntohs(tcp->th_sum), l4Cksum, ntohs(tcp->th_urp));
	else if(l4Flag & PKT_PARSE_L4_CKSUM_OK)
		rtlglue_printf("Checksum:%04x Correct! Urgent pointer:%u\n",ntohs(tcp->th_sum), ntohs(tcp->th_urp));
	else
		rtlglue_printf("Checksum:%04x Unchecked! Urgent pointer:%u\n",ntohs(tcp->th_sum), ntohs(tcp->th_urp));
}

void dumpUdp(udpHdr_t * udp, uint32 l4Flag, uint16 l4Cksum) {
	if(udp == NULL)
		return;
	rtlglue_printf("Source port:%d Destination port:%d\n",
		ntohs(udp->uh_sport), ntohs(udp->uh_dport));

	if(l4Flag & PKT_PARSE_UDP_NO_CKSUM)
		rtlglue_printf("Payload len:%d Checksum:%04x (No Cksum)\n", ntohs(udp->uh_ulen), ntohs(udp->uh_sum));
	else if (l4Flag & PKT_PARSE_L4_CKSUM_ERR)
		rtlglue_printf("Payload len:%d Checksum:%04x Error! (%04x)\n", ntohs(udp->uh_ulen), ntohs(udp->uh_sum), l4Cksum);
	else if(l4Flag & PKT_PARSE_L4_CKSUM_OK)
		rtlglue_printf("Payload len:%d Checksum:%04x Correct! \n", ntohs(udp->uh_ulen), ntohs(udp->uh_sum));
	else
		rtlglue_printf("Payload len:%d Checksum:%04x Unchecked! \n", ntohs(udp->uh_ulen), ntohs(udp->uh_sum));
}

void dumpIpv6(ip6Hdr_t * ipv6, uint32 exthdr){
	if (ipv6 == NULL){
	//	rtlglue_printf("n0 IPV6\n");
		return;
	}
	rtlglue_printf("Soure V6IP:");
	rtlglue_printf("%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",ipv6->ip_src[0],ipv6->ip_src[1],ipv6->ip_src[2],ipv6->ip_src[3],ipv6->ip_src[4],ipv6->ip_src[5],ipv6->ip_src[6],ipv6->ip_src[7],ipv6->ip_src[8],ipv6->ip_src[9],ipv6->ip_src[10],ipv6->ip_src[11],ipv6->ip_src[12],ipv6->ip_src[13],ipv6->ip_src[14],ipv6->ip_src[15]);
	rtlglue_printf("Destination V6IP:");
        rtlglue_printf("%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",ipv6->ip_dst[0],ipv6->ip_dst[1],ipv6->ip_dst[2],ipv6->ip_dst[3],ipv6->ip_dst[4],ipv6->ip_dst[5],ipv6->ip_dst[6],ipv6->ip_dst[7],ipv6->ip_dst[8],ipv6->ip_dst[9],ipv6->ip_dst[10],ipv6->ip_dst[11],ipv6->ip_dst[12],ipv6->ip_dst[13],ipv6->ip_dst[14],ipv6->ip_dst[15]);
	if (exthdr & hopbyhopHdr)
		rtlglue_printf("the ipv6 pkt has hop by hop header\n");
	if (exthdr & DesHdr)
		rtlglue_printf("the ipv6 pkt has Destination header\n");
	if (exthdr & RouHdr)
		rtlglue_printf("the ipv6 pkt has Routing header\n");
	if (exthdr & FraHdr)
		rtlglue_printf("the ipv6 pkt has Fragment header\n");
	if (exthdr & AutHdr)
		rtlglue_printf("the ipv6 pkt has Authentication header\n");
	if (exthdr & DesHdr2)
		rtlglue_printf("the ipv6 pkt has two destination header\n");
}
void dumpInfo(char *pChar, int len)
{
		pktParse_t result;

		// dump raw packet
 		memDump(pChar, len, NULL);

		// packet parse
		pktParse(pChar, &result, (uint32)len);
		dumpEther(result.ether2);
		dumpVlan(result.vlan);
		dumpLlc(result.llc);
		dumpPppoe(result.pppoe);
		dumpArp(result.arp); 
		dumpIp(result.ip, result.l3Flag, result.l3Cksum);
		dumpIpv6(result.ipv6, result.exheader);
		dumpIcmp(result.icmp, result.l4Flag, result.l4Cksum);
		dumpIgmp(result.igmp, result.l4Flag, result.l4Cksum);
		dumpTcp(result.tcp, result.l4Flag, result.l4Cksum);
		dumpUdp(result.udp, result.l4Flag, result.l4Cksum);
//		rtlglue_printf("before dump ipv6");
//		dumpIpv6(result.ipv6, result.exheader);
		return;
}

