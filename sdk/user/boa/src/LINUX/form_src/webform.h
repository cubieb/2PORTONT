/*
 *      Include file of form handler
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */


#ifndef _INCLUDE_WEBFORM_H
#define _INCLUDE_WEBFORM_H

#include <stdlib.h>
#include <string.h>
#include "../globals.h"
#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
typedef enum {false = 0, true = 1} bool;
#endif

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../include/linux/autoconf.h"
#endif
#include "options.h"
#include "mib.h"
#include "multilang.h"

#include "../defs.h"

#ifdef __i386__
  #define _CONFIG_SCRIPT_PATH "."
  #define _LITTLE_ENDIAN_
#else
  #define _CONFIG_SCRIPT_PATH "/bin"
#endif

//#define _UPMIBFILE_SCRIPT_PROG  "mibfile_run.sh"
#define _CONFIG_SCRIPT_PROG "init.sh"
#define _WLAN_SCRIPT_PROG "wlan.sh"
#define _PPPOE_SCRIPT_PROG "pppoe.sh"
#define _FIREWALL_SCRIPT_PROG	"firewall.sh"
#define _PPPOE_DC_SCRIPT_PROG	"disconnect.sh"

static int __inline__ string_to_hex(char *string, unsigned char *key, int len)
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

static int __inline__ string_to_dec(char *string, int *val)
{
	int i;
	int len = strlen(string);

	for (i=0; i<len; i++)
	{
		if (!isdigit(string[i]))
			return 0;
	}

	*val = strtol(string, (char**)NULL, 10);
	return 1;
}

//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
#define ERR_MSG2(msg) { \
	boaHeader(wp); \
	boaWrite(wp, "<body><blockquote><form>\n"); \
	boaWrite(wp, "<TABLE width=\"100%%\">\n"); \
	boaWrite(wp, "<TR><TD align = middle><h4>%s</h4></TD></TR>\n",msg); \
	boaWrite(wp, "<TR><TD align = middle><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></TD></TR>"); \
	boaWrite(wp, "</TABLE></form></blockquote></body>\n"); \
	boaFooter(wp); \
	boaDone(wp, 200); \
}
#endif

#define ERR_MSG(msg) { \
	boaHeader(wp); \
	boaWrite(wp, "<meta http-equiv=Content-Type content=text/html charset=utf-8>\n"); \
	boaWrite(wp, "<body><blockquote><h4>%s</h4>\n", msg); \
	boaWrite(wp, "<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>"); \
	boaFooter(wp); \
	boaDone(wp, 200); \
}

#define OK_MSG(url) { \
	boaHeader(wp); \
	boaWrite(wp, "<meta http-equiv=Content-Type content=text/html charset=utf-8>\n"); \
	boaWrite(wp, "<body><blockquote><h4>%s</h4>\n", multilang(LANG_CHANGE_SETTING_SUCCESSFULLY)); \
	if (url[0]) boaWrite(wp, "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", url);\
	else boaWrite(wp, "<form><input type=button value=\"  OK  \" OnClick=window.close()></form></blockquote></body>");\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

#define OK_MSG1(msg, url) { \
	boaHeader(wp); \
	boaWrite(wp, "<meta http-equiv=Content-Type content=text/html charset=utf-8>\n"); \
	boaWrite(wp, "<body><blockquote><h4>%s</h4>\n", msg); \
	if (url) boaWrite(wp, "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", url);\
	else boaWrite(wp, "<form><input type=button value=\"  OK  \" OnClick=window.close()></form></blockquote></body>");\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

#define APPLY_COUNTDOWN_TIME 5
#define OK_MSG_FW(msg, url, c, ip) { \
	boaHeader(wp); \
	boaWrite(wp, "<head><script language=JavaScript><!--\n");\
	boaWrite(wp, "<meta http-equiv=Content-Type content=text/html charset=utf-8>\n"); \
	boaWrite(wp, "var count = %d;function get_by_id(id){with(document){return getElementById(id);}}\n", c);\
	boaWrite(wp, "function do_count_down(){get_by_id(\"show_sec\").innerHTML = count\n");\
	boaWrite(wp, "if(count == 0) {parent.location.href='http://%s/'; return false;}\n", ip);\
	boaWrite(wp, "if (count > 0) {count--;setTimeout('do_count_down()',1000);}}");\
	boaWrite(wp, "//-->\n");\
	boaWrite(wp,"</script></head>");\
	boaWrite(wp, "<body onload=\"do_count_down();\"><blockquote><h4>%s</h4>\n", msg);\
	boaWrite(wp, "<P align=left><h4>Please wait <B><SPAN id=show_sec></SPAN></B>&nbsp;seconds ...</h4></P>");\
	boaWrite(wp, "</blockquote></body>");\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

