#ifndef FLAGDEF_H
#define FLAGDEF_H

//Ethernet is always exist
#define L2_VLAN			0x00000001
#define L2_SNAP			0x00000002
#define L2_PPPoE		0x00000004
#define L2_MASK			0x00000007

#define L3_IPX			0x00000010
#define L3_ARP			0x00000020
#define L3_IP			0x00000040
#define L3_IPV6                0x00000080   //added by liujuan
#define L3_MASK			0x000000f0

#define L4_ICMP			0x00000100
#define L4_IGMP			0x00000200
#define L4_UDP			0x00000400
#define L4_TCP			0x00000800
#define	L4_PPTP			0x00001000		
#define L4_MASK			0x00001f00

#define L2_CRC_ERR		0x80000000
#define L3_CKSUM_ERR		0x40000000
#define L4_CKSUM_ERR		0x20000000
#define L4_NO_CKSUM		0x10000000
#define CTL_MASK		0xf0000000
//added by liujuan
#define hopbyhopHdr  0x00100000
#define DesHdr            0x00200000
#define RouHdr            0x00400000
#define FraHdr            0x00800000
#define AutHdr            0x01000000
#define ESPHdr            0x02000000 
#define DesHdr2          0x04000000
#define NoNHdr            0x08000000
#define ehMASK           0x0ff00000

#define PKT_PARSE_L3_CKSUM_OK	0x00000001
#define PKT_PARSE_L3_CKSUM_ERR	0x00000002

#define PKT_PARSE_L4_CKSUM_OK	0x00000001
#define PKT_PARSE_L4_CKSUM_ERR	0x00000002
#define PKT_PARSE_UDP_NO_CKSUM	0x00000004
#define PKT_PARSE_UDP_LEN_ERR	0x00000008

#define L2_BCAST		0x00010000
#define L2_MCAST		0x00020000
#define L3_BCAST		0x00040000
#define L3_MCAST		0x00080000
#define RELAY_MASK		0x000f0000

#endif

