
#include "if_sppp.h"
#include "pppoe.h"
#include "pppoa.h"
#ifdef CONFIG_USER_PPPOMODEM
#include "pppomodem.h"
#endif //CONFIG_USER_PPPOMODEM
#include "signal.h"
#include <rtk/utility.h>
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#include "pptp.h"
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
#include "vpn.h"
#endif//endof CONFIG_USER_L2TPD_L2TPD
// netlink event
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define SPPPD_RUNFILE		"/var/run/spppd.pid"
//N_SPPP has defined in if_sppp.h
//#define N_SPPP 8

#ifdef CONFIG_NO_REDIAL
int  no_redial = 0;
#endif
// Specifies how many seconds to wait before re-initiating the link after it terminates.
int holdoff_interval;

struct spppreq sprt;
struct pppoe_param_s poert;
struct pppoa_param_s poart;
#ifdef CONFIG_USER_PPPOMODEM
struct pppomodem_param_s pomrt;
#endif //#ifdef CONFIG_USER_PPPOMODEM
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
struct vpn_param_s vpnrt;
extern int ppp_dev_fd[N_SPPP];
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD
#if defined(CONFIG_USER_PPTPD_PPTPD) ||defined(CONFIG_USER_L2TPD_LNS)
extern struct VPN_ACCOUNT_ST pppd_account[MAX_VPN_SERVER_ACCOUNT];
int vpn_server_idx = -1;
#endif //endof CONFIG_USER_PPTPD_PPTPD

#ifdef CONFIG_USER_PPPOE_PROXY
int ppp_proxy_unit = -1;
#endif

// Kaohj, list of outstanding PPPoE request
//struct spppreq *sprt_list = NULL;
//int in_pppoe_disc = 0;

#define pppoatm_overhead(x) (x ? 6 : 2)

void dial_start(int signum);

int add_cfg_ppp(char *arg)
{
	sprt.cmd = 0;
	sprt.ppp.unit = atoi(arg);
	sprt.ppp.debug = 0;
	sprt.ppp.lcp.mru = PP_MTU;
}

int del_cfg_ppp(char *arg)
{
	sprt.cmd = 1;
	sprt.ppp.unit = atoi(arg);
}

int show_cfg_ppp(char *arg)
{
	sprt.cmd = 2;
	sprt.ppp.unit = atoi(arg);
}

// Added by Jenny for PPP connect manually
int up_cfg_ppp(char *arg)
{
	sprt.cmd = 3;
	sprt.ppp.unit = atoi(arg);
}

// Added by Jenny for PPP disconnect manually
int down_cfg_ppp(char *arg)
{
	sprt.cmd = 4;
	sprt.ppp.unit = atoi(arg);
}

// Added by Jenny for PPP connect manually
int new_cfg_ppp(char *arg)
{
	sprt.cmd = 5;
	sprt.ppp.unit = atoi(arg);
	sprt.ppp.debug = 0;
	sprt.ppp.lcp.mru = PP_MTU;
}



// Jenny, keepalive timer setting
int keepalive_cfg(char *arg)
{
	sprt.cmd = 6;
	if (atoi(arg)!=0) {
		keepalive_timer = atoi(arg);
#ifdef _CWMP_MIB_
		sppp_lcp_echo_log();
#endif
		return 0;
	}
	return -1;
}


// Added by Mason Yu for Remote Management
int set_rm_pppoe_test(char *arg)
{
	sprt.cmd = 7;
	sprt.ppp.unit = atoi(arg);
}

int reconnect_cfg_ppp(char *arg)
{
	sprt.cmd = 8;
	sprt.ppp.unit = atoi(arg);
}

int dev_pppoe(char *arg)
{
	FILE *fp;
	int in_turn=0;
	char	buff[64], tmp1[20];
	/* device is PPPoE */
	sprt.ppp.over = 1;
	sprt.dev = &poert;
	strcpy(poert.dev_name, arg);
#ifdef CONFIG_RTL8672_SAR
	if (fp=fopen("/proc/net/atm/br2684", "r")) {
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (in_turn==0) {
				sscanf(buff, "%*s%s", tmp1);
				tmp1[strlen(tmp1)-1]='\0';
				if (strcmp(arg, tmp1) != 0) {
					in_turn ^= 0x01;
					fgets(buff, sizeof(buff), fp);
				}
			}
			else {
				sscanf(buff, "%*s%s", tmp1);
				sscanf(tmp1, "0.%hu.%hu:", &poert.vpi, &poert.vci);
				break;
			}
			in_turn ^= 0x01;
		}
		fclose(fp);
	}
#endif
	return 0;
}

int dev_pppoa(char *arg)
{
	sprt.ppp.over = 0;
	/* device is PPPoA */
	sprt.dev = &poart;

	memset(&poart.addr, 0, sizeof(struct sockaddr_atmpvc));
	if (text2atm(arg, &poart.addr, sizeof(struct sockaddr_atmpvc), T2A_PVC | T2A_NAME) < 0)
		return -1;

	/* default value */
	poart.encaps = 1;
	memset(&poart.qos, 0, sizeof(struct atm_qos));
	poart.qos.txtp.traffic_class = poart.qos.rxtp.traffic_class = ATM_UBR;
	if (text2qos("ubr:pcr=7600", &poart.qos, 0)) {
		printf("Can't parse QoS: %s", arg);
		return -1;
	}
	poart.qos.txtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
	poart.qos.rxtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
	poart.qos.aal = ATM_AAL5;

	return 0;
}

