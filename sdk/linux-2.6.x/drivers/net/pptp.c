/*
 *  Point-to-Point Tunneling Protocol for Linux
 *
 *	Authors: Dmitry Kozlov <xeb@mail.ru>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 *
 */

#include <linux/string.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/net.h>
#include <linux/skbuff.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/ppp_channel.h>
#include <linux/ppp_defs.h>
#include <linux/if_pppox.h>
#include <linux/if_ppp.h>
#include <linux/notifier.h>
#include <linux/file.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/version.h>
#include <linux/rcupdate.h>
#include <linux/spinlock.h>

#include <net/sock.h>
#include <net/protocol.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/gre.h>

#ifdef CONFIG_IPV6_VPN
#include <linux/in6.h>
#include <net/ipv6.h>
#include <net/inet_ecn.h>
#include <net/ip6_route.h>
#endif

#include <linux/uaccess.h>

#define PPTP_DRIVER_VERSION "0.8.5"

#define MAX_CALLID 65535

static DECLARE_BITMAP(callid_bitmap, MAX_CALLID + 1);
static struct pppox_sock **callid_sock;

static DEFINE_SPINLOCK(chan_lock);

static struct proto pptp_sk_proto __read_mostly;
/*static */const struct ppp_channel_ops pptp_chan_ops;
static const struct proto_ops pptp_ops;

#define PPP_LCP_ECHOREQ 0x09
#define PPP_LCP_ECHOREP 0x0A
#define SC_RCV_BITS	(SC_RCV_B7_1|SC_RCV_B7_0|SC_RCV_ODDP|SC_RCV_EVNP)

#define MISSING_WINDOW 20
#define WRAPPED(curseq, lastseq)\
	((((curseq) & 0xffffff00) == 0) &&\
	(((lastseq) & 0xffffff00) == 0xffffff00))

#define PPTP_GRE_PROTO  0x880B
#define PPTP_GRE_VER    0x1

#define PPTP_GRE_FLAG_C	0x80
#define PPTP_GRE_FLAG_R	0x40
#define PPTP_GRE_FLAG_K	0x20
#define PPTP_GRE_FLAG_S	0x10
#define PPTP_GRE_FLAG_A	0x80

#define PPTP_GRE_IS_C(f) ((f)&PPTP_GRE_FLAG_C)
#define PPTP_GRE_IS_R(f) ((f)&PPTP_GRE_FLAG_R)
#define PPTP_GRE_IS_K(f) ((f)&PPTP_GRE_FLAG_K)
#define PPTP_GRE_IS_S(f) ((f)&PPTP_GRE_FLAG_S)
#define PPTP_GRE_IS_A(f) ((f)&PPTP_GRE_FLAG_A)

#define PPTP_HEADER_OVERHEAD (2+sizeof(struct pptp_gre_header))
struct pptp_gre_header {
	u8  flags;
	u8  ver;
	u16 protocol;
	u16 payload_len;
	u16 call_id;
	u32 seq;
	u32 ack;
} __packed;

static struct pppox_sock *lookup_chan(u16 call_id, __be32 s_addr)
{
	struct pppox_sock *sock;
	struct pptp_opt *opt;

	rcu_read_lock();
	sock = rcu_dereference(callid_sock[call_id]);
	if (sock) {
		opt = &sock->proto.pptp;
		if (
#ifdef CONFIG_IPV6_VPN
			opt->dst_addr.sin_addr.in.s_addr != s_addr
#else
			opt->dst_addr.sin_addr.s_addr != s_addr
#endif
			)
			sock = NULL;
		else
			sock_hold(sk_pppox(sock));
	}
	rcu_read_unlock();

	return sock;
}

static int lookup_chan_dst(u16 call_id, __be32 d_addr)
{
	struct pppox_sock *sock;
	struct pptp_opt *opt;
	int i;

	rcu_read_lock();
	for (i = find_next_bit(callid_bitmap, MAX_CALLID, 1); i < MAX_CALLID;
	     i = find_next_bit(callid_bitmap, MAX_CALLID, i + 1)) {
		sock = rcu_dereference(callid_sock[i]);
		if (!sock)
			continue;
		opt = &sock->proto.pptp;
		if (opt->dst_addr.call_id == call_id &&
#ifdef CONFIG_IPV6_VPN
			  opt->dst_addr.sin_addr.in.s_addr == d_addr
#else
			  opt->dst_addr.sin_addr.s_addr == d_addr
#endif
			  )
			break;
	}
	rcu_read_unlock();

	return i < MAX_CALLID;
}

#ifdef CONFIG_IPV6_VPN
static struct pppox_sock *ipv6_lookup_chan(u16 call_id, struct in6_addr s_addr)
{
	struct pppox_sock *sock;
	struct pptp_opt *opt;

	rcu_read_lock();
	sock = rcu_dereference(callid_sock[call_id]);
	if (sock) {
		opt = &sock->proto.pptp;
		if ((1 == opt->dst_addr.ipType) || !ipv6_addr_equal(&opt->dst_addr.sin_addr.in6, &s_addr))
			sock = NULL;
		else
			sock_hold(sk_pppox(sock));
	}
	rcu_read_unlock();

	return sock;
}

static int ipv6_lookup_chan_dst(u16 call_id, struct in6_addr d_addr)
{
	struct pppox_sock *sock;
	struct pptp_opt *opt;
	int i;

	rcu_read_lock();
	for (i = find_next_bit(callid_bitmap, MAX_CALLID, 1); i < MAX_CALLID;
		 i = find_next_bit(callid_bitmap, MAX_CALLID, i + 1)) {
		sock = rcu_dereference(callid_sock[i]);
		if (!sock)
			continue;
		opt = &sock->proto.pptp;
		if (opt->dst_addr.call_id == call_id &&
			  (1 == opt->dst_addr.ipType) && 
			  ipv6_addr_equal(&opt->dst_addr.sin_addr.in6, &d_addr))
			break;
	}
	rcu_read_unlock();

	return i < MAX_CALLID;
}
#endif

