#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>
#include <net/udp.h>


#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_ipsec.h>


//#ifdef CONFIG_IP_NF_ALG_ONOFF
//#define PROCFS_NAME "algonoff_ipsec"
//ALGONOFF_INIT(ipsec)
//#endif


/*
 * Prototypes of IPSec NAT hook
 */
unsigned int (*ip_nat_ipsec_inbound_hook) 
				(struct sk_buff *skb,
				struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp) __read_mostly;
EXPORT_SYMBOL_GPL(ip_nat_ipsec_inbound_hook);

unsigned int (*ip_nat_ipsec_outbound_hook) 
				(struct sk_buff *skb,
				 struct nf_conn *ct,
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp) __read_mostly;
EXPORT_SYMBOL_GPL(ip_nat_ipsec_outbound_hook);

#ifdef IPSEC_ESP_HOOK
unsigned int (*ip_nat_esp_hook)	
				(struct sk_buff *skb,
				struct nf_conn *ct,	
				 enum ip_conntrack_info ctinfo,
				 struct nf_conntrack_expect *exp) __read_mostly;
EXPORT_SYMBOL_GPL(ip_nat_esp_hook);
#endif //IPSEC_ESP_HOOK


#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif


static int ports[IPSEC_MAX_PORTS];
static int ports_n_c = 0;


#ifdef IPSEC_ESP_HOOK
extern char ipsec_flag;
static unsigned long ip_ct_esp_timeout = 30 *HZ;
static unsigned long ip_ct_esp_timeout_stream = 180*HZ;

static bool esp_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff,
			     struct nf_conntrack_tuple *tuple)
{
	struct udphdr _hdr, *hdr;

	hdr = skb_header_pointer(skb, dataoff, sizeof(_hdr), &_hdr);
	if(hdr == NULL)
		return false;

	tuple->src.u.all = hdr->source;
	tuple->dst.u.all = hdr->dest;

	DEBUGP("%s: tuple->src = src %x, tuple->dst = dst %x\n", __func__, tuple->src.u.all, tuple->dst.u.all);
	
	return true;
}

static bool esp_invert_tuple(struct nf_conntrack_tuple *tuple,
			     const struct nf_conntrack_tuple *orig)
{
	tuple->src.u.all = orig->dst.u.all;
	tuple->dst.u.all = orig->src.u.all;

	DEBUGP("%s: tuple->src = dst %x, dst = src %x\n", __func__, orig->dst.u.all, orig->src.u.all);

	return true;
}

/* Print out the per-protocol part of the tuple. */
static int esp_print_tuple(struct seq_file *buffer,
			   const struct nf_conntrack_tuple *tuple)
{

	return seq_printf(buffer, "sport=%hu dport=%hu ",
		       ntohs(tuple->src.u.all),
		       ntohs(tuple->dst.u.all));
}

/* Print out the private part of the conntrack. */
static int esp_print_conntrack(struct seq_file *s, const struct nf_conn *ct)
{
	return 0;
}

/* Returns verdict for packet, and may modify conntracktype */
static int esp_packet(struct nf_conn *ct,
		      const struct sk_buff *skb,
		      unsigned int dataoff,
		      enum ip_conntrack_info ctinfo,
		      u_int8_t pf,
		      unsigned int hooknum)
{
	typeof(ip_nat_esp_hook) ip_nat_esp;

	ip_nat_esp = rcu_dereference(ip_nat_esp_hook);
	if (ip_nat_esp) {
		if (!ip_nat_esp(skb, ct, ctinfo, NULL)) 
			return  NF_DROP;
	}

	/* If we've seen traffic both ways, this is some kind of UDP
	   stream.  Extend timeout. */
	if(ctinfo== IP_CT_IS_REPLY)
	{
		nf_ct_refresh(ct, skb, ip_ct_esp_timeout_stream);	
		DEBUGP("%s: refresh ip_ct on kinda UDP stream.\n", __func__);

		/* Also, more likely to be important, and not a probe */
		set_bit(IPS_ASSURED_BIT, &ct->status);
		nf_conntrack_event_cache(IPCT_STATUS, ct);
	} else {
		nf_ct_refresh(ct, skb, ip_ct_esp_timeout);
		DEBUGP("%s: refresh ip_ct with shorter expected timeout.\n", __func__);
	}

	return NF_ACCEPT;
}

