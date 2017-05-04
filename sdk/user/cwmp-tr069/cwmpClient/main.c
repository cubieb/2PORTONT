#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include <signal.h>
#include <sys/wait.h>
#include <cwmp_rpc.h>
#include "prmt_igd.h"
#include "prmt_wancondevice.h"
#include "parameter_api.h"
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif
#ifdef CONFIG_USER_CWMP_UPNP_DM
#include <sys/epoll.h>
#include "prmt_ctcom_proxy_dev_list.h"
#endif
#ifdef CONFIG_CWMP_TR181_SUPPORT
#include "tr181/prmt_device2.h"
#endif

void cwmp_show_help( void )
{
	fprintf( stderr, "cwmpClient:\n" );
	fprintf( stderr, "	-SendGetRPC:	send GetRPCMethods to ACS\n" );
	fprintf( stderr, "	-SSLAuth:	ACS need certificate the CPE\n" );
	fprintf( stderr, "	-SkipMReboot:	do not send 'M Reboot' event code\n" );
	fprintf( stderr, "	-Delay: 	delay some seconds to start\n" );
	fprintf( stderr, "	-NoDebugMsg: 	do not show debug message\n" );
#ifdef CONFIG_CWMP_TR181_SUPPORT
	fprintf( stderr, "	-TR181:		Use Device:2 as root node\n" );
#endif
	fprintf( stderr, "	-h or -help: 	show help\n" );
	fprintf( stderr, "\n" );
	fprintf( stderr, "	if no arguments, read the setting from mib\n" );
	fprintf( stderr, "\n" );
}

/*refer to climenu.c*/
#define CWMP_RUNFILE	"/var/run/cwmp.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CWMP_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

static void clr_pid()
{
	unlink(CWMP_RUNFILE);
#ifdef CONFIG_MIDDLEWARE
	unlink(CWMP_MIDPROC_RUNFILE);
#endif
}

#ifdef CONFIG_E8B
#ifdef CONFIG_MIDDLEWARE
extern void handle_alarm(int sig, siginfo_t *si, void *uc);
extern void updateMidprocTimer();
void cwmp_handle_alarm(int sig, siginfo_t *si, void *uc)
{
	handle_alarm( sig, si, uc );
	updateMidprocTimer();
}

void handle_x_ct_event(int sig)
{
	unsigned char vChar;
	unsigned int vUint=0;

	printf("\n%s\n",__FUNCTION__);
	mib_get(CWMP_TR069_ENABLE,(void*)&vChar);
	if(vChar == 1 || vChar == 2){
		if( mib_get(CWMP_INFORM_EVENTCODE, &vUint)!=0 ){
			mib_get(MIB_MIDWARE_INFORM_EVENT,(void *)&vChar);
			switch(vChar){
			case CTEVENT_SEND_INFORM:
				vUint = vUint|(EC_PERIODIC);
				break;
			case CTEVENT_ACCOUNTCHANGE:
				vUint = vUint|(EC_X_CT_COM_ACCOUNT);
				break;
#ifdef _PRMT_X_CT_COM_USERINFO_
			case CTEVENT_BIND:
				vUint = vUint|(EC_X_CT_COM_BIND);
				break;
#endif
			default:
				break;
			}
			mib_set(CWMP_INFORM_EVENTCODE, &vUint);
		}
	}
}
#else
void handle_x_ct_account(int sig)
{
	unsigned int vUint=0;

	warn("%s():%d ", __FUNCTION__, __LINE__);
	if (mib_get(CWMP_INFORM_EVENTCODE, &vUint)) {
		vUint = vUint | EC_X_CT_COM_ACCOUNT;
		mib_set(CWMP_INFORM_EVENTCODE, &vUint);
	}
	cwmp_reset_retry_timer();
}

#ifdef _PRMT_X_CT_COM_USERINFO_
void handle_x_ct_bind(int sig)
{
	unsigned int vUint;

	warn("%s():%d ", __FUNCTION__, __LINE__);
	if (mib_get(CWMP_INFORM_EVENTCODE, &vUint)) {
		vUint = vUint | EC_X_CT_COM_BIND;
		mib_set(CWMP_INFORM_EVENTCODE, &vUint);
	}
}
#endif
#endif //#ifdef CONFIG_MIDDLEWARE
#endif // end of CONFIG_E8B


