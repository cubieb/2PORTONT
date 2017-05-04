#ifndef __LINUX_BRIDGE_EBT_DHCP_H
#define __LINUX_BRIDGE_EBT_DHCP_H

#define OPT_CODE 0
#define OPT_LEN 1
#define OPT_DATA 2
#define OPTION_FIELD		0
#define FILE_FIELD		1
#define SNAME_FIELD		2

/* DHCP Message types */
#define DHCPDISCOVER		1
#define DHCPOFFER		2
#define DHCPREQUEST		3
#define DHCPDECLINE		4
#define DHCPACK			5
#define DHCPNAK			6
#define DHCPRELEASE		7
#define DHCPINFORM		8

/* Used DHCP option codes (partial list) */
#define DHCP_PADDING		0x00
#define DHCP_OPTION_OVER	0x34
#define DHCP_MESSAGE_TYPE	0x35
#define DHCP_MESSAGE		0x38
#define DHCP_VENDOR		0x3c
#define DHCP_CLIENT_ID	0x3d
#define DHCP_VI_VENSPEC	0x7d
#define DHCP_END			0xFF

#define EBT_DHCP_OPT_MATCH "dhcp_opt"

struct ebt_dhcp_opt_info
{
	unsigned char opt_inverse;
	unsigned char opt60_enable;
	char opt60_value[100];
	unsigned char opt61_enable;
	unsigned int opt61_iaid;
	unsigned char opt61_duid_type;
	unsigned short duid_hw_type;
	unsigned char duid_mac[6];
	unsigned int duid_time;
	unsigned int duid_ent_num;
	char duid_ent_id[32];
	unsigned char opt125_enable;
	unsigned int opt125_ent_num;
	char opt125_manufacturer[32];
	char opt125_product_class[32];
	char opt125_model[32];
	char opt125_serial[32];	
};

#endif
