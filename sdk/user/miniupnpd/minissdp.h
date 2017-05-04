/* $Id: minissdp.h,v 1.3 2012/08/01 04:27:13 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2007 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */
#ifndef __MINISSDP_H__
#define __MINISSDP_H__

/*#include "miniupnpdtypes.h"*/

int
OpenAndConfSSDPReceiveSocket();
/* OpenAndConfSSDPReceiveSocket(int n_lan_addr, struct lan_addr_s * lan_addr);*/

/*int
OpenAndConfSSDPNotifySocket(const char * addr);*/

int
OpenAndConfSSDPNotifySockets(int * sockets);
/*OpenAndConfSSDPNotifySockets(int * sockets,
                             struct lan_addr_s * lan_addr, int n_lan_addr);*/

/*void
SendSSDPNotifies(int s, const char * host, unsigned short port,
                 unsigned int lifetime);*/
void
SendSSDPNotifies2(int * sockets,
                  unsigned short port,
                  unsigned int lifetime);
/*SendSSDPNotifies2(int * sockets, struct lan_addr_s * lan_addr, int n_lan_addr,
                  unsigned short port,
                  unsigned int lifetime);*/

void
ProcessSSDPRequest(int s, unsigned short port);
/*ProcessSSDPRequest(int s, struct lan_addr_s * lan_addr, int n_lan_addr,
                   unsigned short port);*/

int
SendSSDPGoodbye(int * sockets, int n);

#endif

extern char **known_service_types;
extern char *known_service_types_igd[];
#ifdef CONFIG_TR_064
extern char *known_service_types_tr064[];
#endif

