/*
 *	Forwarding decision
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/netfilter_bridge.h>
#include "br_private.h"
#include <net/rtl/rtl_alias.h>


#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH  	
#include "../../drivers/net/brg_shortcut.h"  
#endif
#if defined (CONFIG_RTL_IGMP_SNOOPING)
#include <linux/ip.h>
#include <linux/in.h>
#if defined (CONFIG_RTL_MLD_SNOOPING)
#include <linux/ipv6.h>
#include <linux/in6.h>
#endif
#include <linux/igmp.h>
#include <net/checksum.h>
#include <net/rtl/rtl865x_igmpsnooping_glue.h>
#include <net/rtl/rtl865x_igmpsnooping.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>
#ifdef CONFIG_RTL_MULTI_ETH_WAN
#include <linux/if_smux.h>
#endif

extern int igmpsnoopenabled;
extern unsigned int brIgmpModuleIndex;
extern unsigned int br0SwFwdPortMask;
#if defined (CONFIG_RTL_MLD_SNOOPING)
extern int mldSnoopEnabled;
#endif
#endif


//static inline int br_flood_check_portmap(struct sk_buff *skb, struct net_bridge_port *p);

/* Don't forward packets to originating port or forwarding diasabled */
static inline int should_deliver(const struct net_bridge_port *p,
				 const struct sk_buff *skb)
{
#if 1 // WAN2WAN_BLOCKING
	/*linux-2.6.19*/
	if (skb->dev == p->dev ||
	    p->state != BR_STATE_FORWARDING ||
	    // Kaohj, not bridged between WAN connections [TR-068, I-122]
	    // not bridged between virtual ELAN, we have only one NIC port
	    ((skb->dev->priv_flags & IFF_DOMAIN_WAN) &&
	    (p->dev->priv_flags & IFF_DOMAIN_WAN))
	#ifndef CONFIG_RTL_MULTI_LAN_DEV	
		 ||((skb->dev->priv_flags & IFF_DOMAIN_ELAN) &&
	    (p->dev->priv_flags & IFF_DOMAIN_ELAN))
	#endif
	    )
		return 0;
		// maybe this is better
		// (skb->dev->priv_flags ^ p->dev->priv_flags) == 0 then forward, else block 
	#if defined (CONFIG_NET_WIRELESS) || defined (CONFIG_WIRELESS)
	// Kaohj --- check for wireless bridge blocking
	//printk("wlan_blocking=%d\n", p->br->wlan_blocking);
	if (p->br->wlan_blocking != WLAN_BLOCK_DISABLE) {
		if (p->br->wlan_blocking == WLAN_BLOCK_ALL) {
			if ((skb->dev->priv_flags & IFF_DOMAIN_WLAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WLAN))
				return 0;
		}
		else if (p->br->wlan_blocking == WLAN_BLOCK_ELAN) {
			if (((skb->dev->priv_flags & IFF_DOMAIN_WLAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WLAN)) &&
				((skb->dev->priv_flags & IFF_DOMAIN_ELAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_ELAN)))
				return 0;
		}
		else if (p->br->wlan_blocking == WLAN_BLOCK_WAN) {
			if (((skb->dev->priv_flags & IFF_DOMAIN_WLAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WLAN)) &&
				((skb->dev->priv_flags & IFF_DOMAIN_WAN) ||
				(p->dev->priv_flags & IFF_DOMAIN_WAN)))
				return 0;
		}
	}

	if (p->br->mbssid_blocking != WLAN_BLOCK_DISABLE) {
		if (p->br->mbssid_blocking == WLAN_BLOCK_MBSSID) {
			if ((skb->dev->priv_flags & IFF_DOMAIN_WLAN) &&
				(p->dev->priv_flags & IFF_DOMAIN_WLAN)){
				return 0;
			}
		}
	}
#endif // CONFIG_NET_WIRELESS
	return 1;
#else
	return (skb->dev != p->dev && p->state == BR_STATE_FORWARDING);
#endif
}

static inline unsigned packet_length(const struct sk_buff *skb)
{
	return skb->len - (skb->protocol == htons(ETH_P_8021Q) ? VLAN_HLEN : 0);
}

