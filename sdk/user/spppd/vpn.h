#ifndef __VPN_H__
#define __VPN_H__

//#include <sys/ioctl.h>
//#include <linux/if_pppox.h>
#include "../../config/autoconf.h"
#include "timeout.h"

#define MAX_DOMAIN_LENGTH		120
#define MAX_MODEM_DEV_NAME		32
/*copy from user/boa/src-project/LINUX/mib.h*/
#define MAX_NAME_LEN					30

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
#define MAX_VPN_SERVER_ACCOUNT		20
#define AUTHNAMELEN	64
//#define AUTHKEYLEN	16
#define AUTHKEYLEN	64

struct VPN_ACCOUNT_ST {
	char name[MAX_NAME_LEN];
	char auth_name[AUTHNAMELEN];
	char auth_secret[AUTHKEYLEN];
	unsigned char type;// PPTP/L2TP
	unsigned char valid;
};

struct VPN_SERVER_AUTH_St {
	unsigned short auth_proto;
	unsigned char chal_type;
	unsigned char enc_type;
	unsigned int  localip;
	unsigned int  peerip;
#ifdef CONFIG_USER_L2TPD_LNS
	int tunnel_auth; /*0: none  1: challenge*/
	char tunnel_secret[MAX_NAME_LEN];
#endif	
	/* support up to 32 clients at the same time, whose IP is from peerip to peerip+31 */
	unsigned int  assignedIpMap;
};
#endif

struct vpn_param_s {
	int index;	//pptp interface mib index
	int dev_fd;	//ppp8-ppp16
	int state;
	char server[MAX_DOMAIN_LENGTH]; /* Server Address String */
	char devname[MAX_MODEM_DEV_NAME]; /* Device name, pptp0... */
	int ipType;	//0: ipv4  1:ipv6

	/* below for l2tp */
#ifdef CONFIG_USER_L2TPD_L2TPD
	char tunnel_nam[MAX_NAME_LEN];
	char tunnel_profile_nam[MAX_NAME_LEN];
	char session_nam[MAX_NAME_LEN];
	char session_profile_nam[MAX_NAME_LEN];
	union {
		struct in_addr in;	/* Server Address */
#ifdef CONFIG_IPV6_VPN
		struct in6_addr in6;
#endif
	} inetaddr;
	
	int no_ppp;
	int tunnel_auth; /*0: none  1: challenge*/
	char secret[MAX_NAME_LEN];
#endif
};


#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2, &handle)
#define MTIMEOUT(fun, arg1, arg2, handle)	mtimeout(fun, arg1, arg2, &handle)
#define UNTIMEOUT(fun, arg, handle)			untimeout(&handle)

#endif
