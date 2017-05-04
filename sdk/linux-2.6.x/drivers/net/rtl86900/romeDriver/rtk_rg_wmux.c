/***************************************************************************
 * File Name    : rtk_rg_wmux.c
 * Description  : wmux mean wan mux.
 ***************************************************************************/
 
#include <linux/ip.h>
#include <linux/rtnetlink.h>
#include <linux/ppp_defs.h>
#include <linux/if_pppox.h>
#include <net/rtl/rtl_alias.h>
#include <rtk_rg_internal.h>

//#ifdef CONFIG_RG_DEBUG
#include <rtk_rg_debug.h>
//#endif

#include "rtk_rg_wmux.h"


/***************************************************************************
                         Global variables 
 ***************************************************************************/
static DEFINE_RWLOCK(wmux_lock);

static LIST_HEAD(total_wmux_grp_devs_head);

static struct notifier_block wmux_notifier_block;/* = {
	.notifier_call = wmux_device_event,
};*/

/***************************************************************************
                         Function Definisions
 ***************************************************************************/
static inline struct wmux_group *list_entry_wmuxgrp(const struct list_head *le)
{
	return list_entry(le, struct wmux_group, grp_list);
}

/***************************************************************************
 * Function Name: __find_wmux_group
 * Description  : returns the wmux group of interfaces/devices from list
 * Returns      : struct wmux_group.
 ***************************************************************************/
static struct wmux_group *__find_wmux_group(const char *ifname)
{
	struct list_head *lh;
	struct wmux_group *wmux_grp;
	struct wmux_group *ret_wmux = NULL;

	read_lock(&wmux_lock);
	list_for_each(lh, &total_wmux_grp_devs_head) {
		wmux_grp = (struct wmux_group *)list_entry_wmuxgrp(lh);
		if (!strncmp(wmux_grp->real_dev->name, ifname, IFNAMSIZ)) {
			ret_wmux = wmux_grp;
			break;
		}
	}
	read_unlock(&wmux_lock);

	return ret_wmux;
} /* __find_wmux_group */

static inline struct wmux_dev_info *list_entry_wmuxdev(const struct list_head *le)
{
  return list_entry(le, struct wmux_dev_info, wdev_list);
}

/***************************************************************************
 * Function Name: __find_wmux_in_wmux_group
 * Description  : returns the wmux device from wmux group of devices 
 * Returns      : struct net_device
 ***************************************************************************/
static struct net_device *__find_wmux_in_wmux_group(
                                     struct wmux_group *wmux_grp, 
                                     const char *ifname)
{
	struct list_head *lh;
	struct wmux_dev_info * wdev = NULL;
	struct net_device    * ret_dev = NULL;

	read_lock(&wmux_lock);
	list_for_each(lh, &wmux_grp->virtual_devs_head) {
		wdev = list_entry_wmuxdev(lh);
		if(!strncmp(wdev->vdev->name, ifname, IFNAMSIZ)) {
			ret_dev = wdev->vdev;
			break;
		}
	}
	read_unlock(&wmux_lock);

	return ret_dev;
} /* __find_wmux_in_wmux_group */

static inline struct sk_buff *wmux_reorder_header(struct sk_buff *skb, struct vlan_hdr *vhdr, unsigned short vlan_tci, unsigned char keep_order)
{
	skb_pull_rcsum(skb, VLAN_HLEN);
		
	if(keep_order)	//move pointer only
	{
		skb->vlan_tci =(vlan_tci & VLAN_VID_MASK)+1;
		skb->mark =((vlan_tci >> 13)& 0x7)+1;						  
		skb->protocol=vhdr->h_vlan_encapsulated_proto;
		skb_reset_network_header(skb);
	}
	else	//memove to remove tag in skb
	{
		if(skb_cow(skb, skb_headroom(skb)) < 0)
			return NULL;
		skb->protocol=vhdr->h_vlan_encapsulated_proto;
		memmove(skb->data - ETH_HLEN, skb->data - VLAN_ETH_HLEN, 12);
		skb->mac_header += VLAN_HLEN;
	}

	return skb;
}



void _wmux_debug_recv(struct sk_buff *skb)
{
	char buf[64];
	snprintf(buf,64,"WMUX[%x] rx_dump:len=%d, dev=%s, proto=%04x",(unsigned int)skb->data&0xffff,skb->len,skb->dev->name,skb->protocol);
	if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_WMUX)
	{
		int32 show=1;
		if(rg_kernel.filter_level&RTK_RG_DEBUG_LEVEL_WMUX)
			show=_rtk_rg_trace_filter_compare(rg_db.pktHdr->skb,rg_db.pktHdr);
		if(show)
			memDump(skb->data,skb->len,buf);
	}


	if(rg_db.systemGlobal.psRxMirrorToPort0)
	{
		skb->data-=14;
		skb->len+=14;
		_rtk_rg_psRxMirrorToPort0(skb,skb->dev);
		skb->data+=14;
		skb->len-=14;
	}	
	netif_rx(skb);
}

/***************************************************************************
 * Function Name: wmux_pkt_recv
 * Description  : packet recv routine for all wmux devices from real dev.
 * Returns      : 0 on Success
 ***************************************************************************/
