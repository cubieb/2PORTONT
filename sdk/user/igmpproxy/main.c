#include "igmpproxy.h"
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#ifdef SEND_GENERAL_QUERY
struct igmp_timer general_query_timer;
#endif
#ifdef SEND_UNSOLICIT_REPORT
struct igmp_timer unsolicit_report_timer;
#endif
struct igmp_timer qtimer;

int IGMPCurrentVersion = 3;
static int IGMPVersionConfig = 1;

// Kaohj -- toggle IGMP rx
int IGMP_rx_enable;
void igmp_timer_expired(void *arg);
void igmp_specific_timer_expired(void *arg);
void igmp_timer_unsolicit_report(void * arg);
int igmp_queryV3(__u32 dst, __u32 grp,__u8 mrt);
#if 0
static void pkt_debug(const char *buf)
{
int num2print = 20;
int i = 0;
	if(buf[0]==0x46)
		num2print = 24;
	for (i = 0; i < num2print; i++) {
		printf("%2.2x ", 0xff & buf[i]);
	}
	printf("\n");
	num2print = buf[3];
	for (; i < num2print; i++) {
		printf("%2.2x ", 0xff & buf[i]);
	}
	printf("\n");
}
#else
#define pkt_debug(buf)	do {} while (0)
#endif


static int  igmp_id = 0;

char igmp_down_if_name[IFNAMSIZ];
char igmp_down_if_idx;

#ifdef CONFIG_IGMPPROXY_MULTIWAN
char igmp_up_if_name[MAXWAN][IFNAMSIZ];
#else
char igmp_up_if_name[IFNAMSIZ];
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
char igmp_up_if_idx[MAXWAN];
#else
char igmp_up_if_idx;
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int igmp_up_if_num;
#endif

struct mcft_entry *mcpq = NULL;


void setIGMPVersionToV2(void)
{
    if((IGMPVersionConfig==1) && (IGMPCurrentVersion!=2))
    {
        IGMPCurrentVersion =2;
        printf("Detected IGMP V2 report, config IGMP Query to V2!\n");
    }
}

//int add_mcft(__u32 grp_addr)
struct mcft_entry * add_mcft(__u32 grp_addr, __u32 src_addr)
{
	struct mcft_entry *mcp;
#ifdef KEEP_GROUP_MEMBER
	struct mbr_entry *gcp;
#endif
	mcp = malloc(sizeof(struct mcft_entry));
	if(!mcp)
		return 0;
#ifdef KEEP_GROUP_MEMBER
	gcp = malloc(sizeof(struct mbr_entry));
	if (!gcp) {
		free(mcp);
		return 0;
	}
#endif
	mcp->grp_addr = grp_addr;
	// Kaohj -- add the first member
#ifdef KEEP_GROUP_MEMBER
	mcp->user_count = 1;
	gcp->user_addr = src_addr;
	gcp->next = NULL;
	mcp->grp_mbr = gcp;
#endif

#ifdef _SUPPORT_IGMPV3_
	mcp->igmp_ver = IGMP_VER_3;
	mcp->filter_mode = MCAST_INCLUDE;
	mcp->srclist = NULL;
	mcp->timer.lefttime = LAST_MEMBER_QUERY_INTERVAL;
	mcp->timer.retry_left = LAST_MEMBER_QUERY_COUNT;
	mcp->mrt_state = 0;
#endif /*_SUPPORT_IGMPV3_*/

	mcp->next = mcpq;
	mcpq = mcp;
	//return 0;
	return mcp;
}

// Kaohj --- add group timer for IGMP group-specific query.
int add_mcft_timer(__u32 grp_addr)
{
	struct mcft_entry **q, *p;

	/* Remove the entry from the  list. */
	for (p = mcpq; p!=0; p = p->next) {
		if(p->grp_addr == grp_addr) {
#ifdef PERIODICAL_SPECIFIC_QUERY
			p->timer.retry_left = MEMBER_QUERY_COUNT+1;
			timeout(igmp_specific_timer_expired , p, MEMBER_QUERY_INTERVAL, &p->timer.ch);
#endif
			return 0;
		}
	}
	return -1;
}

int del_mcft(__u32 grp_addr)
{
	struct mcft_entry **q, *p;
#ifdef KEEP_GROUP_MEMBER
	struct mbr_entry *gt, *gc;
#endif


	/* Remove the entry from the  list. */
	for (q = &mcpq; (p = *q); q = &p->next) {
		if(p->grp_addr == grp_addr) {
			*q = p->next;
			// Kaohj -- free member list
#ifdef KEEP_GROUP_MEMBER
			gc = p->grp_mbr;
			while (gc) {
				gt = gc->next;
				free(gc);
				gc = gt;
			}
#endif
#ifdef _SUPPORT_IGMPV3_
			{
			  struct src_entry *s, *sn;
			  s=p->srclist;
			  while(s)
			  {
			  	sn=s->next;
			  	free(s);
			  	s=sn;
			  }
			}
#endif
			untimeout(&p->timer.ch);
			free(p);
			return 0;
		}
	}
	return -1;
}

// Kaohj --- delete group timer for IGMP group-specific query.
int del_mcft_timer(__u32 grp_addr)
{
	struct mcft_entry *p;

	/* Remove the entry from the  list. */
	for (p = mcpq; p!=0; p = p->next) {
		if(p->grp_addr == grp_addr) {
			untimeout(&p->timer.ch);
			return 0;
		}
	}
	return -1;
}

int chk_mcft(__u32 grp_addr)
{
struct mcft_entry *mcp = mcpq;
	while(mcp) {
		if(mcp->grp_addr == grp_addr)
			return 1;
		mcp = mcp->next;
	}
	return 0;
}

struct mcft_entry * get_mcft(__u32 grp_addr)
{
struct mcft_entry *mcp = mcpq;
	while(mcp) {
		if(mcp->grp_addr == grp_addr)
			return mcp;
		mcp = mcp->next;
	}
	return NULL;
}

int num_mcft(void)
{
struct mcft_entry *mcp = mcpq;
int n = 0;
	while(mcp) {
		n++;
		mcp = mcp->next;
	}
	return n;
}

#ifdef KEEP_GROUP_MEMBER
// Kaohj -- attach user to group member list
//	0: fail
//	1: duplicate user
//	2: added successfully
int add_user(struct mcft_entry *mcp, __u32 src)
{
	struct mbr_entry *gcp;

	// check user
	gcp = mcp->grp_mbr;
	while (gcp) {
		if (gcp->user_addr == src)
			return 1;	// user exists
		gcp = gcp->next;
	}

	// add user
	gcp = malloc(sizeof(struct mbr_entry));
	if (!gcp) {
		return 0;
	}
	gcp->user_addr = src;
	gcp->next = mcp->grp_mbr;
	mcp->grp_mbr = gcp;
	mcp->user_count++;
	return 2;
}

