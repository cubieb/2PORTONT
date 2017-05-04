/*
 *  ebt_ftos
 *
 *	Authors:
 *	 Song Wang <songw@broadcom.com>
 *
 *  Feb, 2004
 *
 */

// The ftos target can be used in any chain
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <linux/if_vlan.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_ftos_t.h>

#include <linux/ipv6.h>
#include <net/dsfield.h>

#include <linux/netfilter/xt_DSCP.h>
#include <linux/netfilter_ipv4/ipt_TOS.h>

// Mason Yu. patch from xt_DSCP.c. This function work fine.
/* DSCP Target for IPv6 */
static unsigned int
ebt_dscp_tg6(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ebt_ftos_t_info *dinfo = par->targinfo;
	u_int8_t dscp = ipv6_get_dsfield(ipv6_hdr(skb)) >> XT_DSCP_SHIFT;

	if (dscp != dinfo->ftos) {
		if (!skb_make_writable(skb, sizeof(struct ipv6hdr)))
			return NF_DROP;

		ipv6_change_dsfield(ipv6_hdr(skb), (__u8)(~XT_DSCP_MASK),
				    dinfo->ftos << XT_DSCP_SHIFT);
	}
	return XT_CONTINUE;
}

// Mason Yu. patch from xt_DSCP.c. This function work fine.
/* traffic class Target for IPv6 */
static unsigned int
ebt_tos_tg6(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ebt_ftos_t_info *dinfo = par->targinfo;
	u_int8_t tos = ipv6_get_dsfield(ipv6_hdr(skb));

	if (tos != dinfo->ftos) {
		if (!skb_make_writable(skb, sizeof(struct ipv6hdr)))
			return NF_DROP;

		ipv6_change_dsfield(ipv6_hdr(skb), 0, dinfo->ftos);
	}
	return XT_CONTINUE;
}

