/*
 *      Web server handler routines for TCP/IP stuffs
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */
/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "utility.h"
#include "../defs.h"
#include "debug.h"
#include "multilang.h"

#ifdef CONFIG_RTK_RG_INIT
#include <rtk_rg_struct.h>
#include "rtusr_rg_api.h"
#endif
#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

#ifdef CONFIG_USER_VLAN_ON_LAN
void formVLANonLAN(request * wp, char *path, char *query)
{
	char *str, *submitUrl;
	char tmpBuf[100];
	unsigned short value;
	unsigned char vUChar, changed, total_changed;
	MIB_CE_SW_PORT_T sw_entry;

	// (1) Clean all rules and setting 
#ifdef CONFIG_USER_BRIDGE_GROUPING
	setup_bridge_grouping(DEL_RULE);
#endif
	setup_VLANonLAN(DEL_RULE);

	// Set LAN1 VID
	if (mib_chain_get(MIB_SW_PORT_TBL, 0, &sw_entry) == 0) {
		sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	changed = 0;
	str = boaGetVar(wp, "lan1_vid", "");
	if (str[0]) {
		value = strtoul(str, NULL, 10);
		if (sw_entry.vid != value) {
			sw_entry.vid = value;
			changed++;
		}
	}
	str = boaGetVar(wp, "lan1_vid_cap", "");
	if (str[0]) {
		vUChar = strtoul(str, NULL, 10);
		if (sw_entry.vlan_on_lan_enabled != vUChar) {
			sw_entry.vlan_on_lan_enabled = vUChar;
			changed++;
		}
	}
	if (changed && mib_chain_update(MIB_SW_PORT_TBL, &sw_entry, 0) == 0) {
		sprintf(tmpBuf, "%s:%s", Tupdate_chain_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	total_changed = changed;

	// Set LAN2 VID
	if (mib_chain_get(MIB_SW_PORT_TBL, 1, &sw_entry) == 0) {
		sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	changed = 0;
	str = boaGetVar(wp, "lan2_vid", "");
	if (str[0]) {
		value = strtoul(str, NULL, 10);
		if (sw_entry.vid != value) {
			sw_entry.vid = value;
			changed++;
		}
	}
	str = boaGetVar(wp, "lan2_vid_cap", "");
	if (str[0]) {
		vUChar = strtoul(str, NULL, 10);
		if (sw_entry.vlan_on_lan_enabled != vUChar) {
			sw_entry.vlan_on_lan_enabled = vUChar;
			changed++;
		}
	}
	if (changed && mib_chain_update(MIB_SW_PORT_TBL, &sw_entry, 1) == 0) {
		sprintf(tmpBuf, "%s:%s", Tupdate_chain_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	total_changed += changed;

	// Set LAN3 VID
	if (mib_chain_get(MIB_SW_PORT_TBL, 2, &sw_entry) == 0) {
		sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	changed = 0;
	str = boaGetVar(wp, "lan3_vid", "");
	if (str[0]) {
		value = strtoul(str, NULL, 10);
		if (sw_entry.vid != value) {
			sw_entry.vid = value;
			changed++;
		}
	}
	str = boaGetVar(wp, "lan3_vid_cap", "");
	if (str[0]) {
		vUChar = strtoul(str, NULL, 10);
		if (sw_entry.vlan_on_lan_enabled != vUChar) {
			sw_entry.vlan_on_lan_enabled = vUChar;
			changed++;
		}
	}
	if (changed && mib_chain_update(MIB_SW_PORT_TBL, &sw_entry, 2) == 0) {
		sprintf(tmpBuf, "%s:%s", Tupdate_chain_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	total_changed += changed;

	// Set LAN4 VID
	if (mib_chain_get(MIB_SW_PORT_TBL, 3, &sw_entry) == 0) {
		sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	changed = 0;
	str = boaGetVar(wp, "lan4_vid", "");
	if (str[0]) {
		value = strtoul(str, NULL, 10);
		if (sw_entry.vid != value) {
			sw_entry.vid = value;
			changed++;
		}
	}
	str = boaGetVar(wp, "lan4_vid_cap", "");
	if (str[0]) {
		vUChar = strtoul(str, NULL, 10);
		if (sw_entry.vlan_on_lan_enabled != vUChar) {
			sw_entry.vlan_on_lan_enabled = vUChar;
			changed++;
		}
	}
	if (changed && mib_chain_update(MIB_SW_PORT_TBL, &sw_entry, 3) == 0) {
		sprintf(tmpBuf, "%s:%s", Tupdate_chain_error, MIB_SW_PORT_TBL);
		goto setErr_vlan_on_lan;
	}
	total_changed += changed;

	// (2) setup all configuration and rules
	setup_VLANonLAN(ADD_RULE);
#ifdef CONFIG_USER_BRIDGE_GROUPING
	setup_bridge_grouping(ADD_RULE);
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
	unsigned int set_wanlist = 0;

	if (set_port_binding_mask(&set_wanlist) > 0)
	{
		rg_set_port_binding_mask(set_wanlist);
	}
#endif
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	if (total_changed)
		Commit();
#endif

#ifndef NO_ACTION
	pid = fork();
	if (pid)
		waitpid(pid, NULL, 0);
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH,
			 _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl(tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl(tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);
	return;

setErr_vlan_on_lan:
	ERR_MSG(tmpBuf);
}
#endif

///////////////////////////////////////////////////////////////////
void formTcpipLanSetup(request * wp, char *path, char *query)
{
	char	*pStr, *strIp, *strMask, *strSnoop, *submitUrl, *strBlock;
	struct in_addr inIp, inMask;
	char tmpBuf[100], mode;
#if defined(CONFIG_SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
	char dhcp_pool;
#endif
#ifndef NO_ACTION
	int pid;
#endif
#ifdef CONFIG_IGMP_FORBID
	char *str_igmpforbid;

#endif
	unsigned char vChar;

//star:for ip change
	struct in_addr origIp,origMask;
	int ip_mask_changed_flag = 0;
	int igmp_changed_flag = 0;
	int ip2_changed_flag = 0;
#ifdef WLAN_SUPPORT
	int wireless_block_flag = 0;
#endif
#ifdef CONFIG_RTK_RG_INIT
	unsigned int mbtd_changed = 0, vInt, cur_vInt;
	unsigned char cur_vChar, ip6_addr[IP6_ADDR_LEN], cur_ip6_addr[IP6_ADDR_LEN], ipv6_mode;
	char *strRGTmp;
#endif

#if defined(CONFIG_USER_UPNPD) || defined(CONFIG_USER_MINIUPNPD)
	unsigned char upnpdEnable;
	unsigned int upnpItf;
	char ext_ifname[IFNAMSIZ];
#endif

	// Set clone MAC address
	strIp = boaGetVar(wp, "ip", "");
	strMask = boaGetVar(wp, "mask", "");

	if (!isValidHostID(strIp, strMask)) {
		strcpy(tmpBuf, multilang(LANG_INVALID_IP_SUBNET_MASK_COMBINATION));
		goto setErr_tcpip;
	}

	if ( strIp[0] ) {
		if ( !inet_aton(strIp, &inIp) ) {
			strcpy(tmpBuf, strWrongIP);
			goto setErr_tcpip;
		}

		mib_get(MIB_ADSL_LAN_IP, (void *)&origIp);

		if(origIp.s_addr != inIp.s_addr)		
		{
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
			delete_dsldevice_on_hosts();
#endif
			ip_mask_changed_flag = 1;
		/*lan ip & dhcp gateway setting should be set independently*/
		#if 1
			{
				struct in_addr dhcp_gw;
				mib_get(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&dhcp_gw);
				if(dhcp_gw.s_addr==origIp.s_addr)
					mib_set(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inIp);
			}
		#else
			mib_set(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inIp);
		#endif

		// Magician: UPnP Daemon Start
		#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			if(mib_get(MIB_UPNP_DAEMON, (void *)&upnpdEnable))
			{
				if(mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf))
				{
					ifGetName(upnpItf, ext_ifname, sizeof(ext_ifname));

					va_cmd("/bin/upnpctrl", 3, 1, "down", ext_ifname, BRIF);
				}
				else
				{
					sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_UPNP_EXT_ITF);
					goto setErr_tcpip;
				}
			}
			else
			{
				sprintf(tmpBuf, "%s:%s", Tget_mib_error, MIB_UPNP_DAEMON);
				goto setErr_tcpip;
			}
		#endif
		// The end of UPnP Daemon Start

			if (!mib_set(MIB_ADSL_LAN_IP, (void *)&inIp)) {
				strcpy(tmpBuf, strSetIPerror);
				goto setErr_tcpip;
			}
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
			add_dsldevice_on_hosts();
#endif
		#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			va_cmd("/bin/upnpctrl", 3, 1, "up", ext_ifname, BRIF);
		#endif
		}
	}
	else { // get current used IP
//		if ( !getInAddr(BRIDGE_IF, IP_ADDR, (void *)&inIp) ) {
//			strcpy(tmpBuf, "Get IP-address error!");
//			goto setErr_tcpip;
//		}
	}

//	strMask = boaGetVar(wp, "mask", "");
	if ( strMask[0] ) {
		if (!isValidNetmask(strMask, 1)) {
			strcpy(tmpBuf, strWrongMask);
			goto setErr_tcpip;
		}
		if ( !inet_aton(strMask, &inMask) ) {
			strcpy(tmpBuf, strWrongMask);
			goto setErr_tcpip;
		}
		mib_get(MIB_ADSL_LAN_SUBNET, (void *)&origMask);
		if(origMask.s_addr!= inMask.s_addr)
			ip_mask_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_SUBNET, (void *)&inMask)) {
			strcpy(tmpBuf, strSetMaskerror);
			goto setErr_tcpip;
		}
	}
	else { // get current used netmask
//		if ( !getInAddr(BRIDGE_IF, SUBNET_MASK, (void *)&inMask )) {
//			strcpy(tmpBuf, "Get subnet-mask error!");
//			goto setErr_tcpip;
//		}
	}

#ifdef CONFIG_RTK_RG_INIT
	strRGTmp = boaGetVar(wp, "lan_vlan_id1", "");
	if(strRGTmp[0])
	{
		vInt = atoi(strRGTmp);

		mib_get(MIB_LAN_VLAN_ID1, (void *)&cur_vInt);

		if(vInt != cur_vInt)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_VLAN_ID1, (void *)&vInt))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_VLAN_ID1);
				goto setErr_tcpip;
			}
		}
	}

#ifdef CONFIG_SECONDARY_IP
	strRGTmp= boaGetVar(wp, "lan_port_mask1", "");
	if(strRGTmp[0])
	{
		vInt = atoi(strRGTmp);

		vInt = (vInt & 1? 1 << RTK_RG_MAC_PORT0: 0) |
		(vInt & (1 << 1)? 1 << RTK_RG_MAC_PORT1: 0) |
		(vInt & (1 << 2)? 1 << RTK_RG_MAC_PORT2: 0) |
		(vInt & (1 << 3)? 1 << RTK_RG_MAC_PORT3: 0);

		mib_get(MIB_LAN_PORT_MASK1, (void *)&cur_vInt);

		if(vInt != cur_vInt)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_PORT_MASK1, (void *)&vInt))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_PORT_MASK1);
				goto setErr_tcpip;
			}
		}
	}
