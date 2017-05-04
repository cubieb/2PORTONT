#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>	
#include <signal.h>
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif
/* RTK libraries*/
#ifdef CONFIG_RTK_RG_INIT
#include <rtk/rtusr_rg_api.h>
#include <rtk_rg_struct.h>
#include <rtk/utility.h>
#endif

#include <pthread.h>

#define DBG(...) ((void)0)
//#define DBG(fmt, ...) fprintf(stderr, "<%s:%d> "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
typedef enum { PON_BW_NORMAL=0, PON_BW_ENLARGE } PON_BW_T;
static unsigned char PPPoE_enlarge_BW_flag = 0;
#endif

static void setup_dnsmasq(int signo)
{
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
	cmd_set_dns_config(NULL);	
	restart_dnsrelay();
#endif
}

#define SYSTEMD_RUNFILE	"/var/run/systemd.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = SYSTEMD_RUNFILE;

	pid = getpid();

	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

#ifdef CONFIG_MIDDLEWARE
extern int sendMsg2MidProcess(struct mwMsg * pMsg);

int sendInformKeyParaMsg2MW( char *ifname )
{
	unsigned char vChar;
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;
	int ret,wan_ifIndex;
	MIB_CE_ATM_VC_T vc_entry = {0};

	wan_ifIndex = getIfIndexByName(ifname);
	if (wan_ifIndex == -1)
		return 0;
	ret = (int)getATMVCEntryByIfIndex(wan_ifIndex, &vc_entry);
	if(vc_entry.ServiceList & X_CT_SRV_TR069){
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(vChar == 0 || vChar == 2){
			*(sendBuf) = OP_informKeyPara;
			return(sendMsg2MidProcess(&sendMsg));
		}else
			return -1;	
	}
}
#endif	//end of CONFIG_MIDDLEWARE


/*
ifn : interface name
state
0 => ip change
1 => ip remove dependency
2 => link change dependency
*/

#ifdef CONFIG_USER_PPPOMODEM
int check_34G_status(char *ifn, int state)
{
        MIB_WAN_3G_T entry,*p;
        // handle 4G case
        if(strcmp ( ifn , "usb0") == 0)
        {
                if(state == 0)
                        system("echo 1 > /proc/internet_flag");
                else if (state == 1 || state == 2)
                        system("echo 0 > /proc/internet_flag");
                return 1;
        }
        // handle 3G case
        p=&entry;
        if(!mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p))
        {
                printf( "%s: error mib_chain_get(MIB_WAN_3G_TBL), line=%d\n", __FUNCTION__, __LINE__ );
                return 0;
        }
        if(p->enable == 1 && p->dgw == 1)
        {
                if(state == 0)
                        system("echo 1 > /proc/internet_flag");
                else if (state == 1 || state == 2)
                        system("echo 0 > /proc/internet_flag");
                return 1;
        }

        return 0;
}
#endif

#ifdef CONFIG_RTK_RG_INIT
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
MIB_PPTP_T *getPPTPEntryByIfIndex(unsigned int ifIndex, MIB_PPTP_T *p)
{
	unsigned int i,num;
	if( (p==NULL) || (ifIndex==DUMMY_IFINDEX) ) return NULL;
	num = mib_chain_total( MIB_PPTP_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_PPTP_TBL, i, (void*)p ))
			continue;
		if( p->ifIndex==ifIndex )
		{
			printf("%s-%d ifIndex=%d\n",__func__,__LINE__,ifIndex);
			return p;
		}
	}
	return NULL;
}
int getIfIndexByPPTPName(char *pIfname)
{
	unsigned int entryNum, i;
	MIB_PPTP_T Entry;
	char ifname[IFNAMSIZ];
	entryNum = mib_chain_total(MIB_PPTP_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PPTP_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		ifGetName(Entry.ifIndex,ifname,sizeof(ifname));
		if(!strcmp(ifname,pIfname)){
			break;
		}
	}
	if(i>= entryNum){
		printf("not find this interface!\n");
		return -1;
	}
	return(Entry.ifIndex);
}
int RG_pptp_ip_chagne(char *ifname, struct in_addr *new_ip, struct in_addr *ifa_local)
{
	int wan_ifIndex;
	MIB_PPTP_T pptp_entry = {0};
	unsigned int entrynum, i;	
	int enable, ret;
	wan_ifIndex = getIfIndexByPPTPName(ifname);

	if (wan_ifIndex == -1) return 0;	
	ret = (int)getPPTPEntryByIfIndex(wan_ifIndex, &pptp_entry);
	if (!ret) return 0;
	/* Two addresses are the same means we have not get remote IP yet.*/	
	if(ifa_local->s_addr == new_ip->s_addr)
		return 0;	

	RG_add_pptp(new_ip->s_addr,ifa_local->s_addr,&pptp_entry);
	return 0;
}

#endif /*CONFIG_USER_PPTP_CLIENT_PPTP*/
#ifdef CONFIG_USER_L2TPD_L2TPD
MIB_L2TP_T *getL2TPEntryByIfIndex(unsigned int ifIndex, MIB_L2TP_T *p)
{
	unsigned int i,num;
	if( (p==NULL) || (ifIndex==DUMMY_IFINDEX) ) return NULL;
	num = mib_chain_total( MIB_L2TP_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_L2TP_TBL, i, (void*)p ))
			continue;
		if( p->ifIndex==ifIndex )
		{
			printf("%s-%d ifIndex=%d\n",__func__,__LINE__,ifIndex);
			return p;
		}
	}
	return NULL;
}
int getIfIndexByL2TPName(char *pIfname)
{
	unsigned int entryNum, i;
	MIB_L2TP_T Entry;
	char ifname[IFNAMSIZ];
	entryNum = mib_chain_total(MIB_L2TP_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_L2TP_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}
		ifGetName(Entry.ifIndex,ifname,sizeof(ifname));
		if(!strcmp(ifname,pIfname)){
			break;
		}
	}
	if(i>= entryNum){
		printf("not find this interface!\n");
		return -1;
	}
	return(Entry.ifIndex);
}
int RG_l2tp_ip_chagne(char *ifname, struct in_addr *new_ip, struct in_addr *ifa_local)
{
	int wan_ifIndex;
	MIB_L2TP_T l2tp_entry = {0};
	unsigned int entrynum, i;	
	int enable, ret;
	wan_ifIndex = getIfIndexByL2TPName(ifname);

	if (wan_ifIndex == -1) return 0;	
	ret = (int)getL2TPEntryByIfIndex(wan_ifIndex, &l2tp_entry);
	if (!ret) return 0;
	/* Two addresses are the same means we have not get remote IP yet.*/
	if(ifa_local->s_addr == new_ip->s_addr)
		return 0;	

	RG_add_l2tp(new_ip->s_addr,ifa_local->s_addr,&l2tp_entry);
	return 0;
}
#endif /*CONFIG_USER_L2TPD_L2TPD*/

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)

int checkSolForSmallBW()
{
	//For solution of APOLLO+Small BW
	FILE *fp_smallbw = NULL;
	char *smallbw_fpath="/proc/rg/pppoe_gpon_small_bandwidth_control";
	if(!(fp_smallbw = fopen(smallbw_fpath, "r"))) {
		fprintf(stderr, "ERROR! for open %s %s\n",smallbw_fpath, strerror(errno));
	}
	else{
		int small_bw_setting;
		fscanf(fp_smallbw, "%d\n", &small_bw_setting);
		if(small_bw_setting==1){
			fclose(fp_smallbw);
			printf("Have hardware solution for small BW! Do Nothing!\n");
			return 1;
		}
		fclose(fp_smallbw);
	}		
	printf("Don't have hardware solution for small BW!\n");
	return 0;
}

void update_PON_BW(unsigned char bw_mode, char *ifname)
{
	unsigned int ChipId, Rev, Subtype;

	DBG("bw_mode == %d, PPPoE_enlarge_BW_flag=%d, ifname=%s\n", bw_mode, PPPoE_enlarge_BW_flag, ifname);
	//check IC version
	rtk_rg_switch_version_get(&ChipId, &Rev, &Subtype);
	if(ChipId!=APOLLOMP_CHIP_ID)
		return;

	if(checkSolForSmallBW())
		return;

	if(!ifname)
		return;

	if(bw_mode == PON_BW_ENLARGE){
		if(PPPoE_enlarge_BW_flag)  // BW already enlarged
			return;
		
		printf("Enlarge PPPoE BW!\n");
		rtk_rg_ponmac_bwThreshold_set(21, 22);
		PPPoE_enlarge_BW_flag = 1;		

	}else if( bw_mode == PON_BW_NORMAL){
		FILE *fp = NULL;
		char buf[256];

		//Check if there is any PPPoE session still existing. check file /var/ppp/pppoe.conf
		if(!(fp = fopen("/var/ppp/pppoe.conf", "r"))) {
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return;
		}

		while(fgets(buf,sizeof(buf),fp)>0)
		{
			if(strstr(buf,"Session") && !strstr(buf, ifname)){
				DBG("buf=%s",buf);
				fclose(fp);
				return;
			}
		}

		printf("Reset PPPoE BW to Normal!\n");
		rtk_rg_ponmac_bwThreshold_set(13, 14);	
		PPPoE_enlarge_BW_flag = 0;		
		fclose(fp);
	}
}
#endif


