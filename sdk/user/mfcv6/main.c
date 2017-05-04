#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/param.h>

#include <net/route.h>

#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <linux/mroute6.h>

#include <arpa/inet.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <signal.h>      // Added by Mason Yu for p2r_test

#define IVIF 1
#define OVIF 2
#define	IFNAMSIZ	16
#define PATH_MFCV6D_PID "/var/run/mfcv6d.pid"

static int raw_icmpv6_sock = -1;
static char raw_recv_buf[8192];
 // Added by Mason Yu for p2r_test
char in_if_name[IFNAMSIZ];
char out_if_name[IFNAMSIZ];
void sigterm_handler(int sig);

int raw_sock_init()
{
	struct icmp6_filter filt;

 	raw_icmpv6_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
 	if (raw_icmpv6_sock < 0) {
   		perror("socket: ");
   		return -1;
 	}

 	ICMP6_FILTER_SETBLOCKALL(&filt);
 	if (setsockopt(raw_icmpv6_sock, IPPROTO_ICMPV6, ICMP6_FILTER, &filt,
		sizeof(filt)) < 0) {
   		perror("setsockopt ICMP6_FILTER");
   		return -1;
 	}

 	return 0;
}

int mrt6_init()
{
 	int v;

 	if (raw_sock_init() < 0)
   		return -1;

 	v = 1;
 	if (setsockopt(raw_icmpv6_sock, IPPROTO_IPV6, MRT6_INIT,
                (void *)&v, sizeof(int)) < 0) {
   		perror("mrt6_init, setsockopt: ");
   		return -1;
 	}
 	return 0;
}

int mrt6_done()
{
 	int v;
 	v = 1;
 	if (setsockopt(raw_icmpv6_sock, IPPROTO_IP, MRT6_DONE,
                (void *)&v, sizeof(int)) < 0) {
   		perror("mrt6_done, setsockopt: ");
   		return -1;
 	}
 	return 0;
}

int mrt6_add_mfc(struct sockaddr_in6 src, struct sockaddr_in6 grp)
{
 	struct mf6cctl mc;

 	bzero(&mc, sizeof(mc));
 	mc.mf6cc_origin     = src;
 	mc.mf6cc_mcastgrp   = grp;
 	mc.mf6cc_parent     = IVIF;

 	IF_ZERO(&mc.mf6cc_ifset);
 	IF_SET(OVIF, &mc.mf6cc_ifset);

 	if (setsockopt(raw_icmpv6_sock, IPPROTO_IPV6, MRT6_ADD_MFC,
                (void *)&mc, sizeof(mc))) {
   		perror("mrt6_add_mfc, setsockopt: ");
   		return -1;
 	}

 	return 0;
}

 // Added by Mason Yu for p2r_test
int getPhysicalIfindexbyName(char *ifname)
{
	int ifindex = 0;
	int s;
	
	s = socket(PF_PACKET, SOCK_DGRAM, 0);	

	if (s < 0) {
		printf("getPhysicalIfindexbyName: socket error");
		exit(2);
	}

	{
		struct ifreq ifr;

		memset(&ifr, 0, sizeof(ifr));
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
		if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
			printf("Interface %s not found", ifname);
			exit(2);
		}
		ifindex = ifr.ifr_ifindex;
	}
	return ifindex;
}

int mrt6_add_mif()
{
 	struct mif6ctl mc;
 	int ifindex;
 	
 	// br0
 	ifindex = getPhysicalIfindexbyName(in_if_name);
 	printf("In's ifindex is %d. Name is %s\n", ifindex, in_if_name);
 	bzero(&mc, sizeof(mc));

 	mc.mif6c_mifi = IVIF;
 	mc.mif6c_pifi = ifindex;

 	if (setsockopt(raw_icmpv6_sock, IPPROTO_IPV6, MRT6_ADD_MIF,
                (void *)&mc, sizeof(mc))) {
   		perror("mrt6_add_mif1, setsockopt: ");
   		return -1;
 	}

 	// vc0 
 	ifindex = getPhysicalIfindexbyName(out_if_name);
 	printf("Out's ifindex is %d. Name is %s\n", ifindex, out_if_name);
 	bzero(&mc, sizeof(mc));

 	mc.mif6c_mifi = OVIF;
 	mc.mif6c_pifi = ifindex;

 	if (setsockopt(raw_icmpv6_sock, IPPROTO_IPV6, MRT6_ADD_MIF,
                (void *)&mc, sizeof(mc))) {
   		perror("mrt6_add_mif2, setsockopt: ");
   		return -1;
 	}
 	return 0;
}

 // Added by Mason Yu for p2r_test
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = PATH_MFCV6D_PID;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

 // Added by Mason Yu for p2r_test
void
sigterm_handler(int sig)
{	
	//signal(SIGTERM, sigterm_handler);
	unlink(PATH_MFCV6D_PID);
	exit(1);
}

//int main()
int main(int argc, char **argv)
{
 	char *pidfile; 
 
 	pidfile = PATH_MFCV6D_PID;
 
 	if(argc < 2) {
		printf("Usage: mfcv6d <In interface> <Out interface>\n\n");
		return;
 	}
 	strcpy(in_if_name, argv[1]);
 	strcpy(out_if_name, argv[2]);	
 
		
 	if (mrt6_init() < 0)
   		exit(1);

 	if (mrt6_add_mif() < 0)
   		exit(1);
	
	signal(SIGTERM, sigterm_handler);
 	log_pid();
	
 	/* wait for upcall */
 	while (1) {
   		int              n      = 0;
   		fd_set           rset;
   		struct mld_hdr * mh     = NULL;
   		struct mrt6msg * upcall = NULL;

   		FD_ZERO(&rset);
   		FD_SET(raw_icmpv6_sock, &rset);

   		n = select(raw_icmpv6_sock + 1, &rset, NULL, NULL, NULL);
   		if (n < 0) {
     			perror("select: ");
     			exit(1);
   		}

   		n = read(raw_icmpv6_sock, raw_recv_buf, sizeof(raw_recv_buf));
   		if (n < 0) {
     			perror("read: ");
     			exit(1);
   		}
  		mh = (struct mld_hdr *)raw_recv_buf;
   		if (mh->mld_icmp6_hdr.icmp6_type == 0) { /* kernel upcall */
     			upcall = (struct mrt6msg *)raw_recv_buf;
     			if (upcall->im6_msgtype == MRT6MSG_NOCACHE) {
       				char sbuf[64], dbuf[64];
       				struct sockaddr_in6 src, grp;
       				fprintf(stderr, "(%s,%s)\n",
               				inet_ntop(AF_INET6, &upcall->im6_src, sbuf, sizeof(sbuf)),
               				inet_ntop(AF_INET6, &upcall->im6_dst, dbuf, sizeof(dbuf)));

       				bzero(&src, sizeof(src));
       				bzero(&grp, sizeof(grp));

       				src.sin6_addr = upcall->im6_src;
       				grp.sin6_addr = upcall->im6_dst;
       				
				// Mason Yu. If the Reserved value = 0, we do not add cache.
				//printf("grp.sin6_addr.s6_addr16[0]=0x%x, value=0x%x\n", grp.sin6_addr.s6_addr16[0], (grp.sin6_addr.s6_addr16[0] & 0xff0f));
				if ( (grp.sin6_addr.s6_addr16[0] & 0xff0f) != 0xff00 ){
					//printf("Add mr cache\n");
       					mrt6_add_mfc(src, grp);
       				}
     			}
   		}

 	} 
 	unlink(pidfile);
 
 	if (mrt6_done() < 0)
   		exit(1);

 	exit(0);
}
