/*-----------------------------------------------------------------
 * File: boamain.c
 *-----------------------------------------------------------------*/

#include <string.h>
#include "webform.h"
#include "fmdefs.h"
#include "mib.h"
#include "../defs.h"
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
//xl_yue
#include "utility.h"
#include "wireless.h"

/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
#include "web_voip.h"
#endif /*VOIP_SUPPORT*/
/*-----end-----*/
#include "multilang.h"
#ifdef CONFIG_RTK_RG_INIT
#include <rtk_rg_struct.h>
#endif

//#ifdef _USE_RSDK_WRAPPER_
void initSyslogPage(request * wp);
void initDgwPage(request * wp);
//#endif //_USE_RSDK_WRAPPER_

void rtl8670_AspInit() {
   /*
 *	ASP script procedures and form functions.
 */
	// fm function for E8
	boaASPDefine("getInfo", getInfo);
	boaASPDefine("getDefaultGWMask", getDefaultGWMask);
	boaASPDefine("getDefaultGW", getDefaultGW);
#ifdef CONFIG_IPV6
	boaASPDefine("getDefaultGW_ipv6", getDefaultGW_ipv6);
#endif

	boaASPDefine("listWanConfig", listWanConfig);
#ifdef CONFIG_IPV6
	boaASPDefine("listWanConfigIpv6", listWanConfigIpv6);
#endif
#ifdef SUPPORT_WAN_BANDWIDTH_INFO
	boaASPDefine("listWanBandwidth", listWanBandwidth);
#endif
	// Kaohj
	boaASPDefine("checkWrite", checkWrite);
	boaASPDefine("atmVcList2", atmVcList2);
	boaASPDefine("initPage", initPage);
#ifdef CONFIG_RTL8672_SAR
	//boaFormDefine(formWanAtm), formAtmVc);			// Atm VC Configuration Setting Form
	boaFormDefine("formAdsl", formAdsl);			// ADSL Configuration Setting Form
	boaASPDefine("initPageAdsl", initPageAdsl);
	boaASPDefine("initPageAdsl2", initPageAdsl2);
#endif
	boaASPDefine("createMenuEx", createMenuEx);
#ifdef WLAN_SUPPORT
	boaFormDefine("formWlanSetup", formWlanSetup);
	boaASPDefine("wlStatsList", wlStatsList);
	boaASPDefine("wlStatus_parm", wlStatus_parm);
	boaASPDefine("wirelessClientList", wirelessClientList);
#ifdef WLAN_WPA
	boaFormDefine("formWlEncrypt", formWlEncrypt);
#endif
#ifdef WIFI_TIMER_SCHEDULE
	boaFormDefine("formWifiTimerEx", formWifiTimerEx);
	boaFormDefine("formWifiTimer", formWifiTimer);
#endif
#endif
	boaASPDefine("E8BPktStatsList", E8BPktStatsList);
	boaASPDefine("E8BDhcpClientList", E8BDhcpClientList);
#ifdef CONFIG_USER_RTK_SYSLOG
	boaFormDefine("formSysLog", formSysLog);
	boaASPDefine("sysLogList", sysLogList);
	boaFormDefine("formSysLogConfig", formSysLogConfig);
#endif

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	boaASPDefine("ponGetStatus", ponGetStatus);
#ifdef CONFIG_GPON_FEATURE
	boaASPDefine("showgpon_status", showgpon_status);
#endif

#ifdef CONFIG_EPON_FEATURE
	boaASPDefine("showepon_status", showepon_status);
#endif
#endif
#ifdef CONFIG_MCAST_VLAN
	boaASPDefine("listWanName", listWanName);
	boaFormDefine("formMcastVlanMapping", formMcastVlanMapping);
#endif

#ifdef USE_LOGINWEB_OF_SERVER
	boaFormDefine("formLogin", formLogin);					// xl_yue added,
	boaFormDefine("formLogout", formLogout);
	// Kaohj
	boaASPDefine("passwd2xmit", passwd2xmit);
#endif
	boaFormDefine("formUSBbak", formUSBbak);
	boaFormDefine("formUSBUmount", formUSBUmount);
	boaFormDefine("formReboot", formReboot);				// Commit/reboot Form
#ifdef _CWMP_MIB_
	boaFormDefine("formTR069Config", formTR069Config);
	boaFormDefine("formTR069CPECert", formTR069CPECert);
	boaFormDefine("formTR069CACert", formTR069CACert);
	boaFormDefine("formTR069CACertDel", formTR069CACertDel);
	boaFormDefine("formMidwareConfig", formMidwareConfig);

	boaASPDefine("TR069ConPageShow", TR069ConPageShow);
#endif
	boaFormDefine("formFinishMaintenance", formFinishMaintenance);
#ifdef E8B_NEW_DIAGNOSE
	boaASPDefine("dumpPingInfo", dumpPingInfo);
	boaFormDefine("formPing", formPing);		// Ping diagnostic Form
	boaASPDefine("createMenuDiag", createMenuDiag);
	boaFormDefine("formTr069Diagnose", formTr069Diagnose);
	boaASPDefine("dumpTraceInfo", dumpTraceInfo);
	boaFormDefine("formTracert", formTracert);

#ifdef CONFIG_DEV_xDSL
	boaFormDefine("formOAMLB", formOamLb);		// OAM Loopback diagnostic Form
#endif
#endif
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
	boaFormDefine("formEthernet", formEthernet);			// Ethernet Configuration Setting Form
	boaASPDefine("initPageEth", initPageEth);
	boaASPDefine("initPageEth2", initPageEth2);
	boaASPDefine("initVlanRange", initVlanRange);	
#endif
	boaASPDefine("initPageQoSAPP", initPageQoSAPP);
	boaFormDefine("formWanRedirect", formWanRedirect);
	boaASPDefine("if_wan_list", ifwanList);
#ifdef CONFIG_USER_IGMPPROXY
	boaFormDefine("formIgmproxy", formIgmproxy);	// IGMP Configuration Form
	boaASPDefine("igmproxyList", igmproxyList);		// Mason Yu. IGMP Proxy for e8b
#endif
#ifdef CONFIG_IPV6
	boaFormDefine("formlanipv6dns", formlanipv6dns);			    // set LAN(br0) IPv6 address
	boaFormDefine("formlanipv6", formlanipv6);			    // set LAN(br0) IPv6 address
	boaFormDefine("formlanipv6prefix", formlanipv6prefix);	// set LAN IPv6 prefix
#ifdef CONFIG_USER_ECMH
	boaFormDefine("formMLDProxy", formMLDProxy);			// MLD Proxy Configuration Form
#endif
	boaFormDefine("formMLDSnooping", formMLDSnooping);		// formIgmpSnooping Configuration Form  // Mason Yu. MLD snooping for e8b

#ifdef CONFIG_USER_RADVD
	boaFormDefine("formRadvdSetup", formRadvdSetup);			// RADVD Configuration Setting Form
#endif
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	boaFormDefine("formDhcpv6Server", formDhcpv6);			        // DHCPv6 Server Setting Form
#if 0 //iulian marked
	boaASPDefine("showDhcpv6SNameServerTable", showDhcpv6SNameServerTable);     // Name Server List for DHCPv6 Server
	boaASPDefine("showDhcpv6SDOMAINTable", showDhcpv6SDOMAINTable);             // Domian search List for DHCPv6 Server
#endif
#endif
#endif
	boaFormDefine("formRouting", formRoute);			// Routing Configuration Form
	boaASPDefine("showStaticRoute", showStaticRoute);
	boaASPDefine("ShowDefaultGateway", ShowDefaultGateway);	// Jenny, for DEFAULT_GATEWAY_V2
	boaASPDefine("GetDefaultGateway", GetDefaultGateway);
	boaFormDefine("formRefleshRouteTbl", formRefleshRouteTbl);
	boaASPDefine("routeList", routeList);
#if defined(CONFIG_USER_ROUTED_ROUTED)
	boaFormDefine("formRip", formRip);			// RIP Configuration Form
	boaASPDefine("showRipIf", showRipIf);
#endif
	boaASPDefine("initPage", initPage);
	boaFormDefine("formIgmpSnooping", formIgmpSnooping);	// formIgmpSnooping Configuration Form  // Mason Yu. IGMP snooping for e8b

	boaFormDefine("formDhcpServer", formDhcpd);				// DHCP Server Setting Form
	boaASPDefine("dhcpClientList", dhcpClientList);
	boaFormDefine("formIpRange", formIpRange);
	boaASPDefine("init_dhcp_device_page", init_dhcp_device_page);

	boaFormDefine("formUpnp", formUpnp);

	// Magician E8B Security pages
	boaASPDefine("listWanif", listWanif);
#ifdef URL_BLOCKING_SUPPORT
	boaASPDefine("initPageURL", initPageURL);
	boaFormDefine("formURL", formURL);                  // URL Configuration Form
#endif
#ifdef CONFIG_LED_INDICATOR_TIMER 
	boaFormDefine("formLedTimer", formLedTimer);
#endif
	boaASPDefine("initPageFirewall", initPageFirewall);
	boaFormDefine("formFirewall", formFirewall);
	boaASPDefine("initPageDos", initPageDos);
	boaFormDefine("formDos", formDos);
	boaASPDefine("brgMacFilterList", brgMacFilterList);
	boaFormDefine("formBrgMacFilter", formBrgMacFilter);
	boaASPDefine("rteMacFilterList", rteMacFilterList);
	boaFormDefine("formRteMacFilter", formRteMacFilter);

#ifdef CONFIG_RG_SLEEPMODE_TIMER
	boaASPDefine("initPageSleepModeRule", initPageSleepModeRule);
	boaFormDefine("formSleepMode", formSleepMode);
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
	boaASPDefine("initPageLanBandwidthMonitor", initPageLanBandwidthMonitor);
	boaFormDefine("formBandwidthMonitor", formBandwidthMonitor);
#endif
#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
	boaASPDefine("initPageBandwidthControl", initPageBandwidthControl);
	boaFormDefine("formBandWidth", formBandWidth);
#endif
#ifdef CONFIG_USER_LANNETINFO
	boaASPDefine("initPageLanNetInfo", initPageLanNetInfo);
#endif
#if defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL) || defined (CONFIG_USER_LAN_BANDWIDTH_MONITOR)
	boaFormDefine("formBandwidthInterval", formBandwidthInterval);
#endif
	boaASPDefine("ipPortFilterBlacklist", ipPortFilterBlacklist);
	boaASPDefine("ipPortFilterWhitelist", ipPortFilterWhitelist);
	boaFormDefine("formPortFilter", formPortFilter);
	boaASPDefine("ipPortFilterConfig", ipPortFilterConfig);
	boaFormDefine("formPortFilterWhite", formPortFilterWhite);
	boaFormDefine("formPortFilterBlack", formPortFilterBlack);
	boaFormDefine("formMacAddrBase", formMacAddrBase);
	boaASPDefine("showMACBaseTable", showMACBaseTable);
#ifdef CONFIG_USER_DDNS
	boaFormDefine("formDDNS", formDDNS);
	boaASPDefine("showDNSTable", showDNSTable);
#endif
	//End Magician

	boaFormDefine("formDMZ", formDMZ);						// Firewall DMZ Setting Form

	boaASPDefine("initPageMgmUser", initPageMgmUser);

#ifdef VIRTUAL_SERVER_SUPPORT
	boaFormDefine("formVrtsrv", formVrtsrv);
	boaASPDefine("virtualSvrList", virtualSvrList);
	boaASPDefine("virtualSvrLeft", virtualSvrLeft);
#endif
#ifdef CONFIG_IP_NF_ALG_ONOFF
	boaFormDefine("formALGOnOff", formALGOnOff);
#endif

#ifdef _PRMT_X_CT_COM_USERINFO_
	boaFormDefine("formAccountReg", formAccountReg);
	boaFormDefine("formUserReg", formUserReg);
	boaFormDefine("formUserReg_inside_menu", formUserReg_inside_menu);
	boaASPDefine("UserRegMsg", UserRegMsg);
	boaASPDefine("initE8clientUserRegPage", initE8clientUserRegPage);
	boaASPDefine("e8clientAccountRegResult", e8clientAccountRegResult);
	boaASPDefine("e8clientAutorunAccountRegResult", e8clientAutorunAccountRegResult);
	boaASPDefine("UserAccountRegResult", UserAccountRegResult);
#endif
	boaFormDefine("formPasswordSetup", formPasswordSetup);

#ifdef TIME_ZONE
	// Mason Yu. 2630-e8b
	boaFormDefine("formTimezone", formTimezone);
	boaASPDefine("init_sntp_page", init_sntp_page);
	boaASPDefine("timeZoneList", timeZoneList);
#endif
#ifdef CONFIG_USER_MINIDLNA
	boaASPDefine("fmDMS_checkWrite", fmDMS_checkWrite);
	boaFormDefine("formDMSConf", formDMSConf);
#endif
	boaASPDefine("initPageStorage", initPageStorage);
	boaFormDefine("formStorage", formStorage);
	boaASPDefine("listUsbDevices", listUsbDevices);
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	boaFormDefine("formPPtP", formPPtP);
	boaASPDefine("pptpList", pptpList);
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
	boaFormDefine("formL2TP", formL2TP);
	boaASPDefine("l2tpList", l2tpList);
#endif //end of CONFIG_USER_L2TPD_L2TPD
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
	boaASPDefine("initPagePBind", initPagePBind);
	boaFormDefine("formVlanMapping", formVlanMapping);
#endif
	boaASPDefine("initdgwoption", initdgwoption);

//#ifdef NEW_IP_QOS_SUPPORT
	boaFormDefine("formQosPolicy", formQosPolicy);
	boaASPDefine("initQueuePolicy", initQueuePolicy);
	boaASPDefine("ifWanList", ifWanList_tc);
	boaFormDefine("formQosTraffictl",formQosTraffictl);
	boaFormDefine("formQosTraffictlEdit",formQosTraffictlEdit);
	boaASPDefine("initTraffictlPage",initTraffictlPage);
	boaFormDefine("formQosRule",formQosRule);
	boaFormDefine("formQosRuleEdit",formQosRuleEdit);
//	boaFormDefine("formQosAppRule",formQosAppRule);
//	boaFormDefine("formQosAppRuleEdit",formQosAppRuleEdit);
	boaASPDefine("initQosRulePage",initQosRulePage);
//	boaASPDefine("initQosAppRulePage",initQosAppRulePage);
	boaASPDefine("initRulePriority",initRulePriority);
	boaASPDefine("initOutif",initOutif);
//	boaASPDefine("if_lan_list", iflanList);
//#endif

	boaASPDefine("ShowPortMapping", ShowPortMapping);

#ifdef CONFIG_USER_RTK_LBD
	boaASPDefine("initLBDPage", initLBDPage);
	boaFormDefine("formLBD",formLBD);
#endif
#ifdef WEB_UPGRADE
    boaFormDefine("formUpload", formUpload); // Management Upload Firmware Setting Form
#endif
#ifdef SUPPORT_WEB_PUSHUP
	boaFormDefine("formUpgradePop", formUpgradePop);
#endif
	boaFormDefine("formSaveConfig",formSaveConfig);
	boaFormDefine("formVersionMod",formVersionMod);
	boaFormDefine("formExportOMCIlog", formExportOMCIlog);
	boaFormDefine("formImportOMCIShell", formImportOMCIShell);
#ifdef VOIP_SUPPORT
printf("web_voip_init()\n");
	web_voip_init();
#endif /*VOIP_SUPPORT*/
	boaASPDefine("multilang", multilang_asp);
}


