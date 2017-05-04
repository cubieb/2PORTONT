/**
*	august_zhang created on 29th. Dec. 2011.
*	for testing RTL hooks
*
**/

//#define CONFIG_RTL_HOOK_CHAIN
#ifdef CONFIG_RTL_HOOK_CHAIN

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/module.h>

//as we always arrange rtl_hook_chain.c in our driver directory.
#include "rtl.h"

//we have some bridge private struct like "net_bridge_port "
#include "../../../net/bridge/br_private.h"

#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/igmp.h>
#include <net/checksum.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
extern unsigned int brIgmpModuleIndex;
extern unsigned int br0SwFwdPortMask;

#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/icmpv6.h>

#define IPV6_HEADER_LENGTH 40
#define MLDV1_REPORT 131
#define MLDV1_DONE 132
#define MLDV2_REPORT 143

extern int mldSnoopEnabled;
#endif
#endif//end of CONFIG_RTL_IGMP_SNOOPING


#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH  	
#include "../brg_shortcut.h"  
#endif



#define CONFIG_RTL_HOOK_DBG
#ifdef CONFIG_RTL_HOOK_DBG
#define RTL_HOOK_DBG(fmt,args...)  printk("\033[1;36;40m<%s %d %s> \033[m"fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define RTL_HOOK_DBG(fmt,args...)
#endif



enum
{
	HOOK_MAIN_SWITCH,
	HOOK_IGMP_SNOOPING = 1,
	HOOK_RTL_FASTBRG 	 = 2,

	
};
#define RTL_HOOK_PROC_DIR_NAME "rtl_hook"


#define RTL_HOOK_ON  1
#define RTL_HOOK_OFF 0

unsigned int rtl_hook_switch = 0;
#define CHECK_RTL_HOOK_ON_OFF(whichbit)  			if(RTL_HOOK_OFF == GET_RTL_HOOK_SWITCH(whichbit)) return NF_ACCEPT
#define GET_RTL_HOOK_SWITCH(whichbit)				(((rtl_hook_switch >> whichbit) & 0x1)?RTL_HOOK_ON : RTL_HOOK_OFF)

#define TURN_ON_RTL_HOOK_SWITCH(whichbit)			(rtl_hook_switch |= (0x1 << whichbit))
#define TURN_OFF_RTL_HOOK_SWITCH(whichbit)			(rtl_hook_switch &= (~(0x1 << whichbit)))

extern void ConvertMulticatIPtoMacAddr(__u32 group, unsigned char *gmac);
extern char igmp_type_check(struct sk_buff *skb, unsigned char *gmac,unsigned int *gIndex,unsigned int *moreFlag);
extern void br_update_igmp_snoop_fdb(unsigned char op, struct net_bridge *br, struct net_bridge_port *p, unsigned char *dest 
										   ,struct sk_buff *skb);
extern int ICMPv6_check(struct sk_buff *skb , unsigned char *gmac);

