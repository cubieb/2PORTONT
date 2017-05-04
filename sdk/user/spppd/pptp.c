#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <utmp.h>
#include <mntent.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#include <asm/types.h>		/* glibc 2 conflicts with linux/types.h */
#include <linux/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/if_ether.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/version.h>
#include <linux/kdev_t.h>

#include <netdb.h>

#include "if_sppp.h"
#include "pptp.h"
#include "pptp_msg.h"
#include "pptp_ctrl.h"
#include "pptp_callmgr.h"
#include "pptp_options.h"
#include <rtk/utility.h>

extern int h_errno;


char *pptp_state[PTP_ST_NUM] = {
	"Dead",
	"Initializing",
	"Ctrl_Conn_Establish",
	"Call_Init",
	"Call_Fail",
	"Call_Establish"
};

fd_set pptp_in_fds;
int pptp_max_fd;

int pptp_up(PPTP_DRV_CTRL *p);


/*
 * pptp_add_fd - add an fd to the set that wait_input waits for.
 */
void pptp_add_fd(int fd)
{
	FD_SET(fd, &pptp_in_fds);
	if (fd > pptp_max_fd)
		pptp_max_fd = fd;
}

/*
 * pptp_remove_fd - remove an fd from the set that wait_input waits for.
 */
void pptp_remove_fd(int fd)
{
	FD_CLR(fd, &pptp_in_fds);
}

/*********************************************************/
int pptp_open(PPTP_DRV_CTRL *p)
{
	PPTP_CONN * conn;
	unsigned short callid, peer_callid;
	int retval;

	conn = (PPTP_CONN *)p->conn;
	callid = get_call_ID(conn, &callid, &peer_callid);
	p->sp.sa_addr.pptp.call_id = peer_callid;
	p->local.sa_addr.pptp.call_id = callid;

#ifdef CONFIG_IPV6_VPN
	if (0 == p->local.sa_addr.pptp.ipType)
		PPTP_DEBUG("bind sock %d with %x/%d", p->fd, p->local.sa_addr.pptp.sin_addr.in.s_addr, callid);
	else {
		char buff[48];
		PPTP_DEBUG("bidn sock %d with %s/%d", p->fd, inet_ntop(AF_INET6, &p->local.sa_addr.pptp.sin_addr.in6, buff, 48), callid);
	}
#else
	PPTP_DEBUG("bind sock %d with %x/%d", p->fd, p->local.sa_addr.pptp.sin_addr.s_addr, callid);
#endif
#ifdef CONFIG_LUNA
{
	int entrynum, i;
	MIB_PPTP_T entry;
	unsigned int serverip;
	entrynum = mib_chain_total(MIB_PPTP_TBL);
	for (i=0; i<entrynum; i++)
	{
		if ( !mib_chain_get(MIB_PPTP_TBL, i, (void *)&entry) )
			return;
		serverip = inet_addr(entry.server);
		if(serverip == inet_addr(inet_ntoa(*(struct in_addr *)(&p->sp.sa_addr.pptp.sin_addr.s_addr)))){
			break;
		}
	}
	entry.callid = callid;
	entry.peer_callid = peer_callid;
	mib_chain_update(MIB_PPTP_TBL, &entry, i);
    //printf pptp callid information for diag cmd user: Luke
    printf("#############PPTP INFO:################\n");
    printf("###IP address: %s\n",inet_ntoa(*(struct in_addr *)(&p->local.sa_addr.pptp.sin_addr.s_addr)));
    printf("###CallID: %d(0x%x)\n",callid,callid);
    printf("###Gateway: %s\n",inet_ntoa(*(struct in_addr *)(&p->sp.sa_addr.pptp.sin_addr.s_addr)));
    printf("###Gateway CallID: %d(0x%x)\n",peer_callid,peer_callid);
    printf("###  message@ %s(%s:%d)\n",__FILE__,__func__,__LINE__);
    printf("######################################\n");
}	
#endif
	retval = bind(p->fd, (struct sockaddr*)&p->local, sizeof(struct sockaddr_pppox));
	if (retval < 0) {
		PPTP_DEBUG("Failed to bind PPTP socket: %d", retval);
		return retval;
	}

#ifdef CONFIG_IPV6_VPN
	if (0 == p->sp.sa_addr.pptp.ipType)
		PPTP_DEBUG("Connect sock %d to server %x/%d", p->fd, p->sp.sa_addr.pptp.sin_addr.in.s_addr, peer_callid);
	else {
		char buff[48];
		PPTP_DEBUG("Connect sock %d to server %s/%d", p->fd, inet_ntop(AF_INET6, &p->sp.sa_addr.pptp.sin_addr.in6, buff, 48), peer_callid);
	}
#else
	PPTP_DEBUG("Connect sock %d to server %x/%d", p->fd, p->sp.sa_addr.pptp.sin_addr.s_addr, peer_callid);
#endif
	retval = connect(p->fd, (struct sockaddr*)&p->sp, sizeof(struct sockaddr_pppox));
	if( retval < 0 ){
		PPTP_DEBUG("Failed to connect PPTP socket: %d",retval);
		return retval;
	}

	pptp_change_state( p, PTP_ST_CALL_ESTABLISH );

	return 0;
}

