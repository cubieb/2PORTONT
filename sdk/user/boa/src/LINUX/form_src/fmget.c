/*
 *      Web server handler routines for get info and index (getinfo(), getindex())
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/sysinfo.h>
#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

#include "../webs.h"
#include "mib.h"
#include "adsl_drv.h"
#include "utility.h"
//added by xl_yue
#include "../defs.h"

// Added by davian kuo
#include "multilang.h"
#include "multilang_set.h"

// remote config status flag: 0: disabled, 1: enabled
int g_remoteConfig=0;
int g_remoteAccessPort=51003;

// Added by Mason Yu
extern char suName[MAX_NAME_LEN];
extern char usName[MAX_NAME_LEN];
// Mason Yu on True
extern unsigned char g_login_username[MAX_NAME_LEN];

#ifdef WLAN_SUPPORT
void translate_control_code(char *buffer)
{
	char tmpBuf[200], *p1 = buffer, *p2 = tmpBuf;


	while (*p1) {
		if (*p1 == '"') {
			memcpy(p2, "&quot;", 6);
			p2 += 6;
		}
		else if (*p1 == '\x27') {
			memcpy(p2, "&#39;", 5);
			p2 += 5;
		}
		else if (*p1 == '\x5c') {
			memcpy(p2, "&#92;", 5);
			p2 += 5;
		}
		else if (*p1 =='<'){
			memcpy(p2, "&lt;", 4);
			p2 += 4;
		}
		else if (*p1 =='>'){
			memcpy(p2, "&gt;", 4);
			p2 += 4;
		}
		else
			*p2++ = *p1;
		p1++;
	}
	*p2 = '\0';

	strcpy(buffer, tmpBuf);
}
void translate_web_code(char *buffer)
{
	char tmpBuf[200], *p1 = buffer, *p2 = tmpBuf;


	while (*p1) {
		if (*p1 == '"') {
			strcpy(p2, "\\\"");
			p2 += 2;
		}
		else if (*p1 == '\'') {
			strcpy(p2, "\\'");
			p2 += 2;
		}
		else if (*p1 == '\\') {
			strcpy(p2, "\\\\");
			p2 += 2;
		}
		else
			*p2++ = *p1;
		p1++;
	}
	*p2 = '\0';

	strcpy(buffer, tmpBuf);
}

#endif

// Kaohj
typedef enum {
	INFO_MIB,
	INFO_SYS
} INFO_T;

typedef struct {
	char *cmd;
	INFO_T type;
	int id;
} web_get_cmd;

typedef struct {
	char *cmd;
	int (*handler)(int , request * , int , char **, char *);
} web_custome_cmd;

web_get_cmd get_info_list[] = {
	{"lan-ip", INFO_MIB, MIB_ADSL_LAN_IP},
	{"lan-subnet", INFO_MIB, MIB_ADSL_LAN_SUBNET},
	{"lan-ip2", INFO_MIB, MIB_ADSL_LAN_IP2},
	{"lan-subnet2", INFO_MIB, MIB_ADSL_LAN_SUBNET2},
	// Kaohj
	#ifndef DHCPS_POOL_COMPLETE_IP
	{"lan-dhcpRangeStart", INFO_MIB, MIB_ADSL_LAN_CLIENT_START},
	{"lan-dhcpRangeEnd", INFO_MIB, MIB_ADSL_LAN_CLIENT_END},
	#else
	{"lan-dhcpRangeStart", INFO_MIB, MIB_DHCP_POOL_START},
	{"lan-dhcpRangeEnd", INFO_MIB, MIB_DHCP_POOL_END},
	#endif
	{"lan-dhcpSubnetMask", INFO_MIB, MIB_DHCP_SUBNET_MASK},
	{"dhcps-dns1", INFO_MIB, MIB_DHCPS_DNS1},
	{"dhcps-dns2", INFO_MIB, MIB_DHCPS_DNS2},
	{"dhcps-dns3", INFO_MIB, MIB_DHCPS_DNS3},
	{"lan-dhcpLTime", INFO_MIB, MIB_ADSL_LAN_DHCP_LEASE},
	{"lan-dhcpDName", INFO_MIB, MIB_ADSL_LAN_DHCP_DOMAIN},
	{"elan-Mac", INFO_MIB, MIB_ELAN_MAC_ADDR},
	{"wlan-Mac", INFO_MIB, MIB_WLAN_MAC_ADDR},
	{"wan-dns1", INFO_MIB, MIB_ADSL_WAN_DNS1},
	{"wan-dns2", INFO_MIB, MIB_ADSL_WAN_DNS2},
	{"wan-dns3", INFO_MIB, MIB_ADSL_WAN_DNS3},
#ifdef CONFIG_USER_DHCP_SERVER
	{"wan-dhcps", INFO_MIB, MIB_ADSL_WAN_DHCPS},
#endif
#ifdef DMZ
	{"dmzHost", INFO_MIB, MIB_DMZ_IP},
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	{"snmpSysDescr", INFO_MIB, MIB_SNMP_SYS_DESCR},
	{"snmpSysContact", INFO_MIB, MIB_SNMP_SYS_CONTACT},
	{"snmpSysLocation", INFO_MIB, MIB_SNMP_SYS_LOCATION},
	{"snmpSysObjectID", INFO_MIB, MIB_SNMP_SYS_OID},
	{"snmpTrapIpAddr", INFO_MIB, MIB_SNMP_TRAP_IP},
	{"snmpCommunityRO", INFO_MIB, MIB_SNMP_COMM_RO},
	{"snmpCommunityRW", INFO_MIB, MIB_SNMP_COMM_RW},
	{"name", INFO_MIB, MIB_SNMP_SYS_NAME},
#endif
	{"snmpSysName", INFO_MIB, MIB_SNMP_SYS_NAME},
	{"name", INFO_MIB, MIB_SNMP_SYS_NAME},
#ifdef TIME_ZONE
	{"ntpTimeZoneDBIndex", INFO_MIB, MIB_NTP_TIMEZONE_DB_INDEX},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
//#ifdef _PRMT_WT107_
	{"ntpServerHost1", INFO_MIB, MIB_NTP_SERVER_HOST1},
	{"ntpServerHost2", INFO_MIB, MIB_NTP_SERVER_HOST2},
//#endif
/*ping_zhang:20081217 END*/
#endif
	{"uptime", INFO_SYS, SYS_UPTIME},
	{"date", INFO_SYS, SYS_DATE},
	{"year", INFO_SYS, SYS_YEAR},
	{"month", INFO_SYS, SYS_MONTH},
	{"day", INFO_SYS, SYS_DAY},
	{"hour", INFO_SYS, SYS_HOUR},
	{"minute", INFO_SYS, SYS_MINUTE},
	{"second", INFO_SYS, SYS_SECOND},
	{"fwVersion", INFO_SYS, SYS_FWVERSION},
	{"dhcplan-ip", INFO_SYS, SYS_DHCP_LAN_IP},
	{"dhcplan-subnet", INFO_SYS, SYS_DHCP_LAN_SUBNET},
	{"dslstate", INFO_SYS, SYS_DSL_OPSTATE},
	{"bridge-ageingTime", INFO_MIB, MIB_BRCTL_AGEINGTIME},
#if defined(CONFIG_RTL_8676HWNAT) || defined(CONFIG_USER_VLAN_ON_LAN)
	{"lan1-vid", INFO_SYS, SYS_LAN1_VID},
	{"lan2-vid", INFO_SYS, SYS_LAN2_VID},
	{"lan3-vid", INFO_SYS, SYS_LAN3_VID},
	{"lan4-vid", INFO_SYS, SYS_LAN4_VID},
	{"lan1-status", INFO_SYS, SYS_LAN1_STATUS},
	{"lan2-status", INFO_SYS, SYS_LAN2_STATUS},
	{"lan3-status", INFO_SYS, SYS_LAN3_STATUS},
	{"lan4-status", INFO_SYS, SYS_LAN4_STATUS},
#endif
#ifdef CONFIG_USER_IGMPPROXY
	{"igmp-proxy-itf", INFO_MIB, MIB_IGMP_PROXY_ITF},
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	{"upnp-ext-itf", INFO_MIB, MIB_UPNP_EXT_ITF},
#endif
#ifdef TIME_ZONE
	{"ntp-ext-itf", INFO_MIB, MIB_NTP_EXT_ITF},
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH
	{"mldproxy-ext-itf", INFO_MIB, MIB_MLD_PROXY_EXT_ITF}, 		// Mason Yu. MLD Proxy
#endif
#endif
#ifdef IP_PASSTHROUGH
	{"ippt-itf", INFO_MIB, MIB_IPPT_ITF},
	{"ippt-lease", INFO_MIB, MIB_IPPT_LEASE},
	{"ippt-lanacc", INFO_MIB, MIB_IPPT_LANACC},
#endif
#ifdef WLAN_SUPPORT
	{"ssid", INFO_SYS, SYS_WLAN_SSID},
	{"channel", INFO_MIB, MIB_WLAN_CHAN_NUM},
	{"chanwid", INFO_MIB, MIB_WLAN_CHANNEL_WIDTH},
	{"fragThreshold", INFO_MIB, MIB_WLAN_FRAG_THRESHOLD},
	{"rtsThreshold", INFO_MIB, MIB_WLAN_RTS_THRESHOLD},
	{"beaconInterval", INFO_MIB, MIB_WLAN_BEACON_INTERVAL},
	{"wlanDisabled",INFO_SYS,SYS_WLAN_DISABLED},
	{"hidden_ssid",INFO_SYS,SYS_WLAN_HIDDEN_SSID},
	{"pskValue", INFO_SYS, SYS_WLAN_PSKVAL},
#ifdef USER_WEB_WIZARD
	{"pskValue_Wizard", INFO_SYS, SYS_WLAN_PSKVAL_WIZARD},
#endif
	{"WiFiTest", INFO_MIB, MIB_WIFI_TEST},
#ifdef WLAN_1x
	{"rsPort",INFO_SYS,SYS_WLAN_RS_PORT},
	{"rsIp",INFO_SYS,SYS_WLAN_RS_IP},
	{"rsPassword",INFO_SYS,SYS_WLAN_RS_PASSWORD},
	{"enable1X", INFO_SYS,SYS_WLAN_ENABLE_1X},
#endif
	{"wlanMode",INFO_SYS,SYS_WLAN_MODE_VAL},
	{"encrypt",INFO_SYS,SYS_WLAN_ENCRYPT_VAL},
	{"wpa_cipher",INFO_SYS,SYS_WLAN_WPA_CIPHER_SUITE},
	{"wpa2_cipher",INFO_SYS,SYS_WLAN_WPA2_CIPHER_SUITE},
	{"wpaAuth",INFO_SYS,SYS_WLAN_WPA_AUTH},
	{"networkType",INFO_MIB,MIB_WLAN_NETWORK_TYPE},
	{"lockdown_stat",INFO_SYS,SYS_WLAN_WPS_LOCKDOWN},

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	{"wscDisable",INFO_SYS,SYS_WSC_DISABLE},
	//{"wscConfig",INFO_MIB,MIB_WSC_CONFIGURED},
	{"wps_auth",INFO_SYS,SYS_WSC_AUTH},
	{"wps_enc",INFO_SYS,SYS_WSC_ENC},
	{"wscLoocalPin", INFO_MIB, MIB_WSC_PIN},
	{"wpsUseVersion", INFO_MIB, MIB_WSC_VERSION},
#endif

#ifdef WLAN_WDS
	{"wlanWdsEnabled",INFO_MIB,MIB_WLAN_WDS_ENABLED},
	{"wdsPskValue",INFO_MIB,MIB_WLAN_WDS_PSK},
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
	{"repeaterEnable", INFO_MIB, MIB_REPEATER_ENABLED1},
	{"repeaterSSID", INFO_MIB, MIB_REPEATER_SSID1},
#endif
#endif // of WLAN_SUPPORT
	//{ "dnsServer", INFO_SYS, SYS_DNS_SERVER},
	{"maxmsglen",INFO_MIB,MIB_MAXLOGLEN},
#ifdef _CWMP_MIB_
	{"acs-url", INFO_MIB, CWMP_ACS_URL},
	{"acs-username", INFO_MIB, CWMP_ACS_USERNAME},
	{"acs-password", INFO_MIB, CWMP_ACS_PASSWORD},
	{"inform-interval", INFO_MIB, CWMP_INFORM_INTERVAL},
	{"conreq-name", INFO_MIB, CWMP_CONREQ_USERNAME},
	{"conreq-pw", INFO_MIB, CWMP_CONREQ_PASSWORD},
	{"cert-pw", INFO_MIB, CWMP_CERT_PASSWORD},
	{"conreq-path", INFO_MIB, CWMP_CONREQ_PATH},
	{"conreq-port", INFO_MIB, CWMP_CONREQ_PORT},
	{"tr069_wan_intf", INFO_MIB, CWMP_WAN_INTERFACE},
#endif
#ifdef DOS_SUPPORT
	{"syssynFlood", INFO_MIB, MIB_DOS_SYSSYN_FLOOD},
	{"sysfinFlood", INFO_MIB, MIB_DOS_SYSFIN_FLOOD},
	{"sysudpFlood", INFO_MIB, MIB_DOS_SYSUDP_FLOOD},
	{"sysicmpFlood", INFO_MIB, MIB_DOS_SYSICMP_FLOOD},
	{"pipsynFlood", INFO_MIB, MIB_DOS_PIPSYN_FLOOD},
	{"pipfinFlood", INFO_MIB, MIB_DOS_PIPFIN_FLOOD},
	{"pipudpFlood", INFO_MIB, MIB_DOS_PIPUDP_FLOOD},
	{"pipicmpFlood", INFO_MIB, MIB_DOS_PIPICMP_FLOOD},
	{"blockTime", INFO_MIB, MIB_DOS_BLOCK_TIME},
#endif
	{"lan-dhcp-gateway", INFO_MIB, MIB_ADSL_LAN_DHCP_GATEWAY},
#ifdef ADDRESS_MAPPING
#ifndef MULTI_ADDRESS_MAPPING
	{"local-s-ip", INFO_MIB, MIB_LOCAL_START_IP},
	{"local-e-ip", INFO_MIB, MIB_LOCAL_END_IP},
	{"global-s-ip", INFO_MIB, MIB_GLOBAL_START_IP},
	{"global-e-ip", INFO_MIB, MIB_GLOBAL_END_IP},
#endif //!MULTI_ADDRESS_MAPPING
#endif
#ifdef CONFIG_USER_RTK_SYSLOG
	{"log-level", INFO_MIB, MIB_SYSLOG_LOG_LEVEL},
	{"display-level", INFO_MIB, MIB_SYSLOG_DISPLAY_LEVEL},
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
	{"syslog-mode", INFO_MIB, MIB_SYSLOG_MODE},
	{"syslog-server-ip", INFO_MIB, MIB_SYSLOG_SERVER_IP},
	{"syslog-server-port", INFO_MIB, MIB_SYSLOG_SERVER_PORT},
#endif
#ifdef SEND_LOG
	{"log-server-ip", INFO_MIB, MIB_LOG_SERVER_IP},
	{"log-server-username", INFO_MIB, MIB_LOG_SERVER_NAME},
#endif
#endif
#ifdef TCP_UDP_CONN_LIMIT
	{"connLimit-tcp", INFO_MIB, MIB_CONNLIMIT_TCP},
	{"connLimit-udp", INFO_MIB, MIB_CONNLIMIT_UDP},
#endif
#ifdef WEB_REDIRECT_BY_MAC
	{"landing-page-time", INFO_MIB, MIB_WEB_REDIR_BY_MAC_INTERVAL},
#endif
	{"super-user", INFO_MIB, MIB_SUSER_NAME},
#ifdef USER_WEB_WIZARD
	{"super-pass", INFO_MIB, MIB_SUSER_PASSWORD},
#endif
	{"normal-user", INFO_MIB, MIB_USER_NAME},
