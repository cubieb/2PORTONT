#ifndef PKTFORM_H
#define PKTFORM_H

#include <net/rtl/rtl_types.h>
#include "flagDef.h"

void l2FormEtherAddress(int8 * pktBuf, int8 * da, int8 * sa);
void l2FormCrc(uint32 flag, int8 * pktBuf, uint32 payloadSize);
// void l2FormVlanTag(uint32 flag, int8 * pktBuf, uint16 vlanTag, uint16 vlanPriority);
void l2FormVlanTag(uint32 flag, int8 * pktBuf, uint16 vlanTag, uint8 vlanCfi, uint16 vlanPriority);
void l2FormLlc(uint32 flag, uint8 dsap, uint8 ssap, int8 * pktBuf);
// void l2FormLlc(uint32 flag, int8 * pktBuf);
void l2FormPppoeSession(uint32 flag, int8 * pktBuf, uint16 etherType, uint16 sessionId);

// static int32 l3Offset(uint32 flag);

void l2FormOtherEtherType(uint32 flag, int8 * pktBuf, uint16 etherType, int8 * content, uint32 payloadLen);

//Following fields are valid for layer2 only packets
//Input length is the payload length, fill LLC length and pppoe length
void l2FormLayer2Len(uint32 flag, int8 * pktBuf, uint16 payloadLen);
void l2FormLayer3Type(uint32 flag, int8 * pktBuf, uint16 type);//EtherType or PPP protocol

void l3FormIpxBasic(uint32 flag, uint8 type, int8 * pktBuf);
void l3FormIpxDnet(uint32 flag, uint32 dnet, uint8 * dnode, uint16 dsock, int8 * pktBuf);
void l3FormIpxSnet(uint32 flag, uint32 snet, uint8 * snode, uint16 ssock, int8 * pktBuf);
void l3FormIpxContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size);

void l3FormArpBasic(uint32 flag, uint16 op, int8 * pktBuf);
void l3FormArpSelfInfo(uint32 flag, int8 * pktBuf, uint32 ipAddr, int8 * macAddr);
void l3FormArpPeerInfo(uint32 flag, int8 * pktBuf, uint16 op, uint32 ipAddr, int8 * macAddr);

void l3FormIpBasic(uint32 flag, int8 * pktBuf, uint8 ver, uint8 len);
void l3FormIpFlow(uint32 flag, int8 * pktBuf, uint8 tos, uint8 ttl);
//Input length is the payload length, function translate payload length to
// the total length
void l3FormIpTotalLen(uint32 flag, int8 * pktBuf, uint16 len);
void l3FormIpIdFrag(uint32 flag, int8 * pktBuf, uint16 id, uint8 df, uint8 mf, uint16 offset);
void l3FormIpProtocol(uint32 flag, int8 * pktBuf, int8 protocol);
void l3FormIpAddr(uint32 flag, int8 * pktBuf, uint32 sip, uint32 dip);
void l3FormIpContent(uint32 flag, int8 * pktBuf, uint8 * content, uint32 size);
void l3FormIpOption(uint32 flag, int8 * pktBuf, uint8 * content, uint32 size);
void l3FormIpChecksum(uint32 flag, int8 *pktBuf);
//added by liujuan
uint32 l3FormIpv6ExtHdr(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 segleft, uint8 * dstip);
void l3FormIpv6TotalLen(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 len);              //added by liujuan
void l3FormIpv6Basic(uint32 flag, int8* pktBuf, uint8 ver);            //added by liujuan
void l3FormIpv6Flow(uint32 flag, int8* pktBuf, uint8 pri, uint8 * fl, uint8 hl );
void l3FormIpv6Addr(uint32 flag, int8* pktBuf, uint8 * sip, uint8 * dip);
void l3FormIpv6NextHeader(uint32 flag, int8 * pktBuf, uint8 nh);
int32 v6ExtHdrLen(uint32 flag, uint8 rhdrlen);
void l3FormIpv6Content(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size);
//static uint32 l3FormIpv6ExtHdr(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 segleft, uint8 * dstip);
void l3FormLastHeader(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint32 lasthdr, uint8 nh);

