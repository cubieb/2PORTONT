#ifndef L3PKT_H
#define L3PKT_H

#include <net/rtl/rtl_types.h>

#pragma pack(push,1)
typedef struct ipxHdr_s {
	uint16	ipx_sum;	/* checksum */
	uint16	ipx_len;	/* packet length */
	uint8	ipx_tctrl;	/* transport control */
	uint8	ipx_type;	/* packet type */
	uint32	ipx_dnet;	/* destination network */
	uint8	ipx_dnode[6];	/* destination node */
	uint16	ipx_dsock;		/* destination socket */
	uint32	ipx_snet;	/* source network */
	uint8	ipx_snode[6];	/* source node */
	uint16	ipx_ssock;		/* source socket */
} ipxHdr_t;

typedef struct	arpHdr_s {
	uint16	ar_hrd;		/* format of hardware address */
#define ARPHRD_ETHER 	1	/* ethernet hardware format */
	uint16	ar_pro;		/* format of protocol address */
	uint8	ar_hln;		/* length of hardware address */
	uint8	ar_pln;		/* length of protocol address */
	uint16	ar_op;		/* one of: */
#define	ARPOP_REQUEST		1	/* request to resolve address */
#define	ARPOP_REPLY			2	/* response to previous request */
#define	ARPOP_REVREQUEST	3	/* request protocol address given hardware */
#define	ARPOP_REVREPLY		4	/* response giving protocol address */
#define ARPOP_DRARPREQUEST	5	/* dynamic RARP Request */
#define ARPOP_DRARPREPLY	6	/* dynamic RARP Reply */
#define ARPOP_DRARPERROR	7	/* dynamic RARP Error */
#define ARPOP_INARPREQUEST	8	/* InARP Request */
#define ARPOP_INARPREPLY	9	/* InARP Reply */
	uint8	ar_sha[6];	/* sender hardware address */
	uint8	ar_spa[4];	/* sender protocol address */
	uint8	ar_tha[6];	/* target hardware address */
	uint8	ar_tpa[4];	/* target protocol address */
} arpHdr_t;

typedef struct ipHdr_s {
#ifdef _LITTLE_ENDIAN
	uint8	ip_hl:4,		/* header length */
			ip_v:4;			/* version */
#else
	uint8	ip_v:4,			/* version */
			ip_hl:4;		/* header length */
#endif
	uint8	ip_tos;			/* type of service */
	uint16	ip_len;			/* total length */
	uint16	ip_id;			/* identification */
	uint16	ip_off;			/* fragment offset field */
#define	IP_RF 0x8000			/* reserved fragment flag */
#define	IP_DF 0x4000			/* dont fragment flag */
#define	IP_MF 0x2000			/* more fragments flag */
#define	IP_OFFMASK 0x1fff		/* mask for fragmenting bits */
	uint8	ip_ttl;			/* time to live */
	uint8	ip_p;			/* protocol */
	uint16	ip_sum;			/* checksum */
	uint32	ip_src,ip_dst;	/* source and dest address */
} ipHdr_t;

#define IP_PROTO_ICMP	0x01
#define IP_PROTO_IGMP	0x02
#define IP_PROTO_TCP	0x06
#define IP_PROTO_UDP	0x11
#define	IP_PROTO_PPTP	0x2f
#define IP_PROTO_V6ICMP  0x3a

typedef struct ip6Hdr_s {
#ifdef _LITTLE_ENDIAN
	uint8	ip_pri:4,		/* priority */
			ip_v:4;			/* version */
#else
	uint8	ip_v:4,			/* version */
			ip_pri:4;		/* priority */
#endif
	uint8	ip_flow[3];		/* flow label */
	uint16	ip_len;			/* the length of (l4 payload+ extension header)*/
	uint8	ip_next;		/* next header */
	uint8	ip_hop;			/* hop limit */
	uint8	ip_src[16];		/* source address */
	uint8	ip_dst[16];		/* destination address */
} ip6Hdr_t;
//total lenth 16 byte.  added by liujuan
typedef struct hopbyhopHdr_s {
	uint8       nexthdr;
	uint8       hdrlen;
}hopbyhopHdr_t;
//total lenth=hdrlen*8+8          added by liujuan
typedef struct RouHdr_s {
	uint8      nexthdr;
	uint8      hdrlen;
	uint8      routingtype;
	uint8      segmentleft;
//	uint8 *    data; 
//	uint8      dstip[16];
}RouHdr_t;
//added by liujuan.  total lenth  8 byte
typedef struct FraHdr_s {
	uint8       nexthdr;
	uint16      fragment;
}FraHdr_t;

//added by liujuan
typedef struct DesHdr_s {
	uint8       nexthdr;
	uint8       hdrlen;
	
}DesHdr_t;

//total lenth=(hdrlen*4 + 8) byte
typedef struct AutHdr_s {
	uint8       nexthdr;
	uint8       hdrlen;
}AutHdr_t;
#pragma pack (pop)

#endif