#endif

	strRGTmp= boaGetVar(wp, "ip_version1", "");
	if(strRGTmp[0])
	{
		vChar = atoi(strRGTmp);

		mib_get(MIB_LAN_IP_VERSION1, (void *)&cur_vChar);

		if(vChar != cur_vChar)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_IP_VERSION1, (void *)&vChar))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IP_VERSION1);
				goto setErr_tcpip;
			}
		}
	}

#ifdef CONFIG_IPV6
	strRGTmp = boaGetVar(wp, "ipv6_mode1", "");
	if(strRGTmp[0])
	{
		ipv6_mode = atoi(strRGTmp);
		mib_get(MIB_LAN_IPV6_MODE1, (void *)&cur_vChar);
		if(ipv6_mode != cur_vChar)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_IPV6_MODE1, (void *)&ipv6_mode))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IPV6_MODE1);
				goto setErr_tcpip;
			}
		}
	}

	if(ipv6_mode == 1) // 1: manual
	{
		strRGTmp = boaGetVar(wp, "ipv6_addr1", "");
		if(strRGTmp[0])
		{
			int i;
			inet_pton(AF_INET6, strRGTmp, (void *)ip6_addr);
			mib_get(MIB_LAN_IPV6_ADDR1, (void *)cur_ip6_addr);

			if(memcmp(ip6_addr, cur_ip6_addr, IP6_ADDR_LEN))
			{
				ip_mask_changed_flag = 1;
				if(!mib_set(MIB_LAN_IPV6_ADDR1, (void *)ip6_addr))
				{
					sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IPV6_ADDR1);
					goto setErr_tcpip;
				}
			}
		}

		strRGTmp = boaGetVar(wp, "ipv6_prefix1", "");
		if(strRGTmp[0])
		{
			vChar = atoi(strRGTmp);
			mib_get(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&cur_vChar);
			if(vChar != cur_vChar)
			{
				ip_mask_changed_flag = 1;
				if(!mib_set(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&vChar))
				{
					sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IPV6_PREFIX_LEN1);
					goto setErr_tcpip;
				}
			}
		}
	}
