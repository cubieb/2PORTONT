//#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/in_route.h>
#include <linux/udp.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <net/dst.h>
#include <net/route.h>
#include <asm/checksum.h>
#include <net/arp.h>
#include <linux/netfilter_ipv4.h>
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <net/ip6_route.h>
#include <net/ip6_fib.h>
#include <net/ip6_checksum.h>
#endif

#include "../include/voip_types.h"
#include "../voip_manager/voip_mgr_netfilter.h"
#include "../voip_rx/rtk_trap.h"
#include "direct_send_mgr.h"
#include "rtk_voip.h"
#include "voip_init.h"

#ifdef FEATURE_COP3_PROFILE
#include "cp3_profile.h"
#endif

#ifdef CONFIG_RTK_VOIP_SRTP
#include "srtp.h"
#endif

#ifdef SUPPORT_VOIP_DBG_COUNTER
extern uint32 gVoipCounterEnable;
extern void RTP_tx_count(uint32 sid);
#endif

#define USE_DST_OUTPUT

#if defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651) || defined (CONFIG_RTK_VOIP_DRIVERS_PCM865xC)
//#include "voip_support.h"
#endif

#ifdef SUPPORT_DSCP
extern int rtp_tos;
#endif

typedef struct {
        uint16 source;
        uint16 dest;
        uint16 len;
        uint16 check;
} Tsudphdr;

typedef struct {
#if defined(__LITTLE_ENDIAN_BITFIELD)
        uint8    ihl:4,
                  version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
        uint8    version:4,
                  ihl:4;
#endif
        uint8    tos;
        uint16   tot_len;
        uint16   id;
        uint16   frag_off;
        uint8    ttl;
        uint8    protocol;
        uint16   check;
        uint32   saddr;
        uint32   daddr;
        /*The options start here. */
} Tsiphdr;

int tx_skb_cnt=0;
extern struct RTK_TRAP_profile *header;

#if ! defined (AUDIOCODES_VOIP)
uint32 rtpConfigOK[MAX_DSP_RTK_SS_NUM] = {0};	// switch for rtp tx/rx
uint32 rtpHold[2*MAX_DSP_RTK_SS_NUM] ={0}; //0->not hold, 1->hold. Multiply 2 for RTCP used.
#else
uint32 rtpConfigOK[MAX_DSP_AC_SS_NUM] = {0};
uint32 rtpHold[MAX_DSP_AC_SS_NUM] = {0};
#endif

#ifndef AUDIOCODES_VOIP
uint32 nTxRtpStatsCountByte[MAX_DSP_RTK_SS_NUM];
uint32 nTxRtpStatsCountPacket[MAX_DSP_RTK_SS_NUM];
#endif

//RtpPacket* g_pst;
//struct RTK_TRAP_profile *g_ptr;

//===============================================================================//


#ifdef RTP_SNED_TASKLET

#define FIFO_NUM 20
struct tasklet_struct	rtp_send_tasklet;
struct RTK_TRAP_profile Rtp_send[MAX_DSP_CH_NUM][FIFO_NUM];
char Rtp_fifo[MAX_DSP_CH_NUM][FIFO_NUM*512];
static unsigned long length = sizeof(struct RTK_TRAP_profile);
static unsigned int pload_len = 0;
static unsigned char rtp_w[MAX_DSP_CH_NUM] = {0}, rtp_r[MAX_DSP_CH_NUM] = {0};
static unsigned char pload_w[MAX_DSP_CH_NUM] ={0}, pload_r[MAX_DSP_CH_NUM] = {0};

#endif

