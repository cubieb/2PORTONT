/*
 * Shared library for ip6tables DSCP mangling
 *
 * (C) 2002 Iain Barnes <igbarn@yahoo.com>
 *
 * Based on code by Harald Welte.
 *
 * This code is released under the GNU GPL
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <ip6tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h> 
#include <linux/netfilter/xt_DSCP.h> 


#include "libipt_dscp_helper.c"


static void 
help(void)
{
	printf(
"DSCP v%s options:\n"
"  --set-dscp value		Set the DSCP field in the packet header to value.\n"
"				This value can be in decimal (ex: 32)\n"
"				or in hex (ex: 0x20)\n"
" --set-dscp-class class		Set the DSCP field in the packet header to\n"
"				the values represented by the DiffServ class.\n"
"				This class may be BE, EF or any of the CSxx or\n"
"				AFxx classes.\n"
"\n"
"				These two options are mutually exclusive !\n",
				IPTABLES_VERSION);
};


static struct option opts[] = 
{
	{"set-dscp", 1, 0, 'F'},
	{"set-dscp-class", 1, 0, 'G'},
	{ 0 }
};


static void
init(struct ip6t_entry_target *t, unsigned int *nfcache)
{
}


static void 
parse_dscp(const unsigned char *s, struct xt_DSCP_info *dinfo)
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
parse_class(const unsigned char *s, struct xt_DSCP_info *dinfo)
{
	unsigned int dscp = class_to_dscp(s);

	dinfo->dscp = (u_int8_t)dscp;
}


static int 
parse(int c, char **argv, int invert, unsigned int *flags,
	const struct ip6t_entry *entry,
	struct ip6t_entry_target **target)
{
	struct xt_DSCP_info *dinfo = 
		(struct xt_DSCP_info*)(*target)->data;

	switch(c) {
	case 'F':
		if(*flags)
			exit_error(PARAMETER_PROBLEM,
				"DSCP target: Only use --set-dscp ONCE !");
		parse_dscp(optarg, dinfo);
		*flags = 1;
		break;

	case 'G':
		if(*flags)
			exit_error(PARAMETER_PROBLEM,
				"DSCP target: Only use --class ONCE !");

		parse_class(optarg, dinfo);
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
			"DSCP target: Parameter --set-dscp or --set-class required");
}


static void
print_dscp(u_int8_t dscp, int numeric)
{
	printf("0x%02x ", dscp);
}


static void
print(const struct ip6t_ip6 *ip,
	const struct ip6t_entry_target *target,
	int numeric)
{
	const struct xt_DSCP_info *dinfo = 
		(const struct xt_DSCP_info*)target->data;

	printf("DSCP set ");
	print_dscp(dinfo->dscp, numeric);
}


static void
save(const struct ip6t_ip6 *ip, const struct ip6t_entry_target *target)
{
	const struct xt_DSCP_info *dinfo = 
		(const struct xt_DSCP_info*)target->data;

	printf("--set-dscp 0x%02x ", dinfo->dscp);
}


static struct
ip6tables_target dscp
= { NULL,
    "DSCP",
    IPTABLES_VERSION,
    IP6T_ALIGN(sizeof(struct xt_DSCP_info)),
    IP6T_ALIGN(sizeof(struct xt_DSCP_info)),
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
	register_target6(&dscp);
}