static unsigned int prehandle_igmp_report(unsigned int hook, struct sk_buff *skb,
				   const struct net_device *in,
				   const struct net_device *out,
				   int (*okfn)(struct sk_buff *))
{
	struct iphdr *iph;
	unsigned char proto;

	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);

	struct net_bridge *br = p->br;

	char tmpOp;
	unsigned char operation;
	unsigned char macAddr[6];

	uint32 fwdPortMask = 0;

	CHECK_RTL_HOOK_ON_OFF(HOOK_MAIN_SWITCH);

	CHECK_RTL_HOOK_ON_OFF(HOOK_IGMP_SNOOPING);

	if ( !(br->dev->flags & IFF_PROMISC) && MULTICAST_MAC(dest) 
										  && (eth_hdr(skb)->h_proto == ETH_P_IP))
	{
		iph = (struct iphdr *)skb_network_header(skb);

		proto =  iph->protocol;  
		if (proto == IPPROTO_IGMP) 
		{	
			unsigned int gIndex=0;
			unsigned int moreFlag=1;

			struct net_bridge *br = p->br;
			
			while(moreFlag)
			{
				tmpOp=igmp_type_check(skb, macAddr, &gIndex, &moreFlag);
				if(tmpOp>0)
				{
					//printk("%s:%d,macAddr is 0x%x:%x:%x:%x:%x:%x\n",__FUNCTION__,__LINE__,macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
					operation=(unsigned char)tmpOp;
					//operation :   2 - leave         1 - join
					br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
				}
			}

			rtl_igmpMldProcess(brIgmpModuleIndex, (uint8_t*)(skb_mac_header(skb)), p->port_no, (uint32_t *)&fwdPortMask);
			RTL_HOOK_DBG("the pre fwdPortMask is 0x%x\n", fwdPortMask);
			//RTL_HOOK_DBG("Handle igmp packet. Add entry in module %d\n", brIgmpModuleIndex);
		}

	}


	if(!(br->dev->flags & IFF_PROMISC) && IPV6_MULTICAST_MAC(dest) 
										&& (eth_hdr(skb)->h_proto == ETH_P_IPV6))
	{

#if defined (CONFIG_RTL_MLD_SNOOPING) 	
		struct ipv6hdr *ipv6h=NULL;

#if defined (IPV6_MCAST_TO_UNICAST)
		tmpOp = ICMPv6_check(skb , macAddr);
		if(tmpOp > 0)
		{
			operation=(unsigned char)tmpOp;
#ifdef	DBG_ICMPv6
			if( operation == 1)
				printk("icmpv6 add from frame finish\n");
			else if(operation == 2)
				printk("icmpv6 del from frame finish\n");	
#endif
			br_update_igmp_snoop_fdb(operation, br, p, macAddr,skb);
		}
#endif
		if(mldSnoopEnabled)
		{
			ipv6h = (struct ipv6hdr *)skb_network_header(skb);
			proto = re865x_getIpv6TransportProtocol(ipv6h);
			/*icmp protocol*/
			if (proto == IPPROTO_ICMPV6) 
			{	
				rtl_igmpMldProcess(brIgmpModuleIndex, skb_mac_header(skb), p->port_no, &fwdPortMask);	
			}
		}
#endif
	}


	return NF_ACCEPT;
}

static unsigned int posthandle_igmp_and_multicast(unsigned int hook, struct sk_buff *skb,
				   const struct net_device *in,
				   const struct net_device *out,
				   int (*okfn)(struct sk_buff *))
{
	unsigned char proto;

	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo  multicastFwdInfo;

	int ret = FAILED;
	//unsigned char reserved = 0;

	const unsigned char *dest = eth_hdr(skb)->h_dest;
	struct net_bridge_port *p = rcu_dereference(skb->dev->br_port);
	struct net_bridge *br = p->br;

	CHECK_RTL_HOOK_ON_OFF(HOOK_MAIN_SWITCH);
	CHECK_RTL_HOOK_ON_OFF(HOOK_IGMP_SNOOPING);

	if ( !(br->dev->flags & IFF_PROMISC) && MULTICAST_MAC(dest) 
										  && (eth_hdr(skb)->h_proto == ETH_P_IP))
	{
		struct iphdr *iph;
		iph = (struct iphdr *)skb_network_header(skb);

		proto =  iph->protocol;  

		if(iph->daddr== 0xEFFFFFFA)
		{
			/*for microsoft upnp*/
			//reserved = 1;
			return NF_ACCEPT;
		}
	
		//august: handle the IGMP Packet, wo just allow the report packet flood to wan;
		if (proto == IPPROTO_IGMP) 
		{	
			struct igmphdr *igmp_hdr;
			int ihl;
			
			ihl = (iph->ihl) * 4;

			igmp_hdr = (struct igmphdr *)((char*)iph + ihl);
			
			if(igmp_hdr && igmp_hdr->type && 
									(igmp_hdr->type   == IGMP_HOST_MEMBERSHIP_REPORT 
									|| igmp_hdr->type == IGMPV2_HOST_MEMBERSHIP_REPORT
									|| igmp_hdr->type == IGMPV3_HOST_MEMBERSHIP_REPORT))
			{
				RTL_HOOK_DBG("igmp report packet last 3 mac is %x.%x.%x ! out is %s\n",skb->data[3], skb->data[4], skb->data[5], out->name);
				//only allow the report packet pass to wan
				if(out->priv_flags & IFF_DOMAIN_WAN)
					return NF_ACCEPT;
				else
					return NF_DROP;
			}
		}

		if((proto == IPPROTO_UDP) || (proto == IPPROTO_TCP))
		{
			multicastDataInfo.ipVersion=4;
			multicastDataInfo.sourceIp[0]=  (uint32)(iph->saddr);
			multicastDataInfo.groupAddr[0]=  (uint32)(iph->daddr);
				
			ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);

			if(ret == SUCCESS)
			{
				if((1 << p->port_no) & multicastFwdInfo.fwdPortMask & br0SwFwdPortMask)
					return NF_ACCEPT;
				else
					return NF_DROP;
			}
			else
			{
				/*drop unknown multicast data*/
				if(multicastFwdInfo.unknownMCast)
					return NF_DROP;
			}
		}
	}

