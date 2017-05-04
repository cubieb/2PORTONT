/* Shared library add-on to iptables to add MAC address support. */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#if defined(__GLIBC__) && __GLIBC__ == 2
#include <net/ethernet.h>
#else
#include <linux/if_ether.h>
#endif
#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_mac.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"MAC v%s options:\n"
" --mac-source [!] XX:XX:XX:XX:XX:XX\n"
"				Match source MAC address\n"
" --src-mask XX:XX:XX:XX:XX:XX\n"
"				Source MAC mask\n"
" --mac-dst [!] XX:XX:XX:XX:XX:XX\n"
"				Match destination MAC address\n"
" --dst-mask XX:XX:XX:XX:XX:XX\n"
"				Destination MAC mask\n"
"\n", IPTABLES_VERSION);
}

static struct option opts[] = {
	{ "mac-source", 1, 0, '1' },
	// Kaohj -- add mac mask
	{ "src-mask", 1, 0, '2' },
	// Added by Mason Yu for dst MAC
	{ "mac-dst", 1, 0, '3' },
	// Kaohj -- add mac mask
	{ "dst-mask", 1, 0, '4' },
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	/* Can't cache this */
	*nfcache |= NFC_UNKNOWN;
}

static void
parse_mac(const char *mac, struct ipt_mac_info *info)
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		exit_error(PARAMETER_PROBLEM, "Bad mac address `%s'", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);

		if (end == mac + i*3 + 2
		    && number >= 0
		    && number <= 255)
			info->srcaddr[i] = number;
		else
			exit_error(PARAMETER_PROBLEM,
				   "Bad mac address `%s'", mac);
	}
}


// Added by Mason Yu for dst NAC
static void
parse_dst_mac(const char *mac, struct ipt_mac_info *info)
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		exit_error(PARAMETER_PROBLEM, "Bad mac address `%s'", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);

		if (end == mac + i*3 + 2
		    && number >= 0
		    && number <= 255)
			info->dstaddr[i] = number;
		else
			exit_error(PARAMETER_PROBLEM,
				   "Bad mac address `%s'", mac);
	}
}

static void
parse_mask(const char *mac, struct ipt_mac_info *info, int src)
{
	unsigned int i = 0;

	if (strlen(mac) != ETH_ALEN*3-1)
		exit_error(PARAMETER_PROBLEM, "Bad mac mask `%s'", mac);

	for (i = 0; i < ETH_ALEN; i++) {
		long number;
		char *end;

		number = strtol(mac + i*3, &end, 16);

		if (end == mac + i*3 + 2
		    && number >= 0
		    && number <= 255) {
		    	if (src==0)
				info->srcmask[i] = number;
			else
				info->dstmask[i] = number;
		}
		else
			exit_error(PARAMETER_PROBLEM,
				   "Bad mac mask `%s'", mac);
	}
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_mac_info *macinfo = (struct ipt_mac_info *)(*match)->data;

	switch (c) {
	case '1':
		if (*flags & MAC_SRC)
			exit_error(PARAMETER_PROBLEM,
				   "mac match: Only use --mac-source ONCE!");
		*flags |= MAC_SRC;

		macinfo->flags |= MAC_SRC;
		check_inverse(optarg, &invert, &optind, 0);
		parse_mac(argv[optind-1], macinfo);
		if (invert)
			macinfo->flags |= MAC_SRC_INV;
			//macinfo->invert = 1;
		//*flags = 1;
		break;
	
	case '2': // src mask
		if (*flags & SRC_MASK)
			exit_error(PARAMETER_PROBLEM,
				   "mac match: Only use --src-mask ONCE!");
		*flags |= SRC_MASK;

		macinfo->flags |= SRC_MASK;
		check_inverse(optarg, &invert, &optind, 0);
		parse_mask(argv[optind-1], macinfo, 0);
		if (invert)
			macinfo->flags |= SRC_MASK_INV;
			//macinfo->invert = 1;
		//*flags = 1;
		break;
	
	// Added by Mason Yu for dst MAC
	case '3':
		if (*flags & MAC_DST)
			exit_error(PARAMETER_PROBLEM,
				   "mac match: Only use --mac-dst ONCE!");
		*flags |= MAC_DST;

		macinfo->flags |= MAC_DST;
		check_inverse(optarg, &invert, &optind, 0);
		parse_dst_mac(argv[optind-1], macinfo);
		if (invert)
			macinfo->flags |= MAC_DST_INV;
			//macinfo->invert = 1;
		//*flags = 1;
		break;
	
	case '4': // dst mask
		if (*flags & DST_MASK)
			exit_error(PARAMETER_PROBLEM,
				   "mac match: Only use --dst-mask ONCE!");
		*flags |= DST_MASK;

		macinfo->flags |= DST_MASK;
		check_inverse(optarg, &invert, &optind, 0);
		parse_mask(argv[optind-1], macinfo, 1);
		if (invert)
			macinfo->flags |= DST_MASK_INV;
			//macinfo->invert = 1;
		//*flags = 1;
		break;
	
	default:
		return 0;
	}

	return 1;
}

static void print_mac(unsigned char macaddress[ETH_ALEN], int invert)
{
	unsigned int i;

	printf("%s%02X", invert ? "!" : "", macaddress[0]);
	for (i = 1; i < ETH_ALEN; i++)
		printf(":%02X", macaddress[i]);
	printf(" ");
}

/* Final check; must have specified --mac. */
/* Final check; must have specified --mac-source or --mac-dst. */
static void final_check(unsigned int flags)
{
	int check;
	
	check = flags & (MAC_SRC | MAC_DST);
	//if (!flags)
	if (!check)
		exit_error(PARAMETER_PROBLEM,
			   "You must specify `--mac-source' or 'mac-dst'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	struct ipt_mac_info *info = (struct ipt_mac_info *)match->data;
	
	if (info->flags & MAC_SRC) {
		printf("SRC MAC: ");
		print_mac(((struct ipt_mac_info *)match->data)->srcaddr,
			  ((struct ipt_mac_info *)match->data)->flags & MAC_SRC_INV);
		if (info->flags & SRC_MASK) {
			printf("SRC MASK: ");
			print_mac(((struct ipt_mac_info *)match->data)->srcmask, 0);
		}
	}
	if (info->flags & MAC_DST) {
		printf("DST MAC: ");
		print_mac(((struct ipt_mac_info *)match->data)->dstaddr,
			  ((struct ipt_mac_info *)match->data)->flags & MAC_DST_INV);
		if (info->flags & DST_MASK) {
			printf("DST MASK: ");
			print_mac(((struct ipt_mac_info *)match->data)->dstmask, 0);
		}
	}
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	printf("--mac ");
	print_mac(((struct ipt_mac_info *)match->data)->srcaddr,
		  ((struct ipt_mac_info *)match->data)->flags & MAC_SRC_INV);
}

static
struct iptables_match mac
= { NULL,
    "mac",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_mac_info)),
    IPT_ALIGN(sizeof(struct ipt_mac_info)),
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
	register_match(&mac);
}
