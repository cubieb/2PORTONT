/*
 * Shared library for ip6tables for DSCP matching
 */

#include <stdio.h>
#include <string.h>
#include <ip6tables.h>
#include <getopt.h>
#include <ip6tables.h> 
#include <linux/netfilter_ipv6/ip6_tables.h> 
#include <linux/netfilter/xt_dscp.h> 

#include "libipt_dscp_helper.c"

static void 
help(void)
{
	printf(
"DSCP match v%s options:\n"
"[!] --dscp value		Match DSCP codepoint with numerical value\n"
"				This value can be in decimal (ex: 32)\n"
"				or in hex (ex: 0x20)\n"
"[!] --dscp-class name		Match the DiffServ class. This value may\n"
"				be any of the BE,EF, AFxx or CSxx classes.\n"
"\n"
"				These two options are mutually exclusive !\n"
				, IPTABLES_VERSION
);
}


static struct option opts[] = {
	{ "dscp", 1, 0, 'F' },
	{ "dscp-class", 1, 0, 'G' },
	{ 0 }
};


static void
init(struct ip6t_entry_match *m, unsigned int *nfcache)
{
	/* Not sure this is correct */
	*nfcache |= NFC_IP6_TOS;
}

static void 
parse_dscp(const unsigned char *s, struct xt_dscp_info *dinfo)
{
	unsigned int dscp;

	if(string_to_number(s, 0, 255, &dscp) == -1)
		exit_error(PARAMETER_PROBLEM,
				"Invalid DSCP `%s'\n", s);

	if(dscp > XT_DSCP_MAX)
		exit_error(PARAMETER_PROBLEM,
				"DSCP `%d' out of range\n", dscp);

	dinfo->dscp = (u_int8_t)dscp;
	return;
}

static void 
parse_class(const char *s, struct xt_dscp_info *dinfo)
{
	unsigned int dscp = class_to_dscp(s);

	dinfo->dscp = (u_int8_t)dscp;
}


static int
parse(int c, char **argv, int invert, unsigned int *flags,
	const struct ip6t_entry *entry,
	unsigned int *nfcache,
	struct ip6t_entry_match **match)
{
	struct xt_dscp_info *dinfo = 
		(struct xt_dscp_info*)(*match)->data;

	switch(c) {
	case 'F':
		if(*flags)
			exit_error(PARAMETER_PROBLEM,
				"DSCP match: Only use --dscp ONCE!");

		check_inverse(optarg, &invert, &optind, 0);
		parse_dscp(argv[optind - 1], dinfo);
		if(invert)
			dinfo->invert = 1;
		*flags = 1;
		break;

	case 'G':
		if(*flags)
			exit_error(PARAMETER_PROBLEM,
				"DSCP match: Only use --class ONCE!");

		check_inverse(optarg, &invert, &optind, 0);
		parse_class(argv[optind - 1], dinfo);
		if(invert)
			dinfo->invert = 1;
		*flags = 1;
		break;
	
	default:
		return 0;

	}


	return 1;
}


static void
final_check(unsigned int flags)
{
	if(!flags)
		exit_error(PARAMETER_PROBLEM,
			"DSCP match: You must specify either --dscp or --class");
}


static void
print_dscp(u_int8_t dscp, int invert, int numeric)
{
	if(invert)
		fputc('!', stdout);

	printf("0x%02x ", dscp);
}



static void
print(const struct ip6t_ip6 *ip,
	const struct ip6t_entry_match *match,
	int numeric)
{
	const struct xt_dscp_info *dinfo = 
		(const struct xt_dscp_info*)match->data;
	printf("DSCP match ");
	print_dscp(dinfo->dscp, dinfo->invert, numeric);
}


static void
save(const struct ip6t_ip6  *ip, const struct ip6t_entry_match *match)
{
	const struct xt_dscp_info *dinfo = 
		(const struct xt_dscp_info*)match->data;

	printf("--dscp ");
	print_dscp(dinfo->dscp, dinfo->invert, 1);
}


static struct 
ip6tables_match dscp
= { NULL,
   "dscp",
   IPTABLES_VERSION,
   IP6T_ALIGN(sizeof(struct xt_dscp_info)),
   IP6T_ALIGN(sizeof(struct xt_dscp_info)),
   &help,
   &init,
   &parse,
   &final_check,
   &print,
   &save,
   opts
};

void _init(void)
{
	register_match6(&dscp);
}