#ifdef CONFIG_RTL8672_SAR
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
void register_pvc(int signum)
{
	FILE *fp;
	unsigned int vpi, vci, entryNum, i, idx;
	MIB_CE_ATM_VC_T Entry;

	printf("signal (%x) handler register_pvc got called\n", signum);
	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "r") ) {
		fscanf( fp, "%d,%d", &vpi, &vci);
		printf("register_pvc (%d, %d)\n", vpi, vci);
		fclose(fp);

		autoHunt_found = 1;  // Timer for auto search PVC
		setVC0Connection(vpi, vci);
		succ_AutoSearchPVC();
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed\n");
	}
}
#endif // of AUTO_PVC_SEARCH_AUTOHUNT
#endif // of CONFIG_RTL8672_SAR

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_RADVD
void initRadvdConfPage(request * wp)
{
	unsigned char vChar;

	mib_get( MIB_V6_PREFIX_ENABLE, (void *)&vChar);
	if( vChar == 1)
	{
		mib_get( MIB_V6_PREFIX_MODE, (void *)&vChar);
		boaWrite(wp, "%s.radvd.PrefixMode.value = %d;\n", DOCUMENT, vChar);
		boaWrite(wp, "updateInput();\n");
	}
}
#endif
#endif

void initLanPage(request * wp)
{
	unsigned char vChar;
#ifdef CONFIG_SECONDARY_IP
	char dhcp_pool;
#endif
#ifdef CONFIG_RTK_RG_INIT
	unsigned int port_mask;
	unsigned char prefix_len;
	struct ipv6_ifaddr ip6_addr[6];
	char tmpBuf[128];
#endif

#ifdef CONFIG_SECONDARY_IP
	mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
	if (vChar!=0) {
		//boaWrite(wp, "%s.tcpip.enable_ip2.value = 1;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.enable_ip2.checked = true;\n", DOCUMENT);
	}
	#ifndef DHCPS_POOL_COMPLETE_IP
	mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&dhcp_pool);
	boaWrite(wp, "%s.tcpip.dhcpuse[%d].checked = true;\n", DOCUMENT, dhcp_pool);
	#endif
	boaWrite(wp, "updateInput();\n");
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
	mib_get( MIB_MPMODE, (void *)&vChar);
	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	// IGMP snooping: bit-2
	boaWrite(wp, "%s.tcpip.snoop[%d].checked = true;\n", DOCUMENT, (vChar>>MP_IGMP_SHIFT)&0x01);
#ifdef CONFIG_IGMP_FORBID
	mib_get( MIB_IGMP_FORBID_ENABLE, (void *)&vChar);
	boaWrite(wp, "%s.tcpip.igmpforbid[%d].checked = true;\n", DOCUMENT, vChar);
#endif
#endif

#ifdef WLAN_SUPPORT
	mib_get( MIB_WLAN_BLOCK_ETH2WIR, (void *)&vChar);
	boaWrite(wp, "%s.tcpip.BlockEth2Wir[%d].checked = true;\n", DOCUMENT, vChar==0?0:1);
#endif

#ifdef CONFIG_RTK_RG_INIT
	mib_get(MIB_MAC_BASED_TAG_DECISION, (void *)&vChar);
	boaWrite(wp, "%s.tcpip.mac_based_tag_decision[%d].checked = true;\n", DOCUMENT, vChar==0?0:1);

	mib_get(MIB_LAN_PORT_MASK1, (void *)&port_mask);
	if(port_mask & (1<<RTK_RG_MAC_PORT0))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[0].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT1))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[1].checked = true;\n", DOCUMENT);
#ifndef CONFIG_RTL9602C_SERIES
	if(port_mask & (1<<RTK_RG_MAC_PORT2))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[2].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT3))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[3].checked = true;\n", DOCUMENT);
#endif
	mib_get(MIB_LAN_IP_VERSION1, (void *)&vChar);
	boaWrite(wp, "%s.tcpip.ip_version1.options[%d].selected = true;\n", DOCUMENT, vChar);

	if(vChar == 0) // IP version is IPv4
	{
		boaWrite(wp, "%s.tcpip.ipv6_mode1[0].disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_mode1[1].disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_addr1.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_prefix1.disabled = true;\n", DOCUMENT);
	}
#ifdef CONFIG_IPV6
	else if(vChar == 1) // IP version is IPv6
	{
		boaWrite(wp, "%s.tcpip.ip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.mask.disabled = true;\n", DOCUMENT);
	}

	mib_get(MIB_LAN_IPV6_MODE1, (void *)&vChar);
	boaWrite(wp, "%s.tcpip.ipv6_mode1[%d].checked = true;\n", DOCUMENT, vChar);

	if(vChar == 0)  // IPv6 mode is auto
	{
		boaWrite(wp, "%s.tcpip.ipv6_addr1.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_prefix1.disabled = true;\n", DOCUMENT);
		getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
		inet_ntop(PF_INET6, &ip6_addr[0].addr, tmpBuf, 128);
		boaWrite(wp, "%s.tcpip.ipv6_addr1.value = \"%s\";\n", DOCUMENT, tmpBuf);
		boaWrite(wp, "%s.tcpip.ipv6_prefix1.value = \"%d\";\n", DOCUMENT, ip6_addr[0].prefix_len);
	}
	else
	{
		mib_get(MIB_LAN_IPV6_ADDR1, (void *)&ip6_addr[0].addr);
		mib_get(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&vChar);
		inet_ntop(PF_INET6, &ip6_addr[0].addr, tmpBuf, 128);
		boaWrite(wp, "%s.tcpip.ipv6_addr1.value = \"%s\";\n", DOCUMENT, tmpBuf);
		boaWrite(wp, "%s.tcpip.ipv6_prefix1.value = \"%u\";\n", DOCUMENT, vChar);
	}
#else
	boaWrite(wp, "%s.getElementById(\"tr_ipv6_mode1\").style.display = \"none\";\n", DOCUMENT);
	boaWrite(wp, "%s.getElementById(\"tr_ipv6_addr1\").style.display = \"none\";\n", DOCUMENT);
	boaWrite(wp, "%s.getElementById(\"tr_ipv6_prefix1\").style.display = \"none\";\n", DOCUMENT);
	boaWrite(wp, "%s.tcpip.ip_version1.disabled = true;\n", DOCUMENT);
#endif

