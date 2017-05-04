/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* Program : protoForm
*
* Original Creator: Edward Jin-Ru Chen 2002/4/29
* $Author: ikevin362 $
*
* $Revision: 1.1 $
* $Id: protoForm.c,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
* $Log: protoForm.c,v $
* Revision 1.1  2012/10/24 04:18:57  ikevin362
* initial version
*
* Revision 1.6  2006/04/28 11:05:43  yjlou
* *: By default, we do not clear packet buffer to speed up
*
* Revision 1.5  2005/07/20 15:29:38  yjlou
* +: porting Model Code to Linux Kernel: check RTL865X_MODEL_KERNEL.
*
* Revision 1.4  2004/07/19 12:54:43  chenyl
* *: bug fix: process udp packets withc zero-checksum
* *: bug fix: process L3/L4 packets whose ip-header length is not 20 byte
* +: add testing functions to compare/clear checksum field of mbuf/pkthdr
*
* Revision 1.3  2004/06/29 07:34:23  chenyl
* +: IGMPv3
* +: igmp proxy filter :
* 		protocol stack can let igmp-proxy
* 			1: trap specific multicast group
* 			2: ignore specific multicast group
*
* Revision 1.2  2004/04/30 08:58:08  chenyl
* +: ip multicast/igmp proxy
*
* Revision 1.1  2004/02/25 14:27:24  chhuang
* *** empty log message ***
*
* Revision 1.4  2004/02/25 06:34:22  chenyl
* * tcp mss option generate for "SYN" packet
*
* Revision 1.3  2004/02/18 13:12:21  chenyl
* *** empty log message ***
*
* Revision 1.1  2004/02/13 05:18:47  chenyl
* + Pkt generating functions
*
* Revision 1.17  2003/10/06 10:16:23  waynelee
* add pptp gre support
*
* Revision 1.16  2003/09/30 09:40:55  waynelee
* support llc_other
*
* Revision 1.15  2002/11/27 14:37:39  waynelee
* modify protoForm_ether -> protoForm_vsp_ether
*
* Revision 1.14  2002/10/20 07:32:20  waynelee
* users can define the content of payload (but only indicate one byte)
*
* Revision 1.13  2002/10/16 02:44:15  waynelee
* tcp header length 24 bytes -> 20 bytes
*
* Revision 1.12  2002/10/07 12:29:12  waynelee
* user-defined vcfi (vlan CFI) and L4 No checksum
*
* Revision 1.11  2002/10/04 14:30:01  waynelee
* support user-defined ipv and iphl
*
* Revision 1.10  2002/10/02 15:12:31  waynelee
* ipx support debug
*
* Revision 1.9  2002/10/01 07:47:33  waynelee
* support user-defined ipid, df, mf, ipoff
*
* Revision 1.8  2002/09/16 07:31:16  waynelee
* support IPX (experimental)
*
*/

#include "protoForm.h"
#include "pktForm.h"
#include "l4pkt.h"
#include "l3pkt.h"
#ifdef __KERNEL__
#else
#include <stdio.h>
#include <string.h>
#endif


/*  CLEAR_PKTBUF
 *  If defined, always clean pktBuf before build packet
 *  Not defined, ignore cleaning packet buffer to speed up
 */
#define CLEAN_PKTBUF

void protoForm_vsp_ether(int8 * pktBuf, uint32 l2Flag,
						int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority, 
						uint8 dsap, uint8 ssap,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint16 etherType,
						int8 * content,
						uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) ;
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, dsap, ssap, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l2FormOtherEtherType(pktFmt, pktBuf, etherType, content, *len);

	*len = formedFrameLen(pktFmt, pktBuf, *len);
	l2FormCrc(pktFmt, pktBuf, *len);
}


