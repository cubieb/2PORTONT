/*
 * Copyright (c) 1983, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * From: @(#)timer.c	5.10 (Berkeley) 2/28/91 
 * From: @(#)timer.c	8.1 (Berkeley) 6/5/93
 */
char timer_rcsid[] = 
  "$Id: timer.c,v 1.19 2012/07/24 07:50:54 masonyu Exp $";


/*
 * Routing Table Management Daemon
 */
#include "defs.h"
// Added by Mason Yu
#include <sys/ioctl.h>
#include <syslog.h>
#include <errno.h>
#include <search.h>
#define FIXLEN(s) { }


extern debug;

static int faketime;


/*
 * Timer routine.  Performs routing information supply
 * duties and manages timers on routing table entries.
 * Management of the RTS_CHANGED bit assumes that we broadcast
 * each time called.
 */

// Added by Mason Yu
// Delete all entry from RIP Hash Table
void rtdeleteall2(int sig)
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;
	int doinghost = 1, i;		
	
delagain:			
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)		
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		//printf("Delete: The route in RIP hash table is %x\n", satosin(rt->rt_dst)->sin_addr.s_addr);		
		rt = rt->rt_back;
		rtdelete(rt->rt_forw);		
	} // for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw)

	
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto delagain;
	}
	
}

// Added by Mason Yu
// When we update the RIP Hash Table, we can delete all entry from RIP Hash Table and 
// then add all routes with flag is "RTF_UP(0x1)" from routing table into RIP Hash Table.
void updateRIPHashTable2(void)
{
	struct sockaddr_in dst, gate, netmask;	
	char buff[256];
	int flgs;
	unsigned long int d,g,m;	
	char iface[6];
	FILE *fp;	
	struct rthash *base = hosthash;	

	// Delete Hash Table all entry
	rtdeleteall2(1);	
	 
	// Add all routes into RIP Hash Table
	if (!(fp=fopen("/proc/net/route", "r"))) {		
		printf("Error: cannot open /proc/net/route - continuing...\n");		
		return -1;
	}
	
	fgets(buff, sizeof(buff), fp);
	
	while( fgets(buff, sizeof(buff), fp) != NULL ) {
		if(sscanf(buff, "%s%lx%lx%X%*d%*d%*d%lx",
		   iface, &d, &g, &flgs, &m)!=5) {
			printf("Unsuported kernel route format\n");
			fclose(fp);			
			return -1;
		}		
		
		if(flgs & RTF_UP) {
			dst.sin_addr.s_addr = d;
			dst.sin_family = AF_INET;
			
			gate.sin_addr.s_addr = g;
			gate.sin_family = AF_INET;
			
			netmask.sin_addr.s_addr = m;					
								
			if ( d != 0x7f000000 && d != 0x0 ) {					
				//printf("Add the route into Hash Table\n");	
				rtadd((struct sockaddr *)&dst, (struct sockaddr *)&gate, 1, RTS_CHANGED | RTS_PASSIVE | RTS_INTERNAL, (struct sockaddr *)&netmask);
							
			}						
		} // if(flgs & RTF_UP)		
	}// while( fgets(buff, sizeof(buff), fp) != NULL )	
	fclose(fp);		
	return 0;		
}


// Added by Mason Yu
// Update RIP Hash Table according to Routing Table.
// When the route is deleted from routing table, we should delete this entry from RIP Hash table.
// When the route is added into routing table, we should add this entry into RIP Hash table.
void updateRIPHashTable(void)
{
	struct sockaddr_in dst, gate, netmask;
	int state;	
	
	char buff[256];
	int flgs;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16], iface[6];
	FILE *fp;
	int setflag = 0;   // Mason Yu. Init hash table for all routes
	int delflag = 1;
	
	struct rt_entry *rt;	
	struct rthash *rh;
	struct rthash *base = hosthash;
	int doinghost = 1;
	int metric=0;

	// (1) Add route into RIP Hash Table
	if (!(fp=fopen("/proc/net/route", "r"))) {		
		printf("Error: cannot open /proc/net/route - continuing...\n");		
		return -1;
	}
	
	fgets(buff, sizeof(buff), fp);
	
	while( fgets(buff, sizeof(buff), fp) != NULL ) {
		// Mason Yu. Parse subnet error
		//if(sscanf(buff, "%s%lx%lx%X%*d%*d%*d%lx",
		//   iface, &d, &g, &flgs, &m)!=5) {
		if(sscanf(buff, "%s%lx%lx%X%*d%*d%d%lx",
		   iface, &d, &g, &flgs, &metric, &m)!=6) {
			printf("Unsuported kernel route format\n");
			fclose(fp);			
			return -1;
		}
		//printf("Search route: d=0x%x, flgs=0x%x\n", d, flgs);
		
		base = hosthash;
		doinghost = 1;		
		setflag = 0;  // Mason Yu. Init hash table for all routes
		
		if(flgs & RTF_UP) {
			// Mason Yu. Parse subnet error
			memset((void *)&dst, 0, sizeof(struct sockaddr_in));
			memset((void *)&gate, 0, sizeof(struct sockaddr_in));
			memset((void *)&netmask, 0, sizeof(struct sockaddr_in));
			
			dst.sin_addr.s_addr = d;
			dst.sin_family = AF_INET;
			
			gate.sin_addr.s_addr = g;
			gate.sin_family = AF_INET;
			
			netmask.sin_addr.s_addr = m;	
						
again:						
			for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)		
			for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
				if (satosin(rt->rt_dst)->sin_addr.s_addr == d || d == 0x7f000000 ) {	
					//printf("Find the route on Hash\n");				
					setflag = 1;
					goto sethashtable;					
				}else if ( satosin(rt->rt_dst)->sin_addr.s_addr != d && d != 0x7f000000 ) {					
					setflag = 0;	
					//printf("Can not Find the route on Hash\n");			
				}
				
			}
			
			if (doinghost) {
				doinghost = 0;
				base = nethash;
				goto again;
			}		