int wmux_pkt_recv(struct sk_buff *skb, struct net_device *dev)
{
	unsigned char *dstAddr;
	struct sk_buff *skb2=NULL;
	struct iphdr *network_header;  // ip header struct
	struct wmux_group *grp;
	struct wmux_dev_info *dev_info;
	struct wmux_dev_info *dev_info_first;
	struct list_head *lh;
	unsigned short protocol,ppp_protocol=0;
	unsigned short vlan_tci=0;
	struct vlan_hdr *vhdr=NULL;
	rtk_rg_ipStaticInfo_t *p_wanStaticInfo=NULL;
	char isTxDone = 0;
	char l3Choosed = 0;

	if(!dev) 
	{
		WMUX("null dev....free skb!");
		_rtk_rg_dev_kfree_skb_any(skb);
		return RE8670_RX_STOP_SKBNOFREE;	//the SKB had been sended or droped, kfree is no need
	}

	grp = __find_wmux_group(dev->name);
	if(!grp) 
	{
		WMUX("%s not wmux device...continue to protocol stack",dev->name);	
		WMUX("skb->protocol is %x, vlan_tci is %d",skb->protocol,skb->vlan_tci);
		WMUX("skb->dev is %s, from_dev is %s",skb->dev->name,skb->from_dev->name);
		WMUX("skb->switch_port is %s, mark is %x",skb->switch_port,skb->mark);
		return RE8670_RX_CONTINUE;
	}

	//dump_packet(skb->data,skb->len,"wmux skb1");

	skb->protocol = eth_type_trans (skb, dev);
	if(skb->protocol==htons(ETH_P_802_2))
	{
		WMUX("the ethernet type can't be recognized(maybe 802.2 LLC)...continue to protocol stack");
		return RE8670_RX_CONTINUE;
	}
	dstAddr = eth_hdr(skb)->h_dest;
	//dump_packet(skb->data,skb->len,"wmux skb2");
	//WMUX("enter=================>protocol is %x\n", skb->protocol);
	
	read_lock(&wmux_lock);

	protocol=skb->protocol;
	WMUX("protocol is %x",protocol);
	skb_reset_network_header(skb);
	if(skb->protocol == htons(ETH_P_8021Q)) 
	{
		vhdr = (struct vlan_hdr *)skb->data;
		vlan_tci = ntohs(vhdr->h_vlan_TCI);
		protocol=vhdr->h_vlan_encapsulated_proto;
		WMUX("h_vlan_encapsulated_proto is 0x%x, h_vlan_TCI is 0x%x", protocol, vlan_tci);
		skb_set_network_header(skb,VLAN_HLEN);
	}		
	//WMUX("skb->data[0] is %x",skb->data[0]);
	//WMUX("dstAddr[0] is %x",dstAddr[0]);
	/* Multicast Traffic will go on all intf.*/
	if(dstAddr[0] & 1)
	{
		WMUX("Multicast to all VLAN-matched devices...");
		dev_info_first = NULL;

		list_for_each(lh, &grp->virtual_devs_head)
		{
			dev_info = list_entry_wmuxdev(lh);

			//if dev's VLAN don't match, pass
			if (((vlan_tci&VLAN_VID_MASK) && !(dev_info->vid>=0)) ||
				((dev_info->vid>=0) && (!(vlan_tci&VLAN_VID_MASK) || ((vlan_tci&VLAN_VID_MASK)!=dev_info->vid))))
	  			continue;

			//if dev PPPoE but packet is, pass
			if (((dev_info->proto == RTK_RG_PPPoE) && (protocol != htons(ETH_P_PPP_DISC)) && (protocol != htons(ETH_P_PPP_SES))) ||
				((dev_info->proto != RTK_RG_PPPoE) && ((protocol == htons(ETH_P_PPP_DISC)) || (protocol == htons(ETH_P_PPP_SES)))))
			{
				WMUX("packet dropped on RX dev %s", dev_info->vdev->name);
				continue;
			}



			//WMUX("intf_idx=%d isWan=%d ingressPort=%d WanNetdevPort=%d",dev_info->intf_idx,rg_db.systemGlobal.interfaceInfo[dev_info->intf_idx].storedInfo.is_wan,rg_db.pktHdr->ingressPort,rg_db.systemGlobal.interfaceInfo[dev_info->intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_port_idx);
			if(rg_db.systemGlobal.interfaceInfo[dev_info->intf_idx].storedInfo.is_wan==1)
			{
				if(rg_db.pktHdr->ingressPort!= rg_db.systemGlobal.interfaceInfo[dev_info->intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_port_idx)
				{
					WMUX("wanDevice %s isn't binding at port %d, don't flood to this net_dev.",dev_info->vdev->name,rg_db.pktHdr->ingressPort);
					continue;
				}
			}

			//keep first dev_info, find next
			if(!dev_info_first) 
			{
				dev_info_first = dev_info;
				continue;
			}			
	
			skb2 = _rtk_rg_skb_copy(skb, GFP_ATOMIC);
			dev_info->stats.rx_packets++;
			dev_info->stats.rx_bytes += skb2->len;
			skb2->dev = dev_info->vdev;
			skb2->from_dev = dev_info->vdev;

			//based on proto and flag to decide VLAN tag order
			if(dev_info->proto != RTK_RG_BRIDGE && vhdr!=NULL)	//bridge don't change order and don't move pointer
			{
				if(wmux_reorder_header(skb2,vhdr,vlan_tci,dev_info->keep_order)==NULL)
					goto errout;
			}

			WMUX("send to %s(copy)!",skb2->dev->name);
			if(rg_db.systemGlobal.fwdStatistic)
				rg_db.systemGlobal.statistic.perPortCnt_ToPS[rg_db.pktHdr->ingressPort]++;			
			_wmux_debug_recv(skb2);

			
		}

		if (!dev_info_first) 	//no any matched VLAN interface
		{
			//_rtk_rg_dev_kfree_skb_any(skb);
			WMUX("send to %s!",skb->dev->name);	
			_wmux_debug_recv(skb);			
		}
		else 
		{	
			dev_info_first->stats.rx_packets++;
			dev_info_first->stats.rx_bytes += skb->len; 
			skb->dev = dev_info_first->vdev;
			skb->from_dev = dev_info_first->vdev;
			//skb->pkt_type = PACKET_HOST;
			
			//based on proto and flag to decide VLAN tag order
			if(dev_info_first->proto != RTK_RG_BRIDGE && vhdr != NULL)	//bridge don't change order and don't move pointer
			{
				WMUX("reorder! keep is %d",dev_info_first->keep_order);
				if(wmux_reorder_header(skb,vhdr,vlan_tci,dev_info_first->keep_order)==NULL)
					goto errout;
			}

			WMUX("skb->protocol is %x",skb->protocol);
			WMUX("send to %s!",skb->dev->name);	
			_wmux_debug_recv(skb);
		}	
		isTxDone = 1;		
	}
	else /* route Traffic.*/
	{
		WMUX("check matched interface...");
		dev_info_first = NULL;

		list_for_each(lh, &grp->virtual_devs_head)
		{
			dev_info = list_entry_wmuxdev(lh);

			//if dev is bridge, pass
			//if (dev_info->proto == RTK_RG_BRIDGE)
				//continue;

			//if dev's VLAN don't match, pass
			if (((vlan_tci&VLAN_VID_MASK) && !(dev_info->vid>=0)) ||
				((dev_info->vid>=0) && (!(vlan_tci&VLAN_VID_MASK) || ((vlan_tci&VLAN_VID_MASK)!=dev_info->vid))))
				continue;

			//if dev not PPPoE but packet is, pass
			if (((dev_info->proto == RTK_RG_PPPoE) && (protocol != htons(ETH_P_PPP_DISC)) && (protocol != htons(ETH_P_PPP_SES))) ||
				((dev_info->proto != RTK_RG_PPPoE) && ((protocol == htons(ETH_P_PPP_DISC)) || (protocol == htons(ETH_P_PPP_SES)))))
			{
				WMUX("packet bypassed on RX dev %s",dev_info->vdev->name);
				continue;
			}

			//20141226LUKE: keep first vlan-matching L2 WAN, if no L34 match, we choose this WAN!!
			if((!dev_info_first&&!l3Choosed)&&(dev_info->proto==RTK_RG_BRIDGE))dev_info_first = dev_info;
			if((dev_info->proto!=RTK_RG_BRIDGE)&&(!memcmp(dstAddr, dev_info->vdev->dev_addr, ETH_ALEN)))
			{
				WMUX("Routing or NAPT Wan Interface[%d]",dev_info->intf_idx);
				p_wanStaticInfo=rg_db.systemGlobal.interfaceInfo[dev_info->intf_idx].p_wanStaticInfo;
				//20141226LUKE: keep first routing WAN
				//20150129LUKE: store all matching routing WAN number
				//if((p_wanStaticInfo)&&(!p_wanStaticInfo->napt_enable))
				if((p_wanStaticInfo)&&((!p_wanStaticInfo->napt_enable)|| ( (p_wanStaticInfo->ip_version==IPVER_V6ONLY || p_wanStaticInfo->ip_version==IPVER_V4V6)&&(protocol==0x86dd)/*ipv6 always routing*/)    ))
				{
					WMUX("l3Choosed",l3Choosed);
					if(!(l3Choosed++))
						dev_info_first = dev_info;
					else
						dev_info_first = NULL;	//multi-match in L3

					if(dev_info_first!=NULL){
						WMUX("dev_info_first = intf[%d]",dev_info_first->intf_idx);
					}
						
					continue;
				}

				WMUX("more intf match VLAN+MAC...compare IP for NAPT");
				//we are more interfaces match VLAN+MAC, so compare IP address to find out which one to use
				if(protocol == htons(ETH_P_PPP_SES)) 
				{
					if(ppp_protocol == 0)
					{
						skb_set_network_header(skb,skb_network_header(skb)-skb->data+sizeof(struct pppoe_hdr)+2);
						ppp_protocol = *((unsigned short *)(skb_network_header(skb)-2));
					}
					WMUX("the protocol of PPPOE is %x",ppp_protocol);
					if (ppp_protocol != htons(PPP_IP))	//if the packet is not ip, check next intf
						continue;
				}
				//else if (protocol != htons(ETH_P_IP))	//if the packet is not ip, check next intf
				else if (!((protocol == htons(ETH_P_IP))||(protocol == htons(ETH_P_ARP))))	//if the packet is not ip/arp, check next intf
					continue;

				if(protocol == htons(ETH_P_IP))
				{
					network_header = ip_hdr(skb);
					if((p_wanStaticInfo)&&(p_wanStaticInfo->ip_addr!=ntohl(network_header->daddr)))	//IP different
						continue;
				}
				else if(protocol == htons(ETH_P_ARP))
				{
					WMUX("ARP packet DIP=%x WAN=%x",rg_db.pktHdr->ipv4Dip,p_wanStaticInfo->ip_addr);
					if((p_wanStaticInfo)&&(p_wanStaticInfo->ip_addr!=rg_db.pktHdr->ipv4Dip)) //IP different
						continue;

				}

				//L4 MATCH IP!!
				skb->dev = dev_info->vdev;
				skb->from_dev = dev_info->vdev;
				dev_info->stats.rx_packets++;
				dev_info->stats.rx_bytes += skb->len;
				skb->pkt_type = PACKET_HOST;
				//rtlglue_printf("(route) receive from %s\n", vdev->name);

				//based on proto and flag to decide VLAN tag order
				if(dev_info->proto!=RTK_RG_BRIDGE && vhdr!=NULL)
				{
					if(wmux_reorder_header(skb,vhdr,vlan_tci,dev_info->keep_order)==NULL)
						goto errout;
				}

				WMUX("send to %s!",skb->dev->name);
				_wmux_debug_recv(skb);

				isTxDone = 1;
				dev_info_first=NULL;	//since this interface match IP, dev_info_first should not match!
				break;
			}
		}

		if (dev_info_first) 
		{
			dev_info_first->stats.rx_packets++;
			dev_info_first->stats.rx_bytes += skb->len; 
			skb->dev = dev_info_first->vdev;
			skb->from_dev = dev_info_first->vdev;
			skb->pkt_type = PACKET_HOST;
			//rtlglue_printf("(route) receive from %s\n", dev_info_first->vdev->name);

			//based on proto and flag to decide VLAN tag order
			if(dev_info_first->proto!=RTK_RG_BRIDGE && vhdr!=NULL)
			{
				if(wmux_reorder_header(skb,vhdr,vlan_tci,dev_info_first->keep_order)==NULL)
					goto errout;
			}

			WMUX("send to %s!",skb->dev->name);
			_wmux_debug_recv(skb);
			
			isTxDone = 1;
		}
	}

	if(!isTxDone)
	{
		dev_info_first=NULL;
		WMUX("do flooding...");
		list_for_each(lh, &grp->virtual_devs_head)
		{
			dev_info = list_entry_wmuxdev(lh);
			//if (dev_info->proto != RTK_RG_BRIDGE)
				//continue;

			//if dev's VLAN don't match, pass
			if (((vlan_tci&VLAN_VID_MASK) && !(dev_info->vid>=0)) ||
				((dev_info->vid>=0) && (!(vlan_tci&VLAN_VID_MASK) || ((vlan_tci&VLAN_VID_MASK)!=dev_info->vid))))
				continue;

			if(l3Choosed &&((dev_info->proto==RTK_RG_BRIDGE)||((p_wanStaticInfo)&&(p_wanStaticInfo->napt_enable))||(memcmp(dstAddr, dev_info->vdev->dev_addr, ETH_ALEN))))
				continue;

			//keep first dev_info, find next
			if(!dev_info_first) 
			{
				dev_info_first = dev_info;
				continue;
			}

			skb2 = _rtk_rg_skb_copy(skb, GFP_ATOMIC);
			skb2->dev = dev_info->vdev;
			skb2->from_dev = dev_info->vdev;
			dev_info->stats.rx_packets++;
			dev_info->stats.rx_bytes += skb2->len; 
			//skb2->pkt_type = PACKET_OTHERHOST;
			skb2->pkt_type = PACKET_HOST;

			//bridge don't change order and don't move pointer
			//based on proto and flag to decide VLAN tag order
			if(dev_info->proto!=RTK_RG_BRIDGE && vhdr!=NULL)
			{
				if(wmux_reorder_header(skb2,vhdr,vlan_tci,dev_info->keep_order)==NULL)
					goto errout;
			}

			WMUX("send to %s(copy)!",skb2->dev->name);
			if(rg_db.systemGlobal.fwdStatistic)
				rg_db.systemGlobal.statistic.perPortCnt_ToPS[rg_db.pktHdr->ingressPort]++;			
			_wmux_debug_recv(skb2);

			
		}

		if (!dev_info_first) 	//no any matched VLAN Bridge interface
		{
			//_rtk_rg_dev_kfree_skb_any(skb);

			WMUX("send to %s!",skb->dev->name);			
			_wmux_debug_recv(skb);			
		}
		else 
		{
			dev_info_first->stats.rx_packets++;
			dev_info_first->stats.rx_bytes += skb->len; 
			skb->dev = dev_info_first->vdev;
			skb->from_dev = dev_info_first->vdev;
			//skb->pkt_type = PACKET_OTHERHOST;
			skb->pkt_type = PACKET_HOST;

			//bridge don't change order and don't move pointer
			//based on proto and flag to decide VLAN tag order
			if(dev_info_first->proto!=RTK_RG_BRIDGE && vhdr!=NULL)
			{
				if(wmux_reorder_header(skb,vhdr,vlan_tci,dev_info_first->keep_order)==NULL)
					goto errout;
			}

			WMUX("send to %s!",skb->dev->name);			
			_wmux_debug_recv(skb);
		}
		isTxDone = 1;
	}

errout:
	if(!isTxDone) 
	{
		WMUX("dropping packet that has wrong dest. on RX dev %s", dev->name);
		if(skb)_rtk_rg_dev_kfree_skb_any(skb);
		if(skb2)_rtk_rg_dev_kfree_skb_any(skb2);
	}

	return RE8670_RX_STOP_SKBNOFREE;	//the SKB had been sended or droped, kfree is no need
} /* wmux_pkt_recv */

/***************************************************************************
 * Function Name: wmux_dev_hard_start_xmit
 * Description  : xmit routine for all wmux devices on real dev.
 * Returns      : 0 on Success
 ***************************************************************************/
int wmux_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = wmux_dev_get_stats(dev);
	struct wmux_dev_info *dev_info;

	stats->tx_packets++; 
	stats->tx_bytes += skb->len;

	dev_info = WMUX_DEV_INFO(dev);
	skb->dev = dev_info->wmux_grp->real_dev;

	WMUX("WMUX_TX[%x]: from interface: %s",(unsigned int)skb->data&0xffff,dev->name);
	
	if (-1 == dev_info->vid) {
		skb->vlan_tci = 0;
	}
	else
	{

		WMUX("skb->dev is %s, vid is %d  pri=%d, proto is %s",skb->dev->name,dev_info->vid,dev_info->m_1p,dev_info->proto==RTK_RG_STATIC?"STATIC":dev_info->proto==RTK_RG_DHCP?"DHCP":dev_info->proto==RTK_RG_PPPoE?"PPPoE":"BRIDGE");
		if(dev_info->proto != RTK_RG_BRIDGE)
		{
#ifdef CONFIG_DEFAULTS_KERNEL_3_18
			if(dev_info->m_1p)
			{
				if(vlan_insert_tag_set_proto(skb,htons(ETH_P_8021Q),(dev_info->vid&VLAN_VID_MASK)|((dev_info->m_1p-1)<<13))==NULL)
				{
					WMUX("error when add cvlan tag\n");
					return 0;
				}			
			}
			else
			{
				if(vlan_insert_tag_set_proto(skb,htons(ETH_P_8021Q),(dev_info->vid&VLAN_VID_MASK))==NULL)
				{
					WMUX("error when add cvlan tag\n");
					return 0;
				}			
			}

#else
			//PATCH20131216:for fwdEngine, we need wmux to remarking ctag directly, not put in skb->vlan_tci
			if(vlan_put_tag(skb,(dev_info->vid&VLAN_VID_MASK))==NULL)
			{
				WMUX("error when add cvlan tag\n");
				return 0;
			}
			if(dev_info->m_1p)
			{
				if (skb->dev->features & NETIF_F_HW_VLAN_TX)
				{
					skb->vlan_tci |= ((dev_info->m_1p-1)<<13);
				}
				else
				{
					struct vlan_ethhdr *veth = (struct vlan_ethhdr *)skb->data;
					veth->h_vlan_TCI |=((dev_info->m_1p-1)<<13);
				}
			}
#endif

		}

	}	
	skb->vlan_member = dev_info->member;
	WMUX("%s,%d::dev_info->member: %x\n",__func__,__LINE__,skb->vlan_member);



#ifdef CONFIG_DEFAULTS_KERNEL_3_18
//	wmux_netdev_ops.ndo_start_xmit(skb, skb->dev);
	skb->dev->netdev_ops->ndo_start_xmit(skb, skb->dev);
#else
	skb->dev->hard_start_xmit(skb, skb->dev);
#endif



	//dev_queue_xmit(skb);

	return 0;
} /* wmux_dev_hard_start_xmit */