int RG_ip_change(char *ifname, struct in_addr *new_ip, struct in_addr *ifa_local)
{
	struct in_addr mask;
	int wan_ifIndex;
	int ret;
	MIB_CE_ATM_VC_T vc_entry = {0};
	char sVal[32];
#ifdef CONFIG_USER_PPPOMODEM
	if(check_34G_status(ifname , 0 ) == 1)
		return 0;
#endif
	wan_ifIndex = getIfIndexByName(ifname);
	if (wan_ifIndex == -1) return 0;
	ret = (int)getATMVCEntryByIfIndex(wan_ifIndex, &vc_entry);
	if (!ret) return 0;
	if (!vc_entry.enable) return 0;

	switch (vc_entry.cmode)
	{
		case CHANNEL_MODE_IPOE:
			if(vc_entry.ipDhcp==DHCP_CLIENT)
			{
				char str_ip[64] = {0}, str_mask[64] = {0};

				if(getInAddr(ifname, SUBNET_MASK, (void *)&mask) != 1)
					break;

				inet_ntop(AF_INET, (const void *) new_ip, str_ip, 64);
				inet_ntop(AF_INET, (const void *) &mask, str_mask, 64);
				DBG("Adding RG DHCP client info: rg_index=%d, ip=%s, mask=%s\n", vc_entry.rg_wan_idx, str_ip, str_mask);

				if(RG_set_dhcp(new_ip->s_addr, mask.s_addr, &vc_entry) != 0)
					fprintf(stderr, "RG_set_dhcp failed: %d\n",__LINE__);
			}
			else
			{
				DBG("Adding RG Static info: rg_index=%d, ip=%s\n", vc_entry.rg_wan_idx, inet_ntoa(*new_ip));
				RG_set_static(&vc_entry);
			}
			Flush_RG_static_route();
			check_RG_static_route();
			break;
		case CHANNEL_MODE_PPPOE:
			{
				/* For ppp device, ifa_local is the new IP address
				 * new_ip is remote IP address. Following comments
				 * are get from linux source.
				 *
				 * Important comment:
				 * IFA_ADDRESS is prefix address, rather than local interface address.
				 * It makes no difference for normally configured broadcast interfaces,
				 * but for point-to-point IFA_ADDRESS is DESTINATION address,
				 * local address is supplied in IFA_LOCAL attribute.
				 */

				int total = mib_chain_total(MIB_PPPOE_SESSION_TBL);
				MIB_CE_PPPOE_SESSION_T session = {0};
				int i;

				/* Two addresses are the same means we have not get remote IP yet.*/
				if(ifa_local->s_addr == new_ip->s_addr)
					return 0;

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
				update_PON_BW(PON_BW_ENLARGE, ifname);
#endif
				for(i = 0 ; i < total ; i++)
				{
					mib_chain_get(MIB_PPPOE_SESSION_TBL, i, &session);
					if(session.uifno == vc_entry.ifIndex)
					{
						char str_ip[64] = {0}, str_local[64] = {0};
						
						inet_ntop(AF_INET, (const void *) new_ip, str_ip, 64);
						inet_ntop(AF_INET, (const void *) ifa_local, str_local, 64);
				
						DBG("Adding PPPoE: rg_index=%d, sesssionId = %d, ip=%s, local=%s\n", vc_entry.rg_wan_idx, session.sessionId, str_ip, str_local);
						RG_add_pppoe(session.sessionId, new_ip->s_addr, ifa_local->s_addr, session.acMac, &vc_entry);
					}
					check_RG_static_route_PPP(&vc_entry);
				}
			}
			break;
		default:
			break;
	}
	if(vc_entry.dgw){
		//AUG_PRT("%s-%d defualt gateway on, internet led on!!\n",__func__,__LINE__);
		system("echo 1 > /proc/internet_flag");
	}
	return 0;
}
#endif

#ifdef CONFIG_USER_DDNS
int do_ddnsc(char *ifname, int msg_type)
{
	int totalEntry=0, i;
	MIB_CE_ATM_VC_T Entry;
	char this_ifname[IFNAMSIZ];
	
	if (msg_type == RTM_NEWADDR)
	{
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++) {
			unsigned char version[5];
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry); /* get the specified chain record */		
			ifGetName(Entry.ifIndex, this_ifname, sizeof(this_ifname));		
		
			if (strcmp(this_ifname, ifname) == 0)
			{
#ifdef CONFIG_IPV6
				sprintf(version,"%d",Entry.IpProtocol);
				va_cmd("/bin/updateddctrl", 2, 1, ifname, version);			
#else
				va_cmd("/bin/updateddctrl", 2, 1, ifname, "1");				
#endif
				break;
			}
		}
	}
	return 0;
}
#endif

int ip_change_dependency(char *ifname, struct in_addr *new_ip, struct in_addr *ifa_local, int msg_type)
{	
	int my_pid;
	int enable=0;
#ifdef CONFIG_RTK_RG_INIT
	RG_ip_change(ifname, new_ip, ifa_local);
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	mib_get(MIB_PPTP_ENABLE, (void *)&enable);
	if(enable){
		if(!strcmp(ifname,"ppp9") || !strcmp(ifname,"ppp10")){
			RG_pptp_ip_chagne(ifname, new_ip, ifa_local);
		}
	}
#endif /*CONFIG_USER_PPTP_CLIENT_PPTP*/	
#ifdef CONFIG_USER_L2TPD_L2TPD
	mib_get(MIB_L2TP_ENABLE, (void *)&enable);
	if(enable){
		if(!strcmp(ifname,"ppp11") || !strcmp(ifname,"ppp12")){
			RG_l2tp_ip_chagne(ifname, new_ip, ifa_local);
		}
	}
#endif /*CONFIG_USER_L2TPD_L2TPD*/	
#endif

	/********************** Important **************************************************
	/  If it is dynamical link and dnsMode = REQUEST_DNS, we should wait the interface's resolv.conf is ready.
	/********************************************************************************/
	sleep(1);
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
	cmd_set_dns_config(ifname);	
	restart_dnsrelay();
#endif

#ifdef TIME_ZONE
	// kick sntpc to sync the time
	my_pid = read_pid(SNTPC_PID);
	if ( my_pid > 0) {			
		kill(my_pid, SIGUSR1);
	}
#endif
#ifdef CONFIG_USER_DDNS
	do_ddnsc(ifname, msg_type);
#endif

	return 0;
}

int ip_removed_dependency(char *ifname, struct in_addr ipaddr)
{
#ifdef CONFIG_RTK_RG_INIT
	int wan_ifIndex;
	int ret;
	MIB_CE_ATM_VC_T vc_entry = {0};
	char sVal[32];
	int dhcpc_pid;
#ifdef CONFIG_USER_PPPOMODEM
        if(check_34G_status(ifname , 1 ) == 1)
                return 0;
#endif
	wan_ifIndex = getIfIndexByName(ifname);
	if (wan_ifIndex == -1) return 0;
	ret = (int)getATMVCEntryByIfIndex(wan_ifIndex, &vc_entry);
	if (!ret) return 0;
	if (!vc_entry.enable) return 0;

	switch (vc_entry.cmode)
	{
		case CHANNEL_MODE_IPOE:
			if(vc_entry.ipDhcp==DHCP_CLIENT)
			{
				DBG("Removing RG DHCP client info for %d\n", vc_entry.rg_wan_idx);
				RG_release_dhcp(vc_entry.rg_wan_idx);
			}
			else
			{
				DBG("Removing RG Static info: rg_index=%d\n", vc_entry.rg_wan_idx);
				RG_release_static(vc_entry.rg_wan_idx);
			}
			break;
		case CHANNEL_MODE_PPPOE:
			DBG("Removing RG PPPOE info: rg_index=%d\n", vc_entry.rg_wan_idx);
			RG_release_pppoe(vc_entry.rg_wan_idx);

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
			update_PON_BW(PON_BW_NORMAL, ifname);
#endif
			break;
		default:
			break;
	}
	if(vc_entry.dgw){
		//AUG_PRT("%s-%d defualt gateway off, internet led off!!\n",__func__,__LINE__);
		system("echo 0 > /proc/internet_flag");
	}
#endif

	return 0;
}