#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
static void send(struct RTK_TRAP_profile *ptr, const void *ptr_data, uint32 data_len)
{
	uint8 *tmp;
	uint32 tos;
	Tsudphdr *udphdr;
	Tsiphdr *iphdr;
	struct sk_buff *skb;
	struct rtable *rt = NULL;
	int rst;
	//int i;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30))
	struct dst_entry *dst = skb_dst(skb);
#else
	struct dst_entry *dst = skb->dst;
#endif
	struct hh_cache *hh;
	struct neighbour *n;
#ifdef SUPPORT_VOICE_QOS
	tos = ptr->tos;
#endif	
#ifdef SUPPORT_DSCP
	tos = rtp_tos;
#endif	


//	printk("enter send function\n");
	//printk("profile: ip_dst = %x, ip_src = %x\n", ptr->ip_dst_addr, ptr->ip_src_addr);
	/* ip_src_addr is destination address */
#ifdef CONFIG_RTK_VOIP_SRTP
	err_status_t stat = 0;
#ifdef FEATURE_COP3_PROFILE	  
	unsigned long flags;
	save_flags(flags); cli();
	ProfileEnterPoint(PROFILE_INDEX_TEMP);
#endif	
	/* apply srtp */
	if (ptr->applySRTP){
#ifndef AUDIOCODES_VOIP
        	//extern int rtcp_sid_offset;
        	//if((ptr->udp_dst_port == 9001) || (ptr->udp_dst_port == 9003) || (ptr->udp_dst_port == 9005) || (ptr->udp_dst_port == 9007)){
        	if(ptr->s_id >= RTCP_SID_OFFSET){
#else
		if(((ptr->s_id)%2) == 1 ){ // ACMW RTP sid = 2*CH, RTCP sid = 2*CH + 1
#endif
        		stat = srtp_protect_rtcp(ptr->tx_srtp_ctx, ptr_data, &data_len);
        	}
		else{
			stat = srtp_protect(ptr->tx_srtp_ctx, ptr_data, &data_len);
		}
	}

#ifdef FEATURE_COP3_PROFILE	  
	ProfileExitPoint(PROFILE_INDEX_TEMP);
	restore_flags(flags);
	ProfilePerDump(PROFILE_INDEX_TEMP, 1000);
#endif	
	if (stat) {
		printk("error: srtp protection failed with code %d\n", stat);
	    	return;
	}	
#endif	
	// I: .7960 (.6416)
	// D: .7806 (.2803)
	{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38))
	struct flowi4 key = { .flowi4_oif = 0, .flowi4_proto = IPPROTO_UDP, .flowi4_flags = FLOWI_FLAG_ANYSRC,
							.saddr = ptr->ip_dst_addr, .daddr = ptr->ip_src_addr,
							.flowi4_tos = (uint32)(RT_TOS(tos))};
	extern struct net init_net;
	rt = ip_route_output_key(&init_net, &key);
	if (rt == NULL)
		goto NO_ROUTE;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	struct flowi key = { .oif = 0, .proto = IPPROTO_UDP, .flags = FLOWI_FLAG_ANYSRC, .nl_u =  { .ip4_u = { .daddr = ptr->ip_src_addr,
                                                          .saddr = ptr->ip_dst_addr,
                                                          .tos = (uint32)(RT_TOS(tos)) }}};
	extern struct net init_net;
	if((rst = ip_route_output_key(&init_net, &rt, &key)))
		goto NO_ROUTE;
#else
	if((rst = ip_route_output(&rt, ptr->ip_src_addr, ptr->ip_dst_addr,(uint32)(RT_TOS(tos)), 0)))
		goto NO_ROUTE;
#endif

	}	// pkshih: avoid compiler warning 

	// I: .6438 (.4964)
	// D: .6290 (.1713)
	//skb = dev_alloc_skb(data_len + 20 + 8);
	skb = alloc_skb(data_len+4+16 + 20 + 8, GFP_ATOMIC);
	if (skb == NULL){
#if 1
		printk("%s-%s(): alloc_skb failed:(%d)\n", __FILE__, __FUNCTION__, tx_skb_cnt);
#else	
		printk("send skb_alloc return NULL. Drop it.\n");
		printk("final [%d] ", tx_skb_cnt);
		cli();
		while(1);
#endif		
		return ;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	skb->sk = (void*)1; // for NAT 
#endif

#if 0
	skb_reserve(skb,4);
	if(skb->data - skb->head >=4){
		tx_skb_cnt+=1;
		(*(int*)(skb->head)) = 0x81868711;
		//printk("[%d] ", tx_skb_cnt);
#if 0		
		if(tx_skb_cnt > 127)
		  printk("[%d] ", tx_skb_cnt);

		if(tx_skb_cnt ==500)
		{
			printk("while(1) due to tx_skb_cnt > 500\n");
			while(1);
		}
#endif		
	}
	
#endif
	skb_reserve(skb,16);

	//printk("skb_put before\n");
	tmp = skb_put(skb, data_len + 20 + 8); //tmp = skb->data
	//printk("******skb_put ok\n");

	iphdr = (Tsiphdr *)tmp;
	udphdr = (Tsudphdr *)(tmp+20);

/* ip */
	iphdr->version = 4;
	iphdr->ihl = 5;
	iphdr->tos = tos;	// TOS
#ifdef SUPPORT_QOS_REMARK_PER_SESSION	
	if(ptr->qos.dscpRemark)
		iphdr->tos = (ptr->qos.dscpRemark)<<2;
#endif	
	iphdr->tot_len = htons(data_len + 8 + 20);
	iphdr->id = 0;
	iphdr->frag_off = 0;
	iphdr->ttl = 0x40;
	iphdr->protocol = 0x11;
	iphdr->check = 0;
	iphdr->saddr = ptr->ip_dst_addr;
	iphdr->daddr = ptr->ip_src_addr;
	iphdr->check = ip_fast_csum((uint8 *)(iphdr), 5);

/* udp */
	udphdr->source = ptr->udp_dst_port;
	udphdr->dest = ptr->udp_src_port;
	udphdr->len = htons(data_len + 8);
	udphdr->check = 0;

/* rtp */	
	memcpy(tmp+28, ptr_data, data_len);

//#ifdef USE_DST_OUTPUT
#ifdef CONFIG_RTK_VOIP_QOS
extern int voip_tx_shortcut_enable;
if( !voip_tx_shortcut_enable )
#endif
{
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
//thlin, for 3.4
	skb_dst_set(skb, dst_clone(&rt->dst));
	skb->dev = skb_dst(skb)->dev;
  #elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30))
//shlee, for 2.6.32
	skb_dst_set(skb, dst_clone(&rt->u.dst));
	skb->dev = skb_dst(skb)->dev;
  #else	
	skb->dst = dst_clone(&rt->u.dst);
	skb->dev = (skb->dst)->dev;
  #endif

  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	skb_reset_network_header(skb);
  #else
	skb->nh.iph = (struct iphdr*)(skb->data);
  #endif
	// Linux default qdisc pfifo has 3 queue (0,1,2). q0 is the highest priority,
	// and skb->priority 6 and 7 will be put into queue 0.
	skb->priority = 7;
	//skb->dst->output(skb);
  #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#define NF_IP_LOCAL_OUT		3
  #endif
	// I: 4.6239 (3.4241)
	// D: 4.6088 (1.2092)
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(3,15,0))
	NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, skb, NULL, skb_dst(skb)->dev,
                   dst_output);	
	
	#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