/*star:20091229 START send signal to cwmp process to let it know that wan connection ip changed*/
void sigusr1_handler()
{
	notify_set_wan_changed();
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	setTr069QosFlag(0);
#endif
/*star:20100305 END*/
}
/*star:20091229 END*/

void sigusr2_handler()
{
	struct cwmp_message msg = {0};
	int cwmp_msgid = msgget((key_t)1234,  0666);

	if(cwmp_msgid != -1)
	{
		msg.msg_type = MSG_PRINT_PRMT;
		msgsnd(cwmp_msgid, (void *)&msg, MSG_SIZE, 0);
	}
}

void clear_child(int i)
{
	int status;
	pid_t childpid = 0;

	//childpid=wait( &status );
#ifdef _PRMT_TR143_
#ifdef CONFIG_USER_FTP_FTP_FTP
	//if(childpid!=-1)
	checkPidforFTPDiag( childpid );
#endif //CONFIG_USER_FTP_FTP_FTP
#endif //_PRMT_TR143_
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
//	if(childpid!=-1)
	checkPidforTraceRouteDiag( childpid );
#endif //_SUPPORT_TRACEROUTE_PROFILE_
#ifdef _PRMT_NSLOOKUP_
	checkPidforNSLookupDiag(childpid);
#endif
	/* clear the rest child */
	childpid = waitpid(-1, &status, WNOHANG);

	return;
}

void handle_term()
{
	clr_pid();
	exit(0);
}