void l4FormPptpBasic(uint32 flag, int8 * pktBuf, 
					 uint8 Cbit, uint8 Rbit, uint8 Kbit, uint8 Sbit, uint8 sbit,
					 uint8 recur, uint8	Abit, uint8	otherflags, uint8 ver);
void l4FormPptpKey (uint32 flag, int8 * pktBuf, 
					uint16 callid, uint32 seqno, uint32 ackno);
void l4FormPptpOther (uint32 flag, int8 * pktBuf, int8 * content, uint32 size);

void l4FormIcmpBasic(uint32 flag, int8 * pktBuf, uint8 type, uint8 code);
void l4FormIcmpEcho(uint32 flag, int8 * pktBuf, uint16 id, uint16 seq);
void l4FormIcmpData(uint32 flag, int8 * pktBuf, int8 * data, uint32 size);
void l4FormIcmpChecksum(uint32 flag, int8 * pktBuf);
//added by liujuan
void l4FormIcmpBasicV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 type, uint8 code, uint8* dip, uint8 segleft);
void l4FormIcmpEchoV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 id, uint16 seq, uint8 * dip, uint8 segleft);
void l4FormIcmpDataV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * data, uint32 size, uint8 * dip, uint8 segleft);
void l4FormIcmpChecksumV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8* dip, uint8 segleft);

void l4FormIgmpBasic(uint32 flag, int8 * pktBuf, uint8 ver, uint8 type, uint8 respTime, uint32 addr,
						uint8 qrsq, uint8 qqic, uint16 qnofs, uint32 * qslist, uint16 rnofg);
void l4FormIgmpContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size, uint8 ver, uint8 type, uint16 qnofs);
void l4FormIgmpChecksum(uint32 flag, int8 * pktBuf);

void l4FormUdpBasic(uint32 flag, int8 * pktBuf, uint16 sPort, uint16 dPort);
void l4FormUdpContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size);
void l4FormUdpChecksum(uint32 flag, int8 * pktBuf);
//added by liujuan
void l4FormUdpBasicV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 sPort, uint16 dPort, uint8* dip, uint8 segleft);
void l4FormUdpContentV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size, uint8* dip, uint8 segleft);
void l4FormUdpChecksumV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 * dip, uint8 segleft);

void l4FormTcpBasic(uint32 flag, int8 * pktBuf, uint16 sPort, uint16 dPort, uint8 hlen);
void l4FormTcpFlow(uint32 flag, int8 * pktBuf, uint32 seq, uint32 ack, uint16 winSize, uint8 tcpFlag, uint16 urgPtr);
void l4FormTcpOption(uint32 flag, int8 * pktBuf, int8 * option, uint8 size);
void l4FormTcpContent(uint32 flag, int8 * pktBuf, int8 * content, uint32 size);
void l4FormTcpChecksum(uint32 flag, int8 * pktBuf);
//added by liujuan
void l4FormTcpBasicV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint16 sPort, uint16 dPort, uint8 hlen, uint8* dip, uint8 segleft);
void l4FormTcpFlowV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint32 seq, uint32 ack, uint16 winSize, uint8 tcpFlag, uint16 urgPtr, uint8* dip, uint8 segleft);
void l4FormTcpOptionV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 * option, uint8 size, uint8* dip, uint8 segleft);
void l4FormTcpContentV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, int8 * content, uint32 size, uint8 * dip, uint8 segleft);
void l4FormTcpChecksumV6(uint32 flag, int8 * pktBuf, uint8 hdrlen, uint8 * dip, uint8 segleft);

uint32 formedPktLen(uint32 flag, int8 * pktBuf);
uint32 formedpktLenV6(uint32 flag, int8 * pktBuf, uint8 hdrlen);      //added by liujuan
uint32 formedFrameLen(uint32 flag, int8 * pktBuf, uint32 payloadLen);

#endif