// Added by davian kuo
extern void langSel(request * wp, char *path, char *query);

/* Routines exported in fmget.c */
extern int check_display(int eid, request * wp, int argc, char **argv);
// Kaohj
extern int checkWrite(int eid, request * wp, int argc, char **argv);
extern int initPage(int eid, request * wp, int argc, char **argv);
#ifdef WLAN_QoS
extern void ShowWmm(int eid, request * wp, int argc, char **argv);
#endif
extern void write_wladvanced(int eid, request * wp, int argc, char **argv);
extern int getInfo(int eid, request * wp, int argc, char **argv);
extern int getNameServer(int eid, request * wp, int argc, char **argv);
extern int getDefaultGW(int eid, request * wp, int argc, char **argv);

#ifdef CONFIG_IPV6
extern int getDefaultGW_ipv6(int eid, request * wp, int argc, char **argv);
#endif

extern int srvMenu(int eid, request * wp, int argc, char **argv);

extern void formUploadWapiCert1(request * wp, char * path, char * query);
extern void formUploadWapiCert2(request * wp, char * path, char * query);
extern void formWapiReKey(request * wp, char * path, char * query);


/* Routines exported in fmtcpip.c */
extern void formTcpipLanSetup(request * wp, char *path, char *query);
#ifdef CONFIG_USER_VLAN_ON_LAN
extern void formVLANonLAN(request * wp, char *path, char *query);
#endif
extern int lan_setting(int eid, request * wp, int argc, char **argv);
extern int checkIP2(int eid, request * wp, int argc, char **argv);
extern int lan_script(int eid, request * wp, int argc, char **argv);
extern int lan_port_mask(int eid, request * wp, int argc, char **argv);
#ifdef WEB_REDIRECT_BY_MAC
void formLanding(request * wp, char *path, char *query);
#endif
//#ifdef CONFIG_USER_PPPOMODEM
extern int wan3GTable(int eid, request * wp, int argc, char **argv);
extern int fm3g_checkWrite(int eid, request * wp, int argc, char **argv);
extern void form3GConf(request * wp, char *path, char *query);

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
extern void formStatus_pon(request * wp, char *path, char *query);
extern int ponGetStatus(int eid, request * wp, int argc, char **argv);
#ifdef CONFIG_GPON_FEATURE
extern int showgpon_status(int eid, request * wp, int argc, char **argv);
extern int fmgpon_checkWrite(int eid, request * wp, int argc, char **argv);
extern void formgponConf(request * wp, char *path, char *query);
#endif

#ifdef CONFIG_EPON_FEATURE
extern int fmepon_checkWrite(int eid, request * wp, int argc, char **argv);
extern void formeponConf(request * wp, char *path, char *query);
extern int showepon_LLID_status(int eid, request * wp, int argc, char **argv);
extern int showepon_LLID_MAC(int eid, request * wp, int argc, char **argv);
extern void formepon_llid_mac_mapping(request * wp, char *path, char *query);
#endif
#endif
#ifdef CONFIG_GPON_FEATURE
extern int fmvlan_checkWrite(int eid, request * wp, int argc, char **argv);
extern void formVlan(request * wp, char *path, char *query);
extern int omciVlanInfo(int eid, request * wp, int argc, char **argv);
extern int showOMCI_OLT_mode(int eid, request * wp, int argc, char **argv);
extern int fmOmciInfo_checkWrite(int eid, request * wp, int argc, char **argv);
extern void formOmciInfo(request * wp, char *path, char *query);
#endif

//#endif //CONFIG_USER_PPPOMODEM
extern int wanPPTPTable(int eid, request * wp, int argc, char **argv);
extern int wanL2TPTable(int eid, request * wp, int argc, char **argv);
extern int wanIPIPTable(int eid, request * wp, int argc, char **argv);
#ifdef CONFIG_USER_CUPS
int printerList(int eid, request * wp, int argc, char **argv);
#endif
//#ifdef _CWMP_MIB_
extern void formTR069Config(request * wp, char *path, char *query);
extern void formTR069CPECert(request * wp, char *path, char *query);
extern void formTR069CACert(request * wp, char *path, char *query);
extern void formVersionMod(request * wp, char *path, char *query);
extern void formExportOMCIlog(request * wp, char *path, char *query);
extern void formImportOMCIShell(request *wp, char *path, char *query);
extern int TR069ConPageShow(int eid, request * wp, int argc, char **argv);
extern int portFwTR069(int eid, request * wp, int argc, char **argv);
//#endif