static int add_chan(struct pppox_sock *sock)
{
	static int call_id;

	spin_lock(&chan_lock);
	if (!sock->proto.pptp.src_addr.call_id)	{
		call_id = find_next_zero_bit(callid_bitmap, MAX_CALLID, call_id + 1);
		if (call_id == MAX_CALLID) {
			call_id = find_next_zero_bit(callid_bitmap, MAX_CALLID, 1);
			if (call_id == MAX_CALLID)
				goto out_err;
		}
		sock->proto.pptp.src_addr.call_id = call_id;
	} else if (test_bit(sock->proto.pptp.src_addr.call_id, callid_bitmap))
		goto out_err;

	set_bit(sock->proto.pptp.src_addr.call_id, callid_bitmap);
	rcu_assign_pointer(callid_sock[sock->proto.pptp.src_addr.call_id], sock);
	spin_unlock(&chan_lock);

	return 0;

out_err:
	spin_unlock(&chan_lock);
	return -1;
}

static void del_chan(struct pppox_sock *sock)
{
	spin_lock(&chan_lock);
	clear_bit(sock->proto.pptp.src_addr.call_id, callid_bitmap);
	rcu_assign_pointer(callid_sock[sock->proto.pptp.src_addr.call_id], NULL);
	spin_unlock(&chan_lock);
	synchronize_rcu();
}

#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
/**************************************************************************/
extern int vpn_ppp_down_fastpath(struct ppp_channel *chan, struct sk_buff **skb, unsigned int headroom);
extern int FastPath_Enter(struct sk_buff *skb);
static unsigned short iphdr_id=0;

/*
 * NAME: pptp_up_fastpath
 * Return value: 0 fail            1 success
 * Date: 20111117
 */
__IRAM int pptp_up_fastpath(struct ppp_channel *chan, struct sk_buff *skb)
{
	struct sock *sk = (struct sock *) chan->private;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	//struct pptp_gre_header *hdr;
	unsigned char *hdr;
	//struct iphdr  *iph;
	unsigned char *iph;
	unsigned int header_len = sizeof(struct pptp_gre_header);//sizeof(*hdr);
	int len;
	//unsigned char *data;
	__u32 seq_recv;

#if 0//QL: move to pptp_ppp_up_fastpath()
	data = skb_push(skb, 2);
	data[0] = PPP_ALLSTATIONS;
	data[1] = PPP_UI;
#endif
	len = skb->len;

	seq_recv = opt->seq_recv;

	if (opt->ack_sent == seq_recv)
		header_len -= 4;//sizeof(hdr->ack);

	/* Push down and install GRE header */
	//skb_push(skb, header_len);
	//hdr = (struct pptp_gre_header *)(skb->data);
	hdr = (unsigned char *)(skb->data-header_len);

#if 0
	hdr->flags       = PPTP_GRE_FLAG_K;
	hdr->ver         = PPTP_GRE_VER;
	hdr->protocol    = htons(PPTP_GRE_PROTO);
	hdr->call_id     = htons(opt->dst_addr.call_id);

	hdr->flags      |= PPTP_GRE_FLAG_S;
	hdr->seq         = htonl(++opt->seq_sent);

	if (opt->ack_sent != seq_recv)	{
		/* send ack with this message */
		hdr->ver |= PPTP_GRE_FLAG_A;
		hdr->ack  = htonl(seq_recv);
		opt->ack_sent = seq_recv;
	}
	hdr->payload_len = htons(len);
#else
#define GRE_HDR_FLAG_VER_PROTO	(((PPTP_GRE_FLAG_K|PPTP_GRE_FLAG_S)<<24)|(PPTP_GRE_VER<<16)|(PPTP_GRE_PROTO))
	if (opt->ack_sent != seq_recv) {
		*(unsigned int *)hdr = GRE_HDR_FLAG_VER_PROTO|(PPTP_GRE_FLAG_A<<16);
		*(unsigned int *)(hdr+12) = htonl(seq_recv);
		opt->ack_sent = seq_recv;
	}
	else
		*(unsigned int *)hdr = GRE_HDR_FLAG_VER_PROTO;
	*(unsigned int *)(hdr+4) = ((htons(len)<<16) | htons(opt->dst_addr.call_id));
	*(unsigned int *)(hdr+8) = htonl(++opt->seq_sent);
#endif

	//skb_push(skb, sizeof(*iph));
	iph = skb_push(skb, sizeof(struct iphdr)+header_len);
	skb_reset_network_header(skb);

#if 0
	iph->version =	4;
	iph->ihl =	sizeof(struct iphdr) >> 2;
	iph->frag_off = 0x4000;
	iph->protocol = IPPROTO_GRE;
	iph->tos	  = 0;
	iph->daddr	  = opt->dst_addr.sin_addr.s_addr;
	iph->saddr	  = opt->src_addr.sin_addr.s_addr;
	iph->ttl	  = 1;
	iph->tot_len  = htons(skb->len);
	iph->id       = ++iphdr_id;
#else
	*(unsigned int *)iph		= 0x45000000 | htons(skb->len);
	*(unsigned int *)(iph+4)	= ((++iphdr_id)<<16) | 0x4000;
	*(unsigned short *)(iph+8)	= 0x012F;
#ifdef CONFIG_IPV6_VPN
	*(unsigned int *)(iph+12)	= opt->src_addr.sin_addr.in.s_addr;
	*(unsigned int *)(iph+16)	= opt->dst_addr.sin_addr.in.s_addr;
#else
	*(unsigned int *)(iph+12)	= opt->src_addr.sin_addr.s_addr;
	*(unsigned int *)(iph+16)	= opt->dst_addr.sin_addr.s_addr;
#endif
#endif

	ip_send_check((struct iphdr *)iph);

	/* skb dst has been initialized to PPP device, so we must release it before redirecting to real device. */
	dst_release(skb->dst);

	if (FastPath_Enter(skb) != 1) {
		struct rtable *rt;
		int err = 0;
		
		{
			struct flowi fl = { .oif = 0,
				.nl_u = {
					.ip4_u = {
#ifdef CONFIG_IPV6_VPN
						.daddr = opt->dst_addr.sin_addr.in.s_addr,
						.saddr = opt->src_addr.sin_addr.in.s_addr,
#else
						.daddr = opt->dst_addr.sin_addr.s_addr,
						.saddr = opt->src_addr.sin_addr.s_addr,
#endif
						.tos = RT_TOS(0) } },
				.proto = IPPROTO_GRE };
			err = ip_route_output_key(&init_net, &rt, &fl);
			if (err)
				goto tx_error;
		}

		skb->dst = &rt->u.dst;
		//nf_reset(skb);
		
		//printk("%s %d go normal path.\n", __func__, __LINE__);
		ip_local_out(skb);
	}
	//else
	//	printk("fastpath OK\n");

	return 1;

tx_error:
	dev_kfree_skb(skb);
	//printk("%s %d drop packet here(sip:%x dip:%x callid:%d).\n", __func__, __LINE__, 
	//	iph->saddr, iph->daddr, hdr->call_id);
	
	return 1;
}

