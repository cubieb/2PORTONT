/*-----------------------------------------------------------------
 * File: boamain.c
 *-----------------------------------------------------------------*/

#include <string.h>
#include "webform.h"
#include "mib.h"
#include "../defs.h"
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
//xl_yue
#include "utility.h"

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
	boaASPDefine("getInfo", getInfo);
	boaASPDefine("check_display", check_display);
	// Kaohj
	boaASPDefine("checkWrite", checkWrite);
	boaASPDefine("initPage", initPage);
#ifdef WLAN_QoS
	boaASPDefine("ShowWmm", ShowWmm);
#endif
	   // add by yq_zhou 1.20
	boaASPDefine("write_wladvanced", write_wladvanced);

/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
/*#ifdef CONFIG_11N_SAGEM_WEB
	boaASPDefine("writeTitle", write_title);
	boaASPDefine("writeLogoBelow", write_logo_below);
#endif*/
#ifdef PORT_FORWARD_GENERAL
	boaASPDefine("portFwList", portFwList);
#endif
#ifdef NATIP_FORWARDING
	boaASPDefine("ipFwList", ipFwList);
#endif
#ifdef IP_PORT_FILTER
	boaASPDefine("ipPortFilterList", ipPortFilterList);
#endif
#ifdef MAC_FILTER
	boaASPDefine("macFilterList", macFilterList);
#endif

	boaASPDefine("atmVcList", atmVcList);
	boaASPDefine("atmVcList2", atmVcList2);
	boaASPDefine("wanConfList", wanConfList);
	boaASPDefine("DHCPSrvStatus", DHCPSrvStatus);
#ifdef CONFIG_RTL8672_SAR
	boaASPDefine("DSLVer", DSLVer);
	boaASPDefine("DSLStatus", DSLStatus);
#endif
#ifdef CONFIG_IPV6
	boaASPDefine("wanip6ConfList", wanip6ConfList);
#endif
	boaASPDefine("getNameServer", getNameServer);
	boaASPDefine("getDefaultGW", getDefaultGW);
#ifdef CONFIG_IPV6
	boaASPDefine("getDefaultGW_ipv6", getDefaultGW_ipv6);
#endif
	boaFormDefine("formTcpipLanSetup", formTcpipLanSetup);	// TCP/IP Lan Setting Form
#ifdef CONFIG_USER_VLAN_ON_LAN
	boaFormDefine("formVLANonLAN", formVLANonLAN);	// formVLANonLAN Setting Form
#endif
	boaASPDefine("lan_setting", lan_setting);
	boaASPDefine("checkIP2", checkIP2);
	boaASPDefine("lanScript", lan_script);
	boaASPDefine("lan_port_mask", lan_port_mask);

#ifdef WEB_REDIRECT_BY_MAC
	boaFormDefine("formLanding", formLanding);
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
	boaFormDefine("formUploadWapiCert1", formUploadWapiCert1);
	boaFormDefine("formUploadWapiCert2", formUploadWapiCert2);
	boaFormDefine("formWapiReKey", formWapiReKey);
#endif


	boaASPDefine("getWanIfDisplay", getWanIfDisplay );
	boaFormDefine("formWanRedirect", formWanRedirect);

//#ifdef CONFIG_USER_PPPOMODEM
	boaASPDefine("wan3GTable", wan3GTable );
//#endif //CONFIG_USER_PPPOMODEM
	boaASPDefine("wanPPTPTable", wanPPTPTable );
	boaASPDefine("wanL2TPTable", wanL2TPTable );
	boaASPDefine("wanIPIPTable", wanIPIPTable );
#ifdef CONFIG_USER_CUPS
	boaASPDefine("printerList", printerList);
#endif
#ifdef CONFIG_USER_PPPOMODEM
	boaASPDefine("fm3g_checkWrite", fm3g_checkWrite);
	boaFormDefine("form3GConf", form3GConf);
#endif //CONFIG_USER_PPPOMODEM

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	boaFormDefine("formStatus_pon", formStatus_pon);
	boaASPDefine("ponGetStatus", ponGetStatus);
#ifdef CONFIG_GPON_FEATURE
	boaASPDefine("showgpon_status", showgpon_status);
	boaASPDefine("fmgpon_checkWrite", fmgpon_checkWrite);
	boaFormDefine("formgponConf", formgponConf);
#endif

#ifdef CONFIG_EPON_FEATURE
	boaASPDefine("showepon_LLID_status", showepon_LLID_status);
	boaASPDefine("showepon_LLID_MAC", showepon_LLID_MAC);
	boaASPDefine("fmepon_checkWrite", fmepon_checkWrite);
	boaFormDefine("formepon_llid_mac_mapping", formepon_llid_mac_mapping);
	boaFormDefine("formeponConf", formeponConf);
#endif
#endif
#ifdef CONFIG_GPON_FEATURE
	boaASPDefine("fmvlan_checkWrite", fmvlan_checkWrite);
	boaFormDefine("formVlan", formVlan);
	boaASPDefine("omciVlanInfo", omciVlanInfo);
	boaASPDefine("showOMCI_OLT_mode", showOMCI_OLT_mode);
	boaASPDefine("fmOmciInfo_checkWrite", fmOmciInfo_checkWrite);
	boaFormDefine("formOmciInfo", formOmciInfo);
#endif

#ifdef _CWMP_MIB_
	boaFormDefine("formTR069Config", formTR069Config);
	boaFormDefine("formTR069CPECert", formTR069CPECert);
	boaFormDefine("formTR069CACert", formTR069CACert);

	boaASPDefine("TR069ConPageShow", TR069ConPageShow);
#endif
	boaASPDefine("portFwTR069", portFwTR069);
#ifdef PORT_FORWARD_GENERAL
	boaFormDefine("formPortFw", formPortFw);					// Firewall Port forwarding Setting Form
#endif
#ifdef NATIP_FORWARDING
	boaFormDefine("formIPFw", formIPFw);					// Firewall NAT IP forwarding Setting Form
#endif
#ifdef PORT_TRIGGERING
	boaFormDefine("formGaming", formGaming);					// Firewall Port Triggering Setting Form
#endif
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER)
	boaFormDefine("formFilter", formFilter);					// Firewall IP, Port, Mac Filter Setting Form
#endif
#ifdef LAYER7_FILTER_SUPPORT
	boaFormDefine("formlayer7", formLayer7);                            //star: for layer7 filter
	boaASPDefine("AppFilterList", AppFilterList);
