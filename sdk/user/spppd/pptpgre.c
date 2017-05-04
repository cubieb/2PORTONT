/*
 * pptpgre.c
 *
 * originally by C. S. Ananian
 * Modified for PoPToP
 *
 * $Id: pptpgre.c,v 1.1 2012/08/27 07:44:58 ql Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __linux__
#define _GNU_SOURCE 1		/* broken arpa/inet.h */
#endif

#include <syslog.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if.h>		/* ifreq struct         */
#include <net/if_arp.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#include <linux/if_pppox.h>

#include "pptpgre.h"
#include "pptpdefs.h"
#include "pptpmanager.h"
#include "if_sppp.h"



int pptp_gre_init(struct sppp *sp)
{
	PPTPD_DRV_CTRL *p = sp->pp_lowerp;
	u_int32_t call_id_pair = p->call_id_pair;
	struct in_addr *inetaddrs = p->inetaddrs;
	struct sockaddr_pppox local, peer;
	int retval;

	printf("%s enter.\n", __func__);
	
	local.sa_family = AF_PPPOX;
	local.sa_protocol = PX_PROTO_PPTP;
	local.sa_addr.pptp.call_id = p->call_id_pair&0xFFFF;
	#ifndef CONFIG_IPV6_VPN
	local.sa_addr.pptp.sin_addr = inetaddrs[0];
	#else
	local.sa_addr.pptp.ipType = 0;
	local.sa_addr.pptp.sin_addr.in = inetaddrs[0];
	#endif

	retval = bind(p->fd, (struct sockaddr*)&local, sizeof(struct sockaddr_pppox));
	if (retval < 0) {
		printf("Failed to bind PPTP socket: %d", retval);
		return retval;
	}

	memcpy(&peer, &local, sizeof(struct sockaddr_pppox));
	peer.sa_addr.pptp.call_id = ((p->call_id_pair&0xFFFF0000)>>16);
	#ifndef CONFIG_IPV6_VPN
	peer.sa_addr.pptp.sin_addr = inetaddrs[1];
	#else
	peer.sa_addr.pptp.ipType = 0;
	peer.sa_addr.pptp.sin_addr.in = inetaddrs[0];
	#endif
	
	retval = connect(p->fd, (struct sockaddr*)&peer, sizeof(struct sockaddr_pppox));
	if( retval < 0 ){
		printf("Failed to connect PPTP socket: %d",retval);
		return retval;
	}
	
	return p->fd;
}