#if defined (CONFIG_RTL_MLD_SNOOPING)
	if(!(br->dev->flags & IFF_PROMISC) && IPV6_MULTICAST_MAC(dest) 
										&& (eth_hdr(skb)->h_proto == ETH_P_IPV6))
	{
		struct ipv6hdr *ipv6h = NULL;
		struct icmp6hdr *ptr = NULL;
		if(mldSnoopEnabled)
		{
			ipv6h  = (struct ipv6hdr *)skb_network_header(skb);
			proto =  re865x_getIpv6TransportProtocol(ipv6h);
			/*icmp protocol*/
			if (proto == IPPROTO_ICMPV6) 
			{
				ptr = (struct icmp6hdr* )((char*)ipv6h + IPV6_HEADER_LENGTH);

				if(ptr && ptr->icmp6_type && 
								(ptr->icmp6_type    == ICMPV6_MGM_REPORT 
								|| ptr->icmp6_type  == ICMPV6_MGM_REDUCTION
								|| ptr->icmp6_type  == MLDV2_REPORT))
				{
					if(out->priv_flags & IFF_DOMAIN_WAN)
						return NF_ACCEPT;
					else
						return NF_DROP;
				}	
			}
			else if ((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
			{
				multicastDataInfo.ipVersion=6;
				memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, sizeof(struct in6_addr));
				memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, sizeof(struct in6_addr));	
				ret = rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
				if(ret == SUCCESS)
				{						
					if((1 << p->port_no) & multicastFwdInfo.fwdPortMask)
						return NF_ACCEPT;
					else
						return NF_DROP;
				}
			}
		}

	}
#endif
	return NF_ACCEPT;
}


static unsigned int handle_fastbrg_learn_destif(unsigned int hook, struct sk_buff *skb,
				   const struct net_device *in,
				   const struct net_device *out,
				   int (*okfn)(struct sk_buff *))
{
	CHECK_RTL_HOOK_ON_OFF(HOOK_MAIN_SWITCH);

	CHECK_RTL_HOOK_ON_OFF(HOOK_RTL_FASTBRG);

	skb_push(skb, ETH_HLEN);

	#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
	if(skb->protocol != ETH_P_ARP)
		brgShortcutLearnDestItf(skb, skb->dev);
	#endif

	skb_pull(skb, ETH_HLEN);
	return NF_ACCEPT;
}


//Source Insight can not parse, I help it.
#define RTL_HOOK_CHAIN_OPS rtl_hook_chain_ops
static struct nf_hook_ops rtl_hook_chain_ops[] __read_mostly = {

	/*	august:
	*	The following two hooks is mainly used for checking igmpsnooping report packets; 
	*/
	{ .hook = prehandle_igmp_report,
	  .owner = THIS_MODULE,
	  .pf = PF_BRIDGE,
	  .hooknum = NF_BR_PRE_ROUTING,
	  .priority = NF_BR_PRI_NAT_DST_BRIDGED - 1, },

	{ .hook = posthandle_igmp_and_multicast,
	  .owner = THIS_MODULE,
	  .pf = PF_BRIDGE,
	  .hooknum = NF_BR_POST_ROUTING,
	  .priority = NF_BR_PRI_NAT_DST_BRIDGED - 1, },


	  
	/*	august:
	*	The following two hooks is mainly used for bridge fast path; 
	*   3ks for kevin's advice;
	*/
	{ .hook = handle_fastbrg_learn_destif,
	  .owner = THIS_MODULE,
	  .pf = PF_BRIDGE,
	  .hooknum = NF_BR_POST_ROUTING,
	  .priority = NF_BR_PRI_NAT_DST_BRIDGED - 1, },


#if 0
	/*
	*	this hook is another example for local in icmp packet,
	*	when kernel get an icmp packet, it clone it and pass it to the user space.
	*   Meanwhile , there is a corresponding daemon process which can get it by socket!
	*/
	{ .hook = rtl_handle_local_icmp,
	  .owner = THIS_MODULE,
	  .pf = PF_BRIDGE,
	  .hooknum = NF_BR_LOCAL_IN,
	  .priority = NF_BR_PRI_LAST, },
#endif
};



