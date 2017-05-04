/*
* Copyright c                  Realtek Semiconductor Corporation, 2007
* All rights reserved.
* 
* Program : Protocol Header rtl8672_proto.h
* Abstract : 
* Author : Yih-Sheng Leu (ysleu@realtek.com.tw)               
*
*/

/* Standard well-defined IP protocols.  */

#ifndef __RTL8672_PROTO
#define __RTL8672_PROTO

#if 0
enum {
  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
  IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
  IPPROTO_IGMP = 2,		/* Internet Group Management Protocol	*/
  IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
  IPPROTO_PUP = 12,		/* PUP protocol				*/
  IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
  IPPROTO_IDP = 22,		/* XNS IDP protocol			*/
  IPPROTO_RSVP = 46,		/* RSVP protocol			*/
  IPPROTO_GRE = 47,		/* Cisco GRE tunnels (rfc 1701,1702)	*/

  IPPROTO_IPV6	 = 41,		/* IPv6-in-IPv4 tunnelling		*/

  IPPROTO_PIM    = 103,		/* Protocol Independent Multicast	*/

  IPPROTO_ESP = 50,            /* Encapsulation Security Payload protocol */
  IPPROTO_AH = 51,             /* Authentication Header protocol       */
  IPPROTO_COMP   = 108,                /* Compression Header protocol */
  IPPROTO_SCTP   = 132,		/* Stream Control Transport Protocol    */

  IPPROTO_RAW	 = 255,		/* Raw IP packets			*/
  IPPROTO_UDPNOCS = 233,	/* NOT standard, just for RTL packet processor, udp no checksum */
  IPPROTO_MAX
};
#endif

enum pcompPPP
{
	PPPNOCOMP = 0,
	PPPCOMP,
	PCOMPPPP_MAX
};

enum pFCSSupp
{
	FCSNOTSUPP = 0,
	FCSSUPP,
	PFCSSUPP_MAX
};

enum ppp_proto
{
	PPP_IPV4 = 0,
	PPP_IPV6,
	PPP_OTHER,
	PPP_PROTO_MAX
};

enum frmctltype
{
	FRAMECTLTYPE1 = 0,
	FRAMECTLTYPE2,
	FRAMECTLTYPE3,
	FRAMECTLTYPE4,
	FRMCTLTYPE_MAX
};

/* Internet address. */
/*struct in_addr {
	uint32	s_addr;
};*/

/************************************************
*****************  IP Header ********************
*************************************************/

struct ip {

	/* replace bit field */
	uint8 ip_vhl;

	uint8	ip_tos;			/* type of service */
	uint16	ip_len;			/* total length */
	uint16	ip_id;			/* identification */
	uint16	ip_off;			/* fragment offset field */
	uint8	ip_ttl;			/* time to live */
	uint8	ip_p;			/* protocol */
	uint16	ip_sum;			/* checksum */
	uint32  	ip_src,ip_dst;	/* source and dest address */
};
#define LenIP 20

/************************************************
*****************  TCP Header ********************
*************************************************/

typedef uint32 tcp_seq;

#define LenTCP 20
struct tcphdr {
	uint16	th_sport;		/* source port */
	uint16	th_dport;		/* destination port */
	tcp_seq	th_seq;			/* sequence number */
	tcp_seq	th_ack;			/* acknowledgement number */

	/* replace bit field */
	uint8  th_off_x;
	
	uint8	th_flags;
		#define	TH_FIN	0x01
		#define	TH_SYN	0x02
		#define	TH_RST	0x04
		#define	TH_PUSH 0x08
		#define	TH_ACK	0x10
		#define	TH_URG	0x20
		#define	TH_ECE	0x40		
		#define	TH_CWR	0x80		
		#define	TH_FLAGS	(TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)

	uint16	th_win;			/* window */
	uint16	th_sum;			/* checksum */
	uint16	th_urp;			/* urgent pointer */

	//Optional TCP options. Max: 40 bytes.
	#define	TCPOPT_EOL			0
	#define	TCPOPT_NOP			1
	#define	TCPOPT_MAXSEG		2
	#define TCPOLEN_MAXSEG	4
	#define TCPOPT_WINDOW	3
	#define TCPOLEN_WINDOW	3
	#define TCPOPT_SACK_PERMITTED		4		/* Experimental */
	#define TCPOLEN_SACK_PERMITTED	2
	#define TCPOPT_SACK		5		/* Experimental */
	#define TCPOPT_TIMESTAMP	8
	#define TCPOLEN_TIMESTAMP		10
	#define TCPOLEN_TSTAMP_APPA	(TCPOLEN_TIMESTAMP+2) /* appendix A */
	#define TCPOPT_TSTAMP_HDR	\
	    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|TCPOPT_TIMESTAMP<<8|TCPOLEN_TIMESTAMP)

	#define	TCPOPT_CC		11		/* CC options: RFC-1644 */
	#define TCPOPT_CCNEW	12
	#define TCPOPT_CCECHO	13
	#define TCPOLEN_CC		6
	#define	TCPOLEN_CC_APPA		(TCPOLEN_CC+2)
	#define	TCPOPT_CC_HDR(ccopt)		\
	    (TCPOPT_NOP<<24|TCPOPT_NOP<<16|(ccopt)<<8|TCPOLEN_CC)
};