#endif
#endif

#ifdef WLAN_SUPPORT
	// set eth to wireless blocking
	strBlock = boaGetVar(wp, "BlockEth2Wir", "");
	if (strBlock[0])
	{
		unsigned char orWblock;
		if (strBlock[0] == '0')
			vChar = 0;
		else // '1'
			vChar = 1;

		mib_get(MIB_WLAN_BLOCK_ETH2WIR, (void *)&orWblock);
		if( orWblock != vChar )
			wireless_block_flag = 1;

		if ( mib_set(MIB_WLAN_BLOCK_ETH2WIR, (void *)&vChar) == 0)
		{
			strcpy(tmpBuf, strSetLanWlanBlokErr);
			goto setErr_tcpip;
		}
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx = 1;
		if ( mib_set(MIB_WLAN_BLOCK_ETH2WIR, (void *)&vChar) == 0)
		{
			strcpy(tmpBuf, strSetLanWlanBlokErr);
			goto setErr_tcpip;
		}
		wlan_idx = 0;
		#endif

		setup_wlan_block();

		TRACE(STA_SCRIPT,"/bin/echo 2 > /proc/fastbridge\n");
		system("/bin/echo 2 > /proc/fastbridge");
	}
#endif

#ifdef CONFIG_SECONDARY_IP
	char ip2mode;
	pStr = boaGetVar(wp, "enable_ip2", "");
	if (pStr[0]) {
		if (pStr[0] == '1') {
			mode = 1;
			strIp = boaGetVar(wp, "ip2", "");
			if ( strIp[0] ) {
				if ( !inet_aton(strIp, &inIp) ) {
					strcpy(tmpBuf, strWrongIP);
					goto setErr_tcpip;
				}
				mib_get(MIB_ADSL_LAN_IP2, (void *)&origIp);
				if(origIp.s_addr != inIp.s_addr)
				{
					ip2_changed_flag = 1;
					ip_mask_changed_flag = 1;
				}
				if ( !mib_set( MIB_ADSL_LAN_IP2, (void *)&inIp)) {
					strcpy(tmpBuf, strSetIPerror);
					goto setErr_tcpip;
				}
			}
			strMask = boaGetVar(wp, "mask2", "");
			if ( strMask[0] ) {
				if ( !inet_aton(strMask, &inMask) ) {
					strcpy(tmpBuf, strWrongMask);
					goto setErr_tcpip;
				}
				mib_get(MIB_ADSL_LAN_SUBNET2, (void *)&origMask);
				if(origMask.s_addr!= inMask.s_addr)
				{
					ip2_changed_flag = 1;
					ip_mask_changed_flag = 1;
				}
				if ( !mib_set(MIB_ADSL_LAN_SUBNET2, (void *)&inMask)) {
					strcpy(tmpBuf, strSetMaskerror);
					goto setErr_tcpip;
				}
			}
#ifndef DHCPS_POOL_COMPLETE_IP
			// DHCP Server pool is on ...
			pStr = boaGetVar(wp, "dhcpuse", "");
			if ( pStr[0] == '0' ) { // Primary LAN
				dhcp_pool = 0;
			}
			else { // Secondary LAN
				dhcp_pool = 1;
			}
			mib_set(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&dhcp_pool);
#endif
		}
		else {
			mode = 0;
		}
	}
	else {
		mode = 0;
	}
	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&ip2mode);
	if(ip2mode != mode)
	{
		ip2_changed_flag = 1;
		ip_mask_changed_flag = 1;
	}
	mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&mode);

