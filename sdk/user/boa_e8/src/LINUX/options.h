/*
 *	option.h
 */


#ifndef INCLUDE_OPTIONS_H
#define INCLUDE_OPTIONS_H

#if defined(EMBED) || defined(__KERNEL__)
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

#ifdef EMBED
#include <config/autoconf.h>
#else
#include "../../../../config/autoconf.h"
#endif

//jiunming, redirect the web page to specific page only once per pc for wired router
#undef WEB_REDIRECT_BY_MAC

//alex
#undef CONFIG_USBCLIENT
//jim for power_led behavior according to TR068..
#undef WLAN_BUTTON_LED
// try to restore the user setting when config checking error
#define KEEP_CRITICAL_HW_SETTING
#undef KEEP_CRITICAL_CURRENT_SETTING

#ifndef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
//ql--if image header error, dont reboot the system.
#define ENABLE_SIGNATURE_ADV
#undef ENABLE_SIGNATURE
#ifdef ENABLE_SIGNATURE
#define SIGNATURE	""
#endif
#endif

//ql-- limit upstream traffic
#undef UPSTREAM_TRAFFIC_CTL

#define BR_ROUTE_ONEPVC   //allow set one br and one route on the same pvc
//august: NEW_PORTMAPPING is used in user space
//#ifdef CONFIG_NEW_PORTMAPPING
#define NEW_PORTMAPPING
//#endif

#define INCLUDE_DEFAULT_VALUE		1

#ifdef CONFIG_USER_UDHCP099PRE2
#define COMBINE_DHCPD_DHCRELAY
#endif

#define APPLY_CHANGE

//#define SECONDARY_IP
//star: for set acl ip range
#undef ACL_IP_RANGE
//star: for layer7 filter
#undef LAYER7_FILTER_SUPPORT

#ifdef CONFIG_USER_WIRELESS_TOOLS

#define WLAN_SUPPORT			1
#define WLAN_WPA			1
#ifdef CONFIG_USER_WIRELESS_WDS
#define WLAN_WDS			1
#endif
#define WLAN_1x				1
#define WLAN_ACL			1
#undef WIFI_TEST
#ifdef CONFIG_USER_WLAN_QOS
#define WLAN_QoS
#endif
#ifdef CONFIG_USER_WIRELESS_MBSSID
#define WLAN_MBSSID			1
#define CTCOM_WLAN_REQ		1   //CTCOM request tr069 wireless mssid entity can be add and del
#endif

#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
#define WLAN_CLIENT
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
#define WLAN_UNIVERSAL_REPEATER
#endif

#if (defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_SLAVE_WLAN1_ENABLE)) || \
	(defined(CONFIG_USE_PCIE_SLOT_0) && defined(CONFIG_USE_PCIE_SLOT_1))
#define WLAN_DUALBAND_CONCURRENT 1
#endif

#if (!defined(WLAN_DUALBAND_CONCURRENT) && defined(CONFIG_RTL_8812_SUPPORT)) || \
	(defined(CONFIG_WLAN0_5G_WLAN1_2G)) || \
	(defined(CONFIG_NO_WLAN_DRIVER) && (defined(CONFIG_RTL_8812_SUPPORT)))
	//single band 5G,  dual band 5G, dual linux slave wlan only 5G
#define WLAN0_5G_SUPPORT  1
#endif

#if (!defined(WLAN_DUALBAND_CONCURRENT) && (defined(CONFIG_RTL_8812_SUPPORT) && !defined(CONFIG_RTL_8812AR_VN_SUPPORT))) || \
	(defined(CONFIG_WLAN0_5G_WLAN1_2G) && (defined(CONFIG_RTL_8812_SUPPORT) && !defined(CONFIG_RTL_8812AR_VN_SUPPORT))) || \
	(defined(CONFIG_NO_WLAN_DRIVER) && (defined(CONFIG_RTL_8812_SUPPORT) && !defined(CONFIG_RTL_8812AR_VN_SUPPORT)))
	// single band 5G 11ac, dual band 5G 11ac, dual linux slave wlan only 5G 11ac