/************************************************
*****************  UDP Header ********************
*************************************************/
#define LenUDP 8
struct udphdr
{
	uint16    uh_sport;				   /* source port */
	uint16    uh_dport;				   /* destination port */
	uint16    uh_ulen;				   /* udp length */
	uint16    uh_sum;				   /* udp checksum */
};

/************************************************
*****************  IPv6 Header ******************
*************************************************/
typedef char				lwres_int8_t;
typedef unsigned char			lwres_uint8_t;
typedef short				lwres_int16_t;
typedef unsigned short			lwres_uint16_t;
typedef int				lwres_int32_t;
typedef unsigned int			lwres_uint32_t;
typedef long long			lwres_int64_t;
typedef unsigned long long		lwres_uint64_t;

//tylo
#include <linux/in6.h>
/*
struct in6_addr {
        union {
		lwres_uint8_t	_S6_u8[16];
		lwres_uint16_t	_S6_u16[8];
		lwres_uint32_t	_S6_u32[4];
        } _S6_un;
};*/

struct ipv6hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	uint8			tclass1:4,
				version:4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	uint8		version:4,
				tclass1:4;
#else
//#error	"Please fix <asm/byteorder.h>"
#endif
	uint8			tclass2_flow[3];

	uint16			payload_len;
	uint8			nexthdr;
	uint8			hop_limit;

	struct	in6_addr	saddr;
	struct	in6_addr	daddr;
};
typedef struct ipv6hdr ipv6hdr_t;

/************************************************
*****************  L2 Header ********************
*************************************************/
#define LenEType 2
typedef struct etherType_s {
	uint16 etherType;
} etherType_t;

#define LenMac 12
typedef struct mac_s {
	uint8	da[6], sa[6];
} mac_t;

#define LenVLAN 4
typedef struct vlanHdr_s {
	uint16	etherType;
	uint16	tag;
} vlanHdr_t;
#if 0
typedef struct vlanHdr_s {
#ifdef _LITTLE_ENDIAN
	uint16	vidh:4;
	uint16	cfi:1;
	uint16	priority:3;
	uint16	vidl:8;
#else
	uint16	priority:3;
	uint16	cfi:1;
	uint16	vidh:4;
	uint16	vidl:8;
#endif
	uint16	ether_type;
} vlanHdr_t;
#endif
#define LenLLC 6
typedef struct llcHdr_s {
	uint8	llc_dsap, llc_ssap, ctrl;
	uint8	org_code[3];
} llcHdr_t;

#define LenLLCPPPOA 4
typedef struct llcpppoa_s {
	uint8	llc_dsap, llc_ssap, ctrl;
	uint8	npid;
} llcpppoa_t;

#define LenPPPOE 6
typedef struct pppoeHdr_s {
  #ifdef _LITTLE_ENDIAN
	uint8	type:4, ver:4;
  #else
	uint8	ver:4, type:4;
  #endif
	uint8	code;
	uint16	sessionId, 
			length; //Length of the PPPoE payload, does not include Ethernet and PPPoE header
//	uint16	proto;	//PPP protocol field
} pppoeHdr_t;

#define LenPPP 2
typedef struct pppproto_s{
	uint16 proto;
} pppproto_t;

#define LenPPPComp 1
typedef struct pppproto_comp_s{
	uint8 proto;
} pppproto_comp_t;

#define LenPID 2
typedef struct aal5pid_s{
	uint16 pid;
} aal5pid_t;

#define LenPAD 2

/************************************************
*****************  AAL5 trailer *****************
*************************************************/

#define LenAAL5 4
typedef struct aal5Trailer_s {
/*	uint8 pad[X];	48N bytes padding */
	uint8 cpcsuu;
	uint8 cpi;
	uint16 length;
} aal5Trailer_t;

#define LenCRC 4
typedef struct crc_s{
	uint32 crc;
} crc_t;

/************************************************
*****************  802.11 frame header***********
*************************************************/

#define LenWLAN 30
typedef struct wlanHdr_s{
	/* frame control */
#if 0
	uint16 proto:2;
	uint16 type:2;
	uint16 subtype:4;
	uint16 tods:1;
	uint16 fromeds:1;
	uint16 morefrag:1;
	uint16 retry:1;
	uint16 pwrmgmt:1;
	uint16 moredata:1;
	uint16 wep:1;
	uint16 order:1;
#endif
	uint16 framectr;
	uint16 durationid;
	uint8 mac1[6];
	uint8 mac2[6];
	uint8 mac3[6];
	uint16 seqctl;
	uint8 mac4[6];
} wlanHdr_t;


/* data encapsulation flag */
enum  l2encap_flagType
{
	L2_IP=0,
	L2_VLAN_IP,
	L2_PPPOE64_IP,
	L2_VLAN_PPPOE64_IP,
	L2_ETHER_OTHER,
	L2_IPV6,
	L2_VLAN_IPV6,
	L2_PPPOE63,
	L2_VLAN_PPPOE63,
	L2_VLAN_ETHER_OTHER,
#if 0
	L2_OTHER,
	L2_LLC,
	L2_SNAP_IP,
	L2_SNAP_PPPOE63,	
	L2_SNAP_PPPOE64_IP,
	L2_SNAP_OTHER,	
	L2_VLAN_OTHER,	
	L2_VLAN_LLC,	
	L2_VLAN_SNAP_IP,
	L2_VLAN_SNAP_PPPOE63,	
	L2_VLAN_SNAP_PPPOE64_IP,
	L2_VLAN_SNAP_OTHER,	
#endif	
//keep this field end of enum	
	L2_TYPE_MAX
};

#endif