sethashtable:			
			if (!setflag) {
				setflag = 1;
				//printf("updateRIPHashTable(ADD): dest=%x\n", dst.sin_addr.s_addr);
				// Mason Yu.  Put the correct interface of route entry to rtioctl() for point-to-point link
				//rtadd((struct sockaddr *)&dst, (struct sockaddr *)&gate, 1, RTS_CHANGED | RTS_PASSIVE | RTS_INTERNAL, (struct sockaddr *)&netmask);
				// Mason Yu. Parse subnet error
				rtadd2((struct sockaddr *)&dst, (struct sockaddr *)&gate, metric+1, RTS_CHANGED | RTS_PASSIVE | RTS_INTERNAL, (struct sockaddr *)&netmask, iface);								
			}			
		} // if(flgs & RTF_UP)		
	}// while( fgets(buff, sizeof(buff), fp) != NULL )	
	fclose(fp);

	// (2) Delete route from RIP Hash Table	
	base = hosthash;
	doinghost = 1;	
	
delagain:			
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++)		
	for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		//printf("Delete: The route in RIP hash table is %x\n", satosin(rt->rt_dst)->sin_addr.s_addr);
		int delete_flag = 1;
		
		if (!(fp=fopen("/proc/net/route", "r"))) {			
			printf("Error: cannot open /proc/net/route - continuing...\n");		
			return -1;
		}
	
		fgets(buff, sizeof(buff), fp);
		
		while( fgets(buff, sizeof(buff), fp) != NULL ) {
			if(sscanf(buff, "%s%lx%lx%X%*d%*d%*d%lx",
			   iface, &d, &g, &flgs, &m)!=5) {
				printf("Unsuported kernel route format\n");
				fclose(fp);			
				return -1;
			}			
			
			if(flgs & RTF_UP) {
				dst.sin_addr.s_addr = d;
				dst.sin_family = AF_INET;					
			
				if (satosin(rt->rt_dst)->sin_addr.s_addr == d ) {
					//printf("Delete: The Hash RIP %x exist in the Routing Table. Route= %x\n", satosin(rt->rt_dst)->sin_addr.s_addr, d);										
					delete_flag = 0;
					break;
				// Mason Yu. Get ifindex of interface the packet was received.				
				//}else if ( satosin(rt->rt_dst)->sin_addr.s_addr != d && d == 0x0) {
				// Mason Yu.  Put the correct interface of route entry to rtioctl() for point-to-point link
				//}else if ( satosin(rt->rt_dst)->sin_addr.s_addr != d && d == 0x0 && g == 0x0) {
				}
#if 0
				else if ( satosin(rt->rt_dst)->sin_addr.s_addr != d && d == 0x0 && g == 0x0 && m == 0x0) {					
					//printf("Delete: The Hash RIP %x is not exist in the Routing Table. Delete the RIP.\n", satosin(rt->rt_dst)->sin_addr.s_addr);					
					rt = rt->rt_back;
					rtdelete(rt->rt_forw);												
					break;		
				}
#endif				
			} // if(flgs & RTF_UP)			
		} // while( fgets(buff, sizeof(buff), fp) != NULL )		
		fclose(fp);

		if (delete_flag) {
			rt = rt->rt_back;
			rtdelete(rt->rt_forw);
		}
		
	} // for (rt = rh->rt_forw; rt != (struct rt_entry *)rh; rt = rt->rt_forw)
	
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto delagain;
	}	
	
	// Mason Yu. Init hash table for all routes
	// We have finished the update for RIP Hash table.
	finish_update_hash = 1;	
	return 0;		
}