int link_change_dependency(char *ifname, int ifi_flags)
{
	int wan_ifIndex;
	int ret;
	MIB_CE_ATM_VC_T vc_entry = {0};
	char sVal[32];
	int dhcpc_pid;

	if(!(ifi_flags & IFF_UP))
	{
#ifdef CONFIG_USER_PPPOMODEM
		if(check_34G_status(ifname , 2 ) == 1)
                	return 0;
#endif
		/*If Droute net dev down, turn off INET LED*/
		wan_ifIndex = getIfIndexByName(ifname);
		if (wan_ifIndex == -1) return 0;
		ret = (int)getATMVCEntryByIfIndex(wan_ifIndex, &vc_entry);	
		if(vc_entry.dgw){
			//AUG_PRT("%s-%d defualt gateway off, internet led off!!\n",__func__,__LINE__);
			system("echo 0 > /proc/internet_flag");
		}	
		DBG("%s is down, skip\n", ifname);
#ifdef CONFIG_MIDDLEWARE
		printf("======%s:Send InformKeyPara to MidProcess: ifname = %s\n",__func__,ifname);
		sendInformKeyParaMsg2MW(ifname);
#endif
		return;
	}
		
	
	wan_ifIndex = getIfIndexByName(ifname);
	if (wan_ifIndex == -1) return 0;
	ret = (int)getATMVCEntryByIfIndex(wan_ifIndex, &vc_entry);
	if (!ret) return 0;
	if (!vc_entry.enable) return 0;
	
	switch (vc_entry.cmode) {
		case CHANNEL_MODE_IPOE:
		case CHANNEL_MODE_RT1483:
		case CHANNEL_MODE_RT1577:
			if ((DHCP_T)vc_entry.ipDhcp == DHCP_DISABLED)
			{
				// Static
				if(ifi_flags & IFF_RUNNING)
				{
					startIP(ifname, &vc_entry, vc_entry.cmode);
#if defined(CONFIG_IPV6)
					if ((vc_entry.IpProtocol & IPVER_IPV6) && (vc_entry.AddrMode==IPV6_WAN_STATIC)){
						startIP_for_V6(&vc_entry);
					}
 
#endif
				}
				else
				{
					char cmd[256] = {0};
					sprintf(cmd, "ifconfig %s 0.0.0.0", ifname);
					system(cmd);

#if defined(CONFIG_IPV6)
					if ((vc_entry.IpProtocol & IPVER_IPV6) && (vc_entry.AddrMode==IPV6_WAN_STATIC)){
						unsigned char   Ipv6AddrStr[48];

						inet_ntop(PF_INET6, (struct in6_addr *)&vc_entry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));	
						snprintf(Ipv6AddrStr, 48, "%s/%d", Ipv6AddrStr, vc_entry.Ipv6AddrPrefixLen);

						sprintf(cmd, "ifconfig %s del %s", ifname,Ipv6AddrStr);
						printf("Execute %s\n",cmd);
						system(cmd);
					}
 
#endif
				}
			}
			else
			{
				// DHCP Client
				snprintf(sVal, 32, "%s.%s", (char*)DHCPC_PID, ifname);
				dhcpc_pid = read_pid((char*)sVal);
				if (dhcpc_pid > 0) {
					if(ifi_flags & IFF_RUNNING)
						kill(dhcpc_pid, SIGUSR1); // force renew
					else
						kill(dhcpc_pid, SIGUSR2); // force release
				}
			}

#if defined(CONFIG_IPV6)
			if (vc_entry.IpProtocol & IPVER_IPV6) {
				if(  (vc_entry.Ipv6Dhcp == 1) && ((vc_entry.Ipv6DhcpRequest & 0x1) == 0x1)){
					if(ifi_flags & IFF_RUNNING)
						startIP_for_V6(&vc_entry);
					else{
						unsigned char   Ipv6AddrStr[48];
						char cmd[256] = {0};
						
						stopIP_PPP_for_V6(&vc_entry);
						inet_ntop(PF_INET6, (struct in6_addr *)&vc_entry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));	
						snprintf(Ipv6AddrStr, 48, "%s/64", Ipv6AddrStr);

						sprintf(cmd, "ifconfig %s del %s", ifname,Ipv6AddrStr);
							printf("Execute %s\n",cmd);
						system(cmd);
					}
				}
			}
 
#endif

			break;
		default:
			break;
	}
}

#if defined(CONFIG_IPV6) && defined(NEW_PORTMAPPING)
void handle_route(char *ifname, char *gateway, int reset)
{
	int i;
	MIB_CE_ATM_VC_T Entry;
	int total =  mib_chain_total(MIB_ATM_VC_TBL);
	int tblid;
	char str_tblid[6] = {0};

	for(i=0 ; i < total ; i++)
	{
		char tmp_ifname[IFNAMSIZ] = {0};
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;

		ifGetName(Entry.ifIndex, tmp_ifname, IFNAMSIZ);
		if(strcmp(tmp_ifname, ifname) == 0)
			break;
	}

	if(i == total)
	{
		fprintf(stderr, "Cannot find interface %s in ATM_VC_TBL\n", ifname);
		return;
	}

	tblid = caculate_tblid(Entry.ifIndex);
	snprintf(str_tblid, 8, "%d", tblid);

	va_cmd("/bin/ip", 6, 1, "-6", "route", "del", "default", "table", str_tblid);

	if(Entry.itfGroup == 0)
		return ;

	if(reset)
	{
		va_cmd("/bin/ip", 8, 1, "-6", "route", "add", "default", "dev", ifname
			, "table", str_tblid);
	}
	else
	{
		va_cmd("/bin/ip", 10, 1, "-6", "route", "add", "default", "dev", ifname
			, "via", gateway, "table", str_tblid);
	}
}
#endif


#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE) && defined(CONFIG_RTK_RG_INIT)

int RG_setup_dslite_IPoE_v6(MIB_CE_ATM_VC_T *pEntry)
{
	rtk_rg_ipDslitStaticInfo_t *dslite_info=NULL;
	rtk_rg_intfInfo_t intf_info;
	rtk_ipv6_addr_t zeroIPv6={{0}};
	int i,ret,entry_index;
	char aftr_addr_str[40]={0},tmpBuf[64]={0};

	if(!pEntry ){
		printf("Error! NULL input!\n");
		return -1;
	}

	printf("[%s] Find rg intf_info, rg_wan_idx=%d \n",__func__,pEntry->rg_wan_idx);
	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &pEntry->rg_wan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",pEntry->rg_wan_idx);
		return -1;
	}
	

	printf("[%s] Update IPv6 part to dslite_infofterDial\n",__func__);
	dslite_info = &(intf_info.wan_intf.dslite_info);
	dslite_info->static_info.mtu = pEntry->mtu;
	dslite_info->static_info.ip_addr = 0;
	dslite_info->static_info.ip_network_mask = 0;
	dslite_info->static_info.ipv6_mask_length = 64;
	dslite_info->static_info.ipv4_default_gateway_on = pEntry->dgw;
	dslite_info->static_info.ipv6_default_gateway_on = pEntry->dgw;
	dslite_info->static_info.gw_mac_auto_learn_for_ipv6=1;
	dslite_info->aftr_mac_auto_learn =1;

	if((pEntry->dslite_aftr_hostname[0]!=0)){
		printf("have dslite_aftr_hostname [%s] , do dns query for this!\n",pEntry->dslite_aftr_hostname);
		query_aftr(pEntry->dslite_aftr_hostname,pEntry->dslite_aftr_addr, aftr_addr_str);
	}

	if (mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)tmpBuf) == 0){
		printf("Error! %s MIB get IPV6_LAN_IP_ADDR fail!\n",__func__);
		return -1;
	}

	if(strcasecmp(tmpBuf,aftr_addr_str)==0){
		printf("Query  %s, result is %s, dns not ready!\n",pEntry->dslite_aftr_hostname,aftr_addr_str);
		return -1;
	}

	printf("dslite_aftr_hostname %s, %s\n",pEntry->dslite_aftr_hostname,aftr_addr_str);

	memcpy((void *)dslite_info->static_info.ipv6_addr.ipv6_addr, (void *)pEntry->Ipv6Addr ,sizeof(struct in6_addr));
	memcpy((void *)dslite_info->static_info.gateway_ipv6_addr.ipv6_addr, (void *)pEntry->RemoteIpv6Addr ,sizeof(struct in6_addr));
	memcpy((void *)dslite_info->rtk_dslite.ipB4.ipv6_addr, (void *)pEntry->Ipv6Addr ,sizeof(struct in6_addr));
	memcpy((void *)dslite_info->rtk_dslite.ipAftr.ipv6_addr, (void *)pEntry->dslite_aftr_addr ,sizeof(struct in6_addr));
		
	dslite_info->static_info.ip_version = IPVER_V4V6;

	ret = rtk_rg_dsliteInfo_set(pEntry->rg_wan_idx, dslite_info);
	printf("[%s] update ret=%d\n",__func__,ret);

	return ret;
}

int RG_setup_dslite_PPPoE_v6(MIB_CE_ATM_VC_T *pEntry)
{
	rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeClientiDslisteInfoA=NULL;
	int total = mib_chain_total(MIB_PPPOE_SESSION_TBL);
	MIB_CE_PPPOE_SESSION_T session = {0};
	rtk_rg_intfInfo_t intf_info;
	rtk_ipv6_addr_t zeroIPv6={{0}};
	int i,ret,entry_index;
	char aftr_addr_str[40]={0},tmpBuf[64]={0};

	printf("[%s-enter]\n",__func__);
	if(!pEntry ){
		printf("Error! NULL input!\n");
		return -1;
	}
	
	printf("[%s] Find rg intf_info, rg_wan_idx=%d \n",__func__,pEntry->rg_wan_idx);
	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &pEntry->rg_wan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",pEntry->rg_wan_idx);
		return -1;
	}

	printf("[%s] Update IPv6 part to pppoeClientiDslisteInfoAfterDial\n",__func__);
	pppoeClientiDslisteInfoA = &(intf_info.wan_intf.pppoe_dslite_info.after_dial);
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.mtu = pEntry->mtu;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ip_addr = 0;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ip_network_mask = 0;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv6_mask_length = 64;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv4_default_gateway_on = pEntry->dgw;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv6_default_gateway_on = pEntry->dgw;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv4=0;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv6=0;

	if((pEntry->dslite_aftr_hostname[0]!=0)){
		printf("have dslite_aftr_hostname [%s] , do dns query for this!\n",pEntry->dslite_aftr_hostname);
		query_aftr(pEntry->dslite_aftr_hostname,pEntry->dslite_aftr_addr, aftr_addr_str);
	}

	if (mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)tmpBuf) == 0){
		printf("Error! %s MIB get IPV6_LAN_IP_ADDR fail!\n",__func__);
		return -1;
	}

	if(strcasecmp(tmpBuf,aftr_addr_str)==0){
		printf("Query  %s, result is %s, dns not ready!\n",pEntry->dslite_aftr_hostname,aftr_addr_str);
		return -1;
	}

	printf("dslite_aftr_hostname %s, %s\n",pEntry->dslite_aftr_hostname,aftr_addr_str);

	memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv6_addr.ipv6_addr, (void *)pEntry->Ipv6Addr ,sizeof(struct in6_addr));
	memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr, (void *)pEntry->RemoteIpv6Addr ,sizeof(struct in6_addr));
	memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, (void *)pEntry->Ipv6Addr ,sizeof(struct in6_addr));
	memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, (void *)pEntry->dslite_aftr_addr ,sizeof(struct in6_addr));
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ip_version = IPVER_V4V6;

	//Get PPPoE Gateway mac
	for(i = 0 ; i < total ; i++)
	{
		mib_chain_get(MIB_PPPOE_SESSION_TBL, i, &session);
		if(session.uifno == pEntry->ifIndex)
		{
			memcpy(pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_mac_addr_for_ipv4.octet, session.acMac, 6);
			memcpy(pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet, session.acMac, 6);
			pppoeClientiDslisteInfoA->dslite_hw_info.aftr_mac_auto_learn =0;
//It's Point to Point, could not auto learn
			memcpy(pppoeClientiDslisteInfoA->dslite_hw_info.aftr_mac_addr.octet, session.acMac, 6);
			pppoeClientiDslisteInfoA->sessionId = session.sessionId;
/*
			printf("sessionId=%d\n",pppoeClientiDslisteInfoA->sessionId);
			{
				int index=0;
				for(index=0;index<6;index++)
					printf("%2x ",pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_mac_addr_for_ipv4.octet[index]);
			}
*/
		}
	}
	
	ret = rtk_rg_pppoeDsliteInfoAfterDial_set(pEntry->rg_wan_idx, pppoeClientiDslisteInfoA);
	printf("[%s] update ret=%d\n",__func__,ret);
	printf("[%s-leave]\n",__func__);
	return ret;
}
static int getATM_VC_ENTRY_enable_dslite(MIB_CE_ATM_VC_T *pEntry, int *entry_index)
{
	char ifname[IFNAMSIZ];
	int entryNum,i;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)pEntry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (pEntry->dslite_enable == 0)
			continue;
		else{
			printf("Found entry %d enable dslite\n",i);
			break;
		}
	}
	
	if(i==entryNum)
		return 0;

	*entry_index =i;
	return 1;
}
#endif

