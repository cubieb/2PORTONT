/* ebt_dhcp_opt
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include "../include/ebtables_u.h"
#include <linux/netfilter_bridge/ebt_dhcp_opt.h>

#define DHCP_OPT_INVERSE	'1'
#define DHCP_OPT_60		'2'
#define DHCP_OPT_61		'3'
#define DHCP_OPT_125		'4'

static struct option opts[] =
{
	{ "dhcp-inverse"	, no_argument,       0, DHCP_OPT_INVERSE },
	{ "dhcp-opt60"	, required_argument, 0, DHCP_OPT_60 },
	{ "dhcp-opt61"	, required_argument, 0, DHCP_OPT_61 },
	{ "dhcp-opt125"	, required_argument, 0, DHCP_OPT_125},
	{ 0 }
};

static void parse_opt61(const char *arg, struct ebt_dhcp_opt_info *info)
{
	int ret;
	char type[4] = {0};
	char others[256] = {0};
	
	ret = sscanf(arg, "%u/%[^/]/%s", &info->opt61_iaid, type, others);

	if(ret < 3)
	{
		ebt_print_error("Invalid option 61 values.");
		return;
	}

	if(strcmp(type, "llt") == 0)
	{
		char mac[32] = {0};
		unsigned char buf[ETH_ALEN] = {0};

		ret = sscanf(others, "%hu/%u/%s", &info->duid_hw_type, &info->duid_time, mac);
		if(ret != 3)
			ebt_print_error("Invalid option 61 values for DUID-LLT.");

		if(ebt_get_mac_and_mask(mac, info->duid_mac, buf) != 0)
			ebt_print_error("Invalid MAC address for DUID-LL.");

		info->opt61_duid_type = 1;
	}
	else if(strcmp(type, "en") == 0)
	{
		ret = sscanf(others, "%u/%s", &info->duid_ent_num, info->duid_ent_id);
		if(ret != 2 || info->duid_ent_id[0] == '\0')
			ebt_print_error("Invalid option 61 values for DUID-EN.");

		info->opt61_duid_type = 2;
	}
	else if(strcmp(type, "ll") == 0)
	{
		char mac[32] = {0};
		unsigned char buf[ETH_ALEN] = {0};

		ret = sscanf(others, "%hu/%s", &info->duid_hw_type, mac);
		if(ret != 2)
			ebt_print_error("Invalid option 61 values for DUID-LL.");

		if(ebt_get_mac_and_mask(mac, info->duid_mac, buf) != 0)
			ebt_print_error("Invalid MAC address for DUID-LL.");

		info->opt61_duid_type = 3;
	}
	else
		ebt_print_error("Invalid DUID type, valid value is one of llt, en and ll.");

}

static void parse_opt125(const char *arg, struct ebt_dhcp_opt_info *info)
{
	int ret;
	char values[256] = {0};
	char *start = NULL, *end = NULL;
	int cnt = 0;

/*
	ret = sscanf(arg, "%u/%[^/]/%[^/]/%[^/]/%[^/]", &info->opt125_ent_num, 
		info->opt125_manufacturer, info->opt125_product_class,
		info->opt125_model, info->opt125_serial);
*/

	ret = sscanf(arg, "%u/%s", &info->opt125_ent_num, values);
	if(ret != 2)
		ebt_print_error("Invalid option 125 values.");

    start = &values[0];

	while(1)
	{
   
		end = strchr(start, '/');
		if(end)
		{
			switch(cnt)
			{
			case 0:
				strncpy(info->opt125_manufacturer, start, end - start);
				break;
			case 1:
				strncpy(info->opt125_product_class, start, end - start);
				break;
			case 2:
				strncpy(info->opt125_model, start, end - start);
				break;
			}
			cnt++;
			start = end + 1;
		}
		else
		{
			if(cnt == 3)	// last one
				strcpy(info->opt125_serial, start);
			else
				ebt_print_error("Parse DHCP option 125 sub-options failed.");

			break;
		}
    }

	if(info->opt125_manufacturer[0] == '\0' && info->opt125_product_class[0] == '\0'
		&& info->opt125_model[0] == '\0' && info->opt125_serial[0] == '\0')
		ebt_print_error("No any sub-option is specified.");
}

static void print_help()
{
	printf(
"DHCP options:\n"
"--dhcp-inverse  invserse match\n"
"--dhcp-opt60    identifier\n"
"--dhcp-opt61    iaid/llt/hardware_type/time/mac_addr\n"
"--dhcp-opt61    iaid/en/ent_num/identifier\n"
"--dhcp-opt61	    iaid/ll/hardware_type/mac_addr\n"
"--dhcp-opt125   ent_num/macnufacturer/product_class/model_name/serial_num\n");
}

static void init(struct ebt_entry_match *match)
{
	struct ebt_dhcp_opt_info *info = (struct ebt_dhcp_opt_info *)match->data;

	memset(info, 0, sizeof(struct ebt_dhcp_opt_info));
}