/* Routines exported in fmfwall.c */
#ifdef PORT_FORWARD_GENERAL
extern void formPortFw(request * wp, char *path, char *query);
#endif
#ifdef NATIP_FORWARDING
extern void formIPFw(request * wp, char *path, char *query);
#endif
#ifdef PORT_TRIGGERING
extern void formGaming(request * wp, char *path, char *query);
#endif
#if defined(IP_PORT_FILTER) || defined(MAC_FILTER)
extern void formFilter(request * wp, char *path, char *query);
#endif
#ifdef LAYER7_FILTER_SUPPORT
extern int AppFilterList(int eid, request * wp, int argc, char **argv);
extern void formLayer7(request * wp, char *path, char *query);
#endif
#ifdef PARENTAL_CTRL
extern void formParentCtrl(request * wp, char *path, char *query);
extern int parentalCtrlList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef DMZ
extern void formDMZ(request * wp, char *path, char *query);
#endif
#ifdef PORT_FORWARD_GENERAL
extern int portFwList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef NATIP_FORWARDING
extern int ipFwList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef IP_PORT_FILTER
extern int ipPortFilterList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef MAC_FILTER
extern int macFilterList(int eid, request * wp, int argc, char **argv);
#endif

/* Routines exported in fmmgmt.c */
extern void formPasswordSetup(request * wp, char *path, char *query);
extern void formUserPasswordSetup(request * wp, char *path, char *query);
#ifdef ACCOUNT_CONFIG
extern void formAccountConfig(request * wp, char *path, char *query);
extern int accountList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef WEB_UPGRADE
extern void formUpload(request * wp, char * path, char * query);
#ifdef CONFIG_DOUBLE_IMAGE
extern void formStopUpload(request * wp, char * path, char * query);
#endif
#endif
extern void formSaveConfig(request * wp, char *path, char *query);
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
extern void formSnmpConfig(request * wp, char *path, char *query);
#endif
#ifdef CONFIG_USER_RADVD
extern void formRadvdSetup(request * wp, char *path, char *query);
#endif
extern void formAdslDrv(request * wp, char *path, char *query);
//#ifdef CONFIG_DSL_VTUO
extern void formSetVTUO(request * wp, char *path, char *query);
extern int vtuo_checkWrite(int eid, request * wp, int argc, char **argv);
//#endif /*CONFIG_DSL_VTUO*/
extern void formSetAdsl(request * wp, char *path, char *query);
#ifdef FIELD_TRY_SAFE_MODE
extern void formAdslSafeMode(request * wp, char *path, char *query);
#endif
extern void formDiagAdsl(request * wp, char *path, char *query);
extern void formSetAdslTone(request * wp, char *path, char *query);
extern void formSetAdslPSD(request * wp, char *path, char *query);
extern void formSetAdslPSD(request * wp, char *path, char *query);
extern void formStatAdsl(request * wp, char *path, char *query);
extern void formStats(request * wp, char *path, char *query);
extern void formRconfig(request * wp, char *path, char *query);
extern void formSysCmd(request * wp, char *path, char *query);
extern int sysCmdLog(int eid, request * wp, int argc, char **argv);
#ifdef CONFIG_USER_RTK_SYSLOG
extern void formSysLog(request * wp, char *path, char *query);
extern int sysLogList(int eid, request * wp, int argc, char **argv);
extern void RemoteSyslog(int eid, request * wp, int argc, char **argv);
#endif
#ifdef DOS_SUPPORT
extern void formDosCfg(request * wp, char *path, char *query);
#endif
#if 0
extern int adslDrvSnrTblGraph(int eid, request * wp, int argc, char **argv);
extern int adslDrvSnrTblList(int eid, request * wp, int argc, char **argv);
extern int adslDrvBitloadTblGraph(int eid, request * wp, int argc, char **argv);
extern int adslDrvBitloadTblList(int eid, request * wp, int argc, char **argv);
#endif
//#ifdef CONFIG_VDSL
int vdslBandStatusTbl(int eid, request * wp, int argc, char **argv);
//#endif /*CONFIG_VDSL*/
extern int adslToneDiagTbl(int eid, request * wp, int argc, char **argv);
extern int adslToneDiagList(int eid, request * wp, int argc, char **argv);
extern int adslToneConfDiagList(int eid, request * wp, int argc, char **argv);
extern int adslPSDMaskTbl(int eid, request * wp, int argc, char **argv);
extern int adslPSDMeasure(int eid, request * wp, int argc, char **argv);
extern int pktStatsList(int eid, request * wp, int argc, char **argv);