int pptp_close(struct sppp *sp)
{
	PPTP_DRV_CTRL *p = sp->pp_lowerp;

	printf("%s %d enter\n", __func__, __LINE__);
	if (p != NULL) {
		/* send PADT will cause server terminate immediatly */
		pptp_conn_close(p->conn, PPTP_STOP_NONE);
		//pptp_write_some(p->conn);

		UNTIMEOUT(0, 0, p->ch);
		UNTIMEOUT(0, 0, p->ctrl_ch);
		UNTIMEOUT(0, 0, p->out_ctrl_ch);
		
		conninfo_release(p->conn);
		pptp_conn_destroy(p->conn, 1);

#if 0
		if(sp->dev_fd >= 0) {
			remove_fd(sp->dev_fd);
			close(sp->dev_fd);
		}
#endif
		if (p->fd >= 0)
			close(p->fd);
		p->fd = -1;

#if 0
		if (p->ctrl_sock >= 0) {
			close(p->ctrl_sock);
			p->ctrl_sock = -1;
		}
#endif
		if (p->previous_local_addr)
		{
			char cmd_str[256];
			sprintf(cmd_str, "iptables -t nat -D POSTROUTING -o ppp%d -j SNAT --to-source %s", sp->if_unit, 
				inet_ntoa(*(struct in_addr *)&p->previous_local_addr));
			system(cmd_str);

			sprintf(cmd_str, "iptables -D FORWARD -o ppp%d -p tcp --syn -j TCPMSS --set-mss 1420", sp->if_unit);
			system(cmd_str);
		}
		
		p->previous_local_addr = sp->ipcp.myipaddr;

		free(p);
		sp->pp_lowerp = NULL;
	}
	printf("%s %d exit\n", __func__, __LINE__);
	
	return (0);
}

int pptp_change_state(PPTP_DRV_CTRL *p, int state)
{
	p->callctrlState = state;
	ppp_status();
	return 0;
}

int pptp_handle_event( struct sppp *sp, int e )
{
	PPTP_DRV_CTRL *p;

	if (!(p = sp->pp_lowerp))
		return -1;
	
	switch (e)
	{
	case PTP_EVENT_OPEN:
		break;
	case PTP_EVENT_CLOSE:
		switch (p->callctrlState)
		{
		case PTP_ST_CALL_ESTABLISH:
			ppp_down(sp->if_unit);
			break;
		default:
			if (sp->fd >= 0) {
				remove_fd(sp->fd);	//star
				close(sp->fd);
				sp->fd = -1;
			}
			break;
		}
		break;
	default:
		printf("%s %d unknown event.\n", __func__, __LINE__);
		break;
	}
}

