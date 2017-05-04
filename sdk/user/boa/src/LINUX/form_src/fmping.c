/*
 *      Web server handler routines for Ping diagnostic stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <rtk/utility.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "../defs.h"
#include "boa.h"
#include "multilang.h"

#define PINGCOUNT	3
#define DEFDATALEN	56
#define	PINGINTERVAL	1	/* second */
#define MAXWAIT		5

static struct sockaddr_in pingaddr;
static int pingsock = -1;
static long ntransmitted = 0, nreceived = 0, nrepeats = 0;
static int myid = 0;
static int finished = 0;
static request * gwp;
static long last_time;

/* common routines */
static void pingfinal()
{
	finished = 1;
}

static void sendping()
{
	struct icmp *pkt;
	int c;
	char packet[DEFDATALEN + 8];

	pkt = (struct icmp *) packet;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

	if (c < 0 || c != sizeof(packet)) {
		boaWrite(gwp, "sendto: %s<br>", strerror(errno));
		ntransmitted--;
		finished = 1;
		printf("sock: sendto fail !");
		return;
	}
#if 0 //not to use SIGALRM in boa
	signal(SIGALRM, sendping);
	if (ntransmitted < PINGCOUNT) {	/* schedule next in 1s */
		alarm(PINGINTERVAL);
	} else {	/* done, wait for the last ping to come back */
		signal(SIGALRM, pingfinal);
		alarm(MAXWAIT);
	}
#endif
}

static void *start_ping(void *data)
{
	while(1){
		if((time_counter - last_time) >= 1){
			if(ntransmitted < PINGCOUNT){
				sendping();
				last_time = time_counter;
			}
			else{
				if((time_counter - last_time) >= MAXWAIT){
					finished = 1;
					pthread_exit(NULL);
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////
void formPing(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
	int c;
	struct hostent *h;
	struct icmp *pkt;
	struct iphdr *iphdr;
	char packet[DEFDATALEN + 8];
	int rcvdseq;
	fd_set rset;
	struct timeval tv;
	pthread_t tid;

#ifndef NO_ACTION
	int pid;
#endif

	str = boaGetVar(wp, "pingAddr", "");
	if (str[0]) {
		if ((pingsock = create_icmp_socket()) < 0) {
			perror("socket");
			snprintf(tmpBuf, 100, multilang(LANG_PING_SOCKET_CREATE_ERROR));
			goto setErr_ping;
		}

		memset(&pingaddr, 0, sizeof(struct sockaddr_in));

		pingaddr.sin_family = AF_INET;

		if ((h = gethostbyname(str)) == NULL) {
			herror("ping: ");
			snprintf(tmpBuf, 100, "ping: %s: %s", str, hstrerror(h_errno));
			goto setErr_ping;
		}

		if (h->h_addrtype != AF_INET) {
			strcpy(tmpBuf, multilang(LANG_UNKNOWN_ADDRESS_TYPE_ONLY_AF_INET_IS_CURRENTLY_SUPPORTED));
			goto setErr_ping;
		}

		memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));
		boaHeader(wp);
		boaWrite(wp, "<body><blockquote><h4>PING %s (%s): %d %s<br><br>",
		   h->h_name,
		   inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
		   DEFDATALEN,
		   Tbytes);
		printf("PING %s (%s): %d data bytes\n",
		   h->h_name,
		   inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),
		   DEFDATALEN);

		myid = getpid() & 0xFFFF;
		gwp = wp;
		ntransmitted = nreceived = nrepeats = 0;
		finished = 0;
		rcvdseq=ntransmitted-1;
		FD_ZERO(&rset);
		FD_SET(pingsock, &rset);
		/* start the ping's going ... */
		sendping();
		last_time = time_counter;
		pthread_create(&tid, NULL, start_ping, NULL);
		pthread_detach(tid);
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

			if (c < DEFDATALEN+ICMP_MINLEN)
				continue;

			iphdr = (struct iphdr *) packet;
			hlen = iphdr->ihl << 2;
			pkt = (struct icmp *) (packet + hlen);	/* skip ip hdr */
			if (pkt->icmp_id != myid) {
//				printf("not myid\n");
				continue;
			}
			if (pkt->icmp_type == ICMP_ECHOREPLY) {
				++nreceived;
				if (pkt->icmp_seq == rcvdseq) {
					// duplicate
					++nrepeats;
					--nreceived;
					dupflag = 1;
				} else {
					rcvdseq = pkt->icmp_seq;
					dupflag = 0;
					if (nreceived < PINGCOUNT)
					// reply received, send another immediately
						sendping();
				}
				boaWrite(wp, Tping_recv, c,
					   inet_ntoa(*(struct in_addr *) &from.sin_addr.s_addr),
					   pkt->icmp_seq);
				printf("%d bytes from %s: icmp_seq=%u", c,
					   inet_ntoa(*(struct in_addr *) &from.sin_addr.s_addr),
					   pkt->icmp_seq);
				if (dupflag) {
					boaWrite(wp, " (DUP!)");
					printf(" (DUP!)");
				}
				boaWrite(wp, "<br>");
				printf("\n");
			}
			if (nreceived >= PINGCOUNT)
				break;
		}
		close(pingsock);
	}


#ifndef NO_ACTION
	pid = fork();
        if (pid)
                waitpid(pid, NULL, 0);
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
                exit(1);
        }
#endif

	boaWrite(wp, "<br>%s<br>", Tping_stat);
	boaWrite(wp, Ttrans_pkt, ntransmitted);
	boaWrite(wp, Trecv_pkt, nreceived);

	printf("\n--- ping statistics ---\n");
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld packets received, ", nreceived);
	if (nrepeats) {
		boaWrite(wp, "%ld duplicates, ", nrepeats);
		printf("%ld duplicates, ", nrepeats);
	}
	boaWrite(wp, "</h4>\n");
	printf("\n");
	submitUrl = boaGetVar(wp, "submit-url", "");
	boaWrite(wp, "<form><input type=button value=\"%s\" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", Tback, submitUrl);
	boaFooter(wp);
	boaDone(wp, 200);

  	return;

setErr_ping:
	ERR_MSG(tmpBuf);
}