#define WLAN0_5G_11AC_SUPPORT 1
#endif

#if defined(CONFIG_WLAN0_2G_WLAN1_5G)
	//dualband
#define WLAN1_5G_SUPPORT 1
#endif

#if defined (CONFIG_WLAN1_5G_11AC_SUPPORT)  || \
	(defined(CONFIG_WLAN0_2G_WLAN1_5G)  && (defined(CONFIG_RTL_8812_SUPPORT) && !defined(CONFIG_RTL_8812AR_VN_SUPPORT)))
	//dual linux slave wlan 5G 11ac, dual band single linux wlan1 5G 11ac
#define WLAN1_5G_11AC_SUPPORT 1
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
#define NUM_WLAN_INTERFACE		2	// number of wlan interface supported
#else
#define NUM_WLAN_INTERFACE		1	// number of wlan interface supported
#endif

#ifdef WLAN_MBSSID
#define WLAN_MBSSID_NUM		4
#else
#define WLAN_MBSSID_NUM		0
#endif
#define WLAN_VAP_ITF_INDEX		1 // root
#define WLAN_REPEATER_ITF_INDEX		(1+WLAN_MBSSID_NUM) // root+VAP
#ifdef WLAN_UNIVERSAL_REPEATER
#define NUM_VWLAN_INTERFACE	WLAN_MBSSID_NUM+1 // VAP+Repeater
#else
#define NUM_VWLAN_INTERFACE	WLAN_MBSSID_NUM // VAP
#endif

#define MAX_WLAN_VAP			4
#undef E8A_CHINA

#undef WLAN_ONOFF_BUTTON_SUPPORT

//xl_yue: support zte531b--light:wps function is ok; die:wps function failed; blink: wps connecting
//#undef	 REVERSE_WPS_LED

#define ENABLE_WPAAES_WPA2TKIP

#ifdef CONFIG_WIFI_TIMER_SCHEDULE
#define WIFI_TIMER_SCHEDULE
#endif

#endif

#ifndef CONFIG_SFU
#define IP_PORT_FILTER
#define MAC_FILTER
#ifdef CONFIG_RTK_RG_INIT
#define MAC_FILTER_SRC_ONLY 1
#endif
#define PORT_FORWARD_GENERAL
#define URL_BLOCKING_SUPPORT
#define URL_ALLOWING_SUPPORT
#define DOMAIN_BLOCKING_SUPPORT
#ifdef CONFIG_USER_PARENTAL_CONTROL
#define PARENTAL_CTRL
#endif
//uncomment for TCP/UDP connection limit
//#define TCP_UDP_CONN_LIMIT	1
#undef TCP_UDP_CONN_LIMIT
#undef NAT_CONN_LIMIT
#undef NATIP_FORWARDING
#undef PORT_TRIGGERING
#define DMZ
#undef ADDRESS_MAPPING
#define ROUTING
#define REMOTE_ACCESS_CTL
#define IP_PASSTHROUGH
#define IP_ACL
#endif

// Mason Yu
#undef PORT_FORWARD_ADVANCE
#define VIRTUAL_SERVER_SUPPORT			// Mason Yu. 2630-e8b
#ifdef CONFIG_USER_RTK_PPPOE_PASSTHROUGH
#define PPPOE_PASSTHROUGH
#endif
#undef  MULTI_ADDRESS_MAPPING
#undef CONFIG_IGMP_FORBID
#define FORCE_IGMP_V2			1


#ifdef CONFIG_8M_SDRAM
#define MINIMIZE_RAM_USAGE 1
#undef SUPPORT_AUTH_DIGEST
#endif

#undef SUPPORT_AUTH_DIGEST

#ifdef CONFIG_USB_SUPPORT
#define USB_SUPPORT					1
#define _PRMT_USBRESTORE            1
#endif

#define WEB_UPGRADE			1
//#undef WEB_UPGRADE			// jimluo e8-A spec, unsupport web upgrade.

// Mason Yu
#undef AUTO_PVC_SEARCH_TR068_OAMPING
#define AUTO_PVC_SEARCH_PURE_OAMPING
#define AUTO_PVC_SEARCH_AUTOHUNT