#ifdef DEFAULT_GATEWAY_V2
	{"wan-default-gateway", INFO_MIB, MIB_ADSL_WAN_DGW_IP},
	{"itf-default-gateway", INFO_MIB, MIB_ADSL_WAN_DGW_ITF},
#endif
//ql 20090119
#ifdef IMAGENIO_IPTV_SUPPORT
	{"stb-dns1", INFO_MIB, MIB_IMAGENIO_DNS1},
	{"stb-dns2", INFO_MIB, MIB_IMAGENIO_DNS2},
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
	{"opch-addr", INFO_MIB, MIB_OPCH_ADDRESS},
	{"opch-port", INFO_MIB, MIB_OPCH_PORT},
#endif
/*ping_zhang:20090930 END*/
#endif

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_RADVD
	{"V6MaxRtrAdvInterval", INFO_MIB, MIB_V6_MAXRTRADVINTERVAL},
	{"V6MinRtrAdvInterval", INFO_MIB, MIB_V6_MINRTRADVINTERVAL},
	{"V6AdvCurHopLimit", INFO_MIB, MIB_V6_ADVCURHOPLIMIT},
	{"V6AdvDefaultLifetime", INFO_MIB, MIB_V6_ADVDEFAULTLIFETIME},
	{"V6AdvReachableTime", INFO_MIB, MIB_V6_ADVREACHABLETIME},
	{"V6AdvRetransTimer", INFO_MIB, MIB_V6_ADVRETRANSTIMER},
	{"V6AdvLinkMTU", INFO_MIB, MIB_V6_ADVLINKMTU},
	{"V6prefix_ip", INFO_MIB, MIB_V6_PREFIX_IP},
	{"V6prefix_len", INFO_MIB, MIB_V6_PREFIX_LEN},
	{"V6ValidLifetime", INFO_MIB, MIB_V6_VALIDLIFETIME},
	{"V6PreferredLifetime", INFO_MIB, MIB_V6_PREFERREDLIFETIME},
	{"V6RDNSS1", INFO_MIB, MIB_V6_RDNSS1},
	{"V6RDNSS2", INFO_MIB, MIB_V6_RDNSS2},
	{"V6ULAPrefixEnable", INFO_MIB, MIB_V6_ULAPREFIX_ENABLE},
	{"V6ULAPrefix", INFO_MIB, MIB_V6_ULAPREFIX},
	{"V6ULAPrefixlen", INFO_MIB, MIB_V6_ULAPREFIX_LEN},
	{"V6ULAPrefixValidLifetime", INFO_MIB, MIB_V6_ULAPREFIX_VALID_TIME},
	{"V6ULAPrefixPreferredLifetime", INFO_MIB, MIB_V6_ULAPREFIX_PREFER_TIME},
#endif

#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	{"dhcpv6s_prefix_length", INFO_MIB, MIB_DHCPV6S_PREFIX_LENGTH},
	{"dhcpv6s_range_start", INFO_MIB, MIB_DHCPV6S_RANGE_START},
	{"dhcpv6s_range_end", INFO_MIB, MIB_DHCPV6S_RANGE_END},
	{"dhcpv6s_default_LTime", INFO_MIB, MIB_DHCPV6S_DEFAULT_LEASE},
	{"dhcpv6s_preferred_LTime", INFO_MIB, MIB_DHCPV6S_PREFERRED_LIFETIME},
	{"dhcpv6_mode", INFO_SYS, SYS_DHCPV6_MODE},
	{"dhcpv6_relay_itf", INFO_SYS, SYS_DHCPV6_RELAY_UPPER_ITF},
	{"dhcpv6s_renew_time", INFO_MIB, MIB_DHCPV6S_RENEW_TIME},
	{"dhcpv6s_rebind_time", INFO_MIB, MIB_DHCPV6S_REBIND_TIME},
	{"dhcpv6s_clientID", INFO_MIB, MIB_DHCPV6S_CLIENT_DUID},
#endif
	{"ip6_ll", INFO_SYS, SYS_LAN_IP6_LL},
	{"ip6_global", INFO_SYS, SYS_LAN_IP6_GLOBAL},
#endif // of CONFIG_IPV6

#ifdef CONFIG_RTL_WAPI_SUPPORT
	{ "wapiUcastReKeyType", INFO_MIB, MIB_WLAN_WAPI_UCAST_REKETTYPE},
	{ "wapiUcastTime", INFO_MIB, MIB_WLAN_WAPI_UCAST_TIME},
	{ "wapiUcastPackets", INFO_MIB, MIB_WLAN_WAPI_UCAST_PACKETS},
	{ "wapiMcastReKeyType", INFO_MIB, MIB_WLAN_WAPI_MCAST_REKEYTYPE},
	{ "wapiMcastTime", INFO_MIB, MIB_WLAN_WAPI_MCAST_TIME},
	{ "wapiMcastPackets", INFO_MIB, MIB_WLAN_WAPI_MCAST_PACKETS},
#endif
#ifdef CONFIG_IPV6
	{"wan-dnsv61", INFO_MIB, MIB_ADSL_WAN_DNSV61},
	{"wan-dnsv62", INFO_MIB, MIB_ADSL_WAN_DNSV62},
	{"wan-dnsv63", INFO_MIB, MIB_ADSL_WAN_DNSV63},
#endif
	{"wan_mode", INFO_MIB, MIB_WAN_MODE},
	{"dhcp_port_filter", INFO_MIB, MIB_DHCP_PORT_FILTER},
#ifdef CONFIG_USER_SAMBA
#ifdef CONFIG_USER_NMBD
	{"samba-netbios-name", INFO_MIB, MIB_SAMBA_NETBIOS_NAME},
#endif
	{"samba-server-string", INFO_MIB, MIB_SAMBA_SERVER_STRING},
#endif
#ifdef CONFIG_RTK_RG_INIT
	{"mac_based_tag_decision", INFO_MIB, MIB_MAC_BASED_TAG_DECISION},
	{"lan_vlan_id1", INFO_MIB, MIB_LAN_VLAN_ID1},
	{"lan_vlan_id2", INFO_MIB, MIB_LAN_VLAN_ID2},
#endif
	{"loid", INFO_MIB, MIB_LOID},
#ifdef CONFIG_GPON_FEATURE
	{"omci_sw_ver1", INFO_MIB, MIB_OMCI_SW_VER1},
	{"omci_sw_ver2", INFO_MIB, MIB_OMCI_SW_VER2},
	{"omcc_ver", INFO_MIB, MIB_OMCC_VER},
	{"omci_tm_opt", INFO_MIB, MIB_OMCI_TM_OPT},
	{"omci_eqid", INFO_MIB, MIB_OMCI_EQID},
	{"omci_ont_ver", INFO_MIB, MIB_OMCI_ONT_VER},
	{"omci_vendor_id", INFO_MIB, MIB_OMCI_VENDOR_ID},
#endif	
#ifdef CONFIG_USER_Y1731
	{"y1731_mode", INFO_MIB, Y1731_MODE},
	{"y1731_megid", INFO_MIB, Y1731_MEGID},
	{"y1731_myid", INFO_MIB, Y1731_MYID},
	{"y1731_meglevel", INFO_MIB, Y1731_MEGLEVEL},
	{"y1731_loglevel", INFO_MIB, Y1731_LOGLEVEL},
	{"y1731_ccminterval", INFO_MIB, Y1731_CCM_INTERVAL},
#endif	
	{"rtk_manufacturer", INFO_MIB, RTK_DEVID_MANUFACTURER},	
	{"rtk_oui", INFO_MIB, RTK_DEVID_OUI},	
	{"rtk_productclass", INFO_MIB, RTK_DEVID_PRODUCTCLASS},	
	{"rtk_serialno", INFO_MIB, MIB_HW_SERIAL_NUMBER},	
#ifdef CONFIG_USER_CWMP_TR069
	{"cwmp_provisioningcode", INFO_MIB, CWMP_PROVISIONINGCODE},	
#endif
	{"rtk_specver", INFO_MIB, RTK_DEVINFO_SPECVER},	
	{"rtk_swver", INFO_MIB, RTK_DEVINFO_SWVER},	
	{"rtk_hwver", INFO_MIB, RTK_DEVINFO_HWVER},
#if defined(CONFIG_GPON_FEATURE)
	{"gpon_sn",INFO_MIB,MIB_GPON_SN},
#endif
	{"elan_mac_addr", INFO_MIB, MIB_ELAN_MAC_ADDR},
	{NULL, 0, 0}
};

#ifdef WLAN_SUPPORT
#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS
static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}


static int fnget_wpsKey(int eid, request * wp, int argc, char **argv, char *buffer) {
	unsigned char key, vChar, type;
	int mib_id;
	MIB_CE_MBSSIB_T Entry;

	wlan_getEntry(&Entry, 0);

	vChar = Entry.wsc_enc;
	buffer[0]='\0';
	if (vChar == WSC_ENCRYPT_WEP) {
		unsigned char tmp[100];
		vChar = Entry.wep;
		type = Entry.wepKeyType;
		key = Entry.wepDefaultKey; //default key
		if (vChar == 1) {
			if (key == 0)
				strcpy(tmp, Entry.wep64Key1);
			else if (key == 1)
				strcpy(tmp, Entry.wep64Key2);
			else if (key == 2)
				strcpy(tmp, Entry.wep64Key3);
			else
				strcpy(tmp, Entry.wep64Key4);
			
			if(type == KEY_ASCII){
				memcpy(buffer, tmp, 5);
				buffer[5] = '\0';
			}else{
				convert_bin_to_str(tmp, 5, buffer);
				buffer[10] = '\0';
			}
		}
		else {
			if (key == 0)
				strcpy(tmp, Entry.wep128Key1);
			else if (key == 1)
				strcpy(tmp, Entry.wep128Key2);
			else if (key == 2)
				strcpy(tmp, Entry.wep128Key3);
			else
				strcpy(tmp, Entry.wep128Key4);
			
			if(type == KEY_ASCII){
				memcpy(buffer, tmp, 13);
				buffer[13] = '\0';
			}else{
				convert_bin_to_str(tmp, 13, buffer);
				buffer[26] = '\0';
			}
		}
	}
	else {
		if (vChar ==0 || vChar == WSC_ENCRYPT_NONE)
			strcpy(buffer, "N/A");
		else
			strcpy(buffer, Entry.wscPsk);
	}
   	return boaWrite(wp, buffer);
}
#endif
#endif

#ifdef CONFIG_RTL8672_SAR
static int fnget_urlWanadsl(int eid, request * wp, int argc, char **argv, char *buffer)
{
	if (strstr(wp->pathname, "web/admin/"))
		return boaWrite(wp, "/admin/wanadsl.asp");
	else
		return boaWrite(wp, "/wanadsl.asp");
	return 0;
}
#endif

web_custome_cmd get_info_custom_list[] = {
	#ifdef WLAN_SUPPORT
	#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS
	{ "wps_key", fnget_wpsKey },
	#endif
	#endif
	#ifdef CONFIG_RTL8672_SAR
	{ "url_wanadsl", fnget_urlWanadsl },
	#endif
	{ NULL, 0 }
};

int getInfo(int eid, request * wp, int argc, char **argv)
{
	char	*name;
	unsigned char buffer[256+1];
	int idx, ret;

   	if (boaArgs(argc, argv, "%s", &name) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}

	memset(buffer,0x00,64);
#ifdef CONFIG_DEV_xDSL
 	if ( !strncmp(name, "adsl-drv-", 9) ) {
 		getAdslDrvInfo(&name[9], buffer, 64);
		return boaWrite(wp, "%s", buffer);
	}
#endif
#ifdef CONFIG_USER_XDSL_SLAVE
	if ( !strncmp(name, "adsl-slv-drv-", 13) ) {
		getAdslSlvDrvInfo(&name[13], buffer, 64);
		return boaWrite(wp, "%s", buffer);
	}
#endif /*CONFIG_USER_XDSL_SLAVE*/
	/*+++++add by Jack for VoIP project 20/03/07+++++*/
#ifdef VOIP_SUPPORT
	if(!strncmp(name, "voip_", 5)){
		return asp_voip_getInfo(eid, wp, argc, argv);
	}
#endif /*VOIP_SUPPORT*/
	if(!strcmp(name, "login-user")){
#ifdef USE_LOGINWEB_OF_SERVER
		ret = boaWrite(wp, "%s", g_login_username);
#else
		ret = boaWrite(wp, "%s", wp->user);
#endif
		goto NEXTSTEP;
	}

#ifdef FIELD_TRY_SAFE_MODE
#ifdef CONFIG_DEV_xDSL
	if (!strncmp(name, "safemodenote", 12)) {
		SafeModeData vSmd;
		memset((void *)&vSmd, 0, sizeof(vSmd));
		adsl_drv_get(RLCM_GET_SAFEMODE_CTRL, (void *)&vSmd, SAFEMODE_DATA_SIZE);
		boaWrite(wp, "%s", vSmd.SafeModeNote);
	}
#endif
#endif

 	for (idx=0; get_info_custom_list[idx].cmd != NULL; idx++) {
 		if (!strcmp(name, get_info_custom_list[idx].cmd)) {
 			return get_info_custom_list[idx].handler(eid, wp, argc, argv, buffer);
 		}
 	}

	for (idx=0; get_info_list[idx].cmd != NULL; idx++) {
		if (!strcmp(name, get_info_list[idx].cmd)) {
			if (get_info_list[idx].type == INFO_MIB) {
				if (getMIB2Str(get_info_list[idx].id, buffer)) {
					fprintf(stderr, "failed to get %s\n", name);
					return -1;
				}
			}
			else {
				if (getSYS2Str(get_info_list[idx].id, buffer))
					return -1;
			}
			// Kaohj
			if ((!strncmp(name, "wan-dns", 7))&& !strcmp(buffer, "0.0.0.0"))
				ret = boaWrite(wp, "");
			else{
				#ifdef WLAN_SUPPORT
				if(!strcmp(name, "ssid")){
					translate_control_code(buffer);
				}
				#endif
				ret = boaWrite(wp, "%s", buffer);
			}
			//fprintf(stderr, "%s = %s\n", name, buffer);
			//printf("%s = %s\n", name, buffer);
			break;
		}
	}

NEXTSTEP:
	return ret;
}