#endif
#ifdef PARENTAL_CTRL
	boaFormDefine("formParentCtrl", formParentCtrl);
	boaASPDefine("parentControlList", parentalCtrlList);
#endif
#ifdef DMZ
	boaFormDefine("formDMZ", formDMZ);						// Firewall DMZ Setting Form
#endif

	boaFormDefine("formPasswordSetup", formPasswordSetup);	// Management Password Setting Form
	boaFormDefine("formUserPasswordSetup", formUserPasswordSetup);	// Management User Password Setting Form
#ifdef ACCOUNT_CONFIG
	boaFormDefine("formAccountConfig", formAccountConfig);	// Management Account Configuration Setting Form
	boaASPDefine("accountList", accountList);
#endif
#ifdef WEB_UPGRADE
	boaFormDefine("formUpload", formUpload);					// Management Upload Firmware Setting Form
#ifdef CONFIG_DOUBLE_IMAGE
	boaFormDefine("formStopUpload", formStopUpload);				// Management Stop Upload Firmware Setting Form
#endif
#endif
#if 1
	boaFormDefine("formSaveConfig", formSaveConfig);			// Management Upload/Download Configuration Setting Form
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	boaFormDefine("formSnmpConfig", formSnmpConfig);			// SNMP Configuration Setting Form
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_RADVD
	boaFormDefine("formRadvdSetup", formRadvdSetup);			// RADVD Configuration Setting Form
#endif
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	boaFormDefine("formDhcpv6Server", formDhcpv6);			        // DHCPv6 Server Setting Form
	boaASPDefine("showDhcpv6SNameServerTable", showDhcpv6SNameServerTable);     // Name Server List for DHCPv6 Server
	boaASPDefine("showDhcpv6SDOMAINTable", showDhcpv6SDOMAINTable);             // Domian search List for DHCPv6 Server
#endif
#endif
#ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_DSL_VTUO
	boaFormDefine("formSetVTUO", formSetVTUO);				// VTU-O Setting Form
	boaASPDefine("vtuo_checkWrite", vtuo_checkWrite);
#endif /*CONFIG_DSL_VTUO*/
	boaFormDefine("formSetAdsl", formSetAdsl);				// ADSL Driver Setting Form
	boaFormDefine("formStatAdsl", formStatAdsl);			// ADSL Statistics Form
#ifdef FIELD_TRY_SAFE_MODE
	boaFormDefine("formAdslSafeMode", formAdslSafeMode);		// ADSL Safe mode Setting Form
#endif
	boaFormDefine("formDiagAdsl", formDiagAdsl);			// ADSL Driver Diag Form
#endif
	boaFormDefine("formStats", formStats);				// Packet Statistics Form
	boaFormDefine("formRconfig", formRconfig);				// Remote Configuration Form
	boaFormDefine("formSysCmd",  formSysCmd);				// hidden page for system command
	boaASPDefine("sysCmdLog", sysCmdLog);
#ifdef CONFIG_USER_RTK_SYSLOG
	boaFormDefine("formSysLog",  formSysLog);				// hidden page for system command
	boaASPDefine("sysLogList", sysLogList);			// Web Log
	boaASPDefine("RemoteSyslog", RemoteSyslog); // Jenny, for remote system log
#endif
#ifdef CONFIG_DEV_xDSL
	boaFormDefine("formSetAdslTone", formSetAdslTone);		// ADSL Driver Setting Tone Form
	boaFormDefine("formSetAdslPSD", formSetAdslPSD);		// ADSL Driver Setting Tone Form
	boaASPDefine("adslToneDiagTbl", adslToneDiagTbl);
	boaASPDefine("adslToneDiagList", adslToneDiagList);
	boaASPDefine("adslToneConfDiagList", adslToneConfDiagList);
	boaASPDefine("adslPSDMaskTbl", adslPSDMaskTbl);
	boaASPDefine("adslPSDMeasure", adslPSDMeasure);
//#ifdef CONFIG_VDSL
	boaASPDefine("vdslBandStatusTbl", vdslBandStatusTbl);
//#endif /*CONFIG_VDSL*/
#endif
	boaASPDefine("pktStatsList", pktStatsList);

	boaFormDefine("formStatus", formStatus);				// Status Form
#if defined(CONFIG_RTL_8676HWNAT)
	boaFormDefine("formLANPortStatus", formLANPortStatus);	// formLANPortStatus Form
	boaASPDefine("showLANPortStatus", showLANPortStatus);
#endif
#ifdef CONFIG_IPV6
	boaFormDefine("formStatus_ipv6", formStatus_ipv6);
#endif
	boaFormDefine("formStatusModify", formDate);

	boaFormDefine("formWanMode", formWanMode);
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
	boaFormDefine("formWanEth", formEth);					// ADSL Configuration Setting Form
#endif
#ifdef CONFIG_RTL8672_SAR
	boaFormDefine("formWanAtm", formAtmVc);					// Atm VC Configuration Setting Form
	boaFormDefine("formWanAdsl", formAdsl);					// ADSL Configuration Setting Form
#endif
	boaFormDefine("formPPPEdit", formPPPEdit);				// PPP interface Configuration Form
	boaFormDefine("formIPEdit", formIPEdit);				// IP interface Configuration Form
	boaFormDefine("formBrEdit", formBrEdit);				// Bridged interface Configuration Form

	boaFormDefine("formBridging", formBridge);				// Bridge Configuration Form
	boaASPDefine("bridgeFdbList", bridgeFdbList);
	boaASPDefine("ARPTableList", ARPTableList);
	boaFormDefine("formRefleshFdbTbl", formRefleshFdbTbl);

#ifdef ROUTING
	boaFormDefine("formRouting", formRoute);				// Routing Configuration Form
	boaASPDefine("showStaticRoute", showStaticRoute);
#endif
#ifdef CONFIG_IPV6
	boaFormDefine("formIPv6EnableDisable", formIPv6EnableDisable);		// IPv6 Enable/Disable Configuration Form
	boaFormDefine("formIPv6Routing", formIPv6Route);		// Routing Configuration Form
	boaASPDefine("showIPv6StaticRoute", showIPv6StaticRoute);
	boaFormDefine("formIPv6RefleshRouteTbl", formRefleshRouteTbl);
	boaASPDefine("routeIPv6List", routeIPv6List);
#ifdef IP_PORT_FILTER
	boaFormDefine("formFilterV6", formFilterV6);					// Firewall IPv6, Port, Setting Form
	boaASPDefine("ipPortFilterListV6", ipPortFilterListV6);
