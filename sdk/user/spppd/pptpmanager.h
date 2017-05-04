/*
 * pptpmanager.h
 *
 * Manager function prototype.
 *
 * $Id: pptpmanager.h,v 1.1 2012/08/27 07:44:19 ql Exp $
 */

#ifndef _PPTPD_PPTPSERVER_H
#define _PPTPD_PPTPSERVER_H

#include <sys/stat.h>
#include <linux/if.h>
#include <sys/ioctl.h>
//#include <linux/if_pppox.h>
#include "timeout.h"
#include "vpn.h"


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


typedef struct pptpd_s {
	int fd;				/* PPTP Data Connection */
	int ctrl_sock;		/* PPTP Mesg Connection */

	struct	callout ch; 				/* pptp timeout handler */
	int	timeout;
	int debug;

	/******* IO buffers ******/
	char * read_buffer;//, *write_buffer;
	size_t read_alloc;//,   write_alloc;
	size_t read_size;//,    write_size;

	/******* CALL INFO *******/
	unsigned int call_id_pair;	//lower 16 bit is local callid(PAC),  higher 16bits is peer callid(PNS)
	struct in_addr inetaddrs[2];//idx 0 for local, idx 1 for peer.
	unsigned int echo_wait;
	unsigned int echo_count;

	int account_idx;

	void *sppp;
} PPTPD_DRV_CTRL;

extern int pptp_manager(void);

#endif	/* !_PPTPD_PPTPSERVER_H */
