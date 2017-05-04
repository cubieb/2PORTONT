/*
 *      arping.c - Ping hosts by ARP requests/replies to do duplicate
 *                 address detection
 *
 *      Authors: Hung-Jen Kao	<kaohj@realtek.com.tw>
 *
 *
 */

#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/time.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <linux/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <netdb.h>

#include 	<netinet/ip_icmp.h>

#include "utility.h"
#include "debug.h"

struct in_addr src;
struct in_addr dst;
struct sockaddr_ll me;
struct sockaddr_ll he;
struct timeval last;

#define MAX_COUNT	3
#define PINGINTERVAL	1	/* second */
#define MAXWAIT		3

static int count;
static int finished;
static int s;

#define MS_TDIFF(tv1,tv2) ( ((tv1).tv_sec-(tv2).tv_sec)*1000 + \
			   ((tv1).tv_usec-(tv2).tv_usec)/1000 )


#define ERR_RESOURCE -1 
#define ERR_UNREACHABLE -2

//ICMP echo request test

/*
INPUT
       intf: name of the outgoing interface
	host: name of the destination host.
     count: number of request to test.
     timeout: time in millisec to wait
     datasize: size of the payload
     tos: specify the IP ToS
OUTPUT
	cntOK: number of successful PING
	cntFail: number of failed PING
	timeAvg: average response time
	timeMin: minimum response time
	timeMax: maximum response time
RETURN
	0: success
     -1: resource error
     -2: host unreachable     
     
**/
int icmp_test(char *intf, char *host, unsigned int count, unsigned int timeout, unsigned int datasize, unsigned char tos,
	unsigned int *cntOK, unsigned int *cntFail, unsigned int *timeAvg, unsigned int *timeMin, unsigned int *timeMax) 
{
	struct protoent *proto;
	struct sockaddr_in sockaddr;
	struct hostent *h;
	struct icmp *icmppkt;
	struct timeval tv;

	
	unsigned char *buffer;
	fd_set rset;	
	int sock, bufsize, ret, pingid, cnt;
	//unsigned int attempt;
	unsigned short uSequence = 0, uExpected;
	unsigned int uOK = 0, uFail = 0;
	unsigned int tAvg = 0, tMin = 0, tMax = 0;
	int int_op;


	// create socket
	proto = getprotobyname("icmp");
	
	if ((sock = socket(AF_INET, SOCK_RAW, (proto ? proto->p_proto : IPPROTO_ICMP))) < 0) {        /* 1 == ICMP */
		//return ERR_RESOURCE;
		goto error_resource;
	}

	int_op = tos;
	if ((ret = setsockopt(sock, IPPROTO_IP, IP_TOS, (char *)&int_op, sizeof (int_op))) < 0) {
		//printf("set QoS %d returns %d\n", int_op, ret);
	}

	if (intf) {
		ret = setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, intf, strlen(intf)+1);
		if (ret != 0) 
			printf("failed to bind to %s. ret=%d\n", intf, ret);
	}

	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	
	sockaddr.sin_family = AF_INET;

	// resolve host name	
	if ((h = gethostbyname(host)) == NULL) {		
		//printf("ping: Get HostbyName Error\n");		
		//return ERR_UNREACHABLE;
		goto error_host;
	}
	
	if (h->h_addrtype != AF_INET) {
		//printf("unknown address type; only AF_INET is currently supported.\n");
		//return ERR_UNREACHABLE;
		goto error_resource;
		
	}
	
	memcpy(&sockaddr.sin_addr, h->h_addr, sizeof(sockaddr.sin_addr));

	printf("PING %s (%s): %d data bytes\n",
           h->h_name,
           inet_ntoa(*(struct in_addr *) &sockaddr.sin_addr.s_addr), datasize);

	FD_ZERO(&rset);
       FD_SET(sock, &rset);

	// build the packet
	bufsize = sizeof(icmppkt) + datasize;
       buffer = malloc ( bufsize );
       if (0==buffer) {
       	printf("no buffer available\n");
       	goto error_resource;
       	//return ERR_RESOURCE;
       }

	pingid = getpid() & 0xFFFF;

	cnt = 0;
	while (count > 0) {
		struct timeval time1, time2;
		
		count --;		

		uSequence++;
		uExpected = uSequence;
		icmppkt = (struct icmp *) buffer;
		icmppkt->icmp_type = ICMP_ECHO;
		icmppkt->icmp_code = 0;
		icmppkt->icmp_cksum = 0;
		icmppkt->icmp_seq = uSequence;
		icmppkt->icmp_id = pingid;
		icmppkt->icmp_cksum = in_cksum((unsigned short *) icmppkt, bufsize);
		
		ret = sendto(sock, buffer, bufsize, MSG_DONTWAIT,  (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in));

		if ((ret < 0) && (ret != EAGAIN)) {
			uFail++;
			continue;			
		}

		
		gettimeofday(&time1, 0);

		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		
		ret = select(sock+1, &rset, NULL, NULL, &tv);
		if (ret==0) { // timeout
			uFail++;

			// debug info.
			//gettimeofday(&time2, 0);
			//printf("timeout: %u.%u\n", time2.tv_sec, time2.tv_usec);
			
		} else if (ret > 0) {
			int c, hlen;
			struct sockaddr_in from;
			struct iphdr *ippkt;
			struct icmp *pkt;
			unsigned int delta;
			socklen_t fromlen = (socklen_t) sizeof(from);
			c = recvfrom(sock, buffer, bufsize, 0,  (struct sockaddr *) &from, &fromlen);
			if (c < 0) { // recv error
				uFail++;
				printf("recv returns %d\n", c);
				continue;
			}

			gettimeofday(&time2, 0);
			
			// size is wrong
			if (c < bufsize) {
				uFail++;
				printf("size is wrong %d,%d\n", c, bufsize);
				continue;
			}

			ippkt = (struct iphdr *)buffer;
			hlen = ippkt->ihl << 2;
			pkt = (struct icmp *) (buffer + hlen);	
			if (ntohs(pkt->icmp_id) != pingid) {
				uFail++;
				//printf("ping id mismatch %d,%d\n", pingid, ntohs(pkt->icmp_id));
				continue;
			}

			if (pkt->icmp_type != ICMP_ECHOREPLY) {
				uFail++;
				//printf("not icmp\n");
				continue;
			}

			if (ntohs(pkt->icmp_seq) != uExpected) {
				uFail++;
				//printf("seq mismatch %d, %d\n", ntohs(pkt->icmp_seq), uExpected);
				continue;
			}
			cnt++;

			delta = (time2.tv_sec * 1000 + time2.tv_usec / 1000) - (time1.tv_sec * 1000 + time1.tv_usec / 1000);
			
			if (cnt == 1) {
				tAvg = tMin = tMax = delta;
			} else {
				tAvg = tAvg + (int)(delta - tAvg) / cnt;
				if (tMin > delta) tMin = delta;
				if (tMax < delta)tMax = delta;			
			}
					
			uOK++;

			
		}
		
	
	}

	if (cntOK) *cntOK = uOK;
	if (cntFail) *cntFail = uFail;
	if (uOK > 0) {
		if (timeAvg) *timeAvg = tAvg;
		if (timeMin) *timeMin= tMin;
		if (timeMax) *timeMax = tMax;
	} else {
		if (timeAvg) *timeAvg = 0;
		if (timeMin) *timeMin= 0;
		if (timeMax) *timeMax = 0;
	}
	free(buffer);
	close(sock);
	return 0;