#endif
#endif

	boaASPDefine("ShowDefaultGateway", ShowDefaultGateway);	// Jenny, for DEFAULT_GATEWAY_V2
	boaASPDefine("GetDefaultGateway", GetDefaultGateway);
	boaASPDefine("DisplayDGW", DisplayDGW);
	boaASPDefine("DisplayTR069WAN", DisplayTR069WAN);
	boaFormDefine("formRefleshRouteTbl", formRefleshRouteTbl);
	boaASPDefine("routeList", routeList);

#ifdef CONFIG_USER_DHCP_SERVER
	boaFormDefine("formDhcpServer", formDhcpd);				// DHCP Server Setting Form
	boaASPDefine("dhcpClientList", dhcpClientList);
	boaFormDefine("formReflashClientTbl", formReflashClientTbl);
#endif

#ifdef CONFIG_USER_DDNS
	boaFormDefine("formDDNS", formDDNS);
	boaASPDefine("showDNSTable", showDNSTable);
#endif
#ifdef CONFIG_USER_DHCP_SERVER
	boaFormDefine("formDhcrelay", formDhcrelay);			// DHCPRelay Configuration Form
#endif
	boaFormDefine("formPing", formPing);					// Ping diagnostic Form
#ifdef CONFIG_USER_TCPDUMP_WEB
	boaFormDefine("formCapture", formCapture);				// Packet Capture
#endif
#ifdef CONFIG_DEV_xDSL
	boaFormDefine("formOAMLB", formOamLb);					// OAM Loopback diagnostic Form
#endif
#ifdef ADDRESS_MAPPING
	boaFormDefine("formAddressMap", formAddressMap);		// AddressMap Configuration Form
#ifdef MULTI_ADDRESS_MAPPING
	boaASPDefine("AddressMapList", showMultAddrMappingTable);
#endif //MULTI_ADDRESS_MAPPING
#endif

#ifdef FINISH_MAINTENANCE_SUPPORT
	boaFormDefine("formFinishMaintenance", formFinishMaintenance);		// xl_yue added,inform itms
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
	boaFormDefine("formAdminLogout", formAdminLogout);		// xl_yue added,
	boaFormDefine("formUserLogout", formUserLogout);		// xl_yue added,
#endif
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
	boaFormDefine("formLogin", formLogin);					// xl_yue added,
	boaFormDefine("formLogout", formLogout);				// xl_yue added,
	// Kaohj
	boaASPDefine("passwd2xmit", passwd2xmit);
	// davian_kuo
	boaFormDefine("formLoginMultilang", formLoginMultilang);
#endif

	boaFormDefine("formReboot", formReboot);				// Commit/reboot Form
#ifdef CONFIG_USER_DHCP_SERVER
	boaFormDefine("formDhcpMode", formDhcpMode);			// DHCP Mode Configuration Form
#endif

#ifdef CONFIG_USER_IGMPPROXY
	boaFormDefine("formIgmproxy", formIgmproxy);			// IGMP Configuration Form
#endif
	boaASPDefine("if_wan_list", ifwanList);
#ifdef QOS_TRAFFIC_SHAPING_BY_SSID
	boaASPDefine("ssid_list", ssid_list);
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	boaFormDefine("formUpnp", formUpnp);					// UPNP Configuration Form
#endif
// Mason Yu. MLD Proxy
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH
	boaFormDefine("formMLDProxy", formMLDProxy);			// MLD Proxy Configuration Form
#endif
	boaFormDefine("formMLDSnooping", formMLDSnooping);		// formIgmpSnooping Configuration Form  // Mason Yu. MLD snooping for e8b
#endif
#ifdef CONFIG_USER_MINIDLNA
	boaASPDefine("fmDMS_checkWrite", fmDMS_checkWrite);
	boaFormDefine("formDMSConf", formDMSConf);
#endif
#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
	boaFormDefine("formRip", formRip);						// RIP Configuration Form
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
	boaASPDefine("showRipIf", showRipIf);
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	boaASPDefine("showOspfIf", showOspfIf);
#endif

#ifdef IP_ACL
	boaFormDefine("formACL", formACL);                  	// ACL Configuration Form
	boaASPDefine("showACLTable", showACLTable);
	boaASPDefine("showLANACLItem", LANACLItem);
	boaASPDefine("showWANACLItem", WANACLItem);
#endif
#ifdef NAT_CONN_LIMIT
	boaFormDefine("formConnlimit", formConnlimit);
	boaASPDefine("showConnLimitTable", showConnLimitTable);
#endif
#ifdef TCP_UDP_CONN_LIMIT
	boaFormDefine("formConnlimit", formConnlimit);
	boaASPDefine("connlmitList", showConnLimitTable);

#endif //TCP_UDP_CONN_LIMIT
#ifdef CONFIG_USER_DHCP_SERVER
	boaFormDefine("formmacBase", formmacBase); 		// MAC-Based Assignment for DHCP Server
#ifdef IMAGENIO_IPTV_SUPPORT
	boaFormDefine("formIpRange", formIpRange);
#endif
	boaASPDefine("showMACBaseTable", showMACBaseTable);
#ifdef IMAGENIO_IPTV_SUPPORT
	boaASPDefine("showDeviceIpTable", showDeviceIpTable);
#endif //#ifdef IMAGENIO_IPTV_SUPPORT
#endif

#ifdef URL_BLOCKING_SUPPORT
	boaFormDefine("formURL", formURL);                  // URL Configuration Form
	boaASPDefine("showURLTable", showURLTable);
	boaASPDefine("showKeywdTable", showKeywdTable);
#endif
#ifdef URL_ALLOWING_SUPPORT
       boaASPDefine("showURLALLOWTable", showURLALLOWTable);
#endif



#ifdef DOMAIN_BLOCKING_SUPPORT
	boaFormDefine("formDOMAINBLK", formDOMAINBLK);                  // Domain Blocking Configuration Form
	boaASPDefine("showDOMAINBLKTable", showDOMAINBLKTable);
#endif

#ifdef TIME_ZONE
	boaFormDefine("formNtp", formNtp);
	boaASPDefine("timeZoneList", timeZoneList);
#endif

#ifdef IP_PASSTHROUGH
	boaFormDefine("formOthers", formOthers);	// Others advance Configuration Form
#endif

#ifdef CONFIG_USER_BRIDGE_GROUPING
	boaFormDefine("formBridgeGrouping", formBridgeGrouping);	// Interface grouping Form
	boaASPDefine("itfGrpList", ifGroupList);
#endif
	//boaFormDefine("formVlanCfg", formVlan);	// Vlan Configuration Form
	//boaASPDefine("vlanPost", vlanPost);
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
	boaFormDefine("formLinkMode", formLink);	// Ethernet Link Form
	boaASPDefine("show_lan", show_lanport);