/* Routines exported in fmatm.c */
extern int atmVcList(int eid, request * wp, int argc, char **argv);
extern int atmVcList2(int eid, request * wp, int argc, char **argv);
extern int wanConfList(int eid, request * wp, int argc, char **argv);
extern int DHCPSrvStatus(int eid, request *wp, int argc, char **argv);
extern int DSLStatus(int eid, request * wp, int argc, char **argv);
extern int DSLVer(int eid, request * wp, int argc, char **argv);
#ifdef CONFIG_IPV6
extern int wanip6ConfList(int eid, request * wp, int argc, char **argv);
#endif
extern void formWanMode(request * wp, char *path, char *query);
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
extern void formEth(request * wp, char *path, char *query);
#endif
#ifdef CONFIG_RTL8672_SAR
extern void formAtmVc(request * wp, char *path, char *query);
extern void formAdsl(request * wp, char *path, char *query);
#endif
extern void formPPPEdit(request * wp, char *path, char *query);
extern void formIPEdit(request * wp, char *path, char *query);
extern void formBrEdit(request * wp, char *path, char *query);
extern void formStatus(request * wp, char *path, char *query);
#if defined(CONFIG_RTL_8676HWNAT)
extern void formLANPortStatus(request * wp, char *path, char *query);
extern void showLANPortStatus(int eid, request * wp, int argc, char **argv);
#endif
#ifdef CONFIG_IPV6
extern void formStatus_ipv6(request * wp, char *path, char *query);
#endif
extern void formDate(request * wp, char *path, char *query);
/* Routines exported in fmbridge.c */
extern void formBridge(request * wp, char *path, char *query);
extern void formRefleshFdbTbl(request * wp, char *path, char *query);
extern int bridgeFdbList(int eid, request * wp, int argc, char **argv);
extern int ARPTableList(int eid, request * wp, int argc, char **argv);

/* Routines exported in fmroute.c */
#ifdef ROUTING
extern void formRoute(request * wp, char *path, char *query);
extern int showStaticRoute(int eid, request * wp, int argc, char **argv);
#endif
#ifdef CONFIG_IPV6
extern void formIPv6EnableDisable(request * wp, char *path, char *query);
extern void formIPv6Route(request * wp, char *path, char *query);
extern void formFilterV6(request * wp, char *path, char *query);
extern int ipPortFilterListV6(int eid, request * wp, int argc, char **argv);
#endif
#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
extern void formRip(request * wp, char *path, char *query);
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
extern int showRipIf(int eid, request * wp, int argc, char **argv);
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
extern int showOspfIf(int eid, request * wp, int argc, char **argv);
#endif
extern void ShowDefaultGateway(int eid, request * wp, int argc, char **argv);
extern void GetDefaultGateway(int eid, request * wp, int argc, char **argv);
extern void DisplayDGW(int eid, request * wp, int argc, char **argv);
extern void DisplayTR069WAN(int eid, request * wp, int argc, char **argv);
#ifdef CONFIG_IPV6
extern int showIPv6StaticRoute(int eid, request * wp, int argc, char **argv);
extern void formIPv6RefleshRouteTbl(request * wp, char *path, char *query);
extern int routeIPv6List(int eid, request * wp, int argc, char **argv);
#endif
extern void formRefleshRouteTbl(request * wp, char *path, char *query);
extern int routeList(int eid, request * wp, int argc, char **argv);
#ifdef CONFIG_USER_RTK_WAN_CTYPE
extern void ShowConnectionType(int eid, request * wp, int argc, char **argv);
#endif
extern void ShowIpProtocolType(int eid, request * wp, int argc, char **argv);
extern void ShowIPV6Settings(int eid, request * wp, int argc, char **argv);
extern void ShowDSLiteSetting(int eid, request * wp, int argc, char **argv);
extern void ShowPortMapping(int eid, request * wp, int argc, char **argv);
extern void ShowPortBaseFiltering(int eid, request * wp, int argc, char **argv);
extern void Show6rdSetting(int eid, request * wp, int argc, char **argv);
#ifdef WLAN_WISP
extern void ShowWispWanItf(int eid, request * wp, int argc, char **argv);
extern void initWispWanItfStatus(int eid, request * wp, int argc, char **argv);
#endif

/* Routines exported in fmdhcpd.c */
extern void formDhcpd(request * wp, char *path, char *query);
extern void formReflashClientTbl(request * wp, char *path, char *query);
extern int dhcpClientList(int eid, request * wp, int argc, char **argv);

#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
extern void formDhcpv6(request * wp, char *path, char *query);
extern int showDhcpv6SNameServerTable(int eid, request * wp, int argc, char **argv);
extern int showDhcpv6SDOMAINTable(int eid, request * wp, int argc, char **argv);
#endif

/* Routines exported in fmDNS.c */
extern void formDns(request * wp, char *path, char *query);

/* Routines exported in fmDDNS.c */
extern void formDDNS(request * wp, char *path, char *query);
extern int showDNSTable(int eid, request * wp, int argc, char **argv);


/* Routines exported in fmDNS.c */
extern void formDhcrelay(request * wp, char *path, char *query);