error_resource:
	close(sock);
	return ERR_RESOURCE;
error_host:
	close(sock);
	return ERR_UNREACHABLE;
	
	
}

static int send_pack(int sock, struct in_addr *src_addr,
					 struct in_addr *dst_addr, struct sockaddr_ll *ME,
					 struct sockaddr_ll *HE)
{
	int err;
	struct timeval now;
	unsigned char buf[256];
	struct arphdr *ah = (struct arphdr *) buf;
	unsigned char *p = (unsigned char *) (ah + 1);

	ah->ar_hrd = htons(ME->sll_hatype);
	ah->ar_hrd = htons(ARPHRD_ETHER);
	ah->ar_pro = htons(ETH_P_IP);
	ah->ar_hln = ME->sll_halen;
	ah->ar_pln = 4;
	ah->ar_op = htons(ARPOP_REQUEST);

	memcpy(p, &ME->sll_addr, ah->ar_hln);
	p += ME->sll_halen;

	memcpy(p, src_addr, 4);
	p += 4;

	memcpy(p, &HE->sll_addr, ah->ar_hln);
	p += ah->ar_hln;

	memcpy(p, dst_addr, 4);
	p += 4;

	gettimeofday(&now, NULL);
	err = sendto(sock, buf, p - buf, 0, (struct sockaddr *) HE, sizeof(*HE));
	if (err == p - buf) {
		last = now;
	}
	return err;
}