//thlin, for 3.4	
		NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, skb, NULL, rt->dst.dev,
						 skb_dst(skb)->output); 

	
  #elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30))
//shlee, for 2.6.32
	NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, skb, NULL, rt->u.dst.dev,
                   skb_dst(skb)->output);	
  #else 
	NF_HOOK(PF_INET, NF_IP_LOCAL_OUT, skb, NULL, rt->u.dst.dev,
                   skb->dst->output);	
  #endif
     #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
	ip_rt_put(rt);
     #endif
        return;

}
//#else
#ifdef CONFIG_RTK_VOIP_QOS
else 
{
	skb->dst = &rt->u.dst;
	skb->dev = (skb->dst)->dev;

/* ethernet */
		dst = skb->dst;
		hh = (skb->dst)->hh;
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
		skb_reset_network_header(skb);
  #else
		skb->nh.raw = skb->data;
  #endif
/*
	        printk("neigh_lookup start\n");
		printk("dst->neighbour->tbl = %x\n",(dst->neighbour)->tbl);
		n = neigh_lookup(dst->neighbour->tbl, ptr->ip_src_addr, skb->dev);
		printk("n = %x\n", n);
*/

#if 1
		//printk("hh = %x\n", hh);
        	if (hh) {
        		read_lock_bh(&hh->hh_lock);
			//for(i = 0;i<4;i++)
			//printk("hh->hh_data[%d] = %08x\n", i,  hh->hh_data[i] );
			//printk("hh->hh_len =%d\n", hh->hh_len);
          		memcpy(skb->data - 16, hh->hh_data, 16);
        		read_unlock_bh(&hh->hh_lock);
        	        skb_push(skb, hh->hh_len);
			//printk("skb->dev = %x\n", skb->dev);
			//printk("skb->dev->name = %s\n", skb->dev->name);
			//printk("hh_output = %x\n", hh->hh_output);
        		//hh->hh_output(skb);
			skb->dev->hard_start_xmit(skb, skb->dev);
			//printk("hh_output\n");
        		//return hh->hh_output(skb);
        	} else if (dst->neighbour) {
			//printk("arp\n");
#if 0
			printk("dst->neighbour->output = %x\n",dst->neighbour->output);
			printk("skb->len = %x\n",skb->len);
			printk("skb->dev = %x\n", skb->dev);
			printk("skb->dev->name = %s\n", skb->dev->name);
#endif
        		dst->neighbour->output(skb);
			//printk("dst->neighbour->output\n");
        		//return dst->neighbour->output(skb);
		}
#endif
#if 0
	printk("(skb->dst)->hh = %x\n", (skb->dst)->hh);
	memcpy(tmp - 2, ((skb->dst)->hh)->hh_data, 16);
	dev_queue_xmit (skb);
#endif
	//printk("finidsh send\n");
	return;
}
#endif   /* end of CONFIG_RTK_VOIP_QOS */
//#endif  /* USE_DST_OUTPUT */