#ifdef CONFIG_RTK_RG_INIT
	strRGTmp= boaGetVar(wp, "lan_vlan_id2", "");
	if(strRGTmp[0])
	{
		vInt= atoi(strRGTmp);

		mib_get(MIB_LAN_VLAN_ID2, (void *)&cur_vInt);

		if(vInt != cur_vInt)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_VLAN_ID2, (void *)&vInt))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_VLAN_ID2);
				goto setErr_tcpip;
			}
		}
	}

	strRGTmp = boaGetVar(wp, "lan_port_mask2", "");
	if(strRGTmp[0])
	{
		vInt = atoi(strRGTmp);

		vInt = (vInt & 1? 1 << RTK_RG_MAC_PORT0: 0) |
		(vInt & (1 << 1)? 1 << RTK_RG_MAC_PORT1: 0) |
		(vInt & (1 << 2)? 1 << RTK_RG_MAC_PORT2: 0) |
		(vInt & (1 << 3)? 1 << RTK_RG_MAC_PORT3: 0);

		mib_get(MIB_LAN_PORT_MASK2, (void *)&cur_vInt);

		if(vInt != cur_vInt)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_PORT_MASK2, (void *)&vInt))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_PORT_MASK2);
				goto setErr_tcpip;
			}
		}
	}

	strRGTmp= boaGetVar(wp, "ip_version2", "");
	if(strRGTmp[0])
	{
		vChar = atoi(strRGTmp);

		mib_get(MIB_LAN_IP_VERSION2, (void *)&cur_vChar);

		if(vChar != cur_vChar)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_IP_VERSION2, (void *)&vChar))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IP_VERSION2);
				goto setErr_tcpip;
			}
		}
	}

