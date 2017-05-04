#ifndef __ICMP_VOIP_H__
#define __ICMP_VOIP_H__

extern void open_voip_icmp_socket( void );

extern void close_voip_icmp_socket( void );

extern int recv_voip_icmp_message( unsigned long *pIP, unsigned short *pPort );

extern int voip_icmp_socket;

#endif /* __ICMP_VOIP_H__ */

