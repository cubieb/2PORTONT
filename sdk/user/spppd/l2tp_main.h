#ifndef __L2TP_MAIN_H__
#define __L2TP_MAIN_H__

#include "atm.h"
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <sys/stat.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include "timeout.h"
#include "vpn.h"

#if 0
#if 1
#define L2TP_DEBUG(...) 	do{printf( "L2TP:%s:%d> ", __FUNCTION__,__LINE__ );printf(__VA_ARGS__);printf("\n");}while(0)
#else
#define L2TP_DEBUG(...)
#endif
#endif

enum L2TP_STATE{
	L2TP_ST_DEAD=0,
	L2TP_ST_TUNNEL_INIT,
	L2TP_ST_TUNNEL_ESTABLISH,
	L2TP_ST_SESSION_INIT,
	L2TP_ST_SESSION_ESTABLISH,

	L2TP_ST_NUM /*last*/
};

typedef struct l2tpd_s {
	unsigned short tunnel_id;
	unsigned short session_id;
	int account_idx;
	int fd;	//l2tp fd
	int in_ctrl_conn_mode;
	int call_state;
	void *sppp;
}L2TPD_DRV_CTRL;


typedef struct l2tp_s {
	char name[IFNAMSIZ];		/*dev name, pptp0... */
	char server[MAX_DOMAIN_LENGTH]; /* Server Address String */
	union {
		struct sockaddr sa;
		struct sockaddr_in sin;
#ifdef CONFIG_IPV6_VPN
		struct sockaddr_in6 sin6;
#endif
	} local;
	union {
		struct sockaddr sa;
		struct sockaddr_in sin;
#ifdef CONFIG_IPV6_VPN
		struct sockaddr_in6 sin6;
#endif
	} remote;
	unsigned long previous_local_addr;	//just used for nat rule.

	unsigned short tunnel_id;
	unsigned short session_id;
	int fd;	//l2tp fd
	int in_ctrl_conn_mode;
	int call_state;
	void *sppp;
	//int up_retry;						// Mason Yu. Set up_retry flag
	//struct	callout out_ctrl_ch; 		// Mason Yu.  l2tp out_ctrl timeout handler
	//int out_ctrl_timeout;				// Mason Yu. Set up_retry time
} L2TP_DRV_CTRL;

#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2, &handle)
#define UNTIMEOUT(fun, arg, handle)			untimeout(&handle)

extern char *l2tp_state[L2TP_ST_NUM];

#endif