#ifdef CONFIG_IPV6
	strRGTmp = boaGetVar(wp, "ipv6_mode2", "");
	if(strRGTmp[0])
	{
		ipv6_mode = atoi(strRGTmp);
		mib_get(MIB_LAN_IPV6_MODE2, (void *)&cur_vChar);
		if(ipv6_mode != cur_vChar)
		{
			ip_mask_changed_flag = 1;
			if(!mib_set(MIB_LAN_IPV6_MODE2, (void *)&ipv6_mode))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IPV6_MODE2);
				goto setErr_tcpip;
			}
		}
	}

	if(ipv6_mode == 1) // 1: manual
	{
		strRGTmp = boaGetVar(wp, "ipv6_addr2", "");
		if(strRGTmp[0])
		{
			int i;
			inet_pton(AF_INET6, strRGTmp, (void *)ip6_addr);
			mib_get(MIB_LAN_IPV6_ADDR2, (void *)cur_ip6_addr);

			if(memcmp(ip6_addr, cur_ip6_addr, IP6_ADDR_LEN))
			{
				ip_mask_changed_flag = 1;
				if(!mib_set(MIB_LAN_IPV6_ADDR2, (void *)ip6_addr))
				{
					sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IPV6_ADDR2);
					goto setErr_tcpip;
				}
			}
		}

		strRGTmp = boaGetVar(wp, "ipv6_prefix2", "");
		if(strRGTmp[0])
		{
			vChar = atoi(strRGTmp);
			mib_get(MIB_LAN_IPV6_PREFIX_LEN2, (void *)&cur_vChar);
			if(vChar != cur_vChar)
			{
				ip_mask_changed_flag = 1;
				if(!mib_set(MIB_LAN_IPV6_PREFIX_LEN2, (void *)&vChar))
				{
					sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_LAN_IPV6_PREFIX_LEN2);
					goto setErr_tcpip;
				}
			}
		}
	}
#endif  // CONFIG_IPV6
#endif  // CONFIG_RG_INIT
#endif	// of CONFIG_SECONDARY_IP

#if defined(CONFIG_RTL_IGMP_SNOOPING)
	char origmode = 0;
	strSnoop = boaGetVar(wp, "snoop", "");
	if ( strSnoop[0] ) {
		// bitmap for virtual lan port function
		// Port Mapping: bit-0
		// QoS : bit-1
		// IGMP snooping: bit-2
		mib_get(MIB_MPMODE, (void *)&mode);
		origmode = mode;
		strSnoop = boaGetVar(wp, "snoop", "");
		if ( strSnoop[0] == '1' ) {
			mode |= MP_IGMP_MASK;
			if(origmode != mode)
				igmp_changed_flag = 1;
			__dev_setupIGMPSnoop(1);
		}
		else {
			mode &= ~MP_IGMP_MASK;
			if(origmode != mode)
				igmp_changed_flag = 1;
			__dev_setupIGMPSnoop(0);
		}
		mib_set(MIB_MPMODE, (void *)&mode);
	}

#ifdef CONFIG_IGMP_FORBID
 str_igmpforbid = boaGetVar(wp, "igmpforbid", "");
 if( str_igmpforbid[0] )
 	{
 	   if(str_igmpforbid[0]=='0')
 	   	{
		  vChar =0;
		  __dev_igmp_forbid(0);
 	   	}
	   else if (str_igmpforbid[0]=='1')
	   	{
	   	  vChar =1;
		   __dev_igmp_forbid(1);
	   	}
	   mib_set(MIB_IGMP_FORBID_ENABLE, (void *)&vChar);
 	}
#endif
#endif

#ifdef CONFIG_RTK_RG_INIT
	strRGTmp = boaGetVar(wp, "mac_based_tag_decision", "");

	if(strRGTmp[0])
	{
		vChar = atoi(strRGTmp);

		mib_get(MIB_MAC_BASED_TAG_DECISION, (void *)&cur_vChar);

		if(vChar != cur_vChar)
		{
			mbtd_changed = 1;
			if(mib_set(MIB_MAC_BASED_TAG_DECISION, (void *)&vChar) == 0)
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_MAC_BASED_TAG_DECISION);
				goto setErr_tcpip;
			}
		}
	}
