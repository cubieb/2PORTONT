#ifndef	__FASTPATH_CORE_H__
#define	__FASTPATH_CORE_H__

#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <net/dst.h>
#include <net/route.h>

#include "../../bridge/br_private.h"
#include "./rtl_queue.h"			/* X-Queue Marco Function */
#include <net/netfilter/nf_conntrack.h>

extern unsigned int tcp_timeouts[];
extern unsigned int nf_ct_udp_timeout_stream;


#if defined(CONFIG_RTL_HARDWARE_NAT) && defined(CONFIG_RTL8676_Dynamic_ACL) && defined(CONFIG_RTL_LAYERED_DRIVER) && defined(CONFIG_RTL_LAYERED_DRIVER_L2)
#define TRAFFIC_MONITOR
#define SIP_LIMIT_CHECK
#endif

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

//#undef __MIPS16
//#define __MIPS16			__attribute__ ((mips16))


/*
	Virtual Rome Driver API & System (Light Rome Driver Simulator)
*/
//#define	__IRAM_GEN	//__attribute__	((section(".iram-gen")))
#define	ip_t		__u32
/* ---------------------------------------------------------------------------------------------------- */

#define	IFNAME_LEN_MAX		16
#define	MAC_ADDR_LEN_MAX		18
#define	ARP_TABLE_LIST_MAX		32
#define	ARP_TABLE_ENTRY_MAX	128
#define	ROUTE_TABLE_LIST_MAX	16
#define	ROUTE_TABLE_ENTRY_MAX	64
#define	NAPT_TABLE_LIST_MAX	2048
#define	NAPT_TABLE_ENTRY_MAX	2048
#define	PATH_TABLE_LIST_MAX	2048
#define	PATH_TABLE_ENTRY_MAX (NAPT_TABLE_ENTRY_MAX*2)	//(NAPT_TABLE_ENTRY_MAX * 2)  //cathy
#define	INTERFACE_ENTRY_MAX	8

#define	ETHER_ADDR_LEN		6
typedef struct ether_s {
        __u8 octet[ETHER_ADDR_LEN];
} ether_t;

#if 0
#define DEBUGP_API printk
#else
#define DEBUGP_API(format, args...)
#endif

#if 0
#define DEBUGP_PKT printk
#else
#define DEBUGP_PKT(format, args...)
#endif

#if 0
#define DEBUGP_SYS printk
#else
#define DEBUGP_SYS(format, args...)
#endif

/* ########### API #################################################################################### */
enum LR_RESULT
{
	/* Common error code */
	LR_SUCCESS = 0,						/* Function Success */
	LR_FAILED = -1,						/* General Failure, not recommended to use */
	LR_ERROR_PARAMETER = -2,				/* The given parameter error */
	LR_EXIST = -3,							/* The entry you want to add has been existed, add failed */
	LR_NONEXIST = -4,						/* The specified entry is not found */
	
	LR_NOBUFFER = -1000,					/* Out of Entry Space */
	LR_INVAPARAM = -1001,					/* Invalid parameters */
	LR_NOTFOUND = -1002,					/* Entry not found */
	LR_DUPENTRY = -1003,					/* Duplicate entry found */
};


enum ARP_FLAGS
{
	ARP_NONE = 0,
};

enum RT_FLAGS
{
	RT_NONE = 0,
};

enum SE_TYPE
{
	SE_PPPOE = 1,
	SE_PPTP = 2,
	SE_L2TP = 3,
};
enum SE_FLAGS
{
	SE_NONE = 0,
};

enum NP_FLAGS
{
	NP_NONE = 0,
};

/* ---------------------------------------------------------------------------------------------------- */
enum LR_RESULT rtl867x_addArp( ip_t ip, ether_t* mac, enum ARP_FLAGS flags );
enum LR_RESULT rtl867x_modifyArp( ip_t ip, ether_t* mac, enum ARP_FLAGS flags );
enum LR_RESULT rtl867x_delArp( ip_t ip );
enum LR_RESULT rtl867x_addRoute( ip_t ip, ip_t mask, ip_t gateway, __u8* ifname, enum RT_FLAGS flags, int type);
enum LR_RESULT rtl867x_modifyRoute( ip_t ip, ip_t mask, ip_t gateway, __u8* ifname, enum RT_FLAGS flags, int type );
enum LR_RESULT rtl867x_delRoute( ip_t ip, ip_t mask );
enum LR_RESULT rtl867x_addSession( __u8* ifname, enum SE_TYPE seType, __u32 sessionId, enum SE_FLAGS flags );
enum LR_RESULT rtl867x_delSession( __u8* ifname );

//andrew
void fastpath_notify(int event);

//cathy
enum LR_RESULT rtl867x_addNaptConnection(struct nf_conn *ct,struct nf_conntrack_tuple ori_tuple,
		struct nf_conntrack_tuple reply_tuple, enum NP_FLAGS flags, int state);

