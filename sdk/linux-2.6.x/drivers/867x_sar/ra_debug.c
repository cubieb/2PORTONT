/*
 *	ra_debug.c
 */

#include <linux/atm.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/igmp.h>
#include <linux/icmp.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include "ra8670.h"

char s_IGMP_REPORT[] = "Report";
char s_IGMP_LEAVE[] = "Leave";
char s_IGMP_QUERY[] = "Query";
char s_IGMP_UNKNOWN[] = "Unknown";

struct SAR_IOCTL_DEBUG dbg_cfg;

struct pkt_nh {
	union {
		struct tcphdr	*th;
		struct udphdr	*uh;
		struct icmphdr	*icmph;
		struct igmphdr	*igmph;
		struct iphdr	*ipiph;
		struct ipv6hdr	*ipv6h;
		unsigned char	*raw;
	} h;

	union {
		struct iphdr	*iph;
		struct ipv6hdr	*ipv6h;
		struct arphdr	*arph;
		unsigned char	*raw;
	} nh;

	union {
		struct ethhdr	*ethernet;
	  	unsigned char 	*raw;
	} mac;
	
	unsigned short protocol;
	unsigned short ipproto;
};

/*
 *	Get headers of the packet for the pvc channel.
 */
static void getPktInfo(const struct sk_buff *skb, int ch_no, struct pkt_nh *hdr)
{
	int cut_len = 0, encap_mode;
	
	memset(hdr, 0, sizeof(struct pkt_nh));
	switch (sar_dev->vcc[ch_no].rfc)
	{
		case RFC1483_BRIDGED:
			encap_mode = sar_dev->vcc[ch_no].rfc * 2 + sar_dev->vcc[ch_no].framing;
			cut_len = FrameHeaderSize_1483[encap_mode];
			hdr->mac.raw = &skb->data[cut_len];
			hdr->protocol = hdr->mac.ethernet->h_proto;
			hdr->nh.raw = &hdr->mac.raw[14];
			if (hdr->protocol == 0x0800) {
				hdr->ipproto = hdr->nh.iph->protocol;
				hdr->h.raw = &hdr->nh.raw[hdr->nh.iph->ihl*4];
			}
			else if (hdr->protocol == 0x8864) {
				hdr->nh.raw = &hdr->nh.raw[8];
				hdr->ipproto = hdr->nh.iph->protocol;
				hdr->h.raw = &hdr->nh.raw[hdr->nh.iph->ihl*4];
			}
			break;
		case RFC1483_ROUTED:
			encap_mode = sar_dev->vcc[ch_no].rfc * 2 + sar_dev->vcc[ch_no].framing;
			cut_len = FrameHeaderSize_1483[encap_mode];
			hdr->protocol = 0x0800;
			hdr->nh.raw = &skb->data[cut_len];
			hdr->ipproto = hdr->nh.iph->protocol;
			hdr->h.raw = &hdr->nh.raw[hdr->nh.iph->ihl*4];
			break;
		case RFC2364:
			//cut_len = FrameHeaderSize_2364[sar_dev->vcc[ch_no].framing];
			break;
		default:
			return;
	}
}

/* flag:
 *	0 : Input
 *	1 : Output
 */
static void skb_debug(const struct sk_buff *skb, int enable, int flag)
{
	if (enable) {
		int i;
		if (!flag)
			printk("\nI: ");
		else
			printk("\nO: ");
		printk("sar len = %d", skb->len);
		for (i=0; i<skb->len && i<debug_num2print; i++) {
			if (i%16==0)
				printk("\n");
			printk("%2.2x ", skb->data[i]);
		}
		printk("\n");
	}
}

/* flag:
 *	0 : Input
 *	1 : Output
 */
