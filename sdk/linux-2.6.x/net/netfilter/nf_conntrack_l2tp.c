/*
 * ip_conntrack_l2tp.c
 */

#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack.h> 
#include <net/netfilter/nf_conntrack_expect.h> 
#include <net/netfilter/nf_conntrack_helper.h> 

#define IP_CT_L2TP_VERSION "3.1"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Harald Welte <laforge@gnumonks.org>");
MODULE_DESCRIPTION("Netfilter connection tracking helper module for l2tp");
#ifdef  CONFIG_IP_NF_ALG_ONOFF
#define PROCFS_NAME "algonoff_l2tp"
ALGONOFF_INIT(l2tp);
#endif
#if 0
#define DEBUGP(format, args...)	printk(KERN_DEBUG "%s:%s: " format, __FILE__, __FUNCTION__, ## args)
#else
#define DEBUGP(format, args...)
#endif
#define L2TP_CONTROL_PORT 1701

/* track caller id inside control connection, call expect_related */
static int
conntrack_l2tp_help(struct sk_buff *skb, unsigned int protoff,
		    struct nf_conn *ct, enum ip_conntrack_info ctinfo)
{
#ifdef  CONFIG_IP_NF_ALG_ONOFF
	if(!algonoff_l2tp) {
		//printk("DROP L2TP PACKET!\n");
		return NF_DROP;
	}
#endif
	return NF_ACCEPT;
}

static const struct nf_conntrack_expect_policy l2tp_exp_policy = {
	.max_expected	= 2,
	.timeout	= 5 * 60,
};

/* control protocol helper */
static struct nf_conntrack_helper l2tp __read_mostly = {
	.name			= "l2tp",
	.me			= THIS_MODULE,
	.tuple.src.l3num	= AF_INET,
	.tuple.src.u.tcp.port	= cpu_to_be16(L2TP_CONTROL_PORT),
	.tuple.dst.protonum	= IPPROTO_UDP,
	.help			= conntrack_l2tp_help,	
	.expect_policy		= &l2tp_exp_policy,
};

/* nf_conntrack_pptp initialization */
int alginit_l2tp(void)
{
	int retcode;
	DEBUGP(" registering helper\n");
	if ((retcode = nf_conntrack_helper_register(&l2tp))) {
		printk(KERN_ERR "Unable to register conntrack application "
				"helper for l2tp: %d\n", retcode);		
		return retcode;
	}

	printk("nf_conntrack_l2tp version %s loaded\n", IP_CT_L2TP_VERSION);
	return 0;
}

static void  fini(void)
{
	nf_conntrack_helper_unregister(&l2tp);	
	printk("ip_conntrack_l2tp version %s unloaded\n", IP_CT_L2TP_VERSION);
}

static int __init init(void)
{
#ifdef CONFIG_IP_NF_ALG_ONOFF	
	Alg_OnOff_init_l2tp();	
#endif
	alginit_l2tp();
#ifdef CONFIG_IP_NF_ALG_ONOFF
	algonoff_l2tp=1;
#endif
	return 0;
}

module_init(init);
module_exit(fini);