enum LR_RESULT rtl867x_delNaptConnection (struct nf_conntrack_tuple ori_tuple,
		struct nf_conntrack_tuple reply_tuple);


/* [MARCO FUNCTION] ========================================================================= */
#define	MAC2STR(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[4], \
	((unsigned char *)&addr)[5]

#define	FASTPATH_MAC2STR(mac, hbuffer) \
	do { \
		int j,k; \
		const char hexbuf[] =  "0123456789ABCDEF"; \
		for (k=0,j=0;k<MAC_ADDR_LEN_MAX && j<6;j++) { \
			hbuffer[k++]=hexbuf[(mac->octet[j]>>4)&15 ]; \
			hbuffer[k++]=hexbuf[mac->octet[j]&15     ]; \
			hbuffer[k++]=':'; \
		} \
		hbuffer[--k]=0; \
	} while(0)	/* Mac Address to String */

#define FASTPATH_ADJUST_CHKSUM_NAT_UDP(ip_mod, ip_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (chksum == 0) break; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_NPT_UDP(port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (chksum == 0) break; \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */


#define FASTPATH_ADJUST_CHKSUM_NAPT_UDP(ip_mod, ip_org, port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (chksum == 0) break; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_NAT(ip_mod, ip_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_NPT(port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */


#define FASTPATH_ADJUST_CHKSUM_NAPT(ip_mod, ip_org, port_mod, port_org, chksum) \
	do { \
		s32 accumulate = 0; \
		if (((ip_mod) != 0) && ((ip_org) != 0)){ \
			accumulate = ((ip_org) & 0xffff); \
			accumulate += (( (ip_org) >> 16 ) & 0xffff); \
			accumulate -= ((ip_mod) & 0xffff); \
			accumulate -= (( (ip_mod) >> 16 ) & 0xffff); \
		} \
		if (((port_mod) != 0) && ((port_org) != 0)){ \
			accumulate += (port_org); \
			accumulate -= (port_mod); \
		} \
		accumulate += ntohs(chksum); \
		if (accumulate < 0) { \
			accumulate = -accumulate; \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) ~accumulate); \
		} else { \
			accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
			accumulate += accumulate >> 16; \
			chksum = htons((__u16) accumulate); \
		} \
	}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_TOS(tos_mod, tos_org, chksum) \
		do { \
			s32 accumulate = 0; \
			if (tos_mod != tos_org){ \
				accumulate += (tos_org); \
				accumulate -= (tos_mod); \
			} \
			else break; \
			accumulate += ntohs(chksum); \
			if (accumulate < 0) { \
				accumulate = -accumulate; \
				accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
				accumulate += accumulate >> 16; \
				chksum = htons((__u16) ~accumulate); \
			} else { \
				accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
				accumulate += accumulate >> 16; \
				chksum = htons((__u16) accumulate); \
			} \
		}while(0)	/* Checksum adjustment */

#define FASTPATH_ADJUST_CHKSUM_TOS_UDP(tos_mod, tos_org, chksum) \
		do { \
			s32 accumulate = 0; \
			if (chksum == 0) break; \
			if (tos_mod != tos_org){ \
				accumulate += (tos_org); \
				accumulate -= (tos_mod); \
			} \
			else break; \
			accumulate += ntohs(chksum); \
			if (accumulate < 0) { \
				accumulate = -accumulate; \
				accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
				accumulate += accumulate >> 16; \
				chksum = htons((__u16) ~accumulate); \
			} else { \
				accumulate = (accumulate >> 16) + (accumulate & 0xffff); \
				accumulate += accumulate >> 16; \
				chksum = htons((__u16) accumulate); \
			} \
		}while(0)	/* Checksum adjustment */

/* ---------------------------------------------------------------------------------------------------- */
__u8 *FastPath_Route(ip_t dIp);
int FastPath_Enter(struct sk_buff *skb);
int rtl867x_clearFastPathEntry(void);
/* ---------------------------------------------------------------------------------------------------- */

struct Arp_List_Entry
{
	__u8 valid;
	ip_t ip;
	ether_t mac;
	enum ARP_FLAGS flags;
	CTAILQ_ENTRY(Arp_List_Entry) arp_link;
	CTAILQ_ENTRY(Arp_List_Entry) tqe_link;
};

struct Route_List_Entry
{
	__u8 valid;
	ip_t ip;
	ip_t mask;
	ip_t gateway;
	__u8 ifname[IFNAME_LEN_MAX];
	enum RT_FLAGS flags;
	CTAILQ_ENTRY(Route_List_Entry) route_link;
	CTAILQ_ENTRY(Route_List_Entry) tqe_link;
};


struct FP_NAPT_entry {
	ip_t intIp;
	ip_t extIp;
	ip_t remIp;
	__u32 intPort;
	__u32 extPort;
	__u32 remPort;
	__u16 protocol;
    struct nf_conn * ct;
	enum NP_FLAGS flags;
};