#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
int create_ppp_dev(void)
{
	int flags;
	int if_unit=-1;

	vpnrt.dev_fd = open("/dev/ppp", O_RDWR);
	if (vpnrt.dev_fd < 0) {
		printf("Couldn't open /dev/ppp\n");
		return -1;
	}

	flags = fcntl(vpnrt.dev_fd, F_GETFL);
	if (flags == -1 || fcntl(vpnrt.dev_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		printf("Couldn't set /dev/ppp to nonblock\n");
		return -1;
	}

	/* creates a new PPP interface and make /dev/ppp own the interface */
	if(ioctl(vpnrt.dev_fd, PPPIOCNEWUNIT, &if_unit) < 0) {
		printf("Couldn't create new ppp unit\n");
		return -1;
	}

	sprt.ppp.unit = if_unit;
	ppp_dev_fd[if_unit] = vpnrt.dev_fd;

	return if_unit;
}
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
int dev_pptp(char *arg)
{
	int if_unit;

	// Mason Yu. Add VPN ifIndex
	// unit declarations for ppp
	// (1) 0 ~ 7: pppoe/pppoa, (2) 8: 3G, (3) 9 ~ 10: PPTP, (4) 11 ~12: L2TP
	if (0 == sprt.cmd)//add
	{
		sprt.ppp.over = SPPP_PPTP;
		/* device is PPtP */
		sprt.dev = &vpnrt;
		// Mason Yu. Add VPN ifIndex
		vpnrt.index = sprt.ppp.unit-9;
		sprintf(vpnrt.devname, "pptp%d", vpnrt.index);
		//printf("PPTP dev index=%d\n", vpnrt.index);
		/*
		if_unit = create_ppp_dev();
		if (if_unit != -1)
			printf("ppp%d create for PPtP.\n", if_unit);
		*/
	}
	else if (1 == sprt.cmd)//del
	{
		//cxy 2015-3-2: no need to set sprt.ppp.unit. it is setted in del_cfg_ppp.
		//sprt.ppp.unit = get_ppp_unit_by_index(sprt.ppp.unit-9);	// Mason Yu. Add VPN ifIndex
		printf("delete ppp%d\n", sprt.ppp.unit);
	}

	return 0;
}
#endif

#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
int set_vpn_server(char *arg)
{
	if (arg) {
		strcpy(vpnrt.server, arg);
		return 0;
	}
	return -1;
}
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TP_L2TP

#if defined(CONFIG_USER_PPTPD_PPTPD) ||defined(CONFIG_USER_L2TPD_LNS)
int add_vpn_account(char *arg)
{
	int i;
	int selected = -1;

	vpn_server_idx = -1;

	if (!arg)
		return -1;

	for (i=0; i<MAX_VPN_SERVER_ACCOUNT; i++)
	{
		if (pppd_account[i].valid && !strcmp(pppd_account[i].name, arg)) {
			printf("account %s has already existed.\n", arg);
			return -1;
		}

		if (!pppd_account[i].valid && (-1 == selected))
			selected = i;
	}

	if (selected != -1) {
		printf("%s %d add account success.\n", __func__, __LINE__);
		vpn_server_idx = selected;
		strcpy(pppd_account[vpn_server_idx].name, arg);
		sprt.cmd = 15;
		return 0;
	}

	return -1;
}

int del_vpn_account(char *arg)
{
	int i;

	for (i=0; i<MAX_VPN_SERVER_ACCOUNT; i++)
	{
		if (!strcmp(pppd_account[i].name, arg))
			break;
	}

	if (i<MAX_VPN_SERVER_ACCOUNT) {
		pppd_account[i].valid = 0;
		vpn_server_idx = i;
		sprt.cmd = 16;
	}
}

int set_vpn_type(char *arg)
{
	if (-1 == vpn_server_idx)
		return -1;

	if (arg) {
		if (!strcmp(arg, "PPTP"))
			pppd_account[vpn_server_idx].type = SPPP_PPTPD;
		else if (!strcmp(arg, "L2TP")){
			pppd_account[vpn_server_idx].type = SPPP_L2TPD;
		}
	}
	return -1;
}

int set_vpn_localip(char *arg)
{
	struct in_addr addr;

	if (arg) {
		inet_aton(arg, &addr);
		sprt.ppp.ipcp.myipaddr = addr.s_addr;
	}
	return -1;
}

int set_vpn_peerip(char *arg)
{
	struct in_addr addr;

	if (arg) {
		inet_aton(arg, &addr);
		sprt.ppp.ipcp.hisipaddr = addr.s_addr;
	}
	return -1;
}
#endif

#ifdef CONFIG_USER_L2TPD_L2TPD
int dev_l2tp(char *arg)
{
	int if_unit;

	// Mason Yu. Add VPN ifIndex
	// unit declarations for ppp
	// (1) 0 ~ 7: pppoe/pppoa, (2) 8: 3G, (3) 9 ~ 10: PPTP, (4) 11 ~12: L2TP
	// Mason Yu.
	if (0 == sprt.cmd || 5 == sprt.cmd)//add or new
	{
		sprt.ppp.over = SPPP_L2TP;
		/* device is L2tP */
		sprt.dev = &vpnrt;
		// Mason Yu. Add VPN ifIndex
		vpnrt.index = sprt.ppp.unit-11;
		sprintf(vpnrt.devname, "l2tp%d", vpnrt.index);
		//printf("L2TP dev index=%d\n", vpnrt.index);
		/*
		if_unit = create_ppp_dev();
		if (if_unit != -1)
			printf("ppp%d create for L2TP.\n", if_unit);
		*/
	}
	else if (1 == sprt.cmd)//del
	{
		//cxy 2015-3-2: no need to set sprt.ppp.unit. it is setted in del_cfg_ppp.
		//sprt.ppp.unit = get_ppp_unit_by_index(sprt.ppp.unit-11);	// Mason Yu. Add VPN ifIndex
		printf("delete ppp%d\n", sprt.ppp.unit);
	}

	return 0;
}

int set_l2tp_tunnel_auth(char *arg)
{
	char *start=arg;

	if(arg) {
		while (*start) {
			*start = tolower(*start);
			start++;
		}
		if (strcmp("challenge", arg)==0)
			vpnrt.tunnel_auth = 1;
		else
			vpnrt.tunnel_auth = 0;

		return 0;
	}
	return -1;
}

int set_l2tp_secret(char *arg)
{
	if (arg)
	{
		strcpy(vpnrt.secret, arg);
		return 0;
	}
	return -1;
}
#endif

//#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_PPP_MPPE_MPPC
int set_encrypt(char *arg)
{

	if(arg) {
//printf("encryption %s\n",arg);
        if(sprt.ppp.myauth.chal_type == CHAP_MICROSOFT_V2)
        {

		if (strcmp("none", arg)==0)
			//sprt.ppp.myauth.chal_type= 0x0;
			sprt.ppp.myauth.enc_type = 0x0;
		else if (strcmp("+MPPE", arg)==0)
			sprt.ppp.myauth.enc_type = MPPE_TYPE;
        else if (strcmp("+MPPC", arg)==0)
			sprt.ppp.myauth.enc_type = MPPC_TYPE;
        else if (strcmp("+BOTH", arg)==0)
			sprt.ppp.myauth.enc_type = MPPE_MPPC_TYPE;
		else
			return -1;
		}
		else
		    sprt.ppp.myauth.enc_type = 0;//MPPE_MPPC_TYPE;
		return 0;
	}
	return -1;

}
#endif//endof CONFIG_PPP_MPPE_MPPC
//#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP

int set_pppoa_encaps(char *arg)
{
	if(sprt.dev == &poart) {
		poart.encaps = atoi(arg);
		return 0;
	}
	return -1;
}

int set_pppoa_qos(char *arg)
{
	if(sprt.dev == &poart) {
  		memset(&poart.qos, 0, sizeof(struct atm_qos));
  		poart.qos.txtp.traffic_class = poart.qos.rxtp.traffic_class = ATM_UBR;
   		if (text2qos(arg, &poart.qos, 0)) {
   			printf("Can't parse QoS: %s", arg);
   			return -1;
   		}
		poart.qos.txtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
		poart.qos.rxtp.max_sdu = 1500 + pppoatm_overhead(poart.encaps);
		poart.qos.aal = ATM_AAL5;
		return 0;
	}
	return -1;
}

int set_username(char *arg)
{
	if(arg) {
		/* Authentication */
		strcpy(sprt.ppp.myauth.name, arg);
		return 0;
	}
	return -1;
}

int set_password(char *arg)
{
	if(arg) {
		/* Authentication */
		strcpy(sprt.ppp.myauth.secret, arg);
		return 0;
	}
	return -1;
}


int set_mode(char *arg)
{
	if(arg) {
		/* Direct/Auto */
		sprt.ppp.mode = atoi(arg);
		return 0;
	}
	return -1;
}

int set_iptype(char *arg)
{
	if(arg) {
		/* Direct/Auto */
		sprt.ppp.ipType = atoi(arg);
		return 0;
	}
	return -1;
}

int set_mru(char *arg)
{
	if(arg) {
		/* our MRU */
		sprt.ppp.lcp.mru = atoi(arg);
		return 0;
	}
	return -1;
}

int set_default_gw(char *arg)
{
int gw;
	if(arg) {
		gw = atoi(arg);
		if(gw == 1)
			sprt.ppp.dgw = 1;
		return 0;
	}
	return -1;
}

// Jenny, keepalive setting
int set_keepalive(char *arg)
{
	if (arg) {
		if (atoi(arg) == 1)
			sprt.ppp.diska = 1;
		return 0;
	}
	return -1;
}

int set_timeout(char *arg)
{
#ifdef AUTO_PPPOE_ROUTE
	unsigned int dgw;
#endif
	if(atoi(arg)!=0) {
		//if(sprt.ppp.dgw) {
#ifdef AUTO_PPPOE_ROUTE
		mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
		if(dgw == DGW_AUTO || sprt.ppp.dgw) // Jenny, if set to auto(0xef)
#else
		if(sprt.ppp.dgw)
#endif
		{
			// Mason Yu
			// Because the sysytem send keepalive packets, every KEEPALIVE_INTERVAL(20) seconds
			//sprt.ppp.timeout = atoi(arg);
//				sprt.ppp.timeout = atoi(arg) * 3; // Mason Yu. means 1 min
				sprt.ppp.timeout = atoi(arg) ;
		}else
			sprt.ppp.timeout = 0;
	}
	return -1;

}

int set_debug(char *arg)
{
	int debug;
	if(arg) {
		debug = atoi(arg);
		if(debug == 1)
			sprt.ppp.debug = 1;
		return 0;
	}
	return -1;
}

#ifdef IP_PASSTHROUGH
// Added by Mason Yu for Half Bridge
int set_ippt_flag(char *arg)
{
    int ippt_flag;
    //printf("Set IPPT Flag\n");

	if(arg) {
		ippt_flag = atoi(arg);
		if(ippt_flag == 1)
			sprt.ppp.ippt_flag = 1;
		return 0;
	}
	return -1;

}
#endif

int set_acname(char *arg)
{
	if(sprt.dev == &poert) {
		if(arg) {
			strcpy(poert.ACName, arg);
			return 0;
		}
	}
	return -1;
}

int set_auth(char *arg)
{
	char *start=arg;

	if(arg) {
		while (*start) {
			*start = tolower(*start);
			start++;
		}
		if (strcmp("auto", arg)==0)
			sprt.ppp.myauth.proto = 0x0;
		else if (strcmp("chap", arg)==0)
			sprt.ppp.myauth.proto = PPP_CHAP;
		else if (strcmp("pap", arg)==0)
			sprt.ppp.myauth.proto = PPP_PAP;
		else if (strcmp("chapms-v1", arg)==0){
			sprt.ppp.myauth.proto = PPP_CHAP;
			sprt.ppp.myauth.chal_type = CHAP_MICROSOFT;
		}
		else if (strcmp("chapms-v2", arg)==0){
			sprt.ppp.myauth.proto = PPP_CHAP;
			sprt.ppp.myauth.chal_type = CHAP_MICROSOFT_V2;
		}
		else
			return -1;

		return 0;
	}
	return -1;
}


#ifdef _CWMP_MIB_
int set_servicename(char *arg)
{
	if(sprt.dev == &poert) {
		if(arg) {
			strcpy(poert.ServiceName, arg);
			return 0;
		}
	}
	return -1;
}

int set_disconnect(char *arg)
{
	if(atoi(arg)!=0)
		sprt.ppp.autoDisconnectTime = atoi(arg);
	else
		sprt.ppp.autoDisconnectTime = 0;
	return 0;
}

int set_delay(char *arg)
{
	if(atoi(arg)!=0)
		sprt.ppp.warnDisconnectDelay = atoi(arg);
	else
		sprt.ppp.warnDisconnectDelay = 0;
	return 0;
}
#endif

int set_debug2(char *arg)
{
	sprt.cmd = 9;
	sprt.ppp.unit = atoi(arg);
}

#ifdef ENABLE_PPP_SYSLOG
// Jenny, debug message output setting
int set_syslog(char *arg)
{
	sprt.cmd = 12;
	dbg_syslog = atoi(arg);
}
#endif

#ifdef CONFIG_USER_PPPOE_PROXY
int set_proxy(char *arg)
{
	if(arg) {
		sprt.ppp.enable_pppoe_proxy= atoi(arg);
		printf("Enable_pppoe_proxy = %d\n",sprt.ppp.enable_pppoe_proxy);
		if(1 == sprt.ppp.enable_pppoe_proxy)
			pppoe_server_init();
		return 0;
	}
	return -1;


}

int set_bind_lanports(char *arg)
{
   if(arg){
   	   sprt.ppp.itfGroup = atoi(arg);
       printf("-->wan_binds_port %d\n",sprt.ppp.itfGroup);
	   return 0;
   }
   return -1;
}

int set_proxy_maxuser(char *arg)
{
   if(arg){
   	   sprt.ppp.maxUser = atoi(arg);
       printf("-->set_proxy_maxuser %d\n",sprt.ppp.maxUser);
	   return 0;
   }
   return -1;
}

int del_proxy_unit(char *arg)
{
	sprt.cmd = 17;
	ppp_proxy_unit = atoi(arg);
}

#endif

#ifdef CONFIG_TR069_DNS_ISOLATION
int  set_tr069_dns(char *arg)
{
   if(arg){
   	sprt.ppp.isTr069_interface = atoi(arg);
	printf("*************set tr069 dns =%d .......\n",sprt.ppp.isTr069_interface);
	return 0;
   }
   return -1;
}

#endif

#ifdef  CONFIG_SPPPD_STATICIP
int set_staticip(char *arg)
{
	unsigned long  ourip  ;

	if (arg) {
		ourip = strtoul(arg, NULL, 16);
		sprt.ppp.ipcp.myipaddr = ourip;
		sprt.ppp.staticip = 1;
	}
}

#endif

#ifdef CONFIG_NO_REDIAL
int  set_no_dial(char *arg)
{


   if(arg){

   	   no_redial= atoi(arg);
	  printf("********no redial  == %d \n",no_redial);
	  sprt.cmd = 11;
   }


}
#endif

#ifdef CONFIG_USER_PPPOMODEM
int dev_pppomodem(char *arg)
{
	/* device is PPPoModem */
	sprt.ppp.over = SPPP_PPPOMODEM;
	sprt.dev = &pomrt;
	//set default
	pppomodem_default( &pomrt );

	if(arg)
	{
		if( !strcmp(arg, "auto") )
		{
			pomrt.finddev=1;
			pomrt.devnam[0]=0;
		}else{
			pomrt.finddev=0;
			strncpy(pomrt.devnam, arg, MAX_MODEM_DEV_NAME-1);
			pomrt.devnam[MAX_MODEM_DEV_NAME-1]=0;
		}
	}else{
		pomrt.finddev=1;
		pomrt.devnam[0]=0;
	}

	//printf( "%s: set finddev=%d, devnam=%s\n", __FUNCTION__, pomrt.finddev, pomrt.devnam );
	return 0;
}
int set_pppomodem_speed(char *arg)
{
	int ret=-1;

	if (arg && (atoi(arg)!=0) )
	{
		pomrt.inspeed=atoi(arg);
		ret=0;
	}

	//printf( "%s: set speed=%d\n", __FUNCTION__, pomrt.inspeed );
	return ret;
}
int set_pppomodem_simpin(char *arg)
{
	int ret=-1;

	if (arg && (strlen(arg)==SIM_PIN_LEN) )
	{
		if( isdigit(arg[0]) && isdigit(arg[1]) && isdigit(arg[2]) && isdigit(arg[3]) )
		{
			pomrt.has_simpin=1;
			strcpy( pomrt.simpin, arg );
			ret=0;
		}
	}

	//printf( "%s: set has_simpin=%d, simpin=%s\n", __FUNCTION__, pomrt.has_simpin, pomrt.simpin );
	return ret;
}
int set_pppomodem_apn(char *arg)
{
	int ret=-1;

	if( arg && arg[0] )
	{
		pomrt.has_apn=1;
		strncpy(pomrt.apn, arg, sizeof(pomrt.apn)-1);
		pomrt.apn[sizeof(pomrt.apn)-1]=0;
		ret=0;
	}

	//printf( "%s: set apn=%s\n", __FUNCTION__, pomrt.apn );
	return ret;
}
int set_pppomodem_dial(char *arg)
{
	int ret=-1;

	if( arg && arg[0] )
	{
		strncpy(pomrt.dial, arg, sizeof(pomrt.dial)-1);
		pomrt.dial[sizeof(pomrt.dial)-1]=0;
		ret=0;
	}

	//printf( "%s: set dial=%s\n", __FUNCTION__, pomrt.dial );
	return ret;
}
//paula, 3g backup PPP

int set_pppomodem_backup(char *arg)
{
	if (arg) {
		if (atoi(arg) == 1){
			ppp_backup_flag = 1;
			ppp_backup_unit = sprt.ppp.unit;
		}
		else{
			ppp_backup_flag = 0;
		}
		ppp_backup_debug = 0;
		return 0;
	}
	return -1;
}

int set_pppomodem_backup_timer(char *arg)
{
	if(arg) {
		ppp_backup_timer = atoi(arg);
		return 0;
	}
	return -1;
}
int set_pppomodem_backup_debug(char *arg)
{
	if(arg) {
		ppp_backup_debug = atoi(arg);
		sprt.cmd = 14;
		return 0;
	}
	return -1;
}
#endif //CONFIG_USER_PPPOMODEM

// Jenny
int dod_cfg(char *arg)
{
	sprt.cmd = 13;
	dial_start(atoi(arg));
	return 0;
}

int status_cfg(char *arg)
{
	sprt.cmd = 14;
	ppp_status();
	return 0;
}
#if defined(CONFIG_LUNA)
int no_hw_nat(char *arg)
{
	poert.NoHardware_NAT = 1;
	return 0;
}
#endif

int set_holdoff_delay(char *arg)
{
	sprt.cmd = 6;
	if(arg){
		holdoff_interval = atoi(arg);
		return 0;
	}
	return -1;
}

#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
int set_poe_retry_threshold(char *arg)
{
	if(arg)
	{
		poert.retry_threshold = atoi(arg);
		return 0;
	}
	return -1;
}

int set_poe_retry_delay(char *arg)
{
	if(arg)
	{
		poert.retry_delay = atoi(arg);
		return 0;
	}
	return -1;
}

int set_poe_retry_delay_long(char *arg)
{
	if(arg)
	{
		poert.retry_delay_long = atoi(arg);
		return 0;
	}
	return -1;
}
#endif

///////////////////////////////////////////////////////////////////////

struct command
{
	int		needs_arg;
	int		num_string_arg;
	char	*name;
	int	(*func)(char *arg);
};


#define SERVER_FIFO_NAME "/tmp/ppp_serv_fifo"
#if 0
#define BUFFER_SIZE	256

struct data_to_pass_st {
	int	id;
	char data[BUFFER_SIZE];
};
#endif


static struct command commands[] = {
	{0, 1, "show", show_cfg_ppp},
	{1, 1, "del", del_cfg_ppp},
	{0, 1, "add", add_cfg_ppp},
	{1, 1, "rec", reconnect_cfg_ppp},
	{0, 1, "down", down_cfg_ppp},	// Added by Jenny for PPP connecting/disconnecting manually
	{0, 1, "up", up_cfg_ppp},
	{0, 1, "new", new_cfg_ppp},
	{0, 1, "katimer", keepalive_cfg},	// Added by Jenny for keepalive timer setting
	{0, 1, "dbg", set_debug2},		// Jenny, for real time debugging
	{0, 1, "pppoa", dev_pppoa},
	{0, 1, "pppoe", dev_pppoe},
#ifdef CONFIG_PPP_MPPE_MPPC
	{0, 1, "enctype", set_encrypt},
#endif//endof CONFIG_PPP_MPPE_MPPC
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	{0, 1, "pptp", dev_pptp},
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	{0, 1, "l2tp", dev_l2tp},
	{0, 1, "tunnel_auth", set_l2tp_tunnel_auth},
	{0, 1, "secret", set_l2tp_secret},
#endif//endof CONFIG_USER_L2TPD_L2TPD
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
	{0, 1, "server", set_vpn_server},
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD
#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
	{0, 1, "addvpn", add_vpn_account},
	{0, 1, "delvpn", del_vpn_account},
	{0, 1, "type", set_vpn_type},
	{0, 1, "localip", set_vpn_localip},
	{0, 1, "peerip", set_vpn_peerip},
#endif
	{1, 1, "encaps", set_pppoa_encaps},
	{1, 1, "qos", set_pppoa_qos},
	{0, 1, "username", set_username},
	{0, 1, "password", set_password},
	{0, 1, "mode", set_mode},
	{0, 1, "ipt", set_iptype},
	{0, 1, "mru", set_mru},
	{0, 1, "gw", set_default_gw},
	{0, 1, "diska", set_keepalive},	// Added by Jenny for keepalive setting
	{0, 1, "timeout", set_timeout},
	{0, 1, "debug", set_debug},
#ifdef IP_PASSTHROUGH
	{0, 1, "ippt", set_ippt_flag},   // Added by Mason Yu for Half Bridge
#endif
	{0, 1, "acname", set_acname},
	{1, 1, "auth", set_auth},
	{0, 1, "pppoetest", set_rm_pppoe_test},  // Added by Mason Yu
#ifdef _CWMP_MIB_
	{0, 1, "disctimer", set_disconnect},	// Jenny, set auto disconnect timer
	{0, 1, "discdelay", set_delay},	// Jenny, set disconnect delay time
	{0, 1, "servicename", set_servicename},	// Jenny, set service name
#endif
#ifdef ENABLE_PPP_SYSLOG
	{0, 1, "syslog", set_syslog},		// Jenny, show debug message to 1: syslog /0: console
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	{0, 1,"proxy", set_proxy},        //alex_huang for pppoe proxy
	{0, 1, "itfgroup", set_bind_lanports},    //wpeng for port mapping
	{0, 1, "maxuser", set_proxy_maxuser},
	{0, 1, "delproxyunit", del_proxy_unit},
#endif
#ifdef CONFIG_TR069_DNS_ISOLATION   //alex_huang for tr069 interface e8b
        {0, 1,"tr069_dns", set_tr069_dns},

#endif
#ifdef CONFIG_SPPPD_STATICIP
	{ 0, 1, "staticip", set_staticip}, //alex_huang for static ip setting
#endif
#ifdef CONFIG_NO_REDIAL
       { 0,1,"noredial",set_no_dial},//alex
#endif
#ifdef CONFIG_USER_PPPOMODEM
	{0, 1, "pppomodem", dev_pppomodem},
	{0, 1, "speed", set_pppomodem_speed},
	{0, 1, "simpin", set_pppomodem_simpin},
	{0, 1, "apn", set_pppomodem_apn},
	{0, 1, "dial", set_pppomodem_dial},
	{0, 1, "backup", set_pppomodem_backup}, //paula, 3g backup
	{0, 1, "backup_timer", set_pppomodem_backup_timer}, //paula, 3g backup
	{0, 1, "backup_debug", set_pppomodem_backup_debug}, //paula, 3g backup
#endif //CONFIG_USER_PPPOMODEM
	// Jenny
	{0, 1, "startdod", dod_cfg},
	{0, 1, "pppstatus", status_cfg},
#if defined(CONFIG_LUNA)
	{0, 1, "nohwnat", no_hw_nat},
#endif
	{0, 1, "holdoff", set_holdoff_delay},	// how many seconds to wait before re-initiating the link after it terminates.
#ifdef CONFIG_USER_PPPOE_2_INTERVALS_RETRY
	{0, 1, "poe_retry_threshold", set_poe_retry_threshold},
	{0, 1, "poe_retry_delay", set_poe_retry_delay},
	{0, 1, "poe_retry_delay_long", set_poe_retry_delay_long},
#endif
	{0, 0, NULL, NULL}
};

struct command *mpoa_command_lookup(char *arg)
{
	int i;
	for(i=0; commands[i].name!=NULL; i++)
		if(!strcmp(arg, commands[i].name))
			return (&commands[i]);
	return NULL;
}


int server_fifo_fd, client_fifo_fd;

int init_msg (void)
{

	mkfifo(SERVER_FIFO_NAME, 0777);

	server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY|O_NONBLOCK);
	fcntl(server_fifo_fd, F_SETFD, fcntl(server_fifo_fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	if (server_fifo_fd == -1) {
		fprintf(stderr, "Server fifo failure\n");
		return -1;
	}

	return 0;
}

int poll_cc(int fd)
{
int bytesToRead = 0;
	(void)ioctl(fd, FIONREAD, (int)&bytesToRead);
	return bytesToRead;
}


#define MAX_ARGS	32
#define MAX_ARG_LEN	64

// Kaohj, put PPPoE request into pending list
/*
static void add_into_list(struct spppreq *req)
{
	struct spppreq *t_sprt, *psprt;
	struct pppoe_param_s *t_poert;

	t_sprt = malloc(sizeof(struct spppreq));
	memcpy(t_sprt, req, sizeof(struct spppreq));
	t_poert = malloc(sizeof(struct pppoe_param_s));
	memcpy(t_poert, req->dev, sizeof(struct pppoe_param_s));
	t_sprt->dev = (void *)t_poert;

	if (sprt_list == NULL) {
		sprt_list = t_sprt;
		t_sprt->next = NULL;
	}
	else {
		psprt = sprt_list;
		while (psprt->next != NULL)
			psprt = psprt->next;
		// add into list
		psprt->next = t_sprt;
		t_sprt->next = NULL;
	}
}

static void process_poe_list()
{
	struct spppreq *psprt;

	psprt = sprt_list;
	while (psprt != NULL) {
		//printf("process_poe_list: add_ppp()\n");
		add_ppp(psprt->ppp.unit, psprt);
		sprt_list = sprt_list->next;
		free(psprt->dev);
		free(psprt);
		psprt = sprt_list;
	}
}
*/

#ifdef EMBED
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = SPPPD_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
#endif

int recv_msg (void)
{
	struct data_to_pass_st 	msg;
	int	read_res;
	int i, c;
  	int argc;
	char argv[MAX_ARGS][MAX_ARG_LEN+1];
  	char *arg_ptr;

  	if(poll_cc(server_fifo_fd)) {
		argc = 0;
		// Kaohj
		//do {
			read_res = read(server_fifo_fd, &msg, sizeof(msg));
			if(read_res > 0) {
				int arg_idx = 0;
				arg_ptr = msg.data;
				for(i=0; arg_ptr[i]!='\0'; i++) {
					if(arg_ptr[i]==' '){
						argv[argc][arg_idx]='\0';
						argc++;
						arg_idx=0;
					}
					else {
						if(arg_idx<MAX_ARG_LEN) {
							argv[argc][arg_idx]=arg_ptr[i];
							arg_idx++;
						}
					}
				}
				argv[argc][arg_idx]='\0';
			}
		// Kaohj
		//} while (read_res > 0);

		memset(&sprt, 0, sizeof sprt);
		memset(&poert, 0, sizeof poert);
		memset(&poart, 0, sizeof poart);
#ifdef CONFIG_USER_PPPOMODEM
		memset(&pomrt, 0, sizeof pomrt);
#endif //CONFIG_USER_PPPOMODEM
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
		memset(&vpnrt, 0, sizeof vpnrt);
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD
		sprt.cmd = -1;
        for(c=0; commands[c].name!=NULL; c++)
		   commands[c].needs_arg=0;

		for(c=0; commands[c].name!=NULL; c++) {
			for(i=1; i<argc; i++) {
				if(!strcmp(argv[i],"password"))
				{
					set_password(argv[++i]);
					continue;
				}
				if(!strcmp(argv[i],"username"))
				{
					set_username(argv[++i]);
					continue;
				}
//#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_PPP_MPPE_MPPC
				if(!strcmp(argv[i],"enctype"))
				{
					set_encrypt(argv[++i]);
					continue;
				}
#endif
//#endif
				if(!strcmp(argv[i], commands[c].name)) {
					if(commands[c].num_string_arg)
						if(commands[c].needs_arg==0){
							commands[c].func(argv[i+1]);
							commands[c].needs_arg=1;
						}
				}
			}
		}

		switch(sprt.cmd) {
		case 0:	/* add */
			// Kaohj
			//if (in_pppoe_disc && sprt.ppp.over == 1)
			//	add_into_list(&sprt);
			//else
			add_ppp(sprt.ppp.unit, &sprt);
			break;
		case 1:
			del_ppp(sprt.ppp.unit);
			break;
		case 2:
			/* for debug only */
			//show_ppp(sprt.ppp.unit);
			ppp_status();
			break;
		case 3:	// Jenny
			ppp_up(sprt.ppp.unit);
			break;
		case 4:	// Jenny
			ppp_down(sprt.ppp.unit);
			sppp_last_connection_error(sprt.ppp.unit, ERROR_USER_DISCONNECT);
			syslog(LOG_INFO, "spppd: %s: user disconnected\n", sprt.ppp.if_name);
			break;
		case 5:	// Jenny
			ppp_new(sprt.ppp.unit, &sprt);
			break;
		case 6:	// Jenny
			break;
		case 7:	// Mason Yu
			rm_pppoe_test(sprt.ppp.unit, &sprt);
			break;
		case 8:	// Jenny
			reconnect_ppp(sprt.ppp.unit);
			break;
		case 9:	// Jenny
			//debug_set(sprt.ppp.unit);
			debug_set(sprt.ppp.unit, &sprt);
			break;
		case 11:
			 printf("redial----\n");
			 break	 ;
#ifdef ENABLE_PPP_SYSLOG
		case 12: // Jenny
			syslog_set(dbg_syslog);
			break;
#endif
		// Jenny
		case 13:
		case 14:
			break;
#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
		case 15: // add PPTP/L2TP server
			add_pppd_account(vpn_server_idx, &sprt);
			break;
		case 16: //del PPTP/L2TP server
			del_pppd_account(vpn_server_idx);
			break;
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
		case 17: //delete ppp proxy unit
			del_pppd(ppp_proxy_unit);
			break;
#endif
		default:
			printf("spppd command error(%d)!\n", sprt.cmd);
			break;
		}
	}

	return 0;
}


static char adslDevice[] = "/dev/adsl0";
static FILE* adslFp = NULL;
#if 0
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
//	    	        printf("ADSL ioctl failed! id=%d\n",id );
			close_adsl_drv();
			return 0;
	       };

		close_adsl_drv();
		return 1;
	}