void timer(int signum)
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;
	int doinghost = 1, timetobroadcast;

	(void)signum;

	(void) gettimeofday(&now, (struct timezone *)NULL);
	faketime += TIMER_RATE;
	if (lookforinterfaces && (faketime % CHECK_INTERVAL) == 0)
		ifinit();
	timetobroadcast = supplier && (faketime % SUPPLY_INTERVAL) == 0;
again:
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
		rt = rh->rt_forw;
		for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
			/*
			 * We don't advance time on a routing entry for
			 * a passive gateway, or any interface if we're
			 * not acting as supplier.
			 */
			
			if(debug) {
				printf("TMR chk: flags=%x, state=%x, timer=%d, ", rt->rt_flags, rt->rt_state, rt->rt_timer);
				printf("dest=%s, ", inet_ntoa(satosin(rt->rt_dst)->sin_addr));
				printf("mask=%s, ", inet_ntoa(satosin(rt->rt_netmask)->sin_addr));
				printf("nhop=%s, ", inet_ntoa(satosin(rt->rt_router)->sin_addr));
				printf("cost=%d\n", rt->rt_metric);
			}
			
			// Casey
			//if (!(rt->rt_state & RTS_PASSIVE) &&
			//    (supplier || !(rt->rt_state & RTS_INTERFACE)))
			if (!(rt->rt_state & RTS_PASSIVE) &&
			    supplier && !(rt->rt_state & RTS_INTERFACE))
			{
				rt->rt_timer += TIMER_RATE;
			}
			if (rt->rt_timer >= GARBAGE_TIME) {
				rt = rt->rt_back;
				rtdelete(rt->rt_forw);
				continue;
			}
			if (rt->rt_timer >= EXPIRE_TIME &&
			    rt->rt_metric < HOPCNT_INFINITY)
				rtchange(rt, &rt->rt_router, HOPCNT_INFINITY);			
			rt->rt_state &= ~RTS_CHANGED;			
		}
	}
	if (doinghost) {
		doinghost = 0;
		base = nethash;
		goto again;
	}
	
	// Added by Mason Yu
	// Update RIP Hash Table according to Routing Table	
	updateRIPHashTable();	
	
	if (timetobroadcast) {
		toall(supply, 0, (struct interface *)NULL);
		lastbcast = now;
		lastfullupdate = now;
		needupdate = 0;		/* cancel any pending dynamic update */
		nextbcast.tv_sec = 0;
	}
	
	read_cfg();
	if_cfg_status();	
	
#ifdef EMBED
	signal(signum, timer);
#endif
}


void clean(void)
{
	register struct rthash *rh;
	register struct rt_entry *rt, *next_rt;
	struct rthash *base = hosthash;
	int doinghost = 1;

	if (supplier) {
again:
		for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
			rt = rh->rt_forw;
			for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw)
				rt->rt_metric = HOPCNT_INFINITY;
		}
		if (doinghost) {
			doinghost = 0;
			base = nethash;
			goto again;
		}
		toall(supply, 0, (struct interface *)NULL);
	}

	/* delete all routing entries */
	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
		rt = rh->rt_forw;
		//for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
		while (rt != (struct rt_entry *)rh) {
			next_rt = rt->rt_forw;
			//rt->rt_metric = 0;
			
			// Added by Mason Yu
			TRACE_ACTION("DELETE", rt);
	                FIXLEN(&(rt->rt_router));
	                FIXLEN(&(rt->rt_dst));
	                if (rt->rt_metric < HOPCNT_INFINITY) {
	                    if ((rt->rt_state & (RTS_INTERFACE|RTS_INTERNAL)) == RTS_INTERFACE)
	                	syslog(LOG_ERR,
	                	    "deleting route to interface %s? (timed out?)",
	                	    rt->rt_ifp->int_name);
	                    if ((rt->rt_state & (RTS_INTERNAL | RTS_EXTERNAL)) == 0 &&
	                				    rtioctl(DELETE, &rt->rt_rt) < 0)
	                	    perror("rtdelete");
	                }
	                //remque((struct qelem *)rt);
	                free((char *)rt);
			rt = next_rt;
		}
	}

}

/*
 * On hangup, let everyone know we're going away.
 */
 
extern char* pidFile;
void hup(int signum)
{
	(void)signum;	
	// Kaohj
#ifdef EMBED
	unlink(pidFile);
#endif
	clean();
	exit(1);
}


void clean_if_rt(char *if_name)
{
	register struct rthash *rh;
	register struct rt_entry *rt;
	struct rthash *base = hosthash;

	for (rh = base; rh < &base[ROUTEHASHSIZ]; rh++) {
		rt = rh->rt_forw;
		for (; rt != (struct rt_entry *)rh; rt = rt->rt_forw) {
			if (!strcmp(rt->rt_ifp->int_name, if_name)) {
				rt = rt->rt_back;
				rtdelete(rt->rt_forw);
			}
		}
	}
}
