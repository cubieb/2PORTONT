/* Kernel module to match MAC address parameters. */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/xt_phyport.h>
#include <linux/netfilter/x_tables.h>
#include <net/dst.h>




MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>");
MODULE_DESCRIPTION("Xtables: MAC address match");
MODULE_ALIAS("ipt_phyport");
MODULE_ALIAS("ip6t_phyport");

static bool
checkentry(const struct xt_mtchk_param *par)
{
	const struct xt_phyport_info *phyportinfo = par->matchinfo;

	if(phyportinfo->flags & PORT_SRC){
		if (phyportinfo->srcport> 4) {
			printk(KERN_WARNING "phy port source port: only supports port number 0~4\n");
			return 0;
		}
	}
	else if(phyportinfo->flags & PORT_DST){
		if (phyportinfo->dstport> 4) {
			printk(KERN_WARNING "phy port dest port: only supports port number 0~4\n");
			return 0;
		}
	}
	else{
		printk(KERN_WARNING "wrong phy port flags 0x%x\n", phyportinfo->flags);
		return 0;
	}
	
	return 1;
}

static bool phyport_mt(const struct sk_buff *skb, const struct xt_match_param *par)
{
	const struct xt_phyport_info *info = par->matchinfo;   

	if (info->flags & PORT_SRC) {
		if((skb->srcPhyPort != info->srcport) ^ (!!(info->flags & PORT_SRC_INV))){
			return false;
		}
	}
	else if (info->flags & PORT_DST) {
		if((skb->dstPhyPort != info->dstport) ^ (!!(info->flags & PORT_DST_INV))){
			return false;
		}
	}
	else{
		printk(KERN_WARNING "wrong phy port flags 0x%x\n", info->flags);
		return false;
	}

	return true;
}



static struct xt_match phyport_mt_reg __read_mostly = {
	.name      = "phyport",
	.revision  = 0,
	.family    = NFPROTO_UNSPEC,
	.checkentry	= checkentry,
	.match     = phyport_mt,
	.matchsize = sizeof(struct xt_phyport_info),
/*	.hooks     = (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_IN) |
	             (1 << NF_INET_FORWARD),
*/
	.me        = THIS_MODULE,


};

static int __init phyport_mt_init(void)
{
	return xt_register_match(&phyport_mt_reg);
}

static void __exit phyport_mt_exit(void)
{
	xt_unregister_match(&phyport_mt_reg);
}

module_init(phyport_mt_init);
module_exit(phyport_mt_exit);