#endif
	return 0;
}
#endif

static int check_wanlink(char *ifname, int state)
{
	Modem_LinkSpeed vLs;

	vLs.upstreamRate=0;
#ifdef WLAN_WISP
	char wisp_wan_name[16];
#endif
	int flags;

	if (ifname) { // netlink linkchange event
		if (!state) {
			stop_interface(ifname);
		}
		else {
			sleep(1); // need to wait for wan ready.
			if(!strcmp(ifname, "atm0")) { // PPPoA or ADSL WAN
				if (!ppp_term_flag){
					sppp_term_send();	// send TERM-REQ to release existed PPP connection
				}
			}
//Note: Now netlink only handle link down event. And if in process_poe, check interface
//      status is RUNNING, then will try to connect. Using this method is because, if in some
//      platform, process_poe is starting to connect, then netlink event down/up again will corrupt
//      the connecting session.
#if 0
			else
				pre_release_pppoe(ifname);	// Jenny, send PADT packet to release existed PPPoE connection
#endif
		}
	}
	else { // some interface without netlink event, we use polling mode.
		#if 0
		#ifdef WLAN_WISP
		getWispWanName(wisp_wan_name);
		if (wlWanLinkIsDown(wisp_wan_name)) {
			stop_interface(wisp_wan_name);
		}
		else{
			if (!wl_PADT_flag){
				pre_release_pppoe(wisp_wan_name);	// Jenny, send PADT packet to release existed PPPoE connection
			}
		}
		#endif
		#endif
	}
	return 0;
}