static void finish(int signum)
{
	finished=1;
}

static void catcher(int signum)
{
	struct timeval tv;
	static struct timeval start;

	gettimeofday(&tv, NULL);

	if (start.tv_sec == 0)
		start = tv;

	if (last.tv_sec == 0 || MS_TDIFF(tv, last) > 500) {
		send_pack(s, &src, &dst, &me, &he);
		count++;
		if (count == MAX_COUNT) {
			signal(SIGALRM, finish);
			alarm(MAXWAIT);
			return;
		}
	}
	alarm(PINGINTERVAL);
}

static int recv_pack(unsigned char *buf, int len, struct sockaddr_ll *FROM)
{
	struct timeval tv;
	struct arphdr *ah = (struct arphdr *) buf;
	unsigned char *p = (unsigned char *) (ah + 1);
	struct in_addr src_ip, dst_ip;

	gettimeofday(&tv, NULL);

	/* Filter out wild packets */
	if (FROM->sll_pkttype != PACKET_HOST &&
		FROM->sll_pkttype != PACKET_BROADCAST &&
		FROM->sll_pkttype != PACKET_MULTICAST)
		return 0;

	/* Only this type are recognised */
	if (ah->ar_op != htons(ARPOP_REPLY))
		return 0;

	/* ARPHRD check and this darned FDDI hack here :-( */
	if (ah->ar_hrd != htons(FROM->sll_hatype) &&
		(FROM->sll_hatype != ARPHRD_FDDI
		 || ah->ar_hrd != htons(ARPHRD_ETHER)))
		return 0;

	/* Protocol must be IP. */
	if (ah->ar_pro != htons(ETH_P_IP))
		return 0;
	if (ah->ar_pln != 4)
		return 0;
	if (ah->ar_hln != me.sll_halen)
		return 0;
	if (len < sizeof(*ah) + 2 * (4 + ah->ar_hln))
		return 0;
	memcpy(&src_ip, p + ah->ar_hln, 4);
	memcpy(&dst_ip, p + ah->ar_hln + 4 + ah->ar_hln, 4);
	
	/* DAD packet was:
	   src_ip = 0 (or some src)
	   src_hw = ME
	   dst_ip = tested address
	   dst_hw = <unspec>

	   We fail, if receive request/reply with:
	   src_ip = tested_address
	   src_hw != ME
	   if src_ip in request was not zero, check
	   also that it matches to dst_ip, otherwise
	   dst_ip/dst_hw do not matter.
	 */
	if (src_ip.s_addr != dst.s_addr)
		return 0;
	if (memcmp(p, &me.sll_addr, me.sll_halen) == 0)
		return 0;
	if (src.s_addr && src.s_addr != dst_ip.s_addr)
		return 0;
	
	{
		int s_printed = 0;

		TRACE(STA_INFO, "%s ",
			   FROM->sll_pkttype == PACKET_HOST ? "Unicast" : "Broadcast");
		TRACE(STA_INFO|STA_NOTAG, "%s from ",
			   ah->ar_op == htons(ARPOP_REPLY) ? "reply" : "request");
		TRACE(STA_INFO|STA_NOTAG, "%s ", inet_ntoa(src_ip));
//		TRACE(STA_INFO|STA_NOTAG, "[%s]", ether_ntoa((struct ether_addr *) p));
		TRACE(STA_INFO|STA_NOTAG, "[%02x:%02x:%02x:%02x:%02x:%02x]", *p, *(p+1), *(p+2),
			*(p+3), *(p+4), *(p+5));
		if (dst_ip.s_addr != src.s_addr) {
			TRACE(STA_INFO|STA_NOTAG, "for %s ", inet_ntoa(dst_ip));
			s_printed = 1;
		}
		if (memcmp(p + ah->ar_hln + 4, me.sll_addr, ah->ar_hln)) {
			if (!s_printed)
				TRACE(STA_INFO|STA_NOTAG, "for ");
			//TRACE("[%s]",
			//	   ether_ntoa((struct ether_addr *) p + ah->ar_hln + 4));
		}
		
		{
			long usecs = (tv.tv_sec - last.tv_sec) * 1000000 +
				tv.tv_usec - last.tv_usec;
			long msecs = (usecs + 500) / 1000;
                	
			usecs -= msecs * 1000 - 500;
			TRACE(STA_INFO|STA_NOTAG, " %ld.%03ldms\n", msecs, usecs);
		}
	}
	
	return 1;
}