/* Called when a new connection for this protocol found. */
static bool esp_new(struct nf_conn *ct, const struct sk_buff *skb,
		    unsigned int dataoff)
{	
	return 1;
}

static int esp_error(struct net *net, struct sk_buff *skb, unsigned int dataoff,
		     enum ip_conntrack_info *ctinfo,
		     u_int8_t pf, unsigned int hooknum)

{
	return 1;
}

struct nf_conntrack_l4proto ip_conntrack_protocol_esp = 
{
	.l3proto		 = PF_INET,
	.l4proto		 = IPPROTO_ESP,	
	.name		 = "esp",
	.pkt_to_tuple	 = esp_pkt_to_tuple,
	.invert_tuple	 = esp_invert_tuple,
	.print_tuple	 = esp_print_tuple,
	.print_conntrack = esp_print_conntrack,
	.packet		 = esp_packet,
	.new		 = esp_new,
	.error 		 = esp_error,
	.destroy	 = NULL,
	.me 		 = THIS_MODULE,
#if defined(CONFIG_IP_NF_CONNTRACK_NETLINK) || \
    defined(CONFIG_IP_NF_CONNTRACK_NETLINK_MODULE)
      //Fix me later- shlee
	.tuple_to_nfattr = NULL,
	.nfattr_to_tuple = NULL,
#endif
};
#endif //IPSEC_ESP_HOOK


/* FIXME: This should be in userspace.  Later. */
static int help(struct sk_buff *skb,
		unsigned int protoff,
		struct nf_conn *ct,
		enum ip_conntrack_info ctinfo)
{
	struct nf_conntrack_expect * exp=NULL;
	int ret = NF_ACCEPT;

//#ifdef CONFIG_IP_NF_ALG_ONOFF
//	if(!algonoff_ipsec) {
//		printk("DROP ipsec PACKET!!!\n");
//		return NF_DROP;
//	}
//#endif


#if 1
	DEBUGP( "\n<ipsec ct help>\n" );
	DEBUGP( "ctinfo=%u\n", (unsigned int) ctinfo );
	DEBUGP( "ct->status=0x%x, mask IPS_NAT_MASK=0x%x\n", 
			ct->status, ct->status & IPS_NAT_MASK );
	{
		struct nf_conntrack_tuple *t;
		t=&ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
		DEBUGP("ori tuple %p: %u %pI4:%hu -> %pI4:%hu\n",
		       t, t->dst.protonum,
		       &t->src.u3.ip, ntohs(t->src.u.all),
		       &t->dst.u3.ip, ntohs(t->dst.u.all));
		t=&ct->tuplehash[IP_CT_DIR_REPLY].tuple;
		DEBUGP("rep tuple %p: %u %pI4:%hu -> %pI4:%hu\n",
		       t, t->dst.protonum,
		       &t->src.u3.ip, ntohs(t->src.u.all),
		       &t->dst.u3.ip, ntohs(t->dst.u.all));
	}
#endif


#if 1
	//workaround- refresh udp:500 conntrack with 10 seconds for 2st client connection.
	nf_ct_refresh_acct(ct, ctinfo, skb, 10*HZ);

	if(ct->status & IPS_NAT_MASK)
	{
		if( CTINFO2DIR(ctinfo)==IP_CT_DIR_ORIGINAL )
		{
			typeof(ip_nat_ipsec_outbound_hook) ip_nat_ipsec_out;

			ip_nat_ipsec_out = rcu_dereference(ip_nat_ipsec_outbound_hook);
			if (ip_nat_ipsec_out) 
				if (!ip_nat_ipsec_out(skb, ct, ctinfo, exp)) 
					return  NF_DROP;
		}else{
			typeof(ip_nat_ipsec_inbound_hook) ip_nat_ipsec_in;

			ip_nat_ipsec_in = rcu_dereference(ip_nat_ipsec_inbound_hook);
			if (ip_nat_ipsec_in) 
				if (!ip_nat_ipsec_in(skb, ct, ctinfo, exp)) 
					return  NF_DROP;
		}
	}
#else
	//workaround- refresh udp:500 conntrack with 10 seconds for 2st client connection.
	nf_ct_refresh_acct(ct, ctinfo, skb, 10*HZ);