NO_ROUTE:
	//printk("ip_route_output failed rst = %d\n", rst);
	//printk("**rt = %x\n", *rt);
	//printk("RTK_TRAP info: ip_dst_addr = %x, ip_src_addr = %x \n", ptr->ip_dst_addr,  ptr->ip_src_addr);
	printk("NR ");
	//printk("TX err: %x->%x\n", ptr->ip_dst_addr,  ptr->ip_src_addr);
	return;

}

#endif //#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP


#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT

static void send6(struct RTK_TRAP_profile *ptr, const void *ptr_data, uint32 data_len)
{
	//printk("s6 ");
	
	uint8 *tmp;
	uint32 tos;
	Tsudphdr *udphdr;

	struct ipv6hdr *iphdr;
	struct rt6_info *rt6 = NULL;			/* Route to the other host */

	struct sk_buff *skb;
	int rst;

#ifdef SUPPORT_VOICE_QOS
	tos = ptr->tos;
#endif	
#ifdef SUPPORT_DSCP
	tos = rtp_tos;
#endif	

#if 0 // IPv6 Tx hard code
#if 1
	//struct in6_addr src_ip6_addr = {{.u6_addr8 = {0xfc, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0xe0, 0x4c, 0xff, 0xfe, 0x86, 0x85, 0x2}}};
	//struct in6_addr dst_ip6_addr = {{.u6_addr8 = {0xfc, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0xe0, 0x4c, 0xff, 0xfe, 0x86, 0x70, 0x5}}};
	struct in6_addr src_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x7005}};
	struct in6_addr dst_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x8502}};
#else
	//struct in6_addr src_ip6_addr = {{.u6_addr8 = {0xfe, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0xe0, 0x4c, 0xff, 0xfe, 0x86, 0x85, 0x2}}};
	//struct in6_addr dst_ip6_addr = {{.u6_addr8 = {0xfe, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0xe0, 0x4c, 0xff, 0xfe, 0x86, 0x70, 0x5}}};
	struct in6_addr src_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x8502}};
	struct in6_addr dst_ip6_addr = {.s6_addr16 = {0xfe80, 0x0, 0x0, 0x0, 0x2e0, 0x4cff, 0xfe86, 0x7005}};
#endif
#endif

#ifdef CONFIG_RTK_VOIP_SRTP
	err_status_t stat = 0;
#ifdef FEATURE_COP3_PROFILE	  
	unsigned long flags;
	save_flags(flags); cli();
	ProfileEnterPoint(PROFILE_INDEX_TEMP);