int br_dev_queue_push_xmit(struct sk_buff *skb)
{
	/* drop mtu oversized packets except gso */
	if (packet_length(skb) > skb->dev->mtu && !skb_is_gso(skb))
		kfree_skb(skb);
	else {
		/* ip_refrag calls ip_fragment, doesn't copy the MAC header. */
		if (nf_bridge_maybe_copy_header(skb))
			kfree_skb(skb);
		else {
			skb_push(skb, ETH_HLEN);


#ifndef CONFIG_NEW_PORTMAPPING
			/* For Portmapping */
			#ifdef CONFIG_RTL_MULTI_ETH_WAN
			
			if ( skb->dev && 
            alias_name_are_eq(skb->dev->name,ALIASNAME_NAS,ALIASNAME_MWNAS)
			//(skb->dev->name[0] == 'n') 		
			&& skb->from_dev && (skb->from_dev->priv_flags&(IFF_DOMAIN_ELAN|IFF_DOMAIN_WLAN)))//which is smux device				 

			{
				if (!smuxUpstreamPortmappingCheck(skb)) {
					kfree_skb(skb);
					return 0;
				}
			}
			#endif
#endif			

#ifndef CONFIG_RTL_HOOK_CHAIN
			// Kevin
			#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
			if(skb->protocol != ETH_P_ARP) //Kevin, there exists unicast arp request , it should not add into bridge shortcut 
										//ref: http://serverfault.com/questions/81651/strange-why-does-linux-respond-to-ping-with-arp-request-after-last-ping-reply
				brgShortcutLearnDestItf(skb, skb->dev);
			#endif
#endif
			dev_queue_xmit(skb);
		}
	}

	return 0;
}

int br_forward_finish(struct sk_buff *skb)
{
	return NF_HOOK(PF_BRIDGE, NF_BR_POST_ROUTING, skb, NULL, skb->dev,
		       br_dev_queue_push_xmit);

}

static void __br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	skb->dev = to->dev;

	NF_HOOK(PF_BRIDGE, NF_BR_LOCAL_OUT, skb, NULL, skb->dev,
			br_forward_finish);
}

static void __br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	struct net_device *indev;

	if (skb_warn_if_lro(skb)) {
		kfree_skb(skb);
		return;
	}

	indev = skb->dev;
	skb->dev = to->dev;
	skb_forward_csum(skb);

	NF_HOOK(PF_BRIDGE, NF_BR_FORWARD, skb, indev, skb->dev,
			br_forward_finish);
}