#endif
#else // of CONFIG_RTL_MULTI_LAN_DEV
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	boaFormDefine("formLinkMode", formLink);
#endif
#endif	// of CONFIG_RTL_MULTI_LAN_DEV
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	boaFormDefine("formIPQoS", formQos);	// IP QoS Form
	boaASPDefine("dft_qos", default_qos);
	boaASPDefine("ipQosList", qosList);
#ifdef QOS_DIFFSERV
	boaFormDefine("formDiffServ", formDiffServ);
	boaASPDefine("diffservList", diffservList);
#endif
	boaASPDefine("pq_egress", priority_outif);
	boaASPDefine("mark_dscp", confDscp);
#ifdef QOS_DSCP_MATCH
	boaASPDefine("match_dscp", match_dscp);
#endif
	boaFormDefine("formQueueAdd", formQueueAdd);
	boaASPDefine("ipQosQueueList", ipQosQueueList);
#endif
#ifdef NEW_IP_QOS_SUPPORT
	boaFormDefine("formQosShape", formQosShape);
	boaASPDefine("initTraffictlPage",initTraffictlPage);
#endif
#ifdef CONFIG_8021P_PRIO
       boaASPDefine("setting1p", setting_1ppriority);
#ifdef NEW_IP_QOS_SUPPORT
       boaASPDefine("settingpred", setting_predprio);
#endif
#endif
	boaASPDefine("if_lan_list", iflanList);
#ifdef IP_QOS
	boaASPDefine("pr_egress", policy_route_outif);
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	boaFormDefine("formPPtP", formPPtP);
	boaASPDefine("pptpList", pptpList);
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_PPTPD_PPTPD
	boaASPDefine("pptpServerList", pptpServerList);
#endif // end of CONFIG_USER_PPTPD_PPTPD
#ifdef CONFIG_USER_L2TPD_L2TPD
	boaFormDefine("formL2TP", formL2TP);
	boaASPDefine("l2tpList", l2tpList);
#endif //end of CONFIG_USER_L2TPD_L2TPD
#ifdef CONFIG_USER_L2TPD_LNS
	boaASPDefine("l2tpServerList", l2tpServerList);
#endif // end of CONFIG_USER_L2TPD_LNS
#ifdef CONFIG_XFRM
	boaFormDefine("formIPsec", formIPsec);
	boaASPDefine("ipsec_wanList", ipsec_wanList);
	boaASPDefine("ipsec_ikePropList", ipsec_ikePropList);
	boaASPDefine("ipsec_saPropList", ipsec_saPropList);
	boaASPDefine("ipsec_infoList", ipsec_infoList);
#endif
#ifdef CONFIG_NET_IPIP
	boaFormDefine("formIPIP", formIPIP);
	boaASPDefine("ipipList", ipipList);
#endif//end of CONFIG_NET_IPIP

#ifdef REMOTE_ACCESS_CTL
	boaFormDefine("formSAC", formAcc);	// Services Access Control
	boaASPDefine("accPost", accPost);
	boaASPDefine("rmtaccItem", accItem);
#endif
	//boaASPDefine("autopvcStatus", autopvcStatus);	// auto-pvc search
	//boaASPDefine("showPVCList", showPVCList);	// auto-pvc search
	boaASPDefine("ShowAutoPVC", ShowAutoPVC);	// auto-pvc search
	boaASPDefine("ShowChannelMode", ShowChannelMode);	// China Telecom jim...
	boaASPDefine("ShowBridgeMode", ShowBridgeMode);	// For PPPoE pass through
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
	boaASPDefine("initPageWaneth", initPageWaneth);
#endif
	boaASPDefine("ShowPPPIPSettings", ShowPPPIPSettings);	// China Telecom jim...
	boaASPDefine("ShowNAPTSetting", ShowNAPTSetting);	// China Telecom jim...
#ifdef CONFIG_USER_RTK_WAN_CTYPE
	boaASPDefine("ShowConnectionType", ShowConnectionType);
#endif
	boaASPDefine("ShowIpProtocolType", ShowIpProtocolType);
	boaASPDefine("ShowIPV6Settings", ShowIPV6Settings);
	boaASPDefine("ShowDSLiteSetting", ShowDSLiteSetting);
	boaASPDefine("ShowPortMapping", ShowPortMapping);
	boaASPDefine("ShowPortBaseFiltering", ShowPortBaseFiltering);
    boaASPDefine("Show6rdSetting", Show6rdSetting);
#ifdef WLAN_WISP
	boaASPDefine("ShowWispWanItf", ShowWispWanItf);
	boaASPDefine("initWispWanItfStatus", initWispWanItfStatus);
#endif

	boaASPDefine("ShowIGMPSetting", ShowIGMPSetting);

#ifdef CONFIG_USER_IP_QOS_3
	boaFormDefine("formQosPolicy", formQosPolicy);
	boaASPDefine("initQueuePolicy", initQueuePolicy);
	boaFormDefine("formQosRule",formQosRule);
	boaFormDefine("formQosRuleEdit",formQosRuleEdit);
	boaASPDefine("initQosRulePage",initQosRulePage);
	boaASPDefine("initRulePriority",initRulePriority);
	boaASPDefine("initQosLanif",initQosLanif);
	boaASPDefine("initOutif",initOutif);
	boaASPDefine("initTraffictlPage",initTraffictlPage);
	boaFormDefine("formQosTraffictl",formQosTraffictl);
	boaFormDefine("formQosTraffictlEdit",formQosTraffictlEdit);
	boaASPDefine("ifWanList_tc", ifWanList_tc);
	boaASPDefine("ipqos_dhcpopt", ipqos_dhcpopt);
	boaASPDefine("ipqos_dhcpopt_getoption60", ipqos_dhcpopt_getoption60);
#endif

	boaASPDefine("createMenu", createMenu);
	boaASPDefine("createMenu_user", createMenu_user);
#ifdef WLAN_SUPPORT
	//for WLAN enable/disable web control
	boaASPDefine("wlanStatus", wlanStatus);
	boaASPDefine("SSID_select", wlan_ssid_select);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	boaFormDefine("formWlanRedirect", formWlanRedirect);
#endif
#if defined(CONFIG_RTL_92D_SUPPORT)
	boaFormDefine("formWlanBand2G5G", formWlanBand2G5G);
#endif //CONFIG_RTL_92D_SUPPORT
	boaFormDefine("formWlanSetup", formWlanSetup);