static int __init rtl_hook_chain_init(void)
{
	int ret;

	ret = nf_register_hooks(RTL_HOOK_CHAIN_OPS, ARRAY_SIZE(RTL_HOOK_CHAIN_OPS));
	if (ret < 0)
		return ret;

	RTL_HOOK_DBG("RTL hook chain registered\n");

	rtl_hook_proc_init();

	return 0;
}

static void __exit rtl_hook_chain_fini(void)
{
	nf_unregister_hooks(RTL_HOOK_CHAIN_OPS, ARRAY_SIZE(RTL_HOOK_CHAIN_OPS));
}





static int main_switch_proc_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
	int len = 0;

	int isOn = GET_RTL_HOOK_SWITCH(HOOK_MAIN_SWITCH);
	
	len += sprintf(page + len, "the rtl hook main switch is %s\n", isOn?"ON":"OFF");
	
	len += sprintf(page + len, "Usage: \n");
	len += sprintf(page + len, "Turn on : echo 1 > /proc/rtl_hook/main_switch\n");
	len += sprintf(page + len, "Turn off: echo 0 > /proc/rtl_hook/main_switch\n");	

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;

	return len;
}



static int32 main_switch_proc_write( struct file *filp, const char *buff, unsigned long len, void *data )
{
	char 	tmpbuf[128];
	char	*strptr;	
	
	int32 	retval;
	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr = tmpbuf;

		if(strlen(strptr)==0)
			goto errout;


		if(memcmp(strptr, "1", 1) == 0)
		{
			TURN_ON_RTL_HOOK_SWITCH(HOOK_MAIN_SWITCH);
			retval = SUCCESS;
		}
		else if(memcmp(strptr, "0", 1) == 0)
		{
			TURN_OFF_RTL_HOOK_SWITCH(HOOK_MAIN_SWITCH);
			retval = SUCCESS;
		}
		else
			retval = FAILED;

		if(retval == SUCCESS)
			printk("Write success ! \n");
		else
			printk("Write failed  ! \n");
		
	}
	else
	{
errout:
		printk("error input!\n");
		printk("Usage: \n");
		printk("Turn on : echo 1 > /proc/rtl_hook/main_switch\n");
		printk("Turn off: echo 0 > /proc/rtl_hook/main_switch\n");
	}

	return len;
}



static int igmp_proc_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
	int len = 0;

	int isOn = GET_RTL_HOOK_SWITCH(HOOK_IGMP_SNOOPING);
	
	len += sprintf(page + len, "the rtl igmpsnooping is %s\n", isOn?"ON":"OFF");
	
	len += sprintf(page + len, "Usage: \n");
	len += sprintf(page + len, "Turn on : echo 1 > /proc/rtl_hook/igmpsnooping\n");
	len += sprintf(page + len, "Turn off: echo 0 > /proc/rtl_hook/igmpsnooping\n");	

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;

	return len;
}

static int32 igmp_proc_write( struct file *filp, const char *buff, unsigned long len, void *data )
{
	char 	tmpbuf[128];
	char	*strptr;	
	
	int32 	retval;
	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr = tmpbuf;

		if(strlen(strptr)==0)
			goto errout;


		if(memcmp(strptr, "1", 1) == 0)
		{
			TURN_ON_RTL_HOOK_SWITCH(HOOK_IGMP_SNOOPING);
			retval = SUCCESS;
		}
		else if(memcmp(strptr, "0", 1) == 0)
		{
			TURN_OFF_RTL_HOOK_SWITCH(HOOK_IGMP_SNOOPING);
			retval = SUCCESS;
		}
		else
			retval = FAILED;

		if(retval == SUCCESS)
			printk("Write success ! \n");
		else
			printk("Write failed  ! \n");
		
	}
	else
	{
errout:
		printk("error input!\n");
		printk("Usage: \n");
		printk("Turn on : echo 1 > /proc/rtl_hook/igmpsnooping\n");
		printk("Turn off: echo 0 > /proc/rtl_hook/igmpsnooping\n");
	}

	return len;
}

