#ifdef _LINUX_2_6_ 
/* Shared library add-on to iptables to add string matching support. 
 * 
 * Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 *
 * ChangeLog
 *     27.01.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 *             Changed --tos to --string in save(). Also
 *             updated to work with slightly modified
 *             ipt_string_info.
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <stddef.h>
#include <linux/netfilter_ipv4/ipt_string.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"STRING match v%s options:\n"
"--from                       Offset to start searching from\n"
"--to                         Offset to stop searching\n"
"--algo                       Algorithm\n"
"--string [!] string          Match a string in a packet\n"
// Added by Mason Yu for URL Blocking
"--url [!] string             Match a url string in a packet\n"
// Added by Mason Yu for Domain Blocking
"--domain [!] string          Match a Domain string in a packet\n",
//Added by alex for URL allowing 
"--urlalw [!]string           Match a url allow string in a packet\n"
IPTABLES_VERSION);

	fputc('\n', stdout);
}

static struct option opts[] = {
	{ "from", 1, 0, '1' },
	{ "to", 1, 0, '2' },
	{ "algo", 1, 0, '3' },
	{ "string", 1, 0, '4' },
	//new added
	{ "url", 1, 0, '5' },
	{ "urlalw", 1, 0, '6' },
	{ "domain", 1, 0, '7' },
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	struct ipt_string_info *i = (struct ipt_string_info *)m->data;

	if (i->to_offset == 0)
		i->to_offset = (u_int16_t) ~0UL;

	*nfcache |= NFC_UNKNOWN;
}

static void
parse_string(const unsigned char *s, struct ipt_string_info *info)
{
#if 0//for linux-2.4
	if (strlen(s) <= BM_MAX_NLEN) strcpy(info->string, s);
	else exit_error(PARAMETER_PROBLEM, "STRING too long `%s'", s);
#else
    int i=0;
	if (strlen(s) <= IPT_STRING_MAX_PATTERN_SIZE) {
		strncpy(info->pattern, s, IPT_STRING_MAX_PATTERN_SIZE);
        //QL 20110629: remove "/r/n" from pattern
        while(1) {
            if ((info->pattern[i]=='\r') || (info->pattern[i]=='\n') || info->pattern[i]=='\0')
                break;
            i++;
        }
        info->pattern[i] = '\0';

		info->patlen = strlen(info->pattern);
		return;
	}
	exit_error(PARAMETER_PROBLEM, "STRING too long `%s'", s);
#endif
}

static void 
parse_algo(const unsigned char *s, struct ipt_string_info *info)
{
	if (strlen(s) <= IPT_STRING_MAX_ALGO_NAME_SIZE) {
		strncpy(info->algo, s, IPT_STRING_MAX_ALGO_NAME_SIZE);
		return;
	}
	exit_error(PARAMETER_PROBLEM, "ALGO too long `%s'", s);
}

#define STRING	0x1
#define ALGO	0x2
#define FROM	0x4
#define TO		0x8
#define URL		0x10
#define URLALW	0x20
#define DOMAIN	0x40

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_string_info *stringinfo = (struct ipt_string_info *)(*match)->data;

	switch (c) {
	case '1'://from
		if (*flags)
			if (*flags & FROM)
				exit_error(PARAMETER_PROBLEM,
							"Can't specify multiple --from");
		stringinfo->from_offset = atoi(optarg);
		*flags |= FROM;
		break;
	case '2'://to
		if (*flags & TO)
			exit_error(PARAMETER_PROBLEM,
						"Can't specify multiple --algo");
		stringinfo->to_offset = atoi(optarg);
		*flags |= TO;
		break;
	case '3'://algo
		if (*flags & ALGO)
			exit_error(PARAMETER_PROBLEM,
						"Can't specify multiple --algo");
		parse_algo(optarg, stringinfo);
		*flags |= ALGO;
		break;
	case '4'://string
		if (*flags & STRING)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple --string");
		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->patlen = strlen((char *)&stringinfo->pattern);
		stringinfo->flagStr= IPT_GENERAL_STRING;
		*flags |= STRING;
		break;
		
	 //new added for url blocking/allowing
	 case '5'://url block
		if (*flags & STRING)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple --url");

		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->patlen = strlen((char *)&stringinfo->pattern);
		stringinfo->flagStr= IPT_URL_STRING;
		*flags |= (URL | STRING);
		break;
	case '6'://url allow
		if (*flags & STRING)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple --urlalw");

		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->patlen = strlen((char *)&stringinfo->pattern);
		stringinfo->flagStr= IPT_URL_ALW_STRING;
		*flags |= (URLALW | STRING);
		break;
	// Added by Mason Yu for Domain Blocking
	 case '7':
		if (*flags & STRING)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple strings");

		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->patlen=strlen((char *)&stringinfo->pattern);
		stringinfo->flagStr=IPT_DOMAIN_STRING;
		*flags |= (DOMAIN | STRING);
		break;
	

	default:
		printf("invalid option\n");
		return 0;
	}
	return 1;
}

static void
print_string(char string[], int invert, int numeric)
{

	if (invert)
		fputc('!', stdout);
	printf("%s ",string);
}

/* Final check; must have specified --string. */
static void
final_check(unsigned int flags)
{
	if (!flags)
	if (!(flags & STRING))
		exit_error(PARAMETER_PROBLEM,
				"STRING match: You must specify `--string' or "
				"`--url' or `--urlalw'");
	if (!(flags & URL) && !(flags & URLALW) && !(flags & ALGO) && !(flags && DOMAIN))
		exit_error(PARAMETER_PROBLEM,
				"STRING match: You must specify `--algo'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	printf("STRING match ");
	print_string(((struct ipt_string_info *)match->data)->pattern,
		  ((struct ipt_string_info *)match->data)->invert, numeric);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	printf("--string ");
	print_string(((struct ipt_string_info *)match->data)->pattern,
		  ((struct ipt_string_info *)match->data)->invert, 0);
}

static struct iptables_match string = {
#if 1
	NULL,
    "string",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_string_info)),
    IPT_ALIGN(sizeof(struct ipt_string_info)),
    &help,
    &init,
    &parse,
    &final_check,
    &print,
    &save,
    opts