#ifdef ADDRESS_MAPPING
extern void formAddressMap(request * wp, char *path, char *query);
#ifdef MULTI_ADDRESS_MAPPING
extern int showMultAddrMappingTable(int eid, request * wp, int argc, char **argv);
#endif // MULTI_ADDRESS_MAPPING
#endif

/* Routines exported in fmping.c */
extern void formPing(request * wp, char *path, char *query);

/* Routines exported in fmcapture.c */
extern void formCapture(request * wp, char *path, char *query);

/* Routines exported in fmoamlb.c */
extern void formOamLb(request * wp, char *path, char *query);

/* Routines exported in fmreboot.c */
extern void formReboot(request * wp, char *path, char *query);
#ifdef FINISH_MAINTENANCE_SUPPORT
//xl_yue added,inform itms that maintenance finished
extern void formFinishMaintenance(request * wp, char *path, char *query);
#endif

#ifdef USE_LOGINWEB_OF_SERVER
//xl_yue added
extern void formLogin(request * wp, char *path, char *query);
extern void formLogout(request * wp, char *path, char *query);
// Kaohj
extern int passwd2xmit(int eid, request * wp, int argc, char **argv);
// davian_kuo
extern void formLoginMultilang(request * wp, char *path, char *query);
#endif


#ifdef ACCOUNT_LOGIN_CONTROL
//xl_yue add
extern void formAdminLogout(request * wp, char *path, char *query);
extern void formUserLogout(request * wp, char *path, char *query);
#endif

/* Routines exported in fmdhcpmode.c */
extern void formDhcpMode(request * wp, char *path, char *query);

/* Routines exported in fmupnp.c */
extern void formUpnp(request * wp, char *path, char *query);
extern int ifwanList_upnp(int eid, request * wp, int argc, char **argv);
extern void formMLDProxy(request * wp, char *path, char *query);				// Mason Yu. MLD Proxy
extern void formMLDSnooping(request * wp, char *path, char *query);    		// Mason Yu. MLD snooping

/* Routines exported in fmdms.c */
#ifdef CONFIG_USER_MINIDLNA
extern int fmDMS_checkWrite(int eid, request * wp, int argc, char **argv);
extern void formDMSConf(request * wp, char *path, char *query);
#endif

/* Routines exported in fmigmproxy.c */
#ifdef CONFIG_USER_IGMPPROXY
extern void formIgmproxy(request * wp, char *path, char *query);
#endif
extern int ifwanList(int eid, request * wp, int argc, char **argv);
#ifdef QOS_TRAFFIC_SHAPING_BY_SSID
extern int ssid_list(int eid, request * wp, int argc, char **argv);
#endif
/* Routines exported in fmothers.c */
#ifdef IP_PASSTHROUGH
extern void formOthers(request * wp, char *path, char *query);
#endif
extern int diagMenu(int eid, request * wp, int argc, char **argv);
extern int adminMenu(int eid, request * wp, int argc, char **argv);

//xl_yue
extern int userAddAdminMenu(int eid, request * wp, int argc, char **argv);

/* Routines exported in fmsyslog.c*/
//extern void formSysLog(request * wp, char *path, char *query);

#ifdef IP_ACL
/* Routines exported in fmacl.c */
extern void formACL(request * wp, char *path, char *query);
extern int showACLTable(int eid, request * wp, int argc, char **argv);
extern int LANACLItem(int eid, request * wp, int argc, char **argv);
extern int WANACLItem(int eid, request * wp, int argc, char **argv);
#endif
#ifdef NAT_CONN_LIMIT
extern int showConnLimitTable(int eid, request * wp, int argc, char **argv);
extern void formConnlimit(request * wp, char *path, char *query);
#endif
#ifdef TCP_UDP_CONN_LIMIT
extern int showConnLimitTable(int eid, request * wp, int argc, char **argv);
extern void formConnlimit(request * wp, char *path, char *query);
#endif

extern void formmacBase(request * wp, char *path, char *query);
#ifdef IMAGENIO_IPTV_SUPPORT
extern void formIpRange(request * wp, char *path, char *query);
#endif
extern int showMACBaseTable(int eid, request * wp, int argc, char **argv);
#ifdef IMAGENIO_IPTV_SUPPORT
extern int showDeviceIpTable(int eid, request * wp, int argc, char **argv);
#endif //#ifdef IMAGENIO_IPTV_SUPPORT

#ifdef URL_BLOCKING_SUPPORT
/* Routines exported in fmurl.c */
extern void formURL(request * wp, char *path, char *query);
extern int showURLTable(int eid, request * wp, int argc, char **argv);
extern int showKeywdTable(int eid, request * wp, int argc, char **argv);
#endif
#ifdef URL_ALLOWING_SUPPORT
extern int showURLALLOWTable(int eid, request * wp, int argc, char **argv);
#endif