static dslite_thread_enabled = 0;
void *thread_setup_dslite(void *data)
{
	MIB_CE_ATM_VC_T Entry;
	int i,ret,entry_index;
#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
	rtk_rg_intfInfo_t intf_info;
#endif
	int retry_count=0;
	char cmd[100];
	char str_ip[64],str_gateway[64];
	char ifname[IFNAMSIZ];
	FILE *fp=NULL;

	
#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
	dslite_thread_enabled = 1;
	printf("[%s:%d]\n",__func__,__LINE__);
	//find the ATM_VC entry enable dslite
	ret= getATM_VC_ENTRY_enable_dslite(&Entry,&entry_index);
		if(ret==0){
			printf("Not found ATM_VC_TBL interface enable dslite !\n");
			goto leave;
		}
	//find the RG entry by rg_wan_idx
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
		if((ret==0) && intf_info.is_wan){
			printf("[%s:%d]\n",__func__,__LINE__);
			do{
				if(getATM_VC_ENTRY_enable_dslite(&Entry,&entry_index)==0){	 //prevent wan deleted, but this thread always continued.
					printf("Not Found ATM_VC_TBL interface enable dslite Leaving thread.. for dslite!\n");
					goto leave;
				}

				switch(intf_info.wan_intf.wan_intf_conf.wan_type)
				{
					//Check IPoE or PPPoE	
					case RTK_RG_DSLITE:
									   printf("[%s] Setup IPv6  part to wan type RTK_RG_DSLITE\n",__func__);
										   ret = RG_setup_dslite_IPoE_v6(&Entry);
									   break;
					case RTK_RG_PPPoE_DSLITE:
											 printf("[%s] Setup IPv6 part to wan type RTK_RG_PPPoE_DSLITE\n",__func__);
												 ret = RG_setup_dslite_PPPoE_v6(&Entry);
												 break;
				}

				printf("[%s:%d]\n",__func__,__LINE__);


				printf("[%s:%d] Reset dnsmasq\n",__func__,__LINE__);
				setup_dnsmasq(0);
				sleep(5);
				retry_count++;
				printf("%s, setup dslite retry_count=%d\n",__func__,retry_count);
			} while(ret!=RT_ERR_RG_OK);
		}

	printf("Finish set DS-Lite for RG, now set software part!\n");
	inet_ntop(AF_INET6, (const void *) Entry.Ipv6Addr, str_ip, 64);
	inet_ntop(AF_INET6, (const void *) Entry.dslite_aftr_addr, str_gateway, 64);
	ifGetName(Entry.ifIndex, ifname, sizeof(ifname));

	//ip -6 tunnel del tun1
	sprintf(cmd,"ip  -6 tunnel del tun1\n");	
	printf("cmd:%s\n",cmd);
	system(cmd);

	//ip -6 tunnel add tun1 mode ipip6 local 2001::2 remote 2001::1 dev ppp0
	sprintf(cmd,"ip -6 tunnel add tun1 mode ipip6 local %s remote %s dev %s\n",str_ip,str_gateway,ifname);	
	printf("cmd:%s\n",cmd);
	system(cmd);

	//ip link set dev tun1 up
	sprintf(cmd,"ip link set dev tun1 up\n");	
	printf("cmd:%s\n",cmd);
	system(cmd);
	
	//ip route add default dev tun0 
	sprintf(cmd,"ip route add default dev tun1\n");	
	printf("cmd:%s\n",cmd);
	system(cmd);

	fp = fopen("/var/dslite_info", "w");
	if(fp)
	{
		fprintf(fp, "ifname: %s\n", ifname);
		fprintf(fp, "aftr hostname: %s\n", Entry.dslite_aftr_hostname);
		fprintf(fp, "aftr IPv6 addr: %s\n", str_gateway);
		fclose(fp);
	}
leave:
	dslite_thread_enabled = 0;
	printf("[%s:%d] Leaving thread.. for dslite\n",__func__,__LINE__);


#endif
	return NULL;
}


void setup_dslite()
{
#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
	pthread_t id;

	printf("[%s:%d] ",__func__,__LINE__);
	if(dslite_thread_enabled==0){
		printf("[%s:%d] Create new theread for dslite\n",__func__,__LINE__);
		pthread_create(&id, NULL, thread_setup_dslite, NULL);
	}
	else{
		printf("[%s:%d] dslite thread already existed!!!! Should not entry here again!\n",__func__,__LINE__);
		return;
	}
	
	pthread_detach(id);
#endif
}
#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)

typedef enum { WANIF_IPV6_ADDR , GATEWAY_IPV6_ADDR } IPV6_ADDR_KIND;

static int getATM_VC_ENTRY_byName(char *pIfname, MIB_CE_ATM_VC_T *pEntry, int *entry_index)
{
	unsigned int entryNum, i;
	char ifname[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)pEntry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (pEntry->enable == 0)
			continue;

		ifGetName(pEntry->ifIndex,ifname,sizeof(ifname));

		if(!strcmp(ifname,pIfname)){
			break;
		}
	}

	if(i>= entryNum){
		//printf("not find this interface!\n");
		return -1;
	}

	*entry_index = i;
	return 0;
}

#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
int RG_update_dslite_IPoE_v6(char *ifname, struct in6_addr *ip6addr, int updateType)
{
	rtk_rg_ipDslitStaticInfo_t *dslite_info=NULL;
	int zero=0;
	int total = mib_chain_total(MIB_PPPOE_SESSION_TBL);
	MIB_CE_PPPOE_SESSION_T session = {0};
	rtk_rg_intfInfo_t intf_info;
	int i,ret,entry_index;
	MIB_CE_ATM_VC_T Entry;
	char aftr_addr_str[40]={0},tmpBuf[64]={0};

	if(!ifname || !ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}

	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry,&entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
		printf("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",Entry.rg_wan_idx);
		return -1;
	}
	

	printf("[%s] Update IPv6 part to dslite_infofterDial\n",__func__);
	dslite_info = &(intf_info.wan_intf.dslite_info);
	dslite_info->static_info.mtu = Entry.mtu;
	dslite_info->static_info.ip_addr = 0;
	dslite_info->static_info.ip_network_mask = 0;
	dslite_info->static_info.ipv6_mask_length = 64;
	dslite_info->static_info.ipv4_default_gateway_on = Entry.dgw;
	dslite_info->static_info.ipv6_default_gateway_on = Entry.dgw;
	dslite_info->static_info.gw_mac_auto_learn_for_ipv6=1;
	dslite_info->aftr_mac_auto_learn =1;

	if((Entry.dslite_aftr_hostname[0]!=0)){
		printf("have dslite_aftr_hostname [%s] , do dns query for this!\n",Entry.dslite_aftr_hostname);
		query_aftr(Entry.dslite_aftr_hostname,Entry.dslite_aftr_addr, aftr_addr_str);
	}

	if (mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)tmpBuf) == 0){
		printf("Error! %s MIB get IPV6_LAN_IP_ADDR fail!\n",__func__);
		return -1;
	}

	if(strcasecmp(tmpBuf,aftr_addr_str)==0){
		printf("Query  %s, result is %s, dns not ready!\n",Entry.dslite_aftr_hostname,aftr_addr_str);
		return -1;
	}

	printf("dslite_aftr_hostname %s, %s\n",Entry.dslite_aftr_hostname,aftr_addr_str);

	if(updateType == WANIF_IPV6_ADDR){
		memcpy((void *)dslite_info->static_info.ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
		memcpy((void *)dslite_info->static_info.gateway_ipv6_addr.ipv6_addr, (void *)&Entry.RemoteIpv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)dslite_info->rtk_dslite.ipB4.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
		memcpy((void *)dslite_info->rtk_dslite.ipAftr.ipv6_addr, (void *)&Entry.dslite_aftr_addr ,sizeof(struct in6_addr));
	}
	else{
		memcpy((void *)dslite_info->static_info.ipv6_addr.ipv6_addr, (void *)&Entry.Ipv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)dslite_info->static_info.gateway_ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
		memcpy((void *)dslite_info->rtk_dslite.ipB4.ipv6_addr, (void *)&Entry.Ipv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)dslite_info->rtk_dslite.ipAftr.ipv6_addr, (void *)&Entry.dslite_aftr_addr ,sizeof(struct in6_addr));
	}
	dslite_info->static_info.ip_version = IPVER_V4V6;

	ret = rtk_rg_dsliteInfo_set(Entry.rg_wan_idx, dslite_info);
	printf("[%s] update ret=%d\n",__func__,ret);
}

