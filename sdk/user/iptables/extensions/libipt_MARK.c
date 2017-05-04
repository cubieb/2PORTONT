/* Shared library add-on to iptables to add MARK target support. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_MARK.h>

struct markinfo {
	struct ipt_entry_target t;
	struct ipt_mark_target_info mark;
};

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"MARK target v%s options:\n"
"  --set-mark value[/mask]	Set nfmark value with optional mask\n"
"\n",
IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "set-mark", 1, 0, '1' },
	{ 0 }
};

/* Initialize the target. */
static void
init(struct ipt_entry_target *t, unsigned int *nfcache)
{
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      struct ipt_entry_target **target)
{
	struct ipt_mark_target_info *markinfo
		= (struct ipt_mark_target_info *)(*target)->data;

	switch (c) {
		// Kaohj -- add optional mask
		char *end;
	case '1':
		// Kaohj -- add optional mask
		#if 0
		if (string_to_number(optarg, 0, 0xffffffff, 
				     (unsigned int *)&markinfo->mark))
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		#endif
		markinfo->mark = strtoul(optarg, &end, 0);
		if (*end == '/') {
			markinfo->mask = strtoul(end+1, &end, 0);
		} else
			markinfo->mask = 0xffffffff; // default mask 0xffffffff
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad MARK value `%s'", optarg);
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
			           "MARK target: Can't specify --set-mark twice");
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
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
		           "MARK target: Parameter --set-mark is required");
}

// Kaohj -- add optional mask
#if 0
static void
print_mark(unsigned long mark, int numeric)
{
	printf("0x%lx ", mark);
}
#endif
static void
print_mark(unsigned long mark, unsigned long mask, int numeric)
{
	if (mask != 0xffffffff)
		printf("0x%lx/0x%lx ", mark, mask);
	else
		printf("0x%lx ", mark);
}

/* Prints out the targinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_target *target,
      int numeric)
{
	const struct ipt_mark_target_info *markinfo =
		(const struct ipt_mark_target_info *)target->data;
	printf("MARK set ");
	// Kaohj -- add optional mask
	//print_mark(markinfo->mark, numeric);
	print_mark(markinfo->mark, markinfo->mask, numeric);
}

/* Saves the union ipt_targinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_target *target)
{
	const struct ipt_mark_target_info *markinfo =
		(const struct ipt_mark_target_info *)target->data;
	
	// Kaohj -- add optional mask
	//printf("--set-mark 0x%lx ", markinfo->mark);
	printf("--set-mark ");
	print_mark(markinfo->mark, markinfo->mask, 0);
}

static
struct iptables_target mark
= { NULL,
    "MARK",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_mark_target_info)),
    IPT_ALIGN(sizeof(struct ipt_mark_target_info)),
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
	register_target(&mark);
}