int addMenuJavaScript( request * wp,int nums,int maxchildrensize)
{
	boaWrite(wp,"<script >\n");
	int i=0;
	boaWrite(wp,"scores = new Array(%d);\n",nums);
	for(i=0;i<nums;i++ )
		boaWrite(wp,"scores[%d]='Submenu%d';\n",i,i);
	boaWrite(wp,"btns = new Array(%d);\n",nums);
	for(i=0;i<nums;i++ )
		boaWrite(wp,"btns[%d]='Btn%d';\n",i,i);
	boaWrite(wp,"\nfunction initIt()\n"
		"{\n\tdivColl = document.all.tags(\"div\");\n"
		"\tfor (i=0; i<divColl.length; i++)\n "
		"\t{\n\t\twhichEl = divColl[i];\n"
		"\t\tif (whichEl.className == \"Child\")\n"
		"\t\t\twhichEl.style.display = \"none\";\n\t}\n}\n\n");
	boaWrite(wp,"function closeMenu(el)\n"
		"{\n"
		"\tfor(i=0;i<%d;i++)\n"
		"\t{\n\t\tfor(j=0;j<%d;j++)"
		"{\n\t\t\tif(scores[i]!=el)\n"
		"\t\t\t{\n\t\t\t\tid=scores[i]+\"Child\"+j.toString();\n"
		"\t\t\t\tif(document.getElementById(id))\n"
		"\t\t\t\t{\n\t\t\t\t\tdocument.getElementById(id).style.display = \"none\";\n"
		"\t\t\t\t\twhichEl = eval(scores[i] + \"Child\");\n"
		"\t\t\t\t\twhichEl.style.display = \"none\";\n"
		"\t\t\t\t\tdocument.getElementById(btns[i]).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n}\n\n",nums, maxchildrensize);

	boaWrite(wp,"function expandMenu(el,imgs, num)\n"
		"{\n\tcloseMenu(el);\n");
	boaWrite(wp,"\tif (num == 0) {\n\t\twhichEl1 = eval(el + \"Child\");\n"
		"\t\tfor(i=0;i<%d;i++)\n",nums);
	boaWrite(wp,"\t\t{\n\t\t\twhichEl = eval(scores[i] + \"Child\");\n"
		"\t\t\tif(whichEl!=whichEl1)\n "
		"\t\t\t{\n\t\t\t\twhichEl.style.display = \"none\";\n"
		"\t\t\t\tdocument.getElementById(btns[i]).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t\t}\n\t\t}\n");
	boaWrite(wp,"\t\twhichEl1 = eval(el + \"Child\");\n"
		"\t\tif (whichEl1.style.display == \"none\")\n "
		"\t\t{\n"
		"\t\t\twhichEl1.style.display = \"\";\n"
		"\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_open.gif\";\n"
		"\t\t}\n\t\telse {\n\t\t\twhichEl1.style.display =\"none\";\n"
		"\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t}\n\t}\n\telse {\n");
	boaWrite(wp,"\t\tfor(i=0;i<num;i++) {\n"
		"\t\t\tid = el + \"Child\"+i.toString();\n"
		"\t\t\twhichEl1 = document.getElementById(id);\n"
		"\t\t\tif (whichEl1) {\n"
		"\t\t\t\tif (whichEl1.style.display == \"none\")\n"
		"\t\t\t\t{\n"
		"\t\t\t\t\twhichEl1.style.display = \"\";\n"
		"\t\t\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_open.gif\";\n"
		"\t\t\t\t}\n\t\t\t\telse {\n\t\t\t\t\twhichEl1.style.display =\"none\";\n"
		"\t\t\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t\t\t}\n\t\t\t}\n\t\t}\n\t}\n}\n</script>\n");

	boaWrite(wp,"<style type=\"text/css\">\n"
		"\n.link {\n"
/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
#ifdef CONFIG_11N_SAGEM_WEB
		"\tfont-family: arial, Helvetica, sans-serif, bold;\n\tfont-size:10pt;\n\twhite-space:nowrap;\n\tcolor: #000000;\n\ttext-decoration: none;\n}\n"
#else
		"\tfont-family: arial, Helvetica, sans-serif, bold;\n\tfont-size:10pt;\n\twhite-space:nowrap;\n\tcolor: #FFFFFF;\n\ttext-decoration: none;\n}\n"
#endif
		"</style>");
}


//modify adsl-stats.asp for adsl and vdsl2
#define DSL_STATS_FMT_UP \
	"<tr>\n" \
	"	<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n" \
	"	<td bgcolor=#f0f0f0>%s</td>\n" \
	"</tr>\n"
#define DSL_STATS_FMT_DSUS \
	"<tr bgcolor=#f0f0f0>\n" \
	"	<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n" \
	"	<td>%s</td><td>%s</td>\n" \
	"</tr>\n"
#define DSL_STATS_FMT_DOWN \
	"<tr bgcolor=#f0f0f0>\n" \
	"	<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n" \
	"	<td colspan=2>%s</td>\n" \
	"</tr>\n"
//end modify adsl-stats.asp for adsl and vdsl2

int check_display(int eid, request * wp, int argc, char **argv)
{
	char *name;

   	if (boaArgs(argc, argv, "%s", &name) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}

	if(!strcmp(name,"vid_mark"))
	{
#ifdef CONFIG_LUNA
		boaWrite(wp,"none");
#else
		boaWrite(wp,"block");
#endif
		return 0;
	}
	else if(!strcmp(name,"wan_interface"))
	{
#ifdef CONFIG_LUNA
		boaWrite(wp,"none");
#else
		boaWrite(wp,"block");
#endif
		return 0;
	}
	else if(!strcmp(name,"qos_direction"))
	{
#ifdef CONFIG_LUNA
		boaWrite(wp,"block");
#else
		boaWrite(wp,"none");
#endif
		return 0;
	}
	else if(!strcmp(name,"vlanID"))
	{
#if defined(QOS_TRAFFIC_SHAPING_BY_VLANID)&&defined(CONFIG_RTK_RG_INIT)
		boaWrite(wp,"block");
#else
		boaWrite(wp,"none");
#endif
		return 0;
	}
	else if(!strcmp(name,"ssid"))
	{
#if defined(QOS_TRAFFIC_SHAPING_BY_SSID)&&defined(CONFIG_RTK_RG_INIT)
		boaWrite(wp,"block");
#else
		boaWrite(wp,"none");
#endif
		return 0;
	}
	return -1;
}