#ifdef CONFIG_SECONDARY_IP
	mib_get(MIB_LAN_PORT_MASK2, (void *)&port_mask);
	if(port_mask & (1<<RTK_RG_MAC_PORT0))
		boaWrite(wp, "%s.tcpip.chk_port_mask2[0].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT1))
		boaWrite(wp, "%s.tcpip.chk_port_mask2[1].checked = true;\n", DOCUMENT);
#ifndef CONFIG_RTL9602C_SERIES
	if(port_mask & (1<<RTK_RG_MAC_PORT2))
		boaWrite(wp, "%s.tcpip.chk_port_mask2[2].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT3))
		boaWrite(wp, "%s.tcpip.chk_port_mask2[3].checked = true;\n", DOCUMENT);
#endif
	mib_get(MIB_LAN_IP_VERSION2, (void *)&vChar);
	boaWrite(wp, "%s.tcpip.ip_version2.options[%d].selected = true;\n", DOCUMENT, vChar);

	if(vChar == 0) // IP version is IPv4
	{
		boaWrite(wp, "%s.tcpip.ipv6_mode2[0].disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_mode2[1].disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_addr2.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_prefix2.disabled = true;\n", DOCUMENT);
	}
#ifdef CONFIG_IPV6
	else if(vChar == 1) // IP version is IPv6
	{
		boaWrite(wp, "%s.tcpip.ip2.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.mask2.disabled = true;\n", DOCUMENT);
	}

	mib_get(MIB_LAN_IPV6_MODE2, (void *)&vChar);
	boaWrite(wp, "%s.tcpip.ipv6_mode2[%d].checked = true;\n", DOCUMENT, vChar);

	if(vChar == 0) // IPv6 mode is auto
	{
		boaWrite(wp, "%s.tcpip.ipv6_addr2.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.tcpip.ipv6_prefix2.disabled = true;\n", DOCUMENT);
		getifip6(LAN_ALIAS, IPV6_ADDR_UNICAST, ip6_addr, 6);
		inet_ntop(PF_INET6, &ip6_addr[0].addr, tmpBuf, 128);
		boaWrite(wp, "%s.tcpip.ipv6_addr2.value = \"%s\";\n", DOCUMENT, tmpBuf);
		boaWrite(wp, "%s.tcpip.ipv6_prefix2.value = \"%d\";\n", DOCUMENT, ip6_addr[0].prefix_len);
	}
	else
	{
		mib_get(MIB_LAN_IPV6_ADDR2, (void *)&ip6_addr[0].addr);
		mib_get(MIB_LAN_IPV6_PREFIX_LEN2, (void *)&vChar);
		inet_ntop(PF_INET6, &ip6_addr[0].addr, tmpBuf, 128);
		boaWrite(wp, "%s.tcpip.ipv6_addr2.value = \"%s\";\n", DOCUMENT, tmpBuf);
		boaWrite(wp, "%s.tcpip.ipv6_prefix2.value = \"%u\";\n", DOCUMENT, vChar);
	}
#endif

	// Magician: RG restriction, 2nd IP only support v4 currently.
	boaWrite(wp, "%s.tcpip.ip_version2.disabled = true;\n", DOCUMENT);
#endif
#endif
}

// Mason Yu. IGMP snooping for e8b
void initIgmpsnoopPage(request * wp)
{
	unsigned char vChar;

#if defined(CONFIG_RTL_IGMP_SNOOPING)
	mib_get( MIB_MPMODE, (void *)&vChar);
	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	// IGMP snooping: bit-2
	boaWrite(wp, "%s.igmpsnoop.snoop[%d].checked = true;\n", DOCUMENT, (vChar>>MP_IGMP_SHIFT)&0x01);
#endif
}

//Martin ZHU US/DS Bandwidth Monitor
#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
void initBandwidthMonitorPage(request * wp)
{
	unsigned char vChar;
	mib_get( MIB_LANHOST_BANDWIDTH_MONITOR_ENABLE, (void *)&vChar);
	boaWrite(wp, "%s.bandwidthmonitor.monitor[%d].checked = true;\n", DOCUMENT, vChar);
}
#endif

#if defined (CONFIG_USER_LAN_BANDWIDTH_MONITOR)|| defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL)
void initBandwidthIntervalPage(request * wp)
{
	unsigned int bdw_interval;
	mib_get( MIB_LANHOST_BANDWIDTH_INTERVAL, (void *)&bdw_interval);
	boaWrite(wp, "%s.bandwidthinterval.bdw_interval.value = %d;\n", DOCUMENT, bdw_interval);
}
#endif


#ifdef CONFIG_IPV6
void initMLDsnoopPage(request * wp)
{
	unsigned char vChar;
#if defined(CONFIG_RTL_IGMP_SNOOPING)
	mib_get( MIB_MPMODE, (void *)&vChar);
	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	// IGMP snooping: bit-2
	// MLD snooping: bit-3
	boaWrite(wp, "%s.mldsnoop.snoop[%d].checked = true;\n", DOCUMENT, (vChar>>MP_MLD_SHIFT)&0x01);
#endif
}
#endif

#ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_VDSL
struct initvd2p_t
{
	unsigned short	profile;
	unsigned char	*name;
	unsigned char	*tagname;
};

static struct initvd2p_t initvd2p[]=
{
{VDSL2_PROFILE_8A,	"8a",	"vdsl2p8a"},
{VDSL2_PROFILE_8B,	"8b",	"vdsl2p8b"},
{VDSL2_PROFILE_8C,	"8c",	"vdsl2p8c"},
{VDSL2_PROFILE_8D,	"8d",	"vdsl2p8d"},
{VDSL2_PROFILE_12A,	"12a",	"vdsl2p12a"},
{VDSL2_PROFILE_12B,	"12b",	"vdsl2p12b"},
{VDSL2_PROFILE_17A,	"17a",	"vdsl2p17a"},
{VDSL2_PROFILE_30A,	"30a",	"vdsl2p30a"},
};

#define initvd2p_listnum (sizeof(initvd2p)/sizeof(struct initvd2p_t))


void initVD2_check(request * wp)
{
	boaWrite(wp, "\t && document.set_adsl.vdsl2.checked == false\n");
}


//#define CHECK_XDSL_ONE_MODE

#ifdef CHECK_XDSL_ONE_MODE
void initVD2_check_xdsl_one_mode(request * wp)
{
	boaWrite(wp, "if((document.set_adsl.glite.checked == true\n");
	boaWrite(wp, "     || document.set_adsl.gdmt.checked == true\n");
	boaWrite(wp, "     || document.set_adsl.t1413.checked == true\n");
	boaWrite(wp, "     || document.set_adsl.adsl2.checked == true\n");
	boaWrite(wp, "     || document.set_adsl.adsl2p.checked == true)\n");
	boaWrite(wp, "  &&(document.set_adsl.vdsl2.checked == true)){\n");
	boaWrite(wp, "	alert(\"you can't select BOTH ADSL and VDSL!\");\n");
	boaWrite(wp, "	return false;\n");
	boaWrite(wp, "}\n");
}
#endif /*CHECK_XDSL_ONE_MODE*/



void initVD2_check_profile(request * wp)
{
	int i;

#ifdef CHECK_XDSL_ONE_MODE
	initVD2_check_xdsl_one_mode(wp);
#endif /*CHECK_XDSL_ONE_MODE*/

	boaWrite(wp, "if(document.set_adsl.vdsl2.checked == true)\n");
	boaWrite(wp, "{\n");
	boaWrite(wp, "	if(\n");
	for(i=0; i<initvd2p_listnum; i++)
	{
		if(i) boaWrite(wp, "&&");
		boaWrite(wp, "(document.set_adsl.%s.checked==false)\n", initvd2p[i].tagname );
	}
	boaWrite(wp, "	){\n");
	boaWrite(wp, "		alert(\"VDSL2 Profile cannot be empty.\");\n");
	boaWrite(wp, "		return false;\n");
	boaWrite(wp, "	}\n");
	boaWrite(wp, "}\n");
}

void initVD2_updatefn(request * wp)
{
	int i;
	boaWrite(wp, "function updateVDSL2()\n{\n");
	boaWrite(wp, "	var vd2dis;\n\n");
	boaWrite(wp, "	if(document.set_adsl.vdsl2.checked==true)\n");
	boaWrite(wp, "		vd2dis=false;\n");
	boaWrite(wp, "	else\n");
	boaWrite(wp, "		vd2dis=true;\n\n");
	for(i=0; i<initvd2p_listnum; i++)
	{
		boaWrite(wp, "	document.set_adsl.%s.disabled=vd2dis;\n", initvd2p[i].tagname );
	}
	boaWrite(wp, "}\n");
}
void initVD2_opt(request * wp)
{
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th></th>\n");
	boaWrite(wp, "	<td><font size=2><input type=checkbox name=vdsl2 value=1 ONCLICK=updateVDSL2()>VDSL2</td>\n");
	boaWrite(wp, "</tr>\n");
}


void initVD2_profile(request * wp)
{
	int i;

	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th align=left width=30%%><font size=2>VDSL2 Profile:</th>\n");
	boaWrite(wp, "	<td width=70%%></td>\n");
	boaWrite(wp, "</tr>\n");
	for(i=0; i<initvd2p_listnum; i++)
	{
		boaWrite(wp, "<tr>\n");
		boaWrite(wp, "	<th></th>\n");
		boaWrite(wp, "	<td><font size=2><input type=checkbox name=%s value=1>%s</td>\n",
				initvd2p[i].tagname, initvd2p[i].name);
		boaWrite(wp, "</tr>\n");
	}
}
#endif /*CONFIG_VDSL*/


void init_adsl_tone_mask(request * wp)
{
#ifndef CONFIG_VDSL
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th align=left><font size=2>ADSL %s:</th>\n", "Tone Mask" );
	boaWrite(wp, "	<td></td>\n");
	boaWrite(wp, "</tr>\n");
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th></th>\n");
	boaWrite(wp, "	<td><font size=2><input type=\"button\" value=\"%s\" name=\"adsltoneTbl\" onClick=\"adsltoneClick('/adsltone.asp')\"></td>\n", "Tone Mask" );
	boaWrite(wp, "</tr>\n");
#endif /*CONFIG_VDSL*/
}


void init_adsl_psd_mask(request * wp)
{
#ifndef CONFIG_VDSL
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th align=left><font size=2>ADSL %s:</th>\n", "PSD Mask" );
	boaWrite(wp, "	<td></td>\n");
	boaWrite(wp, "</tr>\n");
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th></th>\n");
	boaWrite(wp, "	<td><font size=2><input type=\"button\" value=\"%s\" name=\"adslPSDMaskTbl\" onClick=\"adsltoneClick('/adslpsd.asp')\"></td>\n", "PSD Mask" );
	boaWrite(wp, "</tr>\n");
#endif /*CONFIG_VDSL*/
}


void init_psd_msm_mode(request * wp)
{
#ifndef CONFIG_VDSL
#if SUPPORT_TR105
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th align=left><font size=2>%s:</th>\n", multilang_bpas("PSD Measurement Mode") );
	boaWrite(wp, "	<td></td>\n");
	boaWrite(wp, "</tr>\n");
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th></th>\n");
	boaWrite(wp, "	<td><font size=2>\n");
	adslPSDMeasure(0, wp, 0, NULL);
	boaWrite(wp, "	</td>\n");
	boaWrite(wp, "</tr>\n");
#endif /*SUPPORT_TR105*/
#endif /*CONFIG_VDSL*/
}

void initSetDslPage(request * wp)
{
	unsigned char vChar;
	unsigned short mode;
#ifdef CONFIG_VDSL
	unsigned short vd2p;
#endif /*CONFIG_VDSL*/


	mib_get( MIB_ADSL_MODE, (void *)&mode);
	// TODO: adsl mode
	if (mode & ADSL_MODE_GLITE)
		boaWrite(wp, "%s.set_adsl.glite.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_T1413)
		boaWrite(wp, "%s.set_adsl.t1413.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_GDMT)
		boaWrite(wp, "%s.set_adsl.gdmt.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_ADSL2)
		boaWrite(wp, "%s.set_adsl.adsl2.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_ADSL2P)
		boaWrite(wp, "%s.set_adsl.adsl2p.checked = true;\n", DOCUMENT);
	if (mode & ADSL_MODE_ANXB) {
		boaWrite(wp, "%s.set_adsl.anxl.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.set_adsl.anxm.disabled = true;\n", DOCUMENT);
	}
	else {
		if (mode & ADSL_MODE_ANXL)
			boaWrite(wp, "%s.set_adsl.anxl.checked = true;\n", DOCUMENT);
		if (mode & ADSL_MODE_ANXM)
			boaWrite(wp, "%s.set_adsl.anxm.checked = true;\n", DOCUMENT);
	}
#ifdef ENABLE_ADSL_MODE_GINP
	if (mode & ADSL_MODE_GINP)
		boaWrite(wp, "%s.set_adsl.ginp.checked = true;\n", DOCUMENT);
#endif

#ifdef CONFIG_VDSL
	if (mode & ADSL_MODE_VDSL2)
		boaWrite(wp, "%s.set_adsl.vdsl2.checked = true;\n", DOCUMENT);

	mib_get( MIB_VDSL2_PROFILE, (void *)&vd2p);
	{
		int i;
		for(i=0; i<initvd2p_listnum; i++)
		{
			if (vd2p & initvd2p[i].profile)
				boaWrite(wp, "%s.set_adsl.%s.checked = true;\n", DOCUMENT, initvd2p[i].tagname);
		}
	}
	boaWrite(wp, "updateVDSL2();\n");
#endif /*CONFIG_VDSL*/

	mib_get( MIB_ADSL_OLR, (void *)&vChar);

	if (vChar & 1)
		boaWrite(wp, "%s.set_adsl.bswap.checked = true;\n", DOCUMENT);
	if (vChar & 2)
		boaWrite(wp, "%s.set_adsl.sra.checked = true;\n", DOCUMENT);
}

void initDiagDslPage(request * wp, XDSL_OP *d)
{
#ifdef CONFIG_VDSL
	int mval=0;
	if(d->xdsl_msg_get(GetPmdMode,&mval))
	{
		if(mval&(MODE_ADSL2|MODE_ADSL2PLUS|MODE_VDSL2))
			boaWrite(wp, "%s.diag_adsl.start.disabled = false;\n", DOCUMENT);
	}
#else
	char chVal[2];
	if(d->xdsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&chVal[0], 1)) {
		chVal[0]&=0x1F;
		if (chVal[0] == 3 || chVal[0] == 5)  // ADSL2/2+
			boaWrite(wp, "%s.diag_adsl.start.disabled = false;\n", DOCUMENT);
	}
#endif /*CONFIG_VDSL*/
}
#endif

#ifdef PORT_TRIGGERING
int portTrgList(request * wp)
{
	unsigned int entryNum, i;
	MIB_CE_PORT_TRG_T Entry;
	char	*type, portRange[20], *ip;

	entryNum = mib_chain_total(MIB_PORT_TRG_TBL);

	boaWrite(wp,"<tr><td bgColor=#808080>%s</td><td bgColor=#808080>%s</td>"
		"<td bgColor=#808080>TCP %s</td><td bgColor=#808080>UDP %s</td><td bgColor=#808080>%s</td><td bgColor=#808080>%s</td></tr>\n",
	multilang_bpas("Name"), multilang_bpas("IP Address"), multilang_bpas("Port to Open"),
	multilang_bpas("Port to Open"), multilang_bpas("Enable"), multilang_bpas("Action"));

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PORT_TRG_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		//Name
		boaWrite(wp,"<tr><td bgColor=#C0C0C0>%s</td>\n",Entry.name);

		//IP
		boaWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",inet_ntoa(*((struct in_addr *)Entry.ip)));

		//TCP port to open
		boaWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",Entry.tcpRange);

		//UDP port to open
		boaWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",Entry.udpRange);

		//Enable
		boaWrite(wp,"<td bgColor=#C0C0C0>%s</td>\n",(Entry.enable==1)?"Enable":"Disable");

		//Action
		boaWrite(wp,"<td bgColor=#C0C0C0>");
		boaWrite(wp,
		"<a href=\"#?edit\" onClick=\"editClick(%d)\">"
		"<image border=0 src=\"graphics/edit.gif\" alt=\"Post for editing\" /></a>", i);

		boaWrite(wp,
		"<a href=\"#?delete\" onClick=\"delClick(%d)\">"
		"<image border=0 src=\"graphics/del.gif\" alt=Delete /></td></tr>\n", i);
	}

	return 0;
}

int gm_postIndex=-1;

void initGamingPage(request * wp)
{
	char *ipaddr;
	char *idx;
	int del;
	char ipaddr2[16]={0};
	MIB_CE_PORT_TRG_T Entry;
	int found=0;

	ipaddr=boaGetVar(wp,"ip","");
	idx=boaGetVar(wp,"idx","");
	del=atoi(boaGetVar(wp,"del",""));

	if (gm_postIndex >= 0) { // post this entry
		if (!mib_chain_get(MIB_PORT_TRG_TBL, gm_postIndex, (void *)&Entry))
			found = 0;
		else
			found = 1;
		gm_postIndex = -1;
	}

	if(del!=0)
	{
		boaWrite(wp,
		"<body onLoad=\"document.formname.submit()\">");
	}
	else
	{
		boaWrite(wp,
		"<body bgcolor=\"#ffffff\" text=\"#000000\" onLoad=\"javascript:formLoad();\">");
		boaWrite(wp, "<blockquote><h2><font color=\"#0000FF\">%s%s</font></h2>\n",
		multilang_bpas("Port Triggering"), multilang_bpas(" Configuration"));
		boaWrite(wp, "<table border=0 width=850 cellspacing=4 cellpadding=0><tr><td><hr size=1 noshade align=top></td></tr>\n");
		//<b>%s Game Rule</b>%s",(strlen(idx)==0)?"Add":"Edit",(strlen(idx)==0)?"":" [<a href=\"gaming.asp\">Add New</a>]");
	}


	boaWrite(wp,
	"<form action=/boaform/formGaming method=POST name=formname>\n");

	if(del!=0)
	{
		int i=atoi(idx);
		boaWrite(wp,"<input type=hidden name=idx value=\"%d\">",i);
		boaWrite(wp,"<input type=hidden name=del value=1></form>");
		return;
	}

	boaWrite(wp,
	"<table width=850 cellSpacing=1 cellPadding=2 border=0>\n" \
	"<tr><font size=2><td bgColor=#808080>%s</td><td bgColor=#808080>%s</td><td bgColor=#808080>TCP %s</td><td bgColor=#808080>UDP %s</td><td bgColor=#808080>%s</td></tr>\n",
	multilang_bpas("Name"), multilang_bpas("IP Address"), multilang_bpas("Port to Open"),
	multilang_bpas("Port to Open"), multilang_bpas("Enable"));

	boaWrite(wp,
	"<tr><td bgColor=#C0C0C0><font size=2><input type=text name=\"game\" size=16  maxlength=20 value=\"%s\">&lt;&lt; <select name=\"gamelist\" onChange=\"javascript:changeItem();\"></select></td>" \
	"<td bgColor=#C0C0C0><input type=text name=\"ip\" size=12  maxlength=15 value=\"%s\"></td>" \
	"<td bgColor=#C0C0C0><input type=text name=\"tcpopen\" size=20  maxlength=31 value=\"%s\"></td>" \
	"<td bgColor=#C0C0C0><input type=text name=\"udpopen\" size=20  maxlength=31 value=\"%s\"></td>" \
	"<td bgColor=#C0C0C0><input type=checkbox name=\"open\" value=1 %s></td>" \
	"<input type=hidden name=idx value=%s>" \
	"</tr></table>\n",
	found ? (char *)Entry.name : "",
	found ? (char *)inet_ntoa(*((struct in_addr *)Entry.ip)) : "0.0.0.0",
	found ? (char *)Entry.tcpRange : "",
	found ? (char *)Entry.udpRange : "",
	found ? (Entry.enable == 1 ? multilang_bpas("Checked") :"") : "",
	(strlen(idx)==0)?"-1":idx
	);

	boaWrite(wp,
	"<input type=submit value=%s name=add onClick=\"return addClick()\">&nbsp;&nbsp;&nbsp;&nbsp;" \
	"<input type=submit value=%s name=modify onClick=\"return addClick()\">&nbsp;&nbsp;&nbsp;&nbsp;" \
	"<input type=reset value=%s><BR><BR>\n",
	multilang_bpas("Add"), multilang_bpas("Modify"), multilang_bpas("Reset"));
	boaWrite(wp,
	"<input type=hidden value=/gaming.asp name=submit-url>");

	boaWrite(wp,
	"<b>%s</b>\n" \
//	"<input type=hidden name=ms value=%d>\n" \
/*	"onSubmit=\"return checkRange();\"" \ */
	"<table cellSpacing=1 cellPadding=2 border=0>\n", multilang_bpas("Game Rules List"));

	portTrgList(wp);

	boaWrite(wp, "</form>\n");
}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
void initRipPage(request * wp)
{
	if (ifWanNum("rt") ==0) {
		boaWrite(wp, "%s.rip.rip_on[0].disabled = true;\n", DOCUMENT);
		boaWrite(wp, "\t%s.rip.rip_on[1].disabled = true;\n", DOCUMENT);
		boaWrite(wp, "\t%s.rip.rip_ver.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "\t%s.rip.rip_if.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "\t%s.rip.ripAdd.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "\t%s.rip.ripSet.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "\t%s.rip.ripReset.disabled = true;", DOCUMENT);
	}
	boaWrite(wp, "\t%s.rip.ripDel.disabled = true;\n", DOCUMENT);
}
#endif

// Mason Yu. combine_1p_4p_PortMapping
#ifdef ITF_GROUP
void initPortMapPage(request * wp)
{
	unsigned char vChar;

	mib_get( MIB_MPMODE, (void *)&vChar);
	boaWrite(wp, "%s.eth2pvc.pmap[%d].checked = true;\n", DOCUMENT, (vChar>>MP_PMAP_SHIFT)&0x01);
}

#endif

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
void initLinkPage(request * wp)
{
	unsigned int entryNum, i;
	MIB_CE_SW_PORT_T Entry;
	char ports[]="p0";

	entryNum = mib_chain_total(MIB_SW_PORT_TBL);

	if (entryNum >= SW_LAN_PORT_NUM)
		entryNum = SW_LAN_PORT_NUM;

	for (i=0; i<entryNum; i++) {
		if (mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Entry)) {
			ports[1]=i + '0';
			boaWrite(wp, "%s.link.%s.value = %d;\n", DOCUMENT, ports, Entry.linkMode);
		}
	}
}
#endif

#else
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
void initLinkPage(request * wp)
{

	unsigned int entryNum, i;
	//MIB_CE_SW_PORT_T Entry;
	char ports[]="p0";
	unsigned char mode;

	//entryNum = mib_chain_total(MIB_SW_PORT_TBL);
	if (mib_get(MIB_ETH_MODE, &mode)) {
		boaWrite(wp, "%s.link.%s.value = %d;\n", DOCUMENT, ports, mode);
	}
}

#endif
#endif	// CONFIG_RTL_MULTI_LAN_DEV

void initIpQosPage(request * wp)
{
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	unsigned char vChar;
	unsigned int entryNum;
#ifdef NEW_IP_QOS_SUPPORT
	unsigned char policy;
#endif
#ifdef QOS_DIFFSERV
	unsigned char qosDomain;

	mib_get(MIB_QOS_DIFFSERV, (void *)&qosDomain);
	mib_get(MIB_MPMODE, (void *)&vChar);
	if (qosDomain == 1)
		boaWrite(wp, "%s.qos.qosen[0].checked = true;\n", DOCUMENT);
	else
		boaWrite(wp, "%s.qos.qosen[%d].checked = true;\n", DOCUMENT, (vChar>>MP_IPQ_SHIFT)&0x01);
#else

	mib_get( MIB_MPMODE, (void *)&vChar);
	boaWrite(wp, "%s.qos.qosen[%d].checked = true;\n", DOCUMENT, (vChar>>MP_IPQ_SHIFT)&0x01);
#endif

#ifdef NEW_IP_QOS_SUPPORT
	mib_get( MIB_QOS_POLICY, (void *)&policy);
	boaWrite(wp, "%s.qos.qosPolicy[%d].checked = true;\n", DOCUMENT, policy&0x01);
#endif
	/*
	if (!(vChar&0x02)) { // IP Qos not enabled
		boaWrite(wp, "%s.qos.sip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.smask.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.dip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.dmask.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.sport.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.dport.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.prot.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.phyport.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.out_if.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.prio.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.ipprio.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.tos.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.m1p.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qos.addqos.disabled = true;\n", DOCUMENT);
	}
	*/
#ifdef QOS_SPEED_LIMIT_SUPPORT
	if ((vChar&0x02)) { // IP Qos  enabled
		boaWrite(wp,"document.getElementById('pvcbandwidth').style.display = 'block';\n");
		unsigned short bandwidth;
		mib_get(MIB_PVC_TOTAL_BANDWIDTH,&bandwidth);
		printf("bandwidth=%d\n",bandwidth);
		boaWrite(wp,"document.upbandwidthfm.upbandwidth.value=%d;",bandwidth);

	}
#endif
	if (ifWanNum("all") == 0)
		boaWrite(wp, "%s.qos.addqos.disabled = true;\n", DOCUMENT);

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	if (entryNum == 0) {
		boaWrite(wp, "%s.qostbl.delSel.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.qostbl.delAll.disabled = true;\n", DOCUMENT);
	}


//#ifndef IP_QOS_VPORT
#ifndef CONFIG_RE8305
	mib_get( MIB_QOS_DOMAIN, (void *)&vChar);
	boaWrite(wp, "%s.qos.qosdmn.value = %d;\n", DOCUMENT, vChar);
#ifdef CONFIG_8021P_PRIO
	boaWrite(wp, "enable8021psetting();\n");
#endif
#endif
//#endif
#endif
}

#ifdef QOS_DIFFSERV
void initDiffservPage(request * wp)
{
	unsigned char vChar, phbclass;
	unsigned int entryNum, i;
	MIB_CE_IP_QOS_T Entry;

	mib_get(MIB_QOS_DIFFSERV, (void *)&vChar);
	mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&phbclass);
	boaWrite(wp, "%s.diffserv.qoscap[%d].checked = true;\n", DOCUMENT, vChar);
	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry)) {
  			boaError(wp, 400, "Get chain record error!\n");
			return;
		}
		if (Entry.enDiffserv == 0) // IP QoS entry
			continue;
		if (Entry.m_ipprio != phbclass) // only get active PHB class
			continue;
		boaWrite(wp, "%s.diffserv.totalbandwidth.value = %d;\n", DOCUMENT, Entry.totalBandwidth);
		boaWrite(wp, "%s.diffserv.htbrate.value = %d;\n", DOCUMENT, Entry.htbRate);
		boaWrite(wp, "%s.diffserv.latency.value = %d;\n", DOCUMENT, Entry.latency);
		boaWrite(wp, "%s.diffserv.phbclass.value = %d;\n", DOCUMENT, phbclass);
		boaWrite(wp, "%s.diffserv.interface.value = %d;\n", DOCUMENT, Entry.ifIndex);
		return;
	}
}
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
void initQosQueue(request * wp)
{
	unsigned int entryNumAtmVC, j;
	MIB_CE_ATM_VC_T Entry_atmVC;
	int enable_IPQoS=0;
	char qosQueue[16],qosItfList[128];
	MEDIA_TYPE_T mType;

	qosItfList[0]='\0';
	entryNumAtmVC = mib_chain_total(MIB_ATM_VC_TBL);
	for (j=0; j<entryNumAtmVC; j++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&Entry_atmVC))
			continue;
		if (Entry_atmVC.enableIpQos == 1 ) {
			mType = MEDIA_INDEX(Entry_atmVC.ifIndex);
			enable_IPQoS = 1;
			if (mType == MEDIA_ATM)
				sprintf(qosQueue,"%d,%d_%d",Entry_atmVC.ifIndex,Entry_atmVC.vpi,Entry_atmVC.vci);
			else if (mType == MEDIA_ETH)
				sprintf(qosQueue,"%d,%s%d",Entry_atmVC.ifIndex,ALIASNAME_NAS, ETH_INDEX(Entry_atmVC.ifIndex));
		#ifdef CONFIG_PTMWAN
			else if (mType == MEDIA_PTM)
				sprintf(qosQueue,"%d,%s%d",Entry_atmVC.ifIndex,ALIASNAME_PTM, PTM_INDEX(Entry_atmVC.ifIndex));
		#endif /*CONFIG_PTMWAN*/
			else
				sprintf(qosQueue,"%d,unknown%d",Entry_atmVC.ifIndex,ETH_INDEX(Entry_atmVC.ifIndex));

			if(strlen(qosItfList))
				strcat(qosItfList,";");
			strcat(qosItfList,qosQueue);
		}
	}
	boaWrite(wp,"qDesclist=\"%s\";\n",qosItfList);
	boaWrite(wp,"%s.qos.check.value=\"%d\";\n", DOCUMENT, enable_IPQoS);
}
#endif

