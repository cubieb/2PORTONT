#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "if_sppp.h"
#include "pppoe.h"
#include "pppoa.h"
#ifdef CONFIG_USER_PPPOMODEM
#include "pppomodem.h"
#endif //CONFIG_USER_PPPOMODEM
#include "signal.h"
#include <sys/file.h>
#include <rtk/utility.h>
#include <config/autoconf.h>
#include <rtk/sysconfig.h>
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#include "pptp.h"
#include <netdb.h>
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
#include "l2tp_main.h"

#endif//endof CONFIG_USER_L2TPD_L2TPD
#ifdef CONFIG_USER_PPTPD_PPTPD
#include "pptpmanager.h"
#endif
#include "assert.h"
// Kaohj
#ifdef CONFIG_IPV6
#ifndef _LINUX_IN6_H
/*
 *    This is in linux/include/net/ipv6.h.
 */

struct in6_ifreq {
    struct in6_addr ifr6_addr;
    __u32 ifr6_prefixlen;
    unsigned int ifr6_ifindex;
};
#endif
#endif

//N_SPPP has defined in if_sppp.h
//#define N_SPPP	8
#include <sys/sysinfo.h> //added by paula, 3g backup
#ifdef CONFIG_USER_PPPOMODEM
#define BUDBGFLAG ppp_backup_debug
#endif

// Added by Mason Yu. Access internet fail.
static unsigned long GetIP=0;

extern int in_pppoe_disc;
extern unsigned int auth_fail_counter[N_SPPP];
int ppp_dev_fd[N_SPPP];

fd_set in_fds;		/* set of fds that wait_input waits for */
int max_in_fd;		/* highest fd set in in_fds */

struct sppp *dod_sp = NULL;
//int dod_flag = 1;

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
struct VPN_ACCOUNT_ST pppd_account[MAX_VPN_SERVER_ACCOUNT];
struct VPN_SERVER_AUTH_St pppd_auth[SPPPD_AUTH_NUM];
int pptp_host_socket = -1;
fd_set pptpConnSet;
int maxPptpConnFd;
int noAccount[SPPPD_AUTH_NUM]={1, 1};
#endif

#define _PATH_PROCNET_DEV               "/proc/net/dev"

/* We can get an EIO error on an ioctl if the modem has hung up */
#define ok_error(num) ((num)==EIO)

/*
 * SET_SA_FAMILY - set the sa_family field of a struct sockaddr,
 * if it exists.
 */

#define SET_SA_FAMILY(addr, family)			\
    memset ((char *) &(addr), '\0', sizeof(addr));	\
    addr.sa_family = (family);

#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2, &handle)



/********************************************************************
 *
 * sifup - Config the interface up and enable IP packets to pass.
 */

