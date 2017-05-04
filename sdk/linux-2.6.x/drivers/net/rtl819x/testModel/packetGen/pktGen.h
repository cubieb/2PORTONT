#ifndef PKT_GEN_
#define PKT_GEN_

#include <net/rtl/rtl_types.h>
#include "flagDef.h"
#include "l2pkt.h"
#include "l3pkt.h"
#include "l4pkt.h"

typedef struct rtl8651_IgmpGrConf_s {
	uint8	type;
	uint8	auxlen;
	uint16	nofs;
	uint32	gaddr;
	uint32 *	slist;
	uint8 *	auxdata;
} rtl8651_IgmpGrConf_t;

#define _PKT_STOP		0	// end of testcase, packet generator stop
#define _PKT_TYPE_ETHER		1
#define _PKT_TYPE_IPX			2
#define _PKT_TYPE_ARP			3
#define _PKT_TYPE_IP			4
#define _PKT_TYPE_PPTP			5
#define _PKT_TYPE_ICMP			6
#define _PKT_TYPE_IGMP			7
#define _PKT_TYPE_TCP			8
#define _PKT_TYPE_UDP			9
#define _PKT_TYPE_UDP_OPTION	10
#define _PKT_TYPE_IPV6                 11        //added by liujuan
#define _PKT_TYPE_V6ICMP            12       //added by liujuan
#define _PKT_TYPE_V6TCP              13       //added by liujuan
#define _PKT_TYPE_V6UDP              14       //added by liujuan

typedef struct rtl8651_PktConf_s {

	uint32		pktType;	/* Packet Type of packet */
	uint32		l2Flag;		/* l2Flags, defined in flagDef.h and masked using L2_MASK */
	uint32		ErrFlag;	/* Let L3 Checksum error with setting this flag, defined in flagDef.h and masked using CTL_MASK */
	uint32		relayType;	/* L2_BCAST L2_MCAST L3_BCAST, for default value setting of dmac,dip and pktHdr */
	uint32            extHdr;         //added by liujuan
	struct {
		uint8		ASIC_pppoeIdx;		/* pppoe index in asic */
		uint8		ASIC_extPortList;	/* dest extension port list. must be 0 for Tx */
		uint8		ASIC_srcExtPortNum;	/*Both in Rx & Tx. Source extension port number. */
		uint16		ASIC_vlanIdx;		/* vlan index in asic, note: it's used to indicate vlan id! */
		uint16		ASIC_portlist;		/* RX: input port, TX: output port */
		uint32		ASIC_reason;		/* reason bits */
		uint16		PKT_category;		/* categorey */
		uint16		PKT_otherFlags;		/* other flags to set :PKTHDR_PPPOE_AUTOADD, PKTHDR_VLAN_AUTOADD, CSUM_IP, CSUM_L4 */

	} pktHdrInfo; 
	// for pktHdr setting
	#define		conf_pppoeIdx			pktHdrInfo.ASIC_pppoeIdx
	/*note: conf_vlanIdx is used to indicate vlan id now!*/
	#define		conf_vlanIdx			pktHdrInfo.ASIC_vlanIdx
	#define		conf_extPortList		pktHdrInfo.ASIC_extPortList
	#define		conf_srcExtPortNum		pktHdrInfo.ASIC_srcExtPortNum
	#define		conf_portlist			pktHdrInfo.ASIC_portlist
	#define		conf_reason			pktHdrInfo.ASIC_reason
	#define		conf_category		pktHdrInfo.PKT_category
	#define		conf_Flags			pktHdrInfo.PKT_otherFlags

	#define		pkt_PKTHDR_PPPOE_AUTOADD	0x0008	// PKTHDR_PPPOE_AUTOADD in mbuf.h
	#define		pkt_PKTHDR_VLAN_AUTOADD		0x0004	// PKTHDR_VLAN_AUTOADD in mbuf.h       

	struct {
		int8*		content;	/* content field in packet */
		uint32		length;		/* content length */
	} payload;

	struct {
		uint8		da[6];		/* Destination mac */
		uint8		sa[6];		/* Destination mac */
		uint16		etherType;	/* ether type */
	} ether_header;
	#define	conf_dmac		ether_header.da
	#define conf_smac		ether_header.sa
	#define conf_ethtype		ether_header.etherType

	struct {
		uint16		vid;		/* vlan id */
		uint8		cfi;		/* vlan CFI bit */
		uint16		prio;		/* vlan priority field */
	} vlan;

	struct {
		uint8		dsap;
		uint8		ssap;
	} llc;

	struct {
		uint16		type;		/* pppoe Type */
		uint16		session;	/* pppoe Session */
	} pppoe;

	struct {
		uint8		type;
		uint32		dnet;
		uint8		dnode[6];
		uint16		dsock;
		uint32		snet;
		uint8		snode[6];
		uint16		ssock;
	} ipx;

	struct {
		int8		mac_src[6];
		uint32		ip_src;
		int8		mac_dst[6];
		uint32		ip_dst;
		uint16		op;		/* options : ARP/ARP_reply/RARP/RARP_reply */
	} arp;

	struct {
		uint8		version;
		uint8		headerLen;
		uint8		tos;
		uint16		id;
		uint8		diffserv;	/* DS and ECN */
		uint8		mf;
		uint16		offset;
		uint8		ttl;
		uint32		src_ip;
		uint32		dst_ip;
		int8		protocol;
	} ip;
	#define conf_sip		ip.src_ip
	#define conf_dip		ip.dst_ip

//added by liujuan
	struct{
		uint8       version;
		uint8       priority;
		uint8       flowlbl[3];
		uint16     pldlen;
		uint8       nxthdr;
		uint8       hoplmt;

	       uint8       src_ip[16];
		uint8       dst_ip[16];
	}ipv6;
	#define conf_v6sip       ipv6.src_ip
	#define conf_v6dip       ipv6.dst_ip

	struct{
		uint8     nexthdr;
		uint8     hdrlen;
		uint8     segments_left;
		uint8     dip[16];
	}rhdr;
	
	struct {
		uint16		callid;
		uint32		seqno;
		uint32		ackno;
	} pptp;

	struct {
		uint8		type;
		uint8		code;
		uint16		id;
		uint16		seq;
	} icmp;

	struct {
		uint8					ver;
		uint8					type;
		uint8					respTime;
		uint32					gaddr;
		// for query
		uint8					qrsq;
		uint8					qqic;
		uint16					qnofs;
		uint32 *					qslist;
		// for report
		uint16					rnofg;
		rtl8651_IgmpGrConf_t *	grCfg;
	} igmp;

	struct {
		uint16		src_port;
		uint16		dst_port;
		struct {
			uint32		seq;
			uint32		ack;
			uint8		tcpFlag;
			uint16		win;
			uint16		mss;
		} tc;
	} l4hdr;
	#define conf_sport		l4hdr.src_port
	#define conf_dport		l4hdr.dst_port
	#define conf_tcp_seq		l4hdr.tc.seq
	#define	conf_tcp_ack		l4hdr.tc.ack
	#define	conf_tcp_flag		l4hdr.tc.tcpFlag
	#define conf_tcp_win		l4hdr.tc.win
	#define conf_tcp_mss		l4hdr.tc.mss
} rtl8651_PktConf_t;

uint32 pktGen(rtl8651_PktConf_t *, int8 *);
uint32 grGen(rtl8651_IgmpGrConf_t *, uint32, int8 *);
#endif