/*
 * NAME: pptp_down_fastpath
 * Description: pptp downstream fastpath process
 * Return value: 0 fail            1 success
 * Date: 20111116
 */
__IRAM int pptp_down_fastpath(struct sk_buff **skb)
{
	struct iphdr *iph;
	struct pptp_gre_header *grehdr;
	struct pppox_sock *po;
	struct pptp_opt *opt;
	struct sock *sk;
	int iphdrsize=0, grehdrsize=0, ppphdrsize=0;
	int headersize=0, payload_len, seq;
	__u32 orig_ack, orig_seq;
	__u8 *payload;
	int compressed=0;
	int ret = -1;

#if 0//pptp interface must be in route mode, don't need to do below check
	if (skb->pkt_type != PACKET_HOST)
		return -1;
#endif
	iph = ip_hdr(*skb);
	if(iph->frag_off & htons(0x3fff))
		return -1;

	iphdrsize = ip_hdrlen(*skb);
	grehdr = (struct pptp_gre_header*)((__u32 *)iph + iph->ihl);
	po = lookup_chan(htons(grehdr->call_id), iph->saddr);
	if (NULL == po)
		return -1;

	sk = sk_pppox(po);
	bh_lock_sock(sk);
	if (!sock_owned_by_user(sk)) {
		opt  = &po->proto.pptp;

		orig_ack = opt->ack_recv;
		orig_seq = opt->seq_recv;
		
		/* test if acknowledgement present */
		if (PPTP_GRE_IS_A(grehdr->ver)) {
			__u32 ack = (PPTP_GRE_IS_S(grehdr->flags)) ?
					grehdr->ack : grehdr->seq; /* ack in different place if S = 0 */
	
			ack = ntohl(ack);
	
			if (ack > opt->ack_recv)
				opt->ack_recv = ack;
			/* also handle sequence number wrap-around	*/
			if (WRAPPED(ack, opt->ack_recv))
				opt->ack_recv = ack;
		}

		grehdrsize = sizeof(*grehdr);
		payload_len = ntohs(grehdr->payload_len);
		seq         = ntohl(grehdr->seq);

		if (!PPTP_GRE_IS_A(grehdr->ver))
			grehdrsize -= sizeof(grehdr->ack);
		headersize = iphdrsize + grehdrsize;
		/* check for incomplete packet (length smaller than expected) */
		if ((*skb)->len - headersize < payload_len) {
			ret = 1;
			goto drop;
		}
		
		payload = (*skb)->data + headersize;
		if (seq < opt->seq_recv + 1 || WRAPPED(opt->seq_recv, seq)) {
			opt->ack_recv = orig_ack;
			goto out;
		}

		opt->seq_recv = seq;
		
		if (payload[0] == PPP_ALLSTATIONS && payload[1] == PPP_UI) {
			ppphdrsize = 2;//don't include protocol byte(2)
			headersize += 2;
		}

		//printk("total len %d iphdrlen %d grehdrlen %d ppphdrlen %d payloadlen %d skb->len %d\n", iph->tot_len, iphdrsize, grehdrsize, ppphdrsize, 
		//	payload_len, skb->len);
		skb_pull(*skb, headersize);

#if 1
		if ( (!((*(*skb)->data) & 1)) && (*(__u16 *)(*skb)->data == PPP_COMP) )
			compressed = 1;
			
		//printk("%s proto:0x%x\n", __func__, *(__u16 *)skb->data);
		if ( (!((*(*skb)->data) & 1)) &&
			((*(__u16 *)(*skb)->data == PPP_IP) || (*(__u16 *)(*skb)->data == PPP_COMP)) && 
			((ret = vpn_ppp_down_fastpath(&po->chan, skb, headersize)) == 1))
			goto out;

		/*if decompress done, we should modify gre hdr and ip header*/
		if (compressed) {
			skb_push(*skb, ppphdrsize+grehdrsize);
			grehdr = (struct pptp_gre_header*)(*skb)->data;
			grehdr->payload_len = (*skb)->len-grehdrsize;

			skb_push(*skb, iphdrsize);
			iph = (struct iphdr*)(*skb)->data;
			iph->tot_len = (*skb)->len;
			ip_send_check((struct iphdr *)iph);
		}
		else
			skb_push(*skb, headersize);
		
		opt->ack_recv = orig_ack;
		opt->seq_recv = orig_seq;
		skb_reset_network_header(*skb);
		skb_set_mac_header(*skb, -ETH_HLEN);

		//printk("%s normal path.\n", __func__);
#else
		//printk("%s proto:0x%x\n", __func__, *(__u16 *)(*skb)->data);
		if (((*(*skb)->data) & 1)/* protocol iscompressed*/ || 
			(*(__u16 *)(*skb)->data != PPP_IP) || 
			((ret = vpn_ppp_down_fastpath(&po->chan, skb, headersize)) != 1)) {
			opt->ack_recv = orig_ack;
			opt->seq_recv = orig_seq;
			skb_push(*skb, headersize);
			skb_reset_network_header(*skb);
			skb_set_mac_header(*skb, -ETH_HLEN);
		}
#endif

		goto out;
	}

drop:
	printk("%s drop packet\n", __func__);
	dev_kfree_skb(*skb);
out:
	bh_unlock_sock(sk);
	sock_put(sk);
	return ret;
}
#endif