struct net_device_stats *wmux_dev_get_stats(struct net_device *dev)
{
  return &(WMUX_DEV_INFO(dev)->stats);
}


/***************************************************************************
 * Function Name: wmux_dev_open
 * Description  : 
 * Returns      : 0 on Success
 ***************************************************************************/
int wmux_dev_open(struct net_device *vdev)
{
	if (!(WMUX_DEV_INFO(vdev)->wmux_grp->real_dev->flags & IFF_UP))
		return -ENETDOWN;

	return 0;
} /* wmux_dev_open */

/***************************************************************************
 * Function Name: wmux_dev_stop
 * Description  : 
 * Returns      : 0 on Success
 ***************************************************************************/
int wmux_dev_stop(struct net_device *dev)
{
	return 0;
} /* wmux_dev_stop */

/***************************************************************************
 * Function Name: wmux_dev_set_mac_address
 * Description  : sets the mac for devs
 * Returns      : 0 on Success
 ***************************************************************************/
int wmux_dev_set_mac_address(struct net_device *dev, 
                             void *addr_struct_p)
{
	struct sockaddr *addr = (struct sockaddr *)(addr_struct_p);

#if 0
#ifdef CONFIG_RTL_8676HWNAT
	rtl865x_netif_t netif;
#endif
#endif
#ifdef UNIQUE_MAC_PER_DEV
	struct wmux_group *grp = NULL;
	struct wmux_dev_info *vdev_info = NULL;
	struct list_head *lh;
#endif

	if (netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	memset(dev->broadcast, 0xff, ETH_ALEN);

	return 0;
} /* wmux_dev_set_mac_address */

/***************************************************************************
 * Function Name: wmux_dev_change_mtu
 * Description  : changes mtu for dev
 * Returns      : 0 on Success
 ***************************************************************************/
int wmux_dev_change_mtu(struct net_device *vdev, int new_mtu)
{
#if 0
	const struct net_device_ops *ops = WMUX_DEV_INFO(vdev)->wmux_grp->real_dev->netdev_ops;
#endif
	
	//MTU should be smaller than real device.
	if (WMUX_DEV_INFO(vdev)->wmux_grp->real_dev->mtu < new_mtu)
		return -ERANGE;

	vdev->mtu = new_mtu;
//20151124LUKE: for multi-WAN device, every virtual device should not change real device's mtu. They could change themselves only.
#if 0
#ifdef CONFIG_DEFAULTS_KERNEL_3_18
	//if((WMUX_DEV_INFO(vdev)->wmux_grp->real_dev->netdev_ops->ndo_change_mtu) == NULL)printk("\n\nERROR!!!real_dev %s don't register ndo_change_mtu.....\n\n",WMUX_DEV_INFO(vdev)->wmux_grp->real_dev->name)
	if (ops->ndo_change_mtu)
		return ops->ndo_change_mtu(vdev, new_mtu);
#else
	WMUX_DEV_INFO(vdev)->wmux_grp->real_dev->change_mtu(vdev, new_mtu);
#endif
#endif
	return 0;
}

/***************************************************************************
 * Function Name: wmux_setup
 * Description  : inits device api
 * Returns      : None
 ***************************************************************************/
static void wmux_setup(struct net_device *new_dev)
{
	SET_MODULE_OWNER(new_dev);

	/* Make this thing known as a SMUX device */
	//new_dev->priv_flags |= IFF_OSMUX;

	new_dev->tx_queue_len = 0;

#ifdef CONFIG_DEFAULTS_KERNEL_3_18
	new_dev->destructor = free_netdev;	
#else
	new_dev->get_stats = wmux_dev_get_stats;
#ifdef CONFIG_COMPAT_NET_DEV_OPS
	/* set up method calls */
	new_dev->change_mtu = wmux_dev_change_mtu;
	new_dev->open = wmux_dev_open;
	new_dev->stop = wmux_dev_stop;
	new_dev->set_mac_address = wmux_dev_set_mac_address;
	new_dev->destructor = free_netdev;
	//new_dev->do_ioctl = wmux_dev_ioctl;
#endif

#endif

} /* wmux_setup */

/***************************************************************************
 * Function Name: wmux_transfer_operstate
 * Description  : updates the operstate of overlay device 
 * Returns      : None.
 ***************************************************************************/
static void wmux_transfer_operstate(const struct net_device *rdev, 
                                    struct net_device *vdev)
{

	if (rdev->operstate == IF_OPER_DORMANT)
		netif_dormant_on(vdev);
	else
		netif_dormant_off(vdev);

	if (netif_carrier_ok(rdev)) {
		if (!netif_carrier_ok(vdev))
			netif_carrier_on(vdev);
	} else {
		if (netif_carrier_ok(vdev))
			netif_carrier_off(vdev);
	}
} /* wmux_transfer_operstate */

/***************************************************************************
 * Function Name: wmux_register_device
 * Description  : regists new overlay device on real device & registers for 
                  packet handlers depending on the protocol types
 * Returns      : 0 on Success
 ***************************************************************************/
static struct net_device *wmux_register_device(const char *rifname, int intf_idx)
{
	struct net_device *new_dev = NULL;
	struct net_device *real_dev = NULL; 
	struct wmux_group *grp = NULL;
	struct wmux_dev_info *vdev_info = NULL;
	char nifname[IFNAMSIZ]={'\0'};
	int wmux_proto;
	int vid=-1;

	//check if interface had been created
	if(!rg_db.systemGlobal.interfaceInfo[intf_idx].valid)
		goto new_dev_invalid;

	if(rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on)
		vid=rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id;

	wmux_proto=rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type;

	snprintf(nifname,IFNAMSIZ,"%s_%d",rifname,intf_idx);

	WMUX("enter register device\n");
	real_dev = dev_get_by_name(&init_net, rifname);
	if (!real_dev) 
	{
		goto real_dev_invalid;
	}

	if (!(real_dev->flags & IFF_UP)) 
	{
		goto real_dev_invalid;
	}
#ifdef CONFIG_RTL_MULTI_ETH_WAN
#if !defined(CONFIG_OPENWRT_RG)
	real_dev->priv_flags &= ~IFF_RSMUX;	//disable SMUX when WMUX is registered.
#endif
#endif

#ifdef CONFIG_DEFAULTS_KERNEL_3_18
	new_dev = alloc_netdev(sizeof(struct wmux_dev_info), nifname, NET_NAME_UNKNOWN, wmux_setup);

#else
	new_dev = alloc_netdev(sizeof(struct wmux_dev_info), nifname, wmux_setup);
#endif

	if (new_dev == NULL)
	{
		WMUX("netdev alloc failure\n");
		goto new_dev_invalid;
	}

	//dev->netdev_ops = &rtl819x_netdev_ops;
	ether_setup(new_dev);
	//if (vid != -1)
		//new_dev->priv_flags |= IFF_VWMUX;

	new_dev->flags &= ~IFF_UP;
	new_dev->flags &= ~IFF_MULTICAST;
	new_dev->priv_flags |= IFF_DOMAIN_WAN;
	//new_dev->priv_flags |= IFF_DOMAIN_ELAN;
	//real_dev->priv_flags |= IFF_RWMUX;

	new_dev->state = (real_dev->state & 
                    ((1<<__LINK_STATE_NOCARRIER) |
                     (1<<__LINK_STATE_DORMANT))) |
                     (1<<__LINK_STATE_PRESENT);

	new_dev->mtu = real_dev->mtu;
	new_dev->type = real_dev->type;
	new_dev->hard_header_len = real_dev->hard_header_len;
#ifdef CONFIG_COMPAT_NET_DEV_OPS
	new_dev->hard_start_xmit = wmux_dev_hard_start_xmit;
	new_dev->set_mac_address = wmux_dev_set_mac_address;
#else
	new_dev->netdev_ops = &wmux_netdev_ops;
#endif
#ifdef CONFIG_DEFAULTS_KERNEL_3_18
	new_dev->ethtool_ops = &wmux_ethtool_ops;
#endif

	/* find wmux group name. if not found create all new wmux group */
	grp = __find_wmux_group(rifname);
	if (!grp) 
	{
		WMUX("allocate new group for %s..\n",rifname);
		grp = kzalloc(sizeof(struct wmux_group), GFP_KERNEL);

		if(grp) 
		{
			INIT_LIST_HEAD(&grp->virtual_devs_head);
			INIT_LIST_HEAD(&grp->grp_list);

			grp->real_dev = real_dev;

			write_lock_irq(&wmux_lock);
			list_add_tail(&grp->grp_list, &total_wmux_grp_devs_head);
			write_unlock_irq(&wmux_lock);
		}
		else 
		{
			free_netdev(new_dev);
			new_dev = NULL;
		}
	}

	if(grp && new_dev) {
		#if 0
		/* Assign default mac to bridge so that we can add it to linux bridge */
		if(wmux_proto == SMUX_PROTO_BRIDGE) 
		{
			memcpy( new_dev->dev_addr, "\xFE\xFF\xFF\xFF\xFF\xFF", ETH_ALEN );
		}
		else 
		{
		#ifdef UNIQUE_MAC_PER_DEV
			if (list_empty(&grp->virtual_devs_head))
			{
				memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
			}
			else
			{
				list_for_each(lh, &grp->virtual_devs_head)
				{
					vdev_info = list_entry_wmuxdev(lh);
					if (!memcmp(real_dev->dev_addr, vdev_info->vdev->dev_addr, ETH_ALEN))
					{
		            	mac_reused = 1;
					}
					if (LSB < vdev_info->vdev->dev_addr[5])
						LSB = vdev_info->vdev->dev_addr[5];
				}

				memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
				if (mac_reused)
				{
					//generate new mac address, real_addr mac addr increased by 1.
					new_dev->dev_addr[5] = LSB+1;
				}
			}
		#else
			memcpy(new_dev->dev_addr, real_dev->dev_addr, ETH_ALEN);
		#endif
		}
		#else
        char landev_ifname[16]="";
		struct net_device *landev;
		sprintf(landev_ifname, "%s%d",ALIASNAME_ELAN_PREFIX,ORIGINATE_NUM);
//		const char landev_ifname[16]="eth0.2";
		landev = dev_get_by_name(&init_net, landev_ifname);
		if (landev) {
			memcpy(new_dev->dev_addr, landev->dev_addr, ETH_ALEN);
			dev_put(landev);
		}
		else
			WMUX("eth0.2 not created.\n");
		#endif
	}
  
	if(grp && new_dev) 
	{
		struct net_device *ret_dev;
		/*find new wmux in wmux group if it does not exit create one*/
		if(NULL == (ret_dev=__find_wmux_in_wmux_group(grp, nifname))) 
		{
			WMUX("create new wmux in group..\n");
			vdev_info = WMUX_DEV_INFO(new_dev);
			memset(vdev_info, 0, sizeof(struct wmux_dev_info));
			//m_1p : 0~8, 0 is meaning disable
			if(vid>=0)
				vdev_info->m_1p=vid>>13;
			else
				vdev_info->m_1p=0;			
			vdev_info->wmux_grp = grp;
			vdev_info->vdev = new_dev;
			vdev_info->proto = wmux_proto;
			vdev_info->intf_idx = intf_idx;
			
			if(vid!=-1)
				vdev_info->vid = (vid&VLAN_VID_MASK);
			else
				vdev_info->vid = vid;
			//vdev_info->napt = napt;
			//vdev_info->brpppoe = brpppoe;
			vdev_info->member = 0xFFFFFFFF;	//init membership to include all interface.

			if(wmux_proto == RTK_RG_BRIDGE) 
			{
				new_dev->promiscuity = 1;
			}
			else /*if(wmux_proto == SMUX_PROTO_IPOE)*/
			{
				new_dev->flags |= IFF_MULTICAST;
			}

			if (register_netdev(new_dev)) 
			{
				WMUX("register_netdev failed\n");
				//list_del(&vdev_info->wdev_list);
				free_netdev(new_dev);
				new_dev = NULL;
			}
			else 
			{
				WMUX("success!\n");
				INIT_LIST_HEAD(&vdev_info->wdev_list);
				write_lock_irq(&wmux_lock);
				list_add_tail(&vdev_info->wdev_list, &grp->virtual_devs_head);
				write_unlock_irq(&wmux_lock);
				wmux_transfer_operstate(real_dev, new_dev);
			}
		}
		else 
		{
			WMUX("device %s already exist.\n", nifname);
			free_netdev(new_dev);
			new_dev = ret_dev;
		}
	}

	return new_dev;

real_dev_invalid:
new_dev_invalid:

	return NULL;
} /* wmux_register_device */

/***************************************************************************
 * Function Name: wmux_unregister_device
 * Description  : unregisters the wmux devices along with releasing mem.
 * Returns      : 0 on Success
 ***************************************************************************/
static int wmux_unregister_device(const char* vifname)
{
	struct net_device *vdev = NULL;
	struct net_device *real_dev = NULL;
	int ret = -EINVAL;
	struct wmux_dev_info *dev_info;

	vdev = dev_get_by_name(&init_net, vifname);

	if(vdev) 
	{
		WMUX("remove wmux dev %s\n",vifname);

		dev_info = WMUX_DEV_INFO(vdev);
		real_dev = dev_info->wmux_grp->real_dev;

		//remove from group's virtual_devs_head
		write_lock_irq(&wmux_lock);
		list_del(&dev_info->wdev_list);
		write_unlock_irq(&wmux_lock);

		if (list_empty(&dev_info->wmux_grp->virtual_devs_head)) 
		{
			//if group is empty, remove from total_wmux_grp_devs_head and free it
			write_lock_irq(&wmux_lock);
			list_del(&dev_info->wmux_grp->grp_list);
			write_unlock_irq(&wmux_lock);

			kfree(dev_info->wmux_grp);
		}

		dev_put(vdev);
		unregister_netdev(vdev);

		synchronize_net();
		dev_put(real_dev); 

		ret = 0;
	}

	return ret;
} /* wmux_unregister_device */

/***************************************************************************
 * Function Name: wmux_device_event
 * Description  : handles real device events to update overlay devs. status
 * Returns      : 0 on Success
 ***************************************************************************/
static int wmux_device_event(struct notifier_block *unused, 
                             unsigned long event, 
                             void *ptr)
{
	struct net_device *rdev = ptr;
	struct wmux_group *grp = __find_wmux_group(rdev->name);
	int flgs;
	struct list_head *lh;
	struct list_head *lhp;
	struct wmux_dev_info *dev_info;


	if (!grp)
		goto out;

	switch (event) {
		case NETDEV_CHANGE:

			/* Propagate real device state to overlay devices */
			read_lock(&wmux_lock);
			list_for_each(lh, &grp->virtual_devs_head) {
				dev_info = list_entry_wmuxdev(lh);
				if(dev_info) {
					wmux_transfer_operstate(rdev, dev_info->vdev);
				}
			}
			read_unlock(&wmux_lock);
			break;

		case NETDEV_DOWN:

			/* Put all Overlay devices for this dev in the down state too.*/
			read_lock(&wmux_lock);
			list_for_each(lh, &grp->virtual_devs_head) {
				dev_info = list_entry_wmuxdev(lh);
				if(dev_info) {
					flgs = dev_info->vdev->flags;

					if (!(flgs & IFF_UP))
						continue;

					dev_change_flags(dev_info->vdev, flgs & ~IFF_UP);
				}
			}
			read_unlock(&wmux_lock);
			break;

		case NETDEV_UP:

			/* Put all Overlay devices for this dev in the up state too.  */
			read_lock(&wmux_lock);
			list_for_each(lh, &grp->virtual_devs_head) {
				dev_info = list_entry_wmuxdev(lh);
				if(dev_info) {
					flgs = dev_info->vdev->flags;

					if (flgs & IFF_UP)
						continue;

					dev_change_flags(dev_info->vdev, flgs & IFF_UP);
				}
			}
			read_unlock(&wmux_lock);
			break;

		case NETDEV_UNREGISTER:
			
			/* Delete all Overlay devices for this dev. */
			write_lock_irq(&wmux_lock);
			list_for_each_safe(lh, lhp, &grp->virtual_devs_head) {
				dev_info = list_entry_wmuxdev(lh);
				if(dev_info) {
					/* delete by l67530 for cpu0 when reboot system. HG551c.2010/12/07 */
					//list_del(&dev_info->wdev_list);
					wmux_unregister_device(dev_info->vdev->name);
				}
			}
			write_unlock_irq(&wmux_lock);
			break;
		}

out:
  return NOTIFY_DONE;
} /* wmux_device_event */

/***************************************************************************
 * Function Name: wmux_drv_init
 * Description  : Initialization of wmux driver
 * Returns      : struct net_device
 ***************************************************************************/
int wmux_drv_init(void)
{
	wmux_notifier_block.notifier_call = wmux_device_event;
	register_netdevice_notifier(&wmux_notifier_block);
	//wmux_ioctl_set(wmux_ioctl_handler);

	return 0;
} /* wmux_drv_init */

/***************************************************************************
 * Function Name: wmux_cleanup_devices
 * Description  : cleans up all the wmux devices and releases memory on exit
 * Returns      : None
 ***************************************************************************/
void wmux_cleanup_devices(void)
{
	struct net_device *dev;
	struct list_head *lh,*lhp;
	struct wmux_dev_info *dev_info;
	struct wmux_group *grp;

	/* clean up all the wmux virtual devices and group */
	for_each_netdev(&init_net, dev)
	{
		grp=__find_wmux_group(dev->name);	//find real_dev
		if(grp)
		{
			list_for_each_safe(lh, lhp, &grp->virtual_devs_head)	//find virtual_dev in grp
			{
				dev_info = list_entry_wmuxdev(lh);
				wmux_unregister_device(dev_info->vdev->name);
			}
		}
	}
} /* wmux_cleanup_devices */

/***************************************************************************
 * Function Name: wmux_drv_exit
 * Description  : wmux module clean routine
 * Returns      : None
 ***************************************************************************/
void wmux_drv_exit(void)
{
	/* Un-register us from receiving netdevice events */
	unregister_netdevice_notifier(&wmux_notifier_block);

	wmux_cleanup_devices();
	
	synchronize_net();
} /* wmux_drv_exit */

/***************************************************************************
                         Proc system Function Definisions
 ***************************************************************************/
//Read Functions
int _rtk_rg_wmux_init_read(struct seq_file *s, void *v)
{
	int len=0;
	//cat /proc/rg/wmux_init
	__wmux_init_usage();
	return len;
}
int _rtk_rg_wmux_add_read(struct seq_file *s, void *v)
{
	int len=0;
	//cat /proc/rg/wmux_add
	__wmux_add_usage();
	return len;
}
int _rtk_rg_wmux_del_read(struct seq_file *s, void *v)
{
	int len=0;
	//cat /proc/rg/wmux_del
	__wmux_del_usage();
	return len;
}
int _rtk_rg_wmux_flag_read(struct seq_file *s, void *v)
{
	int len=0;
	//cat /proc/rg/wmux_flag
	__wmux_flag_usage();
	return len;
}
int _rtk_rg_wmux_info(struct seq_file *s, void *v)
{
	struct net_device *dev;
	struct list_head *lh;
	struct list_head *lhp;
	struct wmux_dev_info *dev_info;
	struct wmux_group *grp;
	int len=0;
	
	//cat /proc/rg/wmux_dump
	//show all wmux settings
	PROC_PRINTF("WMUX virtual devices:\n");
	for_each_netdev(&init_net, dev)
	{
		grp=__find_wmux_group(dev->name);	//find real_dev
		if(grp)
		{
			PROC_PRINTF("{%s}\n",dev->name);
			PROC_PRINTF("=========================================================\n");
			list_for_each_safe(lh, lhp, &grp->virtual_devs_head)	//find virtual_dev in grp
			{
				dev_info = list_entry_wmuxdev(lh);
				if(dev_info->vid<0)
				{
					PROC_PRINTF("%s\tvlan:[untag] order_type=%s\n",dev_info->vdev->name,
						dev_info->proto==RTK_RG_BRIDGE?"[BRIDGE],KEEP,REORDER":dev_info->keep_order==1?"BRIDGE,[KEEP],REORDER":"BRIDGE,KEEP,[REORDER]");
				}
				else
				{
					PROC_PRINTF("%s\tvlan:[%d] order_type=%s\n",dev_info->vdev->name,dev_info->vid,
						dev_info->proto==RTK_RG_BRIDGE?"[BRIDGE],KEEP,REORDER":dev_info->keep_order==1?"BRIDGE,[KEEP],REORDER":"BRIDGE,KEEP,[REORDER]");
				}
				PROC_PRINTF("\tRX_PktCnt:%ld RX_ByteCnt=%ld\n",dev_info->stats.rx_packets,dev_info->stats.rx_bytes);
				PROC_PRINTF("\tTX_PktCnt:%ld TX_ByteCnt:%ld\n",dev_info->stats.tx_packets,dev_info->stats.tx_bytes);
			}
			PROC_PRINTF("=========================================================\n");
		}
	}

	return len;
}

//Write Functions
int _rtk_rg_wmux_init_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64]={'\0'};
	char		*strptr;
	unsigned int init;
	char		*tokptr;

	//echo 1 >wmux_init
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		//tmpbuf[len] = '\0';
		strptr=tmpbuf;
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
			goto errout;
		
		init = simple_strtol(tokptr, NULL, 0);
		if(init!=1)
			goto errout;
		
		//do init
		wmux_cleanup_devices();
	}
	else
	{
		struct seq_file *s=NULL;//used by PROC_PRINTF
		// char *buf=NULL; //used by PROC_PRINTF
		// int len=0;//used by PROC_PRINTF
errout:		
		__wmux_init_usage();
	}

	return len;
}