int pptp_recv(PPTP_CONN * conn)
{
	fd_set read_set;//, write_set;
	struct timeval tv;
	int ret;

	assert(conn != NULL);
	
	FD_ZERO(&read_set);
	//FD_ZERO(&write_set);

	pptp_fd_set(conn, &read_set, NULL/*&write_set*/);

	tv.tv_sec = 0;
	tv.tv_usec = 100000;

	ret = select(pptp_max_fd+1, &read_set, NULL, NULL, &tv);

	if (ret <= 0)
		return 0;

	pptp_dispatch(conn, &read_set, NULL);

	return 0;
}

extern struct sppp *spppq;
// Mason Yu. Set up_retry
void pptp_out_ctrl(PPTP_DRV_CTRL *p)
{	
	// 0: do up_retry, 1: not do up_retry
	p->up_retry=0;
}

void process_pptp()
{
	struct sppp *sp;
	PPTP_DRV_CTRL *p;
	PPTP_CONN * conn;

	sp = spppq;
	while(sp) {
		if (sp->over != SPPP_PPTP) {
			sp = sp->pp_next;
			continue;
		}
		if (!(p = sp->pp_lowerp)) {
			sp = sp->pp_next;
			continue;
		}		
		
		conn = p->conn;
		if (PTP_ST_DEAD != p->callctrlState)
			pptp_recv(conn);		
		
		if (p->in_pptp_ctrl) {
			assert(conn!=NULL);
			//pptp_handle_state(p);
			
			if (p->callctrlState == PTP_ST_CTRL_ESTABLISH) {
				PPTP_CALL * call;
				
				/* Give the initiator time to write the PIDs while we open the call */
				call = pptp_call_open(conn, call_callback);
				if (call == NULL){
				  PPTP_DEBUG("Couldn't allocate new call");
				}
				else {
					pptp_change_state( p, PTP_ST_CALL_INIT );
				}
			}
			
			if (p->callctrlState == PTP_ST_CALL_ESTABLISH) {
				p->in_pptp_ctrl = 0;

				PPTP_DEBUG("start ppp phase.");
				start_ppp_real(sp);
				return;
			}
		}
		else {
			if ((PTP_ST_DEAD == p->callctrlState) || (PTP_ST_CALL_FAIL == p->callctrlState)) {				
				// Mason Yu. Set up_retry
				if ( !p->up_retry) {
					if ((sp->fd = pptp_up(p)) != -1)
						p->in_pptp_ctrl = 1;					
					else {
						p->up_retry = 1;
						TIMEOUT(pptp_out_ctrl, p, p->out_ctrl_timeout, p->out_ctrl_ch);
						
						p->out_ctrl_timeout *= 2;
						if (p->out_ctrl_timeout >= 16)
							p->out_ctrl_timeout = 1;						
					}					
				}			
				
			}
			/*
			if ((PTP_ST_DEAD == p->callctrlState) || (PTP_ST_CALL_FAIL == p->callctrlState)) {				
				if ((sp->fd = pptp_up(p)) != -1)
					p->in_pptp_ctrl = 1;
			}
			*/
		}

		sp = sp->pp_next;
	}
   	return;
}

/*
 * search through a possible list of ',' seperated ip addresses, try
 * each one,  if it works then use that one
 */
static struct in_addr get_ip_address(char *name)
{
	struct in_addr retval;
	struct sockaddr_in dest;
	int s;
	char *cp, *np;

	retval.s_addr = 0;

	for (cp = name; cp && *cp; cp = np) {
		if ((np = strchr(cp, ',')) != 0) {
			*np++ = '\0';
		}

		if (inet_aton(cp, &retval) == 0) {
			struct hostent *host = gethostbyname(cp);
			if (host==NULL) {
				if (h_errno == HOST_NOT_FOUND)
					printf("gethostbyname: HOST NOT FOUND");
				else if (h_errno == NO_ADDRESS)
					printf("gethostbyname: NO IP ADDRESS");
				else
					printf("gethostbyname: name server error");
				continue;
			}
			if (host->h_addrtype != AF_INET) {
				printf("Host has non-internet address");
				continue;
			}
			memcpy(&retval.s_addr, host->h_addr, sizeof(retval.s_addr));
		}

		if (np)
			*(np - 1) = ','; /* put string back how we found it */

		bzero(&dest, sizeof(dest));
		dest.sin_family = AF_INET;
		dest.sin_port   = htons(PPTP_PORT);
		dest.sin_addr   = retval;

		if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("Cannot get socket: %s", strerror(errno));
			continue;
		}

