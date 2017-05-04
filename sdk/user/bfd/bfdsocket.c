#include "bfdsocket.h"

#define BFD_DEF_TC_PRIO		7 /*TC_PRIO_CONTROL*/

int bfd_setsockopt( int sockfd, unsigned int dscp, unsigned int ttl, unsigned int ethprio )
{
	int tc_prio=BFD_DEF_TC_PRIO;
	
	dscp=(dscp&0x3f)<<2;
	if (setsockopt(sockfd, IPPROTO_IP, IP_TOS, (char*)&dscp, sizeof(int))<0)
		printf("%s> setsockopt IP_TOS error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));

	ttl=ttl&0xff;
	if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, (char*)&ttl, sizeof(int))<0)
		printf("%s> setsockopt IP_TTL error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));

#ifdef EMBED
	ethprio=ethprio&7;
	if(setsockopt(sockfd, IPPROTO_IP, IP_SENDETHPRIO, (char *) &ethprio, sizeof(int))<0)
		printf("%s> setsockopt IP_SENDETHPRIO error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
#endif /*EMBED*/

	if(setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, (char *) &tc_prio, sizeof(int))<0)
		printf("%s> setsockopt SO_PRIORITY error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));

	return 0;
}

int bfd_bind( int port, char *intf)
{
	int 			tc_prio=BFD_DEF_TC_PRIO;
	int			sockfd,ret,n;
	struct sockaddr_in	servaddr;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if( sockfd<0 )
	{
		printf("%s> open socket error ret=%d, errno=%d(%s)\n", 
			__FUNCTION__, sockfd, errno, strerror(errno));
		return -1;
	}

	if(intf && intf[0])
	{
		ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, intf, strlen(intf)+1);
		if(ret<0)
		{
			printf("%s> setsockopt SO_BINDTODEVICE error interface=%s, ret=%d, errno=%d(%s)\n", 
				__FUNCTION__, intf, ret, errno, strerror(errno));
		}
	}
	
	n=1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n))<0)
		printf("%s> setsockopt SO_REUSEADDR error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));

	n=1;
	if(setsockopt(sockfd, IPPROTO_IP, IP_RECVTOS, (char *) &n, sizeof(n))<0)
		printf("%s> setsockopt IP_RECVTOS error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));

	n=1;
	if(setsockopt(sockfd, IPPROTO_IP, IP_RECVTTL, (char *) &n, sizeof(n))<0)
		printf("%s> setsockopt IP_RECVTTL error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));

#ifdef EMBED
	n=1;
	if(setsockopt(sockfd, IPPROTO_IP, IP_RECVETHPRIO, (char *) &n, sizeof(n))<0)
		printf("%s> setsockopt IP_RECVETHPRIO error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
#endif /*EMBED*/

	if(setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, (char *) &tc_prio, sizeof(int))<0)
		printf("%s> setsockopt SO_PRIORITY error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));


	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	{
		int bindloop=10;
		while(bindloop--)
		{
			ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
			if( ret<0 )
				printf("%s> bind error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
			else
				break;				
		}
		if(bindloop==0)
		{
			printf("%s> bind failed\n", __FUNCTION__);
			close(sockfd);
			return -1;
		}
	}

	ret=fcntl(sockfd, F_GETFL);
	if(ret==-1)
	{
		printf("%s> fcntl(F_GETFL) failed, errno=%d(%s) \n", __FUNCTION__, errno, strerror(errno) );
		close(sockfd);
		return -1;
	}else{
		fcntl(sockfd, F_SETFL, ret|O_NONBLOCK);
	}

	return sockfd;
}

int bfd_close(int s)
{
	return close(s);
}

int bfd_recvfrom( int s, void *buf, int len )
{
	struct sockaddr_in	cliaddr;
	int			n;
	socklen_t		fromlen;
	
	fromlen = sizeof(cliaddr);
	n = recvfrom(s, buf, len, 0, (struct sockaddr *)&cliaddr, &fromlen );
	if(n<0)
	{ 
		printf("%s> recvfrom error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
	}else{
		//printf("%s> Get from %s:%d, len=%d\n", __FUNCTION__, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), n );
	}
	
	return n;
}

int bfd_recvfrom_with_srcinfo( int s, void *buf, int len, struct sockaddr_in *pcliaddr)
{
	struct sockaddr_in	cliaddr;
	int			n;
	socklen_t		fromlen;

	if(pcliaddr==NULL) pcliaddr=&cliaddr;	
	fromlen = sizeof(struct sockaddr_in);
	n = recvfrom(s, buf, len, 0, (struct sockaddr *)pcliaddr, &fromlen );
	if(n<0)
	{ 
		printf("%s> recvfrom error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
	}else{
		//printf("%s> Get from %s:%d, len=%d\n", __FUNCTION__, inet_ntoa(pcliaddr->sin_addr), ntohs(pcliaddr->sin_port), n );
	}
	
	return n;
}

int bfd_recvmsg(int sock, void *buf, int size, struct sockaddr_in *from, unsigned int *dscp, unsigned int *ttl, unsigned int *ethprio)
{
	int ret;
	struct msghdr msg;
	struct iovec iov;
	struct cmsghdr *ptr;
	char adata[1024];

	if( (buf==NULL) || (from==NULL) ) return -1;
	if(dscp) *dscp=0;
	if(ttl) *ttl=0;
	if(ethprio) *ethprio=0;

	msg.msg_name = (void *) from;
	msg.msg_namelen = sizeof (struct sockaddr_in);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = (void *) adata;
	msg.msg_controllen = sizeof adata;
	iov.iov_base = buf;
	iov.iov_len = size;
	
	ret = recvmsg (sock, &msg, 0);
	if (ret < 0)
	{
		printf("%s> recvfrom error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		return ret;
	}

	for (ptr = CMSG_FIRSTHDR(&msg); ptr != NULL; ptr = CMSG_NXTHDR(&msg, ptr))
	{
		//printf("%s> ptr->cmsg_level=%d \n", __FUNCTION__, ptr->cmsg_level );
		if(ptr->cmsg_level==IPPROTO_IP) 
		{
			//printf("%s> ptr->cmsg_type=%d \n", __FUNCTION__, ptr->cmsg_type );
			if(ptr->cmsg_type == IP_TOS)
			{
				unsigned char *ptos;
				ptos=(unsigned char *)CMSG_DATA(ptr);
				if(ptos)
				{
					//printf("%s> recvmsg() with IP_TOS=%u (DSCP=%u)\n", __FUNCTION__, *ptos, *ptos>>2 );
					if(dscp) *dscp=*ptos>>2;
				}
			}else if(ptr->cmsg_type == IP_TTL)
			{
				unsigned int *pttl;
				pttl=(unsigned int *)CMSG_DATA(ptr);
				if(pttl)
				{
					//printf("%s> recvmsg() with IP_TTL=%u\n", __FUNCTION__, *pttl );
					if(ttl) *ttl=*pttl;
				}
#ifdef EMBED
			}else if(ptr->cmsg_type == IP_RECVETHPRIO)
			{
				unsigned int *pethprio;
				pethprio=(unsigned int *)CMSG_DATA(ptr);
				if(pethprio)
				{
					//printf("%s> recvmsg() with IP_RECVETHPRIO=%u\n", __FUNCTION__, *pethprio );
					if(ethprio) *ethprio=*pethprio;
				}
#endif /*EMBED*/
			}
		}
	}
	
	return ret;
}

int bfd_sendto( int s, void *buf, int len, unsigned char *rip, unsigned short rport )
{
	struct sockaddr_in	servaddr;
	int			n;
	socklen_t		servlen;
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(rport);
	inet_aton( rip, &servaddr.sin_addr );
	servlen = sizeof(servaddr);
	n = sendto(s, buf, len, 0, (struct sockaddr *)&servaddr, servlen );
	if(n<0)
	{ 
		printf("%s> sendto error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
	}

	//debug
	if(n!=len) printf("%s> NOTICE!!! sendto() sent n=%d, not len=%d\n", __FUNCTION__, n, len );

	return n;
}
/*******************************************************************************/
struct bfdudp_data
{
	unsigned char data[256];
};
struct bfdudp_packet
{
	struct iphdr ip;
	struct udphdr udp;
	struct bfdudp_data data;
};

static u_int16_t bfdudp_checksum(void *addr, int count)
{
	/* Compute Internet Checksum for "count" bytes
	 *         beginning at location "addr".
	 */
	register int32_t sum = 0;
	u_int16_t *source = (u_int16_t *) addr;

	while (count > 1)  {
		/*  This is the inner loop */
		sum += *source++;
		count -= 2;
	}

	/*  Add left-over byte, if any */
	if (count > 0) {
		/* Make sure that the left-over byte is added correctly both
		 * with little and big endian hosts */
		u_int16_t tmp = 0;
		*(unsigned char *) (&tmp) = * (unsigned char *) source;
		sum += tmp;
	}
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~sum;
}

int bfd_raw_bind(int ifindex, unsigned int ethprio)
{
	int fd;
	int result;
	struct sockaddr_ll dest;
	int tc_prio=BFD_DEF_TC_PRIO;

	if ((fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
		printf( "%s> socket call failed: %s\n", __FUNCTION__, strerror(errno) );
		return -1;
	}

#ifdef EMBED
	ethprio=ethprio&0x7;
	if(setsockopt(fd, SOL_PACKET, PACKET_SENDETHPRIO, (char *) &ethprio, sizeof(int))<0)
		printf("%s> setsockopt PACKET_SENDETHPRIO error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
#endif /*EMBED*/

	if(setsockopt(fd, SOL_SOCKET, SO_PRIORITY, (char *) &tc_prio, sizeof(int))<0)
		printf("%s> setsockopt SO_PRIORITY error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
	
	memset(&dest, 0, sizeof(dest));	
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	dest.sll_ifindex = ifindex;
	if (bind(fd, (struct sockaddr *)&dest, sizeof(struct sockaddr_ll)) < 0) 
	{
		printf("%s> bind call failed: %s\n", __FUNCTION__, strerror(errno));
		close(fd);
		return -1;
	}

	return fd;
}

int bfd_raw_sendto(
	int fd, 
	unsigned char *payload, int len, 
	unsigned char *psource_ip, int source_port,
	unsigned char *pdest_ip, int dest_port, 
	unsigned char *dest_arp, int ifindex,
	unsigned int dscp, unsigned int ttl)
{
	int result;
	struct in_addr sin;
	struct in_addr din;
	struct sockaddr_ll dest;
	struct bfdudp_packet packet;
	unsigned short tot_len;

	if( (fd<0) || (payload==NULL) || (len<=0) || 
		(psource_ip==NULL) || (pdest_ip==NULL) ||
		(len>sizeof(struct bfdudp_data)) || (dest_arp==NULL) )
		return -1;

	inet_aton( psource_ip, &sin);
	inet_aton( pdest_ip, &din);
	
	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_IP);
	dest.sll_ifindex = ifindex;
	dest.sll_halen = 6;
	memcpy(dest.sll_addr, dest_arp, 6);

	memset(&packet, 0, sizeof(packet));
	packet.ip.protocol = IPPROTO_UDP;
	packet.ip.saddr = sin.s_addr;
	packet.ip.daddr = din.s_addr;
	packet.udp.source = htons(source_port);
	packet.udp.dest = htons(dest_port);
	packet.udp.len = htons(sizeof(packet.udp) + len); /* cheat on the psuedo-header */
	packet.ip.tot_len = packet.udp.len;
	memcpy(&(packet.data), payload, len);
	tot_len=sizeof(struct iphdr)+sizeof(struct udphdr)+len;
	packet.udp.check = bfdudp_checksum(&packet, tot_len );
	
	packet.ip.tot_len = htons(tot_len);
	packet.ip.ihl = sizeof(packet.ip) >> 2;
	packet.ip.tos= (dscp&0x3f)<<2;
	packet.ip.version = IPVERSION;
	packet.ip.ttl = ttl&0xff; //IPDEFTTL;
	packet.ip.check = bfdudp_checksum(&(packet.ip), sizeof(packet.ip));

	result = sendto(fd, &packet, tot_len, 0, (struct sockaddr *) &dest, sizeof(dest));
	if (result < 0) {
		printf("%s> write on socket failed: %s\n", __FUNCTION__, strerror(errno));
	}

	//debug
	if(result!=tot_len) printf("%s> NOTICE!!! sendto() sent result=%d, not tot_len=%d\n", __FUNCTION__, result, tot_len );

	return result;
}


/*******************************************************************************/
int bfd_get_arp( unsigned char *intfname, unsigned char *ipaddr, unsigned char *macaddr )
{
	int ret=-1;
	int sockfd;
	struct arpreq ar;
	struct sockaddr_in *sin;

	if( (intfname==NULL)||(ipaddr==NULL)||(macaddr==NULL) )
		return ret;

	sockfd=socket( AF_INET, SOCK_DGRAM, 0 );
	if(sockfd<0)
	{
		printf("%s> socket error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		return ret;
	}

	memset( &ar, 0, sizeof(struct arpreq) );
	sin=(struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family=AF_INET;
	if( inet_aton( ipaddr, &sin->sin_addr)==0)
	{
		printf("%s> inet_aton() error\n", __FUNCTION__);
		close(sockfd);
		return ret;
	}
	strcpy( ar.arp_dev, intfname );
	if( ioctl( sockfd, SIOCGARP, &ar)<0 )
	{
		printf("%s> ioctl(SIOCGARP) error errno=%d(%s)\n", __FUNCTION__, errno, strerror(errno));
		close(sockfd);
		return ret;
	}
	close(sockfd);
	ret=0;
	memcpy( macaddr, ar.arp_ha.sa_data, 6);
	//printf( "%s>intfname=%s,  ipaddr=%s, macaddr=%02x%02x%02x%02x%02x%02x\n", 
	//	__FUNCTION__, intfname, ipaddr, macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5] );

	return ret;
}

int bfd_get_ifindex(unsigned char *intfname, int *pifindex)
{
	int fd, ret=-1;
	struct ifreq ifr;

	if( (intfname==NULL) || (intfname[0]==0) || (pifindex==NULL) )
		return ret;

	*pifindex=0;
	memset(&ifr, 0, sizeof(struct ifreq));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if( fd>=0 )
	{
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, intfname);
		if (ioctl(fd, SIOCGIFINDEX, &ifr)==0)
		{
			*pifindex = ifr.ifr_ifindex;
			ret=0;
		}
		close(fd);
	}else{
		//printf("%s> socket failed!: %s\n", __FUNCTION__, strerror(errno));
	}

	return ret;
}

int bfd_get_ifflags(unsigned char *intfname, int *pifflags)
{
	int fd, ret=-1;
	struct ifreq ifr;

	if( (intfname==NULL) || (intfname[0]==0) || (pifflags==NULL) )
		return ret;

	*pifflags=0;
	memset(&ifr, 0, sizeof(struct ifreq));
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if( fd>=0 )
	{
		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, intfname);
		if (ioctl(fd, SIOCGIFFLAGS, &ifr)==0)
		{
			*pifflags = ifr.ifr_flags;
			ret=0;
		}
		close(fd);
	}else{
		//printf("%s> socket failed!: %s\n", __FUNCTION__, strerror(errno));
	}

	return ret;
}
/*******************************************************************************/