#ifndef AUTO_PVC_SEARCH_TR068_OAMPING
#define AUTO_PVC_SEARCH_TABLE
#endif

#ifdef CONFIG_USER_VSNTP
#define TIME_ZONE			1
#endif

#ifdef CONFIG_USER_IPROUTE2_TC_TC

/*#ifdef CONFIG_NET_SCH_DSMARK*/
#define QOS_DIFFSERV
/*#endif*/
#define	IP_QOS_VPORT		1
#undef   CONFIG_8021P_PRIO
#ifdef CONFIG_IP_NF_TARGET_DSCP
#define QOS_DSCP		1
#endif
//#ifdef NEW_IP_QOS_SUPPORT
#if defined(NEW_IP_QOS_SUPPORT) || defined(QOS_DIFFSERV)
#ifdef CONFIG_IP_NF_MATCH_DSCP
#define QOS_DSCP_MATCH		1
#endif
#endif
#endif
#ifdef CONFIG_USER_IPROUTE2_IP_IP
#ifdef IP_QOS
#define IP_POLICY_ROUTING		1
#endif
#endif

#ifndef NEW_PORTMAPPING
#define ITF_GROUP			1
#endif

#define ENABLE_802_1Q		// enable_802_1p_090722
// Mason Yu. combine_1p_4p_PortMapping
#ifdef ITF_GROUP
	#define ITF_GROUP_1P
#endif

#undef VLAN_GROUP
#undef ELAN_LINK_MODE
#undef ELAN_LINK_MODE_INTRENAL_PHY
#define DIAGNOSTIC_TEST			1

//xl_yue
#undef	DOS_SUPPORT

#define NEW_DGW_POLICY   // E8B: the dgw is useless in MIB_ATM_VC_TBL, the INTERNET type connection who first get ip will be default gateway
#define DEFAULT_GATEWAY_V1	//set dgw per pvc
#undef DEFAULT_GATEWAY_V2	// set dgw interface in routing page
#ifndef DEFAULT_GATEWAY_V2
#ifndef DEFAULT_GATEWAY_V1
#define DEFAULT_GATEWAY_V1	1
#endif
#endif
#ifdef DEFAULT_GATEWAY_V2
#define AUTO_PPPOE_ROUTE	1
//#undef AUTO_PPPOE_ROUTE
#endif

//alex_huang
#undef  CONFIG_SPPPD_STATICIP
#undef XOR_ENCRYPT
#undef XML_TR069
#define TELNET_IDLE_TIME	600 //10*60 sec. Please compile boa and telnetd

/* wpeng defined for support dhcp option 33/121/249*/
#define _CONFIG_DHCPC_OPTION33_         1
/* wpeng 20120412 END*/

#ifdef CONFIG_USER_CWMP_TR069
// Mason Yu
#ifndef XML_TR069
#define XML_TR069
#endif  //XML_TR069
#define _CWMP_MIB_				1
#ifdef CONFIG_USER_CWMP_WITH_SSL
#define _CWMP_WITH_SSL_				1
#endif //CONFIG_USER_CWMP_WITH_SSL
#define _PRMT_SERVICES_				1
#define _PRMT_CAPABILITIES_			1
#define _PRMT_DEVICECONFIG_			1
//#define _PRMT_USERINTERFACE_			1
/*disable connection request authentication*/
//#define _TR069_CONREQ_AUTH_SELECT_		1
#ifdef CONFIG_USER_TR143
#define _PRMT_TR143_				1
#endif //CONFIG_USER_TR143
#ifdef CONFIG_USB_ETH
#define _PRMT_USB_ETH_				1
#endif //CONFIG_USB_ETH


/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#define _PRMT_WT107_					1
#ifdef _PRMT_WT107_
#ifdef CONFIG_USER_BUSYBOX_TRACEROUTE
#define _SUPPORT_TRACEROUTE_PROFILE_		1
#endif //CONFIG_USER_BUSYBOX_TRACEROUTE
#define _SUPPORT_CAPTIVEPORTAL_PROFILE_		1
#define _SUPPORT_ADSL2DSLDIAG_PROFILE_		1
#define _SUPPORT_ADSL2WAN_PROFILE_		1
#endif //_PRMT_WT107_
/*ping_zhang:20081217 END*/