#ifdef WLAN_ACL
	boaASPDefine("wlAcList", wlAcList);
#endif
	boaASPDefine("wirelessClientList", wirelessClientList);
	boaFormDefine("formWirelessTbl", formWirelessTbl);

#ifdef WLAN_ACL
	boaFormDefine("formWlAc", formWlAc);
#endif
	boaFormDefine("formAdvanceSetup", formAdvanceSetup);

#ifdef WLAN_WPA
	boaFormDefine("formWlEncrypt", formWlEncrypt);
#endif
#ifdef WLAN_WDS
	boaFormDefine("formWlWds", formWlWds);
	boaFormDefine("formWdsEncrypt", formWdsEncrypt);
	boaASPDefine("wlWdsList", wlWdsList);
	boaASPDefine("wdsList", wdsList);
#endif
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
	boaFormDefine("formWlSiteSurvey", formWlSiteSurvey);
	boaASPDefine("wlSiteSurveyTbl",wlSiteSurveyTbl);
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
	boaFormDefine("formWsc", formWsc);
#endif
#ifdef WLAN_11R
	boaFormDefine("formFt", formFt);
	boaASPDefine("wlFtKhList", wlFtKhList);
#endif
	boaASPDefine("wlStatus_parm", wlStatus_parm);
#endif	// of WLAN_SUPPORT

#ifdef CONFIG_DEV_xDSL
	boaASPDefine("oamSelectList", oamSelectList);
#endif
#ifdef DIAGNOSTIC_TEST
	boaFormDefine("formDiagTest", formDiagTest);	// Diagnostic test
	boaASPDefine("lanTest", lanTest);	// Ethernet LAN connection test
	boaASPDefine("adslTest", adslTest);	// ADSL service provider connection test
	boaASPDefine("internetTest", internetTest);	// Internet service provider connection test
#endif
#ifdef DOS_SUPPORT
	boaFormDefine("formDosCfg", formDosCfg);
#endif
#ifdef WLAN_MBSSID
	boaASPDefine("SSIDStr", SSIDStr);
	boaASPDefine("getVirtualIndex", getVirtualIndex);
	boaFormDefine("formWlanMultipleAP", formWlanMultipleAP);
	boaASPDefine("wirelessVAPClientList", wirelessVAPClientList);
	boaFormDefine("formWirelessVAPTbl", formWirelessVAPTbl);
#endif

#ifdef PORT_FORWARD_ADVANCE
	boaFormDefine("formPFWAdvance", formPFWAdvance);
	boaASPDefine("showPFWAdvTable", showPFWAdvTable);
#endif
#ifdef PORT_FORWARD_GENERAL
	boaASPDefine("showPFWAdvForm", showPFWAdvForm);
#endif

/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
	boaFormDefineUserMgmt();
#endif
//add by ramen for ALG on-off
#ifdef CONFIG_IP_NF_ALG_ONOFF
	boaFormDefine("formALGOnOff", formALGOnOff);
#endif
/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
	web_voip_init();
#endif /*VOIP_SUPPORT*/
/*-----end-----*/
#ifdef WEB_ENABLE_PPP_DEBUG
boaASPDefine("ShowPPPSyslog", ShowPPPSyslog); // Jenny, show PPP debug to syslog
#endif
#ifdef CONFIG_DEV_xDSL
	boaASPDefine("DSLuptime", DSLuptime);
#ifdef CONFIG_USER_XDSL_SLAVE
	boaASPDefine("DSLSlvuptime", DSLSlvuptime);
#endif /*CONFIG_USER_XDSL_SLAVE*/
#endif
	boaASPDefine("multilang", multilang_asp);
	boaASPDefine("WANConditions", WANConditions);
	boaASPDefine("ShowWanMode", ShowWanMode);

#ifdef CONFIG_USER_SAMBA
	boaFormDefine("formSamba", formSamba);
#endif
#ifdef CONFIG_INIT_SCRIPTS
	boaFormDefine("formInitStartScript", formInitStartScript);
	boaFormDefine("formInitStartScriptDel", formInitStartScriptDel);
	boaFormDefine("formInitEndScript", formInitEndScript);
	boaFormDefine("formInitEndScriptDel", formInitEndScriptDel);
#endif
#ifdef CONFIG_USER_DOT1AG_UTILS
	boaASPDefine("dot1ag_init", dot1ag_init);
	boaFormDefine("formDot1agConf", formDot1agConf);
	boaFormDefine("formDot1agAction", formDot1agAction);
	boaFormDefine("dot1agActionRefresh", dot1agActionRefresh);
	boaASPDefine("dot1ag_status_init", dot1ag_status_init);
#endif

#ifdef OSGI_SUPPORT
	boaASPDefine("getOSGIInfo", getOSGIInfo);
	boaASPDefine("getOSGIBundleList", getOSGIBundleList);
	boaFormDefine("formOsgiUpload", formOsgiUpload);
	boaFormDefine("formOsgiMgt", formOsgiMgt);
#endif

#ifdef CONFIG_USER_Y1731
	boaFormDefine("formY1731", formY1731);
#endif
	boaFormDefine("formVersionMod",formVersionMod);
	boaFormDefine("formExportOMCIlog", formExportOMCIlog);
	boaFormDefine("formImportOMCIShell", formImportOMCIShell);

#ifdef USER_WEB_WIZARD
	boaFormDefine("form2WebWizardMenu", formWebWizardMenu);
	boaFormDefine("form2WebWizard1", formWebWizard1);
	boaFormDefine("form2WebWizard4", formWebWizard4);
	boaFormDefine("form2WebWizard5", formWebWizard5);
	boaFormDefine("form2WebWizard6", formWebWizard6);
	boaASPDefine("ShowWebWizardPage", ShowWebWizardPage);
#endif
	// Added by davian kuo
	boaFormDefine("langSel", langSel);
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
	int i;
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

#ifdef CONFIG_SECONDARY_IP
	mib_get(MIB_LAN_PORT_MASK1, (void *)&port_mask);
	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3 && i<SW_LAN_PORT_NUM; i++){
		if(port_mask & (1 << i))
			boaWrite(wp, "%s.tcpip.chk_port_mask1[%d].checked = true;\n", DOCUMENT, i);
	}
#endif
#if 0
	if(port_mask & (1<<RTK_RG_MAC_PORT0))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[0].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT1))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[1].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT2))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[2].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT3))
		boaWrite(wp, "%s.tcpip.chk_port_mask1[3].checked = true;\n", DOCUMENT);