void initOthersPage(request * wp)
{
	unsigned int vInt;
	unsigned char vChar;

#ifdef IP_PASSTHROUGH
	mib_get( MIB_IPPT_ITF, (void *)&vInt);
	//if (vInt == 0xff) {
	if (vInt == DUMMY_IFINDEX) {
		boaWrite(wp, "%s.others.ltime.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.others.lan_acc.disabled = true;\n", DOCUMENT);
	}

	mib_get( MIB_IPPT_LANACC, (void *)&vChar);
	if (vChar == 1)
		boaWrite(wp, "%s.others.lan_acc.checked = true\n", DOCUMENT);
#endif
}

#ifdef WLAN_SUPPORT

#ifdef WLAN_WPA
void initWlWpaPage(request * wp)
{
	unsigned char buffer[255];
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);

	boaWrite(wp, "%s.formEncrypt.pskFormat.value = %d;\n", DOCUMENT, Entry.wpaPSKFormat);

#ifdef WLAN_1x
	if(Entry.wep!=0)
		boaWrite(wp, "%s.formEncrypt.wepKeyLen[%d].checked = true;\n", DOCUMENT, Entry.wep-1);

	if(Entry.enable1X==1)
		boaWrite(wp, "%s.formEncrypt.use1x.checked = true;\n", DOCUMENT);
	boaWrite(wp, "%s.formEncrypt.wpaAuth[%d].checked = true;\n", DOCUMENT, Entry.wpaAuth-1);