int RG_update_dslite_PPPoE_v6(char *ifname, struct in6_addr *ip6addr, int updateType)
{
	rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeClientiDslisteInfoA=NULL;
	int zero=0;
	int total = mib_chain_total(MIB_PPPOE_SESSION_TBL);
	MIB_CE_PPPOE_SESSION_T session = {0};
	rtk_rg_intfInfo_t intf_info;
	int i,ret,entry_index;
	MIB_CE_ATM_VC_T Entry;
	char aftr_addr_str[40]={0},tmpBuf[64]={0};

	if(!ifname || !ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}

	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry,&entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
		printf("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",Entry.rg_wan_idx);
		return -1;
	}
	

	printf("[%s] Update IPv6 part to pppoeClientiDslisteInfoAfterDial\n",__func__);
	pppoeClientiDslisteInfoA = &(intf_info.wan_intf.pppoe_dslite_info.after_dial);
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.mtu = Entry.mtu;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ip_addr = 0;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ip_network_mask = 0;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv6_mask_length = 64;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv4_default_gateway_on = Entry.dgw;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv6_default_gateway_on = Entry.dgw;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv4=0;
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gw_mac_auto_learn_for_ipv6=0;

	if((Entry.dslite_aftr_hostname[0]!=0)){
		printf("have dslite_aftr_hostname [%s] , do dns query for this!\n",Entry.dslite_aftr_hostname);
		query_aftr(Entry.dslite_aftr_hostname,Entry.dslite_aftr_addr, aftr_addr_str);
	}

	if (mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)tmpBuf) == 0){
		printf("Error! %s MIB get IPV6_LAN_IP_ADDR fail!\n",__func__);
		return -1;
	}

	if(strcasecmp(tmpBuf,aftr_addr_str)==0){
		printf("Query  %s, result is %s, dns not ready!\n",Entry.dslite_aftr_hostname,aftr_addr_str);
		return -1;
	}

	printf("dslite_aftr_hostname %s, %s\n",Entry.dslite_aftr_hostname,aftr_addr_str);

	if(updateType == WANIF_IPV6_ADDR){
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr, (void *)&Entry.RemoteIpv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, (void *)&Entry.dslite_aftr_addr ,sizeof(struct in6_addr));
	}
	else{
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ipv6_addr.ipv6_addr, (void *)&Entry.Ipv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, (void *)&Entry.Ipv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientiDslisteInfoA->dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, (void *)&Entry.dslite_aftr_addr ,sizeof(struct in6_addr));
	}
	pppoeClientiDslisteInfoA->dslite_hw_info.static_info.ip_version = IPVER_V4V6;

	//Get PPPoE Gateway mac
	for(i = 0 ; i < total ; i++)
	{
		mib_chain_get(MIB_PPPOE_SESSION_TBL, i, &session);
		if(session.uifno == Entry.ifIndex)
		{
			memcpy(pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_mac_addr_for_ipv4.octet, session.acMac, 6);
			memcpy(pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_mac_addr_for_ipv6.octet, session.acMac, 6);
			pppoeClientiDslisteInfoA->dslite_hw_info.aftr_mac_auto_learn =0;//It's Point to Point, could not auto learn
			memcpy(pppoeClientiDslisteInfoA->dslite_hw_info.aftr_mac_addr.octet, session.acMac, 6);
			pppoeClientiDslisteInfoA->sessionId = session.sessionId;
/*
			printf("sessionId=%d\n",pppoeClientiDslisteInfoA->sessionId);
			{
				int index=0;
				for(index=0;index<6;index++)
					printf("%2x ",pppoeClientiDslisteInfoA->dslite_hw_info.static_info.gateway_mac_addr_for_ipv4.octet[index]);
			}
*/
		}
	}
	
	ret = rtk_rg_pppoeDsliteInfoAfterDial_set(Entry.rg_wan_idx, pppoeClientiDslisteInfoA);
	printf("[%s] update ret=%d\n",__func__,ret);
}
#endif

int RG_update_PPPoE_v6(char *ifname, struct in6_addr *ip6addr, int updateType)
{
	rtk_rg_pppoeClientInfoAfterDial_t *pppoeClientInfoA=NULL;
	int zero=0;
	int total = mib_chain_total(MIB_PPPOE_SESSION_TBL);
	MIB_CE_PPPOE_SESSION_T session = {0};
	rtk_rg_intfInfo_t intf_info;
	int i,ret,entry_index=0;
	MIB_CE_ATM_VC_T Entry;

	if(!ifname || !ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}

	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry,&entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
		printf("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",Entry.rg_wan_idx);
		return -1;
	}
	

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
	update_PON_BW(PON_BW_ENLARGE, ifname);
#endif

	printf("[%s] Update IPv6 part to pppoeClientInfoAfterDial\n",__func__);
	pppoeClientInfoA = &(intf_info.wan_intf.pppoe_info.after_dial);
	pppoeClientInfoA->hw_info.ip_version = (Entry.IpProtocol==IPVER_IPV6)?IPVER_V6ONLY:IPVER_V4V6;
	pppoeClientInfoA->hw_info.ipv6_mask_length = 64;
	pppoeClientInfoA->hw_info.mtu = Entry.mtu;
	pppoeClientInfoA->hw_info.gw_mac_auto_learn_for_ipv6 = 0;
	pppoeClientInfoA->hw_info.ipv6_default_gateway_on = Entry.dgw;

	if(updateType == WANIF_IPV6_ADDR){
		memcpy((void *)pppoeClientInfoA->hw_info.gateway_ipv6_addr.ipv6_addr, (void *)&Entry.RemoteIpv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientInfoA->hw_info.ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
	}
	else{
		memcpy((void *)pppoeClientInfoA->hw_info.ipv6_addr.ipv6_addr, (void *)&Entry.Ipv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)pppoeClientInfoA->hw_info.gateway_ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
	}

	if(Entry.IpProtocol==IPVER_IPV4_IPV6){
		//If IPv4 is not ready, set IPv6 only
		if(pppoeClientInfoA->hw_info.ip_addr == 0 || pppoeClientInfoA->hw_info.ip_network_mask == 0)
			pppoeClientInfoA->hw_info.ip_version = IPVER_V6ONLY;
		else
			pppoeClientInfoA->hw_info.ip_version = IPVER_V4V6;
	} else if(Entry.IpProtocol==IPVER_IPV6)
			pppoeClientInfoA->hw_info.ip_version = IPVER_V6ONLY;

	//Get PPPoE Gateway mac
	for(i = 0 ; i < total ; i++)
	{
		mib_chain_get(MIB_PPPOE_SESSION_TBL, i, &session);
		if(session.uifno == Entry.ifIndex)
		{
			memcpy(pppoeClientInfoA->hw_info.gateway_mac_addr_for_ipv6.octet, session.acMac, 6);
			pppoeClientInfoA->sessionId = session.sessionId;
		}
	}
	
	ret = rtk_rg_pppoeClientInfoAfterDial_set(Entry.rg_wan_idx, pppoeClientInfoA);
	printf("[%s] update ret=%d\n",__func__,ret);

}
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_E8B)
const char RG_PATCH_FOR_AVALANCHE_WAN_ACL[] = "/var/rg_patch_for_avalanche_wan_acl";
const char RG_PATCH_FOR_AVALANCHE_LAN_ACL[] = "/var/rg_patch_for_avalanche_lan_acl";
static struct in6_addr ip6addr_wan;
static struct in6_addr ip6addr_lan;

int Flush_RG_ACL_PATCH_FOR_AVALANCHE_LAN(void)
{
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_PATCH_FOR_AVALANCHE_LAN_ACL, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_PATCH_FOR_AVALANCHE_LAN_ACL);
	return 0;
}
int Flush_RG_ACL_PATCH_FOR_AVALANCHE_WAN(void)
{
	FILE *fp;
	int acl_idx;

	if(!(fp = fopen(RG_PATCH_FOR_AVALANCHE_WAN_ACL, "r")))
		return -2;

	while(fscanf(fp, "%d\n", &acl_idx) != EOF)
	{
		if(rtk_rg_aclFilterAndQos_del(acl_idx))
			DBPRINT(1, "rtk_rg_aclFilterAndQos_del failed! idx = %d\n", acl_idx);
	}

	fclose(fp);
	unlink(RG_PATCH_FOR_AVALANCHE_WAN_ACL);
	return 0;
}
#endif
int handle_ipv6_addr_for_RG_lan(struct in6_addr *ip6addr)
{
	int ret=-1;
	rtk_rg_intfInfo_t intf_info;
	const int lan_idx=0; //NOTE: LAN should be first interface to add, so be 0.
	rtk_rg_lanIntfConf_t *lanIntfConf;

	if(!ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}
	
	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &lan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",lan_idx);
		return -1;
	}
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_E8B)
	if(patch_for_avalanche)
	{
		if(memcmp((void*)&ip6addr_lan,(void*)ip6addr,sizeof(struct in6_addr)) == 0){
			//AUG_PRT("%s-%d\n",__func__,__LINE__);
			goto SKIP_ACL_LAN;
		}
		memcpy((void*)&ip6addr_lan,(void*)ip6addr,sizeof(struct in6_addr));	
		Flush_RG_ACL_PATCH_FOR_AVALANCHE_LAN();
		/*
		rg clear acl-filter
		rg set acl-filter fwding_type_and_direction 0
		rg set acl-filter action action_type 2
		rg set acl-filter pattern ingress_dest_ipv6_addr 2013:0012:0000:0000:0000:0000:0000:0090 //br0 ip
		rg set acl-filter pattern ingress_dest_ipv6_addr_mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
		rg set acl-filter pattern ingress_port_mask 0xf
		rg add acl-filter entry
		*/
		FILE *fp;
	 	int aclIdx;
		rtk_rg_aclFilterAndQos_t aclRule;
		
		if(!(fp = fopen(RG_PATCH_FOR_AVALANCHE_LAN_ACL, "a")))
		{
			fprintf(stderr, "ERROR! %s\n", strerror(errno));
			return -2;
		}
		memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
		aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
		aclRule.action_type = ACL_ACTION_TYPE_TRAP;
		aclRule.filter_fields |= INGRESS_PORT_BIT;
#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)			
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)
		aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