#endif	
	/* apply srtp */
	if (ptr->applySRTP){
#ifndef AUDIOCODES_VOIP
        	//extern int rtcp_sid_offset;
        	//if((ptr->udp_dst_port == 9001) || (ptr->udp_dst_port == 9003) || (ptr->udp_dst_port == 9005) || (ptr->udp_dst_port == 9007)){
        	if(ptr->s_id >= RTCP_SID_OFFSET){
#else
		if(((ptr->s_id)%2) == 1 ){ // ACMW RTP sid = 2*CH, RTCP sid = 2*CH + 1
#endif
        		stat = srtp_protect_rtcp(ptr->tx_srtp_ctx, ptr_data, &data_len);
        	}
		else{
			stat = srtp_protect(ptr->tx_srtp_ctx, ptr_data, &data_len);
		}
	}

#ifdef FEATURE_COP3_PROFILE	  
	ProfileExitPoint(PROFILE_INDEX_TEMP);
	restore_flags(flags);
	ProfilePerDump(PROFILE_INDEX_TEMP, 1000);
#endif	
	if (stat) {
		printk("error: srtp protection failed with code %d\n", stat);
	    	return;
	}	
#endif	
	// I: .7960 (.6416)
	// D: .7806 (.2803)
	{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,38))
	struct flowi6 key = { .flowi6_oif = 0, .flowi6_proto = IPPROTO_UDP, .flowi6_flags = FLOWI_FLAG_ANYSRC,
							.saddr = ptr->ip6_dst_addr, .daddr = ptr->ip6_src_addr,
							.flowi6_tos = (uint32)(RT_TOS(tos)),
							.flowlabel = 0};
	extern struct net init_net;
	rt6 = (struct rt6_info *)ip6_route_output(&init_net, NULL, &key);
	if(!rt6)
		goto NO_ROUTE;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
	struct flowi key = { .oif = 0, .proto = IPPROTO_UDP, .flags = FLOWI_FLAG_ANYSRC,
						.nl_u =  { .ip6_u = { .daddr = ptr->ip6_src_addr,
												.saddr = ptr->ip6_dst_addr,
												.flowlabel = 0 }}};
	extern struct net init_net;
	rt6 = (struct rt6_info *)ip6_route_output(&init_net, NULL, &key);
	if(!rt6)
		goto NO_ROUTE;
#else
	?? //Not test for this linux version
	struct flowi key = { .oif = 0, .proto = IPPROTO_UDP, .flags = FLOWI_FLAG_MULTIPATHOLDROUTE,
					.nl_u =  { .ip6_u = { .daddr = ptr->ip6_src_addr,
								.saddr = ptr->ip6_dst_addr,
								.flowlabel = 0 }}};

	rt6 = (struct rt6_info *) ip6_route_output(NULL, &key);
	if(!rt6)
		goto NO_ROUTE;
#endif

	}	// pkshih: avoid compiler warning 

	// I: .6438 (.4964)
	// D: .6290 (.1713)
	skb = alloc_skb(data_len+4+16 + 40 + 8, GFP_ATOMIC); //ip:40, udp:8, rtp:16, vlan:4 

	if (skb == NULL){
#if 1
		printk("%s-%s(): alloc_skb failed:(%d)\n", __FILE__, __FUNCTION__, tx_skb_cnt);
#else	
		printk("send skb_alloc return NULL. Drop it.\n");
		printk("final [%d] ", tx_skb_cnt);
		cli();
		while(1);
#endif		
		return ;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
	skb->sk = (void*)1; // for NAT 
#endif

#if 0
	skb_reserve(skb,4);
	if(skb->data - skb->head >=4){
		tx_skb_cnt+=1;
		(*(int*)(skb->head)) = 0x81868711;
		//printk("[%d] ", tx_skb_cnt);
#if 0		
		if(tx_skb_cnt > 127)
		  printk("[%d] ", tx_skb_cnt);

		if(tx_skb_cnt ==500)
		{
			printk("while(1) due to tx_skb_cnt > 500\n");
			while(1);
		}
#endif		
	}
	
#endif

	skb_reserve(skb,16);	//eth header: 6(src mac) + 6(dst mac) + 2(eth type) + 2(4-byte align)

	tmp = skb_put(skb, data_len + 40 + 8); //tmp = skb->data
	//printk("******skb_put ok\n");

	iphdr = (struct ipv6hdr *)tmp;
	udphdr = (Tsudphdr *)(tmp+40);

/* ip */
	iphdr->version = 6;
	iphdr->flow_lbl[0] = 0;
	iphdr->flow_lbl[1] = 0;
	iphdr->flow_lbl[2] = 0;
	//update tos
	iphdr->priority = ((tos&0xf0)>>4);
	iphdr->flow_lbl[0] |= ((tos&0xf)<<4);
#ifdef SUPPORT_QOS_REMARK_PER_SESSION	
	if(ptr->qos.dscpRemark)
	{
		iphdr->priority = (((ptr->qos.dscpRemark)<<2)&0xf0)>>4;
		iphdr->flow_lbl[0] |= (((ptr->qos.dscpRemark)<<2)&0xf)<<4;
	}
#endif
	iphdr->payload_len = htons(data_len + 8); //exclude IPv6 header length
	iphdr->nexthdr = IPPROTO_UDP;
	iphdr->hop_limit = 0x40;
	iphdr->saddr = ptr->ip6_dst_addr;
	iphdr->daddr = ptr->ip6_src_addr;

/* udp */
	udphdr->source = ptr->udp_dst_port;
	udphdr->dest = ptr->udp_src_port;
	udphdr->len = htons(data_len + 8);
	udphdr->check = 0;

/* rtp */	
	memcpy(tmp+48, ptr_data, data_len);
	udphdr->check = csum_ipv6_magic(&iphdr->saddr, &iphdr->daddr, udphdr->len, IPPROTO_UDP, csum_partial((char *)udphdr, udphdr->len, 0));

#ifdef USE_DST_OUTPUT

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
//thlin, for 3.4
	skb_dst_set(skb, dst_clone(&rt6->dst));
	skb->dev = skb_dst(skb)->dev;
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30))
//shlee, for 2.6.32
	skb_dst_set(skb, dst_clone(&rt6->u.dst));
	skb->dev = skb_dst(skb)->dev;
