/* 	ptm_pp.c: 

	1. A Linux PTM driver with PKTA for the RealTek VDSL chips. 
	2. The release version v1
*/

#define DRV_NAME		"RTL8681_PTM_PP"
#define DRV_VERSION		"0.0.1"
#define DRV_RELDATE		"June 12, 2012"


/* Include header files */
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#ifdef CONFIG_RTL8672	
#include <bspchip.h>
#include <linux/version.h>
#include "../../arch/rlx/bsp/gpio.h"
#endif

/*PTM registers' definitions*/
#include "ptm_regs.h"

/* PKTA related definitions */
#include "../packet_processor/rtl8672PacketProcessor.h"

#define DRV_NAME		"RTL8681_PTM_PP"
#define DRV_VERSION		"0.0.1"
#define DRV_RELDATE		"June 12, 2012"

/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT			(6*HZ)

/* PTM & PKTA rx tasklet */
struct tasklet_struct ppptm_rx_tasklets;

/*
	Description : IOCTL function for PTM
	Input	  :  net_device structure, ifreq structure, cmd
	Output	  :  0 means success, others mean fail
*/
static int ptm_pp_ioctl (struct net_device *dev, struct ifreq *rq, int cmd)
{
	int rc = 0;

	if (!netif_running(dev) && cmd!=SIOCETHTEST)
		return -EINVAL;

	switch (cmd) {

	default:
		rc = -EOPNOTSUPP;
		break;
	}

	return rc;
}

/*
	Description : Initialize PTM TX timeout function
	Input	  :  net_device structure
	Output	  :  none
*/
static void ptm_pp_tx_timeout (struct net_device *dev)
{
	struct mac_private *cp = dev->priv;
	unsigned long flags;

	spin_lock_irqsave(&cp->lock, flags);
	
	/* Do nothing, under implement */

	spin_unlock_irqrestore(&cp->lock,flags);
	
	if (netif_queue_stopped(cp->dev))
		netif_wake_queue(cp->dev);

}

int __init ptm_pp_probe (void)
{
	int i, rc;
	struct net_device *dev;
	struct mac_private *tp;	

	i=0;  /*interface of packet processor, PTM 0~7 is ok*/
	dev = alloc_etherdev(sizeof(struct mac_private));
	if (!dev)
		return -ENOMEM;

	ptmPP_dev = dev;

	tp = dev->priv;
	((struct mac_private *)(ptmPP_dev->priv))->tx_intf=i;

	dev->open = start_pp_ptm_rx;
	dev->do_ioctl = ptm_pp_ioctl;
	dev->stop=stop_pp_ptm_rx;
	dev->set_mac_address = rtl8672_pp_set_mac_addr; 			
	dev->hard_start_xmit = rtl8672_ptm_vtx_start_xmit;
	dev->get_stats = rtl8672_mac_get_stats; 
	dev->features |= NETIF_F_SG | NETIF_F_HW_CSUM | NETIF_F_HIGHDMA;
	dev->tx_timeout = ptm_pp_tx_timeout;
	dev->watchdog_timeo = TX_TIMEOUT;

	dev->irq=BSP_PKT_SAR_IRQ;
	dev->priv_flags = IFF_DOMAIN_WAN;
	strcpy(dev->name, "ptm0_pp");

	
	rc = register_netdev(dev);
	if (rc)
		goto err_out_iomap;
		
	printk(KERN_INFO "Register %s as WAN netdev to kernel\n", DRV_NAME);	

	ppptm_rx_tasklets.func=(void (*)(unsigned long))ppptm_rx_bh;

	return 0;

err_out_iomap:
	kfree(dev);
	return -1 ;


}

static void __exit ptm_pp_exit (void)
{
	
	unregister_netdev(ptmPP_dev);
		
	return;
}


module_init(ptm_pp_probe);
module_exit(ptm_pp_exit);


