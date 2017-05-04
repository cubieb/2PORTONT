#include <rtk/utility.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/signal.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

struct rcvst {
	int operStatus;
	int sent;
	int received;
	int repeats;
	int lost;
	int min;
	int max;
	int average;
};

#define		PINGINTERVAL	1	/* second */
#define 	MAXWAIT		5

// Kaohj --- cpePingOperStatus
#define	OPER_NORMAL		1
#define	OPER_PINGING		2
#define OPER_SUCCESS		3
#define OPER_FAILURE		4

static 		struct sockaddr_in pingaddr;
static 		int pingsock = -1;
static const int DEFDATALEN = 56;
long 		ntransmitted = 0, nreceived = 0, nrepeats = 0;
long		pingcount = 3;
int		myid = 0;
int		finished = 0;
static int datalen=56;
static long	tmin = LONG_MAX;	/* minimum round trip time */
static long	tmax = 0;		/* maximum round trip time */
static u_long	tsum;			/* sum of all times, for doing average */
struct rcvst rmStatus;

void update_rmstatus(struct rcvst *status);

int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
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



static void pingfinal()
{
	//printf("pingfinal\n");
	finished = 1;
}


static void sendping()
{
	struct icmp *pkt;
	int c;
	char packet[datalen + 8];
	
	//printf("sendping\n");
	
	pkt = (struct icmp *) packet;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	
	gettimeofday((struct timeval *) &packet[8], NULL);
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));
	
	if (c < 0 || c != sizeof(packet)) {		
		ntransmitted--;
		finished = 1;
		printf("sock: sendto fail !");
		return;
	}
	
	rmStatus.sent = ntransmitted;
	rmStatus.received = nreceived;
	rmStatus.repeats = nrepeats;
	rmStatus.lost = ntransmitted - nreceived;
	rmStatus.min = tmin;
	rmStatus.max = tmax;
	if ((nreceived + nrepeats) > 0)
		rmStatus.average = tsum / (nreceived + nrepeats);
	update_rmstatus(&rmStatus);
	
	signal(SIGALRM, sendping);
	if (ntransmitted < pingcount) {
		//printf("alarm(PINGINTERVAL)(1)\n");	/* schedule next in 1s */
		alarm(PINGINTERVAL);
		//printf("alarm(PINGINTERVAL)(2)\n");
	} else {	/* done, wait for the last ping to come back */
		//printf("signal(SIGALRM, pingfinal)\n");
		signal(SIGALRM, pingfinal);
		//printf("alarm(MAXWAIT)(1)\n");
		alarm(MAXWAIT);
		//printf("alarm(MAXWAIT)(2)\n");
		
	}
}

static int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		printf("cannot create raw socket\n");
	}

	/* drop root privs if running setuid */
//	setuid(getuid());

	return sock;
}

