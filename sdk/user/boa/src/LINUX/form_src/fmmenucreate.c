/*
*  fmmenucreate.c is used to create menu
*  added by xl_yue
*/
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../webs.h"
#include "mib.h"
#include "webform.h"
#include "utility.h"
#include "multilang.h"
//add by ramen to include the autoconf.h created by kernel
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#define GPON_SETTINGS_STR "GPON Settings"
#define EPON_SETTINGS_STR "EPON Settings"
#define PON_STR "PON"
#endif

#ifdef CONFIG_DEFAULT_WEB	// default pages
/*
 *	2nd Layer Menu
 */
struct RootMenu childmenu_dns[] = {
  //{"DNS Server", MENU_URL, "dns.asp", "DNS Server Configuration", 0, 0, MENU_DISPLAY, LANG_DNS_SERVER},
#ifdef CONFIG_USER_DDNS
  {"Dynamic DNS", MENU_URL, "ddns.asp", "DDNS Configuration", 0, 0, MENU_DISPLAY, LANG_DYNAMIC_DNS},
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};

#ifdef CONFIG_IPV6
struct RootMenu childmenu_ipv6[] = {
  {"IPv6", MENU_URL, "ipv6_enabledisable.asp", "IPv6 Enable/Disable Configuration", 0, 0, MENU_DISPLAY, LANG_IPV6},
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
  {"RADVD", MENU_URL, "radvdconf.asp", "RADVD configuration", 0, 0, MENU_DISPLAY, LANG_RADVD},
#endif
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
  {"DHCPv6", MENU_URL, "dhcpdv6.asp", "DHCPv6 Configuration", 0, 0, MENU_DISPLAY, LANG_DHCPV6},
#endif
#ifdef CONFIG_USER_ECMH
  {"MLD Proxy", MENU_URL, "app_mldProxy.asp", "MLD Proxy Configuration", 0, 0, MENU_DISPLAY, LANG_MLD_PROXY},
  {"MLD Snooping", MENU_URL, "app_mld_snooping.asp", "MLD Snooping Configuration", 0, 0, MENU_DISPLAY, LANG_MLD_SNOOPING},
#endif
  {"IPv6 Routing", MENU_URL, "routing_ipv6.asp", "IPv6 Routing Configuration", 0, 0, MENU_DISPLAY, LANG_IPV6_ROUTING},
#ifdef IP_PORT_FILTER
#ifdef CONFIG_IPV6_OLD_FILTER
#ifdef CONFIG_RTK_RG_INIT
  {"IP/Port Filtering", MENU_URL, "fw-ipportfilter-v6_rg.asp", "Setup IPv6/Port filering", 0, 0, MENU_DISPLAY, LANG_IP_PORT_FILTERING},
#else
  {"IP/Port Filtering", MENU_URL, "fw-ipportfilter-v6.asp", "Setup IPv6/Port filering", 0, 0, MENU_DISPLAY, LANG_IP_PORT_FILTERING},
#endif
#else
#ifdef CONFIG_RTK_RG_INIT
  {"IP/Port Filtering", MENU_URL, "fw-ipportfilter-v6_IfId_rg.asp", "Setup IPv6/Port filering", 0, 0, MENU_DISPLAY, LANG_IP_PORT_FILTERING},
#else
  {"IP/Port Filtering", MENU_URL, "fw-ipportfilter-v6_IfId.asp", "Setup IPv6/Port filering", 0, 0, MENU_DISPLAY, LANG_IP_PORT_FILTERING},
#endif
#endif  
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};
#endif

#ifdef CONFIG_USER_DOT1AG_UTILS
struct RootMenu childmenu_dot1ag[] = {
  {"Configuration", MENU_URL, "dot1ag_conf.asp", "802.1ag (CFM) Configuration", 0, 0, MENU_DISPLAY, LANG_CONFIGURATION},
  {"Action", MENU_URL, "dot1ag_action.asp", "802.1ag (CFM) Action", 0, 0, MENU_DISPLAY, LANG_ACTION},
  {"Status", MENU_URL, "dot1ag_status.asp", "802.1ag (CFM) Status", 0, 0, MENU_DISPLAY, LANG_STATUS_1},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
#endif

struct RootMenu childmenu_fw[] = {
#ifdef CONFIG_IP_NF_ALG_ONOFF
  {"ALG", MENU_URL, "algonoff.asp", "ALG on-off", 0, 0, MENU_DISPLAY, LANG_ALG},
#endif
#ifdef IP_PORT_FILTER
#ifdef CONFIG_RTK_RG_INIT
 {"IP/Port Filtering", MENU_URL, "fw-ipportfilter_rg.asp",
  "Setup IP/Port filering", 0, 0, MENU_DISPLAY, LANG_IP_PORT_FILTERING},
#else
  {"IP/Port Filtering", MENU_URL, "fw-ipportfilter.asp",
   "Setup IP/Port filering", 0, 0, MENU_DISPLAY, LANG_IP_PORT_FILTERING},
#endif
#endif
#ifdef MAC_FILTER
#ifdef CONFIG_RTK_RG_INIT
	{"MAC Filtering", MENU_URL, "fw-macfilter_rg.asp", "Setup MAC filering", 0, 0, MENU_DISPLAY, LANG_MAC_FILTERING},
#else
  {"MAC Filtering", MENU_URL, "fw-macfilter.asp", "Setup MAC filering", 0, 0, MENU_DISPLAY, LANG_MAC_FILTERING},
#endif
#endif
#ifdef PORT_FORWARD_GENERAL
  {"Port Forwarding", MENU_URL, "fw-portfw.asp", "Setup port-forwarding", 0,
   0, MENU_DISPLAY, LANG_PORT_FORWARDING},
#endif
#ifdef URL_BLOCKING_SUPPORT
  {"URL Blocking", MENU_URL, "url_blocking.asp", "URL Blocking Setting", 0,
   0, MENU_DISPLAY, LANG_URL_BLOCKING},
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
  {"Domain Blocking", MENU_URL, "domainblk.asp", "Domain Blocking Setting", 0,
   0, MENU_DISPLAY, LANG_DOMAIN_BLOCKING},
#endif
#ifdef PARENTAL_CTRL
  {"Parental Control", MENU_URL, "parental-ctrl.asp", "Parental Control Setting", 0,
   0, MENU_DISPLAY, LANG_PARENTAL_CONTROL},
#endif
#ifdef TCP_UDP_CONN_LIMIT
  {"Connection Limit", MENU_URL, "connlimit.asp", "Connection Limit Setting", 0,
   0, MENU_DISPLAY, LANG_CONNECTION_LIMIT},
#endif // TCP_UDP_CONN_LIMIT
#ifdef NATIP_FORWARDING
  {"NAT IP Forwarding", MENU_URL, "fw-ipfw.asp", "Setup NAT IP Mapping", 0,
   0, MENU_DISPLAY, LANG_NAT_IP_FORWARDING},
#endif
#ifdef PORT_TRIGGERING
  {"Port Triggering", MENU_URL, "gaming.asp", "Setup Port Triggering", 0, 0, MENU_DISPLAY, LANG_PORT_TRIGGERING},
#endif
#ifdef DMZ
  {"DMZ", MENU_URL, "fw-dmz.asp", "Setup DMZ",0, 0, MENU_DISPLAY, LANG_DMZ},
#endif
#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
 // Eric Chen add for True
  {"NAT Rule Configuration", MENU_URL, "multi_addr_mapping.asp", "Setup NAT Rule",0, 0, MENU_DISPLAY, LANG_NAT_RULE_CONFIGURATION},
#else //!MULTI_ADDRESS_MAPPING
 // Mason Yu on True
  {"NAT Rule Configuration", MENU_URL, "addr_mapping.asp", "Setup NAT Rule",0, 0, MENU_DISPLAY, LANG_NAT_RULE_CONFIGURATION},
  #endif// end of !MULTI_ADDRESS_MAPPING
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}

};