#define OPT_INVERSE	0x01
#define OPT_60		0x02
#define OPT_61		0x04
#define OPT_125		0x08
static int parse(int c, char **argv, int argc, const struct ebt_u_entry *entry,
   unsigned int *flags, struct ebt_entry_match **match)
{
	struct ebt_dhcp_opt_info *info = (struct ebt_dhcp_opt_info *)(*match)->data;
	//printf("parsing 0x%x: %s\n", c, optarg);

	switch (c) {
	case DHCP_OPT_INVERSE:
		info->opt_inverse = 1;
		break;
	case DHCP_OPT_60:
		ebt_check_option2(flags, OPT_60);
		info->opt60_enable = 1;
		strcpy(info->opt60_value, optarg);
		if(info->opt60_value[0] == '\0')
			ebt_print_error("Must give DHCP option 60 value to match");
		break;

	case DHCP_OPT_61:
		ebt_check_option2(flags, OPT_61);
		info->opt61_enable = 1;

		parse_opt61(optarg, info);
		break;

	case DHCP_OPT_125:
		ebt_check_option2(flags, OPT_125);
		info->opt125_enable = 1;

		parse_opt125(optarg, info);
		break;
	default:
		fprintf(stderr, "Unknow option %x\n", c);
		return 0;
	}
	return 1;
}

static void final_check(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match, const char *name,
   unsigned int hookmask, unsigned int time)
{
	struct ebt_dhcp_opt_info *info = (struct ebt_dhcp_opt_info *)match->data;

	if(!info->opt60_enable && !info->opt61_enable && !info->opt125_enable)
		ebt_print_error("Must enable one of DHCP option");	
}

static void print(const struct ebt_u_entry *entry,
   const struct ebt_entry_match *match)
{
	struct ebt_dhcp_opt_info *info = (struct ebt_dhcp_opt_info *)match->data;

	if(info->opt_inverse)
		printf("--dhcp-inverse ");

	if(info->opt60_enable)
	{
		printf("--dhcp-opt60 %s ", info->opt60_value);
	}

	if(info->opt61_enable)
	{
		printf("--dhcp-opt61 %u/", info->opt61_iaid);

		switch(info->opt61_duid_type)
		{
		case 1:
			ebt_printstyle_mac = 2;
			printf("llt/%u/%u/", info->duid_hw_type, info->duid_time);
			ebt_print_mac(info->duid_mac);
			printf(" ");
			break;
		case 2:
			printf("en/%u/%s ", info->duid_ent_num, info->duid_ent_id);
			break;
		case 3:
			ebt_printstyle_mac = 2;
			printf("ll/%u/", info->duid_hw_type);
			ebt_print_mac(info->duid_mac);
			printf(" ");
			break;
		}
	}

	if(info->opt125_enable)
	{
		printf("--dhcp-opt125 %u/%s/%s/%s/%s ", info->opt125_ent_num
			, info->opt125_manufacturer, info->opt125_product_class
			, info->opt125_model, info->opt125_serial);
	}
}

static int compare(const struct ebt_entry_match *m1,
   const struct ebt_entry_match *m2)
{
	struct ebt_dhcp_opt_info *info1 = (struct ebt_dhcp_opt_info *)m1->data;
	struct ebt_dhcp_opt_info *info2 = (struct ebt_dhcp_opt_info *)m2->data;


	if(info1->opt_inverse!= info2->opt_inverse)
		return 0;

	if(info1->opt60_enable != info2->opt60_enable)
		return 0;

	if(info1->opt60_enable
		&& strcmp(info1->opt60_value, info2->opt60_value) != 0)
		return 0;

	if(info1->opt61_enable != info2->opt61_enable)
		return 0;

	if(info1->opt61_enable)
	{
		if(info1->opt61_iaid != info2->opt61_iaid)
			return 0;
		if(info1->opt61_duid_type != info1->opt61_duid_type)
			return 0;

		switch(info1->opt61_duid_type)
		{
		case 1:
			if(info1->duid_time != info2->duid_time)
				return 0;
		case 3:
			if(info1->duid_hw_type != info2->duid_hw_type)
				return 0;
			if(memcmp(info1->duid_mac, info2->duid_mac, ETH_ALEN) != 0)
				return 0;
			break;
		case 2:
			if(info1->duid_ent_num != info2->duid_ent_num)
				return 0;
			if(strcmp(info1->duid_ent_id, info2->duid_ent_id) != 0)
				return 0;
			break;
		}
	}

	if(info1->opt125_enable != info2->opt125_enable)
		return 0;

	if(info1->opt125_enable)
	{
		if(info1->opt125_ent_num != info1->opt125_ent_num)
			return 0;

		if(strcmp(info1->opt125_manufacturer, info2->opt125_manufacturer) != 0
			|| strcmp(info1->opt125_product_class, info2->opt125_product_class) != 0
			|| strcmp(info1->opt125_model, info2->opt125_model) != 0
			|| strcmp(info1->opt125_serial, info2->opt125_serial) != 0)
			return 0;
	}
		
	return 1;
}

static struct ebt_u_match dhcp_opt_match =
{
	.name		= EBT_DHCP_OPT_MATCH,
	.size		= sizeof(struct ebt_dhcp_opt_info),
	.help		= print_help,
	.init		= init,
	.parse		= parse,
	.final_check	= final_check,
	.print		= print,
	.compare	= compare,
	.extra_ops	= opts,
};

void _init(void)
{
	ebt_register_match(&dhcp_opt_match);
}