#else
	boaWrite(wp, "%s.formEncrypt.wpaAuth.disabled = true;\n", DOCUMENT);
	boaWrite(wp, "%s.formEncrypt.wepKeyLen.disabled = true;\n", DOCUMENT);
	boaWrite(wp, "%s.formEncrypt.use1x.disabled = true;\n", DOCUMENT);

#endif
}
#endif

void initWlBasicPage(request * wp)
{
	unsigned char vChar;
	MIB_CE_MBSSIB_T Entry;
	wlan_getEntry(&Entry, 0);

#ifdef WLAN_UNIVERSAL_REPEATER
	boaWrite(wp, "%s.getElementById(\"repeater_check\").style.display = \"\";\n", DOCUMENT);
	boaWrite(wp, "%s.getElementById(\"repeater_SSID\").style.display = \"\";\n", DOCUMENT);
#endif
	if (Entry.wlanDisabled!=0)
		// hidden type
		boaWrite(wp, "%s.wlanSetup.wlanDisabled.value = \"ON\";\n", DOCUMENT);
		// checkbox type
		//boaWrite(wp, "%s.wlanSetup.wlanDisabled.checked = true;\n", DOCUMENT);
	boaWrite(wp, "%s.wlanSetup.band.value = %d;\n", DOCUMENT, Entry.wlanBand-1);
	mib_get( MIB_WLAN_CHANNEL_WIDTH,(void *)&vChar);
	boaWrite(wp, "%s.wlanSetup.chanwid.value = %d;\n", DOCUMENT, vChar);
	mib_get( MIB_WLAN_CONTROL_BAND,(void *)&vChar);
	boaWrite(wp, "%s.wlanSetup.ctlband.value = %d;\n", DOCUMENT, vChar);
	mib_get( MIB_TX_POWER, (void *)&vChar);
	boaWrite(wp, "%s.wlanSetup.txpower.selectedIndex = %d;\n", DOCUMENT, vChar);

}

void initWlE8BasicPage(request * wp)
{
	int i;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	int orig_wlan_idx = wlan_idx;
#endif
	char ssid[36];
	char txPower;
	unsigned int vUInt;
	bss_info bss;

	//cathy
	unsigned char vChar,vChar2;
	unsigned char buffer[10];
	//unsigned char strbf[20];
#define RTL8185_IOCTL_GET_MIB 0x89f2
	int skfd;
	struct iwreq wrq;
	int ret;
	struct _misc_data_ misc_data;
	struct user_info * pUser_info;
	MIB_CE_MBSSIB_T Entry;
	pUser_info = search_login_list(wp);

#if defined(CONFIG_RTL_92D_SUPPORT)
	mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&vChar);
	boaWrite(wp, "wlBandMode=%d;\n", vChar);
#elif defined(WLAN0_5G_SUPPORT) && !defined(WLAN_DUALBAND_CONCURRENT)
	boaWrite(wp, "wlBandMode=1;\n");
#endif

	for(i=0; i<NUM_WLAN_INTERFACE;i++){
		wlan_idx = i;

		if(!wlan_getEntry(&Entry, i))
			continue;

		if(pUser_info->priv)//admin
		{
			mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
			boaWrite(wp, "_Band2G5GSupport[%d]=%d;\n", i, vChar);

			//CurrentChannel
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			strncpy(wrq.ifr_name, getWlanIfName(), IFNAMSIZ);
			strcpy(buffer, "channel");
			wrq.u.data.pointer = (caddr_t)buffer;
			wrq.u.data.length = sizeof(buffer);
			ret =ioctl(skfd, RTL8185_IOCTL_GET_MIB, &wrq);
			close(skfd);
			if ( ret != -1) {
				boaWrite(wp, "_wlCurrentChannel[%d]=%d;\n", i, buffer[wrq.u.data.length - 1]);
				//dbg("cgi_wlCurrentChannel=%d\n", buffer[wrq.u.data.length - 1]);
			}
			else
				boaWrite(wp, "_wlCurrentChannel[%d]='N/A';\n", i);
		}

		getWlBssInfo(getWlanIfName(), &bss);
		boaWrite(wp, "_bssid[%d]='%02x:%02x:%02x:%02x:%02x:%02x';\n",
			i, bss.bssid[0], bss.bssid[1], bss.bssid[2],
			bss.bssid[3], bss.bssid[4], bss.bssid[5]);
		//	bssid
		//	mib_get(MIB_ELAN_MAC_ADDR, (void *)buffer);
		//	snprintf(strbf, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
		//				buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
		//	boaWrite(wp, "_bssid[%d]='%s';\n", i, strbf);
		//	dbg("cgi_wlBssid = %s\n", (void *)strbf);

		//get wlanDisable
		vChar = Entry.wlanDisabled;
		boaWrite(wp, "_wlanEnabled[%d]=%s;\n", i, vChar ? "false" : "true");
		//dbg("MIB_WLAN_DISABLED=%d\n", wlanDisable);

		//get hiddenSSID
		vChar = Entry.hidessid;
		boaWrite(wp, "_hiddenSSID[%d]=%s;\n", i, vChar ? "true" : "false");
		//dbg("MIB_WLAN_HIDDEN_SSID=%d\n",hiddenSSID);

		//get SSID
		boaWrite(wp, "_ssid[%d]='%s';\n", i, Entry.ssid);
		//dbg("MIB_WLAN_SSID=%s\n", ssid);

		//get wlTxPower
		if (pUser_info->priv) {
			mib_get(MIB_TX_POWER, (void *)&txPower);
			boaWrite(wp, "_txpower[%d]=%d;\n", i, txPower);
		}
		//dbg("MIB_TX_POWER=%d\n", txPower);

		if(pUser_info->priv)//admin
		{
			//get channel
			mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, &vChar);
			if(vChar)
				boaWrite(wp, "_chan[%d]=0;\n", i);
			else{
				mib_get(MIB_WLAN_CHAN_NUM, (void *)&vChar);
				boaWrite(wp, "_chan[%d]=%d;\n", i, vChar);
			}
			//dbg("MIB_WLAN_CHAN_NUM=%d\n", defChannel);

			//get 54TM mode
			vChar = Entry.wlanBand;
			//dbg("MIB_WLAN_BAND=%d\n", vChar);

			boaWrite(wp, "_band[%d]=%d;\n", i, vChar-1);


			mib_get( MIB_WLAN_CHANNEL_WIDTH,(void *)&vChar);
			if(vChar==1){
				mib_get( MIB_WLAN_11N_COEXIST,(void *)&vChar2);
				if(vChar2)
					vChar=2;
			}
			boaWrite(wp, "_chanwid[%d]=%d;\n", i, vChar);

			mib_get( MIB_WLAN_CONTROL_BAND,(void *)&vChar);
			boaWrite(wp, "_ctlband[%d]=%d;\n", i, vChar);

#ifdef WLAN_QoS
			//get wlWme
			vChar = Entry.wmmEnabled;
			//boaWrite(wp, "wme = \"%d\";\n", vChar);
			//dbg("MIB_WLAN_QoS=%d\n", vChar);
#endif

			vUInt = Entry.fixedTxRate;
			boaWrite(wp, "_txRate[%d]=%u;\n", i,vUInt);

			vChar = Entry.rateAdaptiveEnabled;
			boaWrite(wp, "_auto[%d]=%d;\n", i,vChar);

			memset(&misc_data, 0, sizeof(struct _misc_data_));
			getMiscData(getWlanIfName(), &misc_data);
			boaWrite(wp, "_rf_used[%d]=%u;\n", i, misc_data.mimo_tr_used);

			mib_get( MIB_WLAN_SHORTGI_ENABLED, (void *)&vChar);
			boaWrite(wp, "_shortGI0[%d]=%d;\n", i, vChar);
		}


	}
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = orig_wlan_idx;
#endif

}


//#ifdef WLAN_WDS
void initWlWDSPage(request * wp){
	unsigned char disWlan,mode;
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
	disWlan = Entry.wlanDisabled;
	mode = Entry.wlanMode;
	if(disWlan || mode != AP_WDS_MODE){
		boaWrite(wp,"%s.formWlWdsAdd.wlanWdsEnabled.disabled = true;\n",DOCUMENT);
	}
}

void initWlSurveyPage(request * wp){
#ifdef WLAN_CLIENT
	boaWrite(wp,"%s.formWlSiteSurvey.refresh.disabled = false;\n",DOCUMENT);
#else
	boaWrite(wp,"%s.formWlSiteSurvey.refresh.disabled = true;\n",DOCUMENT);
#endif
}
//#endif

void initWlAdvPage(request * wp)
{
	unsigned char vChar;
#ifdef WIFI_TEST
	unsigned short vShort;
#endif
	MIB_CE_MBSSIB_T Entry;
	wlan_getEntry(&Entry, 0);
	mib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)&vChar);
	boaWrite(wp, "%s.advanceSetup.preamble[%d].checked = true;\n", DOCUMENT, vChar);
	boaWrite(wp, "%s.advanceSetup.hiddenSSID[%d].checked = true;\n", DOCUMENT, Entry.hidessid);
	boaWrite(wp, "%s.advanceSetup.block[%d].checked = true;\n", DOCUMENT, Entry.userisolation==0?1:0);
	mib_get( MIB_WLAN_PROTECTION_DISABLED, (void *)&vChar);
	boaWrite(wp, "%s.advanceSetup.protection[%d].checked = true;\n", DOCUMENT, vChar);
	mib_get( MIB_WLAN_AGGREGATION, (void *)&vChar);
	boaWrite(wp, "%s.advanceSetup.aggregation[%d].checked = true;\n", DOCUMENT, vChar==0?1:0);
	mib_get( MIB_WLAN_SHORTGI_ENABLED, (void *)&vChar);
	boaWrite(wp, "%s.advanceSetup.shortGI0[%d].checked = true;\n", DOCUMENT, vChar==0?1:0);
#ifdef WLAN_QoS
	boaWrite(wp, "%s.advanceSetup.WmmEnabled[%d].checked = true;\n", DOCUMENT, Entry.wmmEnabled==0?1:0);
#endif
}

#ifdef WLAN_MBSSID
void initWLMBSSIDPage(request * wp)
{
	MIB_CE_MBSSIB_T Entry;
	int i=0;
	unsigned char vChar;

	if (mib_get(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar) == 0) {
		printf("get MBSSID error!");
	}
	boaWrite(wp, "%s.WlanMBSSID.mbssid_block[%d].checked = true;\n", DOCUMENT, vChar);

	for (i=1; i<=4; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {
  			printf("Error! Get MIB_MBSSIB_TBL(initWLMBSSIDPage) error.\n");
  			return;
		}
		boaWrite(wp, "%s.WlanMBSSID.wlAPIsolation_wl%d[%d].checked = true;\n", DOCUMENT, i-1, Entry.userisolation?0:1);
	}
}

void initWLMultiApPage(request * wp)
{
	MIB_CE_MBSSIB_T Entry;
	int i=0;
	unsigned char vChar;

	if (mib_get(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar) == 0) {
		printf("get MBSSID error!");
	}
	boaWrite(wp, "%s.MultipleAP.mbssid_block[%d].checked = true;\n", DOCUMENT, vChar);

	for (i=1; i<=4; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {
  			printf("Error! Get MIB_MBSSIB_TBL(initWLMultiApPage) error.\n");
  			return;
		}
		boaWrite(wp, "%s.MultipleAP.elements[\"wl_hide_ssid%d\"].selectedIndex = %d;\n", DOCUMENT, i, Entry.hidessid?0:1);
		boaWrite(wp, "%s.MultipleAP.elements[\"wl_access%d\"].selectedIndex = %d;\n", DOCUMENT, i, Entry.userisolation);
	}
}