#ifdef CONFIG_IPV6_VPN
static int ipv6_pptp_xmit(struct ppp_channel *chan, struct sk_buff *skb)
{
	struct sock *sk = (struct sock *) chan->private;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	struct pptp_gre_header *hdr;
	unsigned int header_len = sizeof(*hdr);
	int err = 0;
	int islcp;
	int len;
	unsigned char *data;
	__u32 seq_recv;


	struct flowi fl;
	struct inet_sock *inet = inet_sk(sk);
	struct ipv6_pinfo *np = inet6_sk(sk);
	struct in6_addr *final_p = NULL, final;
	struct dst_entry *dst;
	struct net_device *tdev;
	int    max_headroom;


	dst = __sk_dst_get(sk);
	if (dst == NULL) {
		memset(&fl, 0, sizeof(fl));
		fl.proto = IPPROTO_GRE;
		ipv6_addr_copy(&fl.fl6_dst, &opt->dst_addr.sin_addr.in6);
		ipv6_addr_copy(&fl.fl6_src, &opt->src_addr.sin_addr.in6);
		fl.fl6_flowlabel = np->flow_label;
		IP6_ECN_flow_xmit(sk, fl.fl6_flowlabel);
		fl.oif = sk->sk_bound_dev_if;
		fl.fl_ip_sport = inet->sport;
		fl.fl_ip_dport = inet->dport;
		security_sk_classify_flow(sk, &fl);
		
		if (np->opt && np->opt->srcrt) {
			struct rt0_hdr *rt0 = (struct rt0_hdr *)np->opt->srcrt;
			ipv6_addr_copy(&final, &fl.fl6_dst);
			ipv6_addr_copy(&fl.fl6_dst, rt0->addr);
			final_p = &final;
		}
		
		err = ip6_dst_lookup(sk, &dst, &fl);

		if (err) {
			sk->sk_err_soft = -err;
			kfree_skb(skb);
			return err;
		}

		if (final_p)
			ipv6_addr_copy(&fl.fl6_dst, final_p);

		sk_setup_caps(sk, dst);
	}

	tdev = dst->dev;

	max_headroom = LL_RESERVED_SPACE(tdev) + sizeof(struct ipv6hdr) + sizeof(*hdr) + 2;

	if (skb_headroom(skb) < max_headroom || skb_cloned(skb) || skb_shared(skb)) {
		struct sk_buff *new_skb = skb_realloc_headroom(skb, max_headroom);
		if (!new_skb) {
			goto tx_error;
		}
		if (skb->sk)
			skb_set_owner_w(new_skb, skb->sk);
		kfree_skb(skb);
		skb = new_skb;
	}

	data = skb->data;
	islcp = ((data[0] << 8) + data[1]) == PPP_LCP && 1 <= data[2] && data[2] <= 7;

	/* compress protocol field */
	if ((opt->ppp_flags & SC_COMP_PROT) && data[0] == 0 && !islcp)
		skb_pull(skb, 1);

	/* Put in the address/control bytes if necessary */
	if ((opt->ppp_flags & SC_COMP_AC) == 0 || islcp) {
		data = skb_push(skb, 2);
		data[0] = PPP_ALLSTATIONS;
		data[1] = PPP_UI;
	}

	len = skb->len;

	seq_recv = opt->seq_recv;

	if (opt->ack_sent == seq_recv)
		header_len -= sizeof(hdr->ack);

	/* Push down and install GRE header */
	skb_push(skb, header_len);
	hdr = (struct pptp_gre_header *)(skb->data);

	hdr->flags		 = PPTP_GRE_FLAG_K;
	hdr->ver		 = PPTP_GRE_VER;
	hdr->protocol	 = htons(PPTP_GRE_PROTO);
	hdr->call_id	 = htons(opt->dst_addr.call_id);

	hdr->flags		|= PPTP_GRE_FLAG_S;
	hdr->seq		 = htonl(++opt->seq_sent);
	if (opt->ack_sent != seq_recv)	{
		/* send ack with this message */
		hdr->ver |= PPTP_GRE_FLAG_A;
		hdr->ack  = htonl(seq_recv);
		opt->ack_sent = seq_recv;
	}
	hdr->payload_len = htons(len);

	/*	Push down and install the IP header. */

	skb_reset_transport_header(skb);

	skb->dst = dst_clone(dst);

	/* Restore final destination back after routing done */
	ipv6_addr_copy(&fl.fl6_dst, &opt->dst_addr.sin_addr.in6);

	return ip6_xmit(sk, skb, &fl, np->opt, 0);

tx_error:
	kfree_skb(skb);
	return 1;
}

#endif

