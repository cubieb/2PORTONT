#ifndef L4PKT_H
#define L4PKT_H

#include <net/rtl/rtl_types.h>

#pragma pack(push,1)
typedef struct icmpHdr_s {
	uint8	icmp_type;		/* type of message, see below */
	uint8	icmp_code;		/* type sub code */
	uint16	icmp_cksum;		/* ones complement cksum of struct */
	union {
		struct echo_s {
			uint16	icd_id;
			uint16	icd_seq;
		}echo_s;
		int8 * gen;
	} content;
} icmpHdr_t;

#define	ICMP_ECHOREPLY		0		/* echo reply */
#define	ICMP_UNREACH		3		/* dest unreachable, codes: */
#define		ICMP_UNREACH_NET	0		/* bad net */
#define		ICMP_UNREACH_HOST	1		/* bad host */
#define		ICMP_UNREACH_PROTOCOL	2		/* bad protocol */
#define		ICMP_UNREACH_PORT	3		/* bad port */
#define		ICMP_UNREACH_NEEDFRAG	4		/* IP_DF caused drop */
#define		ICMP_UNREACH_SRCFAIL	5		/* src route failed */
#define		ICMP_UNREACH_NET_UNKNOWN 6		/* unknown net */
#define		ICMP_UNREACH_HOST_UNKNOWN 7		/* unknown host */
#define		ICMP_UNREACH_ISOLATED	8		/* src host isolated */
#define		ICMP_UNREACH_NET_PROHIB	9		/* prohibited access */
#define		ICMP_UNREACH_HOST_PROHIB 10		/* ditto */
#define		ICMP_UNREACH_TOSNET	11		/* bad tos for net */
#define		ICMP_UNREACH_TOSHOST	12		/* bad tos for host */
#define		ICMP_UNREACH_FILTER_PROHIB 13		/* admin prohib */
#define		ICMP_UNREACH_HOST_PRECEDENCE 14		/* host prec vio. */
#define		ICMP_UNREACH_PRECEDENCE_CUTOFF 15	/* prec cutoff */
#define	ICMP_SOURCEQUENCH	4		/* packet lost, slow down */
#define	ICMP_REDIRECT		5		/* shorter route, codes: */
#define		ICMP_REDIRECT_NET	0		/* for network */
#define		ICMP_REDIRECT_HOST	1		/* for host */
#define		ICMP_REDIRECT_TOSNET	2		/* for tos and net */
#define		ICMP_REDIRECT_TOSHOST	3		/* for tos and host */
#define	ICMP_ECHO		8		/* echo service */
#define	ICMP_ROUTERADVERT	9		/* router advertisement */
#define	ICMP_ROUTERSOLICIT	10		/* router solicitation */
#define	ICMP_TIMXCEED		11		/* time exceeded, code: */
#define		ICMP_TIMXCEED_INTRANS	0		/* ttl==0 in transit */
#define		ICMP_TIMXCEED_REASS	1		/* ttl==0 in reass */
#define	ICMP_PARAMPROB		12		/* ip header bad */
#define		ICMP_PARAMPROB_ERRATPTR 0		/* error at param ptr */
#define		ICMP_PARAMPROB_OPTABSENT 1		/* req. opt. absent */
#define		ICMP_PARAMPROB_LENGTH 2			/* bad length */
#define	ICMP_TSTAMP		13		/* timestamp request */
#define	ICMP_TSTAMPREPLY	14		/* timestamp reply */
#define	ICMP_IREQ		15		/* information request */
#define	ICMP_IREQREPLY		16		/* information reply */
#define	ICMP_MASKREQ		17		/* address mask request */
#define	ICMP_MASKREPLY		18		/* address mask reply */
#if 0
typedef struct igmpHdr_s {
	uint8		igmp_type;	/* version & type of IGMP message  */
#define IGMP_MEMBERSHIP_QUERY   	0x11	/* membership query         */
#define IGMP_V1_MEMBERSHIP_REPORT	0x12	/* Ver. 1 membership report */
#define IGMP_V2_MEMBERSHIP_REPORT	0x16	/* Ver. 2 membership report */
#define IGMP_V2_LEAVE_GROUP			0x17	/* Leave-group message	    */
#define IGMP_DVMRP					0x13	/* DVMRP routing message    */
#define IGMP_PIM					0x14	/* PIM routing message	    */
	uint8		igmp_respTime;	/* Max response time       */
	uint16		igmp_cksum;		/* IP-style checksum               */
	uint32		igmp_group;		/* group address being reported    */
} igmpHdr_t;
#endif
#define IGMP_MINLEN		     8

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// igmp v3 group record
typedef struct igmpGr_s {
	uint8	igmp_gr_rt;				/* Record Type */
	uint8	igmp_gr_auxlen;			/* aux data length */
	uint16	igmp_gr_nofs;			/* number of sources */
	uint32	igmp_gr_group;			/* group address being reported */
	uint32	src_list;					/* first entry of src list */
	//	auxiliary data is unused now
 } igmpGr_t;