#endif

	mib_get(MIB_LAN_IP_VERSION1, (void *)&vChar);
	if(vChar==2) //remove the LAN ipv6 only option in UI, so index changed
		boaWrite(wp, "%s.tcpip.ip_version1.options[1].selected = true;\n", DOCUMENT);
	else
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
	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3 && i<SW_LAN_PORT_NUM; i++){
		if(port_mask & (1 << i))
			boaWrite(wp, "%s.tcpip.chk_port_mask2[%d].checked = true;\n", DOCUMENT, i);
	}

#if 0
	if(port_mask & (1<<RTK_RG_MAC_PORT0))
		boaWrite(wp, "%s.tcpip.chk_port_mask2[0].checked = true;\n", DOCUMENT);

	if(port_mask & (1<<RTK_RG_MAC_PORT1))
		boaWrite(wp, "%s.tcpip.chk_port_mask2[1].checked = true;\n", DOCUMENT);

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

// Mason Yu. MLD snooping
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
	boaWrite(wp, "	<th align=left><font size=2>ADSL %s:</th>\n", multilang(LANG_TONE_MASK) );
	boaWrite(wp, "	<td></td>\n");
	boaWrite(wp, "</tr>\n");
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th></th>\n");
	boaWrite(wp, "	<td><font size=2><input type=\"button\" value=\"%s\" name=\"adsltoneTbl\" onClick=\"adsltoneClick('/adsltone.asp')\"></td>\n", multilang(LANG_TONE_MASK) );
	boaWrite(wp, "</tr>\n");
#endif /*CONFIG_VDSL*/
}


void init_adsl_psd_mask(request * wp)
{
#ifndef CONFIG_VDSL
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th align=left><font size=2>ADSL %s:</th>\n", multilang(LANG_PSD_MASK) );
	boaWrite(wp, "	<td></td>\n");
	boaWrite(wp, "</tr>\n");
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th></th>\n");
	boaWrite(wp, "	<td><font size=2><input type=\"button\" value=\"%s\" name=\"adslPSDMaskTbl\" onClick=\"adsltoneClick('/adslpsd.asp')\"></td>\n", multilang(LANG_PSD_MASK) );
	boaWrite(wp, "</tr>\n");
#endif /*CONFIG_VDSL*/
}


void init_psd_msm_mode(request * wp)
{
#ifndef CONFIG_VDSL
#if SUPPORT_TR105
	boaWrite(wp, "<tr>\n");
	boaWrite(wp, "	<th align=left><font size=2>%s:</th>\n", multilang(LANG_PSD_MEASUREMENT_MODE) );
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
	multilang(LANG_NAME_1), multilang(LANG_IP_ADDRESS), multilang(LANG_PORT_TO_OPEN),
	multilang(LANG_PORT_TO_OPEN), multilang(LANG_ENABLE), multilang(LANG_ACTION));

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
		multilang(LANG_PORT_TRIGGERING), multilang(LANG_CONFIGURATION));
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
	multilang(LANG_NAME_1), multilang(LANG_IP_ADDRESS), multilang(LANG_PORT_TO_OPEN),
	multilang(LANG_PORT_TO_OPEN), multilang(LANG_ENABLE));

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
	found ? (Entry.enable == 1 ? multilang(LANG_CHECKED) :"") : "",
	(strlen(idx)==0)?"-1":idx
	);

	boaWrite(wp,
	"<input type=submit value=%s name=add onClick=\"return addClick()\">&nbsp;&nbsp;&nbsp;&nbsp;" \
	"<input type=submit value=%s name=modify onClick=\"return addClick()\">&nbsp;&nbsp;&nbsp;&nbsp;" \
	"<input type=reset value=%s><BR><BR>\n",
	multilang(LANG_ADD), multilang(LANG_MODIFY), multilang(LANG_RESET));
	boaWrite(wp,
	"<input type=hidden value=/gaming.asp name=submit-url>");

	boaWrite(wp,
	"<b>%s</b>\n" \
//	"<input type=hidden name=ms value=%d>\n" \
/*	"onSubmit=\"return checkRange();\"" \ */
	"<table cellSpacing=1 cellPadding=2 border=0>\n", multilang(LANG_GAME_RULES_LIST));

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

#ifdef IP_PASSTHROUGH
void initOthersPage(request * wp)
{
	unsigned int vInt;
	unsigned char vChar;

	mib_get( MIB_IPPT_ITF, (void *)&vInt);
	//if (vInt == 0xff) {
	if (vInt == DUMMY_IFINDEX) {
		boaWrite(wp, "%s.others.ltime.disabled = true;\n", DOCUMENT);
		boaWrite(wp, "%s.others.lan_acc.disabled = true;\n", DOCUMENT);
	}

	mib_get( MIB_IPPT_LANACC, (void *)&vChar);
	if (vChar == 1)
		boaWrite(wp, "%s.others.lan_acc.checked = true\n", DOCUMENT);
}
#endif

#ifdef TIME_ZONE
void initNtpPage(request * wp)
{
	unsigned char vChar = 1;

	mib_get(MIB_DST_ENABLED, (void *)&vChar);
	if (vChar == 1)
		boaWrite(wp, "%s.time.dst_enabled.checked = true;\n", DOCUMENT);

	mib_get(MIB_NTP_ENABLED, (void *)&vChar);
	if (vChar == 1)
		boaWrite(wp, "%s.time.enabled.checked = true;\n", DOCUMENT);

	mib_get(MIB_NTP_SERVER_ID, (void *)&vChar);
	boaWrite(wp, "%s.time.ntpServerId[%d].checked = true;\n", DOCUMENT,
		 vChar & 0x01);
}
#endif

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
	//boaWrite(wp, "%s.getElementById(\"repeater_SSID\").style.display = \"\";\n", DOCUMENT);
#endif
	if (Entry.wlanDisabled!=0)
		// hidden type
		boaWrite(wp, "%s.wlanSetup.wlanDisabled.value = \"ON\";\n", DOCUMENT);
		// checkbox type
		//boaWrite(wp, "%s.wlanSetup.wlanDisabled.checked = true;\n", DOCUMENT);
		boaWrite(wp, "%s.wlanSetup.band.value = %d;\n", DOCUMENT, Entry.wlanBand-1);