#ifdef CONFIG_USER_FON
void initWLFonPage(request * wp)
{
	MIB_CE_MBSSIB_T Entry;
	int i=0;
	unsigned char vChar;

	if (mib_get(MIB_WLAN_BLOCK_MBSSID, (void *)&vChar) == 0) {
		printf("get MBSSID error!");
	}
	boaWrite(wp, "%s.MultipleAP.mbssid_block[%d].checked = true;\n", DOCUMENT, vChar);

//	for (i=1; i<=4; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, 4, (void *)&Entry)) {
  			printf("Error! Get MIB_MBSSIB_TBL(initWLMultiApPage) error.\n");
  			return;
		}
		boaWrite(wp, "%s.MultipleAP.elements[\"wl_hide_ssid%d\"].selectedIndex = %d;\n", DOCUMENT, 4, Entry.hidessid?0:1);
		boaWrite(wp, "%s.MultipleAP.elements[\"wl_access%d\"].selectedIndex = %d;\n", DOCUMENT, 4, Entry.userisolation);
//	}
}
#endif
#endif

extern void wapi_mod_entry(MIB_CE_MBSSIB_T *, char *, char *);
static void wlan_ssid_helper(MIB_CE_MBSSIB_Tp pEntry, char *psk, char *RsIp)
{
	int len;

	// wpaPSK
	for (len=0; len<strlen(pEntry->wpaPSK); len++)
		psk[len]='*';
	psk[len]='\0';

	// RsIp
	if ( ((struct in_addr *)pEntry->rsIpAddr)->s_addr == INADDR_NONE ) {
		sprintf(RsIp, "%s", "");
	} else {
		sprintf(RsIp, "%s", inet_ntoa(*((struct in_addr *)pEntry->rsIpAddr)));
	}
	#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (pEntry->encrypt == WIFI_SEC_WAPI) {
		wapi_mod_entry(pEntry, psk, RsIp);
	}
	#endif
}

void initWlWpaMbssidPage(request * wp)
{
	MIB_CE_MBSSIB_T Entry;
	char strbuf[MAX_PSK_LEN+1], strbuf2[20];
	int isNmode;
	int i, k;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	char *strVal;
	int orig_wlan_idx = wlan_idx;
	strVal = boaGetVar(wp, "wlan_idx", "");
	if ( strVal[0] ) {
		//printf("wlan_idx=%d\n", strVal[0]-'0');
		wlan_idx = strVal[0]-'0';
	}
#endif

	k=0;
	for (i=0; i<=NUM_VWLAN_INTERFACE; i++) {
		wlan_getEntry(&Entry, i);
		if (Entry.wlanDisabled)
			continue;
		wlan_ssid_helper(&Entry, strbuf, strbuf2);
		boaWrite(wp, "_wlan_mode[%d]=%d;\n", k, Entry.wlanMode);
		boaWrite(wp, "\t_encrypt[%d]=%d;\n", k, Entry.encrypt);
		boaWrite(wp, "\t_enable1X[%d]=%d;\n", k, Entry.enable1X);
		boaWrite(wp, "\t_wpaAuth[%d]=%d;\n", k, Entry.wpaAuth);
		boaWrite(wp, "\t_wpaPSKFormat[%d]=%d;\n", k, Entry.wpaPSKFormat);
		//boaWrite(wp, "\t_wpaPSK[%d]='%s';\n", k, strbuf);
		boaWrite(wp, "\t_wpaPSK[%d]='%s';\n", k, Entry.wpaPSK);	//fix web check psk-key invalid problem
		boaWrite(wp, "\t_rsPort[%d]=%d;\n", k, Entry.rsPort);
		boaWrite(wp, "\t_rsIpAddr[%d]='%s';\n", k, strbuf2);
		boaWrite(wp, "\t_rsPassword[%d]='%s';\n", k, Entry.rsPassword);
		boaWrite(wp, "\t_uCipher[%d]=%d;\n", k, Entry.unicastCipher);
		boaWrite(wp, "\t_wpa2uCipher[%d]=%d;\n", k, Entry.wpa2UnicastCipher);
		boaWrite(wp, "\t_wepAuth[%d]=%d;\n", k, Entry.authType);
		boaWrite(wp, "\t_wepLen[%d]=%d;\n", k, Entry.wep);
		boaWrite(wp, "\t_wepKeyFormat[%d]=%d;\n\t", k, Entry.wepKeyType);
		isNmode=wl_isNband(Entry.wlanBand);
		boaWrite(wp, "\t_wlan_isNmode[%d]=%d;\n\t", k, isNmode);
		k++;
	}
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = orig_wlan_idx;
#endif

}

#ifdef WLAN_ACL
void initWlAclPage(request * wp)
{
	unsigned char vChar;
	MIB_CE_MBSSIB_T Entry;
	mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);

	if (Entry.wlanDisabled==0) // enabled
		boaWrite(wp,"wlanDisabled=0;\n");
	else // disabled
		boaWrite(wp,"wlanDisabled=1;\n");

	boaWrite(wp,"wlanMode=%d;\n", Entry.wlanMode);

	mib_get( MIB_WLAN_AC_ENABLED, (void *)&vChar);

	boaWrite(wp,"%s.formWlAcAdd.wlanAcEnabled.selectedIndex=%d\n", DOCUMENT, vChar);

}
#endif
#ifdef WIFI_TIMER_SCHEDULE
void initWlTimerExPage(request * wp)
{
	MIB_CE_WIFI_TIMER_EX_T Entry;
	int totalEntry, i;

	totalEntry = mib_chain_total(MIB_WIFI_TIMER_EX_TBL);
	for(i=0; i<totalEntry; i++)
	{
		mib_chain_get(MIB_WIFI_TIMER_EX_TBL, i, &Entry);
		boaWrite(wp, "\t_enable[%d]=%u;\n", i, Entry.enable);
		boaWrite(wp, "\t_onoff[%d]=%u;\n", i, Entry.onoff);
		boaWrite(wp, "\t_time[%d]=\"%s\";\n", i, Entry.Time);
		boaWrite(wp, "\t_day[%d]=%u;\n", i, Entry.day);
	}
}
void initWlTimerPage(request * wp)
{
	MIB_CE_WIFI_TIMER_T Entry;
	int totalEntry, i;

	totalEntry = mib_chain_total(MIB_WIFI_TIMER_TBL);
	for(i=0; i<totalEntry; i++)
	{
		mib_chain_get(MIB_WIFI_TIMER_TBL, i, &Entry);
		boaWrite(wp, "\t_enable[%d]=%u;\n", i, Entry.enable);
		boaWrite(wp, "\t_startTime[%d]=\"%s\";\n", i, Entry.startTime);
		boaWrite(wp, "\t_endTime[%d]=\"%s\";\n", i, Entry.endTime);
		boaWrite(wp, "\t_controlCycle[%u]=%u;\n", i, Entry.controlCycle);
	}
}
#endif
#endif // of WLAN_SUPPORT

#ifdef CONFIG_LED_INDICATOR_TIMER
void initLEDTimerPage(request * wp)
{
	MIB_CE_DAY_SCHED_T Entry;
	int totalEntry, i;
	unsigned char status;

	totalEntry = mib_chain_total(MIB_LED_INDICATOR_TIMER_TBL);
	for(i=0; i<totalEntry; i++)
	{
		mib_chain_get(MIB_LED_INDICATOR_TIMER_TBL, i, &Entry);
		boaWrite(wp, "\t_enable[%d]=%u;\n", i, Entry.enable);
		boaWrite(wp, "\t_startTime[%d]=\"%d:%d\";\n", i, Entry.startHour, Entry.startMin);
		boaWrite(wp, "\t_endTime[%d]=\"%d:%d\";\n", i, Entry.endHour, Entry.endMin);
		boaWrite(wp, "\t_controlCycle[%u]=%u;\n", i, Entry.ctlCycle);
	}
	mib_get(MIB_LED_STATUS, (void *)&status);
	boaWrite(wp, "\t_ledsts=%u;\n",status);
}
#endif

#ifdef DIAGNOSTIC_TEST
void initDiagTestPage(request * wp)
{
	unsigned int inf;
	FILE *fp;

	if (fp = fopen("/tmp/diaginf", "r")) {
		fscanf(fp, "%d", &inf);
		if (inf != DUMMY_IFINDEX)
			boaWrite(wp, "%s.diagtest.wan_if.value = %d;\n", DOCUMENT, inf);
		fclose(fp);
		fp = fopen("/tmp/diaginf", "w");
		fprintf(fp, "%d", DUMMY_IFINDEX); // reset to dummy
		fclose(fp);
	}
}
#endif

// Mason Yu. t123
#if 0
void ShowWmm(int eid, request * wp, int argc, char **argv)
{
#ifdef WLAN_QoS
	boaWrite(wp,
		"<tr><td width=\"30%%\"><font size=2><b>%s:</b></td>" \
		"<td width=\"70%%\"><font size=2>" \
		"<input type=\"radio\" name=WmmEnabled value=1>%s&nbsp;&nbsp;" \
		"<input type=\"radio\" name=WmmEnabled value=0>%s</td></tr>",
		multilang_bpas("WMM Support"), multilang_bpas("Enabled"), multilang_bpas("Disabled"));
#endif
}
#endif

void initDhcpMode(request * wp)
{
	unsigned char vChar;
	char buf[16];

// Kaohj --- assign DHCP pool ip prefix; no pool prefix for complete IP pool
#ifdef DHCPS_POOL_COMPLETE_IP
	boaWrite(wp, "	pool_ipprefix='';\n");
#else
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, buf);
	boaWrite(wp, "	pool_ipprefix='%s';\n", buf);
#endif
// Kaohj
#ifdef DHCPS_DNS_OPTIONS
	boaWrite(wp, "	en_dnsopt=1;\n");
	mib_get(MIB_DHCP_DNS_OPTION, (void *)&vChar);
	boaWrite(wp, "	dnsopt=%d;\n", vChar);
#else
	boaWrite(wp, "	en_dnsopt=0;\n");
#endif
	mib_get(MIB_DHCP_MODE, (void *)&vChar);

	if(vChar == 1) vChar = 2;
	else if(vChar == 2) vChar = 1;

	boaWrite(wp, "	%s.dhcpd.uDhcpType[%d].checked = true;\n", DOCUMENT, vChar);
}

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
void initDhcpv6Mode(request * wp)
{
	unsigned char vChar;
	char buf[16];

	mib_get( MIB_DHCPV6_MODE, (void *)&vChar);
	boaWrite(wp, "%s.dhcpd.dhcpdenable[%d].checked = true;\n", DOCUMENT, vChar);
}
#endif
#endif

void initDhcpMacbase(request * wp)
{
	char buf[16];
// Kaohj --- assign DHCP pool ip prefix; no pool prefix for complete IP pool
#ifdef DHCPS_POOL_COMPLETE_IP
	boaWrite(wp, "pool_ipprefix='';\n");
#else
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, buf);
	boaWrite(wp, "pool_ipprefix='%s';\n", buf);
#endif
}

/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#ifdef IMAGENIO_IPTV_SUPPORT
void initDhcpIPRange(request * wp)
{
	char buf[16];
	unsigned int i, entryNum;
	DHCPS_SERVING_POOL_T Entry;
	MIB_CE_DHCP_OPTION_T rsvOptEntry;
	char startIp[16], endIp[16];
/*ping_zhang:20090526 START:Add gateway for each ip range*/
	char gwIp[16];
/*ping_zhang:20090526 END*/
	int id=-1;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
	boaWrite(wp, "var devname=new Array(%d), devtype=new Array(%d), startip=new Array(%d), endip=new Array(%d), gwip=new Array(%d), option=new Array(%d), opCode=new Array(%d), opStr=new Array(%d);\n",
			entryNum, entryNum, entryNum, entryNum, entryNum, entryNum, entryNum, entryNum);

// Kaohj --- assign DHCP pool ip prefix; no pool prefix for complete IP pool
#ifdef DHCPS_POOL_COMPLETE_IP
	boaWrite(wp, "pool_ipprefix='';\n");
#else
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, buf);
	boaWrite(wp, "pool_ipprefix='%s';\n", buf);
#endif

	for (i=0; i<entryNum; i++) {

		mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL, i, (void *)&Entry);
		strcpy(startIp, inet_ntoa(*((struct in_addr *)Entry.startaddr)));
		strcpy(endIp, inet_ntoa(*((struct in_addr *)Entry.endaddr)));