int sendPingTest(const char *host)
{
	char *submitUrl;
	char tmpBuf[100];
	int c;
	struct hostent *h;
	struct icmp *pkt;
	struct iphdr *iphdr;
	char packet[datalen + 8];
	int rcvdseq;
	fd_set rset;
	struct timeval tv, tn, *tp;
	unsigned long triptime = 0;
	
	if ((pingsock = create_icmp_socket()) < 0) {		
		printf("ping: socket create error");		
	}
	
	memset(&pingaddr, 0, sizeof(struct sockaddr_in));
	
	pingaddr.sin_family = AF_INET;
	
	if ((h = gethostbyname(host)) == NULL) {		
		printf("ping: Get HostbyName Error\n");		
	}
	
	if (h->h_addrtype != AF_INET) {
		printf("unknown address type; only AF_INET is currently supported.\n");
		
	}
	
	memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
	
	printf("PING %s (%s): %d data bytes\n",
           h->h_name,
           inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
           datalen);
        
        myid = getpid() & 0xFFFF;
        //gwp = wp;
        ntransmitted = nreceived = nrepeats = 0;
        finished = 0;
        rcvdseq=ntransmitted-1;
        FD_ZERO(&rset);
        FD_SET(pingsock, &rset);
        
        rmStatus.operStatus = OPER_PINGING;
        update_rmstatus(&rmStatus);
        /* start the ping's going ... */
        sendping();

	
	/* listen for replies */
	while (1) {
		struct sockaddr_in from;
		socklen_t fromlen = (socklen_t) sizeof(from);
		int c, hlen, dupflag;
		
		if (finished)
			break;
		
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		
		if (select(pingsock+1, &rset, NULL, NULL, &tv) > 0) {
			if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
							  (struct sockaddr *) &from, &fromlen)) < 0) {
				if (errno == EINTR)
					continue;
				
				printf("sock: recvfrom fail !");
				continue;
			}
		}
		else // timeout or error
			continue;
		
		gettimeofday(&tn, NULL);
		
		//if (c < datalen+ICMP_MINLEN)
		//	continue;
		
		iphdr = (struct iphdr *) packet;
		hlen = iphdr->ihl << 2;
		pkt = (struct icmp *) (packet + hlen);	/* skip ip hdr */
		if (pkt->icmp_id != myid) {
//			printf("not myid\n");
			continue;
		}
		if (pkt->icmp_type == ICMP_ECHOREPLY) {
			++nreceived;
			tp = (struct timeval *) pkt->icmp_data;
                	
			if ((tn.tv_usec -= tp->tv_usec) < 0) {
				--tn.tv_sec;
				tn.tv_usec += 1000000;
			}
			tn.tv_sec -= tp->tv_sec;
                	
			triptime = tn.tv_sec * 10000 + (tn.tv_usec / 100);
			tsum += triptime;
			if (triptime < tmin)
				tmin = triptime;
			if (triptime > tmax)
				tmax = triptime;
			
			if (pkt->icmp_seq == rcvdseq) {
				// duplicate
				++nrepeats;
				--nreceived;
				dupflag = 1;
			} else {
				rcvdseq = pkt->icmp_seq;
				dupflag = 0;
				// Kaohj
				//if (nreceived < pingcount)
				// reply received, send another immediately
				//	sendping();
			}
			
			printf("%d bytes from %s: icmp_seq=%u", c,
				   inet_ntoa(*(struct in_addr *) &from.sin_addr.s_addr),
				   pkt->icmp_seq);
			printf(" ttl=%d", iphdr->ttl);
			printf(" time=%lu.%lu ms", triptime / 10, triptime % 10);
			if (dupflag) {				
				printf(" (DUP!)");
			}
			
			printf("\n");
		}
		
		if (nreceived >= pingcount) {
			//printf("Success: All ICMP Ping Reply are receive\n");
			alarm(0);			
			break;
		}	
	}
	close(pingsock);	
	
	//if ( ntransmitted == nreceived )
	//	alarm(0);
	
	printf("\n--- ping statistics ---\n");
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld packets received, ", nreceived);
	if (nrepeats) {		
		printf("%ld duplicates, ", nrepeats);
	}
	if (ntransmitted)
		printf("%ld%% packet loss\n",
			   (ntransmitted - nreceived) * 100 / ntransmitted);
	if (nreceived) {
		rmStatus.operStatus = OPER_SUCCESS;
		printf("round-trip min/avg/max = %lu.%lu/%lu.%lu/%lu.%lu ms\n",
			   tmin / 10, tmin % 10,
			   (tsum / (nreceived + nrepeats)) / 10,
			   (tsum / (nreceived + nrepeats)) % 10, tmax / 10, tmax % 10);
	}
	else
		rmStatus.operStatus = OPER_FAILURE;
	
	printf("\n");
	
	rmStatus.sent = ntransmitted;
	rmStatus.received = nreceived;
	rmStatus.repeats = nrepeats;
	rmStatus.lost = ntransmitted - nreceived;
	rmStatus.min = tmin;
	rmStatus.max = tmax;
	rmStatus.average = tsum / (nreceived + nrepeats);
	update_rmstatus(&rmStatus);
	
}

// Kaohj --- update the RM ping status
void
update_rmstatus(struct rcvst *status)
{
	FILE *fp;
	
	if ((fp = fopen("/var/pingStatus.txt", "w")) == NULL)
	{
		printf("***** Open file /var/pingStatus.txt failed !\n");
	}
	else {
		fprintf(fp, "%d\n", status->operStatus);
		fprintf(fp, "%d\n", status->sent);
		fprintf(fp, "%d\n", status->received);	
		fprintf(fp, "%d\n", status->repeats);	
		//fprintf(fp, "%d\n", status->lost);
		fprintf(fp, "%d\n", status->min);
		fprintf(fp, "%d\n", status->max);
		fprintf(fp, "%d\n", status->average);
		fclose(fp);
	}
}

void
show_usage(void)
{
	(void)fprintf(stderr,
	    "usage: rmping [-c count] [-s packetsize] host\n");
	exit(2);
}

int main(int argc, char **argv)
{
	char *thisarg;
	
	datalen = DEFDATALEN; /* initialized here rather than in global scope to work around gcc bug */
	argc--;
	argv++;
	/* Parse any options */
	while (argc >= 1 && **argv == '-') {
		thisarg = *argv;
		thisarg++;
		switch (*thisarg) {
		case 'c':
			if (--argc <= 0)
			        show_usage();
			argv++;
			pingcount = atoi(*argv);
			break;
		case 's':
			if (--argc <= 0)
			        show_usage();
			argv++;
			datalen = atoi(*argv);
			break;
		default:
			show_usage();
		}
		argc--;
		argv++;
	}
	if (argc < 1)
		show_usage();
	
	// init the status file
	memset(&rmStatus, 0, sizeof(rmStatus));
	rmStatus.operStatus = OPER_NORMAL;
	update_rmstatus(&rmStatus);
	
	sendPingTest(*argv);
}