/* read netlink event */
static int read_event(int sockint)
{
	int status;
	int ret = 0;
	char buf[4096];
	struct iovec iov = { buf, sizeof buf };
	struct sockaddr_nl snl;
	struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
	struct nlmsghdr *h;
	struct ifinfomsg *ifi;
	struct rtattr *attribute;
	int len;

	status = recvmsg (sockint, &msg, 0);

	if (status < 0)
	{
		/* Socket non-blocking so bail out once we have read everything */
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return ret;

		/* Anything else is an error */
		perror ("read_netlink: Error: ");
		return status;
	}

	if (status == 0)
	{
		printf ("read_netlink: EOF\n");
	}

	// We need to handle more than one message per 'recvmsg'
	for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, (unsigned int) status);
	     h = NLMSG_NEXT (h, status))
	{
		//Finish reading
		if (h->nlmsg_type == NLMSG_DONE)
			return ret;

		// Message is some kind of error
		if (h->nlmsg_type == NLMSG_ERROR)
		{
	        	printf ("read_netlink: Message is an error - decode TBD\n");
	        	return -1;        // Error
		}

		if (h->nlmsg_type == RTM_NEWLINK)
		{
			// check link status on wan link change
			#if 1
			ifi = NLMSG_DATA (h);
			//printf ("NETLINK::%s\n", (ifi->ifi_flags & IFF_RUNNING) ? "Up" : "Down");
			len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
			for (attribute = IFLA_RTA(ifi); RTA_OK(attribute, len); attribute = RTA_NEXT(attribute, len))
			{
				switch(attribute->rta_type)
				{
					case IFLA_IFNAME:
						//printf("Interface %d : %s\n", ifi->ifi_index, (char *) RTA_DATA(attribute));
						check_wanlink((char *) RTA_DATA(attribute), ifi->ifi_flags & IFF_RUNNING);
						break;
					default:
						break;
				}
			}
			#endif
			return ret;
	  	}
	}

	return ret;
}

