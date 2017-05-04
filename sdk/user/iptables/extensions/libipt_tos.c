/* Shared library add-on to iptables to add tos match support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_tos.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"tos match options:\n"
"[!] --tos value[/mask]    Match Type of Service with optional mask\n"
"\n");
}

static struct option opts[] = {
	{ "tos", 1, 0, '1'},
	{ 0 }
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	/* Can't cache this. */
	*nfcache |= NFC_UNKNOWN;
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_tos_info *tosinfo = (struct ipt_tos_info *)(*match)->data;

	switch (c) {
		char *end;
	case '1':
		check_inverse(optarg, &invert, &optind, 0);
		tosinfo->tos = (u_int8_t)strtoul(optarg, &end, 0);
		if (*end == '/') {
			tosinfo->mask = (u_int8_t)strtoul(end+1, &end, 0);
		} else
			tosinfo->mask = 0xff; // default mask 0xff
		if (*end != '\0' || end == optarg)
			exit_error(PARAMETER_PROBLEM, "Bad tos value `%s'", optarg);
		if (invert)
			tosinfo->invert = 1;
		*flags = 1;
		break;

	default:
		return 0;
	}
	return 1;
}

static void
print_tos(unsigned long tos, unsigned long mask, int invert, int numeric)
{
	if (invert)
		fputc('!', stdout);

	if(mask != 0xffffffff)
		printf("0x%lx/0x%lx ", tos, mask);
	else
		printf("0x%lx ", tos);
}

/* Final check; must have specified --tos. */
static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "tos match: You must specify `--tos'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	printf("tos match ");
	print_tos(((struct ipt_tos_info *)match->data)->tos,
		  ((struct ipt_tos_info *)match->data)->mask,
		  ((struct ipt_tos_info *)match->data)->invert, numeric);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	printf("--tos ");
	print_tos(((struct ipt_tos_info *)match->data)->tos,
		  ((struct ipt_tos_info *)match->data)->mask,
		  ((struct ipt_tos_info *)match->data)->invert, 0);
}

static
struct iptables_match tos
= { NULL,
    "tos",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_tos_info)),
    IPT_ALIGN(sizeof(struct ipt_tos_info)),
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
	register_match(&tos);
}
