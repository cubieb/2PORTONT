/*
**  smcroute - static multicast routing control 
**  Copyright (C) 2001 Carsten Schill <carsten@cschill.de>
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
**
**  $Id: ifvc.c,v 1.3 2006/01/17 16:51:31 kaohj Exp $	
**
**  This module manages an interface vector of the machine
**
*/

//#include "mclab.h"
#include "igmpproxy.h"
#include <linux/sockios.h>

struct IfDesc IfDescVc[ MAX_IF ], *IfDescEp = IfDescVc;

struct IfDesc *getIfByName( const char *IfName );

void buildIfVc()
/*
** Builds up a vector with the interface of the machine. Calls to the other functions of 
** the module will fail if they are called before the vector is build.
**          
*/
{
	struct ifreq IfVc[ sizeof( IfDescVc ) / sizeof( IfDescVc[ 0 ] )  ];
	struct ifreq *IfEp;

	int Sock;

	memset(IfDescVc, 0, sizeof( IfDescVc ) );
	IfDescEp = IfDescVc;
	
	if( (Sock = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
		log( LOG_ERR, errno, "RAW socket open" );
	
	/* get If vector
	 */
	{
		struct ifconf IoCtlReq;
		
		IoCtlReq.ifc_buf = (void *)IfVc;
		IoCtlReq.ifc_len = sizeof( IfVc );
		
		if( ioctl( Sock, SIOCGIFCONF, &IoCtlReq ) < 0 )
			log( LOG_ERR, errno, "ioctl SIOCGIFCONF" );
		
		IfEp = (void *)((char *)IfVc + IoCtlReq.ifc_len);
	}
	
	/* loop over interfaces and copy interface info to IfDescVc
	 */
	{
		struct ifreq  *IfPt;
		struct IfDesc *IfDp;
		
		for( IfPt = IfVc; IfPt < IfEp; IfPt++ ) {
			char FmtBu[ 32 ];
		
			strncpy( IfDescEp->Name, IfPt->ifr_name, sizeof( IfDescEp->Name ) );
			
			/* don't retrieve more info for non-IP interfaces
			 */
			if( IfPt->ifr_addr.sa_family != AF_INET ) {
				IfDescEp->InAdr.s_addr = 0;  /* mark as non-IP interface */
				IfDescEp++;
				continue;
	    		}
	
			IfDescEp->InAdr = ((struct sockaddr_in *)&IfPt->ifr_addr)->sin_addr;
			
			/* get if flags
			**
			** typical flags:
			** lo    0x0049 -> Running, Loopback, Up
			** ethx  0x1043 -> Multicast, Running, Broadcast, Up
			** ipppx 0x0091 -> NoArp, PointToPoint, Up 
			** grex  0x00C1 -> NoArp, Running, Up
			** ipipx 0x00C1 -> NoArp, Running, Up
			*/
			{
				struct ifreq IfReq;
				
				memcpy( IfReq.ifr_name, IfDescEp->Name, sizeof( IfReq.ifr_name ) );
				
				if( ioctl( Sock, SIOCGIFFLAGS, &IfReq ) < 0 )
					log( LOG_ERR, errno, "ioctl SIOCGIFFLAGS" );
				
				IfDescEp->Flags = IfReq.ifr_flags;
			}
				
			log( LOG_DEBUG, 0, "buildIfVc: Interface %s Addr: %s, Flags: 0x%04x",
			IfDescEp->Name,
			fmtInAdr( FmtBu, IfDescEp->InAdr ),
			IfDescEp->Flags );
			IfDescEp++; 
		} 
	}
	
	// find the missing upstream interface, especially for dynamic interface (IP not ready)
	{
		void *p;
		struct ifreq IfReq;
		//char FmtBu[ 32 ];
	#ifdef CONFIG_IGMPPROXY_MULTIWAN	
		int idx;
		for(idx=0;idx<igmp_up_if_num;idx++) {
			p = getIfByName(igmp_up_if_name[idx]);
			if (p==NULL && (IfDescEp-IfDescVc)<MAX_IF) { // interface without IP, add one and mark as non-IP
				strncpy( IfDescEp->Name, igmp_up_if_name[idx], sizeof( IfDescEp->Name ) );
				IfDescEp->InAdr.s_addr = 0;  /* mark as non-IP interface */
				memcpy( IfReq.ifr_name, igmp_up_if_name[idx], sizeof( IfReq.ifr_name ) );
				if( ioctl( Sock, SIOCGIFFLAGS, &IfReq ) < 0 )
					log( LOG_ERR, errno, "ioctl SIOCGIFFLAGS" );
				IfDescEp->Flags = IfReq.ifr_flags;
				//printf( "buildIfVc brg: Interface %s Addr: %s, Flags: 0x%04x\n",
				//	IfDescEp->Name,
				//	fmtInAdr( FmtBu, IfDescEp->InAdr ),
				//	IfDescEp->Flags );
				IfDescEp++;
			}
		}
	#else
		p = getIfByName(igmp_up_if_name);
		if (p==NULL && (IfDescEp-IfDescVc)<MAX_IF) { // interface without IP, add one and mark as non-IP
			strncpy( IfDescEp->Name, igmp_up_if_name, sizeof( IfDescEp->Name ) );
			IfDescEp->InAdr.s_addr = 0;  /* mark as non-IP interface */
			memcpy( IfReq.ifr_name, igmp_up_if_name, sizeof( IfReq.ifr_name ) );
			if( ioctl( Sock, SIOCGIFFLAGS, &IfReq ) < 0 )
				log( LOG_ERR, errno, "ioctl SIOCGIFFLAGS" );
			IfDescEp->Flags = IfReq.ifr_flags;
			//printf( "buildIfVc brg: Interface %s Addr: %s, Flags: 0x%04x\n",
			//	IfDescEp->Name,
			//	fmtInAdr( FmtBu, IfDescEp->InAdr ),
			//	IfDescEp->Flags );
			IfDescEp++;
		}
	#endif
	}
	
	close( Sock );
}

void updateIfVc()
{
	struct IfDesc *Dp;
	int Sock;
	struct ifreq ifr;
	struct sockaddr_in *addr;
	//char FmtBu[ 32 ];

	if( (Sock = socket( AF_INET, SOCK_DGRAM, 0 )) < 0 )
		log( LOG_ERR, errno, "RAW socket open" );
	
	for( Dp = IfDescVc; Dp < IfDescEp; Dp++ ) {
		strcpy(ifr.ifr_name, Dp->Name);
		if( ioctl( Sock, SIOCGIFADDR, &ifr ) < 0 ) {
			Dp->InAdr.s_addr = 0;
		}
		else {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			Dp->InAdr = *((struct in_addr *)&addr->sin_addr);
		}
		
		if( ioctl( Sock, SIOCGIFFLAGS, &ifr ) < 0 ) {
			log( LOG_ERR, errno, "ioctl SIOCGIFFLAGS" );
			continue;
		}
		Dp->Flags = ifr.ifr_flags;
		//printf( "updateIfVc: Interface %s Addr: %s, Flags: 0x%04x\n",
		//	Dp->Name,
		//	fmtInAdr( FmtBu, Dp->InAdr ),
		//	Dp->Flags );
	}
	close(Sock);
}

struct IfDesc *getIfByName( const char *IfName )
/*
** Returns a pointer to the IfDesc of the interface 'IfName'
**
** returns: - pointer to the IfDesc of the requested interface
**          - NULL if no interface 'IfName' exists
**          
*/
{
  struct IfDesc *Dp;

  for( Dp = IfDescVc; Dp < IfDescEp; Dp++ ) 
    if( ! strcmp( IfName, Dp->Name ) ) 
      return Dp;

  return NULL;
}

struct IfDesc *getIfByIx( unsigned Ix )
/*
** Returns a pointer to the IfDesc of the interface 'Ix'
**
** returns: - pointer to the IfDesc of the requested interface
**          - NULL if no interface 'Ix' exists
**          
*/
{
  struct IfDesc *Dp = &IfDescVc[ Ix ];
  return Dp < IfDescEp ? Dp : NULL;
}



int chk_local( __u32 addr )
/*
** Returns a pointer to the IfDesc of the interface 'IfName'
**
** returns: - pointer to the IfDesc of the requested interface
**          - NULL if no interface 'IfName' exists
**          
*/
{
	struct IfDesc *Dp;

	for( Dp = IfDescVc; Dp < IfDescEp; Dp++ ) 
  		if( Dp->InAdr.s_addr == addr )
  			return 1;

  return 0;
}