#endif
		aclRule.filter_fields |= INGRESS_IPV6_DIP_BIT;
		memcpy((void *)aclRule.ingress_dest_ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
		memset((void *)aclRule.ingress_dest_ipv6_addr_mask, 0xff ,sizeof(struct in6_addr));
		//rg add acl-filter entry
		if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
			fprintf(fp,"%d\n",aclIdx);
			AUG_PRT("add %s-%d index=%d success\n",__func__,__LINE__, aclIdx);
		}else{
			fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
			fclose(fp);
			return -1;
		}
		fclose(fp);
	}

	SKIP_ACL_LAN:;
#endif
	lanIntfConf = &(intf_info.lan_intf);
	lanIntfConf->replace_subnet = 1;
	lanIntfConf->ipv6_network_mask_length = 64; //NOTE: hard code to 64 temporarily
	memcpy((void *)lanIntfConf->ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));

	if(lanIntfConf->ip_addr == 0 || lanIntfConf->ip_network_mask == 0)
		lanIntfConf->ip_version = IPVER_V6ONLY;
	else
		lanIntfConf->ip_version = IPVER_V4V6;
	
	ret = rtk_rg_lanInterface_add(lanIntfConf, &lan_idx);
	DBG("[%s] ret=%d\n",__func__,ret);

	return ret;	
}

int RG_update_DHCP_v6(char *ifname, struct in6_addr *ip6addr, int updateType)
{
	rtk_rg_ipDhcpClientInfo_t *dhcpClientInfo = NULL;
	int zero=0;
	rtk_rg_intfInfo_t intf_info;
	int i,ret,entry_index=0;
	MIB_CE_ATM_VC_T Entry;

	if(!ifname || !ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}

	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry, &entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
		printf("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",Entry.rg_wan_idx);
		return -1;
	}

	dhcpClientInfo = &(intf_info.wan_intf.dhcp_client_info);
	dhcpClientInfo->hw_info.mtu = Entry.mtu;
	dhcpClientInfo->hw_info.gw_mac_auto_learn_for_ipv6 = 1;
	dhcpClientInfo->hw_info.ipv6_default_gateway_on = Entry.dgw;
	dhcpClientInfo->hw_info.ipv6_mask_length = 64;

	if(updateType == WANIF_IPV6_ADDR){
		if(dhcpClientInfo->hw_info.ipv6_default_gateway_on)
		memcpy((void *)dhcpClientInfo->hw_info.gateway_ipv6_addr.ipv6_addr, (void *)&Entry.RemoteIpv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)dhcpClientInfo->hw_info.ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
	}
	else{
		memcpy((void *)dhcpClientInfo->hw_info.ipv6_addr.ipv6_addr, (void *)&Entry.Ipv6Addr ,sizeof(struct in6_addr));
		if(dhcpClientInfo->hw_info.ipv6_default_gateway_on)
		memcpy((void *)dhcpClientInfo->hw_info.gateway_ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
	}

	if(Entry.IpProtocol==IPVER_IPV4_IPV6){
		//If IPv4 is not ready, set IPv6 only
		if(dhcpClientInfo->hw_info.ip_addr == 0 || dhcpClientInfo->hw_info.ip_network_mask == 0)
			dhcpClientInfo->hw_info.ip_version = IPVER_V6ONLY;
		else
			dhcpClientInfo->hw_info.ip_version = IPVER_V4V6;
	}else if(Entry.IpProtocol==IPVER_IPV6)
			dhcpClientInfo->hw_info.ip_version = IPVER_V6ONLY;

	ret = rtk_rg_dhcpClientInfo_set(Entry.rg_wan_idx, dhcpClientInfo);
	printf("[%s] update ret=%d\n",__func__,ret);

}


int RG_update_Static_v6(char *ifname, struct in6_addr *ip6addr, int updateType)
{
	rtk_rg_ipStaticInfo_t *staticInfo=NULL;
	int zero=0;
	rtk_rg_intfInfo_t intf_info;
	int i,ret,entry_index=0;
	MIB_CE_ATM_VC_T Entry;

	if(!ifname || !ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}

	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry,&entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
		printf("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	if(ret!=0){
		printf("Error! Find RG interface index %d Fail!\n",Entry.rg_wan_idx);
		return -1;
	}

	staticInfo = &(intf_info.wan_intf.static_info);
	staticInfo->mtu = Entry.mtu;
	staticInfo->gw_mac_auto_learn_for_ipv6 = 1;
	staticInfo->ipv6_default_gateway_on = Entry.dgw;
	staticInfo->ipv6_mask_length = Entry.Ipv6AddrPrefixLen;

	if(updateType == WANIF_IPV6_ADDR){
		if(staticInfo->ipv6_default_gateway_on)
		memcpy((void *)staticInfo->gateway_ipv6_addr.ipv6_addr, (void *)&Entry.RemoteIpv6Addr ,sizeof(struct in6_addr));
		memcpy((void *)staticInfo->ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
	}
	else{
		memcpy((void *)staticInfo->ipv6_addr.ipv6_addr, (void *)&Entry.Ipv6Addr ,sizeof(struct in6_addr));
		if(staticInfo->ipv6_default_gateway_on)
		memcpy((void *)staticInfo->gateway_ipv6_addr.ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
	}

	if(Entry.IpProtocol==IPVER_IPV4_IPV6){
		//If IPv4 is not ready, set IPv6 only
		if(staticInfo->ip_addr == 0 || staticInfo->ip_network_mask == 0)
			staticInfo->ip_version = IPVER_V6ONLY;
		else
			staticInfo->ip_version = IPVER_V4V6;
	}else if(Entry.IpProtocol==IPVER_IPV6)
			staticInfo->ip_version = IPVER_V6ONLY;

	ret = rtk_rg_staticInfo_set(Entry.rg_wan_idx, staticInfo);
	printf("[%s] update ret=%d\n",__func__,ret);

}


//Save the IPv6 address into ATM_VC_TBL entry, since RG need all the information to set
//together
int saveIPv6AddressByIfname(char *pIfname, struct in6_addr *ip6addr, int kindOfAddr)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		ifGetName(Entry.ifIndex,ifname,sizeof(ifname));

		if(!strcmp(ifname,pIfname)){
			break;
		}
	}

	if(i>= entryNum){
		//printf("not find this interface!\n");
		return -1;
	}

	if(Entry.AddrMode == IPV6_WAN_STATIC) //If wan is static , no need to save, only need if DHCP/PPPoE/SLAAC
		return 0;

	if(kindOfAddr==WANIF_IPV6_ADDR){	
		memcpy(&Entry.Ipv6Addr, ip6addr, sizeof(struct in6_addr));
	}
	else if(kindOfAddr==GATEWAY_IPV6_ADDR){
		memcpy(&Entry.RemoteIpv6Addr, ip6addr, sizeof(struct in6_addr));
	}

	//Save this the mib entry
	mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);	
	return 0;
}

static int checkIPv6AddrIsGlobal(struct in6_addr *ip6addr)
{
	int st = ip6addr->s6_addr32[0];

	if ((st & htonl(0xE0000000)) != htonl(0x00000000) &&
	    (st & htonl(0xE0000000)) != htonl(0xE0000000))
			return 0;
	
	return -1;
}

int handle_ipv6_route_for_RG(char *ifname, struct in6_addr * gateway)
{
	int ret=-1;
	rtk_rg_intfInfo_t intf_info;
	int index=0,entry_index=0;
	MIB_CE_ATM_VC_T Entry;

	if(!ifname || !gateway){
		printf("Error! NULL input!\n");
		return -1;
	}
	
	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry,&entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
//		printf("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	if(Entry.IpProtocol==IPVER_IPV4){
		printf("%s IPv4 only, no need to handle ipv6 routing for this interface %s\n",__func__,ifname);
		return -1;
	}

	if(Entry.rg_wan_idx<=0)
		return -1;

#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
	//Save this information into ATM_VC_TBL since RG need all the information to set	
	saveIPv6AddressByIfname(ifname, gateway, GATEWAY_IPV6_ADDR);