#if defined (WLAN_11N_COEXIST)
	unsigned char phyband;
	mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);
	if(phyband == PHYBAND_5G) {
		mib_get( MIB_WLAN_CHANNEL_WIDTH,(void *)&vChar);
		boaWrite(wp, "%s.wlanSetup.chanwid.value = %d;\n", DOCUMENT, vChar);
	}
	else{
		mib_get( MIB_WLAN_11N_COEXIST,(void *)&vChar);
		if(vChar!=0)
			vChar = 3;
		else
			mib_get( MIB_WLAN_CHANNEL_WIDTH,(void *)&vChar);
		boaWrite(wp, "%s.wlanSetup.chanwid.value = %d;\n", DOCUMENT, vChar);
	}
#else
	mib_get( MIB_WLAN_CHANNEL_WIDTH,(void *)&vChar);
		boaWrite(wp, "%s.wlanSetup.chanwid.value = %d;\n", DOCUMENT, vChar);
#endif
	mib_get( MIB_WLAN_CONTROL_BAND,(void *)&vChar);
		boaWrite(wp, "%s.wlanSetup.ctlband.value = %d;\n", DOCUMENT, vChar);
	mib_get( MIB_TX_POWER, (void *)&vChar);
	boaWrite(wp, "%s.wlanSetup.txpower.selectedIndex = %d;\n", DOCUMENT, vChar);
	mib_get( MIB_WIFI_REGDOMAIN_DEMO, (void *)&vChar);
	if(vChar != 0)
	{
		boaWrite(wp, "%s.getElementById(\"wifi_regdomain_demo\").style.display = \"\";\n", DOCUMENT);
		mib_get( MIB_HW_REG_DOMAIN, (void *)&vChar);
		boaWrite(wp, "%s.wlanSetup.regdomain_demo.selectedIndex = %d;\n", DOCUMENT, (vChar-1));
	}

#ifdef WLAN_LIMITED_STA_NUM
		boaWrite(wp, "%s.getElementById(\"wl_limit_stanum\").style.display = \"\";\n", DOCUMENT);
		boaWrite(wp, "%s.wlanSetup.wl_limitstanum.selectedIndex = %d;\n", DOCUMENT, Entry.stanum>0? 1:0);
		if(Entry.stanum)
			boaWrite(wp, "%s.wlanSetup.wl_stanum.value = %d;\n", DOCUMENT, Entry.stanum);
		else
			boaWrite(wp, "%s.wlanSetup.wl_stanum.disabled = true;\n", DOCUMENT);
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
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
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
	#ifndef CONFIG_RTK_RG_INIT
	boaWrite(wp, "%s.WlanMBSSID.mbssid_block[%d].checked = true;\n", DOCUMENT, vChar);
	#endif
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
	#ifndef CONFIG_RTK_RG_INIT
	boaWrite(wp, "%s.MultipleAP.mbssid_block[%d].checked = true;\n", DOCUMENT, vChar);
	#endif
	for (i=1; i<=4; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry)) {
  			printf("Error! Get MIB_MBSSIB_TBL(initWLMultiApPage) error.\n");
  			return;
		}
		boaWrite(wp, "%s.MultipleAP.elements[\"wl_hide_ssid%d\"].selectedIndex = %d;\n", DOCUMENT, i, Entry.hidessid?0:1);
		boaWrite(wp, "%s.MultipleAP.elements[\"wl_access%d\"].selectedIndex = %d;\n", DOCUMENT, i, Entry.userisolation);
#ifdef WLAN_LIMITED_STA_NUM
		boaWrite(wp, "%s.getElementById(\"wl_limit_stanum_div%d\").style.display = \"\";\n", DOCUMENT, i);
		boaWrite(wp, "%s.MultipleAP.elements[\"wl_limitstanum%d\"].selectedIndex = %d;\n", DOCUMENT, i, Entry.stanum>0?1:0);
		if(Entry.stanum)
			boaWrite(wp, "%s.MultipleAP.elements[\"wl_stanum%d\"].value = %d;\n", DOCUMENT, i, Entry.stanum);
#endif		
	}
#ifdef WLAN_LIMITED_STA_NUM
	boaWrite(wp, "%s.getElementById(\"wl_limit_stanum_div\").style.display = \"\";\n", DOCUMENT);
#endif
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

#ifdef WLAN_11R
void initWlFtPage(request * wp)
{
	MIB_CE_MBSSIB_T Entry;
	char strbuf[MAX_PSK_LEN+1], strbuf2[20];
	int isNmode;
	int i, k;
	char wlanDisabled;

	k=0;
	for (i=0; i<=NUM_VWLAN_INTERFACE; i++) {
		wlan_getEntry(&Entry, i);
		if (i==0) // root
			wlanDisabled = Entry.wlanDisabled;
		if (Entry.wlanDisabled)
			continue;
		if (Entry.wlanMode == CLIENT_MODE)
			continue;
		wlan_ssid_helper(&Entry, strbuf, strbuf2);
		boaWrite(wp, "\t_encrypt[%d]=%d;\n", k, Entry.encrypt);

		boaWrite(wp, "\t_ft_enable[%d]=%d;\n", k, Entry.ft_enable);
		boaWrite(wp, "\t_ft_mdid[%d]=\"%s\";\n", k, Entry.ft_mdid);
		boaWrite(wp, "\t_ft_over_ds[%d]=%d;\n", k, Entry.ft_over_ds);
		boaWrite(wp, "\t_ft_res_request[%d]=%d;\n", k, Entry.ft_res_request);
		boaWrite(wp, "\t_ft_r0key_timeout[%d]=%d;\n", k, Entry.ft_r0key_timeout);
		boaWrite(wp, "\t_ft_reasoc_timeout[%d]=%d;\n", k, Entry.ft_reasoc_timeout);
		boaWrite(wp, "\t_ft_r0kh_id[%d]=\"%s\";\n", k, Entry.ft_r0kh_id);
		boaWrite(wp, "\t_ft_push[%d]=%d;\n", k, Entry.ft_push);
		boaWrite(wp, "\t_ft_kh_num[%d]=%d;\n", k, Entry.ft_kh_num);
		k++;
	}
	boaWrite(wp, "\tssid_num=%d;\n", k);

	if(wlanDisabled) {
		boaWrite(wp, "\t%s.getElementById(\"wlan_dot11r_table\").style.display = 'none';\n", DOCUMENT);
		boaWrite(wp, "\t%s.write(\"<font size=2> WLAN Disabled !</font>\")", DOCUMENT);
	}
	else {
		boaWrite(wp, "\t%s.getElementById(\"wlan_dot11r_table\").style.display = \"\";\n", DOCUMENT);
	}
}
#endif

