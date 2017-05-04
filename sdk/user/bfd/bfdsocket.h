#ifndef _BFDSOCKET_H_
#define _BFDSOCKET_H_

#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>

extern int bfd_setsockopt( int sockfd, unsigned int dscp, unsigned int ttl, unsigned int ethprio );
extern int bfd_bind( int port, char *intf);
extern int bfd_close(int s);
extern int bfd_recvfrom( int s, void *buf, int len );
extern int bfd_recvfrom_with_srcinfo( int s, void *buf, int len, struct sockaddr_in *pcliaddr);
extern int bfd_recvmsg(int sock, void *buf, int size, struct sockaddr_in *from, unsigned int *dscp, unsigned int *ttl, unsigned int *ethprio);
extern int bfd_sendto( int s, void *buf, int len, unsigned char *rip, unsigned short rport );

extern int bfd_raw_bind(int ifindex, unsigned int ethprio);
extern int bfd_raw_sendto(
	int fd, 
	unsigned char *payload, int len, 
	unsigned char *psource_ip, int source_port,
	unsigned char *pdest_ip, int dest_port, 
	unsigned char *dest_arp, int ifindex,
	unsigned int dscp, unsigned int ttl);

extern int bfd_get_arp( unsigned char *intfname, unsigned char *ipaddr, unsigned char *macaddr );
extern int bfd_get_ifindex(unsigned char *intfname, int *pifindex);
extern int bfd_get_ifflags(unsigned char *intfname, int *pifflags);
#endif /*_BFDSOCKET_H_*/