//ql 20081119 START
#ifdef NEW_IP_QOS_SUPPORT
struct RootMenu childmenu_qos[] = {
	{"QoS Classification", MENU_URL, "ipqos_sc.asp", "Setup IP QoS", 0, 0, MENU_DISPLAY, LANG_QOS_CLASSIFICATION},
	{"Traffic Shaping", MENU_URL, "ipqos_shape.asp", "traffic control", 0, 0, MENU_DISPLAY, LANG_TRAFFIC_SHAPING},
	{0, 0, 0, 0, 0, 0, 0, 0}
};
#endif
#ifdef IP_QOS
struct RootMenu childmenu_qos[] = {
	{"Classification", MENU_URL, "ipqos.asp", "Traffic Configuration", 0, 0, MENU_DISPLAY, LANG_CLASSIFICATION},
	{"QoS Queue", MENU_URL, "ipqos_queue.asp", "Queue Config", 0, 0, MENU_DISPLAY, LANG_QOS_QUEUE},
	{0, 0, 0, 0, 0, 0, 0, 0}
};
#endif
#ifdef CONFIG_USER_IP_QOS_3
struct RootMenu childmenu_qos[] = {
	{"QoS Policy", MENU_URL, "net_qos_imq_policy.asp", "Setup IP QoS", 0, 0, MENU_DISPLAY, LANG_QOS_POLICY},
	{"QoS Classification", MENU_URL, "net_qos_cls.asp", "Setup IP QoS Classification", 0, 0, MENU_DISPLAY, LANG_QOS_CLASSIFICATION},
	{"Traffic Shaping", MENU_URL, "net_qos_traffictl.asp", "traffic control", 0, 0, MENU_DISPLAY, LANG_TRAFFIC_SHAPING},
	{0, 0, 0, 0, 0, 0, 0, 0}
};
#endif

#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD) || defined(CONFIG_NET_IPIP) || defined(CONFIG_XFRM)
struct RootMenu childmenu_vpn[] = {
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#ifndef CONFIG_IPV6_VPN
#ifdef CONFIG_USER_PPTPD_PPTPD
	{"PPTP", MENU_URL, "pptpd.asp", "PPTP Configuration", 0, 0, MENU_DISPLAY, LANG_PPTP},
#else
	{"PPTP", MENU_URL, "pptp.asp", "PPTP Configuration", 0, 0, MENU_DISPLAY, LANG_PPTP},
#endif
#else
	{"PPTP", MENU_URL, "pptpv6.asp", "PPTP Configuration", 0, 0, MENU_DISPLAY, LANG_PPTP},
#endif
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_L2TPD_L2TPD
#ifndef CONFIG_IPV6_VPN
#ifdef CONFIG_USER_L2TPD_LNS
	{"L2TP", MENU_URL, "l2tpd.asp", "L2TP Configuration", 0, 0, MENU_DISPLAY, LANG_L2TP},
#else
	{"L2TP", MENU_URL, "l2tp.asp", "L2TP Configuration", 0, 0, MENU_DISPLAY, LANG_L2TP},
#endif
#else
	{"L2TP", MENU_URL, "l2tpv6.asp", "L2TP Configuration", 0, 0, MENU_DISPLAY, LANG_L2TP},
#endif
#endif //end of CONFIG_USER_L2TPD_L2TPD
#ifdef CONFIG_NET_IPIP
	{"IPIP", MENU_URL, "ipip.asp", "IPIP Configuration", 0, 0, MENU_DISPLAY, LANG_IPIP},
#endif //end of CONFIG_NET_IPIP
#ifdef CONFIG_XFRM
	{"IPsec", MENU_URL, "ipsec.asp", "IPsec Configuration", 0, 0, MENU_DISPLAY, LANG_IPSEC},
#endif