#endif

#ifdef CONFIG_RTK_RG_INIT
	if(igmp_changed_flag || mbtd_changed)
		Init_rg_api();

	if( ip_mask_changed_flag || ip2_changed_flag || mbtd_changed || igmp_changed_flag )
	{
		if(Init_RG_ELan(UntagCPort, RoutingWan)!=SUCCESS)
		{
			printf("Init_RG_ELan failed!!! \n");
			return;
		}
	}

	if(igmp_changed_flag || mbtd_changed)
		restartWAN(CONFIGALL, NULL);
#endif

	if(ip_mask_changed_flag == 1)
	{
		//ql: when ip mask changed, then dhcpd should be changed simultaneously
		//applyLanChange2Dhcpd();
		if(ip2_changed_flag == 1)
		{
			submitUrl = boaGetVar(wp, "submit-url", "");
			OK_MSG(submitUrl);
		}
		else
		{
			submitUrl = boaGetVar(wp, "submit-url", "");
			OK_MSG(submitUrl);
		}

		restart_lanip();
	}
	else if(igmp_changed_flag == 1)
	{
		submitUrl = boaGetVar(wp, "submit-url", "");
		OK_MSG(submitUrl);
	}
	else
	{
		submitUrl = boaGetVar(wp, "submit-url", "");
		if (submitUrl[0])
			boaRedirect(wp, submitUrl);
		else
			boaDone(wp, 200);
	}

#if defined(CONFIG_RTK_RG_INIT) && defined (CONFIG_IPV6)
	set_LAN_IPv6_IP();
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	if( ip_mask_changed_flag || ip2_changed_flag || igmp_changed_flag
#ifdef CONFIG_RTK_RG_INIT
		|| mbtd_changed
#endif
	)
		Commit();
#ifdef WLAN_SUPPORT
	else if(wireless_block_flag)
		Commit();
#endif
#endif // of #if COMMIT_IMMEDIATELY
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

	return;
setErr_tcpip:
	ERR_MSG(tmpBuf);
}

int lan_setting(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	int i;
#ifdef CONFIG_SECONDARY_IP
	char buf[32];

	nBytesSent += boaWrite(wp, "<tr>\n<td><font size=2>\n");
	nBytesSent += boaWrite(wp, "<input type=checkbox name=enable_ip2 value=1 ");
	nBytesSent += boaWrite(wp, "onClick=updateInput()>&nbsp;&nbsp;");
	nBytesSent += boaWrite(wp, "<b>%s</b>\n</td></tr>\n", multilang(LANG_SECONDARY_IP));

	getMIB2Str(MIB_ADSL_LAN_IP2, buf);
	nBytesSent += boaWrite(wp, "<div ID=\"secondIP\" style=\"display:none\">\n");
	nBytesSent += boaWrite(wp, "<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n");
	nBytesSent += boaWrite(wp, "<tr><td width=150><font size=2><b>%s:</b></td>\n", multilang(LANG_IP_ADDRESS));
	nBytesSent += boaWrite(wp, "<td width=350><input type=text name=ip2 size=15 maxlength=15 ");
	nBytesSent += boaWrite(wp, "value=%s></td>\n</tr>\n", buf);

	getMIB2Str(MIB_ADSL_LAN_SUBNET2, buf);
	nBytesSent += boaWrite(wp, "<tr><td><font size=2><b>%s:</b></td>\n", multilang(LANG_SUBNET_MASK));
	nBytesSent += boaWrite(wp, "<td><input type=text name=mask2 size=15 maxlength=15 ");
	nBytesSent += boaWrite(wp, "value=%s></td>\n</tr>\n", buf);

#ifdef CONFIG_RTK_RG_INIT
	nBytesSent += boaWrite(wp, "	<tr style=\"display: none\">");
	nBytesSent += boaWrite(wp, "		<td><font size=2><b>%s:</b></td>", multilang(LANG_IPV6_ADDRESS_MODE));
	nBytesSent += boaWrite(wp, "		<td><font size=2>");
	nBytesSent += boaWrite(wp, "			<input type=radio name=ipv6_mode2 value=0 OnChange=ipv6_mode2_change()>Auto");
	nBytesSent += boaWrite(wp, "			<input type=radio name=ipv6_mode2 value=1 OnChange=ipv6_mode2_change()>Manual");
	nBytesSent += boaWrite(wp, "		</td>");
	nBytesSent += boaWrite(wp, "	</tr>");
	nBytesSent += boaWrite(wp, "  <tr style=\"display: none\">");
	nBytesSent += boaWrite(wp, "		<td><font size=2><b>%s:</b></td>", multilang(LANG_IPV6_ADDRESS));
	nBytesSent += boaWrite(wp, "		<td><input type=text name=ipv6_addr2 size=30 maxlength=60 value=0></td>");
	nBytesSent += boaWrite(wp, "  </tr>");
	nBytesSent += boaWrite(wp, "  <tr style=\"display: none\">");
	nBytesSent += boaWrite(wp, "		<td><font size=2><b>%s:</b></td>", multilang(LANG_IPV6_PREFIX_LENGTH));
	nBytesSent += boaWrite(wp, "		<td><input type=text name=ipv6_prefix2 size=5 maxlength=5 value=0></td>");
	nBytesSent += boaWrite(wp, "  </tr>");

	getMIB2Str(MIB_LAN_VLAN_ID2, buf);
	nBytesSent += boaWrite(wp, "	<tr>");
	nBytesSent += boaWrite(wp, "		<td><font size=2><b>%s:</b></td>", multilang(LANG_VLAN_ID));
	nBytesSent += boaWrite(wp, "		<td><input type=text name=lan_vlan_id2 size=15 maxlength=15 value=%s></td>", buf);
	nBytesSent += boaWrite(wp, "	</tr>");

	nBytesSent += boaWrite(wp, "<tr>");
	nBytesSent += boaWrite(wp, "	<td><font size=2><b>%s:</b></td>", multilang(LANG_IP_VERSION));
	nBytesSent += boaWrite(wp, "	<td>");
	nBytesSent += boaWrite(wp, "		<select size=1 name=ip_version2>");
	nBytesSent += boaWrite(wp, "			<option selected value=0>IPv4</option>");
	nBytesSent += boaWrite(wp, "			<option value=1>IPv6</option>");
	nBytesSent += boaWrite(wp, "			<option value=2>Both</option>");
	nBytesSent += boaWrite(wp, "		</select>");
	nBytesSent += boaWrite(wp, "	</td>");

	nBytesSent += boaWrite(wp, "<tr>");
	nBytesSent += boaWrite(wp, "	<td><font size=2><b>%s:</b></td>", multilang(LANG_PORT_MASK));
	nBytesSent += boaWrite(wp, "	<td>");
	
	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3 && i<SW_LAN_PORT_NUM; i++)
		nBytesSent += boaWrite(wp, "<font size=2><input type=checkbox name=chk_port_mask2>Port %d", i);

	nBytesSent += boaWrite(wp, "	</td>");
	nBytesSent += boaWrite(wp, "</tr>");