#ifdef DOMAIN_BLOCKING_SUPPORT
/* Routines exported in fmdomainblk.c */
extern void formDOMAINBLK(request * wp, char *path, char *query);
extern int showDOMAINBLKTable(int eid, request * wp, int argc, char **argv);
#endif

#ifdef TIME_ZONE
extern void formNtp(request * wp, char *path, char *query);
extern int timeZoneList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef CONFIG_USER_BRIDGE_GROUPING
extern void formBridgeGrouping(request * wp, char *path, char *query);
extern int ifGroupList(int eid, request * wp, int argc, char **argv);
#endif 

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
extern void formLink(request * wp, char *path, char *query);
extern int show_lanport(int eid, request * wp, int argc, char **argv);
#endif
#else // of CONFIG_RTL_MULTI_LAN_DEV
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
extern void formLink(request * wp, char *path, char *query);
#endif
#endif	// of CONFIG_RTL_MULTI_LAN_DEV
//#ifdef IP_QOS
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
extern void formQos(request * wp, char *path, char *query);
extern int default_qos(int eid, request * wp, int argc, char **argv);
extern int qosList(int eid, request * wp, int argc, char **argv);
extern int priority_outif(int eid, request * wp, int argc, char **argv);
extern int confDscp(int eid, request * wp, int argc, char **argv);
#ifdef QOS_DIFFSERV
extern void formDiffServ(request * wp, char *path, char *query);
extern int diffservList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef QOS_DSCP_MATCH
extern int match_dscp(int eid, request * wp, int argc, char **argv);
#endif
extern void formQueueAdd(request * wp, char *path, char *query);
extern int ipQosQueueList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef NEW_IP_QOS_SUPPORT
extern void formQosShape(request * wp, char *path, char *query);
extern int  initTraffictlPage(int eid, request * wp, int argc, char **argv);
#endif
extern int iflanList(int eid, request * wp, int argc, char **argv);
extern int policy_route_outif(int eid, request * wp, int argc, char **argv);

#ifdef CONFIG_8021P_PRIO
extern int setting_1ppriority(int eid, request * wp, int argc, char **argv);
#ifdef NEW_IP_QOS_SUPPORT
extern int setting_predprio(int eid, request * wp, int argc, char **argv);
#endif
#endif
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
extern int pptpList(int eid, request * wp, int argc, char **argv);
extern void formPPtP(request * wp, char *path, char *query);
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_PPTPD_PPTPD
extern int pptpServerList(int eid, request * wp, int argc, char **argv);
#endif
#ifdef CONFIG_USER_L2TPD_L2TPD
extern void formL2TP(request * wp, char *path, char *query);
extern int l2tpList(int eid, request * wp, int argc, char **argv);
#endif //end of CONFIG_USER_L2TPD_L2TPD
#ifdef CONFIG_USER_L2TPD_LNS
extern int l2tpServerList(int eid, request * wp, int argc, char **argv);
#endif

#ifdef CONFIG_XFRM
extern void formIPsec(request * wp, char *path, char *query);
extern int ipsec_wanList(int eid, request * wp, int argc, char **argv);
extern int ipsec_ikePropList(int eid, request * wp, int argc, char **argv);
extern int ipsec_saPropList(int eid, request * wp, int argc, char **argv);
extern int ipsec_infoList(int eid, request * wp, int argc, char **argv);
#endif

#ifdef CONFIG_NET_IPIP
extern void formIPIP(request * wp, char *path, char *query);
extern int ipipList(int eid, request * wp, int argc, char **argv);
#endif //end of CONFIG_NET_IPIP
#ifdef CONFIG_USER_IP_QOS_3
extern void formQosPolicy(request * wp, char *path, char *query);
extern int  initQueuePolicy(int eid, request * wp, int argc, char **argv);
extern void formQosRule(request * wp, char* path, char* query);
extern void formQosRuleEdit(request * wp, char* path, char* query);
extern int  initQosRulePage(int eid, request * wp, int argc, char **argv);
extern int  initRulePriority(int eid, request * wp, int argc, char **argv);
extern int  initQosLanif(int eid, request * wp, int argc, char **argv);
extern int  initOutif(int eid, request * wp, int argc, char **argv);
extern int  initTraffictlPage(int eid, request * wp, int argc, char **argv);
extern void formQosTraffictl(request * wp, char *path, char *query);
extern void formQosTraffictlEdit(request * wp, char *path, char *query);
extern int  ifWanList_tc(int eid, request * wp, int argc, char **argv);
extern int ipqos_dhcpopt(int eid, request * wp, int argc, char **argv);
extern int ipqos_dhcpopt_getoption60(int eid, request * wp, int argc, char **argv);
#endif