//--------------------------------------------------------
// Check if this IP is duplicate using arping
// return value:
// 1  : yes, it is duplicate
// 0  : no, not duplicate
int isDuplicate(struct in_addr *ipAddr, char *device)
{
	int socket_errno;
	int ifindex = 0;
	fd_set rset;
	struct timeval tv;

	s = socket(PF_PACKET, SOCK_DGRAM, 0);
	socket_errno = errno;

	if (s < 0) {
		printf("socket error");
		exit(socket_errno);
	}

	{
		struct ifreq ifr;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, device, IFNAMSIZ - 1);
		if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
			printf("Interface %s not found", device);
			exit(2);
		}
		ifindex = ifr.ifr_ifindex;
	}
	
	dst.s_addr = ipAddr->s_addr;	

	me.sll_family = AF_PACKET;
	me.sll_ifindex = ifindex;
	me.sll_protocol = htons(ETH_P_ARP);
	if (bind(s, (struct sockaddr *) &me, sizeof(me)) == -1) {
		printf("bind error");
		exit(2);
	}

	{
		int alen = sizeof(me);

		if (getsockname(s, (struct sockaddr *) &me, &alen) == -1) {
			printf("getsockname error");
			exit(2);
		}
	}
	if (me.sll_halen == 0) {
		printf("Interface \"%s\" is not ARPable (no ll address)", device);
		exit(0);
	}
	he = me;
	memset(he.sll_addr, -1, he.sll_halen);

	TRACE(STA_INFO, "ARPING to %s", inet_ntoa(dst));
	TRACE(STA_INFO|STA_NOTAG, " from %s via %s\n", inet_ntoa(src),
		   device ? device : "unknown");
	
	count = 0;
	signal(SIGALRM, catcher);
	catcher(0);
	finished=0;
	FD_ZERO(&rset);
	FD_SET(s, &rset);

	while (!finished) {
		sigset_t sset, osset;
		char packet[4096];
		struct sockaddr_ll from;
		int alen = sizeof(from);
		int cc;
		
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		if (select(s+1, &rset, NULL, NULL, &tv) > 0) {
			if ((cc = recvfrom(s, packet, sizeof(packet), 0,
			   (struct sockaddr *) &from, &alen)) < 0) {
				perror("recvfrom");
				continue;
			}
		}
		else
			continue;
		
		sigemptyset(&sset);
		sigaddset(&sset, SIGALRM);
		sigaddset(&sset, SIGINT);
		sigprocmask(SIG_BLOCK, &sset, &osset);
		if (recv_pack(packet, cc, &from)) {
			TRACE(STA_INFO, "packet received\n");
			sigprocmask(SIG_SETMASK, &osset, NULL);
			signal(SIGALRM, SIG_DFL);
			return 1;
		}
		sigprocmask(SIG_SETMASK, &osset, NULL);
	}
	
	signal(SIGALRM, SIG_DFL);
	TRACE(STA_INFO, "not duplicate IP\n");
	return 0;
}
