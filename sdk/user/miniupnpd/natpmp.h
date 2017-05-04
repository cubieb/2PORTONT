/* $Id: natpmp.h,v 1.1 2008/05/23 09:06:59 adsmt Exp $ */
/* MiniUPnP project
 * author : Thomas Bernard
 * website : http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 */
#ifndef __NATPMP_H__
#define __NATPMP_H__

#define NATPMP_PORT (5351)

int OpenAndConfNATPMPSocket();

void ProcessIncomingNATPMPPacket(int s);

int ScanNATPMPforExpiration();

int CleanExpiredNATPMP();

#endif