static int fastbrg_proc_read(char *page, char **start, off_t off, 
		int count, int *eof, void *data)
{	  
	int len = 0;

	int isOn = GET_RTL_HOOK_SWITCH(HOOK_RTL_FASTBRG);
	
	len += sprintf(page + len, "the rtl fast bridge is %s\n", isOn?"ON":"OFF");
	
	len += sprintf(page + len, "Usage: \n");
	len += sprintf(page + len, "Turn on : echo 1 > /proc/rtl_hook/fast_bridge\n");
	len += sprintf(page + len, "Turn off: echo 0 > /proc/rtl_hook/fast_bridge\n");	

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;

	return len;
}

static int32 fastbrg_proc_write( struct file *filp, const char *buff, unsigned long len, void *data )
{
	char 	tmpbuf[128];
	char	*strptr;	
	
	int32 	retval;
	
	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		
		strptr = tmpbuf;

		if(strlen(strptr)==0)
			goto errout;


		if(memcmp(strptr, "1", 1) == 0)
		{
			TURN_ON_RTL_HOOK_SWITCH(HOOK_RTL_FASTBRG);
			retval = SUCCESS;
		}
		else if(memcmp(strptr, "0", 1) == 0)
		{
			TURN_OFF_RTL_HOOK_SWITCH(HOOK_RTL_FASTBRG);
			retval = SUCCESS;
		}
		else
			retval = FAILED;

		if(retval == SUCCESS)
			printk("Write success ! \n");
		else
			printk("Write failed  ! \n");
		
	}
	else
	{
errout:
		printk("error input!\n");
		printk("Usage: \n");
		printk("Turn on : echo 1 > /proc/rtl_hook/fast_bridge\n");
		printk("Turn off: echo 0 > /proc/rtl_hook/fast_bridge\n");
	}

	return len;
}


int rtl_hook_proc_init(void)
{
	int retval;

	struct proc_dir_entry *rtl_hook_proc_dir;

	struct proc_dir_entry *_hook_main_switch, *igmp_proc_entry, *fastbrg_proc_entry;
	
	if((rtl_hook_proc_dir = proc_mkdir(RTL_HOOK_PROC_DIR_NAME, NULL)) == NULL)
		return FAILED;

	_hook_main_switch = create_proc_entry("main_switch", 0, rtl_hook_proc_dir);
	if(_hook_main_switch != NULL)
	{
		_hook_main_switch->read_proc  = main_switch_proc_read;
		_hook_main_switch->write_proc = main_switch_proc_write;

		RTL_HOOK_DBG("sucessfully create proc entry for main switch\n");
		retval = SUCCESS;				
	}
	else
	{
		RTL_HOOK_DBG("can't create proc entry for main switch\n");
		retval = FAILED;
	}
	
	igmp_proc_entry = create_proc_entry("igmpsnooping", 0, rtl_hook_proc_dir);
	if(igmp_proc_entry != NULL)
	{
		igmp_proc_entry->read_proc  = igmp_proc_read;
		igmp_proc_entry->write_proc = igmp_proc_write;

		RTL_HOOK_DBG("sucessfully create proc entry for igmpsnooping\n");
		retval = SUCCESS;				
	}
	else
	{
		RTL_HOOK_DBG("can't create proc entry for igmpsnooping\n");
		retval = FAILED;
	}

	fastbrg_proc_entry = create_proc_entry("fast_bridge", 0, rtl_hook_proc_dir);
	if(igmp_proc_entry != NULL)
	{
		fastbrg_proc_entry->read_proc  = fastbrg_proc_read;
		fastbrg_proc_entry->write_proc = fastbrg_proc_write;

		RTL_HOOK_DBG("sucessfully create proc entry for fastbrg_proc_read\n");
		retval = SUCCESS;				
	}
	else
	{
		RTL_HOOK_DBG("can't create proc entry for fastbrg_proc_read\n");
		retval = FAILED;
	}

	return retval;				
}


module_init(rtl_hook_chain_init);
module_exit(rtl_hook_chain_fini);





#endif //end of CONFIG_RTL_HOOK_CHAIN