	{0, 0, 0, 0, 0, 0, 0, 0}
};
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD || CONFIG_NET_IPIP

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
static struct RootMenu childmenu_wapi0[] = {
	{"Certification Installation", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwapiinstallcert.asp&wlan_idx=0", "WAPI certificate install", 0, 0, MENU_DISPLAY, LANG_CERTIFICATION_INSTALLATION},
	//{"Key Update", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwapiRekey.asp&wlan_idx=0", "WAPI key update", 0, 0, MENU_DISPLAY},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

static struct RootMenu childmenu_wapi1[] = {
	{"Certification Installation", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwapiinstallcert.asp&wlan_idx=1", "WAPI certificate install", 0, 0, MENU_DISPLAY, LANG_CERTIFICATION_INSTALLATION},
	//{"Key Update", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwapiRekey.asp&wlan_idx=1", "WAPI key update", 0, 0, MENU_DISPLAY},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

struct RootMenu childmenu_wlan0[] = {
	{"Basic Settings", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlbasic.asp&wlan_idx=0",	"Setup wireless basic configuration", 0, 0, MENU_DISPLAY, LANG_BASIC_SETTINGS},
	{"Advanced Settings", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wladvanced.asp&wlan_idx=0",   "Setup wireless advanced configuration", 0, 0, MENU_DISPLAY, LANG_ADVANCED_SETTINGS},
	{"Security", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwpa.asp&wlan_idx=0", "Setup wireless security", 0, 0, MENU_DISPLAY, LANG_SECURITY},
#ifdef WLAN_11R
	{"Fast Roaming", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlft.asp&wlan_idx=0", "Fast BSS Transition", 0, 0, MENU_DISPLAY, LANG_FAST_ROAMING},
#endif
#ifdef WLAN_ACL
	{"Access Control", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlactrl.asp&wlan_idx=0",	"Setup access control list for wireless clients", 0, 0, MENU_DISPLAY, LANG_ACCESS_CONTROL},
#endif
#ifdef WLAN_WDS
	{"WDS", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwds.asp&wlan_idx=0", "WDS Settings", 0, 0, MENU_DISPLAY, LANG_WDS},
#endif
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
	{"Site Survey", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlsurvey.asp&wlan_idx=0", "Wireless Site Survey", 0, 0, MENU_DISPLAY, LANG_SITE_SURVEY},
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS
	{"WPS", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwps.asp&wlan_idx=0", "Wireless Protected Setup", 0, 0, MENU_DISPLAY, LANG_WPS},
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	{"WAPI", MENU_FOLDER, &childmenu_wapi0, "",
		sizeof (childmenu_wapi0) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WAPI},
#endif
	{"Status", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlstatus.asp&wlan_idx=0", "Wireless Current Status", 0, 0, MENU_DISPLAY, LANG_STATUS},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

struct RootMenu childmenu_wlan1[] = {
	{"Basic Settings", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlbasic.asp&wlan_idx=1",	"Setup wireless basic configuration", 0, 0, MENU_DISPLAY, LANG_BASIC_SETTINGS},
	{"Advanced Settings", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wladvanced.asp&wlan_idx=1",   "Setup wireless advanced configuration", 0, 0, MENU_DISPLAY, LANG_ADVANCED_SETTINGS},
	{"Security", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwpa.asp&wlan_idx=1", "Setup wireless security", 0, 0, MENU_DISPLAY, LANG_SECURITY},
#ifdef WLAN_11R
	{"Fast Roaming", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlft.asp&wlan_idx=1", "Fast BSS Transition", 0, 0, MENU_DISPLAY, LANG_FAST_ROAMING},
#endif
#ifdef WLAN_ACL
	{"Access Control", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlactrl.asp&wlan_idx=1",	"Setup access control list for wireless clients", 0, 0, MENU_DISPLAY, LANG_ACCESS_CONTROL},
#endif
#ifdef WLAN_WDS
	{"WDS", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwds.asp&wlan_idx=1", "WDS Settings", 0, 0, MENU_DISPLAY, LANG_WDS},
#endif
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
	{"Site Survey", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlsurvey.asp&wlan_idx=1", "Wireless Site Survey", 0, 0, MENU_DISPLAY, LANG_SITE_SURVEY},
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS
	{"WPS", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlwps.asp&wlan_idx=1", "Wireless Protected Setup", 0, 0, MENU_DISPLAY, LANG_WPS},
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	{"WAPI", MENU_FOLDER, &childmenu_wapi1, "",
		sizeof (childmenu_wapi1) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WAPI},
#endif
	{"Status", MENU_URL, "boaform/formWlanRedirect?redirect-url=/wlstatus.asp&wlan_idx=1", "Wireless Current Status", 0, 0, MENU_DISPLAY, LANG_STATUS},
	{0, 0, 0, 0, 0, 0, 0, 0}
};

#endif //CONFIG_RTL_92D_SUPPORT

/*
 *	First Layer Menu
 */
struct RootMenu childmenu_status[] = {
  {"Device", MENU_URL, "status.asp",   "Device Status", 0, 0, MENU_DISPLAY, LANG_DEVICE},
#ifdef CONFIG_IPV6
  {"IPv6", MENU_URL, "status_ipv6.asp",   "IPv6 Status", 0, 0, MENU_DISPLAY, LANG_IPV6},
#endif
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
  {"PON", MENU_URL, "status_pon.asp",   "PON Status", 0, 0, MENU_DISPLAY, LANG_PON},
#endif
#if defined(CONFIG_RTL_8676HWNAT)
  {"LAN Port", MENU_URL, "lan_port_status.asp",   "LAN Port Status", 0, 0, MENU_DISPLAY, LANG_LAN_PORT},
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};

struct RootMenu childmenu_lan[] = {
  {0, 0, 0, 0, 0, 0, 0, 0}
};

#ifdef CONFIG_RTL_WAPI_SUPPORT
static struct RootMenu childmenu_wapi[] = {
	{"Certification Installation", MENU_URL, "wlwapiinstallcert.asp", "WAPI certificate install", 0, 0, MENU_DISPLAY, LANG_CERTIFICATION_INSTALLATION},
	//{"Key Update", MENU_URL, "wlwapiRekey.asp", "WAPI key update", 0, 0, MENU_DISPLAY},
	{0, 0, 0, 0, 0, 0, 0, 0}
};
#endif

struct RootMenu childmenu_wlan[] = {
#if defined(CONFIG_RTL_92D_SUPPORT)
  {"Wireless Band Mode", MENU_URL, "wlbandmode.asp",   "Setup wireless band mode", 0, 0, MENU_DISPLAY, LANG_WIRELESS_BAND_MODE},
  {"wlan0 (5GHz)", MENU_FOLDER, &childmenu_wlan0, "", sizeof (childmenu_wlan0) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN0_5GHZ},
  {"wlan1 (2.4GHz)", MENU_FOLDER, &childmenu_wlan1, "", sizeof (childmenu_wlan1) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN1_2_4GHZ},
#elif defined (WLAN_DUALBAND_CONCURRENT)
#if defined (CONFIG_WLAN0_2G_WLAN1_5G) || defined(WLAN1_QTN)
  {"wlan0 (2.4GHz)", MENU_FOLDER, &childmenu_wlan0, "", sizeof (childmenu_wlan0) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN0_2_4GHZ},
  {"wlan1 (5GHz)", MENU_FOLDER, &childmenu_wlan1, "", sizeof (childmenu_wlan1) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN1_5GHZ},
#else
  {"wlan0 (5GHz)", MENU_FOLDER, &childmenu_wlan0, "", sizeof (childmenu_wlan0) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN0_5GHZ},
  {"wlan1 (2.4GHz)", MENU_FOLDER, &childmenu_wlan1, "", sizeof (childmenu_wlan1) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN1_2_4GHZ},
#endif
//#elif defined (CONFIG_MASTER_WLAN0_ENABLE)
//  {"wlan0", MENU_FOLDER, &childmenu_wlan0, "", sizeof (childmenu_wlan0) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY},
//#elif defined (CONFIG_SLAVE_WLAN1_ENABLE)
//  {"wlan1", MENU_FOLDER, &childmenu_wlan0, "", sizeof (childmenu_wlan0) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY},
#else //CONFIG_RTL_92D_SUPPORT || WLAN_DUALBAND_CONCURRENT || CONFIG_MASTER_WLAN0_ENABLE || CONFIG_SLAVE_WLAN1_ENABLE
  {"Basic Settings", MENU_URL, "wlbasic.asp",   "Setup wireless basic configuration", 0, 0, MENU_DISPLAY, LANG_BASIC_SETTINGS},
 #ifdef CONFIG_USER_FON
  {"FON Spot Settings", MENU_URL, "wlfon.asp",   "Setup FON Spot configuration", 0, 0, MENU_DISPLAY, LANG_FON_SPOT_SETTINGS},
 #endif
  {"Advanced Settings", MENU_URL, "wladvanced.asp",   "Setup wireless advanced configuration", 0, 0, MENU_DISPLAY, LANG_ADVANCED_SETTINGS},
  {"Security", MENU_URL, "wlwpa.asp", "Setup wireless security", 0, 0, MENU_DISPLAY, LANG_SECURITY},
#ifdef WLAN_11R
  {"Fast Roaming", MENU_URL, "wlft.asp", "Fast BSS Transition", 0, 0, MENU_DISPLAY, LANG_FAST_ROAMING},
#endif
#ifdef WLAN_ACL
  {"Access Control", MENU_URL, "wlactrl.asp",   "Setup access control list for wireless clients", 0, 0, MENU_DISPLAY, LANG_ACCESS_CONTROL},
#endif
#ifdef WLAN_WDS
  {"WDS", MENU_URL, "wlwds.asp", "WDS Settings", 0, 0, MENU_DISPLAY, LANG_WDS},
#endif
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
  {"Site Survey", MENU_URL, "wlsurvey.asp", "Wireless Site Survey", 0, 0, MENU_DISPLAY, LANG_SITE_SURVEY},
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	// WPS
  {"WPS", MENU_URL, "wlwps.asp", "Wireless Protected Setup", 0, 0, MENU_DISPLAY, LANG_WPS},
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
  {"WAPI", MENU_FOLDER, &childmenu_wapi, "",
   sizeof (childmenu_wapi) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WAPI},
#endif
  {"Status", MENU_URL, "wlstatus.asp", "Wireless Current Status", 0, 0, MENU_DISPLAY, LANG_STATUS},
#endif //CONFIG_RTL_92D_SUPPORT
  {0, 0, 0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_wan[] = {
	{WANMODE_CONF_STR, MENU_URL, "wanmode.asp", "WAN Mode Configuration", 0, 0, MENU_HIDDEN, LANG_WAN_MODE},
#ifdef CONFIG_ETHWAN
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	{PON_CONF_STR, MENU_URL, "boaform/formWanRedirect?redirect-url=/multi_wan_generic.asp&if=pon", "PON WAN Configuration", 0, 0, MENU_HIDDEN, LANG_PON_WAN},
#else
#ifdef CONFIG_RTL_MULTI_ETH_WAN
	{ETHWAN_CONF_STR, MENU_URL, "boaform/formWanRedirect?redirect-url=/multi_wan_generic.asp&if=eth", "ETH WAN Configuration", 0, 0, MENU_HIDDEN, LANG_ETHERNET_WAN},
#else
	{ETHWAN_CONF_STR, MENU_URL, "waneth.asp", "Ethernet WAN Configuration", 0, 0, MENU_HIDDEN, LANG_ETHERNET_WAN},
#endif
#endif
#endif
#ifdef CONFIG_PTMWAN
	{PTMWAN_CONF_STR, MENU_URL, "boaform/formWanRedirect?redirect-url=/multi_wan_generic.asp&if=ptm", "PTM WAN Configuration", 0, 0, MENU_HIDDEN, LANG_PTM_WAN},
#endif /*CONFIG_PTMWAN*/
#ifdef WLAN_WISP
	{WLWAN_CONF_STR, MENU_URL, "wanwl.asp", "Wireless WAN Configuration", 0, 0, MENU_HIDDEN, LANG_WIRELESS_WAN},
#endif
#ifdef CONFIG_RTL8672_SAR
  {DSLWAN_CONF_STR, MENU_URL, "wanadsl.asp", "ADSL Channel Configuration", 0, 0, MENU_HIDDEN, LANG_ATM_WAN},
  {ATM_SETTINGS_STR, MENU_URL, "wanatm.asp", "Setup ATM", 0, 0, MENU_HIDDEN, LANG_ATM_SETTINGS},
  {ADSL_SETTINGS_STR, MENU_URL, "/admin/adsl-set.asp", "Setup ADSL", 0, 0, MENU_HIDDEN, LANG_DSL_SETTINGS},
  #ifdef CONFIG_DSL_VTUO
  {VTUO_SETTINGS_STR, MENU_URL, "/admin/vtuo-set.asp", "Setup VTU-O DSL", 0, 0, MENU_HIDDEN, LANG_VTUO_SETTINGS},
  #endif /*CONFIG_DSL_VTUO*/
#endif
#ifdef CONFIG_USER_PPPOMODEM
  {"3G Settings", MENU_URL, "wan3gconf.asp", "Setup 3G WAN", 0, 0, MENU_DISPLAY, LANG_3G_SETTINGS},
#endif //CONFIG_USER_PPPOMODEM
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD) || defined(CONFIG_NET_IPIP)
  {"VPN", MENU_FOLDER, &childmenu_vpn, "", sizeof(childmenu_vpn)/sizeof(struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_VPN},
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD || CONFIG_NET_IPIP
  {0, 0, 0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_service[] = {
#ifdef CONFIG_USER_DHCP_SERVER
  //{"DHCP Mode", MENU_URL, "dhcpmode.asp", "DHCP Mode Configuration", 0, 0, MENU_DISPLAY},
#ifdef IMAGENIO_IPTV_SUPPORT
  {"DHCP", MENU_URL, "dhcpd_sc.asp", "DHCP Configuration", 0, 0, MENU_DISPLAY, LANG_DHCP},
#else
  {"DHCP", MENU_URL, "dhcpd.asp", "DHCP Configuration", 0, 0, MENU_DISPLAY, LANG_DHCP},
#endif
#endif
#ifdef CONFIG_USER_VLAN_ON_LAN
  {"VLAN on LAN", MENU_URL, "vlan_on_lan.asp", "VLAN on LAN Configuration", 0, 0, MENU_DISPLAY, LANG_VLAN_ON_LAN},
#endif
#ifndef CONFIG_SFU
  {"DNS", MENU_FOLDER, &childmenu_dns, "",
   sizeof (childmenu_dns) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_DNS},
  {"Firewall", MENU_FOLDER, &childmenu_fw, "",
   sizeof (childmenu_fw) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_FIREWALL},
#endif
#if defined(CONFIG_USER_IGMPPROXY)&&!defined(CONFIG_IGMPPROXY_MULTIWAN)
  {"IGMP Proxy", MENU_URL, "igmproxy.asp", "IGMP Proxy Configuration", 0, 0, MENU_DISPLAY, LANG_IGMP_PROXY},
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
  {"UPnP", MENU_URL, "upnp.asp", "UPnP Configuration", 0, 0, MENU_DISPLAY, LANG_UPNP},
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
  {"RIP", MENU_URL, "rip.asp", "RIP Configuration", 0, 0, MENU_DISPLAY, LANG_RIP},
#endif
#ifdef WEB_REDIRECT_BY_MAC
  {"Landing Page", MENU_URL, "landing.asp", "Landing Page Configuration", 0, 0, MENU_DISPLAY, LANG_LANDING_PAGE},
#endif
#if defined(CONFIG_USER_MINIDLNA)
	{"DMS", MENU_URL, "dms.asp", "DMS Configuration", 0, 0, MENU_DISPLAY, LANG_DMS},
#endif
#ifdef CONFIG_USER_SAMBA
  {"Samba", MENU_URL, "samba.asp", "Samba Configuration", 0, 0, MENU_DISPLAY, LANG_SAMBA},
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_adv[] = {
#ifdef CONFIG_RTL9601B_SERIES
  {"VLAN Settings", MENU_URL, "vlan.asp", "VLAN Settings", 0, 0, MENU_DISPLAY, LANG_VLAN_SETTINGS},
#endif
  {"ARP Table", MENU_URL, "arptable.asp", "ARP Table", 0, 0, MENU_DISPLAY, LANG_ARP_TABLE},
#ifndef CONFIG_SFU
  {"Bridging", MENU_URL, "bridging.asp", "Bridge Configuration", 0, 0, MENU_DISPLAY, LANG_BRIDGING},
#endif
#ifdef ROUTING
  {"Routing", MENU_URL, "routing.asp", "Routing Configuration", 0, 0, MENU_DISPLAY, LANG_ROUTING},
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
  {"SNMP", MENU_URL, "snmp.asp", "SNMP Protocol Configuration", 0, 0, MENU_DISPLAY, LANG_SNMP},
#endif
#ifdef CONFIG_USER_BRIDGE_GROUPING
  {"Bridge Grouping", MENU_URL, "bridge_grouping.asp", "Bridge Grouping Configuration", 0, 0, MENU_DISPLAY, LANG_BRIDGE_GROUPING},
#endif
#ifdef VLAN_GROUP
  {"Port Mapping", MENU_URL, "eth2pvc_vlan.asp", "Port-vlan mapping", 0, 0, MENU_DISPLAY, LANG_PORT_MAPPING},
#endif
#ifdef QOS_DIFFSERV
  {"DiffServ", MENU_URL, "diffserv.asp", "Differentiated Services Setting", 0, 0, MENU_DISPLAY, LANG_DIFFSERV},
#endif
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT) || defined(CONFIG_USER_IP_QOS_3)
  {"IP QoS", MENU_FOLDER, &childmenu_qos, "",
    sizeof(childmenu_qos)/sizeof(struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_IP_QOS},
#endif
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
  {"Link Mode", MENU_URL, "linkmode.asp", "Ethernet Link Mode Setting", 0, 0, MENU_DISPLAY, LANG_LINK_MODE},
#endif
#else
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
	{"Link Mode", MENU_URL, "linkmode_eth.asp", "Ethernet Link Mode Setting", 0, 0, MENU_DISPLAY, LANG_LINK_MODE},
#endif
#endif
#ifdef REMOTE_ACCESS_CTL
  {"Remote Access", MENU_URL, "rmtacc.asp", "Services Access Control", 0, 0, MENU_DISPLAY, LANG_REMOTE_ACCESS},
#endif
#ifdef CONFIG_USER_CUPS
  {"Print Server", MENU_URL, "printServer.asp", "Printer URL(s)", 0, 0, MENU_DISPLAY, LANG_PRINT_SERVER},
#endif //CONFIG_USER_CUPS
#ifdef IP_PASSTHROUGH
  {"Others", MENU_URL, "others.asp", "Other advanced Configuration", 0, 0, MENU_DISPLAY, LANG_OTHERS},
#endif
#ifdef CONFIG_IPV6
  {"IPv6", MENU_FOLDER, &childmenu_ipv6, "",
  sizeof (childmenu_ipv6) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_IPV6},
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_diag[] = {
  {"Ping", MENU_URL, "ping.asp", "Ping Diagnostics", 0, 0, MENU_DISPLAY, LANG_PING},
#ifdef CONFIG_USER_TCPDUMP_WEB
  {"Packet Dump", MENU_URL, "pdump.asp", "Packet Dump Diagnostics", 0, 0, MENU_DISPLAY, LANG_PACKET_DUMP},
#endif
  {ATM_LOOPBACK_STR, MENU_URL, "oamloopback.asp", "ATM Loopback Diagnostics", 0, 0, MENU_HIDDEN, LANG_ATM_LOOPBACK},
  {ADSL_TONE_STR, MENU_URL, "/admin/adsl-diag.asp", "ADSL Tone Diagnostics", 0, 0, MENU_HIDDEN, LANG_DSL_TONE},
#ifdef CONFIG_USER_XDSL_SLAVE
  {ADSL_SLV_TONE_STR, MENU_URL, "/admin/adsl-slv-diag.asp", "ADSL Slave Tone Diagnostics", 0, 0, MENU_HIDDEN, LANG_DSL_SLAVE_TONE},
#endif /*CONFIG_USER_XDSL_SLAVE*/
#ifdef DIAGNOSTIC_TEST
  {ADSL_CONNECTION_STR, MENU_URL, "diag-test.asp", "ADSL Connection Diagnostics", 0, 0, MENU_HIDDEN, LANG_ADSL_CONNECTION},
#endif
#ifdef CONFIG_USER_DOT1AG_UTILS
  {"802.1ag", MENU_FOLDER, &childmenu_dot1ag, "", sizeof (childmenu_dot1ag) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_802_1AG},
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_admin[] = {
#if defined(CONFIG_GPON_FEATURE)
  {GPON_SETTINGS_STR, MENU_URL, "gpon.asp", "Setup GPON", 0, 0, MENU_HIDDEN, LANG_GPON_SETTINGS},
#endif
#if defined(CONFIG_EPON_FEATURE)
  {EPON_SETTINGS_STR, MENU_URL, "epon.asp", "Setup EPON", 0, 0, MENU_HIDDEN, LANG_EPON_SETTINGS},
#endif
#if defined(CONFIG_GPON_FEATURE)
	{"OMCI Info", MENU_URL, "omci_info.asp", "OMCI Info", 0, 0, MENU_DISPLAY, LANG_OMCI_INFO},
#endif
#ifdef CONFIG_INIT_SCRIPTS
  {"Init Scripts", MENU_URL, "init_script.asp", "System initiating scripts", 0, 0, MENU_DISPLAY, LANG_INIT_SCRIPTS},
#endif
#ifdef FINISH_MAINTENANCE_SUPPORT
  {"Finish Maintenance", MENU_URL, "finishmaintenance.asp",
   "Finish Maintenance", 0, 0, MENU_DISPLAY, LANG_FINISH_MAINTENANCE},
#endif
  {"Commit/Reboot", MENU_URL, "reboot.asp", "Commit/reboot the system", 0, 0, MENU_DISPLAY, LANG_COMMIT_REBOOT},
// Added by davian kuo
#ifdef CONFIG_USER_BOA_WITH_MULTILANG
  {"Multi-lingual Settings", MENU_URL, "multi_lang.asp",
   "Multi-language setting", 0, 0, MENU_DISPLAY, LANG_MULTI_LINGUAL_SETTINGS},
#endif
#ifdef CONFIG_SAVE_RESTORE
  {"Backup/Restore", MENU_URL, "saveconf.asp",
   "Backup/restore current settings", 0, 0, MENU_DISPLAY, LANG_BACKUP_RESTORE},
#endif
#ifdef ACCOUNT_LOGIN_CONTROL
  {"Logout", MENU_URL, "/admin/adminlogout.asp", "Logout", 0, 0, MENU_DISPLAY, LANG_LOGOUT},
#endif
#ifdef CONFIG_USER_RTK_SYSLOG
#ifndef SEND_LOG
  {"System Log", MENU_URL, "syslog.asp", "Show system log", 0, 0, MENU_DISPLAY, LANG_SYSTEM_LOG},
#else
  {"System Log", MENU_URL, "syslog_server.asp", "Show system log", 0, 0, MENU_DISPLAY, LANG_SYSTEM_LOG},
#endif
#endif
#ifdef DOS_SUPPORT
  {"DOS", MENU_URL, "dos.asp", "Denial of service", 0, 0, MENU_DISPLAY, LANG_DOS},
#endif
#ifdef ACCOUNT_CONFIG
  {"User Account", MENU_URL, "userconfig.asp", "Setup user account", 0, 0, MENU_DISPLAY, LANG_USER_ACCOUNT},	// Jenny, user account config page
#else
  {"Password", MENU_URL, "password.asp", "Setup access password", 0, 0, MENU_DISPLAY, LANG_PASSWORD},
#endif
#ifdef WEB_UPGRADE
#ifdef UPGRADE_V1
  {"Firmware Upgrade", MENU_URL, "upgrade.asp", "Firmware Upgrade", 0, 0, MENU_DISPLAY, LANG_FIRMWARE_UPGRADE},
#endif // of UPGRADE_V1
#endif // of WEB_UPGRADE
#ifdef IP_ACL
  {"ACL", MENU_URL, "acl.asp", "ACL Configuration", 0, 0, MENU_DISPLAY, LANG_ACL},
#endif
#ifdef TIME_ZONE
  {"Time Zone", MENU_URL, "tz.asp", "Time Zone Configuration", 0, 0, MENU_DISPLAY, LANG_TIME_ZONE},
#endif
#ifdef _CWMP_MIB_
  {"TR-069", MENU_URL, "tr069config.asp", "TR-069 Configuration", 0,
   0, MENU_DISPLAY, LANG_TR_069},
#endif
#ifdef CONFIG_USER_Y1731
  {"EthOAM", MENU_URL, "ethoam.asp", "Ethernet OAM", 0,
   0, MENU_DISPLAY, LANG_ETH_OAM},
#endif 
//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
  {"Logout", MENU_URL, "/admin/logout.asp", "Logout", 0, 0, MENU_DISPLAY, LANG_LOGOUT},
#endif
  {0, 0, 0, 0, 0, 0, 0, 0}
};
struct RootMenu childmenu_statis[] = {
#ifdef CONFIG_SFU
{"Statistics", MENU_URL, "stats.asp", "Display packet statistics", 0, 0, MENU_DISPLAY, LANG_STATISTICS},
#else
  {"Interface", MENU_URL, "stats.asp", "Display packet statistics", 0, 0, MENU_DISPLAY, LANG_INTERFACE},
#endif
  {ADSL_STR, MENU_URL, "/admin/adsl-stats.asp",
#ifdef CONFIG_VDSL
  	"Display DSL statistics",
#else
  	"Display ADSL statistics",
#endif /*CONFIG_VDSL*/
  	0, 0, MENU_HIDDEN, LANG_DSL},

#ifdef CONFIG_DSL_VTUO
  {VTUO_STATUS_STR, MENU_URL, "/admin/vtuo-stats.asp", "Display VTU-O DSL statistics", 0, 0, MENU_HIDDEN, LANG_VTUO_DSL},
#endif /*CONFIG_DSL_VTUO*/

#ifdef CONFIG_USER_XDSL_SLAVE
  {ADSL_SLV_STR, MENU_URL, "/admin/adsl-slv-stats.asp",
#ifdef CONFIG_VDSL
  	"Display DSL Slave statistics",
#else
  	"Display ADSL Slave statistics",
#endif /*CONFIG_VDSL*/
  	0, 0, MENU_HIDDEN, LANG_DSL_SLAVE},
#endif /*CONFIG_USER_XDSL_SLAVE*/

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
  {PON_STR, MENU_URL, "/admin/pon-stats.asp", "Display PON Statistics", 0, 0, MENU_DISPLAY, LANG_PON},
#endif /*CONFIG_DSL_VTUO*/

  {0, 0, 0, 0, 0, 0, 0, 0}
};

//added by eric for VOIP web pages  20070904
#ifdef VOIP_SUPPORT
struct RootMenu childmenu_VoIP[] = {
  {"Port1", MENU_URL, "voip_general.asp?port=0", "Setup VoIP Port 1", 0, 0, MENU_DISPLAY, LANG_PORT1},
#if CONFIG_RTK_VOIP_CON_CH_NUM > 1
  {"Port2", MENU_URL, "voip_general.asp?port=1", "Setup VoIP Port 2", 0, 0, MENU_DISPLAY, LANG_PORT2},
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_FXO
  {"FXO", MENU_URL, "voip_general.asp?port=2", "Configure FXO connection", 0, 0, MENU_DISPLAY, LANG_FXO},
#endif
  {"Tone", MENU_URL, "voip_tone.asp", "Configure SLIC tones", 0, 0, MENU_DISPLAY, LANG_TONE},
  {"Ring", MENU_URL, "voip_ring.asp", "Configure Ring Types of Phone", 0, 0, MENU_DISPLAY, LANG_RING},
  {"Other", MENU_URL, "voip_other.asp", "Other Services", 0, 0, MENU_DISPLAY, LANG_OTHERS},
//  {"Config", MENU_URL, "voip_config.asp", "Configuration Upload/Download", 0, 0, MENU_DISPLAY, LANG_CONFIG},
 {"Network",MENU_URL,"voip_network.asp","Network",0,0,MENU_DISPLAY, LANG_NETWORK},

  {0, 0, 0, 0, 0, 0, 0, 0}
};

#endif

#ifdef OSGI_SUPPORT
struct RootMenu childmenu_osgi[] = {
  {"Framework Info.", MENU_URL, "osgi_fwork.asp", "OSGi Framework Management", 0, 0, MENU_DISPLAY, LANG_FRAMEWORK_INFO},
  {"Bundle Installation", MENU_URL, "osgi_bndins.asp", "Install OSGi Bundles", 0, 0, MENU_DISPLAY, LANG_BUNDLE_INSTALLATION},
  {"Bundle Management", MENU_URL, "osgi_bndmgt.asp", "OSGi Bundles Management", 0, 0, MENU_DISPLAY, LANG_BUNDLE_MANAGEMENT},
  {0, 0, 0, 0, 0, 0, 0, 0}
};
#endif // OSGI_SUPPORT


/*
 *	Root Menu
 */
struct RootMenu rootmenu[] = {
  {"Status", MENU_FOLDER, &childmenu_status, "",
   sizeof (childmenu_status) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_STATUS},
//#ifdef USER_WEB_WIZARD
//  {"Wizard", MENU_URL, "web_wizard_menu.asp", "Setup Wizard", 0, 0, MENU_DISPLAY, LANG_WIZARD},
//#endif
  {"LAN", MENU_URL, "tcpiplan.asp", "Setup LAN Interface", 0, 0, MENU_DISPLAY, LANG_LAN},
#ifdef WLAN_SUPPORT
  {"WLAN", MENU_FOLDER, &childmenu_wlan, "",
   sizeof (childmenu_wlan) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WLAN},
#endif
#ifndef CONFIG_SFU
  {"WAN", MENU_FOLDER, &childmenu_wan, "",
   sizeof (childmenu_wan) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_WAN},
  {"Services", MENU_FOLDER, &childmenu_service, "",
   sizeof (childmenu_service) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_SERVICES},
#else
#ifdef CONFIG_USER_DHCP_SERVER
/*if configure SFU but want have dhcp server*/
  {"Services", MENU_FOLDER, &childmenu_service, "",
   sizeof (childmenu_service) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_SERVICES},
#endif
#endif /*CONFIG_SFU*/   
#ifdef VOIP_SUPPORT
  {"VoIP", MENU_FOLDER, &childmenu_VoIP, "",
   sizeof (childmenu_VoIP) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_VOIP},
#endif
  {"Advance", MENU_FOLDER, &childmenu_adv, "",
   sizeof (childmenu_adv) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_ADVANCE},
  {"Diagnostics", MENU_FOLDER, &childmenu_diag, "",
   sizeof (childmenu_diag) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_DIAGNOSTICS},
  {"Admin", MENU_FOLDER, &childmenu_admin, "",
   sizeof (childmenu_admin) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_ADMIN},
  {"Statistics", MENU_FOLDER, &childmenu_statis, "",
   sizeof (childmenu_statis) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_STATISTICS},
#ifdef OSGI_SUPPORT
  {"OSGi", MENU_FOLDER, &childmenu_osgi, "",
   sizeof (childmenu_osgi) / sizeof (struct RootMenu) - 1, 0, MENU_DISPLAY, LANG_OSGI},
#endif // OSGI_SUPPORT
  {0, 0, 0, 0, 0, 0, 0, 0}
};
#endif // of CONFIG_DEFAULT_WEB

/*
 *	set the member value of menu by name
 *	return 0: member of root or sub- menu is not found.
 *	return 1: member is found and modified by value.
 */
static int
setMemberByName(struct RootMenu *menu, char *name, int nameLen, MENU_MEMBER_T member, void *value)
{
	while(menu->u.addr) {
		if(!strncmp(menu->name, name, nameLen)) {
			switch(member) {
				case MEM_NAME:
					strcpy(menu->name, value);
					break;
				case MEM_HIDDEN:
					menu->hidden = *(int *)value;
					break;
				case MEM_U:
					menu->u.addr = value;
					break;
				case MEM_TYPE:
				case MEM_TIP:
				case MEM_CHILDRENNUMS:
				case MEM_EOL:
				default:
					printf("%s: not implement for this member: %d!\n", __func__, member);
					break;
			}
			return 1;
		}
		else if((menu->type == MENU_FOLDER) && menu->hidden == MENU_DISPLAY) {
			if(setMemberByName((struct RootMenu *)menu->u.addr, name, nameLen, member, value))
				return 1;
		}
		menu++;
	}
	return 0;
}

static int setMenuVisibilityByName(struct RootMenu *menu, char *name, int value)
{
	while(menu->u.addr)
	{
		if(!strcmp(menu->name, name))
		{
			menu->hidden = value;
			return 1;
		}
		else if((menu->type == MENU_FOLDER) && menu->hidden == MENU_DISPLAY)
		{
			if(setMenuVisibilityByName((struct RootMenu *)menu->u.addr, name, value))
				return 1;
		}

		menu++;
	}
	return 0;
}

static void AdminMenuUpdate(struct RootMenu *menu)
{
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	unsigned int pon_mode;

	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) != 0)
	{
		if ( pon_mode == GPON_MODE)
		{
			setMenuVisibilityByName(menu, GPON_SETTINGS_STR, MENU_DISPLAY);
		}
		else if ( pon_mode == EPON_MODE)
		{
			setMenuVisibilityByName(menu, EPON_SETTINGS_STR, MENU_DISPLAY);
		}
	}
#endif
}
static void WANMenuUpdate(struct RootMenu *menu)
{
	int wanmode = WAN_MODE;
	int show_wanmode = 0;

	if((wanmode & MODE_ATM) || (WAN_MODE & MODE_PTM))
	{
		setMenuVisibilityByName(menu, ADSL_SETTINGS_STR, MENU_DISPLAY);
		setMenuVisibilityByName(menu, ADSL_TONE_STR, MENU_DISPLAY);
		setMenuVisibilityByName(menu, ADSL_STR, MENU_DISPLAY);
		#ifdef CONFIG_USER_XDSL_SLAVE
		setMenuVisibilityByName(menu, ADSL_SLV_TONE_STR, MENU_DISPLAY);
		setMenuVisibilityByName(menu, ADSL_SLV_STR, MENU_DISPLAY);
		#endif /*CONFIG_USER_XDSL_SLAVE*/
		setMenuVisibilityByName(menu, ADSL_CONNECTION_STR, MENU_DISPLAY);
		#ifdef CONFIG_DSL_VTUO
		if(VTUOCheck())
		{
			setMenuVisibilityByName(menu, ADSL_SETTINGS_STR, MENU_HIDDEN);
			setMenuVisibilityByName(menu, ADSL_STR, MENU_HIDDEN);
			setMenuVisibilityByName(menu, VTUO_SETTINGS_STR, MENU_DISPLAY);
			setMenuVisibilityByName(menu, VTUO_STATUS_STR, MENU_DISPLAY);
		}
		#endif /*CONFIG_DSL_VTUO*/
	}

	if (wanmode & MODE_ATM) {
		setMenuVisibilityByName(menu, DSLWAN_CONF_STR, MENU_DISPLAY);
		setMenuVisibilityByName(menu, ATM_SETTINGS_STR, MENU_DISPLAY);
		setMenuVisibilityByName(menu, ATM_LOOPBACK_STR, MENU_DISPLAY);
	}

	if (wanmode & MODE_PTM) {
		setMenuVisibilityByName(menu, PTMWAN_CONF_STR, MENU_DISPLAY);
	}

	if (wanmode & MODE_Ethernet) {
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
		setMenuVisibilityByName(menu, PON_CONF_STR, MENU_DISPLAY);
#else
		setMenuVisibilityByName(menu, ETHWAN_CONF_STR, MENU_DISPLAY);
#endif
	}
	
#ifdef WLAN_WISP
	if (wanmode & MODE_Wlan) {
		setMenuVisibilityByName(menu, WLWAN_CONF_STR, MENU_DISPLAY);
	}
#endif

#if	defined (CONFIG_RTL8672_SAR)
	show_wanmode++;	
#endif
#if defined (CONFIG_ETHWAN)
	show_wanmode++;
#endif
#if defined (WLAN_WISP)
	show_wanmode++;
#endif

	if(show_wanmode>1)
		setMenuVisibilityByName(menu, WANMODE_CONF_STR, MENU_DISPLAY);
}

#ifdef CONFIG_RTL_92D_SUPPORT
void
wlanMenuUpdate(struct RootMenu *menu)
{
	unsigned char wlanBand2G5GSelect, wlanBand2G5GSelect_single, phyBand[NUM_WLAN_INTERFACE];
	char buffer[16];
	int hidden, i;

	mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect);
	mib_get(MIB_WLAN_BAND2G5G_SINGLE_SELECT, (void *)&wlanBand2G5GSelect_single);

	for(i=0; i<NUM_WLAN_INTERFACE; i++) {
		wlan_idx = i;
		mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyBand[i]);

		if((phyBand[i] == PHYBAND_5G) && (wlanBand2G5GSelect == BANDMODEBOTH)) {
			sprintf(buffer, "%s %s", getWlanIfName(), "(5GHz)");
			setMemberByName(menu, getWlanIfName(), 5, MEM_NAME, buffer);
			setMemberByName(menu, getWlanIfName(), 5, MEM_U, &childmenu_wlan0);
		}
		else if((phyBand[i] == PHYBAND_2G) && (wlanBand2G5GSelect == BANDMODEBOTH)) {
			sprintf(buffer, "%s %s", getWlanIfName(), "(2.4GHz)");
			setMemberByName(menu, getWlanIfName(), 5, MEM_NAME, buffer);
		}
		else if(wlanBand2G5GSelect == BANDMODESINGLE) {
			setMemberByName(menu, getWlanIfName_web(), 5, MEM_NAME, getWlanIfName_web());
		}

		//if(i != 0) {
		if(wlanBand2G5GSelect == BANDMODEBOTH){
			hidden = MENU_DISPLAY;
			setMemberByName(menu, getWlanIfName(), 5, MEM_HIDDEN, &hidden);
		}
		else if(wlanBand2G5GSelect == BANDMODESINGLE){
			if(i != 0)
				hidden = MENU_HIDDEN;
			else{
				hidden = MENU_DISPLAY;
				if(wlanBand2G5GSelect_single == BANDMODE2G)
					setMemberByName(menu, getWlanIfName_web(), 5, MEM_U, &childmenu_wlan1);
				else
					setMemberByName(menu, getWlanIfName_web(), 5, MEM_U, &childmenu_wlan0);
			}
			setMemberByName(menu, getWlanIfName_web(), 5, MEM_HIDDEN, &hidden);
		}
	}

	return;
}
#endif //CONFIG_RTL_92D_SUPPORT

/*
 * return: number of menu folders
 */
int
calcFolderNum (struct RootMenu *menu, int *maxchild)
{
  int i;
  int num;
  num = 0;
  while (menu->u.addr)
    {
      if ((menu->type == MENU_FOLDER) && (menu->hidden == MENU_DISPLAY))
	{
	  if (menu->childrennums > *maxchild)
	    *maxchild = menu->childrennums;
	  num++;
	  num += calcFolderNum ((struct RootMenu *) menu->u.addr, maxchild);
	}
      menu++;
    }
  return num;
}
int
addMenu (request * wp, struct RootMenu *menu, int layer, int *folder_id,
	 int isMenuEnd)
{
	struct RootMenu *pMenu, *pChild;
	int index;
	int bytes = 0;
	int fid;
	int i;
	unsigned int peol, mask;
	index = 0;
	fid = *folder_id;
	pMenu = menu;
	peol = pMenu->eol;
	// traverse this layer
	while (pMenu->u.addr != 0)
	{
		if(pMenu->hidden == MENU_HIDDEN)
		{
			index++;
			pMenu++;
			continue;
		}
		if (pMenu->type == MENU_FOLDER)
		{
			if (layer == 0)
			{
				bytes +=
				boaWrite (wp,
					   "<div ID='Main%d' onClick=\"expandMenu('Submenu%d','Btn%d', 0); return false\">\n"
					    "<table border=0 cellpadding=0 cellspacing=0>\n",
					   *folder_id, *folder_id, *folder_id);
						bytes += boaWrite (wp, "<tr>");
			}
			else
			{
				bytes +=
				boaWrite (wp,
						"<tr ID='Main%d' onClick=\"expandMenu('Submenu%d','Btn%d', %d); return false\">\n",
						*folder_id, *folder_id, *folder_id,
						pMenu->childrennums);
			}
			for (i = 0; i < layer; i++)
			{
				bytes +=
				boaWrite (wp, "<td width=18 height=18>%s</td>\n",
						(peol & (0x01 << i)) ? "" :
						"<img src=menu-images/menu_bar.gif width=18 height=18>");
			}
	  		// display DIR
			bytes +=
				boaWrite (wp,
						"<td width=18 height=18><img src=menu-images/%s width=18 height=18></td>\n",
						(pMenu +
						1)->u.addr ? "menu_tee.gif" : "menu_corner.gif");
			bytes +=
				boaWrite (wp,
						"<td width=18 height=18><img src=menu-images/menu_folder_closed.gif id='Btn%d' width=18 height=18></td>\n",
						*folder_id);
			bytes +=
	    		boaWrite (wp,
						"<td height=18 colspan=%d><a href=\"\"><span class=\"link\">%s</span></a></td>\n",
//						3 - layer, multilang(pMenu->name));
						3 - layer, multilang(pMenu->lang_tag));
			if (layer == 0)
				bytes += boaWrite (wp, "</tr>\n</table></div>\n");
			else
				bytes += boaWrite (wp, "</tr>\n");
			// expanding ....
			bytes +=
				boaWrite (wp,
						"<div ID='Submenu%dChild' style=\"display:none\">\n",
						*folder_id);
			if (layer == 0)
				bytes +=
					boaWrite (wp,
						"<table border=0 cellpadding=0 cellspacing=0>\n");
			//if (layer != 0 || index != 0)
			*folder_id = *folder_id + 1;
			pMenu->eol = peol;	// copy the eol history
			// calculate my eol bit
			// clear bit
			mask = 0xffffffff ^ (0x01 << layer);
			pMenu->eol &= mask;
			// add bit
			if ((pMenu + 1)->u.addr == 0)	// It's EoL
				pMenu->eol |= (0x01 << layer);
			pChild = (struct RootMenu *) pMenu->u.addr;
			pChild->eol = pMenu->eol;	// deliver eol to the succeed
			if ((pMenu + 1)->u.addr)
				addMenu (wp, pChild, layer + 1, folder_id, 0);
			else
				addMenu (wp, pChild, layer + 1, folder_id, 1);
			if (layer == 0)
				bytes += boaWrite (wp, "</table>");
	  		bytes += boaWrite (wp, "</div>\n");
		}
		else if (pMenu->type == MENU_URL)
		{
			if (layer == 0)
			{
				bytes += boaWrite (wp, "<div ID='Menu%dChild'>\n", index);
				bytes +=
					boaWrite (wp,
							"<table border=0 cellpadding=0 cellspacing=0>\n");
				bytes += boaWrite (wp, "<tr>");
	    	}
			else
			{
				if (layer != 1)
					bytes +=
						boaWrite (wp, "<tr ID='Submenu%dChild%d'>\n", fid - 1,
								index);
				else
					bytes += boaWrite (wp, "<tr>\n");
			}
			for (i = 0; i < layer; i++)
			{
				bytes +=
					boaWrite (wp, "<td width=18 height=18>%s</td>\n",
							(peol & (0x01 << i)) ? "" :
							"<img src=menu-images/menu_bar.gif width=18 height=18>");
			}
			// display URL
			bytes +=
					boaWrite (wp,
							"<td width=18 height=18><img src=menu-images/%s width=18 height=18></td>\n",
							(pMenu +
							1)->u.addr ? "menu_tee.gif" : "menu_corner.gif");
			bytes +=
					boaWrite (wp,
							"<td width=18 height=18><img src=menu-images/menu_link_default.gif width=18 height=18></td>\n");
			bytes +=
					boaWrite (wp,
							"<td height=18 colspan=%d><a href=%s target=\"view\" class=\"link\"><span title=\"%s\" class=\"link\">%s</span></a></td></tr>\n",
//							3 - layer, pMenu->u.url, pMenu->tip, multilang(pMenu->name));
							3 - layer, pMenu->u.url, pMenu->tip, multilang(pMenu->lang_tag));
			if (layer == 0)
				bytes += boaWrite (wp, "</table></div>\n");
		}
		index++;
		pMenu++;
	}				// of while()
	return bytes;
}

int addMenuHead (request * wp, char *menuname, int index, unsigned char isEnd)
{
  int bytes = 0;
  bytes += boaWrite (wp,
  	"<div ID='Main%d' onClick=\"expandMenu('Submenu%d','Btn%d','menuChildTable%d'); return false\" >\n"
		"<table  border=0 cellpadding=0 cellspacing=0>\n <tr>\n"
		" <td width=18 height=18><img src=\"menu-images/%s\" width=18 height=18></td>\n"
		" <td width=18 height=18><img src=menu-images/menu_folder_closed.gif id='Btn%d'  width=18 height=18></td>\n"
		"<td  height=18 colspan=3><a href=\"\"><span class=\"link\">%s</span></a></td>\n</tr>\n</table></div>\n",
	       index, index, index, index,
	       ((isEnd) ? "menu_corner.gif" : "menu_tee.gif"), index,
	       menuname);

  bytes += boaWrite (wp, "<div ID='Submenu%dChild' class='Child' >\n"
			     "<table   border=0 cellpadding=0 cellspacing=0 style=\"display:none\" id=\"menuChildTable%d\">\n",
		      index, index);
  return bytes;
}

int addinMenuHead (request * wp, char *menuname, int index, unsigned char nums,
	       unsigned char isChildEnd)
{
  int bytes = 0;
  bytes += boaWrite (wp,
  	"<tr  ID='Main%d' onClick=\"onlyexpandMenu('Submenu%d','Btn%d','pbtn%d',%d,%d); return false\">\n"
		" <td width=18 height=18><img src=menu-images/menu_bar.gif width=18 height=18></td>\n",
	       index, index, index, index, nums, isChildEnd);

  bytes += boaWrite (wp,
  	"<td width=18 height=18><img src=\"menu-images/%s\" width=18 height=18 id='pbtn%d'></td>\n"
		"<td height=18 width=18><img src=menu-images/menu_folder_closed.gif width=18 height=18 id='Btn%d' ></td>\n"
		" <td height=18  colspan=2><span class=\"link\">%s</span></td>\n </tr>\n",
	       ((isChildEnd) ? "menu_corner_plus.gif" : "menu_tee_plus.gif"),
	       index, index, menuname);

  return bytes;
}

int addMenuTail (request * wp)
{
  return boaWrite (wp, "</table></div>\n");
}

int addChildMenu (request * wp, char *menuname, char *url, char *tip,
	      unsigned char isEnd, unsigned char isRootMenuEnd)
{
  return boaWrite (wp, "<tr><td width=18 height=18>%s</td>\n"
			   "<td width=18 height=18><img src=\"menu-images/%s\" width=18 height=18></td>\n<td width=18 height=18><img src=menu-images/menu_link_default.gif width=18 height=18></td>\n"
			   "<td  height=18 colspan=2><a href=%s target=\"view\" class=\"link\"><span title=\"%s\" class=\"link\">%s</span></a></td></tr>\n",
		    ((isRootMenuEnd) ? "&nbsp&nbsp" :
		     "<img src=\"menu-images/menu_bar.gif\" width=18 height=18>"),
		    ((isEnd) ? "menu_corner.gif" : "menu_tee.gif"), url, tip,
		    menuname);
}

int addInChildMenu (request * wp, char *menuname, char *url, char *tip, int index,
		int IdIndex, unsigned char isEnd, unsigned char isChildEnd)
{
  int nBytes = 0;
  if (index == 0)
    nBytes +=
      boaWrite (wp, "<div ID='Submenu%dChild' class='Child'>\n",
		 IdIndex);
  nBytes =
    boaWrite (wp,
	       "<tr ID='Submenu%dChild%d' style=\"display:none\">"
		  "<td width=18 height=18><img src=menu-images/menu_bar.gif width=18 height=18></td>\n"
		  "<td width=18 height=18>%s</td>\n"
		  "<td width=18 height=18><img src=menu-images/%s width=18 height=18></td>\n"
		  "<td width=18 height=18><img src=menu-images/menu_link_default.gif width=18 height=18></td>\n"
		  "<td ><a href=%s target=\"view\" class=\"link\"><span title=\"%s\" >%s</span></a></td></tr>\n",
	       IdIndex, index,
	       ((isChildEnd) ? "&nbsp&nbsp" :
		"<img src=menu-images/menu_bar.gif width=18 height=18>"),
	       ((isEnd) ? "menu_corner.gif" : "menu_tee.gif"), url, tip,
	       menuname);
  if (isEnd)
    boaWrite (wp, "</div>\n");
  return nBytes;
}
int
createMenu (int eid, request * wp, int argc, char ** argv)
{
  int i = 0, totalIdNums = 0, maxchildrensize = 0;
  int IdIndex = 0;
  unsigned char isRootMenuEnd = 0;

  WANMenuUpdate(rootmenu);
  AdminMenuUpdate(rootmenu);
#ifdef CONFIG_RTL_92D_SUPPORT
  wlanMenuUpdate(rootmenu);
  wlanMenuUpdate(rootmenu_user);
#endif //CONFIG_RTL_92D_SUPPORT
  //calc the id nums and the max children size
  totalIdNums = calcFolderNum (rootmenu, &maxchildrensize);
  //product the js code
  addMenuJavaScript (wp, totalIdNums, maxchildrensize);
  //create the header
  /* add by yq_zhou 09.2.02 add sagem logo for 11n*/
#ifdef CONFIG_11N_SAGEM_WEB
  boaWrite (wp, "<body  onload=\"initIt()\" bgcolor=\"#FFFFFF\" >\n");
#else
  boaWrite (wp, "<body  onload=\"initIt()\" bgcolor=\"#000000\" >\n");
#endif
  boaWrite (wp,
	     "<table width=100%% border=0 cellpadding=0 cellspacing=0>\n<tr><td width=100%% align=left>\n");
  boaWrite (wp, "<table border=0 cellpadding=0 cellspacing=0>\n"
		    "<tr><td width=18 height=18><img src=menu-images/menu_root.gif width=18 height=18></td>\n"
		    "<td height=18 colspan=4 class=link><font size=3>%s:</font></td></tr>\n</table>\n",
	     multilang(LANG_SITE_CONTENTS));
  if (rootmenu[1].u.addr)
    addMenu (wp, &rootmenu[0], 0, &IdIndex, 0);
  else
    addMenu (wp, &rootmenu[0], 0, &IdIndex, 1);
  boaWrite (wp, "</td></tr>\n</table>\n");
  return 0;
}