extern void formAcc(request * wp, char *path, char *query);
extern int accPost(int eid, request * wp, int argc, char **argv);
extern int accItem(int eid, request * wp, int argc, char **argv);
extern void ShowAutoPVC(int eid, request * wp, int argc, char **argv);	// auto-pvc-search

extern void ShowChannelMode(int eid, request * wp, int argc, char **argv); // China telecom e8-a
extern void ShowBridgeMode(int eid, request * wp, int argc, char **argv); // For PPPoE pass through
extern void ShowPPPIPSettings(int eid, request * wp, int argc, char **argv); // China telecom e8-a
extern void ShowNAPTSetting(int eid, request * wp, int argc, char **argv); // China telecom e8-a
//#ifdef CONFIG_RTL_MULTI_ETH_WAN
extern void initPageWaneth(int eid, request * wp, int argc, char **argv);
//#endif
extern void ShowIGMPSetting(int eid, request * wp, int argc, char **argv); // Telefonica
extern int createMenu(int eid, request * wp, int argc, char **argv);
extern int createMenu_user(int eid, request * wp, int argc, char **argv);

extern void formWanRedirect(request * wp, char *path, char *query);
extern int getWanIfDisplay(int eid, request * wp, int argc, char **argv);
#ifdef WLAN_SUPPORT
extern int wlanMenu(int eid, request * wp, int argc, char **argv);
extern int wlanStatus(int eid, request * wp, int argc, char **argv);
extern int wlan_ssid_select(int eid, request * wp, int argc, char **argv);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
extern void formWlanRedirect(request * wp, char *path, char *query);
#endif
#if defined(CONFIG_RTL_92D_SUPPORT)
extern void formWlanBand2G5G(request * wp, char *path, char *query);
#endif //CONFIG_RTL_92D_SUPPORT
/* Routines exported in fmwlan.c */
extern void formWlanSetup(request * wp, char *path, char *query);
#ifdef WLAN_ACL
extern int wlAcList(int eid, request * wp, int argc, char **argv);
extern void formWlAc(request * wp, char *path, char *query);
#endif
extern void formAdvanceSetup(request * wp, char *path, char *query);
extern int wirelessClientList(int eid, request * wp, int argc, char **argv);
extern void formWirelessTbl(request * wp, char *path, char *query);

#ifdef WLAN_WPA
extern void formWlEncrypt(request * wp, char *path, char *query);
#endif
//WDS
#ifdef WLAN_WDS
extern void formWlWds(request * wp, char *path, char *query);
extern void formWdsEncrypt(request * wp, char *path, char *query);
extern int wlWdsList(int eid, request * wp, int argc, char **argv);
extern int wdsList(int eid, request * wp, int argc, char **argv);
#endif
#if defined(WLAN_CLIENT) || defined(WLAN_SITESURVEY)
extern void formWlSiteSurvey(request * wp, char *path, char *query);
extern int wlSiteSurveyTbl(int eid, request * wp, int argc, char **argv);
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
extern void formWsc(request * wp, char *path, char *query);
#endif
#ifdef WLAN_11R
extern void formFt(request * wp, char *path, char *query);
extern int wlFtKhList(int eid, request * wp, int argc, char **argv);
#endif
extern int wlStatus_parm(int eid, request * wp, int argc, char **argv);
#endif // of WLAN_SUPPORT
extern int oamSelectList(int eid, request * wp, int argc, char **argv);
#ifdef DIAGNOSTIC_TEST
extern void formDiagTest(request * wp, char *path, char *query);	// Diagnostic test
extern int lanTest(int eid, request * wp, int argc, char **argv);	// Ethernet LAN connection test
extern int adslTest(int eid, request * wp, int argc, char **argv);	// ADSL service provider connection test
extern int internetTest(int eid, request * wp, int argc, char **argv);	// Internet service provider connection test
#endif
#ifdef WLAN_MBSSID
extern int getVirtualIndex(int eid, request * wp, int argc, char **argv);
extern void formWlanMultipleAP(request * wp, char *path, char *query);
extern int wirelessVAPClientList(int eid, request * wp, int argc, char **argv);
extern void formWirelessVAPTbl(request * wp, char *path, char *query);
#if 0
extern int postSSID(int eid, request * wp, int argc, char **argv);
extern int postSSIDWEP(int eid, request * wp, int argc, char **argv);
#endif
extern int SSIDStr(int eid, request * wp, int argc, char **argv);
#endif