#else	
	skb->dst = dst_clone(&rt6->u.dst);
	skb->dev = (skb->dst)->dev;
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
	skb_reset_network_header(skb);
#else
	skb->nh.iph = (struct iphdr*)(skb->data);
#endif

	// Linux default qdisc pfifo has 3 queue (0,1,2). q0 is the highest priority,
	// and skb->priority 6 and 7 will be put into queue 0.
	skb->priority = 7;
	//skb->dst->output(skb);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#define NF_IP_LOCAL_OUT		3
#endif

	// I: 4.6239 (3.4241)
	// D: 4.6088 (1.2092)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35))
//thlin, for 3.4
	NF_HOOK(PF_INET6, NF_IP_LOCAL_OUT, skb, NULL, rt6->dst.dev,
                   skb_dst(skb)->output);	
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30))
//shlee, for 2.6.32
	NF_HOOK(PF_INET6, NF_IP_LOCAL_OUT, skb, NULL, rt6->u.dst.dev,
                   skb_dst(skb)->output);	
#else 
	NF_HOOK(PF_INET6, NF_IP_LOCAL_OUT, skb, NULL, rt6->u.dst.dev,
                   skb->dst->output);	
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0))
	ip6_rt_put(rt6);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,30))
	dst_release(skb_dst(skb));
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
	dst_release(skb->dst);
#endif

	return;
        
#else //!USE_DST_OUTPUT

	skb->dst = &rt->u.dst;
	skb->dev = (skb->dst)->dev;

/* ethernet */
		dst = skb->dst;
		hh = (skb->dst)->hh;
  #if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,21))
		skb_reset_network_header(skb);
  #else
		skb->nh.raw = skb->data;
  #endif
/*
	        printk("neigh_lookup start\n");
		printk("dst->neighbour->tbl = %x\n",(dst->neighbour)->tbl);
		n = neigh_lookup(dst->neighbour->tbl, ptr->ip_src_addr, skb->dev);
		printk("n = %x\n", n);
*/

#if 1
		//printk("hh = %x\n", hh);
        	if (hh) {
        		read_lock_bh(&hh->hh_lock);
			//for(i = 0;i<4;i++)
			//printk("hh->hh_data[%d] = %08x\n", i,  hh->hh_data[i] );
			//printk("hh->hh_len =%d\n", hh->hh_len);
          		memcpy(skb->data - 16, hh->hh_data, 16);
        		read_unlock_bh(&hh->hh_lock);
        	        skb_push(skb, hh->hh_len);
			//printk("skb->dev = %x\n", skb->dev);
			//printk("skb->dev->name = %s\n", skb->dev->name);
			//printk("hh_output = %x\n", hh->hh_output);
        		//hh->hh_output(skb);
			skb->dev->hard_start_xmit(skb, skb->dev);
			//printk("hh_output\n");
        		//return hh->hh_output(skb);
        	} else if (dst->neighbour) {
			//printk("arp\n");
#if 0
			printk("dst->neighbour->output = %x\n",dst->neighbour->output);
			printk("skb->len = %x\n",skb->len);
			printk("skb->dev = %x\n", skb->dev);
			printk("skb->dev->name = %s\n", skb->dev->name);
#endif
        		dst->neighbour->output(skb);
			//printk("dst->neighbour->output\n");
        		//return dst->neighbour->output(skb);
		}
