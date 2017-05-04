/* options.h */
#ifndef _OPTIONS_H
#define _OPTIONS_H

#include "packet.h"

#define TYPE_MASK	0x0F

//#define DHCP_OPTION_43_ENABLE // Enable DHCP option 43 - Vendor Specific Information.
//#define DHCP_OPTION_82_ENABLE // Enable DHCP option 82 - Relay Agent Information.

#ifdef DHCP_OPTION_82_ENABLE
#define OPT82_CIRCUIT_ID "RTK_ADSL"
#define OPT82_REMOTE_ID "035780211"

int OPT82_AddSubOpt(unsigned char *, int, const unsigned char *, int);
#endif

enum {
	OPTION_IP=1,
	OPTION_IP_PAIR,
	OPTION_STRING,
	OPTION_BOOLEAN,
	OPTION_U8,
	OPTION_U16,
	OPTION_S16,
	OPTION_U32,
	OPTION_S32,
	OPTION_120
#ifdef _CWMP_TR111_
	,OPTION_125
#endif
};

#define OPTION_REQ	0x10 /* have the client request this option */
#define OPTION_LIST	0x20 /* There can be a list of 1 or more of these */

struct dhcp_option {
	char name[10];
	char flags;
	unsigned char code;
};

extern struct dhcp_option options[];
extern int option_lengths[];

unsigned char *get_option(struct dhcpMessage *packet, int code);
int end_option(unsigned char *optionptr);
int add_option_string(unsigned char *optionptr, unsigned char *string);
int add_simple_option(unsigned char *optionptr, unsigned char code, u_int32_t data);
struct option_set *find_option(struct option_set *opt_list, char code);
void attach_option(struct option_set **opt_list, struct dhcp_option *option, char *buffer, int length);

#ifdef CONFIG_USER_RTK_VOIP
#define SIP_PROXY_NUM 2
void restart_voip_callmanager();
void voip_SetDhcpOption120( char* outbound_addr);
void voip_SetDhcpOption15( char* sip_domain);
#endif

#endif