/*ping_zhang:20090526 START:Add gateway for each ip range*/
		strcpy(gwIp, inet_ntoa(*((struct in_addr *)Entry.iprouter)));
/*ping_zhang:20090526 END*/

		boaWrite(wp, "devname[%d]=\'%s\';\n", i, Entry.poolname);
		boaWrite(wp, "devtype[%d]=\'%d\';\n", i, Entry.deviceType);
		boaWrite(wp, "startip[%d]=\'%s\';\n", i, startIp);
		boaWrite(wp, "endip[%d]=\'%s\';\n", i, endIp);
/*ping_zhang:20090526 START:Add gateway for each ip range*/
		boaWrite(wp, "gwip[%d]=\'%s\';\n", i, gwIp);
/*ping_zhang:20090526 END*/
		boaWrite(wp, "option[%d]=\'%s\';\n", i, Entry.vendorclass);
		boaWrite(wp, "opCode[%d]=\'%d\';\n", i, Entry.rsvOptCode);

		getSPDHCPRsvOptEntryByCode(Entry.InstanceNum, Entry.rsvOptCode, &rsvOptEntry, &id);
		if(id != -1)
			boaWrite(wp, "opStr[%d]=\'%s\';\n", i, rsvOptEntry.value);
		else
			boaWrite(wp, "opStr[%d]=\'\';\n");
	}
}
#endif
#endif
/*ping_zhang:20090319 END*/

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
initAddressMap(request * wp)
{
	boaWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
	boaWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
	boaWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
	boaWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
	boaWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 0;\n", DOCUMENT);
}
#else
initAddressMap(request * wp)
{
	unsigned char vChar;

	mib_get( MIB_ADDRESS_MAP_TYPE, (void *)&vChar);

	if(vChar == ADSMAP_NONE) {         // None
		boaWrite(wp, "%s.addressMap.lsip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.gsip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 0;\n", DOCUMENT);

	} else if (vChar == ADSMAP_ONE_TO_ONE) {  // One-to-One
		boaWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 1;\n", DOCUMENT);

	} else if (vChar == ADSMAP_MANY_TO_ONE) {  // Many-to-One
		boaWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.leip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.geip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 2;\n", DOCUMENT);

	} else if (vChar == ADSMAP_MANY_TO_MANY) {   // Many-to-Many
		boaWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.leip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.geip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 3;\n", DOCUMENT);

	}
	// Masu Yu on True
#if 1
	else if (vChar == ADSMAP_ONE_TO_MANY) {   // One-to-Many
		boaWrite(wp, "%s.addressMap.lsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.leip.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.gsip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.geip.disabled = false;\n", DOCUMENT);
		boaWrite(wp, "%s.addressMap.addressMapType.selectedIndex= 4;\n", DOCUMENT);

	}
#endif
}
#endif // MULTI_ADDRESS_MAPPING
#endif

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
//ql
void initOspf(request * wp)
{
	unsigned char vChar;

#ifdef CONFIG_USER_ROUTED_ROUTED
	mib_get( MIB_RIP_ENABLE, (void *)&vChar );
	if (1 == vChar) {
		boaWrite(wp, "%s.rip.igp.selectedIndex = 0;\n", DOCUMENT);
		boaWrite(wp, "%s.rip.rip_on[1].checked = true;\n", DOCUMENT);
		return;
	}
	mib_get( MIB_OSPF_ENABLE, (void *)&vChar);
	if (1 == vChar) {
		boaWrite(wp, "%s.rip.igp.selectedIndex = 1;\n", DOCUMENT);
		boaWrite(wp, "%s.rip.rip_on[1].checked = true;\n", DOCUMENT);
		return;
	}
#else
	mib_get( MIB_OSPF_ENABLE, (void *)&vChar);
	if (1 == vChar) {
		boaWrite(wp, "%s.rip.igp.selectedIndex = 0;\n", DOCUMENT);
		boaWrite(wp, "%s.rip.rip_on[1].checked = true;\n", DOCUMENT);
		return;
	}
#endif
	//default
	boaWrite(wp, "%s.rip.igp.selectedIndex = 0;\n", DOCUMENT);
	boaWrite(wp, "%s.rip.rip_on[0].checked = true;\n", DOCUMENT);
}
#endif

#ifdef FIELD_TRY_SAFE_MODE
#ifdef CONFIG_DEV_xDSL
void initAdslSafeMode(request * wp)
{
	SafeModeData vSmd;

	memset((void *)&vSmd, 0, sizeof(vSmd));
	adsl_drv_get(RLCM_GET_SAFEMODE_CTRL, (void *)&vSmd, SAFEMODE_DATA_SIZE);
	if (vSmd.FieldTrySafeMode > 0)
		boaWrite(wp, "%s.adsl_safe.safemode.checked = true;\n", DOCUMENT);
	boaWrite(wp, "%s.adsl_safe.psdtime.value = \"%d\";\n", DOCUMENT, vSmd.FieldTryTestPSDTimes);
	boaWrite(wp, "%s.adsl_safe.ctrlin.value = \"%x\";\n", DOCUMENT, vSmd.FieldTryCtrlIn);
}
#endif
#endif
#ifdef CONFIG_ETHWAN
void initEthWan(request * wp)
{
	MIB_CE_ATM_VC_T Entry;
	int index;
#ifdef CONFIG_IPV6
	unsigned char 	Ipv6AddrStr[INET6_ADDRSTRLEN], RemoteIpv6AddrStr[INET6_ADDRSTRLEN];
#endif

	index = getWanEntrybyMedia(&Entry, MEDIA_ETH);
	if (index == -1)
		printf("EthWan interface not found !\n");
	boaWrite(wp, "%s.ethwan.naptEnabled.checked = %s;\n", DOCUMENT, Entry.napt?"true":"false");
	boaWrite(wp, "%s.ethwan.igmpEnabled.checked = %s;\n", DOCUMENT, Entry.enableIGMP?"true":"false");
#ifdef CONFIG_USER_IP_QOS
	boaWrite(wp, "%s.ethwan.qosEnabled.checked = %s;\n", DOCUMENT, Entry.enableIpQos?"true":"false");
#endif

	boaWrite(wp, "%s.ethwan.ipMode[%d].checked = true;\n", DOCUMENT, Entry.ipDhcp == 0? 0:1);
	if(Entry.cmode == CHANNEL_MODE_IPOE){//mer
#ifdef CONFIG_IPV6
		if (Entry.IpProtocol & IPVER_IPV4) {
#endif
		if(Entry.ipDhcp == 0){//ip
			boaWrite(wp, "%s.ethwan.ip.value = \"%s\";\n", DOCUMENT, inet_ntoa(*((struct in_addr *)&Entry.ipAddr)));
			//printf("ip %s\n", Entry.ipAddr);
			boaWrite(wp, "%s.ethwan.remoteIp.value = \"%s\";\n", DOCUMENT, inet_ntoa(*((struct in_addr *)&Entry.remoteIpAddr)));
			boaWrite(wp, "%s.ethwan.netmask.value = \"%s\";\n", DOCUMENT, inet_ntoa(*((struct in_addr *)&Entry.netMask)));
		}
#ifdef CONFIG_IPV6
		}
#endif
	}
	else if(Entry.cmode == CHANNEL_MODE_PPPOE){//pppoe
		boaWrite(wp, "%s.ethwan.pppUserName.value = \"%s\";\n", DOCUMENT, Entry.pppUsername);
		boaWrite(wp, "%s.ethwan.pppPassword.value = \"%s\";\n", DOCUMENT, Entry.pppPassword);
		boaWrite(wp, "%s.ethwan.pppConnectType[%d].checked = true;\n", DOCUMENT, Entry.pppCtype);
		if(Entry.pppCtype == '1'){//connect on demand
			boaWrite(wp, "%s.ethwan.pppConnectType.value = \"%d\";\n", DOCUMENT, Entry.pppIdleTime);
		}
	}
	if (Entry.cmode != CHANNEL_MODE_BRIDGE)
		boaWrite(wp, "%s.ethwan.droute[%d].checked = true;\n", DOCUMENT, Entry.dgw);
#ifdef CONFIG_IPV6
	boaWrite(wp, "%s.ethwan.IpProtocolType.value=%d;\n", DOCUMENT, Entry.IpProtocol);
	if (Entry.AddrMode & 1)
		boaWrite(wp, "%s.ethwan.slacc.checked=true;\n", DOCUMENT);
	else
		boaWrite(wp, "%s.ethwan.slacc.checked=false;\n", DOCUMENT);
	if (Entry.AddrMode & 2)
		boaWrite(wp, "%s.ethwan.staticIpv6.checked=true;\n", DOCUMENT);
	else
		boaWrite(wp, "%s.ethwan.staticIpv6.checked=false;\n", DOCUMENT);
	inet_ntop(PF_INET6, Entry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
	inet_ntop(PF_INET6, Entry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
	boaWrite(wp, "%s.ethwan.Ipv6Addr.value=\"%s\";\n", DOCUMENT, Ipv6AddrStr);
	boaWrite(wp, "%s.ethwan.Ipv6PrefixLen.value=%d;\n", DOCUMENT, Entry.Ipv6AddrPrefixLen);
	boaWrite(wp, "%s.ethwan.Ipv6Gateway.value=\"%s\";\n", DOCUMENT, RemoteIpv6AddrStr);
	if (Entry.Ipv6Dhcp)
		boaWrite(wp, "%s.ethwan.itfenable.checked=true;\n", DOCUMENT);
	else
		boaWrite(wp, "%s.ethwan.itfenable.checked=false;\n", DOCUMENT);
	if (Entry.Ipv6DhcpRequest&1)
		boaWrite(wp, "%s.ethwan.iana.checked=true;\n", DOCUMENT);
	else
		boaWrite(wp, "%s.ethwan.iana.checked=false;\n", DOCUMENT);
	if (Entry.Ipv6DhcpRequest&2)
		boaWrite(wp, "%s.ethwan.iapd.checked=true;\n", DOCUMENT);
	else
		boaWrite(wp, "%s.ethwan.iapd.checked=false;\n", DOCUMENT);
	boaWrite(wp, "ipver=%s.ethwan.IpProtocolType.value;\n", DOCUMENT);
#endif
}
#endif

#ifdef CONFIG_USER_PPTPD_PPTPD
void initPptp(request * wp)
{
	MIB_VPND_T entry;
	int total, i;
	char peeraddr[16];
	char localaddr[16];

	total = mib_chain_total(MIB_VPN_SERVER_TBL);
	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_VPN_SERVER_TBL, i, &entry))
			continue;

		if (VPN_PPTP == entry.type)
			break;
	}

	if (i < total) {
		boaWrite(wp, "document.pptp.s_auth.selectedIndex=%d;\n", entry.authtype);
		boaWrite(wp, "\tdocument.pptp.s_enctype.selectedIndex=%d;\n", entry.enctype);
		sprintf(peeraddr, "%s", inet_ntoa(*(struct in_addr *)&entry.peeraddr));
		sprintf(localaddr, "%s", inet_ntoa(*(struct in_addr *)&entry.localaddr));
		boaWrite(wp, "\tdocument.pptp.peeraddr.value=\"%s\";\n", peeraddr);
		boaWrite(wp, "\tdocument.pptp.localaddr.value=\"%s\";\n", localaddr);
	}
}
#endif

#ifdef CONFIG_USER_L2TPD_LNS
void initL2tp(request * wp)
{
	MIB_VPND_T entry;
	int total, i;
	char peeraddr[16];
	char localaddr[16];

	total = mib_chain_total(MIB_VPN_SERVER_TBL);
	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_VPN_SERVER_TBL, i, &entry))
			continue;

		if (VPN_L2TP == entry.type)
			break;
	}

	if (i < total) {
		boaWrite(wp, "document.l2tp.s_auth.selectedIndex=%d;\n", entry.authtype);
		boaWrite(wp, "\tdocument.l2tp.s_enctype.selectedIndex=%d;\n", entry.enctype);
		boaWrite(wp, "\tdocument.l2tp.s_tunnelAuth.checked=%s;\n", entry.tunnel_auth==1?"true":"false");
		boaWrite(wp, "\tdocument.l2tp.s_authKey.value=\"%s\";\n", entry.tunnel_key);
		sprintf(peeraddr, "%s", inet_ntoa(*(struct in_addr *)&entry.peeraddr));
		sprintf(localaddr, "%s", inet_ntoa(*(struct in_addr *)&entry.localaddr));
		boaWrite(wp, "\tdocument.l2tp.peeraddr.value=\"%s\";\n", peeraddr);
		boaWrite(wp, "\tdocument.l2tp.localaddr.value=\"%s\";\n", localaddr);
	}
}
#endif