// Kaohj
int checkWrite(int eid, request * wp, int argc, char **argv)
{
	char *name;
	unsigned char vChar;
	unsigned short vUShort;
	unsigned int vUInt;
#ifdef CONFIG_USER_VLAN_ON_LAN
	MIB_CE_SW_PORT_T sw_entry;
#endif

   	if (boaArgs(argc, argv, "%s", &name) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}
	if ( !strcmp(name, "devType") ) {
		if ( !mib_get( MIB_DEVICE_TYPE, (void *)&vChar) )
			return -1;
#ifdef EMBED
		if (0 == vChar)
			boaWrite(wp, "disableTextField(document.adsl.adslConnectionMode);");
#endif
		return 0;
	}

	// Added by davian kuo
#ifdef CONFIG_USER_BOA_WITH_MULTILANG
	if (!strcmp(name, "selinit")) {
		int i = 0;
		char mStr[MAX_LANGSET_LEN] = {0};

		if (!mib_get (MIB_MULTI_LINGUAL, (void *)mStr)) {
			fprintf (stderr, "mib get multi-lingual setting failed!\n");
			return -1;
		}

		for (i = 0; i < LANG_MAX; i++)
			boaWrite(wp, "<option %s value=\"%d\">%s</option>\n",
				(!(strcmp(mStr, lang_set[i].langType)))?"selected":"", i, lang_set[i].langStr);
		return 0;
	}
#endif

#ifdef USE_LOGINWEB_OF_SERVER
	if (!strcmp(name, "loginSelinit")) {
#ifdef CONFIG_USER_BOA_WITH_MULTILANG
		int i = 0;
		char mStr[MAX_LANGSET_LEN] = {0};

		if (!mib_get (MIB_MULTI_LINGUAL, (void *)mStr)) {
			fprintf (stderr, "mib get multi-lingual setting (login) failed!\n");
			return -1;
		}

		boaWrite(wp, "<td><div align=right><font color=#0000FF size=2>%s:</font></div></td>\n", multilang(LANG_LANGUAGE_SELECT));
		boaWrite(wp, "<td>&nbsp;&nbsp;</td>\n");
		boaWrite(wp, "<td><font size=2><select size=\"1\" name=\"loginSelinit\" onChange=\"mlhandle();\">\n");
		for (i = 0; i < LANG_MAX; i++)
			boaWrite(wp, "<option %s value=\"%d\">%s</option>\n",
				(!(strcmp(mStr, lang_set[i].langType)))?"selected":"", i, lang_set[i].langStr);
		boaWrite(wp, "</select></font></td>\n");

		return 0;
#endif
	}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
	else if ( !strcmp(name, "rip-on-0") ) {
		if ( !mib_get( MIB_RIP_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "rip-on-1") ) {
		if ( !mib_get( MIB_RIP_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif

#ifdef CONFIG_DEV_xDSL
	//modify adsl-set.asp for adsl and vdsl2
	if (!strcmp (name, "adsl_set_title"))
	{
		#ifdef CONFIG_VDSL
		boaWrite (wp, "%s", multilang(LANG_DSL_SETTINGS) );
		#else
		boaWrite (wp, "%s", multilang(LANG_ADSL_SETTINGS) );
		#endif /*CONFIG_VDSL*/
		return 0;
	}
	if (!strcmp (name, "xdsl_type"))
	{
		#ifdef CONFIG_VDSL
		boaWrite (wp, "%s", "DSL" );
		#else
		boaWrite (wp, "%s", "ADSL" );
		#endif /*CONFIG_VDSL*/
		return 0;
	}


	if( !strncmp (name, "adsl_", 5) || !strncmp (name, "adsl_slv_", 9) )
	{
		int ofs;
		XDSL_OP *d;
#ifdef CONFIG_USER_XDSL_SLAVE
		if(!strncmp (name, "adsl_slv_", 9))
		{
			ofs=9;
			d=xdsl_get_op(1);
		}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
		{
			ofs=5;
			d=xdsl_get_op(0);
		}


		//modify adsl-diag.asp for adsl and vdsl2
		if (!strcmp (&name[ofs], "diag_title"))
		{
#ifdef CONFIG_USER_XDSL_SLAVE
			if(d->id)
			{
				#ifdef CONFIG_VDSL
				boaWrite (wp, "%s", multilang(LANG_DSL_SLAVE_TONE_DIAGNOSTICS) );
				#else
				boaWrite (wp, "%s", multilang(LANG_ADSL_SLAVE_TONE_DIAGNOSTICS) );
				#endif /*CONFIG_VDSL*/
			}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
			{
				#ifdef CONFIG_VDSL
				boaWrite (wp, "%s", multilang(LANG_DSL_TONE_DIAGNOSTICS) );
				#else
				boaWrite (wp, "%s", multilang(LANG_ADSL_TONE_DIAGNOSTICS) );
				#endif /*CONFIG_VDSL*/
			}
			return 0;
		}
		if (!strcmp (&name[ofs], "diag_cmt"))
		{
#ifdef CONFIG_USER_XDSL_SLAVE
			if(d->id)
			{
				#ifdef CONFIG_VDSL
				boaWrite (wp, "%s", multilang(LANG_DSL_SLAVE_TONE_DIAGNOSTICS_ONLY_ADSL2_ADSL2_VDSL2_SUPPORT_THIS_FUNCTION) );
				#else
				boaWrite (wp, "%s", multilang(LANG_ADSL_SLAVE_TONE_DIAGNOSTICS_ONLY_ADSL2_2_SUPPORT_THIS_FUNCTION) );
				#endif /*CONFIG_VDSL*/
			}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
			{
				#ifdef CONFIG_VDSL
				boaWrite (wp, "%s", multilang(LANG_DSL_TONE_DIAGNOSTICS_ONLY_ADSL2_ADSL2_VDSL2_SUPPORT_THIS_FUNCTION) );
				#else
				boaWrite (wp, "%s", multilang(LANG_ADSL_TONE_DIAGNOSTICS_ONLY_ADSL2_2_SUPPORT_THIS_FUNCTION) );
				#endif /*CONFIG_VDSL*/
			}
			return 0;
		}
		//modify adsl-stats.asp for adsl and vdsl2
		if (!strcmp (&name[ofs], "title"))
		{
#ifdef CONFIG_USER_XDSL_SLAVE
			if(d->id)
			{
				#ifdef CONFIG_VDSL
				boaWrite (wp, "%s", multilang(LANG_DSL_SLAVE_STATISTICS) );
				#else
				boaWrite (wp, "%s", multilang(LANG_ADSL_SLAVE_STATISTICS) );
				#endif /*CONFIG_VDSL*/
			}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
			{
				#ifdef CONFIG_VDSL
				boaWrite (wp, "%s", multilang(LANG_DSL_STATISTICS) );
				#else
				boaWrite (wp, "%s", multilang(LANG_ADSL_STATISTICS) );
				#endif /*CONFIG_VDSL*/
			}
			return 0;
		}
		if (!strcmp (&name[ofs], "tpstc"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buffer[64];
			d->xdsl_get_info(DSL_GET_TPS, buffer, 64);
			boaWrite (wp, DSL_STATS_FMT_UP, "TPS-TC", buffer );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "trellis"))
		{
			#ifndef CONFIG_VDSL
			unsigned char buffer[64];
			d->xdsl_get_info(ADSL_GET_TRELLIS, buffer, 64);
			boaWrite (wp, DSL_STATS_FMT_UP, "Trellis Coding", buffer );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "trellis_dsus"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(DSL_GET_TRELLIS_DS, buf_ds, 64);
			d->xdsl_get_info(DSL_GET_TRELLIS_US, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "Trellis", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "ginp_dsus"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(DSL_GET_PHYR_DS, buf_ds, 64);
			d->xdsl_get_info(DSL_GET_PHYR_US, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "G.INP", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "k_dsus"))
		{
			#ifndef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(ADSL_GET_K_DS, buf_ds, 64);
			d->xdsl_get_info(ADSL_GET_K_US, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "K (number of bytes in DMT frame)", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "n_dsus"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(DSL_GET_N_DS, buf_ds, 64);
			d->xdsl_get_info(DSL_GET_N_US, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "N (RS codeword size)", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "l_dsus"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(DSL_GET_L_DS, buf_ds, 64);
			d->xdsl_get_info(DSL_GET_L_US, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "L (number of bits in DMT frame)", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "inp_dsus"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(DSL_GET_INP_DS, buf_ds, 64);
			d->xdsl_get_info(DSL_GET_INP_US, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "INP (DMT frame)", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "fec_name"))
		{
			#ifdef CONFIG_VDSL
			boaWrite (wp, "FEC errors");
			#else
			boaWrite (wp, "FEC");
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "ohframe_dsus"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(ADSL_GET_RX_FRAMES, buf_ds, 64);
			d->xdsl_get_info(ADSL_GET_TX_FRAMES, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "OH Frame", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "crc_name"))
		{
			#ifdef CONFIG_VDSL
			boaWrite (wp, "OH Frame errors");
			#else
			boaWrite (wp, "CRC");
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "llr_dsus"))
		{
			#ifdef CONFIG_VDSL
			unsigned char buf_ds[64],buf_us[64];
			d->xdsl_get_info(ADSL_GET_LAST_LINK_DS, buf_ds, 64);
			d->xdsl_get_info(ADSL_GET_LAST_LINK_US, buf_us, 64);
			boaWrite (wp, DSL_STATS_FMT_DSUS, "Last Link Rate", buf_ds, buf_us );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "llr"))
		{
			#ifndef CONFIG_VDSL
			unsigned char buffer[64];
			d->xdsl_get_info(ADSL_GET_LAST_LINK_DS, buffer, 64);
			boaWrite (wp, DSL_STATS_FMT_DOWN, "Last Link DS Rate", buffer );
			d->xdsl_get_info(ADSL_GET_LAST_LINK_US, buffer, 64);
			boaWrite (wp, DSL_STATS_FMT_DOWN, "Last Link US Rate", buffer );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		if (!strcmp (&name[ofs], "txrx_frame"))
		{
			#ifndef CONFIG_VDSL
			unsigned char buffer[64];
			d->xdsl_get_info(ADSL_GET_TX_FRAMES, buffer, 64);
			boaWrite (wp, DSL_STATS_FMT_DOWN, "TX frames", buffer );
			d->xdsl_get_info(ADSL_GET_RX_FRAMES, buffer, 64);
			boaWrite (wp, DSL_STATS_FMT_DOWN, "RX frames", buffer );
			#endif /*CONFIG_VDSL*/
			return 0;
		}
		//end modify adsl-stats.asp for adsl and vdsl2
	}
#endif // of CONFIG_DEV_xDSL

	if ( !strcmp(name, "dhcpMode") ) {
		boaWrite(wp, "<input type=\"radio\" name=dhcpdenable value=0 onClick=\"disabledhcpd()\">%s&nbsp;&nbsp;\n", multilang(LANG_NONE));
		boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=1 onClick=\"enabledhcprelay()\">DHCP %s&nbsp;&nbsp;\n", multilang(LANG_RELAY));
		boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=2 onClick=\"enabledhcpd()\">DHCP %s&nbsp;&nbsp;\n", multilang(LANG_SERVER));
		return 0;
	}

	if ( !strcmp(name, "dhcpV6Mode") ) {
		boaWrite(wp, "<input type=\"radio\" name=dhcpdenable value=0 onClick=\"disabledhcpd()\">%s&nbsp;\n", multilang(LANG_NONE));
		boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=1 onClick=\"enabledhcprelay()\">DHCP%s&nbsp;\n", multilang(LANG_RELAY));
		boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=2 onClick=\"enabledhcpd()\">DHCP%s(%s)&nbsp;\n", multilang(LANG_SERVER), multilang(LANG_MANUAL));
		boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=3 onClick=\"autodhcpd()\">DHCP%s(%s)&nbsp;\n", multilang(LANG_SERVER), multilang(LANG_AUTO));
		return 0;
	}

#ifdef ADDRESS_MAPPING
#ifndef MULTI_ADDRESS_MAPPING
	if ( !strcmp(name, "addressMapType") ) {
 		if ( !mib_get( MIB_ADDRESS_MAP_TYPE, (void *)&vChar) )
			return -1;

		boaWrite(wp, "<option value=0>%s</option>\n", multilang(LANG_NONE));
		boaWrite(wp, "<option value=1>%s</option>\n", multilang(LANG_ONE_TO_ONE));
		boaWrite(wp, "<option value=2>%s</option>\n", multilang(LANG_MANY_TO_ONE));
		boaWrite(wp, "<option value=3>%s</option>\n", multilang(LANG_MANY_TO_MANY_OVERLOAD));

		// Mason Yu on True
		boaWrite(wp, "<option value=4>%s</option>\n", multilang(LANG_ONE_TO_MANY));
		return 0;
	}
#endif	// end of !MULTI_ADDRESS_MAPPING
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
	if (!strcmp(name, "wlanAcNum")) {
		MIB_CE_WLAN_AC_T entry;
		int i;
		vUInt = mib_chain_total(MIB_WLAN_AC_TBL);
		for (i=0; i<vUInt; i++) {
			if (!mib_chain_get(MIB_WLAN_AC_TBL, i, (void *)&entry)) {
				i = vUInt;
				break;
			}
			if(entry.wlanIdx == wlan_idx)
				break;
		}
		if (i == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef WLAN_WDS
	if (!strcmp(name, "wlanWDSNum")) {
		vUInt = mib_chain_total(MIB_WDS_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
	if ( !strcmp(name, "wdsEncrypt")) {
		int i;
		char* encryp[5] = {multilang(LANG_NONE), "WEP 64bits", "WEP 128bits", "WPA (TKIP)", "WPA2 (AES)"};
		if ( !mib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&vChar) )
			return -1;
	       for(i=0;i<5;i++){
		   	if(i == WDS_ENCRYPT_TKIP){//skip tkip....by ap team's web
				continue;
		   	}
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
			if(wlan_idx==1)
#else
			if(wlan_idx==0)
#endif
			if((i!=WDS_ENCRYPT_DISABLED) && (i!=WDS_ENCRYPT_AES))
				continue;
#endif
		   	boaWrite(wp, "<option %s value=\"%d\">%s</option>\n", (i==vChar)?"selected":"", i, encryp[i]);
	       }
		return 0;
	}
	if ( !strcmp(name, "wdsWepFormat")) {
		int i;
		unsigned char tmp;
		char* format[4] = {"ASCII (5 characters)", "Hex (10 characters)", "ASCII (13 characters)", "Hex (26 characters)"};
		int skip = 0;
		if ( !mib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&tmp) )
			return -1;
		if(tmp == WDS_ENCRYPT_WEP128){
			skip = 2;
		}
		if ( !mib_get( MIB_WLAN_WDS_WEP_FORMAT, (void *)&vChar) )
			return -1;
		for(i=0;i<2;i++){
		   	boaWrite(wp, "<option %s value=\"%d\">%s</option>\n", (i==vChar)?"selected":"", i, format[i + skip]);
	       }
		return 0;
	}
	if ( !strcmp(name, "wdsPskFormat")) {
		int i;
		char* format[2] = {"Passphrase", "Hex (64 characters)"};
		if ( !mib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&vChar) )
			return -1;
		for(i=0;i<2;i++){
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
			if(wlan_idx==1)
#else
			if(wlan_idx==0)
#endif
			if(i!=KEY_HEX)
				continue;
#endif
		   	boaWrite(wp, "<option %s value=\"%d\">%s</option>\n", (i==vChar)?"selected":"", i, format[i]);
	       }
		return 0;
	}
#endif
	if ( !strcmp(name, "wlmode") ) {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		vChar = Entry.wlanMode;
		if (vChar == AP_MODE) {
			boaWrite(wp, "<option selected value=\"0\">AP</option>\n" );
#ifdef WLAN_CLIENT
			boaWrite(wp, "<option value=\"1\">Client</option>\n" );
#endif
#ifdef WLAN_WDS
			boaWrite(wp, "<option value=\"3\">AP+WDS</option>\n" );
#endif
		}
#ifdef WLAN_CLIENT
		if (vChar == CLIENT_MODE) {
			boaWrite(wp, "<option value=\"0\">AP</option>\n" );
			boaWrite(wp, "<option selected value=\"1\">Client</option>\n" );
#ifdef WLAN_WDS

			boaWrite(wp, "<option value=\"3\">AP+WDS</option>\n" );
#endif
		}
#endif
#ifdef WLAN_WDS
		if (vChar == AP_WDS_MODE) {
			boaWrite(wp, "<option value=\"0\">AP</option>\n" );
#ifdef WLAN_CLIENT
			boaWrite(wp, "<option value=\"1\">Client</option>\n" );
#endif
			boaWrite(wp, "<option selected value=\"3\">AP+WDS</option>\n" );
		}
#endif
		return 0;
	}
#ifdef WLAN_WDS
	if ( !strcmp(name, "wlanWdsEnabled") ) {
		if ( !mib_get( MIB_WLAN_WDS_ENABLED, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}

#endif
	if ( !strcmp(name, "mband") ) {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		boaWrite(wp, "%d", Entry.wlanBand);
		return 0;
	}
	if ( !strcmp(name, "band") ) {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		boaWrite(wp, "%d", Entry.wlanBand-1);
		return 0;
	}
	if ( !strcmp(name, "wlband") ) {
#ifdef WIFI_TEST
		boaWrite(wp, "<option value=3>WiFi-G</option>\n" );
		boaWrite(wp, "<option value=4>WiFi-BG</option>\n" );
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
		if ( !mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar) )
			return -1;

#if defined (CONFIG_RTL_92D_SUPPORT)
		unsigned char wlanBand2G5GSelect;
		if ( !mib_get(MIB_WLAN_BAND2G5G_SELECT, (void *)&wlanBand2G5GSelect) )
				return -1;
		if((vChar == PHYBAND_5G) || (wlanBand2G5GSelect == BANDMODESINGLE))
#else
		if(vChar == PHYBAND_5G)
#endif
		{
#ifdef WLAN_QTN
			boaWrite(wp, "<option value=3>5 GHz (A)</option>\n" );
			boaWrite(wp, "<option value=11>5 GHz (A+N)</option>\n" );
			boaWrite(wp, "<option value=63>5 GHz (AC)</option>\n" );
#else
			boaWrite(wp, "<option value=3>5 GHz (A)</option>\n" );
			boaWrite(wp, "<option value=7>5 GHz (N)</option>\n" );
			boaWrite(wp, "<option value=11>5 GHz (A+N)</option>\n" );
#if defined (WLAN0_5G_11AC_SUPPORT) || defined(WLAN1_5G_11AC_SUPPORT)
			boaWrite(wp, "<option value=63>5 GHz (AC)</option>\n" );
			boaWrite(wp, "<option value=71>5 GHz (N+AC)</option>\n" );
			boaWrite(wp, "<option value=75>5 GHz (A+N+AC)</option>\n" );
#endif
#endif
		}
#endif

#if (defined (WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)) && !defined(WLAN_DUALBAND_CONCURRENT)
		boaWrite(wp, "<option value=3>5 GHz (A)</option>\n" );
		boaWrite(wp, "<option value=7>5 GHz (N)</option>\n" );
		boaWrite(wp, "<option value=11>5 GHz (A+N)</option>\n" );
#if defined (WLAN0_5G_11AC_SUPPORT) || defined(WLAN1_5G_11AC_SUPPORT)
		boaWrite(wp, "<option value=63>5 GHz (AC)</option>\n" );
		boaWrite(wp, "<option value=71>5 GHz (N+AC)</option>\n" );		
		boaWrite(wp, "<option value=75>5 GHz (A+N+AC)</option>\n" );
#endif
#endif

#if defined (CONFIG_RTL_92D_SUPPORT)
		if((vChar == PHYBAND_2G) || (wlanBand2G5GSelect == BANDMODESINGLE))
#elif defined(WLAN_DUALBAND_CONCURRENT)
		if(vChar == PHYBAND_2G)
#endif
		{
			boaWrite(wp, "<option value=0>2.4 GHz (B)</option>\n");
			boaWrite(wp, "<option value=1>2.4 GHz (G)</option>\n");
			boaWrite(wp, "<option value=2>2.4 GHz (B+G)</option>\n");
			boaWrite(wp, "<option value=7>2.4 GHz (N)</option>\n" );
			boaWrite(wp, "<option value=9>2.4 GHz (G+N)</option>\n" );
			boaWrite(wp, "<option value=10>2.4 GHz (B+G+N)</option>\n" );
		}
		return 0;
	}
	if ( !strcmp(name, "wlchanwid") ) {
		boaWrite(wp, "<option value=\"0\">20MHZ</option>\n" );
		boaWrite(wp, "<option value=\"1\">40MHZ</option>\n" );
#if defined (WLAN0_5G_11AC_SUPPORT) || defined(WLAN1_5G_11AC_SUPPORT)
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
		if(vChar == PHYBAND_5G) {
			boaWrite(wp, "<option value=\"2\">80MHZ</option>\n" );
		}
#endif
#if defined (WLAN_11N_COEXIST)
#if defined (WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
		if(vChar == PHYBAND_2G)
#endif
			boaWrite(wp, "<option value=\"3\">20/40MHZ</option>\n" );
#endif
		return 0;
	}
	if ( !strcmp(name, "wlctlband") ) {
#if defined (WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
		if(vChar == PHYBAND_5G) {
			boaWrite(wp, "<option value=\"0\">%s</option>\n", multilang(LANG_AUTO) );
			boaWrite(wp, "<option value=\"1\">%s</option>\n", multilang(LANG_AUTO) );
		}
		else{
			boaWrite(wp, "<option value=\"0\">%s</option>\n", multilang(LANG_UPPER) );
			boaWrite(wp, "<option value=\"1\">%s</option>\n", multilang(LANG_LOWER) );
		}

#else
		boaWrite(wp, "<option value=\"0\">%s</option>\n", multilang(LANG_UPPER) );
		boaWrite(wp, "<option value=\"1\">%s</option>\n", multilang(LANG_LOWER) );
#endif
		return 0;
	}
	// Added by Mason Yu for TxPower
	if ( !strcmp(name, "txpower") ) {
		boaWrite(wp, "<option value=\"0\">100%%</option>\n" );
		boaWrite(wp, "<option value=\"1\">70%%</option>\n" );
		boaWrite(wp, "<option value=\"2\">50%%</option>\n" );
		boaWrite(wp, "<option value=\"3\">35%%</option>\n" );
		boaWrite(wp, "<option value=\"4\">15%%</option>\n" );
		return 0;
	}

	if (!strcmp(name, "wifiSecurity")) {
		unsigned char mode = 0;
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		mode = Entry.wlanMode;
		boaWrite(wp, "<option value=%d>%s</option>\n", WIFI_SEC_NONE, multilang(LANG_NONE));
#ifdef WLAN_QTN //QTN do not support WEP
#ifdef WLAN1_QTN
		if(wlan_idx==0)
#elif defined(WLAN0_QTN)
		if(wlan_idx==1)
#endif
#endif
		boaWrite(wp, "<option value=%d>WEP</option>\n", WIFI_SEC_WEP);
#ifndef NEW_WIFI_SEC
		boaWrite(wp, "<option value=%d>WPA</option>\n", WIFI_SEC_WPA);
#endif
		boaWrite(wp, "<option value=%d>WPA2</option>\n", WIFI_SEC_WPA2);
		if (mode != CLIENT_MODE)
		boaWrite(wp, "<option value=%d>%s</option>\n", WIFI_SEC_WPA2_MIXED, multilang(LANG_WPA2_MIXED));
#ifdef CONFIG_RTL_WAPI_SUPPORT
		boaWrite(wp, "<option value=%d>WAPI</option>\n", WIFI_SEC_WAPI);
#endif
		return 0;
	}
#ifdef WLAN_CLIENT
	if (!strcmp(name, "wifiClientSecurity")) {
		boaWrite(wp, "<option value=%d>%s</option>\n", WIFI_SEC_NONE, multilang(LANG_NONE));
		boaWrite(wp, "<option value=%d>WEP</option>\n", WIFI_SEC_WEP);
		boaWrite(wp, "<option value=%d>WPA</option>\n", WIFI_SEC_WPA);
		boaWrite(wp, "<option value=%d>WPA2</option>\n", WIFI_SEC_WPA2);
		boaWrite(wp, "<option value=%d>%s</option>\n", WIFI_SEC_WPA2_MIXED, multilang(LANG_WPA2_MIXED));
#ifdef CONFIG_RTL_WAPI_SUPPORT
		boaWrite(wp, "<option value=%d>WAPI</option>\n", WIFI_SEC_WAPI);
#endif
		return 0;
	}
#endif
	#ifdef WLAN_UNIVERSAL_REPEATER
	if ( !strcmp(name, "repeaterEnabled") ) {
		mib_get( MIB_REPEATER_ENABLED1, (void *)&vChar);
		if (vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	#endif
	if( !strcmp(name, "wlan_idx") ) {
		boaWrite(wp, "%d", wlan_idx);
		return 0;
	}
	if( !strcmp(name, "2G_ssid") ) {
		char ssid[200];
		int i, orig_wlan_idx = wlan_idx;
		MIB_CE_MBSSIB_T Entry;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
		for(i=0; i<NUM_WLAN_INTERFACE; i++) {
			wlan_idx = i;
			mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
			if(vChar == PHYBAND_2G) {
				if(!wlan_getEntry(&Entry, 0))
					return -1;
				strcpy(ssid, Entry.ssid);
				translate_web_code(ssid);
				boaWrite(wp, "%s", ssid);
				break;
			}
		}
#else
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		strcpy(ssid, Entry.ssid);
		translate_web_code(ssid);
		boaWrite(wp, "%s", ssid);
#endif
		wlan_idx = orig_wlan_idx;
		return 0;
	}
	if( !strcmp(name, "5G_ssid") ) {
		char ssid[200];
		int i, orig_wlan_idx = wlan_idx;
		MIB_CE_MBSSIB_T Entry;
#if defined (WLAN_QTN)
#ifdef WLAN1_QTN
		wlan_idx = 1;
#else WLAN0_QTN
		wlan_idx = 0;
#endif			
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		strcpy(ssid, Entry.ssid);
		translate_web_code(ssid);
		boaWrite(wp, "%s", ssid);
#elif defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
		for(i=0; i<NUM_WLAN_INTERFACE; i++) {
			wlan_idx = i;
			mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
			if(vChar == PHYBAND_5G) {
				if(!wlan_getEntry(&Entry, 0))
					return -1;
				strcpy(ssid, Entry.ssid);
				translate_web_code(ssid);
				boaWrite(wp, "%s", ssid);
				break;
			}
		}
#else
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		strcpy(ssid, Entry.ssid);
		boaWrite(wp, "%s", ssid);
#endif
		wlan_idx = orig_wlan_idx;
		return 0;
	}
	if (!strcmp(name, "dfs_enable")) {
#if defined(CONFIG_RTL_DFS_SUPPORT)
		boaWrite(wp, "1");
#else
		boaWrite(wp, "0");
#endif
		return 0;
	}
	if( !strcmp(name, "Band2G5GSupport") ) {
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
		boaWrite(wp, "%d", vChar);
#else
		vChar = PHYBAND_2G;
		boaWrite(wp, "%d", vChar);
#endif //CONFIG_RTL_92D_SUPPORT
		return 0;
	}
#if defined(CONFIG_RTL_92D_SUPPORT)
	if( !strcmp(name, "wlanBand2G5GSelect") ) {
		mib_get( MIB_WLAN_BAND2G5G_SELECT, (void *)&vChar);
		boaWrite(wp, "%d", vChar);
		return 0;
	}
	if( !strcmp(name, "onoff_dmdphy_comment_start") ) {
#ifdef CONFIG_RTL_92D_DMDP
		boaWrite(wp, "");
#else //CONFIG_RTL_92D_DMDP
		boaWrite(wp, "<!--");
#endif //CONFIG_RTL_92D_DMDP
		return 0;
	}

	if( !strcmp(name, "onoff_dmdphy_comment_end") ) {
#ifdef CONFIG_RTL_92D_DMDP
		boaWrite(wp, "");
#else //CONFIG_RTL_92D_DMDP
		boaWrite(wp, "-->");
#endif //CONFIG_RTL_92D_DMDP
		return 0;
	}
#elif defined(WLAN_DUALBAND_CONCURRENT)
	if( !strcmp(name, "wlanBand2G5GSelect") ) {
		boaWrite(wp, "%d", BANDMODEBOTH);
		return 0;
	}
#elif defined(WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)
	if( !strcmp(name, "wlanBand2G5GSelect") ) {
		boaWrite(wp, "%d", BANDMODESINGLE);
		return 0;
	}
#else
	if( !strcmp(name, "wlanBand2G5GSelect") ) {
		boaWrite(wp, "");
		return 0;
	}
#endif //CONFIG_RTL_92D_SUPPORT
	if ( !strcmp(name, "wlan_mssid_num")) {
		boaWrite(wp, "%d", WLAN_MBSSID_NUM);
		return 0;
	}
	if(!strcmp(name,"new_wifi_security")) //wifi security requirements after 2014.01.01
	{
#ifdef NEW_WIFI_SEC
		boaWrite(wp, "1");
#else
		boaWrite(wp, "0");
#endif
		return 0;
	}
	if( !strcmp(name, "is_regdomain_demo") ) {
		mib_get( MIB_WIFI_REGDOMAIN_DEMO, (void *)&vChar);
		boaWrite(wp, "%d", vChar);
		return 0;
	}
	if ( !strcmp(name, "regdomain_list") ) {
		boaWrite(wp, "<option value=\"1\">FCC(1)</option>\n" );
		boaWrite(wp, "<option value=\"2\">IC(2)</option>\n" );
		boaWrite(wp, "<option value=\"3\">ETSI(3)</option>\n" );
		boaWrite(wp, "<option value=\"4\">SPAIN(4)</option>\n" );
		boaWrite(wp, "<option value=\"5\">FRANCE(5)</option>\n" );
		boaWrite(wp, "<option value=\"6\">MKK(6)</option>\n" );
		boaWrite(wp, "<option value=\"7\">ISREAL(7)</option>\n" );
		boaWrite(wp, "<option value=\"8\">MKK1(8)</option>\n" );
		boaWrite(wp, "<option value=\"9\">MKK2(9)</option>\n" );
		boaWrite(wp, "<option value=\"10\">MKK3(10)</option>\n" );
		boaWrite(wp, "<option value=\"11\">NCC(11)</option>\n" );
		boaWrite(wp, "<option value=\"12\">RUSSIAN(12)</option>\n" );
		boaWrite(wp, "<option value=\"13\">CN(13)</option>\n" );
		boaWrite(wp, "<option value=\"14\">GLOBAL(14)</option>\n" );
		boaWrite(wp, "<option value=\"15\">WORLD-WIDE(15)</option>\n" );

		return 0;
	}
	if(!strcmp(name, "11w_support")){
#ifdef WLAN_11W
		boaWrite(wp, "1");
#else
		boaWrite(wp, "0");
#endif
		return 0;
	}
#ifdef WLAN_11R
	if(!strcmp(name, "11r_ftkh_num")){
		boaWrite(wp, "%d", MAX_VWLAN_FTKH_NUM);
		return 0;
	}
#endif
#ifdef WLAN_QTN
	if(!strcmp(name, "wlan_qtn_hidden_function")){
#ifdef WLAN1_QTN
		if(wlan_idx==1)
#else
		if(wlan_idx==0)
#endif
			boaWrite(wp, "style=\"display: none\"");
		return 0;
	}
#endif
	if(!strcmp(name, "is_wlan_qtn")){
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(wlan_idx==1)
#else
		if(wlan_idx==0)
#endif
			boaWrite(wp, "1");
		else
#endif
		boaWrite(wp, "0");
		return 0;
	}
	if(!strcmp(name, "ch_list_20")){
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(wlan_idx==1)
		{
			int mib_id = MIB_HW_WLAN1_REG_DOMAIN;
#else
		if(wlan_idx==0)
		{
			int mib_id = MIB_HW_REG_DOMAIN;
#endif
		
			char regdomain;
			char ch_list[1025];
			mib_get(mib_id, &regdomain);
			if(regdomain==0)
				boaWrite(wp, "0");
			else{
				rt_qcsapi_get_channel_list(rt_get_qtn_ifname(getWlanIfName()), regdomain, 20, ch_list);
				boaWrite(wp, "\"%s\"", ch_list);
			}
		}
		else
#endif
		boaWrite(wp, "0");

		return 0;
	}
	if(!strcmp(name, "ch_list_40")){
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(wlan_idx==1)
		{
			int mib_id = MIB_HW_WLAN1_REG_DOMAIN;
#else
		if(wlan_idx==0)
		{
			int mib_id = MIB_HW_REG_DOMAIN;
#endif
			char regdomain;
			char ch_list[1025];
			mib_get(mib_id, &regdomain);
			if(regdomain==0)
				boaWrite(wp, "0");
			else{
				rt_qcsapi_get_channel_list(rt_get_qtn_ifname(getWlanIfName()), regdomain, 40, ch_list);
				boaWrite(wp, "\"%s\"", ch_list);
			}
		}
		else
#endif
		boaWrite(wp, "0");

		return 0;
	}
	if(!strcmp(name, "ch_list_80")){
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(wlan_idx==1)
		{
			int mib_id = MIB_HW_WLAN1_REG_DOMAIN;
#else
		if(wlan_idx==0)
		{
			int mib_id = MIB_HW_REG_DOMAIN;
#endif
			char regdomain;
			char ch_list[1025];
			mib_get(mib_id, &regdomain);
			if(regdomain==0)
				boaWrite(wp, "0");
			else{
				rt_qcsapi_get_channel_list(rt_get_qtn_ifname(getWlanIfName()), regdomain, 80, ch_list);
				boaWrite(wp, "\"%s\"", ch_list);
			}
		}
		else
#endif
		boaWrite(wp, "0");

		return 0;
	}
	if(!strcmp(name,"wlan_support_11ncoexist"))
	{
#ifdef	WLAN_11N_COEXIST
		boaWrite(wp, "1");
#else
		boaWrite(wp, "0");
#endif
	}
#endif // of WLAN_SUPPORT
	if(!strcmp(name,"wlan_support_8812e")) //8812
	{
#if defined(CONFIG_WLAN_HAL_8814AE) ||defined(CONFIG_RTL_8812_SUPPORT) || (defined(WLAN_DUALBAND_CONCURRENT) &&!defined(WLAN_QTN))
		boaWrite(wp, "1");
#else
		boaWrite(wp, "0");
#endif
		return 0;
	}
	if ( !strcmp(name, "wlan_fon")) {
#ifdef CONFIG_USER_FON
			boaWrite(wp, "1");
#else
			boaWrite(wp, "0");
#endif
		return 0;
	}
#ifdef CONFIG_USER_FON
	if ( !strcmp(name, "wlan_fon_onoffline")) {
		mib_get( MIB_FON_ONOFF, (void *)&vChar);
		boaWrite(wp, "%d", vChar);
		return 0;
	}
#endif
	// Added by Mason Yu for 2 level web page
	if ( !strcmp(name, "userMode") ) {
		#ifdef ACCOUNT_CONFIG
		MIB_CE_ACCOUNT_CONFIG_T Entry;
		int totalEntry, i;
		#else
		char suStr[100], usStr[100];
		#endif
#ifdef ACCOUNT_CONFIG
		#ifdef USE_LOGINWEB_OF_SERVER
		if (!strcmp(g_login_username, suName))
		#else
		if (!strcmp(wp->user, suName))
		#endif
		{
			boaWrite(wp, "<option selected value=\"0\">%s</option>\n", suName);
			boaWrite(wp, "<option value=\"1\">%s</option>\n", usName);
		}
		#ifdef USE_LOGINWEB_OF_SERVER
		else if (!strcmp(g_login_username, usName))
		#else
		else if (!strcmp(wp->user, usName))
		#endif
		{
			boaWrite(wp, "<option value=\"0\">%s</option>\n", suName);
			boaWrite(wp, "<option selected value=\"1\">%s</option>\n", usName);
		}
		totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
				continue;
			#ifdef USE_LOGINWEB_OF_SERVER
			if (!strcmp(g_login_username, Entry.userName))
			#else
			if (strcmp(wp->user, Entry.userName) == 0)
			#endif
				boaWrite(wp, "<option selected value=\"%d\">%s</option>\n", i+2, Entry.userName);
			else
				boaWrite(wp, "<option value=\"%d\">%s</option>\n", i+2, Entry.userName);
		}
#else
		#ifdef USE_LOGINWEB_OF_SERVER
		if (!strcmp(g_login_username, suName))
		#else
		if(!strcmp(wp->user,suName))
		#endif
			{
			sprintf(suStr, "<option selected value=\"0\">%s</option>\n", suName);
			sprintf(usStr, "<option value=\"1\">%s</option>\n", usName);
			}
		else
			sprintf(usStr, "<option selected value=\"1\">%s</option>\n", usName);

		boaWrite(wp, suStr );
		boaWrite(wp, usStr );
#endif
		return 0;
	}
	if ( !strcmp(name, "lan-dhcp-st") ) {
		#ifdef CONFIG_USER_DHCP_SERVER
		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;

		if (DHCP_LAN_SERVER == vChar)
			boaWrite(wp, (char *)multilang(LANG_ENABLED));
		else
		#endif
			boaWrite(wp, (char *)multilang(LANG_DISABLED));
		return 0;
	}
	else if ( !strcmp(name, "br-stp-0") ) {
		if ( !mib_get( MIB_BRCTL_STP, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "br-stp-1") ) {
		if ( !mib_get( MIB_BRCTL_STP, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#ifdef CONFIG_USER_IGMPPROXY
	else if ( !strcmp(name, "igmpProxy0") ) {
		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			boaWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, "igmpProxy1") ) {
		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			boaWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, "igmpProxy0d") ) {
		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "disabled");
		return 0;
	}
#endif
#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_PPTPD_PPTPD)
	else if (!strcmp(name, "pptpenable0")) {
		if ( !mib_get( MIB_PPTP_ENABLE, (void *)&vUInt) )
			return -1;
		//printf("pptp %s\n", vUInt?"enable":"disable");
		if (0 == vUInt)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "pptpenable1")) {
		if ( !mib_get( MIB_PPTP_ENABLE, (void *)&vUInt) )
			return -1;
		//printf("pptp %s\n", vUInt?"enable":"disable");
		if (1 == vUInt)
			boaWrite(wp, "checked");
		return 0;
	}
#endif //end of CONFIG_USER_PPTP_CLIENT_PPTP
#if defined(CONFIG_USER_L2TPD_L2TPD) || defined(CONFIG_USER_L2TPD_LNS)
	else if (!strcmp(name, "l2tpenable0")) {
		if (!mib_get( MIB_L2TP_ENABLE, (void *)&vUInt))
			return -1;
		if (0 == vUInt)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "l2tpenable1")) {
		if ( !mib_get( MIB_L2TP_ENABLE, (void *)&vUInt) )
			return -1;
		if (1 == vUInt)
			boaWrite(wp, "checked");
		return 0;
	}
#endif //endof CONFIG_USER_L2TPD_L2TPD
#ifdef CONFIG_NET_IPIP
	else if (!strcmp(name, "ipipenable0")) {
		if (!mib_get( MIB_IPIP_ENABLE, (void *)&vUInt))
			return -1;
		if (0 == vUInt)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "ipipenable1")) {
		if ( !mib_get( MIB_IPIP_ENABLE, (void *)&vUInt) )
			return -1;
		if (1 == vUInt)
			boaWrite(wp, "checked");
		return 0;
	}
#endif//endof CONFIG_NET_IPIP
#ifdef CONFIG_USER_MINIUPNPD
	else if ( !strcmp(name, "upnp0") ) {
		if ( !mib_get( MIB_UPNP_DAEMON, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			boaWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, "upnp1") ) {
		if ( !mib_get( MIB_UPNP_DAEMON, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		if (ifWanNum("rt") ==0)
			boaWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, "upnp0d") ) {
		//if ( !mib_get( MIB_UPNP_DAEMON, (void *)&vChar) )
		//	return -1;
		if (ifWanNum("rt") ==0)
			boaWrite(wp, "disabled");
		return 0;
	}
#ifdef CONFIG_TR_064
	else if(!strcmp(name, "tr064_switch"))
	{
		int sw = TR064_STATUS;

		boaWrite(wp, "<tr>\n");
		boaWrite(wp, "\t\t<td><b>%s:</b></td>\n", multilang(LANG_TR064));
		boaWrite(wp, "\t\t<td>\n");
		boaWrite(wp, "\t\t\t<input type=radio value=0 name=tr_064_sw %s>%s&nbsp;&nbsp;\n", sw? "":"checked", multilang(LANG_DISABLE));
		boaWrite(wp, "\t\t\t<input type=radio value=1 name=tr_064_sw %s>%s\n", sw? "checked": "", multilang(LANG_ENABLE));
		boaWrite(wp, "\t\t</td>\n\t</tr>\n");
	}
#endif
#endif
#ifdef TIME_ZONE
	else if ( !strcmp(name, "sntp0d") ) {		
		if (ifWanNum("rt") ==0)
			boaWrite(wp, "disabled");
		return 0;
	}
#endif
	else if(!strcmp(name, "config_rtk_rg"))
	{
#ifdef CONFIG_RTK_RG_INIT
		boaWrite(wp, "\"yes\"");
#else
		boaWrite(wp, "\"no\"");
#endif
	}
#ifdef CONFIG_RTK_RG_INIT
	else if(!strcmp(name, "rg_hidden_function"))
	{
		boaWrite(wp, "style=\"display: none\"");
	}
	else if(!strcmp(name, "show_vlan_id"))
	{
		int elan_vid;

		mib_get(MIB_LAN_VLAN_ID1, (void *)&elan_vid);

		boaWrite(wp, "				if(itlk.cmode == 0)\n");
		boaWrite(wp, "					vid.value = %d\n", elan_vid);
		boaWrite(wp, "				else\n");
		boaWrite(wp, "					vid.value = 8\n");
	}
#else
	else if(!strcmp(name, "rg_only_function"))
	{
		boaWrite(wp, "style=\"display: none\"");
	}
#endif
// Mason Yu. MLD Proxy
#ifdef CONFIG_IPV6
	else if ( !strcmp(name, "ipv6enabledisable0") ) {
		if ( !mib_get( MIB_V6_IPV6_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "ipv6enabledisable1") ) {
		if ( !mib_get( MIB_V6_IPV6_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#ifdef CONFIG_USER_ECMH
	else if ( !strcmp(name, "mldproxy0") ) {
		if ( !mib_get( MIB_MLD_PROXY_DAEMON, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		if (ifWanNum("rtv6") ==0)
			boaWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, "mldproxy1") ) {
		if ( !mib_get( MIB_MLD_PROXY_DAEMON, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		if (ifWanNum("rtv6") ==0)
			boaWrite(wp, " disabled");
		return 0;
	}
	else if ( !strcmp(name, "mldproxy0d") ) {
		//if ( !mib_get( MIB_MLD_PROXY_DAEMON, (void *)&vChar) )
		//	return -1;
		if (ifWanNum("rtv6") ==0)
			boaWrite(wp, "disabled");
		return 0;
	}
#endif

#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
	else if ( !strcmp(name, "prefix_delegation_info") ) {
		struct in6_addr ip6Prefix;
		unsigned char value[48], len;

		len = cmd_get_PD_prefix_len();
		if (0 == len) {
			boaWrite(wp, "");
		}
		else {
			cmd_get_PD_prefix_ip((void *)&ip6Prefix);
			inet_ntop(PF_INET6, &ip6Prefix, value, sizeof(value));
			boaWrite(wp, "%s/%d", value, len);
		}
		return 0;
	}
#endif
#endif	// #ifdef CONFIG_IPV6

#ifdef NAT_CONN_LIMIT
	else if (!strcmp(name, "connlimit")) {
		if (!mib_get(MIB_NAT_CONN_LIMIT, (void *)&vChar))
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
	}
#endif
#ifdef TCP_UDP_CONN_LIMIT
	else if ( !strcmp(name, "connLimit-cap0") ) {
   		if ( !mib_get( MIB_CONNLIMIT_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "connLimit-cap1") ) {
   		if ( !mib_get( MIB_CONNLIMIT_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}

#endif
#ifdef IP_ACL
	else if ( !strcmp(name, "acl-cap0") ) {
   		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "acl-cap1") ) {
   		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	else if ( !strcmp(name, "snmpd-on") ) {
   		if ( !mib_get( MIB_SNMPD_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "snmpd-off") ) {
   		if ( !mib_get( MIB_SNMPD_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
#ifdef URL_BLOCKING_SUPPORT
	else if ( !strcmp(name, "url-cap0") ) {
   		if ( !mib_get( MIB_URL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "url-cap1") ) {
   		if ( !mib_get( MIB_URL_CAPABILITY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
//alex_huang
#ifdef URL_ALLOWING_SUPPORT
       else if( !strcmp(name ,"url-cap2") ) {
	   	if( !mib_get (MIB_URL_CAPABILITY,(void*)&vChar) )
			return -1;
		if(2 == vChar)
			{
			    boaWrite(wp, "checked");
			}
		return 0;

       	}
#endif


#ifdef DOMAIN_BLOCKING_SUPPORT
	else if ( !strcmp(name, "domainblk-cap0") ) {
   		if ( !mib_get( MIB_DOMAINBLK_CAPABILITY, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "domainblk-cap1") ) {
   		if ( !mib_get( MIB_DOMAINBLK_CAPABILITY, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
	else if ( !strcmp(name, "dns0") ) {
		if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "dns1") ) {
		if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#ifdef PORT_FORWARD_GENERAL
	else if ( !strcmp(name, "portFw-cap0") ) {
   		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "portFw-cap1") ) {
   		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "portFwNum")) {
		vUInt = mib_chain_total(MIB_PORT_FW_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef NATIP_FORWARDING
	else if ( !strcmp(name, "ipFwEn")) {
		if ( !mib_get( MIB_IP_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	if ( !strcmp(name, "ipFwNum")) {
		vUInt = mib_chain_total(MIB_IP_FW_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_RADVD
	else if ( !strcmp(name, "radvd_SendAdvert0")) {
		if ( !mib_get( MIB_V6_SENDADVERT, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_SendAdvert1")) {
		if ( !mib_get( MIB_V6_SENDADVERT, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_ManagedFlag0")) {
		if ( !mib_get( MIB_V6_MANAGEDFLAG, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_ManagedFlag1")) {
		if ( !mib_get( MIB_V6_MANAGEDFLAG, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_OtherConfigFlag0")) {
		if ( !mib_get( MIB_V6_OTHERCONFIGFLAG, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_OtherConfigFlag1")) {
		if ( !mib_get( MIB_V6_OTHERCONFIGFLAG, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_EnableULAFlag0")) {
		if ( !mib_get( MIB_V6_ULAPREFIX_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_EnableULAFlag1")) {
		if ( !mib_get( MIB_V6_ULAPREFIX_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_OnLink0")) {
		if ( !mib_get( MIB_V6_ONLINK, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_OnLink1")) {
		if ( !mib_get( MIB_V6_ONLINK, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_Autonomous0")) {
		if ( !mib_get( MIB_V6_AUTONOMOUS, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_Autonomous1")) {
		if ( !mib_get( MIB_V6_AUTONOMOUS, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif // of CONFIG_USER_RADVD
#endif
#ifdef IP_PORT_FILTER
	else if ( !strcmp(name, "ipf_out_act0")) {
		if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "ipf_out_act1")) {
		if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "ipf_in_act0")) {
		if ( !mib_get( MIB_IPF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "ipf_in_act1")) {
		if ( !mib_get( MIB_IPF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
#ifdef CONFIG_IPV6
	else if ( !strcmp(name, "v6_ipf_out_act0")) {
		if ( !mib_get( MIB_V6_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "v6_ipf_out_act1")) {
		if ( !mib_get( MIB_V6_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "v6_ipf_in_act0")) {
		if ( !mib_get( MIB_V6_IPF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "v6_ipf_in_act1")) {
		if ( !mib_get( MIB_V6_IPF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
	else if ( !strcmp(name, "macf_out_act0")) {
		if ( !mib_get( MIB_MACF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "macf_out_act1")) {
		if ( !mib_get( MIB_MACF_OUT_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "macf_in_act0")) {
		if ( !mib_get( MIB_MACF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "macf_in_act1")) {
		if ( !mib_get( MIB_MACF_IN_ACTION, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#ifdef DMZ
	else if ( !strcmp(name, "dmz-cap0") ) {
   		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "dmz-cap1") ) {
   		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
#ifdef CONFIG_USER_VLAN_ON_LAN
	else if ( !strcmp(name, "lan1-vid-cap0") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 0, &sw_entry))
			return -1;
		if (0 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "lan1-vid-cap1") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 0, &sw_entry))
			return -1;
		if (1 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "lan2-vid-cap0") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 1, &sw_entry))
			return -1;
		if (0 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "lan2-vid-cap1") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 1, &sw_entry))
			return -1;
		if (1 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "lan3-vid-cap0") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 2, &sw_entry))
			return -1;
		if (0 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "lan3-vid-cap1") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 2, &sw_entry))
			return -1;
		if (1 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "lan4-vid-cap0") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 3, &sw_entry))
			return -1;
		if (0 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "lan4-vid-cap1") ) {
		if ( !mib_chain_get(MIB_SW_PORT_TBL, 3, &sw_entry))
			return -1;
		if (1 == sw_entry.vlan_on_lan_enabled)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
#ifdef IP_PORT_FILTER
	else if ( !strcmp(name, "ipFilterNum")) {
		vUInt = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef CONFIG_IPV6
	else if ( !strcmp(name, "ipFilterNumV6")) {
		vUInt = mib_chain_total(MIB_V6_IP_PORT_FILTER_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef TCP_UDP_CONN_LIMIT
	else if ( !strcmp(name, "connLimitNum")) {
		vUInt = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef MULTI_ADDRESS_MAPPING
	else if ( !strcmp(name, "AddresMapNum")) {
		vUInt = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif  // end of MULTI_ADDRESS_MAPPING
#ifdef URL_BLOCKING_SUPPORT
	else if ( !strcmp(name, "keywdNum")) {
		vUInt = mib_chain_total(MIB_KEYWD_FILTER_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelKeywdButton();");
		return 0;
	}
	else if ( !strcmp(name, "FQDNNum")) {
		vUInt = mib_chain_total(MIB_URL_FQDN_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelFQDNButton();");
		return 0;
	}
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
	else if ( !strcmp(name, "domainNum")) {
		vUInt = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
	else if ( !strcmp(name, "ripNum")) {
		vUInt = mib_chain_total(MIB_RIP_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#ifdef IP_ACL
	else if ( !strcmp(name, "aclNum")) {
		vUInt = mib_chain_total(MIB_ACL_IP_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef MAC_FILTER
	else if ( !strcmp(name, "macFilterNum")) {
		vUInt = mib_chain_total(MIB_MAC_FILTER_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#endif
#ifdef PARENTAL_CTRL
	else if ( !strcmp(name, "parentCtrlNum")) {
		vUInt = mib_chain_total(MIB_PARENTAL_CTRL_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
	else if ( !strcmp(name, "parental-ctrl-on-0") ) {
		if ( !mib_get( MIB_PARENTAL_CTRL_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "parental-ctrl-on-1") ) {
		if ( !mib_get( MIB_PARENTAL_CTRL_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
	else if ( !strcmp(name, "vcMax")) {
		vUInt = mib_chain_total(MIB_ATM_VC_TBL);
		if (vUInt >= 16) {
			boaWrite(wp, "alert(\"Max number of ATM VC Settings is 16!\");");
			boaWrite(wp, "return false;");
		}
		return 0;
	}
	else if ( !strcmp(name, "vcCount")) {
		vUInt = mib_chain_total(MIB_ATM_VC_TBL);
		if (vUInt == 0) {
			boaWrite(wp, "disableButton(document.adsl.delvc);");
			// Commented by Mason Yu. The "refresh" button is be disabled on wanadsl.asp
			//boaWrite(wp, "disableButton(document.adsl.refresh);");
		}
		return 0;
	}
#ifdef CONFIG_USER_PPPOE_PROXY
  else if(!strcmp(name,"pppoeProxy"))
  	{
  	boaWrite(wp,"<tr><td><font size=2><b>PPPoE Proxy:</b></td>"
         "<td><b><input type=\"radio\" value=1 name=\"pppEnable\" >Enable&nbsp;&nbsp;"
	"<input type=\"radio\" value=0 name=\"pppEnable\" checked>Disable</b></td></tr>");
  	}
  else if(!strcmp(name,"pppSettingsDisable"))
  	{
  	  boaWrite(wp,"{document.adsl.pppEnable[0].disabled=true;\n"
	  	"document.adsl.pppEnable[1].disabled=true;}");
  	}
    else if(!strcmp(name,"pppSettingsEnable"))
  	{
  	  boaWrite(wp,"{document.adsl.pppEnable[0].disabled=false;\n"
	  	"document.adsl.pppEnable[1].disabled=false;}else{document.adsl.pppEnable[0].disabled=true;\n"
	  	"document.adsl.pppEnable[1].disabled=true;}"
	  	"document.adsl.pppEnable[0].checked=false;"
	  	"document.adsl.pppEnable[1].checked=true;");
  	}

 #endif
  #ifdef CONFIG_USER_PPPOE_PROXY
     else if(!strcmp(name,"pppoeProxyEnable"))
     	{
	boaWrite(wp,"  if(mode==\"PPPoE\")"
		"{if(pppoeProxyEnable)"
		"{ document.adsl.pppEnable[0].checked=true;\n"
                  "document.adsl.pppEnable[1].checked=false;}\n"
		"else {document.adsl.pppEnable[0].checked=false;"
		 " document.adsl.pppEnable[1].checked=true;}  "
		" document.adsl.pppEnable[0].disabled=false;"
			  " document.adsl.pppEnable[1].disabled=false;");
	boaWrite(wp," }else"
		"{"
		"	  document.adsl.pppEnable[0].checked=false;"
		"	   document.adsl.pppEnable[1].checked=true;"
		"	   document.adsl.pppEnable[0].disabled=true;"
		"	   document.adsl.pppEnable[1].disabled=true;}"
		);
     	}
  #else
     else if(!strcmp(name,"pppoeProxyEnable"))
     	{

     	}
  #endif

#ifdef WLAN_SUPPORT
	else if ( !strcmp(name, "wl_txRate")) {
		struct _misc_data_ misc_data;
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		boaWrite(wp, "band=%d\n", Entry.wlanBand);
		boaWrite(wp, "txrate=%u\n",Entry.fixedTxRate);
		boaWrite(wp, "auto=%d\n",Entry.rateAdaptiveEnabled);
		mib_get( MIB_WLAN_CHANNEL_WIDTH, (void *)&vChar);
		boaWrite(wp, "chanwid=%d\n",vChar);

		//cathy, get rf number
		memset(&misc_data, 0, sizeof(struct _misc_data_));
		getMiscData(getWlanIfName(), &misc_data);
		boaWrite(wp, "rf_num=%u\n", misc_data.mimo_tr_used);
		return 0;
	}
	else if ( !strcmp(name, "wl_chno")) {
//xl_yue:
		mib_get( MIB_HW_REG_DOMAIN, (void *)&vChar);
		boaWrite(wp, "regDomain=%d\n",vChar);
#ifdef WLAN1_QTN
		mib_get( MIB_HW_WLAN1_REG_DOMAIN, (void *)&vChar);
		boaWrite(wp, "regDomain_qtn=%d\n",vChar);
#elif defined(WLAN0_QTN)
		mib_get( MIB_HW_REG_DOMAIN, (void *)&vChar);
		boaWrite(wp, "regDomain_qtn=%d\n",vChar);
#endif
		mib_get( MIB_WLAN_AUTO_CHAN_ENABLED,(void *)&vChar);
		if(vChar)
			boaWrite(wp, "defaultChan=0\n");
		else
		{
			mib_get( MIB_WLAN_CHAN_NUM ,(void *)&vChar);
			boaWrite(wp, "defaultChan=%d\n",vChar);
		}
		return 0;
	}
	else if ( !strcmp(name, "rf_used")) {
		struct _misc_data_ misc_data;
		memset(&misc_data, 0, sizeof(struct _misc_data_));
		getMiscData(getWlanIfName(), &misc_data);
		boaWrite(wp, "%u\n", misc_data.mimo_tr_used);
		return 0;
	}
#endif

	//for web log
	else if ( !strcmp(name, "log-cap0") ) {
   		if ( !mib_get( MIB_SYSLOG, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "log-cap1") ) {
   		if ( !mib_get( MIB_SYSLOG, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#ifdef CONFIG_USER_SAMBA
	else if (!strcmp(name, "samba-cap0")) {
		if (!mib_get(MIB_SAMBA_ENABLE, &vChar))
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	} else if (!strcmp(name, "samba-cap1")) {
		if (!mib_get(MIB_SAMBA_ENABLE, &vChar))
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "nmbd-cap")) {
#ifndef CONFIG_USER_NMBD
		boaWrite(wp, "style=\"display: none\"");
#endif
		return 0;
	}
#endif
	else if (!strcmp(name, "anxb-cap")) {
		mib_get(MIB_ADSL_MODE, (void *)&vUShort);
		if(vUShort&ADSL_MODE_ANXB)
			boaWrite(wp, "style=\"display: none\"");
		return 0;
	}
	else if (!strcmp(name, "ginp-cap")) {
#ifndef ENABLE_ADSL_MODE_GINP
		boaWrite(wp, "style=\"display: none\"");
#endif
		return 0;
	}
	else if (!strcmp(name, "vdsl-cap")) {
#ifndef CONFIG_VDSL
		boaWrite(wp, "style=\"display: none\"");
#endif
		return 0;
	}
	else if (!strcmp(name, "IPQoS")) {
#if !defined(CONFIG_USER_IP_QOS) || defined(BRIDGE_ONLY_ON_WEB)
		boaWrite(wp, "style=\"display: none\"");
#endif
		return 0;
	}
	else if (!strcmp(name, "bridge-only")) {
#ifdef CONFIG_SFU
		boaWrite(wp, "style=\"display: none\"");
#endif
		return 0;
	}

	if (!strcmp(name, "syslog-log") || !strcmp(name, "syslog-display")) {
		char *SYSLOGLEVEL[] = {"Emergency", "Alert", "Critical", "Error", "Warning", "Notice", "Infomational", "Debugging"};
		int i;
		if (!strcmp(name, "syslog-log")) {
			if (!mib_get(MIB_SYSLOG_LOG_LEVEL, (void *)&vChar))
				return -1;
		}
		else if (!strcmp(name, "syslog-display")) {
			if (!mib_get(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&vChar))
				return -1;
		}
		for (i=0; i<8; i++) {
			if (i == vChar)
				boaWrite(wp,"<option selected value=\"%d\">%s</option>", i, SYSLOGLEVEL[i]);
			else
				boaWrite(wp,"<option value=\"%d\">%s</option>", i, SYSLOGLEVEL[i]);
		}
		return 0;
	}
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
	if (!strcmp(name, "syslog-mode")) {
		char *SYSLOGMODE[] = { "", "Local", "Remote", "Both" };
		int i;
		if (!mib_get(MIB_SYSLOG_MODE, &vChar))
			return -1;
		for (i = 1; i <= 3; i++) {
			if (i == vChar)
				boaWrite(wp, "<option selected value=\"%d\">%s</option>", i, SYSLOGMODE[i]);
			else
				boaWrite(wp, "<option value=\"%d\">%s</option>", i, SYSLOGMODE[i]);
		}
	}
#endif

#ifdef _CWMP_MIB_
	else if ( !strcmp(name, "tr069-interval") ) {
   		if ( !mib_get( CWMP_INFORM_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "disabled");
		return 0;
	}
	else if ( !strcmp(name, "tr069-inform-0") ) {
   		if ( !mib_get( CWMP_INFORM_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-inform-1") ) {
   		if ( !mib_get( CWMP_INFORM_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-dbgmsg-0") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DEBUG_MSG)==0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-dbgmsg-1") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DEBUG_MSG)!=0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-sendgetrpc-0") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SENDGETRPC)==0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-sendgetrpc-1") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SENDGETRPC)!=0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-skipmreboot-0") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SKIPMREBOOT)==0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-skipmreboot-1") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_SKIPMREBOOT)!=0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-delay-0") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DELAY)==0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-delay-1") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_DELAY)!=0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-autoexec-0") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_AUTORUN)==0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-autoexec-1") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_AUTORUN)!=0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-enable-cwmp-1") ) {
   		if ( !mib_get( CWMP_FLAG2, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG2_CWMP_DISABLE)==0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-enable-cwmp-0") ) {
   		if ( !mib_get( CWMP_FLAG2, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG2_CWMP_DISABLE)!=0 )
			boaWrite(wp, "checked");
		return 0;
	}

#endif
#ifdef WLAN_SUPPORT
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	else if (!strcmp(name, "wpsVer") ) {
		#ifdef WPS20
			boaWrite(wp, "wps20 = 1;\n");
		#else
			boaWrite(wp, "wps20 = 0;\n");
		#endif
		return 0;
	}
	else if (!strcmp(name, "wpsVerConfig") ) {
		#ifdef WPS_VERSION_CONFIGURABLE
			boaWrite(wp, "wps_version_configurable = 1;\n");
		#else
			boaWrite(wp, "wps_version_configurable = 0;\n");
		#endif
		return 0;
	}
	else if (!strcmp(name, "wpsUseVersion")) {
		boaWrite(wp, "<option value=0>V1</option>\n");
		boaWrite(wp, "<option value=1>V2</option>\n");
		return 0;
	}
	else if (!strcmp(name, "wscConfig-0") ) {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		if (!Entry.wsc_configured) boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "wscConfig-1")) {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		if (Entry.wsc_configured)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "wscConfig-A")) {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		if (Entry.wsc_configured)
			boaWrite(wp, "isConfig=1;");
		else
			boaWrite(wp, "isConfig=0;");
		return 0;
	}
	else if (!strcmp(name,"wscConfig")){
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		if (Entry.wsc_configured)
			boaWrite(wp, "enableButton(form.elements['resetUnConfiguredBtn']);");
		else
			boaWrite(wp, "disableButton(form.elements['resetUnConfiguredBtn']);");
		return 0;
	}
	else if (!strcmp(name, "wlanMode"))  {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		if (Entry.wlanMode == CLIENT_MODE)
			boaWrite(wp, "isClient=1;");
		else
			boaWrite(wp, "isClient=0;");
		return 0;

	}
	else if (!strcmp(name, "wscDisable"))  {

		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		if (Entry.wsc_disabled)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "wps_auth"))  {

		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		switch(Entry.wsc_auth) {
			case WSC_AUTH_OPEN: boaWrite(wp, "Open"); break;
			case WSC_AUTH_WPAPSK: boaWrite(wp, "WPA PSK"); break;
			case WSC_AUTH_SHARED: boaWrite(wp, "WEP Shared"); break;
			case WSC_AUTH_WPA: boaWrite(wp, "WPA Enterprise"); break;

			case WSC_AUTH_WPA2: boaWrite(wp, "WPA2 Enterprise"); break;
			case WSC_AUTH_WPA2PSK: boaWrite(wp, "WPA2 PSK"); break;
			case WSC_AUTH_WPA2PSKMIXED: boaWrite(wp, "WPA2-Mixed PSK"); break;
			default:
				break;
		}
		return 0;
	}
	else if (!strcmp(name, "wps_enc"))  {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		vChar = Entry.encrypt;
		if ((WIFI_SECURITY_T)vChar == WIFI_SEC_WPA2_MIXED) {
			vUInt = 0;
			vUInt |= Entry.unicastCipher;
			vUInt |= Entry.wpa2UnicastCipher;
			switch (vUInt) {
				case WPA_CIPHER_TKIP: boaWrite(wp, "TKIP"); break;
				case WPA_CIPHER_AES: boaWrite(wp, "AES"); break;
				case WPA_CIPHER_MIXED: boaWrite(wp, "TKIP+AES"); break;
				default:
					break;
			}
		}
		else {
			switch(Entry.wsc_enc) {
				case 0:
				case WSC_ENCRYPT_NONE: boaWrite(wp, "None"); break;
				case WSC_ENCRYPT_WEP: boaWrite(wp, "WEP"); break;
				case WSC_ENCRYPT_TKIP: boaWrite(wp, "TKIP"); break;
				case WSC_ENCRYPT_AES: boaWrite(wp, "AES"); break;
				case WSC_ENCRYPT_TKIPAES: boaWrite(wp, "TKIP+AES"); break;
				default:
					break;
			}
		}
		return 0;
	}
	else if(!strcmp(name, "blockingvap"))
		{
			#ifndef CONFIG_RTK_RG_INIT
			boaWrite(wp, "<tr><td><b>%s%s", multilang(LANG_BLOCKING_BETWEEN_VAP),"</b></td>");
			boaWrite(wp, "<td><input type=\"radio\" name=\"mbssid_block\" value=\"disable\" >%s", multilang(LANG_DISABLE));
			boaWrite(wp, "<input type=\"radio\" name=\"mbssid_block\" value=\"enable\" >%s%s",multilang(LANG_ENABLE),"</td></tr>");
			#endif
		}
#endif
#endif
	// Jenny, for RFC1577
	if ( !strcmp(name, "adslcmode") ) {
#ifdef CONFIG_IPV6
	char vChar=-1;

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
#endif
#ifdef CONFIG_ATM_CLIP
		boaWrite(wp, "<option value=\"5\">1577 Routed</option>" );
#endif

#ifdef CONFIG_IPV6
	if (vChar == 1){
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
		boaWrite(wp, "<option value=\"6\">DS-Lite</option>" );
#endif
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
		boaWrite(wp, "<option value=\"8\">6rd</option>" );
#endif
	}
#endif
		return 0;
	}
	else if ( !strcmp(name, "ethcmode") ) {
#ifdef CONFIG_IPV6
	char vChar=-1;

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
	if (vChar == 1){
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
		boaWrite(wp, "<option value=\"6\">DS-Lite</option>" );
#endif
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
		boaWrite(wp, "<option value=\"8\">6rd</option>" );
#endif
	}
#endif
		return 0;
	}
//add by ramen for create new menu
       else if(!strcmp(name,"CreateMenu")){

	   createMenu(0,wp,0,0);
	   boaWrite(wp,"</body>\n</html>\n");
	   	return 0;
       	}if(!strcmp(name,"CreateMenu_user"))

       		{
       		createMenu_user(0,wp,0,0);
			return 0;
       		}

	else if(!strcmp(name,"naptEnable"))
	{
		boaWrite(wp,"\tif ((document.adsl.adslConnectionMode.selectedIndex == 1) ||\n"
			"\t\t(document.adsl.adslConnectionMode.selectedIndex == 2))\n"
			"\t\tdocument.adsl.naptEnabled.checked = true;\n"
			"\telse\n"
			"\t\tdocument.adsl.naptEnabled.checked = false;\n");
		return 0;
	}
	else if(!strcmp(name,"TrafficShapingByVid"))
	{
#if defined(QOS_TRAFFIC_SHAPING_BY_VLANID)&&defined(CONFIG_RTK_RG_INIT)
		boaWrite(wp,"1");
#else
		boaWrite(wp,"0");
#endif

	}
	else if(!strcmp(name,"TrafficShapingBySsid"))
	{
#if defined(QOS_TRAFFIC_SHAPING_BY_SSID)&&defined(CONFIG_RTK_RG_INIT)
		boaWrite(wp,"1");
#else
		boaWrite(wp,"0");
#endif

	}
	else if(!strcmp(name,"IPv6Show"))
	{
#ifdef CONFIG_IPV6
		if ( mib_get( MIB_V6_IPV6_ENABLE, (void *)&vChar) )
		{
			if (0 == vChar)
				boaWrite(wp,"0");
			else
				boaWrite(wp,"1");
		}
#else
		boaWrite(wp,"0");
#endif
		return 0;
	}
	else if(!strcmp(name,"LUNAShow"))
	{
#ifdef CONFIG_LUNA
		boaWrite(wp,"1");
#else
		boaWrite(wp,"0");
#endif
		return 0;
	}
	else if(!strcmp(name,"DSLiteShow"))
	{
#ifdef DUAL_STACK_LITE
		boaWrite(wp,"1");
#else
		boaWrite(wp,"0");
#endif
		return 0;
	}
	else if(!strcmp(name,"6rdShow"))
	{
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
		boaWrite(wp,"1");
#else
		boaWrite(wp,"0");
#endif
		return 0;
	}
#ifdef CONFIG_IP_NF_ALG_ONOFF
	else if(!strcmp(name,"GetAlgType"))
		{
		GetAlgTypes(wp);
		return 0;
		}
	else if(!strcmp(name,"AlgTypeStatus"))
		{
		CreatejsAlgTypeStatus( wp);
	 	return 0;
		}
#endif
#ifdef DNS_BIND_PVC_SUPPORT
	else if(!strcmp(name,"DnsBindPvc"))
		{
		unsigned char dnsBindPvcEnable=0;
		mib_get(MIB_DNS_BIND_PVC_ENABLE,(void*)&dnsBindPvcEnable);
		//printf("dns bind pvc = %d\n",dnsBindPvcEnable);
		boaWrite(wp,"<font size=2>IPv4 %s:<input type=\"checkbox\" name=\"enableDnsBind\" value=\"on\" %s onClick=\"DnsBindPvcClicked();\"></font>",
		multilang(LANG_WAN_INTERFACE_BINDING), (dnsBindPvcEnable) ? "checked" : "");
		return 0;
		}
#endif
#ifdef CONFIG_IPV6
#ifdef DNSV6_BIND_PVC_SUPPORT
	else if(!strcmp(name,"Dnsv6BindPvc"))
		{
		unsigned char dnsv6BindPvcEnable=0;
		mib_get(MIB_DNSV6_BIND_PVC_ENABLE,(void*)&dnsv6BindPvcEnable);
		//printf("dnsv6 bind pvc = %d\n",dnsv6BindPvcEnable);
		boaWrite(wp,"<font size=2>IPv6 %s:<input type=\"checkbox\" name=\"enableDnsv6Bind\" value=\"on\" %s onClick=\"Dnsv6BindPvcClicked();\"></font>",
		multilang(LANG_WAN_INTERFACE_BINDING), (dnsv6BindPvcEnable) ? "checked" : "");
		return 0;
		}
#endif
#endif
	else  if(!strcmp(name,"WanPvcRouter"))
		{
#ifdef DNS_BIND_PVC_SUPPORT
				MIB_CE_ATM_VC_T Entry;
				int entryNum;
				int mibcnt;
				char interfacename[MAX_NAME_LEN];
				entryNum = mib_chain_total(MIB_ATM_VC_TBL);
				unsigned char forSelect=0;
		            for(mibcnt=0;mibcnt<entryNum;mibcnt++)
		            {
		            if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)&Entry))
						{
		  					boaError(wp, 400, "Get chain record error!\n");
							return -1;
						}
			      if(Entry.cmode!=CHANNEL_MODE_BRIDGE)// CHANNEL_MODE_BRIDGE CHANNEL_MODE_IPOE CHANNEL_MODE_PPPOE CHANNEL_MODE_PPPOA	CHANNEL_MODE_RT1483	CHANNEL_MODE_RT1577
			      	{
			      	boaWrite(wp,"0");
				return 0;
		                  }

		            }
		           boaWrite(wp,"1");
#else
	 		 boaWrite(wp,"0");
#endif
			return 0;

		}

#ifdef DNS_BIND_PVC_SUPPORT
	else if(!strcmp(name,"dnsBindPvcInit"))
			{
				unsigned int dnspvc1,dnspvc2,dnspvc3;
				if(!mib_get(MIB_DNS_BIND_PVC1,(void*)&dnspvc1))
					{
					boaError(wp, 400, "Get MIB_DNS_BIND_PVC1 record error!\n");
							return -1;
					}
				if(!mib_get(MIB_DNS_BIND_PVC2,(void*)&dnspvc2))
					{
					boaError(wp, 400, "Get MIB_DNS_BIND_PVC2 record error!\n");
							return -1;
					}
				if(!mib_get(MIB_DNS_BIND_PVC3,(void*)&dnspvc3))
					{
					boaError(wp, 400, "Get MIB_DNS_BIND_PVC3 record error!\n");
							return -1;
					}

				    boaWrite(wp,"DnsBindSelectdInit('wanlist1',%d);\n",dnspvc1);
				    boaWrite(wp,"DnsBindSelectdInit('wanlist2',%d);\n",dnspvc2);
				    boaWrite(wp,"DnsBindSelectdInit('wanlist3',%d);\n",dnspvc3);
				boaWrite(wp,"DnsBindPvcClicked();");
				return 0;
			}
#endif

#ifdef CONFIG_IPV6
#ifdef DNSV6_BIND_PVC_SUPPORT
	else if(!strcmp(name,"dnsv6BindPvcInit"))
			{
				unsigned int dnspvc1,dnspvc2,dnspvc3;
				if(!mib_get(MIB_DNSV6_BIND_PVC1,(void*)&dnspvc1))
					{
					boaError(wp, 400, "Get MIB_DNSV6_BIND_PVC1 record error!\n");
							return -1;
					}
				if(!mib_get(MIB_DNSV6_BIND_PVC2,(void*)&dnspvc2))
					{
					boaError(wp, 400, "Get MIB_DNSV6_BIND_PVC2 record error!\n");
							return -1;
					}
				if(!mib_get(MIB_DNSV6_BIND_PVC3,(void*)&dnspvc3))
					{
					boaError(wp, 400, "Get MIB_DNSV6_BIND_PVC3 record error!\n");
							return -1;
					}

				    boaWrite(wp,"DnsBindSelectdInit('v6wanlist1',%d);\n",dnspvc1);
				    boaWrite(wp,"DnsBindSelectdInit('v6wanlist2',%d);\n",dnspvc2);
				    boaWrite(wp,"DnsBindSelectdInit('v6wanlist3',%d);\n",dnspvc3);
				boaWrite(wp,"Dnsv6BindPvcClicked();");
				return 0;
			}
#endif
#endif
	else if(!strcmp(name, "dgw")){
#ifdef DEFAULT_GATEWAY_V1
		boaWrite(wp, "\tif (droute == 1)\n");
		boaWrite(wp, "\t\tdocument.adsl.droute[1].checked = true;\n");
		boaWrite(wp, "\telse\n");
		boaWrite(wp, "\t\tdocument.adsl.droute[0].checked = true;\n");
#else
		GetDefaultGateway(eid, wp, argc, argv);
		boaWrite(wp, "\tautoDGWclicked();\n");
#endif
	}
/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
	else if(!strncmp(name, "title", 5))	{
#ifdef CONFIG_SFU
		boaWrite(wp,	"<img src=\"graphics/topbar_sfu.gif\" width=900 height=60 border=0>");
#elif CONFIG_VDSL
		boaWrite(wp,	"<img src=\"graphics/topbar_vdsl.gif\" width=900 height=60 border=0>");
#elif CONFIG_11N_SAGEM_WEB
		boaWrite(wp,	"<img src=\"graphics/sagemlogo1.gif\" width=1350 height=60 border=0>");
#else
		boaWrite(wp,	"<img src=\"graphics/topbar.gif\" width=900 height=60 border=0>");
#endif
	}
	else if(!strncmp(name, "logobelow", 9))	{
#ifdef CONFIG_11N_SAGEM_WEB
		boaWrite(wp,	"<img src=\"graphics/sagemlogo2.gif\" width=180 height=60 border=0>");
#endif
	}
#ifdef CONFIG_ETHWAN
	else if(!strncmp(name, "ethwanSelection", 15)){
		MIB_CE_ATM_VC_T Entry;

		memset((void *)&Entry, 0, sizeof(Entry));
		if (getWanEntrybyMedia(&Entry, MEDIA_ETH)>=0)
			boaWrite(wp, "document.ethwan.adslConnectionMode.value = \"%d\";\n", Entry.cmode);
	}
#endif
#ifdef CONFIG_INIT_SCRIPTS
	else if(!strncmp(name, "getStartScriptContent", 21))
	{
		FILE *sct_fp;
		char line[128];

		if(sct_fp = fopen("/var/config/start_script", "r"))
		{
			while(fgets(line, 127, sct_fp))
				boaWrite(wp, "%s<br>", line);
		}
		else
			boaWrite(wp, "Open script file failed!\n");
	}
	else if(!strncmp(name, "getEndScriptContent", 21))
	{
		FILE *sct_fp;
		char line[128];

		if(sct_fp = fopen("/var/config/end_script", "r"))
		{
			while(fgets(line, 127, sct_fp))
				boaWrite(wp, "%s<br>", line);
		}
		else
			boaWrite(wp, "Open script file failed!\n");
	}
#endif
#ifndef CONFIG_DEV_xDSL
	else if(!strcmp(name, "wan_mode_atm"))
	{
		boaWrite(wp, "style=\"display: none\"");
	}
#endif
#ifndef CONFIG_ETHWAN
	else if(!strcmp(name, "wan_mode_ethernet"))
	{
		boaWrite(wp, "style=\"display: none\"");
	}
#endif
#ifndef CONFIG_PTMWAN
	else if(!strcmp(name, "wan_mode_ptm"))
	{
		boaWrite(wp, "style=\"display: none\"");
	}
#endif
#ifndef CONFIG_PTM_BONDING
	else if(!strcmp(name, "wan_mode_bonding"))
	{
		boaWrite(wp, "style=\"display: none\"");
	}
#endif
#ifndef WLAN_WISP
	else if(!strcmp(name, "wan_mode_wireless"))
	{
		boaWrite(wp, "style=\"display: none\"");
	}
#endif
#if defined(CONFIG_RTK_RG_INIT) && defined (CONFIG_IPV6)
	else if (!strcmp(name, "lan_ipv6_mode_auto")) {
		if (!mib_get(MIB_LAN_IPV6_MODE1, (void *)&vChar))
			return -1;
		if (!vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "lan_ipv6_mode_manual")) {
		if (!mib_get(MIB_LAN_IPV6_MODE1, (void *)&vChar))
			return -1;
		if (vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if (!strcmp(name, "lan_ipverion_v4only")) {
		if (!mib_get(MIB_LAN_IP_VERSION1, (void *)&vChar))
			return -1;
		if (vChar==0)
			boaWrite(wp, "selected");
		return 0;
	}
	else if (!strcmp(name, "lan_ipverion_v4v6")) {
		if (!mib_get(MIB_LAN_IP_VERSION1, (void *)&vChar))
			return -1;
		if (vChar==1)
			boaWrite(wp, "selected");
		return 0;
	}
#endif
	else
		return -1;

	return 0;
}

#ifdef WLAN_MBSSID
int getVirtualIndex(int eid, request *wp, int argc, char **argv)
{
	int ret, vwlan_idx;
	char s_ifname[16];
	MIB_CE_MBSSIB_T Entry;
	int i=0;

	snprintf(s_ifname, 16, "%s", (char *)getWlanIfName());
	vwlan_idx = atoi(argv[--argc]);

	if (vwlan_idx > NUM_VWLAN_INTERFACE) {
		//fprintf(stderr, "###%s:%d wlan_idx=%d vwlan_idx=%d###\n", __FILE__, __LINE__, wlan_idx, vwlan_idx);
		boaWrite(wp, "0");
		return 0;
	}

	if (vwlan_idx > 0)
		snprintf(s_ifname, 16, "%s-vap%d", (char *)getWlanIfName(), vwlan_idx - 1);

	if (!mib_chain_get(MIB_MBSSIB_TBL, vwlan_idx, (void *)&Entry)) {
		printf("Error! Get MIB_MBSSIB_TBL(getVirtualIndex) error.\n");
		return 0;
	}

	if (!strcmp("band", argv[0]))
		boaWrite(wp, "%d", Entry.wlanBand);
	else if (!strcmp("wlanDisabled", argv[0]))
		boaWrite(wp, "%d", Entry.wlanDisabled);
	else if (!strcmp("wlanAccess", argv[0]))
		boaWrite(wp, "%d", Entry.userisolation);
	else if (!strcmp("rateAdaptiveEnabled", argv[0]))
		boaWrite(wp, "%d", Entry.rateAdaptiveEnabled);
	else if (!strcmp("fixTxRate", argv[0]))
		boaWrite(wp, "%u", Entry.fixedTxRate);
	else if (!strcmp("wmmEnabled", argv[0]))
		boaWrite(wp, "%u", Entry.wmmEnabled);

	return ret;
}
#endif

void write_wladvanced(int eid, request * wp, int argc, char **argv)        //add by yq_zhou 1.20
{
#ifdef WPS20
	unsigned char wpsUseVersion;
	mib_get(MIB_WSC_VERSION, (void *) &wpsUseVersion);
#endif

#ifdef WLAN_SUPPORT
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx==1)
#elif defined(WLAN0_QTN)
	if(wlan_idx==0)
#endif
		boaWrite(wp,"<tr style=\"display: none\">");
	else
#endif
	boaWrite(wp,"<tr>");
    boaWrite(wp, "<td width=\"30%%\"><font size=2><b>%s:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" value=\"long\" name=\"preamble\">%s&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"preamble\" value=\"short\">%s</td></tr>\n",
		multilang(LANG_PREAMBLE_TYPE), multilang(LANG_LONG_PREAMBLE),
		multilang(LANG_SHORT_PREAMBLE));

#ifdef WPS20
	if (wpsUseVersion) {
			boaWrite(wp,
			 "<tr><td width=\"30%%\"><font size=2><b>%s %s:</b></td>\n"
			 "<td width=\"70%%\"><font size=2>\n"
			 "<input type=\"radio\" name=\"hiddenSSID\" value=\"no\">%s&nbsp;&nbsp;\n"
			 "<input type=\"radio\" name=\"hiddenSSID\" value=\"yes\" onClick=\"alert('If you disable this, WPS will be disabled!')\">%s</td></tr>\n",
			multilang(LANG_BROADCAST), multilang(LANG_SSID), multilang(LANG_ENABLED), multilang(LANG_DISABLED));
	}
	else
#endif
	{
		boaWrite(wp,
	     "<tr><td width=\"30%%\"><font size=2><b>%s %s:</b></td>\n"
	     "<td width=\"70%%\"><font size=2>\n"
	     "<input type=\"radio\" name=\"hiddenSSID\" value=\"no\">%s&nbsp;&nbsp;\n"
	     "<input type=\"radio\" name=\"hiddenSSID\" value=\"yes\">%s</td></tr>\n",
		multilang(LANG_BROADCAST), multilang(LANG_SSID), multilang(LANG_ENABLED), multilang(LANG_DISABLED));
	}
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>%s:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=block value=1>%s&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=block value=0>%s</td></tr>\n",
	multilang(LANG_RELAY_BLOCKING), multilang(LANG_ENABLED), multilang(LANG_DISABLED));

#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx==1)
#elif defined(WLAN0_QTN)
	if(wlan_idx==0)
#endif
		boaWrite(wp,"<tr style=\"display: none\">");
	else
#endif
	boaWrite(wp,"<tr>");
	boaWrite(wp,"<td width=\"30%%\"><font size=2><b>%s:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"protection\" value=\"yes\">%s&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"protection\" value=\"no\">%s</td></tr>\n",
	multilang(LANG_PROTECTION), multilang(LANG_ENABLED), multilang(LANG_DISABLED));
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
	if(wlan_idx==1)
#elif defined(WLAN0_QTN)
	if(wlan_idx==0)
#endif
		boaWrite(wp,"<tr style=\"display: none\">");
	else
#endif
	boaWrite(wp,"<tr>");
  	boaWrite(wp,"<td width=\"30%%\"><font size=2><b>%s:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"aggregation\" value=\"enable\">%s&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"aggregation\" value=\"disable\">%s</td></tr>\n",
	multilang(LANG_AGGREGATION), multilang(LANG_ENABLED), multilang(LANG_DISABLED));
       boaWrite(wp,
     "<tr id=\"ShortGi\" style=\"display:\">\n"
     "<td width=\"30%%\"><font size=2><b>%s:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"shortGI0\" value=\"on\">%s&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"shortGI0\" value=\"off\">%s</td></tr>\n",
	multilang(LANG_SHORT_GI), multilang(LANG_ENABLED), multilang(LANG_DISABLED));
#endif /*WLAN_SUPPORT*/
}

int getNameServer(int eid, request * wp, int argc, char **argv) {

	FILE *fp;
	char buffer[128], tmpbuf[64];
	int count = 0;
	//fprintf(stderr, "getNameServer %x\n", gResolvFile);
	//boaWrite(wp, "[]", tmpbuf);
	//if ((gResolvFile == NULL) ||
	// for IPv4
	if ( (fp = fopen(RESOLV_BACKUP, "r")) == NULL ) {
		//printf("Unable to open resolver file\n");
		return -1;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {		
		if (sscanf(buffer, "%s", tmpbuf) != 1) {
			continue;
		}

		if (count == 0)
			boaWrite(wp, "%s", tmpbuf);
		else
			boaWrite(wp, ", %s", tmpbuf);
		count ++;
	}
	fclose(fp);

	return 0;
}

#ifndef RTF_UP
/* Keep this in sync with /usr/src/linux/include/linux/route.h */
#define RTF_UP          0x0001	/* route usable                 */
#define RTF_GATEWAY     0x0002	/* destination is a gateway     */
#define RTF_HOST        0x0004	/* host entry (net otherwise)   */
#define RTF_REINSTATE   0x0008	/* reinstate route after tmout  */
#define RTF_DYNAMIC     0x0010	/* created dyn. (by redirect)   */
#define RTF_MODIFIED    0x0020	/* modified dyn. (by redirect)  */
#define RTF_MTU         0x0040	/* specific MTU for this route  */
#ifndef RTF_MSS
#define RTF_MSS         RTF_MTU	/* Compatibility :-(            */
#endif
#define RTF_WINDOW      0x0080	/* per route window clamping    */
#define RTF_IRTT        0x0100	/* Initial round trip time      */
#define RTF_REJECT      0x0200	/* Reject route                 */
#endif

// Jenny, get default gateway information
int getDefaultGW(int eid, request * wp, int argc, char **argv)
{
	char buff[256];
	int flgs;
	struct in_addr gw, dest, mask, inAddr;
	char ifname[16], dgw[16];
	int found=0;
	FILE *fp;

	if (!(fp=fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		//if (sscanf(buff, "%*s%*lx%lx%X%", &g, &flgs) != 2) {
		if (sscanf(buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flgs, &mask) != 5) {
			printf("Unsuported kernel route format\n");
			fclose(fp);
			return -1;
		}

		//printf("ifname=%s, dest=%x, gw=%x, flgs=%x, mask=%x\n", ifname, dest.s_addr, gw.s_addr, flgs, mask.s_addr);
		if(flgs & RTF_UP  && strcmp(ifname, "lo")!=0) {
			// default gateway
			if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1) {
				if (inAddr.s_addr == 0x40404040) {
					boaWrite(wp, "");
					continue;
				}
			}
		
			if (found)
				boaWrite(wp, ", ");
			if (dest.s_addr == 0 && mask.s_addr == 0) {
				found = 1;
				if (gw.s_addr != 0) {
					strncpy(dgw,  inet_ntoa(gw), 16);
					boaWrite(wp, "%s", dgw);
				}
				else {
					boaWrite(wp, "%s", ifname);
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

#ifdef CONFIG_IPV6
int getDefaultGW_ipv6(int eid, request * wp, int argc, char **argv)
{
	char buff[256];
	struct in6_addr addr;
	unsigned char len;
	unsigned char devname[10];
	unsigned char value[48];
	FILE *fp;
	int i;

	if (!(fp=fopen("/proc/net/ipv6_route", "r"))) {
		printf("Error: cannot open /proc/net/ipv6_route - continuing...\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if(sscanf( buff,
			"%*32s%02hhx%*32s%*02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%*x%*x%*x%*x%s",
			&len,
			&addr.s6_addr[ 0], &addr.s6_addr[ 1], &addr.s6_addr[ 2], &addr.s6_addr[ 3],
			&addr.s6_addr[ 4], &addr.s6_addr[ 5], &addr.s6_addr[ 6], &addr.s6_addr[ 7],
			&addr.s6_addr[ 8], &addr.s6_addr[ 9], &addr.s6_addr[10], &addr.s6_addr[11],
			&addr.s6_addr[12], &addr.s6_addr[13], &addr.s6_addr[14], &addr.s6_addr[15], devname)) {

			//printf("len=%d, devname=%s\n", len, devname);
			//for ( i=0; i<16; i++)
			//	printf("%x ", addr.s6_addr[i]);
			//printf("\n");

			if( len == 0 && (strcmp(devname, "lo") !=0) ) {
				inet_ntop(PF_INET6, &addr, value, sizeof(value));
				boaWrite(wp, "%s", value);
				fclose(fp);
				return 0;
			}
		}
	}
	boaWrite(wp, "");
	fclose(fp);
	return 0;
}
#endif

int multilang_asp(int eid, request * wp, int argc, char **argv)
{
	int key;

   	if (boaArgs(argc, argv, "%d", &key) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}

	return boaWrite(wp, "%s", multilang(key));
}

int WANConditions(int eid, request * wp, int argc, char **argv)
{
	int wan_bitmap = WAN_MODE_MASK;
	int i, count = 0;

	for(i = 0; i < 5; i++)
	{
		if(1 & (wan_bitmap >> i))
			count++;
	}

	if(count > 1)
		return 0;
	else
		return boaWrite(wp, "style=\"display:none\"");
}
void ShowWanMode(int eid, request * wp, int argc, char **argv)
{
	char *name;
#ifdef WLAN_WISP
	char mode;
	char rptEnabled;
	int orig_idx, i;
	int enable_wisp=0;
#endif

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return;
	}

	if(!strcmp(name, "wlan"))
#ifdef WLAN_WISP
	{
		orig_idx = wlan_idx;
		for(i=0;i<NUM_WLAN_INTERFACE;i++){
			wlan_idx = i;
			mib_get(MIB_WLAN_MODE, &mode);
			mib_get(MIB_REPEATER_ENABLED1, &rptEnabled);

			if((mode == AP_MODE || mode == AP_WDS_MODE) && rptEnabled){
				enable_wisp=1;
				break;
			}
		}
		
		wlan_idx = orig_idx;

		if(enable_wisp)
			boaWrite(wp, "");
		else
			boaWrite(wp, "disabled");
	}
#else
	boaWrite(wp, "disabled");				
#endif
}