// Kaohj -- remove user from group member list
// return: user count
int del_user(struct mcft_entry *mcp, __u32 src)
{
	struct mbr_entry **q, *p;

	/* Remove the entry from the  list. */
	q = &mcp->grp_mbr;
	p = *q;
	while (p) {
		if(p->user_addr == src) {
			*q = p->next;
			free(p);
			mcp->user_count--;
			return mcp->user_count;
		}
		q = &p->next;
		p = p->next;
	}

	return mcp->user_count;
}
#endif

/*
 * u_short in_cksum(u_short *addr, int len)
 *
 * Compute the inet checksum
 */
unsigned short in_cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    answer = ~sum;
    return (answer);
}

fd_set in_fds;		/* set of fds that wait_input waits for */
int max_in_fd;		/* highest fd set in in_fds */

/*
 * add_fd - add an fd to the set that wait_input waits for.
 */
void add_fd(int fd)
{
    FD_SET(fd, &in_fds);
    if (fd > max_in_fd)
	max_in_fd = fd;
}

/*
 * remove_fd - remove an fd from the set that wait_input waits for.
 */
void remove_fd(int fd)
{
    FD_CLR(fd, &in_fds);
}

/////////////////////////////////////////////////////////////////////////////
//	22/04/2004, Casey
/*
	Modified the following items:
	1.	delete all muticast router functions, xDSL router never use such function
	2.	igmp_handler only accept message for IGMP PROXY
	3.	IGMP proxy keep track on multicast address by mcft table,
		not multicast router module.

	igmp_handler rule:
	1.	only accept IGMP query from upstream interface, and it trigger
		downstream interface to send IGMP query.
	2.	only accept IGMP report from downstream interface, and it trigger
		upstream interface to send IGMP report.
	3.	when received IGMP report, recorded its group address as forwarding rule.
	4.	only accept IGMP leave from downstream interface, downstream interface
		will send IGMP general query twice to make sure there is no other member.
		If it cannot find any member, upstream interface will send IGMP leave.

	forwarding rule:
	1.	system only forward multicast packets from upstream interface to downstream interface.
	2.	system only forward multicast packets which group address learned by IGMP report.

*/
/////////////////////////////////////////////////////////////////////////////
//



#define RECV_BUF_SIZE	2048
char *recv_buf, *send_buf;
int igmp_socket;	/* down */
int igmp_socket2;	/* up */

int igmp_inf_create(char *ifname)
{
	struct ip_mreq mreq;
	int i;
	int ret;
	struct IfDesc *dp;


	dp = getIfByName(ifname);
	if(dp==NULL)
		return 0;

    	if ((dp->sock = socket(AF_INET, SOCK_RAW, IPPROTO_IGMP)) < 0)
		log(LOG_ERR, errno, "IGMP socket");

//#ifdef _SUPPORT_IGMPV3_
{
	/* Set router alert */
	char ra[4];
	ra[0] = 148;
	ra[1] = 4;
	ra[2] = 0;
	ra[3] = 0;
	setsockopt(dp->sock, IPPROTO_IP, IP_OPTIONS, ra, 4);
}
//#endif /*_SUPPORT_IGMPV3_*/

	/* init igmp */
	/* Set reuseaddr, ttl, loopback and set outgoing interface */
	i = 1;
	ret = setsockopt(dp->sock, SOL_SOCKET, SO_REUSEADDR, (void*)&i, sizeof(i));
	if(ret)
		printf("setsockopt SO_REUSEADDR error!\n");
	i = 1;
	ret = setsockopt(dp->sock, IPPROTO_IP, IP_MULTICAST_TTL,
		(void*)&i, sizeof(i));
	if(ret)
		printf("setsockopt IP_MULTICAST_TTL error!\n");

	//eddie disable LOOP
	i = 0;
	ret = setsockopt(dp->sock, IPPROTO_IP, IP_MULTICAST_LOOP,
		(void*)&i, sizeof(i));
	if(ret)
		printf("setsockopt IP_MULTICAST_LOOP error!\n");
	ret = setsockopt(dp->sock, IPPROTO_IP, IP_MULTICAST_IF,
		(void*)&dp->InAdr, sizeof(struct in_addr));

	if(ret)
		printf("setsockopt IP_MULTICAST_IF error!\n");

	/* In linux use IP_PKTINFO */
	//IP_RECVIF returns the interface of received datagram
	i = 1;
	ret = setsockopt(dp->sock, IPPROTO_IP, IP_PKTINFO, &i, sizeof(i));
	if(ret)
		printf("setsockopt IP_PKTINFO error!\n");

	//ret = fcntl(dp->sock, F_SETFL, O_NONBLOCK);
	//if(ret)
	//	printf("fcntl O_NONBLOCK error!\n");

	return 0;

}

int init_igmp(void)
{
int val;
    recv_buf = malloc(RECV_BUF_SIZE);
    send_buf = malloc(RECV_BUF_SIZE);

    FD_ZERO(&in_fds);
    max_in_fd = 0;

	igmp_inf_create(igmp_down_if_name);

#ifdef CONFIG_IGMPPROXY_MULTIWAN
         int idx;
        for(idx=0;idx<igmp_up_if_num;idx++)
	    igmp_inf_create(igmp_up_if_name[idx]);

#else
      igmp_inf_create(igmp_up_if_name);
#endif

#ifdef SEND_GENERAL_QUERY
	// Kaohj -- start general IGMP Query peridically
	general_query_timer.type = TIMER_GENERAL_QUERY;
	timeout(igmp_timer_expired , &general_query_timer, GENERAL_QUERY_INTERVAL, &general_query_timer.ch);
#endif

#ifdef SEND_UNSOLICIT_REPORT
  unsolicit_report_timer.type = TIMER_UNSOLT_REPORT;
  timeout(igmp_timer_unsolicit_report, &unsolicit_report_timer,
  																					UNSOLT_REPORT_INTERVAL, &unsolicit_report_timer.ch);
#endif
	// Kaohj --- enable IGMP rx
	IGMP_rx_enable = 1;
	return 0;
}

void shut_igmp_proxy(void)
{
	/* all interface leave multicast group */
}
#ifdef CONFIG_IGMPPROXY_MULTIWAN
// Kaohj -- add multicast membership to upstream interface(s)
int add_membership(__u32 group)
{
struct ip_mreq mreq;
struct IfDesc *up_dp ;
int index;
int ret;

	for(index=0;index<igmp_up_if_num;index++)
	{
		up_dp= getIfByName(igmp_up_if_name[index]);

		if(up_dp==NULL)
			continue;

		/* join multicast group */
		mreq.imr_multiaddr.s_addr = group;
		mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
		ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
		if(ret) {
			//fprintf(stderr,"setsockopt IP_ADD_MEMBERSHIP %s error! up_dp=%s, addr=%s\n", inet_ntoa(mreq.imr_multiaddr),up_dp, inet_ntoa( mreq.imr_interface.s_addr));
			return ret;
		}
	}
}