/////////////////////////////////////////////////////////////
// Kaohj
int initPage(int eid, request * wp, int argc, char **argv)
{
	char *name;

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if ( !strcmp(name, "lan") )
		initLanPage(wp);
	if ( !strcmp(name, "igmpsnooping") )    // Mason Yu. IGMP snooping for e8b
	{
		printf("initIgmpsnoopPage\n");
		initIgmpsnoopPage(wp);
	}
	
#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR	
	if ( !strcmp(name, "bandwidth_monitor") )    //Martin ZHU. US/DS bandwidth monitor for yueme
	{
		printf("initBandwidthMonitorPage\n");
		initBandwidthMonitorPage(wp);
	}
#endif

#if defined (CONFIG_USER_LAN_BANDWIDTH_MONITOR)|| defined (CONFIG_USER_LAN_BANDWIDTH_CONTROL)
	if ( !strcmp(name, "bdw_interval") )    //Martin ZHU. US/DS bandwidth monitor for yueme
	{
		printf("initBandwidthIntervalPage\n");
		initBandwidthIntervalPage(wp);
	}	
#endif

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_RADVD
	if ( !strcmp(name, "radvd_conf") )
		initRadvdConfPage(wp);
#endif
	if ( !strcmp(name, "mldsnooping") )    // Mason Yu. MLD snooping
		initMLDsnoopPage(wp);
#endif
#ifdef CONFIG_DEV_xDSL
	if ( !strcmp(name, "setdsl") )
		initSetDslPage(wp);
	if ( !strcmp(name, "diagdsl") )
		initDiagDslPage(wp, xdsl_get_op(0) );
#ifdef CONFIG_USER_XDSL_SLAVE
	if ( !strcmp(name, "diagdslslv") )
		initDiagDslPage(wp, xdsl_get_op(1) );
#endif /*CONFIG_USER_XDSL_SLAVE*/
	if ( !strcmp(name, "adsl_tone_mask") )
		init_adsl_tone_mask(wp);
	if ( !strcmp(name, "adsl_psd_mask") )
		init_adsl_psd_mask(wp);
	if ( !strcmp(name, "psd_msm_mode") )
		init_psd_msm_mode(wp);
#ifdef CONFIG_VDSL
	if ( !strcmp(name, "vdsl2_check") )
		initVD2_check(wp);
	if ( !strcmp(name, "vdsl2_check_profile") )
		initVD2_check_profile(wp);
	if ( !strcmp(name, "vdsl2_updatefn") )
		initVD2_updatefn(wp);
	if ( !strcmp(name, "vdsl2_opt") )
		initVD2_opt(wp);
	if ( !strcmp(name, "vdsl2_profile") )
		initVD2_profile(wp);
#endif /*CONFIG_VDSL*/
#endif
#ifdef PORT_TRIGGERING
	if ( !strcmp(name, "gaming") )
		initGamingPage(wp);
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
	if ( !strcmp(name, "rip") )
		initRipPage(wp);
#endif

// Mason Yu. combine_1p_4p_PortMapping
#ifdef ITF_GROUP
	if ( !strcmp(name, "portMap") )
		initPortMapPage(wp);
#endif

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
	if ( !strcmp(name, "linkMode") )
		initLinkPage(wp);
#endif
#else
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	if ( !strcmp(name, "linkMode") )
		initLinkPage(wp);
#endif
#endif	// of CONFIG_RTL_MULTI_LAN_DEV

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	if ( !strcmp(name, "ipqos") )
		initIpQosPage(wp);
	if ( !strcmp(name, "qosQueue") )
		initQosQueue(wp);
#endif
#ifdef QOS_DIFFSERV
	if (!strcmp(name, "diffserv"))
		initDiffservPage(wp);
#endif
	if ( !strcmp(name, "others") )
		initOthersPage(wp);
#ifdef WLAN_SUPPORT
#ifdef WLAN_WPA
	if ( !strcmp(name, "wlwpa") )
		initWlWpaPage(wp);
#endif
	// Mason Yu. 201009_new_security
	if ( !strcmp(name, "wlwpa_mbssid") )
		initWlWpaMbssidPage(wp);
	if ( !strcmp(name, "wlbasic") )
		initWlBasicPage(wp);
	if ( !strcmp(name, "wle8basic") )
		initWlE8BasicPage(wp);
	if ( !strcmp(name, "wladv") )
		initWlAdvPage(wp);
#ifdef WLAN_MBSSID
	// Mason Yu
	if ( !strcmp(name, "wlmbssid") ) {
		initWLMBSSIDPage(wp);
	}
	if ( !strcmp(name, "wlmultipleap") ) {
		initWLMultiApPage(wp);
	}
#ifdef CONFIG_USER_FON
	if ( !strcmp(name, "wlfon") ) {
		initWLFonPage(wp);
	}
#endif
#endif
#ifdef WLAN_WDS
	if ( !strcmp(name, "wlwds") )
		initWlWDSPage(wp);
#endif
#ifdef WLAN_CLIENT
	if ( !strcmp(name, "wlsurvey") )
		initWlSurveyPage(wp);

#endif
#ifdef WLAN_ACL
	if ( !strcmp(name, "wlactrl") )
		initWlAclPage(wp);
#endif
#ifdef WIFI_TIMER_SCHEDULE
	if ( !strcmp(name, "wltimerEx") )
		initWlTimerExPage(wp);
	if( !strcmp(name, "wltimer") )
		initWlTimerPage(wp);
#endif
#ifdef CONFIG_LED_INDICATOR_TIMER 
	if ( !strcmp(name, "ledtimer") )
	{
		initLEDTimerPage(wp);
	}
#endif
#endif

#ifdef DIAGNOSTIC_TEST
	if ( !strcmp(name, "diagTest") )
		initDiagTestPage(wp);
#endif

#ifdef DOS_SUPPORT
	if ( !strcmp(name, "dos") )
		initDosPage(wp);
#endif


#ifdef ADDRESS_MAPPING
	if( !strcmp(name, "addressMap"))
		initAddressMap(wp);
#endif

	if ( !strcmp(name, "dhcp-mode") )
		initDhcpMode(wp);
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	if ( !strcmp(name, "dhcpv6-mode") )
		initDhcpv6Mode(wp);
#endif
#endif
	if ( !strcmp(name, "dhcp-macbase") )
		initDhcpMacbase(wp);
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#ifdef IMAGENIO_IPTV_SUPPORT
	if ( !strcmp(name, "dhcp-iprange") )
		initDhcpIPRange(wp);
#endif
#endif
//add by ramen
#ifdef CONFIG_IP_NF_ALG_ONOFF
if (!strcmp(name, "algonoff"))
	initAlgOnOff(wp);
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	if (!strcmp(name, "ospf"))
		initOspf(wp);
#endif
	if (!strcmp(name, "syslog"))
		initSyslogPage(wp);
#ifdef WEB_ENABLE_PPP_DEBUG
	if ( !strcmp(name, "pppSyslog") )
		initPPPSyslog(wp);
#endif
	if (!strcmp(name, "dgw"))
		initDgwPage(wp);
#ifdef FIELD_TRY_SAFE_MODE
	if ( !strcmp(name, "adslsafemode") )
		initAdslSafeMode(wp);
#endif
#ifdef CONFIG_ETHWAN
	if ( !strcmp(name, "ethwan") )
		initEthWan(wp);
#endif
#ifdef CONFIG_USER_PPTPD_PPTPD
	if (!strcmp(name, "pptp"))
		initPptp(wp);
#endif
#ifdef CONFIG_USER_L2TPD_LNS
	if (!strcmp(name, "l2tp"))
		initL2tp(wp);
#endif

	return 0;
}

#ifdef DOS_SUPPORT
#define DOSENABLE	0x1
#define DOSSYSFLOODSYN	0x2
#define DOSSYSFLOODFIN	0x4
#define	DOSSYSFLOODUDP	0x8
#define DOSSYSFLOODICMP	0x10
#define DOSIPFLOODSYN	0x20
#define DOSIPFLOODFIN	0x40
#define DOSIPFLOODUDP	0x80
#define DOSIPFLOODICMP	0x100
#define DOSTCPUDPPORTSCAN 0x200
#define DOSPORTSCANSENSI  0x800000
#define DOSICMPSMURFENABLED	0x400
#define DOSIPLANDENABLED	0x800
#define DOSIPSPOOFENABLED	0x1000
#define DOSIPTEARDROPENABLED	0x2000
#define DOSPINTOFDEATHENABLED	0x4000
#define DOSTCPSCANENABLED	0x8000
#define DOSTCPSYNWITHDATAENABLED	0x10000
#define DOSUDPBOMBENABLED		0x20000
#define DOSUDPECHOCHARGENENABLED	0x40000
#define DOSSOURCEIPBLOCK		0x400000
void initDosPage(request * wp){
	unsigned int mode;

	mib_get( MIB_DOS_ENABLED, (void *)&mode);
	if (mode & DOSENABLE){
		boaWrite(wp, "%s.DosCfg.dosEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODSYN)
			boaWrite(wp, "%s.DosCfg.sysfloodSYN.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODFIN)
			boaWrite(wp, "%s.DosCfg.sysfloodFIN.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODUDP)
			boaWrite(wp, "%s.DosCfg.sysfloodUDP.checked = true;\n", DOCUMENT);
		if (mode & DOSSYSFLOODICMP)
			boaWrite(wp, "%s.DosCfg.sysfloodICMP.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODSYN)
			boaWrite(wp, "%s.DosCfg.ipfloodSYN.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODFIN)
			boaWrite(wp, "%s.DosCfg.ipfloodFIN.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODUDP)
			boaWrite(wp, "%s.DosCfg.ipfloodUDP.checked = true;\n", DOCUMENT);
		if (mode & DOSIPFLOODICMP)
			boaWrite(wp, "%s.DosCfg.ipfloodICMP.checked = true;\n", DOCUMENT);
		if (mode & DOSTCPUDPPORTSCAN)
			boaWrite(wp, "%s.DosCfg.TCPUDPPortScan.checked = true;\n", DOCUMENT);
		if (mode & DOSPORTSCANSENSI)
			boaWrite(wp, "%s.DosCfg.portscanSensi.value = 1;\n", DOCUMENT);
		if (mode & DOSICMPSMURFENABLED)
			boaWrite(wp, "%s.DosCfg.ICMPSmurfEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSIPLANDENABLED)
			boaWrite(wp, "%s.DosCfg.IPLandEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSIPSPOOFENABLED)
			boaWrite(wp, "%s.DosCfg.IPSpoofEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSIPTEARDROPENABLED)
			boaWrite(wp, "%s.DosCfg.IPTearDropEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSPINTOFDEATHENABLED)
			boaWrite(wp, "%s.DosCfg.PingOfDeathEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSTCPSCANENABLED)
			boaWrite(wp, "%s.DosCfg.TCPScanEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSTCPSYNWITHDATAENABLED)
			boaWrite(wp, "%s.DosCfg.TCPSynWithDataEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSUDPBOMBENABLED)
			boaWrite(wp, "%s.DosCfg.UDPBombEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSUDPECHOCHARGENENABLED)
			boaWrite(wp, "%s.DosCfg.UDPEchoChargenEnabled.checked = true;\n", DOCUMENT);
		if (mode & DOSSOURCEIPBLOCK)
			boaWrite(wp, "%s.DosCfg.sourceIPblock.checked = true;\n", DOCUMENT);

	}
}
#endif

void initSyslogPage(request * wp)
{
	boaWrite(wp, "changelogstatus();");
}

void initDgwPage(request * wp)
{
#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgwip[16];
	unsigned int dgw;
	mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);
	getMIB2Str(MIB_ADSL_WAN_DGW_IP, dgwip);
	boaWrite(wp, "\tdgwstatus = %d;\n", dgw);
	boaWrite(wp, "\tgtwy = '%s';\n", dgwip);
#endif
#ifdef CONFIG_RTL_MULTI_PVC_WAN
	boaWrite(wp, "%s.getElementById('vlan_show').style.display = 'block';\n", DOCUMENT);
#else
	boaWrite(wp, "%s.getElementById('vlan_show').style.display = 'none';\n", DOCUMENT);
#endif

	// Kaohj, differentiate user-level from admin-level
	if (strstr(wp->pathname, "web/admin/"))
		boaWrite(wp, "%s.adsl.add.disabled = true;\n", DOCUMENT);
	else
		boaWrite(wp, "%s.adsl.add.disabled = false;\n", DOCUMENT);
}

#ifdef WEB_ENABLE_PPP_DEBUG
void initPPPSyslog(request * wp)
{
	int enable = 0;
	FILE *fp;

	if (fp = fopen(PPP_SYSLOG, "r")) {
		fscanf(fp, "%d", &enable);
		fclose(fp);
	}
	boaWrite(wp, "%s.formSysLog.pppcap[%d].checked = true;\n", DOCUMENT, enable);
}
#endif