/* called with rcu_read_lock */
void br_deliver(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_deliver(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called with rcu_read_lock */
void br_forward(const struct net_bridge_port *to, struct sk_buff *skb)
{
	if (should_deliver(to, skb)) {
		__br_forward(to, skb);
		return;
	}

	kfree_skb(skb);
}

/* called under bridge lock */
static void br_flood(struct net_bridge *br, struct sk_buff *skb,
	void (*__packet_hook)(const struct net_bridge_port *p,
			      struct sk_buff *skb))
{
	struct net_bridge_port *p;
	struct net_bridge_port *prev;

	// Kevin
    #ifdef CONFIG_RTL8672_BRIDGE_FASTPATH  	
    int dir=DIR_LAN;
	if (skb->dev) {
		if (skb->dev->priv_flags & IFF_DOMAIN_WAN)//wan
			dir = DIR_WAN;		
		brgEntryDelete(eth_hdr(skb)->h_source,eth_hdr(skb)->h_dest,dir);   
	}
    #endif

	prev = NULL;

	list_for_each_entry_rcu(p, &br->port_list, list) {

		//AUG_DBG("p->dev->name: %s. p->dev->priv_flags: 0x%x\n", p->dev->name, p->dev->priv_flags);
		
		/*linux-2.6.19*/
		// Kaohj --- block multicast traffic from pvc to pvc
		// from_dev: put at br2684.c->br2684_push() to keep the device info we are arriving on
		// For routing case. The multicast source comes from pvc0(routing interface) and will
		// be flooded to pvc1(bridging interface).
		// Purpose: preventing multicast traffic from WAN (pvc) to another WAN (pvc)
		if (skb->from_dev && (skb->from_dev->priv_flags &
		 p->dev->priv_flags & IFF_DOMAIN_WAN)) {
		 	// Mason Yu		 	
		 	continue;
		}
//#ifdef CONFIG_NEW_PORTMAPPING
		//{
			//if(0 == br_flood_check_portmap(skb, p))
				//continue;
		//}
//#endif

		if (should_deliver(p, skb)) {
			if (prev != NULL) {
				struct sk_buff *skb2;

				if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
					br->dev->stats.tx_dropped++;
					kfree_skb(skb);
					return;
				}

				__packet_hook(prev, skb2);
			}

			prev = p;
		}
	}

	if (prev != NULL) {
		__packet_hook(prev, skb);
		return;
	}

	kfree_skb(skb);
}


/* called with rcu_read_lock */
void br_flood_deliver(struct net_bridge *br, struct sk_buff *skb)
{
	br_flood(br, skb, __br_deliver);
}

/* called under bridge lock */
void br_flood_forward(struct net_bridge *br, struct sk_buff *skb)
{
	br_flood(br, skb, __br_forward);
}

#if defined (CONFIG_RTL_IGMP_SNOOPING)
int bitmask_to_id(unsigned char val)
{
	int i;
	for (i=0; i<8; i++) {
		if (val & (1 <<i))
			break;
	}

	if(i>=8)
	{
		i=7;
	}
	return (i);
}

static void br_multicast(struct net_bridge *br, unsigned int fwdPortMask, struct sk_buff *skb, int clone, 
		  void (*__packet_hook)(struct net_bridge_port *p, struct sk_buff *skb))
{
//	char i;
	struct net_bridge_port *prev;
	struct net_bridge_port *p, *n;
	unsigned short port_bitmask=0;
        if (clone) {
                struct sk_buff *skb2;
                                                                                                                                              
                if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
                        br->dev->stats.tx_dropped++;
                        return;
                }
                                                                                                                                              
                skb = skb2;
        }
                                                                                                                                              
	prev = NULL;
	
	list_for_each_entry_safe(p, n, &br->port_list, list) {
		port_bitmask = (1 << p->port_no);
                if ((port_bitmask & fwdPortMask) && should_deliver(p, skb)) {
                        if (prev != NULL) {
                                struct sk_buff *skb2;
                                                                                                                                 
                                if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) {
                                        br->dev->stats.tx_dropped++;
                                        kfree_skb(skb);
                                        return;
                                }
                                        
                                __packet_hook(prev, skb2);
                        }
                                                                                                                       
                        prev = p;
                }
	}

        if (prev != NULL) {
                __packet_hook(prev, skb);
                return;
        }

	kfree_skb(skb);
}

void br_multicast_deliver(struct net_bridge *br, unsigned int fwdPortMask, struct sk_buff *skb, int clone)
{
	br_multicast(br, fwdPortMask, skb, clone, __br_deliver);
}
void br_multicast_forward(struct net_bridge *br, unsigned int fwdPortMask, struct sk_buff *skb, int clone)
{
	br_multicast(br, fwdPortMask, skb, clone, __br_forward);
}

extern struct net_bridge *bridge0;
extern int ipMulticastFastFwd;
extern int needCheckMfc;
#if defined (CONFIG_IP_MROUTE)
extern int rtl865x_checkMfcCache(struct net *net,__be32 origin,__be32 mcastgrp);
#endif