void initWlWpaMbssidPage(request * wp)
{
	MIB_CE_MBSSIB_T Entry;
	char strbuf[MAX_PSK_LEN+1], strbuf2[20];
	int isNmode;
	int i, k;
	char wlanDisabled;

	k=0;
	for (i=0; i<=NUM_VWLAN_INTERFACE; i++) {
		wlan_getEntry(&Entry, i);
		if (i==0) // root
			wlanDisabled = Entry.wlanDisabled;
		if (Entry.wlanDisabled)
			continue;
		if (Entry.wlanMode == CLIENT_MODE)
			continue;
		wlan_ssid_helper(&Entry, strbuf, strbuf2);
		boaWrite(wp, "_wlan_mode[%d]=%d;\n", k, Entry.wlanMode);
		boaWrite(wp, "\t_encrypt[%d]=%d;\n", k, Entry.encrypt);
		boaWrite(wp, "\t_enable1X[%d]=%d;\n", k, Entry.enable1X);
		boaWrite(wp, "\t_wpaAuth[%d]=%d;\n", k, Entry.wpaAuth);
		boaWrite(wp, "\t_wpaPSKFormat[%d]=%d;\n", k, Entry.wpaPSKFormat);
		//boaWrite(wp, "\t_wpaPSK[%d]='%s';\n", k, strbuf);
		boaWrite(wp, "\t_wpaPSK[%d]='%s';\n", k, Entry.wpaPSK);	//fix web check psk-key invalid problem
		boaWrite(wp, "\t_wpaGroupRekeyTime[%d]='%lu';\n", k, Entry.wpaGroupRekeyTime);
		boaWrite(wp, "\t_rsPort[%d]=%d;\n", k, Entry.rsPort);
		boaWrite(wp, "\t_rsIpAddr[%d]='%s';\n", k, strbuf2);
		boaWrite(wp, "\t_rsPassword[%d]='%s';\n", k, Entry.rsPassword);
		boaWrite(wp, "\t_uCipher[%d]=%d;\n", k, Entry.unicastCipher);
		boaWrite(wp, "\t_wpa2uCipher[%d]=%d;\n", k, Entry.wpa2UnicastCipher);
		boaWrite(wp, "\t_wepAuth[%d]=%d;\n", k, Entry.authType);
		boaWrite(wp, "\t_wepLen[%d]=%d;\n", k, Entry.wep);
		boaWrite(wp, "\t_wepKeyFormat[%d]=%d;\n", k, Entry.wepKeyType);
		isNmode=wl_isNband(Entry.wlanBand);
		boaWrite(wp, "\t_wlan_isNmode[%d]=%d;\n", k, isNmode);
#ifdef WLAN_11W
		boaWrite(wp, "\t_dotIEEE80211W[%d]=%d;\n", k, Entry.dotIEEE80211W);
		boaWrite(wp, "\t_sha256[%d]=%d;\n\t", k, Entry.sha256);
#else
		boaWrite(wp, "\t_dotIEEE80211W[%d]=%d;\n", k, 0);
		boaWrite(wp, "\t_sha256[%d]=%d;\n\t", k, 0);
#endif
		k++;
	}
	boaWrite(wp, "\tssid_num=%d;\n", k);
	if(k==0 || wlanDisabled){
		boaWrite(wp, "\t%s.getElementById(\"wlan_security_table\").style.display = 'none';\n", DOCUMENT);
		if (wlanDisabled)
			boaWrite(wp, "%s.write(\"<font size=2> WLAN Disabled !</font>\")", DOCUMENT);
		else
			boaWrite(wp, "%s.write(\"<font size=2> Please use Site Survey Page to configure Client Security.</font>\")", DOCUMENT);
	}
	else{
		boaWrite(wp, "\t%s.getElementById(\"wlan_security_table\").style.display = \"\";\n", DOCUMENT);
	}
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
#endif // of WLAN_SUPPORT

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

void ShowWmm(int eid, request * wp, int argc, char **argv)
{
#ifdef WLAN_QoS
	boaWrite(wp,
		"<tr><td width=\"30%%\"><font size=2><b>%s:</b></td>" \
		"<td width=\"70%%\"><font size=2>" \
		"<input type=\"radio\" name=WmmEnabled value=1>%s&nbsp;&nbsp;" \
		"<input type=\"radio\" name=WmmEnabled value=0>%s</td></tr>",
		multilang(LANG_WMM_SUPPORT), multilang(LANG_ENABLED), multilang(LANG_DISABLED));
#endif
}

#ifdef CONFIG_USER_DHCP_SERVER
void initDhcpMode(request * wp)
{
	unsigned char vChar;
	char buf[16];

// Kaohj --- assign DHCP pool ip prefix; no pool prefix for complete IP pool
#ifdef DHCPS_POOL_COMPLETE_IP
	boaWrite(wp, "pool_ipprefix='';\n");
#else
	getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, buf);
	boaWrite(wp, "pool_ipprefix='%s';\n", buf);
#endif
// Kaohj
#ifdef DHCPS_DNS_OPTIONS
	boaWrite(wp, "en_dnsopt=1;\n");
	mib_get(MIB_DHCP_DNS_OPTION, (void *)&vChar);
	boaWrite(wp, "dnsopt=%d;\n", vChar);
#else
	boaWrite(wp, "en_dnsopt=0;\n");
#endif
	mib_get( MIB_DHCP_MODE, (void *)&vChar);
	boaWrite(wp, "%s.dhcpd.dhcpdenable[%d].checked = true;\n", DOCUMENT, vChar);
}
#endif

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
#ifdef IP_PASSTHROUGH
	if ( !strcmp(name, "others") )
		initOthersPage(wp);
#endif
#ifdef TIME_ZONE
	if ( !strcmp(name, "ntp") )
		initNtpPage(wp);
#endif
#ifdef WLAN_SUPPORT
#ifdef WLAN_WPA
	if ( !strcmp(name, "wlwpa") )
		initWlWpaPage(wp);
#endif
#ifdef WLAN_11R
	if ( !strcmp(name, "wlft") )
		initWlFtPage(wp);
#endif
	// Mason Yu. 201009_new_security
	if ( !strcmp(name, "wlwpa_mbssid") )
		initWlWpaMbssidPage(wp);
	if ( !strcmp(name, "wlbasic") )
		initWlBasicPage(wp);
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
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
	if ( !strcmp(name, "wlsurvey") )
		initWlSurveyPage(wp);

#endif
#ifdef WLAN_ACL
	if ( !strcmp(name, "wlactrl") )
		initWlAclPage(wp);
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

#ifdef CONFIG_USER_DHCP_SERVER
	if ( !strcmp(name, "dhcp-mode") )
		initDhcpMode(wp);
#endif
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