void protoForm_vsp_ipx(int8 * pktBuf, uint32 l2Flag,
					   int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority, 
	//					uint8 dsap, uint8 ssap,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 type,
						uint32 dnet, uint8 dnode[6], uint16 dsock,
						uint32 snet, uint8 snode[6], uint16 ssock,
						int8 * content, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IPX ;
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xe0, 0xe0, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	// printf("before ipxb address: %02x%02x\n",pktBuf[20],pktBuf[21]);
	l3FormIpxBasic(pktFmt, type, pktBuf);
	// printf("before ipxd address: %02x%02x\n",pktBuf[20],pktBuf[21]);
	l3FormIpxDnet(pktFmt, dnet, dnode, dsock, pktBuf);
	// printf("before ipxs address: %02x%02x\n",pktBuf[20],pktBuf[21]);
	l3FormIpxSnet(pktFmt, snet, snode, ssock, pktBuf);
	// printf("after ipx address: %02x%02x\n",pktBuf[20],pktBuf[21]);

	l3FormIpxContent(pktFmt,pktBuf,content,*len);
	// printf("after ipxc address: %02x%02x\n",pktBuf[20],pktBuf[21]);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}


void protoForm_vs_arp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority, 
						int8 * sha, uint32 spa,
						int8 * tha, uint32 tpa,
						uint16 op, uint32 * len) {
	uint32 pktFmt;
	int8	zero[6]={0};

	pktFmt = (l2Flag & L2_MASK) | L3_ARP; //pppoe is ignored automatically
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l3FormArpBasic(pktFmt, op, pktBuf);

	// if users set "sha", use "sha"; if not, use "sa" as "sha"
	if(memcmp(sha,zero,6)==0)
		l3FormArpSelfInfo(pktFmt, pktBuf, spa, sa);
	else
		l3FormArpSelfInfo(pktFmt, pktBuf, spa, sha);

	// ARP: if users set "tha", use "tha" ; if not, use "da" as "tha"
	// RARP: if users set "tha", use "tha" ; if not, use "sa" as "tha" 
	if(memcmp(tha,zero,6)==0)
		if(op == ARPOP_REVREQUEST)
			l3FormArpPeerInfo(pktFmt, pktBuf, op, tpa, sa);
		else
			l3FormArpPeerInfo(pktFmt, pktBuf, op, tpa, da);
	else
		l3FormArpPeerInfo(pktFmt, pktBuf, op, tpa, tha);


	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_ip(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl,uint32 sip, uint32 dip, int8 proto,
						int8 * content,
						uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_ICMP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	// l3FormIpIdFlag(uint32 flag, int8 * pktBuf, uint16 id, uint8 df, uint8 mf, uint16 offset)
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);
	l3FormIpProtocol(pktFmt, pktBuf, proto);
	l3FormIpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_ip_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl,uint32 sip, uint32 dip, int8 proto,
						int8 * content,
						uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;
	uint8	opt[4];

	// Add: router alert Option
	opt[0] = 0x94;
	opt[1] = 0x04;
	opt[2] = 0;
	opt[3] = 0;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_ICMP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	// l3FormIpIdFlag(uint32 flag, int8 * pktBuf, uint16 id, uint8 df, uint8 mf, uint16 offset)
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);
	l3FormIpProtocol(pktFmt, pktBuf, proto);
	l3FormIpContent(pktFmt, pktBuf, content, *len);

	// option
	l3FormIpOption(pktFmt, pktBuf, opt, 4);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}