#if defined(CONFIG_RTL_MLD_SNOOPING)
extern int re865x_getIpv6TransportProtocol(struct ipv6hdr* ipv6h);
#endif
#if 0
int rtl865x_ipMulticastFastFwd(struct sk_buff *skb)
{
	const unsigned char *dest = NULL;
	unsigned char *ptr;
	struct iphdr *iph=NULL;
	unsigned char proto=0;
	unsigned char reserved=0;
	int ret=-1;
	
	struct net_bridge_port *prev;
	struct net_bridge_port *p, *n;
	struct rtl_multicastDataInfo multicastDataInfo;
	struct rtl_multicastFwdInfo multicastFwdInfo;
	struct sk_buff *skb2;
	
	unsigned short port_bitmask=0;
	#if defined (CONFIG_RTL_MLD_SNOOPING)
	struct ipv6hdr * ipv6h=NULL;
	#endif
	unsigned int fwdCnt;
	
	/*check fast forward enable or not*/
	if(ipMulticastFastFwd==0)
	{
		return -1;
	}
	
	/*check dmac is multicast or not*/
	dest=eth_hdr(skb)->h_dest;
	if((dest[0]&0x01)==0)
	{
		return -1;
	}
	
	//printk("%s:%d,dest is 0x%x-%x-%x-%x-%x-%x\n",__FUNCTION__,__LINE__,dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]);
	if(igmpsnoopenabled==0)
	{
		return -1;
	}
	
	/*check bridge0 exist or not*/
	if((bridge0==NULL) ||(bridge0->dev->flags & IFF_PROMISC))
	{
		return -1;
	}

	if((skb->dev==NULL) ||(strncmp(skb->dev->name,RTL_PS_BR0_DEV_NAME,3)==0))
	{
		return -1;
	}

	/*check igmp snooping enable or not, and check dmac is ipv4 multicast mac or not*/
	if  ((dest[0]==0x01) && (dest[1]==0x00) && (dest[2]==0x5e))
	{
		//printk("%s:%d,skb->dev->name is %s\n",__FUNCTION__,__LINE__,skb->dev->name );
		ptr=(unsigned char *)eth_hdr(skb) + ETH_HLEN;
		/*check vlan tag exist or not*/
		if(*(int16 *)(ptr)==(int16)htons(0x8100))
		{
			ptr=ptr+4;
		}

		/*check it's ipv4 packet or not*/
		if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IP))
		{
			return -1;
		}
		
		iph=(struct iphdr *)(ptr+2);
	
		if(iph->daddr== 0xEFFFFFFA)
		{
			/*for microsoft upnp*/
			reserved=1;
		}
		
		/*only speed up udp and tcp*/
		proto =  iph->protocol;  
		//printk("%s:%d,proto is %d\n",__FUNCTION__,__LINE__,proto);
		 if(((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP)) && (reserved ==0))
		{
			
			#if defined (CONFIG_IP_MROUTE)
			//august: first check the skb->dev is a route wan device
			if((skb->dev->br_port == NULL) && (skb->dev->priv_flags & IFF_DOMAIN_WAN) && needCheckMfc )
			{
				/*multicast data comes from wan, need check multicast forwardig cache*/
				if(rtl865x_checkMfcCache(&init_net,iph->saddr,iph->daddr)!=0)
				{
					return -1;
				}

			}
			#endif
			
			multicastDataInfo.ipVersion=4;
			multicastDataInfo.sourceIp[0]=  (unsigned int)(iph->saddr);
			multicastDataInfo.groupAddr[0]=  (unsigned int)(iph->daddr);
			
			ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			//printk("%s:%d,ret is %d\n",__FUNCTION__,__LINE__,ret);
			if(ret!=0)
			{

				return -1;
			}


			//printk("%s:%d,br0SwFwdPortMask is 0x%x,multicastFwdInfo.fwdPortMask is 0x%x\n",__FUNCTION__,__LINE__,br0SwFwdPortMask,multicastFwdInfo.fwdPortMask);
			#if defined (CONFIG_RTL_HARDWARE_MULTICAST)
			if((skb->srcVlanId!=0) && (skb->srcPort!=0xFFFF))
			{
				/*multicast data comes from ethernet port*/
				if( (br0SwFwdPortMask & multicastFwdInfo.fwdPortMask)==0)
				{
					/*hardware forwarding ,let slow path handle packets trapped to cpu*/
					return -1;
				}
			}
			#endif
			
			skb_push(skb, ETH_HLEN);
			
			prev = NULL;
			fwdCnt=0;
			list_for_each_entry_safe(p, n, &bridge0->port_list, list) 
			{
				port_bitmask = (1 << p->port_no);
				if ((port_bitmask & multicastFwdInfo.fwdPortMask) && (skb->dev != p->dev && p->state == BR_STATE_FORWARDING)
#ifdef CONFIG_NEW_PORTMAPPING
							&& ( skb->dev->br_port 	&& ( skb->dev->br_port->fgroup & port_bitmask ))
#endif
																	) 
				{
					if (prev != NULL) 
					{                                                                                       
						if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) 
						{
							bridge0->dev->stats.tx_dropped++;
							kfree_skb(skb);
							return 0;
						}
						skb2->dev=prev->dev;
						//printk("%s:%d,prev->dev->name is %s\n",__FUNCTION__,__LINE__,prev->dev->name);
						#if defined(CONFIG_COMPAT_NET_DEV_OPS)
						prev->dev->hard_start_xmit(skb2, prev->dev);
						#else
						prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
						#endif                  
						fwdCnt++;
					}
						                                                                               
					prev = p;
				}
			}

			if (prev != NULL) 
			{
				skb->dev=prev->dev;
				//printk("%s:%d,prev->dev->name is %s\n",__FUNCTION__,__LINE__,prev->dev->name);
			    #if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb, prev->dev);
				#else
				prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
				#endif                            
				fwdCnt++;
			}

			if(fwdCnt==0)
			{
				/*avoid memory leak*/
				skb_pull(skb, ETH_HLEN);
				return -1;
			}
			
			return 0;

		}

	}

