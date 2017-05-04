#include "climenu.h"
#include "../defs.h"
#include <sys/wait.h>

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int i, j = 0;

	for (i = 0; i < len; i += 2)
	{
		tmpBuf[0] = string[i];
		tmpBuf[1] = string[i+1];
		tmpBuf[2] = 0;

		if (!isxdigit(tmpBuf[0]) || !isxdigit(tmpBuf[1]))
			return 0;

		key[j++] = (unsigned char)strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

static int check_access(SECURITY_LEVEL level)
{
	if (loginLevel > level) {
		printf(NOT_AUTHORIZED);
		printWaitStr();
		return 0;
	}
	return 1;
}

#ifdef CONFIG_USER_PPPOMODEM
static void show3GStatus(void)
{
	printf("\n3G Configuration\n");
	printf("%-10s%-9s%-16s%-16s%-7s\n","Interface","Protocol","IP Address","Gateway","Status");
	MSG_LINE;

	{//copy from wan3GTable()
		MIB_WAN_3G_T entry,*p;
		p=&entry;
		if( mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p) && p->enable )
		{
			int mppp_idx;
			char mppp_ifname[IFNAMSIZ];
			char mppp_protocol[10];
			char mppp_ipaddr[20];
			char mppp_remoteip[20];
			char *mppp_status;
			char mppp_uptime[20]="";
			char mppp_totaluptime[20]="";
			struct in_addr inAddr;
			int flags;
			char *temp;
			int pppConnectStatus;

			mppp_idx=MODEM_PPPIDX_FROM;
			sprintf( mppp_ifname, "ppp%d", mppp_idx );
			strcpy( mppp_protocol, "PPP" );

			if (getInAddr( mppp_ifname, IP_ADDR, (void *)&inAddr) == 1)
			{
				sprintf( mppp_ipaddr, "%s",   inet_ntoa(inAddr) );
				if (strcmp(mppp_ipaddr, "64.64.64.64") == 0)
					strcpy(mppp_ipaddr, "");
			}else
				strcpy( mppp_ipaddr, "" );

			if (getInAddr( mppp_ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			{
				struct in_addr gw_in;
				char gw_tmp[20];
				gw_in.s_addr=htonl(0x0a404040+mppp_idx);
				sprintf( gw_tmp, "%s",    inet_ntoa(gw_in) );

				sprintf( mppp_remoteip, "%s",   inet_ntoa(inAddr) );
				if( strcmp(mppp_remoteip, gw_tmp)==0 )
					strcpy(mppp_remoteip, "");
				else if (strcmp(mppp_remoteip, "64.64.64.64") == 0)
					strcpy(mppp_remoteip, "");
			}else
				strcpy( mppp_remoteip, "" );


			if (getInFlags( mppp_ifname, &flags) == 1)
			{
				if (flags & IFF_UP) {
					if (getInAddr(mppp_ifname, IP_ADDR, (void *)&inAddr) == 1) {
						temp = inet_ntoa(inAddr);
						if (strcmp(temp, "64.64.64.64"))
							mppp_status = (char *)IF_UP;
						else
							mppp_status = (char *)IF_DOWN;
					}else
						mppp_status = (char *)IF_DOWN;
				}else
					mppp_status = (char *)IF_DOWN;
			}else
				mppp_status = (char *)IF_NA;

			if (strcmp(mppp_status, (char *)IF_UP) == 0)
				pppConnectStatus = 1;
			else{
				pppConnectStatus = 0;
				mppp_ipaddr[0] = '\0';
				mppp_remoteip[0] = '\0';
			}

			printf("%-10s%-9s%-16s%-16s%-7s\n",
				mppp_ifname, mppp_protocol, mppp_ipaddr, mppp_remoteip, mppp_status);
		}
	}
}
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_IPV6
void ShowIPv6()
{
	char buf[256], str_ip6[INET6_ADDRSTRLEN];
	struct in6_addr ip6Prefix;
	unsigned char value[48], len;
	int ifcount, i, j, k;
	struct wstatus_info sEntry[MAX_VC_NUM+MAX_PPP_NUM];
	struct ipv6_ifaddr ip6_addr[6];

	CLEAR;
	printf("\n");
	MENU_LINE;
	printf("                            IPv6 Status                           \n");
	MENU_LINE;
	printf("LAN Configuration\n");
	printf("IPv6 Address: ");
	getSYS2Str(SYS_LAN_IP6_GLOBAL, buf);
	printf("%s\n", buf);
	printf("IPv6 Link-Local Address: ");
	getSYS2Str(SYS_LAN_IP6_LL, buf);
	printf("%s\n", buf);
	putchar('\n');

#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	printf("Prefix Delegation\n");
	printf("Prefix: ");

	len = cmd_get_PD_prefix_len();
	if (0 == len)
		putchar('\n');
	else
	{
		cmd_get_PD_prefix_ip((void *)&ip6Prefix);
		inet_ntop(PF_INET6, &ip6Prefix, value, sizeof(value));
		printf("%s/%d\n", value, len);
	}
#endif

	printf("\nWAN Configuration\n");
	printf("%-11s%-9s%-15s%-10s%-40s%s\n",	"Interface", "VPI/VCI", "Encapsulation", "Protocol", "IP Address", "Status");

	ifcount = getWanStatus(sEntry, MAX_VC_NUM+MAX_PPP_NUM);

	for( i = 0; i < ifcount; i++ )
	{
		if (sEntry[i].cmode == CHANNEL_MODE_BRIDGE || (sEntry[i].ipver != 0 && sEntry[i].ipver != 2))
			continue; // not IPv6 capable

		k = getifip6(sEntry[i].ifname, IPV6_ADDR_UNICAST, ip6_addr, 6);

		buf[0]=0;

		if(k)
		{
			for( j = 0; j < k; j++ )
			{
				inet_ntop(PF_INET6, &ip6_addr[j].addr, str_ip6, INET6_ADDRSTRLEN);
				if (j == 0)
					sprintf(buf, "%s/%d", str_ip6, ip6_addr[j].prefix_len);
				else
					sprintf(buf, "%s, %s/%d", buf, str_ip6, ip6_addr[j].prefix_len);
			}
		}

		printf("%-11s%-9s%-15s%-10s%-40s%s\n", sEntry[i].ifDisplayName, sEntry[i].vpivci, sEntry[i].encaps,
			sEntry[i].protocol, buf, sEntry[i].strStatus);
	}
	printWaitStr();
}
#endif

#ifdef CONFIG_DEV_xDSL
static void showStatus_dsl(XDSL_OP *d)
{
	char strbuf[256];

#ifdef CONFIG_USER_XDSL_SLAVE
	if(d->id)
		printf("\nDSL Slave\n");
	else
#endif /*CONFIG_USER_XDSL_SLAVE*/
		printf("\nDSL\n");

	printf("Operational Status\t: ");
	d->xdsl_get_info(ADSL_GET_MODE, strbuf, 256);
	if (strbuf[0])
		printf("%s, ", strbuf);
	d->xdsl_get_info(ADSL_GET_STATE, strbuf, 256);
	printf("%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_RATE_US, strbuf, 256);
	printf("Upstream Speed\t\t: %s kbps\n", strbuf);


	d->xdsl_get_info(ADSL_GET_RATE_DS, strbuf, 256);
	printf("Downstream Speed\t: %s kbps", strbuf);

	d->xdsl_get_info(ADSL_GET_LATENCY, strbuf, 256);
	printf("\nChannel mode\t\t: %s\n", strbuf);
}
#endif /*CONFIG_DEV_xDSL*/

void showStatus()
{
	char strbuf[256];
	int num, min, max;

	CLEAR;
	printf("\n");
	MENU_LINE;
	printf("                            (1) Device Status                           \n");
	MENU_LINE;
	printf("This page shows the current status and some basic settings of the device.\n");
	MSG_LINE;
	printf("System\n");
	printf("Alias Name\t\t: %s\n", getMibInfo(MIB_SNMP_SYS_NAME));
	getSYS2Str(SYS_UPTIME, strbuf);
	printf("Uptime\t\t\t: %s\n", strbuf);
	getSYS2Str(SYS_DATE, strbuf);
	printf("Date/Time\t\t: %s\n", strbuf);
	getSYS2Str(SYS_FWVERSION, strbuf);
	printf("Firmware Version\t: %s\n", strbuf);
#ifdef CONFIG_DEV_xDSL
	getAdslInfo(ADSL_GET_VERSION, strbuf, 256);
	printf("DSP Version\t\t: %s\n", strbuf);
#ifdef CONFIG_USER_XDSL_SLAVE
	getAdslSlvInfo(ADSL_GET_VERSION, strbuf, 256);
	printf("DSP Slave Version\t\t: %s\n", strbuf);
#endif /*CONFIG_USER_XDSL_SLAVE*/
#endif /*CONFIG_DEV_xDSL*/
#ifdef WLAN_SUPPORT
	getWlVersion(WLANIF[0], strbuf);
	printf("Wireless Version\t: %s\n", strbuf);
#endif
	getNameServers(strbuf);
	printf("Name Servers\t\t: %s\n", strbuf);
	getDGW(strbuf);
	printf("Default Gateway\t\t: %s\n", strbuf);

#ifdef CONFIG_DEV_xDSL
	showStatus_dsl( xdsl_get_op(0) );
#ifdef CONFIG_USER_XDSL_SLAVE
	showStatus_dsl( xdsl_get_op(1) );
#endif /*CONFIG_USER_XDSL_SLAVE*/
#endif /*CONFIG_DEV_xDSL*/
	printf("\nLAN Configuration\n");
	printf("IP Address\t\t: %s\n", getMibInfo(MIB_ADSL_LAN_IP));
	printf("Subnet Mask\t\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET));
#ifdef CONFIG_USER_DHCP_SERVER
	getSYS2Str(SYS_LAN_DHCP, strbuf);
	printf("DHCP Server\t\t: %s\n", strbuf);
#endif
	printf("MAC Address\t\t: %s\n", getMibInfo(MIB_ELAN_MAC_ADDR));
	printf("\nWAN Configuration\n");
	/* list all types of WANs */
	wanConnList(0, 0, 0, NULL);
#ifdef CONFIG_USER_PPPOMODEM
	show3GStatus();
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_IPV6
	min=1;max=2;
	while(1)
	{
		if (0 != getInputUint("Show IPv6 status? (1)Yes (2)No: ", &num, &min, &max))
			break;
	}

	if(num==1)
		ShowIPv6();
#else
	printWaitStr();
#endif
}

/*************************************************************************************************/
void setLANInterface()
{
	struct in_addr inIp, inMask;
	char mode;
	int snum;
	unsigned int snoop, num, min, max;
#ifdef CONFIG_SECONDARY_IP
	unsigned char vChar;
	unsigned int ip2en, dhcp_pool;
	struct in_addr inIp2, inMask2;
#endif
#ifdef WLAN_SUPPORT
	unsigned char block_eth2wir;
#endif
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                                (2) LAN Interface Settings                           \n");
		MENU_LINE;
		printf("This page is used to configure the LAN interface of your Device.    \n");
		printf("Here you may change the setting for IP address, subnet mask, etc..      \n");
		MSG_LINE;
		printf("Interface Name\t: br0\n");
		printf("IP Address\t: %s\n",getMibInfo(MIB_ADSL_LAN_IP));
		printf("Subnet Mask\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET));
#ifdef CONFIG_SECONDARY_IP
		getSYS2Str(SYS_LAN_IP2, strbuf);
		printf("Secondary IP\t: %s\n", strbuf);
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
		if (vChar != 0) {
			printf("IP Address\t: %s\n",getMibInfo(MIB_ADSL_LAN_IP2));
			printf("Subnet Mask\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET2));
#ifndef DHCPS_POOL_COMPLETE_IP
			getSYS2Str(SYS_LAN_DHCP_POOLUSE, strbuf);
			printf("DHCP pool\t: %s\n", strbuf);
#endif
		}
#endif
#if defined(CONFIG_RTL_IGMP_SNOOPING)
		getSYS2Str(SYS_IGMP_SNOOPING, strbuf);
		printf("IGMP Snooping\t: %s\n", strbuf);
#endif
		// Magician
#ifdef WLAN_SUPPORT
		getSYS2Str(SYS_WLAN_BLOCK_ETH2WIR, strbuf);
		printf("Ethernet to Wireless Blocking: %s\n", strbuf);
		putchar('\n');
#endif
		// Magician End

		printf("\n(1) Set                          (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;
		switch( snum)
		{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			printf("Old IP address: %s\n", getMibInfo(MIB_ADSL_LAN_IP));
			if (0 == getInputIpAddr("New IP address:", &inIp))
				continue;

			printf("Old subnet mask address: %s\n",  getMibInfo(MIB_ADSL_LAN_SUBNET));
			if (0 == getInputIpMask("New subnet mask address:", &inMask))
				continue;

#ifdef CONFIG_SECONDARY_IP
			min=1;max=2;
			if (0 == getInputUint("Secondary IP (1)Disable (2)Enable :", &num, &min, &max))
				continue;

			ip2en = (unsigned char)(num - 1);
			if (ip2en == 1) {
				printf("Old IP address: %s\n", getMibInfo(MIB_ADSL_LAN_IP2));
				if (0 == getInputIpAddr("New IP address:", &inIp2))
					continue;

				printf("Old subnet mask address: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET2));
				if (0 == getInputIpMask("New subnet mask address:", &inMask2))
					continue;

#ifndef DHCPS_POOL_COMPLETE_IP
				min=1;max=2;
				if (0 == getInputUint("DHCP pool (1)Primary LAN (2)Secondary LAN :", &num, &min, &max))
					continue;

				dhcp_pool = (unsigned char)(num - 1);
#endif
			}
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
			min=1;max=2;
			if (0 == getInputUint("IGMP Snooping (1)Disable (2)Enable :", &num, &min, &max))
				continue;

			snoop = (unsigned char)(num - 1);
			// bitmap for virtual lan port function
			// Port Mapping: bit-0
			// QoS : bit-1
			// IGMP snooping: bit-2
			mib_get(MIB_MPMODE, (void *)&mode);
			if (snoop) {
				mode |= MP_IGMP_MASK;
				// take effect immediately
				__dev_setupIGMPSnoop(1);
			}
			else {
				mode &= (~MP_IGMP_MASK);
				__dev_setupIGMPSnoop(0);
			}
#endif

			// Magician
#ifdef WLAN_SUPPORT
			min=1;max=2;
			if (0 == getInputUint("Ethernet to Wireless Blocking (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			block_eth2wir = ((unsigned char)num - 1)^1;

			mib_set(MIB_WLAN_BLOCK_ETH2WIR, (void *)&block_eth2wir);
			setup_wlan_block();
			#endif
			// Magician End

			if ( !mib_set( MIB_ADSL_LAN_IP, (void *)&inIp)) {
				printf("Set IP-address error!");
			}

			if ( !mib_set(MIB_ADSL_LAN_SUBNET, (void *)&inMask)) {
				printf("Set subnet-mask error!");
			}

#ifdef CONFIG_SECONDARY_IP
			if (ip2en == 1)
				vChar = 1;
			else
				vChar = 0;
			if (!mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar))
				printf("Set secondary IP error!");

			if (ip2en == 1) {
				if (!mib_set( MIB_ADSL_LAN_IP2, (void *)&inIp2))
					printf("%s\n", "Set IP-address error!");

				if (!mib_set(MIB_ADSL_LAN_SUBNET2, (void *)&inMask2))
					printf("%s\n", "Set subnet-mask error!");

#ifndef DHCPS_POOL_COMPLETE_IP
				if (dhcp_pool == 1)
					vChar = 1;
				else
					vChar = 0;
				if (!mib_set(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar))
					printf("Set DHCP pool error!");
#endif
			}
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
			if (!mib_set(MIB_MPMODE, (void *)&mode))
				printf("Set IGMP Snooping error!");
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
#if defined(APPLY_CHANGE)
			// Take effect in real time
			restart_lanip();
#endif
			break;

		case 2://(2) Quit
			return;
		}//end switch, WAN Interface Menu
	}//end while, WAN Interface Menu
}

int login_flag=0; // Jenny, login_flag=1 from console
int console_flag=0; // cli forked by console
#ifdef WLAN_SUPPORT
void showClient() // Jenny
{
	char *buff;
	WLAN_STA_INFO_Tp pInfo;
	int i;
	char tmpbuf[20];

	if ( (buff = calloc(1, sizeof(WLAN_STA_INFO_T) * (MAX_STA_NUM+1))) == 0 ) {
		printf("Allocate buffer failed!\n");
		return;
	}

	if ( getWlStaInfo((char *)WLANIF[0],  (WLAN_STA_INFO_Tp)buff ) < 0 ) {
		printf("Read wlan sta info failed!\n");
		free(buff);
		return;
	}

	printf("\n");
	printf("Active Wireless Client Table\n");
	MSG_LINE;
	printf("This table shows the MAC address, transmission, reception packet \n");
	printf("counters and encrypted status for each associated wireless client.\n");
	MSG_LINE;
	printf("MAC Address        Tx Pkt    Rx Pkt    TxRate  PowerSaving  Expired Time\n");
	MSG_LINE;

	for (i=1; i<=MAX_STA_NUM; i++) {
		pInfo = (WLAN_STA_INFO_Tp)&buff[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC)) {
			snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
				pInfo->addr[0], pInfo->addr[1], pInfo->addr[2],
				pInfo->addr[3], pInfo->addr[4], pInfo->addr[5]);
			printf("%-19s%-10d%-10d%d%-6s%-13s%d\n",
				tmpbuf, pInfo->tx_packets, pInfo->rx_packets, pInfo->txOperaRates/2, ((pInfo->txOperaRates%2) ? ".5" : "" ),
				( (pInfo->flag & STA_INFO_FLAG_ASLEEP) ? "Yes" : "No"), pInfo->expired_time/100);
		}
	}
	free(buff);
	printWaitStr();
}

void setWlanBasic()
{
	unsigned int choice,num,min,max;
	unsigned char wlan, chwd, ctlbnd, band, auto_chan;
	unsigned char ssid[33];
	int flags;
	char strbuf[256];
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Wireless Basic Settings                       \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the parameters for wireless LAN clients   \n");
		printf("which may connect to your Access Point. Here you may change wireless     \n");
		printf("encryption settings as well as wireless network parameters.              \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_WLAN, strbuf);
		printf("Wireless Interface: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_BAND, strbuf);
		printf("Band: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_MODE, strbuf);
		printf("Mode: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_SSID, strbuf);
		printf("SSID: %s\n", strbuf);
		band = Entry.wlanBand;
		if (wl_isNband(band)) {
			mib_get(MIB_WLAN_CHANNEL_WIDTH, (void *)strbuf);
			printf("Channel Width: %s\n", strbuf[0]? "40MHz": "20MHz");
			mib_get(MIB_WLAN_CONTROL_BAND, (void *)strbuf);
			printf("Control Sideband: %s\n", strbuf[0]? "Lower": "Upper");
		}
		mib_get( MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&auto_chan);
		if(auto_chan)
			printf("Channel Number: 0\n");
		else
			printf("Channel Number: %s\n", getMibInfo(MIB_WLAN_CHAN_NUM));
		getSYS2Str(SYS_TX_POWER, strbuf);	 // Jenny
		printf("Radio Power: %s\n\n", strbuf);
		printf("(1) Set                          (2) Show Active Clients\n");
		printf("(3) Quit\n");
		choice = -1;
		if (console_flag)
			getInputOption(&choice, 0, 3); // Jenny, hidden wireless enable/disable selection
		else
			getInputOption(&choice, 1, 3); // Jenny, unable to enable/disable wireless from remote login
		switch(choice)
		{
		case 0://(0) Jenny, Wireless enable/disable selection
			min=1;max=2;
			if (0 == getInputUint("Wireless Interface (1)Enable (2)Disable :", &num, &min, &max))
				continue;
			wlan = (unsigned char)(num - 1);
			Entry.wlanDisabled = wlan;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			break;
		case 1: //(1) Set
			min=1;max=2;
			if (0 == getInputUint("Wireless Interface (1)Enable (2)Disable :", &num, &min, &max))
				continue;
			wlan = (unsigned char)(num - 1);
			//mib_set(MIB_WLAN_DISABLED, (void *)&wlan);
			//mib_get(MIB_WLAN_DISABLED, (void *)&wlan); // Jenny, get current wireless interface status
			if (wlan == 0) { // enable
				unsigned char mode, chno, txPower;

				min=1;max=6;
				if (0 == getInputUint("Band (1)B (2)G (3)B+G (4)N (5)G+N (6)B+G+N: ", &num, &min, &max))
					continue;
				switch(num) {
				case 4: band = 8; break; // N-only
				case 5: band = 10; break;	// G+N
				case 6: band = 11; break;	// B+G+N
				default:
				band = (unsigned char)num;
				}

				mode = Entry.wlanMode;
#if (defined(WLAN_WDS) && defined(WLAN_CLIENT))
				min=1;max=3;	// Jenny, WLAN mode
				if (0 == getInputUint("Mode (1)AP (2)Client (3)WDS: ", &num, &min, &max))
					continue;
				mode = (unsigned char)(num-1);
				if (mode == CLIENT_MODE) {
					WIFI_SECURITY_T encrypt;
					char vChar;
					vChar = Entry.encrypt;
					encrypt = (WIFI_SECURITY_T)vChar;
					if (encrypt == WIFI_SEC_WPA || encrypt == WIFI_SEC_WPA2) {
						vChar = Entry.wpaAuth;
						if (vChar & 1) { // radius
							printf("You cannot set client mode with WPA-RADIUS!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
					else if (encrypt == WIFI_SEC_WEP) {
						vChar = Entry.enable1X;
						if (vChar & 1) {
							printf("You cannot set client mode with WEP-802.1x!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
				}
#else
#ifdef WLAN_WDS
				min=1;max=2;
				if (0 == getInputUint("Mode (1)AP (2)AP+WDS: ", &num, &min, &max))
					continue;
				if (num == 1)
					mode = (unsigned char)AP_MODE;
				else if (num == 2)
					mode = (unsigned char)AP_WDS_MODE;
				else
					continue;
#endif
#ifdef WLAN_CLIENT
				min=1;max=2;
				if (0 == getInputUint("Mode (1)AP (2)Client: ", &num, &min, &max))
					continue;
				mode = (unsigned char)(num-1);
				if (mode == CLIENT_MODE) {
					WIFI_SECURITY_T encrypt;
					char vChar;
					vChar = Entry.encrypt;
					encrypt = (WIFI_SECURITY_T)vChar;
					if (encrypt == WIFI_SEC_WPA || encrypt == WIFI_SEC_WPA2) {
						vChar = Entry.wpaAuth;
						if (vChar & 1) { // radius
							printf("You cannot set client mode with WPA-RADIUS!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
					else if (encrypt == WIFI_SEC_WEP) {
						vChar = Entry.enable1X;
						if (vChar & 1) {
							printf("You cannot set client mode with WEP-802.1x!\nPlease change the encryption method in security menu first.\n");
							continue;
						}
					}
				}
#endif
#endif	// of WLAN_WDS && WLAN_CLIENT

				if (0 == getInputString("SSID: ", ssid, sizeof(ssid)))
					continue;

				if (wl_isNband(band)) {
					min=1;max=2;
					if (0 == getInputUint("Channel Width (1)20MHz (2)40MHz: ", &num, &min, &max))
						continue;
					chwd = (unsigned char)num-1;

					min=1;max=2;
					if (0 == getInputUint("Control Sideband: (1)Upper (2)Lower: ", &num, &min, &max))
						continue;
					ctlbnd = (unsigned char)num-1;
				}

				min=0;max=11;
				if (0 == getInputUint("Channel Number (0 ~ 11): ", &num, &min, &max))
					continue;
				chno = (unsigned char)num;

				min=1; max=5;
				if (0 == getInputUint("Radio Power (1)100%% (2)70%% (3)50%% (4)35%% (5)15%% :", &num, &min, &max))
					continue;

				txPower = (unsigned char)(num-1); // Jenny

				wl_updateSecurity(band);  // disable TKIP if necessary.

				Entry.wlanBand = band;
				Entry.wlanMode = mode;
				strcpy(Entry.ssid, ssid);

				if (wl_isNband(band)) {
					mib_set(MIB_WLAN_CHANNEL_WIDTH, (void *)&chwd);
					mib_set(MIB_WLAN_CONTROL_BAND, (void *)&ctlbnd);
				}
				if(chno != 0)
				{
					auto_chan = 0;
					mib_set(MIB_WLAN_CHAN_NUM, (void *)&chno);
				}
				else
					auto_chan = 1;

				mib_set(MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&auto_chan);

				// Jenny: txPower
				mib_set(MIB_TX_POWER, (void *)&txPower);
//#ifdef APPLY_CHANGE
				//config_WLAN(ACT_RESTART);
//#endif
			}

			if (getInFlags("wlan0", &flags) == 1) {
				if (wlan)
					flags &= ~IFF_UP;
				else
					flags |= IFF_UP;
				setInFlags("wlan0", flags);
			}
			Entry.wlanDisabled = wlan;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)& Entry, 0);

#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WIFI_SIMPLE_CONFIG
			update_wps_configured(0);
#endif
			config_WLAN(ACT_RESTART);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			break;
		case 2://(2) Show Active Clients
			showClient();
			break;
		case 3://(3) Quit
			return;
		}//end switch, Wireless Basic Settings
	}//end while, Wireless Basic Settings
}

void setWlanAdv()
{
	unsigned int choice,num,min,max;
	unsigned char preamble, hiddenSSID, blockRelay;
#ifdef WPS20
	unsigned char wscUseVersion;
#endif
	unsigned short frag, rts, beacon;
	const char rate_mask[] = {15,1,1,1,1,2,2,2,2,2,2,2,2,4,4,4,4,4,4,4,4,8,8,8,8,8,8,8,8};
	const char *rate_name[] = {"Auto","1M","2M","5.5M","11M","6M","9M","12M","18M","24M","36M","48M","54M", "MCS0", "MCS1",
		"MCS2", "MCS3", "MCS4", "MCS5", "MCS6", "MCS7", "MCS8", "MCS9", "MCS10", "MCS11", "MCS12", "MCS13", "MCS14", "MCS15"};
	unsigned char band, rf_num, autort, mask, tmpstr[10], reqstr[128];;
	unsigned short txrate, txRate;
	int i, idx, defidx;
	unsigned char protection, aggre, shgi, wmm;
	char strbuf[256];
	struct _misc_data_ misc_data;
	unsigned int usInt;
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Wireless Advanced Settings                    \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("These settings are only for more technically advanced users who have a   \n");
		printf("sufficient knowledge about wireless LAN. These settings should not be    \n");
		printf("changed unless you know what effect the changes will have on your        \n");
		printf("Access Point.\n");
		MSG_LINE;
		printf("Fragment Threshold: %s\n", getMibInfo(MIB_WLAN_FRAG_THRESHOLD));
		printf("RTS Threshold: %s\n", getMibInfo(MIB_WLAN_RTS_THRESHOLD));
		printf("Beacon Interval: %s\n", getMibInfo(MIB_WLAN_BEACON_INTERVAL));
		getSYS2Str(SYS_WLAN_TXRATE, strbuf);	// Jenny, Data Rate
		printf("Data Rate: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_PREAMBLE, strbuf);
		printf("Preamble Type: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_BCASTSSID, strbuf);
		printf("Broadcast SSID: %s\n", strbuf);
		getSYS2Str(SYS_WLAN_BLOCKRELAY, strbuf);	// Jenny, Relay Blocking
		printf("Relay Blocking: %s\n", strbuf);
		mib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)&protection);  // Magician
		printf("Protection: %s\n", protection? "Disabled": "Enabled");
		mib_get(MIB_WLAN_AGGREGATION, (void *)&aggre);
		printf("Aggregation: %s\n", aggre? "Enabled": "Disabled");
		mib_get(MIB_WLAN_SHORTGI_ENABLED, (void *)&shgi);
		printf("Short GI: %s\n", shgi? "Enabled": "Disabled");
#ifdef WLAN_QoS
		printf("WMM Support: %s\n", Entry.wmmEnabled? "Enabled": "Disabled");
#endif
		putchar('\n');

		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption(&choice, 1, 2))
			continue;
		switch(choice)
		{
		case 1://(1) Set
			min=256;max=2346;
			if (0 == getInputUint("Fragment Threshold (256 ~ 2346): ", &num, &min, &max))
				continue;
			frag = (unsigned short)num;

			min=0;max=2347;
			if (0 == getInputUint("RTS Threshold (0 ~ 2347): ", &num, &min, &max))
				continue;
			rts = (unsigned short)num;

			min=20;max=1024;
			if (0 == getInputUint("Beacon Interval (20 ~ 1024 ms): ", &num, &min, &max))
				continue;
			beacon = (unsigned short)num;

			// Data Rate
			band = 0, rf_num = 0, autort = 0, mask = 0;
			band = Entry.wlanBand;
			autort = Entry.rateAdaptiveEnabled;

			//cathy, get rf number
			memset(&misc_data, 0, sizeof(struct _misc_data_));
			getMiscData(getWlanIfName(), &misc_data);
			rf_num = misc_data.mimo_tr_used;

			if (band & 1)
				mask |= 1;

			if ((band&2) || (band&4))
				mask |= 2;

			if (band & 8)
			{
				if (rf_num == 2)
					mask |= 12;
				else
					mask |= 4;
			}

			strcpy(reqstr, "Data Rate");

			for (idx=0, i=0; i<=28; i++)
			{
				if (rate_mask[i] & mask)
				{
					idx++;
					if (i == 0)
						txrate = 0;
					else
						txrate = (1 << (i-1));

					sprintf(tmpstr, "(%d)%s ", idx, rate_name[i]);
					strcat(reqstr, tmpstr);
				}
			}

			strcat(reqstr, ": ");
			min=1;max=idx;
			if (0 == getInputUint(reqstr, &num, &min, &max))
				continue;
			txRate = (unsigned short)(num-1);

			// set tx rate
			if ( txRate == 0 ) // auto
				autort = 1;
			else
			{
				autort = 0;
				usInt = 1 << (txRate-1);
			}

			min=1;max=2;
			if (0 == getInputUint("Preamble Type (1)Long Preamble (2)Short Preamble: ", &num, &min, &max))
				continue;

			preamble = (unsigned char)num - 1;

			min=1;max=2;
			if (0 == getInputUint("Broadcast SSID (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			hiddenSSID = (unsigned char)num - 1;

			min=1;max=2;
			if (0 == getInputUint("Relay Blocking (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			blockRelay = ((unsigned char)num - 1)^1;	// Jenny

			min=1;max=2;
			if (0 == getInputUint("Protection (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			protection = (unsigned char)(num - 1);

			min=1;max=2;
			if (0 == getInputUint("Aggregation (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			aggre = (unsigned char)(num - 1)^1;

			min=1;max=2;
			if (0 == getInputUint("Short GI (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			shgi = (unsigned char)(num - 1)^1;
#ifdef WLAN_QoS
			min=1;max=2;
			if (0 == getInputUint("WMM Support (1)Enable (2)Disable: ", &num, &min, &max))
				continue;
			wmm = (unsigned char)(num - 1)^1;
#endif

			mib_set(MIB_WLAN_FRAG_THRESHOLD, (void *)&frag);
			mib_set(MIB_WLAN_RTS_THRESHOLD, (void *)&rts);
			mib_set(MIB_WLAN_BEACON_INTERVAL, (void *)&beacon);
			// Jenny: Data Rate
			Entry.rateAdaptiveEnabled = autort;
			Entry.fixedTxRate = usInt;
			mib_set(MIB_WLAN_PREAMBLE_TYPE, (void *)&preamble);
			Entry.hidessid = hiddenSSID;
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
			#ifdef WPS20
			mib_get(MIB_WSC_VERSION, (void *) &wscUseVersion);
			if (hiddenSSID && wscUseVersion != 0) // force to disable WPS
				Entry.wsc_disabled = hiddenSSID;
			#endif
#endif
			Entry.userisolation = blockRelay;
			mib_set(MIB_WLAN_PROTECTION_DISABLED, (void *)&protection);  // Magician
			mib_set(MIB_WLAN_AGGREGATION, (void *)&aggre);
			mib_set(MIB_WLAN_SHORTGI_ENABLED, (void *)&shgi);
#ifdef WLAN_QoS
			Entry.wmmEnabled = wmm;
#endif
			mib_chain_update(MIB_MBSSIB_TBL, (void *) &Entry, 0);
#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WIFI_SIMPLE_CONFIG
			update_wps_configured(0);
#endif
//#ifdef APPLY_CHANGE
			config_WLAN(ACT_RESTART);
//#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif

			break;
		case 2://(2) Quit
			return;
		}//end switch, Wireless Advanced Settings
	}//end while, Wireless Advanced Settings
}

void setWlanSecurity()
{
	unsigned int choice, num, min, max, len;
	unsigned char keylen, keyfmt, defkey, enct, pskfmt, vChar, use1x, wpaAuth, unicastCipher, wpa2UnicastCipher, auth, enct2save;
	unsigned char key1[32], key2[32], key3[32], key4[32], tmpBuf[256], cipher_su[2];
	int enableRS, getPSK;
	unsigned short rsPort;
	char psk[70];
	char strbuf[256], encmode[12];
	unsigned char band;
	int isNmode;
#ifdef WPS20
	unsigned char disableWps;
	unsigned char wpsUseVersion;
#endif //WPS20
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           WLAN Security Settings                        \n");
		MSG_LINE;
		printf("This page allows you setup the wireless security. Turn on WEP or WPA by  \n");
		printf("using Encryption Keys could prevent any unauthorized access to your      \n");
		printf("wireless network.\n");
		MSG_LINE;

		enct = Entry.encrypt;
		getSYS2Str(SYS_WLAN_ENCRYPT, encmode);
		printf("Encryption: %s\n", encmode);

#ifdef WPS20
		mib_get(MIB_WSC_VERSION, (void *) &wpsUseVersion);
#endif

		use1x = 0;
		if (enct==WIFI_SEC_NONE || enct==WIFI_SEC_WEP)
		{
			use1x = Entry.enable1X;
			printf("Use 802.1x Authentication: %s\n", use1x? STR_ENABLE: STR_DISABLE);
		}

		if (enct==WIFI_SEC_WEP)
		{
			if(use1x)
				strcpy(strbuf, "Auto");
			else
				getSYS2Str(SYS_WLAN_AUTH, strbuf);             // Mason Yu. 201009_new_security

			printf("Authentication Type: %s\n", strbuf);          // Mason Yu. 201009_new_security
			printf("WEP Key:\n");
			getSYS2Str(SYS_WLAN_WEP_KEYLEN, strbuf);
			printf("  Key Length: %s\n", strbuf);

			if(!use1x)
			{
				getSYS2Str(SYS_WLAN_WEP_KEYFMT, strbuf);
				printf("  Key Format: %s\n", strbuf);
			}
		}

		wpaAuth = WPA_AUTH_PSK;
		if (enct>=WIFI_SEC_WPA && enct<=WIFI_SEC_WPA2_MIXED) {
			wpaAuth = Entry.wpaAuth;

			getSYS2Str(SYS_WLAN_WPA_MODE, strbuf);
			printf("Authentication Mode: %s\n", strbuf);

			if (enct == WIFI_SEC_WPA || enct == WIFI_SEC_WPA2_MIXED) {
				getSYS2Str(SYS_WLAN_WPA_CIPHER, strbuf);
				printf("WPA Cipher Suite: %s\n", strbuf);
			}
			if (enct == WIFI_SEC_WPA2 || enct == WIFI_SEC_WPA2_MIXED) {
				getSYS2Str(SYS_WLAN_WPA2_CIPHER, strbuf);
				printf("WPA2 Cipher Suite: %s\n", strbuf);
			}

			if( wpaAuth == WPA_AUTH_PSK )
			{
				getSYS2Str(SYS_WLAN_PSKFMT, strbuf);
				printf("Pre-Shared Key Format: %s\n", strbuf);
				getSYS2Str(SYS_WLAN_PSKVAL, strbuf);
				printf("Pre-Shared Key: %s\n", strbuf);
			}
		}

		if(use1x || wpaAuth == WPA_AUTH_AUTO)
		{
			printf("\nAuthentication RADIUS Server:\n");
			printf("  IP address: %d.%d.%d.%d\n", Entry.rsIpAddr[0], Entry.rsIpAddr[1], Entry.rsIpAddr[2], Entry.rsIpAddr[3]);
			rsPort = Entry.rsPort;
			printf("  Port: %d\n", rsPort);
		}
		printf("\n(1) Set                          (2) Quit\n");

		if (!getInputOption(&choice, 1, 2))
			continue;

		enableRS = 0, getPSK = 0;
		#ifdef WPS20
		if (wpsUseVersion != 0)
			disableWps = 0;
		#endif

		switch(choice)
		{
		case 1://(1) Set
			min = 0;
			max = CLI_ENC_COUNT - 1;

			band = Entry.wlanBand;
			isNmode = wl_isNband(band);
#ifndef NEW_WIFI_SEC
			strcpy(tmpBuf, "Encryption (0)None (1)WEP (2)WPA");
			strcat(tmpBuf, " (3)WPA2");
			strcat(tmpBuf, " (4)WPA2 Mixed");
#ifdef CONFIG_RTL_WAPI_SUPPORT
			strcat(tmpBuf, " (5)WAPI");
#endif
#else
			strcpy(tmpBuf, "Encryption (0)None (1)WEP");
			strcat(tmpBuf, " (2)WPA2");
			strcat(tmpBuf, " (3)WPA2 Mixed");
#ifdef CONFIG_RTL_WAPI_SUPPORT
			strcat(tmpBuf, " (4)WAPI");
#endif
#endif
			strcat(tmpBuf, ": ");

			if (0 == getInputUint(tmpBuf, &num, &min, &max))
				continue;

			enct2save = enct = (unsigned char)num;
#ifndef NEW_WIFI_SEC
			if( enct == CLI_ENC_WPA )
			{
				if (isNmode) { // not support TKIP for N mode
					printf("WPA Cipher Suite: AES\n");
					cipher_su[0] = WPA_CIPHER_AES;
				}
				else {
					strcpy(tmpBuf, "WPA Cipher Suite (1)TKIP (2)AES (3)Both: ");

					min = 1, max = 3;
					if (0 == getInputUint(tmpBuf, &num, &min, &max))
						continue;

					cipher_su[0] = (unsigned char)num;
				}
				#ifdef WPS20
				// WPA only
				if (wpsUseVersion != 0)
					disableWps = 1;
				#endif
			}
#endif
			if( enct == CLI_ENC_WPA2 )
			{
#ifndef NEW_WIFI_SEC
				if (isNmode) { // not support TKIP for N mode
					printf("WPA2 Cipher Suite: AES\n");
					cipher_su[1] = WPA_CIPHER_AES;
				}
				else {
					strcpy(tmpBuf, "WPA2 Cipher Suite (1)TKIP (2)AES (3)Both: ");

					min = 1, max = 3;
					if (0 == getInputUint(tmpBuf, &num, &min, &max))
						continue;

					cipher_su[1] = (unsigned char)num;
				}
#else
				cipher_su[1] = WPA_CIPHER_AES;
#endif
				#ifdef WPS20
				if (cipher_su[1] == WPA_CIPHER_TKIP && wpsUseVersion != 0)
					disableWps = 1; // TKIP only
				#endif
			}
			if( enct == CLI_ENC_WPA2_MIXED )
			{
#ifndef NEW_WIFI_SEC
				strcpy(tmpBuf, "WPA Cipher Suite (1)TKIP (2)AES (3)Both: ");

				min = 1, max = 3;
				if (0 == getInputUint(tmpBuf, &num, &min, &max))
					continue;

				cipher_su[0] = (unsigned char)num;

				strcpy(tmpBuf, "WPA2 Cipher Suite (1)TKIP (2)AES (3)Both: ");

				min = 1, max = 3;
				if (0 == getInputUint(tmpBuf, &num, &min, &max))
					continue;

				cipher_su[1] = (unsigned char)num;
#else
				cipher_su[0] = WPA_CIPHER_TKIP;
				cipher_su[1] = WPA_CIPHER_AES;
#endif
				#ifdef WPS20
				if (cipher_su[0] == WPA_CIPHER_TKIP && cipher_su[1] == WPA_CIPHER_TKIP && wpsUseVersion != 0)
					disableWps = 1; // TKIP only
				#endif
			}
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
			#ifdef WPS20
			if (disableWps && wpsUseVersion != 0) // force to disabled
				Entry.wsc_disabled = disableWps;
			#endif //WPS20
#endif
			if (enct == CLI_ENC_DISABLED) {
#ifdef WLAN_1x
				if (0 == getInputUint("802.1x Authentication (1)Disable (2)Enable: ", &num, &min, &max))
					continue;
				use1x = (unsigned char)(num-1);
				if (use1x == 1)
					enableRS = 1;
#endif
			}
			else if (enct == CLI_ENC_WEP) {
#ifdef WLAN_1x
				if (0 == getInputUint("802.1x Authentication (1)Disable (2)Enable: ", &num, &min, &max))
					continue;
				use1x = (unsigned char)(num-1);
				if (use1x == 1)
					enableRS = 1;
#endif

				if(enableRS == 0)
				{
					// Mason Yu. 201009_new_security
					min=1;max=3;
					if (0 == getInputUint("Authentication Type (1)Open System (2)Shared Key (3)Auto: ", &num, &min, &max))
						continue;
					auth = (unsigned char)num-1;
				}

				// set WEP key
				min=1;max=2;
				if (0 == getInputUint("Key Length (1)64-bit (2)128-bit: ", &num, &min, &max))
					continue;
				keylen = (unsigned char)num;

				if(enableRS == 0)
				{
					min=1;max=2;
					if (keylen == WEP64) {
						if (0 == getInputUint("Key Format (1)ASCII (5 chars) (2)Hex (10 chars): ", &num, &min, &max))
							continue;
						keyfmt = (unsigned char)num - 1;
						if (keyfmt == 0)
							len = WEP64_KEY_LEN;
						else
							len = WEP64_KEY_LEN*2;
					}
					else {
						if (0 == getInputUint("Key Format (1)ASCII (13 chars) (2)Hex (26 chars): ", &num, &min, &max))
							continue;
						keyfmt = (unsigned char)num - 1;
						if (keyfmt == 0)
							len = WEP128_KEY_LEN;
						else
							len = WEP128_KEY_LEN*2;
					}

					key1[0] = key2[0] = key3[0] = key4[0] = 0;

					// Key1
					if (0 == getInputString("Key1: ", tmpBuf, sizeof(tmpBuf)))
						continue;
					if (strlen(tmpBuf) == len) {
						if (keyfmt == 0) // ASCII
							strcpy(key1, tmpBuf);
						else {// Hex
							if (!string_to_hex(tmpBuf, key1, len))
								continue;
						}
					}
					else if (strlen(tmpBuf) != 0)
						continue;
				}
			}
#ifndef NEW_WIFI_SEC
			else if (enct >= CLI_ENC_WPA
#else
			else if (enct >= CLI_ENC_WPA2
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
				&& enct < CLI_ENC_WAPI
#endif
								) {
				min=1; max=2;
				if (0 == getInputUint("Authentication Mode: (1)Enterprise (RADIUS) (2)Personal (Pre-Shared Key): ", &num, &min, &max))
					continue;
				wpaAuth = (unsigned char)num;
				if (wpaAuth == WPA_AUTH_AUTO) {
					enableRS = 1;
					getPSK = 0;
				}
				else if (wpaAuth == WPA_AUTH_PSK) {
					enableRS = 0;
					getPSK = 1;
				}

				unicastCipher = 0;
				wpa2UnicastCipher = 0;

				switch (enct)
				{
#ifndef NEW_WIFI_SEC
					case CLI_ENC_WPA:
						unicastCipher = cipher_su[0];
						enct2save = WIFI_SEC_WPA;
						break;
#endif
					case CLI_ENC_WPA2:
						wpa2UnicastCipher = cipher_su[1];
						enct2save = WIFI_SEC_WPA2;
						break;
					case CLI_ENC_WPA2_MIXED:
						unicastCipher = cipher_su[0];
						wpa2UnicastCipher = cipher_su[1];
						enct2save = WIFI_SEC_WPA2_MIXED;
						break;
					default:
						break;
				}

				if (getPSK == 1) {
					// set pre-shared key
					min=1;max=2;
					if (0 == getInputUint("Pre-Shared Key Format (1)Passphrase (2)Hex (64 chars): ", &num, &min, &max))
						continue;
					pskfmt = (unsigned char)num - 1;

					if (pskfmt == 0) {// Passphrase
						if (0 == getInputString("Pre-Shared Key (8 ~ 63 chars): ", psk, 65))
							continue;
						len = strlen(psk);
						if (len < 8 || len > 63)
							continue;
					}
					else { // Hex
						if (0 == getInputString("Pre-Shared Key (64 chars): ", psk, 66))
							continue;
						len = strlen(psk);
						if (len != 64 || !string_to_hex(psk, tmpBuf, 64))
							continue;
					}

					//mib_set(MIB_WLAN_WPA_PSK_FORMAT, (void *)&pskfmt);
					//mib_set(MIB_WLAN_WPA_PSK, (void *)strbuf);
				}
			}

			if (enableRS == 1) {
				int intVal;
				struct in_addr inIp;
				printf("Authentication RADIUS Server:\n");

				if (0 == getInputIpAddr("IP Address: ", &inIp))
					continue;

				min = 1; max = 65535;
				if (0 == getInputUint("Port: ", &intVal, &min, &max))
					continue;
				rsPort = (unsigned short)intVal;

				if (0 == getInputString("Password: ", strbuf, MAX_PSK_LEN - 1))
					continue;

				if (strbuf[0] && strlen(strbuf) > MAX_PSK_LEN) {
					printf("RS password length too long!");
					printWaitStr();
					continue;
				}
				Entry.rsPort = rsPort;
				*((unsigned long *)Entry.rsIpAddr) = inIp.s_addr;
				strcpy(Entry.rsPassword, strbuf);
			}
			if (enct == WIFI_SEC_WEP) {
				Entry.wep = keylen;
				if(enableRS == 0 )
				{
					Entry.wepKeyType = keyfmt;
					Entry.authType = auth;

					if (keylen == WEP64) {
						if (key1[0] != 0)
							strcpy(Entry.wep64Key1, key1);
					}
					else {
						if (key1[0] != 0)
							strcpy(Entry.wep128Key1, key1);
					}
				}
			}
#ifndef NEW_WIFI_SEC
			else if (enct >= CLI_ENC_WPA
#else
			else if (enct >= CLI_ENC_WPA2
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
				&& enct < CLI_ENC_WAPI
#endif
				) {
				Entry.wpaAuth = wpaAuth;

				switch (enct)
				{
#ifndef NEW_WIFI_SEC
					case CLI_ENC_WPA:
						Entry.unicastCipher = unicastCipher;
						break;
#endif
					case CLI_ENC_WPA2:
						Entry.wpa2UnicastCipher = wpa2UnicastCipher;
						break;
					case CLI_ENC_WPA2_MIXED:
						Entry.unicastCipher = unicastCipher;
						Entry.wpa2UnicastCipher = wpa2UnicastCipher;
						break;
					default:
						break;
				}

				if (getPSK == 1) {
					Entry.wpaPSKFormat = pskfmt;
					strcpy(Entry.wpaPSK, psk);
				}
			}
#ifdef WLAN_1x
			Entry.enable1X = use1x;
#endif
			Entry.encrypt = enct2save;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS def WIFI_SIMPLE_CONFIG
			update_wps_configured(0);
#endif
			config_WLAN(ACT_RESTART);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			break;
		case 2://(2) Quit
			return;
		}//end switch, Wireless Security Setup
	}//end while, Wireless Security Setup
}

#ifdef WLAN_ACL
int addWLanAC(unsigned char *mac) // Jenny
{
	MIB_CE_WLAN_AC_T macEntry;
	MIB_CE_WLAN_AC_T Entry;
	int entryNum, i, intVal;

	if (strlen(mac) != 12 || !string_to_hex(mac, macEntry.macAddr, 12) || !isValidMacAddr(macEntry.macAddr)) {
		printf("Error! Invalid MAC address.");
		return 0;
	}

	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
	if (entryNum >= MAX_WLAN_AC_NUM) {
		printf("Cannot add new entry because table is full!");
		return 0;
	}

	// set to MIB. Check if entry exists
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry)) {
			printf("Get chain record error!\n");
			return 0;
		}

		if (!memcmp(macEntry.macAddr, Entry.macAddr, 6)) {
			printf("Entry already exists!");
			return 0;
		}
	}

	intVal = mib_chain_add(MIB_WLAN_AC_TBL, (unsigned char*)&macEntry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		return 0;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		return 0;
	}
	return 1;
}

void showWLanACL(); // Jenny

int delWLanAC()
{
	int mibTblId;
	unsigned int totalEntry;
	unsigned int index;
	int del,min,max;

	mibTblId = MIB_WLAN_AC_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return 0;
	}
	min=1;max=2;
	if (getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (del==2)
		mib_chain_clear(mibTblId); /* clear chain record */
	else if (del==1) {
		showWLanACL();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if (index>totalEntry || index<=0) {
			printf("Error selection!\n");
			return 0;
		}
		if (mib_chain_delete(mibTblId, index-1) != 1) {
			printf("Delete chain record error!");
			return 0;
		}
	}
	return 1;
}

// Jenny
void showWLanACL()
{
	int entryNum, i;
	MIB_CE_WLAN_AC_T Entry;
	char tmpbuf[20];

	CLEAR;
	printf("\n");
	printf("Current Access Control List\n");
	printf("Idx  MAC Address\n");
	printf("---------------------------------------\n");

	entryNum = mib_chain_total(MIB_WLAN_AC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
		snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		printf("%-5d%-12s\n", i+1, tmpbuf);
	}
}

// Jenny, Wireless Access Control
void setWlanAC()
{
	int snum, min, max, sel, ret;
	unsigned char vChar;
	unsigned char mac[16];
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Wireless Access Control                         \n");
		MSG_LINE;
		printf("If you choose 'Allowed Listed', only those clients whose wireless MAC \n");
		printf("addresses are in the access control list will be able to connect to your \n");
		printf("Access Point. When 'Deny Listed' is selected, these wireless clients on \n");
		printf("the list will not be able to connect the Access Point.\n");
		MSG_LINE;
		getSYS2Str(SYS_WLAN_AC_ENABLED, strbuf);
		printf("Wireless Access Control Mode: %s\n",strbuf);
		MSG_LINE;
		printf("(1) Set                          (2) Add\n");
		printf("(3) Delete                       (4) Show Access Control List\n");
		printf("(5) Quit\n");
		if (!getInputOption( &snum, 1, 5))
			continue;
		switch( snum)
		{
			case 1://(1) Set
				min=1;max=3;
				if(getInputUint("Set Wireless access control mode (1)Disable (2)Allow Listed (3)Deny Listed:", &sel, &min, &max)==0)
					continue;
				vChar = (unsigned char)(sel-1);
				mib_set(MIB_WLAN_AC_ENABLED, (void *)&vChar);
				config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY //Magician: Commit immediately
				Commit();
#endif
				break;
			case 2://(2) Add
				mib_get(MIB_WLAN_AC_ENABLED, (void *)&vChar);
				if (vChar) {	// WLACL enabled
					if (0 == getInputString("MAC Address: (ex. 00E086710502) ", mac, sizeof(mac)))
						continue;
					ret = addWLanAC(mac);

					if (ret)
						config_WLAN(ACT_RESTART);
					else
						printWaitStr();
				}
				else {
					printf("Wireless Access Control is disabled!!\n");
					printWaitStr();
				}
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 3://(3) Delete
				mib_get(MIB_WLAN_AC_ENABLED, (void *)&vChar);
				if (vChar) {	// WLACL enabled
					ret = delWLanAC();

					if (ret)
						config_WLAN(ACT_RESTART);
					else
						printWaitStr();
				}
				else {
					printf("Wireless Access Control is disabled!!\n");
					printWaitStr();
				}
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 4://(4) Show Access Control List
				showWLanACL();
				printWaitStr();
       	break;
			case 5://(5) Quit
				return;
		}//end switch, Port Forwarding
	}
}
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
void ShowKeyInfo()
{
	char auth[28], enc[20], keystr[64];
	unsigned char key, vChar, type;
	int mib_id, vInt;
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

		vChar = Entry.wsc_auth;

		switch(vChar)
		{
			case WSC_AUTH_OPEN: strcpy(auth, "Open"); break;
			case WSC_AUTH_WPAPSK: strcpy(auth, "WPA PSK"); break;
			case WSC_AUTH_SHARED: strcpy(auth, "WEP Shared"); break;
			case WSC_AUTH_WPA: strcpy(auth, "WPA Enterprise"); break;
			case WSC_AUTH_WPA2: strcpy(auth, "WPA2 Enterprise"); break;
			case WSC_AUTH_WPA2PSK: strcpy(auth, "WPA2 PSK"); break;
			case WSC_AUTH_WPA2PSKMIXED: strcpy(auth, "WPA2-Mixed PSK"); break;
			default: strcpy(auth, "Error!"); break;
		}

	vChar = Entry.encrypt;
	if ((WIFI_SECURITY_T)vChar == WIFI_SEC_WPA2_MIXED) {
		vInt = 0;
		vChar = Entry.unicastCipher;
		vInt |= vChar;
		vChar = Entry.wpa2UnicastCipher;
		vInt |= vChar;
		switch (vInt) {
			case WPA_CIPHER_TKIP: strcpy(enc, "TKIP"); break;
			case WPA_CIPHER_AES: strcpy(enc, "AES"); break;
			case WPA_CIPHER_MIXED: strcpy(enc, "TKIP+AES"); break;
			default: strcpy(enc, "Error!"); break;
		}
	}
	else {
		vChar = Entry.wsc_enc;

		switch(vChar)
		{
			case WSC_ENCRYPT_NONE: strcpy(enc, "None"); break;
			case WSC_ENCRYPT_WEP: strcpy(enc, "WEP"); break;
			case WSC_ENCRYPT_TKIP: strcpy(enc, "TKIP"); break;
			case WSC_ENCRYPT_AES: strcpy(enc, "AES"); break;
			case WSC_ENCRYPT_TKIPAES: strcpy(enc, "TKIP+AES"); break;
			default: strcpy(enc, "Error!"); break;
		}
	}

	vChar = Entry.wsc_enc;

	if (vChar == WSC_ENCRYPT_WEP)
	{
		unsigned char tmp[100];
		vChar = Entry.wep;
		type = Entry.wepKeyType;
		key = Entry.wepDefaultKey;

		if (vChar == 1)
		{
			if (key == 0)
				strcpy(tmp, Entry.wep64Key1);
			else if (key == 1)
				strcpy(tmp, Entry.wep64Key2);
			else if (key == 2)
				strcpy(tmp, Entry.wep64Key3);
			else
				strcpy(tmp, Entry.wep64Key4);

			if(type == KEY_ASCII)
			{
				memcpy(keystr, tmp, 5);
				keystr[5] = '\0';
			}
			else
			{
				bin_to_str(tmp, 5, keystr);
				keystr[10] = '\0';
			}
		}
		else
		{
			if (key == 0)
				strcpy(tmp, Entry.wep128Key1);
			else if (key == 1)
				strcpy(tmp, Entry.wep128Key2);
			else if (key == 2)
				strcpy(tmp, Entry.wep128Key3);
			else
				strcpy(tmp, Entry.wep128Key4);

			if(type == KEY_ASCII)
			{
				memcpy(keystr, tmp, 13);
				keystr[13] = '\0';
			}
			else
			{
				bin_to_str(tmp, 13, keystr);
				keystr[26] = '\0';
			}
		}
	}
	else
	{
		if (vChar ==0 || vChar == WSC_ENCRYPT_NONE)
			strcpy(keystr, "N/A");
		else
			strcpy(keystr, Entry.wscPsk);
	}

	printf("%-20s%-16s%s\n", auth, enc, keystr);
}

void setWlWPS()  // Magician
{
	int snum, min, max, sel, errcode;
	unsigned char vChar, mode, disWlan;
	unsigned char mac[16], comment[25];
	char strbuf[256];
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	if(Entry.wlanDisabled)
	{
		printf("WLan is disabled.\n");
		printWaitStr();
		return;
	}

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           WPS Settings                         \n");
		MSG_LINE;
		printf("This page allows you to change the setting for WPS (Wi-Fi Protected Setup).\n");
		printf("Using this feature could let your WLAN client automically syncronize its\n");
		printf("setting and connect to the Access Point in a minute without any hassle.\n");
		MSG_LINE;
		getSYS2Str(SYS_WLAN_WPS_ENABLED, strbuf);
		printf("WPS: %s\n",strbuf);
		getSYS2Str(SYS_WLAN_WPS_STATUS, strbuf);
		printf("WPS Status: %s\n",strbuf);
		getSYS2Str(SYS_WLAN_WPS_LOCKDOWN, strbuf);
		printf("Auto-lock-down state: %s\n",strbuf[0]-'0'? "Locked": "Unlocked");
		mib_get(MIB_WSC_PIN, (void *)strbuf);
		printf("Self-PIN Number: %s\n", strbuf);
		printf("\nCurrent Key Info:\n");
		MSG_LINE;
		printf("%-20s%-16s%s\n", "Authentication", "Encryption", "Key");
		MSG_LINE;
		ShowKeyInfo();
		MSG_LINE;
		printf("(1) Set                          (2) Regenerate PIN\n");
		printf("(3) Start PBC                    (4) Start PIN\n");
		printf("(5) Quit\n");
		if (!getInputOption(&snum, 1, 5))
			continue;

		switch(snum)
		{
			case 1:  //  (1) Set
				min=1;max=2;
				if(getInputUint("Set WPS (1)Disable (2)Enable:",&sel,&min,&max)==0)
					continue;
				vChar = (unsigned char)(sel-1)^1;
				Entry.wsc_disabled = vChar;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
				update_wps_mib();
				config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY //Magician: Commit immediately
				Commit();
#endif
				break;

			case 2:  // (2) Regenerate PIN
				if(Entry.wsc_disabled == 0)	 // WPS enabled
				{
					do
					{
						RegeneratePIN(strbuf);

						printf("New PIN Number: %s\n", strbuf);

						min=1; max=2;

						while(1)
						{
							if (0 == getInputUint("Accept this numner? (1)Yes (2)No: ", &snum, &min, &max))
								continue;
							else
								break;
						}

					} while(snum == 2);

					mib_set(MIB_WSC_PIN, (void *)strbuf);
					config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY  //Magician: Commit immediately
					Commit();
#endif
				}
				else
				{
					printf("WPS is disabled!!\n");
					printWaitStr();
				}
				break;

			case 3: // (3) Start PBC
				if(Entry.wsc_disabled)	 // WPS disabled
				{
					printf("WPS is disabled!!\n");
					printWaitStr();
					continue;
				}

				mode = Entry.wlanMode;
				va_cmd("/bin/wscd", 1, 1, "-sig_pbc");
				printf("Start PBC successfully!\nYou have to run Wi-Fi Protected Setup in %s within 2 minutes.\n", mode == AP_MODE? "client": "AP");
				printWaitStr();
				break;
			case 4:  // (4) Start PIN
				if(Entry.wsc_disabled)	 // WPS disabled
				{
					printf("WPS is disabled!!\n");
					printWaitStr();
					continue;
				}

				if (0 == getInputString("Client PIN Number: ", strbuf, 10))
					continue;

				errcode = CheckPINCode(strbuf);

				switch(errcode)
				{
					case 0:
						sprintf(strbuf, "pin=%s", strbuf);
						va_cmd("/bin/iwpriv", 3, 1, getWlanIfName(), "set_mib", strbuf);
						printf("Applied client's PIN successfully!\nYou have to run Wi-Fi Protected Setup in client within 2 minutes.\n");
						printWaitStr();
						break;
					case -1:
						printf("'Invalid Enrollee PIN length! The device PIN is usually 4 or 8 digits long.\n");
						printWaitStr();
						break;
					case -2:
						printf("Invalid Enrollee PIN! Enrollee PIN must be numeric digits.\n");
						printWaitStr();
						break;
					case -3:
						printf("Checksum failed!\n");
						printWaitStr();
						break;
					default:
						printf("Unexpected error!\n");
						printWaitStr();
						break;
				}
				break;
			case 5://(5) Quit
				return;
		}//end switch, Port Forwarding
	}
}
#endif

#ifdef WLAN_WDS
int addWLanWDS(unsigned char *mac, unsigned char *comment) // Jenny
{
	WDS_T macEntry;
	unsigned char entryNum;
	int intVal;

	if (strlen(mac) != 12 || !string_to_hex(mac, macEntry.macAddr, 12) || !isValidMacAddr(macEntry.macAddr)) {
		printf("Error! Invalid MAC address.");
		return 0;
	}

	if (comment != NULL) {
		if (strlen(comment) > COMMENT_LEN-1) {
			printf("Error! Comment length too long.");
			return 0;
		}
		strcpy(macEntry.comment, comment);
	}
	else
		macEntry.comment[0] = '\0';

	if (!mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
		printf("Get entry number error!");
		return 0;
	}
	if ( (entryNum + 1) > MAX_WDS_NUM) {
		printf("Cannot add new entry because table is full!");
		return 0;
	}

	// set to MIB.
	intVal = mib_chain_add(MIB_WDS_TBL, (void *)&macEntry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		return 0;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		return 0;
	}
	entryNum++;
	if (!mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
		printf("Set entry number error!");
		return 0;
	}
	return 1;
}

// Jenny
void showWLanWDS();
//void delWLanWDS()
int delWLanWDS()
{
	unsigned char entryNum, delNum=0;
	unsigned int index;
	int del,min,max;

	delNum=0;
	mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum); /* get chain record size */
	if (entryNum==0){
		printf("Empty table!\n");
		return 0;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (del == 2) {
		mib_chain_clear(MIB_WDS_TBL); /* clear chain record */
		entryNum = 0;
		if (!mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			printf("Set entry number error!");
			return 0;
		}
	}
	else if (del==1) {
		showWLanWDS();
		min=1; max=entryNum;
		getInputUint("Select the index to delete:",&index, &min,&max);
		if (index>entryNum || index<=0) {
			printf("Error selection!\n");
			return 0;
		}
		if (mib_chain_delete(MIB_WDS_TBL, index-1) != 1) {
			printf("Delete chain record error!");
			return 0;
		}
		delNum++;
		entryNum -= delNum;
		if (!mib_set(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
			printf("Get entry number error!");
			return 0;
		}
	}
	return 1;
}

// Jenny
void showWLanWDS()
{
	unsigned char entryNum;
	int i;
	WDS_T entry;
	WDS_T Entry;
	char tmpbuf[20];

	printf("\n");
	printf("Current WDS AP List\n");
	printf("Idx  MAC Address        Comment\n");
	printf("---------------------------------------------\n");

	mib_get(MIB_WLAN_WDS_NUM, (void *)&entryNum);
	for (i=0; i<entryNum; i++) {
		*((char *)&entry) = (char)i;

		if (!mib_chain_get(MIB_WDS_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
		snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
			Entry.macAddr[0], Entry.macAddr[1], Entry.macAddr[2],
			Entry.macAddr[3], Entry.macAddr[4], Entry.macAddr[5]);
		printf("%-5d%-19s%-20s\n", i+1, tmpbuf, Entry.comment);
	}
}

// Jenny, WDS
void setWlWDS()
{
	int snum, min, max, sel, ret;
	unsigned char vChar, mode, disWlan;
	unsigned char mac[16], comment[25];
	char strbuf[256];
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);

	disWlan = Entry.wlanDisabled;
	mode = Entry.wlanMode;
	if (disWlan || mode == AP_MODE || mode == CLIENT_MODE) {
		printf("WDS mode is disabled.\n");
		printWaitStr();
		return;
	}

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           WDS Settings                         \n");
		MSG_LINE;
		printf("Wireless Distribution System uses wireless media to communicate with \n");
		printf("other APs, like the Ethernet does. To do this, you must set these APs \n");
		printf("in the same channel and set MAC address of other APs which you want \n");
		printf("to communicate with in the table and then enable the WDS.\n");
		MSG_LINE;
		getSYS2Str(SYS_WLAN_WDS_ENABLED, strbuf);
		printf("WDS: %s\n",strbuf);
		MSG_LINE;
		printf("(1) Set                          (2) Add WDS AP\n");
		printf("(3) Delete                       (4) Show WDS AP List\n");
		printf("(5) Quit\n");
		if (!getInputOption(&snum, 1, 5))
			continue;

		switch( snum)
		{
			case 1://(1) Set
				min=1;max=2;
				if(getInputUint("Set WDS (1)Disable (2)Enable:",&sel,&min,&max)==0)
					continue;
				vChar = (unsigned char)(sel-1);
				mib_set( MIB_WLAN_WDS_ENABLED, (void *)&vChar);
				config_WLAN(ACT_RESTART);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2://(2) Add
				mib_get(MIB_WLAN_WDS_ENABLED, (void *)&vChar);
				if (vChar) {	// WDS enabled
					if (0 == getInputString("MAC Address: (ex. 00E086710502) ", mac, sizeof(mac)))
						continue;

				getTypedInputDefault(INPUT_TYPE_STRING,"Comment[None]: ", comment, (void *)(sizeof(comment) - 1), 0);
				ret = addWLanWDS(mac, comment);

				if (ret)
					config_WLAN(ACT_RESTART);
				else
					printWaitStr();
				}
				else {
					printf("WDS is disabled!!\n");
					printWaitStr();
				}
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 3://(3) Delete
				ret = delWLanWDS();

				if (ret)
					config_WLAN(ACT_RESTART);
				else
					printWaitStr();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 4://(4) Show WDS AP List
				showWLanWDS();
				printWaitStr();
				break;
			case 5://(5) Quit
				return;
		}//end switch, Port Forwarding
	}
}
#endif	// of WLAN_WDS

#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
// Jenny
static SS_STATUS_Tp pStatus=NULL;
int ssNum;
void showWlSiteSurvey()
{
	WLAN_MODE_T mode;
	bss_info bss;
	int i=0;
	BssDscr *pBss;
	char tmpbuf[20], ssidbuf[40];
	MIB_CE_MBSSIB_T Entry;

	if (pStatus==NULL) {
		pStatus = calloc(1, sizeof(SS_STATUS_T));
		if ( pStatus == NULL ) {
			printf("Allocate buffer failed!\n");
			return;
		}
	}
	pStatus->number = 0; // request BSS DB
	if ( getWlSiteSurveyResult(WLANIF[0], pStatus) < 0 ) {
		printf("Read site-survey status failed!");
		free(pStatus);
		pStatus = NULL;
		return;
	}

	if ( !mib_chain_get( MIB_MBSSIB_TBL, 0, (void *)&Entry) ) {
		printf("Get MIB_MBSSIB_TBL failed!");
		return;
	}
	mode=Entry.wlanMode;
	if ( getWlBssInfo(WLANIF[0], &bss) < 0) {
		printf("Get bssinfo failed!");
		return;
	}

	printf("\n");
	printf("Idx  SSID             BSSID              Channel  Type    Encrypt  Signal\n");
	MSG_LINE;

	ssNum = 0;
	for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
		pBss = &pStatus->bssdb[i];
		snprintf(tmpbuf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
			pBss->bdBssId[0], pBss->bdBssId[1], pBss->bdBssId[2],
			pBss->bdBssId[3], pBss->bdBssId[4], pBss->bdBssId[5]);
		memcpy(ssidbuf, pBss->bdSsIdBuf, pBss->bdSsId.Length);
		ssidbuf[pBss->bdSsId.Length] = '\0';
		printf("%-5d%-17s%-19s%-9s%-8s%-9s%-8s\n",
			i+1, ssidbuf, tmpbuf, pBss->ChannelNumber, ((pBss->bdCap & cIBSS) ? "Ad Hoc" : "AP"),
			((pBss->bdCap & cPrivacy) ? "Yes" : "No"), pBss->rssi);
		ssNum ++;
	}
}

// Jenny
void connectWlSiteSurvey()
{
	unsigned int idx, min, max;
	unsigned char mode, res;
	NETWORK_TYPE_T net;
	char tmpBuf[100];
	int chan, wait_time;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
	mode = Entry.wlanMode; // get current WLAN mode
	if (mode != CLIENT_MODE) {
		printf("Client mode is disabled.\n");
		return;
	}
	if (ssNum == 0) {
		printf("No Access Point found!!\n");
		return;
	}

	min=1; max=ssNum;
	if (0 == getInputUint("\nConnect: ", &idx, &min, &max))
		return;
	if ( idx > pStatus->number ) { // invalid index
		printf("Connect failed!");
		return;
	}

	// Set SSID, network type to MIB
	memcpy(tmpBuf, pStatus->bssdb[idx].bdSsIdBuf, pStatus->bssdb[idx].bdSsId.Length);
	tmpBuf[pStatus->bssdb[idx].bdSsId.Length] = '\0';
	strcpy(Entry.ssid, tmpBuf);
	if ( mib_chain_update(MIB_MBSSIB_TBL, &Entry, 0) == 0) {
		printf("Set SSID error!");
		return;
	}
	if ( pStatus->bssdb[idx].bdCap & cESS )
		net = INFRASTRUCTURE;
	else
		net = ADHOC;
	if ( mib_set(MIB_WLAN_NETWORK_TYPE, (void *)&net) == 0) {
		printf("Set MIB_WLAN_NETWORK_TYPE failed!");
		return;
	}
	if (net == ADHOC) {
		chan = pStatus->bssdb[idx].ChannelNumber;
		if ( mib_set( MIB_WLAN_CHAN_NUM, (void *)&chan) == 0) {
			printf("Set channel number error!");
			return;
		}
	}

	res = idx-1;
	wait_time = 0;
	while (1) {
		if ( getWlJoinRequest(WLANIF[0], &pStatus->bssdb[idx], &res) < 0 ) {
			printf("Join request failed!");
			return;
		}
		if ( res == 1 ) { // wait
			if (wait_time++ > 5) {
				printf("connect-request timeout!");
				return;
			}
			sleep(1);
			continue;
		}
		break;
	}

	if ( res == 2 ) // invalid index
		printf("Connect failed!\n");
	else {
		wait_time = 0;
		while (1) {
			if ( getWlJoinResult(WLANIF[0], &res) < 0 ) {
				printf("Get Join result failed!");
				return;
			}
			if ( res != 0xff ) { // completed
				if (wait_time++ > 10) {
					printf("connect timeout!");
					return;
				}
				break;
			}
			sleep(1);
		}

		if ( res!=STATE_Bss && res!=STATE_Ibss_Idle && res!=STATE_Ibss_Active )
			printf("Connect failed!\n");
		else
			printf("Connect successfully!\n");
	}
}

// Jenny, Wireless Site Survey
void setWlSiteSurvey()
{
	int snum;
	unsigned char disWlan;
	MIB_CE_MBSSIB_T Entry;

	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
	if (Entry.wlanDisabled)
		return;

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Wireless Site Survey                         \n");
		MSG_LINE;
		printf("This page provides tool to scan the wireless network. If any Access \n");
		printf("Point or IBSS is found, you could choose to connect it manually when \n");
		printf("client mode is enabled.\n");
		MSG_LINE;
		printf("(1) Show                         (2) Connect\n");
		printf("(3) Quit\n");
		if (!getInputOption( &snum, 1, 3))
			continue;

		switch( snum)
		{
			case 1://(1) Show
				showWlSiteSurvey();
				break;
			case 2://(2) Connect
				connectWlSiteSurvey();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 3://(2) Quit
				return;
		}//end switch, Port Forwarding
	}
}
#endif	// of WLAN_CLIENT || WLAN_SITESURVEY

#ifdef WLAN_MBSSID
int setMBSSID()
{
	int snum, min, max, sel, errcode, total_entries, i;
	unsigned char vChar, mode, disWlan, vapSw, relBlk;
	unsigned char mac[16], comment[25];
	char strbuf[256];
	MIB_CE_MBSSIB_T mbssid_tbl;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&mbssid_tbl);

	if(mbssid_tbl.wlanDisabled)
	{
		printf("WLan is disabled!\n");
		printWaitStr();
		return;
	}

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                         Multiple BSSID Settings                      \n");
		MSG_LINE;
		if (mib_get(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar) == 0)
		{
			fprintf(stderr, "Get MIB_WLAN_BLOCK_MBSSID failed!\n");
			return -1;
		}
		printf("Blocking between VAP: %s\n", vChar? "Enabled": "Disabled");
		MSG_LINE;
		printf("%-8s%-12s%-20s%s\n", "", "Status", "SSID", "Relay Blocking");
		MSG_LINE;
		//total_entries = mib_chain_total(MIB_MBSSIB_TBL);
		for( i = 1; i < 5; i++ )
		{
			if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&mbssid_tbl))
			{
	  		printf("Error! Get MIB_MBSSIB_TBL error.\n");
	 			return -1;
			}

			printf("VAP%d    ", i-1);
			printf("%-12s", mbssid_tbl.wlanDisabled? "Disabled": "Enabled");
			printf("%-20s", mbssid_tbl.ssid);
			printf("%s\n", mbssid_tbl.userisolation? "Enabled": "Disabled");
		}
		MSG_LINE;
		printf("%-30s%-30s\n", "(1) Set Blocking between VAP", "(2) Set VAP0");
		printf("%-30s%-30s\n", "(3) Set VAP1", "(4) Set VAP2");
		printf("%-30s%-30s\n", "(5) Set VAP3", "(6) Quit", "");
		if (!getInputOption(&snum, 1, 6))
			continue;

		switch(snum)
		{
			case 1:  //  Set Blocking between VAP
				min = 1, max = 2;
				if(getInputUint("Set Blocking between VAP (1)Disable (2)Enable:", &sel, &min, &max) == 0)
					continue;
				vChar = (unsigned char)(sel-1);
				mib_set(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar);
				config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY //Magician: Commit immediately
				Commit();
#endif
				break;
			case 2: // Set VAP0
			case 3: // Set VAP1
			case 4: // Set VAP2
			case 5: // Set VAP3
				min = 1, max = 2;
				if(getInputUint("Set (1)Disable (2)Enable:", &sel, &min, &max) == 0)
					continue;

				vapSw = (unsigned char)(sel-1)^1;

				if (0 == getInputString("SSID: ", strbuf, 17))
					continue;

				if(getInputUint("Relay blocking (1)Disable (2)Enable:", &sel, &min, &max) == 0)
					continue;

				relBlk = (unsigned char)(sel-1);

				if(!mib_chain_get(MIB_MBSSIB_TBL, snum-1, (void *)&mbssid_tbl))
				{
		  		printf("Error! Get MIB_MBSSIB_TBL error.\n");
		  		printWaitStr();
		 			continue;
				}

				mbssid_tbl.wlanDisabled = vapSw;
				strcpy(mbssid_tbl.ssid, strbuf);
				mbssid_tbl.userisolation = relBlk;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&mbssid_tbl, snum-1);
				config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY  //Magician: Commit immediately
				Commit();
#endif
				break;
			case 6: // Quit
				return;
		}//end switch, Port Forwarding
	}
}
#endif

int showWLANStatus()
{
	int snum, min, max, sel, errcode, total_entries, i;
	unsigned char vChar, mode, disWlan, vapSw, relBlk;
	unsigned char mac[16], comment[25];
	char strbuf[256];
	MIB_CE_MBSSIB_T mbssid_tbl;
	mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&mbssid_tbl);

	if(mbssid_tbl.wlanDisabled)
	{
		printf("WLan is disabled!\n");
		printWaitStr();
		return;
	}

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                         Multiple BSSID Settings                      \n");
		MSG_LINE;
#ifdef WLAN_MBSSID
		if (mib_get(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar) == 0)
		{
			fprintf(stderr, "Get MIB_WLAN_BLOCK_MBSSID failed!\n");
			return -1;
		}
		printf("Blocking between VAP: %s\n", vChar? "Enabled": "Disabled");
#endif
		MSG_LINE;
		printf("%-8s%-12s%-20s%s\n", "", "Status", "SSID", "Relay Blocking");
		MSG_LINE;
		//total_entries = mib_chain_total(MIB_MBSSIB_TBL);
		for( i = 1; i < 5; i++ )
		{
			if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&mbssid_tbl))
			{
	  		printf("Error! Get MIB_MBSSIB_TBL error.\n");
	 			return -1;
			}

			printf("VAP%d    ", i-1);
			printf("%-12s", mbssid_tbl.wlanDisabled? "Disabled": "Enabled");
			printf("%-20s", mbssid_tbl.ssid);
			printf("%s\n", mbssid_tbl.userisolation? "Enabled": "Disabled");
		}
		MSG_LINE;
		printf("%-30s%-30s\n", "(1) Set Blocking between VAP", "(2) Set VAP0");
		printf("%-30s%-30s\n", "(3) Set VAP1", "(4) Set VAP2");
		printf("%-30s%-30s\n", "(5) Set VAP3", "(6) Quit", "");
		if (!getInputOption(&snum, 1, 6))
			continue;

		switch(snum)
		{
#ifdef WLAN_MBSSID
			case 1:  //  Set Blocking between VAP
				min = 1, max = 2;
				if(getInputUint("Set Blocking between VAP (1)Disable (2)Enable:", &sel, &min, &max) == 0)
					continue;
				vChar = (unsigned char)(sel-1);
				mib_set(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar);
				config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY //Magician: Commit immediately
				Commit();
#endif
				break;
#endif
			case 2: // Set VAP0
			case 3: // Set VAP1
			case 4: // Set VAP2
			case 5: // Set VAP3
				min = 1, max = 2;
				if(getInputUint("Set (1)Disable (2)Enable:", &sel, &min, &max) == 0)
					continue;

				vapSw = (unsigned char)(sel-1)^1;

				if (0 == getInputString("SSID: ", strbuf, 17))
					continue;

				if(getInputUint("Relay blocking (1)Disable (2)Enable:", &sel, &min, &max) == 0)
					continue;

				relBlk = (unsigned char)(sel-1);

				if(!mib_chain_get(MIB_MBSSIB_TBL, snum-1, (void *)&mbssid_tbl))
				{
		  		printf("Error! Get MIB_MBSSIB_TBL error.\n");
		  		printWaitStr();
		 			continue;
				}

				mbssid_tbl.wlanDisabled = vapSw;
				strcpy(mbssid_tbl.ssid, strbuf);
				mbssid_tbl.userisolation = relBlk;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&mbssid_tbl, snum-1);
				config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY  //Magician: Commit immediately
				Commit();
#endif
				break;
			case 6: // Quit
				return;
		}//end switch, Port Forwarding
	}

}

void setWireless()
{
	int snum, i;
	unsigned char vChar;
	MIB_CE_MBSSIB_T Entry;

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                          (3) Wireless Menu                                       \n");
		MENU_LINE;

		for( i = 0; i < MENU_WLAN_QUIT; i++ )  // Magician: List menu depends on configuration selections.
		{
			printf("(%d) %-28s", i+1, Menu_WLAN[i]);

			if(i&1) putchar('\n');
		}
		if(i&1) putchar('\n');

		if(getInputOption(&snum, 1, MENU_WLAN_QUIT) == 0)
			continue;

		switch(snum)
		{
			case MENU_WLAN_BASIC:  // Basic Settings
				setWlanBasic();
				break;
			case MENU_WLAN_ADVANCED:  // Advanced Settings
				mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
				if (Entry.wlanDisabled) {	// Wireless disabled
					printf("Wireless interface is disabled!!\n");
					printWaitStr();
				}
				else
					setWlanAdv();
				break;
			case MENU_WLAN_SECURITY:  // Security
				mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
				if (Entry.wlanDisabled) {	// Wireless disabled
					printf("Wireless interface is disabled!!\n");
					printWaitStr();
				}
				else
					setWlanSecurity();
				break;
#ifdef WLAN_ACL
			case MENU_WLAN_ACC_CTRL:  // Access Control
				mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
				if (Entry.wlanDisabled) {	// Wireless disabled
					printf("Wireless interface is disabled!!\n");
					printWaitStr();
				}
				else
					setWlanAC();
				break;
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
			case MENU_WLAN_WPS:
				setWlWPS();
				break;
#endif
#ifdef WLAN_WDS
			case MENU_WLAN_WDS:  // WDS
				mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
				if (Entry.wlanDisabled) {	// Wireless disabled
					printf("Wireless interface is disabled!!\n");
					printWaitStr();
				}
				else
					setWlWDS();
				break;
#endif
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
			case MENU_WLAN_SITE_SURVEY:  // Site Survey
				mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
				if (Entry.wlanDisabled) {	// Wireless disabled
					printf("Wireless interface is disabled!!\n");
					printWaitStr();
				}
				else
					setWlSiteSurvey();
				break;
#endif
#ifdef WLAN_MBSSID
		case MENU_WLAN_MBSSID:
				setMBSSID();
			break;
#endif
		case MENU_WLAN_STATUS:
				showWLANStatus();
			break;
		case MENU_WLAN_QUIT:  // Quit
			return;
		default:
			printf("!! Invalid Selection !!\n");
		}
	}
}
#endif // #ifdef WLAN_SUPPORT

void setWANMode()
{
	unsigned int wanmode;
	int num, min, max;
	unsigned int cur_wan_mode = (unsigned int)WAN_MODE;

	wanmode = cur_wan_mode;

	while(1)
	{
		CLEAR;
		printf("===============================================================================\n");
		printf("                               WAN Mode\n");
		printf("===============================================================================\n");
		printf("       ATM: %s\n", wanmode & MODE_ATM? "Enabled": "Disabled");
		printf("  Ethernet: %s\n", wanmode & MODE_Ethernet? "Enabled": "Disabled");
		printf("       PTM: %s\n", wanmode & MODE_PTM? "Enabled": "Disabled");
		printf("-------------------------------------------------------------------------------\n");

		min=1; max=4;
		if (0 == getInputUint("Swtich on/off WAN mode (1)ATM (2)Ethernet (3)PTM (4)Quit: ", &num, &min, &max))
			continue;

		switch(num)
		{
			case 1:
				wanmode ^= MODE_ATM;
				break;
			case 2:
				wanmode ^= MODE_Ethernet;
				break;
			case 3:
				wanmode ^= MODE_PTM;
				break;
			case 4:
				if(wanmode == 0)
				{
					max = 2;
					while(0 == getInputUint("\nYou can't disable all WAN modes! (1)Continue (2)Quit without save: ", &num, &min, &max));

					if(num == 1)
						continue;
					else
						return;
				}
				else if(wanmode != cur_wan_mode)
				{
					max = 2;
					while(0 == getInputUint("\nChanging WAN mode needs rebooting to take effect... (1)Cancel (2)Reboot: ", &num, &min, &max));

					if(num == 1)
						return;
					else
						goto WAN_Mode_Apply;
				}
				return;
			default:
				continue;
		}
	}

WAN_Mode_Apply:
	if(!mib_set(MIB_WAN_MODE, (void *)&wanmode))
	{
		fprintf(stderr, "Set MIB_WAN_MODE failed!\n");
		printWaitStr();
		return;
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	/* Save and reboot the system */
	printf("%s\n\n", rebootWord0);
	printf("%s\n", rebootWord1);
	printf("%s\n", rebootWord2);
	cmd_reboot();
}

#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
void showEthWanConfiguration(MEDIA_TYPE_T m_type)
{
	MIB_CE_ATM_VC_T Entry;
	int i, mibtotal, index = 0;
	const char *mode;
	char *strNapt, ifname[IFNAMSIZ], ipAddr[20], ipMask[20], remoteIp[20], UserName[MAX_PPP_NAME_LEN+1], PassWord[MAX_NAME_LEN];
	char *temp;

	printf("%-6s%-7s%-8s%-5s%-5s","Index","IfName","Mode","NAPT","IGMP");
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	printf("%-4s%-6s%-12s","Vid","Vprio","PortMapping");
#endif
#ifdef CONFIG_USER_IP_QOS
	printf("%-4s","Qos");
#endif
#ifdef DEFAULT_GATEWAY_V1
	printf("%-7s","DRoute");
#endif
	printf("%-16s%-16s%-16s%-20s%-20s\n","IPAddr","RemoteIP","Mask","UserName","PassWord");

	MSG_LINE;

	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++){
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if(MEDIA_INDEX(Entry.ifIndex) == m_type){
			index ++;

			/* Initialize */
			ipAddr[0]	  = '\0';
			ipMask[0]	  = '\0';
			remoteIp[0]   = '\0';
			UserName[0] = '\0';
			PassWord[0] = '\0';
			strNapt 	  = "";
			mode = 0;
			if (Entry.cmode <= CHANNEL_MODE_PPPOE)
			{
				mode = g_pszEthMode[Entry.cmode];
			}

			if (Entry.napt == 0)
				strNapt = (char*)IF_OFF;
			else
				strNapt = (char*)IF_ON;

			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
			if (Entry.cmode == CHANNEL_MODE_PPPOE)
			{
				strncpy(UserName, Entry.pppUsername, MAX_PPP_NAME_LEN);
				UserName[MAX_NAME_LEN] = '\0';
				strncpy(PassWord, Entry.pppPassword, MAX_NAME_LEN);
				PassWord[MAX_NAME_LEN] = '\0';
#ifdef CONFIG_SPPPD_STATICIP
				if (Entry.pppIp)
				{
					temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
					strcpy(ipAddr, temp);
					temp = inet_ntoa(*((struct in_addr *)Entry.netMask));
					strcpy(ipMask, temp);
				}
#endif
			}
			else {
				if ((Entry.ipDhcp == (char)DHCP_DISABLED)
					&& (!Entry.ipunnumbered)
					&& (Entry.cmode != CHANNEL_MODE_BRIDGE))
				{
					// static IP address
					temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
					strcpy(ipAddr, temp);

					temp = inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr));
					strcpy(remoteIp, temp);

					temp = inet_ntoa(*((struct in_addr *)Entry.netMask));
					strcpy(ipMask, temp);
				}
			}
			printf("%-6d%-7s%-8s%-5s", index, ifname, mode, strNapt);
			if (Entry.enableIGMP == 0)
				printf("%-5s",IF_OFF);
			else
				printf("%-5s",IF_ON);
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
			printf("%-4d%-6d0x%-10X", Entry.vid, Entry.vprio,Entry.itfGroup);
#endif
#ifdef CONFIG_USER_IP_QOS
			if (Entry.enableIpQos== 0)
				printf("%-4s",IF_OFF);
			else
				printf("%-4s",IF_ON);
#endif
#ifdef DEFAULT_GATEWAY_V1
			if (Entry.dgw == 0)
				printf("%-7s",IF_OFF);
			else
				printf("%-7s",IF_ON);
#endif
			printf("%-16s%-16s%-16s%-20s%-20s\n", ipAddr, remoteIp, ipMask, UserName, PassWord);

		}
	}

	printWaitStr();
}

static int getEthWanInput(MIB_CE_ATM_VC_Tp entry, MIB_CE_ATM_VC_Tp old)
{
	unsigned int min,max;
	unsigned int num;
	unsigned int i, totalEntry;
	MIB_CE_ATM_VC_T Entry;
	unsigned int ifMap; // high half for PPP bitmap, low half for vc bitmap

	min = 1;
#ifdef CONFIG_PPP
	max = 3;
#else
	max = 2;
#endif
	if (0 == getInputUint("Channel Mode (1)Bridged (2)IPoE "
#ifdef CONFIG_PPP
				"(3)PPPoE "
#endif
				":", &num, &min, &max))
		return 0;
	entry->cmode = num-1;
	entry->enable = 1;

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	min=1; max=2;
	if (0 == getInputUint("Enable VLAN (1)Disable (2)Enable :", &num, &min, &max))
		return 0;
	entry->vlan = num-1;
	if(entry->vlan == 1){
		min = 10; max = 4095;
		if(0 == getInputUint("VLAN ID(0-9 are reserved) :", &num, &min, &max))
			return 0;
		entry->vid = num;
		min = 0; max = 7;
		if(0 == getInputUint("Priority (0-7) :", &num, &min, &max))
			return 0;
		entry->vprio= num;
	}
#endif

	// 1483 bridged
	if (entry->cmode == CHANNEL_MODE_BRIDGE)
	{
		entry->napt = 0;
		entry->brmode = BRIDGE_ETHERNET;
		entry->ipAddr[0]=0;
		entry->ipAddr[1]=0;
		entry->ipAddr[2]=0;
		entry->ipAddr[3]=0;
		entry->remoteIpAddr[0]=0;
		entry->remoteIpAddr[1]=0;
		entry->remoteIpAddr[2]=0;
		entry->remoteIpAddr[3]=0;
		entry->netMask[0]=0;
		entry->netMask[1]=0;
		entry->netMask[2]=0;
		entry->netMask[3]=0;
#ifdef CONFIG_USER_IP_QOS
	min=1; max=2;
	if (0 == getInputUint("IP QoS (1)Disable (2)Enable :", &num, &min, &max))
		return 0;
	entry->enableIpQos = num-1;
#endif
#ifdef DEFAULT_GATEWAY_V1
		entry->dgw=0;
#endif
		entry->cdvt=0;

	}
	else {
		entry->brmode = BRIDGE_DISABLE;
		entry->mtu = 1500;
		min=1; max=2;
		if (0 == getInputUint("NAPT (1)Disable (2)Enable :", &num, &min, &max))
			return 0;
		entry->napt = num-1;
		if (0 == getInputUint("IGMP (1)Disable (2)Enable :", &num, &min, &max))
			return 0;
		entry->enableIGMP = num-1;
#ifdef CONFIG_USER_IP_QOS
		if (0 == getInputUint("IP QoS (1)Disable (2)Enable :", &num, &min, &max))
			return 0;
		entry->enableIpQos = num-1;
#endif
#ifdef CONFIG_IPV6
		entry->IpProtocol = IPVER_IPV4;
#endif
#ifdef CONFIG_PPP
		// PPP connection
		if (entry->cmode == CHANNEL_MODE_PPPOE)
		{
			printf("PPPoE Settings\n");
			if (0 == getInputString("Username: ", entry->pppUsername, MAX_PPP_NAME_LEN))
				return 0;

			if (0 == getInputString("Password: ", entry->pppPassword, MAX_NAME_LEN-1))
				return 0;
			min=1;max=3;
			if (0 == getInputUint("Connect Type (1)Continuous (2)Connect on Demand (3)Manual :", &num, &min, &max))
				return 0;

			if (1 == num)
				entry->pppCtype = CONTINUOUS;
			else if (2 == num)
				entry->pppCtype = CONNECT_ON_DEMAND;
			else
				entry->pppCtype = MANUAL;

			if (entry->pppCtype == CONNECT_ON_DEMAND) {	// Jenny
				min=1;max=65535;
				if (0 == getInputUint("Idle Time (min) :", &num, &min, &max))
					return 0;
				entry->pppIdleTime = num;
			}
			entry->mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry->PPPoEProxyMaxUser=4;
#endif
		}
		else
#endif
		{
			printf("IPoE Settings :\n");
			entry->mtu = 1500;
			min=1;max=2;
			if (0 == getInputUint("Type (1)Fixed IP (2)DHCP :", &num, &min, &max))
				return 0;
			if (num == 1) {
				char *temp, strIp[20], strGW[20], strMask[20];
				entry->ipDhcp = DHCP_DISABLED;
				if (0 == getInputIpAddr("Local IP Address :", (struct in_addr *)&entry->ipAddr)) return 0;
				if (0 == getInputIpAddr("Remote IP Address :", (struct in_addr *)&entry->remoteIpAddr)) return 0;
				if (0 == getInputIpMask("Subnet Mask :", (struct in_addr *)&entry->netMask)) return 0;

				temp = inet_ntoa(*((struct in_addr *)entry->ipAddr));
				strcpy(strIp, temp);
				temp = inet_ntoa(*((struct in_addr *)entry->remoteIpAddr));
				strcpy(strGW, temp);
				temp = inet_ntoa(*((struct in_addr *)entry->netMask));
				strcpy(strMask, temp);
				if (!isValidHostID(strIp, strMask)) {
					printf("Invalid IP/Subnet Mask combination!!\n");
					return 0;
				}
				if (!isSameSubnet(strIp, strGW, strMask)) {
					printf("Invalid IP address! It should be located in the same subnet.\n");
					return 0;
				}
			} else {
				entry->ipDhcp = DHCP_CLIENT;
			}
		}
		#ifdef DEFAULT_GATEWAY_V1
		min=1; max=2;
		if (0 == getInputUint("Default Route Setting (1)Disable (2)Enable :", &num, &min, &max))
			return 0;
		entry->dgw = num-1;
		#endif
	}

	ifMap = 0;
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	for (i = 0; i < totalEntry; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &Entry))
			break;
		ifMap |= 1 << ETH_INDEX(Entry.ifIndex);	// VC map
		if ((Entry.cmode == CHANNEL_MODE_PPPOE) || (Entry.cmode == CHANNEL_MODE_PPPOA))
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
	}
	entry->ifIndex = if_find_index(entry->cmode, ifMap);
	entry->ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(entry->ifIndex), ETH_INDEX(entry->ifIndex));
#ifdef _CWMP_MIB_
	/*start use_fun_call_for_wan_instnum*/
	resetWanInstNum(entry);
	updateWanInstNum(entry);
	dumpWanInstNum(entry, "new");
	/*end use_fun_call_for_wan_instnum*/
#endif /*_CWMP_MIB_*/

	return 1;
}

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
int addEthWanConfiguration(MEDIA_TYPE_T m_type)
{
	MIB_CE_ATM_VC_T entry, tmpEntry;
	int idx, i;
	int vc_index=-1, ppp_index=-1;
	unsigned int ifMap; // high half for PPP bitmap, low half for vc bitmap

	memset(&entry, 0x00, sizeof(entry));
	if(getEthWanInput(&entry, 0) == 0)
		return 0;

	//set wan_ifindex
	ifMap = getWanIfMapbyMedia(m_type);
	for(i=0;i<8;i++){
		if(((ifMap>>i) & 1) != 1){
			vc_index = i;
			break;
		}
	}
	if(vc_index == -1)
		return 0;
	if(entry.cmode==CHANNEL_MODE_PPPOE){
		for(i=0;i<8;i++){
			if(((ifMap>>(i+16)) & 1) != 1)
			{
				ppp_index = i;
				break;
			}
		}
		if(ppp_index == -1)
			return 0;
	}
	entry.ifIndex = TO_IFINDEX(m_type, (entry.cmode==CHANNEL_MODE_PPPOE?ppp_index:DUMMY_PPP_INDEX), vc_index);

	idx = mib_chain_add(MIB_ATM_VC_TBL, (void *)&entry);
	if(idx > 0)
	{
		restartWAN(CONFIGONE, &entry);
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
	}
	else {
		restartWAN(CONFIGONE, NULL);
		printf("Add chain record error!\n");
	}

	return 1;
}

int delEthWanConfiguration(MEDIA_TYPE_T m_type)
{
	int index, min, max;
	int idx;
	char ifname[IFNAMSIZ];
	MIB_CE_ATM_VC_T Entry, PvcEntry;
	int idxArray[MAX_VC_NUM+1];

	max = mib_chain_total(MIB_ATM_VC_TBL);
	if(max == 0){
		printf("none wan exist!");
		printWaitStr();
		return 0;
	}

	for(min=0; min<=max; min++)
		idxArray[min] = -1;

	min = 0;
	printf("%-8s%-12s%-12s\n","Index","IfName","Mode");

	for(idx=0; idx<max; idx++){
		if(!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry))
			continue;
		if(MEDIA_INDEX(Entry.ifIndex) == m_type){
			idxArray[++min] = idx;
			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
			printf("%-8d%-12s%-12s\n",min, ifname, g_pszEthMode[Entry.cmode]);
		}
	}
	min = 1;
	if (0 == getInputUint("Index to delete: ", &index, &min, &max)){
		printf("Get index error\n");
		return 0;
	}

	if(idxArray[index] == -1){
		printf("Can't find ethwan index %d\n",index);
		return 0;
	}

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	if ((Entry.cmode == CHANNEL_MODE_IPOE) && (Entry.ipDhcp == DHCP_CLIENT))
		delDhcpcOption(Entry.ifIndex);
#endif

#ifdef NEW_IP_QOS_SUPPORT
	delIpQosTcRule(&Entry);
#endif

	if(mib_chain_get(MIB_ATM_VC_TBL,idxArray[index],&PvcEntry)&&(PvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
	{
#ifdef DNS_BIND_PVC_SUPPORT
		int tempi=0;
		unsigned int pvcifIdx=0;
		for(tempi=0;tempi<3;tempi++)
		{
			mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
			if(pvcifIdx==PvcEntry.ifIndex) //I get it
			{
				pvcifIdx=DUMMY_IFINDEX;
				mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
			}
		}
#endif
	}

	deleteConnection(CONFIGONE, &PvcEntry);
	if(mib_chain_delete(MIB_ATM_VC_TBL, idxArray[index]) != 1){
		printf("Delete chain record error!\n");
		restartWAN(CONFIGONE, NULL);
		return 0;
	}

	restartWAN(CONFIGONE, NULL);
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	return 1;
}
#endif

#if !defined(CONFIG_RTL_MULTI_ETH_WAN) && !defined(CONFIG_PTMWAN)
int setEthWanConfiguration()
{
	MIB_CE_ATM_VC_T entry, tmpEntry;
	int idx, i;
	unsigned int totalEntry;
	unsigned int ifMap; // high half for PPP bitmap, low half for vc bitmap
	int addNewmib=0;

	memset(&entry, 0x00, sizeof(entry));
	if (getEthWanInput(&entry, 0) == 0)
		return 0;

	ifMap = 0;
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tmpEntry))
		{
			printf("Get chain record error!\n");
			return 0;
		}
		if (tmpEntry.cmode == CHANNEL_MODE_PPPOE)
			ifMap |= (1 << 16) << PPP_INDEX(tmpEntry.ifIndex);	// PPP map
	}
	idx = getWanEntrybyMedia(&tmpEntry, MEDIA_ETH);
	if (idx < 0) {		// add one

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		init_ethwan_config(&tmpEntry);
		idx = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&tmpEntry);
		idx--;
		addNewmib = 1;
	}
	entry.pppAuth = tmpEntry.pppAuth;
	entry.rip = tmpEntry.rip;
	entry.mtu = tmpEntry.mtu;
#ifdef PPPOE_PASSTHROUGH
	if (entry.cmode != CHANNEL_MODE_PPPOE)
		if (entry.cmode == tmpEntry.cmode)
			entry.brmode = tmpEntry.brmode;
#endif

#ifdef CONFIG_USER_WT_146
	wt146_copy_config( &entry, &tmpEntry );
#endif //CONFIG_USER_WT_146
	strcpy(entry.pppACName, tmpEntry.pppACName);

	if (entry.cmode != tmpEntry.cmode)
	{
		entry.ifIndex = if_find_index(entry.cmode, ifMap);
		entry.ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(entry.ifIndex), 0);
		if (entry.ifIndex == NA_PPP)
		{
			printf("%s\n", "Maximum number of PPPoE connections exceeds in this vc!");
			if (addNewmib== 1) {
				if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1){
					printf("Delete chain record error!\n");
				}
			}
			return 0;
		}
		// mode changed, restore to default
		if (entry.cmode == CHANNEL_MODE_PPPOE) {
			entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=4;
#endif
			entry.pppAuth = 0;
		}
		else {
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=0;
#endif
			entry.mtu = 1500;
		}
	}
	else
		entry.ifIndex = tmpEntry.ifIndex;
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	if ((tmpEntry.cmode == CHANNEL_MODE_IPOE) && (tmpEntry.ipDhcp == DHCP_CLIENT))
		delDhcpcOption(tmpEntry.ifIndex);
#endif

	if( entry.ifIndex!=tmpEntry.ifIndex )
	{
#ifdef PORT_FORWARD_GENERAL
		updatePortForwarding( tmpEntry.ifIndex, entry.ifIndex );
#endif
#ifdef ROUTING
		updateRoutingTable( tmpEntry.ifIndex, entry.ifIndex );
#endif
		//ql 20081125 add
#ifdef NEW_IP_QOS_SUPPORT
		delIpQosTcRule(&tmpEntry);
#endif
	}
#ifdef NEW_IP_QOS_SUPPORT
	//ql 20081125 Add else
	else if (entry.cmode != tmpEntry.cmode)
	{
		delIpQosTcRule(&tmpEntry);
	}
#endif

	if (addNewmib !=1)
		deleteConnection(CONFIGONE, &tmpEntry);
	mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, idx);
	restartWAN(CONFIGONE, &entry);
	return 1;
}
#endif

void setEthWanConfig(MEDIA_TYPE_T m_type)
{
	int snum;

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		if(m_type==MEDIA_PTM)
			printf("                           PTM Configuration                         \n");
		else
		printf("                           Ethernet Configuration                         \n");
		MENU_LINE;

		cli_show_menu(strSetEthWanConf, SET_ETHWAN_CONF_END);
		if (!getInputOption( &snum, 1, SET_ETHWAN_CONF_END))
			continue;

		switch(snum-1)
		{
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
		case SET_ETHWAN_CONF_ADD: // Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			addEthWanConfiguration(m_type);
			break;
#else
		case SET_ETHWAN_CONF_SET: // Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			setEthWanConfiguration();
			break;
#endif
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
		case SET_ETHWAN_CONF_DEL: // Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			delEthWanConfiguration(m_type);
			break;
#ifdef NEW_PORTMAPPING
		case SET_ETHWAN_CONF_PORT_MAPPING: // Port Mapping
			setNewPortMapping(m_type);
			break;
#endif
#endif
		case SET_ETHWAN_CONF_SHOW: // Show
			showEthWanConfiguration(m_type);
			break;
		case SET_ETHWAN_CONF_QUIT: // Quit
			return;
		}//end switch, EthWan Configuration
	}//end while, EthWan Configuration
}
#endif // of CONFIG_ETHWAN

#ifdef CONFIG_DEV_xDSL
static int getVcInput(MIB_CE_ATM_VC_Tp entry, MIB_CE_ATM_VC_Tp old)
{
	unsigned int min,max;
	unsigned int num;

	entry->qos = 0;
	entry->pcr = 7600;

	min=0;max=255;
	if (0 == getInputUint("VPI:", &num, &min, &max))
		return 0;
	entry->vpi = (unsigned char)num;

	min=0;max=65535;
	if (0 == getInputUint("VCI:", &num, &min, &max))
		return 0;
	entry->vci = (unsigned short)num;

#ifndef CONFIG_PPP
retry:
#endif
#ifdef DUAL_STACK_LITE
	max=7;
	if (0 == getInputUint("Channel Mode (1)1483 Bridged (2)1483 MER "
#ifdef CONFIG_PPP
				"(3)PPPoE (4)PPPoA "
#endif
				"(5)1483 Routed (6)1577 Routed (7)DS-Lite: ", &num, &min, &max))
		return 0;
#else
	max=6;
	if (0 == getInputUint("Channel Mode (1)1483 Bridged (2)1483 MER "
#ifdef CONFIG_PPP
				"(3)PPPoE (4)PPPoA "
#endif
				"(5)1483 Routed (6)1577 Routed: ", &num, &min, &max))
		return 0;
#endif
#ifndef CONFIG_PPP
	if (num == 3 || num == 4)
		goto retry;
#endif
	entry->cmode = (unsigned char)num-1;

	if(entry->cmode == CHANNEL_MODE_RT1577)
		entry->encap = 1;
	else
	{
		min=1;max=2;
		if (0 == getInputUint("Encapsulation (1)LLC (2)VC-Mux: ", &num, &min, &max))
			return 0;
		if (num == 1)
			entry->encap = 1;
		else
			entry->encap = 0;
	}

	if (entry->cmode != CHANNEL_MODE_BRIDGE) {
#ifdef DUAL_STACK_LITE
		if(entry->cmode != CHANNEL_MODE_DSLITE)
#endif
		{
			max=2;
			if (0 == getInputUint("NAPT (1)Enable (2)Disable: ", &num, &min, &max))
				return 0;
			if (num == 1)
				entry->napt = 1;
			else
				entry->napt = 0;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
			if (0 == getInputUint("IGMP (1)Enable (2)Disable: ", &num, &min, &max))
				return 0;
			if (num==1)
				entry->enableIGMP = 1;
			else
				entry->enableIGMP = 0;
#endif
		}
#ifdef CONFIG_USER_IP_QOS
		if (0 == getInputUint("IP QoS (1)Enable (2)Disable: ", &num, &min, &max))
			return 0;
		if (num==1)
			entry->enableIpQos = 1;
		else
			entry->enableIpQos = 0;
#endif
#ifdef CONFIG_IPV6
		if(entry->cmode == CHANNEL_MODE_DSLITE)
		{
			entry->AddrMode |= 4;
#ifdef CONFIG_IPV6
			entry->IpProtocol = IPVER_IPV6; 	// IPv6 only
#endif
		}
		else
		{
			max = 3;
			if (0 == getInputUint("IP Protocol (1)IPv4 (2)IPv6 (3)IPv4/IPv6: ", &num, &min, &max))
				return 0;
#ifdef CONFIG_IPV6
			entry->IpProtocol = (unsigned char)num;
#endif
		}
#endif
	}
	else
		entry->napt = 0;

	max=2;
	if (0 == getInputUint("The Status of Channel (1)Enable (2)Disable: ", &num, &min, &max))
		return 0;
	if (num==1)
		entry->enable = 1;
	else
		entry->enable = 0;

	entry->brmode = BRIDGE_DISABLE;

	// 1483 bridged
	if (entry->cmode == CHANNEL_MODE_BRIDGE)
	{
		entry->brmode = BRIDGE_ETHERNET;
		entry->ipAddr[0]=0;
		entry->ipAddr[1]=0;
		entry->ipAddr[2]=0;
		entry->ipAddr[3]=0;
		entry->remoteIpAddr[0]=0;
		entry->remoteIpAddr[1]=0;
		entry->remoteIpAddr[2]=0;
		entry->remoteIpAddr[3]=0;
		entry->netMask[0]=0;
		entry->netMask[1]=0;
		entry->netMask[2]=0;
		entry->netMask[3]=0;
#ifdef DEFAULT_GATEWAY_V1
		entry->dgw=0;
#endif
		entry->cdvt=0;

	}
	else
	{
#ifdef CONFIG_PPP
		if (entry->cmode == CHANNEL_MODE_PPPOE || entry->cmode == CHANNEL_MODE_PPPOA)
		{
			printf("PPP Settings\n");
			if (0 == getInputString("Username: ", entry->pppUsername, MAX_PPP_NAME_LEN))
				return 0;

			if (0 == getInputString("Password: ", entry->pppPassword, MAX_NAME_LEN-1))
				return 0;

			min=1;max=3;
			if (0 == getInputUint("Connect Type (1)Continuous (2)Connect on Demand (3)Manual: ", &num, &min, &max))
				return 0;

			if (1 == num)
				entry->pppCtype = CONTINUOUS;
			else if (2 == num)
				entry->pppCtype = CONNECT_ON_DEMAND;
			else
				entry->pppCtype = MANUAL;

	//		if (entry->pppCtype != CONTINUOUS) {
			if (entry->pppCtype == CONNECT_ON_DEMAND) {	// Jenny
				min=1;max=65535;
				if (0 == getInputUint("Idle Time (min): ", &num, &min, &max))
					return 0;
				entry->pppIdleTime = num;
			}
		}
		else
#endif
		if (entry->cmode == CHANNEL_MODE_IPOE || entry->cmode == CHANNEL_MODE_RT1483 || entry->cmode == CHANNEL_MODE_RT1577)
		{
#ifdef CONFIG_IPV6
			if(entry->IpProtocol & IPVER_IPV4)
#endif
			{
				printf("WAN IP Settings:\n");
				min=1;max=2;
				if (entry->cmode == CHANNEL_MODE_RT1483) {
					//num == 1;
					if (0 == getInputUint("Type (1)Fixed IP (2)Unnumbered: ", &num, &min, &max))
						return 0;
				}
				else if(entry->cmode != CHANNEL_MODE_RT1577) {
					//min=1;max=2;
					if (0 == getInputUint("Type (1)Fixed IP (2)DHCP: ", &num, &min, &max))
						return 0;
				}
				if (num == 1) {
					char *temp, strIp[20], strGW[20], strMask[20];
					entry->ipDhcp = DHCP_DISABLED;
					if (0 == getInputIpAddr("Local IP Address: ", (struct in_addr *)&entry->ipAddr)) return 0;
					if (0 == getInputIpAddr("Remote IP Address: ", (struct in_addr *)&entry->remoteIpAddr)) return 0;
					if (entry->cmode != CHANNEL_MODE_RT1483)
						if (0 == getInputIpMask("Subnet Mask: ", (struct in_addr *)&entry->netMask)) return 0;

					temp = inet_ntoa(*((struct in_addr *)entry->ipAddr));
					strcpy(strIp, temp);
					temp = inet_ntoa(*((struct in_addr *)entry->remoteIpAddr));
					strcpy(strGW, temp);
					temp = inet_ntoa(*((struct in_addr *)entry->netMask));
					strcpy(strMask, temp);
					if (!isValidHostID(strIp, strMask)) {
						printf("Invalid IP/Subnet Mask combination!!\n");
						return 0;
					}
					if (!isSameSubnet(strIp, strGW, strMask)) {
						printf("Invalid IP address! It should be located in the same subnet.\n");
						return 0;
					}
				} else {
					if (entry->cmode == CHANNEL_MODE_RT1483)
						entry->ipunnumbered = 1;
					else
						entry->ipDhcp = DHCP_CLIENT;
				}
			}
		}

#ifdef CONFIG_IPV6
		if(entry->IpProtocol & IPVER_IPV6)
		{
#ifdef DUAL_STACK_LITE
			if(entry->cmode == CHANNEL_MODE_DSLITE)
			{
				if (0 == getInputIp6Addr("Local IPv6 Address: ", (struct in6_addr *)&entry->Ipv6Addr))
					return 0;
				if (0 == getInputIp6Addr("Remote IPv6 End point Address: ", (struct in6_addr *)&entry->RemoteIpv6EndPointAddr))
					return 0;
				if (0 == getInputIp6Addr("Gateway IPv6 Address: ", (struct in6_addr *)&entry->RemoteIpv6Addr))
					return 0;

				entry->cmode = CHANNEL_MODE_IPOE; // DS-Lite only for MER
			}
			else
#endif
			{
				printf("IPv6 WAN Setting:\n");
				min = 1, max = 3;
				if (0 == getInputUint("Address Mode (1)Slaac (2)Static (3)Both: ", &num, &min, &max))
					return 0;

				entry->AddrMode = (unsigned char)num;

				if(num | 2) // IPv6 static address mode is selected.
				{
					if (0 == getInputIp6Addr("IPv6 Address: ", (struct in6_addr *)&entry->Ipv6Addr))
						return 0;

					min = 1, max = 128;
					if (0 == getInputUint("IPv6 prefix length: ", &num, &min, &max))
						return 0;

					entry->Ipv6AddrPrefixLen = (unsigned char)num;

					if (0 == getInputIp6Addr("IPv6 Gateway: ", (struct in6_addr *)&entry->RemoteIpv6Addr))
						return 0;
				}
			}
		}
#endif
	}

#ifdef DEFAULT_GATEWAY_V1
	if (entry->cmode != CHANNEL_MODE_BRIDGE)
	{
		max=2;
		if (0 == getInputUint("Default Route Setting (1)Disable (2)Enable :", &num, &min, &max))
			return 0;
		if (num==1)
			entry->dgw = 0;
		else
			entry->dgw = 1;
	}
#endif

	return 1;
}

/*************************************************************************************************
 *	Return:
 *	0: fail
 *	1: successful
 */
int addDSLConfiguration()
{
	MIB_CE_ATM_VC_T entry;
	MEDIA_TYPE_T mType;

	// Added by Mason Yu for AC Name is not NULL(WAN can not get IP Address on PPPoE Mode)
	memset(&entry, 0x00, sizeof(entry));

	if (getVcInput(&entry, 0) == 0)
	   return 0;

	// now do the actual set.
	do {
		unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
#ifdef DEFAULT_GATEWAY_V1
		int drflag = 0;
#endif
		int i, cnt, pIdx, intVal;
		unsigned int totalEntry;
		MIB_CE_ATM_VC_T Entry;
		//MIB_CE_ATM_VC_Tp pmyEntry;

		ifMap = 0;
		cnt=0;
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		if (totalEntry >= MAX_VC_NUM) {
			printf("Error: Maximum number of VC exceeds !");
			printf("\n");
			return 0;
		}

		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
				printf("Get chain record error!\n");
				return 0;
			}

			mType = MEDIA_INDEX(Entry.ifIndex);
			if (mType == MEDIA_ATM && Entry.vpi == entry.vpi && Entry.vci == entry.vci)
			{
				cnt++;
				pIdx = i;
			}

#ifdef DEFAULT_GATEWAY_V1
			if (Entry.cmode != CHANNEL_MODE_BRIDGE)
				if (Entry.dgw)
					drflag = 1;
#endif

			if (mType == MEDIA_ATM)
				ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		}

		if (cnt == 0)	// pvc not exists
		{
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == NA_VC)
			{
				printf("Error: Maximum number of VC exceeds !\n");
				return 0;
			}
			else if (entry.ifIndex == NA_PPP)
			{
				printf("Error: Maximum number of PPP exceeds !\n");
				return 0;
			}
#ifdef _CWMP_MIB_
			/*start use_fun_call_for_wan_instnum*/
			resetWanInstNum(&entry);
			updateWanInstNum(&entry);
			dumpWanInstNum(&entry, "new");
			/*end use_fun_call_for_wan_instnum*/
#endif /*_CWMP_MIB_*/
		}
		else	// pvc exists
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&Entry)) {	// for multisession PPPoE, get existed pvc config
				printf(errGetEntry);
				printf("\n");
				return 0;
			}
			//if (pmyEntry->cmode == CHANNEL_MODE_PPPOE)
			if (Entry.cmode == CHANNEL_MODE_PPPOE && entry.cmode == CHANNEL_MODE_PPPOE)
			{
				if (cnt<MAX_POE_PER_VC)
				{	// get the pvc info.
					//entry.qos = pmyEntry->qos;
					//entry.pcr = pmyEntry->pcr;
					//entry.encap = pmyEntry->encap;
					entry.qos = Entry.qos;
					entry.pcr = Entry.pcr;
					entry.encap = Entry.encap;
					ifMap &= 0xffff0000; // don't care the vc part
					entry.ifIndex = if_find_index(entry.cmode, ifMap);
					if (entry.ifIndex == NA_PPP)
					{
						printf("Error: Maximum number of PPP exceeds !\n");
						return 0;
					}
					entry.ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(entry.ifIndex), VC_INDEX(Entry.ifIndex));
#ifdef _CWMP_MIB_
					/*start use_fun_call_for_wan_instnum*/
					resetWanInstNum(&entry);
					entry.ConDevInstNum = Entry.ConDevInstNum;
					updateWanInstNum(&entry);
					dumpWanInstNum(&entry, "new");
					/*end use_fun_call_for_wan_instnum*/
#endif /*_CWMP_MIB_*/
				}
				else
				{
					printf("Maximum number of PPPoE connections exceeds in this vc!\n");
					return 0;
				}
			}
			else
			{
				printf("Connection already exists!\n");
				return 0;
			}
		}

#ifndef CONFIG_USER_RTK_WAN_CTYPE
#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != CHANNEL_MODE_BRIDGE)
			if (drflag && entry.dgw) {
				printf("Default route already exists !");
				printf("\n");
				return 0;
			}
#endif
#endif

#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == CHANNEL_MODE_RT1577)
			entry.encap = 1;	// LLC
#endif

		if (entry.cmode == CHANNEL_MODE_PPPOE)
		{
			entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyMaxUser=4;
			entry.PPPoEProxyEnable=0;
#endif
		}
		else
			entry.mtu = 1500;

#ifdef CONFIG_USER_WT_146
		wt146_set_default_config( &entry );
#endif //CONFIG_USER_WT_146
#ifdef NEW_PORTMAPPING
		check_itfGroup(&entry, 0);
#endif

		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			printf("Error! Add chain record.");
			printf("\n");
			restartWAN(CONFIGONE, NULL);
			return 0;
		}
		else if (intVal == -1) {
			printf("Error! Table Full.");
			printf("\n");
			restartWAN(CONFIGONE, NULL);
			return 0;
		}
		restartWAN(CONFIGONE, &entry);
	} while (0);

	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
	return 1;
}

/*************************************************************************************************
 *	Return:
 *	0: fail
 *	1: successful
 */
int deleteDSLConfiguration()
{
   //char vpi[4], vci[4];
   //unsigned int vpi, vci;
   unsigned int idx, min, max;
   int idx2entry[16];
   MIB_CE_ATM_VC_T tmpEntry;

#ifdef	WLAN_SUPPORT
	int ori_wlan_idx;
#endif

   CLEAR;
   max = wanConnList(1, MEDIA_ATM, 1, idx2entry);
   printf("\n\n");

   min=0; max--;
   if (0 == getInputUint("PVC to delete: ", &idx, &min, &max))
      return 0;
   // get the entry index
   idx = idx2entry[idx];
   resolveServiceDependency(idx);

   if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&tmpEntry))
   {
	   printf("Get chain record error!\n");
	   return 0;
   }
   deleteConnection(CONFIGONE, &tmpEntry);

   if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1) {
       printf("Delete chain record error!\n");

#ifdef IP_QOS
	update_qos_tbl(); /*delete qos entry related to this deleted channel*/
#endif
	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
      restartWAN(CONFIGONE, NULL);
      return 0;
   }
   restartWAN(CONFIGONE, NULL);
#ifdef IP_QOS
	update_qos_tbl(); /*delete qos entry related to this deleted channel*/
#endif
	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
	return 1;
}

void showDSLConfiguration()
{
	printf("\n");
	wanConnList(1, MEDIA_ATM, 1, NULL);
	printWaitStr();
}

void connectDSLConfiguration()
{
   unsigned int idx, min, max;
   int idx2entry[16];

   max = wanConnList(1, MEDIA_ATM, 1, idx2entry);
   printf("\n\n");

   min=0; max--;
   if (0 == getInputUint("Connect: ", &idx, &min, &max))
      return;
   // get the entry index
   idx = idx2entry[idx];

#ifdef CONFIG_PPP
   pppConnect(idx, 1);
#endif
}

void disconnectDSLConfiguration()
{
   unsigned int idx, min, max;
   int idx2entry[16];

   max = wanConnList(1, MEDIA_ATM, 1, idx2entry);
   printf("\n\n");

   min=0;max--;
   if (0 == getInputUint("Disconnect: ", &idx, &min, &max))
      return;
   // get the entry index
   idx = idx2entry[idx];

#ifdef CONFIG_PPP
   pppConnect(idx, 0);
#endif
}
#endif

#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
void showAutoPVCTbl()
{
	unsigned int entryNum, i;
	MIB_AUTO_PVC_SEARCH_T Entry;

	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	printf("\n");
	printf("Current Auto-PVC Table:\n");
	printf("Idx   VPI       VCI\n");
	printf("-------------------------------------------------------------------------\n");
	if (!entryNum)
		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		printf("%-6d%-10d%d\n", i+1, Entry.vpi, Entry.vci);
	}
}

void addAutoPVCPair()
{
	MIB_AUTO_PVC_SEARCH_T entry, Entry;
	unsigned int num, min, max;
	int mibtotal, i, intVal;

	memset(&entry, 0x00, sizeof(entry));
	min=0; max=255;
	if (0 == getInputUint("VPI:", &num, &min, &max))
		return;
	entry.vpi = (unsigned short)num;

	min=0;max=65535;
	if (0 == getInputUint("VCI:", &num, &min, &max))
		return;
	entry.vci = (unsigned int)num;

	mibtotal = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	for (i=0; i<mibtotal; i++) {
		mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void*)&Entry);
		if(entry.vpi == Entry.vpi && entry.vci == Entry.vci){
			printf("Auto-PVC pair already exists!\n");
			goto setErr_pvc;
		}
	}

	intVal = mib_chain_add(MIB_AUTO_PVC_SEARCH_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		goto setErr_pvc;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		goto setErr_pvc;
	}
	return;

	setErr_pvc:
		printWaitStr();
		return;
}

void setAutoPVCSearch()
{
	unsigned int choice, num, min, max;
	unsigned char vChar;
	char enabled;
	int intVal;

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                           Auto PVC Search                             \n");
		MENU_LINE;
		mib_get(MIB_ATM_VC_AUTOSEARCH, (void *)&enabled);
		printf("Auto-PVC Search: %s\n", enabled? STR_ENABLE: STR_DISABLE);
		if (enabled)
			showAutoPVCTbl();
		MSG_LINE;
		printf("(1) Set                          (2) Add\n");
		printf("(3) Delete                       (4) Quit\n");
		if (!getInputOption(&choice, 1, 4))
			continue;
		switch (choice) {
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			min = 1; max = 2;
			if (0 == getInputUint("Auto-PVC Search (1) Disable (2) Enable: ", &num, &min, &max))
				continue;
			vChar = (unsigned char)(num - 1);
			if (!mib_set(MIB_ATM_VC_AUTOSEARCH, (void *)&vChar)) {
				printf("Set auto-pvc search error!");
				printWaitStr();
			}
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			break;

		case 2://(2) Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			addAutoPVCPair();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			break;

		case 3://(3) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			num = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL); /* get chain record size */
			if (num == 0) {
				printf("Empty table!\n");
				printWaitStr();
				continue;
			}
			showAutoPVCTbl();
			min=1; max=num;
			if (getInputUint("Select the index to delete:", &intVal, &min, &max) == 0) {
				printf("Error selection!\n");
				printWaitStr();
				continue;
			}
			if(mib_chain_delete(MIB_AUTO_PVC_SEARCH_TBL, intVal - 1) != 1) {
				printf("Delete chain record error!");
				printWaitStr();
			}
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			break;

		case 4://(4) Quit
			return;
		}
	}
}
#endif

#ifdef CONFIG_USER_WT_146
void setBFDSetting_show(void)
{
	unsigned int idx, min, max;
	MIB_CE_ATM_VC_T Entry, *p;
	int idx2entry[16];

	CLEAR;
	max = wanConnList(1, MEDIA_ATM, 1, idx2entry);
	printf("\n\n");

	min=0;max--;
	if (0 == getInputUint("PVC to Show: ", &idx, &min, &max))
		return;
	printf( "\n" );
	// get the entry index
	idx = idx2entry[idx];

	p=&Entry;
	if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)p))
	{
		printf( "internal error: mib_chain_get() failed\n");
		printWaitStr();
		return;
	}

	if ( (Entry.cmode==CHANNEL_MODE_IPOE)||(Entry.cmode==CHANNEL_MODE_RT1483) )
	{
		int bfdlen;

		printf( "BFD                  : %s\n", p->bfd_enable?"Enable":"Disable" );
		printf( "Mode                 : " );
			switch(p->bfd_opmode)
			{
			case BFD_ASYNC_MODE:
				printf( "Asynchronous" );
				break;
			case BFD_DEMAND_MODE:
				printf( "Demand" );
				break;
			}
			printf( "\n" );
		printf( "Role                 : " );
			switch(p->bfd_role)
			{
			case BFD_ACTIVE_ROLE:
				printf( "Active" );
				break;
			case BFD_PASSIVE_ROLE:
				printf( "Passive" );
				break;
			}
			printf( "\n" );
		printf( "Detect Mult          : %u\n", p->bfd_detectmult );
		printf( "Min Tx Interval      : %u microseconds\n", p->bfd_mintxint );
		printf( "Min Rx Interval      : %u microseconds\n", p->bfd_minrxint );
		printf( "Min Echo Rx Interval : %u microseconds\n", p->bfd_minechorxint );
		printf( "Auth Type            : " );
			switch(p->bfd_authtype)
			{
			case BFD_AUTH_NONE:
				printf( "None" );
				break;
			case BFD_AUTH_PASSWORD:
				printf( "Simple Password" );
				break;
			case BFD_AUTH_MD5:
				printf( "Keyed MD5" );
				break;
			case BFD_AUTH_METI_MD5:
				printf( "Meticulous Keyed MD5" );
				break;
			case BFD_AUTH_SHA1:
				printf( "Keyed SHA1" );
				break;
			case BFD_AUTH_METI_SHA1:
				printf( "Meticulous Keyed SHA1" );
				break;
			}
			printf( "\n" );
		printf( "Auth Key ID          : %u\n", p->bfd_authkeyid );
		printf( "Auth Key (Hex)       : " );
			bfdlen=0;
			while(bfdlen<p->bfd_authkeylen)
			{
				printf( "%02x", p->bfd_authkey[bfdlen] );
				bfdlen++;
			}
			printf( "\n" );
		printf( "DSCP                 : %u\n", p->bfd_dscp );
		if(p->cmode==CHANNEL_MODE_IPOE)
			printf( "Ethernet Priority    : %u\n", p->bfd_ethprio );
	}else{
		printf("not allowed for this connection !\n");
	}

	printf( "\n" );
	printWaitStr();

	return;
}

void setBFDSetting_modify(void)
{
	unsigned int idx, min, max, num;
	MIB_CE_ATM_VC_T Entry, *p;
	MIB_CE_ATM_VC_T OldEntry;
	unsigned char tmpbuf[BFD_MAX_KEY_LEN*2+1];
	int idx2entry[16];

	CLEAR;
	max = wanConnList(1, MEDIA_ATM, 1, idx2entry);
	printf("\n\n");

	min=0;max--;
	if (0 == getInputUint("PVC to Modify: ", &idx, &min, &max))
		return;
	printf( "\n" );
	// get the entry index
	idx = idx2entry[idx];

	p=&Entry;
	if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)p))
	{
		printf( "internal error: mib_chain_get() failed\n");
		printWaitStr();
		return;
	}
	memcpy( &OldEntry, p, sizeof(OldEntry) );

	if ( (Entry.cmode!=CHANNEL_MODE_IPOE)&&(Entry.cmode!=CHANNEL_MODE_RT1483) )
	{
		printf("not allowed for this connection !\n");
		printf( "\n" );
		printWaitStr();
		return;
	}


	/*bfd_enable*/
	min=1;max=2;
	if( getInputUint("Set BFD (1)Disable (2)Enable : ", &num, &min, &max)==0 ) return;
	p->bfd_enable=(num==1)?0:1;
	if(p->bfd_enable==0) goto setBFD_modify_done;

	/*bfd_opmode*/
	min=1;max=2;
	if( getInputUint("Set Mode (1)Asynchronous (2)Demand : ", &num, &min, &max)==0 ) return;
	switch(num)
	{
	case 1:
		p->bfd_opmode=BFD_ASYNC_MODE;
		break;
	case 2:
		p->bfd_opmode=BFD_DEMAND_MODE;
		break;
	}

	/*bfd_role*/
	min=1;max=2;
	if( getInputUint("Set Role (1)Active (2)Passive : ", &num, &min, &max)==0 ) return;
	switch(num)
	{
	case 1:
		p->bfd_role=BFD_ACTIVE_ROLE;
		break;
	case 2:
		p->bfd_role=BFD_PASSIVE_ROLE;
		break;
	}

	/*bfd_detectmult*/
	min=1;max=255;
	if( getInputUint("Set Detect Mult : ", &num, &min, &max)==0 ) return;
	p->bfd_detectmult=num;

	/*bfd_mintxint*/
	min=0;max=4294967295;
	if( getInputUint("Set Min Tx Interval : ", &num, &min, &max)==0 ) return;
	p->bfd_mintxint=num;

	/*bfd_minrxint*/
	min=0;max=4294967295;
	if( getInputUint("Set Min Rx Interval : ", &num, &min, &max)==0 ) return;
	p->bfd_minrxint=num;

	/*bfd_minechorxint*/
	min=0;max=4294967295;
	if( getInputUint("Set Min Echo Rx Interval : ", &num, &min, &max)==0 ) return;
	p->bfd_minechorxint=num;

	/*bfd_authtype*/
	min=1;max=6;
	if( getInputUint("Set Auth Type (1)None (2)Simple Password (3)Keyed MD5 (4)Meticulous Keyed MD5  (5)Keyed SHA1 (6)Meticulous Keyed SHA1 : ", &num, &min, &max)==0 ) return;
	switch(num)
	{
	case 1:
		p->bfd_authtype=BFD_AUTH_NONE;
		break;
	case 2:
		p->bfd_authtype=BFD_AUTH_PASSWORD;
		break;
	case 3:
		p->bfd_authtype=BFD_AUTH_MD5;
		break;
	case 4:
		p->bfd_authtype=BFD_AUTH_METI_MD5;
		break;
	case 5:
		p->bfd_authtype=BFD_AUTH_SHA1;
		break;
	case 6:
		p->bfd_authtype=BFD_AUTH_METI_SHA1;
		break;
	}

	if( p->bfd_authtype!=BFD_AUTH_NONE )
	{
		/*bfd_authkeyid*/
		min=0;max=255;
		if( getInputUint("Set Auth Key ID : ", &num, &min, &max)==0 ) return;
		p->bfd_authkeyid=num;

again_bfdkeyinput:
		/*bfd_authkey, bfd_authkeylen*/
		if( getInputString( "Set Auth Key (Hex) : ", tmpbuf, sizeof(tmpbuf) ) == 0)
		{
			//printf( "Invalid Auth Key value!\n");
			//goto setBFD_modify_error;
			return;
		}else{
			int bfdstrlen;
			int bfd_i, bfd_idx;
			unsigned char bfdtmpbuf[4];
			int bfdchecklen;

			//check len
			bfdchecklen=0;
			bfdstrlen=strlen( tmpbuf );
			if(Entry.bfd_authtype==BFD_AUTH_PASSWORD)
			{
				if( (bfdstrlen&0x1) ||
					(bfdstrlen<(BFD_AUTH_PASS_MINKEYLEN*2)) ||
					(bfdstrlen>(BFD_AUTH_PASS_MAXKEYLEN*2)) )
					bfdchecklen=1;
			}else if( (Entry.bfd_authtype==BFD_AUTH_MD5) ||
				 (Entry.bfd_authtype==BFD_AUTH_METI_MD5) )
			{
				if( bfdstrlen!=(BFD_AUTH_MD5_KEYLEN*2) )
					bfdchecklen=1;
			}else if( (Entry.bfd_authtype==BFD_AUTH_SHA1) ||
				 (Entry.bfd_authtype==BFD_AUTH_METI_SHA1) )
			{
				if( bfdstrlen!=(BFD_AUTH_SHA1_KEYLEN*2) )
					bfdchecklen=1;
			}else{
				bfdchecklen=1;
			}
			if(bfdchecklen)
			{
				printf( "Invalid Auth Key Length!\n");
				//goto setBFD_modify_error;
				goto again_bfdkeyinput;
			}

			//check key
			for( bfd_idx=0; bfd_idx<bfdstrlen; bfd_idx++ )
			{

				if(!(	((tmpbuf[bfd_idx]>='0')&&(tmpbuf[bfd_idx]<='9')) ||
					((tmpbuf[bfd_idx]>='A')&&(tmpbuf[bfd_idx]<='F')) ||
					((tmpbuf[bfd_idx]>='a')&&(tmpbuf[bfd_idx]<='f')) ))
				{
					printf( "Invalid Auth Key value (0-9,a-f,A-F)!\n");
					//goto setBFD_modify_error;
					goto again_bfdkeyinput;
				}
			}
			bfd_i=0;
			for( bfd_idx=0; bfd_idx<bfdstrlen; bfd_idx+=2 )
			{
				bfdtmpbuf[0]=tmpbuf[bfd_idx];
				bfdtmpbuf[1]=tmpbuf[bfd_idx+1];
				bfdtmpbuf[2]=0;
				p->bfd_authkey[bfd_i] = (unsigned char) strtol(bfdtmpbuf, (char**)NULL, 16);
				bfd_i++;
			}
			p->bfd_authkeylen=bfd_i;
		}
	}

	/*bfd_dscp*/
	min=0;max=63;
	if( getInputUint("Set DSCP : ", &num, &min, &max)==0 ) return;
	p->bfd_dscp=num;

	if(p->cmode==CHANNEL_MODE_IPOE)
	{
		/*bfd_ethprio*/
		min=0;max=7;
		if( getInputUint("Set Ethernet Priority : ", &num, &min, &max)==0 ) return;
		p->bfd_ethprio=num;
	}

setBFD_modify_done:
	if(!mib_chain_update( MIB_ATM_VC_TBL, (void*)p, idx))
	{
		printf( "internal error: mib_chain_update() failed\n");
		goto setBFD_modify_error;
	}
#ifdef APPLY_CHANGE
	wt146_del_wan(&OldEntry);
	wt146_create_wan(p, 1);
#endif //APPLY_CHANGE
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	//debug
	//printWaitStr();
	return;

setBFD_modify_error:
	printWaitStr();
	return;
}

static void setBFDSetting(void)
{
	int snum;

	while (1)
   	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                      BFD Setting             \n");
		MENU_LINE;
		printf("This page is used to configure the BFD parameters for WAN\n");
		printf("channels\n");
		MENU_LINE;
		cli_show_menu( strSetBFDSetting, SET_BFD_SETTING_END );
		if (!getInputOption( &snum, 1, SET_BFD_SETTING_END))
			continue;

		switch(snum-1)
		{
		case SET_BFD_SETTING_SHOW:
			setBFDSetting_show();
			break;
		case SET_BFD_SETTING_MODIFY:
			setBFDSetting_modify();
			break;
		case SET_BFD_SETTING_QUIT:
			return;
		default:
			break;
		}
	}
}
#endif //CONFIG_USER_WT_146

#ifdef NEW_PORTMAPPING
struct pmap_lan {
	PMAP_LAN_T index;
	char *ifname;
};

int setNewPortMapping(int mType)
{
	MIB_CE_ATM_VC_T vc_entry;
	int total_entries, i, flags, pvcnum, min, max, num = 0, pvcidx[MAX_VC_NUM], j = 0;
	char *temp, ifname[IFNAMSIZ], pvc[10], *aal5Encap, *mode, ipaddr[20], remoteip[20];
	struct in_addr inAddr;
	FILE *fp;
	char *map_ports[] = {"LAN_1", "LAN_2", "LAN_3", "LAN_4", "WLAN0", "WLAN0-AP1", "WLAN0-AP2", "WLAN0-AP3", "WLAN0-AP4"
														   , "WLAN1", "WLAN1-AP1", "WLAN1-AP2", "WLAN1-AP3", "WLAN1-AP4"};
	unsigned int num_lan, valid_lan;
	struct pmap_lan lan_list[10];
	char optStr[256];

	total_entries = mib_chain_total(MIB_ATM_VC_TBL);

	CLEAR;
	printf("Current ATM VC Table:\n");
	MSG_LINE;
	printf("%-4s%-8s%-8s%-7s%-13s%-17s%-17s\n", "Idx", "Inf", "PVC", "Encap", "Mode", "IP Address", "Remote IP");
	MSG_LINE;

	for( i = 0; i < total_entries; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vc_entry))
			continue;

		if(MEDIA_INDEX(vc_entry.ifIndex) != mType)
			continue;

		pvcidx[j] = i;

		ifGetName(vc_entry.ifIndex, ifname, sizeof(ifname));
		sprintf(pvc, "%d/%d", vc_entry.vpi, vc_entry.vci);

		if (vc_entry.encap == 0)
			aal5Encap = "VCMUX";
		else
			aal5Encap = "LLC";

		if (vc_entry.cmode == CHANNEL_MODE_PPPOE)
			mode = "PPPoE";
		else if (vc_entry.cmode == CHANNEL_MODE_PPPOA)
			mode = "PPPoA";
		else if (vc_entry.cmode == CHANNEL_MODE_BRIDGE)
			mode = "br1483";
		else if (vc_entry.cmode == CHANNEL_MODE_IPOE)
			mode = "mer1483";
		else if (vc_entry.cmode == CHANNEL_MODE_RT1483)
			mode = "rt1483";
#ifdef CONFIG_ATM_CLIP
		else if (vc_entry.cmode == CHANNEL_MODE_RT1577)
			mode = "rt1577";
#endif
#ifdef DUAL_STACK_LITE
		if ((vc_entry.AddrMode & 0x4) == 0x4 )
			mode = "DS-Lite-mer";
#endif

		if(getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			if (getInFlags(ifname, &flags) == 1)
				if ((strcmp(temp, "10.0.0.1") == 0) && flags & IFF_POINTOPOINT)	// IP Passthrough or IP unnumbered
					strcpy(ipaddr, STR_UNNUMBERED);
				else if (strcmp(temp, "64.64.64.64") == 0)
					strcpy(ipaddr, "");
				else
					strcpy(ipaddr, temp);
		}
		else
			strcpy(ipaddr, "");

		if (getInAddr(ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			if (strcmp(temp, "10.0.0.2") == 0)
				strcpy(remoteip, STR_UNNUMBERED);
			else if (strcmp(temp, "64.64.64.64") == 0)
				strcpy(remoteip, "");
			else
				strcpy(remoteip, temp);
			if (getInFlags(ifname, &flags) == 1)
				if (flags & IFF_BROADCAST) {
					unsigned char value[32];
					snprintf(value, 32, "%s.%s", (char *)MER_GWINFO, ifname);
					if (fp = fopen(value, "r")) {
						fscanf(fp, "%s\n", remoteip);
						fclose(fp);
					}
					else
						strcpy(remoteip, "");
				}
		}
		else
			strcpy(remoteip, "");

		printf("%-4d%-8s%-8s%-7s%-13s%-17s%-17s\n", j++, ifname, pvc, aal5Encap, mode, ipaddr, remoteip);
	}
	MSG_LINE;

	min = 0, max = j - 1;
	if (0 == getInputUint("Please select index of channel to set port mapping: ", &pvcnum, &min, &max))
		return 0;

	if(!mib_chain_get(MIB_ATM_VC_TBL, pvcidx[pvcnum], (void *)&vc_entry))
	{
		fprintf(stderr, "Get %dth entry of MIB_ATM_VC_TBL failed!\n", pvcidx[pvcnum]);
		printWaitStr();
		return -1;
	}

	sprintf(optStr, "Please select ports for port mapping, selecting a mapped port will remove\nit from mapped port list:\n\n(0)Clear all\n");
	j=0;
	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3,i<SW_LAN_PORT_NUM; i++) {
		lan_list[j].index = i;
		lan_list[j].ifname = map_ports[i];
		sprintf(optStr, "%s(%d)%s ", optStr, j+1, map_ports[i]);
		j++;
	}
#ifdef WLAN_SUPPORT
	lan_list[j].index = PMAP_WLAN0;
	lan_list[j].ifname = map_ports[PMAP_WLAN0];
	sprintf(optStr, "%s\n(%d)%s ", optStr, j+1, map_ports[PMAP_WLAN0]);
	j++;
#ifdef WLAN_MBSSID
	for (i=0; i<=(PMAP_WLAN0_VAP3-PMAP_WLAN0_VAP0); i++) {
		lan_list[j].index = PMAP_WLAN0_VAP0+i;
		lan_list[j].ifname = map_ports[PMAP_WLAN0_VAP0+i];
		sprintf(optStr, "%s(%d)%s ", optStr, j+1, map_ports[PMAP_WLAN0_VAP0+i]);
		j++;
	}
#endif
#ifdef WLAN_DUALBAND_CONCURRENT
	lan_list[j].index = PMAP_WLAN1;
	lan_list[j].ifname = map_ports[PMAP_WLAN1];
	sprintf(optStr, "%s\n(%d)%s", optStr, j+1, map_ports[PMAP_WLAN1]);
	j++;
#ifdef WLAN_MBSSID
	for (i=0; i<=(PMAP_WLAN1_VAP3-PMAP_WLAN1_VAP0); i++) {
		lan_list[j].index = PMAP_WLAN1_VAP0+i;
		lan_list[j].ifname = map_ports[PMAP_WLAN1_VAP0+i];
		sprintf(optStr, "%s(%d)%s ", optStr, j+1, map_ports[PMAP_WLAN1_VAP0+i]);
		j++;
	}
#endif
#endif
#endif
	sprintf(optStr, "%s\n(%d)Save: ", optStr, j+1);
	num_lan = j;
	max = j+1; // clear + lan + save
	while(1)
	{
		int mp_len = 14;

		CLEAR;
		MSG_LINE;
		printf("Mapped ports: ");
		for (i = PMAP_ETH0_SW0; i < PMAP_ITF_END; i++) {
			if((vc_entry.itfGroup & (1 << i))) {
				valid_lan = 0;
				for (j = 0; j < num_lan; j++) {
					if (lan_list[j].index == i) { // found in lan_list -> valid lan itf
							valid_lan = 1;
							mp_len += strlen(map_ports[i]) + 1;
							if(mp_len > 79)
							{
								printf("\n%14s", " ");
								mp_len = 14;
							}

							printf("%s ", map_ports[i]);
					}
				}
				if (!valid_lan) // remove the mapped lan
					vc_entry.itfGroup &= ~(1 << i);
			}
		}
		putchar('\n');
		MSG_LINE;
		putchar('\n');

		if (0 == getInputUint(optStr, &num, &min, &max))
			return 0;

		if(num == max)
		{
			MIB_CE_ATM_VC_T tmpEntry;

			if (!mib_chain_get(MIB_ATM_VC_TBL, pvcidx[pvcnum], (void *)&tmpEntry))
			{
				printf("Get chain record error!\n");
				return 0;
			}
			deleteConnection(CONFIGONE, &tmpEntry);

			if(!mib_chain_update(MIB_ATM_VC_TBL, (void *)&vc_entry, pvcidx[pvcnum]))
			{
				fprintf(stderr, "Update %dth entry of MIB_ATM_VC_TBL failed!\n", pvcidx[pvcnum]);
				printWaitStr();
				return -1;
			}
	#ifdef COMMIT_IMMEDIATELY
			Commit();
	#endif
			restartWAN(CONFIGONE, &vc_entry);
			break;
		}

		if(num == 0)
			vc_entry.itfGroup = 0;
		else
			vc_entry.itfGroup ^= 1 << (lan_list[num-1].index);

	}
}
#endif

#ifdef CONFIG_DEV_xDSL
void setDSLConfig()
{
	int snum;

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                           DSL Configuration                         \n");
		MENU_LINE;
		printf("This page is used to configure the parameters for the channel operation  \n");
		printf("modes of your ADSL Modem/Router.                                         \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		cli_show_menu(strSetChanConf, SET_DSL_CONF_END);
		if (!getInputOption( &snum, 1, SET_DSL_CONF_END))
			continue;

		switch(snum-1)
		{
		case SET_DSL_CONF_ADD: // Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			snum = addDSLConfiguration();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			if (snum)
				Commit();
#endif
			break;
		case SET_DSL_CONF_DELETE: // Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			snum = deleteDSLConfiguration();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			if (snum)
				Commit();
#endif
			break;
#ifdef NEW_PORTMAPPING
		case SET_DSL_CONF_PORT_MAPPING:
			setNewPortMapping(MEDIA_ATM);
			break;
#endif
		case SET_DSL_CONF_SHOW: // Show
			showDSLConfiguration();
			break;
		case SET_DSL_CONF_CONNECT: // Connect
			connectDSLConfiguration();
			break;
		case SET_DSL_CONF_DISCONNECT: // Disconnect
			disconnectDSLConfiguration();
			break;
		case SET_DSL_CONF_PVCSEARCH: // Auto-PVC Search
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
			setAutoPVCSearch();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
#ifdef CONFIG_USER_WT_146
		case SET_DSL_CONF_BFDSETTING: // BFD Setting
			setBFDSetting();
			break;
#endif //CONFIG_USER_WT_146
		case SET_DSL_CONF_QUIT: // Quit
			return;
		}//end switch, Channel Configuration
	}//end while, Channel Configuration
}
#endif

int modifyATMSettings()
{
	unsigned int pvcnum, num, min, max;
	//char vpi[4], vci[4];
	//int qos, pcr, cdvt, scr, mbs;
	int i, k,totalEntry;
	MIB_CE_ATM_VC_T Entry;
	MIB_CE_ATM_VC_T entry;
	int idx2entry[16];

	CLEAR;
	max = cli_atmVcList(idx2entry);
	printf("\n\n");

	min=0;max--;
	if (0 == getInputUint("PVC to modify: ", &pvcnum, &min, &max))
		return 0;
	// get the entry index
	pvcnum = idx2entry[pvcnum];

	if (!mib_chain_get(MIB_ATM_VC_TBL, pvcnum, (void *)&Entry))
	{
		printf("Get chain record error!\n");
		return 0;
	}

	i = pvcnum;

	memcpy(&entry, &Entry, sizeof(entry));


	/*printf("VPI:");
	if ( getInputStr( vpi, sizeof(vpi)-1)==0) return;
	printf("VCI:");
	if ( getInputStr( vci, sizeof(vci)-1)==0) return;*/

	printf("Old QoS: %s\n", strQos[Entry.qos]);
	//printf("New QoS (1)UBR (2)CBR (3)rt-VBR (4)nrt-VBR :");
	min=1;max=4;
	if (0 == getInputUint("New QoS (1)UBR (2)CBR (3)nrt-VBR (4)rt-VBR: ", &num, &min, &max))
		return 0;
	entry.qos = num - 1;

	printf("Old PCR: %d\n", Entry.pcr);

	min=0;max=65535;
	if (0 == getInputUint("New PCR:", &num, &min, &max))
	   return 0;
	entry.pcr = num;

	max=0xffffffff;
	printf("Old CDVT: %d\n", Entry.cdvt);
	if (0 == getInputUint("New CDVT:", &num, &min, &max))
		return 0;
	entry.cdvt = num;

	if (entry.qos == 2 || entry.qos == 3)
	{
		max=65535;
		printf("Old SCR: %d\n", Entry.scr);
		printf("New SCR:");
		if (0 == getInputUint("New SCR:", &num, &min, &max))
			return 0;
		entry.scr = num;

		printf("Old MBS: %d\n", Entry.mbs);
		if (0 == getInputUint("New MBS:", &num, &min, &max))
			return 0;
		entry.mbs = num;
	}

#if defined(APPLY_CHANGE)
		// Take effect in real time
		stopConnection(&Entry);
#endif
	memcpy(&Entry, &entry, sizeof(entry));
	mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, pvcnum);
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

	// synchronize this vc across all interfaces
	for (k=i+1; k<totalEntry; k++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&Entry)) {
			if (MEDIA_INDEX(Entry.ifIndex) != MEDIA_ATM)
				continue;
			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
				Entry.qos = entry.qos;
				Entry.pcr = entry.pcr;
				Entry.cdvt = entry.cdvt;
				Entry.scr = entry.scr;
				Entry.mbs = entry.mbs;
				mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, k);
			}
		}
	}

#if defined(APPLY_CHANGE)
		// Take effect in real time
		startConnection(&entry, pvcnum);
#endif
	return 1;
}

void setATMSettings()
{
	int snum;

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           ATM Settings                                  \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the parameters for the ATM of your ADSL   \n");
		printf("Router. Here you may change the setting for VPI, VCI, QoS etc ...        \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n\n");
		printf("Current ATM VC Table:\n\n");
		cli_atmVcList(0);
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n\n");
		printf("\n(1) Modify                       (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
		case 1://(1) Modify
			if (!check_access(SECURITY_SUPERUSER))
				break;
			snum = modifyATMSettings();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			if (snum)
				Commit();
#endif
			break;
		case 2://(2) Quit
			return;
		}//end switch, ATM Settings
	}//end while, ATM Settings
}

#ifdef CONFIG_DEV_xDSL
//Jenny
void showDSLSettings(){
	unsigned char vChar;
	unsigned short mode;

	mib_get( MIB_ADSL_MODE, (void *)&mode);
	#ifdef CONFIG_VDSL
	printf("DSL modulation:\n");	// DSL modulation
	#else
	printf("ADSL modulation:\n");	// ADSL modulation
	#endif /*CONFIG_VDSL*/
	if (!(mode & ADSL_MODE_ANXB)) {
		if (mode & ADSL_MODE_GLITE)
			printf("\tG.Lite:\t  Enable\n");
		else
			printf("\tG.Lite:\t  Disable\n");
	}
	if (mode & ADSL_MODE_GDMT)
		printf("\tG.Dmt:\t  Enable\n");
	else
		printf("\tG.Dmt:\t  Disable\n");
	if (!(mode & ADSL_MODE_ANXB)) {
		if (mode & ADSL_MODE_T1413)
			printf("\tT1.413:\t  Enable\n");
		else
			printf("\tT1.413:\t  Disable\n");
	}
	if (mode & ADSL_MODE_ADSL2)
		printf("\tADSL2:\t  Enable\n");
	else
		printf("\tADSL2:\t  Disable\n");
	if (mode & ADSL_MODE_ADSL2P)
		printf("\tADSL2+:\t  Enable\n");
	else
		printf("\tADSL2+:\t  Disable\n");
#ifdef CONFIG_VDSL
	if (mode & ADSL_MODE_VDSL2)
		printf("\tVDSL2:\t  Enable\n");
	else
		printf("\tVDSL2:\t  Disable\n");
#endif /*CONFIG_VDSL*/
	if (!(mode & ADSL_MODE_ANXB)) {
		printf("AnnexL Option:\n");	// AnnexL Option
		if (mode & ADSL_MODE_ANXL)
			printf("\tAnnexL:\t  Enable\n");
		else
			printf("\tAnnexL:\t  Disable\n");
		printf("AnnexM Option:\n");	// AnnexM Option
		if (mode & ADSL_MODE_ANXM)
			printf("\tAnnexM:\t  Enable\n");
		else
			printf("\tAnnexM:\t  Disable\n");
	}
#ifdef ENABLE_ADSL_MODE_GINP
	printf("G.INP Option:\n");	// G.INP Option
	if (mode & ADSL_MODE_GINP)
		printf("\tG.INP:\t  Enable\n");
	else
		printf("\tG.INP:\t  Disable\n");
#endif


#ifdef CONFIG_VDSL
{
	unsigned short profile;
	mib_get(MIB_VDSL2_PROFILE, (void *)&profile);
	printf("VDSL2 Profile:\n");	// VDSL2
	printf("\t8a:\t  %s\n", (profile&VDSL2_PROFILE_8A)?"Enable":"Disable" );
	printf("\t8b:\t  %s\n", (profile&VDSL2_PROFILE_8B)?"Enable":"Disable" );
	printf("\t8c:\t  %s\n", (profile&VDSL2_PROFILE_8C)?"Enable":"Disable" );
	printf("\t8d:\t  %s\n", (profile&VDSL2_PROFILE_8D)?"Enable":"Disable" );
	printf("\t12a:\t  %s\n", (profile&VDSL2_PROFILE_12A)?"Enable":"Disable" );
	printf("\t12b:\t  %s\n", (profile&VDSL2_PROFILE_12B)?"Enable":"Disable" );
	printf("\t17a:\t  %s\n", (profile&VDSL2_PROFILE_17A)?"Enable":"Disable" );
	printf("\t30a:\t  %s\n", (profile&VDSL2_PROFILE_30A)?"Enable":"Disable" );
}
#endif /*CONFIG_VDSL*/

	mib_get( MIB_ADSL_OLR, (void *)&vChar);
	printf("ADSL Capability:\n");	// OLR type
	if (vChar & 1)
		printf("\tBitswap:  Enable\n");
	else
		printf("\tBitswap:  Disable\n");
	if (vChar & 2)
		printf("\tSRA:\t  Enable\n");
	else
		printf("\tSRA:\t  Disable\n");
}

//Jenny
void changeADSLSettings(){
	int min,max,sel, xmode, anxb;
	char olr;
	unsigned short mode;
#ifdef CONFIG_VDSL
	unsigned short profile;
#endif /*CONFIG_VDSL*/

	// annex B and annex A are incompatible,
	mib_get(MIB_ADSL_MODE, (void *)&mode);
	if (mode & ADSL_MODE_ANXB) {
		anxb = 1;
		// annex B will not display T1.413, G.lite, annex.L and annex.M, so leave it as is.
		mode &= (ADSL_MODE_ANXB|ADSL_MODE_T1413|ADSL_MODE_GLITE|ADSL_MODE_ANXL|ADSL_MODE_ANXM);
	}
	else {
		anxb = 0;
		mode = 0;
	}


	min=1;max=2;
	if (!anxb) {
		if (getInputUint("G.Lite (1) Enable (2) Disable:",&sel,&min,&max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		if (sel==1)
			mode |= ADSL_MODE_GLITE;
	}
	if (getInputUint("G.Dmt (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_GDMT;
	if (!anxb) {
		if (getInputUint("T1.413 (1) Enable (2) Disable:",&sel,&min,&max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		if (sel==1)
			mode |= ADSL_MODE_T1413;
	}
	if (getInputUint("ADSL2 (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_ADSL2;
	if (getInputUint("ADSL2+ (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_ADSL2P;
#ifdef CONFIG_VDSL
	if (getInputUint("VDSL2 (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_VDSL2;
#endif /*CONFIG_VDSL*/

	if (!anxb) {
		if (getInputUint("AnnexL (1) Enable (2) Disable:",&sel,&min,&max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		if (sel==1)
			mode |= ADSL_MODE_ANXL;

		if (getInputUint("AnnexM (1) Enable (2) Disable:",&sel,&min,&max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		if (sel==1)
			mode |= ADSL_MODE_ANXM;
	}
#ifdef ENABLE_ADSL_MODE_GINP
	if (getInputUint("G.INP (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		mode |= ADSL_MODE_GINP;
#endif


#ifdef CONFIG_VDSL
if(mode&ADSL_MODE_VDSL2)
{
	profile=0;
	if (getInputUint("8a (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_8A;
	if (getInputUint("8b (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_8B;
	if (getInputUint("8c (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_8C;
	if (getInputUint("8d (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_8D;
	if (getInputUint("12a (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_12A;
	if (getInputUint("12b (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_12B;
	if (getInputUint("17a (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_17A;
	if (getInputUint("30a (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		profile |= VDSL2_PROFILE_30A;
}
#endif /*CONFIG_VDSL*/


	olr = 0;
	if (getInputUint("Bitswap (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		olr |= 1;
	if (getInputUint("SRA (1) Enable (2) Disable:",&sel,&min,&max)==0) {
		printf("Invalid selection!\n");
		return;
	}
	if (sel==1)
		olr |= 2;

	mib_set(MIB_ADSL_MODE, (void *)&mode);
	mib_set(MIB_ADSL_OLR, (void *)&olr);
#ifdef CONFIG_VDSL
	if(mode&ADSL_MODE_VDSL2)
		mib_set(MIB_VDSL2_PROFILE, (void *)&profile);
#endif /*CONFIG_VDSL*/

#ifdef CONFIG_USER_XDSL_SLAVE
	mib_slv_sync_dsl();
	sys_slv_init( UC_STR_DSL_SETUP );
#endif /*CONFIG_USER_XDSL_SLAVE*/
#ifdef CONFIG_VDSL
	/*use function call instead, like Web UI*/
	setupDsl();
#ifdef CONFIG_VDSL
	/*when ad<->vd, need retrain*/
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif /*CONFIG_VDSL*/
#else
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
	xmode=0;
	if (mode & (ADSL_MODE_GDMT|ADSL_MODE_T1413))
		xmode |= 1;	// ADSL1
	if (mode & ADSL_MODE_ADSL2)
		xmode |= 2;	// ADSL2
	if (mode & ADSL_MODE_ADSL2P)
		xmode |= 4;	// ADSL2+
	adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);

	xmode = mode & (ADSL_MODE_T1413|ADSL_MODE_GDMT);	//  1: ansi, 2: g.dmt, 8:g.lite
	adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&xmode, 4);

	if (mode & ADSL_MODE_ANXL)	// Annex L
		xmode = 3; // Wide-Band & Narrow-Band Mode
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&xmode, 4);

	if (mode & ADSL_MODE_ANXM)	// Annex M
		xmode = 1;
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&xmode, 4);

#ifdef ENABLE_ADSL_MODE_GINP
	if (mode & ADSL_MODE_GINP)	// G.INP
		xmode = DSL_FUNC_GINP;
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_DSL_FUNC, (void *)&xmode, 4);
#endif

	xmode = (int)olr;
	if (xmode == 2)// SRA (should include bitswap)
		xmode = 3;
	adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&xmode, 4);
#endif /*CONFIG_VDSL*/
}

//Jenny
void setADSLSettings()
{
	int min,max,sel;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		#ifdef CONFIG_VDSL
		printf("                      DSL Settings             \n");
		#else
		printf("                      ADSL Settings             \n");
		#endif /*CONFIG_VDSL*/
		MSG_LINE;
		showDSLSettings();
		min=1;max=2;
		#ifdef CONFIG_VDSL
		if(getInputUint("Select index (1) Set DSL Settings (2) Quit:",&sel,&min,&max)==0)
		#else
		if(getInputUint("Select index (1) Set ADSL Settings (2) Quit:",&sel,&min,&max)==0)
		#endif /*CONFIG_VDSL*/
		{
			printf("Invalid selection!\n");
			return;
		}
		switch(sel){
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				changeADSLSettings();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2:
				return;
		}
	}
}
#endif // of CONFIG_DEV_xDSL

#ifdef CONFIG_USER_PPPOMODEM
extern const char *ppp_auth[];
static void set3GSettings_show()
{
	MIB_WAN_3G_T entry,*p;

	printf("\n");
	MENU_LINE;

	p=&entry;
	if(!mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p))
	{
		printf( "internal error: mib_chain_get() failed\n");
		printWaitStr();
		return;
	}

	printf( "3G WAN          : %s\n", p->enable?"Enable":"Disable" );
	printf( "PIN Code        : " );
		if(p->pin==NO_PINCODE)
			printf("\n");
		else
			printf("%04u\n", p->pin);
	printf( "APN             : %s\n", p->apn );
	printf( "Dial Number     : %s\n", p->dial );
	printf( "Authentication  : %s\n", ppp_auth[p->auth] );
	printf( "User Name       : %s\n", p->username );
	printf( "Password        : %s\n", p->password );
	printf( "Connection Type : " );
		switch(p->ctype)
		{
		case CONTINUOUS:
			printf( "Continuous\n" );
			break;
		case CONNECT_ON_DEMAND:
			printf( "Connect on Demand\n" );
			break;
		case MANUAL:
			printf( "Manual\n" );
			break;
		}
	printf( "Idle Time(min)  : %u\n", p->idletime );
	printf( "NAPT            : %s\n", p->napt?"Enable":"Disable" );
	printf( "Default Route   : %s\n", p->dgw?"Enable":"Disable" );
	printf( "MTU             : %u\n", p->mtu );
	//paula, 3g backup PPP
	printf( "Backup for ADSL  : %s\n", p->backup?"Enable":"Disable" );
	printf( "Backup Timer (sec): %u\n", p->backup_timer );

	printWaitStr();
	return;
}

static void set3GSettings_modify()
{
	unsigned int num,min,max;
	MIB_WAN_3G_T entry,*p;
	unsigned char tmpbuf[80];
	MIB_CE_ATM_VC_T vc_Entry;

	p=&entry;
	if(!mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p))
	{
		printf( "internal error: mib_chain_get() failed\n");
		goto set3GSet_modify_error;
	}

	printf("\n");
	MENU_LINE;

	/*enable*/
	min=1;max=2;
	if( getInputUint("Set 3G WAN (1)Disable (2)Enable : ", &num, &min, &max)==0 ) return;
	p->enable=(num==1)?0:1;
	if(p->enable==0) goto set3GSet_modify_done;

	/*pin*/
	if( getInputString( "Set PIN Code : ", tmpbuf, sizeof(tmpbuf) ) == 0)
	{
		//error, but may be an empty string
		if( tmpbuf[0]==0 )
		{
			p->pin = NO_PINCODE;
		}else{
			printf( "Invalid PIN Code value!\n");
			goto set3GSet_modify_error;
		}
	}else{
		if( strlen(tmpbuf)==4 &&
		    isdigit( tmpbuf[0] ) &&
		    isdigit( tmpbuf[1] ) &&
		    isdigit( tmpbuf[2] ) &&
		    isdigit( tmpbuf[3] ) )
		{
			p->pin= (unsigned short) atoi( tmpbuf );
		}else{
			printf( "Invalid PIN Code value!\n");
			goto set3GSet_modify_error;
		}
	}

	/*apn*/
	if( getInputString( "Set APN : ", tmpbuf, sizeof(tmpbuf) ) == 0)
	{
		//error, but may be an empty string
		if( tmpbuf[0]!=0 )
		{
			printf( "Invalid APN value!\n");
			goto set3GSet_modify_error;
		}
		p->apn[0]=0;
	}else{
		if( isStrIncludedSpace(tmpbuf) || strlen(tmpbuf)>=sizeof(p->apn) )
		{
			printf( "Invalid APN value! APN must be a string without any space.\n");
			goto set3GSet_modify_error;
		}
		strcpy( p->apn, tmpbuf );
	}

	/*dial*/
	if( getInputString( "Set Dial Number : ", tmpbuf, sizeof(tmpbuf) ) == 0)
	{
		printf( "Invalid Dial Number value!\n");
		goto set3GSet_modify_error;
	}else{
		if( isPhoneNumber(tmpbuf)==0 || strlen(tmpbuf)>=sizeof(p->dial) )
		{
			printf( "Invalid Dial Number value! Dial Number must be a phone number[0-9*#].\n");
			goto set3GSet_modify_error;
		}
		strcpy( p->dial, tmpbuf );
	}

	/*auth*/
	min=1;max=4;
	if( getInputUint("Set Authentication (1)AUTO (2)PAP (3)CHAP (4)NONE : ", &num, &min, &max)==0 ) return;
	p->auth=(unsigned char)(num-1);
	if(p->auth!=PPP_AUTH_NONE)
	{
		/*username*/
		if( getInputString( "Set User Name : ", tmpbuf, sizeof(tmpbuf) )==0 ||
		    isStrIncludedSpace(tmpbuf) ||
		    strlen(tmpbuf)>=sizeof(p->username) )
		{
			printf( "Invalid User Name.\n");
			goto set3GSet_modify_error;
		}
		strcpy( p->username, tmpbuf );

		/*password*/
		if( getInputString( "Set Password : ", tmpbuf, sizeof(tmpbuf) )==0 ||
		    isStrIncludedSpace(tmpbuf) ||
		    strlen(tmpbuf)>=sizeof(p->password) )
		{
			printf( "Invalid Password.\n");
			goto set3GSet_modify_error;
		}
		strcpy( p->password, tmpbuf );
	}

	//paula, 3g backup
	/*backup for adsl*/
	min=1;max=2;
	if( getInputUint("Backup for ADSL (1)Disable (2)Enable : ", &num, &min, &max)==0 ) return;
	p->backup=(num==1)?0:1;


	if(p->backup==1)
	{
		/*backup timer*/
		min=0;max=600;
		if( getInputUint("Backup Timer (sec) : ", &num, &min, &max)==0 ) return;
		p->backup_timer=(unsigned short)num;
		/*ctype, backup for adsl, default Continous*/
		p->ctype=0;
	}
	else
	{
		/*ctype*/
		min=1;max=3;
		if( getInputUint("Set Connection Type (1)Continuous (2)Connect on Demand (3)Manual : ", &num, &min, &max)==0 ) return;
		p->ctype=(unsigned char)(num-1);
		if(p->ctype==CONNECT_ON_DEMAND)
		{
			/*idletime*/
			min=1;max=65535;
			if( getInputUint("Set Idle Time(min) : ", &num, &min, &max)==0 ) return;
			p->idletime=(unsigned short)num;
		}
	}

	/*napt*/
	min=1;max=2;
	if( getInputUint("Set NAPT (1)Disable (2)Enable : ", &num, &min, &max)==0 ) return;
	p->napt=(num==1)?0:1;

	/*dgw*/
	if(p->backup==0)
	{
		min=1;max=2;
		if( getInputUint("Set Default Route (1)Disable (2)Enable : ", &num, &min, &max)==0 ) return;
		p->dgw=(num==1)?0:1;
	}
	else /*dgw, backup for adsl, default route disable*/
		p->dgw=0;

	/*mtu*/
	min=65;max=1500;
	if( getInputUint("Set MTU : ", &num, &min, &max)==0 ) return;
	p->mtu=(unsigned short)num;

set3GSet_modify_done:
#ifdef APPLY_CHANGE
	//wan3g_stop();
	vc_Entry.ifIndex = TO_IFINDEX(MEDIA_3G,  MODEM_PPPIDX_FROM, 0);
	deleteConnection(CONFIGONE, &vc_Entry);
#endif //APPLY_CHANGE
	if(!mib_chain_update( MIB_WAN_3G_TBL, (void*)p, 0))
	{
		printf( "internal error: mib_chain_update() failed\n");
		goto set3GSet_modify_error;
	}
#ifdef APPLY_CHANGE
	//wan3g_start();
	restartWAN(CONFIGONE, &vc_Entry);
#endif //APPLY_CHANGE

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	//debug
	//printWaitStr();
	return;

set3GSet_modify_error:
	printWaitStr();
	return;
}

static void set3GSettings()
{
	int snum;

	while (1)
   	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                      3G Settings             \n");
		MENU_LINE;
		printf("This page is used to configure the parameters for your 3G\n");
		printf("network access.\n");
		MENU_LINE;
		cli_show_menu( strSet3GSettings, SET_3G_SETTINGS_END );
		if (!getInputOption( &snum, 1, SET_3G_SETTINGS_END))
			continue;

		switch(snum-1)
		{
		case SET_3G_SETTINGS_SHOW:
			set3GSettings_show();
			break;
		case SET_3G_SETTINGS_MODIFY:
			set3GSettings_modify();
			break;
		case SET_3G_SETTINGS_QUIT:
			return;
		default:
			break;
		}
	}
}
#endif //CONFIG_USER_PPPOMODEM

void setWANInterface()
{
	int snum;
	int wanmode = GetWanMode();

	while (1)
	{
		int i = 0, menu_size;

		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                           (4) WAN Interface Menu                         \n");
		MENU_LINE;
		menu_size = cli_show_dyn_menu(WANMenu);

		if (!getInputOption(&snum, 1, menu_size))
			continue;

		switch(wan_menu_idx[snum-1])
		{
		case SET_WAN_ITF_WAN_MODE: // WAN Mode
			setWANMode();
			break;
#ifdef CONFIG_ETHWAN
		case SET_WAN_ITF_ETH_CONFIG: // Ethernet Config
			setEthWanConfig(MEDIA_ETH);
			break;
#endif
#ifdef CONFIG_PTMWAN
		case SET_WAN_ITF_PTM_CONFIG: // PTM Config
			setEthWanConfig(MEDIA_PTM);
			break;
#endif /*CONFIG_PTMWAN*/
#ifdef CONFIG_DEV_xDSL
		case SET_WAN_ITF_DSL_CONFIG: // ATM Config
			setDSLConfig();
			break;
#endif
		case SET_WAN_ITF_ATM_SETTINGS: // ATM Settings
			setATMSettings();
			break;
		case SET_WAN_ITF_ADSL_SETTINGS: // DSL Settings
#ifdef CONFIG_DEV_xDSL
			setADSLSettings();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
#ifdef CONFIG_USER_PPPOMODEM
		case SET_WAN_ITF_3G_SETTINGS:
			set3GSettings();
			break;
#endif //CONFIG_USER_PPPOMODEM
		case SET_WAN_ITF_QUIT://(4) Quit
			return;
		default:
			printf("!! Invalid Selection !!\n");
		}//end switch, WAN Interface Menu
	}//end while, WAN Interface Menu
}
/*************************************************************************************************/

#ifdef CONFIG_USER_DHCP_SERVER
void setDHCPMode(){
	int snum;
	DHCP_TYPE_T dtmode;
	unsigned char vChar;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Mode Configuration                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Use this page to set and configure the Dynamic Host Protocol mode for your device. With DHCP, IP  \n");
		printf("addresses for your LAN are administered and distributed as needed by this device or an ISP device.  \n");
		printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_DHCP_MODE, strbuf);
		printf("DHCP Mode: %s\n",strbuf);
		printf("(1) Set                          (2) Quit\n\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set DHCP Mode
				if (!check_access(SECURITY_SUPERUSER))
					break;

				printf("Set DHCP Mode 0:None, 1:DHCP Relay, 2:DHCP Server\n");
				if (!getInputOption( &snum, 0, 2))
					break;
				if ( snum==0 )
					dtmode = DHCP_LAN_NONE;
				else if ( snum == 1 )
					dtmode = DHCP_LAN_RELAY;
				else if ( snum == 2 )
					dtmode = DHCP_LAN_SERVER;
				else {
					printf("Invalid selection\n");
					break;
				}

				vChar = (unsigned char) dtmode;
				if ( mib_set(MIB_DHCP_MODE, (void *)&vChar) == 0) {
					printf("Set DHCP Mode error!\n");
				}
				else
				{
#ifdef APPLY_CHANGE
					// Take effect in real time
					restart_dhcp();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

			case 2://(2) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (7) Diagnostic Menu
	}
}

void addMacBase()
{
	MIB_CE_MAC_BASE_DHCP_T entry, Entry;
	int mibtotal, i, intVal;
	unsigned char macAddr[MAC_ADDR_LEN], mac[17], ip[15];
	struct in_addr assignedIP;
	unsigned long v1, v2;

	memset(&entry, 0x00, sizeof(entry));
	if (getInputString( "Host MAC Address(xx-xx-xx-xx-xx-xx): ", mac, sizeof(mac)+2) == 0)
		 return;
	//strcpy(entry.macAddr, mac);
	for (i=0; i<17; i++) {
		if ((i+1)%3 != 0)
			mac[i-(i+1)/3] = mac[i];
	}
	mac[12] = '\0';
	if (strlen(mac) != 12  || !string_to_hex(mac, entry.macAddr_Dhcp, 12) || !isValidMacAddr(entry.macAddr_Dhcp)) {
		printf("Error! Invalid MAC address.");
		goto setErr_mac;
	}

	if (getInputString( "Assigned IP Address(xxx.xxx.xxx.xxx): ", ip, sizeof(ip)+2) == 0)
		return;
	//strcpy(entry.ipAddr, ip);
	if (!inet_aton(ip, (struct in_addr *)&entry.ipAddr_Dhcp ) || !isValidIpAddr(ip)) {
		printf("Invalid IP address %s.\n", ip);
		goto setErr_mac;
	}

	mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<mibtotal; i++) {
		mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void*)&Entry);
		v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
		v2 = *((unsigned long *)entry.ipAddr_Dhcp);

		if ( (	Entry.macAddr_Dhcp[0]==entry.macAddr_Dhcp[0] && Entry.macAddr_Dhcp[1]==entry.macAddr_Dhcp[1] && Entry.macAddr_Dhcp[2]==entry.macAddr_Dhcp[2] &&
	     		Entry.macAddr_Dhcp[3]==entry.macAddr_Dhcp[3] && Entry.macAddr_Dhcp[4]==entry.macAddr_Dhcp[4] && Entry.macAddr_Dhcp[5]==entry.macAddr_Dhcp[5] ) || (v1==v2)  ) {

			printf("This static ip configuration already exists!");
			goto setErr_mac;
		}
	}

	intVal = mib_chain_add(MIB_MAC_BASE_DHCP_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		goto setErr_mac;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		goto setErr_mac;
	}
	return;
setErr_mac:
	printWaitStr();
	return;
}

void showMACTable()
{
	unsigned int entryNum, i;
	MIB_CE_MAC_BASE_DHCP_T Entry;
	char macaddr[20];

	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	CLEAR;
	printf("\n");
	printf("MAC-Based Assignment Table:\n");
	printf("Idx  Host MAC Address         Assigned IP Address\n");
	printf("-------------------------------------------------------------------------\n");
	if (!entryNum)
		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
				Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
				Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);

		printf("%-5d%-25s%s\n", i+1, macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp)) );
	}
}

void setMacBase()
{
	int snum, num, i, intVal;
	unsigned int min, max;

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("			Static IP Assignment Table							  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the static IP base on MAC Address. \n");
		printf("You can assign/delete the static IP. The Host MAC Address, please \n");
		printf("input a string with hex number. Such as \"00-d0-59-c6-12-43\". \n");
		printf("The Assigned IP Address, please input a string with digit. Such as \n");
		printf("\"192.168.1.100\". \n");
		printf("-------------------------------------------------------------------------\n");

		printf("(1) Add                (2) Delete\n");
		printf("(3) Show               (4) Quit\n");
		if (!getInputOption(&snum, 1, 4))
			continue;

		switch (snum) {
			case 1://(1) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				addMacBase();
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 2://(2) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				num = mib_chain_total(MIB_MAC_BASE_DHCP_TBL); /* get chain record size */
				if (num == 0) {
					printf("Empty table!\n");
					printWaitStr();
					continue;
				}
				showMACTable();
				min=1; max=num;
				if (getInputUint("Select the index to delete:", &intVal, &min, &max) == 0) {
					printf("Error selection!\n");
					printWaitStr();
					continue;
				}
				if(mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, intVal-1) != 1) {
					printf("Delete chain record error!");
					printWaitStr();
				}
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 3://(3)Show
				showMACTable();
				printWaitStr();
				break;

			case 4://(4) Quit
				return;
		}
	}
}

/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#ifdef IMAGENIO_IPTV_SUPPORT //IP_BASED_CLIENT_TYPE
void showDeviceTable()
{
	unsigned int entryNum, i;
	DHCPS_SERVING_POOL_T Entry;
	MIB_CE_DHCP_OPTION_T rsvOptEntry;
	char macaddr[20];
	char devType[16];
	struct in_addr opchAddr;
	unsigned short opchport;
	char optionStr[DHCP_OPT_VAL_LEN];
	int id=-1;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
	CLEAR;
	printf("			Device IP Range Table\n");
	printf("---------------------------------------------------------------------------------------\n");
	printf("This table shows the IP range and option 60 for different device.\n");
	printf("---------------------------------------------------------------------------------------\n");
	printf("Idx   DevName  DevType  Start Address  End Address    Option 60      Rsvd OptCode   Rsvd OptStr\n");
	printf("---------------------------------------------------------------------------------------\n");
	if (!entryNum)
		printf("No data!\n\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		switch(Entry.deviceType)
		{
		case CTC_Computer:
			strcpy(devType,"PC");
			break;
		case CTC_Camera:
			strcpy(devType,"Camera");
			break;
		case CTC_HGW:
			strcpy(devType,"HGW");
			break;
		case CTC_STB:
			strcpy(devType,"STB");
			break;
		case CTC_PHONE:
			strcpy(devType,"Phone");
			break;
		case CTC_UNKNOWN:
		default:
			strcpy(devType,"Unknown");
			break;
		}
		getSPDHCPRsvOptEntryByCode(Entry.InstanceNum, Entry.rsvOptCode, &rsvOptEntry, &id);
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
		if(Entry.deviceType == CTC_STB) {
			mib_get(MIB_OPCH_ADDRESS, (void *)&opchAddr);
			mib_get(MIB_OPCH_PORT, (void *)&opchport);
			sprintf(optionStr, ":::::%s:%d", inet_ntoa(opchAddr), opchport);
		}else
#endif
/*ping_zhang:20090930 END*/
		if(Entry.deviceType == CTC_UNKNOWN) {
			strcpy(optionStr,"");
		}else {
			if(id!=-1)
				strcpy(optionStr,rsvOptEntry.value);
			else
				strcpy(optionStr,"");
		}

		printf("%-6d%-9s%-9s%-15s",i+1,Entry.poolname, devType,inet_ntoa(*((struct in_addr *)Entry.startaddr)));
		printf("%-15s%-15soption %-8d%-15s\n",inet_ntoa(*((struct in_addr *)Entry.endaddr)),Entry.vendorclass,Entry.rsvOptCode,optionStr);
	}
}

void setDeviceTable()
{
	int snum, num, i, intVal;
	unsigned int min, max,isNotValid=0;
	unsigned char reservedOptCode;
	struct in_addr stbDns1,stbDns2,opchIP;
	unsigned char opchAddr[15];
	unsigned short opchport;
	char strbuf[256];

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("			Device IP Range Configuration							  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the IP address range based on device type.  \n");
		printf("You can add/delete the special device type with different IP range and different \n");
		printf("DHCP option 60.\n");
		printf("For example: For pc, the IP range is 192.168.1.40-192.168.1.49, and the value of \n");
		printf("option 60 is \"MSFT\".\n");
		printf("-------------------------------------------------------------------------\n");

/*ping_zhang:20090316 START:Cli for telefonica DHCP option new request*/
		printf("CONF to STBs:\n");
		printf("  STB-DNS1:%s\n",getMibInfo(MIB_IMAGENIO_DNS1));
		printf("  STB-DNS2:%s\n",getMibInfo(MIB_IMAGENIO_DNS2));
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
		printf("  OPCH-Address:%s\n",getMibInfo(MIB_OPCH_ADDRESS));
		printf("  OPCH-Port:%s\n",getMibInfo(MIB_OPCH_PORT));
#endif
/*ping_zhang:20090930 END*/
/*ping_zhang:20090316 END*/

		printf("(1) Add                (2) Modify\n");
		printf("(3) Delete             (4) Show\n");
		printf("(5) Quit\n");
		if (!getInputOption(&snum, 1, 5))
			continue;

		switch (snum) {
			case 1://(1) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				{
					unsigned char ipStart[15],ipEnd[15],buf[256];
					struct in_addr inPoolStart, inPoolEnd,deviceStart,deviceEnd;
					DHCPS_SERVING_POOL_T Entry,rangeEntry;
					MIB_CE_DHCP_OPTION_T optEntry;
					unsigned int i,entryNum;

					initSPDHCPOptEntry(&rangeEntry);

					if (0 == getInputString("Device Name: ", strbuf, sizeof(strbuf)-1))
						continue;
					strcpy(rangeEntry.poolname,strbuf);

					min=1;max=6;
					if (0 == getInputUint("Device Type: (1)PC (2)Carema (3)HGW (4)STB (5)Phone (6)Unknown: ", &num , &min, &max))
						continue;
					if(num==6) //unknown dev type
						rangeEntry.deviceType = CTC_UNKNOWN;
					else
						rangeEntry.deviceType = num - 1;//CTC_Computer, CTC_Camera,.....

					if (getInputString( "Start IP Address: ", ipStart, sizeof(ipStart)+2) == 0)
						continue;
					/*check if device start ip is greater than DHCP pool Start IP*/
#ifdef DHCPS_POOL_COMPLETE_IP
					getMIB2Str(MIB_DHCP_POOL_START, buf);
#else
					int ipadd1=0, ipadd2=0, ipadd3=0, ipadd4=0;
					GetDhcpPrefix( ipadd1,ipadd2,ipadd3,ipadd4 );
					getMIB2Str(MIB_ADSL_LAN_CLIENT_START, buf);
					sscanf( buf,"%d", &ipadd4 );
					sprintf( buf, "%d.%d.%d.%d", ipadd1, ipadd2, ipadd3, ipadd4 );
#endif //DHCPS_POOL_COMPLETE_IP
					if(!inet_aton(buf, &inPoolStart))
						continue;
					if (!inet_aton(ipStart, &deviceStart ) || !isValidIpAddr(ipStart)) {
						printf("Invalid IP address %s.\n", ipStart);
						printWaitStr();
						continue;
					}
					if (deviceStart.s_addr < inPoolStart.s_addr){
						printf("Device IP range should in DHCP pool range!\n");
						printWaitStr();
						continue;
					}
					inet_aton(ipStart, (struct in_addr *)&rangeEntry.startaddr);

					if (getInputString( "End IP Address: ", ipEnd, sizeof(ipEnd)+2) == 0)
						continue;
					/*check if device end ip is less than DHCP pool End IP*/
#ifdef DHCPS_POOL_COMPLETE_IP
					getMIB2Str(MIB_DHCP_POOL_END, buf);
#else
					int ipadd1=0, ipadd2=0, ipadd3=0, ipadd4=0;
					GetDhcpPrefix( ipadd1,ipadd2,ipadd3,ipadd4 );
					getMIB2Str(MIB_ADSL_LAN_CLIENT_END, buf);
					sscanf( buf,"%d", &ipadd4 );
					sprintf( buf, "%d.%d.%d.%d", ipadd1, ipadd2, ipadd3, ipadd4 );
#endif //DHCPS_POOL_COMPLETE_IP
					if(!inet_aton(buf, &inPoolEnd))
						continue;
					if (!inet_aton(ipEnd, &deviceEnd ) || !isValidIpAddr(ipEnd)) {
						printf("Invalid IP address %s.\n", ipEnd);
						printWaitStr();
						continue;
					}
					if (deviceEnd.s_addr > inPoolEnd.s_addr){
						printf("Device IP range should in DHCP pool range!\n");
						printWaitStr();
						continue;
					}
					if (deviceStart.s_addr >= deviceEnd.s_addr) {
						printf("Invalid Device IP range!\n");
						printWaitStr();
						continue;
					}
					inet_aton(ipEnd, (struct in_addr *)&rangeEntry.endaddr);

					if (0 == getInputString("Options 60: ", strbuf, sizeof(strbuf)-1))
						continue;
					strcpy(rangeEntry.vendorclass, strbuf);

					if(rangeEntry.deviceType == CTC_STB)
					{
						printf("Reserved Option Code: option 240\n");
						printf("Conf to STB:\n");
						if (0==getInputIpAddr(" STB-DNS1: ", &stbDns1))
							continue;

						if (0==getInputIpAddr(" STB-DNS2: ", &stbDns2))
							continue;

						if (getInputString( " OPCH-Address: ", opchAddr, sizeof(opchAddr)+2) == 0)
							continue;
						if(!inet_aton(opchAddr, &opchIP))
							continue;

						min = 1; max=65535;
						if(0 == getInputUint(" OPCH-Port: ", &intVal, &min, &max))
							continue;
						opchport = (unsigned short)intVal;
						rangeEntry.rsvOptCode = DHCP_OPT_240;
					}
					else if(rangeEntry.deviceType == CTC_UNKNOWN)
					{
						rangeEntry.rsvOptCode = DHCP_END;
					}
					else
					{
						memset( &optEntry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
						optEntry.enable = 1;
						optEntry.usedFor = eUsedFor_DHCPServer_ServingPool;
						optEntry.dhcpConSPInstNum = rangeEntry.InstanceNum;
						optEntry.dhcpOptInstNum = 1;

						min = 241; max=245;
						if(0 == getInputUint("Reserved Option Code(241-245): ", &intVal, &min, &max))
							continue;
						rangeEntry.rsvOptCode = (unsigned char)intVal;
						optEntry.tag = (unsigned char)intVal;

						if (getInputString( "Reserved Option Str: ", strbuf, sizeof(strbuf)-1) == 0)
							continue;
						strcpy(optEntry.value, strbuf);
						optEntry.len = strlen(optEntry.value);
					}

					/*check if the entry is valid*/
					entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
					for (i=0; i<entryNum; i++) {
						if (!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&Entry))
				  			continue;
						if(strcmp(Entry.poolname,rangeEntry.poolname)==0 ) {
							isNotValid = 1;
							break;
						}
					}

					if(isNotValid == 1)
					{
						printf("Device %s is already exist in table!\n",rangeEntry.poolname);
						printWaitStr();
						continue;
					}

					if (!mib_chain_add(MIB_DHCPS_SERVING_POOL_TBL, (void *)&rangeEntry) )
					{
						printf("Error! Add chain record.");
						printWaitStr();
						continue;
					} else
						if( (rangeEntry.deviceType!=CTC_STB)&&(rangeEntry.deviceType!=CTC_UNKNOWN) )
						{
							if( !mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (void*)&optEntry) )
							{
								printf("Error! Add chain record.");
								printWaitStr();
								continue;
							}
						}

						if(rangeEntry.deviceType == CTC_STB) {
							mib_set(MIB_IMAGENIO_DNS1, (void *)&stbDns1);
							mib_set(MIB_IMAGENIO_DNS2, (void *)&stbDns2);
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
							mib_set(MIB_OPCH_ADDRESS, (void *)&opchIP);
							mib_set(MIB_OPCH_PORT, (void *)&opchport);
#endif
/*ping_zhang:20090930 END*/
						}
#if defined(APPLY_CHANGE)
						// Take effect in real time
						restart_dhcp();
#endif
#ifdef COMMIT_IMMEDIATELY
						Commit();
#endif

				}
				break;
			case 2://(2) Modify
				if (!check_access(SECURITY_SUPERUSER))
					break;
				{
					unsigned char ipStart[15],ipEnd[15],buf[256];
					struct in_addr inPoolStart, inPoolEnd,deviceStart,deviceEnd;
					DHCPS_SERVING_POOL_T Entry,rangeEntry;
					MIB_CE_DHCP_OPTION_T optEntry;
					unsigned int i,entryNum,modifyIndex;

					showDeviceTable();

					min = 1; max = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
					if(0 == getInputUint("Select the index to modify:", &intVal, &min, &max))
						continue;
					modifyIndex = intVal - 1;

					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, modifyIndex, (void*)&rangeEntry ) ) {
						printf("Get chain record error!\n");
						continue;
					}

					if (0 == getInputString("Device Name: ", strbuf, sizeof(strbuf)-1))
						continue;
					strcpy(rangeEntry.poolname,strbuf);

					min=1;max=6;
					if (0 == getInputUint("Device Type: (1)PC (2)Carema (3)HGW (4)STB (5)Phone (6)Unknown: ", &num , &min, &max))
						continue;
					if(num==6) //unknown dev type
						rangeEntry.deviceType = CTC_UNKNOWN;
					else
						rangeEntry.deviceType = num - 1;//CTC_Computer, CTC_Camera,.....

					if (getInputString( "Start IP Address: ", ipStart, sizeof(ipStart)+2) == 0)
						continue;
					/*check if device start ip is greater than DHCP pool Start IP*/
#ifdef DHCPS_POOL_COMPLETE_IP
					getMIB2Str(MIB_DHCP_POOL_START, buf);
#else
					int ipadd1=0, ipadd2=0, ipadd3=0, ipadd4=0;
					GetDhcpPrefix( ipadd1,ipadd2,ipadd3,ipadd4 );
					getMIB2Str(MIB_ADSL_LAN_CLIENT_START, buf);
					sscanf( buf,"%d", &ipadd4 );
					sprintf( buf, "%d.%d.%d.%d", ipadd1, ipadd2, ipadd3, ipadd4 );
#endif //DHCPS_POOL_COMPLETE_IP
					if(!inet_aton(buf, &inPoolStart))
						continue;
					if (!inet_aton(ipStart, &deviceStart ) || !isValidIpAddr(ipStart)) {
						printf("Invalid IP address %s.\n", ipStart);
						printWaitStr();
						continue;
					}
					if (deviceStart.s_addr < inPoolStart.s_addr){
						printf("Device IP range should in DHCP pool range!\n");
						printWaitStr();
						continue;
					}
					inet_aton(ipStart, (struct in_addr *)&rangeEntry.startaddr);

					if (getInputString( "End IP Address: ", ipEnd, sizeof(ipEnd)+2) == 0)
						continue;
					/*check if device end ip is less than DHCP pool End IP*/
#ifdef DHCPS_POOL_COMPLETE_IP
					getMIB2Str(MIB_DHCP_POOL_END, buf);
#else
					int ipadd1=0, ipadd2=0, ipadd3=0, ipadd4=0;
					GetDhcpPrefix( ipadd1,ipadd2,ipadd3,ipadd4 );
					getMIB2Str(MIB_ADSL_LAN_CLIENT_END, buf);
					sscanf( buf,"%d", &ipadd4 );
					sprintf( buf, "%d.%d.%d.%d", ipadd1, ipadd2, ipadd3, ipadd4 );
#endif //DHCPS_POOL_COMPLETE_IP
					if(!inet_aton(buf, &inPoolEnd))
						continue;
					if (!inet_aton(ipEnd, &deviceEnd ) || !isValidIpAddr(ipEnd)) {
						printf("Invalid IP address %s.\n", ipEnd);
						printWaitStr();
						continue;
					}
					if (deviceEnd.s_addr > inPoolEnd.s_addr){
						printf("Device IP range should in DHCP pool range!\n");
						printWaitStr();
						continue;
					}
					if (deviceStart.s_addr >= deviceEnd.s_addr) {
						printf("Invalid Device IP range!\n");
						printWaitStr();
						continue;
					}
					inet_aton(ipEnd, (struct in_addr *)&rangeEntry.endaddr);

					if (0 == getInputString("Options 60: ", strbuf, sizeof(strbuf)-1))
						continue;
					strcpy(rangeEntry.vendorclass, strbuf);

					//clear option table of the server pool
					clearOptTbl(rangeEntry.InstanceNum);
					if(rangeEntry.deviceType == CTC_STB)
					{
						printf("Reserved Option Code: option 240\n");
						printf("Conf to STB:\n");
						if (0==getInputIpAddr(" STB-DNS1: ", &stbDns1))
							continue;

						if (0==getInputIpAddr(" STB-DNS2: ", &stbDns2))
							continue;

						if (getInputString( " OPCH-Address: ", opchAddr, sizeof(opchAddr)+2) == 0)
							continue;
						if(!inet_aton(opchAddr, &opchIP))
							continue;

						min = 1; max=65535;
						if(0 == getInputUint(" OPCH-Port: ", &intVal, &min, &max))
							continue;
						opchport = (unsigned short)intVal;
						rangeEntry.rsvOptCode = DHCP_OPT_240;
					}
					else if(rangeEntry.deviceType == CTC_UNKNOWN)
					{
						rangeEntry.rsvOptCode = DHCP_END;
					}
					else
					{
						memset( &optEntry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
						optEntry.enable = 1;
						optEntry.usedFor = eUsedFor_DHCPServer_ServingPool;
						optEntry.dhcpConSPInstNum = rangeEntry.InstanceNum;
						optEntry.dhcpOptInstNum = 1;

						min = 241; max=245;
						if(0 == getInputUint("Reserved Option Code(241-245): ", &intVal, &min, &max))
							continue;
						rangeEntry.rsvOptCode = (unsigned char)intVal;
						optEntry.tag = (unsigned char)intVal;

						if (getInputString( "Reserved Option Str: ", strbuf, sizeof(strbuf)-1) == 0)
							continue;
						strcpy(optEntry.value, strbuf);
						optEntry.len = strlen(optEntry.value);
					}

					/*check if the entry is valid*/
					entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
					for (i=0; i<entryNum; i++) {
						if(i == modifyIndex) continue;
						if (!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&Entry))
				  			continue;
						if(strcmp(Entry.poolname,rangeEntry.poolname)==0 ) {
							isNotValid = 1;
							break;
						}
					}

					if(isNotValid == 1)
					{
						printf("Device %s is already exist in table!\n",rangeEntry.poolname);
						printWaitStr();
						continue;
					}

					if (!mib_chain_update(MIB_DHCPS_SERVING_POOL_TBL, (void *)&rangeEntry,modifyIndex))
					{
						printf("Error! Update chain record.");
						printWaitStr();
						continue;
					} else
						if( (rangeEntry.deviceType!=CTC_STB)&&(rangeEntry.deviceType!=CTC_UNKNOWN) )
						{
							if( !mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (void*)&optEntry) )
							{
								printf("Error! Update chain record.");
								printWaitStr();
								continue;
							}
						}


						if(rangeEntry.deviceType == CTC_STB) {
							mib_set(MIB_IMAGENIO_DNS1, (void *)&stbDns1);
							mib_set(MIB_IMAGENIO_DNS2, (void *)&stbDns2);
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
							mib_set(MIB_OPCH_ADDRESS, (void *)&opchIP);
							mib_set(MIB_OPCH_PORT, (void *)&opchport);
#endif
/*ping_zhang:20090930 END*/
						}
#if defined(APPLY_CHANGE)
						// Take effect in real time
						restart_dhcp();
#endif
#ifdef COMMIT_IMMEDIATELY
						Commit();
#endif

				}
				break;
			case 3://(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				num = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL); /* get chain record size */
				if (num == 0) {
					printf("Empty table!\n");
					printWaitStr();
					continue;
				}
				showDeviceTable();
				min=1; max=num;
				if (getInputUint("Select the index to delete:", &intVal, &min, &max) == 0) {
					printf("Error selection!\n");
					printWaitStr();
					continue;
				}

				{
					DHCPS_SERVING_POOL_T rangeEntry;

					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, intVal-1, (void*)&rangeEntry ) ) {
						printf("Get chain record error!\n");
						continue;
					}
					//clear option table of the server pool
					clearOptTbl(rangeEntry.InstanceNum);
				#ifdef SUPPORT_DHCP_RESERVED_IPADDR
					clearDHCPReservedIPAddrByInstNum( rangeEntry.InstanceNum );
				#endif //SUPPORT_DHCP_RESERVED_IPADDR
				}

				if(mib_chain_delete(MIB_DHCPS_SERVING_POOL_TBL, intVal-1) != 1) {
					printf("Delete chain record error!");
					printWaitStr();
				}
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 4://(4)Show
				showDeviceTable();
				printWaitStr();
				break;

			case 5://(5) Quit
				return;
		}
	}
}
#endif
#endif

void setDHCPServer()
{
	//char dhcpRangeStart[16], dhcpRangeEnd[16], dname[32];
	//int snum, dhcp, ltime;
	unsigned int choice,num,min,max;
	unsigned char dhcp, buffer[32], *ip, *mask;
	struct in_addr lanIp, lanMask, inGatewayIp;
	unsigned char ipprefix[16];
#ifdef DHCPS_POOL_COMPLETE_IP
	struct in_addr inPoolStart, inPoolEnd;
#endif
#ifdef DHCPS_DNS_OPTIONS
	char *pDns;
	unsigned char dnsopt;
	struct in_addr dns1, dns2, dns3;
#endif
	char strbuf[256];

// Kaohj
#ifdef CONFIG_SECONDARY_IP
	mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&buffer );
	if (buffer[0])
		mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)buffer);
#else
	buffer[0] = 0;
#endif
	if (buffer[0] == 0) { // primary LAN
	if(!mib_get(MIB_ADSL_LAN_IP, (void *)&lanIp) || !mib_get(MIB_ADSL_LAN_SUBNET, (void *)&lanMask))
		return;
	}
	else { // secondary LAN
		if(!mib_get(MIB_ADSL_LAN_IP2, (void *)&lanIp) || !mib_get(MIB_ADSL_LAN_SUBNET2, (void *)&lanMask))
			return;
	}

// Kaohj
#ifndef DHCPS_POOL_COMPLETE_IP
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, ipprefix);
#endif

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Server                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Enable the DHCP Server if you are using this device as a DHCP server.    \n");
		printf("This page lists the IP address pools available to hosts on your LAN.     \n");
		printf("The device distributes numbers in the pool to hosts on your network as   \n");
		printf("they request Internet access.                                            \n");
		printf("-------------------------------------------------------------------------\n");
		printf("LAN IP Address: %s\t", inet_ntoa(lanIp));
		printf("Subnet Mask: %s\n", inet_ntoa(lanMask));
		getSYS2Str(SYS_LAN_DHCP, strbuf);
		printf("DHCP Server: %s\n", strbuf);
		mib_get( MIB_ADSL_LAN_IP, (void *)buffer);
		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		printf("IP Pool Range: %s%s - ", ipprefix, getMibInfo(MIB_ADSL_LAN_CLIENT_START));
		printf("%s%s\n",  ipprefix, getMibInfo(MIB_ADSL_LAN_CLIENT_END));
		#else
		printf("IP Pool Range: %s - ", getMibInfo(MIB_DHCP_POOL_START));
		printf("%s\n", getMibInfo(MIB_DHCP_POOL_END));
		printf("Subnet Mask: %s\n", getMibInfo(MIB_DHCP_SUBNET_MASK));
		#endif
		printf("Max Lease Time: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_LEASE));
		printf("Domain Name: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_DOMAIN));
		printf("Gateway Address: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_GATEWAY));
		// Kaohj
#ifdef DHCPS_DNS_OPTIONS
		mib_get(MIB_DHCP_DNS_OPTION, (void *)&dnsopt);
		printf("DNS Options: %s\n", dnsopt==0?"Use DNS Relay":"Use Manual Setting");
		if (dnsopt == 1) { // manual setting
			printf("  DNS1: %s\n", getMibInfo(MIB_DHCPS_DNS1));
			pDns = getMibInfo(MIB_DHCPS_DNS2);
			if (!strcmp(pDns, "0.0.0.0"))
				printf("  DNS2: \n");
			else
				printf("  DNS2: %s\n", pDns);
			pDns = getMibInfo(MIB_DHCPS_DNS3);
			if (!strcmp(pDns, "0.0.0.0"))
				printf("  DNS3: \n");
			else
				printf("  DNS3: %s\n", pDns);
		}
#endif
/*ping_zhang:20090310 START:add cli configuration of dhcp options*/
#ifdef IMAGENIO_IPTV_SUPPORT //IP_BASED_CLIENT_TYPE
		printf("(1) Basic Settings         (2) Device IP Range Settings\n");
		printf("(3) Show Client            (4) MAC-Based Assignment\n");
		printf("(5) Quit\n");
 		if (!getInputOption(&choice, 1, 5))
			continue;
#else
		printf("(1) Set                          (2) Show Client\n");
		printf("(3) MAC-Based Assignment          (4) Quit\n");
 		if (!getInputOption(&choice, 1, 4))
			continue;
#endif
/*ping_zhang:20090310 END*/

		switch(choice)
		{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			{
				char prompt[64];
				unsigned char uStart, uEnd;
				unsigned int uLTime;
#ifdef DHCPS_POOL_COMPLETE_IP
				struct in_addr dhcp_mask;
#endif

				ip = (unsigned char *)&lanIp;
				dhcp = DHCP_SERVER;
				// Kaohj
				#ifndef DHCPS_POOL_COMPLETE_IP
				sprintf(prompt, "Start IP: %s", ipprefix);
				min=0;max=255;
				if (0 == getInputUint(prompt, &num, &min, &max))
					continue;
				uStart = (unsigned char)num;

				sprintf(prompt, "End IP: %s", ipprefix);
				min=0;max=255;
				if (0 == getInputUint(prompt, &num, &min, &max))
					continue;
				uEnd = (unsigned char)num;

				if (uEnd <= uStart) {
					printf("Invalid DHCP client range!\n");
					continue;
				}
				#else
				if (0 == getInputIpAddr("Start IP: ", &inPoolStart))
					continue;
				if (0 == getInputIpAddr("End IP: ", &inPoolEnd))
					continue;
				if (inPoolStart.s_addr >= inPoolEnd.s_addr) {
					printf("Invalid DHCP client range!\n");
					printWaitStr();
					continue;
				}
				if (0 == getInputIpMask("Subnet Mask: ", &dhcp_mask))
					continue;
				#endif

				min=0;max=0xffffffff;
				if (0 == getInputUint("Max Lease Time: ", &uLTime, &min, &max))
					continue;

				if (0 == getInputString("Domain Name: ", strbuf, sizeof(strbuf)-1))
					continue;

				// Kaohj
				#ifndef DHCPS_POOL_COMPLETE_IP
				if ( !mib_set(MIB_ADSL_LAN_CLIENT_START, (void *)&uStart)) {
					printf("Set DHCP client start range error!\n");
					return;
				}
				if ( !mib_set(MIB_ADSL_LAN_CLIENT_END, (void *)&uEnd)) {
					printf("Set DHCP client end range error!\n");
					return;
				}
				#else
				mib_set( MIB_DHCP_POOL_START, (void *)&inPoolStart);
				mib_set( MIB_DHCP_POOL_END, (void *)&inPoolEnd);
				mib_set( MIB_DHCP_SUBNET_MASK, (void *)&dhcp_mask);
				#endif
				if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime)) {
					printf("Set DHCP lease time error!\n");
					return;
				}
				if ( !mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)strbuf)) {
					printf("Set DHCP Domain Name error!\n");
					return;
				}

				if (0 == getInputString("Gateway Address: ", strbuf, sizeof(strbuf)-1))
					continue;
				//printf("strbuf=%s\n", strbuf);

				if ( !inet_aton(strbuf, &inGatewayIp) ) {
					printf("CLI: Invalid DHCP Server Gateway Address value!");
					continue;
				}
				if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inGatewayIp)) {
					printf("CLI: Set DHCP Server Gateway Address error!");
					continue;
				}
				// Kaohj
				#ifdef DHCPS_DNS_OPTIONS
				min=1;max=2;
				if (0 == getInputUint("DNS options: (1)Use DNS Relay (2)Set DNS Manually: ", &num , &min, &max))
					continue;
				dnsopt = num-1;
				if (dnsopt == 1) { // set by manual
					dns1.s_addr=INADDR_NONE;
					dns2.s_addr=INADDR_NONE;
					dns3.s_addr=INADDR_NONE;

					if (0==getInputIpAddr("DNS1: ", &dns1))
						continue;
					if (0!=getInputIpAddr("DNS2: ", &dns2))
						getInputIpAddr("DNS3: ", &dns3);
				}
				#endif
				#ifdef DHCPS_DNS_OPTIONS
				mib_set(MIB_DHCP_DNS_OPTION, (void *)&dnsopt);
				mib_set(MIB_DHCPS_DNS1, (void *)&dns1);
				mib_set(MIB_DHCPS_DNS2, (void *)&dns2);
				mib_set(MIB_DHCPS_DNS3, (void *)&dns3);
				#endif
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_dhcp();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
			}

			break;
/*ping_zhang:20090310 START:add cli configuration of dhcp options*/
#ifdef IMAGENIO_IPTV_SUPPORT //IP_BASED_CLIENT_TYPE
		case 2: //Device IP Range Settings
			setDeviceTable();
			break;
		case 3: //Show Client
#else
		case 2: //Show Client
#endif
/*ping_zhang:20090310 END*/
			{
				struct stat status;
				int element=0, ret, pid;
				char ipAddr[40], macAddr[40], liveTime[80], *buf=NULL, *ptr;
				FILE *fp;

				printf("\n");
				printf("Active DHCP Client Table\n");
				printf("--------------------------------------------------------------------------\n");
				printf("This table shows the assigned IP address, MAC address and time expired for \n");
				printf("each DHCP leased client. \n");
				printf("--------------------------------------------------------------------------\n");
				printf("IP Address          MAC Address         Time Expired (sec)\n");
				printf("--------------------------------------------------------------------------\n");
				// siganl DHCP server to update lease file
				pid = read_pid((char *)DHCPSERVERPID);
				if (pid > 0)
					kill(pid, SIGUSR1);
				usleep(1000);
				if (stat(DHCPD_LEASE, &status) < 0)
					goto end_dhcp;

				// read DHCP server lease file
				buf = malloc(status.st_size);
				if (buf == NULL)
					goto end_dhcp;
				fp = fopen(DHCPD_LEASE, "r");
				if (fp == NULL)
					goto end_dhcp;
				fread(buf, 1, status.st_size, fp);
				fclose(fp);

				ptr = buf;
				while (1) {
					ret = getOneDhcpClient(&ptr, &status.st_size, ipAddr, macAddr, liveTime);
					if (ret < 0)
						break;
					if (ret == 0)
						continue;
					printf("%-20s%-20s%s\n", ipAddr, macAddr, liveTime);
					element ++;
				}
				if (!element)
					printf("No data!\n\n");
				end_dhcp:
					if (buf)
						free(buf);
					printWaitStr();
					break;
			}
/*ping_zhang:20090310 START:add cli configuration of dhcp options*/
#ifdef IMAGENIO_IPTV_SUPPORT //IP_BASED_CLIENT_TYPE
		case 4: //(4) MAC-Based Assignment
			setMacBase();
			break;
		case 5://(5) Quit
			return;
#else
		case 3: //(3) MAC-Based Assignment
			setMacBase();
			break;
		case 4://(4) Quit
			return;
#endif
/*ping_zhang:20090310 END*/
		}//end switch, WAN Interface Menu
	}//end while, WAN Interface Menu
}


void setDHCPRelay(){
	int snum;
	DHCP_TYPE_T dtmode;
	unsigned char vChar;
	char str[16];
	struct in_addr dhcps;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Relay Configuration                                  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the DHCP server ip addresses for DHCP Relay. \n");
		printf("-------------------------------------------------------------------------\n");
		printf("DHCP Server Address: %s\n",getMibInfo(MIB_ADSL_WAN_DHCPS));
		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set DHCP Mode

				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (0 == getInputString("DHCP Server Address: ", str, 16))
					break;

				if ( !inet_aton(str, &dhcps) ) {
					printf("Invalid DHCPS address value!\n");
				}

				if ( !mib_set(MIB_ADSL_WAN_DHCPS, (void *)&dhcps)) {
	  				printf("Set DHCPS MIB error!");
				}
				else
				{
#ifdef APPLY_CHANGE
					// Take effect in real time
					restart_dhcp();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

			case 2://(2) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (7) Diagnostic Menu
	}

}


void setDHCPPage(){
	int snum;
	unsigned char vChar;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCP Menu                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) DHCP Mode				(2) DHCP Server\n");
		printf("(3) DHCP Relay				(4) Quit		\n");
		if (!getInputOption( &snum, 1, 4))
			continue;

		switch( snum)
		{
			case 1: // (1) DHCP Mode
				setDHCPMode();
				break;
			case 2: // (2) DHCP Server
				mib_get(MIB_DHCP_MODE, (void *)&vChar);
				if (vChar)	// DHCP enabled
					setDHCPServer();
				else {
					printf("DHCP mode is none!!\n");
					printWaitStr();
				}
				break;
			case 3: // (3) DHCP Relay
				mib_get(MIB_DHCP_MODE, (void *)&vChar);
				if (vChar)	// DHCP enabled
					setDHCPRelay();
				else {
					printf("DHCP mode is none!!\n");
					printWaitStr();
				}
				break;
			case 4: // (4) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}
	}
}
#endif // of CONFIG_USER_DHCP_SERVER

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
void setDHCPv6Mode()
{
	int snum;
	DHCP_TYPE_T dtmode;
	unsigned char vChar;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCPv6 Mode Configuration                     \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Use this page to set and configure the Dynamic Host Protocol mode for your device. With DHCP, IP  \n");
		printf("addresses for your LAN are administered and distributed as needed by this device or an ISP device.  \n");
		printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_DHCPV6_MODE, strbuf);
		printf("DHCPv6 Mode: %s\n",strbuf);
		printf("(1) Set                          (2) Quit\n\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set DHCPv6 Mode
				if (!check_access(SECURITY_SUPERUSER))
					break;

				printf("Set DHCPv6 Mode 0:None, 1:DHCP Relay, 2:DHCP Server\n");
				if (!getInputOption( &snum, 0, 2))
					break;
				if ( snum==0 )
					dtmode = DHCP_LAN_NONE;
				else if ( snum == 1 )
					dtmode = DHCP_LAN_RELAY;
				else if ( snum == 2 )
					dtmode = DHCP_LAN_SERVER;
				else {
					printf("Invalid selection\n");
					break;
				}

				vChar = (unsigned char) dtmode;
				if ( mib_set(MIB_DHCPV6_MODE, (void *)&vChar) == 0) {
					printf("Set DHCPV6 Mode error!\n");
				}
				else
				{
#ifdef APPLY_CHANGE
					// Take effect in real time
					start_dhcpv6(1);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

			case 2://(2) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (7) Diagnostic Menu
	}
}

void setDHCPv6Relay()
{
	int snum;
	unsigned int min,max,idxsel;
	MIB_CE_ATM_VC_T vcEntry;
	unsigned char vChar;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCPv6 Relay Configuration                                  \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the upper interface(server link) for DHCP Relay. \n");
		printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_DHCPV6_RELAY_UPPER_ITF, strbuf);
		printf("The upper interface of DHCPv6 Relay: %s\n",strbuf);
		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set DHCPv6 Relay

				if (!check_access(SECURITY_SUPERUSER))
					break;

				max=showItf(2);
				if(max==0)
					return;

				min=1;
				if(getInputUint("Select index:",&idxsel,&min,&max)==0){
					return;
				}
				mib_chain_get(MIB_ATM_VC_TBL, idxsel-1, (void *)&vcEntry);

				if ( !mib_set(MIB_DHCPV6R_UPPER_IFINDEX, (void *)&(vcEntry.ifIndex))) {
	  				printf("Set MIB_DHCPV6R_UPPER_IFINDEX MIB error!");
				}
				else
				{
#ifdef APPLY_CHANGE
					// Take effect in real time
					mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
					if ( vChar == DHCP_LAN_RELAY)	// DHCPv6 Relay enabled
						start_dhcpv6(1);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

			case 2://(2) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (7) Diagnostic Menu
	}

}

void showDomainSearchTable()
{
	unsigned int entryNum, i;
	MIB_DHCPV6S_DOMAIN_SEARCH_T Entry;

	entryNum = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL);

	CLEAR;
	printf("\n");
	printf("Domain Search Table:\n");
	printf("Idx     Domain\n");
	printf("-------------------------------------------------------------------------\n");
	if (!entryNum)
		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, i, (void *)&Entry)) {
  			printf("showDomainSearchTable: Get chain record error!\n");
			return;
		}
		printf("%-8d%s\n", i+1, Entry.domain);
	}
}

void DomainSearchDel()
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;

	totalEntry = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(MIB_DHCPV6S_DOMAIN_SEARCH_TBL); /* clear chain record */
	else if (del == 1) {
		showDomainSearchTable();
		min=1; max=totalEntry+1;
		getInputUint("Select the index to delete:", &index, &min, &max);
		if (index > totalEntry || index <= 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, index-1) != 1)
			printf("Delete MIB_DHCPV6S_DOMAIN_SEARCH_TBL chain record error!");
	}
}

void showNameServerTable(){
	unsigned int entryNum, i;
	MIB_DHCPV6S_NAME_SERVER_T Entry;

	entryNum = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL);

	CLEAR;
	printf("\n");
	printf("Name Server Table:\n");
	printf("Idx     Name Server\n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DHCPV6S_NAME_SERVER_TBL, i, (void *)&Entry)) {
  			printf("showNameServerTable: Get chain record error!\n");
			return;
		}
		printf("%-8d%s\n", i+1, Entry.nameServer);
	}
}

void NameServerDel()
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;

	totalEntry = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(MIB_DHCPV6S_NAME_SERVER_TBL); /* clear chain record */
	else if (del == 1) {
		showNameServerTable();
		min=1; max=totalEntry+1;
		getInputUint("Select the index to delete:", &index, &min, &max);
		if (index > totalEntry || index <= 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_DHCPV6S_NAME_SERVER_TBL, index-1) != 1)
			printf("Delete MIB_DHCPV6S_NAME_SERVER_TBL chain record error!");
	}
}

void setDHCPv6Server()
{
	unsigned int choice,min,max;
	unsigned char vChar;
	int i, intVal;
	struct in6_addr ip6Addr;
	unsigned int totalEntry;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCPv6 Server                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Enable the DHCP Server if you are using this device as a DHCP server.    \n");
		printf("This page lists the IP address pools available to hosts on your LAN.     \n");
		printf("The device distributes numbers in the pool to hosts on your network as   \n");
		printf("they request Internet access.                                            \n");
		printf("-------------------------------------------------------------------------\n");
		printf("\nThe Basic parameters are as follows.\n");
		printf("IP Pool Range: %s - ", getMibInfo(MIB_DHCPV6S_RANGE_START));
		printf("%s\n", getMibInfo(MIB_DHCPV6S_RANGE_END));
		printf("Prefix Length: %s\n", getMibInfo(MIB_DHCPV6S_PREFIX_LENGTH));
		printf("Valid lifetime: %s seconds\n", getMibInfo(MIB_DHCPV6S_DEFAULT_LEASE));
		printf("Preferred-lifetime: %s seconds\n", getMibInfo(MIB_DHCPV6S_PREFERRED_LIFETIME));
		printf("Renew Time: %s seconds\n", getMibInfo(MIB_DHCPV6S_RENEW_TIME));
		printf("Rebind Time: %s seconds\n", getMibInfo(MIB_DHCPV6S_REBIND_TIME));
		printf("Client DUID: %s\n\n", getMibInfo(MIB_DHCPV6S_CLIENT_DUID));

		printf("(1) Set Basic Parameter          (2) Add Name Server\n");
		printf("(3) Delete Name Server           (4) Show Name Server\n");
		printf("(5) Add Domain Search            (6) Delete Domain Search\n");
		printf("(7) show Domain Search           (8) Quit\n");

 		if (!getInputOption(&choice, 1, 8))
			continue;

		switch(choice)
		{
		case 1://(1) Set Basic Parameter
			if (!check_access(SECURITY_SUPERUSER))
				break;
			{
				unsigned int DLTime, PFTime, RNTime, RBTime;
				unsigned int pLen;

				if (0 == getInputIp6Addr("RangeStart:", &ip6Addr))
					continue;
				mib_set(MIB_DHCPV6S_RANGE_START, (void *)&ip6Addr);

				if (0 == getInputIp6Addr("RangeEnd:", &ip6Addr))
					continue;
				mib_set(MIB_DHCPV6S_RANGE_END, (void *)&ip6Addr);

				min=1; max=128;
				if (0 == getInputUint("Prefix Length: ", &pLen, &min, &max))
					continue;
				vChar = (char)pLen;
				mib_set(MIB_DHCPV6S_PREFIX_LENGTH, (void *)&vChar);

				min=0;max=0xffffffff;
				if (0 == getInputUint("Valid lifetime: ", &DLTime, &min, &max))
					continue;
				if ( !mib_set(MIB_DHCPV6S_DEFAULT_LEASE, (void *)&DLTime)) {
					printf("Set MIB_DHCPV6S_DEFAULT_LEASE error!\n");
					return;
				}

				min=0;max=0xffffffff;
				if (0 == getInputUint("Preferred-lifetime: ", &PFTime, &min, &max))
					continue;
				if ( !mib_set(MIB_DHCPV6S_PREFERRED_LIFETIME, (void *)&PFTime)) {
					printf("Set MIB_DHCPV6S_PREFERRED_LIFETIME error!\n");
					return;
				}

				min=0;max=0xffffffff;
				if (0 == getInputUint("Renew Time: ", &RNTime, &min, &max))
					continue;
				if ( !mib_set(MIB_DHCPV6S_RENEW_TIME, (void *)&RNTime)) {
					printf("Set MIB_DHCPV6S_RENEW_TIME error!\n");
					return;
				}

				min=0;max=0xffffffff;
				if (0 == getInputUint("Rebind Time: ", &RBTime, &min, &max))
					continue;
				if ( !mib_set(MIB_DHCPV6S_REBIND_TIME, (void *)&RBTime)) {
					printf("Set MIB_DHCPV6S_REBIND_TIME error!\n");
					return;
				}

				if (0 == getInputString("Client DUID: ", strbuf, MAX_DUID_LEN))
					continue;
				if ( !mib_set(MIB_DHCPV6S_CLIENT_DUID, (void *)strbuf)) {
					printf("Set MIB_DHCPV6S_CLIENT_DUID error!\n");
					return;
				}

#if defined(APPLY_CHANGE)
				// Take effect in real time
				mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
				if ( vChar == DHCP_LAN_SERVER)	// DHCPv6 Server enabled
					start_dhcpv6(1);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
			}

			break;

		case 2:	//(2) Add Name Server
		{
				MIB_DHCPV6S_NAME_SERVER_T entry;

				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (getInputString("IP of Name Server: ", strbuf, MAX_V6_IP_LEN) == 0)
					break;
				totalEntry = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL);
				for (i = 0 ; i < totalEntry; i++) {
					if (!mib_chain_get(MIB_DHCPV6S_NAME_SERVER_TBL, i, (void *)&entry)) {
						printf("Error: Get MIB_DHCPV6S_NAME_SERVER_TBL fail\n");
						goto setErr_domain;
					}
					if (!strcmp(entry.nameServer, strbuf)){
						printf("This is a duplicate entry on Name Server table.\n");
						goto setErr_domain;
					}
				}
				strcpy(entry.nameServer, strbuf);
				intVal = mib_chain_add(MIB_DHCPV6S_NAME_SERVER_TBL, (unsigned char*)&entry);
				if (intVal == 0) {
					printf("Add Name Server chain record error.\n");
					return;
				}
				else if (intVal == -1) {
					printf("Error! Table Full.");
					goto setErr_domain;
					//return;
				}
		}
#if defined(APPLY_CHANGE)
				// Take effect in real time
				mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
				if ( vChar == DHCP_LAN_SERVER)	// DHCPv6 Server enabled
					start_dhcpv6(1);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

		case 3:	//(3) Delete Name Server
				if (!check_access(SECURITY_SUPERUSER))
					break;
				NameServerDel();
				//printWaitStr();
#if defined(APPLY_CHANGE)
				// Take effect in real time
				mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
				if ( vChar == DHCP_LAN_SERVER)	// DHCPv6 Server enabled
					start_dhcpv6(1);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

		case 4:	//(4)Show Name Server
			showNameServerTable();
			//printWaitStr();
			break;

		case 5:	//(5) Add Domain
		{
				MIB_DHCPV6S_DOMAIN_SEARCH_T entry;

				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (getInputString("Domain: ", strbuf, MAX_DOMAIN_LENGTH) == 0)
					break;
				totalEntry = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL);
				for (i = 0 ; i < totalEntry; i++) {
					if (!mib_chain_get(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, i, (void *)&entry)) {
						printf("Error: Get MIB_DHCPV6S_DOMAIN_SEARCH_TBL fail\n");
						goto setErr_domain;
					}
					if (!strcmp(entry.domain, strbuf)){
						printf("This is a duplicate entry on Domain Search table.\n");
						goto setErr_domain;
					}
				}
				strcpy(entry.domain, strbuf);
				intVal = mib_chain_add(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, (unsigned char*)&entry);
				if (intVal == 0) {
					printf("Add Domain Search chain record error.\n");
					return;
				}
				else if (intVal == -1) {
					printf("Error! Table Full.");
					goto setErr_domain;
					//return;
				}
		}
#if defined(APPLY_CHANGE)
				// Take effect in real time
				mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
				if ( vChar == DHCP_LAN_SERVER)	// DHCPv6 Server enabled
					start_dhcpv6(1);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

		case 6:	//(6) Delete Domain
				if (!check_access(SECURITY_SUPERUSER))
					break;
				DomainSearchDel();
				//printWaitStr();
#if defined(APPLY_CHANGE)
				// Take effect in real time
				mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
				if ( vChar == DHCP_LAN_SERVER)	// DHCPv6 Server enabled
					start_dhcpv6(1);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

		case 7:	//(7)Show domain
			showDomainSearchTable();
			//printWaitStr();
			break;

		case 8://(8) Quit
			return;
		}//end switch, switch(choice)
		setErr_domain:
			printWaitStr();
			printf("\n");
	}//end while,
}

void setDHCPv6Page(){
	int snum;
	unsigned char vChar;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DHCPv6 Menu                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) DHCPv6 Mode				(2) DHCPv6 Server\n");
		printf("(3) DHCPv6 Relay			(4) Quit		\n");

		if (!getInputOption( &snum, 1, 4))
			continue;

		switch(snum)
		{
			case 1: // (1) DHCPv6 Mode
				setDHCPv6Mode();
				break;
			case 2: // (2) DHCPv6 Server
				mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
				if (vChar)	// DHCPv6 enabled
					setDHCPv6Server();
				else {
					printf("DHCPv6 mode is none(1)!!\n");
					printWaitStr();
				}
				break;

			case 3: // (3) DHCPv6 Relay
				mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
				if (vChar)	// DHCPv6 enabled
					setDHCPv6Relay();
				else {
					printf("DHCPv6 mode is none(2)!!\n");
					printWaitStr();
				}
				break;
			case 4: // (4) Quit
				return;
			default:
				printf("!! DHCPv6 Menu: Invalid Selection !!\n");
		}
	}
}
#endif  // #ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
#endif  // #ifdef CONFIG_IPV6

void setDNSServer()
{
	unsigned int choice,num,min,max;
	unsigned char vChar;
	char *pDns;
	DNS_TYPE_T dns;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           DNS Configuration                             \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the DNS server ip addresses for DNS Relay.\n");
		printf("-------------------------------------------------------------------------\n");
		printf("DNS Configuration:\n\n");
		getSYS2Str(SYS_DNS_MODE, strbuf);
		printf("  Mode: %s\n", strbuf);
		printf("  DNS1: %s\n", getMibInfo(MIB_ADSL_WAN_DNS1));
		pDns = getMibInfo(MIB_ADSL_WAN_DNS2);
		if (!strcmp(pDns, "0.0.0.0"))
			printf("  DNS2: \n");
		else
			printf("  DNS2: %s\n", pDns);
		pDns = getMibInfo(MIB_ADSL_WAN_DNS3);
		if (!strcmp(pDns, "0.0.0.0"))
			printf("  DNS3: \n");
		else
			printf("  DNS3: %s\n", pDns);

		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption( &choice, 1, 2))
			continue;
		min=1;max=2;
		switch (choice)
		{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			if (0 == getInputUint("(1)Attain DNS Automatically (2)Set DNS Manually: ", &num , &min, &max))
				continue;

			if(2==num)
			{
				struct in_addr dns1, dns2, dns3;
				dns = DNS_MANUAL;

				dns1.s_addr=0;
				dns2.s_addr=0;
				dns3.s_addr=0;

				if (0==getInputIpAddr("DNS1: ", &dns1))
					continue;
				if (0!=getInputIpAddr("DNS2: ", &dns2))
					getInputIpAddr("DNS3: ", &dns3);

				if (
					!mib_set(MIB_ADSL_WAN_DNS1, (void *)&dns1) ||
					!mib_set(MIB_ADSL_WAN_DNS2, (void *)&dns2) ||
					!mib_set(MIB_ADSL_WAN_DNS3, (void *)&dns3)
				)
				{
					printf("Set DNS MIB error!");
					return;
				}
			} else {
				dns = DNS_AUTO;
			}

			vChar = (unsigned char) dns;
			if ( !mib_set(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
				printf("Set DNS MIB error!");
			}
			else
			{
#ifdef APPLY_CHANGE
				// Take effect in real time
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
				restart_dnsrelay();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
			}
			printWaitStr();
			break;

		case 2://(2) Quit
			return;
		}//end switch, DNS Configuration
	}//end while, DNS Configuration
}

#ifdef CONFIG_USER_DDNS
static int getDDNS(MIB_CE_DDNS_Tp entry) {
	unsigned int num, min, max, sel;

	min=1;max=2;
	if (0 == getInputUint("DDNS provider: (1)DynDNS.org (2)TZO (3)No-IP :", &sel, &min, &max))
		return 0;
	if (0 == getInputString("Hostname: ", entry->hostname, 34))
		return 0;
	if (0 == getInputUint("Status (1)Enable (2)Disable :", &num, &min, &max))
		return 0;
	if (num==1)
		entry->Enabled = 1;
	else
		entry->Enabled = 0;
	if (sel == 1 || sel == 3) {
		if (sel == 1)
			strcpy(entry->provider, "dyndns");
		else 
			strcpy(entry->provider, "noip");
		if (0 == getInputString("Username: ", entry->username, 34))
			return 0;
		if (0 == getInputString("Password: ", entry->password, 34))
			return 0;
	}
	else {
		strcpy(entry->provider, "tzo");
		if (0 == getInputString("Email: ", entry->username, 34))
			return 0;
		if (0 == getInputString("Key: ", entry->password, 34))
			return 0;
	}
	strcpy(entry->interface, "all");
	return 1;
}

void showDDNSTable()
{
	unsigned int entryNum, i;
	MIB_CE_DDNS_T Entry;

	entryNum = mib_chain_total(MIB_DDNS_TBL);

	CLEAR;
	printf("\n");
	printf("Dynamic DNS Table:\n");
	printf("Idx State   Hostname                           Service   Username\n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DDNS_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		printf("%-4d%-8s%-35s%-10s%s\n", i+1, Entry.Enabled ? "Enable" : "Disable", Entry.hostname, Entry.provider, Entry.username);
	}
}

void setDDNS()
{
	unsigned int choice, min, max;
	DNS_TYPE_T dns;
	MIB_CE_DDNS_T entry, tmpEntry;
	int num, i, intVal;

	memset(&entry, 0x00, sizeof(entry));
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           Dynamic DNS                             \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the Dynamic DNS address from DynDNS.org \n");
		printf("or TZO or No-IP. Here you can Add/Remove to configure Dynamic DNS. \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Add                          (2) Remove\n");
		printf("(3) Show                         (4) Quit\n");
		if (!getInputOption(&choice, 1, 4))
			continue;
		min=1; max=4;
		switch (choice) {
			case 1://(1) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (getDDNS(&entry) == 0)
					continue;

				num = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
				// check duplication
				for (i=0; i<num; i++) {
					mib_chain_get(MIB_DDNS_TBL, i, (void *)&tmpEntry);
					if (strcmp(entry.provider, tmpEntry.provider))
						continue;
					if (strcmp(entry.hostname, tmpEntry.hostname))
						continue;
					if (strcmp(entry.username, tmpEntry.username))
						continue;
					if (strcmp(entry.password, tmpEntry.password))
						continue;
					// entry duplication
					printf("Entry already exists!\n");
					goto setErr_ddns;
				}
				intVal = mib_chain_add(MIB_DDNS_TBL, (unsigned char*)&entry);
				if (intVal == 0)
					printf("Add Dynamic DNS chain record error.\n");
				else if (intVal == -1)
					printf("Error! Table Full.");
				else
				{
#ifdef APPLY_CHANGE
					// Take effect in real time
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
					restart_dnsrelay();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

		case 2: //(2) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			num = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
			if (num == 0) {
				printf("Empty table!\n");
				printWaitStr();
				continue;
			}
			showDDNSTable();
			min=1; max=num;
			if (getInputUint("Select the index to delete:", &intVal, &min, &max) == 0) {
				printf("Error selection!\n");
				printWaitStr();
				continue;
			}
			if(mib_chain_delete(MIB_DDNS_TBL, intVal-1) != 1)
				printf("Delete chain record error!");
			else
			{
#ifdef APPLY_CHANGE
				// Take effect in real time
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
				restart_dnsrelay();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
			}
			break;

		case 3: //(4)Show
			showDDNSTable();
			break;

		case 4://(4) Quit
			return;
		}//end switch, DNS Configuration
	setErr_ddns:
		printWaitStr();
		printf("\n");
	}//end while, DNS Configuration
}
#endif

void setDNS()
{
	int snum;

	while (1) {
		CLEAR;
		printf("\n");
		printf("-----------------------------------------------------------\n");
		printf("                         (2) DSN                                    \n");
		printf("-----------------------------------------------------------\n");
		printf("(1) DNS Server                  (2) Dynamic DNS\n");
		printf("(3) Quit\n");
		if (!getInputOption(&snum, 1, 3))
			continue;

		switch (snum) {
			case 1://(1) DNS Server
				setDNSServer();
				break;
			case 2://(2) Dynamic DNS
#ifdef CONFIG_USER_DDNS
				setDDNS();
#else
				printf("Not supported\n");
				printWaitStr();
#endif
				break;
			case 3://(3) Quit
				return;
			default:
				printf("!! Invalid Selection !!\n");
		}
	}
}

/*************************************************************************************************/
#ifdef IP_PORT_FILTER
void showIPPortFilteringTable()
{
	unsigned int entryNum, i;
	MIB_CE_IP_PORT_FILTER_T Entry;
	char *dir, *ract;
	char *type, *sip, *dip;
	char sipaddr[20],dipaddr[20], sportRange[20], dportRange[20];
	unsigned char vChar;

	CLEAR;
	// Mason Yu
	printf("Default Action:\n");
	printf("----------------------------------------------\n");
	if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		printf("Outgoing Default Action: Deny\n");
	else
		printf("Outgoing Default Action: Allow\n");


	if ( !mib_get( MIB_IPF_IN_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		printf("InComing Default Action: Deny\n");
	else
		printf("InComing Default Action: Allow\n");


	// IP/Filter Rule Table
	entryNum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);

   	printf("\n");
   	printf("Current Filter Table:\n");
	//printf("Idx  Local IP Address  Port Range  Port Type  Protocol Rule  Action  Comment \n");
	printf("Idx Direction Protocol SrcAddress         SrcPort     DstAddress         DstPort     Rule_Action\n");
   	printf("--------------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&Entry))
		{
  			printf("showIPPortFilteringTable: Get chain record error!\n");
			return;
		}

		if (Entry.dir == DIR_OUT)
			dir = "Outgoing";
		else
			dir = "Incoming";

		// Modified by Mason Yu for Block ICMP packet
		if ( Entry.protoType == PROTO_ICMP )
		{
			type = (char *)ARG_ICMP;
		}
		else if ( Entry.protoType == PROTO_TCP )
			type = (char *)ARG_TCP;
		else
			type = (char *)ARG_UDP;

		sip = inet_ntoa(*((struct in_addr *)Entry.srcIp));
		//printf("Entry.srcIp=%x\n", *(unsigned long *)(Entry.srcIp));

		if ( !strcmp(sip, "0.0.0.0"))
			sip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sipaddr, 20, "%s", sip);
			else
				snprintf(sipaddr, 20, "%s/%d", sip, Entry.smaskbit);
			sip = sipaddr;
		}


		if ( Entry.srcPortFrom == 0)
			strcpy(sportRange, BLANK);
		else if ( Entry.srcPortFrom == Entry.srcPortTo )
			snprintf(sportRange, 20, "%d", Entry.srcPortFrom);
		else
			snprintf(sportRange, 20, "%d-%d", Entry.srcPortFrom, Entry.srcPortTo);

		dip = inet_ntoa(*((struct in_addr *)Entry.dstIp));
		if ( !strcmp(dip, "0.0.0.0"))
			dip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dipaddr, 20, "%s", dip);
			else
				snprintf(dipaddr, 20, "%s/%d", dip, Entry.dmaskbit);
			dip = dipaddr;
		}

		if ( Entry.dstPortFrom == 0)
			strcpy(dportRange, BLANK);
		else if ( Entry.dstPortFrom == Entry.dstPortTo )
			snprintf(dportRange, 20, "%d", Entry.dstPortFrom);
		else
			snprintf(dportRange, 20, "%d-%d", Entry.dstPortFrom, Entry.dstPortTo);

		if ( Entry.action == 0 )
			ract = "Deny";
		else
			ract = "Allow";

	 	printf("%-4d%-10s%-9s%-19s%-12s%-19s%-12s%s\n",i+1, dir, type, sip, sportRange, dip, dportRange, ract);
	}



}

void filterAdd(unsigned char filterMode,struct in_addr *sip, struct in_addr *smask,unsigned short sfromPort,unsigned short stoPort,
                struct in_addr *dip, struct in_addr *dmask,unsigned short dfromPort,unsigned short dtoPort,
		unsigned char ptType,unsigned char prType){
	int mibTblId;
	unsigned int totalEntry;
	char * strVal;
	MIB_CE_IP_PORT_FILTER_T filterEntry;
	unsigned char noIP=1;
	char *strFrom, *strTo;
	unsigned long v1, v2, v3;
	struct in_addr *pAddr;
	unsigned char *tmpip;
	int i, intVal;

	unsigned long mask, mbit;


	mibTblId = MIB_IP_PORT_FILTER_TBL;
	totalEntry = mib_chain_total(mibTblId); /* get chain record size */

	memset(&filterEntry, 0x00, sizeof(filterEntry));

	// Protocol Type
	if ( prType==1 ) {
		filterEntry.protoType = PROTO_ICMP;
	}
	else if ( prType==2  ) {
		filterEntry.protoType = PROTO_TCP;
	}
	else if ( prType==3 ) {
		filterEntry.protoType = PROTO_UDP;
	}
	else {
		printf("Error! Invalid protocol type.\n");
		goto setErr_filter;
	}

	// Direction
	if ( ptType==1 ) {
		filterEntry.dir = DIR_OUT;
	}
	else if ( ptType==2  ) {
		filterEntry.dir = DIR_IN;
	}
	else {
		printf("Error! Invalid direction.\n");
		goto setErr_filter;
	}


	// Src Host
	if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP)
		sfromPort = 0;

	pAddr = (struct in_addr *)filterEntry.srcIp;
	pAddr->s_addr = sip->s_addr;

	mbit=0;

	while (1) {
		if (smask->s_addr&0x80000000) {
			mbit++;
			smask->s_addr <<= 1;
		}
		else
			break;
	}

	filterEntry.smaskbit = mbit;
	filterEntry.srcPortFrom = (unsigned short)sfromPort;
	filterEntry.srcPortTo = (unsigned short)stoPort;

	if ( filterEntry.srcPortFrom  > filterEntry.srcPortTo ) {
		printf("Error! Invalid src port range.\n");
		goto setErr_filter;
	}

	// Dst Host
	if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP)
		dfromPort = 0;

	pAddr = (struct in_addr *)filterEntry.dstIp;
	pAddr->s_addr = dip->s_addr;

	mbit=0;

	while (1) {
		if (dmask->s_addr&0x80000000) {
			mbit++;
			dmask->s_addr <<= 1;
		}
		else
			break;
	}

	filterEntry.dmaskbit = mbit;

	filterEntry.dstPortFrom = (unsigned short)dfromPort;
	filterEntry.dstPortTo = (unsigned short)dtoPort;

	if ( filterEntry.dstPortFrom  > filterEntry.dstPortTo ) {
		printf("Error! Invalid Dst port range.\n");
		goto setErr_filter;
	}

	// Rule Action
	if (filterMode==1)
		filterEntry.action = 0;
	else if (filterMode==2)
		filterEntry.action = 1;
	else {
		printf("Invalid Rule Action value!\n");
		goto setErr_filter;
	}

	intVal = mib_chain_add(MIB_IP_PORT_FILTER_TBL, (unsigned char*)&filterEntry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		goto setErr_filter;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		goto setErr_filter;
	}
	return;


setErr_filter:
	printf("set FW error!\n");
}

//tylo
void filterdel(){
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;

	mibTblId = MIB_IP_PORT_FILTER_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
	}
	else if(del==1)
	{
		showIPPortFilteringTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index-1) != 1) {
			printf("Delete chain record error!");
		}
	}

}

void setIPPortFiltering()
{
	int min, max, sel;
	struct in_addr sip, smask, dip, dmask;
	unsigned char filterMode,ptType,prType;
	unsigned short sfromPort, stoPort, dfromPort, dtoPort;
	int snum, del;
	char inAct, outAct;
	unsigned char vChar;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              IP/Port Filtering                          \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Entries in this table are used to restrict certain types of data packets \n");
		printf("from your local network to Internet through the Gateway. Use of such     \n");
		printf("filters can be helpful in securing or restricting your local network.    \n");
		printf("Incoming Domain: LAN    Outgoing Domain: WAN                             \n");
		printf("-------------------------------------------------------------------------\n");
		//printf("Default Action :\n");
		printf("(1) Set                    (2) Add\n");
		printf("(3) Delete                 (4) Show IP/Port Filtering Default Action and Table\n");
		printf("(5) Quit\n");
		if (!getInputOption( &snum, 1, 5))
			continue;

		switch( snum)
		{
		case 1://(1) Set
			if (!check_access(SECURITY_SUPERUSER))
				break;
			getSYS2Str(SYS_IPF_OUT_ACTION, strbuf);
			printf("Old Outgoing Default Action: %s\n",strbuf);
			min = 1; max = 2;
			if (0 == getInputUint("New Outgoing Default Action (1)Deny (2)Allow: ", &sel, &min, &max))
				continue;
			outAct = (char)sel-1;

			getSYS2Str(SYS_IPF_IN_ACTION, strbuf);
			printf("Old Incoming Default Action : %s\n",strbuf);
			if (0 == getInputUint("New Incoming Default Action (1)Deny (2)Allow: ", &sel, &min, &max))
				continue;
			inAct = (char)sel-1;

			if ( mib_set( MIB_IPF_OUT_ACTION, (void *)&outAct) == 0) {
				printf("Set Default Filter Action error!");
			}

			if(!mib_set( MIB_IPF_OUT_ACTION, (void *)&outAct) || !mib_set( MIB_IPF_IN_ACTION, (void *)&inAct))
				printf("Set Default Filter Action error!");
			else
			{
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_IPFilter_DMZ_MACFilter();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
			}
			break;
		case 2://(2) Add
			if (!check_access(SECURITY_SUPERUSER))
				break;
			// Rule Action
			min = 1; max = 2;
			if (0 == getInputUint("Rule Action (1)Deny (2)Allow: ", &sel, &min, &max))
				continue;
			filterMode = (char)sel;

         		// Src Host
			if (0 == getInputIpAddr("Src IP Address:", &sip))
				continue;

			//if (0 == getInputIpAddr("Src Subnet Mask:", &smask))
			if (0 == getInputIpMask("Src Subnet Mask:", &smask))
				continue;

			min = 0; max = 65535;
			if (0 == getInputUint("Src Port Range Start(1~65535, 0: ignore port range) Start:", &sel, &min, &max))
			    continue;
			sfromPort = (unsigned short)sel;

			if (0 != sfromPort)
			{
				min = sel;
				if (0 == getInputUint("Src Port Range(1~65535) End:", &sel, &min, &max))
					continue;
				stoPort = (unsigned short)sel;
			}
			else
			{
				stoPort = 0;
			}

			// Dst Host
			if (0 == getInputIpAddr("Dst IP Address:", &dip))
				continue;

			//if (0 == getInputIpAddr("Dst Subnet Mask:", &dmask))
			if (0 == getInputIpMask("Dst Subnet Mask:", &dmask))
				continue;

			min = 0; max = 65535;
			if (0 == getInputUint("Dst Port Range Start(1~65535, 0: ignore port range) Start:", &sel, &min, &max))
			    continue;
			dfromPort = (unsigned short)sel;

			if (0 != dfromPort)
			{
				min = sel;
				if (0 == getInputUint("Dst Port Range(1~65535) End:", &sel, &min, &max))
					continue;
				dtoPort = (unsigned short)sel;
			}
			else
			{
				dtoPort = 0;
			}

			// Direction
			min = 1; max = 2;
			if (0 == getInputUint("Direction (1)Outgoing (2)Incoming: ", &sel, &min, &max))
				continue;
			ptType = (unsigned char)sel;

			// Protocol
			min = 1; max = 3;
			if (0 == getInputUint("Protocol (1)ICMP (2)TCP (3)UDP: ", &sel, &min, &max))
				continue;
			prType = (unsigned char)sel;

			filterAdd(filterMode, &sip, &smask, sfromPort, stoPort, &dip, &dmask, dfromPort, dtoPort, ptType, prType);
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
			// Take effect in real time
			restart_IPFilter_DMZ_MACFilter();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			break;
		case 3://(3) Delete
			if (!check_access(SECURITY_SUPERUSER))
				break;
			filterdel();
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
			// Take effect in real time
			restart_IPFilter_DMZ_MACFilter();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
			printWaitStr();
			break;
		case 4://(4) Show IP/Port Filtering Table
			showIPPortFilteringTable();
			printWaitStr();
			break;
		case 5://(5) Quit
			return;
		}//end switch, IP/Port Filtering
	}//end while, IP/Port Filtering
}
#endif

#ifdef MAC_FILTER
//tylo
void showIMACFilteringTable()
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_MAC_FILTER_T Entry;
	char ract[6];
	char tmpbuf[100], tmpbuf2[100];
	char *dir, *outAct, *inAct;
	unsigned char vChar;

	entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);

	CLEAR;
	printf("\n");
	printf("Outgoing and Incoming Default Action :\n");
	printf("-------------------------------------------------------------------------\n");
	if ( !mib_get( MIB_MACF_OUT_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		outAct = "Deny";
	else
		outAct = "Allow";
	printf("Outgoing : %s\n", outAct);


	if ( !mib_get( MIB_MACF_IN_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		inAct = "Deny";
	else
		inAct = "Allow";
	printf("Incoming : %s\n", inAct);


   	printf("\n");
   	printf("Current Filter Table:\n");
	printf("Idx  Direction  Src MAC Address     Dst MAC Address      Rule Action  \n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		if (Entry.dir == DIR_OUT)
			dir = "Outgoing";
		else
			dir = "Incoming";

		if ( Entry.action == 0 )
			strcpy(ract,"Deny");
		else
			strcpy(ract,"Allow");

		if (   Entry.srcMac[0]==0 && Entry.srcMac[1]==0
		    && Entry.srcMac[2]==0 && Entry.srcMac[3]==0
		    && Entry.srcMac[4]==0 && Entry.srcMac[5]==0 ) {
			strcpy(tmpbuf, "------");
		}else {
			snprintf(tmpbuf, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
				Entry.srcMac[0], Entry.srcMac[1], Entry.srcMac[2],
				Entry.srcMac[3], Entry.srcMac[4], Entry.srcMac[5]);
		}

		if (   Entry.dstMac[0]==0 && Entry.dstMac[1]==0
		    && Entry.dstMac[2]==0 && Entry.dstMac[3]==0
		    && Entry.dstMac[4]==0 && Entry.dstMac[5]==0 ) {
			strcpy(tmpbuf2, "------");
		}else {
			snprintf(tmpbuf2, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
				Entry.dstMac[0], Entry.dstMac[1], Entry.dstMac[2],
				Entry.dstMac[3], Entry.dstMac[4], Entry.dstMac[5]);
		}

		printf("%-5d%-11s%-20s%-21s%-10s\n",i+1,dir,tmpbuf,tmpbuf2,ract);

	}


}

void MACFilterAdd(unsigned char act,unsigned char *src_macaddr,unsigned char *dst_macaddr,unsigned char dir){
	MIB_CE_MAC_FILTER_T macEntry;
	int mibTblId,i, intVal;
	unsigned int totalEntry;
	memset(&macEntry, 0x00, sizeof(macEntry));
	mibTblId = MIB_MAC_FILTER_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */

	if (act==1)
		macEntry.action = 0;
	else if (act==2)
		macEntry.action = 1;
	else {
		printf("Invalid Rule Action value!\n");
		goto setErr_filter;
	}

	if (dir==1)
		macEntry.dir = 0;
	else if (dir==2)
		macEntry.dir = 1;
	else {
		printf("Invalid Direction value!\n");
		goto setErr_filter;
	}

	if ( !src_macaddr[0] && !dst_macaddr[0]) {
		printf("Error! No mac address to set.\n");
		goto setErr_filter;
	}

	if (src_macaddr[0]) {
		if (strlen(src_macaddr) != 12 || !string_to_hex(src_macaddr, macEntry.srcMac, 12) || !isValidMacAddr(macEntry.srcMac)) {
			printf("Error! Invalid Src MAC address.\n");
			goto setErr_filter;
		}
	}

	if (dst_macaddr[0]) {
		if (strlen(dst_macaddr) != 12 || !string_to_hex(dst_macaddr, macEntry.dstMac, 12) || !isValidMacAddr(macEntry.dstMac)) {
			printf("Error! Invalid Dst MAC address.\n");
			goto setErr_filter;
		}
	}

	intVal = mib_chain_add(MIB_MAC_FILTER_TBL, (unsigned char*)&macEntry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		goto setErr_filter;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		goto setErr_filter;
	}
	return;

setErr_filter:
	printWaitStr();
	return;

}

//tylo
void MACFilterDel(){
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;

	mibTblId = MIB_MAC_FILTER_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
	}
	else if(del==1)
	{
		showIMACFilteringTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index-1) != 1) {
			printf("Delete chain record error!");
		}
	}
}


//tylo
void setMACFiltering()
{
	unsigned char src_mac[12], dst_mac[12];
	int snum,  del;
	unsigned char filterMode, outgoingMode, incomingMode, dir;
	unsigned char vChar;
	char inAct, outAct;
	int min, max, sel;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              MAC Filtering                              \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Entries in this table are used to restrict certain types of data packets \n");
		printf("from your local network to Internet through the Gateway. Use of such     \n");
		printf("filters can be helpful in securing or restricting your local network.    \n");
		printf("Incoming Domain: LAN    Outgoing Domain: WAN                             \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Current Filter Table:\n");
		printf("MAC Address Rule Action Comment Select \n");
		printf("-------------------------------------------------------------------------\n");

		printf("(1) Set Default Action           (2) Add\n");
		printf("(3) Delete			 (4) Show\n");
		printf("(5) Quit\n");
		if (!getInputOption( &snum, 1, 5))
			continue;

		switch( snum)
		{
			case 1://(1) Set Default Action
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("Outgoing (1) Deny (2) Allow: ", &sel, &min, &max))
					continue;
				outAct = (char)sel - 1;
				if (0 == getInputUint("Incoming (1) Deny (2) Allow: ", &sel, &min, &max))
					continue;
				inAct = (char)sel - 1;

				mib_set( MIB_MACF_OUT_ACTION, (void *)&outAct);
				outAct = !outAct;
				mib_set( MIB_ETH_MAC_CTRL, (void *)&outAct);
				mib_set( MIB_WLAN_MAC_CTRL, (void *)&outAct);

				mib_set( MIB_MACF_IN_ACTION, (void *)&inAct);
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_IPFilter_DMZ_MACFilter();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 2://(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("Rule Action (1)Deny (2)Allow: ", &sel, &min, &max))
					continue;
				filterMode = (unsigned char)sel;

				if (0 == getInputUint("Direction (1)Outgoing (2)Incoming: ", &sel, &min, &max))
					continue;
				dir = (unsigned char)sel;

				if ( getInputString( "Source MAC Address:",src_mac, sizeof(src_mac)+2)==0);//continue;
				if ( getInputString( "Destination MAC Address:",dst_mac, sizeof(dst_mac)+2)==0);//continue;

				MACFilterAdd(filterMode, src_mac, dst_mac, dir);
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_IPFilter_DMZ_MACFilter();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 3://(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				MACFilterDel();
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
				// Take effect in real time
				restart_IPFilter_DMZ_MACFilter();
#endif
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				printWaitStr();
				break;

			case 4://(4)Show
				showIMACFilteringTable();
				printWaitStr();
				break;

			case 5://(5) Quit
				return;
		}//end switch, MAC Filtering
	}//end while, MAC Filtering
}
#endif // of MAC_FILTER

int getInputWanIfIndex(int *piIfIndex)
{
	unsigned int uiEntryNum, i, uiIfNum = 1, uiMinIfSeq, uiIfSeq;
	MIB_CE_ATM_VC_T stEntry;
	unsigned int *pucIfIndex;
	int retVal;
	char wanif[IFNAMSIZ];

	printf("Wan interface:\n(1) any\n");

	uiEntryNum = mib_chain_total(MIB_ATM_VC_TBL);
	pucIfIndex = (unsigned int *)malloc(uiEntryNum + 1);
	if (NULL == pucIfIndex)
	{
		return 0;
	}

	*pucIfIndex = (unsigned int)DUMMY_IFINDEX;
	for (i = 0; i < uiEntryNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&stEntry))
		{
			printf("Can't get wan interface!\n");
			free(pucIfIndex);
			return 0;
		}

		if ((0 == stEntry.enable) || (CHANNEL_MODE_BRIDGE == stEntry.cmode))
		{
			continue;
		}

		ifGetName(stEntry.ifIndex, wanif, sizeof(wanif));
		printf("(%u) %s\n", uiIfNum + 1, wanif);

		*(pucIfIndex + uiIfNum) = (unsigned int)(stEntry.ifIndex);
		uiIfNum++;
	}

	uiMinIfSeq = 1;
	retVal=getTypedInputDefault(INPUT_TYPE_UINT,"Select Wan Interface[(1) any]: ",&uiIfSeq,&uiMinIfSeq,&uiIfNum);
	if(retVal==0){
		printf("Invalid value!\n");
		printWaitStr();
		free(pucIfIndex);
		return 0;
	} else if(retVal==-2)
		uiIfSeq=1; // default value

	*piIfIndex = *(pucIfIndex + uiIfSeq - 1);
	free(pucIfIndex);
	return 1;
}

//tylo
#ifdef PORT_FORWARD_GENERAL
void showForwardingTable()
{
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_T Entry;
	char	type[8], portRange[20], *ip, szLocalIP[20], *pszStatus, szRemotHost[20], szPublicPortRange[20], szIfName[IFNAMSIZ];

	CLEAR;
	printf("\n");
	printf("Current Port Forwarding Table\n");
	printf("%-5s%-18s%-10s%-15s%-15s%-9s%-16s%-15s%-10s\n", "Idx",  "Local IP Address",  "Protocol",
           "Local Port", "Comment", "Enable", "Remote Host", "Public Port", "Interface");
	printf("-------------------------------------------------------------------------------------------------------\n");

	entryNum = mib_chain_total(MIB_PORT_FW_TBL);
	if (!entryNum)
		printf("No data!\n");

	for (i = 0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		ip = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
		strcpy(szLocalIP, ip);
		if ( !strcmp(szLocalIP, "0.0.0.0"))
		{
			strcpy(szLocalIP ,"----");
		}

		if ( Entry.protoType == PROTO_UDPTCP )
		{
			strcpy(type,"TCP+UDP");
		}
		else if ( Entry.protoType == PROTO_TCP )
		{
			strcpy(type,"TCP");
		}
		else
		{
			strcpy(type,"UDP");
		}

		if ( Entry.fromPort == 0)
		{
			strcpy(portRange, "----");
		}
		else if ( Entry.fromPort == Entry.toPort )
		{
			snprintf(portRange, 20, "%d", Entry.fromPort);
		}
		else
		{
			snprintf(portRange, 20, "%d-%d", Entry.fromPort, Entry.toPort);
		}

		if (Entry.enable)
		{
			pszStatus = (char *)IF_ENABLE;
		}
		else
		{
			pszStatus = (char *)IF_DISABLE;
		}

		ip = inet_ntoa(*((struct in_addr *)Entry.remotehost));
		strcpy(szRemotHost, ip);

		if ( !strcmp(szRemotHost, "0.0.0.0"))
		{
			strcpy(szRemotHost ,"----");
		}

		if ( Entry.externalfromport == 0)
		{
			strcpy(szPublicPortRange, "----");
		}
		else if ( Entry.externalfromport == Entry.externaltoport )
		{
			snprintf(szPublicPortRange, 20, "%d", Entry.externalfromport);
		}
		else
		{
			snprintf(szPublicPortRange, 20, "%d-%d", Entry.externalfromport, Entry.externaltoport);
		}

		if (!ifGetName(Entry.ifIndex, szIfName, sizeof(szIfName)))
			strcpy( szIfName, "----" );

		printf("%-5d%-18s%-10s%-15s%-15s%-9s%-16s%-15s%-10s\n", i + 1, szLocalIP, type, portRange,
            Entry.comment, pszStatus, szRemotHost, szPublicPortRange, szIfName);
	}

	printWaitStr();
	return;
}

//tylo
void addPortFW(struct in_addr *ip, unsigned short fromPort,unsigned short toPort,
                    unsigned char protocol,unsigned char *comment, int iWanInf, int enable,
                    char *pszRemoteHost, unsigned short usExternalFromPort, unsigned short usExternalToPort)
{

    char *submitUrl, *strAddPort, *strDelPort, *strVal, *strDelAllPort;
    char *strIp, *strFrom, *strTo;
    char tmpBuf[100];
    int intVal;
    unsigned int entryNum, i, uiCommentLen;
    MIB_CE_PORT_FW_T entry;
    struct in_addr curIpAddr, curSubnet;
    unsigned long v1, v2, v3;
    unsigned char vChar;
    struct in_addr *paddr;

    memset(  &entry, 0, sizeof(MIB_CE_PORT_FW_T) );

    paddr = (struct in_addr *)&entry.ipAddr;
    paddr->s_addr = ip->s_addr;
    mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
    mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

    v1 = *((unsigned long *)entry.ipAddr);
    v2 = *((unsigned long *)&curIpAddr);
    v3 = *((unsigned long *)&curSubnet);

    if ( (v1 & v3) != (v2 & v3) ) {
    	printf("Invalid IP address! It should be set within the current subnet.\n");
			printWaitStr();
    	return;
    }

    entry.fromPort = fromPort;
    entry.toPort = toPort;
    if ( entry.fromPort  > entry.toPort )
    {
    	printf("Error! Invalid local port range.\n");
			printWaitStr();
    	return;
    }

    if (0 == protocol)
    {
    	entry.protoType = PROTO_UDPTCP;
    }
    else if (1== protocol)
    {
    	entry.protoType = PROTO_TCP;
    }
    else if (2 == protocol)
    {
    	entry.protoType = PROTO_UDP;
    }
    else
    {
    	printf("Error! Invalid protocol type.");
			printWaitStr();
    	return;
    }

    if (comment[0])
    {
			uiCommentLen = strlen(comment);

    	if (uiCommentLen > COMMENT_LEN-1)
			{
				printf("Error! Comment length too long.");
				printWaitStr();
				return;
    	}

    	strcpy(entry.comment, comment);
    }

    entry.ifIndex = (unsigned int)iWanInf; /* iWanInf has been checked. */
    entry.enable  = enable;
    entry.externalfromport = usExternalFromPort;
    entry.externaltoport = usExternalToPort;
    if ( entry.externalfromport  > entry.externaltoport )
    {
			printf("Error! Invalid public port range.\n");
			printWaitStr();
			return;
    }

    if (pszRemoteHost[0])
    {
			if (1 != inet_aton(pszRemoteHost, (struct in_addr *)&entry.remotehost))
			{
				printf("Error! Incorrect remote host ip addr.");
				printWaitStr();
				return;
			}
    }

	intVal = mib_chain_add(MIB_PORT_FW_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		printWaitStr();
		return;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		printWaitStr();
		return;
	}
	return;
}

//tylo
void delPortFW(){
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;

	mibTblId = MIB_PORT_FW_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		for (i=0; i<totalEntry; i++)
		{
			MIB_CE_PORT_FW_T CheckEntry;

			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&CheckEntry))
			{
				printf("Error get port fw rule!\n");
				return;
			}
		}

		mib_chain_clear(mibTblId); /* clear chain record */
	}
	else if(del==1)
	{
		showForwardingTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}

		if(mib_chain_delete(mibTblId, index - 1) != 1)
		{
			printf("Delete chain record error!");
		}
	}
	return;
}

void setPortForwarding()
{
	char ip[16], comment[20], szRemoteHost[16], cIsInputInvalid;
	int snum, del, enabled;
	unsigned short fromPort, toPort, usExternalFromPort, usExternalToPort;
	int iWanIfIndex, iRuleEnable;
	unsigned char protocol;
	int min,max,sel, retVal;
	struct in_addr remoteIp;
	char pbuf[64];
	unsigned char vChar;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              Port Forwarding                            \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Entries in this table allow you to automatically redirect common network \n");
		printf("services to a specific machine behind the NAT firewall. These settings   \n");
		printf("are only necessary if you wish to host some sort of server like a web    \n");
		printf("server or mail server on the private local network behind your Gateway's \n");
		printf("NAT firewall.                                                            \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Port Forwarding:\n");
		printf("(1) Set 				(2) Add\n");
		printf("(3) Delete				(4) Show Forwarding Table\n");
		printf("(5) Quit\n");

		if (!getInputOption( &snum, 1, 5))
			continue;

		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				getSYS2Str(SYS_DEFAULT_PORT_FW_ACTION, strbuf);
				printf("Old Port Forwarding: %s\n",strbuf);
				min=1;max=2;
				if(getInputUint("New Port Forwarding (1)Enable (2)Disable:",&sel,&min,&max)==0){
					printf("Invalid selection!\n");
					continue;
				}

				if ( sel==1)
					vChar = 1;
				else
					vChar = 0;
				if ( mib_set( MIB_PORT_FW_ENABLE, (void *)&vChar) == 0)
					printf("Set enabled flag error!\n");
				else
				{
#if defined(APPLY_CHANGE)
					setupPortFW();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;
			case 2://(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (0 == getInputIpAddr("Local IP Address: ", (struct in_addr *)&ip))
					continue;

				//min = 0;
				min = 1;
				max = 65535;
				retVal=getTypedInputDefault(INPUT_TYPE_UINT,"Local Port Range(1~65535) Start[None]: ",&sel,&min,&max);

				if(retVal==0)
				{
					printf("Invalid value!\n");
					printWaitStr();
					continue;
				}
				else if(retVal==-2)
					fromPort=0;
				else
					fromPort=(unsigned short)sel;

				if (0 != fromPort)
				{
					min = fromPort;
					sprintf(pbuf, "Local Port Range(%d~65535) End[%d]: ", fromPort, fromPort);
					retVal=getTypedInputDefault(INPUT_TYPE_UINT,pbuf,&sel,&min,&max);
					if(retVal==0)
					{
						printf("Invalid value!\n");
						printWaitStr();
						continue;
					}
					else if(retVal==-2)
						toPort=fromPort;
					else
						toPort=(unsigned short)sel;
				}
				else
					toPort = 0;

				min = 1;
				max = 3;
				if (0 == getInputUint("Protocol (1)Both (2)TCP (3)UDP :", &sel, &min, &max))
					continue;

				protocol = (unsigned char)(sel - 1);

				retVal=getTypedInputDefault(INPUT_TYPE_STRING,"Comment[None]: ",comment, (void *)(sizeof(comment) - 1),0);

				if (0 == getInputWanIfIndex(&iWanIfIndex))
					continue;

				min = 1;
				max = 2;
				if (0 == getInputUint("Rule (1)Enable (2)Disable:", &sel, &min, &max))
					continue;

				if (1 == sel)
					iRuleEnable = 1;
				else
					iRuleEnable = 0;

				szRemoteHost[0] = '\0';
				retVal=getTypedInputDefault(INPUT_TYPE_IPADDR,"Remote IP Address[None]: ",&remoteIp,0,0);

				if(retVal==0)
				{
					printf("Invalid value!\n");
					printWaitStr();
					continue;
				}
				else if(retVal==1)
					strcpy(szRemoteHost, inet_ntoa(remoteIp));

				//min = 0;
				min = 1;
				max = 65535;
				retVal=getTypedInputDefault(INPUT_TYPE_UINT,"Public Port Range(1~65535) Start[None]: ",&sel,&min,&max);

				if(retVal==0)
				{
					printf("Invalid value!\n");
					printWaitStr();
					continue;
				}
				else if(retVal==-2)
					usExternalFromPort=0;
				else
					usExternalFromPort=(unsigned short)sel;

				if (0 != usExternalFromPort)
				{
					min = usExternalFromPort;
					sprintf(pbuf, "Public Port Range(%d~65535) End[%d]: ", usExternalFromPort, usExternalFromPort);
					retVal=getTypedInputDefault(INPUT_TYPE_UINT,pbuf,&sel,&min,&max);

					if(retVal==0)
					{
						printf("Invalid value!\n");
						printWaitStr();
						continue;
					}
					else if(retVal==-2)
						usExternalToPort=usExternalFromPort;
					else
						usExternalToPort=(unsigned short)sel;
				}
				else
				usExternalToPort = 0;

				addPortFW((struct in_addr *)&ip, fromPort, toPort, protocol, comment, iWanIfIndex, iRuleEnable, szRemoteHost, usExternalFromPort
					, usExternalToPort);
#if defined(APPLY_CHANGE)
				setupPortFW();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 3://(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				delPortFW();
#if defined(APPLY_CHANGE)
				setupPortFW();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				printWaitStr();
				break;
			case 4://(4) Show Forwarding Table
				showForwardingTable();
				break;
			case 5://(5) Quit
				return;
		}//end switch, Port Forwarding
	}//end while, Port Forwarding
}
#endif // of PORT_FORWARD_GENERAL

#ifdef URL_BLOCKING_SUPPORT
//Jenny
void showURLBlkTable(int mibTblId){
	unsigned int entryNum, i;
	MIB_CE_URL_FQDN_T EntryFQDN;
	MIB_CE_KEYWD_FILTER_T EntryKeyword;

	entryNum = mib_chain_total(mibTblId);

	CLEAR;
	printf("\n");
	if (mibTblId == MIB_URL_FQDN_TBL) {
   		printf("URL Blocking Table:\n");
		printf("Idx     FQDN\n");
	}
	else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
		printf("Keyword Filtering Table:\n");
		printf("Idx 	Filtered Keyword\n");
	}
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	if (mibTblId == MIB_URL_FQDN_TBL) {
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(mibTblId, i, (void *)&EntryFQDN)) {
	  			printf("Get chain record error!\n");
				return;
			}
			printf("%-8d%s\n", i+1, EntryFQDN.fqdn);
		}
	}
	else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(mibTblId, i, (void *)&EntryKeyword)) {
	  			printf("Get chain record error!\n");
				return;
			}
			printf("%-8d%s\n", i+1, EntryKeyword.keyword);
		}
	}
}

//Jenny
void URLBlkDel(int mibTblId)
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(mibTblId); /* clear chain record */
	else if (del == 1) {
		showURLBlkTable(mibTblId);
		min=1; max=totalEntry+1;
		getInputUint("Select the index to delete:", &index, &min, &max);
		if (index > totalEntry || index <= 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index-1) != 1)
			printf("Delete chain record error!");
	}
}

//Jenny
void setURL(int mibTblId)
{
	MIB_CE_URL_FQDN_T entryFQDN;
	MIB_CE_KEYWD_FILTER_T entryKeyword;
	int snum, i, intVal;
	unsigned int totalEntry;
	char strbuf[256];

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		if (mibTblId == MIB_URL_FQDN_TBL)
			printf("            FQDN Blocking                             \n");
		else if (mibTblId == MIB_KEYWD_FILTER_TBL)
			printf("            Keyword Filtering                             \n");
		printf("-------------------------------------------------------------------------\n");

		printf("(1) Add				(2) Delete\n");
		printf("(3) Show			(4) Quit\n");
		if (!getInputOption(&snum, 1, 4))
			continue;

		switch (snum) {
			case 1:	//(1) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				totalEntry = mib_chain_total(mibTblId); /* get chain record size */
				if (mibTblId == MIB_URL_FQDN_TBL) {
					if (getInputString("FQDN: ", strbuf, MAX_URL_LENGTH - 1) == 0) ; //continue;
					for (i = 0 ; i < totalEntry; i++) {
						if (!mib_chain_get(mibTblId, i, (void *)&entryFQDN)) {
							printf(errGetEntry);
							printf("\n");
							return;
						}
						if (!strcmp(entryFQDN.fqdn, strbuf)){
							printf(" FQDN already exists!");
							printf("\n");
							return;
						}
					}
					strcpy(entryFQDN.fqdn, strbuf);
					intVal = mib_chain_add(mibTblId, (unsigned char*)&entryFQDN);
				}
				else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
					if (getInputString("Keyword: ", strbuf, MAX_KEYWD_LENGTH - 1) == 0) ;
					for (i = 0 ; i < totalEntry; i++) {
						if (!mib_chain_get(mibTblId, i, (void *)&entryKeyword)) {
							printf(errGetEntry);
							printf("\n");
							return;
						}
						if (!strcmp(entryKeyword.keyword, strbuf)){
							printf(" FQDN already exists!");
							printf("\n");
							return;
						}
					}
					strcpy(entryKeyword.keyword, strbuf);
					intVal = mib_chain_add(mibTblId, (unsigned char*)&entryKeyword);
				}
				if (intVal == 0) {
					printf("Add URL chain record error.\n");
					return;
				}
				else if (intVal == -1) {
					printf("Error! Table Full.");
					return;
				}
				restart_urlblocking();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 2:	//(2) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				URLBlkDel(mibTblId);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				printWaitStr();
				break;

			case 3:	//(3)Show
				showURLBlkTable(mibTblId);
				printWaitStr();
				break;

			case 4:	//(4) Quit
				return;
		}
	}
}

// Jenny
void setURLBlocking()
{
	int snum, min, max, sel;
	unsigned char vChar;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("			  URL Blocking							 \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the Blocked FQDN(Such as tw.yahoo.com)\n");
		printf("and filtered keyword. Here you can add/delete FQDN and filtered keyword.\n");
		printf("-------------------------------------------------------------------------\n");
		printf("URL Blocking:\n");
		printf("(1) Set 					 (2) FQDN Blocking\n");
		printf("(3) Keyword Filtering				 (4) Quit\n");
		if (!getInputOption(&snum, 1, 4))
		continue;

		switch (snum) {
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				getSYS2Str(SYS_DEFAULT_URL_BLK_ACTION, strbuf);
				printf("Old URL Blocking: %s\n", strbuf);
				min=1; max=2;
				if (getInputUint("New URL Blocking (1)Enable (2)Disable:", &sel, &min, &max) == 0) {
					printf("Invalid selection!\n");
					continue;
				}
				if (sel == 1)
					vChar = 1;
				else
					vChar = 0;
				if (mib_set(MIB_URL_CAPABILITY, (void *)&vChar) == 0)
					printf("Set enabled flag error!\n");
				else
				{
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

			case 2://(2) FQDN Blocking
				if (!check_access(SECURITY_SUPERUSER))
					break;
				setURL(MIB_URL_FQDN_TBL);
				break;

			case 3://(3) Keyword Filtering
				if (!check_access(SECURITY_SUPERUSER))
					break;
				setURL(MIB_KEYWD_FILTER_TBL);
				break;

			case 4://(4) Quit
				return;
		}//end switch, Port Forwarding
	}//end while, Port Forwarding
}
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
//Jenny
void showDomainBlkTable(){
	unsigned int entryNum, i;
	MIB_CE_DOMAIN_BLOCKING_T Entry;

	entryNum = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);

	CLEAR;
	printf("\n");
	printf("Domain Block Table:\n");
	printf("Idx     Domain\n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		printf("%-8d%s\n", i+1, Entry.domain);
	}
}

// Jenny
void DomainBlkDel()
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;

	totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(MIB_DOMAIN_BLOCKING_TBL); /* clear chain record */
	else if (del == 1) {
		showDomainBlkTable();
		min=1; max=totalEntry+1;
		getInputUint("Select the index to delete:", &index, &min, &max);
		if (index > totalEntry || index <= 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_DOMAIN_BLOCKING_TBL, index-1) != 1)
			printf("Delete chain record error!");
	}
}

// Jenny
void setDomainBlocking()
{
	MIB_CE_DOMAIN_BLOCKING_T entry;
	int snum, i, intVal, min, max, sel;
	unsigned int totalEntry;
	unsigned char vChar;
	char strbuf[256];

	while (1) {
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("            Domain Blocking                             \n");
		printf("-------------------------------------------------------------------------\n");

		printf("(1) Set			(2) Add\n");
		printf("(3) Delete		(4) Show\n");
		printf("(5) Quit\n");
		if (!getInputOption(&snum, 1, 5))
			continue;

		switch (snum) {
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				getSYS2Str(SYS_DEFAULT_DOMAIN_BLK_ACTION, strbuf);
				printf("Old Domain Blocking: %s\n",strbuf);
				min=1; max=2;
				if (getInputUint("New Domain Blocking (1)Enable (2)Disable:", &sel, &min, &max) == 0) {
					printf("Invalid selection!\n");
					continue;
				}
				if (sel == 1)
					vChar = 1;
				else
					vChar = 0;
				if (mib_set(MIB_DOMAINBLK_CAPABILITY, (void *)&vChar) == 0)
					printf("Set enabled flag error!\n");
				else
				{
#if defined(APPLY_CHANGE)
					restart_domainBLK();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

			case 2:	//(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				if (getInputString("Domain: ", strbuf, MAX_DOMAIN_LENGTH - 1) == 0)
					break;
				totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
				for (i = 0 ; i < totalEntry; i++) {
					if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&entry)) {
						printf(errGetEntry);
						goto setErr_domainblk;
					}
					if (!strcmp(entry.domain, strbuf)){
						printf("Entry already exists!");
						goto setErr_domainblk;
					}
				}
				strcpy(entry.domain, strbuf);
				intVal = mib_chain_add(MIB_DOMAIN_BLOCKING_TBL, (unsigned char*)&entry);
				if (intVal == 0) {
					printf("Add Domain Blocking chain record error.\n");
					return;
				}
				else if (intVal == -1) {
					printf("Error! Table Full.");
					return;
				}
#if defined(APPLY_CHANGE)
				restart_domainBLK();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 3:	//(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				DomainBlkDel();
				//printWaitStr();
#if defined(APPLY_CHANGE)
				restart_domainBLK();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 4:	//(4)Show
				showDomainBlkTable();
				//printWaitStr();
				break;

			case 5:	//(5) Quit
				return;
		}
		setErr_domainblk:
			printWaitStr();
			printf("\n");
	}
}
#endif

#ifdef DMZ
//tylo
/*
 * Return: 0: fail, 1: successful
 */
int dmzset(int enable, char *ip){
	char vChar;
	struct in_addr ipAddr, curIpAddr, curSubnet, secondIpAddr, secondSubnet;
	unsigned long v1, v2, v3;
#ifdef CONFIG_SECONDARY_IP
	unsigned long v4, v5;
	char ip2Enable;
#endif

	vChar = (char)(enable-1);

	if(vChar==1) {
		memcpy(&ipAddr,ip,4);
		//inet_aton(ip, (struct in_addr *)&ipAddr);
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);
#ifdef CONFIG_SECONDARY_IP
		mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&ip2Enable );
		mib_get( MIB_ADSL_LAN_IP2, (void *)&secondIpAddr );
		mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&secondSubnet);
#endif

		v1 = *((unsigned long *)&ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);
#ifdef CONFIG_SECONDARY_IP
		v4 = *((unsigned long *)&secondIpAddr);
		v5 = *((unsigned long *)&secondSubnet);
#endif

		if (v1) {
			if ( (((v1 & v3) != (v2 & v3))
#ifdef CONFIG_SECONDARY_IP
			 && !ip2Enable) ||
			( enable && ((v1 & v5) != (v4 & v5)) && ((v1 & v3) != (v2 & v3))
#endif
			 )) {
				printf("Invalid IP address! It should be set within the current subnet.\n");
				goto setErr_dmz;
			}
		}
		if ( mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0) {
			printf("Set DMZ MIB error!");
			goto setErr_dmz;
		}
	} // of if (vChar == 1)
	if ( mib_set(MIB_DMZ_ENABLE, (void *)&vChar) == 0) {
		printf("Set enabled flag error!");
		goto setErr_dmz;
	}

	return 1;
setErr_dmz:
	printf("Set DMZ error!\n");
	return 0;
}

//tylo
void setDemilitarizedZone()
{
	char dmzip[16], state;
	int snum, dmz;
	int min, max, sel;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                                   DMZ                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("A Demilitarized Zone is used to provide Internet services without        \n");
		printf("sacrificing unauthorized access to its local private network. Typically, \n");
		printf("the DMZ host contains devices accessible to Internet traffic, such as Web\n");
		printf("(HTTP ) servers, FTP servers, SMTP (e-mail) servers and DNS servers.     \n");
		printf("-------------------------------------------------------------------------\n");
		/*if( DMZ is enable)
		printf("DMZ Host IP Address: \n");
		*/
		mib_get(MIB_DMZ_ENABLE, (void *)&state);
		printf("\nDMZ: %s\n", state? STR_ENABLE:STR_DISABLE);
		printf("DMZ IP: %s\n\n", getMibInfo(MIB_DMZ_IP));

		printf("(1) Set                          (2) Quit\n\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("New DMZ (1)Disable (2)Enable :", &dmz, &min, &max))
					continue;

				if( dmz==2)
				{
					if (0 == getInputIpAddr("DMZ Host IP Address:", (struct in_addr *)&dmzip[0]))
						continue;
				}

				if (!dmzset(dmz,dmzip))
					printWaitStr();
#if defined(APPLY_CHANGE)
				setupDMZ();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2://(2) Quit
				return;
		}//end switch, DMZ
	}//end while, DMZ
}
#endif


typedef struct menu_item{
	char  display_name[128];
	void (*fn)(void);
} MENU_ITEM;


#ifdef TCP_UDP_CONN_LIMIT
void showconnlimitTable()
{
	unsigned int entryNum, i,unum ,tnum;
	MIB_CE_TCP_UDP_CONN_LIMIT_T 	Entry;
	char *dir, *ract;
	char ipaddr[20];
	struct in_addr 			dest;
	unsigned char vChar;

	CLEAR;

	mib_get( MIB_CONNLIMIT_ENABLE, (void *)&vChar);
	mib_get( MIB_CONNLIMIT_TCP, (void *)&tnum);
	mib_get( MIB_CONNLIMIT_UDP, (void *)&unum);
	printf(" Connection Limit Global Settings:\n");
	printf("----------------------------------------------\n");
	printf(" Connection Limit: %s\n", vChar? "Enable":"Disable");
	printf(" Global TCP connection Limit: %d\n", tnum);
	printf(" Global UDP connection Limit: %d\n", unum);

	printf("\n Connection Limit Current Entries:\n");
	printf("%8s %16s %10s %10s\n","Index","Local IP","Protocol","Max ports");
	printf("----------------------------------------------\n");

	entryNum = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);

	for (i=0; i<entryNum; i++)
	{
		if (!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL, i, (void *)&Entry))
		{
			continue;
		}
		dest.s_addr = *(unsigned long *)Entry.ipAddr;
		strcpy(ipaddr, inet_ntoa(dest));

		printf("%8d%16s %10s %10d\n",i+1,ipaddr,Entry.protocol?"UDP":"TCP",Entry.connNum);
	}
	printWaitStr();
	return;

}

void delConnlimt()
{
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;

	mibTblId = MIB_TCP_UDP_CONN_LIMIT_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		mib_chain_clear(mibTblId); /* clear chain record */
		restart_connlimit();// iptable control goes here
	}
	else if(del==1)
	{
		showconnlimitTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(mibTblId, index - 1) != 1)
		{
			printf("Delete chain record error!");
		}
		restart_connlimit();// iptable control goes here
	}

    return;
}

void setConnLimit()
{
	int snum, i;
	int min, max;
	int sel,tcpl,udpl;
 	MIB_CE_TCP_UDP_CONN_LIMIT_T	entry,tmpentry;
	char buff[32];
	unsigned 	char	selc,exist = 0;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              Connection Limit Setting                          \n");
		printf("-------------------------------------------------------------------------\n");
		printf(" Entries in this table allow you to limit the number of TCP/UDP ports used  \n");
		printf(" by internal users \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Set                     (2) Add\n");
		printf("(3) Delete                  (4) Show Connection Limit Table\n");
		printf("(5) Quit\n");
		if (!getInputOption( &snum, 1, 5))
			continue;

		switch( snum)
		{
			case 1://(1) Set Default Action
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("Connection Limit (1) Enable (2) Disable: ", &sel, &min, &max))
					continue;
				min = 0; max = 9999;
				if (0 == getInputUint("Global TCP maximun ports per user (0 for no limit): ", &tcpl, &min, &max))
					continue;
				if (0 == getInputUint("Global UDP maximun ports per user (0 for no limit): ", &udpl, &min, &max))
					continue;
				if (sel != 1)
					selc = 0;//disable
				else
					selc =1;
				mib_set( MIB_CONNLIMIT_ENABLE, &selc);
				mib_set( MIB_CONNLIMIT_TCP, (void *)&tcpl);
				mib_set( MIB_CONNLIMIT_UDP, (void *)&udpl);
				restart_connlimit();// iptable control goes here
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 2://(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("Protocol: (1)TCP (2)UDP: ", &sel, &min, &max))
					continue;
				entry.protocol = sel -1; //0 = TCP

				entry.Enabled = 1;
				if (0 == getInputString("IP Address: ", &buff, sizeof(buff)-1))
					continue;
				else {
					struct in_addr curIpAddr, curSubnet;
					unsigned long v1, v2, v3;
					if ( !inet_aton(buff, (struct in_addr *)&entry.ipAddr) ) {
						continue;
					}
					mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
					mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

					v1 = *((unsigned long *)entry.ipAddr);
					v2 = *((unsigned long *)&curIpAddr);
					v3 = *((unsigned long *)&curSubnet);

					if ( (v1 & v3) != (v2 & v3) ) {
						continue;
					}
				}
				min = 1;
				if (entry.protocol == 0)
					mib_get(MIB_CONNLIMIT_TCP,&max);
				else
					mib_get(MIB_CONNLIMIT_UDP,&max);
				if (max == 0) // 0 means no limit
					max = 9999;
		         	if (0 == getInputUint("Max number of ports: (1~ Global Limitation): ", &sel, &min, &max))
					continue;
				entry.connNum = sel;
				max = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
				exist = 0;
				for(i=0;i<max;i++){
					if(!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL,i,(void*)&tmpentry))
						continue;
					if(((*(int*)(entry.ipAddr))==(*(int*)(tmpentry.ipAddr))) &&
						(entry.protocol == tmpentry.protocol))
					{
						exist = 1;
					}
				}

				if (exist == 0)
				{
					mib_chain_add(MIB_TCP_UDP_CONN_LIMIT_TBL, (void *)&entry) ;
					restart_connlimit();// iptable control goes here
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}
				break;

			case 3://(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				delConnlimt();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 4://show
				showconnlimitTable();
				break;

			case 5://(5) Quit
				return;
		}
	}
}
#endif
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING


static void showAddrMapTable()
{
	unsigned int entryNum, i,unum ,tnum;
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T 	Entry;
	char *dir, *ract;
	char lsip[20],leip[20],gsip[20],geip[20];
	struct in_addr 		dest;
	unsigned char vChar;

	CLEAR;
	printf("\n Address Mapping Current Entries:\n");
	printf("%6s %16s %16s %16s %16s\n","Index","Local Start IP","Local End IP","Global Start IP", "Global End IP");
	printf("---------------------------------------------------------------------------\n");

	entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);

	for (i=0; i<entryNum; i++)
	{
		if (!mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, i, (void *)&Entry))
		{
			continue;
		}
		dest.s_addr = *(unsigned long *)Entry.lsip;
		strcpy(lsip, inet_ntoa(dest));
		dest.s_addr = *(unsigned long *)Entry.leip;
		if (dest.s_addr)
			strcpy(leip, inet_ntoa(dest));
		else
			strcpy(leip, "  ---  ");
		dest.s_addr = *(unsigned long *)Entry.gsip;
		strcpy(gsip, inet_ntoa(dest));
		dest.s_addr = *(unsigned long *)Entry.geip;
		if (dest.s_addr)
			strcpy(geip, inet_ntoa(dest));
		else
			strcpy(geip, "  ---  ");

		printf("%6d %16s %16s %16s %16s\n",i+1, lsip, leip, gsip, geip);
	}
	printWaitStr();
	return;

}

static void delAddrMap()
{
	int mibTblId;
	unsigned int totalEntry;
	unsigned int i,index;
	int del,min,max;

	mibTblId = MULTI_ADDRESS_MAPPING_LIMIT_TBL;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return;
	}
	min=1;max=2;
	if(getInputUint("Delete (1)One (2)All :",&del,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(del==2)
	{
		config_AddressMap(ACT_STOP);
		mib_chain_clear(mibTblId); /* clear chain record */
		config_AddressMap(ACT_START);
	}
	else if(del==1)
	{
		showAddrMapTable();
		min=1;max=totalEntry+1;
		getInputUint( "Select the index to delete:",&index, &min,&max);
		if(index>totalEntry || index<=0){
			printf("Error selection!\n");
			return;
		}
		config_AddressMap(ACT_STOP);
		if(mib_chain_delete(mibTblId, index - 1) != 1)
		{
			printf("Delete chain record error!");
		}
		config_AddressMap(ACT_START);
	}
    return;
}

static void setAddrMap()
{
	int snum, i;
	int min, max;
	int sel,tcpl,udpl;
 	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T	entry,tmpentry;
	char buff[32];
	unsigned char	selc, exist = 0;
//typedef enum { ADSMAP_NONE=0, ADSMAP_ONE_TO_ONE=1, =2, ADSMAP_MANY_TO_MANY=3, ADSMAP_ONE_TO_MANY=4 } ADSMAP_T;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                              Address Mappinig Setting                         	 \n");
		printf("-------------------------------------------------------------------------\n");
		printf(" Entries in this table allow you to add one to one, many to one,  \n");
		printf(" many to many, many to one addres mappings. The maximum number is 16. \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Show                     (2) Add\n");
		printf("(3) Delete                   (4) Quit\n");
		if (!getInputOption( &snum, 1, 4))
			continue;

		memset(&entry, 0, sizeof(entry));
		switch( snum)
		{
			case 1://show
				showAddrMapTable();
				break;
			case 2://(2) Add
				if (!check_access(SECURITY_SUPERUSER))
					break;
				max = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);
				if (max >=15)
					break;
				min = 1; max = 4;
				if (0 == getInputUint("Address Mapping Type: \r\n(1)One to One (2)Many to one: (3)Many to Many (4)One to Many :", &sel, &min, &max))
					continue;
				entry.addressMapType = sel;

				// local start ip
				if (0 == getInputString("Local Start IP: ", &buff, sizeof(buff)-1))
					continue;
				else {
					if ( !inet_aton(buff, (struct in_addr *)&entry.lsip) ) {
						continue;
					}
				}
				//local end ip
				if ((entry.addressMapType==ADSMAP_MANY_TO_ONE)||(entry.addressMapType==ADSMAP_MANY_TO_MANY))
				{
					if (0 == getInputString("Local End IP: ", &buff, sizeof(buff)-1))
						continue;
					else {
						if ( !inet_aton(buff, (struct in_addr *)&entry.leip) ) {
							continue;
						}
					}

				}
				//Global Start IP
				if (0 == getInputString("Global Start IP: ", &buff, sizeof(buff)-1))
						continue;
				else {
					if ( !inet_aton(buff, (struct in_addr *)&entry.gsip) ) {
						continue;
					}
				}

				//global end ip
				if ((entry.addressMapType==ADSMAP_ONE_TO_MANY)||(entry.addressMapType==ADSMAP_MANY_TO_MANY))
				{
					if (0 == getInputString("Global End IP: ", &buff, sizeof(buff)-1))
						continue;
					else {
						if ( !inet_aton(buff, (struct in_addr *)&entry.geip) ) {
							continue;
						}
					}
				}
				config_AddressMap(ACT_STOP);
				mib_chain_add(MULTI_ADDRESS_MAPPING_LIMIT_TBL, (void *)&entry) ;
				config_AddressMap(ACT_START);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 3://(3) Delete
				if (!check_access(SECURITY_SUPERUSER))
					break;
				delAddrMap();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;

			case 4://(5) Quit
				return;
		}
	}
}

#endif
#endif

void setFirewall()
{
	int snum,x;
	int submenu,tmpi=0;;
	MENU_ITEM	*pm;
	unsigned char	buf[256];
	MENU_ITEM fw_menu[] = {
#ifdef IP_PORT_FILTER
		{"IP/Port Filtering", setIPPortFiltering},
#endif
#ifdef MAC_FILTER
		{"MAC Filtering", setMACFiltering},
#endif
#ifdef PORT_FORWARD_GENERAL
		{"Port Forwarding", setPortForwarding},
#endif
#ifdef URL_BLOCKING_SUPPORT
		{"URL Blocking", setURLBlocking},
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
		{"Domain Blocking", setDomainBlocking},
#endif
#ifdef DMZ
		{"DMZ", setDemilitarizedZone},
#endif
#ifdef TCP_UDP_CONN_LIMIT
		{"Connection Limit", setConnLimit},
#endif
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
		{"Address Mapping", setAddrMap},
#endif
#endif
		{"",NULL}
	};

	while (1)
	{
		tmpi = 0;
		CLEAR;
		printf("\n");
		printf("-----------------------------------------------------------\n");
		printf("                (3) Firewall Menu                          \n");
		printf("-----------------------------------------------------------\n");
		for (pm =&fw_menu[0] ;pm->fn!= NULL;pm++)
		{
			tmpi++;
			printf("(%d) %s",tmpi, pm->display_name);
			if (tmpi&0x01)
				printspace(33-4-strlen(pm->display_name));
			else
				printf("\n");
		}
		tmpi++;
		printf("(%d) Quit \n",tmpi);
		if (!getInputOption( &snum, 1, tmpi))
		      	continue;
		if (snum == tmpi)
			return;
		fw_menu[snum-1].fn();
	}
}

#ifdef CONFIG_USER_IGMPPROXY
//tylo
unsigned int IGMPsel[8];
int IGMPProxylist()
{
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	//char  buffer[3];
	char *name;
	int type=1;//rt
	unsigned char IGMPselcnt=0;
	char wanif[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if (type == 2) {
			if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			{
				IGMPsel[IGMPselcnt]=Entry.ifIndex;
				IGMPselcnt++;
				printf("%-5dvc%u\n",
					IGMPselcnt, VC_INDEX(Entry.ifIndex));

				ifnum++;
			}
		}
		else { // rt or all (1 or 0)
			if (type == 1 && Entry.cmode == CHANNEL_MODE_BRIDGE)
				continue;
			ifGetName(Entry.ifIndex, wanif, sizeof(wanif));
			IGMPsel[IGMPselcnt]=Entry.ifIndex;
			IGMPselcnt++;

			printf("%-5d%s\n",
				IGMPselcnt, wanif);

			ifnum++;
		}
	}

	return ifnum;
}

//tylo
void IGMPProxy(unsigned char act){
	char	*str, *submitUrl;
	char tmpBuf[100];
	FILE *fp;
	char * argv[8];
	char ifname[IFNAMSIZ];
	unsigned char proxy;
	unsigned int proxy_if;
	int igmp_pid;
	unsigned int min,max,sel,test;

	if (act==1)
		proxy = 1;
	else
		proxy = 0;
	if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
		strcpy(tmpBuf, "Set IGMP proxy error!");
		goto setErr_igmp;
	}

	if(proxy==0 && act==0)
		return;
	if ( act == 2 )
		goto KILL_IGMPD;

	max=IGMPProxylist();
	min=1;
	if(getInputUint("Select interface:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}
	proxy_if=IGMPsel[sel-1];
	if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
		printf("Set IGMP proxy interface index error!\n");
		goto setErr_igmp;
	}

#ifdef EMBED
#define RUNFILE "/var/run/igmp_pid"
KILL_IGMPD:
#if defined(APPLY_CHANGE)
	// Take effect in real time
	igmp_pid = read_pid(RUNFILE);

	if (igmp_pid >= 1) {
		// kill it
		if (kill(igmp_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", igmp_pid);
		}
	}

	if (proxy == 1) {
		// start the igmp proxy
		ifGetName(proxy_if, ifname, sizeof(ifname));

		//igmpproxy -c 1 -d br0 -u nas0_0,vc0 -D
		argv[1] = "-c";
		argv[2] = "1";
		argv[3] = "-d";
		argv[4] = (char *)LANIF;
		argv[5] = "-u";
		argv[6] = ifname;
		argv[7] = NULL;
		do_cmd(IGMPROXY, argv, 0);
		printWaitStr();
	}
#endif
#endif
	return;
setErr_igmp:
	printf("Set IGMP Proxy error!\n");
	return;
}

//tylo
void setIGMPProxy()
{
	int snum, proxy_act;
	unsigned int proxy_if;
	char ifname[IFNAMSIZ];
	char enabled;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           IGMP Proxy Configuration                      \n");
		printf("-------------------------------------------------------------------------\n");
		printf("IGMP proxy enables the system to issue IGMP host messages on behalf of   \n");
		printf("hosts that the system discovered through standard IGMP interfaces. The   \n");
		printf("system acts as a proxy for its hosts when you enable it by doing the     \n");
		printf("follows:                                                                 \n");
		printf(". Enable IGMP proxy on WAN interface (upstream), which connects to a     \n");
		printf("  running IGMP. \n");
		printf(". Enable IGMP on LAN interface (downstream), which connects to its hosts.\n");
		printf("-------------------------------------------------------------------------\n");
		mib_get(MIB_IGMP_PROXY, (void *)&enabled);
		printf("IGMP Proxy: %s\n", enabled? STR_ENABLE:STR_DISABLE);
		if (enabled) {
			mib_get(MIB_IGMP_PROXY_ITF, (void *)&proxy_if);
			ifname[0] = 0;
			ifGetName(proxy_if, ifname, sizeof(ifname));
			printf("Proxy Interface: %s\n", ifname);
		}
		printf("\n(1) Set                          (2) Quit\n\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				printf("IGMP Proxy (1)Enable (2)Disable :");
				if( (proxy_act=getInputNum()) ==0) continue;
				IGMPProxy(proxy_act);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2://(2) Quit
				return;
		}//end switch, IGMP Proxy Configuration
	}//end while, IGMP Proxy Configuration
}
#endif	// of CONFIG_USER_IGMPPROXY

#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
void setUPnP()
{
	int snum, min, max, sel;
	unsigned int ext_if;
	unsigned char vChar, upnp;
	char ifname[IFNAMSIZ];
	char enabled;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                         UPnP Configuration                      \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure UPnP. The system acts as a daemon when you \n");
		printf("enable it and select WAN interface (upstream) that will use UPnP. \n");
		printf("-------------------------------------------------------------------------\n");
		mib_get(MIB_UPNP_DAEMON, (void *)&enabled);
		printf("UPnP: %s\n", enabled? STR_ENABLE:STR_DISABLE);
		if (enabled) {
			mib_get(MIB_UPNP_EXT_ITF, (void *)&ext_if);
			ifname[0] = 0;
			ifGetName(ext_if, ifname, sizeof(ifname));
			printf("WAN Interface: %s\n", ifname);
		}
		printf("\n(1) Set                          (2) Quit\n\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch(snum) {
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				min = 1; max = 2;
				if (0 == getInputUint("UPnP (1) Disable (2) Enable: ", &sel, &min, &max))
					continue;
				upnp = (unsigned char)(sel - 1);
				if (upnp==0)
					goto end_upnp;
				max = IGMPProxylist();
				min=1;
				if (getInputUint("Select interface:", &sel, &min, &max)==0){
					printf("Invalid selection!\n");
					goto setErr_upnp;
				}
				ext_if = IGMPsel[sel-1];
				if (!mib_set(MIB_UPNP_DAEMON, (void *)&upnp)) {
					printf("Set UPnP error!");
					goto setErr_upnp;
				}

				if (!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if)) {
					printf("Set UPnP interface index error!\n");
					goto setErr_upnp;
				}
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2://(2) Quit
				return;
		}
		setErr_upnp:
			printWaitStr();
		end_upnp:
			printf("\n");
	}
}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
//tylo
/* Return value:
 * 0: fail
 * 1: successful
 */
int RIPconfig(){
	unsigned char ripVal;
	unsigned char ripValGet;
	unsigned int min,max,sel;
	char msgstr[10];

	mib_get(MIB_RIP_ENABLE, (void *)&ripValGet);
	if(ripValGet==1)
		strcpy(msgstr,"ON");
	else
		strcpy(msgstr,"OFF");
	printf("RIP: %s\n",msgstr);
	min=1;max=2;
	if(getInputUint("New RIP (1)ON (2)OFF :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (sel==1)
		ripVal = 1;
	else
		ripVal = 0;	// default "off"
	if (!mib_set(MIB_RIP_ENABLE, (void *)&ripVal)) {
		printf("Set RIP error!");
	}

	return 1;
}
#endif	// of CONFIG_USER_ROUTED_ROUTED

// Kaohj
int showRip()
{
	unsigned int entryNum, i;
	MIB_CE_RIP_T Entry;
	char ifname[IFNAMSIZ];
	char *receive_mode, *send_mode;
	char mode_none[] = "None";
	char mode_rip1[] = "RIP1";
	char mode_rip2[] = "RIP2";
	char mode_both[] = "Both";
	char mode_rip1comp[] = "RIP1COMPAT";

	entryNum = mib_chain_total(MIB_RIP_TBL);
	printf("Index\tIface\tReceive\tSend\n");
	MSG_LINE;
	if (!entryNum) {
		printf("No data!\n\n");
		return 0;
	}

	for (i=0; i<entryNum; i++) {

		mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry);

		if( Entry.ifIndex == DUMMY_IFINDEX) {
			strncpy(ifname, BRIF, 6);
			ifname[5] = '\0';
		} else {
			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
		}

		if ( Entry.receiveMode == RIP_NONE ) {
			receive_mode = mode_none;
		} else if ( Entry.receiveMode == RIP_V1 ) {
			receive_mode = mode_rip1;
		} else if ( Entry.receiveMode == RIP_V2 ) {
			receive_mode = mode_rip2;
		} else if ( Entry.receiveMode == RIP_V1_V2 ) {
			receive_mode = mode_both;
		} else {
			receive_mode = mode_none;
		}

		if ( Entry.sendMode == RIP_NONE ) {
			send_mode = mode_none;
		} else if ( Entry.sendMode == RIP_V1 ) {
			send_mode = mode_rip1;
		} else if ( Entry.sendMode == RIP_V2 ) {
			send_mode = mode_rip2;
		} else if ( Entry.sendMode == RIP_V1_COMPAT ) {
			send_mode = mode_rip1comp;
		} else {
			send_mode = mode_none;
		}

		printf("%d\t%s\t%s\t%s\n", i+1, ifname, receive_mode, send_mode);
	}
	return(entryNum);
}

//tylo
unsigned int RIPsel[8];
int showItf(unsigned char sel)//0:show all interface without bridge   1://show rip interface 2:show all itf
{
	int ifnum=0;

	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
	char  buffer[3];

	// check WAN
	printf("Idx  Interface\n");
	printf("-------------------\n");
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	//printf("entrynum=%d\n",entryNum);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;
		//printf("cmode:%d  rip:%d\n",Entry.cmode,Entry.rip);
		if(sel==1){
			if (Entry.cmode != CHANNEL_MODE_BRIDGE && Entry.rip)
			{
				ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
				printf("%-5d%s\n",ifnum+1, ifname);
				RIPsel[ifnum]=Entry.ifIndex;
				ifnum++;
			}
		}
		else if(sel==0){
			if (Entry.cmode != CHANNEL_MODE_BRIDGE)
			{
				ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
				printf("%-5d%s\n",ifnum+1, ifname);
				RIPsel[ifnum]=Entry.ifIndex;
				ifnum++;
			}
		}
		else{
			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
			printf("%-5d%s\n",ifnum+1, ifname);
			RIPsel[ifnum]=Entry.ifIndex;
			ifnum++;
		}
	}
	//snprintf(buffer, 3, "%u", ifnum);
	//ejSetResult(eid, buffer);
	return ifnum;
}


#ifdef CONFIG_USER_ROUTED_ROUTED
//tylo
/* Return value:
 * 0: fail
 * 1: successful
 */
int RIPAddDel(){
	unsigned int min,max,sel,idxsel;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T vcEntry;
	MIB_CE_RIP_T ripEntry;


	min=1;max=2;
	if(getInputUint("(1)Add RIP Interface (2)Del RIP Interface :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if(sel==1){//add
		int intVal;
		max=showItf(0);
		if(max==0)
			return 0;
		if(getInputUint("Select index:",&idxsel,&min,&max)==0){
			return 0;
		}

		//mib_chain_get(MIB_ATM_VC_TBL, idxsel-1, (void *)&vcEntry);
		// get the selected VC
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<entryNum; i++) {

			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vcEntry);

			if (vcEntry.enable == 0)
				continue;
			if (vcEntry.cmode != CHANNEL_MODE_BRIDGE)
			{
				idxsel--;
				if (!idxsel)
					break;
			}
		}

		// check RIP table
		entryNum = mib_chain_total(MIB_RIP_TBL);
		for (i=0; i<entryNum; i++) {
			mib_chain_get(MIB_RIP_TBL, i, (void *)&ripEntry);
			if (ripEntry.ifIndex == vcEntry.ifIndex) {
				printf("Entry already exists!\n");
				printWaitStr();
				return 0;
			}
		}

		memset(&ripEntry, '\0', sizeof(MIB_CE_RIP_T));
		ripEntry.ifIndex = vcEntry.ifIndex;

		max = 4;
		if(getInputUint("Receive Mode (1)None (2)RIPv1 (3)RIPv2 (4)Both: ",&sel,&min,&max)==0){
			return 0;
		}
		ripEntry.receiveMode = sel-1;

		max = 4;
		if(getInputUint("Send Mode (1)None (2)RIPv1 (3)RIPv2 (4)RIP1COMPAT: ",&sel,&min,&max)==0){
			return 0;
		}
		ripEntry.sendMode = sel-1;
		if (sel == 4)
			ripEntry.sendMode = RIP_V1_COMPAT;

		intVal = mib_chain_add(MIB_RIP_TBL, (unsigned char*)&ripEntry);
		if (intVal == 0) {
			printf("Error! Add chain record.");
			return 0;
		}
		else if (intVal == -1) {
			printf("Error! Table Full.");
			return 0;
		}
	}
	else{//del
		max = showRip();
		if(max==0) {
			printWaitStr();
			return 0;
		}
		if(getInputUint("Select index:",&idxsel,&min,&max)==0){
			printf("Invalid selection!\n");
			return 0;
		}

		if(mib_chain_delete(MIB_RIP_TBL, idxsel-1) != 1)
			printf("Failed\n");
	}
	return 1;
}

//tylo
void setRIP()
{
	int snum, ret;
	unsigned char ripOn;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           RIP Configuration                             \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Enable the RIP if you are using this device as a RIP-enabled router to   \n");
		printf("communicate with others using the Routing Information Protocol. This page\n");
		printf("is used to select the interfaces on your device is that use RIP, and the  \n");
		printf("version of the protocol used.                                            \n");
		printf("-------------------------------------------------------------------------\n");
		mib_get(MIB_RIP_ENABLE, (void *)&ripOn);
		printf("RIP: %s\n", ripOn?STR_ENABLE:STR_DISABLE);
		printf("(1) RIP Enable/Disable         (2)Add/Del Interface\n");
		printf("(3) Show RIP interface         (4)Quit\n\n");
		if (!getInputOption( &snum, 1, 4))
			continue;
		ret = 0;
		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				ret = RIPconfig();
				break;
			case 2://(2)add/del
				if (!check_access(SECURITY_SUPERUSER))
					break;
				ret = RIPAddDel();
				break;
			case 3://(3)show
				showRip();
				printWaitStr();
				break;
			case 4://(4) Quit
				return;
		}//end switch, RIP Configuration

   	if(ret)
   	{
#if defined(APPLY_CHANGE)
	// Take effect in real time
			startRip();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
   	}
	}//end while, RIP Configuration
}
#endif	// of CONFIG_USER_ROUTED_ROUTED

#ifdef CONFIG_USER_MINIDLNA
static void setDMSSettings_show()
{
	unsigned int enable;

	printf("\n");
	MENU_LINE;

	// Mason Yu. use table not chain
	mib_get(MIB_DMS_ENABLE, (void *)&enable);

	printf( "Digital Media Server: %s\n", enable?"Enable":"Disable" );
	printf( "Directory : %s\n", "/mnt" );

	printWaitStr();
	return;
}

static void setDMSSettings_modify()
{
	unsigned int num,min,max,enable;
	unsigned char tmpbuf[80];

	// Mason Yu. use table not chain
	mib_get(MIB_DMS_ENABLE, (void *)&enable);

	printf("\n");
	MENU_LINE;

	/*enable*/
	min=1;max=2;
	if( getInputUint("Set DMS (1)Disable (2)Enable : ", &num, &min, &max)==0 ) return;
	if((num-1)==enable) goto setDMSSet_modify_error; //nothing to do
	enable=(num==1)?0:1;
	//if(enable==0) goto setDMSSet_modify_done;

setDMSSet_modify_done:
	mib_set(MIB_DMS_ENABLE, (void *)&enable);

	if(enable)
		startMiniDLNA();
	else
		stopMiniDLNA();

	return;

setDMSSet_modify_error:
	printWaitStr();
	return;
}

enum _SET_DMS_SETTINGS_IDX_
{
	SET_DMS_SETTINGS_SHOW,
	SET_DMS_SETTINGS_MODIFY,
	SET_DMS_SETTINGS_QUIT,
	SET_DMS_SETTINGS_END
};
static char *strSetDMSSettings[]=
{
	"Show",
	"Modify",
	"Quit",
	NULL
};

static void setDMSSettings()
{
	int snum;

	while (1)
   	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                      DMS Settings             \n");
		MENU_LINE;
		printf("This page is used to configure the parameters for your DMS\n");
		MENU_LINE;
		cli_show_menu( strSetDMSSettings, SET_DMS_SETTINGS_END );
		if (!getInputOption( &snum, 1, SET_DMS_SETTINGS_END))
			continue;

		switch(snum-1)
		{
		case SET_DMS_SETTINGS_SHOW:
			setDMSSettings_show();
			break;
		case SET_DMS_SETTINGS_MODIFY:
			setDMSSettings_modify();
			break;
		case SET_DMS_SETTINGS_QUIT:
			return;
		default:
			break;
		}
	}
}
#endif //CONFIG_USER_MINIDLNA

void setServices()
{
	int snum;
	int tmpi=0;;
	MENU_ITEM	*pm;
	unsigned char	buf[256];
	MENU_ITEM adv_menu[] = {
		#ifdef CONFIG_USER_DHCP_SERVER
		#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		{"DHCPv4", setDHCPPage},
		#else
		{"DHCP", setDHCPPage},
		#endif
		#endif
		{"DNS", setDNS},
		{"Firewall", setFirewall},
		#ifdef CONFIG_USER_IGMPPROXY
		{"IGMP Proxy", setIGMPProxy},
		#endif
		#if defined (CONFIG_USER_UPNPD) || defined (CONFIG_USER_MINIUPNPD)
		{"UPnP", setUPnP},
		#endif
		#ifdef CONFIG_USER_ROUTED_ROUTED
		{"RIP", setRIP},
		#endif
		#if defined (CONFIG_IPV6) && defined (CONFIG_USER_DHCPV6_ISC_DHCP411)
		{"DHCPv6", setDHCPv6Page},
		#endif
		#ifdef CONFIG_USER_MINIDLNA
		{"DMS", setDMSSettings},
		#endif
		{"",NULL}
	};

	while (1)
	{
		tmpi = 0;
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                (5) Services Menu                          \n");
		MSG_LINE;
		for (pm =&adv_menu[0] ;pm->fn!= NULL;pm++)
		{
			tmpi++;
			printf("(%d) %s",tmpi, pm->display_name);
			if (tmpi&0x01)
				printspace(33-4-strlen(pm->display_name));
			else
				printf("\n");
		}
		tmpi++;
		printf("(%d) Quit \n",tmpi);
		if (!getInputOption( &snum, 1, tmpi))
		      	continue;
		if (snum == tmpi)
			return;
		adv_menu[snum-1].fn();
	}
}

#if 0
void old_setServices()
{
	int snum;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-----------------------------------------------------------\n");
		printf("                (5) Services Menu                          \n");
		printf("-----------------------------------------------------------\n");
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		printf("(1) DHCPv4                       (2) DNS\n");
#else
		printf("(1) DHCP                         (2) DNS\n");
#endif
		printf("(3) Firewall                     (4) IGMP Proxy\n");
		printf("(5) UPnP                         (6) RIP\n");
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		printf("(7) DHCPv6 			 (8) Quit\n");
#else
		printf("(7) Quit\n");
#endif

		if (!getInputOption( &snum, 1, 8))
			continue;

		switch( snum)
		{
			case 1://(1) DHCP Server
				setDHCPPage();
				break;
			case 2://(2) DNS
				setDNS();
				break;
			case 3://(3) Firewall
				setFirewall();
				break;
#ifdef CONFIG_USER_IGMPPROXY
			case 4://(4) IGMP Proxy
				setIGMPProxy();
				break;
#endif
			case 5://(5) UPnP
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
				setUPnP();
#else
				printf("Not supported\n");
				printWaitStr();
#endif
				break;
#ifdef CONFIG_USER_ROUTED_ROUTED
			case 6://(6) RIP
				setRIP();
				break;
#endif

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
			case 7://(7) DHCPv6 Type
				setDHCPv6Page();
				break;
			case 8://(8) Quit
				return;
#endif
#else		// #ifdef CONFIG_IPV6
			case 7://(7) Quit
				return;
#endif
			default:
				printf("!! Invalid Selection !!\n");
		}//end switch, (5) Services Menu
	}//end while, (5) Services Menu
}
#endif
/*************************************************************************************************/
void showARPTable()
{
	FILE *fp;
	char arg1[20], arg2[20];
	int arg3;
	char strbuf[256];

	CLEAR;
	printf("\n");
	printf("-------------------------------------------------------------------------\n");
	printf("		   ARP Table 				 \n");
	printf("-------------------------------------------------------------------------\n");
	printf("This table shows a list of learned MAC addresses.\n");
	printf("-------------------------------------------------------------------------\n");
	printf("IP Address          MAC Address\n");
	printf("-------------------------------------------------------------------------\n");

	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL){
		printf("read arp file fail!\n");
		printWaitStr();
		return;
	}
	fgets(strbuf, 256, fp);
	while (fgets(strbuf, 256, fp)) {
		sscanf(strbuf, "%s%*x 0x%x%s", arg1, &arg3, arg2);
		if (!arg3)
			continue;
		printf("%-20s%s\n", arg1, arg2);
	}

	fclose(fp);
	printWaitStr();
}

//tylo
int br_socket_fd;
static void __dump_fdb_entry(struct __fdb_entry *f)
{
	unsigned long long tvusec;
	int sec,usec;

	// jiffies to tv
	tvusec = (1000000ULL*f->ageing_timer_value)/HZ;
	sec = tvusec/1000000;
	usec = tvusec - 1000000 * sec;

	printf("%-9i%.2x:%.2x:%.2x:%.2x:%.2x:%.2x  %-11s%4i.%.2i\n",
		f->port_no, f->mac_addr[0], f->mac_addr[1], f->mac_addr[2],
		f->mac_addr[3], f->mac_addr[4], f->mac_addr[5],
		f->is_local?"yes":"no", sec, usec/10000);
}

void showMACs()
{
	struct bridge *br;
	struct __fdb_entry fdb[256];
	int offset;
	unsigned long args[4];
	struct ifreq ifr;

   printf("\n");
   printf("-------------------------------------------------------------------------\n");
   printf("                       Bridge Forwarding Database Table                  \n");
   printf("-------------------------------------------------------------------------\n");
   printf("This table shows a list of learned MAC addresses for this bridge.        \n");
   printf("-------------------------------------------------------------------------\n");
   printf("Port No      MAC Address            Is Local?         Ageing Timer       \n");
   printf("-------------------------------------------------------------------------\n");

	offset = 0;
	args[0] = BRCTL_GET_FDB_ENTRIES;
	args[1] = (unsigned long)fdb;
	args[2] = 256;
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket not avaiable !!\n");
		return;
	}
//	memcpy(ifr.ifr_name, br->ifname, IFNAMSIZ);
	memcpy(ifr.ifr_name, BRIF, IFNAMSIZ);
	((unsigned long *)(&ifr.ifr_data))[0] = (unsigned long)args;
	while (1) {
		int i;
		int num;

		args[3] = offset;
		num = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

		if (num <= 0)
		{
			if (num < 0)
				printf("br0 interface not exists !!\n");
			break;
}

		for (i=0;i<num;i++)
			__dump_fdb_entry(fdb+i);

		offset += num;
	}
	close(br_socket_fd);

}

//tylo
void setBridging()
{
	int snum, ageingTime;
	unsigned short time;
	unsigned char stp;
	char msgstr[10];
	int min,max,sel;
#if defined(APPLY_CHANGE)
	char *argv[5];
	char str[10];
	//unsigned char str_cmd[100];
#endif

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           Bridge Configuration                          \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the bridge parameters. Here you can change\n");
		printf("the settings or view some information on the bridge and its attached     \n");
		printf("ports.                                                                   \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Set                          (2) Show MACs\n");
		printf("(3) Quit\n");
		if (!getInputOption( &snum, 1, 3))
		continue;

		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				mib_get(MIB_BRCTL_AGEINGTIME, (void *)&time);
				printf("Old Ageing Time: %d(seconds)\n",time);
				printf("New Ageing Time(seconds):");
				if( (ageingTime=getInputNum()) ==0) continue;
				time=(unsigned short)ageingTime;
				if ( mib_set(MIB_BRCTL_AGEINGTIME, (void *)&time) == 0) {
					printf("Set bridge ageing time MIB error!");
				}

#if defined(APPLY_CHANGE)
				sprintf(str,"%u",time);
				//printf("str=%s\n", str);
				argv[1]="setageing";
				argv[2]=(char*)BRIF;
				argv[3]=str;
				argv[4]=NULL;
				//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
				//printf("str_cmd=%s\n", str_cmd);
				do_cmd(BRCTL, argv, 1);
#endif

				mib_get(MIB_BRCTL_STP, (void *)&stp);
				if(stp==1)
					strcpy(msgstr,"Enabled");
				else
					strcpy(msgstr,"Disabled");
				min=1;max=2;
				printf("Old 802.1d Spanning Tree: %s\n",msgstr);
				printf("New 802.1d Spanning Tree (1)Enabled (2)Disabled:\n");
				if(getInputUint("Select index:",&sel,&min,&max)==0){
					printf("Invalid selection!\n");
					return;
				}
				if (sel== 1)
					stp = 1;
				else
					stp = 0;
				if ( !mib_set(MIB_BRCTL_STP, (void *)&stp)) {
					printf("Set STP mib error!");
				}
				else
				{
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif
				}

#if defined(APPLY_CHANGE)
				if (stp == 1)	// on
				{	// brctl setfd br0 20
					argv[1]="setfd";
					argv[2]=(char*)BRIF;
					argv[3]="20";
					argv[4]=NULL;
					//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
					//printf("str_cmd=%s\n", str_cmd);
					do_cmd(BRCTL, argv, 1);
				}

				argv[1]="stp";
				argv[2]=(char*)BRIF;

				if (stp == 0)
					argv[3]="off";
				else
					argv[3]="on";

				argv[4]=NULL;
				//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
				//printf("str_cmd=%s\n", str_cmd);
				do_cmd(BRCTL, argv, 1);
#endif
				break;
			case 2://(2) Show MACs
				showMACs();
				printWaitStr();
				break;
			case 3://(3) Quit
				return;
		}//end switch, Bridge Configuration
	}//end while, Bridge Configuration
}

//tylo
void showRoutes()
{

	int nBytesSent=0;
	char buff[256];
	int flgs;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16], iface[6];
	FILE *fp;

	CLEAR;
	printf("\n");
	printf("-------------------------------------------------------------------------\n");
	printf("                       IP Route Table                                    \n");
	printf("-------------------------------------------------------------------------\n");
	printf("This table shows a list of destination routes commonly accessed by your  \n");
	printf("network.                                                                 \n");
	printf("-------------------------------------------------------------------------\n");
	printf("Destination      Subnet Mask      NextHop          Iface\n");
	printf("-------------------------------------------------------------------------\n");

	if (!(fp=fopen("/proc/net/route", "r"))) {
		fclose(fp);
		printf("Error: cannot open /proc/net/route !!\n");
		printWaitStr();
		return;
	}


	fgets(buff, sizeof(buff), fp);

	while( fgets(buff, sizeof(buff), fp) != NULL ) {
		if(sscanf(buff, "%s%lx%lx%X%*d%*d%*d%lx",
		   iface, &d, &g, &flgs, &m)!=5) {
			printf("Error: Unsuported kernel route format !!\n");
			printWaitStr();
			return;
		}

		if(flgs & RTF_UP) {
			dest.s_addr = d;
			gw.s_addr   = g;
			mask.s_addr = m;
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(sdest, inet_ntoa(dest));
			strcpy(sgw,  (gw.s_addr==0   ? "*" : inet_ntoa(gw)));

			printf("%-17s%-17s%-17s%s\n",sdest, inet_ntoa(mask), sgw, iface);
		}
	}

	fclose(fp);
	printWaitStr();
	return;
}

#ifdef ROUTING
//tylo
int showStaticRoutes()
{
	unsigned int entryNum, i;
	MIB_CE_IP_ROUTE_T Entry;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16];
	char interface_name[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);

	printf("\n-------------------------------------------------------------------------------\n");
	printf("                       Static Route Table                                    \n");
	printf("-------------------------------------------------------------------------------\n");
	//printf("Idx  Subnet Mask      NextHop          Iface\n");
	printf("Idx  State    Destination      Subnet Mask      NextHop          Metric   Iface\n");
	printf("-------------------------------------------------------------------------------\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		dest.s_addr = *(unsigned long *)Entry.destID;
		gw.s_addr   = *(unsigned long *)Entry.nextHop;
		mask.s_addr = *(unsigned long *)Entry.netMask;
		// inet_ntoa is not reentrant, we have to
		// copy the static memory before reuse it
		strcpy(sdest, inet_ntoa(dest));
		strcpy(sgw, inet_ntoa(gw));
		if (!ifGetName(Entry.ifIndex, interface_name, sizeof(interface_name)))
			strcpy( interface_name, "---" );
		//printf("%-5d%-17s%-17s%s\n",i+1, sdest, inet_ntoa(mask), sgw);
		printf("%-5d%-9s%-17s%-17s%-17s%-9d%s\n", i+1, Entry.Enable ? "Enable" : "Disable", sdest, inet_ntoa(mask), sgw, Entry.FWMetric, interface_name);
	}

	return 0;
}
#endif

int routeIflist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char selcnt = 0;
	char wanif[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	printf("0    Any\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		ifGetName(Entry.ifIndex, wanif, sizeof(wanif));
		selcnt ++;
		printf("%-5d%s\n", selcnt, wanif);
		ifnum ++;
	}

	return ifnum;
}

#ifdef ROUTING
//tylo
//void addroutes(char *destNet,char *subMask,char *nextHop){
void addroutes()
{
	MIB_CE_IP_ROUTE_T entry;
	MIB_CE_ATM_VC_T vcEntry;
	int min, max, num, intVal, i;
	int totalEntry = 0;
	unsigned char destNet[4];

	if (0 == getInputIpAddr("Destination IP address: ", (struct in_addr *)&entry.destID))
		return;
	if (0 == getInputIpMask("Subnet Mask: ", (struct in_addr *)&entry.netMask))
		return;
	for (i=0; i<4; i++) {
		destNet[i] = entry.destID[i] & entry.netMask[i];
		if (destNet[i] != entry.destID[i]) {
			printf("The specified mask parameter is invalid. (Destination & Mask) != Destination.\n");
			return;
		}
	}

	min = 0; max = 65535;
	intVal=getTypedInputDefault(INPUT_TYPE_UINT,"Metric[1]: ",&num,&min,&max);
	if (intVal == 0)
		return;
	else if (intVal == -2)
		entry.FWMetric = 1;
	else
		entry.FWMetric = num;

	max = routeIflist();
	min = 0;
	if (getInputUint("Select interface: ", &num, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}
	if (num == min) {
		entry.ifIndex = DUMMY_IFINDEX;
		if (0 == getInputIpAddr("Next Hop: ", (struct in_addr *)&entry.nextHop))
			return;
	}
	else {
		mib_chain_get(MIB_ATM_VC_TBL, num-1, (void *)&vcEntry);
		entry.ifIndex = vcEntry.ifIndex;
		getInputIpAddr("Next Hop: ", (struct in_addr *)&entry.nextHop);
	}

	max=2;
	if (0 == getInputUint("State (1)Enable (2)Disable : ", &num, &min, &max))
		return;
	if (num==1)
		entry.Enable = 1;
	else
		entry.Enable = 0;

	if (!checkRoute(entry, -1)) {
		printf("Invaild! This is a duplicate or conflicting rule !");
		printf("\n");
		return;
	}

	intVal = mib_chain_add(MIB_IP_ROUTE_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		return;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		return;
	}
	totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL);
	route_cfg_modify(&entry, 0, totalEntry-1);
}

//tylo
void delroutes(){

	unsigned int i;
	int min,max,sel,skfd;
	unsigned int idx;
	MIB_CE_IP_ROUTE_T Entry;
	struct rtentry rt;
	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	struct sockaddr_in *s_in;

	CLEAR;
	showStaticRoutes();

	min=1;max=totalEntry;
	if(getInputUint("Select index:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	idx = sel-1;

	/* get the specified chain record */
	if (!mib_chain_get(MIB_IP_ROUTE_TBL, idx, (void *)&Entry))
		return;
	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	rt.rt_flags = RTF_UP;
	s_in = (struct sockaddr_in *)&rt.rt_dst;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.destID;

	s_in = (struct sockaddr_in *)&rt.rt_genmask;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.netMask;

	s_in = (struct sockaddr_in *)&rt.rt_gateway;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.nextHop;

	rt.rt_flags |= RTF_GATEWAY;

	// delete from chain record
	if(mib_chain_delete(MIB_IP_ROUTE_TBL, idx) != 1) {
		printf("Delete chain record error!");
		return;
	}

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Create socket error!\n");
		return;
	}

	/* Tell the kernel to delete this route. */
	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
		printf("kernel delete route error!\n");
		close(skfd);
		return;
	}

	/* Close the socket. */
	(void) close(skfd);
	return;
}
#endif

#ifdef DEFAULT_GATEWAY_V2
//Jenny
unsigned int DGWsel[8];
int DGWlist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	int type=1;//rt
	unsigned char DGWselcnt = 0;
	char wanif[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	printf("0    None\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if (type == 2) {
			if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			{
				DGWsel[DGWselcnt] = Entry.ifIndex;
				DGWselcnt ++;
				printf("%-5dvc%u\n",	DGWselcnt, VC_INDEX(Entry.ifIndex));
				ifnum ++;
			}
		}
		else { // rt or all (1 or 0)
			if (type == 1 && Entry.cmode == CHANNEL_MODE_BRIDGE)
				continue;
			ifGetName(Entry.ifIndex, wanif, sizeof(wanif));
			DGWsel[DGWselcnt] = Entry.ifIndex;
			DGWselcnt ++;
			printf("%-5d%s\n", DGWselcnt, wanif);
			ifnum ++;
		}
	}
#ifdef AUTO_PPPOE_ROUTE
	ifnum ++;
	printf("%d    Auto\n", ifnum);
#endif

	return ifnum;
}

//Jenny
void showDGW(){
	unsigned int dgw;
	char wanif[IFNAMSIZ];

	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);
	if (dgw == DGW_NONE)
		printf("Default Gateway: None\n");
#ifdef AUTO_PPPOE_ROUTE
	else if (dgw == DGW_AUTO)
		printf("Default Gateway: Auto\n");
#endif
	else {
		ifGetName(dgw, wanif, sizeof(wanif));
		printf("Default Gateway: %s\n", wanif);
	}
}

//Jenny
void setDGW(){
	int min, max, sel;
	unsigned int dgw_itf;

	max = DGWlist();
	min = 0;
	if(getInputUint("Select interface: ", &sel, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (sel == min)
		dgw_itf = DGW_NONE;
#ifdef AUTO_PPPOE_ROUTE
	else if (sel == max)
		dgw_itf = DGW_AUTO;
#endif
	else
		dgw_itf = DGWsel[sel-1];

	if ( !mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw_itf)) {
		printf("Set default gateway interface index error!\n");
		return;
	}
	return;
}
#endif

#ifdef ROUTING
//tylo
void setRouting()
{
	//char destNet[16], subMask[16], nextHop[16];
	int snum;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                           Routing Configuration                         \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the routing information. Here you can     \n");
		printf("add/delete IP routes.                                                    \n");
		//printf("-------------------------------------------------------------------------\n");
#ifdef DEFAULT_GATEWAY_V2
		printf("-------------------------------------------------------------------------\n");
		showDGW();
#endif
		//printf("Static Route Table:\n");
		//printf("Select Destination Subnet Mask NextHop\n");
		showStaticRoutes();
		printf("-------------------------------------------------------------------------------\n");
		printf("(1) Add                          (2) Delete\n");
#ifdef DEFAULT_GATEWAY_V2
		printf("(3) Show Routes                  (4) Set Default Gateway\n");
		printf("(5) Quit\n");
		if (!getInputOption( &snum, 1, 5))
#else
		printf("(3) Show Routes                  (4) Quit\n");
		if (!getInputOption( &snum, 1, 4))
#endif
			continue;

		switch( snum)
		{
			case 1://(1) Add
#ifdef ROUTING
				if (!check_access(SECURITY_SUPERUSER))
					break;
				CLEAR;
				addroutes();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
#else
				printf("Not supported\n");
				printWaitStr();
#endif
				break;
			case 2://(2) Delete
#ifdef ROUTING
				if (!check_access(SECURITY_SUPERUSER))
					break;
				delroutes();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
#else
				printf("Not supported\n");
				printWaitStr();
#endif
				break;
			case 3://(3) Show Routes
				showRoutes();
				break;
#ifdef DEFAULT_GATEWAY_V2
			case 4://(4) Set Default Gateway
				setDGW();
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 5://(5) Quit
#else
			case 4://(4) Quit
#endif
				return;
		}//end switch, Routing Configuration
	}//end while, Routing Configuration
}
#endif

//tylo
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
void SNMPconfig(char *snmpSysDescr,char *snmpSysContact,char *snmpSysName,char *snmpSysLocation,
				char *snmpSysObjectID,char *snmpTrapIpAddr,char *snmpCommunityRO,char *snmpCommunityRW){
	char *str, *submitUrl;
	struct in_addr trap_ip;
	static char tmpBuf[100];

	str = snmpTrapIpAddr;
	if(str[0]==0)
		goto setErr_pass;

	if ( !inet_aton(str, &trap_ip) ) {
		printf("Invalid Trap IP value!\n");
		goto setErr_pass;
	}
	if ( !mib_set(MIB_SNMP_TRAP_IP, (void *)&trap_ip)) {
		printf("Set snmpTrapIpAddr mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysObjectID;
	if(str[0]==0)
		goto setErr_pass;

	if(!validateKey(str)){
		printf("Invalid Object ID value. It should be fill with OID string.\n");
		goto setErr_pass;
	}
	if ( !mib_set(MIB_SNMP_SYS_OID, (void *)str)) {
		printf("Set snmpSysObjectID mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysDescr;
	if(str[0]==0)
		goto setErr_pass;
	if ( !mib_set(MIB_SNMP_SYS_DESCR, (void *)str)) {
		printf("Set snmpSysDescr mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysContact;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_SYS_CONTACT, (void *)str)) {
		printf("Set snmpSysContact mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysName;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_SYS_NAME, (void *)str)) {
		printf("Set snmpSysName mib error!\n");
		goto setErr_pass;
	}

	str = snmpSysLocation;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_SYS_LOCATION, (void *)str)) {
		printf("Set snmpSysLocation mib error!\n");
		goto setErr_pass;
	}

	str = snmpCommunityRO;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_COMM_RO, (void *)str)) {
		printf("Set snmpCommunityRO mib error!\n");
		goto setErr_pass;
	}

	str = snmpCommunityRW;
	if(str[0]==0)
		goto setErr_pass;

	if ( !mib_set(MIB_SNMP_COMM_RW, (void *)str)) {
		printf("Set snmpCommunityRW mib error!\n");
		goto setErr_pass;
	}

	printf("Set SNMP config success!\n");
  	return;

 setErr_pass:
	printf("Set SNMP config fail!\n");

}

void showSNMP(){
	char str[256];
	struct in_addr trap_ip;
	unsigned char enable;

	if ( !mib_get( MIB_SNMPD_ENABLE, (void *)&enable)) {
		printf("Get snmpdEnable error!\n");
	}
	printf("SNMP:%s\n", (enable==1)? "enable":"disable");

	if ( !mib_get(MIB_SNMP_SYS_DESCR, (void *)str)) {
		printf("Get snmpSysDescr mib error!\n");
	}
	printf("System Description:%s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_CONTACT, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Contact:%s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_NAME, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Name:%s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_LOCATION, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Location:%s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_OID, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Object ID:%s\n",str);

	if ( !mib_get(MIB_SNMP_TRAP_IP, (void *)&trap_ip)) {
		printf("Get mib error!\n");
	}
	printf("Trap IP Address:%s\n",inet_ntoa(trap_ip));

	if ( !mib_get(MIB_SNMP_COMM_RO, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("Community name (read-only):%s\n",str);

	if ( !mib_get(MIB_SNMP_COMM_RW, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("Community name (read-write):%s\n\n",str);

}


void setSNMP()
{
	char snmpSysDescr[65], snmpSysContact[65], snmpSysName[65], snmpTrapIpAddr[16];
	char snmpSysLocation[65], snmpSysObjectID[65], snmpCommunityRO[65], snmpCommunityRW[65];
	int snum, enable;
	unsigned char set_enable;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      SNMP Protocol Configuration                        \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the SNMP protocol. Here you may change the\n");
		printf("setting for system description, trap ip address, community name, etc.. \n");
		printf("-------------------------------------------------------------------------\n");
		showSNMP();

		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;
				// Mason Yu
				printf("SNMP:(1) enable (2) disable\n");
				if (!getInputOption( &enable, 1, 2))
					continue;
				if ( enable == 1 )
					set_enable = 1;   // on
				else
					set_enable = 0;   // off

				mib_set(MIB_SNMPD_ENABLE, (void *)&set_enable);

				if ( enable == 1 )
				{
					if (0 == getInputString("System Description :", snmpSysDescr, sizeof(snmpSysDescr)-1))
						continue;
					if (0 == getInputString("System Contact :", snmpSysContact, sizeof(snmpSysContact)-1))
						continue;
					if (0 == getInputString("System Name :", snmpSysName, sizeof(snmpSysName)-1))
						continue;
					if (0 == getInputString("System Location :", snmpSysLocation, sizeof(snmpSysLocation)-1))
						continue;
					if (0 == getInputString("System Object ID :", snmpSysObjectID, sizeof(snmpSysObjectID)-1))
						continue;
					if (0 == getInputString("Trap IP Address:", snmpTrapIpAddr,sizeof(snmpTrapIpAddr)-1))
						continue;
					if (0 == getInputString("Community name (read-only) :", snmpCommunityRO, sizeof(snmpCommunityRO)-1))
						continue;
					if (0 == getInputString("Community name (read-write) :", snmpCommunityRW, sizeof(snmpCommunityRW)-1))
						continue;

					SNMPconfig(snmpSysDescr, snmpSysContact, snmpSysName, snmpSysLocation, snmpSysObjectID, snmpTrapIpAddr
						, snmpCommunityRO, snmpCommunityRW);
				}

#if defined(APPLY_CHANGE)
				if ( enable == 1 )  // on
					restart_snmp(1);
				else
					restart_snmp(0);  // off
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2://(2) Quit
				return;
		}//end switch, SNMP Protocol Configuration
	}//end while, SNMP Protocol Configuration
}
#endif

//tylo
unsigned int BridgeVC[8];
int showBridgeVC()
{
	int ifnum=0;

	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
	char  buffer[3];

	// check WAN
	printf("Idx  Interface\n");
	printf("-------------------\n");
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;


		if (Entry.cmode == CHANNEL_MODE_BRIDGE)
		{
			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
			printf("%-5d%s\n",ifnum+1, ifname);
			BridgeVC[ifnum]=Entry.ifIndex;
			ifnum++;
		}
	}
	return ifnum;
}

// Jenny
unsigned int WanVC[16];
//int showWanVC()
int showWanVC(int dft)
{
	int ifnum=0;

	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
	char  buffer[3];

	// check WAN
	printf("Idx  Interface\n");
	printf("-------------------\n");
	if (dft)
		printf("0    Default\n");
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;


		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
		printf("%-5d%s\n",ifnum+1, ifname);
		WanVC[ifnum]=Entry.ifIndex;
		ifnum++;
	}
	return ifnum;
}

#ifdef ITF_GROUP
// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4

// int action=0: printf all interface
// int action=1: printf all interface to be added
// int action=2: printf all interface to be removed
int display_grp_ifName(int grp, int action)
{
	int i, ifnum, num;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	char availitf[512];
	char tmp[5];

	ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, grp);
	availitf[0]=0;
	if (ifnum>=1) {
		strncat( availitf, "(1)", 64);
		strncat( availitf, itfs[0].name, 64);

		ifnum--;
		for (i=0; i<ifnum; i++) {
			sprintf(tmp, "(%d)", i+2);
			strncat(availitf, tmp, 64);
			strncat(availitf, itfs[i+1].name, 64);
		}
		++ifnum;
	}

	if ( action == PM_PRINT ) {
		if ( availitf[0] == 0 )
			printf("The Group %d does not include any interface.\n", grp);
		else
			printf("The Group %d include %s interfaces.\n", grp, availitf);
	} else if ( action == PM_ADD ) {
		if ( availitf[0] == 0 )
			printf("There is no any available interface that can be added into Group %d.\n", grp);
		else
			printf("Please select one of the following interfaces and add it into Group.\n%s\n", availitf);
	} else if  ( action == PM_REMOVE ) {
		if ( availitf[0] == 0 )
			printf("There is no any available interface that can be removed from Group %d.\n", grp);
		else
			printf("Please select one of the following interfaces and remove it from Group %d.\n%s\n", grp, availitf);
	} else
		printf("display_grp_ifName: error action.\n");

	return ifnum;

}

void config_grp(enum PortMappingGrp grp){
	int snum, num;
	char *default_IFID, *select_IFID;

	while (1)
   	{
   		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      Group %d Configuration                        \n", grp);
		printf("-------------------------------------------------------------------------\n");
		// Get all interface from specify gruop
       		display_grp_ifName(grp, PM_PRINT);
		printf("\n");
		printf("Please select one of the following methods.\n");
       		printf("(1) Add interface				(2) Remove Interface\n");
       		printf("(3) Quit\n");
       		if (!getInputOption( &snum, 1, 3))
       			continue;
   		switch( snum)
		{
			case 1: // (1) Add interface into specify group
				if ( get_port_mapping_status() == 1 ) {
					num = display_grp_ifName(PM_DEFAULTGRP, PM_ADD);
					if ( num >= 1 ) {
						if (!getInputOption( &snum, 1, num))
       							continue;

       						// set specify Group
       						select_IFID = get_specify_one_IFID(PM_DEFAULTGRP, snum);  // get select IFID for default Group
       						setgroup(select_IFID, grp);
       						free(select_IFID);

       						// set default Group
       						default_IFID = get_all_IFID(PM_DEFAULTGRP);
       						setgroup(default_IFID, PM_DEFAULTGRP);
       						free(default_IFID);
       					} else
       						printWaitStr();

       					// Take effect
       					setupEth2pvc();
       				} else {
       					printf("Please enable Port Mapping first.\n");
       					printWaitStr();
       				}
         			break;
         		case 2: // (2) Remove Interface from specify group
         			if ( get_port_mapping_status() == 1 ) {
         				num = display_grp_ifName(grp, PM_REMOVE);
         				if ( num >= 1 ) {
						if (!getInputOption( &snum, 1, num))
       							continue;

       						del_IFID_from_group(grp, snum);
       					} else
       						printWaitStr();

       					// Take effect
       					setupEth2pvc();
       				} else {
       					printf("Please enable Port Mapping first.\n");
       					printWaitStr();
       				}
         			break;
     	 		case 3: //(3) Quit
         			return;
      		}//end switch
   	} // end while
}

//tylo
void eth2pvclist()
{
	unsigned int entryNum, i;
	MIB_CE_SW_PORT_T Entry;
	char wanif[IFNAMSIZ];
	char strbuf[256];

	entryNum = mib_chain_total(MIB_SW_PORT_TBL);
	getSYS2Str(SYS_PORT_MAPPING, strbuf);
	printf("Port Mapping\t: %s\n\n", strbuf);
	printf("Current Port Mapping\n");
	printf("---------------------\n");
	for (i=1; i<=entryNum; i++) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, entryNum-i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}
		if (!ifGetName(Entry.pvcItf, wanif, sizeof(wanif)))
			printf("LAN%d   Default\n",i);
		else
			printf("LAN%d   %s\n",i, wanif);
	}
	printf("---------------------\n");
}

void setPortMapping()
{
	int snum;
	unsigned char mode;

	CLEAR;
	eth2pvclist();
	if (!check_access(SECURITY_SUPERUSER))
		return;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      Port Mapping Configuration                         \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the Port mapping. Here you may change the \n");
		printf("Group interface with ADD/Remove method\n");
		printf("-------------------------------------------------------------------------\n");
		printf("Port Mapping: %s\n", (get_port_mapping_status())? "enabled":"disabled");
		printf("\n");
		printf("(1) Enable/Disable Port Mapping			(2) Config Group 1\n");
		printf("(3) Config Group 2				(4) Config Group 3\n");
		printf("(5) Config Group 4				(6) Quit\n");
		if (!getInputOption( &snum, 1, 6))
			continue;

		switch( snum)
		{
			case 1://(1) Enable/Disable Port Mapping
				if (!check_access(SECURITY_SUPERUSER))
					break;

				printf("Port Mapping:(1) enable (2) disable\n");
				if (!getInputOption( &snum, 1, 2))
					continue;

				mib_get(MIB_MPMODE, (void *)&mode);
				if ( snum == 1 ) {
					mode |= MP_PMAP_MASK;
				}else
					mode &= (~MP_PMAP_MASK);
				mib_set(MIB_MPMODE, (void *)&mode);

				if(mode&MP_PMAP_MASK)
					setupEth2pvc();
				else {
					setupEth2pvc_disable();
				}
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2: // (2) Config Group 1
				config_grp(PM_GROUP1);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 3: // (3) Config Group 2
				config_grp(PM_GROUP2);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 4: // (4) Config Group 3
				config_grp(PM_GROUP3);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 5: // (5) Config Group 4
				config_grp(PM_GROUP4);
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 6://(2) Quit
				return;
		}//end switch, Port Mappping Configuration
	}//end while, Port Mapping Configuration
}

#endif	// of ITF_GROUP

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
//tylo
void showLinkmode()
{
	int i;
	MIB_CE_SW_PORT_T Entry;

	printf("Current Link Mode\n");
	for(i=0;i<SW_LAN_PORT_NUM;i++){
		printf("LAN%d: ", virt2user[i]);
		if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Entry))
			continue;
		if(Entry.linkMode==LINK_10HALF)
			printf("10 Half Mode\n");
		else if(Entry.linkMode==LINK_10FULL)
			printf("10 Full Mode\n");
		else if(Entry.linkMode==LINK_100HALF)
			printf("100 Half Mode\n");
		else if(Entry.linkMode==LINK_100FULL)
			printf("100 Full Mode\n");
		else
			printf("Auto Mode\n");
	}
}

//tylo
void setLink(){
	int min,max,sel,sel2;
	MIB_CE_SW_PORT_T Entry;
	char strLanSel[128];
	int i;

	min = 1; max = SW_LAN_PORT_NUM;
	snprintf(strLanSel, 128, "Select LAN port");
	for (i=1; i<=SW_LAN_PORT_NUM; i++) {
		snprintf(strLanSel, 128, "%s (%d)LAN%d", strLanSel, i, i);
	}
	snprintf(strLanSel, 128, "%s :", strLanSel);
	if(getInputUint(strLanSel,&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	min=1;max=5;
	if(getInputUint("Select switch port (1)10 Half (2)10 Full (3)100 Half (4)100 Full (5)Auto Mode :",&sel2,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	for (i=0; i<SW_LAN_PORT_NUM; i++) {
		if (virt2user[i] == sel)
			break;
	}
	if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Entry))
		return;
	Entry.linkMode = sel2-1;
	mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, i);
	return;
}

//tylo
void setLinkmode()
{
	int snum;
	unsigned char vChar;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                     ethernet Link Speed/Duplex Mode                     \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Set the Ethernet link speed/duplex mode. \n ");
		printf("-------------------------------------------------------------------------\n");
		showLinkmode();
		printf("------------------------------------\n");
		printf("(1) Set link			(2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch(snum)
		{
			case 1:
				setLink();
#if defined(APPLY_CHANGE)
				setupLinkMode();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2:
				return;
		}
	}
}
#endif	// of ELAN_LINK_MODE

#else // of CONFIG_RTL_MULTI_LAN_DEV

#ifdef ELAN_LINK_MODE_INTRENAL_PHY
//tylo
void showLinkmode(){
	int i;
	unsigned char mode;
	printf("Current Link Mode\n");
	if (!mib_get(MIB_ETH_MODE, (void *)&mode))
		return;
	if(mode==LINK_10HALF)
		printf("10 Half Mode\n");
	else if(mode==LINK_10FULL)
		printf("10 Full Mode\n");
	else if(mode==LINK_100HALF)
		printf("100 Half Mode\n");
	else if(mode==LINK_100FULL)
		printf("100 Full Mode\n");
	else
		printf("Auto Mode\n");

}

//tylo
void setLink(){
	int min,max,sel,sel2;
	unsigned char mode;
	//MIB_CE_SW_PORT_T Entry;

	min=1;max=5;

	if(getInputUint("Select switch port (1)10 Half (2)10 Full (3)100 Half (4)100 Full (5)Auto Mode :",&sel2,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	mode = (unsigned char)(sel2 - 1);
	mib_set(MIB_ETH_MODE, (void *)&mode);

	restart_ethernet(1);
}

//tylo
void setLinkmode(){
	int snum;
	unsigned char vChar;
	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                     Ethernet Link Speed/Duplex Mode                     \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Set the Ethernet link speed/duplex mode. \n ");
		printf("-------------------------------------------------------------------------\n");
		showLinkmode();
		printf("------------------------------------\n");
		printf("(1) Set link			(2) Quit\n");

		if (!getInputOption( &snum, 1, 2))
			continue;

		switch(snum){
			case 1:
				setLink();
				break;
			case 2:
				return;
		}
	}
}
#endif

#endif	// of CONFIG_RTL_MULTI_LAN_DEV

//tylo
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
#ifdef NEW_IP_QOS_SUPPORT
int QosTcList()
{
	MIB_CE_IP_TC_T Entry;
	unsigned int entryNum, i;
	char ifname[IFNAMSIZ];
	char *psip, *pdip, sip[20], dip[20];
	const char *type;
	char sport[10], dport[10];
	char rate[10], index[10];

	printf("%-5s%-6s%-19s%-8s%-19s%-8s%-8s%-8s\n", "Idx","Inf","Src IP","SrcPort","Dst IP","DstPort","Proto","Rate");
 	//printf("Idx  Wan Inf  Src IP               Src Port  Des IP               Des Port  Protocol Rate\n");

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_QOS_TC_TBL, i, (void *)&Entry))
		{
 			printf("Get chain record error!\n");
			return -1;
		}

		//index
		snprintf(index, 10, "%d", Entry.entryid);

		//wan interface
		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));

		//source ip
		psip = inet_ntoa(*(struct in_addr *)&Entry.srcip);
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbits==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbits);
			psip = sip;
		}

		// source port
		if (Entry.sport == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sport);

		// destination ip
		pdip = inet_ntoa(*(struct in_addr *)&Entry.dstip);
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbits==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbits);
			pdip = dip;
		}

		// destination port
		if (Entry.dport == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dport);

		 if ( Entry.protoType == 1 )
			type = ARG_ICMP;
		if ( Entry.protoType == 2 )
			type = ARG_TCP;
		else if ( Entry.protoType == 3 )
			type = ARG_UDP;
		else if ( Entry.protoType == 4 )
			type = ARG_TCPUDP;
		else
			type = (char *)BLANK;

		snprintf(rate, 10, "%d", Entry.limitSpeed);

		printf("%-5s%-6s%-19s%-8s%-19s%-8s%-8s%-8s\n", index, ifname, psip, sport, pdip, dport, type, rate);
	}

	return entryNum;
}

unsigned int tcSelIfindex[8];
int QosTcInflist()
{
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum, i;
	int ifnum=0;
	char wanif[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			continue;

		ifGetName(Entry.ifIndex, wanif, sizeof(wanif));
		tcSelIfindex[ifnum++]=Entry.ifIndex;
		printf("%-5d%s\n",
			ifnum, wanif);
	}

	return ifnum;
}

void QosTcadd()
{
	char strSAddr[18], strDAddr[18], strSport[10], strDport[10];
	int intVal, retVal;
	int min,max,sel;
	unsigned int  totalEntry, i;
	MIB_CE_IP_TC_T entry;
	unsigned long  mask, mbit;
	unsigned char vChar;
	char strVal[5];
	char tmpstr[256] = {0};
	const char *pPrio;
	struct itfInfo itfs[16];
	int type, ifnum = 0, itfid, itfdomain;
	struct in_addr ipaddr;
	unsigned char map[MAX_QOS_RULE+1]={0};
	int entryid = 1;

	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_IP_QOS_TC_TBL); /* get chain record size */

	/* Add new qos entry */
	if (totalEntry >= MAX_QOS_RULE)
	{
		printf("Error: Maximum number of Qos rule exceeds !");
		goto setErr_qos;
	}

	//allocate a free rule id for new entry
	for(i=0; i<totalEntry;i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
			continue;
		map[entry.entryid] = 1;
	}
	for(i=1;i<=MAX_QOS_RULE;i++)
	{
		if(!map[i])
		{
			entryid = i;
			break;
		}
	}
	entry.entryid = entryid;

	//interface
	max=QosTcInflist();
	min=1;
	if(getInputUint("Select interface:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}
	entry.ifIndex = tcSelIfindex[sel-1];

	// protocol type
	min=0;max=4;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Protocol: (0)NONE (1)ICMP (2)TCP (3)UDP (4)TCP/UDP: ", &sel, &min, &max);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.protoType = 0;
	else
		entry.protoType = (unsigned char)sel;

	//source IP
	strSAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Source IP[None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	}else if (retVal==1) {
		strcpy(strSAddr, inet_ntoa(ipaddr));
	}

	// Source address
	if (strSAddr[0]) {
		if (!isValidIpAddr(strSAddr)) {
			printf("Invalid IP address %s.\n", strSAddr);
			goto setErr_qos;
		}
		inet_aton(strSAddr, (struct in_addr *)&entry.srcip);
		if (entry.srcip == 0) {
			printf("Error! Source IP.");
			goto setErr_qos;
		}

		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Source Netmask[None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strSAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strSAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strSAddr);
				goto setErr_qos;
			}
			inet_aton(strSAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.smaskbits = mbit;
		} else if (retVal == -2)
			entry.smaskbits = 32;
	} else {
		memset(entry.srcip, 0, IP_ADDR_LEN);
		entry.smaskbits = 0;
	}

	// Destination address
	strDAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Destination IP[None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	} else if (retVal == 1)
		strcpy(strDAddr, inet_ntoa(ipaddr));

	if (strDAddr[0]) {
		if (!isValidIpAddr(strDAddr)) {
			printf("Invalid IP address %s.\n", strDAddr);
			goto setErr_qos;
		}
		inet_aton(strDAddr, (struct in_addr *)&entry.dstip);
		if (entry.dstip==0) {
			printf("Error! Destination IP.");
			goto setErr_qos;
		}

		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Destination Netmask[None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strDAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strDAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strDAddr);
				goto setErr_qos;
			}
			inet_aton(strDAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.dmaskbits = mbit;
		} else if (retVal == -2)
			entry.dmaskbits = 32;
	} else {
		memset(entry.dstip, 0, IP_ADDR_LEN);
		entry.dmaskbits = 0;
	}

	// source port
	if (entry.protoType >= 2) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Source Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.sport =0;
		else
			entry.sport = (unsigned short)sel;
	} else {
		entry.sport = 0;
	}

	// destination port
	if (entry.protoType >= 2) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Destination Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.dport =0;
		else
			entry.dport = (unsigned short)sel;
	} else {
		entry.dport = 0;
	}

	//uplink rate
	min=0;max=0xffffffff;
	if (0 == getInputUint("Uplink Rate(0): ", &entry.limitSpeed, &min, &max)) {
		printf("Invalid value!\n");
		printWaitStr();
		return;
	}

	intVal = mib_chain_add(MIB_IP_QOS_TC_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		goto setErr_qos;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		goto setErr_qos;
	}
#if defined(APPLY_CHANGE)
	/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
#endif

	return;
setErr_qos:
	printf("Set QoS rule error!\n");
	printWaitStr();

}
#endif

#ifdef NEW_IP_QOS_SUPPORT
void QosTcdel()
{
	int min,max,sel,sel2;

	min=1;max=2;
	if(getInputUint("(1) Delete one  (2) Delete all: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(sel==1){//delete one
		min=1;max=QosTcList();
		if(getInputUint("Select index :",&sel2,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_IP_QOS_TC_TBL, sel2-1) != 1) {
			printf("Delete chain record error!");
		}

	}
	else{//delete all
		mib_chain_clear(MIB_IP_QOS_TC_TBL); /* clear chain record */
	}
#if defined(APPLY_CHANGE)
	/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
#endif
}

//ql
void setQosPolicy()
{
	unsigned char policy;
	int min=1, max=2, sel;

	mib_get(MIB_QOS_POLICY, (void *)&policy);
	printf("IP QoS Policy:\t %s\n", (policy==1)?"WRR":"PRIO");

	printf("new IP QoS Policy:\t (1)PRIO (2)WRR\n\n");
	if (getInputUint("Select index:",&sel,&min,&max)) {
		if (sel == 1)
			policy = 0;//PRIO
		else
			policy = 1;//WRR
		mib_set(MIB_QOS_POLICY, (void *)&policy);

#if defined(APPLY_CHANGE)
		/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
	}

	return;
}

void setQosTotalbandwidth()
{
	unsigned int bandwidth;
	unsigned char enable;
	unsigned int min, max, sel;

	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&enable);
	printf("\ntotalbandwidth limit:\t %s\n", enable?"Enable":"Disable");

	mib_get(MIB_TOTAL_BANDWIDTH, &bandwidth);
	printf("totalbandwidth:\t %uKbit\n\n", bandwidth);

	min = 0;
	max = UINT_MAX;
	if (getInputUint("set totalbandwidth(Kbit):", &sel, &min, &max)) {
		if (sel==0) {
			bandwidth = 0;
			enable = 0;
		} else {
			bandwidth = sel;
			enable = 1;
		}
		mib_set(MIB_TOTAL_BANDWIDTH, &bandwidth);
		mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&enable);

#if defined(APPLY_CHANGE)
		/*ql 20081117 START modify qos take effect function*/
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
	} else {
		printf("Invalid value!\n");
		printWaitStr();
	}

	return;
}
#endif

void classificationAdd()
{
	char strSAddr[18], strDAddr[18], strSport[10], strDport[10];
	int intVal, retVal;
	int min,max,sel;
	unsigned int  totalEntry, i;
	MIB_CE_IP_QOS_T entry;
	MIB_CE_IP_QOS_QUEUE_T Entry_queue;
	unsigned long  mask, mbit;
	char tmpstr[256] = {0};
	const char *pPrio;
	struct itfInfo itfs[16];
	int type, ifnum = 0, itfid, itfdomain;
	struct in_addr ipaddr;

	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */

#ifdef _CWMP_MIB_
	// enable
	entry.enable = 1;
#endif

	/* Add new qos entry */
	if (totalEntry >= MAX_QOS_RULE)
	{
		printf("Error: Maximum number of Qos rule exceeds !");
		goto setErr_qos;
	}

	// Check Queue Table
	totalEntry = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
	if (totalEntry == 0) {
		printf("\nEmpty queue table!\n");
		printWaitStr();
		return;
	}
	// ---------------------- Classification Criterion -------------------
	printf("\n-------------- Classification Criterion -----------------\n");
	// protocol type
	min=0;max=3;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Protocol: (1)TCP (2)UDP (3)ICMP [None]: ", &sel, &min, &max);
	if (retVal == 0) {
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.protoType = 0;
	else
		entry.protoType = (unsigned char)sel;

	// source ip
	strSAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Source IP [None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printWaitStr();
		return;
	} else if (retVal==1)
		strcpy(strSAddr, inet_ntoa(ipaddr));


	// Source address
	if (strSAddr[0]) {
		if (!isValidIpAddr(strSAddr)) {
			printf("Invalid IP address %s.\n", strSAddr);
			goto setErr_qos;
		}
		inet_aton(strSAddr, (struct in_addr *)&entry.sip);
		if (entry.sip[0]=='\0') {
			printf("Error! Source IP.");
			goto setErr_qos;
		}

		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Source Netmask [None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strSAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strSAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strSAddr);
				goto setErr_qos;
			}
			inet_aton(strSAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.smaskbit = mbit;
		} else if (retVal == -2)
			entry.smaskbit = 32;
	}

	// source port
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Source Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.sPort =0;
		else
			entry.sPort = (unsigned short)sel;
	}

	// Destination address
	strDAddr[0] = 0;
	retVal = getTypedInputDefault(INPUT_TYPE_IPADDR, "Destination IP [None]: ", &ipaddr, 0, 0);
	if (retVal == 0) {
		printWaitStr();
		return;
	} else if (retVal == 1)
		strcpy(strDAddr, inet_ntoa(ipaddr));

	if (strDAddr[0]) {
		if (!isValidIpAddr(strDAddr)) {
			printf("Invalid IP address %s.\n", strDAddr);
			goto setErr_qos;
		}
		inet_aton(strDAddr, (struct in_addr *)&entry.dip);
		if (entry.dip[0]=='\0') {
			printf("Error! Destination IP.");
			goto setErr_qos;
		}

		retVal = getTypedInputDefault(INPUT_TYPE_IPMASK, "Destination Netmask [None]: ", &ipaddr, 0, 0);
		if (retVal == 0) {
			printWaitStr();
			return;
		} else if (retVal == 1) {
			strcpy(strDAddr, inet_ntoa(ipaddr));
			if (!isValidNetmask(strDAddr, 1)) {
				printf("Invalid subnet mask %s.\n", strDAddr);
				goto setErr_qos;
			}
			inet_aton(strDAddr, (struct in_addr *)&mask);
			if (mask==0) {
				printf("Error! Source Netmask.");
				goto setErr_qos;
			}
			mask = htonl(mask);
			mbit=0; intVal=0;
			for (i=0; i<32; i++) {
				if (mask&0x80000000) {
					if (intVal) {
						printf("Error! Source Netmask.");
						goto setErr_qos;
					}
					mbit++;
				}
				else
					intVal=1;
				mask <<= 1;
			}
			entry.dmaskbit = mbit;
		} else if (retVal == -2)
			entry.dmaskbit = 32;
	}

	// destination port
	if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
		min = 1; max = 65535;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Destination Port(1~65535) [None]: ", &sel, &min, &max);
		if (retVal == 0) {
			printWaitStr();
			return;
		} else if (retVal == -2)
			entry.dPort =0;
		else
			entry.dPort = (unsigned short)sel;
	}

	// physical port
#ifdef CONFIG_USB_ETH
	type = (DOMAIN_ELAN|DOMAIN_WLAN|DOMAIN_ULAN);
#else
	type = (DOMAIN_ELAN|DOMAIN_WLAN);
#endif //CONFIG_USB_ETH
	ifnum = get_domain_ifinfo(itfs, 16, type);
	if (ifnum == 0)
		goto setErr_qos;
	snprintf(tmpstr, sizeof(tmpstr), "Physical Port:");
	for (i=0; i<ifnum; i++) {
		snprintf(tmpstr, sizeof(tmpstr), "%s (%u)%s", tmpstr, i+1, itfs[i].name);
	}
	min = 1; max = ifnum;
	snprintf(tmpstr, sizeof(tmpstr), "%s [None] : ", tmpstr);
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, tmpstr, &sel, &min, &max);
	if (retVal == 0) {
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.phyPort = 0xff;
	else {
		itfid = IF_ID(itfs[sel-1].ifdomain, itfs[sel-1].ifid);
		itfdomain = IF_DOMAIN(itfid);
		itfid = itfid & 0x0ff;
		if (itfdomain == DOMAIN_ELAN)
			entry.phyPort = itfid;
#ifdef WLAN_SUPPORT
		else if (itfdomain == DOMAIN_WLAN)
#ifdef WLAN_MBSSID
			entry.phyPort = 5 + itfid;
#else
			entry.phyPort = 5;	// wlan0
#endif
#endif
#ifdef CONFIG_USB_ETH
		else if (itfdomain == DOMAIN_ULAN)
			entry.phyPort = IFUSBETH_PHYNUM;	// usb0
#endif //CONFIG_USB_ETH
	}

	//--------------------- Classification Results -----------------------
	printf("\n-------------- Classification Results -----------------\n");
	// select ClassQueue
	totalEntry = queueShow();
	min=0; max=totalEntry-1;
	if (getInputUint("\nClassQueue: ",&sel,&min,&max)==0){
		printf("Invalid value!\n");
		printWaitStr();
		return;
	}
	else {
		mib_chain_get(MIB_IP_QOS_QUEUE_TBL, sel, (void *)&Entry_queue);
		// outbound interface
		entry.outif = Entry_queue.outif;
		// outbound priority
		entry.prior =  Entry_queue.prior;
	}

	min = 1; max = 8;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "802.1p(0~7) : [None] ", &sel, &min, &max);
	if (retVal == 0) {
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.m_1p = 0;
	else
		entry.m_1p = (unsigned char)sel+1;

	min = 1; max = 8;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "IP.Pred(0~7) : [None] ", &sel, &min, &max);
	if (retVal == 0) {
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.m_ipprio = 0;
	else
		entry.m_ipprio = (unsigned char)sel+1;

	min = 1; max = 5;
	retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TOS: (1)Normal Service (2)Minimize Cost (3)Maximize Reliability (4)Maximize Throughput (5)Minimize Delay [None]: ", &sel, &min, &max);
	if (retVal == 0) {
		printWaitStr();
		return;
	} else if (retVal == -2)
		entry.m_iptos = 0xff;
	else
		entry.m_iptos = (unsigned char)(1 << (sel - 1));

	intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Error! Add chain record.");
		goto setErr_qos;
	}
	else if (intVal == -1) {
		printf("Error! Table Full.");
		goto setErr_qos;
	}

	has_changed = 1;
	return;
setErr_qos:
	printf("Set QoS rule error!\n");
	printWaitStr();

}

void classificationDelete()
{
	int min,max,sel,totalEntry;

	totalEntry=classificationShow();
	if (totalEntry == 0) {
		printWaitStr();
		return;
	}
	min=1;max=2;
	if(getInputUint("(1) Delete one  (2) Delete all: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if(sel==1){//delete one
		min=0;max=totalEntry-1;
		if(getInputUint("Select index :",&sel,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_IP_QOS_TBL, sel) != 1) {
			printf("Delete chain record error!");
		}
	}
	else{//delete all
		min=1; max=2;
		if (getInputUint("Are you sure? (1)Yes (2)No :[No] ",&sel,&min,&max)==0)
			return;
		if (sel==1)
			mib_chain_clear(MIB_IP_QOS_TBL);
		else
			return;
	}
	has_changed = 1;
}

int classificationShow()
{
	int entryNum, i;
	MIB_CE_IP_QOS_T Entry;
	MIB_CE_ATM_VC_T vc_entry;
	char pItf[16];
	char *psip, *pdip, sip[20], dip[20];
	const char *type;
	char sport[10], dport[10];
	unsigned int mask, smask, dmask;
	char strPhy[]="LAN0", *pPhy;
	const char *pPrio, *pIPrio, *pTos, *p1p;
	char pDscp[5];

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);

	printf("\n--------------------------------------------------------------------------");
	printf("-----------------------------------\n");
	printf("                       Classification Table                                    \n");
	printf("--------------------------------------------------------------------------");
	printf("-----------------------------------\n");
	printf("%-6s%-19s%-8s%-19s%-8s%-7s%-9s%-6s%-6s%-7s%-8s%s\n",
		"Index", "Src", "Src", "Dst", "Dst", "Proto", "LanPort", "QoS", "QoS", "IPrec", "802.1p", "IPTos");
	printf("%-6s%-19s%-8s%-19s%-8s%-7s%-9s%-6s%-6s%-7s%-8s%s\n",
		"", "IP", "Port", "IP", "Port", "", "", "Itf", "Pred", "Mark", "Mark", "Mark");
	printf("--------------------------------------------------------------------------  ");
	printf("---------------------------------\n");

	if (entryNum == 0)
		printf("Empty Table !\n");
	for (i=0; i<entryNum; i++) {

		mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry);
		//source ip
		psip = inet_ntoa(*((struct in_addr *)Entry.sip));
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbit);
			psip = sip;
		}
		// source port
		if (Entry.sPort == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sPort);

		// destination ip
		pdip = inet_ntoa(*((struct in_addr *)Entry.dip));
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbit);
			pdip = dip;
		}
		// destination port
		if (Entry.dPort == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dPort);

		// protocol
		if ( Entry.protoType == PROTO_TCP )
			type = ARG_TCP;
		else if ( Entry.protoType == PROTO_UDP )
			type = ARG_UDP;
		else if ( Entry.protoType == PROTO_ICMP )
			type = ARG_ICMP;
		else
			type = (char *)BLANK;

		// QoS Interface
		if (Entry.outif == DUMMY_IFINDEX)
			strcpy(pItf, BLANK);
		else
		{
			if(getATMVCEntryByIfIndex(Entry.outif, &vc_entry))
			{
				getDisplayWanName(&vc_entry, pItf);
			}
			else
				strcpy(pItf, BLANK);
		}
		// QoS Precedence
		if (Entry.prior <= (IPQOS_NUM_PRIOQ-1))
			pPrio = prioLevel[Entry.prior];
		else
			// should not be here !!
			pPrio = prioLevel[2];

		if (Entry.m_ipprio == 0)
			pIPrio = BLANK;
		else
			pIPrio = n0to7[Entry.m_ipprio];

		if (Entry.m_iptos == 0xff)
			pTos = BLANK;
		else {
			int mask, i;
			mask = i = 1;
			while (i <= 5) {
				if (Entry.m_iptos & mask)
					break;
				else {
					i++;
					mask<<=1;
				}
			}
			if (i >= 6)
				i = 1;
			pTos = ipTos[i];
		}

#ifdef NEW_IP_QOS_SUPPORT
		if (Entry.m_dscp == 0)
			strcpy(pDscp, BLANK);
		else
			snprintf(pDscp, 5, "%d", Entry.m_dscp>>2);

#ifdef QOS_DSCP_MATCH
		if (Entry.qosDscp == 0)
			strcpy(mDscp, BLANK);
		else
			snprintf(mDscp, 5, "%d", Entry.qosDscp>>2);
#endif
#endif

		if (Entry.m_1p == 0)
			p1p = BLANK;
		else
			p1p = n0to7[Entry.m_1p];

		pPhy = strPhy;
		if (Entry.phyPort == 0xff)
			pPhy = (char *)BLANK;
#if defined(IP_QOS_VPORT)
		else if (Entry.phyPort < SW_LAN_PORT_NUM)
			strPhy[3] = '0' + virt2user[Entry.phyPort];
#else
		else if (Entry.phyPort == 0)
			strPhy[3] = '0';
#endif
#ifdef CONFIG_USB_ETH
		else if (Entry.phyPort == IFUSBETH_PHYNUM)
			pPhy = (char *)USBETHIF;
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		else
			#ifdef WLAN_MBSSID
			if(Entry.phyPort==5) //wlan0
				pPhy = (char *)WLANIF[0];
			else 	// 6,7,8,9 vap0,vap1 vap2 vap3
			{
				strncpy(strPhy, "vap0", 4);
				strPhy[3]='0'+ (Entry.phyPort-6);
			}
			#else
			pPhy = (char *)WLANIF[0];
			#endif
#endif
		printf("%-6d%-19s%-8s%-19s%-8s%-7s%-9s%-6s%-6s%-7s%-8s%s\n",
			i, psip, sport, pdip, dport, type, pPhy, pItf, pPrio, pIPrio, p1p, pTos);
	}
	return entryNum;
}

void set_qos_classification()
{
	int snum;
	unsigned char vChar;

	while(1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                         QoS Classification                              \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Add				(2) Delete\n");
		printf("(3) Show			(4) Quit\n");
		if (!getInputOption( &snum, 1, 4))
			continue;
		has_changed = 0;
		switch(snum)
		{
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				classificationAdd();
				break;
			case 2:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				classificationDelete();
				break;
			case 3:
				classificationShow();
				printWaitStr();
				break;
			case 4:
				return;
		}
		if (has_changed) {
			#ifdef APPLY_CHANGE
			#ifdef IP_QOS
			mib_get(MIB_MPMODE, (void *)&vChar);
			if (vChar&MP_IPQ_MASK) {
				stopIPQ();
				setupIPQ();
			}
			#endif
			#endif
			#ifdef COMMIT_IMMEDIATELY
			Commit();
			#endif
		}
	}
}

int qosIflist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char selcnt = 0;
	char wanif[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	//printf("0    Any\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return -1;
		}

		if (!Entry.enableIpQos)
			continue;
		ifGetName(Entry.ifIndex, wanif, sizeof(wanif));
		printf("%-5d%s\n", selcnt++, wanif);
		ifnum ++;
	}

	return ifnum;
}

void queueAdd()
{
	int j, num;
	MIB_CE_ATM_VC_T vcEntry;
	int enable_IPQoS=0;
	int min, max, sel;
	MIB_CE_IP_QOS_QUEUE_T entry;
	char descStr[MAX_QUEUE_DESC_LEN];
	char qosItf[8];

	memset(qosItf, 0, sizeof(qosItf));
	sel = 0;
	num = mib_chain_total(MIB_ATM_VC_TBL);
	for (j=0; j<num; j++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&vcEntry))
			continue;
		if (vcEntry.enableIpQos == 1 ) {
			enable_IPQoS = 1;
			qosItf[sel++] = j; // list of qos interfaces
		}
	}

	if (!enable_IPQoS) {
		printf("Please create an Internet Setting with QoS enabled\n");
		printWaitStr();
		return;
	}

	min=1; max=2;
	if(getInputUint("Status (1)Enable (2)Disable:",&sel,&min,&max)==0)
		return;

	if ( sel==1)
		entry.enable = 1;
	else
		entry.enable = 0;

	max = qosIflist()-1;
	min = 0;
	if (getInputUint("Interface: ", &num, &min, &max) == 0)
		return;
	if (!mib_chain_get(MIB_ATM_VC_TBL, qosItf[num], (void *)&vcEntry))
		return;
	entry.outif = vcEntry.ifIndex;

	min=0; max=3;
	if (getInputUint("Precedence(0 ~ 3): ", &num, &min, &max) == 0)
		return;
	entry.prior = num;

	//check duplication
	if (findQosQueue(&entry)) {
		printf("Queue entry already exists !\n");
		printWaitStr();
		return;
	}

	snprintf(entry.desc, MAX_QUEUE_DESC_LEN, "%d_%d_p%d", vcEntry.vpi, vcEntry.vci, entry.prior);
	mib_chain_add(MIB_IP_QOS_QUEUE_TBL, (unsigned char*)&entry);
	has_changed = 1;
}

int queueShow()
{
	int entryNum, i;
	MIB_CE_IP_QOS_QUEUE_T Entry;
	char itfStr[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);

	printf("\n-------------------------------------------------------------------------------\n");
	printf("                       Queue Table                                    \n");
	printf("-------------------------------------------------------------------------------\n");
	printf("Index  Interface Description    Precedence Status\n");
	printf("-------------------------------------------------------------------------------\n");

	if (entryNum == 0)
		printf("Empty Table !\n");
	for (i=0; i<entryNum; i++) {

		mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void *)&Entry);
		ifGetName(Entry.outif, itfStr, sizeof(itfStr));
		printf("%-7d%-10s%-15s%-11d%-5s\n", i, itfStr, Entry.desc, Entry.prior, Entry.enable ? "Enable" : "Disable");
	}
	return entryNum;
}

void queueDelete()
{
	unsigned int totalEntry;
	int min, max, sel;

	totalEntry = queueShow();
	if (totalEntry == 0) {
		printWaitStr();
		return;
	}
	min=0;max=totalEntry-1;
	if(getInputUint("\nSelect queue index:",&sel,&min,&max)==0)
		return;
	mib_chain_delete(MIB_IP_QOS_QUEUE_TBL, sel);
	update_qos_tbl();
	has_changed = 1;
}

void set_qos_queue()
{
	int snum;
	unsigned char vChar;
	unsigned char mode=0;

	while(1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                         QoS Queue \n");
		printf("-------------------------------------------------------------------------\n");
		printf("(1) Add				(2) Delete\n");
		printf("(3) Show			(4) Quit\n");
		if (!getInputOption( &snum, 1, 4))
			continue;
		has_changed = 0;
		switch(snum)
		{
			case 1:
				queueAdd();
				break;
			case 2:
				queueDelete();
				break;
			case 3:
				queueShow();
				printWaitStr();
				break;
			case 4:
				return;
		}
		if (has_changed) {
			#ifdef APPLY_CHANGE
			#ifdef IP_QOS
			mib_get(MIB_MPMODE, (void *)&vChar);
			if (vChar&MP_IPQ_MASK) {
				stopIPQ();
				setupIPQ();
			}
			#endif
			#endif
			#ifdef COMMIT_IMMEDIATELY
			Commit();
			#endif
		}
	}
}

void setQoS()
{
	int snum, min,max,sel;
	unsigned char vChar, qos;
	unsigned char mode=0;
	char strbuf[256];

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                     	IP QoS                      \n");
		printf("-------------------------------------------------------------------------\n");
		getSYS2Str(SYS_IP_QOS, strbuf);
		printf("IP QoS\t: %s\n\n", strbuf);

		printf("(1) Enable/Disable IPQoS			(2) Classification\n");
		printf("(3) QoS Queue					(4) Quit\n");
		if (!getInputOption( &snum, 1, 4))
			continue;

		switch( snum)
		{
			case 1://(1) Enable/Disable IPQoS
				min=1;max=2;
				if (0 == getInputUint("IP QoS (1)Disable (2)Enable: ", &sel, &min, &max))
					continue;
				sel--;
				mib_get(MIB_MPMODE, (void *)&mode);
				if (sel)
					mode |= MP_IPQ_MASK;
				else
					mode &= ~MP_IPQ_MASK;
				mib_set(MIB_MPMODE, (void *)&mode);
				#ifdef APPLY_CHANGE
				#ifdef IP_QOS
				if (mode&MP_IPQ_MASK)
					setupIPQ();
				else
					stopIPQ();
				#endif
				#endif

//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2: // Classification
				set_qos_classification();
				break;
			case 3: // QoS Queue
				set_qos_queue();
				break;
			case 4:
				return;
		}
	}
}
#endif	// of IP_QOS

#ifdef REMOTE_ACCESS_CTL
//tylo
void showRemoteAccess()
{
	MIB_CE_ACC_T Entry;
	int sel=0;

	if (!mib_chain_get(MIB_ACC_TBL,sel, (void *)&Entry)) {
		printf("Get MIB fail!\n");
		return;
	}

	//TELNET
#ifdef CONFIG_USER_TELNETD_TELNETD
	if (Entry.telnet & 0x02)
		printf("Telnet  LAN:  Enabled\n");
	else
		printf("Telnet  LAN:  Disabled\n");

	if (Entry.telnet & 0x01)
	{
		if (Entry.telnet_port == 23)
			printf("Telnet  WAN:  Enabled\n");
		else
			printf("Telnet  WAN:  Enabled on port %d\n", Entry.telnet_port);
	}
	else
		printf("Telnet  WAN:  Disabled\n");
#endif
#ifdef CONFIG_USER_FTP_FTP_FTP
	//FTP
	if (Entry.ftp & 0x02)
		printf("FTP     LAN:  Enabled\n");
	else
		printf("FTP     LAN:  Disabled\n");

	if (Entry.ftp & 0x01)
	{
		if (Entry.ftp_port == 21)
			printf("FTP     WAN:  Enabled\n");
		else
			printf("FTP     WAN:  Enabled on port %d\n", Entry.ftp_port);
	}
	else
		printf("FTP     WAN:  Disabled\n");
#endif
#ifdef CONFIG_USER_TFTPD_TFTPD
	//TFTP
	if (Entry.tftp & 0x02)
		printf("TFTP    LAN:  Enabled\n");
	else
		printf("TFTP    LAN:  Disabled\n");

	if (Entry.tftp & 0x01)
		printf("TFTP    WAN:  Enabled\n");
	else
		printf("TFTP    WAN:  Disabled\n");
#endif

	//HTTP
	if (Entry.web & 0x02)
		printf("WEB     LAN:  Enabled\n");
	else
		printf("WEB     LAN:  Disabled\n");

	if (Entry.web & 0x01)
	{
		if (Entry.web_port == 80)
			printf("WEB     WAN:  Enabled\n");
		else
			printf("WEB     WAN:  Enabled on port %d\n", Entry.web_port);
	}
	else
		printf("WEB     WAN:  Disabled\n");
//HTTPS
#ifdef CONFIG_USER_BOA_WITH_SSL
	if (Entry.https & 0x02)
		printf("HTTPS   LAN:  Enabled\n");
	else
		printf("HTTPS   LAN:  Disabled\n");

	if (Entry.https & 0x01)
	{
		if (Entry.https_port == 443)
			printf("HTTPS   WAN:  Enabled\n");
		else
			printf("HTTPS   WAN:  Enabled on port %d\n", Entry.https_port);
	}
	else
		printf("HTTPS   WAN:  Disabled\n");
#endif //end of CONFIG_USER_BOA_WITH_SSL

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	//SNMP
	if (Entry.snmp & 0x02)
		printf("SNMP    LAN:  Enabled\n");
	else
		printf("SNMP    LAN:  Disabled\n");

	if (Entry.snmp & 0x01)
		printf("SNMP    WAN:  Enabled\n");
	else
		printf("SNMP    WAN:  Disabled\n");
#endif

#ifdef CONFIG_USER_SSH_DROPBEAR
	//SSH
	if (Entry.ssh & 0x02)
		printf("SSH     LAN:  Enabled\n");
	else
		printf("SSH     LAN:  Disabled\n");

	if (Entry.ssh & 0x01)
		printf("SSH     WAN:  Enabled\n");
	else
		printf("SSH     WAN:  Disabled\n");
#endif

	//ICMP
	printf("ICMP    LAN:  Enabled\n");

	if (Entry.icmp & 0x01)
		printf("ICMP    WAN:  Enabled\n");
	else
		printf("ICMP    WAN:  Disabled\n");

}

//tylo
void changeRemoteAccess(int pvcIndex)
{
	int min,max,sel;
	int portmin,portmax;
	MIB_CE_ACC_T Entry;
	MIB_CE_ACC_T entry;

	if (!mib_chain_get(MIB_ACC_TBL, pvcIndex, (void *)&Entry)) {
		memset(&entry, '\0', sizeof(MIB_CE_ACC_T));
		mib_chain_add(MIB_ACC_TBL, (unsigned char*)&entry);
		if (!mib_chain_get(MIB_ACC_TBL, pvcIndex, (void *)&Entry))
			return;
	}

	filter_set_remote_access(0);

	min=1;max=2;
	portmin=1; portmax=65535;
#ifdef CONFIG_USER_TELNETD_TELNETD
	//TELNET
	if(getInputUint("Telnet  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.telnet = 0;//default Disable
	if(sel==1)
		Entry.telnet |= 0x02;

	if(getInputUint("Telnet  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		{
		Entry.telnet |= 0x01;
		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"Telnet  WAN open port[23]:",&sel,&portmin,&portmax);
		if(inputRet==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
		}else if(inputRet==-2)
		Entry.telnet_port=23;
		else
		Entry.telnet_port=sel;
		}
#endif
	//FTP
	if(getInputUint("FTP  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.ftp = 0;//default Disable
	if(sel==1)
		Entry.ftp |= 0x02;

	if(getInputUint("FTP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}

	if(sel==1)
	{
		Entry.ftp |= 0x01;

		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"FTP  WAN open port[21]:",&sel,&portmin,&portmax);
		if(inputRet==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
		}else if(inputRet==-2)
		Entry.ftp_port=21;
		else
		Entry.ftp_port=sel;
		}
#ifdef CONFIG_USER_TFTPD_TFTPD
	//TFTP
	if(getInputUint("TFTP  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.tftp = 0;//default Disable
	if(sel==1)
		Entry.tftp |= 0x02;

	if(getInputUint("TFTP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		Entry.tftp |= 0x01;
#endif

	//WEB
	if(getInputUint("WEB  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.web  = 0;//default Disable
	if(sel==1)
		Entry.web |= 0x02;

	if(getInputUint("WEB  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}

	if(sel==1)
	{
		Entry.web |= 0x01;

		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"WEB  WAN open port[80]:",&sel,&portmin,&portmax);
		if(inputRet==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
		}else if(inputRet==-2)
		Entry.web_port=80;
		else
		Entry.web_port=sel;
		}
#ifdef CONFIG_USER_BOA_WITH_SSL
	//HTTPS
	if(getInputUint("HTTPS  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.https  = 0;//default Disable
	if(sel==1)
		Entry.https |= 0x02;

	if(getInputUint("HTTPS  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}

	if(sel==1) {
		Entry.https |= 0x01;
		int inputRet=getTypedInputDefault(INPUT_TYPE_UINT,"HTTPS  WAN open port[443]:",&sel,&portmin,&portmax);
		if(inputRet==0){
			printf("Invalid selection!\n");
			goto err_rmacc;
		}else if(inputRet==-2)
			Entry.https_port=443;
		else
			Entry.https_port=sel;
	}
#endif //end of CONFIG_USER_BOA_WITH_SSL
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	//SNMP
	if(getInputUint("SNMP  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.snmp  = 0;//default Disable
	if(sel==1)
		Entry.snmp |= 0x02;

	if(getInputUint("SNMP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		Entry.snmp |= 0x01;
#endif
#ifdef CONFIG_USER_SSH_DROPBEAR
	//SSH
	if(getInputUint("SSH  LAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.ssh  = 0;//default Disable
	if(sel==1)
		Entry.ssh |= 0x02;

	if(getInputUint("SSH  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	if(sel==1)
		Entry.ssh |= 0x01;
#endif
	//ICMP
	printf("ICMP  LAN: Enable\n");

	if(getInputUint("ICMP  WAN (1) Enable (2) Disable:",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		goto err_rmacc;
	}
	Entry.icmp  = 0x02; // always LAN enable
	if(sel==1)
		Entry.icmp |= 0x01;

	mib_chain_update(MIB_ACC_TBL, (void *)&Entry, pvcIndex);
err_rmacc:
	filter_set_remote_access(1);
}

//tylo
void setRemoteAccess()
{
	int min,max,sel;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                      Remote Access             \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to enable/disable management services for the LAN and WAN. \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");

		showRemoteAccess();
		min=1;max=2;
		if(getInputUint("Select index (1) Set Remote Access (2) Quit:",&sel,&min,&max)==0){
			printf("Invalid selection!\n");
			return;
		}

		switch(sel)
		{
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				changeRemoteAccess(0);
#ifdef COMMIT_IMMEDIATELY //Magician: Commit immediately
				Commit();
#endif
				break;
			case 2:
				return;
		}
	}
}
#endif // of REMOTE_ACCESS_CTL

#ifdef IP_PASSTHROUGH
//Jenny
void showOthers(){
	unsigned char laChar;
	unsigned int vInt;
	char wanif[IFNAMSIZ];

	mib_get( MIB_IPPT_ITF, (void *)&vInt);
	if (vInt == DUMMY_IFINDEX)
		printf("IP PassThrough: None\n");
	else {
		ifGetName(vInt, wanif, sizeof(wanif));
		printf("IP PassThrough: %s\n", wanif);

		printf("Lease Time: %s Seconds\n", getMibInfo(MIB_IPPT_LEASE));

		mib_get( MIB_IPPT_LANACC, (void *)&laChar);
		if (laChar)
			printf("LAN Access: Enable\n");
		else
			printf("LAN Access: Disable\n");
	}
}

//tylo
unsigned int IPPTsel[8];
int IPPTlist(){
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char *name;
	int type=3;	// point-to-point interface
	unsigned char IPPTselcnt=0;
	char wanif[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx  Interface\n");
	printf("----------------\n");
	printf("0    None\n");
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if (type != 2) { // rt or all (1 or 0)
			if ((type == 1 || type == 3) && Entry.cmode == CHANNEL_MODE_BRIDGE)
				continue;

			// check for p-2-p link
			if (type == 3 && Entry.cmode == CHANNEL_MODE_IPOE)
				continue;

			ifGetName(Entry.ifIndex, wanif, sizeof(wanif));
			IPPTsel[IPPTselcnt]=Entry.ifIndex;
			IPPTselcnt++;
			printf("%-5d%s\n", IPPTselcnt, wanif);
			ifnum++;
		}
	}

	return ifnum;
}

//Jenny
//void setIPPT(struct ippt_para para)
struct ippt_para setIPPT()
{

	int min, max, sel, num;

	unsigned char ippt_lanacc=0, old_ippt_lanacc;
	unsigned int ippt_itf=DUMMY_IFINDEX, old_ippt_itf;
	unsigned int ippt_lease=0, old_ippt_lease;
	struct ippt_para para;

	// Get old index of IPPT interface
	mib_get(MIB_IPPT_ITF, (void *)&old_ippt_itf);
	para.old_ippt_itf= old_ippt_itf;
	//printf("para.old_ippt_itf=%d\n", para.old_ippt_itf);

	// Get old IPPT Lease Time
	mib_get(MIB_IPPT_LEASE, (void *)&old_ippt_lease);
	para.old_ippt_lease= old_ippt_lease;
	//printf("para.old_ippt_lease=%d\n", para.old_ippt_lease);

	// Get old IPPT LAN access flag
	mib_get(MIB_IPPT_LANACC, (void *)&old_ippt_lanacc);
	para.old_ippt_lanacc= old_ippt_lanacc;
	//printf("para.old_ippt_lanacc=%d\n", para.old_ippt_lanacc);

	max=IPPTlist();
	min=0;
	if(getInputUint("Select interface: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return;
	}

	if (sel != 0) {
		ippt_itf=IPPTsel[sel-1];
		para.new_ippt_itf= ippt_itf;

		min=0;max=0xffffffff;
		if (0 == getInputUint("Lease Time (seconds) : ", &ippt_lease, &min, &max))
			return;
		para.new_ippt_lease= ippt_lease;

		min=1;max=2;
		if (0 == getInputUint("LAN Access (1)Disable (2)Enable :", &num, &min, &max))
			return;
		ippt_lanacc = (unsigned char)(num - 1);
		para.new_ippt_lanacc= ippt_lanacc;

		if ( !mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease)) {
			printf("Set IP passthrough lease time error!\n");
			return;
		}
		if ( !mib_set(MIB_IPPT_LANACC, (void *)&ippt_lanacc)) {
			printf("Set IP passthrough LAN access error!\n");
			return;
		}
	}
	else {
		ippt_itf=DUMMY_IFINDEX;
		para.new_ippt_itf= ippt_itf;
		para.new_ippt_lease= ippt_lease;
		para.new_ippt_lanacc= ippt_lanacc;
	}

	if ( !mib_set(MIB_IPPT_ITF, (void *)&ippt_itf)) {
		printf("Set IP passthrough interface index error!\n");
		return;
	}
	return para;
}
#endif

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_RADVD
void showRADVD()
{
	unsigned char str[MAX_RADVD_CONF_PREFIX_LEN];
	unsigned char str2[MAX_RADVD_CONF_PREFIX_LEN];
	unsigned char vChar;
	int radvdpid;

	printf("The radvd.conf is as follows:\n\n");
	printf("interface br0\n");
	printf("{\n");

	// MaxRtrAdvIntervalAct
	if ( !mib_get(MIB_V6_MAXRTRADVINTERVAL, (void *)str)) {
		printf("Get MaxRtrAdvIntervalAct mib error!");
	}
	if (str[0]) {
		printf("\tMaxRtrAdvInterval %s;\n", str);
	}

	// MinRtrAdvIntervalAct
	if ( !mib_get(MIB_V6_MINRTRADVINTERVAL, (void *)str)) {
		printf("Get MinRtrAdvIntervalAct mib error!");
	}
	if (str[0]) {
		printf("\tMinRtrAdvInterval %s;\n", str);
	}

	// AdvCurHopLimitAct
	if ( !mib_get(MIB_V6_ADVCURHOPLIMIT, (void *)str)) {
		printf("Get AdvCurHopLimitAct mib error!");
	}
	if (str[0]) {
		printf("\tAdvCurHopLimit %s;\n", str);
	}

	// AdvDefaultLifetime
	if ( !mib_get(MIB_V6_ADVDEFAULTLIFETIME, (void *)str)) {
		printf("Get AdvDefaultLifetime mib error!");
	}
	if (str[0]) {
		printf("\tAdvDefaultLifetime %s;\n", str);
	}

	// AdvReachableTime
	if ( !mib_get(MIB_V6_ADVREACHABLETIME, (void *)str)) {
		printf("Get AdvReachableTime mib error!");
	}
	if (str[0]) {
		printf("\tAdvReachableTime %s;\n", str);
	}

	// AdvRetransTimer
	if ( !mib_get(MIB_V6_ADVRETRANSTIMER, (void *)str)) {
		printf("Get AdvRetransTimer mib error!");
	}
	if (str[0]) {
		printf("\tAdvRetransTimer %s;\n", str);
	}

	// AdvLinkMTU
	if ( !mib_get(MIB_V6_ADVLINKMTU, (void *)str)) {
		printf("Get AdvLinkMTU mib error!");
	}
	if (str[0]) {
		printf("\tAdvLinkMTU %s;\n", str);
	}

	// AdvSendAdvert
	if ( !mib_get( MIB_V6_SENDADVERT, (void *)&vChar) )
		printf("Get MIB_V6_SENDADVERT error!");
	if (0 == vChar)
		printf("\tAdvSendAdvert off;\n");
	else
		printf("\tAdvSendAdvert on;\n");

	// AdvManagedFlag
	if ( !mib_get( MIB_V6_MANAGEDFLAG, (void *)&vChar) )
		printf("Get MIB_V6_MANAGEDFLAG error!");
	if (0 == vChar)
		printf("\tAdvManagedFlag off;\n");
	else
		printf("\tAdvManagedFlag on;\n");

	// AdvOtherConfigFlag
	if ( !mib_get( MIB_V6_OTHERCONFIGFLAG, (void *)&vChar) )
		printf("Get MIB_V6_OTHERCONFIGFLAG error!");
	if (0 == vChar)
		printf("\tAdvOtherConfigFlag off;\n");
	else
		printf("\tAdvOtherConfigFlag on;\n");

	// Prefix
	mib_get( MIB_V6_PREFIX_MODE, (void *)&vChar);
	if (vChar!=0) {
		if ( !mib_get(MIB_V6_PREFIX_IP, (void *)str)) {
			printf("Get Prefix_IP mib error!");
		}
		if ( !mib_get(MIB_V6_PREFIX_LEN, (void *)str2)) {
			printf("Get Prefix_Length mib error!");
		}
		printf("\t\n");
		printf("\tprefix %s/%s\n", str, str2);
		printf("\t{\n");
			// AdvValidLifetime
			if ( !mib_get(MIB_V6_VALIDLIFETIME, (void *)str)) {
				printf("Get AdvValidLifetime mib error!");
			}
			if (str[0]) {
				printf("\t\tAdvValidLifetime %s;\n", str);
			}

			// AdvPreferredLifetime
			if ( !mib_get(MIB_V6_PREFERREDLIFETIME, (void *)str)) {
				printf("Get AdvPreferredLifetime mib error!");
			}
			if (str[0]) {
				printf("\t\tAdvPreferredLifetime %s;\n", str);
			}

			// AdvOnLink
			if ( !mib_get( MIB_V6_ONLINK, (void *)&vChar) )
				printf("Get MIB_V6_ONLINK error!");
			if (0 == vChar)
				printf("\t\tAdvOnLink off;\n");
			else
				printf("\t\tAdvOnLink on;\n");

			// AdvAutonomous
			if ( !mib_get( MIB_V6_AUTONOMOUS, (void *)&vChar) )
				printf("Get MIB_V6_AUTONOMOUS error!");
			if (0 == vChar)
				printf("\t\tAdvAutonomous off;\n");
			else
				printf("\t\tAdvAutonomous on;\n");

		printf("\t};\n");

	}

	printf("};\n\n");
	return;

}

void setRADVD()
{
	unsigned char str[MAX_RADVD_CONF_LEN], str2[MAX_RADVD_CONF_PREFIX_LEN];
	int min, max, num, snum;
	unsigned char mode, prefix_mode;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      RADVD Configuration                                \n");
		printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the RADVD's parameters.                   \n");
		printf("-------------------------------------------------------------------------\n");
		showRADVD();

		printf("(1) Set                          (2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch( snum)
		{
			case 1://(1) Set
				if (!check_access(SECURITY_SUPERUSER))
					break;

				getInputString("MaxRtrAdvInterval :", str, sizeof(str)-1);
				if ( !mib_set(MIB_V6_MAXRTRADVINTERVAL, (void *)str)) {
					printf("setRADVD:Set MaxRtrAdvInterval mib error!");
					continue;
				}

				getInputString("MinRtrAdvInterval :", str, sizeof(str)-1);
				if ( !mib_set(MIB_V6_MINRTRADVINTERVAL, (void *)str)) {
					printf("setRADVD:Set MinRtrAdvInterval mib error!");
					continue;
				}

				getInputString("AdvCurHopLimit :", str, sizeof(str)-1);
				if ( !mib_set(MIB_V6_ADVCURHOPLIMIT, (void *)str)) {
					printf("setRADVD:Set AdvCurHopLimit mib error!");
					continue;
				}

				getInputString("AdvDefaultLifetime :", str, sizeof(str)-1);
				if ( !mib_set(MIB_V6_ADVDEFAULTLIFETIME, (void *)str)) {
					printf("setRADVD:Set AdvDefaultLifetime mib error!");
					continue;
				}

				getInputString("AdvReachableTime :", str, sizeof(str)-1);
				if ( !mib_set(MIB_V6_ADVREACHABLETIME, (void *)str)) {
					printf("setRADVD:Set AdvReachableTime mib error!");
					continue;
				}

				getInputString("AdvRetransTimer :", str, sizeof(str)-1);
				if ( !mib_set(MIB_V6_ADVRETRANSTIMER, (void *)str)) {
					printf("setRADVD:Set AdvRetransTimer mib error!");
					continue;
				}

				getInputString("AdvLinkMTU :", str, sizeof(str)-1);
				if ( !mib_set(MIB_V6_ADVLINKMTU, (void *)str)) {
					printf("setRADVD:Set AdvLinkMTU mib error!");
					continue;
				}

				min=1;max=2;
				if (0 == getInputUint("AdvSendAdvert (1)off (2)on :", &num, &min, &max))
					continue;
				mode = (unsigned char)(num - 1);
				mib_set( MIB_V6_SENDADVERT, (void *)&mode);

				min=1;max=2;
				if (0 == getInputUint("AdvManagedFlag (1)off (2)on :", &num, &min, &max))
					continue;
				mode = (unsigned char)(num - 1);
				mib_set( MIB_V6_MANAGEDFLAG, (void *)&mode);

				min=1;max=2;
				if (0 == getInputUint("AdvOtherConfigFlag (1)off (2)on :", &num, &min, &max))
					continue;
				mode = (unsigned char)(num - 1);
				mib_set( MIB_V6_OTHERCONFIGFLAG, (void *)&mode);

				printf("(1) Set Prefix Options       (2) Quit from Setting Prefix options\n");
				if (!getInputOption( &snum, 1, 2))
					continue;

				if ( snum == 1 ) {
					prefix_mode = 1;

					getInputString("prefix_ip :", str2, sizeof(str2)-1);
					if ( !mib_set(MIB_V6_PREFIX_IP, (void *)str2)) {
						printf("setRADVD:Set prefix_ip mib error!");
						continue;
					}

					getInputString("prefix_len :", str, sizeof(str)-1);
					if ( !mib_set(MIB_V6_PREFIX_LEN, (void *)str)) {
						printf("setRADVD:Set prefix_len mib error!");
						continue;
					}

					getInputString("AdvValidLifetime :", str, sizeof(str)-1);
					if ( !mib_set(MIB_V6_VALIDLIFETIME, (void *)str)) {
						printf("setRADVD:Set AdvValidLifetime mib error!");
						continue;
					}

					getInputString("AdvPreferredLifetime :", str, sizeof(str)-1);
					if ( !mib_set(MIB_V6_PREFERREDLIFETIME, (void *)str)) {
						printf("setRADVD:Set AdvPreferredLifetime mib error!");
						continue;
					}

					min=1;max=2;
					if (0 == getInputUint("AdvOnLink (1)off (2)on :", &num, &min, &max))
						continue;
					mode = (unsigned char)(num - 1);
					mib_set( MIB_V6_ONLINK, (void *)&mode);

					min=1;max=2;
					if (0 == getInputUint("AdvAutonomous (1)off (2)on :", &num, &min, &max))
						continue;
					mode = (unsigned char)(num - 1);
					mib_set( MIB_V6_AUTONOMOUS, (void *)&mode);

				} else
					prefix_mode = 0;
			      	mib_set(MIB_V6_PREFIX_MODE, (void *)&prefix_mode);

#if defined(APPLY_CHANGE)
				setup_radvd_conf(1);
#endif

//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2://(2) Quit
				return;
		}//end switch, RADVD Configuration
	}//end while, RADVD Configuration
}
#endif // of CONFIG_USER_RADVD
#endif // of CONFIG_IPV6

#ifdef CONFIG_USER_CUPS
void showPrtServer()
{
	char str[BUFSIZ / 4];

	getPrinterList(str, sizeof(str));

	puts(str);
}

void setPrinterServer()
{
	int snum, tmpi;
	unsigned char buf[256];

	while (1) {
		tmpi = 0;
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf
		    ("				 (6) Advance Menu							\n");
		MSG_LINE;
		printf("This page is used to show printer URL(s).\n");
		MSG_LINE;
		showPrtServer();
		tmpi++;
		printf("(%d) Quit \n", tmpi);
		if (!getInputOption(&snum, 1, tmpi))
			continue;
		if (snum == tmpi)
			return;
	}
}
#endif //CONFIG_USER_CUPS

#ifdef IP_PASSTHROUGH
//Jenny
void setOthers()
{
	int snum;
	struct ippt_para para;

	while (1)
	{
		CLEAR;
		printf("\n");
		printf("-------------------------------------------------------------------------\n");
		printf("                      Other Advanced Configuration             \n");
		printf("-------------------------------------------------------------------------\n");
		printf("Here you can set some other advanced settings.\n");
		printf("-------------------------------------------------------------------------\n");
		showOthers();
		printf("\n(1) Set IP PassThrough			(2) Quit\n");
		if (!getInputOption( &snum, 1, 2))
			continue;

		switch(snum)
		{
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				para = setIPPT();
#if defined(APPLY_CHANGE)
				restartIPPT(para);
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2:
				return;
		 	default:
				printf("!! Invalid Selection !!\n");
		}
	}
}
#endif

void setAdvance()
{
	int snum;
	int tmpi=0;;
	MENU_ITEM	*pm;
	unsigned char	buf[256];
	MENU_ITEM adv_menu[] = {
		{"ARP Table", showARPTable},
		{"Bridging", setBridging},
		#ifdef ROUTING
		{"Routing", setRouting},
		#endif
		#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		{"SNMP", setSNMP},
		#endif //CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		#if defined(ITF_GROUP)
		{"Port Mapping", setPortMapping},
		#endif //defined(ITF_GROUP)
		#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
		{"IP Qos", setQoS},
		#endif //defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
		#if defined(ELAN_LINK_MODE) || defined(ELAN_LINK_MODE_INTRENAL_PHY)
		{"Link Mode", setLinkmode},
		#endif //defined(ELAN_LINK_MODE) || defined(ELAN_LINK_MODE_INTRENAL_PHY)
		#ifdef REMOTE_ACCESS_CTL
		{"Remote Access", setRemoteAccess},
		#endif //REMOTE_ACCESS_CTL
		#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		{"RADVD", setRADVD},
		#endif //defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		#ifdef CONFIG_USER_CUPS
		{"Print Server", setPrinterServer},
		#endif //#ifdef CONFIG_USER_CUPS
		#ifdef IP_PASSTHROUGH
		{"Others", setOthers},
		#endif //IP_PASSTHROUGH
		{"",NULL}
	};

	while (1)
	{
		tmpi = 0;
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                 (6) Advance Menu                           \n");
		MSG_LINE;
		for (pm =&adv_menu[0] ;pm->fn!= NULL;pm++)
		{
			tmpi++;
			printf("(%d) %s",tmpi, pm->display_name);
			if (tmpi&0x01)
				printspace(33-4-strlen(pm->display_name));
			else
				printf("\n");
		}
		tmpi++;
		printf("(%d) Quit \n",tmpi);
		if (!getInputOption( &snum, 1, tmpi))
		      	continue;
		if (snum == tmpi)
			return;
		adv_menu[snum-1].fn();
	}
}

void old_setAdvance()
{
	int snum;

	while (1)
	{
		CLEAR;
		CLEAR;
		printf("\n");
		printf("------------------------------------------------------------\n");
		printf("                 (6) Advance Menu                           \n");
		printf("------------------------------------------------------------\n");
		printf("(1)  ARP Table                    (2)  Bridging\n");
		printf("(3)  Routing                      (4)  SNMP\n");
		printf("(5)  Port Mapping                 (6)  IP Qos\n");
		printf("(7)  Link Mode                    (8)  Remote Access\n");
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		printf("(9)  RADVD                        (10) Others\n");
		printf("(11) Quit\n");
#else
		printf("(9)  Others                       (10) Quit\n");
#endif

#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		if (!getInputOption( &snum, 1, 11))
#else
		if (!getInputOption( &snum, 1, 10))
#endif
			continue;
		switch( snum)
		{
		case 1://(1) ARP table
			CLEAR;
			showARPTable();
			printWaitStr();
			break;
		case 2://(2) Bridging
			setBridging();
			break;
		case 3://(3) Routing
#ifdef ROUTING
			setRouting();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 4://(4) SNMP
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		   	setSNMP();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
		   break;
		case 5://(5) Port Mapping
#if defined(ITF_GROUP)
			CLEAR;
			eth2pvclist();
			if (!check_access(SECURITY_SUPERUSER))
				break;
			setPortMapping();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 6://(6) IP QoS
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
			setQoS();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 7://(7) Link Mode
#if defined(ELAN_LINK_MODE) || defined(ELAN_LINK_MODE_INTRENAL_PHY)
			setLinkmode();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 8://(8) Remote access
#ifdef REMOTE_ACCESS_CTL
			setRemoteAccess();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		case 9: // (9) RADVD
			printf("RADVD supported\n");
			setRADVD();
			break;
		case 10://(10) Others
#ifdef IP_PASSTHROUGH
			setOthers();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 11://(11) Quit
			return;
#else
		case 9://(9) Others
#ifdef IP_PASSTHROUGH
			setOthers();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 10://(10) Quit
			return;
#endif
		default:
			printf("!! Invalid Selection !!\n");
		}//end switch, (6) Advance Menu
	}//end while, (6) Advance Menu
}
/*************************************************************************************************/

#ifdef CONFIG_DEV_xDSL
#define MAX_DSL_TONE 512
#ifdef CONFIG_VDSL
static void startADSLDiagnostic(XDSL_OP *d)
{
	int dbgmode;
	int mode;
	int ldstate, diagflag = 1, i, chan = 256;
	short *hlog, *snr, *qln;
	int intp, fp;
	char str[16];
	Modem_LinkSpeed vLs;
	int mval=0;
	int isVDSL2, vd2loop;
	ComplexShort *hlin;
	VDSL2DiagOthers vd2other;
	short *pother;


  hlog = malloc(sizeof(short)*MAX_DSL_TONE*2);
  snr = malloc(sizeof(short)*MAX_DSL_TONE*2);
  qln = malloc(sizeof(short)*MAX_DSL_TONE*2);
  hlin = malloc(sizeof(ComplexShort)*MAX_DSL_TONE*2);
  if( !hlog || !snr || !qln || !hlin )
  {
		printf( "%s: malloc failed!\n", __FUNCTION__ );
  }else{
	//reset
	pother=&vd2other;
	memset( &vd2other, 0, sizeof(vd2other) );
	memset( hlog, 0,  sizeof(short)*MAX_DSL_TONE*2 );
	memset( snr, 0,  sizeof(short)*MAX_DSL_TONE*2 );
	memset( qln, 0,  sizeof(short)*MAX_DSL_TONE*2 );
	memset( hlin, 0,  sizeof(ComplexShort)*MAX_DSL_TONE*2 );


	printf("\nDSL Diagnostic starts!! \nThe test result will come out later. Please wait ...\n\n");
#ifdef _USE_NEW_IOCTL_FOR_DSLDIAG_
	//fprintf( stderr, "use RLCM_ENABLE_DIAGNOSTIC to start dsldiag\n" );
	mode=0;
	d->xdsl_drv_get(RLCM_ENABLE_DIAGNOSTIC, (void *)&mode, sizeof(int));//Lupin
#else
	dbgmode = 41;
	d->xdsl_drv_get(RLCM_DEBUG_MODE, (void *)&dbgmode, sizeof(int));
#endif
	d->xdsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
	// wait until showtime
	while (!d->xdsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		usleep(1000000);
	usleep(5000000);
	d->xdsl_drv_get(RLCM_GET_LD_STATE, (void *)&ldstate, 4);
	if (ldstate != 0)
		printf("ADSL Diagnostics successful !!");
	else
		printf("ADSL Diagnostics failed !!");


	// get the channel number
	chan=256;
	isVDSL2=0;
	if(d->xdsl_msg_get(GetPmdMode,&mval))
	{
		if(mval&MODE_VDSL2)
			isVDSL2=1;
		else
			isVDSL2=0;

		if(mval<MODE_ADSL2PLUS)
			chan=256;
		else
			chan=512;
	}

	intp = fp = 0;
	str[0] = 0;

	if( d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_HLOG, (void *)hlog, sizeof(short)*MAX_DSL_TONE*2))
	{
		d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_SNR, (void *)snr, sizeof(short)*MAX_DSL_TONE*2);
		d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_QLN, (void *)qln, sizeof(short)*MAX_DSL_TONE*2);
		d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_HLIN, (void *)hlin, sizeof(ComplexShort)*MAX_DSL_TONE*2);
		d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_OTHER, (void *)&vd2other, sizeof(vd2other));
		diagflag = 1;
		printf("\n\t\t\t%s\t%s\n", "Downstream", "Upstream");
		if(isVDSL2)
		{
			printf("Hlin Scale\t\t%d\t\t %d\n", (unsigned short)vd2other.HlinScale_ds, (unsigned short)vd2other.HlinScale_us );
			//printf("Loop Attenuation(dB)\t%d.%d\t\t %d.%d\n", 0, 0, 0, 0);
			//printf("Signal Attenuation(dB)\t%d.%d\t\t %d.%d\n", 0, 0, 0, 0);
			printf("SNR Margin(dB)\t\t%d.%d\t\t %d.%d\n", vd2other.SNRMds/10, abs(vd2other.SNRMds%10), vd2other.SNRMus/10, abs(vd2other.SNRMus%10) );
			printf("Attainable Rate(Kbps)\t%d\t\t %d\n", vd2other.ATTNDRds, vd2other.ATTNDRus);
			printf("Output Power(dBm)\t%d.%d\t\t %d.%d\n", vd2other.ACTATPds/10, abs(vd2other.ACTATPds%10), vd2other.ACTATPus/10, abs(vd2other.ACTATPus%10));
		}else{
			printf("Hlin Scale\t\t%d\t\t %d\n", (unsigned short)pother[1], (unsigned short)pother[0]);
			printf("Loop Attenuation(dB)\t%d.%d\t\t %d.%d\n", pother[3]/10, abs(pother[3]%10), pother[2]/10, abs(pother[2]%10));
			printf("Signal Attenuation(dB)\t%d.%d\t\t %d.%d\n", pother[5]/10, abs(pother[5]%10), pother[4]/10, abs(pother[4]%10));
			printf("SNR Margin(dB)\t\t%d.%d\t\t %d.%d\n", pother[7]/10, abs(pother[7]%10), pother[6]/10, abs(pother[6]%10));
			printf("Attainable Rate(Kbps)\t%d\t\t %d\n", pother[9], pother[8]);
			printf("Output Power(dBm)\t%d.%d\t\t %d.%d\n", pother[11]/10, abs(pother[11]%10), pother[10]/10, abs(pother[10]%10));
		}
	}else{
		diagflag = 0;
		printf("\t\t\t%s\t%s\n", "Downstream", "Upstream");
		printf("Hlin Scale\n");
		printf("Loop Attenuation(dB)\n");
		printf("Signal Attenuation(dB)\n");
		printf("SNR Margin(dB)\n");
		printf("Attainable Rate(Kbps)\n");
		printf("Output Power(dBm)\n");
	}


	if(isVDSL2)
	{
		//Band Status
		printf("\n\nBand\nStatus\tU0\tU1\tU2\tU3\tU4\tD1\tD2\tD3\tD4\n");
		//LATN
		printf("LATN");
		if(diagflag)
		{
			for(i=0;i<5;i++)
				printf("\t%g", ((float)vd2other.LATNpbus[i])/10);
			for(i=0;i<4;i++)
				printf("\t%g", ((float)vd2other.LATNpbds[i])/10);
		}
		printf("\n");

		//SATN
		printf("SATN");
		if(diagflag)
		{
			for(i=0;i<5;i++)
				printf("\t%g", ((float)vd2other.SATNpbus[i])/10);
			for(i=0;i<4;i++)
				printf("\t%g", ((float)vd2other.SATNpbds[i])/10);
		}
		printf("\n");

		//SNRM
		printf("SNRM");
		if(diagflag)
		{
			for(i=0;i<5;i++)
				printf("\t%g", ((float)vd2other.SNRMpbus[i])/10);
			for(i=0;i<4;i++)
				printf("\t%g", ((float)vd2other.SNRMpbds[i])/10);
		}
		printf("\n");
	}


	for(vd2loop=0; vd2loop<=isVDSL2; vd2loop++)
	{
		int offset;

		if(vd2loop==1)
		{
			offset=MAX_DSL_TONE;
			printf("\n\nDownstream (Group Number=%u)", vd2other.SNRGds);
		}else{ //vd2loop=0
			offset=0;
			if(isVDSL2) printf("\n\nUpstream (Group Number=%u)", vd2other.SNRGus);
		}

		printf("\n%s\tH.Real\tH.Image\tSNR\tQLN\tHlog", "Tone Number");
		for (i = 0; i < chan; i++)
		{
			printf("\n%d", i);
			if (diagflag) {
				intp = hlin[offset+i].real/1000;
				fp = hlin[offset+i].real%1000;
				if (fp<0) {
					fp = -fp;
					if (intp == 0)
						snprintf(str, 16, "-0.%03d", fp);
					else
						snprintf(str, 16, "%d.%03d", intp, fp);
				}
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			printf("\t\t%s", str);
			if (diagflag) {
				intp = hlin[offset+i].imag/1000;
				fp = hlin[offset+i].imag%1000;
				if (fp<0) {
					fp = -fp;
					if (intp == 0)
						snprintf(str, 16, "-0.%03d", fp);
					else
						snprintf(str, 16, "%d.%03d", intp, fp);
				}
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			printf("\t%s", str);
			if (diagflag) {
				intp = snr[offset+i]/10;
				fp = abs(snr[offset+i]%10);
				snprintf(str, 16, "%d.%d", intp, fp);
			}
			printf("\t%s", str);
			if (diagflag) {
				intp = qln[offset+i]/10;
				fp = qln[offset+i]%10;
				if (fp<0) {
					if (intp != 0)
						snprintf(str, 16, "%d.%d", intp, -fp);
					else
						snprintf(str, 16, "-%d.%d", intp, -fp);
				}
				else
					snprintf(str, 16, "%d.%d", intp, fp);
			}
			printf("\t%s", str);
			if (diagflag) {
				intp = hlog[offset+i]/10;
				fp = hlog[offset+i]%10;
				if (fp<0) {
					if (intp != 0)
						snprintf(str, 16, "%d.%d", intp, -fp);
					else
						snprintf(str, 16, "-%d.%d", intp, -fp);
				}
				else
					snprintf(str, 16, "%d.%d", intp, fp);
			}
			printf("\t%s", str);
		}
	}
  }

  printWaitStr();

  if(snr) free(snr);
  if(qln) free(qln);
  if(hlog) free(hlog);
  if(hlin) free(hlin);
}
#else
static void startADSLDiagnostic(XDSL_OP *d)
{
	int dbgmode;
	char mode;
	unsigned char tone[64];
	int ldstate, diagflag = 1, i, chan = 256;
	short *hlog, *snr, *qln;
	int intp, fp;
	char str[16];
	Modem_LinkSpeed vLs;

	memset(tone, 0, sizeof(tone));
	hlog = malloc(sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE));
	snr = malloc(sizeof(short)*MAX_DSL_TONE);
	qln = malloc(sizeof(short)*MAX_DSL_TONE);

	printf("\nADSL Diagnostic starts!! \nThe test result will come out later. Please wait ...\n\n");
#ifdef _USE_NEW_IOCTL_FOR_DSLDIAG_
	//fprintf( stderr, "use RLCM_ENABLE_DIAGNOSTIC to start dsldiag\n" );
	mode=0;
	d->xdsl_drv_get(RLCM_ENABLE_DIAGNOSTIC, (void *)&mode, sizeof(int));//Lupin
#else
	dbgmode = 41;
	d->xdsl_drv_get(RLCM_DEBUG_MODE, (void *)&dbgmode, sizeof(int));
#endif
	d->xdsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
	// wait until showtime
	while (!d->xdsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		usleep(1000000);
	usleep(5000000);
	d->xdsl_drv_get(RLCM_GET_LD_STATE, (void *)&ldstate, 4);
	if (ldstate != 0)
		printf("ADSL Diagnostics successful !!");
	else
		printf("ADSL Diagnostics failed !!");
	// get the channel number
	if(d->xdsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&mode, 1)) {
		//ramen to clear the first 3 bit
		mode &= 0x1F;
		if (mode < 5) //adsl1/adsl2
			chan = 256;
		else
			chan = 512;
	}
	intp = fp = 0;
	str[0] = 0;

	if (d->xdsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE))) {
		d->xdsl_drv_get(RLCM_GET_DIAG_SNR, (void *)snr, sizeof(short)*MAX_DSL_TONE);
		d->xdsl_drv_get(RLCM_GET_DIAG_QLN, (void *)qln, sizeof(short)*MAX_DSL_TONE);
		diagflag = 1;
		printf("\n\t\t\t%s\t%s\n", "Downstream", "Upstream");
		printf("Hlin Scale\t\t%d\t\t %d\n", (unsigned short)hlog[chan*3+1], (unsigned short)hlog[chan*3]);
		printf("Loop Attenuation(dB)\t%d.%d\t\t %d.%d\n", hlog[chan*3+3]/10, abs(hlog[chan*3+3]%10), hlog[chan*3+2]/10, abs(hlog[chan*3+2]%10));
		printf("Signal Attenuation(dB)\t%d.%d\t\t %d.%d\n", hlog[chan*3+5]/10, abs(hlog[chan*3+5]%10), hlog[chan*3+4]/10, abs(hlog[chan*3+4]%10));
		printf("SNR Margin(dB)\t\t%d.%d\t\t %d.%d\n", hlog[chan*3+7]/10, abs(hlog[chan*3+7]%10), hlog[chan*3+6]/10, abs(hlog[chan*3+6]%10));
		printf("Attainable Rate(Kbps)\t%d\t\t %d\n", hlog[chan*3+9], hlog[chan*3+8]);
		printf("Output Power(dBm)\t%d.%d\t\t %d.%d\n", hlog[chan*3+11]/10, abs(hlog[chan*3+11]%10), hlog[chan*3+10]/10, abs(hlog[chan*3+10]%10));
	}
	else {
		diagflag = 0;
		printf("\t\t\t%s\t%s\n", "Downstream", "Upstream");
		printf("Hlin Scale\n");
		printf("Loop Attenuation(dB)\n");
		printf("Signal Attenuation(dB)\n");
		printf("SNR Margin(dB)\n");
		printf("Attainable Rate(Kbps)\n");
		printf("Output Power(dBm)\n");
	}

	printf("\n%s\tH.Real\tH.Image\tSNR\tQLN\tHlog", "Tone Number");
	for (i = 0; i < chan; i++) {
		printf("\n%d", i);
		if (diagflag) {
			intp = hlog[i+chan]/1000;
			fp = abs(hlog[i+chan]%1000);
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}
		printf("\t\t%s", str);
		if (diagflag) {
			intp = hlog[i+chan*2]/1000;
			fp = abs(hlog[i+chan*2]%1000);
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}
		printf("\t%s", str);
		if (diagflag) {
			intp = snr[i]/10;
			fp = abs(snr[i]%10);
			snprintf(str, 16, "%d.%d", intp, fp);
		}
		printf("\t%s", str);
		if (diagflag) {
			intp = qln[i]/10;
			fp = abs(qln[i]%10);
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		printf("\t%s", str);
		if (diagflag) {
			intp = hlog[i]/10;
			fp = abs(hlog[i]%10);
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		printf("\t%s", str);
	}
	printWaitStr();
	free(snr);
	free(qln);
	free(hlog);
}
#endif /*CONFIG_VDSL*/

static void ADSLDiagnostic(XDSL_OP *d)
{
	char chVal[2];

#ifdef CONFIG_VDSL
	int mval=0;
	if(d->xdsl_msg_get(GetPmdMode,&mval))
	{
		if(mval&(MODE_ADSL2|MODE_ADSL2PLUS|MODE_VDSL2))
			startADSLDiagnostic(d);
		else {
			printf("Not supported! Only ADSL2/ADSL2+/VDSL2 support this function.\n");
			printWaitStr();
		}
	}
#else
	if (d->xdsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&chVal[0], 1)) {
		chVal[0] &= 0x1F;
		if (chVal[0] == 3 || chVal[0] == 5)  // ADSL2/2+
			startADSLDiagnostic(d);
		else {
			printf("Not supported! Only ADSL2/2+ support this function.\n");
			printWaitStr();
		}
	}
#endif /*CONFIG_VDSL*/
}
#endif /*CONFIG_DEV_xDSL*/

void setPing()
{
	char pingAddr[16];
	int conti;

	while (1)
	{
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Ping Diagnostic                               \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to send ICMP ECHO_REQUEST packets to network host. The \n");
		printf("diagnostic result will then be displayed.                                \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");

		printf("Host Address :");
		if ( getInputStr( pingAddr, sizeof(pingAddr)-1, NULL)==0)return;
		//cliping(pingAddr);
		utilping(pingAddr);
		printf("Continue (1)Yes (2)No :");
		if( (conti=getInputNum()) ==2) return;
		continue;
	}//end while, Ping Diagnostic
}

#ifdef CONFIG_DEV_xDSL
#define MAXWAIT		5
static int finished = 0;
void cliOamLb(int oam_flow,int oam_vpi,int oam_vci,char *oam_llid)
{
#ifdef EMBED
	char	*str, *submitUrl;
	char tmpBuf[100];
	int	skfd,i,j;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	int curidx, len;
	char *tmpStr;
	unsigned char *tmpValue;
	time_t  curTime, preTime = 0;


	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		printf("socket open error");
		return;
	}

	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));

	if (oam_flow == 0)
		lbReq.Scope = 0;	// Segment
	else if (oam_flow== 1)
		lbReq.Scope = 1;	// End-to-End

	// VPI (0~255)
	if ( oam_vpi>255) {
		strcpy(tmpBuf, "Connection not exists!");
		goto setErr_oamlb;
	}
	else
		lbReq.vpi = oam_vpi;

	// VCI (0~65535)
	if ( oam_vci>65535) {
		strcpy(tmpBuf, ("Connection not exists!"));
		goto setErr_oamlb;
	}
	else
		lbReq.vci = oam_vci;

	str = oam_llid;
	// convert max of 32 hex decimal string into its 16 octets value
	len = strlen(str);
	curidx = 16;
	for (i=0; i<32; i+=2)
	{
		// Loopback Location ID
		curidx--;
		tmpValue = (unsigned char *)&lbReq.LocID[curidx];
		if (len > 0)
		{
			len -= 2;
			if (len < 0)
				len = 0;
			tmpStr = str + len;
			*tmpValue = strtoul(tmpStr, 0, 16);
			*tmpStr='\0';
		}
		else
			*tmpValue = 0;
	}

	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_START failed !");
		close(skfd);
		goto setErr_oamlb;
	}

	finished = 0;
	time(&preTime);
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = oam_vpi;
	lbState.vci = oam_vci;
	lbState.Tag = lbReq.Tag;

	while (1)
	{
		time(&curTime);
		if (curTime - preTime >= MAXWAIT)
		{
			//printf("OAMLB timeout!\n");
			finished = 1;
			break;	// break for timeout
		}

		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STATUS failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			goto setErr_oamlb;
		}

		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}

	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STOP failed !");
		close(skfd);
		goto setErr_oamlb;
	}
	close(skfd);


	if (!finished)
	{
		printf("\n--- Loopback cell received successfully ---\n");
	}
	else
	{
		printf("\n--- Loopback failed ---\n");
	}
	printf("\n");

  	return;

setErr_oamlb:
	printf("%s\n\n",tmpBuf);
#endif
}

void setATMLoopback()
{
	char oam_llid[33];
	int oam_flow, oam_vpi, oam_vci;
	int conti;

	while (1)
	{
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("           OAM Fault Management - Connectivity Verification              \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("Connectivity verification is supported by the use of the OAM loopback    \n");
		printf("capability for both VP and VC connections. This page is used to perform  \n");
		printf("the VCC loopback function to check the connectivity of the VCC.          \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");

		printf("Flow Type (1)F5 Segment (2)F5 End-to-End (3)Quit: ");
		oam_flow = getInputNum();

		if(oam_flow == 3)
			break;
		else if(oam_flow == 1 || oam_flow == 2 )
		{
			printf("VPI: ");
			oam_vpi=getInputNum();
			printf("VCI: ");
			oam_vci=getInputNum();
			if( getInputString( "Loopback Location ID(HEX) (default:FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF) :\n", oam_llid, sizeof(oam_llid) ) == 0) {
				if (oam_llid[0] == 0)
					strcpy(oam_llid, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
				else {
					printf("Invalid Location ID!\n");
					continue;
				}
			}

			cliOamLb(oam_flow,oam_vpi,oam_vci,oam_llid);
			printf("Continue (1)Yes (2)No : ");
			if((conti=getInputNum()) ==2) return;
			continue;
		}
		else
			continue;
	}//end while, OAM Fault Management
}

#ifdef DIAGNOSTIC_TEST
static const char R_PASS[] = ": PASS";
static const char R_FAIL[] = " : FAIL";
static int cmode = 0;
int eth=0, adslflag=0, pppserver=0, auth=0, ipup=0, lb5s=0, lb5e=0, lb4s=0, lb4e=0, dgw=0, pdns=0;

void runDiagTest(int idx)
{
	int inf=DUMMY_IFINDEX, i, pppif;
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum;
	FILE *fp;
	char buff[10], ifname[IFNAMSIZ];
	MIB_CE_ATM_VC_Tp pEntry;

	Modem_LinkSpeed vLs;

	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;

	mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry);
	inf = Entry.ifIndex;
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0;i<entryNum;i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("Get chain record error!\n");
			return;
		}
		if (Entry.enable == 0)
			continue;
		if (inf == DUMMY_IFINDEX)
			inf = Entry.ifIndex;
		if (Entry.ifIndex == inf) {
			if (Entry.cmode != CHANNEL_MODE_BRIDGE) {
				struct in_addr inAddr;
				int flags;
				cmode = 1;
				ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
				if (PPP_INDEX(Entry.ifIndex) != DUMMY_PPP_INDEX) { // PPP Interface
					int pppflag;
					cmode = 2;
					if (fp = fopen("/tmp/ppp_diag_log", "r")) {
						while (fgets(buff, sizeof(buff), fp) != NULL) {
							sscanf(buff, "%d:%d", &pppif, &pppflag);
							if (pppif == PPP_INDEX(Entry.ifIndex))
									break;
						}
						fclose(fp);
					}
					switch(pppflag) {
					case 1:
						pppserver = 1;
						auth = ipup = 0;
						break;
					case 2:
						pppserver = auth = 1;
						ipup = 0;
						break;
					case 3:
						pppserver = auth = ipup = 1;
						break;
					case 0:
					default:
						pppserver = auth = ipup = 0;
						break;
					}
				}
			}
			else
				cmode = 0;
			break;
		}
	}
	pEntry = &Entry;
	if (adslflag) {
		lb5s = testOAMLookback(pEntry, 0, 5);
		lb5e = testOAMLookback(pEntry, 1, 5);
		lb4s = testOAMLookback(pEntry, 0, 4);
		lb4e = testOAMLookback(pEntry, 1, 4);
		if (cmode > 0) {
			char pingaddr[16];
			memset(pingaddr, 0x00, 16);
			if (defaultGWAddr(pingaddr))
				dgw = 0;
			else
				dgw = utilping(pingaddr);
			memset(pingaddr, 0x00, 16);
			if (pdnsAddr(pingaddr))
				pdns = 0;
			else
				pdns = utilping(pingaddr);
		}
	}
}

void diagTestResult(int idx)
{
	struct ifreq ifrq;

	strncpy(ifrq.ifr_name, ELANIF, sizeof(ifrq.ifr_name));
	ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1] = 0;
	eth = getLinkStatus(&ifrq);
	runDiagTest(idx);
	CLEAR;
	printf("\n");
	printf("%s\n", "LAN Connection Check");
	/***nic and switch are always linked***/
	//printf("%s%s\n\n", "Test Ethernet LAN Connection", (eth)?R_PASS: R_FAIL);
	printf("%s\n", "ADSL Connection Check");
	printf("%s%s\n", "Test ADSL Synchronization", (adslflag)?R_PASS: R_FAIL);
	printf("%s%s\n", "Test ATM OAM F5 Segment Loopback", (lb5s)?R_PASS: R_FAIL);
	printf("%s%s\n", "Test ATM OAM F5 End-to-end Loopback", (lb5e)?R_PASS: R_FAIL);
	printf("%s%s\n", "Test ATM OAM F4 Segment Loopback", (lb4s)?R_PASS: R_FAIL);
	printf("%s%s\n\n", "Test ATM OAM F4 End-to-end Loopback", (lb4e)?R_PASS: R_FAIL);
	if (cmode > 0) {
		printf("%s\n", "Internet Connection Check");
		if (cmode == 2) {
			printf("%s%s\n", "Test PPP Server Connection", (pppserver)?R_PASS: R_FAIL);
			printf("%s%s\n", "Test Authentication with ISP", (auth==1)?R_PASS: R_FAIL);
			printf("%s%s\n", "Test the assigned IP Address", (ipup)?R_PASS: R_FAIL);
		}
		printf("%s%s\n", "Ping Default Gateway", (dgw)?R_PASS: R_FAIL);
		printf("%s%s\n\n", "Ping Primary Domain Name Server", (pdns)?R_PASS: R_FAIL);
	}
	cmode = eth = adslflag = pppserver = auth = ipup = lb5s = lb5e = lb4s = lb4e = dgw = pdns=0;
	printWaitStr();
}

void setDiagnosticTest()
{
	int sel, min, max, conti;

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("			Diagnostic Test 				  \n");
		MENU_LINE;
		printf("The DSL Router is capable of testing your DSL connection. The individual \n");
		printf("tests are listed below. If a test displays a fail status, click \"Run \n");
		printf("Diagnostic Test\" button again to make sure the fail status is consistent. \n");
		MSG_LINE;

		max = showWanVC(0);
		if (max == 0) {
			printf("No WAN Interface!\n");
			printWaitStr();
			return;
		}
		min = 1;
		if (getInputUint("Select the Internet Connection:", &sel, &min, &max) == 0) {
			printf("Invalid selection!\n");
			printWaitStr();
			return;
		}
		diagTestResult(sel-1);
		printf("Continue (1)Yes (2)No : ");
		if ((conti=getInputNum()) == 2)
			return;
		continue;
	}
}
#endif
#endif	//CONFIG_DEV_xDSL

void setDiagnostic()
{
	int snum, menu_size;
	char chVal[2];

	while (1)
	{
		CLEAR;
	 	printf("\n");
	 	MENU_LINE;
	 	printf("                           (7) Diagnostic Menu                        \n");
	 	MENU_LINE;

		menu_size = cli_show_dyn_menu(DiagnosticsMenu);

		if (!getInputOption(&snum, 1, menu_size))
			continue;

	 	switch(diag_menu_idx[snum-1])
	 	{
	 	case MENU_DIAG_PING: // Ping
	 		setPing();
	 		break;
	 	case MENU_DIAG_ATM_LOOPBACK: // ATM Loopback
	 		#ifdef CONFIG_DEV_xDSL
	 		setATMLoopback();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
	 		break;
		case MENU_DIAG_ADSL_TONE: // ADSL Tone Diagnostic
			#ifdef CONFIG_DEV_xDSL
			ADSLDiagnostic( xdsl_get_op(0) );
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
#ifdef CONFIG_USER_XDSL_SLAVE
		case MENU_DIAG_ADSL_SLV_TONE: // DSL Slave Tone Diagnostic
			#ifdef CONFIG_DEV_xDSL
			ADSLDiagnostic( xdsl_get_op(1) );
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
#endif /*CONFIG_USER_XDSL_SLAVE*/
		case MENU_DIAG_ADSL_CONNECTION: // ADSL Connection Diagnostic
			#if defined(DIAGNOSTIC_TEST) && defined(CONFIG_DEV_xDSL)
			setDiagnosticTest();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
	 	case MENU_DIAG_QUIT: // Quit
	 		return;
	 	default:
	 		printf("!! Invalid Selection !!\n");
	 	}//end switch, (7) Diagnostic Menu
	}//end while, (7) Diagnostic Menu
}

void setCommitReboot()
{
	int snum, rebootMode, rebootflag;
	unsigned char vChar;

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           Commit/Reboot                                 \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to commit changes to system memory and reboot your     \n");
		printf("system.                                                                  \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("(1) Commit&Reboot                (2) Restore to Default\n");
		printf("(3) Quit\n");
		if (!getInputOption( &snum, 1, 3))
			continue;

		switch( snum)
		{
		case 1://(1) Set
			printf("Commit and Reboot? (1)Yes (2)No :");
			if( (rebootMode=getInputNum()) ==0) break;
			if (rebootMode == 1) {
				mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
				/* Save and reboot the system */
				printf("%s\n\n", rebootWord0);
				printf("%s\n", rebootWord1);
				printf("%s\n", rebootWord2);
				cmd_reboot();
			}
			break;
		case 2://(2) Restore to Default
			printf("Restore to Default and Reboot? (1)Yes (2)No :");
			if( (rebootMode=getInputNum()) ==0) break;
			if (rebootMode == 1) {
				reset_cs_to_default(1);
				cmd_reboot();
			}
			break;
		case 3://(3) Quit
			return;
		}//end switch, Commit/Reboot
	}//end while, Commit/Reboot
}

#ifdef ACCOUNT_CONFIG
static void addUser()
{
	char newuser[MAX_NAME_LEN], newpass[MAX_NAME_LEN], confpass[MAX_NAME_LEN], suName[MAX_NAME_LEN], usName[MAX_NAME_LEN];
	int entryNum, i, intVal;
	MIB_CE_ACCOUNT_CONFIG_T Entry;

	printf("Please input User Name:");
	if (getInputStr(newuser, sizeof(newuser) - 1, NULL) == 0) return;
	if (newuser[strlen(newuser) - 1] == '\n')
		newuser[strlen(newuser) - 1] = 0;

	if (strlen(newuser) > 0) {
		mib_get(MIB_SUSER_NAME, (void *)suName);
		mib_get(MIB_USER_NAME, (void *)usName);
		if ((strcmp(suName, newuser) == 0) || (strcmp(usName, newuser) == 0)) {
			printf("ERROR: user already exists!");
			return;
		}
		entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
	  			printf ("Get chain record error!\n");
			if (strcmp(Entry.userName, newuser) == 0) {
				printf("ERROR: user already exists!");
				return;
			}
		}
	}
	else {
		printf("Error! User name cannot be empry!\n");
		return;
	}
	strncpy(Entry.userName, newuser, MAX_NAME_LEN-1);
	Entry.userName[MAX_NAME_LEN-1] = '\0';
	//Entry.userName[MAX_NAME_LEN] = '\0';

	printf("Password:");
	if (getInputStr(newpass, sizeof(newpass) - 1, NULL) == 0) return;
	printf("Confirme Password:");
	if (getInputStr(confpass, sizeof(confpass) - 1, NULL) == 0) return;
	if (newpass[strlen(newpass) - 1] == '\n')
		newpass[strlen(newpass) - 1] = 0;
	if (confpass[strlen(confpass) - 1] == '\n')
		confpass[strlen(confpass) - 1] = 0;

	if (strcmp(newpass, confpass) == 0) {
		strncpy(Entry.userPassword, newpass, MAX_NAME_LEN-1);
		Entry.userPassword[MAX_NAME_LEN-1] = '\0';
		//Entry.userPassword[MAX_NAME_LEN] = '\0';
		Entry.privilege = (unsigned char)PRIV_USER;
		intVal = mib_chain_add(MIB_ACCOUNT_CONFIG_TBL, (unsigned char*)&Entry);
		if (intVal == 0) {
			printf("Error! Add chain record.");
   			return;
		}
		else if (intVal == -1) {
			printf("Error! Table Full.");
   			return;
		}
	} else
		printf("Confirmed Password is not corrct ! Plaese Input it again\n");
}

static void modifyPassword()
{
	char oldpass[MAX_NAME_LEN], newpass[MAX_NAME_LEN], confpass[MAX_NAME_LEN], oldMIBpass[MAX_NAME_LEN];
	int entryNum, i, selnum, min;
	MIB_CE_ACCOUNT_CONFIG_T Entry;

	entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL) + 2;
	min = 1;;
	if(getInputUint("Select user account:", &selnum, &min, &entryNum) == 0){
		printf("Invalid selection!\n");
		return;
	}
	printf("Old Password:");
	if (getInputStr(oldpass, sizeof(oldpass) - 1, NULL) == 0) return;
	if (oldpass[strlen(oldpass) - 1] == '\n')
		oldpass[strlen(oldpass) - 1] = 0;
	if (selnum == 1) {
   		if (!mib_get(MIB_SUSER_PASSWORD, (void *)oldMIBpass))
			printf("ERROR(modifyPassword): Get super user password from MIB database failed.\n");
   		if (strcmp(oldpass, oldMIBpass) != 0) {
   			printf("Old Password is not correct ! Please Input it again\n");
			return;
   		}
	}
	else if (selnum == 2) {
   		if (!mib_get(MIB_USER_PASSWORD, (void *)oldMIBpass))
			printf("ERROR(modifyPassword): Get user password from MIB database failed.\n");
   		if (strcmp(oldpass, oldMIBpass) != 0) {
   			printf("Old Password is not correct ! Please Input it again\n");
			return;
   		}
	}
	else {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, selnum -3, (void *)&Entry))
			printf("Get chain record error!\n");
		if (strcmp(Entry.userPassword, oldpass) != 0) {
			printf("Old Password is not corrct ! Plaese Input it again\n");
			return;
		}
	}

	printf("New Password:");
	if (getInputStr(newpass, sizeof(newpass) - 1, NULL) == 0) return;
	printf("Confirme Password:");
	if (getInputStr(confpass, sizeof(confpass) - 1, NULL) == 0) return;

	if (newpass[strlen(newpass) - 1] == '\n')
		newpass[strlen(newpass) - 1] = 0;
	if (confpass[strlen(confpass) - 1] == '\n')
		confpass[strlen(confpass) - 1] = 0;

	if (strcmp(newpass, confpass) == 0) {
		if (selnum == 1) {
			if (!mib_set(MIB_SUSER_PASSWORD, (void *)confpass))
				printf("ERROR(modifyPassword): Set Super user password to MIB database failed.\n");
		} else if (selnum == 2) {
			if (!mib_set(MIB_USER_PASSWORD, (void *)confpass))
				printf("ERROR(modifyPassword): Set user password to MIB database failed.\n");
		} else {
			strncpy(Entry.userPassword, newpass, MAX_NAME_LEN-1);
			Entry.userPassword[MAX_NAME_LEN-1] = '\0';
			//Entry.userPassword[MAX_NAME_LEN] = '\0';
			Entry.privilege = (unsigned char)getAccPriv(Entry.userName);
			mib_chain_update(MIB_ACCOUNT_CONFIG_TBL, (void *)&Entry, selnum -3);
		}
	} else
		printf("Confirmed Password is not corrct ! Plaese Input it again\n");
}
#endif

// Kaohj -- tftp update image/configuration and backup configuration
#if defined CONFIG_USER_BUSYBOX_TFTP || defined CONFIG_USER_BUSYBOX_BUSYBOX1124 && CONFIG_TFTP
void setTftp()
{
	char serverIP[64], fname[32];
	struct in_addr inIp;
#ifdef CONFIG_IPV6
	struct in6_addr in6Ip;
#endif
	int snum;

#ifdef CONFIG_IPV6
	printf("(1) IPv4 server.                          (2) IPv6 server\n");
	if (!getInputOption( &snum, 1, 2))
		return;
	if (snum == 1) {
#endif
		if (0 == getInputIpAddr("Tftp server address:", &inIp))
			return;
		inet_ntop(AF_INET, (struct in_addr *)&inIp, serverIP, sizeof(serverIP));
#ifdef CONFIG_IPV6
	} else {
		if (0 == getInputIp6Addr("Tftp server address:", &in6Ip))
			return;
		inet_ntop(AF_INET6, (struct in_addr *)&in6Ip, serverIP, sizeof(serverIP));
	}
#endif

	while (1) {
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Tftp utility                                \n");
		MSG_LINE;
		printf("Update firmware/configuration or backup configuration from/to a tftp server.\n");
		printf("Server address: %s\n", serverIP);
		MSG_LINE;

		printf("(1) Update image                            (2) Update config.\n");
   		printf("(3) Backup config.                          (4) Quit\n");
   		if (!getInputOption( &snum, 1, 4))
   			continue;

		switch( snum)
		{
		case 1:
			printf("Image update\n");
			if (0==getInputString("Image file name:", fname, sizeof(fname)))
				break;
			va_cmd("/bin/tftp", 5, 1, "-g", "-i", "-f", fname, serverIP);
			printWaitStr();
			break;
		case 2:
			printf("Configuration update\n");
			if (0==getInputString("Configuration file name:", fname, sizeof(fname)))
				break;
			va_cmd("/bin/tftp", 5, 1, "-g", "-c", "-f", fname, serverIP);
			printWaitStr();
			break;
		case 3:
			printf("Configuration backup\n");
			if (0==getInputString("Configuration file name:", fname, sizeof(fname)))
				break;
			va_cmd("/bin/tftp", 5, 1, "-p", "-c", "-f", fname, serverIP);
			printWaitStr();
			break;
		case 4:
			return;
		}
   	}
}
#endif

void setPassword()
{
	char oldpass[30], newpass[30], confpass[30], oldMIBpass[30];
	int snum;
	int i;
	int nameChange;
	char usName[MAX_NAME_LEN];
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int entryNum;
#endif

	while(1)
	{
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
#ifdef ACCOUNT_CONFIG
		printf("                           Account Configuration                                \n");
#else
		printf("                           Password Setup                                \n");
#endif
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to set the account to access the web server of ADSL    \n");
		printf("Router.                                                                  \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");

#ifdef ACCOUNT_CONFIG
		printf("\nIdx  UserName\n");
		printf("----------------\n");
		mib_get(MIB_SUSER_NAME, (void *)usName);
		printf ("1    %s\n", usName);
		mib_get(MIB_USER_NAME, (void *)usName);
		printf ("2    %s\n", usName);
		entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
				printf ("Get chain record error!\n");
				printf ("%-5d%s\n", i+3, Entry.userName);
		}

		printf("\n(1) Add User                          (2) Set Password\n");
		printf("(3) Quit\n");
		if (!getInputOption(&snum, 1, 3))
			continue;

	switch (snum) {
		case 1:
			addUser();
#if defined(APPLY_CHANGE)
   			// Added by Mason Yu for take effect on real time
			writePasswdFile();
			write_etcPassword();	// Jenny
#endif
   			break;
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
		case 2:
			modifyPassword();
#if defined(APPLY_CHANGE)
   			// Added by Mason Yu for take effect on real time
			writePasswdFile();
			write_etcPassword();	// Jenny
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
   			break;

		case 3:
			return;
   	}
#else
		printf("(1) Set Super User                          (2) Set General User\n");
		printf("(3) Quit\n\n");
		if (!getInputOption( &snum, 1, 3))
			continue;

		switch( snum)
		{
			case 1:
			case 2:
			//printf("User Name:");
			//if ( getInputStr( username, sizeof(username)-1)==0)return;
			if (snum == 1) {
				mib_get(MIB_SUSER_NAME, (void *)usName);
				printf("User Name: %s\n", usName);
			}
			else {
				mib_get(MIB_USER_NAME, (void *)usName);
				printf("User Name: %s\n", usName);
			}
			if (0==getInputString("User Name(Press 'Enter' to leave unchanged):", usName, sizeof(usName)))
				nameChange = 0;
			else
				nameChange = 1;

CONFIRM:
			printf("Old Password:");
			if ( getInputStr( oldpass, sizeof(oldpass)-1, NULL)==0)break;

			printf("New Password:");
			if ( getInputStr( newpass, sizeof(newpass)-1, NULL)==0)break;

			printf("Confirmed Password:");
			if ( getInputStr( confpass, sizeof(confpass)-1, NULL)==0)break;

			if ( snum == 1 ) {
				if ( !mib_get(MIB_SUSER_PASSWORD, (void *)oldMIBpass) ) {
					printf("ERROR(setPassword): Get super user password from MIB database failed.\n");
				}
				}else if ( snum == 2 ) {
					if ( !mib_get(MIB_USER_PASSWORD, (void *)oldMIBpass) ) {
						printf("ERROR(setPassword): Get user password from MIB database failed.\n");
				}
			}

			if ( strcmp(oldpass, oldMIBpass) != 0 ) {
				printf("Old Password is not correct ! Please Input it again\n");
				//goto CONFIRM;
				break;
			}

			if ( strcmp(newpass, confpass) == 0 ) {
			/* Set user account to MIB */
			//printf("username=%s  newpass=%s  confpass=%s\n", username, newpass, confpass);

				if ( snum == 1 ) {
					if (nameChange)
						mib_set(MIB_SUSER_NAME, (void *)usName);
					if ( !mib_set(MIB_SUSER_PASSWORD, (void *)confpass) )
						printf("ERROR(setPassword): Set Super user password to MIB database failed.\n");
				}else if ( snum == 2 ) {
					if (nameChange)
						mib_set(MIB_USER_NAME, (void *)usName);
					if ( !mib_set(MIB_USER_PASSWORD, (void *)confpass) )
						printf("ERROR(setPassword): Set user password to MIB database failed.\n");
				}
			}else  {
				printf("Confirmed Password is not correct ! Please Input it again\n");
				//goto CONFIRM;
				break;
			}
#if defined(APPLY_CHANGE)
			// Added by Mason Yu for take effect on real time
			writePasswdFile();
			write_etcPassword();	// Jenny
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif
   		break;

   	case 3:
		return;
		}
#endif

	} // while(1)

}


// remote config status flag: 0: disabled, 1: enabled
//#define MAX_NAME_LEN					30
static int srandomCalled = 0;

#ifdef IP_ACL
void aclDel()
{
	unsigned int totalEntry;
	unsigned int i, index;
	int del, min, max;

	totalEntry = mib_chain_total(MIB_ACL_IP_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return;
	}
	min=1; max=2;
	if (getInputUint("Delete (1)One (2)All :", &del, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return;
	}

	if (del == 2)
		mib_chain_clear(MIB_ACL_IP_TBL); /* clear chain record */
	else if (del == 1) {
		min=1; max=totalEntry;
		if (getInputUint("Select the index to delete:", &index, &min, &max) == 0) {
			printf("Error selection!\n");
			return;
		}
		if(mib_chain_delete(MIB_ACL_IP_TBL, index-1) != 1)
			printf("Delete chain record error!");
	}
}

void setACL()
{
	int snum, aclmode, conf, delnum;
	unsigned char vChar;
	unsigned int entryNum, i;
	MIB_CE_ACL_IP_T Entry;
	struct in_addr dest;
	char sdest[16];
	unsigned long mask, mbit;
	int min, max, sel, intVal;

	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                           ACL Configuration                             \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page is used to configure the IP Address for Access Control List.   \n");
		printf("Here you can add/delete IP Address.                                      \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("ACL Configuration:\n\n");

		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return;
		if (0 == vChar)
			printf("1. ACL Capability: Disabled\n");
		else
			printf("1. ACL Capability: Enabled\n");

		printf("2. ACL Table\n");
		printf("   Idx  State   Interface  IP Address\n");
		printf("---------------------------------------------\n");

		entryNum = mib_chain_total(MIB_ACL_IP_TBL);
		for (i=0; i<entryNum; i++) {

			if (!mib_chain_get(MIB_ACL_IP_TBL, i, (void *)&Entry))
			{
  				printf("Get chain record error!\n");
			}

			dest.s_addr = *(unsigned long *)Entry.ipAddr;

			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(sdest, inet_ntoa(dest));
			snprintf(sdest, 20, "%s/%d", sdest, Entry.maskbit);
			printf("   %-5d%-8s%-11s%s\n", i+1, Entry.Enabled ? "Enable" : "Disable", (Entry.Interface == IF_DOMAIN_LAN)? "LAN" : "WAN", sdest);
			//printf("(%d) IP Address: %s\n", i+1, sdest);
		}

		printf("\n(1) Capability                   (2) Add");
		printf("\n(3) Delete                       (4) Quit\n\n");
		if (!getInputOption( &snum, 1, 4))
			continue;

		switch(snum)
		{
		case 1: // (1) Capability
			min = 1; max = 2;
			if (0 == getInputUint("ACL Capability (1) Disable (2) Enable: ", &sel, &min, &max))
				continue;
			vChar = (unsigned char)(sel-1);
			mib_set(MIB_ACL_CAPABILITY, (void *)&vChar);
			break;
		case 2: // (2) Add
			if (0==getInputIpAddr("IP Address(xxx.xxx.xxx.xxx): ", (struct in_addr *)&Entry.ipAddr[0]))
				continue;

			//if (0==getInputIpAddr("Subnet Mask: ", (struct in_addr *)&mask))
			if (0==getInputIpMask("Subnet Mask: ", (struct in_addr *)&mask))
				continue;

			mbit=0;
			while (1) {
				if (mask&0x80000000) {
					mbit++;
					mask <<= 1;
				}
				else
					break;
			}
			Entry.maskbit = mbit;

			min = 1; max = 2;
			if (0 == getInputUint("Interface (1) LAN (2) WAN: ", &sel, &min, &max))
				continue;
			Entry.Interface = (sel==1 ? IF_DOMAIN_LAN:IF_DOMAIN_WAN);

			if (0 == getInputUint("State (1)Disable (2)Enable: ", &sel, &min, &max))
				continue;
			Entry.Enabled = sel-1;

			intVal = mib_chain_add(MIB_ACL_IP_TBL, (unsigned char*)&Entry);
			if (intVal == 0) {
				printf("Error! Add chain record.");
			}
			else if (intVal == -1)
				printf("Error! Table Full.");
			break;
		case 3: // (3) Delete
			aclDel();
			printWaitStr();
			break;
		case 4: // (4) Quit
			return;
		}

#if defined(APPLY_CHANGE)
		restart_acl();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
		//return;
	}


}
#endif

#ifdef CONFIG_USER_CWMP_TR069
static void setTr069Config(int action)
{
	char tmpStr[256+1];
	char vChar;
	unsigned int vInt, vMin, vMax;
	char old_vChar;
	char changeflag=0;
	unsigned int informInterv;
	char strPath[256+1];
	int old_port;
	unsigned char cwmp_flag=0;
	char cwmp_flag_value=0;

	// Enable/disable TR069
	if ( action == 1) {    // Enable/disable TR069

		if( mib_get( CWMP_FLAG, (void *)&cwmp_flag ) )
		{
			printf("TR069: %s\n", (cwmp_flag & CWMP_FLAG_AUTORUN)? "Enable":"Disable");

			vMin = 1; vMax =2;
			if (0==getInputUint("Config TR069 (1) Disable (2) Enable :", &vInt, &vMin, &vMax))
				return;
			vChar = (vInt == 1) ? 0 : 1;

			if( vChar == 0) {
				if ( cwmp_flag & CWMP_FLAG_AUTORUN )
					changeflag = 1;

				cwmp_flag = cwmp_flag & (~CWMP_FLAG_AUTORUN);
				cwmp_flag_value = 0;
			}else {
				if ( !(cwmp_flag & CWMP_FLAG_AUTORUN) )
					changeflag = 1;

				cwmp_flag = cwmp_flag | CWMP_FLAG_AUTORUN;
				cwmp_flag_value = 1;
			}

			if ( !mib_set( CWMP_FLAG, (void *)&cwmp_flag)) {
				return;
			}
		}else{
			return;
		}
	} else if ( action == 2 ){    // Modify TR069
		// Get TR069 adminStatus(Disable or Enable)
		mib_get( CWMP_FLAG, (void *)&cwmp_flag );
		if ( cwmp_flag & CWMP_FLAG_AUTORUN )
			cwmp_flag_value = 1;
		else
			cwmp_flag_value = 0;

		// ACS URL
		if (0==getInputString("ACS URL:", tmpStr, sizeof(tmpStr)))
			return;
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
		storeOldACS();
/*star:20100305 END*/

		mib_set(CWMP_ACS_URL, (void *)tmpStr);

		// ACS Username
		if (0==getInputString("ACS Username:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_ACS_USERNAME, (void *)tmpStr);

		// ACS Password
		if (0==getInputString("ACS Password:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_ACS_PASSWORD, (void *)tmpStr);

		// Inform
		vMin = 1; vMax =2;
		if (0==getInputUint("Inform (1) Disable (2) Enable :", &vInt, &vMin, &vMax))
			return;
		vChar = (vInt == 1) ? 0 : 1;
		mib_get( CWMP_INFORM_ENABLE, (void*)&old_vChar);
		if(old_vChar != vChar){
			changeflag = 1;
			if ( !mib_set( CWMP_INFORM_ENABLE, (void *)&vChar)) {
				return;
			}
		}

		// Inform Interval
		if (vChar) {
			vMin=1; vMax =4294967295u;
			if (0==getInputUint("Inform Interval :", &vInt, &vMin, &vMax))
				return;
			mib_get( CWMP_INFORM_INTERVAL, (void*)&informInterv);
			if(vInt != informInterv){
				changeflag = 1;
				if ( !mib_set( CWMP_INFORM_INTERVAL, (void *)&vInt)) {
					return;
				}
			}
		}


		printf("\nConnection Request\n");
		if (0==getInputString("Username:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_CONREQ_USERNAME, (void *)tmpStr);

		if (0==getInputString("Password:", tmpStr, sizeof(tmpStr)))
			return;
		mib_set(CWMP_CONREQ_PASSWORD, (void *)tmpStr);

		// Path
		if (0==getInputString("Path:", tmpStr, 32 ))
			return;
		mib_get( CWMP_CONREQ_PATH, (void *)strPath);
		if (strcmp(tmpStr,strPath)!=0){
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PATH, (void *)tmpStr)) {
				return;
			}
		}

		// Port
		vMin = 1; vMax =65535;
		if (0==getInputUint("Port:", &vInt, &vMin, &vMax))
			return;
		mib_get( CWMP_CONREQ_PORT, (void *)&old_port);
		if ( vInt != old_port ) {
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PORT, (void *)&vInt)) {
				return;
			}
		}
	}

	// Mason Yu
#ifdef APPLY_CHANGE
	if ( changeflag ) {
		if ( cwmp_flag_value == 0 ) {  // disable TR069
			off_tr069();
		} else {                       // enable TR069
			off_tr069();
			if (-1==startCWMP()){
				printf("Start tr069 Fail(CLI)\n");
				return;
			}
		}
	}
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	if(changeflag)
		Commit();
#endif

}

void setTR069() {
	int snum;
	char tmpStr[256+1];
	char vChar;
	int vInt;
	while (1)
	{
		CLEAR;
		MSG_LINE;
		printf("                           TR-069 Configuration                             \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("(1) Show                 (2) Modify\n");
		printf("(3) Enable/Disable       (4) Quit\n");

		if (!getInputOption( &snum, 1, 4))
			continue;

		switch( snum)
		{
		case 1://(1) Show
			mib_get(CWMP_ACS_URL, (void *)tmpStr);
			printf("ACS URL: %s\n", tmpStr);
			mib_get(CWMP_ACS_USERNAME, (void *)tmpStr);
			printf("ACS Username: %s\n", tmpStr);
			mib_get(CWMP_ACS_PASSWORD, (void *)tmpStr);
			printf("ACS Password: %s\n", tmpStr);
			mib_get(CWMP_INFORM_ENABLE, (void *)&vChar);
			printf("Inform: %s\n", vChar ? "Enabled" : "Disabled");
			if (vChar) {
				mib_get(CWMP_INFORM_INTERVAL, (void *)&vInt);
				printf("Inform Interval: %d\n", vInt);
			}
			printf("\nConnection Request\n");
			mib_get(CWMP_CONREQ_USERNAME, (void *)tmpStr);
			printf("Username: %s\n", tmpStr);
			mib_get(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
			printf("Password: %s\n", tmpStr);
			mib_get(CWMP_CONREQ_PATH, (void *)tmpStr);
			printf("PATH: %s\n", tmpStr);
			mib_get(CWMP_CONREQ_PORT, (void *)&vInt);
			printf("Port: %d\n", vInt);

			printWaitStr();
			break;
		case 2:
			setTr069Config(2);   // Modify TR069
			break;
		case 3:
			setTr069Config(1);   // Enable/disable TR069
			break;
		case 4:

			return;
		}

	}
}

#endif

#ifdef CONFIG_USER_FTP_FTP_FTP
static void backupConfig_FTP() {
	char serverIP[16], username[30], password[30], filename[30], fname[30];
	FILE *fp;

	if (0 == getInputString("FTP server IP:", serverIP, sizeof(serverIP)))
		return;
	if (0 == getInputString("User name:", username, sizeof(username)))
		return;
	if (0 == getInputString("Password:", password, sizeof(password)))
		return;
	if (0 == getInputString("Config file name:", filename, sizeof(filename)))
		return;

	//call_cmd("/bin/CreatexmlConfig", 0, 1);
	call_cmd("/bin/saveconfig", 0, 1);
	sprintf(fname, "/tmp/%s", filename);
	cmd_xml2file("/tmp/config.xml", fname);

	if ((fp = fopen("/var/ftpput_conf.txt", "w")) == NULL) {
		printf("***** Open file /var/ftpput_conf.txt failed !\n");
		return;
	}

	fprintf(fp, "open %s\n", serverIP);
	fprintf(fp, "user %s %s\n", username, password);
	fprintf(fp, "lcd /tmp\n");             // It is a directory that config file exist on ADSL
	fprintf(fp, "bin\n");
	fprintf(fp, "put %s\n", filename);       // It is name of the config file on ADSL
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);

	system("/bin/ftp -inv < /var/ftpput_conf.txt");
	//va_cmd("/bin/ftp", 3, 1, "-inv", "-f", "/var/ftpput_conf.txt");
}


static void restoreConfig_FTP() {
	char serverIP[16], username[30], password[30], filename[30], fname[30];
	int ret = -1;
	FILE *fp;

	if (0 == getInputString("FTP server IP:", serverIP, sizeof(serverIP)))
		return;
	if (0 == getInputString("User name:", username, sizeof(username)))
		return;
	if (0 == getInputString("Password:", password, sizeof(password)))
		return;
	if (0 == getInputString("Config file name:", filename, sizeof(filename)))
		return;

	if ((fp = fopen("/var/ftpget_conf.txt", "w")) == NULL) {
		printf("***** Open file /var/ftpget_conf.txt failed !\n");
		return;
	}

	fprintf(fp, "open %s\n", serverIP);
	fprintf(fp, "user %s %s\n", username, password);
	fprintf(fp, "lcd /tmp\n");             // The config file will be saved to this directory on ADSL
	fprintf(fp, "bin\n");
	fprintf(fp, "get %s\n", filename);       // It is name of the config file on ADSL
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);

	system("/bin/ftp -inv < /var/ftpget_conf.txt");

	sprintf(fname, "/tmp/%s", filename);
	cmd_file2xml(fname, "/tmp/config.xml");

	printf("Load config, please wait ...\n");
	//ret = call_cmd("/bin/LoadxmlConfig", 0, 1);
	ret = call_cmd("/bin/loadconfig", 0, 1);
	if (ret == 0) { // load ok
		printf("Writing ...\n");
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
	else { // load fail
		printf("Parsing error!\n");
	}
	/* reboot the system */
	printf("Rebooting ...\n");
	cmd_reboot();

	/*
	if (ret == 0)
		printf("Please commit and reboot the systme to take effect the new configuration.\n");
	else
		printf("ERROR: Restore Config file failed! Invalid config file!\n");
	*/
}

static void updateImage_FTP() {
	char serverIP[16], username[30], password[30], filename[30], fname[30];
	int ret = -1;
	FILE *fp, *fp2;
	struct stat st;

	if (0 == getInputString("FTP server IP:", serverIP, sizeof(serverIP)))
		return;
	if (0 == getInputString("User name:", username, sizeof(username)))
		return;
	if (0 == getInputString("Password:", password, sizeof(password)))
		return;
	if (0 == getInputString("Image file name:", filename, sizeof(filename)))
		return;

	if ((fp = fopen("/var/ftpget_img.txt", "w")) == NULL) {
		printf("***** Open file /var/ftpget_img.txt failed !\n");
		return;
	}

	fprintf(fp, "open %s\n", serverIP);
	fprintf(fp, "user %s %s\n", username, password);
	fprintf(fp, "lcd /tmp\n");               // The config file will be saved to this directory on ADSL
	fprintf(fp, "bin\n");
	fprintf(fp, "get %s\n", filename);       // It is name of the config file on ADSL
	fprintf(fp, "bye\n");
	fprintf(fp, "quit\n");
	fclose(fp);

	system("/bin/ftp -inv < /var/ftpget_img.txt");

	sprintf(fname, "/tmp/%s", filename);
	printf("Updating image ...\n");
	fflush(0);
#ifndef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
	if (cmd_check_image(fname, 0)) {
#endif
		if ((fp2 = fopen(fname, "rb")) == NULL) {
			printf("File %s open fail\n", fname);
			return;
		}

		if (fstat(fileno(fp2), &st) < 0) {
			printf("File %s get status fail\n", fname);
			fclose(fp2);
			return;
		}

		if (st.st_size <= 0) {
			printf("File %s size error\n", fname);
			fclose(fp2);
			return;
		}
		fclose(fp2);
		cmd_upload(fname, 0, st.st_size);
#ifndef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
	}
#endif
}

void setFtp()
{
	int snum;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Ftp utility                                      \n");
		MSG_LINE;
		printf("Update firmware/configuration or backup configuration from/to a Ftp server. \n");
		MSG_LINE;

		/*
		printf("\n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("                 Backup/Restore Settings                                 \n");
		MSG_LINE;
		//printf("-------------------------------------------------------------------------\n");
		printf("This page allows you to backup current settings to a file or restore the \n");
		printf("file which was saved previously by FTP. Besides, you could reset the current\n");
		printf("configuration to factory default.                                        \n");
		MSG_LINE;
		*/

		//printf("-------------------------------------------------------------------------\n");
	   	printf("(1) Backup Configuration File        (2) Restore Configuration File\n");
   		printf("(3) Update image                     (4) Quit\n");
	   	if (!getInputOption(&snum, 1, 4))
   			continue;

		switch (snum) {
			case 1:
				backupConfig_FTP();
				printWaitStr();
				break;
			case 2:
				restoreConfig_FTP();
				printWaitStr();
				break;
			case 3:
				updateImage_FTP();
				printWaitStr();
				break;
			case 4:
				return;
		}
	}
}
#endif

#ifdef CONFIG_USER_RTK_SYSLOG
int ShowSysLog()
{
	FILE *fp;
	char  buf[150];
	unsigned char vChar;

	if(first_time==1){
		mib_get(MIB_ADSL_DEBUG, (void *)&vChar);
		if(vChar==1)
			dbg_enable=1;
		first_time=0;
	}

	if(dbg_enable==0){
		fp = fopen("/var/log/messages.old", "r");
		if (fp) {
			while (fgets(buf, 150, fp))
				printf("%s\n", buf);
			fclose(fp);
		}

		fp = fopen("/var/log/messages", "r");
		if (fp == NULL){
			//printf("read log file fail!\n");
			goto err1;
		}

		while(fgets(buf,150,fp)){
			printf("%s\n", buf);
		}

		fclose(fp);
		printWaitStr();
		return 1;
	}
err1:
	return 0;
}



/* Return value:
 * 0: fail
 * 1: successful
 */
int SystemLogConfig(){
	unsigned char setVal;
	unsigned char getVal;
	unsigned int min,max,sel;
	char msgstr[10];

	mib_get(MIB_SYSLOG, (void *)&getVal);
	if(getVal==1)
		strcpy(msgstr,"ON");
	else
		strcpy(msgstr,"OFF");
	printf("System Log: %s\n",msgstr);
	min=1;max=2;
	if(getInputUint("New System Log (1)ON (2)OFF :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (sel==1)
		setVal = 1;
	else
		setVal = 0;	// default "off"
	if (!mib_set(MIB_SYSLOG, (void *)&setVal)) {
		printf("Set System Log error!");
	}

#if defined(APPLY_CHANGE)
	stopLog();
	startLog();
#endif
	return 1;
}

void setSystemLog()
{
	int snum;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           System Log                                       \n");
		MSG_LINE;
		//printf("Update firmware/configuration or backup configuration from/to a Ftp server. \n");
		MSG_LINE;

		//printf("-------------------------------------------------------------------------\n");
	   	printf("(1) System Log Enable/Disable        (2) Sow Log\n");
   		printf("(3) clear Log                        (4) Quit\n");
	   	if (!getInputOption(&snum, 1, 4))
   			continue;

		switch (snum) {
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				SystemLogConfig();
				break;
			case 2:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				ShowSysLog();
				break;
			case 3:
				unlink("/var/log/messages.old");
				unlink("/var/log/messages");
				printf("Clear Log is finished !!\n");
				printWaitStr();
				break;
			case 4:
				return;
		}
	}
}
#endif

#ifdef TIME_ZONE
void showCurTime()
{
	time_t tm;
	struct tm tm_time, *ptm_time;
	unsigned char strbuf[70];

	time(&tm);
	ptm_time = localtime(&tm);
	snprintf(strbuf, 64, "%d", (ptm_time->tm_year+ 1900));
	printf("Yr:%s ", strbuf);

	snprintf(strbuf, 64, "%d", (ptm_time->tm_mon+ 1));
	printf("Mon:%s ", strbuf);

	snprintf(strbuf, 64, "%d", (ptm_time->tm_mday));
	printf("Day:%s ", strbuf);

	snprintf(strbuf, 64, "%d", (ptm_time->tm_hour));
	printf("Hr:%s ", strbuf);

	snprintf(strbuf, 64, "%d", (ptm_time->tm_min));
	printf("Mn:%s ", strbuf);

	snprintf(strbuf, 64, "%d", (ptm_time->tm_sec));
	printf("Sec:%s\n", strbuf);

	printWaitStr();
}

int setCurTime()
{
	struct tm tm_time;
	time_t tm;
	int min, max, num, intVal;

	tm_time.tm_isdst = -1;  /* Be sure to recheck dst. */

	min = 1902; max = 65535;
	if ( 0 == getInputInt("Year(1902 ~): ", &num, &min, &max) )
            return 0;
	tm_time.tm_year = num - 1900;

	min = 1; max = 12;
	if (0 == getInputInt("Mon: ", &num, &min, &max))
             return 0;
	tm_time.tm_mon = num-1;

	min = 1; max = 31;
	if (0 == getInputInt("Day: ", &num, &min, &max))
             return 0;
	tm_time.tm_mday = num;

	min = 0; max = 24;
	if (0 == getInputInt("Hour: ", &num, &min, &max))
             return 0;
	tm_time.tm_hour = num;

	min = 0; max = 60;
	if (0 == getInputInt("Minute: ", &num, &min, &max))
             return 0;
	tm_time.tm_min = num;

	min = 0; max = 60;
	if (0 == getInputInt("Second: ", &num, &min, &max))
             return 0;
	tm_time.tm_sec = num;

	tm = mktime(&tm_time);
	if(tm < 0){
		printf("set Time Error(1)\n");
	}
	if(stime(&tm) < 0){
		printf("set Time Error(2)\n");
	}
	 return 1;
}

/* Return value:
 * 0: fail
 * 1: successful
 */
int SNTPCconfig(){
	unsigned char ValSet;
	unsigned char ValGet;
	unsigned int min,max,sel;
	char msgstr[10];

	mib_get(MIB_NTP_ENABLED, (void *)&ValGet);
	if(ValGet==1)
		strcpy(msgstr,"ON");
	else
		strcpy(msgstr,"OFF");
	printf("SNTP Client: %s\n",msgstr);
	min=1;max=2;
	if(getInputUint("New SNTP Client (1)ON (2)OFF :",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if (sel==1)
		ValSet = 1;
	else
		ValSet = 0;	// default "off"
	if (!mib_set(MIB_NTP_ENABLED, (void *)&ValSet)) {
		printf("Set SNTP Client error!");
	}

	return 1;
}

/* Return value:
 * 0: fail
 * 1: successful
 */
int selTimeZone()
{
	unsigned int i, min, max, sel_list = 0;
	FILE *fp;
	char tmpBuf[15], setstr[15];
	unsigned char dst_enabled = 1;

	min = 0;
	max = nr_tz - 1;
	printf("Time Zone List:\n");
	for (i = 0; i < nr_tz; i++) {
		printf("\t(%u) %s (UTC%s)\n", i,
				get_tz_location(i, FOR_CLI), get_tz_utc_offset(i));
	}

	if (getInputUint(": ", &sel_list, &min, &max) == 0) {
		printf("Invalid selection!\n");
		return 0;
	}

	if (mib_set(MIB_NTP_TIMEZONE_DB_INDEX, &sel_list) == 0) {
		printf("Set Index of Time Zone Database error for CLI!");
		return 0;
	}

	if ((fp = fopen("/etc/TZ", "w")) != NULL) {
		mib_get(MIB_DST_ENABLED, &dst_enabled);
		fprintf(fp, "%s\n", get_tz_string(sel_list, dst_enabled));
		fclose(fp);
	}

	return 1;
}


/* Return value:
 * 0: fail
 * 1: successful
 */
int setSNTPServer(){
	unsigned int min,max,sel, sel_list;
	char server_id;
	char strbuf[MAX_NAME_LEN];

	printf("Please select one method to set SNTP Server.\n");
	min=1;max=2;
	if(getInputUint("Method: (1)Select a SNTP Server from List (2) Manual SNTP Server setting: ",&sel,&min,&max)==0){
		printf("Invalid selection!\n");
		return 0;
	}

	if ( sel == 1 ) {
		min=1;max=4;
		printf("Time Zone List:\n");
		printf("\t(1)192.5.41.41    - North America\n");
		printf("\t(2)192.5.41.209   - North America\n");
		printf("\t(3)130.149.17.8   - Europe\n");
		printf("\t(4)203.117.180.36 - Asia Pacific\n");

		if(getInputUint(": ",&sel_list,&min,&max)==0){
			printf("Invalid selection!\n");
			return 0;
		}

		server_id = 0;
		mib_set( MIB_NTP_SERVER_ID, (void *)&server_id);

		if ( mib_set(MIB_NTP_SERVER_HOST1, (void *)NTPServer[sel_list - 1]) == 0) {
			printf("Set NTP server from List error!");
			return 0;
		}
	} else {
		if (0 == getInputString("New SNTP Server address:", strbuf, sizeof(strbuf)))
			return 0;
		server_id = 1;
		mib_set( MIB_NTP_SERVER_ID, (void *)&server_id);

		if ( mib_set(MIB_NTP_SERVER_HOST2, strbuf) == 0) {
			printf("Set NTP server Manually error!");
			return 0;
		}
	}
	return 1;
}

void setTimeZone()
{
	int snum, ret;
	while (1)
	{
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                           Time Zone                                          \n");
		MSG_LINE;
		printf("You can maintain the system time by synchronizing with a public time server   \n");
		printf("over the Internet  \n");
		MSG_LINE;

		//printf("-------------------------------------------------------------------------\n");
	   	printf("(1) Show Current Time                (2) Set Current Time\n");
   		printf("(3) Select Time Zone                 (4) Enable/Disable SNTP Client\n");
   		printf("(5) Set SNTP server                  (6) Quit\n");
	   	if (!getInputOption(&snum, 1, 6))
   			continue;
		ret = 0;
		switch (snum) {
			case 1:  //(1) Show Current Time
				if (!check_access(SECURITY_SUPERUSER))
					break;
				showCurTime();
				break;
			case 2: // (2) Set Current Time
				if (!check_access(SECURITY_SUPERUSER))
					break;
				setCurTime();
				break;
			case 3:	// (3) Select Time Zone
				ret = selTimeZone();
				break;
			case 4: // (4) Enable/Disable SNTP Client
				ret = SNTPCconfig();
				break;
			case 5: // (5) Set SNTP server
				ret = setSNTPServer();
				break;
			case 6:
				return;
		}
#ifdef APPLY_CHANGE
		if ( ret )
			restartSNTPC();
#endif
	}
}
#endif

#ifdef DOS_SUPPORT
void showDoS()
{
	unsigned int mode = 0, uIntValue = 0;

	mib_get(MIB_DOS_ENABLED, (void *)&mode);

	printf("DoS Block\t\t\t: %s\n", (mode & DOSENABLE)?STR_ENABLE: STR_DISABLE);
	mib_get(MIB_DOS_SYSSYN_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: SYN\t\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODSYN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_SYSFIN_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: FIN\t\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODFIN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_SYSUDP_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: UDP\t\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODUDP)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_SYSICMP_FLOOD, (void *)&uIntValue);
	printf("Whole System Flood: ICMP\t: %s\t%d packets/second\n", (mode & DOSSYSFLOODICMP)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPSYN_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: SYN\t: %s\t%d packets/second\n", (mode & DOSIPFLOODSYN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPFIN_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: FIN\t: %s\t%d packets/second\n", (mode & DOSIPFLOODFIN)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPUDP_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: UDP\t: %s\t%d packets/second\n", (mode & DOSIPFLOODUDP)?STR_ENABLE: STR_DISABLE, uIntValue);
	mib_get(MIB_DOS_PIPICMP_FLOOD, (void *)&uIntValue);
	printf("Per-Source IP Flood: ICMP\t: %s\t%d packets/second\n", (mode & DOSIPFLOODICMP)?STR_ENABLE: STR_DISABLE, uIntValue);
	printf("TCP/UDP PortScan\t\t: %s\t%s Sensitivity\n", (mode & DOSTCPUDPPORTSCAN)?STR_ENABLE: STR_DISABLE, (mode & DOSPORTSCANSENSI)?"High": "Low");
	printf("ICMP Smurf\t\t\t: %s\n", (mode & DOSICMPSMURFENABLED)?STR_ENABLE: STR_DISABLE);
	printf("IP Land\t\t\t\t: %s\n", (mode & DOSIPLANDENABLED)?STR_ENABLE: STR_DISABLE);
	printf("IP Spoof\t\t\t: %s\n", (mode & DOSIPSPOOFENABLED)?STR_ENABLE: STR_DISABLE);
	printf("IP TearDrop\t\t\t: %s\n", (mode & DOSIPTEARDROPENABLED)?STR_ENABLE: STR_DISABLE);
	printf("PingOfDeath\t\t\t: %s\n", (mode & DOSPINTOFDEATHENABLED)?STR_ENABLE: STR_DISABLE);
	printf("TCP Scan\t\t\t: %s\n", (mode & DOSTCPSCANENABLED)?STR_ENABLE: STR_DISABLE);
	printf("TCP SynWithData\t\t\t: %s\n", (mode & DOSTCPSYNWITHDATAENABLED)?STR_ENABLE: STR_DISABLE);
	printf("UDP Bomb\t\t\t: %s\n", (mode & DOSUDPBOMBENABLED)?STR_ENABLE: STR_DISABLE);
	printf("UDP EchoChargen\t\t\t: %s\n", (mode & DOSUDPECHOCHARGENENABLED)?STR_ENABLE: STR_DISABLE);
	mib_get(MIB_DOS_BLOCK_TIME, (void *)&uIntValue);
	printf("Source IP Blocking\t\t: %s\t%d Block Interval (second)\n", (mode & DOSSOURCEIPBLOCK)?STR_ENABLE: STR_DISABLE, uIntValue);
}

void changeDoS()
{
	int min, max, sel;
	unsigned int enabled = 0;
	unsigned int floodCount = 0, blockTimer = 0;

	mib_get(MIB_DOS_ENABLED, (void *)&enabled);
	min=1; max=2;
	if (0 == getInputUint("DoS Block (1)Disable (2)Enable :", &sel, &min, &max))
		return;
	if (sel == 1) {
		enabled = 0;
		mib_set(MIB_DOS_ENABLED, (void *)&enabled);
		return;
	}
	else if (sel == 2) {
		unsigned int num, floodCount, retVal, floodmin, floodmax;
		unsigned int sysfloodSYNcount, sysfloodFINcount, sysfloodUDPcount, sysfloodICMPcount;
		unsigned int ipfloodSYNcount, ipfloodFINcount, ipfloodUDPcount, ipfloodICMPcount;
		unsigned int blockTimer;

		enabled |= DOSENABLE;

		min=1; max=2;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: SYN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSSYN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODSYN;
			else if (num == 2) {
				enabled |= DOSSYSFLOODSYN;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodSYNcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: FIN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSFIN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODFIN;
			else if (num == 2) {
				enabled |= DOSSYSFLOODFIN;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodFINcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: UDP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSUDP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODUDP;
			else if (num == 2) {
				enabled |= DOSSYSFLOODUDP;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodUDPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Whole System Flood: ICMP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_SYSICMP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSYSFLOODICMP;
			else if (num == 2) {
				enabled |= DOSSYSFLOODICMP;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		sysfloodICMPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: SYN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPSYN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODSYN;
			else if (num == 2) {
				enabled |= DOSIPFLOODSYN;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodSYNcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: FIN (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPFIN_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODFIN;
			else if (num == 2) {
				enabled |= DOSIPFLOODFIN;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodFINcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: UDP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPUDP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODUDP;
			else if (num == 2) {
				enabled |= DOSIPFLOODUDP;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodUDPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Per-Source IP Flood: ICMP (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_PIPICMP_FLOOD, (void *)&floodCount);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPFLOODICMP;
			else if (num == 2) {
				enabled |= DOSIPFLOODICMP;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Flood Count (packets/second) :", &floodCount, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		ipfloodICMPcount = floodCount;
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TCP/UDP PortScan (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSTCPUDPPORTSCAN;
			else if (num == 2) {
				enabled |= DOSTCPUDPPORTSCAN;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Sensitivity: (1)Low (2)High :", &floodCount, &min, &max);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				} else if (inputRet == -2)
					enabled &= ~DOSPORTSCANSENSI;
				else {
					if (floodCount == 1)
						enabled &= ~DOSPORTSCANSENSI;
					else
						enabled |= DOSPORTSCANSENSI;
				}
			}
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "ICMP Smurf (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSICMPSMURFENABLED;
			else
				enabled |= DOSICMPSMURFENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "IP Land (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPLANDENABLED;
			else
				enabled |= DOSIPLANDENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "IP Spoof (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPSPOOFENABLED;
			else
				enabled |= DOSIPSPOOFENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "IP TearDrop (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSIPTEARDROPENABLED;
			else
				enabled |= DOSIPTEARDROPENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "PingOfDeath (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSPINTOFDEATHENABLED;
			else
				enabled |= DOSPINTOFDEATHENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TCP Scan (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSTCPSCANENABLED;
			else
				enabled |= DOSTCPSCANENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "TCP SynWithData (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSTCPSYNWITHDATAENABLED;
			else
				enabled |= DOSTCPSYNWITHDATAENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "UDP Bomb (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSUDPBOMBENABLED;
			else
				enabled |= DOSUDPBOMBENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "UDP EchoChargen (1)Disable (2)Enable :", &num, &min, &max);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSUDPECHOCHARGENENABLED;
			else
				enabled |= DOSUDPECHOCHARGENENABLED;
		}
		retVal = getTypedInputDefault(INPUT_TYPE_UINT, "Source IP Blocking: (1)Disable (2)Enable :", &num, &min, &max);
		mib_get(MIB_DOS_BLOCK_TIME, (void *)&blockTimer);
		if (retVal == 0) {
			printf("Invalid value!\n");
			printWaitStr();
			return;
		} else if (retVal == 1) {
			if (num == 1)
				enabled &= ~DOSSOURCEIPBLOCK;
			else if (num == 2) {
				enabled |= DOSSOURCEIPBLOCK;
				floodmin = 0; floodmax = 65535;
				int inputRet = getTypedInputDefault(INPUT_TYPE_UINT,"Block Interval (second) :", &blockTimer, &floodmin, &floodmax);
				if (inputRet == 0) {
					printf("Invalid selection!\n");
					printWaitStr();
					return;
				}
			}
		}
		mib_set(MIB_DOS_ENABLED, (void *)&enabled);
		mib_set(MIB_DOS_SYSSYN_FLOOD, (void *)&sysfloodSYNcount);
		mib_set(MIB_DOS_SYSFIN_FLOOD, (void *)&sysfloodFINcount);
		mib_set(MIB_DOS_SYSUDP_FLOOD, (void *)&sysfloodUDPcount);
		mib_set(MIB_DOS_SYSICMP_FLOOD, (void *)&sysfloodICMPcount);
		mib_set(MIB_DOS_PIPSYN_FLOOD, (void *)&ipfloodSYNcount);
		mib_set(MIB_DOS_PIPFIN_FLOOD, (void *)&ipfloodFINcount);
		mib_set(MIB_DOS_PIPUDP_FLOOD, (void *)&ipfloodUDPcount);
		mib_set(MIB_DOS_PIPICMP_FLOOD, (void *)&ipfloodICMPcount);
		mib_set(MIB_DOS_BLOCK_TIME, (void *)&blockTimer);
	}
}

void setDoS()
{
	int min, max, sel;
	while (1) {
		CLEAR;
		printf("\n");
		MSG_LINE;
		printf("                      DoS Setting             \n");
		MSG_LINE;
		printf("DoS(\"denial-of-service\") attack which is launched by hacker aims to prevent \n");
		printf("legal user from taking normal services. In this page you can configure to \n");
		printf("prevent some kinds of DOS attack.\n");
		MSG_LINE;
		showDoS();
		min=1; max=2;
		if (getInputUint("(1) Set DoS Block (2) Quit:", &sel, &min, &max)==0) {
			printf("Invalid selection!\n");
			return;
		}
		switch (sel) {
			case 1:
				if (!check_access(SECURITY_SUPERUSER))
					break;
				changeDoS();
#if defined(APPLY_CHANGE)
				setup_dos_protection();
#endif
//Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
				break;
			case 2:
				return;
		}
	}
}
#endif

void setAdmin()
{
	int snum;

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                      (8) Admin Menu                             \n");
		MENU_LINE;
		printf("(1) Commit/Reboot                (2) TFTP\n");
#ifdef ACCOUNT_CONFIG
		printf("(3) Account Configuration        (4) ACL Configuration\n");
#else
		printf("(3) Password                     (4) ACL Configuration\n");
#endif
		printf("(5) TR-069                       (6) FTP\n");
		printf("(7) System Log                   (8) Time Zone\n");
		printf("(9) DOS                          (10) Quit\n");

		//if (!getInputOption( &snum, 1, 5))
		if (!getInputOption( &snum, 1, 10))
			continue;

		switch( snum)
		{
		case 1://(1) Commit/Reboot
			setCommitReboot();
			break;
		// kaotets
		case 2://(2) Tftp
			#if defined CONFIG_USER_BUSYBOX_TFTP || defined CONFIG_USER_BUSYBOX_BUSYBOX1124 && CONFIG_TFTP
			setTftp();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
		case 3://(//(2) Password
			setPassword();
			break;
#if 0
		case 4://(4) Upgrade Firmware
			printf("(4) Upgrade Firmware\n");
			break;
#endif
		case 4://(3) ACL Config
#ifdef IP_ACL
			setACL();
#endif
			break;
		case 5:
			#if CONFIG_USER_CWMP_TR069
			setTR069();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
		case 6:  // (6) Ftp
			#ifdef CONFIG_USER_FTP_FTP_FTP
			setFtp();
			#else
			printf("Not supported\n");
			printWaitStr();
			#endif
			break;
		case 7:  // (7) System Log
#ifdef CONFIG_USER_RTK_SYSLOG
			setSystemLog();
#else
			printf("Not supported\n");
#endif
			break;
		case 8:  // (8) Time Zone
#ifdef TIME_ZONE
			setTimeZone();
#else
			printf("Not supported\n");
#endif
			break;
		case 9:  // (9) DoS
#ifdef DOS_SUPPORT
			setDoS();
#else
			printf("Not supported\n");
			printWaitStr();
#endif
			break;
		case 10:
			return;
		default:
			break;
		}//end switch, (8) Admin Menu
	}//end while, (8) Admin Menu
}
/*************************************************************************************************/
void setStatistics()
{
	int i;
#if defined(EMBED) && defined(CONFIG_DEV_xDSL)
	int skfd;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;
#endif
	char *names[16];
	char ifname[IFNAMSIZ];
	int num_itf;
	struct net_device_stats nds;

	CLEAR;
	printf("\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("                                Statistics                               \n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("This page shows the packet statistics for transmission and reception     \n");
	printf("regarding to network interface.                                          \n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("\nItf\tRx_pkt\tRx_err\tRx_drop\tTx_pkt\tTx_err\tTx_drop\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");

	// Ethernet statistics
	num_itf=0;
	for (i=0; i<ELANVIF_NUM; i++) {
		names[i] = (char *)ELANVIF[i];
		num_itf++;
	}
#ifdef CONFIG_USB_ETH
	names[num_itf++] = (char *)USBETHIF;
#endif
#ifdef WLAN_SUPPORT
	names[num_itf++] = "wlan0";
#endif
#ifdef CONFIG_ETHWAN
	names[num_itf++] = ALIASNAME_NAS0;
#endif

	// LAN statistics
	for (i = 0; i < num_itf; i++) {
		get_net_device_stats(names[i], &nds);
		printf("%s\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
			names[i], nds.rx_packets, nds.rx_errors,
			nds.rx_dropped, nds.tx_packets,
			nds.tx_errors, nds.tx_dropped);
	}

#if defined(EMBED) && defined(CONFIG_DEV_xDSL)
	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return;
	}

	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return;
		}

		if (cfg.created == 0)
			continue;

      printf("%u/%u\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
      cfg.vpi, cfg.vci, cfg.stat.rx_pkt_cnt, cfg.stat.rx_pkt_fail,
		cfg.stat.rx_crc_error, cfg.stat.tx_desc_ok_cnt,
		cfg.stat.tx_pkt_fail_cnt, cfg.stat.send_desc_lack);
	}
	(void)close(skfd);
#endif

#ifdef CONFIG_PTMWAN
if(WAN_MODE & MODE_PTM)
{
	int entryNum;
	int mibcnt;
	MIB_CE_ATM_VC_T Entry;
	char ifDisplayName[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (mibcnt = 0; mibcnt < entryNum; mibcnt++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, &Entry)) {
			printf("MULTI_PTM_WAN: get MIB chain error\n");
			continue;
		}

		if (Entry.enable == 0)
			continue;

		if(MEDIA_INDEX(Entry.ifIndex) != MEDIA_PTM)
			continue;

		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
		get_net_device_stats(ifname, &nds);
		getDisplayWanName(&Entry, ifDisplayName);

		printf("%s\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
			ifDisplayName, nds.rx_packets,
			nds.rx_errors, nds.rx_dropped,
			nds.tx_packets, nds.tx_errors,
			nds.tx_dropped);
	}
}
#endif	/*CONFIG_PTMWAN*/


#ifdef CONFIG_USER_PPPOMODEM
	{
		MIB_WAN_3G_T entry, *p;

		p = &entry;
		if (mib_chain_get(MIB_WAN_3G_TBL, 0, p)) {
			if (p->enable) {
				sprintf(ifname, "ppp%d", MODEM_PPPIDX_FROM);

				get_net_device_stats(ifname, &nds);
				printf("3G_%s\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
					ifname, nds.rx_packets,
					nds.rx_errors, nds.rx_dropped,
					nds.tx_packets, nds.tx_errors,
					nds.tx_dropped);
			}
		}
	}
#endif //CONFIG_USER_PPPOMODEM

	MSG_LINE;
	printWaitStr();
}

#ifdef CONFIG_DEV_xDSL
static void adslStatistic(XDSL_OP *d)
{
	char strbuf[256];
#ifdef CONFIG_VDSL
	char *pDSL="DSL";
#else
	char *pDSL="ADSL";
#endif /*CONFIG_VDSL*/

	CLEAR;
	printf("\n");
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("                        Statistics -- %s                           \n", pDSL);
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");
	printf("This page shows the current %s statistics.\n", pDSL);
	MSG_LINE;
	//printf("-------------------------------------------------------------------------\n");

	d->xdsl_get_info(ADSL_GET_MODE, strbuf, 256);
	printf("Mode\t\t\t: %s\n", strbuf);
#ifdef CONFIG_VDSL
	d->xdsl_get_info(DSL_GET_TPS, strbuf, 256);
	printf("TPS-TC\t\t\t: %s\n", strbuf);
#endif /*CONFIG_VDSL*/
	d->xdsl_get_info(ADSL_GET_LATENCY, strbuf, 256);
	printf("Latency\t\t\t: %s\n", strbuf);
#ifndef CONFIG_VDSL
	d->xdsl_get_info(ADSL_GET_TRELLIS, strbuf, 256);
	printf("Trellis Coding\t\t: %s\n", strbuf);
#endif /*CONFIG_VDSL*/
	d->xdsl_get_info(ADSL_GET_STATE, strbuf, 256);
	printf("Status\t\t\t: %s\n", strbuf);
	d->xdsl_get_info(ADSL_GET_POWER_LEVEL, strbuf, 256);
	printf("Power Level\t\t: %s\n", strbuf);
	printf("\n");

	printf("\t\t\t\t\t\tDownstream");
	printf("\tUptream\n");
#ifdef CONFIG_VDSL
	d->xdsl_get_info(DSL_GET_TRELLIS_DS, strbuf, 256);
	printf("Trellis\t\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(DSL_GET_TRELLIS_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
#endif /*CONFIG_VDSL*/
	d->xdsl_get_info(ADSL_GET_SNR_DS, strbuf, 256);
	printf("SNR Margin (dB)\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_SNR_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_LPATT_DS, strbuf, 256);
	printf("Attenuation (dB)\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_LPATT_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_POWER_DS, strbuf, 256);
	printf("Power (dBm)\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_POWER_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_ATTRATE_DS, strbuf, 256);
	printf("Attainable Rate (Kbps)\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_ATTRATE_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

#ifdef CONFIG_VDSL
	d->xdsl_get_info(DSL_GET_PHYR_DS, strbuf, 256);
	printf("G.INP\t\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(DSL_GET_PHYR_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
#endif /*CONFIG_VDSL*/

	d->xdsl_get_info(ADSL_GET_RATE_DS, strbuf, 256);
	printf("Rate (Kbps)\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_RATE_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

#ifndef CONFIG_VDSL
	d->xdsl_get_info(ADSL_GET_K_DS, strbuf, 256);
	printf("K (number of bytes in DMT frame)\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_K_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
#endif /*CONFIG_VDSL*/

	d->xdsl_get_info(ADSL_GET_R_DS, strbuf, 256);
	printf("R (number of check bytes in RS code word)\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_R_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

#ifdef CONFIG_VDSL
	d->xdsl_get_info(DSL_GET_N_DS, strbuf, 256);
	printf("N (RS codeword size)\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(DSL_GET_N_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
	d->xdsl_get_info(DSL_GET_L_DS, strbuf, 256);
	printf("L (number of bits in DMT frame)\t\t\t: %s", strbuf);
	d->xdsl_get_info(DSL_GET_L_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
#endif /*CONFIG_VDSL*/

	d->xdsl_get_info(ADSL_GET_S_DS, strbuf, 256);
	printf("S (RS code word size in DMT frame)\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_S_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_D_DS, strbuf, 256);
	printf("D (interleaver depth)\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_D_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_DELAY_DS, strbuf, 256);
	printf("Delay (msec)\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_DELAY_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

#ifdef CONFIG_VDSL
	d->xdsl_get_info(DSL_GET_INP_DS, strbuf, 256);
	printf("INP (DMT frame)\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(DSL_GET_INP_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);
#endif /*CONFIG_VDSL*/

	d->xdsl_get_info(ADSL_GET_FEC_DS, strbuf, 256);
#ifdef CONFIG_VDSL
	printf("FEC errors\t\t\t\t\t: %s", strbuf);
#else
	printf("FEC\t\t\t\t\t\t: %s", strbuf);
#endif /*CONFIG_VDSL*/
	d->xdsl_get_info(ADSL_GET_FEC_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

#ifdef CONFIG_VDSL
	d->xdsl_get_info(ADSL_GET_RX_FRAMES, strbuf, 256);
	printf("OH Frame\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_TX_FRAMES, strbuf, 256);
	printf("\t\t%s\n", strbuf);
#endif /*CONFIG_VDSL*/

	d->xdsl_get_info(ADSL_GET_CRC_DS, strbuf, 256);
#ifdef CONFIG_VDSL
	printf("OH Frame errors\t\t\t\t\t: %s", strbuf);
#else
	printf("CRC\t\t\t\t\t\t: %s", strbuf);
#endif /*CONFIG_VDSL*/
	d->xdsl_get_info(ADSL_GET_CRC_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_ES_DS, strbuf, 256);
	printf("Total ES\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_ES_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_SES_DS, strbuf, 256);
	printf("Total SES\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_SES_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	d->xdsl_get_info(ADSL_GET_UAS_DS, strbuf, 256);
	printf("Total UAS\t\t\t\t\t: %s", strbuf);
	d->xdsl_get_info(ADSL_GET_UAS_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	printf("\n");
	printWaitStr();
}
#endif

void setStatistics_start()
{
	int snum, menu_size;

	while (1)
	{
		CLEAR;
		printf("\n");
		MENU_LINE;
		printf("                          (9) Statistic Menu                         \n");
		MENU_LINE;

		menu_size = cli_show_dyn_menu(StatisticsMenu);

		if(0 == getInputOption(&snum, 1, menu_size))
			continue;

		switch(sta_menu_idx[snum-1])
		{
			case MENU_STA_INTERFACE: // Interface
				setStatistics();
				break;
			case MENU_STA_ADSL: // ADSL
#ifdef CONFIG_DEV_xDSL
				adslStatistic( xdsl_get_op(0) );
#else
				printf("Not supported\n");
				printWaitStr();
#endif
				break;

#ifdef CONFIG_USER_XDSL_SLAVE
			case MENU_STA_ADSL_SLAVE: // DSL Slave
				adslStatistic( xdsl_get_op(1) );
				break;
#endif /*CONFIG_USER_XDSL_SLAVE*/

			case MENU_STA_QUIT: // Quit
				return;
			default:
				break;
		}
	}
}

void leave(void)
{
	_exit(exstat);
	/* NOTREACHED */
}

#ifdef EMBED
#define CLI_RUNFILE	"/var/run/cli.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CLI_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
#endif

// Kaohj
static struct passwd *getrealpass(const char *user) {
	struct passwd *pwp;

	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp;
}

static void sigexit(int dummy)
{
	if (!console_flag)
		unlink(CLI_RUNFILE);
	exit(0);
}

static void sigchld(int dummy)
{
	int status;
	pid_t pid;
	
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0);
	return;
}

int main(int argc, char *argv[])
{
	int snum, auth_denied=1; //auth_denied=1: Login Incorrect, auth_denied=0: Login Success
	int flag, login=0;
	unsigned char uname[10], *upasswd;
	struct passwd *pwp;
	char loginUser[MAX_NAME_LEN];

	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, sigexit);
	signal(SIGHUP, sigexit);
	signal(SIGCHLD, sigchld);

	// check the login option
	// Usage: cli -l -c -u loginName
	while ((flag = getopt(argc, argv, "lcu:")) != EOF) {
		switch (flag) {
			case 'l':
				login = 1;
				break;
			case 'c':
				console_flag = 1;
				break;
			case 'u':
				if (optarg == NULL)
					break;
				strncpy(loginUser, optarg, MAX_NAME_LEN);
				break;
			default:
				break;
		}
	}

	if (!console_flag)
#ifdef EMBED
		log_pid();
#endif
	// Kaohj, get the login priviledge
	pwp = getrealpass(loginUser);
	//printf("uid=%d\n", pwp->pw_uid);
	if (pwp && pwp->pw_uid == 0)
		loginLevel = SECURITY_ROOT;
	else
		loginLevel = SECURITY_USER;

	login_flag = login;

	InitDynamicMenu();

	while (1)
	{
		// Jenny, CLI user/passwd
		if (login && auth_denied) {
			uname[0] = '\0';
			while (strlen(uname) == 0) {
				if (0 == getInputString("User: ", uname, sizeof(uname)))
					printf("!! Input User Name !!\n");
			}

			upasswd = getpass("Password: ");
			if ((auth_denied = auth_cli(uname, upasswd)) != 0)
			{
				printf("Login Incorrect!\n\n");
				auth_denied = 1;
			}
		}
		else
		{
			CLEAR;
			printf("\n");
			MENU_LINE;
			#ifdef CONFIG_VDSL
			printf("                              VDSL Main Menu                     \n");
			#else
			printf("                              ADSL Main Menu                     \n");
			#endif /*CONFIG_VDSL*/
			MENU_LINE;
			printf("(1)  Status                       (2)  LAN Interface\n");
			printf("(3)  Wireless                     (4)  WAN Interface\n");
			printf("(5)  Services                     (6)  Advance\n");
			printf("(7)  Diagnostic                   (8)  Admin\n");
			printf("(9)  Statistics                   (10) Logout\n");
			snum = -1;
			getInputOption( &snum, 0, 10);
			if (exstat==0)
				leave();
			switch( snum)
			{
				case 0:// shell
					if (!check_access(SECURITY_SUPERUSER))
						break;
					printf("\nShell command mode\nEnter \"exit\" to exit shell...\n");
					va_cmd("/bin/sh", 0, 1);
					break;
				case 1://(1) Status
					showStatus();
					break;
				case 2://(2) LAN Interface
					setLANInterface();
					break;
				case 3://(3) Wireless
#ifdef WLAN_SUPPORT
					setWireless();
#else
					printf("Not supported\n");
					printWaitStr();
#endif
					break;
				case 4://(4) WAN Interface
					setWANInterface();
					break;
				case 5://(5) Services
					setServices();
					break;
				case 6://(6) Advance
					setAdvance();
					break;
				case 7://(7) Diagnostic
					setDiagnostic();
					break;
				case 8://(8) Admin
					if (!check_access(SECURITY_SUPERUSER))
						break;
					setAdmin();
					break;
				case 9://(9) Statistics
					//setStatistics();
					setStatistics_start();
					break;
				case 10://(10) Logout
					CLEAR;
#ifdef EMBED
					if (!console_flag)
						unlink(CLI_RUNFILE);
#endif
					return 0;
					break;
				default:
					break;
			}//end switch, Realtek ADSL Main Menu
		} // Jenny, end if auth
	}//end While, Realtek ADSL Main Menu
	return 0;
}

