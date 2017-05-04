#ifndef PKTDUMP_H
#define PKTDUMP_H

#include "l2pkt.h"
#include "l3pkt.h"
#include "l4pkt.h"

void dumpEther(_etherHdr_t * ether);
void dumpVlan(_vlanHdr_t * vlan);
void dumpLlc(_llcHdr_t * llc);
void dumpPppoe(_pppoeHdr_t * pppoe);

void dumpIpx(ipxHdr_t * ipx);
void dumpArp(arpHdr_t * arp);
void dumpIp(ipHdr_t * ip, uint32 l3Flag, uint16 l3Cksum);

void dumpPptp(greHdr_t * pptp);
void dumpIcmp(icmpHdr_t * icmp, uint32 l4Flag, uint16 l4Cksum);
void dumpIgmp(igmpHdr_t * igmp, uint32 l4Flag, uint16 l4Cksum);
void dumpTcp(tcpHdr_t * tcp, uint32 l4Flag, uint16 l4Cksum);
void dumpUdp(udpHdr_t * udp, uint32 l4Flag, uint16 l4Cksum);
void dumpIpv6(ip6Hdr_t * ipv6, uint32 exthdr);
void dumpInfo(char *pChar, int len);
#endif