#if 0 //defined (CONFIG_RTL_MLD_SNOOPING)
	/*check igmp snooping enable or not, and check dmac is ipv4 multicast mac or not*/
	if  ((dest[0]==0x33) && (dest[1]==0x33) && (dest[2]!=0xff))
	{
		struct net_bridge_port *p;
		if(mldSnoopEnabled==0)
		{
			return -1;
		}
		
		/*due to ipv6 passthrough*/
		p= rcu_dereference(skb->dev->br_port);
		if(p==NULL)
		{
			return -1;
		}
		
		//printk("%s:%d,skb->dev->name is %s\n",__FUNCTION__,__LINE__,skb->dev->name );
		ptr=(unsigned char *)eth_hdr(skb)+12;
		/*check vlan tag exist or not*/
		if(*(int16 *)(ptr)==(int16)htons(0x8100))
		{
			ptr=ptr+4;
		}

		/*check it's ipv6 packet or not*/
		if(*(int16 *)(ptr)!=(int16)htons(ETH_P_IPV6))
		{
			return -1;
		}
		
		ipv6h=(struct ipv6hdr *)(ptr+2);
		proto =  re865x_getIpv6TransportProtocol(ipv6h);
		
		//printk("%s:%d,proto is %d\n",__FUNCTION__,__LINE__,proto);
		 if((proto ==IPPROTO_UDP) ||(proto ==IPPROTO_TCP))
		{			
			multicastDataInfo.ipVersion=6;
			memcpy(&multicastDataInfo.sourceIp, &ipv6h->saddr, sizeof(struct in6_addr));
			memcpy(&multicastDataInfo.groupAddr, &ipv6h->daddr, sizeof(struct in6_addr));
			/*
			printk("%s:%d,sourceIp is %x-%x-%x-%x\n",__FUNCTION__,__LINE__,
				multicastDataInfo.sourceIp[0],multicastDataInfo.sourceIp[1],multicastDataInfo.sourceIp[2],multicastDataInfo.sourceIp[3]);
			printk("%s:%d,groupAddr is %x-%x-%x-%x\n",__FUNCTION__,__LINE__,
				multicastDataInfo.groupAddr[0],multicastDataInfo.groupAddr[1],multicastDataInfo.groupAddr[2],multicastDataInfo.groupAddr[3]);
			*/
			ret= rtl_getMulticastDataFwdInfo(brIgmpModuleIndex, &multicastDataInfo, &multicastFwdInfo);
			//printk("%s:%d,ret is %d\n",__FUNCTION__,__LINE__,ret);
			if(ret!=0)
			{
				if(multicastFwdInfo.unknownMCast)
				{
					multicastFwdInfo.fwdPortMask=0xFFFFFFFF;
				}
				else
				{
					return -1;
				}
	
			}
			
			//printk("%s:%d,multicastFwdInfo.fwdPortMask is 0x%x\n",__FUNCTION__,__LINE__,multicastFwdInfo.fwdPortMask);
			
			skb_push(skb, ETH_HLEN);

			prev = NULL;
			fwdCnt=0;
			list_for_each_entry_safe(p, n, &bridge0->port_list, list) 
			{
				port_bitmask = (1 << p->port_no);
				if ((port_bitmask & multicastFwdInfo.fwdPortMask) && (skb->dev != p->dev && p->state == BR_STATE_FORWARDING)) 
				{
					if (prev != NULL) 
					{                                                                                       
						if ((skb2 = skb_clone(skb, GFP_ATOMIC)) == NULL) 
						{
							kfree_skb(skb);
							return 0;
						}
						skb2->dev=prev->dev;
						//printk("%s:%d,prev->dev->name is %s\n",__FUNCTION__,__LINE__,prev->dev->name);
						#if defined(CONFIG_COMPAT_NET_DEV_OPS)
						prev->dev->hard_start_xmit(skb2, prev->dev);
						#else
						prev->dev->netdev_ops->ndo_start_xmit(skb2,prev->dev);
						#endif                             
						fwdCnt++;
					}
						                                                                               
					prev = p;
				}
			}

			if (prev != NULL) 
			{
				skb->dev=prev->dev;
				//printk("%s:%d,prev->dev->name is %s\n",__FUNCTION__,__LINE__,prev->dev->name);
			       #if defined(CONFIG_COMPAT_NET_DEV_OPS)
				prev->dev->hard_start_xmit(skb, prev->dev);
				#else
				prev->dev->netdev_ops->ndo_start_xmit(skb,prev->dev);
				#endif                            
				fwdCnt++;
			}
			
			if(fwdCnt==0)
			{
				//printk("%s:%d\n",__FUNCTION__,__LINE__);
				/*avoid memory leak*/
				skb_pull(skb, ETH_HLEN);
				return -1;	
			}
				
			return 0;	
		}

	}