#else
	nBytesSent += boaWrite(wp, "<input type=hidden name=chk_port_mask2>");
#endif

#ifndef DHCPS_POOL_COMPLETE_IP
	nBytesSent += boaWrite(wp, "<tr></tr><tr><td><font size=2><b>DHCP %s:</b></td>\n", multilang(LANG_POOL));
	nBytesSent += boaWrite(wp, "<td><font size=2>\n<input type=\"radio\"");
	nBytesSent += boaWrite(wp, " name=dhcpuse value=0>Primary LAN&nbsp;&nbsp;\n");
	nBytesSent += boaWrite(wp, "<input type=\"radio\" name=dhcpuse value=1>Secondary LAN</td>\n</tr>");
#endif
	nBytesSent += boaWrite(wp, "</table></div>\n");
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING) && !defined(CONFIG_SFU)
	nBytesSent += boaWrite(wp, "<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n");
	nBytesSent += boaWrite(wp, "<tr><td width=150><font size=2><b>%s:</b></td>\n", multilang(LANG_IGMP_SNOOPING));
	nBytesSent += boaWrite(wp, "<td width=350><font size=2>\n<input type=\"radio\"");
	nBytesSent += boaWrite(wp, " name=snoop value=0>%s&nbsp;&nbsp;\n", multilang(LANG_DISABLED));
	nBytesSent += boaWrite(wp, "<input type=\"radio\" name=snoop value=1>%s</td>\n</tr></table>\n", multilang(LANG_ENABLED));
#endif

#ifdef WLAN_SUPPORT
	nBytesSent += boaWrite(wp, "<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n");
  	nBytesSent += boaWrite(wp, "<tr><td width=150><font size=2><b>%s:</b></td>\n", multilang(LANG_ETHERNET_TO_WIRELESS_BLOCKING));
  	nBytesSent += boaWrite(wp, "<td width=350><font size=2>\n");
  	nBytesSent += boaWrite(wp, "<input type=\"radio\" name=BlockEth2Wir value=0>%s&nbsp;&nbsp;\n", multilang(LANG_DISABLED));
  	nBytesSent += boaWrite(wp, "<input type=\"radio\" name=BlockEth2Wir value=1>%s</td></tr></table>\n", multilang(LANG_ENABLED));
