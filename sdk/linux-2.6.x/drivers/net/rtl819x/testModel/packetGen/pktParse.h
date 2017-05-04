#ifndef PKTPARSE_H
#define PKTPARSE_H

#include <net/rtl/rtl_types.h>
#include "flagDef.h"
#include "l2pkt.h"
#include "l3pkt.h"
#include "l4pkt.h"

typedef struct pktParse_s {
	uint32		pktFlag;
	_etherHdr_t	*ether2;
	_vlanHdr_t	*vlan;
	_llcHdr_t	*llc;
	_pppoeHdr_t	*pppoe;
	//Following are mutual exclusive
	ipxHdr_t	*ipx;
	arpHdr_t	*arp;
	ipHdr_t		*ip;
	uint32		l3Flag;
	uint16		l3Cksum;
	ip6Hdr_t         *ipv6;                      //added by liujuan
	//Following are mutual exclusive
	greHdr_t	*pptp;
	icmpHdr_t	*icmp;
	icmpHdr_t       *v6icmp;
	igmpHdr_t	*igmp;
	udpHdr_t	*udp;
	udpHdr_t	*v6udp;
	tcpHdr_t	*tcp;
	tcpHdr_t	*v6tcp;
	uint32		l4Flag;
	uint16		l4Cksum;
	// payload
	uint8		*payload;
	// Total Length
	uint32		totalLen;
	// Data Length
	uint32		dataLen;
	//extension header
	uint32		exheader;                  //added by liujuan
} pktParse_t;

void pktParse(int8 * pkt, pktParse_t * result, uint32 totalLen);

#endif