// Add MRoute
int add_mfc(__u32 group, __u32 src)
{
	struct MRouteDesc	mrd;
	int index;
	struct IfDesc *up_dp ;

	for(index=0;index<igmp_up_if_num;index++) {
		up_dp= getIfByName(igmp_up_if_name[index]);

		if(up_dp==NULL)
			continue;
		/* add multicast routing entry */
		mrd.OriginAdr.s_addr = 0;
		// Kaohj --- special case, save the subscriber IP to kernel
		// in order to take the subscriber IP (source IP) to the upstream server
		mrd.SubsAdr.s_addr = src;
		mrd.McAdr.s_addr = group;
		mrd.InVif = igmp_up_if_idx[index];
		memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
		mrd.TtlVc[igmp_down_if_idx] = 1;
		addMRoute(&mrd);
	}
	return 1;
}

// Add group membershipt and MRoute
int add_mr(__u32 group, __u32 src)
{
	add_membership(group);
	add_mfc(group, src);
	return 1;
}

// Kaohj -- delete multicast membership to upstream interface(s)
int del_membership(__u32 group)
{
struct ip_mreq mreq;
struct IfDesc *up_dp ;
int ret;
int index;

	for(index=0;index<igmp_up_if_num;index++){
		up_dp= getIfByName(igmp_up_if_name[index]);
		if(up_dp==NULL)
			continue;
		/* drop multicast group */
		mreq.imr_multiaddr.s_addr = group;
		mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
		ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
		if(ret)
			printf("setsockopt IP_DROP_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
	}
}

// Delete MRoute
int del_mfc(__u32 group)
{
	struct MRouteDesc	mrd;
	int index;

	for(index=0;index<igmp_up_if_num;index++) {
		/* delete multicast routing entry */
		mrd.OriginAdr.s_addr = 0;
		mrd.McAdr.s_addr = group;
		mrd.InVif = igmp_up_if_idx[index];
		memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
		delMRoute(&mrd);
	}
	return 1;
}

// Delete group membership and MRoute
int del_mr(__u32 group)
{
	del_membership(group);
	del_mfc(group);
	return 1;
}
#else

// Kaohj -- add multicast membership to upstream interface(s)
int add_membership(__u32 group)
{
struct ip_mreq mreq;
struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
int ret;

	if(up_dp==NULL)
		return 0;
	/* join multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret) {
		//printf("setsockopt IP_ADD_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
		return ret;
	}

	syslog(LOG_INFO, "igmpproxy: Add membership %s", inet_ntoa(mreq.imr_multiaddr));
	return 1;
}

// Add MRoute
int add_mfc(__u32 group, __u32 src)
{
	struct MRouteDesc	mrd;

	/* add multicast routing entry */
	mrd.OriginAdr.s_addr = 0;
	// Kaohj --- special case, save the subscriber IP to kernel
	// in order to take the subscriber IP (source IP) to the upstream server
	mrd.SubsAdr.s_addr = src;
	mrd.McAdr.s_addr = group;
	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
	mrd.TtlVc[igmp_down_if_idx] = 1;
	addMRoute(&mrd);
	return 1;
}

// Add group membershipt and MRoute
int add_mr(__u32 group, __u32 src)
{
	add_membership(group);
	add_mfc(group, src);
	return 1;
}

// Kaohj -- delete multicast membership to upstream interface(s)
int del_membership(__u32 group)
{
struct ip_mreq mreq;
struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
int ret;

	if(up_dp==NULL)
		return 0;
	/* drop multicast group */
	mreq.imr_multiaddr.s_addr = group;
	mreq.imr_interface.s_addr = up_dp->InAdr.s_addr;
	ret = setsockopt(up_dp->sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret)
		printf("setsockopt IP_DROP_MEMBERSHIP %s error!\n", inet_ntoa(mreq.imr_multiaddr));
	syslog(LOG_INFO, "igmpproxy: Drop membership %s", inet_ntoa(mreq.imr_multiaddr));
	return 1;
}

// Delete MRoute
int del_mfc(__u32 group)
{
	struct MRouteDesc	mrd;

	/* delete multicast routing entry */
	mrd.OriginAdr.s_addr = 0;
	mrd.McAdr.s_addr = group;
	mrd.InVif = igmp_up_if_idx;
	memset(mrd.TtlVc, 0, sizeof(mrd.TtlVc));
	delMRoute(&mrd);
	return 1;
}

// Delete group membership and MRoute
int del_mr(__u32 group)
{
	del_membership(group);
	del_mfc(group);
	return 1;
}
#endif

void igmp_specific_timer_expired(void *arg)
{
struct mcft_entry *mcp = arg;
	int query_interval;

	//printf("igmp_specific_timer_expired()\n");
	if(!mcp)
		return;

	mcp->timer.retry_left--;

	if(mcp->timer.retry_left <= 0) {
		// Kaohj --- check if group has already been dropped
#ifdef KEEP_GROUP_MEMBER
		if (mcp->user_count != 0) {
#endif
			del_mr(mcp->grp_addr);
			del_mcft(mcp->grp_addr);
#ifdef KEEP_GROUP_MEMBER
		}
#endif
		untimeout(&mcp->timer.ch);
	}
	else {
		//printf( "<t>\n" );
		// Kaohj --- last mmber ?
		if (mcp->user_count==0) {
			query_interval = LAST_MEMBER_QUERY_INTERVAL;
		}
		else {
			query_interval = LAST_MEMBER_QUERY_INTERVAL*3;
		}
		timeout(igmp_specific_timer_expired , mcp, query_interval, &mcp->timer.ch);
        if(IGMPCurrentVersion==3)
		    igmp_queryV3(ALL_SYSTEMS, mcp->grp_addr, query_interval);
        else
		    igmp_query(ALL_SYSTEMS, mcp->grp_addr, query_interval);
	}
}

// Kaohj --- send general Query
void igmp_timer_expired(void *arg)
{
	struct igmp_timer *pqtimer;

	pqtimer = (struct igmp_timer *)arg;
	if (pqtimer->type == TIMER_GENERAL_QUERY) {
		timeout(igmp_timer_expired, pqtimer, GENERAL_QUERY_INTERVAL, &pqtimer->ch);
	}
	else if (pqtimer->type == TIMER_DELAY_QUERY) {
	pqtimer->retry_left--;
	if (pqtimer->retry_left <= 0)
		untimeout(&pqtimer->ch);
	else
			timeout(igmp_timer_expired, pqtimer, 3, &pqtimer->ch);
	}

   if(IGMPCurrentVersion==3)
       igmp_queryV3(ALL_SYSTEMS, 0, 1);
   else
	igmp_query(ALL_SYSTEMS, 0, 1);
}

void igmp_timer_unsolicit_report(void *arg)
{
	struct igmp_timer *ptimer;
	struct mcft_entry *mcp = mcpq;

	ptimer = (struct igmp_timer *)arg;

	ptimer->unsol_report_count++;

	timeout(igmp_timer_unsolicit_report, ptimer, UNSOLT_REPORT_INTERVAL, &ptimer->ch);

	while(mcp)
	{
		//printf("the mcp->grp_addr is 0x%x\n", mcp->grp_addr);
		igmp_report(mcp->grp_addr, 0);
		mcp = mcp->next;
	}
}

int add_group_and_src( __u32 group, __u32 src )
{
	struct mcft_entry *mymcp;

	if(!IN_MULTICAST(group))
		return 0;
	/* check if it's protocol reserved group */
	if((group&0xFFFFFF00)==0xE0000000)
		return 0;
	/* TBD */
	/* should check if it's from downtream interface */

	if(!chk_mcft(group)) {
		// Group does not exist on router, add multicast address into if_table
	//	struct IfDesc *up_dp = getIfByName(igmp_up_if_name);
		int ret;

		mymcp = add_mcft(group, src);
		if(!mymcp) {
			//printf("igmp_accept> add group to list fail!\n");
			return 0;
		}
		// Kaohj
		//add_mr(group);
		add_mr(group, src);
#ifdef PERIODICAL_SPECIFIC_QUERY
		mymcp->timer.retry_left = MEMBER_QUERY_COUNT+1;
		timeout(igmp_specific_timer_expired , mymcp, MEMBER_QUERY_INTERVAL, &mymcp->timer.ch);
#endif
	}
	else {
		mymcp = get_mcft(group);
		if (mymcp)
		{
			untimeout(&mymcp->timer.ch);
#ifdef KEEP_GROUP_MEMBER
			add_user(mymcp, src);
#endif
#ifdef PERIODICAL_SPECIFIC_QUERY
			mymcp->timer.retry_left = MEMBER_QUERY_COUNT+1;
			timeout(igmp_specific_timer_expired , mymcp, MEMBER_QUERY_INTERVAL, &mymcp->timer.ch);
#endif
		}
	}


	return 0;
}

int del_group_and_src( __u32 group, __u32 src )
{
	struct mcft_entry *mymcp;
	int query_count, query_interval;
#ifdef KEEP_GROUP_MEMBER
	int count;
#endif

	if(!IN_MULTICAST(group)) {
		//printf("igmp_accept> invalid multicast address or IGMP leave\n");
		return 0;
	}
	//printf("igmp_accept> receive IGMP Leave from %s,", inet_ntoa(ip->saddr));
	//printf("group = %s\n", inet_ntoa(igmp->group));

	/* TBD */
	/* should check if it's from downtream interface */
	if(chk_mcft(group)) {
		mymcp = get_mcft(group);
		// Group does exist on router
		if(mymcp) {
			query_count = LAST_MEMBER_QUERY_COUNT;
			query_interval = LAST_MEMBER_QUERY_INTERVAL;
#ifdef KEEP_GROUP_MEMBER
			count = del_user(mymcp, src);
			if (count == 0) {// no member, drop it!
				del_mr(mymcp->grp_addr);
				del_mcft(mymcp->grp_addr);
				query_count = LAST_MEMBER_QUERY_COUNT;
				query_interval = LAST_MEMBER_QUERY_INTERVAL;
			}
			else {
#ifdef PERIODICAL_SPECIFIC_QUERY
				query_count = MEMBER_QUERY_COUNT;
				query_interval = LAST_MEMBER_QUERY_INTERVAL*3;
#else
				query_count = LAST_MEMBER_QUERY_COUNT;
				query_interval = LAST_MEMBER_QUERY_INTERVAL*3;
#endif
			}
#endif
			mymcp->timer.retry_left = query_count;
#ifdef LEAVE_SPECIFIC_QUERY
			timeout(igmp_specific_timer_expired , mymcp, query_interval, &mymcp->timer.ch);
            if(IGMPCurrentVersion==3)
			    igmp_queryV3(ALL_SYSTEMS, mymcp->grp_addr, query_interval);
            else
			    igmp_query(ALL_SYSTEMS, mymcp->grp_addr, query_interval);
#endif
		}
	}

	return 0;
}

/*
 * igmp_accept - handles the incoming IGMP packets
 *
 */

#define FROM_LAN 0
#define FROM_WAN 1

#ifdef NEW_IGMPPROXY_RCV
int igmp_accept(int recvlen, int flag)
#else
int igmp_accept(int recvlen, struct IfDesc *dp)
#endif
{
	register __u32 src, dst, group, group_src;
	struct iphdr *ip;
	struct igmphdr *igmp;
	int ipdatalen, iphdrlen, igmpdatalen;
	struct mcft_entry *mymcp;
	struct igmpmsg *msg;

	if (recvlen < sizeof(struct iphdr)) {
		log(LOG_WARNING, 0,
		    "received packet too short (%u bytes) for IP header", recvlen);
		return 0;
	}

	ip  = (struct iphdr *)recv_buf;
	src = ip->saddr;
	dst = ip->daddr;

	if(!IN_MULTICAST(dst))	/* It isn't a multicast */
		return -1;
	if(chk_local(src)) 		/* It's our report looped back */
		return -1;
	if(dst == ALL_PRINTER)	/* It's MS-Windows UPNP all printers notify */
		return -1;

	pkt_debug(recv_buf);

	iphdrlen  = ip->ihl << 2;
	ipdatalen = ip->tot_len;

	igmp        = (struct igmphdr *)(recv_buf + iphdrlen);
	group   = igmp->group;

	/* determine message type */
	//printf("igmp_accept> receive IGMP type [%x] from %s:", igmp->type, inet_ntoa(ip->saddr));
	//printf("%s\n", inet_ntoa(ip->daddr));
	switch (igmp->type) {
		case IGMP_HOST_MEMBERSHIP_QUERY:
			/* Linux Kernel will process local member query, it wont reach here */
			#if 0
			// send General Query downstream
			igmp_query(ALL_SYSTEMS, group, 1);
			#endif
			break;

		case IGMP_HOST_MEMBERSHIP_REPORT:
		case IGMPV2_HOST_MEMBERSHIP_REPORT:

#ifdef NEW_IGMPPROXY_RCV
			if(flag == FROM_WAN)
				break;
#endif
			add_group_and_src( group, src );
			break;

 		case IGMP_HOST_V3_MEMBERSHIP_REPORT:
			/* TBD */
			/* should check if it's from downtream interface */
#if 0
{
			group = *(__u32 *)((char *)igmp+12);
			//igmp_query(ALL_SYSTEMS, group, 1);
			igmp_query(ALL_SYSTEMS, 0, 1);
			break;
}
#else
{
			struct igmpv3_report *igmpv3;
			struct igmpv3_grec *igmpv3grec;
			__u16 rec_id;

			igmpv3 = (struct igmpv3_report *)igmp;
			//printf( "recv IGMP_HOST_V3_MEMBERSHIP_REPORT\n" );
			//printf( "igmpv3->type:0x%x\n", igmpv3->type );
			//printf( "igmpv3->ngrec:0x%x\n", ntohs(igmpv3->ngrec) );

			rec_id=0;
			igmpv3grec =  &igmpv3->grec[0];
			while( rec_id < ntohs(igmpv3->ngrec) )
			{

				//printf( "igmpv3grec[%d]->grec_type:0x%x\n", rec_id, igmpv3grec->grec_type );
				//printf( "igmpv3grec[%d]->grec_auxwords:0x%x\n", rec_id, igmpv3grec->grec_auxwords );
				//printf( "igmpv3grec[%d]->grec_nsrcs:0x%x\n", rec_id, ntohs(igmpv3grec->grec_nsrcs) );
				//printf( "igmpv3grec[%d]->grec_mca:%s\n", rec_id, inet_ntoa(igmpv3grec->grec_mca) );

				group = igmpv3grec->grec_mca;

				switch( igmpv3grec->grec_type )
				{
				case IGMPV3_MODE_IS_INCLUDE:
				case IGMPV3_MODE_IS_EXCLUDE:
					if(chk_mcft(group))
					{
						//printf( "IS_IN or IN_EX\n" );
						add_group_and_src( group, src );
					}
					break;
				case IGMPV3_CHANGE_TO_INCLUDE:
					//printf( "TO_IN\n" );
					if( igmpv3grec->grec_nsrcs )
						add_group_and_src( group, src );
					else //empty
						del_group_and_src( group, src );
					break;
				case IGMPV3_CHANGE_TO_EXCLUDE:
					//printf( "TO_EX\n" );
					add_group_and_src( group, src );
					break;
				case IGMPV3_ALLOW_NEW_SOURCES:
					//printf( "ALLOW\n" );
					break;
				case IGMPV3_BLOCK_OLD_SOURCES:
					//printf( "BLOCK\n" );
					break;
				default:
					//printf( "!!! can't handle the group record types: %d\n", igmpv3grec->grec_type );
					break;
				}

				rec_id++;
				//printf( "count next: 0x%x %d %d %d %d\n", igmpv3grec, sizeof( struct igmpv3_grec ), igmpv3grec->grec_auxwords, ntohs(igmpv3grec->grec_nsrcs), sizeof( __u32 ) );
				igmpv3grec = (struct igmpv3_grec *)( (char*)igmpv3grec + sizeof( struct igmpv3_grec ) + (igmpv3grec->grec_auxwords+ntohs(igmpv3grec->grec_nsrcs))*sizeof( __u32 ) );
				//printf( "count result: 0x%x\n", igmpv3grec );
			}
			break;
}
#endif
			break;

		case IGMP_HOST_LEAVE_MESSAGE :
			del_group_and_src( group, src );
			break;
		// Kaohj
		case IGMPMSG_NOCACHE: // ipmr_cache_report (no route for incomming group)
			msg = (struct igmpmsg*)recv_buf;
			// The multicast stream may be dead, drop it!
			if (msg->im_vif != igmp_down_if_idx) // if not from downstream, send leave
				igmp_leave(dst, 0);
			break;

		default:
			//printf("igmp_accept> receive IGMP Unknown type [%x] from %s:", igmp->type, inet_ntoa(ip->saddr));
			//printf("%s\n", inet_ntoa(ip->daddr));
			break;
    }
    return 0;
}


/*
 * igmp_report - send an IGMP Report packet, directly to linkp->send(), not via ip
 *
 * int igmp_report( longword ina, int ifno )
 * Where:
 *	ina	the group address to report.
 *      ifno	interface number
 *
 * Returns:
 *	0	if unable to send report
 *	1	report was sent successfully
 */

int igmp_report(__u32 dst, int if_idx)
{
    struct iphdr *ip;
    struct igmphdr *igmp;
    struct sockaddr_in sdst;
    struct IfDesc *dp;
    int index;

    int if_index;

    ip                      = (struct iphdr *)send_buf;
    ip->saddr       = getAddrByVifIx(if_idx);
    ip->daddr       = dst;
    ip->tot_len              = MIN_IP_HEADER_LEN + IGMP_MINLEN;
    if (IN_MULTICAST(ntohl(dst))) {
		ip->ttl = 1;
		#if 0
	    //if (setsockopt(igmp_socket, IPPROTO_IP, IP_MULTICAST_IF,
	    if (setsockopt(dp->sock, IPPROTO_IP, IP_MULTICAST_IF,
		   	(char *)&ip->saddr, sizeof(struct in_addr)) < 0)
		   	printf("igmp_report> set multicast interface error\n");
		#endif

	}
    else
		ip->ttl = MAXTTL;

    igmp                    = (struct igmphdr *)(send_buf + MIN_IP_HEADER_LEN);
    igmp->type         = IGMPV2_HOST_MEMBERSHIP_REPORT;
    igmp->code         = 0;
    igmp->group		   = dst;
    igmp->csum        = 0;
    igmp->csum        = in_cksum((u_short *)igmp, IGMP_MINLEN);

    bzero(&sdst, sizeof(sdst));
    sdst.sin_family = AF_INET;
    sdst.sin_addr.s_addr = dst;
    //syslog(LOG_INFO, "igmpproxy: send report to %s: %s\n", igmp_up_if_name, inet_ntoa(dst));
    //if (sendto(igmp_socket, send_buf,
    // Kaohj
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		for(index=0;index<igmp_up_if_num;index++)
		{
			dp= getIfByName(igmp_up_if_name[index]);

			if(dp==NULL)
				continue;

			ip->saddr = dp->InAdr.s_addr;

			if (sendto(dp->sock, igmp, IGMP_MINLEN, 0,
				(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
				printf("igmp_report> sendto error, from %s ", inet_ntoa(ip->saddr));
				printf("to %s\n", inet_ntoa(ip->daddr));
			}
		}
#else
		// Kaohj
		dp = getIfByName(igmp_up_if_name);
		if (!dp)
			return 0;
    if (sendto(dp->sock, igmp,
			IGMP_MINLEN, 0,
			(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
		printf("igmp_report> sendto error, from %s ", inet_ntoa(ip->saddr));
		printf("to %s\n", inet_ntoa(ip->daddr));
    }
#endif

    return 1;
}




/*
 * igmp_query - send an IGMP Query packet to downstream interface
 *
 * int igmp_query(__u32 dst, __u32 grp,__u8 mrt)
 * Where:
 *  dst		destination address
 *  grp		query group address
 *  MRT		Max Response Time in IGMP header (in 1/10 second unit)
 *
 * Returns:
 *	0	if unable to send
 *	1	packet was sent successfully
 */

int igmp_query(__u32 dst, __u32 grp,__u8 mrt)
{
    struct iphdr *ip;
    struct igmphdr *igmp;
    struct sockaddr_in sdst;
	struct IfDesc *dp = getIfByName(igmp_down_if_name);

    ip              = (struct iphdr *)send_buf;
    ip->saddr       = dp->InAdr.s_addr;
    ip->daddr       = dst;
    ip->tot_len              = MIN_IP_HEADER_LEN + IGMP_MINLEN;
	ip->ttl = 1;

    igmp               = (struct igmphdr *)(send_buf + MIN_IP_HEADER_LEN);
   	igmp->type         = 0x11;
    igmp->code         = mrt;
    igmp->group 	   = grp;
    igmp->csum        = 0;
    igmp->csum        = in_cksum((u_short *)igmp, IGMP_MINLEN);

    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    sdst.sin_addr.s_addr = dst;

    //syslog(LOG_INFO, "igmpproxy: send igmp query\n");
    if (sendto(dp->sock, igmp, IGMP_MINLEN, 0,
			(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
		printf("igmp_query> sendto error, from %s ", inet_ntoa(ip->saddr));
		printf("to %s\n", inet_ntoa(ip->daddr));
    }

    return 0;
}

int igmp_queryV3(__u32 dst, __u32 grp,__u8 mrt)
{
    struct igmpv3_query	*igmpv3;
    struct sockaddr_in	sdst;
    struct IfDesc 	*dp = getIfByName(igmp_down_if_name);
    int		i,totalsize=0;

    igmpv3            = (struct igmpv3_query *)send_buf;
    igmpv3->type      = 0x11;
    igmpv3->code      = mrt;
    igmpv3->csum      = 0;
    igmpv3->group     = grp;
    igmpv3->resv      = 0;
    igmpv3->suppress  = 1;
    igmpv3->qrv       = 2;
    igmpv3->qqic      = MEMBER_QUERY_INTERVAL;
    igmpv3->nsrcs     = 0;

    totalsize	      = sizeof(struct igmpv3_query);
    igmpv3->csum      = in_cksum((u_short *)igmpv3, totalsize );

    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    if(grp)
    	sdst.sin_addr.s_addr = grp;
    else
    	sdst.sin_addr.s_addr = ALL_SYSTEMS;

    if (sendto(dp->sock, igmpv3, totalsize, 0, (struct sockaddr *)&sdst, sizeof(sdst)) < 0)
    {
	printf("igmpv3_query> sendto error, from %s ", inet_ntoa(dp->InAdr.s_addr));
	printf("to %s\n", inet_ntoa(sdst.sin_addr.s_addr));
    }

    return 0;
}

/*
 * igmp_leave - send an IGMP LEAVE packet, directly to linkp->send(), not via ip
 *
 * int igmp_leave( longword ina, int ifno )
 * Where:
 *  	ina	the IP address to leave
 *  	ifno	interface number
 *
 * Returns:
 *	0	if unable to send leave
 *	1	report was sent successfully
 */

int igmp_leave(__u32 grp, int if_idx)
{
    struct iphdr *ip;
    struct igmphdr *igmp;
    struct sockaddr_in sdst;
		struct IfDesc *dp;
		int index;

    ip              = (struct iphdr *)send_buf;
    ip->daddr       = ALL_ROUTERS;
    ip->tot_len              = MIN_IP_HEADER_LEN + IGMP_MINLEN;
	ip->ttl = 1;

    igmp               = (struct igmphdr *)(send_buf + MIN_IP_HEADER_LEN);
   	igmp->type         = 0x17;
    igmp->code         = 0;
    igmp->group 	   = grp;
    igmp->csum        = 0;
    igmp->csum        = in_cksum((u_short *)igmp, IGMP_MINLEN);

    bzero(&sdst, sizeof(struct sockaddr_in));
    sdst.sin_family = AF_INET;
    sdst.sin_addr.s_addr = ALL_ROUTERS;
    //printf("send igmp leave\n");
    //syslog(LOG_INFO, "igmpproxy: send leave to %s: %s\n", igmp_up_if_name, inet_ntoa(grp));

    // Kaohj
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		for(index=0;index<igmp_up_if_num;index++)
		{
			dp= getIfByName(igmp_up_if_name[index]);

			if(dp==NULL)
				continue;
			ip->saddr       = dp->InAdr.s_addr;
			if (sendto(dp->sock, igmp, IGMP_MINLEN, 0,
					(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
				printf("igmp_leave> sendto error, from %s ", inet_ntoa(ip->saddr));
				printf("to %s\n", inet_ntoa(ip->daddr));
			}
    }
#else
		// Kaohj
		dp = getIfByName(igmp_up_if_name);
		ip->saddr       = dp->InAdr.s_addr;
    if (sendto(dp->sock, igmp, IGMP_MINLEN, 0,
			(struct sockaddr *)&sdst, sizeof(sdst)) < 0) {
		printf("igmp_leave> sendto error, from %s ", inet_ntoa(ip->saddr));
		printf("to %s\n", inet_ntoa(ip->daddr));
    }
#endif

    return 0;
}






////////////////////////////////////////////////////////////////////////////////////


char* runPath = "/bin/igmpproxy";
char* pidfile = "/var/run/igmp_pid";

#if 0
static void clean(void)
/*
** Cleans up, i.e. releases allocated resources. Called via atexit().
**
*/
{
  log( LOG_DEBUG, 0, "clean handler called" );
  disableMRouter();

  unlink(pidfile);
  exit(EXIT_SUCCESS);
}
#endif

/*
 * On hangup, let everyone know we're going away.
 */

void hup(int signum)
{
	(void)signum;

  log( LOG_DEBUG, 0, "clean handler called" );
  disableMRouter();

  unlink(pidfile);
  exit(EXIT_SUCCESS);

}

// Kaohj added
// Comes here because upstream or downstream interface ip changed
// Usually, it is used by dynamic interface to sync its interface with
// the igmpproxy local database.
void sigifup(int signum)
{
	(void)signum;
	struct IfDesc *Dup, *Ddp;
	int Sock;

	syslog(LOG_INFO, "igmpproxy: SIGUSR1 caught\n");
	updateIfVc();
#ifdef CONFIG_IGMPPROXY_MULTIWAN
        int index;

	Ddp = getIfByName(igmp_down_if_name);
	if ( Ddp == NULL)
		return;

	// update upstream/downstream interface ip into local database
	for(index = 0;index<igmp_up_if_num;index++)
	{
		Dup = getIfByName(igmp_up_if_name[index]);
		if ( Dup == NULL)
			continue;
		if (Dup->InAdr.s_addr != 0)
			igmp_up_if_idx[index] = addVIF(Dup);
		//printf("update upstream ip to %s\n", inet_ntoa(Dup->InAdr));
		// Update default multicast interface for this socket.
		setsockopt(Dup->sock, IPPROTO_IP, IP_MULTICAST_IF,
			(void*)&Dup->InAdr, sizeof(struct in_addr));
	}

	//printf("update downstream ip to %s\n", inet_ntoa(Ddp->InAdr));
	// Update default multicast interface for this socket.
	setsockopt(Ddp->sock, IPPROTO_IP, IP_MULTICAST_IF,
		(void*)&Ddp->InAdr, sizeof(struct in_addr));

#else

	// get descriptors of upstream and downstream interfaces
	Dup = getIfByName(igmp_up_if_name);
	Ddp = getIfByName(igmp_down_if_name);
	if (Dup == NULL || Ddp == NULL)
		return;

	// update upstream/downstream interface ip into local database
	//printf("update upstream ip to %s\n", inet_ntoa(Dup->InAdr));
	// Update default multicast interface for this socket.
	setsockopt(Dup->sock, IPPROTO_IP, IP_MULTICAST_IF,
		(void*)&Dup->InAdr, sizeof(struct in_addr));
	//printf("update downstream ip to %s\n", inet_ntoa(Ddp->InAdr));
	// Update default multicast interface for this socket.
	setsockopt(Ddp->sock, IPPROTO_IP, IP_MULTICAST_IF,
		(void*)&Ddp->InAdr, sizeof(struct in_addr));
#endif
}

#include <rtk/adslif.h>
#if defined(CONFIG_DSL_ON_SLAVE)
static char adslDevice[] = "/dev/xdsl_ipc";
#else
static char adslDevice[] = "/dev/adsl0";
#endif

static FILE* adslFp = NULL;
static char open_adsl_drv(void)
{
	if ((adslFp = fopen(adslDevice, "r")) == NULL) {
		printf("ERROR: failed to open %s, error(%s)\n",adslDevice, strerror(errno));
		return 0;
	};
	return 1;
}

static void close_adsl_drv(void)
{
	if(adslFp)
		fclose(adslFp);

	adslFp = NULL;
}

char adsl_drv_get(unsigned int id, void *rValue, unsigned int len)
{
#ifdef EMBED
	if(open_adsl_drv()) {
		obcif_arg	myarg;
	    	myarg.argsize = (int) len;
	    	myarg.arg = (int) (rValue);

		if (ioctl(fileno(adslFp), id, &myarg) < 0) {
//	    	        printf("ADSL ioctl failed! id=%x\n",id );
			close_adsl_drv();
			return 0;
	       };

		close_adsl_drv();
		return 1;
	}
#endif
	return 0;
}

// Kaohj --- reset multicast mcft and MFC, send general IGMP Query
void sigResetMRT(int signum)
{
	(void)signum;
	struct mcft_entry *mcp = mcpq;
	Modem_LinkSpeed vLs;

	syslog(LOG_INFO, "igmpproxy: SIGUSR2 caught\n");
#ifdef SEND_GENERAL_QUERY
	untimeout(&general_query_timer.ch);
#endif
#ifdef SEND_UNSOLICIT_REPORT
	untimeout(&unsolicit_report_timer.ch);
#endif
	untimeout(&qtimer.ch);
	IGMP_rx_enable = 0;
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
	if(1){
#else
	if ( adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
			RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0) { // DSL link up
#endif
		// delay sending IGMP Query on link-up
		printf("igmpproxy: DSL up\n");
		// wait seconds to have more stable stream
		sleep(6);
	while(mcp) {
			//del_membership(mcp->grp_addr);
			add_membership(mcp->grp_addr);
			add_mfc(mcp->grp_addr, 0);
			add_mcft_timer(mcp->grp_addr);
		mcp = mcp->next;
	}
		IGMP_rx_enable = 1;
	qtimer.type = TIMER_DELAY_QUERY;
	qtimer.retry_left = 3;
	timeout(igmp_timer_expired , &qtimer, 2, &qtimer.ch);
#ifdef SEND_GENERAL_QUERY
	// Kaohj -- start general IGMP Query
	general_query_timer.type = TIMER_GENERAL_QUERY;
	timeout(igmp_timer_expired , &general_query_timer, GENERAL_QUERY_INTERVAL, &general_query_timer.ch);
#endif
#ifdef SEND_UNSOLICIT_REPORT
	unsolicit_report_timer.type = TIMER_UNSOLT_REPORT;
  timeout(igmp_timer_unsolicit_report, &unsolicit_report_timer,
  																					UNSOLT_REPORT_INTERVAL, &unsolicit_report_timer.ch);
#endif
}
	else {
		printf("igmpproxy: DSL down\n");
		IGMP_rx_enable = 0;
		while(mcp) {
			del_mfc(mcp->grp_addr);
			del_mcft_timer(mcp->grp_addr);
			del_membership(mcp->grp_addr);
			mcp = mcp->next;
		}
	}
}

static int initMRouter(void)
/*
** Inits the necessary resources for MRouter.
**
*/
{
	int Err;
	int i;
	struct IfDesc *Ddp, *Dup;
	int proxyUp = 0;

	buildIfVc();

	switch( Err = enableMRouter() ) {
		case 0: break;
		case EADDRINUSE: log( LOG_ERR, EADDRINUSE, "MC-Router API already in use" ); break;
		default: log( LOG_ERR, Err, "MRT_INIT failed" );
	}
   #ifdef CONFIG_IGMPPROXY_MULTIWAN
        Ddp = getIfByName(igmp_down_if_name);

	if (Ddp==NULL )
		return 0;

	/* add downstream interface */
	igmp_down_if_idx = addVIF(Ddp);

	/* add upstream interface */
	for(i = 0;i<igmp_up_if_num;i++)
	{
		Dup = getIfByName(igmp_up_if_name[i]);
		if (Dup==NULL || Dup->InAdr.s_addr==0)
			continue;
		else
			proxyUp = 1;
		igmp_up_if_idx[i] = addVIF(Dup);
	}

	if(proxyUp == 0)
	{
		return 0;
	}
   #else
	Ddp = getIfByName(igmp_down_if_name);
	Dup = getIfByName(igmp_up_if_name);
	if (Ddp==NULL || Dup==NULL)
		return 0;

	/* add downstream interface */
	igmp_down_if_idx = addVIF(Ddp);

	/* add upstream interface */
	if (Dup->InAdr.s_addr!=0)
		igmp_up_if_idx = addVIF(Dup);
#endif

	signal(SIGTERM, hup);
  //atexit( clean );
  	return 1;
}

void
write_pid()
{
	FILE *fp = fopen(pidfile, "w+");
	if (fp) {
		fprintf(fp, "%d\n", getpid());
		fclose(fp);
	}
	else
	 	printf("Cannot create pid file\n");
}

void
clear_pid()
{
	FILE *fp = fopen(pidfile, "w+");
	if (fp) {
		fprintf(fp, "%d\n", 0);
		fclose(fp);
	}
	else
	 	printf("Cannot create pid file\n");
}


extern int MRouterFD;

void printUsage()
{
    printf("Usage: igmpproxy [-c 1|2|3] -d <down if> -u <up if1[,up if2,up if3...]>\n\n");
    printf("-c: IGMP Query Version config \n\t1: Auto detection\n\t2: Fix to use V2 to send Query\n\t3: Fix to use V3 to send query\n");
    printf("\tif there is no -c option, default using Auto dectection\n");
    printf("-d: Downstream interface config\n");
    printf("-u: Upstream interfaces config, use ',' to seperate interfaces. Ex: nas0_0,vc0\n\n");

    printf("Example: igmpproxy -c 1 -d br0 -u nas0_0,vc0\n");
    printf("Example: igmpproxy -d br0 -u nas0_0\n");
}

int main(int argc, char **argv)
{
	int _argc = 0;
	char *_argv[9];
	pid_t pid;
	int execed = 0;
	int index;
	int i,option_index = 0;
	char *upstreamIfStr=NULL,*downstreamIfStr=NULL,*ifStr=NULL,*igmpVersonConfigStr=NULL;
	char *saveptr=NULL;

	/*
	 * Max argc: igmpproxy -c 1 -d br0 -u nas0_0,vc0 -D
	 * Min argc: igmpproxy -d br0 -u nas0_0,vc0
	 */
	if ((argc>=9) ||(argc<5))
	{
		printUsage();
		exit(1);
	}

	if (strcmp(argv[argc-1], "-D") == 0) {
		argc--;
		execed = 1;
	}

	while ((i = getopt(argc, argv, "c:d:u:")) != EOF)
	{
		switch(i)
		{
			case 'd':
				downstreamIfStr= strdup(optarg);
				break;
			case 'u':
				upstreamIfStr= strdup(optarg);
				break;
			case 'c':
				igmpVersonConfigStr = strdup(optarg);
				IGMPVersionConfig = atoi(igmpVersonConfigStr);
				if(IGMPVersionConfig ==2 )
					IGMPCurrentVersion = 2;
				break;
			default:
				printf("Error! igmpproxy not support this argument! -%c\n ",i);
				break;
		}
	}

	if (!execed) {
		if ((pid = vfork()) < 0) {
			fprintf(stderr, "vfork failed\n");
			exit(1);
		} else if (pid != 0) {
			exit(0);
		}

		for (_argc=0; _argc < argc; _argc++ )
			_argv[_argc] = argv[_argc];
		_argv[0] = runPath;
		_argv[argc++] = "-D";
		_argv[argc++] = NULL;
		execv(_argv[0], _argv);
		/* Not reached */
		fprintf(stderr, "Couldn't exec\n");
		_exit(1);

	} else {
		setsid();
	}

#ifdef CONFIG_IGMPPROXY_MULTIWAN
	igmp_up_if_num = 0;
	index =0;

	ifStr =  strtok_r(upstreamIfStr,",\0", &saveptr);
	while(ifStr !=NULL)
	{
		strcpy(igmp_up_if_name[index], ifStr);
		index++;
		igmp_up_if_num++;
		ifStr = strtok_r(NULL,",",&saveptr);
	}
#else
	strcpy(igmp_up_if_name, upstreamIfStr);
#endif
	strcpy(igmp_down_if_name, downstreamIfStr);
	if(upstreamIfStr)
		free(upstreamIfStr);
	if(downstreamIfStr)
		free(downstreamIfStr);
	if(igmpVersonConfigStr)
		free(igmpVersonConfigStr);

	write_pid();

	signal(SIGHUP, hup);
	signal(SIGTERM, hup);
	signal(SIGUSR1, sigifup);
	signal(SIGUSR2, sigResetMRT);

	while (!initMRouter())
	{
		// Kaohj, polling every 2 seconds
		//printf("initMRouter fail\n");
		sleep(2);
	}
	init_igmp();

	add_fd(MRouterFD);

	{int flags;
	flags = fcntl(MRouterFD, F_GETFL);
	if (flags == -1 || fcntl(MRouterFD, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set MRouterFD to nonblock: %m");
	}

	/* process loop */
	while(1)
	{
		fd_set in;
		struct timeval tv;
		int ret;
		int recvlen;
		struct IfDesc *IfDp;

// Kaohj
#ifdef _SUPPORT_IGMPV3_
		igmpv3_timer();
#endif
//#else
		calltimeout();
//#endif /*_SUPPORT_IGMPV3_*/

		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		in = in_fds;

		ret = select(max_in_fd+1, &in, NULL, NULL, &tv);

		if( ret <= 0 ){
			//printf("igmp: timeout\n");
			continue;
		}

#ifdef NEW_IGMPPROXY_RCV
		{
				struct iovec iov;
				struct cmsghdr *cmsg;
				struct in_pktinfo *pktinfo;
				struct sockaddr_in from2;
				struct msghdr msg;

				unsigned int ifindex = 0;

				char adata[100];

				int ret, ii;
				struct ifreq ifr;

				msg.msg_name = (void *) &from2;                    // The from2 is src_IP of packet
				msg.msg_namelen = sizeof (struct sockaddr_in);
				msg.msg_iov = &iov;
				msg.msg_iovlen = 1;
				msg.msg_control = (void *) adata;                  // This is a buf for ancillary data. Such as ifindex.
				msg.msg_controllen = sizeof adata;

				iov.iov_base = recv_buf;                          // The is a buf for the desired data. Such as RIP Packet(RIP header + RIP payload).
				iov.iov_len = RECV_BUF_SIZE;

				recvlen = recvmsg(MRouterFD, &msg, 0);
				if(recvlen < 0)
				{
					if (errno != EINTR && errno !=EAGAIN) log(LOG_ERR, errno, "recvfrom");
					continue;
				}

				cmsg = CMSG_FIRSTHDR (&msg);
				if (cmsg != NULL && cmsg->cmsg_level == IPPROTO_IP
													&& cmsg->cmsg_type  == IP_PKTINFO )
				{
						pktinfo = (struct in_pktinfo *)CMSG_DATA(cmsg);

						ifindex = pktinfo->ipi_ifindex;

						memset(&ifr, 0, sizeof(ifr));
						ifr.ifr_ifindex = ifindex;

//printf("[%s . %d]The ifr.ifr_ifindex is %d\n", __FILE__, __LINE__, ifr.ifr_ifindex);

						if (ioctl(MRouterFD, SIOCGIFNAME, &ifr))
						{
								printf("Get iface name error\n");
								continue;
						}
				}

//printf("[%s . %d]The ifr.ifr_name is %s\n", __FILE__, __LINE__, ifr.ifr_name);

				if(IGMP_rx_enable)
				{
#ifdef _SUPPORT_IGMPV3_
						if(!strncmp(ifr.ifr_name, "br0", 3))
								igmpv3_accept(recvlen, FROM_LAN);
						else
								igmpv3_accept(recvlen, FROM_WAN);
#else
						if(!strncmp(ifr.ifr_name, "br0", 3))
								//it is from lan.
							 	igmp_accept(recvlen, FROM_LAN);
						else
								//it is from wan
								igmp_accept(recvlen, FROM_WAN);
#endif
				}
		}

#else// if we do not define NEW_IGMPPROXY_RCV

		if(FD_ISSET(MRouterFD, &in_fds)) {
			recvlen = recvfrom(MRouterFD, recv_buf, RECV_BUF_SIZE,
				   0, NULL, &recvlen);
			if (recvlen < 0) {
		    	if (errno != EINTR && errno !=EAGAIN) log(LOG_ERR, errno, "recvfrom");
		    	continue;
			}
			IfDp = getIfByName(igmp_down_if_name);
			if (IGMP_rx_enable)
        			igmpv3_accept(recvlen, IfDp);
	    }

#endif//end NEW_IGMPPROXY_RCV

	}

	return 0;
}