#endif
#if 0
	printk("(skb->dst)->hh = %x\n", (skb->dst)->hh);
	memcpy(tmp - 2, ((skb->dst)->hh)->hh_data, 16);
	dev_queue_xmit (skb);
#endif
	//printk("finidsh send\n");
	return;
#endif  /* USE_DST_OUTPUT */

NO_ROUTE:
	//printk("ip_route_output failed rst = %d\n", rst);
	//printk("**rt = %x\n", *rt);
	//printk("RTK_TRAP info: ip_dst_addr = %x, ip_src_addr = %x \n", ptr->ip_dst_addr,  ptr->ip_src_addr);
	printk("V6 NR ");
	//printk("TX err: %x->%x\n", ptr->ip_dst_addr,  ptr->ip_src_addr);
	return;

}

#endif //CONFIG_RTK_VOIP_IPV6_SUPPORT
#endif //#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP

#if defined (AUDIOCODES_VOIP)
void rtp_send_aux(uint32 ip_src_addr, uint32 ip_dst_addr, uint16 udp_src_port, uint16 udp_dst_port, void *ptr_data, uint32 data_len)
{
	struct RTK_TRAP_profile rtkTrapPrf;

	rtkTrapPrf.ip_dst_addr = ip_dst_addr;
	rtkTrapPrf.ip_src_addr = ip_src_addr;
	rtkTrapPrf.udp_dst_port = udp_dst_port;
	rtkTrapPrf.udp_src_port = udp_src_port;
	rtkTrapPrf.rtk_trap_callback = NULL;
	rtkTrapPrf.next = NULL;

	send(&rtkTrapPrf, ptr_data, data_len);
}
#endif
//====================================================================================//

#ifdef RTP_SNED_TASKLET

void rtp_send_2(unsigned long *dummy)
{	
	unsigned char chid;
	unsigned long flags;
        //unsigned char rtp_w_now[MAX_VOIP_CH_NUM], pload_w_now[MAX_VOIP_CH_NUM];
	//static unsigned char cnt=0;
	
	for (chid=0; chid < DSP_CH_NUM; chid++)
	{
		//rtp_w_now[chid]= rtp_w[chid];
		//pload_w_now[chid]=pload_w[chid];
		
		//printk("%s-%d\n",__FUNCTION__, __LINE__);	
		//if ( (rtp_r[chid] == rtp_w_now[chid]) && (pload_r[chid] == pload_w_now[chid]) )
		//	printk("RTP SNED & Payload FIFO Empty\n");
		//else
		//{	
			//while (!((rtp_r[chid] == rtp_w_now[chid]) && ( pload_r[chid] == pload_w_now[chid])))
			while (!((rtp_r[chid] == rtp_w[chid]) && ( pload_r[chid] == pload_w[chid])))
			{
				//printk("%s-%d\n",__FUNCTION__, __LINE__);	
				send(&Rtp_send[chid][rtp_r[chid]], &Rtp_fifo[chid][pload_r[chid] * 512], pload_len);
				//printk("%s-%d\n",__FUNCTION__, __LINE__);
				save_flags(flags); cli();	
				rtp_r[chid] = (rtp_r[chid] + 1)%FIFO_NUM;
				pload_r[chid] = (pload_r[chid] + 1)%FIFO_NUM;
				restore_flags(flags);
				
				if (rtp_r[chid]!=pload_r[chid])
					printk("Error!! rtp_r!=pload_r\n");
			}
			//if (cnt >= 2)
				//printk("%d ", cnt);
			//cnt=0;
		//}
	}
}

#endif



