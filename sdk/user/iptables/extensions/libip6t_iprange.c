/* Shared library add-on to iptables to add IP range matching support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <netinet/in.h>
#include <xtables.h>
#include <ip6tables.h> 
#include <linux/netfilter_ipv6/ip6_tables.h> 
#include <linux/netfilter.h>
#include <linux/netfilter/xt_iprange.h>

extern const char *xtables_ip6addr_to_numeric(const struct in6_addr *);

struct ipt_iprange {
	/* Inclusive: network order. */
	__be32 min_ip, max_ip;
};

struct ipt_iprange_info {
	struct ipt_iprange src;
	struct ipt_iprange dst;

	/* Flags from above */
	u_int8_t flags;
};

enum {
	F_SRCIP = 1 << 0,
	F_DSTIP = 1 << 1,
};

static void iprange_mt_help(void)
{
	printf(
"iprange match options:\n"
"[!] --src-range ip[-ip]    Match source IP in the specified range\n"
"[!] --dst-range ip[-ip]    Match destination IP in the specified range\n");
}

static void
init(struct ip6t_entry_match *m, unsigned int *nfcache)
{	
	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

static const struct option iprange_mt_opts[] = {
	{"src-range", 1, 0, '1'},
	{"dst-range", 1, 0, '2'},
	{0}
};

static void
iprange_parse_spec(const char *from, const char *to, union nf_inet_addr *range,
		   uint8_t family, const char *optname)
{
	const char *spec[2] = {from, to};
	struct in6_addr *ia6;
	struct in_addr *ia4;
	unsigned int i;

	memset(range, 0, sizeof(union nf_inet_addr) * 2);

	if (family == NFPROTO_IPV6) {
		for (i = 0; i < ARRAY_SIZE(spec); ++i) {
			ia6 = xtables_numeric_to_ip6addr(spec[i]);
			if (ia6 == NULL)
				//xtables_param_act(XTF_BAD_VALUE, "iprange",
				//	optname, spec[i]);
				exit_error(PARAMETER_PROBLEM, "iprange: ia6 is NULL");
			range[i].in6 = *ia6;
		}
	} 
}

static void iprange_parse_range(char *arg, union nf_inet_addr *range,
				u_int8_t family, const char *optname)
{
	char *dash;

	dash = strchr(arg, '-');
	if (dash == NULL) {
		iprange_parse_spec(arg, arg, range, family, optname);
		return;
	}

	*dash = '\0';
	iprange_parse_spec(arg, dash + 1, range, family, optname);
	if (memcmp(&range[0], &range[1], sizeof(*range)) > 0)
		fprintf(stderr, "xt_iprange: range %s-%s is reversed and "
			"will never match\n", arg, dash + 1);
}

static int
iprange_mt6_parse(int c, char **argv, int invert, unsigned int *flags,
	const struct ip6t_entry *entry,
	unsigned int *nfcache,
	struct ip6t_entry_match **match)
{
	struct xt_iprange_mtinfo *info = (void *)(*match)->data;

	switch (c) {
	case '1': /* --src-range */
		iprange_parse_range(optarg, &info->src_min, NFPROTO_IPV6,
			"--src-range");
		info->flags |= IPRANGE_SRC;
		if (invert)
			info->flags |= IPRANGE_SRC_INV;
		*flags |= F_SRCIP;		
		return true;
		
	case '2': /* --dst-range */
		iprange_parse_range(optarg, &info->dst_min, NFPROTO_IPV6,
			"--dst-range");
		info->flags |= IPRANGE_DST;
		if (invert)
			info->flags |= IPRANGE_DST_INV;
		*flags |= F_DSTIP;		
		return true;	
	}	
	return false;	
}

static void iprange_mt_check(unsigned int flags)
{
	if (flags == 0)
		exit_error(PARAMETER_PROBLEM,
			   "iprange match: You must specify `--src-range' or `--dst-range'");	
}

static void
iprange_mt6_print(const struct ip6t_ip6 *ip,
	const struct ip6t_entry_match *match,
	int numeric)
{
	const struct xt_iprange_mtinfo *info = (const void *)match->data;

	if (info->flags & IPRANGE_SRC) {
		printf("source IP range ");
		if (info->flags & IPRANGE_SRC_INV)
			printf("! ");
		/*
		 * ipaddr_to_numeric() uses a static buffer, so cannot
		 * combine the printf() calls.
		 */
		printf("%s", xtables_ip6addr_to_numeric(&info->src_min.in6));
		printf("-%s ", xtables_ip6addr_to_numeric(&info->src_max.in6));
	}
	if (info->flags & IPRANGE_DST) {
		printf("destination IP range ");
		if (info->flags & IPRANGE_DST_INV)
			printf("! ");
		printf("%s", xtables_ip6addr_to_numeric(&info->dst_min.in6));
		printf("-%s ", xtables_ip6addr_to_numeric(&info->dst_max.in6));
	}
}

static void
iprange_mt6_save(const struct ip6t_ip6  *ip, const struct ip6t_entry_match *match)
{
	const struct xt_iprange_mtinfo *info = (const void *)match->data;

	if (info->flags & IPRANGE_SRC) {
		if (info->flags & IPRANGE_SRC_INV)
			printf("! ");
		printf("--src-range %s", xtables_ip6addr_to_numeric(&info->src_min.in6));
		printf("-%s ", xtables_ip6addr_to_numeric(&info->src_max.in6));
	}
	if (info->flags & IPRANGE_DST) {
		if (info->flags & IPRANGE_DST_INV)
			printf("! ");
		printf("--dst-range %s", xtables_ip6addr_to_numeric(&info->dst_min.in6));
		printf("-%s ", xtables_ip6addr_to_numeric(&info->dst_max.in6));
	}
}

static struct 
ip6tables_match iprange
= { NULL,
   "iprange",
   IPTABLES_VERSION,
   IP6T_ALIGN(sizeof(struct xt_iprange_mtinfo)),
   IP6T_ALIGN(sizeof(struct xt_iprange_mtinfo)),
   &iprange_mt_help,
   &init,
   &iprange_mt6_parse,
   &iprange_mt_check,
   &iprange_mt6_print,
   &iprange_mt6_save,
   iprange_mt_opts
};

void _init(void)
{	
	register_match6(&iprange);
}