typedef struct igmpHdr_s {
	uint8		igmp_type;			/* version & type of IGMP message */
	uint8		igmp_respTime;		/* subtype for routing msgs */
	uint16		igmp_cksum;		/* IP-style checksum */

	union {
		uint32			group;		/* group address being presented (v1/v2/v3 query) */
		struct igmpRep1_s {
			uint16			resv;	/* reserved */
			uint16			nofg;	/* number of group records */
		} igmpRep1;
	} grp;

	union {
		struct igmpQ_s {
			uint8	rsq;				/* 4bit: reserved, 1bit: suppress router-side processing, 3bit: querier's robustness variable*/
			uint8	qqic;			/* querier's query interval code */
			uint16	nofs;			/* number of sources */
			uint32	src_list;			/* first entry of src list */
		} igmpQ;
		struct igmpRep2_s {
			igmpGr_t		gr_list;	/* first entry of group record */
		} igmpRep2;
	} un_v3;

	#define	igmp_group		grp.group
	#define	igmp_QRSQ		un_v3.igmpQ.rsq
	#define	igmp_QQIC		un_v3.igmpQ.qqic
	#define	igmp_QNOFS		un_v3.igmpQ.nofs
	#define	igmp_QSLIST	un_v3.igmpQ.src_list
	#define	igmp_RNOFG		grp.igmpRep1.nofg
	#define	igmp_RRESV		grp.igmpRep1.resv
	#define	igmp_GRLIST	un_v3.igmpRep2.gr_list

}igmpHdr_t;	

/* IGMP Type */
#define	IGMP_MEMBERSHIP_QUERY			0x11		/* igmp group membership query */
#define	IGMP_V1_MEMBERSHIP_REPORT		0x12		/* igmp v1 membership report */
#define	IGMP_DVMRP						0x13		/* DVMRP */
#define	IGMP_PIM							0x14		/* PIM v1 */
#define	IGMP_V2_MEMBERSHIP_REPORT		0x16		/* igmp v2 membership report */
#define	IGMP_V2_LEAVE_GROUP				0x17		/* igmp v2 leave group message */
#define	IGMP_V3_MEMBERSHIP_REPORT		0x22		/* igmp v3 membership report */

/* IGMP v3 Group Record Type */
#define	IGMPV3_MODE_ISIN			0x01
#define	IGMPV3_MODE_ISEX			0x02
#define	IGMPV3_MODE_TOIN			0x03
#define	IGMPV3_MODE_TOEX			0x04
#define	IGMPV3_MODE_ALLOW		0x05
#define	IGMPV3_MODE_BLOCK		0x06

#if 0
#define	IGMP_MASK(value, nb)		((nb)>=32 ? (value) : ((1<<(nb))-1) & (value))
#define	IGMP_EXP(thresh, nbmant, nbexp, value) \
			((value) < (thresh) ? (value) : \
			((IGMP_MASK(value, nbmant) | (1<<(nbmant+nbexp))) << \
			(IGMP_MASK((value) >> (nbmant), nbexp) + (nbexp))))
#define	IGMP_QQIC(value)			IGMP_EXP(0x80, 4, 3, value)
#define	IGMP_MRC(value)			IGMP_EXP(0x80, 4, 3, value)
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef struct udpHdr_s {
	uint16	uh_sport;		/* source port */
	uint16	uh_dport;		/* destination port */
	uint16	uh_ulen;		/* udp length */
	uint16	uh_sum;			/* udp checksum */
} udpHdr_t;

typedef struct tcpHdr_s {
	uint16	th_sport;		/* source port */
	uint16	th_dport;		/* destination port */
	uint32	th_seq;			/* sequence number */
	uint32	th_ack;			/* acknowledgement number */
#ifdef _LITTLE_ENDIAN
	uint8	th_x2:4,		/* (unused) */
			th_off:4;		/* data offset */
#else
	uint8	th_off:4,		/* data offset */
			th_x2:4;		/* (unused) */
#endif
	uint8	th_flags;
#define	TH_FIN	0x01
#define	TH_SYN	0x02
#define	TH_RST	0x04
#define	TH_PUSH	0x08
#define	TH_ACK	0x10
#define	TH_URG	0x20
#define TH_ECE  0x40
#define TH_CWR  0x80
#define TH_FLAGS        (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)

	uint16	th_win;			/* window */
	uint16	th_sum;			/* checksum */
	uint16	th_urp;			/* urgent pointer */
}tcpHdr_t;

typedef struct greHdr_s {
	uint8	gre_flags_x1;
#define	GRE_C_BIT	0x80
#define	GRE_R_BIT	0x40
#define	GRE_K_BIT	0x20
#define	GRE_S_BIT	0x10
#define	GRE_s_BIT	0x08
#define	GRE_FLAGS_x1	(GRE_C_BIT|GRE_R_BIT|GRE_K_BIT|GRE_S_BIT|GRE_s_BIT)
	uint8	gre_flags_x2;
	uint16	gre_protocol;
	uint16	gre_length;
	uint16	gre_callid;
	uint32	gre_seqno;
	uint32	gre_ackno;
} greHdr_t;

//#define PPTP_GRE_PROTO          0x880b
#pragma pack(pop)

#endif