int poll_linkstatus(int sock)
{
	fd_set rfds;
	int retval;
	struct timeval tout;

	memset (&tout, 0, sizeof(tout));

	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);

	// polling netlink socket, non-blocking
	retval = select (FD_SETSIZE, &rfds, NULL, NULL, &tout);
	if (retval == -1)
		printf ("Error select() \n");
	else if (retval) {
		read_event(sock);
	}
	//check_wanlink(0, 0); // polling those interfaces without linkchange event.

	return 0;
}

// Jenny
static void dial_sig(int signum) {
	char cmd[128];
	printf("%s process dial signal %d\n", __FUNCTION__, signum);

	sprintf(cmd,"spppctl startdod %d", signum);
	printf("%s %s\n", __FUNCTION__, cmd);
	system(cmd);
	return;
}

#if defined(CONFIG_LUNA)
static void raise_spppd(int signum) {
	printf("continue spppd.\n");
	return;
}
#endif

static void handle_pipe(int sig) {
	fprintf(stderr, "got epipe! %d\n", sig);
	return;
}

static void handle_term(int sig)
{
	if (server_fifo_fd > 0) {
		close(server_fifo_fd);
	}
	unlink(SERVER_FIFO_NAME);
	exit(0);
}

int main(void)
{
	struct sigaction act, actio;
	int sock_nl;
	struct sockaddr_nl nl_local;

	init_msg();
	init_ppp_unit();

#if defined(CONFIG_USER_PPTPD_PPTPD) || defined(CONFIG_USER_L2TPD_LNS)
	init_vpn_server();
#endif

#ifdef EMBED
	log_pid();
#endif

#if defined(CONFIG_LUNA)
	/* ql use SIGUSR2 to raise spppd */
	signal(SIGUSR2, raise_spppd);
	pause();
#endif

	// setup
	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGPIPE, &act, 0);

	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);

	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGQUIT, &act, 0);

	act.sa_handler = handle_pipe;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGHUP, &act, 0);
	signal(SIGTERM, handle_term);
	signal(SIGKILL, handle_term);
	// Jenny
	//signal(SIGUSR1, dial_start);  //for trap
	signal(SIGUSR1, dial_sig);  //for trap
	//signal(SIGUSR2, ppp_status);	// Jenny, update PPP status file

	// netlink to receive linkchange event
	if ((sock_nl = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
		perror ("socket failure\n");
	memset (&nl_local,0,sizeof(nl_local));
	nl_local.nl_family = AF_NETLINK;
	nl_local.nl_pad = 0;
	nl_local.nl_pid = getpid();
	nl_local.nl_groups = RTMGRP_LINK;

	if (bind(sock_nl, (struct sockaddr *)&nl_local, sizeof(nl_local)) == -1)
		perror ("bind failure\n");

	/* update ppp status files */
	ppp_status();

	while(1) {
		recv_msg();
		// Kaohj
		// process the outstanding commands for PPPoE
		//process_poe_list();
		process_poe();
#ifdef CONFIG_USER_PPPOMODEM
		pppomodem_process();
		if(ppp_backup_flag) // paula, 3g backup PPP
			poll_3g_backup_status();
#endif //CONFIG_USER_PPPOMODEM
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
		process_pptp();
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
		process_l2tp();
#endif //endof CONFIG_USER_L2TPD_L2TPD
		calltimeout();
		sppp_recv();

#ifdef CONFIG_USER_PPTPD_PPTPD
		pptp_manager();
#endif //end of CONFIG_USER_PPTPD_PPTPD
#ifdef CONFIG_USER_L2TPD_LNS
		process_l2tpd();
#endif //end of CONFIG_USER_L2TPD_LAC

		poll_linkstatus(sock_nl);
	}
}