static void* ip_link_listener(void *data)
{
	struct sockaddr_nl addr;
	int sock, len, rtl;
	char buffer[4096] = {0};
	struct nlmsghdr *nlh;
	struct ifaddrmsg *ifa;
	struct rtattr *rth;

	if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
		perror ("socket failure\n");

	memset (&addr,0,sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_LINK;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		perror ("bind failure\n");

	while ((len = recv(sock, buffer, sizeof(buffer), 0)) > 0)
	{
		nlh = (struct nlmsghdr *)buffer;
		for (;(NLMSG_OK (nlh, len)) && (nlh->nlmsg_type != NLMSG_DONE); nlh = NLMSG_NEXT(nlh, len))
		{
			char cwmp_ifname[IFNAMSIZ] = {0};
			char *ifname = NULL;
			struct in_addr *ipaddr;

			if (nlh->nlmsg_type != RTM_NEWADDR)
				continue; /* some other kind of announcement */

			ifa = (struct ifaddrmsg *) NLMSG_DATA (nlh);

			if(ifa->ifa_family != AF_INET)
				continue;

			rth = IFA_RTA (ifa);
			rtl = IFA_PAYLOAD (nlh);
			for (;rtl && RTA_OK (rth, rtl); rth = RTA_NEXT (rth,rtl))
			{
				switch(rth->rta_type)
				{
				case IFA_ADDRESS:
					ipaddr = (struct in_addr *)RTA_DATA(rth);
					//fprintf(stderr, "Address got: %s\n", inet_ntoa(*ipaddr));
					break;
				case IFA_LABEL:
					ifname = (char *)RTA_DATA(rth);
					//fprintf(stderr, "Interface got: %s\n", ifname);
					break;
				default:
					//fprintf(stderr, "Ignored rta_type: %d\n", rth->rta_type);
					break;
				}
			}

			port_get_tr069_ifname(cwmp_ifname);
			if(ifname && strcmp(cwmp_ifname, ifname) == 0)
				cwmp_reset_retry_timer();
		}
	}

	if(sock != -1)
		close(sock);

	return NULL;
}


void start_ip_link_listener()
{
	pthread_t id;

	pthread_create(&id, NULL, ip_link_listener, NULL);
	pthread_detach(id);
}

static void* event_listener(void *data)
{
#ifdef CONFIG_USER_CWMP_UPNP_DM
#define MAX_EPOLL_EVENT 5
	struct epoll_event ev, events[MAX_EPOLL_EVENT];
	int epollfd;

	// initialize epoll
	epollfd = epoll_create(MAX_EPOLL_EVENT);
	if(epollfd < 0)
	{
		perror("<cwmpClient> epoll_create\n");
		return NULL;
	}

	init_upnpdm_socket(epollfd);

	while(1)
	{
		int num_ev;
		int i;

		num_ev = epoll_wait(epollfd, events, MAX_EPOLL_EVENT, -1);
		if(num_ev > 0)
		{
			for(i = 0 ; i < num_ev ; i++)
			{
				handle_upnpdm_event(events[i].data.fd);
			}
		}
	}

#endif
	return NULL;
}


void start_event_listener()
{
	pthread_t id;

	pthread_create(&id, NULL, event_listener, NULL);
	pthread_detach(id);
}


int main(int argc, char **argv)
{
	struct net_device_stats nds;
#ifdef CONFIG_MIDDLEWARE
	unsigned char vChar;
	struct CWMP_NODE * selRoot;
	struct sigaction act;
	struct itimerspec its;
	struct sigevent te;
	timer_t timer;
#endif

	log_pid();

	get_net_device_stats("br0", &nds);
	srand(nds.rx_bytes + nds.tx_bytes);
#ifdef CONFIG_E8B
#ifdef CONFIG_MIDDLEWARE
	signal(SIGUSR1, handle_x_ct_event);		//xl_yue: SIGUSR2 is used by midware
#else
	signal(SIGUSR1, handle_x_ct_account);
#ifdef _PRMT_X_CT_COM_USERINFO_
	signal(SIGUSR2, handle_x_ct_bind);
#endif	//end of CONFIG_MIDDLEWARE
#endif	//end of CONFIG_MIDDLEWARE
#else	//CONFIG_E8B
	signal(SIGUSR1, sigusr1_handler);
	signal(SIGUSR2, sigusr2_handler);
#endif

	signal(SIGCHLD, clear_child);	//set this signal process function according to CWMP_TR069_ENABLE below if MIDDLEWARE is defined
	signal(SIGTERM, handle_term);

#ifdef CONFIG_MIDDLEWARE
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = cwmp_handle_alarm;
	sigemptyset(&act.sa_mask);
	sigaction(SIGALRM, &act, NULL);

	te.sigev_notify = SIGEV_SIGNAL;
	te.sigev_signo = SIGALRM;
	te.sigev_value.sival_int = 0;
	timer_create(CLOCK_REALTIME, &te, &timer);

	its.it_interval.tv_sec = 1;
	its.it_interval.tv_nsec = 0;
	its.it_value.tv_sec = 1;
	its.it_value.tv_nsec = 0;
	timer_settime(timer, 0, &its, NULL);
#endif

/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	setTr069QosFlag(0);
#endif
/*star:20100305 END*/

	if( argc >= 2 )
	{
		int i;
		for(i=1;i<argc;i++)
		{
			if( strcmp( argv[i], "-SendGetRPC" )==0 )
			{
				cwmpinit_SendGetRPC(1);
				fprintf( stderr, "<%s>Send GetPRCMethods to ACS\n",__FILE__ );
			}else if( strcmp( argv[i], "-SSLAuth" )==0 )
			{
				cwmpinit_SSLAuth(1);
				fprintf( stderr, "<%s>Set using certificate auth.\n",__FILE__ );
			}else if( strcmp( argv[i], "-SkipMReboot" )==0 )
			{
				cwmpinit_SkipMReboot(1);
				fprintf( stderr, "<%s>Set skipping MReboot event code\n",__FILE__ );
			}else if( strcmp( argv[i], "-Delay" )==0 )
			{
				cwmpinit_DelayStart(30);
				fprintf( stderr, "<%s>Set Delay!\n", __FILE__ );
			}else if( strcmp( argv[i], "-NoDebugMsg" )==0 )
			{
				cwmpinit_NoDebugMsg(1);
				fprintf( stderr, "<%s>Set No Debug Message!\n", __FILE__ );

#ifdef CONFIG_CWMP_TR181_SUPPORT
			}else if( strcmp( argv[i], "-TR181" )==0 )
			{
				cwmpinit_UseTR181();
#endif
			}else if( strcmp( argv[i], "-h" )==0 || strcmp( argv[i], "-help" )==0 )
			{
				cwmp_show_help();
				exit(0);
			}else
			{
				fprintf( stderr, "<%s>Error argument: %s\n", __FILE__,argv[i] );
			}
		}
	}else{
		unsigned char cwmp_flag=0;
		//read the flag, CWMP_FLAG, from mib
		if ( mib_get( CWMP_FLAG, (void *)&cwmp_flag)!=0 )
		{
			printf("\ncwmp_flag=%x\n",cwmp_flag);
			if( (cwmp_flag&CWMP_FLAG_DEBUG_MSG)==0 )
			{
				fprintf( stderr, "<%s>Set No Debug Message!\n", __FILE__ );
				cwmpinit_NoDebugMsg(1);
			}

			if( cwmp_flag&CWMP_FLAG_CERT_AUTH )
			{
				fprintf( stderr, "<%s>Set using certificate auth.\n",__FILE__ );
				cwmpinit_SSLAuth(1);
			}

			if( cwmp_flag&CWMP_FLAG_SENDGETRPC )
			{
				fprintf( stderr, "<%s>Send GetPRCMethods to ACS\n",__FILE__ );
				cwmpinit_SendGetRPC(1);
			}

			if( cwmp_flag&CWMP_FLAG_SKIPMREBOOT )
			{
				fprintf( stderr, "<%s>Set skipping MReboot event code\n",__FILE__ );
				cwmpinit_SkipMReboot(1);
			}

			if( cwmp_flag&CWMP_FLAG_DELAY )
			{
				fprintf( stderr, "<%s>Set Delay!\n", __FILE__ );
				cwmpinit_DelayStart(30);
			}

			if( cwmp_flag&CWMP_FLAG_SELFREBOOT)
			{
				fprintf( stderr, "<%s>Set SelfReboot!\n", __FILE__ );
				cwmpinit_SelfReboot(1);
			}

		}

		if ( mib_get( CWMP_FLAG2, (void *)&cwmp_flag)!=0 )
		{
			if( cwmp_flag&CWMP_FLAG2_DIS_CONREQ_AUTH)
			{
				fprintf( stderr, "<%s>Set DisConReqAuth!\n", __FILE__ );
				cwmpinit_DisConReqAuth(1);
			}

			if( cwmp_flag&CWMP_FLAG2_DEFAULT_WANIP_IN_INFORM)
				cwmpinit_OnlyDefaultWanIPinInform(1);
			else
				cwmpinit_OnlyDefaultWanIPinInform(0);
#ifdef CONFIG_CWMP_TR181_SUPPORT
			if( cwmp_flag&CWMP_FLAG2_USE_TR181)
				cwmpinit_UseTR181();
#endif

			if(cwmp_flag & CWMP_FLAG2_HTTP_503)
				cwmpinit_Return503(1);
		}
	}

#ifdef TELEFONICA_DEFAULT_CFG
//#ifdef WLAN_SUPPORT
	cwmpinit_BringLanMacAddrInInform(1);
//#endif //WLAN_SUPPORT
	cwmpinit_SslSetAllowSelfSignedCert(0);
#endif //TELEFONICA_DEFAULT_CFG

#ifdef CONFIG_USER_TR104
	cwmp_solarOpen();
#endif
//startRip();

	start_ip_link_listener();

	start_event_listener();

	printf("\nenter cwmp_main!\n");

#ifdef CONFIG_MIDDLEWARE
	cwmp_main(mw_tROOT);
#else

#ifdef CONFIG_CWMP_TR181_SUPPORT
	if(gUseTR181)
	{
		printf("Root is Device:2.\n");
		cwmp_main(device_tROOT);
	}
	else
	{
		printf("Root is InternetGatewayDevice.\n");
		cwmp_main(tROOT);
	}
#else	//CONFIG_CWMP_TR181_SUPPORT
	cwmp_main(tROOT);
#endif	//CONFIG_CWMP_TR181_SUPPORT
#endif

	return 0;
}