#define _PRMT_X_TELEFONICA_ES_DHCPOPTION_	1

/*ping_zhang:20081223 START:define for support multi server by default*/
#define SNTP_MULTI_SERVER			1
/*ping_zhang:20081223 END*/

// Mason Yu. for e8b_2630, client limit
#if defined(CONFIG_LUNA) || defined(CONFIG_CTC_E8_CLIENT_LIMIT)
#define IP_BASED_CLIENT_TYPE
#endif

/* copy from e8-user*/
#define CTC_TELECOM_ACCOUNT
//#define FTP_ACCOUNT_INDEPENDENT
//#define TELNET_ACCOUNT_INDEPENDENT

#ifdef CONFIG_YUEME
#define DHCPC_RIGOROUSNESS_SUPPORT
#endif

//#define ENABLE_WPAAES_WPA2TKIP
#define E8B_NEW_DIAGNOSE
#define _PRMT_X_CT_EXT_ENABLE_

#ifdef _PRMT_X_CT_EXT_ENABLE_
	/*TW's ACS has some problem with this extension field*/
	#define _INFORM_EXT_FOR_X_CT_		1
    #ifdef _PRMT_SERVICES_
	#define _PRMT_X_CT_COM_IPTV_		1
	#define _PRMT_X_CT_COM_MWBAND_		1
    #endif //_PRMT_SERVICES_
	#define	_PRMT_X_CT_COM_DDNS_		1
	#define _PRMT_X_CT_COM_ALG_		1
	#define _PRMT_X_CT_COM_ACCOUNT_		1
	#define _PRMT_X_CT_COM_RECON_		1
	#define _PRMT_X_CT_COM_PORTALMNT_	1
	#define _PRMT_X_CT_COM_SRVMNG_		1	/*ServiceManage*/
	#define _PRMT_X_CT_COM_PPPOE_PROXY_	1
    #ifdef WLAN_SUPPORT
	#define _PRMT_X_CT_COM_WLAN_		1
    #endif //WLAN_SUPPORT
	#define _PRMT_X_CT_COM_DHCP_		1
	#define _PRMT_X_CT_COM_WANEXT_		1
	#define _PRMT_X_CT_COM_DLNA_		1
	#define _PRMT_X_CT_COM_UPNP_		1
	#define _PRMT_X_CT_COM_DEVINFO_		1
	#define _PRMT_X_CT_COM_ALARM_MONITOR_	1
	#define _PRMT_X_CT_COM_IPv6_		1
	#define _PRMT_X_CT_COM_ETHLINK_		1
	#define _PRMT_X_CT_COM_PING_		1
	#define _PRMT_X_CT_COM_TIME_		1
	#define _PRMT_X_CT_COM_QOS_		1
	//#define _PRMT_X_STD_QOS_			1
	#define _PRMT_X_CT_COM_USERINFO_	1
	#define _PRMT_X_CT_COM_SYSLOG_	1
	#define _PRMT_X_CT_COM_VLAN_BOUND_	1
	#ifdef CONFIG_USER_RTK_LBD
	#define _PRMT_X_CT_COM_LBD_ 		1
	#endif
	#ifdef CONFIG_USER_CWMP_UPNP_DM
	#define _PRMT_X_CT_COM_PROXY_DEV_ 		1
	#endif
#endif //_PRMT_X_CT_EXT_ENABLE_
#endif //CONFIG_USER_CWMP_TR069

#define E8B_GET_OUI

#ifdef WEB_UPGRADE
#define	UPGRADE_V1			1
#endif // if WEB_UPGRADE

//ql add
#ifdef CONFIG_RESERVE_KEY_SETTING
#define	RESERVE_KEY_SETTING
#endif