#ifdef PORT_FORWARD_ADVANCE
extern void formPFWAdvance(request * wp, char *path, char *query);
extern int showPFWAdvTable(int eid, request * wp, int argc, char **argv);
#endif
#ifdef PORT_FORWARD_GENERAL
extern int showPFWAdvForm(int eid, request * wp, int argc, char **argv);
#endif
#ifdef WEB_ENABLE_PPP_DEBUG
extern void ShowPPPSyslog(int eid, request * wp, int argc, char **argv);
#endif
#ifdef USER_WEB_WIZARD
extern void formWebWizardMenu(request * wp, char *path, char *query);
extern void formWebWizard1(request * wp, char *path, char *query);
extern void formWebWizard4(request * wp, char *path, char *query);
extern void formWebWizard5(request * wp, char *path, char *query);
extern void formWebWizard6(request * wp, char *path, char *query);
extern void ShowWebWizardPage(int eid, request * wp, int argc, char **argv);
#endif
extern void DSLuptime(int eid, request * wp, int argc, char **argv);
//#ifdef CONFIG_USER_XDSL_SLAVE
extern void DSLSlvuptime(int eid, request * wp, int argc, char **argv);
//#endif /*CONFIG_USER_XDSL_SLAVE*/
extern int multilang_asp(int eid, request * wp, int argc, char **argv);
extern int WANConditions(int eid, request * wp, int argc, char **argv);
extern void ShowWanMode(int eid, request * wp, int argc, char **argv);
extern void set_user_profile(void);
// Kaohj -- been move to utility.c from fmigmproxy.c
//extern int ifWanNum(char *type);
#ifdef CONFIG_USER_ROUTED_ROUTED
extern int ifRipNum(); // fmroute.c
#endif

extern const char * const BRIDGE_IF;
extern const char * const ELAN_IF;
//extern const char * const WLAN_IF;

extern int g_remoteConfig;
extern int g_remoteAccessPort;
extern int g_rexpire;
extern short g_rSessionStart;

/* constant string */
static const char DOCUMENT[] = "document";
extern int lanSetting(int eid, request * wp, int argc, char **argv);
extern int getPortMappingInfo(int eid, request * wp, int argc, char **argv);
//cathy, MENU_MEMBER_T should be updated if RootMenu structure is modified
typedef enum {
	MEM_NAME,
	MEM_TYPE,
	MEM_U,
	MEM_TIP,
	MEM_CHILDRENNUMS,
	MEM_EOL,
	MEM_HIDDEN
} MENU_MEMBER_T;

typedef enum {
	MENU_DISPLAY,
	MENU_HIDDEN
} MENU_HIDDEN_T;

typedef enum {
	MENU_FOLDER,
	MENU_URL
} MENU_T;

struct RootMenu{
	char  name[128];
	MENU_T type;
	union {
		void *addr;
		void *url;
	} u;
	char tip[128];
	int childrennums;
	int eol;	// end of layer
	int hidden;
	int lang_tag; // Added by davian_kuo, for multi-lingual.
};

#ifdef CONFIG_DEFAULT_WEB
extern struct RootMenu rootmenu[];
extern struct RootMenu rootmenu_user[];
#endif	//CONFIG_DEFAULT_WEB

#if defined(CONFIG_RTL_92D_SUPPORT)
extern void wlanMenuUpdate(struct RootMenu *menu);
#endif //CONFIG_RTL_92D_SUPPORT

//add by ramen for ALG ON-OFF
#ifdef CONFIG_IP_NF_ALG_ONOFF
void formALGOnOff(request * wp, char *path, char *query);
void GetAlgTypes(request * wp);
void initAlgOnOff(request * wp);
void CreatejsAlgTypeStatus(request * wp);
#endif

#ifdef CONFIG_USER_SAMBA
void formSamba(request * wp, char *path, char *query);
#endif

#ifdef CONFIG_INIT_SCRIPTS
extern void formInitStartScript(request * wp, char *path, char *query);
extern void formInitStartScriptDel(request * wp, char *path, char *query);
extern void formInitEndScript(request * wp, char *path, char *query);
extern void formInitEndScriptDel(request * wp, char *path, char *query);
#endif

#ifdef CONFIG_USER_DOT1AG_UTILS
extern int dot1ag_init(int eid, request * wp, int argc, char **argv);
extern void formDot1agConf(request * wp, char *path, char *query);
extern void formDot1agAction(request * wp, char *path, char *query);
extern void dot1agActionRefresh(request * wp, char *path, char *query);
extern int dot1ag_status_init(int eid, request * wp, int argc, char **argv);
#endif

#ifdef CONFIG_APACHE_FELIX_FRAMEWORK
extern int getOSGIInfo(int eid, request * wp, int argc, char **argv);
extern int getOSGIBundleList(int eid, request * wp, int argc, char **argv);
extern void formOsgiUpload(request * wp, char *path, char *query);
extern void formOsgiMgt(request * wp, char *path, char *query);

#endif

extern void formY1731(request * wp, char *path, char *query);

#endif // _INCLUDE_APFORM_H