int _rtk_rg_wmux_add_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64]={'\0'};
	char		*strptr;
	unsigned int intf_idx;
	char		*tokptr=NULL,*rifname=NULL;

	//echo nas0 1 >wmux_add
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		//tmpbuf[len] = '\0';
		strptr=tmpbuf;
		rifname = strsep(&strptr," ");
		if (rifname==NULL)
			goto errout;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
			goto errout;
		
		intf_idx = simple_strtol(tokptr, NULL, 0);
		if(intf_idx > MAX_NETIF_SW_TABLE_SIZE)
			goto errout;
		
		if(wmux_register_device(rifname,intf_idx)==NULL)
			goto errout;
	}
	else
	{
		struct seq_file *s=NULL;//used by PROC_PRINTF
		// char *buf=NULL; //used by PROC_PRINTF
		// int len=0;//used by PROC_PRINTF
errout:
		__wmux_add_usage();
	}

	return len;
}

int _rtk_rg_wmux_del_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64]={'\0'};
	char		*strptr;
	char		*vifname=NULL;
	char		vif_not_exist=1;
	char		vif_name_length=0;
	struct net_device 		*dev;
	struct list_head 		*lh,*lhp;
	struct wmux_dev_info 	*dev_info;
	struct wmux_group 		*grp;

	//echo nas0_1 >wmux_del
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		//tmpbuf[len] = '\0';
		strptr=tmpbuf;
		vifname = strsep(&strptr," ");
		vifname = strsep(&vifname,"\n");
		if (vifname==NULL)
			goto errout;
		vif_name_length=strnlen(vifname,IFNAMSIZ);
		if(vif_name_length>IFNAMSIZ)
			goto errout;
		
		/* clean up wmux virtual device */
		rtnl_lock();
		for_each_netdev(&init_net, dev)
		{
			grp=__find_wmux_group(dev->name);	//find real_dev
			if(grp)
			{
				list_for_each_safe(lh, lhp, &grp->virtual_devs_head)	//find virtual_dev in grp
				{
					dev_info = list_entry_wmuxdev(lh);
					//compare the name and its length
					if(strnlen(dev_info->vdev->name,IFNAMSIZ)==vif_name_length && 
						!memcmp(dev_info->vdev->name,vifname,vif_name_length))
					{
						vif_not_exist=0;
						break;
					}
				}
			}
			if(!vif_not_exist)
				break;
		}
		rtnl_unlock();
		
		if(vif_not_exist)
		{
			rtlglue_printf("%s doesn't exist or not WMUX device.\n",vifname);
			goto errout;
		}
		
		if(wmux_unregister_device(vifname))
		 	goto errout;
	}
	else
	{
		struct seq_file *s=NULL;//used by PROC_PRINTF
		// char *buf=NULL; //used by PROC_PRINTF
		// int len=0;//used by PROC_PRINTF
errout:
		__wmux_del_usage();
	}
	return len;
}