static int pptp_xmit(struct ppp_channel *chan, struct sk_buff *skb)
{
	struct sock *sk = (struct sock *) chan->private;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	struct pptp_gre_header *hdr;
	unsigned int header_len = sizeof(*hdr);
	int err = 0;
	int islcp;
	int len;
	unsigned char *data;
	__u32 seq_recv;


	struct rtable *rt;
	struct net_device *tdev;
	struct iphdr  *iph;
	int    max_headroom;

	//printk("%s %d\n", __func__, __LINE__);
	if (sk_pppox(po)->sk_state & PPPOX_DEAD)
		goto tx_error;

#ifdef CONFIG_IPV6_VPN
	if (1 == opt->dst_addr.ipType)
	{
		return ipv6_pptp_xmit(chan, skb);
	}
#endif

	{
		struct flowi fl = { .oif = 0,
			.nl_u = {
				.ip4_u = {
#ifdef CONFIG_IPV6_VPN
					.daddr = opt->dst_addr.sin_addr.in.s_addr,
					.saddr = opt->src_addr.sin_addr.in.s_addr,
#else
					.daddr = opt->dst_addr.sin_addr.s_addr,
					.saddr = opt->src_addr.sin_addr.s_addr,
#endif
					.tos = RT_TOS(0) } },
			.proto = IPPROTO_GRE };
		err = ip_route_output_key(&init_net, &rt, &fl);
		if (err)
		goto tx_error;
	}
	tdev = rt->u.dst.dev;

	max_headroom = LL_RESERVED_SPACE(tdev) + sizeof(*iph) + sizeof(*hdr) + 2;

	if (skb_headroom(skb) < max_headroom || skb_cloned(skb) || skb_shared(skb)) {
		struct sk_buff *new_skb = skb_realloc_headroom(skb, max_headroom);
		if (!new_skb) {
			ip_rt_put(rt);
			goto tx_error;
		}
		if (skb->sk)
			skb_set_owner_w(new_skb, skb->sk);
		kfree_skb(skb);
		skb = new_skb;
	}

	data = skb->data;
	islcp = ((data[0] << 8) + data[1]) == PPP_LCP && 1 <= data[2] && data[2] <= 7;

	/* compress protocol field */
	if ((opt->ppp_flags & SC_COMP_PROT) && data[0] == 0 && !islcp)
		skb_pull(skb, 1);

	/* Put in the address/control bytes if necessary */
	if ((opt->ppp_flags & SC_COMP_AC) == 0 || islcp) {
		data = skb_push(skb, 2);
		data[0] = PPP_ALLSTATIONS;
		data[1] = PPP_UI;
	}

	len = skb->len;

	seq_recv = opt->seq_recv;

	if (opt->ack_sent == seq_recv)
		header_len -= sizeof(hdr->ack);

	/* Push down and install GRE header */
	skb_push(skb, header_len);
	hdr = (struct pptp_gre_header *)(skb->data);

	hdr->flags       = PPTP_GRE_FLAG_K;
	hdr->ver         = PPTP_GRE_VER;
	hdr->protocol    = htons(PPTP_GRE_PROTO);
	hdr->call_id     = htons(opt->dst_addr.call_id);

	hdr->flags      |= PPTP_GRE_FLAG_S;
	hdr->seq         = htonl(++opt->seq_sent);
	if (opt->ack_sent != seq_recv)	{
		/* send ack with this message */
		hdr->ver |= PPTP_GRE_FLAG_A;
		hdr->ack  = htonl(seq_recv);
		opt->ack_sent = seq_recv;
	}
	hdr->payload_len = htons(len);

	/*	Push down and install the IP header. */

	skb_reset_transport_header(skb);
	skb_push(skb, sizeof(*iph));
	skb_reset_network_header(skb);
	memset(&(IPCB(skb)->opt), 0, sizeof(IPCB(skb)->opt));
	IPCB(skb)->flags &= ~(IPSKB_XFRM_TUNNEL_SIZE | IPSKB_XFRM_TRANSFORMED | IPSKB_REROUTED);

	iph =	ip_hdr(skb);
	iph->version =	4;
	iph->ihl =	sizeof(struct iphdr) >> 2;
	if (ip_dont_fragment(sk, &rt->u.dst))
		iph->frag_off	=	htons(IP_DF);
	else
		iph->frag_off	=	0;
	iph->protocol = IPPROTO_GRE;
	iph->tos      = 0;
	iph->daddr    = rt->rt_dst;
	iph->saddr    = rt->rt_src;
	iph->ttl      = dst_metric(&rt->u.dst, RTAX_HOPLIMIT);
	iph->tot_len  = htons(skb->len);

	dst_release(skb->dst);
	skb->dst = &rt->u.dst;

	nf_reset(skb);

	skb->ip_summed = CHECKSUM_NONE;
	ip_select_ident(iph, &rt->u.dst, NULL);
#ifdef CONFIG_RTL867X_IPTABLES_FAST_PATH
	iphdr_id = iph->id;
#endif
	ip_send_check(iph);

#if 0
	skb_set_mac_header(skb, -14);
#else
	skb->mac_header = NULL;
#endif
	
	ip_local_out(skb);
	return 1;

tx_error:
	kfree_skb(skb);
	return 1;
}