#endif

	//Find this RG interface data structure by index
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	if((ret==0) && intf_info.is_wan){
		switch(intf_info.wan_intf.wan_intf_conf.wan_type)
		{
			case RTK_RG_STATIC:
				printf("[%s] Update IPv6 gateway part to wan type RTK_RG_STATIC\n",__func__);
				RG_update_Static_v6(ifname, gateway, GATEWAY_IPV6_ADDR);
				break;
			case RTK_RG_DHCP:
				printf("[%s] Update IPv6 gateway part to wan type RTK_RG_DHCP\n",__func__);
				RG_update_DHCP_v6(ifname, gateway, GATEWAY_IPV6_ADDR);
				break;
			case RTK_RG_PPPoE:
				printf("[%s] Update IPv6 gateway part to wan type RTK_RG_PPPoE\n",__func__);
				RG_update_PPPoE_v6(ifname, gateway, GATEWAY_IPV6_ADDR);
				break;
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
			case RTK_RG_DSLITE:
				printf("[%s] Update IPv6 gateway part to wan type RTK_RG_DSLITE\n",__func__);
				RG_update_dslite_IPoE_v6(ifname, gateway, GATEWAY_IPV6_ADDR);
				break;
			case RTK_RG_PPPoE_DSLITE:
				printf("[%s] Update IPv6 gateway part to wan type RTK_RG_PPPoE_DSLITE\n",__func__);
				RG_update_dslite_PPPoE_v6(ifname, gateway, GATEWAY_IPV6_ADDR);
				break;
#endif
			default:
				break;
		}
                                                                                                                          	
	}
	
	return 0;
}


int handle_ipv6_addr_for_RG(char *ifname, struct in6_addr *ip6addr)
{
	int ret=-1;
	rtk_rg_intfInfo_t intf_info;
	int index,entry_index=0;
	MIB_CE_ATM_VC_T Entry;

	if(!ifname || !ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}
	//check is IPv6 address is Global IP
	if(checkIPv6AddrIsGlobal(ip6addr)==-1){
//		printf("Not Global or Unicast IPv6 address\n");
		return -1;
	}
	
	if(strcmp("br0",ifname)==0)
		return handle_ipv6_addr_for_RG_lan(ip6addr);

	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry,&entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
		DBG("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	if(Entry.IpProtocol==IPVER_IPV4){
		printf("%s IPv4 only, no need to handle ipv6 routing for this interface %s\n",__func__,ifname);
		return -1;
	}

#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
	//Save this information into ATM_VC_TBL since RG need all the information to set	
	saveIPv6AddressByIfname(ifname, ip6addr, WANIF_IPV6_ADDR);
	//addIPv6RemoteAccRule(ifname, ip6addr);
#endif

	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	//printf("rtk_rg_intfInfo_find index %d =%d\n",Entry.rg_wan_idx,ret);
	if((ret==0) && intf_info.is_wan){
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT) && defined(CONFIG_E8B)
		if(patch_for_avalanche)
		{
//AUG_PRT("%s-%d\n",__func__,__LINE__);		
			if(memcmp((void*)&ip6addr_wan,(void*)ip6addr,sizeof(struct in6_addr)) == 0){
//				AUG_PRT("%s-%d\n",__func__,__LINE__);
				goto SKIP_ACL_WAN;
			}
//AUG_PRT("%s-%d\n",__func__,__LINE__);
			memcpy((void*)&ip6addr_wan,(void*)ip6addr,sizeof(struct in6_addr));
			Flush_RG_ACL_PATCH_FOR_AVALANCHE_WAN();
			/*
			rg clear acl-filter
			rg set acl-filter fwding_type_and_direction 0
			rg set acl-filter action action_type 2
			rg set acl-filter pattern ingress_dest_ipv6_addr 2013:0012:0000:0000:0000:0000:0000:0090 //br0 ip
			rg set acl-filter pattern ingress_dest_ipv6_addr_mask ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
			rg set acl-filter pattern ingress_port_mask 0xf
			rg add acl-filter entry
			*/
			FILE *fp;
		 	int aclIdx;
			rtk_rg_aclFilterAndQos_t aclRule;
			if(!(fp = fopen(RG_PATCH_FOR_AVALANCHE_WAN_ACL, "a")))
			{
				fprintf(stderr, "ERROR! %s\n", strerror(errno));
				return -2;
			}
			memset(&aclRule, 0, sizeof(rtk_rg_aclFilterAndQos_t));
			aclRule.fwding_type_and_direction =  ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;
			aclRule.action_type = ACL_ACTION_TYPE_TRAP;
			aclRule.filter_fields |= INGRESS_PORT_BIT;
			#if defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9607)			
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0xf);
			aclRule.ingress_port_mask.portmask |= RG_get_wan_phyPortMask();
			#elif defined(CONFIG_PON_LED_PROFILE_DEMO_RTL9602B)
			aclRule.ingress_port_mask.portmask = RG_get_lan_phyPortMask(0x3);
			aclRule.ingress_port_mask.portmask |= RG_get_wan_phyPortMask();
			#endif
			aclRule.filter_fields |= INGRESS_IPV6_DIP_BIT;
			memcpy((void *)aclRule.ingress_dest_ipv6_addr, (void *)ip6addr ,sizeof(struct in6_addr));
			memset((void *)aclRule.ingress_dest_ipv6_addr_mask, 0xff ,sizeof(struct in6_addr));
			//rg add acl-filter entry
			if((ret = rtk_rg_aclFilterAndQos_add(&aclRule, &aclIdx)) == 0){
				fprintf(fp,"%d\n",aclIdx);
				AUG_PRT("add %s-%d index=%d success\n",__func__,__LINE__, aclIdx);
			}else{
				fprintf(stderr,"%s-%d add rule failed! (ret = %d)\n",__func__,__LINE__, ret);
				fclose(fp);
				return -1;
			}
			fclose(fp);
		}
		SKIP_ACL_WAN:;
#endif
		switch(intf_info.wan_intf.wan_intf_conf.wan_type)
		{
			case RTK_RG_STATIC:
				printf("[%s] Update IPv6 WAN addr part to wan type RTK_RG_STATIC\n",__func__);
				RG_update_Static_v6(ifname, ip6addr, WANIF_IPV6_ADDR);
				break;
			case RTK_RG_DHCP:
				printf("[%s] Update IPv6 WAN addr part to wan type RTK_RG_DHCP\n",__func__);
				RG_update_DHCP_v6(ifname, ip6addr, WANIF_IPV6_ADDR);
				break;
			case RTK_RG_PPPoE:
				printf("[%s] Update IPv6 WAN addr part to wan type RTK_RG_PPPoE\n",__func__);
				RG_update_PPPoE_v6(ifname, ip6addr, WANIF_IPV6_ADDR);
				break;
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
			case RTK_RG_DSLITE:
				printf("[%s] Update IPv6 WAN addr part to wan type RTK_RG_DSLITE\n",__func__);
				RG_update_dslite_IPoE_v6(ifname, ip6addr, WANIF_IPV6_ADDR);
				break;
			case RTK_RG_PPPoE_DSLITE:
				printf("[%s] Update IPv6 WAN addr part to wan type RTK_RG_PPPoE_DSLITE\n",__func__);
				RG_update_dslite_PPPoE_v6(ifname, ip6addr, WANIF_IPV6_ADDR);
				break;
#endif
			default:
				break;
		}
	}
	
	return 0;
}
#endif

#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
int handle_ipv6_remove_addr_for_RG(char *ifname, struct in6_addr *ip6addr)
{
	int ret=-1;
	rtk_rg_intfInfo_t intf_info;
	int index,entry_index=0;
	MIB_CE_ATM_VC_T Entry;

	if(!ifname || !ip6addr){
		printf("Error! NULL input!\n");
		return -1;
	}
	//check is IPv6 address is Global IP
	if(checkIPv6AddrIsGlobal(ip6addr)==-1){
//		printf("Not Global or Unicast IPv6 address\n");
		return -1;
	}
	
	if(strcmp("br0",ifname)==0)
		return handle_ipv6_addr_for_RG_lan(ip6addr);

	//Got ATM_VC_TBL index by ifname
	ret= getATM_VC_ENTRY_byName(ifname,&Entry,&entry_index);
	//printf("ret =%d, RG WAN Index = %d, IPProtocl=%d, mtu=%d\n",ret, Entry.rg_wan_idx, Entry.IpProtocol, Entry.mtu);

	if(ret!=0){
		DBG("Find ATM_VC_TBL interface %s Fail!\n",ifname);
		return -1;
	}

	if(Entry.IpProtocol==IPVER_IPV4){
		printf("%s IPv4 only, no need to handle ipv6 routing for this interface %s\n",__func__,ifname);
		return -1;
	}

	//Find this RG 
	ret = rtk_rg_intfInfo_find(&intf_info, &Entry.rg_wan_idx);
	if((ret==0) && intf_info.is_wan){

		switch(intf_info.wan_intf.wan_intf_conf.wan_type)
		{
			case RTK_RG_PPPoE:
				RG_release_pppoe(Entry.rg_wan_idx);
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
				update_PON_BW(PON_BW_NORMAL, ifname);
#endif
				break;

			default:
				break;
		}
	}

	return 0;
}
#endif

int read_event(int sockint)
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
	int len, rtl;
	struct ifaddrmsg *ifa;
	struct rtattr *rth;
	
	status = recvmsg (sockint, &msg, 0);
	
	if (status < 0)
	{
		/* Socket non-blocking so bail out once we have read everything */
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return ret;
		
		/* Anything else is an error */
		printf ("read_netlink: Error recvmsg: %d\n", status);
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
		char *ifname = NULL;
		struct in_addr *ipaddr = NULL;
			
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
			ifi = NLMSG_DATA (h);
			len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*ifi));
			for (attribute = IFLA_RTA(ifi); RTA_OK(attribute, len); attribute = RTA_NEXT(attribute, len))
			{
				switch(attribute->rta_type)
				{
					case IFLA_IFNAME:
						ifname = (char *) RTA_DATA(attribute);
						break;
					default:
						break;
				}
			}
			if(ifname)
			{
				DBG("[RTM_NEWLINK] %s %s %s\n", ifname,
					(ifi->ifi_flags & IFF_UP) ? "up" : "down",
					(ifi->ifi_flags & IFF_RUNNING) ? "running" : "not running");
				link_change_dependency(ifname, ifi->ifi_flags);
			}
	  	}