		if (connect(s, (struct sockaddr *) &dest, sizeof(dest)) != -1) {
			PPTP_DEBUG("connect %x(%d) success.", dest.sin_addr.s_addr, dest.sin_port);
			close(s);
			return(retval);
		}

		close(s);
	}
	
	retval.s_addr = 0;
	
	return retval;
}

/*QL 20120514: support IPv6*/
#ifdef CONFIG_IPV6_VPN
static struct in6_addr get_ip6_address(char *name)
{
	struct in6_addr retval;
	struct sockaddr_in6 dest;
	char *cp, *np;
	char buf[100];
	int s, ret;

	memset(&retval, 0, sizeof(retval));
	
	if (!strchr(name, ':'))
	{
		printf("name is not IPv6 address.\n");
		return retval;
	}
	
	for (cp = name; cp && *cp; cp = np) {
		if ((np = strchr(cp, ',')) != 0) {
			*np++ = '\0';
		}

		if ((ret=inet_pton(AF_INET6, cp, &retval)) <= 0) {
			printf("%s %d fail(%d)\n", __func__, __LINE__, ret);
		}

		if (np)
			*(np - 1) = ','; /* put string back how we found it */

		bzero(&dest, sizeof(dest));
		dest.sin6_family = AF_INET6;
		dest.sin6_port	= htons(PPTP_PORT);
		dest.sin6_addr	= retval;
		
		if ((s = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
			printf("Cannot get socket: %s", strerror(errno));
			continue;
		}
	
		if (connect(s, (struct sockaddr *) &dest, sizeof(dest)) != -1) {
			PPTP_DEBUG("connect %s(%d) success.", 
				inet_ntop(AF_INET6, &dest.sin6_addr, buf, 100), 
				dest.sin6_port);
			close(s);
			return(retval);
		}
		
		close(s);
	}
	
	memset(&retval, 0, sizeof(retval));
	
	return retval;
}
#endif // end of CONFIG_IPV6_VPN

void pptp_down(PPTP_DRV_CTRL *p)
{
	struct sppp *sp;
	printf("pptp down.\n");

	if (!p || !(sp = p->sppp))
		return;
	
	UNTIMEOUT(0, 0, p->ch);
	UNTIMEOUT(0, 0, p->ctrl_ch);
	
	pptp_handle_event(sp, PTP_EVENT_CLOSE);
	
	if (p->fd >= 0)
		close(p->fd);
	p->fd = -1;
	
	if (p->ctrl_sock >= 0) {
		pptp_remove_fd(p->ctrl_sock);
		close(p->ctrl_sock);
	}
	p->ctrl_sock = -1;
	
	memset(&p->local, 0, sizeof(struct sockaddr_pppox));
	/* if server address has been taken, it will not be changed. */
	//memset(&p->sp, 0, sizeof(struct sockaddr_pppox));

	p->in_pptp_ctrl = 0;

	pptp_change_state(p, PTP_ST_DEAD);
}

int pptp_up(PPTP_DRV_CTRL *p)
{
	struct sockaddr_in dest;
	struct in_addr inetaddr;
	struct sppp *sp;
#ifdef CONFIG_IPV6_VPN
	struct in6_addr inet6addr;
#endif // end of CONFIG_IPV6_VPN
	int flags;

	if (NULL == p)
		return -1;
	
	PPTP_DEBUG("pptp_up enter.\n");
	sp = (struct sppp*)p->sppp;
	
#ifndef CONFIG_IPV6_VPN
	inetaddr = p->sp.sa_addr.pptp.sin_addr;
	if (0 == inetaddr.s_addr) {
		inetaddr = get_ip_address(p->server);
		if (0 == inetaddr.s_addr) {			
			PPTP_DEBUG("Server %s is unreachable.\n", p->server);
			return -1;
		}
	}
#else
	if (0 == sp->ipType) {
		inetaddr = p->sp.sa_addr.pptp.sin_addr.in;
		if (0 == inetaddr.s_addr) {
			inetaddr = get_ip_address(p->server);
			if (0 == inetaddr.s_addr) {
				PPTP_DEBUG("Server %s is unreachable.\n", p->server);
				return -1;
			}
		}
	}
	else {
		inet6addr = p->sp.sa_addr.pptp.sin_addr.in6;
		if (ipv6_addr_any(&p->sp.sa_addr.pptp.sin_addr.in6))
		{
			inet6addr = get_ip6_address(p->server);
			if (ipv6_addr_any(&inet6addr)) {
				PPTP_DEBUG("Server %s is unreachable.\n", p->server);
				return -1;
			}
		}
	}
#endif // end of CONFIG_IPV6_VPN

	if (
#ifdef CONFIG_IPV6_VPN
		(0 == sp->ipType) && 
#endif // end of CONFIG_IPV6_VPN
		(p->ctrl_sock = open_inetsock(inetaddr)) == -1) {
		PPTP_DEBUG("create ctrl sock fail.\n");
		return -1;
	}
#ifdef CONFIG_IPV6_VPN
	else if ((1 == sp->ipType) && 
		(p->ctrl_sock = open_inet6sock(inet6addr)) == -1) {
		printf("create ctrl sock fail.\n");
		return -1;
	}
#endif // end of CONFIG_IPV6_VPN
	
	if (!FD_ISSET(p->ctrl_sock, &pptp_in_fds))
		pptp_add_fd(p->ctrl_sock);

	if (p->conn)
		pptp_conn_init(p->conn, p, NULL/* callback */);
	else {
		PPTP_CONN *conn;
		if ((conn = pptp_conn_open(p, 1, NULL/* callback */)) == NULL) {
			PPTP_DEBUG("Could not open connection.");
			goto stop;
		}
		
		if (conninfo_init(conn) < 0) {
			PPTP_DEBUG("No memory.");
			goto stop;
		}
		
		p->conn = conn;
		
		pptp_drv_ctl_put(conn, p);
	}
	
	p->fd = socket(AF_PPPOX,SOCK_STREAM,PX_PROTO_PPTP);
	if (p->fd < 0)	{
		PPTP_DEBUG("Failed to create PPTP socket\n");
		goto stop;
	}

	fcntl(p->fd, F_SETFD, fcntl(p->fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	p->sp.sa_family = AF_PPPOX;
	p->sp.sa_protocol = PX_PROTO_PPTP;
	p->sp.sa_addr.pptp.call_id = 0;
#ifndef CONFIG_IPV6_VPN
	p->sp.sa_addr.pptp.sin_addr.s_addr = inetaddr.s_addr;
#else
	if (0 == sp->ipType) //IPv4
	{
		p->sp.sa_addr.pptp.ipType = 0;
		p->sp.sa_addr.pptp.sin_addr.in = inetaddr;
	}
	else //IPv6
	{
		p->sp.sa_addr.pptp.ipType = 1;
		p->sp.sa_addr.pptp.sin_addr.in6 = inet6addr;
	}
#endif // end of CONFIG_IPV6_VPN

	struct sockaddr_in local_addr;
#ifdef CONFIG_IPV6_VPN
	struct sockaddr_in6 local_addr6;
	char srcbuf[100], dstbuf[100];
#endif // end of CONFIG_IPV6_VPN
	socklen_t   local_addr_len;
	/* get local contrl connection IP address */
#ifdef CONFIG_IPV6_VPN
	if (0 == sp->ipType) //IPv4
	{
#endif // end of CONFIG_IPV6_VPN
		if (getsockname(p->ctrl_sock, (struct sockaddr *)&local_addr, &local_addr_len) < 0)
			PPTP_DEBUG("getsockname fail");
		PPTP_DEBUG("PPTP local %x remote %x", local_addr.sin_addr.s_addr, inetaddr.s_addr);

		memcpy(&p->local, &p->sp, sizeof(struct sockaddr_pppox));
#ifndef CONFIG_IPV6_VPN
		p->local.sa_addr.pptp.sin_addr = local_addr.sin_addr;
#else
		p->local.sa_addr.pptp.ipType = 0;
		p->local.sa_addr.pptp.sin_addr.in = local_addr.sin_addr;
#endif // end of CONFIG_IPV6_VPN
#ifdef CONFIG_IPV6_VPN
	}
	else
	{
		if (getsockname(p->ctrl_sock, (struct sockaddr *)&local_addr6, &local_addr_len) < 0)
			PPTP_DEBUG("getsockname fail");
		PPTP_DEBUG("PPTP local %s remote %s", 
			inet_ntop(AF_INET6, &local_addr6.sin6_addr, srcbuf, 100), 
			inet_ntop(AF_INET6, &inet6addr, dstbuf, 100));

		memcpy(&p->local, &p->sp, sizeof(struct sockaddr_pppox));
		p->local.sa_addr.pptp.ipType = 1;
		p->local.sa_addr.pptp.sin_addr.in6 = local_addr6.sin6_addr;
	}
#endif // end of CONFIG_IPV6_VPN

	flags = fcntl(p->fd, F_GETFL);	// Jenny
	if (flags == -1 || fcntl(p->fd, F_SETFL, flags | O_NONBLOCK) == -1)
		PPTP_DEBUG("Couldn't set PPPoE socket to nonblock\n");
	
	return p->fd;

stop:
	pptp_remove_fd(p->ctrl_sock);
	close(p->ctrl_sock);
	return -1;
}

int pptp_client_init(struct sppp *sppp)
{
	PPTP_CONN * conn;
	struct vpn_param_s *ptpr;
	PPTP_DRV_CTRL *p;
	struct in_addr inetaddr;
#ifdef CONFIG_IPV6_VPN
	struct in6_addr inet6addr;//QL 20120514 support IPv6
#endif // end of CONFIG_IPV6_VPN
	int flags;

	printf("%s %d Enter.\n", __func__, __LINE__);
	
	p = (PPTP_DRV_CTRL *)malloc(sizeof(PPTP_DRV_CTRL));
	if (!p)
		return -1;
	memset (p, 0, sizeof(PPTP_DRV_CTRL));
	p->sppp = (void *)sppp;

	ptpr = sppp->dev;
	if (NULL == ptpr)
		return -1;

	if (0 == sppp->ipType)
		ptpr->ipType = 0;
#ifdef CONFIG_IPV6_VPN
	else
		ptpr->ipType = 1;
#endif

	p->fd = -1;
	p->ctrl_sock = -1;
	p->ctrl_timeout = PPTP_TIMEOUT;
	p->out_ctrl_timeout = 1;		// Mason Yu. Set up_retry
	strcpy(p->name, ptpr->devname);
	memcpy(p->server, ptpr->server, MAX_DOMAIN_LENGTH);
	
	sppp->pp_lowerp = (void *)p;

	if (0 == sppp->ipType) {//ipv4
		inetaddr = get_ip_address(ptpr->server);
		if (0 == inetaddr.s_addr) {
			printf("V4 Server %s is unreachable.\n", ptpr->server);
			return -1;
		}
	}
#ifdef CONFIG_IPV6_VPN
	else {//ipv6
		inet6addr = get_ip6_address(ptpr->server);
		if (ipv6_addr_any(&inet6addr)) {
			printf("V6 Server %s is unreachable.\n", ptpr->server);
			return -1;
		}
	}
#endif

	if ((0 == sppp->ipType) && 
		(p->ctrl_sock = open_inetsock(inetaddr)) == -1) {
		printf("create ctrl sock fail.\n");
		return -1;
	}
#ifdef CONFIG_IPV6_VPN
	else if ((1 == sppp->ipType) && 
			 (p->ctrl_sock = open_inet6sock(inet6addr)) == -1) {
		printf("create v6 ctrl sock fail.\n");
		return -1;
	}
#endif // end of CONFIG_IPV6_VPN

	if ((conn = pptp_conn_open(p, 1, NULL/* callback */)) == NULL) {
		printf("Could not open connection.");
		goto cleanup;
	}

    if (conninfo_init(conn) < 0) {
		printf("No memory.");
		goto shutdown;
    }

	p->in_pptp_ctrl = 1;
	p->conn = conn;

	pptp_drv_ctl_put(conn, p);

	p->fd = socket(AF_PPPOX,SOCK_STREAM,PX_PROTO_PPTP);
	if (p->fd < 0)	{
		printf("Failed to create PPTP socket\n");
		return -1;
	}

	fcntl(p->fd, F_SETFD, fcntl(p->fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	p->sp.sa_family = AF_PPPOX;
	p->sp.sa_protocol = PX_PROTO_PPTP;
	p->sp.sa_addr.pptp.call_id = 0;
#ifndef CONFIG_IPV6_VPN
	p->sp.sa_addr.pptp.sin_addr.s_addr = inetaddr.s_addr;
#else
	if (0 == sppp->ipType) //IPv4
	{
		p->sp.sa_addr.pptp.ipType = 0;
		p->sp.sa_addr.pptp.sin_addr.in = inetaddr;
	}
	else //IPv6
	{
		p->sp.sa_addr.pptp.ipType = 1;
		p->sp.sa_addr.pptp.sin_addr.in6 = inet6addr;
	}
#endif // end of CONFIG_IPV6_VPN

	struct sockaddr_in local_addr;
#ifdef CONFIG_IPV6_VPN
	struct sockaddr_in6 local_addr6;
	char srcbuf[100], dstbuf[100];
#endif // end of CONFIG_IPV6_VPN
	socklen_t   local_addr_len;
	/* get local contrl connection IP address */
#ifdef CONFIG_IPV6_VPN
	if (0 == sppp->ipType) //IPv4
	{
#endif // end of CONFIG_IPV6_VPN
		if (getsockname(p->ctrl_sock, (struct sockaddr *)&local_addr, &local_addr_len) < 0)
			PPTP_DEBUG("getsockname fail");
		PPTP_DEBUG("PPTP local %x remote %x", local_addr.sin_addr.s_addr, inetaddr.s_addr);

		memcpy(&p->local, &p->sp, sizeof(struct sockaddr_pppox));
#ifndef CONFIG_IPV6_VPN
		p->local.sa_addr.pptp.sin_addr = local_addr.sin_addr;
#else
		p->local.sa_addr.pptp.ipType = 0;
		p->local.sa_addr.pptp.sin_addr.in = local_addr.sin_addr;
#endif // end of CONFIG_IPV6_VPN
#ifdef CONFIG_IPV6_VPN
	}
	else
	{
		if (getsockname(p->ctrl_sock, (struct sockaddr *)&local_addr6, &local_addr_len) < 0)
			PPTP_DEBUG("getsockname fail");
		PPTP_DEBUG("PPTP local %s remote %s", 
			inet_ntop(AF_INET6, &local_addr6.sin6_addr, srcbuf, 100), 
			inet_ntop(AF_INET6, &inet6addr, dstbuf, 100));

		memcpy(&p->local, &p->sp, sizeof(struct sockaddr_pppox));
		p->local.sa_addr.pptp.ipType = 1;
		p->local.sa_addr.pptp.sin_addr.in6 = local_addr6.sin6_addr;
	}
#endif // end of CONFIG_IPV6_VPN
	
	flags = fcntl(p->fd, F_GETFL);	// Jenny
	if (flags == -1 || fcntl(p->fd, F_SETFL, flags | O_NONBLOCK) == -1)
		printf("Couldn't set PPPoE socket to nonblock\n");

	return p->fd;

shutdown:
	PPTP_DEBUG("shutdown");
	conninfo_release(conn);
	pptp_conn_destroy(conn, 1);
	return -1;
	
cleanup:
	PPTP_DEBUG("cleanup");
	close_inetsock(p->ctrl_sock, inetaddr);
	p->ctrl_sock = -1;

	return -1;
}