//void system_process_rtp_tx(unsigned char CH, unsigned int media_type, void *ptr_data, unsigned int data_len)
void system_process_rtp_tx(unsigned char CH, unsigned int sid, const void *ptr_data, unsigned int data_len)
//void system_process_rtp_tx( RtpPacket* pst)
{

#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	PRINT_R("%s should not be called by IPC DSP, line%d\n", __FUNCTION__, __LINE__);
#else

	struct RTK_TRAP_profile *ptr;

//	Tuint32 s_id;
	
#ifdef RTP_SNED_TASKLET


	if (in_irq()==1)// Check in interrupt or not, RFC2833 is send in pcm ISR.
	{
		if((((rtp_w[CH] + 1)%FIFO_NUM) == rtp_r[CH]) && (((pload_w[CH] + 1)%FIFO_NUM) == pload_r[CH]))
			printk("RTP SEND & Payload FIFO Full\n");
		else
		{
			memcpy(&Rtp_send[CH][rtp_w[CH]], header, length);
			rtp_w[CH] = (rtp_w[CH] + 1)%FIFO_NUM;
		
			pload_len = data_len;
			memcpy(&Rtp_fifo[CH][pload_w[CH]*512], ptr_data, pload_len);
			pload_w[CH] = (pload_w[CH] + 1)%FIFO_NUM;
		
			if (rtp_w[CH]!=pload_w[CH])
				printk("Error!! rtp_w!=pload_w\n");
	
		}
	}
	

#endif
	
//	s_id = API_GetSid(CH, sid);
	//sid = pst->sid;
	if(rtpHold[sid]==1) {
		//printk("h%d",sid);
		return;
	}
#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8651) && !defined(CONFIG_RTK_VOIP_RX)
        voip_TxRtpPkt(CH, sid, ptr_data, data_len, TYPE_NONE );
        /*thlin: RTCP tx for 8651 need to test. 2006-07-04*/
#else

	ptr = header;
	if(ptr == NULL)
	{
		PRINT_MSG("profile not exist\n");
		return;
	}
	else
	{
		while(ptr){
			//printk("j%d-%d ",sid,ptr->s_id);
			if(ptr->s_id == sid)
            {
				ptr ->tx_packets ++;
				ptr ->tx_bytes += data_len;
#ifndef AUDIOCODES_VOIP
#ifdef SUPPORT_RTCP
				//extern int rtcp_sid_offset;
				if(sid >= RTCP_SID_OFFSET)
				{
					// This is RTCP packet
					// Must keep this sid check, because RTCP sid > MAX_DSP_RTK_SS_NUM
				}
				else
#endif
				{
					nTxRtpStatsCountByte[ sid ] += data_len;	/* UDP payload is excluded */
					nTxRtpStatsCountPacket[ sid ] ++;
				}

#ifdef SUPPORT_VOIP_DBG_COUNTER
				if (gVoipCounterEnable)
					RTP_tx_count(sid);
#endif
#endif

#ifdef RTP_SNED_TASKLET
				//printk("%s-%d\n",__FUNCTION__, __LINE__);
					
				if (in_irq()==1)// Check in interrupt or not, RFC2833 is send in pcm ISR.
				{
					tasklet_hi_schedule(&rtp_send_tasklet);
				}
				else
				{
					send(ptr, ptr_data, data_len);
				}
#else
				// I: 6.2338 (4.6806)
				// D: 6.2125 (1.7386)

#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
				if (ptr->ipv6 == 0)
#endif
					send(ptr, ptr_data, data_len);
#if 1 //#ifdef CONFIG_RTK_VOIP_IPV6_SUPPORT
				else if (ptr->ipv6 == 1)
					send6(ptr, ptr_data, data_len);
#endif
#endif
				//printk("send ok system_process_rtp_tx\n");
				//printk("k%d-%d ",sid,data_len);
				return;
			}
	        ptr=ptr->next;
		} //while()

	}
	PRINT_MSG("!c_p %d %d\n", sid, rtpConfigOK[sid]);
#endif	
#endif //#ifndef CONFIG_RTK_VOIP_IPC_ARCH_IS_DSP
	return;
}


#ifdef RTP_SNED_TASKLET

void RTP_FIFO_Init(void)
{	
	unsigned char i;
	for (i=0; i < DSP_CH_NUM; i++)
	{
		rtp_w[i] = 0; rtp_r[i] = 0;
		pload_w[i] =0; pload_r[i] = 0;
	}
	printk("RTP FIFO Initialization.\n");
}
#endif