int _rtk_rg_wmux_flag_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 		tmpbuf[64]={'\0'};
	char		*strptr;
	char		*vifname=NULL,*tokptr=NULL;
	char		vif_not_exist=1;
	char		vif_name_length=0;
	char		keep_flag=0;
	struct net_device 		*dev;
	struct list_head 		*lh,*lhp;
	struct wmux_dev_info 	*dev_info;
	struct wmux_group 		*grp;

	//echo nas0_1 1 >wmux_flag
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		//tmpbuf[len] = '\0';
		strptr=tmpbuf;
		vifname = strsep(&strptr," ");
		if (vifname==NULL)
			goto errout;
		vif_name_length=strnlen(vifname,64);
		if(vif_name_length>IFNAMSIZ)
			goto errout;
		
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
			goto errout;
		keep_flag = simple_strtol(tokptr, NULL, 0);
		if(keep_flag != 0 && keep_flag != 1)
			goto errout;
		
		/* set flag to wmux virtual device */
		rtnl_lock();
		for_each_netdev(&init_net, dev)
		{
			grp=__find_wmux_group(dev->name);	//find real_dev
			if(grp)
			{
				list_for_each_safe(lh, lhp, &grp->virtual_devs_head)	//find virtual_dev in grp
				{
					dev_info = list_entry_wmuxdev(lh);
					//compare the name and its length
					if(strnlen(dev_info->vdev->name,IFNAMSIZ)==vif_name_length && 
						!memcmp(dev_info->vdev->name,vifname,vif_name_length))
					{
						write_lock_irq(&wmux_lock);
						dev_info->keep_order=keep_flag;
						write_unlock_irq(&wmux_lock);
						vif_not_exist=0;
						break;
					}
				}
			}
			if(!vif_not_exist)
				break;
		}
		rtnl_unlock();
		
		if(vif_not_exist)
			goto errout;
	}
	else
	{
		struct seq_file *s=NULL; //used by PROC_PRINTF
		// char *buf=NULL; //used by PROC_PRINTF
		// int len=0;//used by PROC_PRINTF
errout:
		__wmux_flag_usage();
	}

	return len;
}