extern int fp_on;
static inline int FastPath_Enabled(void) {
	return fp_on;
}
#ifdef CONFIG_DSL_CODESWAP
extern int DSPInShowtime;
#endif
extern int index_dev2ch(struct net_device* dev);
extern int pppoe_fastpath(struct sk_buff *skb);
extern int fp_iproute_input(void *pSt  /*struct skbuff * */, struct iphdr *iph, __u32 *fp_dip);
//#if defined(CONFIG_PPTP) || defined(CONFIG_NET_IPIP)
int fp_iproute_output(void *pSt /*struct skbuff* */, struct iphdr *iph);
#ifdef CONFIG_PPPOE
int isPPPoEDev(const char *ifname);
#endif
//#endif//end of CONFIG_PPTP || CONFIG_NET_IPIP
extern void setSkbDst(void *pSt  /*struct skbuff * */, void *dst /*struct dst_entry * */);
extern void SetFPDst(void *pSt /*struct skbuff * */, void **dst /*struct dst_entry ** */);
extern void FastPathHoldDst(void *pSt  /*struct skbuff * */);
extern void initSkbHdr(void *pSt  /*struct skbuff * */);
extern int isDestLo(void *pSt /*struct skbuff * */);
extern int isSkbDstAssigned(void *pSt /*struct skbuff * */);
//extern void setQoSMark(void *pSt /*struct skbuff * */, unsigned int mark);
//extern void setQosDscp(struct iphdr *iph, unsigned int mdscp, unsigned int mark);
extern int isNotFromPPPItf(void *pSt /*struct skbuff * */);
//#if defined(CONFIG_PPTP) || defined(CONFIG_PPPOL2TP) || defined(CONFIG_NET_IPIP)
ip_t getNetAddrbyName(const char *ifname, ip_t dst);
//#endif//end of CONFIG_PPTP || CONFIG_PPPOL2TP || CONFIG_NET_IPIP
//#ifdef CONFIG_NET_IPIP
void * getSkbDst(void *pSt);
int isIPIPTunnelDev(void *dev);
int ipip_sanity_check(void *pskb, void *dst/* struct dst_entry * */);
//#endif//end of CONFIG_NET_IPIP
extern int ppp_proxy_output(struct sk_buff *skb);
enum LR_RESULT  fastpath_addNaptConnection(struct FP_NAPT_entry *napt, int state);
enum LR_RESULT  fastpath_delNaptConnection(struct FP_NAPT_entry *napt);
enum LR_RESULT	fastpath_updateNaptConnection(struct FP_NAPT_entry *napt, unsigned int mark, unsigned int mdscp);
extern int checkUpstreamPPP(void *pdev,unsigned int course);//for 3G
extern int FastPath_Process(void *pskb, struct iphdr *iph,struct net_bridge_port *br_port);
extern void fp_br_fdb_update( struct net_bridge_port *br_port, struct sk_buff *skb);
#ifdef CONFIG_PPPOE
extern int pppoe_proxy_output(struct sk_buff *skb, struct net_device *pdev, unsigned int course);
#endif
extern int is3G_output(struct net_device * pdev);
extern int imq_enqueue(struct sk_buff *skb);
/* --- PATH Table Structures --- */
struct Path_List_Entry
{
	__u8			valid;
	//cathy
	__u16			*protocol;
	ip_t		*in_sIp;
	__u32			*in_sPort;
	ip_t		*in_dIp;
	__u32			*in_dPort;
	ip_t		*out_sIp;
	__u32			*out_sPort;
	ip_t		*out_dIp;
	__u32			*out_dPort;
	__u8			*out_ifname;
	struct Arp_List_Entry	*arp_entry;		/* for Out-dMac */
//#ifdef CONFIG_NET_IPIP
	__u8			arp_ignore;
//#endif //end of CONFIG_NET_IPIP
	__u8			course;					/* 1:In-Bonud 2:Out-Bound */
	//struct dst_entry *dst;
	void			*dst;
	__u8			type;
	__u32			mark;	//for IP QoS, TC
	__u32			last_refresh_time;//Kevin, for refreshing nf_conn timer
	struct nf_conn * ct;
	unsigned long  add_into_asic_checked;
	__u32 pps;	/* packets per second */
	CTAILQ_ENTRY(Path_List_Entry) path_link;
	CTAILQ_ENTRY(Path_List_Entry) tqe_link;
};
extern int ip_finish_output3(struct sk_buff *skb, struct Path_List_Entry *entry_path);

void fp_monitor_timeout(unsigned long data);
unsigned int add_fastpath_to_asic(struct sk_buff *skb, struct Path_List_Entry *entry_path);
#define ip_ct_tcp_timeout_established tcp_timeouts[TCP_CONNTRACK_ESTABLISHED]
#define ip_ct_udp_timeout_stream nf_ct_udp_timeout_stream
void deref_srcIp_entry(u_int32_t refIP,__u8 refCount);


#endif	/* __FASTPATH_CORE_H__ */

