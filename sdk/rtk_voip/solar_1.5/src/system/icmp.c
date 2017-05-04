#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#include "linphonecore.h"
#include "icmp.h"

int voip_icmp_socket = 0;

void open_voip_icmp_socket( void )
{
	voip_icmp_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP );
	
	if( voip_icmp_socket < 0 )
		printf( "Open ICMP socket error\n" );
}

void close_voip_icmp_socket( void )
{
	if( voip_icmp_socket > 0 ) {
		close( voip_icmp_socket );
		voip_icmp_socket = 0;
	}
}

#if 0
/* common routines */
static unsigned short in_cksum( const unsigned short *buf, int sz )
{
    int nleft = sz;
    int sum = 0;
    const unsigned short *w = buf;
    unsigned short ans = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char *) (&ans) = *(unsigned char *) w;
        sum += ans;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    ans = ~sum;
    return (ans);
}
#endif

int recv_voip_icmp_message( unsigned long *pIP, unsigned short *pPort )
{
#define PACKET_BUFFER_SIZE		1024

	unsigned long packet[ PACKET_BUFFER_SIZE / sizeof( unsigned long ) ];

	int count;
	struct sockaddr_in from;
	size_t fromlen = sizeof(from);
	
	const struct iphdr *iphdr;
	const struct icmp *icmphdr;
	//unsigned short checksum;
	
	const struct iphdr *iphdr_old;
	const struct udphdr *udphdr_old;
	
	if( voip_icmp_socket <= 0 )
		return 0;
	
	count = recvfrom( voip_icmp_socket, packet, PACKET_BUFFER_SIZE, 
						0,
						(struct sockaddr *) &from, &fromlen );
	
	if( count <= 0 )
		return 0;
		
#if 0
	/* add string's zero padding for parsing SIP message */
	if( count < PACKET_BUFFER_SIZE )
		*( ( unsigned char * )packet + count ++ ) = '\x0';
	else
		*( ( unsigned char * )packet + count - 1 ) = '\x0';
#endif
		
	/* IP header */
	iphdr = (struct iphdr *) packet;

#if 0	/* we had specified this socket is ICMP protocol */
	if( ( checksum = in_cksum( ( const unsigned short * )packet, count ) ) != 0 ) {
		printf( "IP checksum:%04X\n", checksum );
		return 0;
	}
	
	if( iphdr ->protocol != 0x01 ) {
		printf( "Not an ICMP packet (protocol:%u)\n", iphdr ->protocol );
		return 0;
	}
#endif
	
	/* ICMP header */
	icmphdr = (struct icmp *) &packet[ iphdr->ihl ]; /* skip ip hdr */

#if 0	/* we had specified this socket is ICMP protocol */
	if( ( const unsigned char * )icmphdr + ICMP_MINLEN > 
		( const unsigned char * )packet + count ) 
	{
		printf( "ICMP header is too short\n" );
		return 0;
	}
	
	if( ( checksum = in_cksum( ( const unsigned short * )icmphdr, count - ( iphdr->ihl << 2 ) ) ) != 0 ) {
		printf( "ICMP checksum:%04X\n", checksum );
		return 0;
	}
#endif

	/* handle 'destination unreachable' only */ 
	if( icmphdr->icmp_type != ICMP_DEST_UNREACH ) {
		g_message( "Not handle ICMP type:%d\n", icmphdr->icmp_type );
		return 0;
	}
	
	switch( icmphdr->icmp_code ) {
	case ICMP_HOST_UNREACH:	/* 0x01 */
	case ICMP_PORT_UNREACH:	/* 0x03 */
		break;
		
	default:
		g_message( "Not handle ICMP code:%d\n", icmphdr->icmp_code );
		return 0;
	}
	
	/* old IP */
#if ICMP_MINLEN % 4 == 0
	iphdr_old = ( struct iphdr * )
					( ( const unsigned long * )icmphdr + ICMP_MINLEN / 4 );
#else
	iphdr_old = ( struct iphdr * )
					( ( const unsigned char * )icmphdr + ICMP_MINLEN );
#endif
	
	if( iphdr_old ->version == 4 &&					/* IPv4 */
		iphdr_old ->protocol == IPPROTO_UDP )		/* UDP (0x11) */
	{
	} else {
		g_warning( "Incorrect version(%d) or protocol(%d)\n", iphdr_old ->version, iphdr_old ->protocol );
		return 0;
	}
	
	if( ( const unsigned char * )iphdr_old + ( iphdr_old ->ihl << 2 ) >
		( const unsigned char * )packet + count )
	{
		g_warning( "Old IP header is not complete\n" );
		return 0;
	}
		
	/* dst IP of ICMP == src IP of original */
#if 0
	if( iphdr ->daddr != iphdr_old ->saddr ) {
		printf( "ICMP: IP address not match\n" );
		return 0;
	}
#endif
	
	/* old UDP */
#if ICMP_MINLEN % 4 == 0
	udphdr_old = ( const struct udphdr * )
					( ( const unsigned long * )iphdr_old + iphdr_old ->ihl );
#else
	udphdr_old = ( const struct udphdr * )
					( ( const unsigned char * )iphdr_old + ( iphdr_old ->ihl << 2 ) );
#endif
	
	if( ( const unsigned char * )udphdr_old + sizeof( struct udphdr ) >
		( const unsigned char * )packet + count )
	{
		g_warning( "Old UDP header is not complete\n" );
		return 0;
	}
	
	/* retrieve old DST (IP,port) */
	*pIP = iphdr_old ->daddr;
	*pPort = udphdr_old ->dest;

#if 0	
	printf( "ICMP: Old Dst IP address:%08X,port:%d\n", iphdr_old ->daddr, udphdr_old ->dest );	
	
	for( i = 0; i < 10; i ++ )
		printf( "%08X, ", packet[ i ] );
		
	printf( "\n" );
#endif
	
	return count;

#undef PACKET_BUFFER_SIZE
}