void protoForm_vsp_pptp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl, uint32 sip, uint32 dip,
						uint16 callid, uint32 seqno, uint32 ackno,
						int8 * content, 
						uint32 errorFlag, uint32 * len) {

	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_PPTP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormPptpBasic(pktFmt, pktBuf, 0, 0, 1, 1, 0, 1, 1, 0, 1);
	l4FormPptpKey(pktFmt, pktBuf, callid, seqno, ackno);
	l4FormPptpOther(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_icmp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl, uint32 sip, uint32 dip,
						uint8 type, uint8 code, uint16 id, uint16 seq,
						int8 * content,
						uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_ICMP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormIcmpBasic(pktFmt, pktBuf, type, code);
	l4FormIcmpEcho(pktFmt, pktBuf, id, seq);

	l4FormIcmpData(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_igmp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority, 
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint8 ver, uint8 type, uint8 respTime, uint32 gaddr, 
					uint8 qrsq, uint8 qqic, uint16 qnofs, uint32* qslist, uint16 rnofg,
					int8 * content,
					uint32 errorFlag, uint32 *len) {
	uint32 pktFmt;
	uint8	opt[4];

	// router alert Option
	opt[0] = 0x94;
	opt[1] = 0x04;
	opt[2] = 0;
	opt[3] = 0;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_IGMP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	if (ver >= 2)	// igmpv1 don't set Router Alert Option in packet
		l3FormIpOption(pktFmt, pktBuf, opt, 4);

	l4FormIgmpBasic(pktFmt, pktBuf, ver, type, respTime, gaddr, qrsq, qqic, qnofs, qslist, rnofg);
	l4FormIgmpContent(pktFmt, pktBuf, content, *len, ver, type, qnofs);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_tcp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_TCP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormTcpBasic(pktFmt, pktBuf, sport, dport, 20);
	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, tcpFlag, 0);
//	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, ack?TH_ACK:0, 0);
	l4FormTcpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_tcp_option(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_TCP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormTcpBasic(pktFmt, pktBuf, sport, dport, (20+op_len));
	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, tcpFlag, 0);
//	l4FormTcpFlow(pktFmt, pktBuf, seq, ack, win, ack?TH_ACK:0, 0);
	l4FormTcpOption(pktFmt, pktBuf, option, op_len);
	l4FormTcpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}


void protoForm_vsp_udp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_UDP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	l4FormUdpBasic(pktFmt, pktBuf, sport, dport);
	l4FormUdpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

void protoForm_vsp_udp_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len) {
	uint32 pktFmt;
	uint8	opt[4];

	// Add: router alert Option
	opt[0] = 0x94;
	opt[1] = 0x04;
	opt[2] = 0;
	opt[3] = 0;

	pktFmt = (l2Flag & L2_MASK) | L3_IP | L4_UDP | (errorFlag&CTL_MASK);
#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpBasic(pktFmt, pktBuf, ipv, iphl); //Version 4, header length 20
	l3FormIpIdFrag(pktFmt, pktBuf, ipid, df, mf, ipoff/8);
	l3FormIpFlow(pktFmt, pktBuf, tos, ttl); //TOS 0, TTL 16
	l3FormIpAddr(pktFmt, pktBuf, sip, dip);

	// option
	l3FormIpOption(pktFmt, pktBuf, opt, 4);

	l4FormUdpBasic(pktFmt, pktBuf, sport, dport);
	l4FormUdpContent(pktFmt, pktBuf, content, *len);

	*len = formedPktLen(pktFmt, pktBuf);
	l2FormCrc(pktFmt, pktBuf, *len);
}

//added by liujuan
void protoForm_vsp_ipv6(int8 * pktBuf, uint32 l2Flag, uint32 extHdr,int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 pri, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					int8 * content,
					uint32 errorFlag, uint32 * len){

	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & L2_MASK) |L3_IPV6 |L4_ICMP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, pri, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip);                              //?????????????????????????//
	nh=2;
	if (lasthdr==L3_IPV6)
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	else
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
	if ((lasthdr&NoNHdr)==0)
		l3FormIpv6Content(pktFmt, pktBuf, hdrlen, content, *len);
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf, hdrlen); 
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	if (*len < 60)
		*len = 60;
	l2FormCrc(pktFmt, pktBuf, *len);
	*len+=4;

}		

//added by liujuan
void protoForm_vsp_v6tcp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 pri, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & L2_MASK) |L3_IPV6 |L4_TCP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, pri, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip); 
	nh=6;
//	rtlglue_printf("the last header is [%d]\n", lasthdr);
	if (lasthdr==L3_IPV6)       // ??????????????????????????????
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
		
//	if(lasthdr!=NoNHdr) {
//		rtlglue_printf("begin form tcp!\n");
		l4FormTcpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, 20, dstip, segleft);
//		rtlglue_printf("after form tcp basic\n");
		l4FormTcpFlowV6(pktFmt, pktBuf, hdrlen, seq, ack, win, tcpFlag, 0, dstip, segleft);
		l4FormTcpContentV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);