#undef WEB_ENABLE_PPP_DEBUG
// Mason Yu
#undef SEND_LOG
#define BB_FEATURE_SAVE_LOG
#define USE_BUSYBOX_KLOGD

//xl_yue add,web logining is maintenanced by web server
#define	USE_LOGINWEB_OF_SERVER

#ifdef USE_LOGINWEB_OF_SERVER
//xl_yue add,if have logined error for three times continuely,please relogin after 1 minute
#define LOGIN_ERR_TIMES_LIMITED 1
//xl_yue add,only one user can login with the same account at the same time
#define ONE_USER_LIMITED	1
#undef USE_BASE64_MD5_PASSWD
#endif

/*######################*/
//jim 2007-05-22
//4 jim_luo Bridge Mode only access on web
//#define BRIDGE_ONLY_ON_WEB
#undef  BRIDGE_ONLY_ON_WEB

//4 E8-A unsupport save and restore configuration file, then should remark belwo macro CONFIG_SAVE_RESTORE
#define CONFIG_SAVE_RESTORE

//E8-A unsupport web upgrade image, we should enable #undef WEB_UPGRADE at line 52
/*########################*/

//add by ramen
//#define  DNS_BIND_PVC_SUPPORT
//#define	 DNSV6_BIND_PVC_SUPPORT
//#define  POLICY_ROUTING_DNSV4RELAY
#undef QOS_SPEED_LIMIT_SUPPORT

#define  DHCPS_POOL_COMPLETE_IP
#define  DHCPS_DNS_OPTIONS
#undef ACCOUNT_CONFIG
#undef MULTI_USER_PRIV
#ifdef MULTI_USER_PRIV
#define ACCOUNT_CONFIG
#endif

/*ql:20080729 START: different image header for different IC type*/
#undef MULTI_IC_SUPPORT
/*ql:20080729 END*/

/*xl_yue:20090210 add cli cmdedit*/
#ifdef CONFIG_USER_CMD_CLI
#define CONFIG_CLI_CMD_EDIT
#define CONFIG_CLI_TAB_FEATURE
#endif

//added by ql to support imagenio service
//#define IMAGENIO_IPTV_SUPPORT		// base on option60 with option240~241


#endif  // INCLUDE_OPTIONS_H

#undef AUTO_DETECT_DMZ
// add by yq_zhou
#undef CONFIG_11N_SAGEM_WEB

// Magician
#define COMMIT_IMMEDIATELY

//cathy
#define USE_11N_UDP_SERVER

//support reserved IP addresses for DHCP, jiunming
#define SUPPORT_DHCP_RESERVED_IPADDR	1

#undef URL_BLOCKING_ON_BRIDGE_MODE
#define CONFIG_IGMPPROXY_MULTIWAN

//for FIELD_TRY_SAFE_MODE web control, need ADSL driver support
#undef FIELD_TRY_SAFE_MODE

#define DEBUG_MEMORY_CHANGE 0  // Magician: for something about memory debugging.
#ifdef CONFIG_IPV6
#define DUAL_STACK_LITE
#endif

#undef ENABLE_ADSL_MODE_GINP

// Mason Yu.
// Define all functions on boa for LUNA
#ifdef CONFIG_LUNA
#define GEN_WAN_MAC
#endif

//#define DEBUGPRINT  fprintf(stderr,"%s %d %s.............\n",__FILE__,__LINE__,__FUNCTION__);
#define DEBUGPRINT
#define CTC_WAN_NAME

// for e8 project, do not redial when connection lost.
#define CONFIG_NO_REDIAL

#define IP_RANGE_FILTER_SUPPORT

#define WEB_AUTH_PRIVILEGE

#ifdef CONFIG_YUEME
#define CTC_DNS_SPEED_LIMIT

#if defined(CONFIG_MCAST_VLAN) && defined(CONFIG_RTK_RG_INIT)
#define SUPPORT_MCAST_TEST
#endif

#define SUPPORT_WAN_BANDWIDTH_INFO
#define SUPPORT_WEB_REDIRECT
#define SUPPORT_ACCESS_RIGHT
#define SUPPORT_WEB_PUSHUP
#endif