static unsigned int
ebt_target_ftos(struct sk_buff *skb, const struct xt_target_param *par)
{
	const struct ebt_ftos_t_info *ftosinfo = par->targinfo;
	struct iphdr *iph;
        struct vlan_hdr *frame;	
	unsigned char prio = 0;
	unsigned short TCI;
        /* Need to recalculate IP header checksum after altering TOS byte */
	u_int16_t diffs[2];
	/* if VLAN frame, we need to point to correct network header */
	if (skb->protocol == __constant_htons(ETH_P_8021Q))
        	iph = (struct iphdr *)(skb_network_header(skb) + VLAN_HLEN);	
    else
		iph = ip_hdr(skb);
#if 0
	{
	    __u32 ipsrc = iph->saddr;
	    __u32 ipdst = iph->daddr;
	    printk("dev name: %s, src:0x%08x, dst:0x%08x\n", (*pskb)->dev->name, ipsrc, ipdst);
	}
#endif

	if ((ftosinfo->ftos_set & FTOS_SETFTOS) && (iph->tos != ftosinfo->ftos)) {
                //printk("ebt_target_ftos:FTOS_SETFTOS ....., skb->protocol=0x%x\n", skb->protocol);
		/* raw socket (tcpdump) may have clone of incoming
                   skb: don't disturb it --RR */
		if (!skb_make_writable(skb, 0))
			return EBT_DROP;
		if (skb->protocol == __constant_htons(ETH_P_8021Q))
	        	iph = (struct iphdr *)(skb_network_header(skb) + VLAN_HLEN);
		// Mason Yu. Added tos(traffic class) Target for IPv6
		else if (skb->protocol == 0x86dd) {
			ebt_tos_tg6(skb, par);
			return ftosinfo->target;
		}
       	else
			iph = ip_hdr(skb);
		#if 0
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;
			if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q))
		        	iph = (struct iphdr *)(skb_network_header(*pskb) + VLAN_HLEN);
        		else
				iph = (struct iphdr *)skb_network_header(*pskb);
		}
		#endif
		
		diffs[0] = htons(iph->tos) ^ 0xFFFF;
		iph->tos = ftosinfo->ftos;
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF));		
		//commented by penggenyao
		//(*pskb)->nfcache |= NFC_ALTERED;
	} else if (ftosinfo->ftos_set & FTOS_WMMFTOS) {
	    //printk("ebt_target_ftos:FTOS_WMMFTOS .....0x%08x\n", (*pskb)->nfmark & 0xf);
		/* raw socket (tcpdump) may have clone of incoming
                   skb: don't disturb it --RR */
		if (!skb_make_writable(skb, 0))
			return EBT_DROP;
		if (skb->protocol == __constant_htons(ETH_P_8021Q))
	        	iph = (struct iphdr *)(skb_network_header(skb) + VLAN_HLEN);
       		else
			iph = ip_hdr(skb);
		#if 0
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;
			if ((*pskb)->protocol == __constant_htons(ETH_P_8021Q))
		        	iph = (struct iphdr *)(skb_network_header(*pskb) + VLAN_HLEN);
        		else
				iph = (struct iphdr *)skb_network_header(*pskb);
		}
		#endif

		diffs[0] = htons(iph->tos) ^ 0xFFFF;
		iph->tos |= ((skb->mark >> PRIO_LOC_NFMARK) & PRIO_LOC_NFMASK) << DSCP_MASK_SHIFT;
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF));
		//commented by penggenyao
        //(*pskb)->nfcache |= NFC_ALTERED;
	} else if ((ftosinfo->ftos_set & FTOS_8021QFTOS) && skb->protocol == __constant_htons(ETH_P_8021Q)) {
	    
	    /* raw socket (tcpdump) may have clone of incoming
           skb: don't disturb it --RR */
		if (!skb_make_writable(skb, 0))
			return EBT_DROP;
        	iph = (struct iphdr *)(skb_network_header(skb) + VLAN_HLEN);
            	frame = (struct vlan_hdr *)skb_network_header(skb);
		TCI = ntohs(frame->h_vlan_TCI);
		prio = (unsigned char)((TCI >> 13) & 0x7);
		#if 0
		if (skb_cloned(*pskb) && !(*pskb)->sk) {
			struct sk_buff *nskb = skb_copy(*pskb, GFP_ATOMIC);
			if (!nskb)
				return NF_DROP;
			kfree_skb(*pskb);
			*pskb = nskb;

	        iph = (struct iphdr *)(skb_network_header(*pskb) + VLAN_HLEN);
            	frame = (struct vlan_hdr *)skb_network_header(*pskb);
			TCI = ntohs(frame->h_vlan_TCI);
			prio = (unsigned char)((TCI >> 13) & 0x7);
		}
		#endif
        //printk("ebt_target_ftos:FTOS_8021QFTOS ..... 0x%08x\n", prio);
        diffs[0] = htons(iph->tos) ^ 0xFFFF;
	    iph->tos |= (prio & 0xf) << DSCP_MASK_SHIFT;
		diffs[1] = htons(iph->tos);
		iph->check = csum_fold(csum_partial((char *)diffs,
		                                    sizeof(diffs),
		                                    iph->check^0xFFFF)); 
		//commented by penggenyao
        //(*pskb)->nfcache |= NFC_ALTERED;
	}

	return ftosinfo->target;
}

static bool ebt_target_ftos_check(const struct xt_tgchk_param *par)
{
	const struct ebt_ftos_t_info *info = par->targinfo;

	if (BASE_CHAIN && info->target == EBT_RETURN)
		return false;
	//CLEAR_BASE_CHAIN_BIT;
	if (INVALID_TARGET)
		return false;
	return true;
}

static struct xt_target ftos_target __read_mostly =
{
        .name           = EBT_FTOS_TARGET,
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
        .target         = ebt_target_ftos,
        .checkentry     = ebt_target_ftos_check,
	.targetsize	= XT_ALIGN(sizeof(struct ebt_ftos_t_info)),
        .me             = THIS_MODULE,
};

static int __init init(void)
{
	return xt_register_target(&ftos_target);
}

static void __exit fini(void)
{
	xt_unregister_target(&ftos_target);
}

module_init(init);
module_exit(fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Song Wang, songw@broadcom.com");
MODULE_DESCRIPTION("Target to overwrite the full TOS byte in IP header");
