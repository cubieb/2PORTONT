/*
* protoForm.h --
* 
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.                                                
* 
* $Header: /usr/local/dslrepos/linux-2.6.30/drivers/net/rtl819x/testModel/packetGen/protoForm.h,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
* 
* Program : protoForm Headers
* Abstract: The header file describes the externally-visible
*           protocol forms. These protocols are ether, ipx,
*           arp, ip, icmp, igmp, tcp, udp. We can use them to 
*           form upper-layer packets.
* 
* $Author: ikevin362 $
*
* $Revision: 1.1 $
* $Id: protoForm.h,v 1.1 2012/10/24 04:18:57 ikevin362 Exp $
* $Log: protoForm.h,v $
* Revision 1.1  2012/10/24 04:18:57  ikevin362
* initial version
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
* Revision 1.15  2003/10/06 10:16:23  waynelee
* add pptp gre support
*
* Revision 1.14  2003/09/30 09:40:55  waynelee
* support llc_other
*
* Revision 1.13  2002/11/27 14:37:39  waynelee
* modify protoForm_ether -> protoForm_vsp_ether
*
*/

#ifndef PROTO_FORM_H
#define PROTO_FORM_H

#include <net/rtl/rtl_types.h>


/* 
 * general ethernet frames; we can indicate the ethertype of this frame
 */

void protoForm_vsp_ether(int8 * pktBuf, uint32 l2Flag,
						int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority, 
						uint8 dsap, uint8 ssap,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint16 etherType,
						int8 * content,
						uint32 * len);

/*
 * IPX packets; only support general IPX/SAP
 * Note: if using l2Flag, only vlan works.
 */

void protoForm_vsp_ipx(int8 * pktBuf, uint32 l2Flag,
					   int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority, 
	//					uint8 dsap, uint8 ssap,
						uint16 pppoeType,
						uint16 pppoeSession,
						uint8 type,
						uint32 dnet, uint8 dnode[6], uint16 dsock,
						uint32 snet, uint8 snode[6], uint16 ssock,
						int8 * content, uint32 * len);

/*
 * ARP packets; including ARP, RARP
 */

void protoForm_vs_arp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority, 
						int8 * sha, uint32 spa,
						int8 * tha, uint32 tpa,
						uint16 op, uint32 * len);
/*
 * IP packets; len is the length of IP payload
 */

void protoForm_vsp_ip(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl,uint32 sip, uint32 dip, int8 proto,
						int8 * content,
						uint32 errorFlag, uint32 * len);

void protoForm_vsp_ip_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl,uint32 sip, uint32 dip, int8 proto,
						int8 * content,
						uint32 errorFlag, uint32 * len);

void protoForm_vsp_pptp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl, uint32 sip, uint32 dip,
						uint16 callid, uint32 seqno, uint32 ackno,
						int8 * content, 
						uint32 errorFlag, uint32 * len);
/*
 * ICMP packets; len is the length of ICMP payload
 */

void protoForm_vsp_icmp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
						uint16 vid, uint8 vlanCfi, uint16 priority,
						uint16 pppoeType,
						uint16 pppoeSession, 
						uint8 ipv, uint8 iphl,
						uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
						uint8 ttl, uint32 sip, uint32 dip,
						uint8 type, uint8 code, uint16 id, uint16 seq,
						int8 * content,
						uint32 errorFlag, uint32 * len);
/*
 * IGMP packets; len doesn't work now.
 */

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
					uint32 errorFlag, uint32 *len);
/*
 * TCP packets; len is the length of TCP payload
 */

void protoForm_vsp_tcp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_tcp_option(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * option, uint32 op_len, int8 * content, uint32 errorFlag, uint32 * len);
/*
 * UDP packets; len is the length of UDP payload
 */

void protoForm_vsp_udp(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_udp_opt(int8 * pktBuf, uint32 l2Flag, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 iphl,
					uint8 tos, uint16 ipid, uint8 df, uint8 mf, uint16 ipoff,
					uint8 ttl, uint32 sip, uint32 dip,
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);
//added by liujuan
void protoForm_vsp_ipv6(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 pri, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					int8 * content,
					uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6tcp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 pri, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 sport, uint16 dport,
					uint32 seq, uint32 ack, uint8 tcpFlag, uint16 win, 
					int8 * content, uint32 errorFlag, uint32 * len);


void protoForm_vsp_v6udp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession,
					uint8 ipv, uint8 pri, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint16 sport, uint16 dport,
					int8 * content, uint32 errorFlag, uint32 * len);

void protoForm_vsp_v6icmp(int8 * pktBuf, uint32 l2Flag, uint32 extHdr, int8 * da, int8 * sa, 
					uint16 vid, uint8 vlanCfi, uint16 priority,
					uint16 pppoeType,
					uint16 pppoeSession, 
					uint8 ipv, uint8 pri, uint8 fl[3],  uint8 hl,
					uint8 sip[16], uint8 dip[16], 
					uint8 hdrlen, uint8 segleft, uint8 dstip[16],
					uint8 type, uint8 code, uint16 id, uint16 seq,
					int8 * content,
					uint32 errorFlag, uint32 * len);

#endif