	//Allocate expectation on next reply, just care bout src ip, dst ip and protocol
	exp = nf_ct_expect_alloc(ct);
	if (exp == NULL)
	{
		DEBUGP( "<ipsec ct help> call nf_ct_expect_alloc() failed! retrun NF_DROP\n" );
		return NF_DROP;
	}

	nf_ct_expect_init(exp, NF_CT_EXPECT_CLASS_DEFAULT, 
			  nf_ct_l3num(ct),
			  &ct->tuplehash[IP_CT_DIR_REPLY].tuple.src.u3, 
			  &ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u3,
			  nf_ct_protonum(ct), 
			  NULL, 
			  &ct->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all);

	//call outbound hook to record and update the track of ipsec 
	if (ip_nat_ipsec_outbound_hook) 
		if (!ip_nat_ipsec_outbound_hook(skb, ct, ctinfo, exp)) 
		{
			DEBUGP( "<ipsec ct help> call ip_nat_ipsec_outbound_hook() failed! retrun NF_DROP\n" );
			return  NF_DROP;
		}

	if (nf_ct_expect_related(exp) != 0)	
	{
		DEBUGP( "<ipsec ct help> call nf_ct_expect_related() failed! set ret=NF_DROP\n" );
		ret = NF_DROP;
	}

	nf_ct_expect_put(exp);
#endif


#ifdef IPSEC_ESP_HOOK
	if(ipsec_flag == '1') 
		nf_conntrack_l4proto_register(&ip_conntrack_protocol_esp);
#endif //IPSEC_ESP_HOOK

		
	return ret;
}

static struct nf_conntrack_helper ipsec_helpers[IPSEC_MAX_PORTS]; //__read_mostly;
static const struct nf_conntrack_expect_policy ipsec_exp_policy = {
	.max_expected	= 2,
	.timeout	= 180, 
};
static void fini(void);
static void alginit_ipsec(void)
{
	int i ;

//#ifdef CONFIG_IP_NF_ALG_ONOFF
//	ports_n_c=0;
//#endif

	/* If no port given, default to standard ipsec port */
	if (ports[0] == 0)
		ports[0] = IPSEC_PORT;


	for (i = 0; (i < IPSEC_MAX_PORTS) && ports[i]; i++) {
		memset(&ipsec_helpers[i], 0, sizeof(struct nf_conntrack_helper));

		ipsec_helpers[i].tuple.src.l3num = PF_INET;
		ipsec_helpers[i].tuple.src.u.udp.port = htons(ports[i]);
		ipsec_helpers[i].tuple.dst.protonum = IPPROTO_UDP;
		ipsec_helpers[i].expect_policy = &ipsec_exp_policy;
		ipsec_helpers[i].me = THIS_MODULE;
		ipsec_helpers[i].name = "ipsec";
		ipsec_helpers[i].help = help;

		if( nf_conntrack_helper_register(&ipsec_helpers[i]))
		{
			printk("nf_conntrack_ipsec: ERROR registering port %d\n",
				ports[i]);
			fini();
			return;
		}
		ports_n_c++;
	}
	printk("nf_conntrack_ipsec loaded\n");  

}

static int __init init(void)
{
//#ifdef CONFIG_IP_NF_ALG_ONOFF
//	Alg_OnOff_init_ipsec();
//#endif
	alginit_ipsec();
//#ifdef CONFIG_IP_NF_ALG_ONOFF
//	algonoff_ipsec=1;	
//#endif
	return 0;
}

/* This function is intentionally _NOT_ defined as __exit, because 
 * it is needed by the init function */
static void fini(void)
{
	int i;
	for (i = 0; (i < IPSEC_MAX_PORTS) && ports[i]; i++) {
		nf_conntrack_helper_unregister(&ipsec_helpers[i]);
	}
}

module_init(init);
module_exit(fini);
