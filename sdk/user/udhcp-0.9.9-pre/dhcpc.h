/* dhcpc.h */
#ifndef _DHCPC_H
#define _DHCPC_H

#include "libbb_udhcp.h"

// Kaohj --- support option 125 Vendor-Identifying Vendor-Specific
//#define	DHCP_OPTION_125

#ifdef _WT_146_
/*RFC 3203 DHCP reconfigure extension*/
#define SUPPORT_DHCPFORCERENEW
/*Forcerenew Nonce Authentication*/
#define SUPPORT_FORCERENEW_NONCE_AUTH
#endif //_WT_146_

#define INIT_SELECTING	0
#define REQUESTING	1
#define BOUND		2
#define RENEWING	3
#define REBINDING	4
#define INIT_REBOOT	5
#define RENEW_REQUESTED 6
#define RELEASED	7

#ifdef DHCP_OPTION_125
// Kaohj --- VI Vendor-Specific detail
struct vendor_info_t {
	unsigned int ent_num;
	unsigned char data_len;
	char option_data;	/* list of option-data */
};
#endif

struct client_config_t {
	char foreground;		/* Do not fork */
	char quit_after_lease;		/* Quit after obtaining lease */
	char abort_if_no_lease;		/* Abort if no lease */
	char background_if_no_lease;	/* Fork to background if no lease */
	char microsoft_auto_ip_enable; /* Dick Tam, 2003-05-16, Microsoft AUTO IP procedure */
	char *interface;		/* The name of the interface to use */
	char *pidfile;			/* Optionally store the process ID */
	char *script;			/* User script to run at dhcp events */
	unsigned char *clientid;	/* Optional client id to use */
	unsigned char *hostname;	/* Optional hostname to use */
#ifdef DHCP_OPTION_125
	unsigned char *vi_vendorSpec;	/* Optional VI Vendor-Specific to use */
#endif
	int ifindex;			/* Index number of the interface to use */
	unsigned char arp[6];		/* Our arp address */

#ifdef SUPPORT_FORCERENEW_NONCE_AUTH
	unsigned char auth_enable;
	//general arguments for dhcp auth.
	unsigned char auth_protocol;
	unsigned char auth_algorithm;
	unsigned char auth_rdm;
	unsigned char auth_replaydetect[8];
	//arguments for forcerenew nonce auth.
	unsigned char auth_nonce[16];
	unsigned char auth_isserversupport;
#endif //SUPPORT_FORCERENEW_NONCE_AUTH
};

extern struct client_config_t client_config;
/*ql:20080926 START: initial MIB_DHCP_CLIENT_OPTION_TBL*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
extern unsigned char wan_ifIndex;
#endif
/*ql:20080926 END*/

#endif
