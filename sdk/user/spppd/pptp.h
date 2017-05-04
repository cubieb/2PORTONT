#ifndef _PPTP_H_
#define _PPTP_H_

#include "atm.h"
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <sys/stat.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <linux/if_pppox.h>
#include "timeout.h"
#include "vpn.h"

#if 0
#define PPTP_DEBUG(...) 	do{printf( "PPtP:%s:%d> ", __FUNCTION__,__LINE__ );printf(__VA_ARGS__);printf("\n");}while(0)
#else
#define PPTP_DEBUG(...)
#endif

enum PPTP_STATE{
	PTP_ST_DEAD=0,
	PTP_ST_CTRL_INIT,
	PTP_ST_CTRL_ESTABLISH,
	PTP_ST_CALL_INIT,
	PTP_ST_CALL_FAIL,
	PTP_ST_CALL_ESTABLISH,

	PTP_ST_NUM /*last*/
};

enum PPTP_EVENT{
	PTP_EVENT_OPEN=0,
	PTP_EVENT_CLOSE,

	PTP_EVENT_NUM /*last*/
};

typedef struct pptp_s {
	char name[IFNAMSIZ];		/*dev name, pptp0... */
	char server[MAX_DOMAIN_LENGTH]; /* Server Address String */
	struct sockaddr_pppox local;
	struct sockaddr_pppox sp;
	unsigned long previous_local_addr;	//just used for nat rule.
	int fd;				/* fd of PPTP socket */
	int ctrl_sock;

	int callctrlState;
	struct	callout ch; 				/* pptp timeout handler */
	int	timeout;
	int debug;
	int in_pptp_ctrl;				/* if in control connection establishing phase */
	struct callout ctrl_ch;
	int ctrl_timeout;

	void *conn;
	void *sppp;
	int up_retry;						// Mason Yu. Set up_retry flag
	struct	callout out_ctrl_ch; 		// Mason Yu.  pptp out_ctrl timeout handler
	int out_ctrl_timeout;				// Mason Yu. Set up_retry time
} PPTP_DRV_CTRL;

extern char *pptp_state[PTP_ST_NUM];
extern fd_set pptp_in_fds;
extern int pptp_max_fd;

int pptp_init(struct sppp *sp);

#endif