#endif

	return -1;	
}
#endif
#endif


#if 0
#ifdef CONFIG_NEW_PORTMAPPING
/**
check wether should skb flood to the port in the portmapping association

return : 0, should continue to the next port;
		 1, go ahead;
**/
static inline int br_flood_check_portmap(struct sk_buff *skb, struct net_bridge_port *p)
{

		if(skb->switch_port && p->fgroup)
		{
			int32_t sw_num;

			uint16_t tmp_fgroup;

#ifdef CONFIG_RTL_8676HWNAT
	#ifdef	CONFIG_RTL_MULTI_ETH_WAN
			if((p->dev->priv_flags & IFF_DOMAIN_WAN) && (p->dev->priv_flags & IFF_OSMUX))
				tmp_fgroup = *(p->fgroup) >> 1;
			else
				tmp_fgroup = *(p->fgroup);
	#else
			tmp_fgroup = *(p->fgroup);
	#endif
#else		
			tmp_fgroup = *(p->fgroup);
#endif
//if( skb->switch_port[0] == 'e')//
if(alias_name_are_eq(skb->switch_port,ALIASNAME_ETH,ALIASNAME_ELAN_PREFIX))
{
#ifdef CONFIG_RTL_8676HWNAT
                           
             TOKEN_NUM(skb->switch_port,&sw_num);
             sw_num-=ORIGINATE_NUM;
            //get the 6th char of eth0.*
				//sw_num = (int32_t)((skb->switch_port)[5] - '0' - 2);//
 
#else		
                TOKEN_NUM(skb->switch_port,&sw_num);            
				//get the 8th char of eth0_sw*
				//sw_num = (int32_t)((skb->switch_port)[7] - '0');//		
#endif

}
//else if(skb->switch_port[5] == '-')//
else if(alias_name_is_eq(CMD_NCMP,skb->switch_port,ALIASNAME_WLAN0_VAP))
{
        TOKEN_NUM(skb->switch_port,&sw_num);
        sw_num+=5;
//        sw_num = 5 + (skb->switch_port)[9] - '0';//

}

			else
				sw_num = 4;

			if(!(tmp_fgroup & (1 << sw_num)))
			{
				return 0;
			}
		}

		return 1;
	
}
#endif	
#endif