#endif
#ifdef CONFIG_RTK_RG_INIT
	nBytesSent += boaWrite(wp, "<table border=0 width=\"500\" cellspacing=4 cellpadding=0>\n");
	nBytesSent += boaWrite(wp, "<tr><td width=150><font size=2><b>%s:</b></td>\n", multilang(LANG_MAC_BASED_TAG_DECISION));
	nBytesSent += boaWrite(wp, "<td width=350><font size=2>\n");
	nBytesSent += boaWrite(wp, "<input type=\"radio\" name=mac_based_tag_decision value=0>%s&nbsp;&nbsp;\n", multilang(LANG_DISABLED));
	nBytesSent += boaWrite(wp, "<input type=\"radio\" name=mac_based_tag_decision value=1>%s</td></tr></table>\n", multilang(LANG_ENABLED));
#endif
	return nBytesSent;
}

int checkIP2(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

#ifdef CONFIG_SECONDARY_IP
	nBytesSent += boaWrite(wp, "if (document.tcpip.enable_ip2.checked) {\n");
	nBytesSent += boaWrite(wp, "\tif (!checkIP(document.tcpip.ip2))\n");
	nBytesSent += boaWrite(wp, "\t\treturn false;\n");
	nBytesSent += boaWrite(wp, "\tif (!checkMask(document.tcpip.mask2))\n");
	nBytesSent += boaWrite(wp, "\t\treturn false;\n");
	nBytesSent += boaWrite(wp, "\t\tif (checkLan1andLan2(document.tcpip.ip, document.tcpip.mask, document.tcpip.ip2, document.tcpip.mask2) == false) {\n");
	nBytesSent += boaWrite(wp, "\t\talert(\"Network Address Conflict !\");\n");
	nBytesSent += boaWrite(wp, "\t\tdocument.tcpip.ip2.value=document.tcpip.ip2.defaultValue;\n");
	nBytesSent += boaWrite(wp, "\t\tdocument.tcpip.ip2.focus();\n");
	nBytesSent += boaWrite(wp, "\t\treturn false;}}\n");
#endif
	return nBytesSent;
}

int lan_script(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

#ifdef CONFIG_SECONDARY_IP
	nBytesSent += boaWrite(wp, "function updateInput()\n");
	nBytesSent += boaWrite(wp, "{\n\tif (document.tcpip.enable_ip2.checked == true) {\n");
	nBytesSent += boaWrite(wp, "\t\tif (document.getElementById)  // DOM3 = IE5, NS6\n");
	nBytesSent += boaWrite(wp, "\t\t\tdocument.getElementById('secondIP').style.display = 'block';\n");
	nBytesSent += boaWrite(wp, "\t\t\telse {\n");
	nBytesSent += boaWrite(wp, "\t\t\tif (document.layers == false) // IE4\n");
	nBytesSent += boaWrite(wp, "\t\t\t\tdocument.all.secondIP.style.display = 'block';\n");
	nBytesSent += boaWrite(wp, "\t\t}\n");
	nBytesSent += boaWrite(wp, "\t} else {\n");
	nBytesSent += boaWrite(wp, "\t\tif (document.getElementById)  // DOM3 = IE5, NS6\n");
	nBytesSent += boaWrite(wp, "\t\t\tdocument.getElementById('secondIP').style.display = 'none';\n");
	nBytesSent += boaWrite(wp, "\t\telse {\n");
	nBytesSent += boaWrite(wp, "\t\t\tif (document.layers == false) // IE4\n");
	nBytesSent += boaWrite(wp, "\t\t\t\tdocument.all.secondIP.style.display = 'none';\n");
	nBytesSent += boaWrite(wp, "\t\t}\n");
	nBytesSent += boaWrite(wp, "\t}\n");
	nBytesSent += boaWrite(wp, "}");
#endif
	return nBytesSent;
}

int lan_port_mask(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
#ifdef CONFIG_SECONDARY_IP
	int i;
	nBytesSent += boaWrite(wp, "<td width=\"30%%\"><font size=2><b>%s:</b></td>", multilang(LANG_PORT_MASK));
	nBytesSent += boaWrite(wp, "<td width=\"70%%\">");

	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3 && i<SW_LAN_PORT_NUM; i++)
			nBytesSent += boaWrite(wp, "<font size=2><input type=checkbox name=chk_port_mask1>Port %d", i);
	nBytesSent += boaWrite(wp, "</td>");
#endif
	return nBytesSent;
}


