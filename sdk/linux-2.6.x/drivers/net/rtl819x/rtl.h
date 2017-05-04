/*
*	Copyright. All rights is diminished
*
*	$Author: August 2011-10-20
*
*
*	Ethtool for adsl-switch-0412
*/
#ifndef _RTL_AUG_H
#define _RTL_AUG_H


#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#include <linux/pagemap.h>
#include <linux/dma-mapping.h>
#include <linux/bitops.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <linux/capability.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <net/pkt_sched.h>
#include <linux/list.h>
#include <linux/reboot.h>
#include <net/checksum.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>

#include <bsp/bspchip.h>

#include <linux/config.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <linux/sched.h>
#include <net/sock.h>

#include <net/rtl/rtl_nic.h>

#include "version.h"
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>

#include "AsicDriver/asicRegs.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER_L3
#include "AsicDriver/rtl865x_asicL3.h"
#endif

#include "common/mbuf.h"
#include <net/rtl/rtl_queue.h>
#include "common/rtl_errno.h"
#include "rtl865xc_swNic.h"

/*common*/
#include "common/rtl865x_vlan.h"
#include <net/rtl/rtl865x_netif.h>
#include "common/rtl865x_netif_local.h"

/*l2*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L2
#include "l2Driver/rtl865x_fdb.h"
#endif

/*l3*/
#ifdef CONFIG_RTL_LAYERED_DRIVER_L3
#include "l3Driver/rtl865x_ip.h"
#include "l3Driver/rtl865x_nexthop.h"
#include <net/rtl/rtl865x_ppp.h>
#include "l3Driver/rtl865x_ppp_local.h"
#include "l3Driver/rtl865x_route.h"
#include "l3Driver/rtl865x_arp.h"
#include <net/rtl/rtl865x_nat.h>
#endif


#ifdef CONFIG_RTL_MULTI_LAN_DEV

static __inline__  int ethtool_get_port_by_dev(struct net_device *dev)
{
	unsigned int j;
	struct dev_priv	 *dp = (struct dev_priv *)dev->priv;
	for(j = 0; j < RTL8651_PORT_NUMBER; ++j) 
	{ 
		if(dp->portmask & (1 << j)) 
			return j; 
	}

	return -1;
}



//august: create a function that get net_device by rtl_sw_port
static __inline__ struct net_device* ethtool_get_netdev_by_rtl_sw_port(const struct re865x_priv* _prtl86xx_dev , const unsigned int switch_port)
{
	struct net_device *dev, *ret;
	struct dev_priv	  *dp;
	unsigned int num;

	ret = NULL;
	
	read_lock(&dev_base_lock);
	for_each_netdev(&init_net, dev) {
		//select the switch device
		if(BSP_SW_IRQ != dev->irq)
			continue;

		//list 8676 lan interface 	
		for(num = 0; num < ETH_INTF_NUM; ++num) {
			//august: compare the ifindex
			if (_prtl86xx_dev->dev[num]->ifindex == dev->ifindex) {
				//august: get priv, use the priv.portmask
				dp = (struct dev_priv *)_prtl86xx_dev->dev[num]->priv;
				if(dp->portmask & (1 << switch_port)){
					//dev_hold(dev);
					ret = dev;
					break; //break for(num =0, ...)
				}
			}
		}

		if(ret)
			break;
	}
	read_unlock(&dev_base_lock);
	return ret;
}



void rtl_set_ethtool_ops(struct net_device *netdev);

#endif



#ifdef CONFIG_RTL_NLMSG_PROTOCOL
extern struct sock *global_rtlmsg_sock;

extern int 	rtl_gbl_rtlmsg_sock_init(void);

extern void rtl_nlmsg_handler(struct sk_buff *skb);

extern int 	rtl_nl_send_lkchg_msg(const unsigned int old_ptmks, 
											const unsigned int new_ptmks);

#endif



#endif