void skb_pvc_debug(const struct sk_buff *skb, int ch_no, int flag)
{
	struct pkt_nh pkt_hdr;
	char *pstr;
	
	if (dbg_cfg.enable&(flag+1)) {
		getPktInfo(skb, ch_no, &pkt_hdr);
		if ((dbg_cfg.chno>=0 && dbg_cfg.chno != ch_no) ||
			(dbg_cfg.etype && dbg_cfg.etype != pkt_hdr.protocol) ||
			(dbg_cfg.ipproto && dbg_cfg.ipproto != pkt_hdr.ipproto))
			return;
		if (dbg_cfg.hr && dbg_cfg.ipproto) { // human readable
			if (pkt_hdr.ipproto == dbg_cfg.ipproto) {
				switch (dbg_cfg.ipproto) {
					case IPPROTO_IGMP:
						if (pkt_hdr.h.igmph->type == 0x11)
							pstr = s_IGMP_QUERY;
						else if (pkt_hdr.h.igmph->type == 0x16)
							pstr = s_IGMP_REPORT;
						else if (pkt_hdr.h.igmph->type == 0x17)
							pstr = s_IGMP_LEAVE;
						else
							pstr = s_IGMP_UNKNOWN;
						if (flag)
							printk("-- Send IGMP %s %u.%u.%u.%u\n", pstr, NIPQUAD(pkt_hdr.h.igmph->group));
						else
							printk("-- Receive IGMP %s %u.%u.%u.%u\n", pstr, NIPQUAD(pkt_hdr.h.igmph->group));
						
						break;
					case IPPROTO_TCP:
						if (flag)
							printk("-- TCP %u.%u.%u.%u:%u --> %u.%u.%u.%u:%u\n",
								NIPQUAD(pkt_hdr.nh.iph->saddr), pkt_hdr.h.th->source,
								NIPQUAD(pkt_hdr.nh.iph->daddr), pkt_hdr.h.th->dest);
						else
							printk("-- TCP %u.%u.%u.%u:%u <-- %u.%u.%u.%u:%u\n",
								NIPQUAD(pkt_hdr.nh.iph->daddr), pkt_hdr.h.th->dest,
								NIPQUAD(pkt_hdr.nh.iph->saddr), pkt_hdr.h.th->source);
						break;
					case IPPROTO_UDP:
						if (flag)
							printk("-- UDP %u.%u.%u.%u:%u --> %u.%u.%u.%u:%u\n",
								NIPQUAD(pkt_hdr.nh.iph->saddr), pkt_hdr.h.uh->source,
								NIPQUAD(pkt_hdr.nh.iph->daddr), pkt_hdr.h.uh->dest);
						else
							printk("-- UDP %u.%u.%u.%u:%u <-- %u.%u.%u.%u:%u\n",
								NIPQUAD(pkt_hdr.nh.iph->daddr), pkt_hdr.h.uh->dest,
								NIPQUAD(pkt_hdr.nh.iph->saddr), pkt_hdr.h.uh->source);
						break;
					case IPPROTO_ICMP:
						if (flag)
							printk("-- ICMP %d %u.%u.%u.%u --> %u.%u.%u.%u\n", pkt_hdr.h.icmph->type,
								NIPQUAD(pkt_hdr.nh.iph->saddr), NIPQUAD(pkt_hdr.nh.iph->daddr));
						else
							printk("-- ICMP %d %u.%u.%u.%u <-- %u.%u.%u.%u\n", pkt_hdr.h.icmph->type,
								NIPQUAD(pkt_hdr.nh.iph->daddr), NIPQUAD(pkt_hdr.nh.iph->saddr));
						break;
					default:;
				}
			}
		}
		else {
			skb_debug(skb, 1, flag);
		}
	}
}

void cell_debug(const void *pcell, int enable, int flag)
{
	struct sk_buff 	debug_skb;
	
	debug_skb.len = 53;
	debug_skb.data = (unsigned char *)pcell;
	skb_debug(&debug_skb, enable, flag);
}

void skb_oamaal5_debug(const struct sk_buff *skb, int enable, int flag)
{
	skb_debug(skb, enable, flag);
}