#if defined(CONFIG_IPV6) && (defined(NEW_PORTMAPPING)|| defined(CONFIG_RTK_RG_INIT))
		else if(h->nlmsg_type == RTM_NEWROUTE
				|| h->nlmsg_type == RTM_DELROUTE)
		{
			char ifname[IFNAMSIZ] = {0};
			struct in6_addr *gateway = NULL;
			struct in6_addr *dst = NULL;
			struct in6_addr zero = {0};
			struct rtmsg *msg= NULL;
			struct rtattr *rth = NULL;
			char str_gateway[INET6_ADDRSTRLEN] = {0};

			msg = (struct rtmsg *)NLMSG_DATA(h);

			// Only handle IPv6 unicast routing in main table
			if(msg->rtm_family != AF_INET6
				|| msg->rtm_type != RTN_UNICAST
				|| msg->rtm_table != RT_TABLE_MAIN
				|| msg->rtm_protocol == RTPROT_STATIC)
				continue;

			rth = (struct rtattr *)RTM_RTA(msg);
			rtl = RTM_PAYLOAD(h);

			for (;rtl && RTA_OK (rth, rtl); rth = RTA_NEXT (rth,rtl))
			{
				switch(rth->rta_type)
				{
				case RTA_OIF:
					if_indextoname(*(int *)RTA_DATA(rth), ifname);
					break;

				case RTA_GATEWAY:
					gateway = (struct in6_addr *)RTA_DATA(rth);
					inet_ntop(AF_INET6, gateway, str_gateway, INET6_ADDRSTRLEN);
					break;

				case RTA_DST:
					dst = (struct in6_addr *)RTA_DATA(rth);
					break;
				}
			}

			if(ifname[0] != '\0' && dst == NULL && gateway)
			{
				int reset = (h->nlmsg_type == RTM_NEWROUTE) ? 0 : 1;

				DBG("[RTM_NEWROUTE] Default Gateway %s for %s found\n", str_gateway, ifname);
#if defined(CONFIG_IPV6) && defined(NEW_PORTMAPPING)
				handle_route(ifname, str_gateway, reset);
#endif
#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
				handle_ipv6_route_for_RG(ifname, gateway);
#endif
			}
		}
#endif
		else if (h->nlmsg_type == RTM_NEWADDR)
		{
			struct in_addr *ifa_local = NULL;
			char str_ip[64] = {0}, str_local[64] = {0};
				
			ifa = (struct ifaddrmsg *) NLMSG_DATA (h);
			
			if(ifa->ifa_family == AF_INET)
			{
				
				rth = IFA_RTA (ifa);
				rtl = IFA_PAYLOAD (h);
				for (;rtl && RTA_OK (rth, rtl); rth = RTA_NEXT (rth,rtl))
				{
					switch(rth->rta_type)
					{
						case IFA_ADDRESS:
										 ipaddr = (struct in_addr *)RTA_DATA(rth);
											 //printf("Address got: %s\n", inet_ntoa(*ipaddr));					
											 break;
						case IFA_LABEL:
									   ifname = (char *)RTA_DATA(rth);
										   //printf("Interface got: %s\n", ifname);	
										   break;
						case IFA_LOCAL:
									   ifa_local = (struct in_addr *)RTA_DATA(rth);
										   break;
						default:
								//printf( "Ignored rta_type: %d\n", rth->rta_type);
								break;
					}
				}
			
				inet_ntop(AF_INET, (const void *) ipaddr, str_ip, 64);
				inet_ntop(AF_INET, (const void *) ifa_local, str_local, 64);
				
				DBG("[RTM_NEWADDR] ifname=%s, address=%s ifa_local=%s\n", ifname, str_ip, str_local);
				ip_change_dependency(ifname, ipaddr, ifa_local, h->nlmsg_type);
#ifdef CONFIG_MIDDLEWARE
				if( strcmp(ipaddr,ifa_local) )//only inform one time
					sendInformKeyParaMsg2MW(ifname);
#endif	//end of CONFIG_MIDDLEWARE
			}
#if defined(CONFIG_IPV6) 
			else if(ifa->ifa_family == AF_INET6)
			{
				char str_ip[64] = {0}, str_local[64] = {0};
				struct in6_addr *ip6addr = NULL;
				struct in6_addr *ifa6_local = NULL;
				char ifname[IFNAMSIZ];
				
				//printf("ifa->ifa_index=0x%x, ifname=%s\n",ifa->ifa_index,if_indextoname(ifa->ifa_index,(char *)ifname));
				if_indextoname(ifa->ifa_index,(char *)ifname);
				
				rth = IFA_RTA (ifa);
				rtl = IFA_PAYLOAD (h);
				for (;rtl && RTA_OK (rth, rtl); rth = RTA_NEXT (rth,rtl))
				{
					switch(rth->rta_type)
					{
						case IFA_ADDRESS:
							 ip6addr = (struct in6_addr *)RTA_DATA(rth);
							 inet_ntop(AF_INET6, (const void *) ip6addr, str_ip, 64);
							 // printf("Address got: %s\n", str_ip);					
							 break;
						case IFA_LOCAL:
									   ifa6_local = (struct in6_addr *)RTA_DATA(rth);
										   break;
						default:
								//printf( "Ignored rta_type: %d\n", rth->rta_type);
								break;
					}
				}
			
				DBG("[RTM_NEWADDR] ifname=%s, new ipv6 Addr=%s\n",ifname,str_ip);
#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
				handle_ipv6_addr_for_RG(ifname,ip6addr);
#endif
			}
#endif
		}
		else if (h->nlmsg_type == RTM_DELADDR)
		{
			ifa = (struct ifaddrmsg *) NLMSG_DATA (h);

			if(ifa->ifa_family == AF_INET)
			{
			rth = IFA_RTA (ifa);
			rtl = IFA_PAYLOAD (h);
			for (;rtl && RTA_OK (rth, rtl); rth = RTA_NEXT (rth,rtl))
			{
				switch(rth->rta_type)
				{
				case IFA_ADDRESS:
					ipaddr = (struct in_addr *)RTA_DATA(rth);
					//printf("Address got: %s\n", inet_ntoa(*ipaddr));					
					break;
				case IFA_LABEL:
					ifname = (char *)RTA_DATA(rth);
					//printf("Interface got: %s\n", ifname);	
					break;
				default:
					//printf( "Ignored rta_type: %d\n", rth->rta_type);
					break;
				}
			}
			DBG("[RTM_DELADDR] ifname=%s, address=%s\n", ifname, (ipaddr == NULL) ? "NULL" : (char *)inet_ntoa(*ipaddr));
			ip_removed_dependency(ifname, *ipaddr);
		}
#if defined(CONFIG_IPV6) 
			else if(ifa->ifa_family == AF_INET6)
			{
				char str_ip[64] = {0}, str_local[64] = {0};
				struct in6_addr *ip6addr = NULL;
				struct in6_addr *ifa6_local = NULL;
				char ifname[IFNAMSIZ];
				
				printf("ifa->ifa_index=0x%x, ifname=%s\n",ifa->ifa_index,if_indextoname(ifa->ifa_index,(char *)ifname));
				if_indextoname(ifa->ifa_index,(char *)ifname);
				
				rth = IFA_RTA (ifa);
				rtl = IFA_PAYLOAD (h);
				for (;rtl && RTA_OK (rth, rtl); rth = RTA_NEXT (rth,rtl))
				{
					switch(rth->rta_type)
					{
						case IFA_ADDRESS:
							 ip6addr = (struct in6_addr *)RTA_DATA(rth);
							 inet_ntop(AF_INET6, (const void *) ip6addr, str_ip, 64);
							 // printf("Address got: %s\n", str_ip);					
							 break;
						case IFA_LOCAL:
									   ifa6_local = (struct in6_addr *)RTA_DATA(rth);
										   break;
						default:
								//printf( "Ignored rta_type: %d\n", rth->rta_type);
								break;
					}
				}
			
				DBG("[RTM_DELADDR] ifname=%s, deleted ipv6 Addr=%s\n",ifname,str_ip);
#if defined(CONFIG_IPV6) && defined(CONFIG_RTK_RG_INIT)
				handle_ipv6_remove_addr_for_RG(ifname,ip6addr);
#endif
			}
#endif
		}
	}
	
	return ret;
}

int main(int argc, char *argv[])
{
	fd_set rfds;
	int retval;
	int sock;
	struct sockaddr_nl nl_local;

	log_pid();
	
	/*when those processes created by parent are killed,
	  they will be zombie processes,*/
	signal(SIGCHLD, SIG_IGN);
	signal(SIGUSR1, setup_dnsmasq);
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
	signal(SIGUSR2, setup_dslite);
#endif
	
	if ((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) == -1)
		perror ("socket failure\n");
	memset (&nl_local,0,sizeof(nl_local));
	nl_local.nl_family = AF_NETLINK;
	nl_local.nl_pad = 0;
	nl_local.nl_pid = getpid();
	// Mason Yu. t123
	//nl_local.nl_groups = RTMGRP_LINK;
#if defined(CONFIG_IPV6) && (defined(NEW_PORTMAPPING)|| defined(CONFIG_RTK_RG_INIT))
	nl_local.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR | RTMGRP_IPV6_ROUTE | RTMGRP_LINK;
#else
	nl_local.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_LINK;
#endif
	
	if (bind(sock, (struct sockaddr *)&nl_local, sizeof(nl_local)) == -1)
		perror ("bind failure\n");
	while(1) {
		FD_ZERO(&rfds);
		FD_CLR(sock, &rfds);
		FD_SET(sock, &rfds);
		
		retval = select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
		if (retval == -1)
			printf ("Error select() \n");
		else if (retval) {
			read_event(sock);
		}		
	}
	
	return 0;
}
