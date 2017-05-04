/* Shared library add-on to iptables to add CLASSIFY target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_CLASSIFY.h>
#include <linux/types.h>
#include <linux/pkt_sched.h>

static void
CLASSIFY_help(void)
{
	printf(
"CLASSIFY target options:\n"
"--set-class MAJOR:MINOR    Set skb->priority value (always hexadecimal!)\n");
}

static struct option CLASSIFY_opts[] = {
	{ "set-class", 1, NULL, '1' },
	{ 0 }
};

/* Initialize the target. */
static void
CLASSIFY_init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

static int CLASSIFY_string_to_priority(const char *s, unsigned int *p)
{
	unsigned int i, j;

	if (sscanf(s, "%x:%x", &i, &j) != 2)
		return 1;
	
	*p = TC_H_MAKE(i<<16, j);
	return 0;
}

static int
CLASSIFY_parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_classify_target_info *clinfo
		= (struct ipt_classify_target_info *)(*target)->data;

	switch (c) {
	case '1':
		if (CLASSIFY_string_to_priority(optarg, &clinfo->priority))
			exit_error(PARAMETER_PROBLEM,
				   "Bad class value `%s'", optarg);
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "CLASSIFY: Can't specify --set-class twice");
		*flags = 1;
		break;

	default:
		return 0;
	}

	return 1;
}

static void
CLASSIFY_final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
		           "CLASSIFY: Parameter --set-class is required");
}

static void
CLASSIFY_print_class(unsigned int priority, int numeric)
{
	printf("%x:%x ", TC_H_MAJ(priority)>>16, TC_H_MIN(priority));
}

static void
CLASSIFY_print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_classify_target_info *clinfo =
		(const struct ipt_classify_target_info *)target->data;
	printf("CLASSIFY set ");
	CLASSIFY_print_class(clinfo->priority, numeric);
}

static void
CLASSIFY_save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	const struct ipt_classify_target_info *clinfo =
		(const struct ipt_classify_target_info *)target->data;

	printf("--set-class %.4x:%.4x ",
	       TC_H_MAJ(clinfo->priority)>>16, TC_H_MIN(clinfo->priority));
}

static struct iptables_target classify_target = { 
	.next		= NULL,
	.name		= "CLASSIFY",
	.version	= IPTABLES_VERSION,
	.size		= IPT_ALIGN(sizeof(struct ipt_classify_target_info)),
	.userspacesize	= IPT_ALIGN(sizeof(struct ipt_classify_target_info)),
	.help		= CLASSIFY_help,
	.init		= CLASSIFY_init,
	.parse		= CLASSIFY_parse,
	.final_check	= CLASSIFY_final_check,
	.print		= CLASSIFY_print,
	.save		= CLASSIFY_save,
	.extra_opts	= CLASSIFY_opts,
};

void _init(void)
{
	register_target(&classify_target);
}