static int pptp_rcv_core(struct sock *sk, struct sk_buff *skb)
{
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	int headersize, payload_len, seq;
	__u8 *payload;
	struct pptp_gre_header *header;

	if (!(sk->sk_state & PPPOX_CONNECTED)) {
		if (sock_queue_rcv_skb(sk, skb))
			goto drop;
		return NET_RX_SUCCESS;
	}

	header = (struct pptp_gre_header *)(skb->data);

	/* test if acknowledgement present */
	if (PPTP_GRE_IS_A(header->ver)) {
		__u32 ack = (PPTP_GRE_IS_S(header->flags)) ?
				header->ack : header->seq; /* ack in different place if S = 0 */

		ack = ntohl(ack);

		if (ack > opt->ack_recv)
			opt->ack_recv = ack;
		/* also handle sequence number wrap-around  */
		if (WRAPPED(ack, opt->ack_recv))
			opt->ack_recv = ack;
	}

	/* test if payload present */
	if (!PPTP_GRE_IS_S(header->flags))
		goto drop;

	headersize  = sizeof(*header);
	payload_len = ntohs(header->payload_len);
	seq         = ntohl(header->seq);

	/* no ack present? */
	if (!PPTP_GRE_IS_A(header->ver))
		headersize -= sizeof(header->ack);
	/* check for incomplete packet (length smaller than expected) */
	if (skb->len - headersize < payload_len)
		goto drop;

	payload = skb->data + headersize;
	/* check for expected sequence number */
	if (seq < opt->seq_recv + 1 || WRAPPED(opt->seq_recv, seq)) {
		if ((payload[0] == PPP_ALLSTATIONS) && (payload[1] == PPP_UI) &&
				(PPP_PROTOCOL(payload) == PPP_LCP) &&
				((payload[4] == PPP_LCP_ECHOREQ) || (payload[4] == PPP_LCP_ECHOREP)))
			goto allow_packet;
	} else {
		opt->seq_recv = seq;
allow_packet:
		skb_pull(skb, headersize);

		if (payload[0] == PPP_ALLSTATIONS && payload[1] == PPP_UI) {
			/* chop off address/control */
			if (skb->len < 3)
				goto drop;
			skb_pull(skb, 2);
		}

		if ((*skb->data) & 1) {
			/* protocol is compressed */
			skb_push(skb, 1)[0] = 0;
		}

		skb->ip_summed = CHECKSUM_NONE;
		skb_set_network_header(skb, skb->head-skb->data);
		ppp_input(&po->chan, skb);

		return NET_RX_SUCCESS;
	}
drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

static int pptp_rcv(struct sk_buff *skb)
{
	struct pppox_sock *po;
	struct pptp_gre_header *header;
	struct iphdr *iph;

	if (skb->pkt_type != PACKET_HOST)
		goto drop;

	if (!pskb_may_pull(skb, 12))
		goto drop;

	iph = ip_hdr(skb);

	header = (struct pptp_gre_header *)skb->data;

	if (ntohs(header->protocol) != PPTP_GRE_PROTO || /* PPTP-GRE protocol for PPTP */
		PPTP_GRE_IS_C(header->flags) ||                /* flag C should be clear */
		PPTP_GRE_IS_R(header->flags) ||                /* flag R should be clear */
		!PPTP_GRE_IS_K(header->flags) ||               /* flag K should be set */
		(header->flags&0xF) != 0)                      /* routing and recursion ctrl = 0 */
		/* if invalid, discard this packet */
		goto drop;

	po = lookup_chan(htons(header->call_id), iph->saddr);
	if (po) {
		//skb_dst_drop(skb);
		dst_release(skb->dst);
		nf_reset(skb);
		return sk_receive_skb(sk_pppox(po), skb, 0);
	}
drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}

#ifdef CONFIG_IPV6_VPN
static int ipv6_pptp_rcv(struct sk_buff *skb)
{
	struct pppox_sock *po;
	struct pptp_gre_header *header;
	struct ipv6hdr *iph;

	if (skb->pkt_type != PACKET_HOST)
		goto drop;

	if (!pskb_may_pull(skb, 12))
		goto drop;

	iph = ipv6_hdr(skb);

	header = (struct pptp_gre_header *)skb->data;

	if (ntohs(header->protocol) != PPTP_GRE_PROTO || /* PPTP-GRE protocol for PPTP */
		PPTP_GRE_IS_C(header->flags) || 			   /* flag C should be clear */
		PPTP_GRE_IS_R(header->flags) || 			   /* flag R should be clear */
		!PPTP_GRE_IS_K(header->flags) ||			   /* flag K should be set */
		(header->flags&0xF) != 0)					   /* routing and recursion ctrl = 0 */
		/* if invalid, discard this packet */
		goto drop;

	po = ipv6_lookup_chan(htons(header->call_id), iph->saddr);
	if (po) {
		//skb_dst_drop(skb);
		dst_release(skb->dst);
		nf_reset(skb);
		return sk_receive_skb(sk_pppox(po), skb, 0);
	}
drop:
	kfree_skb(skb);
	return NET_RX_DROP;
}
#endif

static int pptp_bind(struct socket *sock, struct sockaddr *uservaddr,
	int sockaddr_len)
{
	struct sock *sk = sock->sk;
	struct sockaddr_pppox *sp = (struct sockaddr_pppox *) uservaddr;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	int error = 0;

	lock_sock(sk);

	opt->src_addr = sp->sa_addr.pptp;
	if (add_chan(po)) {
		release_sock(sk);
		error = -EBUSY;
	}

	release_sock(sk);
	return error;
}

static int pptp_connect(struct socket *sock, struct sockaddr *uservaddr,
	int sockaddr_len, int flags)
{
	struct sock *sk = sock->sk;
	struct sockaddr_pppox *sp = (struct sockaddr_pppox *) uservaddr;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	struct rtable *rt;
	int ipver = 4;
	int error = 0;

	if (sp->sa_protocol != PX_PROTO_PPTP)
		return -EINVAL;

#ifdef CONFIG_IPV6_VPN
	if (1 == sp->sa_addr.pptp.ipType)
		ipver = 6;
#endif
	
#ifdef CONFIG_IPV6_VPN
	if ((4 == ipver) && lookup_chan_dst(sp->sa_addr.pptp.call_id, sp->sa_addr.pptp.sin_addr.in.s_addr))
		return -EALREADY;
	else if ((6 == ipver) &&
		ipv6_lookup_chan_dst(sp->sa_addr.pptp.call_id, sp->sa_addr.pptp.sin_addr.in6))
		return -EALREADY;
#else
	if ((4 == ipver) && lookup_chan_dst(sp->sa_addr.pptp.call_id, sp->sa_addr.pptp.sin_addr.s_addr))
		return -EALREADY;
#endif

	lock_sock(sk);
	/* Check for already bound sockets */
	if (sk->sk_state & PPPOX_CONNECTED) {
		error = -EBUSY;
		goto end;
	}

	/* Check for already disconnected sockets, on attempts to disconnect */
	if (sk->sk_state & PPPOX_DEAD) {
		error = -EALREADY;
		goto end;
	}

#ifndef CONFIG_IPV6_VPN
	if (!opt->src_addr.sin_addr.s_addr || !sp->sa_addr.pptp.sin_addr.s_addr) {
		error = -EINVAL;
		goto end;
	}
#else
	if (((4 == ipver) && !opt->src_addr.sin_addr.in.s_addr) || 
		((6 == ipver) && ipv6_addr_any(&opt->src_addr.sin_addr.in6)) ||
		((4 == ipver) && !sp->sa_addr.pptp.sin_addr.in.s_addr) ||
		((6 == ipver) && ipv6_addr_any(&sp->sa_addr.pptp.sin_addr.in6))) {
		error = -EINVAL;
		goto end;
	}
#endif

	po->chan.private = sk;
	po->chan.ops = &pptp_chan_ops;

#ifdef CONFIG_IPV6_VPN
	if (4 == ipver)
#endif// end of CONFIG_IPV6_VPN
	{
		struct flowi fl = {
			.nl_u = {
				.ip4_u = {
#ifdef CONFIG_IPV6_VPN
					.daddr = opt->dst_addr.sin_addr.in.s_addr,
					.saddr = opt->src_addr.sin_addr.in.s_addr,
#else
					.daddr = opt->dst_addr.sin_addr.s_addr,
					.saddr = opt->src_addr.sin_addr.s_addr,
#endif
					.tos = RT_CONN_FLAGS(sk) } },
			.proto = IPPROTO_GRE };
		security_sk_classify_flow(sk, &fl);
		if (ip_route_output_key(&init_net, &rt, &fl)) {
			error = -EHOSTUNREACH;
			goto end;
		}
		sk_setup_caps(sk, &rt->u.dst);
	}
#ifdef CONFIG_IPV6_VPN
	else {
		//maybe we can get ipv6 dst_entry now.
	}
#endif
	po->chan.mtu = dst_mtu(&rt->u.dst);
	if (!po->chan.mtu)
		po->chan.mtu = PPP_MTU;
	ip_rt_put(rt);
	po->chan.mtu -= PPTP_HEADER_OVERHEAD;

	po->chan.hdrlen = 2 + sizeof(struct pptp_gre_header);
	error = ppp_register_channel(&po->chan);
	if (error) {
		pr_err("PPTP: failed to register PPP channel (%d)\n", error);
		goto end;
	}

	opt->dst_addr = sp->sa_addr.pptp;
	sk->sk_state = PPPOX_CONNECTED;

 end:
	release_sock(sk);
	return error;
}

static int pptp_getname(struct socket *sock, struct sockaddr *uaddr,
	int *usockaddr_len, int peer)
{
	int len = sizeof(struct sockaddr_pppox);
	struct sockaddr_pppox sp;

	sp.sa_family	  = AF_PPPOX;
	sp.sa_protocol  = PX_PROTO_PPTP;
	sp.sa_addr.pptp = pppox_sk(sock->sk)->proto.pptp.src_addr;

	memcpy(uaddr, &sp, len);

	*usockaddr_len = len;

	return 0;
}

static int pptp_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct pppox_sock *po;
	struct pptp_opt *opt;
	int error = 0;

	if (!sk)
		return 0;

	lock_sock(sk);

	if (sock_flag(sk, SOCK_DEAD)) {
		release_sock(sk);
		return -EBADF;
	}

	po = pppox_sk(sk);
	opt = &po->proto.pptp;
	del_chan(po);

	pppox_unbind_sock(sk);
	sk->sk_state = PPPOX_DEAD;

	sock_orphan(sk);
	sock->sk = NULL;

	release_sock(sk);
	sock_put(sk);

	return error;
}