#else
	.name			= "string",
	.version		= IPTABLES_VERSION,
	.size			= IPT_ALIGN(sizeof(struct ipt_string_info)),
	.userspacesize	= offsetof(struct ipt_string_info, config),  
	.help			= help,
	.init			= init,
	.parse			= parse,
	.final_check	= final_check,
	.print			= print,
	.save			= save,
	.extra_opts		= opts
#endif
};

void _init(void)
{	
	register_match(&string);
}
#else// _LINUX_2_4_
/* Shared library add-on to iptables to add string matching support. 
 * 
 * Copyright (C) 2000 Emmanuel Roger  <winfield@freegates.be>
 *
 * ChangeLog
 *     27.01.2001: Gianni Tedesco <gianni@ecsc.co.uk>
 *             Changed --tos to --string in save(). Also
 *             updated to work with slightly modified
 *             ipt_string_info.
 */
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include <iptables.h>
#include <linux/netfilter_ipv4/ipt_string.h>

/* Function which prints out usage message. */
static void
help(void)
{
	printf(
"STRING match v%s options:\n"
"--string [!] string             Match a string in a packet\n"
// Added by Mason Yu for URL Blocking
"--url [!] string              	Match a url string in a packet\n"
// Added by Mason Yu for Domain Blocking
"--domain [!] string              	Match a Domain string in a packet\n",
//Added by alex for URL allowing 
"--urlalw [!]string            Match a url allow string in a packet\n"
IPTABLES_VERSION);

	fputc('\n', stdout);
}

static struct option opts[] = {
	{ "string", 1, 0, '1' },
	// Added by Mason Yu for URL Blocking
	{ "url", 1, 0, '2' },
	// Added by Mason Yu for domain Blocking
	{ "domain", 1, 0, '3' },
	//Added by alex huang for URL allowing 
	{ "urlalw",1,0,'4'},
	{0}
};

/* Initialize the match. */
static void
init(struct ipt_entry_match *m, unsigned int *nfcache)
{
	*nfcache |= NFC_UNKNOWN;
}

static void
parse_string(const unsigned char *s, struct ipt_string_info *info)
{	
        if (strlen(s) <= BM_MAX_NLEN) strcpy(info->string, s);
	else exit_error(PARAMETER_PROBLEM, "STRING too long `%s'", s);
}

/* Function which parses command options; returns true if it
   ate an option */
static int
parse(int c, char **argv, int invert, unsigned int *flags,
      const struct ipt_entry *entry,
      unsigned int *nfcache,
      struct ipt_entry_match **match)
{
	struct ipt_string_info *stringinfo = (struct ipt_string_info *)(*match)->data;

	switch (c) {
	case '1':
		 // Added by Mason Yu for URL Blocking
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple strings");
				   
		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
                stringinfo->len=strlen((char *)&stringinfo->string);
                stringinfo->flagStr=IPT_GENERAL_STRING;
		*flags = 1;
		break;
	
	 // Added by Mason Yu for URL Blocking
	 case '2':
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple strings");

		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->len=strlen((char *)&stringinfo->string);
		stringinfo->flagStr=IPT_URL_STRING;
		*flags = 1;
		break;
		
	// Added by Mason Yu for Domain Blocking
	 case '3':
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple strings");

		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->len=strlen((char *)&stringinfo->string);
		stringinfo->flagStr=IPT_DOMAIN_STRING;
		*flags = 1;
		break;
	case '4':
		if (*flags)
			exit_error(PARAMETER_PROBLEM,
				   "Can't specify multiple strings");

		check_inverse(optarg, &invert, &optind, 0);
		parse_string(argv[optind-1], stringinfo);
		if (invert)
			stringinfo->invert = 1;
		stringinfo->len=strlen((char *)&stringinfo->string);
		stringinfo->flagStr=IPT_URL_ALW_STRING;
		*flags = 1;
		break;
		

	default:
		return 0;
	}
	return 1;
}

static void
print_string(char string[], int invert, int numeric)
{

	if (invert)
		fputc('!', stdout);
	printf("%s ",string);
}

/* Final check; must have specified --string. */
static void
final_check(unsigned int flags)
{
	if (!flags)
		exit_error(PARAMETER_PROBLEM,
			   "STRING match: You must specify `--string'");
}

/* Prints out the matchinfo. */
static void
print(const struct ipt_ip *ip,
      const struct ipt_entry_match *match,
      int numeric)
{
	printf("STRING match ");
	print_string(((struct ipt_string_info *)match->data)->string,
		  ((struct ipt_string_info *)match->data)->invert, numeric);
}

/* Saves the union ipt_matchinfo in parsable form to stdout. */
static void
save(const struct ipt_ip *ip, const struct ipt_entry_match *match)
{
	printf("--string ");
	print_string(((struct ipt_string_info *)match->data)->string,
		  ((struct ipt_string_info *)match->data)->invert, 0);
}

static
struct iptables_match string
= { NULL,
    "string",
    IPTABLES_VERSION,
    IPT_ALIGN(sizeof(struct ipt_string_info)),
    IPT_ALIGN(sizeof(struct ipt_string_info)),
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
	register_match(&string);
}
#endif