int sifup (struct sppp *sp)
{
    struct ifreq ifr;

    memset (&ifr, '\0', sizeof (ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
    if (ioctl(sp->sock_fd, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
	    	perror("ioctl (SIOCGIFFLAGS)");
		return 0;
    }

    ifr.ifr_flags |= (IFF_UP | IFF_POINTOPOINT);
    if (ioctl(sp->sock_fd, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
	    	perror("ioctl(SIOCSIFFLAGS)");
		return 0;
    }
    return 1;
}

/********************************************************************
 *
 * sifdown - Config the interface down and disable IP.
 */

int sifdown (struct sppp *sp)
{
    struct ifreq ifr;

    memset (&ifr, '\0', sizeof (ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
    if (ioctl(sp->sock_fd, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
	if (! ok_error (errno))
	    perror("ioctl (SIOCGIFFLAGS)");
	return 0;
    }

    ifr.ifr_flags &= ~IFF_UP;
    ifr.ifr_flags |= IFF_POINTOPOINT;
    if (ioctl(sp->sock_fd, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) {
	if (! ok_error (errno))
	    perror("ioctl(SIOCSIFFLAGS)");
	return 0;
    }
    return 1;
}

/********************************************************************
 *
 * sifaddr - Config the interface IP addresses and netmask.
 */

int sifaddr (struct sppp *sp)
{
    struct ifreq   ifr;
    struct rtentry rt;
    u_long	net_mask;
    // Added by Mason Yu
    u_long 	alais_ip;
#ifdef AUTO_PPPOE_ROUTE
    unsigned int dgw;
#endif
	struct in_addr myip = {0}, hisip = {0};

    memset (&ifr, '\0', sizeof (ifr));
    memset (&rt,  '\0', sizeof (rt));

    SET_SA_FAMILY (ifr.ifr_addr,    AF_INET);
    SET_SA_FAMILY (ifr.ifr_dstaddr, AF_INET);
    SET_SA_FAMILY (ifr.ifr_netmask, AF_INET);

    strlcpy (ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
#ifdef IP_PASSTHROUGH
    // Added by Mason Yu for Half Bridge
    // Write gloable variables
    if (sp->ippt_flag ==1) {
       FILE *fp;

       fp = fopen("/tmp/PPPHalfBridge", "w+");
       if (fp) {
       	  fwrite( &sp->ipcp.myipaddr, 4, 1, fp);

       	  // Modified by Mason Yu
       	  alais_ip = sp->ipcp.myipaddr + 1 ;
       	  //*(((char *)(&alais_ip)) + 3) = 254;
       	  fwrite( &alais_ip, 4, 1, fp);

       	  // Added by Mason Yu. Access internet fail.
       	  fwrite( &sp->ipcp.primary_dns, 4, 1, fp);
       	  fclose(fp);
       }
    }
#endif

	memcpy(&myip, &sp->ipcp.myipaddr, sizeof(sp->ipcp.myipaddr));
	syslog(LOG_INFO, "spppd: %s: IP address %s\n", sp->if_name, inet_ntoa(myip));
	memcpy(&hisip, &sp->ipcp.hisipaddr, sizeof(sp->ipcp.hisipaddr));
	syslog(LOG_INFO, "spppd: %s: gateway %s\n", sp->if_name, inet_ntoa(hisip));

#if defined(CONFIG_LUNA)
	//printf pppoe information for diag cmd user: Chuck
	printf("#############PPP INFO:################\n");
	printf("###interface: %s\n",sp->if_name);
	printf("###IP address: %s\n",inet_ntoa(myip));
	printf("###gateway: %s\n",inet_ntoa(hisip));
	printf("###  message@ %s(%s:%d)\n",__FILE__,__func__,__LINE__);
	printf("######################################\n");
#endif

#ifdef CONFIG_USER_PPPOMODEM
	if((sp->over==SPPP_PPPOMODEM) && (sp->ipcp.hisipaddr==0) )
	{
		unsigned int tmp_hisaddr=0x0a404040;
		sp->ipcp.hisipaddr=htonl( tmp_hisaddr+sp->if_unit );
		printf( "gateway==0.0.0.0, change to %s\n", inet_ntoa(hisip) );
		syslog(LOG_INFO, "spppd: %s: new gateway %s\n", sp->if_name, inet_ntoa(hisip));
	}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPPOE_CTC_DELAY_INT
	sp->ctc_retry_cnt = 0;
#endif
/*
 *  Set our IP address
 */
#ifdef IP_PASSTHROUGH
    // Modified by Mason Yu for Half Bridge
    if (sp->ippt_flag )  {
//        ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x0a010101;
        ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x0a000001;	// Jenny, for IP passthrough determination temporarily
    }else
#endif
         ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = sp->ipcp.myipaddr;

    if (ioctl(sp->sock_fd, SIOCSIFADDR, (caddr_t) &ifr) < 0) {
		if (errno != EEXIST) {
		    if (! ok_error (errno))
			perror("ioctl(SIOCSIFADDR)");
		}
    	else {
	    	printf("ioctl(SIOCSIFADDR): Address already exists");
		}
        return (0);
    }
/*
 *  Set the gateway address
 */
#ifdef IP_PASSTHROUGH
    // Modified by Mason Yu for Half Bridge
    if (sp->ippt_flag){
//         ((struct sockaddr_in *) &ifr.ifr_dstaddr)->sin_addr.s_addr = 0x0a010101;
        ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x0a000002;	// Jenny, for IP passthrough determination temporarily
    } else
#endif
    {
         ((struct sockaddr_in *) &ifr.ifr_dstaddr)->sin_addr.s_addr = sp->ipcp.hisipaddr;
    }

    if (ioctl(sp->sock_fd, SIOCSIFDSTADDR, (caddr_t) &ifr) < 0) {
	if (! ok_error (errno))
	    perror("ioctl(SIOCSIFDSTADDR)");
	return (0);
    }
/*
 *  Set the netmask.
 *  For recent kernels, force the netmask to 255.255.255.255.
 */
	/* for kernel version > 2.1.16 */
	net_mask = ~0L;
	/* */
    if (net_mask != 0) {
	((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr = net_mask;
	if (ioctl(sp->sock_fd, SIOCSIFNETMASK, (caddr_t) &ifr) < 0) {
	    if (! ok_error (errno))
		perror("ioctl(SIOCSIFNETMASK)");
	    return (0);
	}
    }


#ifdef IP_PASSTHROUGH
	// Modifieded by Mason Yu for PPP Half Bridge
	// Half Bridge rule
	if (sp->ippt_flag ==1) {

               int hisip[4];
               char br0_alias_cmd[60];
               unsigned long net, mask;

               // Modified by Mason Yu
               hisip[0] = (alais_ip >> 24 ) & 0xff;
               hisip[1] = (alais_ip >> 16 ) & 0xff;
               hisip[2] = (alais_ip >> 8 ) & 0xff;
               hisip[3] = (alais_ip ) & 0xff;

               /* Set br0 alias br0:1  */
               sprintf(br0_alias_cmd, "ifconfig %s %d.%d.%d.%d", (char*)LAN_IPPT, hisip[0], hisip[1], hisip[2], hisip[3]);
               system(br0_alias_cmd);

               	/* Add route for Public IP */
               	if (IN_CLASSA(alais_ip)) {
			net = alais_ip & IN_CLASSA_NET;
			mask = IN_CLASSA_NET;
		}
		else if (IN_CLASSB(alais_ip)) {
			net = alais_ip & IN_CLASSB_NET;
			mask = IN_CLASSB_NET;
		}
		else {
			net = alais_ip & IN_CLASSC_NET;
			mask = IN_CLASSC_NET;
		}

               	sprintf(br0_alias_cmd, "route del -net %d.%d.%d.%d netmask %d.%d.%d.%d",
               		*((unsigned char *)(&net)), *((unsigned char *)(&net)+1), *((unsigned char *)(&net)+2), *((unsigned char *)(&net)+3),
               		*((unsigned char *)(&mask)), *((unsigned char *)(&mask)+1), *((unsigned char *)(&mask)+2), *((unsigned char *)(&mask)+3)
               		 );
               	system(br0_alias_cmd);

               	sprintf(br0_alias_cmd, "route add -host %d.%d.%d.%d dev br0",
               		(int)(sp->ipcp.myipaddr>>24)&0xff, (int)(sp->ipcp.myipaddr>>16)&0xff, (int)(sp->ipcp.myipaddr>>8)&0xff, (int)(sp->ipcp.myipaddr)&0xff);
               	system(br0_alias_cmd);
	}
#endif

	/*
 	*  Add default route
 	*/
#ifdef AUTO_PPPOE_ROUTE
	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
//	if((dgw == DGW_AUTO && sp->over == SPPP_PPPOE) || sp->dgw)	// Jenny, if set to auto(0xef)
	if(dgw == DGW_AUTO || sp->dgw)	// Jenny, if set to auto(0xef)
#else
	if(sp->dgw)
#endif
	{

#ifdef IP_PASSTHROUGH
          if (sp->ippt_flag ==1) {
          	char default_route_cmd[40];
		/* Set dafault route */
#ifdef DEFAULT_GATEWAY_V2
			if (ifExistedDGW() == 1) {	// Jenny, delete existed default gateway first
				sprintf(default_route_cmd, "route del default");
				system(default_route_cmd);
			}
#endif
               	sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);
               	system(default_route_cmd);
          }
	  else if (sp->ippt_flag==2){
		      char default_route_cmd[40];
                       /* Set dafault route */
#ifdef DEFAULT_GATEWAY_V2
			if (ifExistedDGW() == 1) {  // Jenny, delete existed default gateway first
				sprintf(default_route_cmd, "route del default");
				system(default_route_cmd);
			}
#endif
                    sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);
                    system(default_route_cmd);

	  }
	  else
#endif
	  {
		       // Normal rule
			char default_route_cmd[40];
			/* Set dafault route */
#ifdef DEFAULT_GATEWAY_V2
			if (ifExistedDGW() == 1) {	// Jenny, delete existed default gateway first
				sprintf(default_route_cmd, "route del default");
				system(default_route_cmd);
			}
#endif
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP)
			if ( sp->over == SPPP_PPTP )
			{
				sprintf(default_route_cmd, "route del default");
				system(default_route_cmd);
			}
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
			if ( sp->over == SPPP_L2TP)
			{
				sprintf(default_route_cmd, "route del default");
				system(default_route_cmd);
			}
#endif//endof CONFIG_USER_L2TPD_L2TPD

			sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);
			system(default_route_cmd);
#if 0
		       rt.rt_dev = sp->if_name;


		       SET_SA_FAMILY (rt.rt_dst,     AF_INET);
    	       SET_SA_FAMILY (rt.rt_genmask, AF_INET);
		       SET_SA_FAMILY (rt.rt_gateway, AF_INET);
		       ((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr  = 0L;
    	       ((struct sockaddr_in *) &rt.rt_genmask)->sin_addr.s_addr = 0L;
		       ((struct sockaddr_in *) &rt.rt_gateway)->sin_addr.s_addr = 0L;
		       ioctl(sp->sock_fd, SIOCDELRT, &rt);


		       SET_SA_FAMILY (rt.rt_dst,     AF_INET);
    	       SET_SA_FAMILY (rt.rt_genmask, AF_INET);
		       SET_SA_FAMILY (rt.rt_gateway, AF_INET);
		       ((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr     = 0L;
    	       ((struct sockaddr_in *) &rt.rt_genmask)->sin_addr.s_addr = 0L;
		       ((struct sockaddr_in *) &rt.rt_gateway)->sin_addr.s_addr = sp->ipcp.hisipaddr;
		       rt.rt_flags = RTF_UP | RTF_GATEWAY;
		       if (ioctl(sp->sock_fd, SIOCADDRT, &rt) < 0) {
	           	if (! ok_error (errno))
		       		perror("ioctl(SIOCADDRT)");
	           	return (0);
		       }
#endif
	  }
	  syslog(LOG_INFO, "spppd: route add default %s", sp->if_name);
    }

#ifdef NEW_PORTMAPPING
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
		if ( sp->over == SPPP_PPTP )
		{
			PPTP_DRV_CTRL *p;

			p = (PPTP_DRV_CTRL *)sp->pp_lowerp;
			extern void modPolicyRouteTable(const char *pptp_ifname, struct in_addr *real_addr);
#ifdef CONFIG_IPV6_VPN
			modPolicyRouteTable((const char *)sp->if_name, &p->local.sa_addr.pptp.sin_addr.in);
#else
			modPolicyRouteTable((const char *)sp->if_name, &p->local.sa_addr.pptp.sin_addr);
#endif
		}
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
		if ( sp->over == SPPP_L2TP )
		{
			L2TP_DRV_CTRL *p;

			p = (L2TP_DRV_CTRL *)sp->pp_lowerp;
			extern void modPolicyRouteTable(const char *pptp_ifname, struct in_addr *real_addr);
			modPolicyRouteTable((const char *)sp->if_name, &p->local.sin.sin_addr);
		}
#endif//endof CONFIG_USER_L2TPD_L2TPD
#endif//endof NEW_PORTMAPPING

#ifdef IP_PASSTHROUGH
    // Added by Mason Yu for HALF Bridge
    // Modified IP Packet filter by iptables command
    if (sp->ippt_flag) {
    	int myip[4];
        //char del_cmd[60];
        char del_cmd[256];
        //char append_cmd[80];
        char append_cmd[256];

        // Added by Mason Yu. Access internet fail.
	if ( sp->previous_ippt_myip != 0 ) {
		/* Delete IP Packet rule */
        	//sprintf(del_cmd, "iptables -t nat -D POSTROUTING 1");
        	sprintf(del_cmd, "iptables -t nat -D POSTROUTING -o ppp%d -j SNAT --to-source %d.%d.%d.%d", sp->if_unit,
        		(sp->previous_ippt_myip >> 24)&0xff, (sp->previous_ippt_myip >> 16)&0xff, (sp->previous_ippt_myip >> 8)&0xff, sp->previous_ippt_myip&0xff );
        	system(del_cmd);
        } else {
        	/* Delete IP packet rule(MASQUERADE)  */
        	sprintf(del_cmd, "iptables -t nat -D POSTROUTING -o ppp%d -j MASQUERADE", sp->if_unit);
        	system(del_cmd);
        }

        myip[0] = (sp->ipcp.myipaddr >> 24 ) & 0xff;
        myip[1] = (sp->ipcp.myipaddr >> 16 ) & 0xff;
        myip[2] = (sp->ipcp.myipaddr >> 8 ) & 0xff;
        myip[3] = (sp->ipcp.myipaddr ) & 0xff;

        // If the src IP is public IP, the packet need not to do SNAT
        //sprintf(append_cmd, "iptables -t nat -I POSTROUTING 1 -o ppp%d -s %d.%d.%d.%d -j RETURN", sp->if_unit, myip[0], myip[1], myip[2], myip[3]);
        //system(append_cmd);

        /* Append IP Packet rule */
        sprintf(append_cmd, "iptables -t nat -I POSTROUTING 1 -o ppp%d -j SNAT --to-source %d.%d.%d.%d", sp->if_unit, myip[0], myip[1], myip[2], myip[3]);
        system(append_cmd);

        sp->previous_ippt_myip = sp->ipcp.myipaddr;
    }
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	if( sp->over==SPPP_PPTP )
	{
		char nat_cmd[256];
		PPTP_DRV_CTRL *p;
		//struct in_addr wanip;
		p = sp->pp_lowerp;
		//wanip = p->local.sa_addr.pptp.sin_addr;

		if (p->previous_local_addr != 0) {
			sprintf(nat_cmd, "iptables -t nat -D POSTROUTING -o ppp%d -j SNAT --to-source %s", sp->if_unit,
				inet_ntoa(*(struct in_addr *)&p->previous_local_addr));
			system(nat_cmd);
		}

		sprintf(nat_cmd, "iptables -t nat -A POSTROUTING -o ppp%d -j SNAT --to-source %s", sp->if_unit,
			inet_ntoa(*(struct in_addr *)&sp->ipcp.myipaddr));
		system(nat_cmd);

		sprintf(nat_cmd, "iptables -D FORWARD -o ppp%d -p tcp --syn -j TCPMSS --set-mss 1420", sp->if_unit);
		system(nat_cmd);

		sprintf(nat_cmd, "iptables -I FORWARD 1 -o ppp%d -p tcp --syn -j TCPMSS --set-mss 1420", sp->if_unit);
		//sprintf(nat_cmd, "iptables -A FORWARD -o ppp%d -p tcp --syn -j TCPMSS --clamp-mss-to-pmtu", sp->if_unit);
		system(nat_cmd);

		p->previous_local_addr = sp->ipcp.myipaddr;
	}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	if( sp->over == SPPP_L2TP )
	{
		char nat_cmd[256];
		L2TP_DRV_CTRL *p;

		p = sp->pp_lowerp;

		if (p->previous_local_addr != 0) {
			sprintf(nat_cmd, "iptables -t nat -D POSTROUTING -o ppp%d -j SNAT --to-source %s", sp->if_unit,
				inet_ntoa(*(struct in_addr *)&p->previous_local_addr));
			system(nat_cmd);
		}

		sprintf(nat_cmd, "iptables -t nat -A POSTROUTING -o ppp%d -j SNAT --to-source %s", sp->if_unit,
			inet_ntoa(*(struct in_addr *)&sp->ipcp.myipaddr));
		system(nat_cmd);

		p->previous_local_addr = sp->ipcp.myipaddr;
	}
#endif//endof CONFIG_USER_L2TPD_L2TPD
    return 1;
}

/********************************************************************
 *
 * cifaddr - Clear the interface IP addresses, and delete routes
 * through the interface if possible.
 */

int cifaddr (struct sppp *sp)
{
	struct rtentry rt;
	struct ifreq   ifr;

	memset (&ifr, '\0', sizeof (ifr));

	SET_SA_FAMILY (ifr.ifr_addr,    AF_INET);

	strlcpy (ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
	if (ioctl(sp->sock_fd, SIOCSIFADDR, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno)) {
			perror("cifaddr ioctl(SIOCSIFADDR)");
			return 0;
		}
	}
	/* for kernel version < 2.1.16 */
	#if 0
	{
/*
 *  Delete the route through the device
 */
	memset (&rt, '\0', sizeof (rt));

	SET_SA_FAMILY (rt.rt_dst,     AF_INET);
	SET_SA_FAMILY (rt.rt_gateway, AF_INET);
	rt.rt_dev = sp->if_name;

	((struct sockaddr_in *) &rt.rt_gateway)->sin_addr.s_addr = 0;
	((struct sockaddr_in *) &rt.rt_dst)->sin_addr.s_addr     = sp->ipcp.hisipaddr;
	rt.rt_flags = RTF_UP | RTF_HOST;

	/* for kernel version > 2.1.0 */
	{
	    SET_SA_FAMILY (rt.rt_genmask, AF_INET);
	    ((struct sockaddr_in *) &rt.rt_genmask)->sin_addr.s_addr = -1L;
	}

	if (ioctl(sp->sock_fd, SIOCDELRT, &rt) < 0 && errno != ESRCH) {
	    if (! ok_error (errno))
			perror("ioctl(SIOCDELRT)");
	    return (0);
	}
    }
    #endif

    return 1;
}

/*
 * Set the MTU and other parameters for the ppp device
 */
int sifmtu(struct sppp *sp)
{
    struct ifreq   ifr;

    memset (&ifr, '\0', sizeof (ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
    ifr.ifr_mtu = (sp->lcp.mru <= sp->lcp.their_mru ? sp->lcp.mru : sp->lcp.their_mru);

    if (ioctl(sp->sock_fd, SIOCSIFMTU, (caddr_t) &ifr) < 0)
		printf("ioctl(SIOCSIFMTU): %m(%d)", errno);
	return 1;
}

#ifdef CONFIG_IPV6
/********************************************************************
 *
 * sif6addr - Config the interface with an IPv6 link-local address
 */
int sif6addr (struct sppp *sp)
{
    struct in6_ifreq ifr6;
    struct ifreq ifr;
    struct in6_rtmsg rt6;

    if (sp->sock6_fd < 0) {
		errno = -sp->sock6_fd;
		perror("sif6addr IPv6 socket creation");
		return 0;
    }
    memset(&ifr, 0, sizeof (ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof(ifr.ifr_name));
    if (ioctl(sp->sock6_fd, SIOCGIFINDEX, (caddr_t) &ifr) < 0) {
		perror("sif6addr ioctl(SIOCGIFINDEX)");
		return 0;
    }

    /* Local interface */
    memset(&ifr6, 0, sizeof(ifr6));
    //IN6_LLADDR_FROM_EUI64(ifr6.ifr6_addr, our_eui64);
    bcopy(sp->ipv6cp.myip6addr, &ifr6.ifr6_addr, sizeof(ifr6.ifr6_addr));
    ifr6.ifr6_ifindex = ifr.ifr_ifindex;
    ifr6.ifr6_prefixlen = 10;

    if (ioctl(sp->sock6_fd, SIOCSIFADDR, &ifr6) < 0) {
		perror("sif6addr ioctl(SIOCSIFADDR)");
		return 0;
    }

#ifdef CONFIG_IPV6_VPN
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
	if (sp->dgw) {
		char default_route_cmd[60];
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP)
		if ( sp->over == SPPP_PPTP )
		{
			sprintf(default_route_cmd, "route -A inet6 del ::/0");
			system(default_route_cmd);
		}
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
		if ( sp->over == SPPP_L2TP)
		{
			sprintf(default_route_cmd, "route -A inet6 del ::/0");
			system(default_route_cmd);
		}
#endif//endof CONFIG_USER_L2TPD_L2TPD

		sprintf(default_route_cmd, "route -A inet6 add ::/0 dev ppp%d", sp->if_unit);
		system(default_route_cmd);
	}
#endif

    /* Route to remote host */
#ifdef NEW_PORTMAPPING
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	if ( sp->over == SPPP_PPTP )
	{
		PPTP_DRV_CTRL *p;

		p = (PPTP_DRV_CTRL *)sp->pp_lowerp;
		extern void modIPv6PolicyRouteTable(const char *pptp_ifname, struct in6_addr *real_addr);
		modIPv6PolicyRouteTable((const char *)sp->if_name, &p->local.sa_addr.pptp.sin_addr.in6);
	}
	else
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	if ( sp->over == SPPP_L2TP )
	{
		L2TP_DRV_CTRL *p;

		p = (L2TP_DRV_CTRL *)sp->pp_lowerp;
		extern void modIPv6PolicyRouteTable(const char *pptp_ifname, struct in6_addr *real_addr);
		modIPv6PolicyRouteTable((const char *)sp->if_name, &p->local.sin6.sin6_addr);
	}
	else
#endif//endof CONFIG_USER_L2TPD_L2TPD
#endif//endof NEW_PORTMAPPING
#endif
	{
#if defined(CONFIG_E8B)
		//FOR e8 2014 test items 2.1.2, 2.1.3, if PPPoE+DHCPv6, add default gateway here.
		char gatewayAddrStr[40]={0};
		char cmdStr[100]={0};
		{
			inet_ntop(PF_INET6, (struct in6_addr *) sp->ipv6cp.hisip6addr, gatewayAddrStr, sizeof(gatewayAddrStr));
			if (sp->dgw) {
				// route -A inet6 add ::/0 gw fe::0200:00ff:fe00:0100 dev ppp0
				sprintf(cmdStr, "/bin/route -A inet6  add ::/0 gw %s dev ppp%d\n", gatewayAddrStr, sp->if_unit);
				system(cmdStr);
				printf("[%s] %s \n",__func__,cmdStr);
			}
		}
#endif

	    memset(&rt6, 0, sizeof(rt6));
	    //IN6_LLADDR_FROM_EUI64(rt6.rtmsg_dst, his_eui64);
	    bcopy(sp->ipv6cp.hisip6addr, &rt6.rtmsg_dst, sizeof(rt6.rtmsg_dst));
	    rt6.rtmsg_flags = RTF_UP;
	    rt6.rtmsg_dst_len = 10;
	    rt6.rtmsg_ifindex = ifr.ifr_ifindex;
	    rt6.rtmsg_metric = 1;

	    if (ioctl(sp->sock6_fd, SIOCADDRT, &rt6) < 0) {
			perror("sif6addr: ioctl(SIOCADDRT)");
			return 0;
	    }
	}

    return 1;
}


/********************************************************************
 *
 * cif6addr - Remove IPv6 address from interface
 */
int cif6addr (struct sppp *sp)
{
    struct ifreq ifr;
    struct in6_ifreq ifr6;
    struct in6_addr *pip6_addr;

    pip6_addr = (struct in6_addr *)sp->ipv6cp.myip6addr;
    if (!memcmp(pip6_addr->s6_addr, in6addr_any.s6_addr, 16))
    	return 1;
    if (sp->sock6_fd < 0) {
	errno = -sp->sock6_fd;
	perror("IPv6 socket creation failed");
	return 0;
    }
    memset(&ifr, 0, sizeof(ifr));
    strlcpy(ifr.ifr_name, sp->if_name, sizeof(ifr.ifr_name));
    if (ioctl(sp->sock6_fd, SIOCGIFINDEX, (caddr_t) &ifr) < 0) {
	perror("cif6addr ioctl(SIOCGIFINDEX)");
	return 0;
    }

    memset(&ifr6, 0, sizeof(ifr6));
    //IN6_LLADDR_FROM_EUI64(ifr6.ifr6_addr, our_eui64);
    bcopy(sp->ipv6cp.myip6addr, &ifr6.ifr6_addr, sizeof(ifr6.ifr6_addr));
    ifr6.ifr6_ifindex = ifr.ifr_ifindex;
    ifr6.ifr6_prefixlen = 10;
    *pip6_addr = in6addr_any; // clean ip addr

    if (ioctl(sp->sock6_fd, SIOCDIFADDR, &ifr6) < 0) {
	if (errno != EADDRNOTAVAIL) {
	    if (! ok_error (errno))
		perror("cif6addr ioctl(SIOCDIFADDR)");
	}
        else {
	    warn("cif6addr: ioctl(SIOCDIFADDR): No such address");
	}
        return (0);
    }
    return 1;
}
#endif /* CONFIG_IPV6 */


struct user_net_device_stats {
    unsigned long rx_packets;	/* total packets received       */
    unsigned long tx_packets;	/* total packets transmitted    */
    unsigned long rx_bytes;	/* total bytes received         */
    unsigned long tx_bytes;	/* total bytes transmitted      */
    unsigned long rx_errors;	/* bad packets received         */
    unsigned long tx_errors;	/* packet transmit problems     */
    unsigned long rx_dropped;	/* no space in linux buffers    */
    unsigned long tx_dropped;	/* no space available in linux  */
    unsigned long rx_multicast;	/* multicast packets received   */
    unsigned long rx_compressed;
    unsigned long tx_compressed;
    unsigned long collisions;

    /* detailed rx_errors: */
    unsigned long rx_length_errors;
    unsigned long rx_over_errors;	/* receiver ring buff overflow  */
    unsigned long rx_crc_errors;	/* recved pkt with crc error    */
    unsigned long rx_frame_errors;	/* recv'd frame alignment error */
    unsigned long rx_fifo_errors;	/* recv'r fifo overrun          */
    unsigned long rx_missed_errors;	/* receiver missed packet     */
    /* detailed tx_errors */
    unsigned long tx_aborted_errors;
    unsigned long tx_carrier_errors;
    unsigned long tx_fifo_errors;
    unsigned long tx_heartbeat_errors;
    unsigned long tx_window_errors;
};

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}


int chk_ifxmtrcv(struct sppp *sp)
{
int retval = 0;
struct user_net_device_stats	if_stats;
char buf[512];
FILE *fh = fopen(_PATH_PROCNET_DEV, "r");
	if(!fh)
		return 0;
    fgets(buf, sizeof buf, fh);	/* eat line */
    fgets(buf, sizeof buf, fh);

    while (fgets(buf, sizeof buf, fh)) {
	char *s, name[IFNAMSIZ];
		s = get_name(name, buf);
		if (strcmp(sp->if_name,name))
			continue;
		sscanf(s,
			"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
	       &if_stats.rx_bytes,
	       &if_stats.rx_packets,
	       &if_stats.rx_errors,
	       &if_stats.rx_dropped,
	       &if_stats.rx_fifo_errors,
	       &if_stats.rx_frame_errors,
	       &if_stats.rx_compressed,
	       &if_stats.rx_multicast,

	       &if_stats.tx_bytes,
	       &if_stats.tx_packets,
	       &if_stats.tx_errors,
	       &if_stats.tx_dropped,
	       &if_stats.tx_fifo_errors,
	       &if_stats.collisions,
	       &if_stats.tx_carrier_errors,
	       &if_stats.tx_compressed);

		if(sp->pp_last_sent == if_stats.tx_packets) {
			// Mason Yu
			//sp->idletime += 10;
			sp->idletime += 1;  // 1 means 20 seconds

			if(sp->idletime >= sp->timeout) {
				sp->idletime = 0;
				retval = -1;
			}
		}
		else {
			sp->idletime = 0;
		}
		sp->pp_last_sent = if_stats.tx_packets;
		sp->pp_last_recv = if_stats.rx_packets;

		fclose(fh);

		return retval;
    }

    if (ferror(fh)) {
		perror(_PATH_PROCNET_DEV);
    }

	fclose(fh);
	return retval;
}

#ifdef CONFIG_MIDDLEWARE
int getMgtPVCIndex()
{
	MIB_CE_ATM_VC_T Entry,*pEntry;
	int ret=-1;
	unsigned int i,num;

	pEntry = &Entry;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode != CHANNEL_MODE_PPPOE){
			continue;
		}

		if(pEntry->ServiceList & X_CT_SRV_TR069)
		{
			ret = PPP_INDEX(pEntry->ifIndex);
			break;
		}
	}

	return ret;
}
#if 0
static int sendMsg2MidProcess(struct mwMsg * pMsg)
{
	int spid;
	FILE * spidfile;
	int msgid;

	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
		fprintf(stdout,"get cwmp msgqueue error!\n");
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(CWMP_MIDPROC_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		fprintf(stdout,"midware interface pidfile not exists in %s\n",__FUNCTION__);
		return -1;
	}

	pMsg->msg_type = spid;
	pMsg->msg_datatype = MSG_MIDWARE;
	if(msgsnd(msgid, (void *)pMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"send message to midwareintf error!\n");
		return -1;
	}

	return 0;
}

void sendInformKeyParaMsg2MidProcess()
{
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	*(sendBuf) = OP_informKeyPara;
	sendMsg2MidProcess(&sendMsg);
}
#endif
#endif

extern struct sppp *spppq;
// Jenny, set to 1 when press "connect" button
int ppp_up_flag[N_SPPP] = {0};

//paula, 3g backup
#ifdef CONFIG_USER_PPPOMODEM

int isLink_DSL_Internet_WAN()
{
	MIB_CE_ATM_VC_T Entry,*pEntry;
	unsigned int i,num;
	char ifname[IFNAMSIZ];
	struct in_addr inAddr;

	pEntry = &Entry;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode == CHANNEL_MODE_BRIDGE){
			continue;
		}

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_ATM && MEDIA_INDEX(pEntry->ifIndex) != MEDIA_PTM)
			continue;

		if(pEntry->applicationtype & X_CT_SRV_INTERNET)
		{
			if(ifGetName( pEntry->ifIndex, ifname, sizeof(ifname))==0)
				return 0;

			if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			{
				//printf("Found %s is DSL link\n",ifname );
				return 1;
			}
		}
	}

	return 0;
}

int isLink_ETH_Internet_WAN()
{
	MIB_CE_ATM_VC_T Entry,*pEntry;
	unsigned int i,num;
	char ifname[IFNAMSIZ];
	struct in_addr inAddr;

	pEntry = &Entry;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->cmode == CHANNEL_MODE_BRIDGE){
			continue;
		}

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_ETH)
			continue;

		if(pEntry->applicationtype & X_CT_SRV_INTERNET)
		{
			if(ifGetName( pEntry->ifIndex, ifname, sizeof(ifname))==0)
				return 0;

			if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			{
				//printf("Found %s is WTHWAN link\n",ifname );
				return 1;
			}
		}
	}

	return 0;
}
struct callout adsl_3g_con, adsl_3g_dis;
static long backup_act_time=0;
static long adsl_act_time=0;
static int manual_down_flag = 0; //adsl ppp is manually down, no backup support
extern long starttime[N_SPPP] ;
void set_manual_down_flag() //support manually down adsl ppp, called by ppp_if_down()
{
	struct sppp *sp;
	sp = spppq;
	while(sp)
	{
		if (sp->over != SPPP_PPPOMODEM)
			if (sp->pp_phase > PHASE_DEAD) {
				manual_down_flag = 0;
				return;
			}
		sp = sp->pp_next;
	}
	manual_down_flag = 1; //last one adsl ppp is manually disconnected
   	return;
}
int poll_3g_backup_link_status()
{
	struct sppp *sp;

	// Mason Yu.
#if 0
	for (sp=spppq; sp; sp=sp->pp_next) {
			if(sp->over == SPPP_PPPOMODEM)
				continue;

			//if(sp->pp_phase >= PHASE_NETWORK)
			if(starttime[sp->if_unit])
				return 1;
	}
#endif

	// Mason Yu. check if the dsl is up.
#if defined(CONFIG_DEV_xDSL) || defined(CONFIG_PTMWAN)
	if (get_net_link_status(ALIASNAME_DSL0) == 1)
	{
		if( isLink_DSL_Internet_WAN())
			return 1;
	}
#endif

#if defined(CONFIG_ETHWAN)
	if (get_net_link_status(ALIASNAME_NAS0) == 1)
	{
		if( isLink_ETH_Internet_WAN())
			return 1;
	}
#endif

	return 0;
}
int poll_3g_ppp() //0: 3g disconnection, 1: otherwise
{
	struct sppp *sp;
	//struct pppomodem_param_s *p;
	for (sp=spppq; sp; sp=sp->pp_next) {
			if(sp->over == SPPP_PPPOMODEM ){

				if(ppp_up_flag[sp->if_unit])
					return 1;
				//p=sp->dev;
				if(sp->pp_phase > PHASE_DEAD)
				//if(p->state >= POM_ST_DIAL)
					return 1;
			}
	}
	return 0;
}
void ppp_backup_act() //3g backup for adsl ppp session
{
	struct sysinfo info;
	int count;
	sysinfo(&info);
	if(manual_down_flag || poll_3g_ppp() || !attached_3g_usb()) { //3g ppp is up or 3g usb is not found
		backup_act_time = 0;
		return;
	}
	if(!poll_3g_backup_link_status()){ //no adsl ppp connection
		if(!backup_act_time) { //set start time for 3g backup
			backup_act_time = info.uptime;
			TIMEOUT(ppp_backup_act, 0, 1, adsl_3g_con);
			//printf("no adsl link start\n");
			return;
		}
		count = info.uptime-backup_act_time;
		if(count >= ppp_backup_timer){ //exceed backup timer, start 3g backup ppp session
			ppp_up(ppp_backup_unit);
			backup_act_time = 0;

		}
		else{
			//printf("no adsl link\n");
			if(BUDBGFLAG) printf("no adsl ppp link counter:%d\n", count);
			TIMEOUT(ppp_backup_act, 0, 1, adsl_3g_con);
		}
	}
	else{ // adsl ppp connection
		backup_act_time = 0;
	}
}
void ppp_adsl_act() //stop 3g backup when adsl ppp session is continually established
{
	struct sysinfo info;
	sysinfo(&info);
	int count;
	if(!poll_3g_ppp()) { //3g ppp is disconnected
		adsl_act_time = 0;
		return;
	}
	if(poll_3g_backup_link_status()){ //adsl ppp session is established
		if(!adsl_act_time) { //set start time for adsl connection
			adsl_act_time = info.uptime;
			TIMEOUT(ppp_adsl_act, 0, 1, adsl_3g_dis);
			//printf("adsl link start\n");
			return;
		}
		count = info.uptime-adsl_act_time;
		if(count >= ppp_backup_timer){ //exceed backup timer, stop 3g backup ppp session
			ppp_down(ppp_backup_unit);
			adsl_act_time = 0;

		}
		else{
			//printf("adsl link\n");
			if(BUDBGFLAG) printf("adsl ppp link counter:%d\n", count);
			TIMEOUT(ppp_adsl_act, 0, 1, adsl_3g_dis);
		}
	}
	else{ // no adsl ppp connection
		adsl_act_time = 0;
	}
}
void poll_3g_backup_status()
{
	int link_st = poll_3g_backup_link_status();
	if(!adsl_act_time && link_st){
		backup_act_time = 0;
		ppp_adsl_act();
	}
	else if(!backup_act_time && !link_st){
		adsl_act_time = 0;
		ppp_backup_act();
	}
}
#endif //CONFIG_USER_PPPOMODEM

int ppp_if_up(struct sppp *sp, int ncp_idx)
{
	FILE *fp;
	char buff[32];
	char buff2[64];
	char value[64];
	char dns1[16]={0};
	char dns2[16]={0};
	char myip[16]={0};

	if(!sifup(sp)) {
		sppp_last_connection_error(sp->if_unit, ERROR_NOT_ENABLED_FOR_INTERNET);
		return 0;
	}
	if(ncp_idx==IDX_IPCP && !sifaddr(sp)) {
		sppp_last_connection_error(sp->if_unit, ERROR_IP_CONFIGURATION);
		return 0;
	}
#ifdef CONFIG_IPV6
	if(ncp_idx==IDX_IPV6CP && !sif6addr(sp)) {
		sppp_last_connection_error(sp->if_unit, ERROR_IP_CONFIGURATION);
		return 0;
	}
#endif
	if(!sifmtu(sp))
		return 0;

	if (1 == sp->role)
		return 0;

	/* change DNS */
	if(sp->ipcp.primary_dns || sp->ipcp.second_dns)
	{
		struct in_addr myip_addr = {0};

#ifdef CONFIG_TR069_DNS_ISOLATION
		if(sp->isTr069_interface)
		{
			set_tr069_dns_isolation(1);
			fp = fopen("/var/resolv_tr069.conf", "w");
		}
		else
#endif
		{
			// Write into per interface resolv.conf
			snprintf(value, 64, "/var/ppp/resolv.conf.%s", sp->if_name);
			fp = fopen(value, "w");
		}

		memcpy(&myip_addr, &sp->ipcp.myipaddr, sizeof(sp->ipcp.myipaddr));
		strncpy(myip, inet_ntoa(myip_addr), 16);
		myip[15] = '\0';
		if(sp->ipcp.primary_dns) {
			struct in_addr dns = {0};
			memcpy(&dns, &sp->ipcp.primary_dns, sizeof(sp->ipcp.primary_dns));
			strncpy(dns1, inet_ntoa(dns), 16);
			dns1[15] = '\0';
			snprintf(buff2, 64, "%s@%s\n", dns1, myip);
			fputs(buff2, fp);
			syslog(LOG_INFO, "spppd: %s: Primary DNS %s", sp->if_name, inet_ntoa(dns));
		}
		if(sp->ipcp.second_dns) {
			struct in_addr dns = {0};
			memcpy(&dns, &sp->ipcp.second_dns, sizeof(sp->ipcp.second_dns));
			strncpy(dns2, inet_ntoa(dns), 16);
			dns1[15] = '\0';
			snprintf(buff2, 64, "%s@%s\n", dns2, myip);
			fputs(buff2, fp);
			syslog(LOG_INFO, "spppd: %s: Secondary DNS %s", sp->if_name, inet_ntoa(dns));
		}
		fclose(fp);
#ifdef CONFIG_TR069_DNS_ISOLATION
//set tr069 dns
		//set_tr069_dns_isolation(1);
		set_tr069_dns_isolation(0);
#endif

#if defined(PORT_FORWARD_GENERAL) || defined(DMZ)
#ifdef NAT_LOOPBACK
	sprintf(buff, "%s\n", inet_ntoa(myip_addr));
	set_ppp_NATLB(sp->if_name, buff);
#endif
#endif

#ifdef CONFIG_MIDDLEWARE
		if(sp->if_unit == getMgtPVCIndex())
		{
			/*save dns for management channel pvc*/
			fp = 	fopen(MGT_DNS_FILE, "w");
			memset(buff,0,32);
			if(sp->ipcp.primary_dns) {
				struct in_addr dns = {0};
				memcpy(&dns, &sp->ipcp.primary_dns, sizeof(sp->ipcp.primary_dns));
				sprintf(buff, "%s\n", inet_ntoa(dns));
				fputs(buff, fp);
			}
			if(sp->ipcp.second_dns) {
				struct in_addr dns = {0};
				memcpy(&dns, &sp->ipcp.second_dns, sizeof(sp->ipcp.second_dns));
				sprintf(buff, "%s\n", inet_ntoa(dns));
				fputs(buff, fp);
			}
			fclose(fp);
		}
		//sendInformKeyParaMsg2MidProcess();
#endif
	}

	if (!(sp->dgw)) {
		struct ppp_policy_route_info ppp_info;
		strcpy(ppp_info.if_name,sp->if_name);
		ppp_info.hisip=sp->ipcp.hisipaddr;
		ppp_info.myip=sp->ipcp.myipaddr;
		//ppp_info.primary_dns=sp->ipcp.primary_dns;
		//ppp_info.second_dns=sp->ipcp.second_dns;
		if(ppp_info.myip!=0)
			set_ppp_source_route(&ppp_info);
	}

	ppp_status();
	sppp_last_connection_error(sp->if_unit, ERROR_NONE);

	return 1;
}

int ppp_down_flag[N_SPPP];	// Added by Jenny, connect/disconnect manually flag

// Jenny, check authentication restart timeout
void authTimeout(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	auth_fail_counter[sp->unit] = 0;
	ppp_down(sp->if_unit);
	ppp_up(sp->if_unit);
}
#ifdef CONFIG_USER_PPPOE_PROXY
void linkTimeout(void *cookie)
{
}
#endif


#ifdef CONFIG_IPV6
const char RADVD_CONF[] = "/var/radvd.conf";
const char RADVD_NEW_CONF[] ="/var/radvd2.conf";
const char KEYWORD1[]= "AdvValidLifetime";
const char KEYWORD2[]= "AdvPreferredLifetime";
const char RADVD_PID[] = "/var/run/radvd.pid";

void modifyRAConf(void)
{

	FILE *fp=NULL;
	FILE *nfp=NULL;
	char buf[512];
	int radvdpid;

	if ((fp = fopen(RADVD_CONF, "r")) == NULL)
	{
		printf("Open file %s Error!\n", RADVD_CONF);
		return;
	}

	if ((nfp = fopen(RADVD_NEW_CONF, "w")) == NULL)
	{
		printf("Open file %s Error!\n", RADVD_NEW_CONF);
		fclose(fp);
		return;
	}

	while(fgets(buf,sizeof(buf),fp)>0)
	{
		if(strstr(buf,KEYWORD1))
			fprintf(nfp,"\t\t%s 7200;\n",KEYWORD1);
		else if (strstr(buf,KEYWORD2))
			fprintf(nfp,"\t\t%s 0;\n",KEYWORD2);
		else
			fprintf(nfp,"%s",buf);
	}

	fclose(fp);
	fclose(nfp);

	sprintf(buf,"cp %s %s\n",RADVD_NEW_CONF, RADVD_CONF);
	system(buf);

	radvdpid = read_pid((char *)RADVD_PID);
	if (radvdpid > 0) {
		kill(radvdpid, SIGHUP);
	}
}
#endif

int ppp_if_down(struct sppp *sp)
{
	char value[64];
	char ifname[10];
	sprintf(ifname,"ppp%d",sp->unit);
#ifdef CONFIG_IPV6

	int inFlags;

	//For if WAN PPPoE connection is down, LAN PC's IPv6 address need to change from
	//from "Prefered" to "Obsolete"
	//So lifetime of previous prefix is need to be set to 0.
	if (getInFlags(ifname, &inFlags) && (inFlags & IFF_RUNNING))
	{
		printf("Now %s is going down, preparing to disconnect IPV6...re setup radvd..\n",ifname);
		modifyRAConf();
	}
#endif

	snprintf(value, 64, "/var/ppp/resolv.conf.%s", ifname);
	unlink(value);

	if(!cifaddr(sp))
		return 0;
#ifdef CONFIG_IPV6
	// Kaohj -- 0: ipv4, 1: ipv6, 2: both
	if (sp->ipType == 1 || sp->ipType == 2)
		cif6addr(sp);
#endif

	if(!sifdown(sp))
		return 0;

	if(sp->over == SPPP_PPPOE && sp->role != 1)
		pppoeDelete(sp);
	// Kaohj
	//else if(sp->over == SPPP_PPPOATM)
	//	pppoatm_close(sp->dev);
#ifdef CONFIG_USER_PPPOMODEM
	else if(sp->over == SPPP_PPPOMODEM)
	{
		struct pppomodem_param_s *pomr=sp->dev;
		//printf( "%s: %s call pppomodem_close()\n", __FUNCTION__, (sp->timeout ||  pomr->hungup)?"to":"NOT to" );
		/* dial on demand => timeout, or hungup */
		//if ( sp->timeout ||  pomr->hungup )
			pppomodem_close( sp );
	}
#endif //CONFIG_USER_PPPOMODEM

	stop_ppp(sp);
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	if (sp->over == SPPP_PPTP)
	{
		pptp_close(sp);
	}
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	if ( sp->over == SPPP_L2TP)
	{
		l2tp_close(sp);
	}
#endif //endof CONFIG_USER_L2TPD_L2TPD
//	sppp_detach(sp);
	//ppp_status();
//	sppp_attach(sp);
	if(sp->role != 1){
		sppp_if_down(sp);	// Jenny
		ppp_status();
	}

#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
#ifdef CONFIG_USER_PPPOE_CTC_DELAY_INT
	if (auth_fail_counter[sp->unit] == 1) {
			TIMEOUT(authTimeout, (void *)sp, get_poe_ctcto(sp,1), sp->auth_ch);
			return 0;
		}
#else
	/* Jenny, Authentication failed max counter, set timer */
	if (auth_fail_counter[sp->unit] >= 3) {
		TIMEOUT(authTimeout, (void *)sp, 120, sp->auth_ch);
		return 0;
	}
#endif
	/* dial on demand */
	if (sp->role != 1 && !dial_setup(sp)){
		return 0;
	}
	/* direct dial */
//	if(sp->over == SPPP_PPPOE || !ppp_down_flag[sp->unit])	// Jenny
	if(!ppp_down_flag[sp->unit]){	// Jenny
	#ifdef CONFIG_USER_PPPOMODEM //paula, 3g backup PPP
		if(sp->over!=SPPP_PPPOMODEM)
			manual_down_flag = 0;
	#endif //CONFIG_USER_PPPOMODEM
		if(sp->role != 1 && !start_ppp(sp))
			return 0;
		}
	#ifdef CONFIG_USER_PPPOMODEM //paula, 3g backup PPP
	else{
		if(sp->over!=SPPP_PPPOMODEM)
			set_manual_down_flag();
	}
	#endif
	return 1;
}

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

/********************************************************************
 *
 * output - Output PPP packet.
 */

void output (struct sppp *sp, unsigned char *p, int len)
{

    if (write(sp->fd, p, len) < 0) {
        if (errno == EWOULDBLOCK || errno == ENOBUFS
            || errno == ENXIO || errno == EIO || errno == EINTR)
            printf("write: warning: %m (%d)", errno);
        else
            printf("write: %m (%d)", errno);
    }
}

//extern struct sppp *spppq;

int get_input()
{
	struct sppp *sp, *sq;
	int nr;
	int len = 1500;
	unsigned char buf[1536];

	sp = spppq;

	while(sp) {
#ifdef CONFIG_USER_PPPOE_PROXY
		if(sp->role == 1 && sp->over == SPPP_PPPOE && get_lcpstate(sp) == 0){
			sq = sp;
			sp = sp->pp_next;
			DEBUG_PRINT("delete a closed server sp unit %d\n", sp->unit);
			del_pppd(sq->unit);
			continue;
		}
#endif
		// Jenny
		if (sp->fd < 0) {
			sp = sp->pp_next;
			continue;
		}
		// Kaohj
		if (!FD_ISSET(sp->fd, &in_fds)) {
			sp = sp->pp_next;
			continue;
		}

		if (sp->fd >= 0) {
			nr = read(sp->fd, buf, len);
			if (nr < 0 && errno != EWOULDBLOCK && errno != EIO && errno != EINTR)
				printf("read: error");
			if (nr < 0 && errno == ENXIO)
#if defined(CONFIG_USER_PPPOMODEM) || defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
				nr=0;
#else
				break;
#endif //CONFIG_USER_PPPOMODEM || CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD
		}
	    if (nr < 0 && sp->if_unit >= 0) {
    	    /* N.B. we read ppp_fd first since LCP packets come in there. */
        	nr = read(sp->dev_fd, buf, len);
        	if (nr < 0 && errno != EWOULDBLOCK && errno != EIO && errno != EINTR)
            	printf("read /dev/ppp: %m");
        	if (nr < 0 && errno == ENXIO)
#if defined(CONFIG_USER_PPPOMODEM) || defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
				nr=0;
#else
            	break;
#endif //CONFIG_USER_PPPOMODEM || CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD
    	}

#ifdef CONFIG_USER_PPPOMODEM
	if( (nr==0) && (sp->over==SPPP_PPPOMODEM) )
	{
		struct pppomodem_param_s *pmdev=sp->dev;
		if(pmdev->hungup==0)
		{
			printf("Modem hangup\n");
			//pmdev->hungup = 1;
			pppomodem_set_hangup(sp);
			stop_interface_modem(sp);
		}
	}
#endif //CONFIG_USER_PPPOMODEM

    	if(nr>0)
	    	sppp_input(sp, buf, nr);

		    sp = sp->pp_next;
    }
   	return 0;
}


int sppp_recv(void)
{
	fd_set in = in_fds;
	struct timeval tv;
	int ret;


	tv.tv_sec = 0;
	tv.tv_usec = 10000;

	ret = select(max_in_fd+1, &in, NULL, NULL, &tv);

	if(ret>0)
		get_input();
	return 0;
}

int stop_ppp(struct sppp *sp)
{
	// Kaohj
//#if 0
	/* disconnect channel to PPP interface */
	//patch for PPPOA, it should disconnect channel as the channel will be reused when reconnect.
	if ((sp->over == SPPP_PPPOATM) && (ioctl(sp->fd, PPPIOCDISCONN, &sp->if_unit) < 0)) {
		printf("%s(%d) Couldn't disconnect PPP unit %d, error %d(%s)\n", __FUNCTION__, __LINE__, sp->if_unit, PPPIOCDISCONN, strerror(errno));
	}
	if (ioctl(sp->fd, PPPIOCDETACH, &sp->chindex) < 0) {
		printf("Couldn't detach channel %d, %s\n", sp->chindex, strerror(errno));
	}
//#endif
	if(sp->dev_fd >= 0)
		remove_fd(sp->dev_fd);
	if(sp->fd>=0)
		remove_fd(sp->fd);
	if(sp->fd>=0) {
		close(sp->fd);
		sp->fd = -1;	// Jenny
	}
	return 0;
}

// Jenny, set to 1 when press "connect" button
//int ppp_up_flag[N_SPPP] = {0};

extern struct callout idle_ch;

static void
sppp_holdoff_out(void *cookie)
{
	struct sppp *sp = (struct sppp *)cookie;
	
	printf("holdoff out !\n");
	sp->holdoff_delay = 0;
	start_ppp_real(sp);
}

// Kaohj
int start_ppp_real(struct sppp *sp)
{
	int flags;
//	struct sppp *tmp_sp = spppq;

	if (sp->holdoff_delay) {
		printf("%s: wait %d seconds to start ...\n", __FUNCTION__, sp->holdoff_delay);
		TIMEOUT(sppp_holdoff_out, (void *)sp, sp->holdoff_delay, sp->holdoff_ch);
		sp->holdoff_delay = 0;
		return -1;
	}
	ppp_up_flag[sp->if_unit] = 0;

#ifdef _CWMP_MIB_
	ppp_pending_flag[sp->if_unit] = 0;
#endif
	if (sp->fd < 0) {
		printf("open ppp fd error!!\n");
		return -1;
	}

	/* get channel number */
	if (ioctl(sp->fd, PPPIOCGCHAN, &sp->chindex) == -1) {
	   	printf("%s: Couldn't get channel number: %d\n", sp->if_name, sp->chindex);
		return -1;
	}

	sp->fd = open("/dev/ppp", O_RDWR);
	if (sp->fd < 0) {
	   	printf("Couldn't reopen /dev/ppp\n");
		return -1;
	}

	fcntl(sp->fd, F_SETFD, fcntl(sp->fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	/* attach PPP interface to exist channel */
	if (ioctl(sp->fd, PPPIOCATTCHAN, &sp->chindex) < 0) {
	   	printf("Couldn't attach to channel %d\n", sp->chindex);
		return -1;
	}

	flags = fcntl(sp->fd, F_GETFL);
	if (flags == -1 || fcntl(sp->fd, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set /dev/ppp (channel) to nonblock\n");

	/* connect channel to PPP interface */
	if (ioctl(sp->fd, PPPIOCCONNECT, &sp->if_unit) < 0) {
		printf("Couldn't attach to PPP unit %d\n", sp->if_unit);
		return -1;
	}

	add_fd(sp->dev_fd);
	add_fd(sp->fd);

	sp->pp_con = (void *)ppp_if_up;
	sp->pp_tlf = (void *)ppp_if_down;

	/* set LCP restart timer timeout */
	sp->lcp.timeout = 3;
	/* set ipcp parameter */
#ifdef CONFIG_SPPPD_STATICIP
	if(sp->staticip == 0)
		sp->ipcp.myipaddr = 0;
#else
	sp->ipcp.myipaddr = 0;
#endif
	sp->ipcp.hisipaddr = 0;
	sp->ipcp.primary_dns = 0;
	sp->ipcp.second_dns = 0;

	/* */
	sp->pp_up(sp);

	if (sp->timeout)	// Jenny
		TIMEOUT(sppp_idle, 0, 1, idle_ch);
	return 0;
}

int start_ppp(struct sppp *sp)
{
int flags;

	if(sp->over == SPPP_PPPOATM) {
		// Kaohj
		struct pppoa_param_s *poar=sp->dev;
		if (poar->fd<=0)
			sp->fd = pppoatm_init(sp->dev);
		else
			sp->fd = poar->fd;
		//sp->fd = pppoatm_init(sp->dev);
		return(start_ppp_real(sp));
	}
	else
	if(sp->over == SPPP_PPPOE) {
		if (sp->fd < 0)	// Jenny
			sp->fd = pppoe_client_init(sp);
		/*
		if (pppoe_client_connect(sp) == -1) {
			in_pppoe_disc = 0;
			return 0;	// pppoe deleted
		}
		in_pppoe_disc = 0;
		*/
	}
#ifdef CONFIG_USER_PPPOMODEM
	else if(sp->over == SPPP_PPPOMODEM)
	{
		pppomodem_init(sp);
		return 0;
	}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	else if (sp->over == SPPP_PPTP)
	{
		sp->fd = pptp_client_init(sp);
	}
#endif //CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	else if (sp->over == SPPP_L2TP)
	{
		sp->fd = l2tp_client_init(sp);
	}
#endif
	else
		return -1;

#if 0
	if(sp->fd<0) {
		printf("open ppp fd error!!\n");
		return -1;
	}

	/* get channel number */
	if (ioctl(sp->fd, PPPIOCGCHAN, &sp->chindex) == -1) {
	   	printf("Couldn't get channel number: %d\n", sp->chindex);
		return -1;
	}


	sp->fd = open("/dev/ppp", O_RDWR);
	if (sp->fd < 0) {
	   	printf("Couldn't reopen /dev/ppp\n");
		return -1;
	}

	/* attach PPP interface to exist channel */
	if (ioctl(sp->fd, PPPIOCATTCHAN, &sp->chindex) < 0) {
	   	printf("Couldn't attach to channel %d\n", sp->chindex);
		return -1;
	}

	flags = fcntl(sp->fd, F_GETFL);
	if (flags == -1 || fcntl(sp->fd, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set /dev/ppp (channel) to nonblock\n");

	/* connect channel to PPP interface */
    if (ioctl(sp->fd, PPPIOCCONNECT, &sp->if_unit) < 0) {
		printf("Couldn't attach to PPP unit %d\n", sp->if_unit);
		return -1;
	}

	add_fd(sp->dev_fd);
	add_fd(sp->fd);

	sp->pp_con = ppp_if_up;
	sp->pp_tlf = ppp_if_down;

	/* set LCP restart timer timeout */
	sp->lcp.timeout = 3;
	/* set ipcp parameter */
	sp->ipcp.myipaddr = 0;
	sp->ipcp.hisipaddr = 0;
	sp->ipcp.primary_dns = 0;
	sp->ipcp.second_dns = 0;

	/* */

	sp->pp_up(sp);
#endif
	return 0;
}

#if 0
int clear_dgw(void)
{
struct sppp	*sp = spppq;

	while(sp) {
		sp->dgw = 0;
	    sp = sp->pp_next;
	}
	return 0;
}
#endif

#if 0//defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)

int get_ppp_unit_by_index(const int index)
{
	struct sppp *sp;
	struct vpn_param_s *ptpr;

	sp = spppq;
	while (sp)
	{
		ptpr = (struct vpn_param_s *)sp->dev;
		if (ptpr) {
			if (index == ptpr->index)
				return sp->unit;
		}
		sp = sp->pp_next;
	}

	return -1;
}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP

int del_ppp(int unit)
{
	struct sppp *sp;
	unsigned int flags;

	printf("%s unit=%d\n",__func__,unit);
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			cifaddr(sp);
#ifdef CONFIG_IPV6
			// Kaohj -- 0: ipv4, 1: ipv6, 2: both
			if (sp->ipType == 1 || sp->ipType == 2)
				cif6addr(sp);
#endif
			sifdown(sp);
			sp->pp_down(sp);
			if(sp->over == SPPP_PPPOE)
				pppoeDelete(sp);
			else if(sp->over == SPPP_PPPOATM) {
				if (sp->dev)
					pppoatm_close(sp->dev);
			}
#ifdef CONFIG_USER_PPPOMODEM
			else if(sp->over == SPPP_PPPOMODEM)
			{
				pppomodem_close(sp);
			}
			if(ppp_backup_flag) //paula, 3g backup PPP
			{
				adsl_act_time = 0;
				backup_act_time = 0;
				manual_down_flag = 0;
			}
#endif //CONFIG_USER_PPPOMODEM

			auth_fail_counter[sp->unit] = 0;
			ppp_down_flag[sp->unit] = 0;
			ppp_up_flag[sp->if_unit] = 0;
			disc_counter[sp->if_unit] = 0;
			pre_PADT_flag[sp->if_unit] = 0;
			sendPADR_flag[sp->if_unit] = 0;
			ioctl(ppp_dev_fd[sp->if_unit], PPPIOCGFLAGS, (caddr_t)&flags);
			if (flags & SC_LOOP_TRAFFIC)
				flags &= ~SC_LOOP_TRAFFIC;
			ioctl(ppp_dev_fd[sp->if_unit], PPPIOCSFLAGS, (caddr_t)&flags);

			/* disconnect channel to PPP interface */
			if (sp->fd >= 0) {
				if (ioctl(sp->fd, PPPIOCDISCONN, &sp->if_unit) < 0)
					printf("Couldn't disconnect PPP unit %d: %m", sp->if_unit);

				if (ioctl(sp->fd, PPPIOCDETACH, &sp->chindex) < 0)
					printf("Couldn't detach channel %d: %m", sp->chindex);
			}

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
			if (sp->over == SPPP_PPTP)
			{
				pptp_close(sp);
			}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
			if (sp->over == SPPP_L2TP)
			{
				l2tp_close(sp);
			}
#endif //endof CONFIG_USER_L2TPD_L2TPD
			close(sp->sock_fd);
			sp->sock_fd = -1;
#ifdef CONFIG_IPV6
			close(sp->sock6_fd);
			sp->sock6_fd = -1;
#endif

			if(sp->dev_fd >= 0)
				remove_fd(sp->dev_fd);
			if(sp->fd>=0)
				remove_fd(sp->fd);	//star
			if(sp->fd>=0) {
				close(sp->fd);
				sp->fd = -1;	// Jenny
			}

			sppp_detach(sp);

#ifdef CONFIG_USER_PPPOE_PROXY
			if(sp->enable_pppoe_proxy)
				freeAllSession();

#endif
//#ifdef CONFIG_USER_PPPOE_PROXY
#if 0
			if(sp->enable_pppoe_proxy)
			{
				no_adsl_link_wan_ppp(sp->unit);
			}
#endif
// Mason Yu. Add VPN ifIndex
// We init 9~10 for PPTP and 11~12 for L2TP in advance. So we do not need to free the sp->if_unit.
#if 0
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
 			if (sp->over == SPPP_PPTP)
				close(ppp_dev_fd[sp->if_unit]);
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
			if (sp->over == SPPP_L2TP)
				close(ppp_dev_fd[sp->if_unit]);
#endif//endof CONFIG_USER_L2TPD_L2TPD
#endif

			ppp_status();

			if(sp->dev)
				free(sp->dev);
			free(sp);
			return 0;
		}
		sp = sp->pp_next;
	}
	return -1;
}


void dial_start(int signum)
{
	unsigned int flags;
	printf("process dial signal %x\n", signum);
	if (dod_sp) {
		if(dod_sp->over == 1) {
			if(dod_sp->pp_lowerp)
				return;
		}
		if(dod_sp->pp_phase != PHASE_DEAD)
			return;
		ioctl(ppp_dev_fd[dod_sp->if_unit], PPPIOCGFLAGS, (caddr_t)&flags);
		flags &= ~SC_LOOP_TRAFFIC;
		ioctl(ppp_dev_fd[dod_sp->if_unit], PPPIOCSFLAGS, (caddr_t)&flags);
		if (!ppp_down_flag[dod_sp->unit])
			start_ppp(dod_sp);
	}
}

int sdifflag(struct sppp *sp)
{
	struct ifreq ifr;

	memset (&ifr, '\0', sizeof (ifr));
	strlcpy(ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
	if (ioctl(sp->sock_fd, SIOCGIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
		perror("ioctl (SIOCGIFFLAGS)");
		return 0;
	}
	ifr.ifr_flags &= ~IFF_POINTOPOINT;
	ifr.ifr_flags |= IFF_UP;
	if (ioctl(sp->sock_fd, SIOCSIFFLAGS, (caddr_t) &ifr) < 0) {
		if (! ok_error (errno))
		perror("ioctl(SIOCSIFFLAGS)");
		return 0;
	}
	return 1;
}

int sdialifaddr(struct sppp *sp)
{
	struct ifreq ifr;
	u_long net_mask;
	char default_route_cmd[40];

	memset (&ifr, '\0', sizeof (ifr));

	SET_SA_FAMILY (ifr.ifr_addr,    AF_INET);
	SET_SA_FAMILY (ifr.ifr_dstaddr, AF_INET);
	SET_SA_FAMILY (ifr.ifr_netmask, AF_INET);

	strlcpy (ifr.ifr_name, sp->if_name, sizeof (ifr.ifr_name));
	/* Set our IP address */
	((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x40404040;
	if (ioctl(sp->sock_fd, SIOCSIFADDR, (caddr_t) &ifr) < 0) {
		if (errno != EEXIST) {
			if (!ok_error (errno))
			perror("ioctl(SIOCSIFADDR)");
		}
		else {
			printf("ioctl(SIOCSIFADDR): Address already exists");
		}
		return (0);
	}
	/* Set the gateway address */
	((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr = 0x40404040;
	if (ioctl(sp->sock_fd, SIOCSIFDSTADDR, (caddr_t) &ifr) < 0) {
		if (!ok_error (errno))
			perror("ioctl(SIOCSIFDSTADDR)");
		return (0);
	}
	/* Set the netmask. For recent kernels, force the netmask to 255.255.255.255. */
	/* for kernel version > 2.1.16 */
	net_mask = ~0L;
	if (net_mask != 0) {
		((struct sockaddr_in *) &ifr.ifr_netmask)->sin_addr.s_addr = net_mask;
		if (ioctl(sp->sock_fd, SIOCSIFNETMASK, (caddr_t) &ifr) < 0) {
			if (!ok_error (errno))
				perror("ioctl(SIOCSIFNETMASK)");
			return (0);
		}
	}
	/* Set dafault route */
	sprintf(default_route_cmd, "route add default ppp%d", sp->if_unit);
	system(default_route_cmd);

	return 1;
}

int dial_setup(struct sppp *sp)
{
	if (sp->timeout) {
		int ppid = (int)getpid();
		unsigned int flags;

		ioctl(ppp_dev_fd[sp->if_unit], PPPIOCSTIMEOUT, &ppid);
		ioctl(ppp_dev_fd[sp->if_unit], PPPIOCGFLAGS, (caddr_t)&flags);
		flags |= SC_LOOP_TRAFFIC;
		ioctl(ppp_dev_fd[sp->if_unit], PPPIOCSFLAGS, (caddr_t)&flags);
		dod_sp = sp;
		printf("set sp timeout %d\n", sp->timeout);
		sifup(sp);
		sdialifaddr(sp);
		sdifflag(sp);
		system("echo '0'>/proc/internet_flag");
/*		if (dod_flag == 1) {	// dial-on-demand setup, used by dns triggering for the first time
			FILE *fp = fopen("/tmp/ppp_dod_flag", "w");
			fprintf(fp, "%d", dod_flag);
			fclose(fp);
			dod_flag = 0;
		}*/
		return 0;
	}
	return -1;
}

int add_ppp(int unit, struct spppreq *spr)
{
	struct sppp *sp;
	//struct pppoe_param_s *poer;
	//struct pppoa_param_s *poar;

	int flags;
#ifdef AUTO_PPPOE_ROUTE
	unsigned int dgw;
#endif
	//printf("add_ppp, unit = %d.\n",unit);
	del_ppp(unit);
	sp = malloc(sizeof(struct sppp));
	if(!sp)
		return -1;
	memset (sp, 0, sizeof (struct sppp));

	sp->role = 0;
	sp->up_flag = 1;
	sp->unit = unit;
	//printf("create unit %d\n", unit);
	sp->mode = spr->ppp.mode;
	// Kaohj -- IP version
	sp->ipType = spr->ppp.ipType;

#ifdef CONFIG_TR069_DNS_ISOLATION
        sp->isTr069_interface = spr->ppp.isTr069_interface;
#endif
	//if(spr->ppp.dgw) {
#ifdef AUTO_PPPOE_ROUTE
	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
	if(dgw == DGW_AUTO || spr->ppp.dgw)	// Jenny, if set to auto(0xef)
#else
	if(spr->ppp.dgw)
#endif
	{
		/* clear all other interface dgw and set this one */
		// Kaohj -- default route restriction should be done by user program
		//clear_dgw();
		sp->dgw = 1;
		sp->timeout = spr->ppp.timeout;
	}
	else {
		sp->dgw = 0;
		sp->timeout = 0;
	}

	// Added by Jenny for keepalive setting
	if (spr->ppp.diska)
		sp->diska = 1;
	else
		sp->diska = 0;

#ifdef IP_PASSTHROUGH
	 // Added by Mason Yu for Half Bridge
	if(spr->ppp.ippt_flag) {
		sp->ippt_flag = 1;
	}
	else
		sp->ippt_flag = 0;
#endif

	/* set debug flag */
	sp->debug = spr->ppp.debug;

	/* setup device */
	sp->over = spr->ppp.over;
	if(sp->over == SPPP_PPPOATM) {
		sp->dev = malloc(sizeof(struct pppoa_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoa_param_s));
	}
	else
	if(sp->over == SPPP_PPPOE) {
		sp->dev = malloc(sizeof(struct pppoe_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoe_param_s));
	}
#ifdef CONFIG_USER_PPPOMODEM
	else if(sp->over == SPPP_PPPOMODEM)
	{
		sp->dev = malloc(sizeof(struct pppomodem_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppomodem_param_s));
		{
			/* set debug flag */
			struct pppomodem_param_s *p_pom=sp->dev;
			if(p_pom) p_pom->debug=spr->ppp.debug;
		}
	}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	else if (sp->over == SPPP_PPTP)
	{
		sp->dev = malloc(sizeof(struct vpn_param_s));
		if (!sp->dev) {
			close(((struct vpn_param_s *)(spr->dev))->dev_fd);
			goto err;
		}
		memcpy(sp->dev, spr->dev, sizeof(struct vpn_param_s));
	}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	else if (sp->over == SPPP_L2TP)
	{
		sp->dev = malloc(sizeof(struct vpn_param_s));
		if (!sp->dev) {
			close(((struct vpn_param_s *)(spr->dev))->dev_fd);
			goto err;
		}
		memcpy(sp->dev, spr->dev, sizeof(struct vpn_param_s));
	}
#endif//endof CONFIG_USER_L2TPD_L2TPD
	else
		goto err;

	/* mtu option */
	sp->lcp.mru = spr->ppp.lcp.mru;

	/* setting/changing myauth */
	// Kaohj -- user setting for auth
	sp->myauth.chal_type= spr->ppp.myauth.chal_type;
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_PPP_MPPE_MPPC
    sp->myauth.enc_type=  spr->ppp.myauth.enc_type;
#endif
		//	printf("!!!!!!!!sp->myauth.enctype %x\n",sp->myauth.enctype);
#endif
	sp->myauth.proto = spr->ppp.myauth.proto;
	sp->myauth.aumode = spr->ppp.myauth.proto;	// Jenny
	//sp->myauth.proto = PPP_PAP;
	strcpy(sp->myauth.name, spr->ppp.myauth.name);
	strcpy(sp->myauth.secret, spr->ppp.myauth.secret);

	/* setting/changing hisauth */
	bzero((char*)&sp->hisauth, sizeof sp->hisauth);

	/* assign interface unit */
	sp->dev_fd = ppp_dev_fd[unit];
	sp->if_unit = unit;
	sprintf(sp->if_name, "ppp%d", sp->if_unit);
	strcpy(spr->ppp.if_name, sp->if_name);
	sp->fd = -1;	// Jenny
#ifdef _CWMP_MIB_
	sp->autoDisconnectTime = spr->ppp.autoDisconnectTime;
	sp->warnDisconnectDelay = spr->ppp.warnDisconnectDelay;
#endif
#ifdef CONFIG_SPPPD_STATICIP
        //alex_huang for static ip setting
	if(spr->ppp.staticip == 1) {
		sp->ipcp.myipaddr = spr->ppp.ipcp.myipaddr;
		sp->staticip = 1;
	}
#endif
    /* Get an internet socket for doing socket ioctls. */
   	sp->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
   	if (sp->sock_fd < 0) {
		if ( ! ok_error ( errno ))
    		printf("Couldn't create IP socket");
   	}


	fcntl(sp->sock_fd, F_SETFD, fcntl(sp->sock_fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
#ifdef CONFIG_IPV6
	sp->sock6_fd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sp->sock6_fd < 0)
		sp->sock6_fd = -errno;	/* save errno for later */
	fcntl(sp->sock6_fd, F_SETFD, fcntl(sp->sock6_fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
#endif
	sppp_attach(sp);
	/* update ppp status file */
	ppp_status();
#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
	sppp_last_connection_error(sp->if_unit, ERROR_UNKNOWN);	// Jenny, last connection error init

//alex_huang for pppoe_proxy
#ifdef CONFIG_USER_PPPOE_PROXY
	sp->enable_pppoe_proxy = spr->ppp.enable_pppoe_proxy;
	sp->itfGroup = spr->ppp.itfGroup;
	sp->maxUser = spr->ppp.maxUser;
	sp->currUser = 0;
#if 0
	if(sp->enable_pppoe_proxy)
	{
		TIMEOUT(linkTimeout, (void *)sp, 100, sp->link_ch);
	}
#endif
#endif

#ifdef CONFIG_USER_PPPOE_CTC_DELAY_INT
	sp->ctc_retry_cnt = 0;
#endif

	/* dial on demand */
	if (!dial_setup(sp))
		return 0;
	/* direct dial */
//	printf("add_ppp(%s)\n", sp->if_name);

	if(!start_ppp(sp))
		return 0;



err:
	printf("add_ppp() error!!\n");
	if(sp->fd>=0)
	{
		close(sp->fd);
		sp->fd = -1;	// Jenny
	}
	if(sp->sock_fd>=0) {
		close(sp->sock_fd);
		sp->sock_fd = -1;
	}
#ifdef CONFIG_IPV6
	if(sp->sock6_fd>=0) {
		close(sp->sock6_fd);
		sp->sock6_fd = -1;
	}
#endif
	if(sp->dev)	// Jenny
		free(sp->dev);
	free(sp);
	return -1;
}

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS) || defined(CONFIG_USER_PPPOE_PROXY)
int start_pppd_real(struct sppp *sp)
{
	int flags;

	if (sp->fd < 0) {
		printf("open pppd fd error!!\n");
		return -1;
	}

	/* get channel number */
	if (ioctl(sp->fd, PPPIOCGCHAN, &sp->chindex) == -1) {
	   	printf("%s: Couldn't get channel number: %d\n", sp->if_name, sp->chindex);
		return -1;
	}

	/* sp->fd is backup in PPTPD_DRV_CTRL */
	sp->fd = open("/dev/ppp", O_RDWR);
	if (sp->fd < 0) {
	   	printf("Couldn't reopen /dev/ppp\n");
		return -1;
	}

	fcntl(sp->fd, F_SETFD, fcntl(sp->fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	/* attach PPP interface to exist channel */
	if (ioctl(sp->fd, PPPIOCATTCHAN, &sp->chindex) < 0) {
	   	printf("Couldn't attach to channel %d\n", sp->chindex);
		return -1;
	}

	flags = fcntl(sp->fd, F_GETFL);
	if (flags == -1 || fcntl(sp->fd, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set /dev/ppp (channel) to nonblock\n");

	/* connect channel to PPP interface */
	if (ioctl(sp->fd, PPPIOCCONNECT, &sp->if_unit) < 0) {
		printf("Couldn't attach to PPP unit %d\n", sp->if_unit);
		return -1;
	}

	add_fd(sp->dev_fd);
	add_fd(sp->fd);

	sp->pp_con = (void *)ppp_if_up;
	sp->pp_tlf = (void *)ppp_if_down;

	/* set LCP restart timer timeout */
	sp->lcp.timeout = 3;
	/* myipaddr and hisipaddr has been initialized in ipcp_init(), so don't initialize it anymore. */
	//sp->ipcp.myipaddr = 0;
	//sp->ipcp.hisipaddr = 0;
	//sp->ipcp.primary_dns = 0;
	//sp->ipcp.second_dns = 0;

	//sp->pp_flags |= IFF_MASTER;

	/* */
	if (1 == sp->role) {
		sp->pp_up(sp);
	}

	if (sp->timeout)	// Jenny
		TIMEOUT(sppp_idle, 0, 1, idle_ch);
	return 0;
}

#ifdef CONFIG_USER_PPTPD_PPTPD
int start_pppd(struct sppp *sp, int clientSocket)
{
	if (sp->over == SPPP_PPTPD)
	{
		sp->fd = pptp_server_init(sp, clientSocket);
		if (sp->fd < 0)
			return -1;
	}
	else
		return -1;

	return 0;
}
#endif

/*
 * over: SPPP_PPTP or SPPP_L2TP
 */
struct sppp * add_pppd(int over)
{
	struct sppp *sp;
	int flags;
	int if_unit=-1;
	int dev_fd = -1;
	int authIdx;

	/* creates a new PPP interface and make /dev/ppp own the interface */
	dev_fd = open("/dev/ppp", O_RDWR);
	if (dev_fd < 0) {
		printf("Couldn't open /dev/ppp\n");
		return NULL;
	}

	flags = fcntl(dev_fd, F_GETFL);
	if (flags == -1 || fcntl(dev_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		printf("Couldn't set /dev/ppp to nonblock\n");
		return NULL;
	}

	if(ioctl(dev_fd, PPPIOCNEWUNIT, &if_unit) < 0) {
		printf("Couldn't create new ppp unit\n");
		close(dev_fd);
		return NULL;
	}

	sp = malloc(sizeof(struct sppp));
	if(!sp)
		return NULL;
	memset (sp, 0, sizeof (struct sppp));

	sp->unit = if_unit;
	//printf("create unit %d\n", if_unit);
	sp->ipType = 0;	/*only support IPv4 now*/
	sp->diska = 0;	/*keepalive*/

	sp->over = over;
	sp->role = 1;

	/* mtu option */
	sp->lcp.mru = 1460;

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
	if(SPPP_PPPOE != over){
		/* setting/changing myauth */
		if (SPPP_PPTPD == over)
			authIdx = PPTPD_AUTH_IDX;
		else if(SPPP_L2TPD == over)
			authIdx = L2TPD_AUTH_IDX;

		sp->myauth.chal_type= pppd_auth[authIdx].chal_type;
#ifdef CONFIG_PPP_MPPE_MPPC
		sp->myauth.enc_type=  pppd_auth[authIdx].enc_type;
#endif
		sp->myauth.proto = pppd_auth[authIdx].auth_proto;
		sp->myauth.aumode = pppd_auth[authIdx].auth_proto;	// Jenny
	}
#endif
		strcpy(sp->myauth.name, "test");
		strcpy(sp->myauth.secret, "test");

	sp->hisauth.flags = AUTHFLAG_NOCALLOUT;
	sp->hisauth.flags = AUTHFLAG_NOCALLOUT;

	/* setting/changing hisauth */
	bzero((char*)&sp->hisauth, sizeof(sp->hisauth));

	/* assign interface unit */
	sp->dev_fd = dev_fd;
	sp->if_unit = if_unit;
	sprintf(sp->if_name, "ppp%d", sp->if_unit);
	sp->fd = -1;	// Jenny

    /* Get an internet socket for doing socket ioctls. */
   	sp->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
   	if (sp->sock_fd < 0) {
		if ( ! ok_error ( errno ))
    		printf("Couldn't create IP socket");
   	}

	fcntl(sp->sock_fd, F_SETFD, fcntl(sp->sock_fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	spppd_attach(sp);

	ppp_status();

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
	if (SPPP_PPTPD == over || SPPP_L2TPD == over)
	{
		//cxy 2015-11-16: let pptp and l2tp clients access pptp and l2tp server.
		va_cmd (IPTABLES, 6,1, FW_ADD,(char *)FW_INPUT,(char *)ARG_I,sp->if_name,"-j",(char *)FW_ACCEPT);
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
		// cxy 2015-11-16: let pptp and l2tp clients  access out network 
		restart_IPFilter_DMZ_MACFilter();
#endif
	}
#endif
	return sp;

err:
	if(sp->fd >= 0)
	{
		close(sp->fd);
		sp->fd = -1;	// Jenny
	}
	if(sp->sock_fd >= 0)
	{
		close(sp->sock_fd);
		sp->sock_fd = -1;
	}
	if (sp->dev_fd >= 0)
	{
		close(sp->dev_fd);
		sp->dev_fd = -1;
	}
	if(sp->dev)	// Jenny
		free(sp->dev);
	if (sp->pp_lowerp)
		free(sp->pp_lowerp);
	free(sp);

	return NULL;
}

int del_pppd(int unit)
{
	struct sppp *sp, *sq;
#ifdef CONFIG_USER_PPTPD_PPTPD
	PPTPD_DRV_CTRL *p_pptp;
#endif
#ifdef CONFIG_USER_L2TPD_LNS
	L2TPD_DRV_CTRL *p_l2tp;
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	PPPOEPROXY_DRV_CTRL *p_pppoe;
#endif

	unsigned int flags;
	//printf("%s unit=%d\n",__func__,unit);
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
#ifdef CONFIG_USER_PPPOE_PROXY
			if(SPPP_PPPOE == sp->over){
				p_pppoe = sp->pp_lowerp;
				sendPADT(p_pppoe);
				sp->pp_down(sp);
				sq = p_pppoe->sp;
				del_policy_routing_table(sp, sq, sq->currUser);
				if(sq->currUser == 0)
					sifdown(sq);
				UNTIMEOUT(0, 0, p_pppoe->ch);
				UNTIMEOUT(0, 0, p_pppoe->disc_ch);

				if(p_pppoe->fd >=0){
					close(p_pppoe->fd);
					p_pppoe->fd = -1;
				}
			}
#endif

			cifaddr(sp);
			sifdown(sp);

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
			/* release client IP to ASSIGNED_IP_POOL */
			if(sp->over != SPPP_PPPOE)
			{
				releasePppdClientIP(sp->over, sp->ipcp.hisipaddr);
				sp->pp_down(sp);
				ioctl(sp->dev_fd, PPPIOCGFLAGS, (caddr_t)&flags);
				if (flags & SC_LOOP_TRAFFIC)
					flags &= ~SC_LOOP_TRAFFIC;
				ioctl(sp->dev_fd, PPPIOCSFLAGS, (caddr_t)&flags);
			}
#endif
			/* disconnect channel to PPP interface */
			if (sp->fd >= 0) {
				if (ioctl(sp->fd, PPPIOCDISCONN, &sp->if_unit) < 0)
					printf("Couldn't disconnect PPP unit %d\n", sp->if_unit);

				if (ioctl(sp->fd, PPPIOCDETACH, &sp->chindex) < 0)
					printf("Couldn't detach channel %d\n", sp->chindex);
			}

			if (sp->sock_fd >= 0) {
				close(sp->sock_fd);
				sp->sock_fd = -1;
			}
#ifdef CONFIG_USER_PPTPD_PPTPD
			if(SPPP_PPTPD == sp->over){
				p_pptp = sp->pp_lowerp;
				if (p_pptp) {
					if (p_pptp->ctrl_sock >= 0) {
						FD_CLR(p_pptp->ctrl_sock, &pptpConnSet);
						close(p_pptp->ctrl_sock);
						p_pptp->ctrl_sock = -1;
					}

					if (p_pptp->fd != sp->fd) {
						close(p_pptp->fd);
						p_pptp->fd = -1;
					}
				}
			}
#endif
#ifdef CONFIG_USER_L2TPD_LNS
			if(SPPP_L2TPD == sp->over){
				p_l2tp = sp->pp_lowerp;
				if(p_l2tp){
					if (p_l2tp->fd != sp->fd) {
						close(p_l2tp->fd);
						p_l2tp->fd = -1;
					}
				}
			}
#endif
			if(sp->dev_fd >= 0){
				remove_fd(sp->dev_fd);
				close(sp->dev_fd);
				sp->dev_fd = -1;
				}

			if(sp->fd >= 0) {
				remove_fd(sp->fd);
				close(sp->fd);
				sp->fd = -1;
			}


			sppp_detach(sp);

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
			if (SPPP_PPTPD == sp->over || SPPP_L2TPD == sp->over)
			{
				va_cmd (IPTABLES, 6,1, FW_DEL,(char *)FW_INPUT,(char *)ARG_I,sp->if_name,"-j",(char *)FW_ACCEPT);
#if defined(IP_PORT_FILTER) 
				va_cmd (IPTABLES, 6,1, FW_DEL,(char *)FW_IPFILTER,(char *)ARG_I,sp->if_name,"-j",(char *)FW_ACCEPT);
#endif
			}
#endif

			if(sp->dev){
				free(sp->dev);
				sp->dev = NULL;
			}
			if (sp->pp_lowerp){
				free(sp->pp_lowerp);
				sp->pp_lowerp = NULL;
			}

			free(sp);
			return 0;
		}

		sp = sp->pp_next;
	}
	return -1;
}

#endif

// Jenny, del ppp & add ppp for PPPoE discovery timeout several times
void reconnect_ppp(int unit)
{
	struct sppp *sp;
	struct spppreq *spr;

	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->over == SPPP_PPPOATM)
				return;
#ifdef CONFIG_USER_PPPOMODEM
			else if(sp->over == SPPP_PPPOMODEM)
				return;
#endif //CONFIG_USER_PPPOMODEM

			disc_counter[unit] = 0;
			// backup original ppp parameters
			spr = malloc(sizeof(struct spppreq));
			memset(spr, 0, sizeof(struct spppreq));
			spr->ppp.unit = sp->unit;
			spr->ppp.mode = sp->mode;
			spr->ppp.dgw = sp->dgw;
			spr->ppp.timeout = sp->timeout;
			spr->ppp.diska = sp->diska;
#ifdef IP_PASSTHROUGH
			spr->ppp.ippt_flag = sp->ippt_flag;
#endif
			spr->ppp.debug = sp->debug;
			spr->ppp.over = sp->over;
			spr->ppp.lcp.mru = sp->lcp.mru;
#ifdef _CWMP_MIB_
			spr->ppp.autoDisconnectTime = sp->autoDisconnectTime;
			spr->ppp.warnDisconnectDelay = sp->warnDisconnectDelay;
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
            spr->ppp.enable_pppoe_proxy=sp->enable_pppoe_proxy;
#endif
#ifdef CONFIG_SPPPD_STATICIP
			if(sp->staticip == 1) {
				spr->ppp.staticip = sp->staticip;
				spr->ppp.ipcp.myipaddr = sp->ipcp.myipaddr;
			}
#endif
			spr->ppp.myauth.proto = sp->myauth.proto;
			strcpy(spr->ppp.myauth.name, sp->myauth.name);
			strcpy(spr->ppp.myauth.secret, sp->myauth.secret);
			spr->dev = malloc(sizeof(struct pppoe_param_s));
			memcpy(spr->dev, sp->dev, sizeof(struct pppoe_param_s));
//			del_ppp(unit);
			add_ppp(unit, spr);
			free(spr->dev);
			free(spr);
		}
		sp = sp->pp_next;
	}
}

extern struct spppreq sprt;
extern struct pppoe_param_s poert;
extern struct pppoa_param_s poart;


int show_ppp(int unit)
{
struct sppp *sp;
struct pppoe_param_s *poer;
struct pppoa_param_s *poar;
char *encaps_str[] = {"VCMUX", "LLC"};
char *qos_str[] = {"none", "UBR", "CBR", "VBR"};
char *aal_str[] = {"AAL0", "AAL5"};

	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			sprt.dev = sp->dev;
			sprt.ppp.unit = unit;
			sprt.ppp.over = sp->over;
			sprt.ppp.mode = sp->mode;
			sprt.ppp.pp_phase = sp->pp_phase;
			strcpy(sprt.ppp.if_name, sp->if_name);
			memcpy(&sprt.ppp.lcp, &sp->lcp, sizeof(struct slcp));
			memcpy(&sprt.ppp.ipcp, &sp->ipcp, sizeof(struct sipcp));
			memcpy(&sprt.ppp.myauth, &sp->myauth, sizeof(struct sauth));
			memcpy(&sprt.ppp.hisauth, &sp->hisauth, sizeof(struct sauth));
			if(sprt.ppp.over == 0) {
				memcpy(&poart, sprt.dev, sizeof poart);
			}
			if(sprt.ppp.over == 1) {
				memcpy(&poert, sprt.dev, sizeof poert);
			}

			#if 0
			//////////////////////////////////////////////
			printf("%s\n", sprt.ppp.if_name);
			if(sprt.ppp.over == 0) {
				poar = sprt.dev;
				printf("PPPoATM PVC = %d.%d, %s, %s, Qos = %s \n", poar->addr.sap_addr.vpi, poar->addr.sap_addr.vci, \
					aal_str[poar->qos.aal],
					encaps_str[poar->encaps],
					qos_str[poar->qos.txtp.traffic_class]);
			}
			if(sprt.ppp.over == 1) {
				poer = sprt.dev;
				printf("PPPoE %s, phase = %d\n", poer->dev_name, poer->DiscoveryState);
				printf("MAC = %02X%02X%02X%02X%02X%02X, Server MAC = %02X%02X%02X%02X%02X%02X\n",
					poer->MyEthAddr[0], poer->MyEthAddr[1], poer->MyEthAddr[2], poer->MyEthAddr[3], poer->MyEthAddr[4], poer->MyEthAddr[5],
					poer->ACEthAddr[0], poer->ACEthAddr[1], poer->ACEthAddr[2], poer->ACEthAddr[3], poer->ACEthAddr[4], poer->ACEthAddr[5]);
			}

			printf("PPP PHASE = %d\n", sprt.ppp.pp_phase);
			printf("username = %s\n", sprt.ppp.myauth.name);
			printf("password = %s\n", sprt.ppp.myauth.secret);
			printf("MRU = %d\n", sprt.ppp.lcp.mru);
			printf("mode = %d\n", sprt.ppp.mode);
			#endif

			break;
		}
		sp = sp->pp_next;
	}
}


int init_ppp_unit(void)
{
	int i;
	int flags;
	int if_unit;

	for(i=0; i<N_SPPP_EXT; i++) {
		ppp_dev_fd[i] = open("/dev/ppp", O_RDWR);
   		if (ppp_dev_fd[i] < 0)
   			printf("Couldn't open /dev/ppp (%d)\n", i);
		fcntl(ppp_dev_fd[i], F_SETFD, fcntl(ppp_dev_fd[i], F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
		flags = fcntl(ppp_dev_fd[i], F_GETFL);
		if (flags == -1 || fcntl(ppp_dev_fd[i], F_SETFL, flags | O_NONBLOCK) == -1)
			printf("Couldn't set /dev/ppp to nonblock: %m");
		/* creates a new PPP interface and make /dev/ppp own the interface */
		if_unit = -1;
		if(ioctl(ppp_dev_fd[i], PPPIOCNEWUNIT, &if_unit) < 0) {
			printf("Couldn't create new ppp unit: %m");
		}
	}

    FD_ZERO(&in_fds);
    max_in_fd = 0;

}

int sameInterfaceType(struct sppp *sp, char* interface_name){
	#define SAME 1
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
	if(!sp || !interface_name)
		return SAME;//return SAME means go into original path, not changing anything

	//we only check pppoa and pppoe for "not the same"!
	if(sp->over == SPPP_PPPOATM) {
		if(strstr(interface_name, ALIASNAME_VC)){
			return SAME;
		}
	}
	else if(sp->over == SPPP_PPPOE) {
		poer = sp->dev;
		//printf("%s %d: %s, %s %d\n", __func__, __LINE__, poer->dev_name, interface_name, sizeof(ALIASNAME_VC));
		if(!strncmp(poer->dev_name, interface_name, strlen(interface_name) - 1)){//i don't want last "\0"
			//printf("%s %d\n", __func__, __LINE__);
			return SAME;
		}
	}
	else{
		//this means we only check pppoa and pppoe type, if sp is other type, we return SAME for not changing
		//original path
		return SAME;
	}
	return 0;
}


int isDslWan(struct sppp *sp){
	#define IS_DSL 1
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
	if(!sp)
		return 0;

	if(sp->over == SPPP_PPPOATM) {
		return IS_DSL;
	}
	else if(sp->over == SPPP_PPPOE) {
		poer = sp->dev;
		//printf("%s %d: %s\n", __func__, __LINE__, poer->dev_name);
		if(strstr(poer->dev_name, ALIASNAME_VC)){
			return IS_DSL;
		}
	}
	else{
		//other type? do nothing
	}
	return 0;
}

#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN) || defined (WLAN_WISP)
static int _ethPortLinkIsDown(char *dev_name)
{
	//printf("%s: get_net_link_status(%s)=%d\n",
	//	__FUNCTION__, dev_name, get_net_link_status(dev_name) );
	return (get_net_link_status(dev_name) != 1);
}

#ifdef CONFIG_ETHWAN
int ethWanLinkIsDown(void){
	return _ethPortLinkIsDown(ALIASNAME_NAS0);
}
#endif

#ifdef CONFIG_PTMWAN
int ptmWanLinkIsDown(void){
	return _ethPortLinkIsDown(ALIASNAME_PTM0);
}
#endif
#ifdef WLAN_WISP
int wlWanLinkIsDown(char *name){
	return _ethPortLinkIsDown(name);
}
#endif
int isEthWan(struct sppp *sp){
	#define IS_ETH 1
	#define IS_OTHER 0
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
	if(!sp)
		return IS_OTHER;

	if(sp->over == SPPP_PPPOATM) {
		return IS_OTHER;
	}
	else if(sp->over == SPPP_PPPOE) {
		poer = sp->dev;
		//printf("%s %d: %s\n", __func__, __LINE__, poer->dev_name);
		if(strstr(poer->dev_name, ALIASNAME_NAS)){
			return IS_ETH;
		}
	}
	else{
		//other type? do nothing
	}
	return IS_OTHER;
}

int isPtmWan(struct sppp *sp){
	#define IS_PTM 1
	#define IS_OTHER 0
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
	if(!sp)
		return IS_OTHER;

	if(sp->over == SPPP_PPPOATM) {
		return IS_OTHER;
	}
	else if(sp->over == SPPP_PPPOE) {
		poer = sp->dev;
		//printf("%s %d: %s\n", __func__, __LINE__, poer->dev_name);
		if(strstr(poer->dev_name, ALIASNAME_PTM)){
			return IS_PTM;
		}
	}
	else{
		//other type? do nothing
	}
	return IS_OTHER;
}
int isWlWan(struct sppp *sp){
	#define IS_WL 1
	#define IS_OTHER 0
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
	if(!sp)
		return IS_OTHER;

	if(sp->over == SPPP_PPPOATM) {
		return IS_OTHER;
	}
	else if(sp->over == SPPP_PPPOE) {
		poer = sp->dev;
		//printf("%s %d: %s\n", __func__, __LINE__, poer->dev_name);
		if(strstr(poer->dev_name, "wlan")){
			//printf("%s %d: %s\n", __func__, __LINE__, poer->dev_name);
			return IS_WL;
		}
	}
	else{
		//other type? do nothing
	}
	return IS_OTHER;
}

#endif
#define FILE_LOCK
int ppp_status(void)
{
	struct sppp *sp;
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
#ifdef CONFIG_USER_PPPOMODEM
	struct pppomodem_param_s *pomr;
#ifdef FILE_LOCK
	struct flock flpom;
	int fdpom;
#endif
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	struct vpn_param_s *ptpr;
	PPTP_DRV_CTRL *ptpctl;
#ifdef FILE_LOCK
	struct flock flptp;
	int fdptp;
#endif
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	struct vpn_param_s *l2tpr;
	L2TP_DRV_CTRL *l2tpctl;
#ifdef FILE_LOCK
	struct flock fll2tp;
	int fdl2tp;
#endif
#endif//endof CONFIG_USER_L2TPD_L2TPD
	PPPOE_DRV_CTRL *p;
	char buff[256];
	char sbuf[20], tbuf[20], overname[20];
	char *encaps_str[] = {"VCMUX", "LLC"};
	char *qos_str[] = {"none", "UBR", "CBR", "VBR"};
	char *aal_str[] = {"AAL0", "AAL5"};
	FILE *fp;
#ifdef FILE_LOCK
	struct flock flpoe, flpoa;
	int fdpoe, fdpoa;
#endif
	signal(SIGUSR2, SIG_IGN);
#ifdef FILE_LOCK
	// Jenny, file locking
	fdpoe = open("/var/ppp/pppoe.conf", O_RDWR);
	if (fdpoe != -1) {
		flpoe.l_type = F_WRLCK;
		flpoe.l_whence = SEEK_SET;
		flpoe.l_start = 0;
		flpoe.l_len = 0;
		flpoe.l_pid = getpid();
		if (fcntl(fdpoe, F_SETLKW, &flpoe) == -1)
			printf("pppoe write lock failed\n");
	}
	
#ifdef CONFIG_DEV_xDSL
	fdpoa = open("/var/ppp/pppoa.conf", O_RDWR);
	if (fdpoa != -1) {
		flpoa.l_type = F_WRLCK;
		flpoa.l_whence = SEEK_SET;
		flpoa.l_start = 0;
		flpoa.l_len = 0;
		flpoa.l_pid = getpid();
		if (fcntl(fdpoa, F_SETLKW, &flpoa) == -1)
			printf("pppoa write lock failed\n");
	}
#endif
	
#ifdef CONFIG_USER_PPPOMODEM
	fdpom = open("/var/ppp/pppom.conf", O_RDWR);
	if (fdpom != -1) {
		flpom.l_type = F_WRLCK;
		flpom.l_whence = SEEK_SET;
		flpom.l_start = 0;
		flpom.l_len = 0;
		flpom.l_pid = getpid();
		if (fcntl(fdpom, F_SETLKW, &flpom) == -1)
			printf("pppom write lock failed\n");
		//printf( "ppp_status: pppom write lock successfully\n" );
	}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	fdptp = open("/var/ppp/pptp.conf", O_RDWR);
	if (fdptp != -1) {
		flptp.l_type = F_WRLCK;
		flptp.l_whence = SEEK_SET;
		flptp.l_start = 0;
		flptp.l_len = 0;
		flptp.l_pid = getpid();
		if (fcntl(fdptp, F_SETLKW, &flptp) == -1)
			printf("pptp write lock failed\n");
		//printf( "ppp_status: pptp write lock successfully\n" );
	}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	fdl2tp = open("/var/ppp/l2tp.conf", O_RDWR);
	if (fdl2tp != -1) {
		fll2tp.l_type = F_WRLCK;
		fll2tp.l_whence = SEEK_SET;
		fll2tp.l_start = 0;
		fll2tp.l_len = 0;
		fll2tp.l_pid = getpid();
		if (fcntl(fdl2tp, F_SETLKW, &fll2tp) == -1)
			printf("l2tp write lock failed\n");
		//printf( "ppp_status: l2tp write lock successfully\n" );
	}
#endif//end of CONFIG_USER_L2TPD_L2TPD
#endif //FILE_LOCK

	fp = fopen("/var/ppp/ppp.conf", "w");
	if(fp) {
		sprintf(buff, "%-7s%-6s%-10s%-3s%-15s%-16s%16s%6s\n", \
			"if", "dev", "dev_v", "gw", "phase", "username", "password", "MRU");
		fputs(buff, fp);
		sp = spppq;
		while(sp) {
			if(sp->over == SPPP_PPPOATM) {
				poar = sp->dev;
				sprintf(sbuf, "%d.%d", poar->addr.sap_addr.vpi, poar->addr.sap_addr.vci);
				strcpy( overname, "PPPoA" );
			}
			else if(sp->over == SPPP_PPPOE && sp->role != 1) {
				poer = sp->dev;
				sprintf(sbuf, "%s", poer->dev_name);
				strcpy( overname, "PPPoE" );
			}
#ifdef CONFIG_USER_PPPOE_PROXY
			else if(sp->over == SPPP_PPPOE && sp->role == 1) {
				sprintf(sbuf, "ppp%d", sp->if_unit);
				strcpy( overname, "PPPoE ProxyServer" );
			}
#endif
#ifdef CONFIG_USER_PPPOMODEM
			else if(sp->over == SPPP_PPPOMODEM)
			{
				char *pname;
				pomr = sp->dev;
				//reduce the name length when showing
				pname=strrchr( pomr->devnam, '/' );
				if(pname)
					sprintf( sbuf, "%s", pname+1 );
				else if(pomr->devnam[0]==0)
					sprintf( sbuf, "%s", "N/A" );
				else
					sprintf( sbuf, "%s", pomr->devnam );
				strcpy( overname, "PPP" );//or PPPoModem
			}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
			else if (sp->over == SPPP_PPTP)
			{
				ptpr = sp->dev;
				sprintf(sbuf, "pptp%d", ptpr->index);
				strcpy(overname, "PPTP");
			}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
			else if (sp->over == SPPP_L2TP)
			{
				l2tpr = sp->dev;
				sprintf(sbuf, "l2tp%d", l2tpr->index);
				strcpy(overname, "L2TP");
			}
#endif//endof CONFIG_USER_L2TPD_L2TPD
			else{
				sbuf[0]=0;
				overname[0]=0;
			}
			sprintf(buff, "%-7s%-6s%-10s%-3d%-15s%-16s%16s%6d\n", \
				sp->if_name, overname, \
				sbuf, sp->dgw, sppp_phase_name(sp->pp_phase), \
				sp->myauth.name, sp->myauth.secret, \
				(sp->lcp.mru <= sp->lcp.their_mru ? sp->lcp.mru : sp->lcp.their_mru));
			fputs(buff, fp);
			sp = sp->pp_next;
		}
		fclose(fp);
	}

	fp = fopen("/var/ppp/pppoe.conf", "w");
	if(fp) {
		sprintf(buff, "%-8s%-10s%-15s %-12s %-12s %-17s %-17s\n", \
			"if", "dev", "phase", "MAC", "AC_MAC", "uptime", "totaluptime");
		fputs(buff, fp);
		sp = spppq;
		while(sp) {
			if(sp->over == SPPP_PPPOE && sp->role != 1) {
				poer = sp->dev;
				p = sp->pp_lowerp;
				sbuf[0] = '\0';
				tbuf[0] = '\0';
				sppp_uptime(sp, sbuf, tbuf);
				if(p) {
					sprintf(buff, "%-8s%-10s%-15s %02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X %-17s %-17s\n", \
						sp->if_name, p->name, _pppoe_state_name(p->DiscoveryState), \
						p->MyEthAddr[0], p->MyEthAddr[1], p->MyEthAddr[2], p->MyEthAddr[3], p->MyEthAddr[4], p->MyEthAddr[5], \
						p->ACEthAddr[0], p->ACEthAddr[1], p->ACEthAddr[2], p->ACEthAddr[3], p->ACEthAddr[4], p->ACEthAddr[5], sbuf, tbuf);
				}
				else {
					sprintf(buff, "%-8s%-10s%-15s %02X%02X%02X%02X%02X%02X %02X%02X%02X%02X%02X%02X %-17s %-17s\n", \
						sp->if_name, poer->dev_name, "Idle", \
						poer->MyEthAddr[0], poer->MyEthAddr[1], poer->MyEthAddr[2], poer->MyEthAddr[3], poer->MyEthAddr[4], poer->MyEthAddr[5], \
						poer->ACEthAddr[0], poer->ACEthAddr[1], poer->ACEthAddr[2], poer->ACEthAddr[3], poer->ACEthAddr[4], poer->ACEthAddr[5], sbuf, tbuf);
				}
				fputs(buff, fp);
			}
			sp = sp->pp_next;
		}
		fclose(fp);
	}	

#ifdef CONFIG_DEV_xDSL
	fp = fopen("/var/ppp/pppoa.conf", "w");
	if(fp) {
		sprintf(buff, "%-5s%-8s%-7s%-7s%-5s%-6s%-6s%-6s%-17s%-17s\n", \
			"if", "dev", "class", "encaps", "qos", "pcr", "scr", "mbs", "uptime", "totaluptime");
		fputs(buff, fp);
        	
		sp = spppq;
		while(sp) {
			if(sp->over == SPPP_PPPOATM) {
				poar = sp->dev;
				sbuf[0] = '\0';
				tbuf[0] = '\0';
				sppp_uptime(sp, sbuf, tbuf);
				sprintf(buff, "%-5s%-3d%-5d%-7s%-7s%-5s%-6d%-6d%-6d%-17s%-17s\n", \
					sp->if_name, poar->addr.sap_addr.vpi, poar->addr.sap_addr.vci, \
					poar->qos.aal==ATM_AAL0?aal_str[0]:aal_str[1], \
					encaps_str[poar->encaps], \
					qos_str[poar->qos.txtp.traffic_class], \
					poar->qos.txtp.pcr, \
					poar->qos.txtp.scr, \
					poar->qos.txtp.mbs, sbuf, tbuf);
				fputs(buff, fp);
			}
			sp = sp->pp_next;
		}
		fclose(fp);
	}	
#endif

#ifdef CONFIG_USER_PPPOMODEM
	fp = fopen("/var/ppp/pppom.conf", "w");
	if(fp) {
		sprintf(buff, "%-5s%-16s%-17s%-17s\n", \
			"if", "dev", "uptime", "totaluptime");
		fputs(buff, fp);
        	
		sp = spppq;
		while(sp) {
			if(sp->over == SPPP_PPPOMODEM) {
				char devnamebuf[20], *pname;
        	
				pomr = sp->dev;
				//reduce the name length when showing
				pname=strrchr( pomr->devnam, '/' );
				if(pname)
					sprintf( devnamebuf, "%s", pname+1 );
				else if(pomr->devnam[0]==0)
					sprintf( devnamebuf, "%s", "N/A" );
				else
					sprintf( devnamebuf, "%s", pomr->devnam );
        	
				sbuf[0] = '\0';
				tbuf[0] = '\0';
				sppp_uptime(sp, sbuf, tbuf);
				sprintf(buff, "%-5s%-16s%-17s%-17s\n", \
					sp->if_name, devnamebuf, sbuf, tbuf);
				fputs(buff, fp);
			}
			sp = sp->pp_next;
		}
		fclose(fp);
	}	
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	fp = fopen("/var/ppp/pptp.conf", "w");
	if(fp) {
		// Mason Yu. Add VPN ifIndex
		sprintf(buff, "%-7s%-16s%-17s%-17s%-17s\n", \
			"if", "dev", "uptime", "totaluptime", "status");
		if (NULL == fp)
			printf("can not open /var/ppp/pptp.conf\n");
		fputs(buff, fp);
        	
		sp = spppq;
		while(sp) {
			if((sp->over == SPPP_PPTP) && (sp->fd != -1)) {
				char devnamebuf[20];
        	
				ptpr = sp->dev;
				ptpctl = sp->pp_lowerp;
				if (!ptpr || !ptpctl)
					break;
        	
				if (ptpr->devname[0] != 0)
					sprintf( devnamebuf, "%s", ptpr->devname);
				else
					sprintf( devnamebuf, "pptp%d", ptpr->index );
        	
				sbuf[0] = '\0';
				tbuf[0] = '\0';
				sppp_uptime(sp, sbuf, tbuf);
				// Mason Yu. Add VPN ifIndex
				sprintf(buff, "%-7s%-16s%-17s%-17s%-17s\n", \
					sp->if_name, devnamebuf, sbuf, tbuf, pptp_state[ptpctl->callctrlState]);
				fputs(buff, fp);
			}
			sp = sp->pp_next;
		}
		fclose(fp);
	}	
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP

#ifdef FILE_LOCK
#ifdef CONFIG_USER_L2TPD_L2TPD
	fp = fopen("/var/ppp/l2tp.conf", "w");
	if(fp) {
		// Mason Yu. Add VPN ifIndex
		sprintf(buff, "%-7s%-16s%-17s%-17s%-17s\n", \
			"if", "dev", "uptime", "totaluptime", "status");
		if (NULL == fp)
			printf("can not open /var/ppp/l2tp.conf\n");
		fputs(buff, fp);
        	
		sp = spppq;
		while(sp) {
			if((sp->over == SPPP_L2TP) && (sp->fd != -1)) {
				char devnamebuf[20];
        	
				l2tpr = sp->dev;
				l2tpctl = sp->pp_lowerp;
				if (!l2tpr || !l2tpctl)
					break;
        	
				if (l2tpr->devname[0] != 0)
					sprintf( devnamebuf, "%s", l2tpr->devname);
				else
					sprintf( devnamebuf, "l2tp%d", l2tpr->index );
        	
				sbuf[0] = '\0';
				tbuf[0] = '\0';
				sppp_uptime(sp, sbuf, tbuf);
				// Mason Yu. Add VPN ifIndex
				sprintf(buff, "%-7s%-16s%-17s%-17s%-17s\n", \
					sp->if_name, devnamebuf, sbuf, tbuf, l2tp_state[l2tpctl->call_state]);
				fputs(buff, fp);
			}
			sp = sp->pp_next;
		}
		fclose(fp);
	}	
#endif //end of CONFIG_USER_L2TPD_L2TPD

	// Jenny, file unlocking
	if (fdpoe != -1) {
		flpoe.l_type = F_UNLCK;
		if (fcntl(fdpoe, F_SETLK, &flpoe) == -1)
			printf("pppoe write unlock failed\n");
		close(fdpoe);
	}

#ifdef CONFIG_DEV_xDSL
	if (fdpoa != -1) {
		flpoa.l_type = F_UNLCK;
		if (fcntl(fdpoa, F_SETLK, &flpoa) == -1)
			printf("pppoa write unlock failed\n");
		close(fdpoa);
	}
#endif
	
#ifdef CONFIG_USER_PPPOMODEM
	if (fdpom != -1) {
		flpom.l_type = F_UNLCK;
		if (fcntl(fdpom, F_SETLK, &flpom) == -1)
			printf("pppom write unlock failed\n");
		close(fdpom);
		//printf( "ppp_status: pppom write unlock successfully\n" );
	}
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	if (fdptp != -1) {
		flptp.l_type = F_UNLCK;
		if (fcntl(fdptp, F_SETLK, &flptp) == -1)
			printf("pptp write unlock failed\n");
		close(fdptp);
	}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	if (fdl2tp != -1) {
		fll2tp.l_type = F_UNLCK;
		if (fcntl(fdl2tp, F_SETLK, &fll2tp) == -1)
			printf("l2tp write unlock failed\n");
		close(fdl2tp);
	}
#endif //end of CONFIG_USER_L2TPD_L2TPD
#endif //FILE_LOCK

	signal(SIGUSR2, ppp_status);
	return 0;
}

// Jenny, for PPP connecting manually
int ppp_new(int unit, struct spppreq *spr)
{
	struct sppp *sp;

	del_ppp(unit);
	sp = malloc(sizeof(struct sppp));
	if(!sp)
		return -1;
	memset (sp, 0, sizeof (struct sppp));

	sp->up_flag = 1;
	sp->unit = unit;
	sp->mode = spr->ppp.mode;
	if(spr->ppp.dgw) {
		// Kaohj -- default route restriction should be done by user program
		//clear_dgw();
		sp->dgw = 1;
		sp->timeout = spr->ppp.timeout;
	}
	else {
		sp->dgw = 0;
		sp->timeout = 0;
	}

#ifdef IP_PASSTHROUGH
        if(spr->ppp.ippt_flag)
		sp->ippt_flag = spr->ppp.ippt_flag;
	 else
		sp->ippt_flag = 0;
#endif


	if (spr->ppp.diska)
		sp->diska = 1;
	else
		sp->diska = 0;
	sp->debug = spr->ppp.debug;
	sp->over = spr->ppp.over;
	if(sp->over == SPPP_PPPOATM) {
		sp->dev = malloc(sizeof(struct pppoa_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoa_param_s));
	}
	else
	if(sp->over == SPPP_PPPOE) {
		sp->dev = malloc(sizeof(struct pppoe_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppoe_param_s));
	}
#ifdef CONFIG_USER_PPPOMODEM
	else if(sp->over == SPPP_PPPOMODEM)
	{
		sp->dev = malloc(sizeof(struct pppomodem_param_s));
		if(!sp->dev)
			goto err;
		memcpy(sp->dev, spr->dev, sizeof(struct pppomodem_param_s));
		{
			/* set debug flag */
			struct pppomodem_param_s *p_pom=sp->dev;
			if(p_pom) p_pom->debug=spr->ppp.debug;
		}
	}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_L2TPD_L2TPD
	else if (sp->over == SPPP_L2TP)
	{
		sp->dev = malloc(sizeof(struct vpn_param_s));
		if (!sp->dev) {
			close(((struct vpn_param_s *)(spr->dev))->dev_fd);
			goto err;
		}
		memcpy(sp->dev, spr->dev, sizeof(struct vpn_param_s));
	}
#endif//endof CONFIG_USER_L2TPD_L2TPD
	else
		goto err;

	sp->lcp.mru = spr->ppp.lcp.mru;
	sp->myauth.proto = spr->ppp.myauth.proto;
	strcpy(sp->myauth.name, spr->ppp.myauth.name);
	strcpy(sp->myauth.secret, spr->ppp.myauth.secret);
	bzero((char*)&sp->hisauth, sizeof sp->hisauth);
	sp->dev_fd = ppp_dev_fd[unit];
	sp->if_unit = unit;
	sprintf(sp->if_name, "ppp%d", sp->if_unit);
	strcpy(spr->ppp.if_name, sp->if_name);
	sp->fd = -1;
#ifdef _CWMP_MIB_
	sp->autoDisconnectTime = spr->ppp.autoDisconnectTime;
	sp->warnDisconnectDelay = spr->ppp.warnDisconnectDelay;
#endif
#ifdef CONFIG_SPPPD_STATICIP
        //Jenny, static ip setting
	if(spr->ppp.staticip == 1) {
		sp->ipcp.myipaddr = spr->ppp.ipcp.myipaddr;
		sp->staticip = 1;
	}
#endif

   	sp->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
   	if (sp->sock_fd < 0) {
		if ( ! ok_error ( errno ))
    		printf("Couldn't create IP socket\n");
   	}

	fcntl(sp->sock_fd, F_SETFD, fcntl(sp->sock_fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
#ifdef CONFIG_IPV6
	sp->sock6_fd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sp->sock6_fd < 0)
		sp->sock6_fd = -errno;	/* save errno for later */
	fcntl(sp->sock6_fd, F_SETFD, fcntl(sp->sock6_fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
#endif
	sppp_attach(sp);
	if(sp->over == SPPP_PPPOE) {
		if (sp->fd < 0)
			sp->fd = pppoe_client_init(sp);
		pppoeDelete(sp);
		sp->fd = -1;
	}
	else if(sp->over == SPPP_PPPOATM) {
		struct pppoa_param_s *poar=sp->dev;
		if (poar->fd <= 0)
			sp->fd = pppoatm_init(sp->dev);
		else
			sp->fd = poar->fd;
		ppp_down_flag[sp->unit] = 1;
		//start_ppp(sp);
		//ppp_down(sp->unit);
	}
#ifdef CONFIG_USER_PPPOMODEM
	else if(sp->over == SPPP_PPPOMODEM)
	{
		sp->fd = -1;
	}
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_L2TPD_L2TPD
	else if (sp->over == SPPP_L2TP)
	{
		sp->fd = -1;
	}
#endif

	ppp_status();
#ifdef DIAGNOSTIC_TEST
	sppp_diag_log(sp->if_unit, 0);	// Jenny, diagnostic test log init
#endif
	sppp_last_connection_error(sp->if_unit, ERROR_UNKNOWN);	// Jenny, last connection error init
//	printf("%s connection manually OK!!\n", sp->if_name);
	pppoe_up_log(unit, 1);
	return 1;

err:
	printf("new ppp() error!!\n");
	if(sp->fd>=0)
	{
		close(sp->fd);
		sp->fd = -1;
	}
	if(sp->sock_fd>=0) {
		close(sp->sock_fd);
		sp->sock_fd = -1;
	}
#ifdef CONFIG_IPV6
	if(sp->sock6_fd>=0) {
		close(sp->sock6_fd);
		sp->sock6_fd = -1;
	}
#endif
	if(sp->dev)
		free(sp->dev);
	free(sp);
	return -1;
}

// Jenny, for PPP connecting manually
int ppp_up(int unit)
{
	struct sppp *sp;
	PPPOE_DRV_CTRL *p;

	if (ppp_up_flag[unit])
		return 1;
//	ppp_up_flag[unit] = 1;
	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->pp_phase != PHASE_DEAD)
				return 1;
			ppp_up_flag[unit] = 1;
			// manually force up, reconnect immediately.
			sp->holdoff_delay = 0;
//			ppp_down(unit);
			if(sp->over == SPPP_PPPOE)
				p = sp->pp_lowerp;
			ppp_down_flag[sp->unit] = 0;
#ifdef CONFIG_USER_PPPOMODEM //paula, 3g backup PPP
			if(sp->over!=SPPP_PPPOMODEM)
				manual_down_flag = 0;
#endif
			break;
		}
		sp = sp->pp_next;
	}

	dial_setup(sp);
	if(!start_ppp(sp))
		return 0;

	printf("!!%s up error!!\n", sp->if_name);
	return -1;
}

// Jenny, for PPP disconnecting manually
int ppp_down(int unit)
{
	struct sppp *sp;
	struct pppoa_param_s *poar;

	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->pp_phase == PHASE_DEAD)
				return 1;
			ppp_down_flag[sp->unit] = 1;
			sp->pp_down(sp);
			if(sp->over == SPPP_PPPOATM) {
				poar = sp->dev;
				remove_fd(poar->fd);
				remove_fd(sp->dev_fd);
			}
#if defined(CONFIG_USER_PPPOMODEM) || defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
			else if ( 0
#ifdef CONFIG_USER_PPPOMODEM
				|| (sp->over == SPPP_PPPOMODEM)
#endif//endof CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
				|| (sp->over == SPPP_PPTP)
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
				|| (sp->over == SPPP_L2TP)
#endif//endof CONFIG_USER_L2TPD_L2TPD
				)
			{
				unsigned int flags;

#ifdef CONFIG_USER_PPPOMODEM
				if (sp->over == SPPP_PPPOMODEM) {
					printf( "%s: sp->over == SPPP_PPPOMODEM\n", __FUNCTION__ );
					pppomodem_close(sp);
				}
#endif

				ioctl(ppp_dev_fd[sp->if_unit], PPPIOCGFLAGS, (caddr_t)&flags);
				if (flags & SC_LOOP_TRAFFIC)
					flags &= ~SC_LOOP_TRAFFIC;
				ioctl(ppp_dev_fd[sp->if_unit], PPPIOCSFLAGS, (caddr_t)&flags);

				/* disconnect channel to PPP interface */
				if (sp->fd >= 0) {
					if (ioctl(sp->fd, PPPIOCDISCONN, &sp->if_unit) < 0)
						printf("Couldn't disconnect PPP unit %d: %m", sp->if_unit);

					if (ioctl(sp->fd, PPPIOCDETACH, &sp->chindex) < 0)
						printf("Couldn't detach channel %d: %m", sp->chindex);
				}

				if(sp->dev_fd >= 0)
					remove_fd(sp->dev_fd);
				if(sp->fd>=0)
					remove_fd(sp->fd);
				if(sp->fd>=0) {
					close(sp->fd);
					sp->fd = -1;
				}
			}
#endif //CONFIG_USER_PPPOMODEM || CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD


//			stop_ppp(sp);
			ppp_status();
//			printf("%s down OK!!\n", sp->if_name);
			ppp_up_flag[sp->if_unit] = 0;
			return 0;
		}
		sp = sp->pp_next;
	}

	//printf("!!%s down error!!\n", sp->if_name);
	return -1;
}


// Mason YU, for PPP connecting manually of Remote Manage
int rm_pppoe_test(int unit, struct spppreq *spr)
{
	struct sppp *sp;
	struct pppoa_param_s *poar;
	struct pppoe_param_s *poer;
	PPPOE_DRV_CTRL *p;

	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			if (sp->pp_phase == PHASE_NETWORK)
				return 1;
			if(sp->over == SPPP_PPPOATM) {
				poar = sp->dev;
				ppp_down_flag[sp->unit] = 0;
			}
			if(sp->over == SPPP_PPPOE) {
				poer = sp->dev;
				p = sp->pp_lowerp;
				if (poer->DiscoveryState == PPPOE_STATE_SESSION)
					return 1;
			}
			break;
		}
		sp = sp->pp_next;
	}

	// Change username ans password
	strcpy(sp->myauth.name, spr->ppp.myauth.name);
	strcpy(sp->myauth.secret, spr->ppp.myauth.secret);

	dial_setup(sp);
	if(!start_ppp(sp))
		return 0;

	printf("!!%s up error!!\n", sp->if_name);
	return -1;
}

//int debug_set(int unit)
int debug_set(int unit, struct spppreq *spr)
{
	struct sppp *sp;

	sp = spppq;
	while(sp) {
		if(sp->unit == unit) {
			/*if (sp->debug == 1)
				sp->debug = 0;
			else
				sp->debug = 1;*/
			sp->debug = spr->ppp.debug;
#ifdef CONFIG_USER_PPPOMODEM
			if(sp->over == SPPP_PPPOMODEM)
			{
				/* set debug flag */
				struct pppomodem_param_s *p_pom=sp->dev;
				if(p_pom) p_pom->debug=spr->ppp.debug;
			}
#endif //CONFIG_USER_PPPOMODEM
			sppp_debug_log(sp->unit, sp->debug);
			return 0;
		}
		sp = sp->pp_next;
	}
	return 0;
}

#ifdef ENABLE_PPP_SYSLOG
void syslog_set(int dbg)
{
	FILE *fp;
	if (fp = fopen(PPP_SYSLOG, "w")) {
		fprintf(fp, "%d", dbg);
		fclose(fp);
	}
}
#endif

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
extern struct vpn_param_s vpnrt;
/*
 * Get Interface Addr (MAC, IP, Mask)
 */
int getItfAddr( char *ifname, void *pAddr )
{
	struct ifreq ifr;
	int skfd;
	struct sockaddr_in *addr;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0) {
		addr = ((struct sockaddr_in *)&ifr.ifr_addr);
		*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
		close(skfd);
		return 0;
	}

	close(skfd);

	return -1;
}


int init_vpn_server(void)
{
	memset(pppd_account, 0, sizeof(pppd_account));
	memset(pppd_auth, 0, sizeof(pppd_auth));

	FD_ZERO(&pptpConnSet);
	maxPptpConnFd = 0;
}

int assignedIPToPppdClient(int over, int *myIP, int *hisIP, int *primaryDNS)
{
	int peerip, localip;
	int i, index, otherIdx;
	int idx;

	if(SPPP_PPTPD == over){
		index = PPTPD_AUTH_IDX;
		otherIdx = L2TPD_AUTH_IDX;
	}
	else if(SPPP_L2TPD == over){
		index = L2TPD_AUTH_IDX;
		otherIdx = PPTPD_AUTH_IDX;
	}

	assert(pppd_auth[index].localip!=0);
	assert(pppd_auth[index].peerip!=0);

	localip = pppd_auth[index].localip;
	peerip  = pppd_auth[index].peerip;

	for (i=0; i<32; i++)
	{
		if (!(pppd_auth[index].assignedIpMap & (1<<i)))/* this Address has not been assigned */
		{
			pppd_auth[index].assignedIpMap |= (1<<i);

			peerip = pppd_auth[index].peerip + i;
			if(0xff == (peerip & 0xff)){
				pppd_auth[index].assignedIpMap &= (~(1<<i));
				printf("%s peerip should not be *.*.*.255 \n", __func__);
				return -1;
			}
			if (peerip == localip)
				continue;
			idx = peerip - pppd_auth[otherIdx].peerip;
			if((idx >= 0) && (idx < 32)){
				if((pppd_auth[otherIdx].assignedIpMap & (1<<idx)))/* this Address has been assigned by other srever*/
				{
					pppd_auth[index].assignedIpMap &= (~(1<<i));
					continue;
				}
			}

			break;
		}
	}
	if (i >= 32) {
		printf("%s too many clients.\n", __func__);
		return -1;
	}

	*myIP = localip;
	*hisIP = peerip;

	if (getItfAddr("br0", primaryDNS) < 0)
		printf("%s get br0 address fail.\n", __func__);
	return 0;
}

int releasePppdClientIP(int over, int hisIP)
{
	int idx;
	int index;
	if(SPPP_PPTPD == over)
		index = PPTPD_AUTH_IDX;
	else if(SPPP_L2TPD == over)
		index = L2TPD_AUTH_IDX;

	printf("%s %x startip:%x\n", __func__, hisIP, pppd_auth[index].peerip);
	if (0 == hisIP)
		return 0;

	idx = hisIP - pppd_auth[index].peerip;
	if (idx >= 32) {
		printf("%s unknown client IP %s\n", __func__, inet_ntoa(*(struct in_addr *)&hisIP));
		return -1;
	}
	pppd_auth[index].assignedIpMap &= (~(1<<idx));
	return 0;
}

int get_pppd_account(struct sppp *sp, char *name, char *passwd, int name_len, int passwd_len)
{
#ifdef CONFIG_USER_PPTPD_PPTPD
	PPTPD_DRV_CTRL *p_pptp;
#endif
#ifdef CONFIG_USER_L2TPD_LNS
	L2TPD_DRV_CTRL *p_l2tp;
#endif
	int i;

	for (i=0; i<MAX_VPN_SERVER_ACCOUNT; i++)
	{
		if (0 == pppd_account[i].valid)
			continue;

		if(pppd_account[i].type != sp->over)
			continue;

		if ((strlen(pppd_account[i].auth_name) == name_len) &&
			(strlen(pppd_account[i].auth_secret) == passwd_len) &&
			!memcmp(pppd_account[i].auth_name, name, name_len) &&
			!memcmp(pppd_account[i].auth_secret, passwd, passwd_len))
			break;
	}

	if (i<MAX_VPN_SERVER_ACCOUNT) {/* found an account */
#ifdef CONFIG_USER_PPTPD_PPTPD
		if(SPPP_PPTPD == sp->over){
			p_pptp = sp->pp_lowerp;
			assert(p_pptp!=NULL);
			p_pptp->account_idx = i;
		}
#endif
#ifdef CONFIG_USER_L2TPD_LNS
		if(SPPP_L2TPD == sp->over){
			p_l2tp = sp->pp_lowerp;
			assert(p_l2tp!=NULL);
			p_l2tp->account_idx = i;
		}
#endif
		memcpy(sp->hisauth.name, name, name_len);
		sp->hisauth.name[name_len] = 0;
		memcpy(sp->hisauth.secret, passwd, passwd_len);
		sp->hisauth.secret[passwd_len] = 0;

		return 0;
	}

	return -1;
}

int get_pppd_account_by_idx(struct sppp *sp, int idx)
{
#ifdef CONFIG_USER_PPTPD_PPTPD
	PPTPD_DRV_CTRL *p_pptp;
#endif
#ifdef CONFIG_USER_L2TPD_LNS
	L2TPD_DRV_CTRL *p_l2tp;
#endif

	assert(sp!=NULL);

	if (idx >= MAX_VPN_SERVER_ACCOUNT)
		return -1;

	if (0 == pppd_account[idx].valid)
		return 0;

	if(pppd_account[idx].type != sp->over)
		return 0;

#ifdef CONFIG_USER_PPTPD_PPTPD
	if(SPPP_PPTPD == sp->over){
		p_pptp = sp->pp_lowerp;
		assert(p_pptp!=NULL);
		p_pptp->account_idx = idx;
	}
#endif
#ifdef CONFIG_USER_L2TPD_LNS
	if(SPPP_L2TPD == sp->over){
		p_l2tp = sp->pp_lowerp;
		assert(p_l2tp!=NULL);
		p_l2tp->account_idx = idx;
	}
#endif
	memcpy(sp->hisauth.name, pppd_account[idx].auth_name, AUTHNAMELEN);
	memcpy(sp->hisauth.secret, pppd_account[idx].auth_secret, AUTHKEYLEN);

	return 1;
}

int add_pppd_account(int idx, struct spppreq *spr)
{
	int authIdx;
	int i;

	pppd_account[idx].valid = 1;
	memcpy(pppd_account[idx].auth_name, spr->ppp.myauth.name, AUTHNAMELEN);
	memcpy(pppd_account[idx].auth_secret, spr->ppp.myauth.secret, AUTHKEYLEN);


	if(SPPP_PPTPD == pppd_account[idx].type)
		authIdx = PPTPD_AUTH_IDX;
	else if(SPPP_L2TPD == pppd_account[idx].type)
		authIdx = L2TPD_AUTH_IDX;
	pppd_auth[authIdx].auth_proto = spr->ppp.myauth.proto;
	pppd_auth[authIdx].chal_type  = spr->ppp.myauth.chal_type;
	pppd_auth[authIdx].enc_type   = spr->ppp.myauth.enc_type;
#ifdef CONFIG_USER_L2TPD_LNS
	pppd_auth[authIdx].tunnel_auth = vpnrt.tunnel_auth;
	memcpy(pppd_auth[authIdx].tunnel_secret, vpnrt.secret, MAX_NAME_LEN);
#endif
	pppd_auth[authIdx].localip    = spr->ppp.ipcp.myipaddr;
	pppd_auth[authIdx].peerip     = spr->ppp.ipcp.hisipaddr;

#ifdef CONFIG_USER_PPTPD_PPTPD
	if (SPPP_PPTPD == pppd_account[idx].type && noAccount[PPTPD_AUTH_IDX]) {
		if (createPptpHostSocket(&pptp_host_socket) < 0)
			printf("%s %d create pptp host socket fail!\n", __func__, __LINE__);

		FD_SET(pptp_host_socket, &pptpConnSet);
		if (pptp_host_socket > maxPptpConnFd)
			maxPptpConnFd = pptp_host_socket;
		//printf("maxConnFd=%d pptp_host_socket=%d\n", maxPptpConnFd, pptp_host_socket);

		noAccount[PPTPD_AUTH_IDX] = 0;
	}
#endif
#ifdef CONFIG_USER_L2TPD_LNS
	if (SPPP_L2TPD == pppd_account[idx].type && noAccount[L2TPD_AUTH_IDX]) {
		l2tp_server_init(noAccount[L2TPD_AUTH_IDX]);
		noAccount[L2TPD_AUTH_IDX] = 0;
	}
#endif
}

struct optstring {
	int valid;
	union {
		char *value;
	} optstring_u;
};
typedef struct optstring optstring;
#define OPTSTRING(_var) _var.optstring_u.value

int del_pppd_account(int idx)
{
#ifdef CONFIG_USER_PPTPD_PPTPD
	PPTPD_DRV_CTRL *p_pptp;
#endif
#ifdef CONFIG_USER_L2TPD_LNS
	L2TPD_DRV_CTRL *p_l2tp;
#endif
	struct sppp *sp, *sq;
	int i, type, index;
	int deleteAll = 0;

	type = pppd_account[idx].type;
	if(SPPP_PPTPD == type)
		index = PPTPD_AUTH_IDX;
	else if(SPPP_L2TPD == type)
		index = L2TPD_AUTH_IDX;

	for (i=0; i<MAX_VPN_SERVER_ACCOUNT; i++)
	{
		if (idx == i)
			continue;

		if (pppd_account[i].valid && (pppd_account[i].type == type))
			break;
	}

	if (i >= MAX_VPN_SERVER_ACCOUNT)
		noAccount[index] = 1;

	if (noAccount[index]) {
		deleteAll = 1;
		pppd_auth[index].auth_proto = 0;
		pppd_auth[index].chal_type = 0;
		pppd_auth[index].enc_type = 0;

#ifdef CONFIG_USER_PPTPD_PPTPD
		if(SPPP_PPTPD == type){
			if (pptp_host_socket >= 0) {
				FD_CLR(pptp_host_socket, &pptpConnSet);
				close(pptp_host_socket);
				pptp_host_socket = -1;
				maxPptpConnFd = -1;
			}
		}
#endif
#ifdef CONFIG_USER_L2TPD_LNS
		if(SPPP_L2TPD == type){
			l2tp_server_close();
		}
#endif
	}

	/* release account idx related pppd */
	sq = spppq;
	while(sp = sq) {
		sq = sq->pp_next;
#ifdef CONFIG_USER_PPTPD_PPTPD
		if(SPPP_PPTPD == sp->over){
			p_pptp = sp->pp_lowerp;
			if ((1 == deleteAll) || (p_pptp->account_idx == idx)) {
				del_pppd(sp->unit);
			}
		}
#endif
#ifdef CONFIG_USER_L2TPD_LNS
		if(SPPP_L2TPD == sp->over){
			p_l2tp = sp->pp_lowerp;
			int result;
			optstring tunnel_name;
			optstring session_name;
			if ((1 == deleteAll) || (p_l2tp->account_idx == idx)) {
				optstring reason={.valid = 0,};
				OPTSTRING(tunnel_name) = "";
				OPTSTRING(session_name) = "";
				l2tp_session_delete_1_svc(p_l2tp->tunnel_id, tunnel_name, p_l2tp->session_id, session_name, reason, &result, NULL);
				if(0 == l2tp_tunnel_has_session(p_l2tp->tunnel_id)){
					l2tp_tunnel_delete_1_svc(p_l2tp->tunnel_id, tunnel_name, reason, &result, NULL);
				}
				del_pppd(sp->unit);
			}
		}
#endif
	}
	/***********END*************/

	return 1;
}
#else
int assignedIPToPppdClient(int over, int *myIP, int *hisIP, int *primaryDNS)
{
	return -1;
}

int get_pppd_account_by_idx(struct sppp *sp, int idx)
{
	return -1;
}
#endif