static void pptp_sock_destruct(struct sock *sk)
{
	if (!(sk->sk_state & PPPOX_DEAD)) {
		del_chan(pppox_sk(sk));
		pppox_unbind_sock(sk);
	}
	skb_queue_purge(&sk->sk_receive_queue);
}

static int pptp_create(struct net *net, struct socket *sock)
{
	int error = -ENOMEM;
	struct sock *sk;
	struct pppox_sock *po;
	struct pptp_opt *opt;

	sk = sk_alloc(net, PF_PPPOX, GFP_KERNEL, &pptp_sk_proto);
	if (!sk)
		goto out;

	sock_init_data(sock, sk);

	sock->state = SS_UNCONNECTED;
	sock->ops   = &pptp_ops;

	sk->sk_backlog_rcv = pptp_rcv_core;
	sk->sk_state       = PPPOX_NONE;
	sk->sk_type        = SOCK_STREAM;
	sk->sk_family      = PF_PPPOX;
	sk->sk_protocol    = PX_PROTO_PPTP;
	sk->sk_destruct    = pptp_sock_destruct;

	po = pppox_sk(sk);
	opt = &po->proto.pptp;

	opt->seq_sent = 0; opt->seq_recv = 0;
	opt->ack_recv = 0; opt->ack_sent = 0;

	error = 0;
out:
	return error;
}