//	}
//	rtlglue_printf("the pktlen before formedpktLenV6 is [%d]\n", *len);
	*len = formedpktLenV6(pktFmt, pktBuf,hdrlen);
//	rtlglue_printf("the pktlen after formedpktLenV6 is [%d]\n", *len);
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after from crc is [%d]\n", *len);
}

//added by liujuan
void protoForm_vsp_v6udp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 pri, uint8 fl[3], uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8  nh;
	pktFmt = (l2Flag & L2_MASK) |L3_IPV6 |L4_UDP|(extHdr & ehMASK)| (errorFlag&CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt,  pktBuf, pri, fl, hl);
	//l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf,hdrlen, segleft, dstip); 
	nh=17;
	if (lasthdr==L3_IPV6)      
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	
	else 
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);

	if (lasthdr!=NoNHdr) {
		l4FormUdpBasicV6(pktFmt, pktBuf, hdrlen, sport, dport, dstip, segleft);
		l4FormUdpContentV6(pktFmt, pktBuf, hdrlen, content, *len,dstip, segleft);
	}

	*len=formedpktLenV6(pktFmt, pktBuf, hdrlen);
	l2FormCrc(pktFmt, pktBuf, *len);

}

//added by liujuan
void protoForm_vsp_v6icmp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 pri, uint8 fl[3], uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint8 type, uint8 code, uint16 id, uint16 seq,
					int8 * content,
					uint32 errorFlag, uint32 * len){
	uint32 pktFmt;
	uint32 lasthdr;
	uint8 nh;
	pktFmt=(l2Flag & L2_MASK) |L3_IPV6 |L4_ICMP |(extHdr & ehMASK)|(errorFlag & CTL_MASK);
	#ifdef CLEAN_PKTBUF
	memset(pktBuf, 0x00, 2048);
	#endif
	l2FormEtherAddress(pktBuf, da, sa);
	l2FormVlanTag(pktFmt, pktBuf, vid, vlanCfi, priority);
	l2FormLlc(pktFmt, 0xAA, 0xAA, pktBuf);
	l2FormPppoeSession(pktFmt, pktBuf, pppoeType, pppoeSession);
	l3FormIpv6Basic(pktFmt, pktBuf, ipv);
	l3FormIpv6Flow(pktFmt, pktBuf,pri, fl, hl);
	l3FormIpv6Addr(pktFmt, pktBuf, sip, dip);
//	rtlglue_printf("the flag in v6icmp is [%d]\n", pktFmt);
	lasthdr=l3FormIpv6ExtHdr(pktFmt, pktBuf, hdrlen, segleft, dstip);	
//	rtlglue_printf("the len in proto is [%d]\n", *len);
	nh=58;
//	lasthdr=L3_IPV6;
	if(lasthdr==L3_IPV6)
		l3FormIpv6NextHeader(pktFmt, pktBuf, nh);
	else
		l3FormLastHeader(pktFmt, pktBuf, hdrlen, lasthdr, nh);
	if (lasthdr!=NoNHdr) {
//		rtlglue_printf("********in form l4 v6 icmp***********\n");
		l4FormIcmpBasicV6(pktFmt, pktBuf, hdrlen, type, code, dstip, segleft);
//		memDump(pktBuf, 62, NULL);
		l4FormIcmpEchoV6(pktFmt, pktBuf, hdrlen, id, seq, dstip, segleft);
//		memDump(pktBuf, 62, NULL);
		l4FormIcmpDataV6(pktFmt, pktBuf, hdrlen, content, *len, dstip, segleft);
//		memDump(pktBuf, 95, NULL);
	} 

	*len=formedpktLenV6(pktFmt, pktBuf, hdrlen);
//	rtlglue_printf("the length after formpktlenv6 is [%d]\n", *len);
	l2FormCrc(pktFmt, pktBuf, *len);
//	rtlglue_printf("the length after form crc is [%d]\n", *len);
//	memDump(pktBuf, *len, NULL);
	
}