static int pptp_ppp_ioctl(struct ppp_channel *chan, unsigned int cmd,
	unsigned long arg)
{
	struct sock *sk = (struct sock *) chan->private;
	struct pppox_sock *po = pppox_sk(sk);
	struct pptp_opt *opt = &po->proto.pptp;
	#if 0
	void __user *argp = (void __user *)arg;
	int __user *p = argp;
	#else
	void *argp = (void *)arg;
	int *p = argp;
	#endif
	int err, val;

	err = -EFAULT;
	switch (cmd) {
	case PPPIOCGFLAGS:
		val = opt->ppp_flags;
		#if 0
		if (put_user(val, p))
			break;
		#else
		*p = val;
		#endif
		err = 0;
		break;
	case PPPIOCSFLAGS:
		#if 0
		if (get_user(val, p))
			break;
		#else
		val = *p;
		#endif
		opt->ppp_flags = val & ~SC_RCV_BITS;
		err = 0;
		break;
	default:
		err = -ENOTTY;
	}

	return err;
}

/*static */const struct ppp_channel_ops pptp_chan_ops = {
	.start_xmit = pptp_xmit,
	.ioctl      = pptp_ppp_ioctl,
};

static struct proto pptp_sk_proto __read_mostly = {
	.name     = "PPTP",
	.owner    = THIS_MODULE,
	.obj_size = sizeof(struct pppox_sock),
};

static const struct proto_ops pptp_ops = {
	.family     = AF_PPPOX,
	.owner      = THIS_MODULE,
	.release    = pptp_release,
	.bind       = pptp_bind,
	.connect    = pptp_connect,
	.socketpair = sock_no_socketpair,
	.accept     = sock_no_accept,
	.getname    = pptp_getname,
	.poll       = sock_no_poll,
	.listen     = sock_no_listen,
	.shutdown   = sock_no_shutdown,
	.setsockopt = sock_no_setsockopt,
	.getsockopt = sock_no_getsockopt,
	.sendmsg    = sock_no_sendmsg,
	.recvmsg    = sock_no_recvmsg,
	.mmap       = sock_no_mmap,
	.ioctl      = pppox_ioctl,
};

static const struct pppox_proto pppox_pptp_proto = {
	.create = pptp_create,
	.owner  = THIS_MODULE,
};

static const struct gre_protocol gre_pptp_protocol = {
	.handler = pptp_rcv,
};

#ifdef CONFIG_IPV6_VPN
static const struct gre_protocol ipv6_gre_pptp_protocol = {
	.handler = ipv6_pptp_rcv,
};
#endif

static int __init pptp_init_module(void)
{
	int err = 0;
	pr_info("PPTP driver version " PPTP_DRIVER_VERSION "\n");

	callid_sock = __vmalloc((MAX_CALLID + 1) * sizeof(void *),
		GFP_KERNEL | __GFP_ZERO, PAGE_KERNEL);
	if (!callid_sock) {
		pr_err("PPTP: cann't allocate memory\n");
		return -ENOMEM;
	}

	/* QL: support version 0 */
	err = gre_add_protocol(&gre_pptp_protocol, GREPROTO_CISCO);
	if (err) {
		pr_err("PPTP: can't add gre protocol\n");
		goto out_mem_free1;
	}
	
	err = gre_add_protocol(&gre_pptp_protocol, GREPROTO_PPTP);
	if (err) {
		pr_err("PPTP: can't add gre protocol\n");
		goto out_mem_free2;
	}

#ifdef CONFIG_IPV6_VPN
	err = ipv6_gre_add_protocol(&ipv6_gre_pptp_protocol, GREPROTO_CISCO);
	if (err) {
		pr_err("PPTP: can't add gre protocol\n");
		goto out_mem_free1;
	}
	
	err = ipv6_gre_add_protocol(&ipv6_gre_pptp_protocol, GREPROTO_PPTP);
	if (err) {
		pr_err("PPTP: can't add gre protocol\n");
		goto out_mem_free2;
	}
#endif

	err = proto_register(&pptp_sk_proto, 0);
	if (err) {
		pr_err("PPTP: can't register sk_proto\n");
		goto out_gre_del_protocol;
	}

	err = register_pppox_proto(PX_PROTO_PPTP, &pppox_pptp_proto);
	if (err) {
		pr_err("PPTP: can't register pppox_proto\n");
		goto out_unregister_sk_proto;
	}

	return 0;

out_unregister_sk_proto:
	proto_unregister(&pptp_sk_proto);
out_gre_del_protocol:
	gre_del_protocol(&gre_pptp_protocol, GREPROTO_PPTP);
out_mem_free2:
	gre_del_protocol(&gre_pptp_protocol, GREPROTO_CISCO);
out_mem_free1:
	vfree(callid_sock);
	return err;
}

static void __exit pptp_exit_module(void)
{
	unregister_pppox_proto(PX_PROTO_PPTP);
	proto_unregister(&pptp_sk_proto);
	gre_del_protocol(&gre_pptp_protocol, GREPROTO_PPTP);
	/* QL: support version 0 */
	gre_del_protocol(&gre_pptp_protocol, GREPROTO_CISCO);
#ifdef CONFIG_IPV6_VPN
	ipv6_gre_del_protocol(&ipv6_gre_pptp_protocol, GREPROTO_PPTP);
	/* QL: support version 0 */
	ipv6_gre_del_protocol(&ipv6_gre_pptp_protocol, GREPROTO_CISCO);
#endif
	vfree(callid_sock);
}

module_init(pptp_init_module);
module_exit(pptp_exit_module);

MODULE_DESCRIPTION("Point-to-Point Tunneling Protocol");
MODULE_AUTHOR("D. Kozlov (xeb@mail.ru)");
MODULE_LICENSE("GPL");
