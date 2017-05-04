/*
 *      Web server handler routines for get info and index (getinfo(), getindex())
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
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
#ifdef WLAN_SUPPORT
#include "wireless.h"
#endif
// Mason Yu. t123
#include "webform.h"

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
	int (*handler)(int , request* , int , char **, char *);
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
	{"wan-dhcps", INFO_MIB, MIB_ADSL_WAN_DHCPS},
	{"dmzHost", INFO_MIB, MIB_DMZ_IP},
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
	{"ntpServerHost1", INFO_MIB, MIB_NTP_SERVER_HOST1},
	{"ntpServerHost2", INFO_MIB, MIB_NTP_SERVER_HOST2},
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
#ifdef CONFIG_USER_IGMPPROXY
	{"igmp-proxy-itf", INFO_MIB, MIB_IGMP_PROXY_ITF},
#endif
//#ifdef CONFIG_USER_UPNPD
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	{"upnp-ext-itf", INFO_MIB, MIB_UPNP_EXT_ITF},
#endif

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH
	{"mldproxy-ext-itf", INFO_MIB, MIB_MLD_PROXY_EXT_ITF}, 		// Mason Yu. MLD Proxy
#endif
#endif

#ifdef AUTO_PROVISIONING
	{"http-ip", INFO_MIB, MIB_HTTP_SERVER_IP},
#endif
#ifdef IP_PASSTHROUGH
	{"ippt-itf", INFO_MIB, MIB_IPPT_ITF},
	{"ippt-lease", INFO_MIB, MIB_IPPT_LEASE},
	{"ippt-lanacc", INFO_MIB, MIB_IPPT_LANACC},
#endif
#ifdef WLAN_SUPPORT
	{"ssid", INFO_SYS, SYS_WLAN_SSID},
	{"channel", INFO_MIB, MIB_WLAN_CHAN_NUM},
	{"fragThreshold", INFO_MIB, MIB_WLAN_FRAG_THRESHOLD},
	{"rtsThreshold", INFO_MIB, MIB_WLAN_RTS_THRESHOLD},
	{"beaconInterval", INFO_MIB, MIB_WLAN_BEACON_INTERVAL},
	{"wlanDisabled",INFO_SYS,SYS_WLAN_DISABLED},
	{"hidden_ssid",INFO_SYS,SYS_WLAN_HIDDEN_SSID},
	{"pskValue", INFO_SYS, SYS_WLAN_PSKVAL},
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

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	{"wscDisable",INFO_SYS,SYS_WSC_DISABLE},
	//{"wscConfig",INFO_MIB,MIB_WSC_CONFIGURED},
	{"wps_auth",INFO_SYS,SYS_WSC_AUTH},
	{"wps_enc",INFO_SYS,SYS_WSC_ENC},
	{"wscLoocalPin", INFO_MIB, MIB_WSC_PIN},

#endif

#ifdef WLAN_WDS
	{"wlanWdsEnabled",INFO_MIB,MIB_WLAN_WDS_ENABLED},
#endif
#ifdef WLAN_UNIVERSAL_REPEATER
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
#endif
#ifdef CONFIG_MIDDLEWARE
	{"midwareServerAddr", INFO_MIB, CWMP_MIDWARE_SERVER_ADDR},
	{"midwareServerPort", INFO_MIB, CWMP_MIDWARE_SERVER_PORT},
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
	{"normal-user", INFO_MIB, MIB_USER_NAME},
#ifdef DEFAULT_GATEWAY_V2
	{"wan-default-gateway", INFO_MIB, MIB_ADSL_WAN_DGW_IP},
	{"itf-default-gateway", INFO_MIB, MIB_ADSL_WAN_DGW_ITF},
#endif
//ql 20090119
#ifdef IMAGENIO_IPTV_SUPPORT
	{"stb-dns1", INFO_MIB, MIB_IMAGENIO_DNS1},
	{"stb-dns2", INFO_MIB, MIB_IMAGENIO_DNS2},
	{"opch-addr", INFO_MIB, MIB_OPCH_ADDRESS},
	{"opch-port", INFO_MIB, MIB_OPCH_PORT},
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
	{"dhcpv6s_min_address", INFO_MIB, MIB_DHCPV6S_MIN_ADDRESS},
	{"dhcpv6s_max_address", INFO_MIB, MIB_DHCPV6S_MAX_ADDRESS},
	{"dhcpv6s_prefix", INFO_MIB, MIB_IPV6_LAN_PREFIX}, //from IPV6_LAN_PREFIX
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
#ifdef CONFIG_RTK_RG_INIT
	{"mac_based_tag_decision", INFO_MIB, MIB_MAC_BASED_TAG_DECISION},
	{"lan_vlan_id1", INFO_MIB, MIB_LAN_VLAN_ID1},
	{"lan_vlan_id2", INFO_MIB, MIB_LAN_VLAN_ID2},
#endif
	{"loid", INFO_MIB, MIB_LOID},
#ifdef _PRMT_X_CT_COM_USERINFO_
	{"cwmp_UserInfo_Status", INFO_MIB, CWMP_USERINFO_STATUS},
#endif
	{"rtk_manufacturer", INFO_MIB, RTK_DEVID_MANUFACTURER},
	{"rtk_oui", INFO_MIB, RTK_DEVID_OUI},
	{"rtk_productclass", INFO_MIB, RTK_DEVID_PRODUCTCLASS},
	{"rtk_serialno", INFO_MIB, MIB_HW_SERIAL_NUMBER},
	{"cwmp_provisioningcode", INFO_MIB, CWMP_PROVISIONINGCODE},
	{"rtk_specver", INFO_MIB, RTK_DEVINFO_SPECVER},
	{"rtk_swver", INFO_MIB, RTK_DEVINFO_SWVER},
	{"rtk_hwver", INFO_MIB, RTK_DEVINFO_HWVER},
#if defined(CONFIG_GPON_FEATURE)
	{"gpon_sn",INFO_MIB,MIB_GPON_SN},
#endif
	{"elan_mac_addr", INFO_MIB, MIB_ELAN_MAC_ADDR},
#ifdef CONFIG_IPV6
	{"prefix-mode", INFO_MIB, MIB_PREFIXINFO_PREFIX_MODE}, 		
	{"prefix-delegation-wan-conn", INFO_MIB, MIB_PREFIXINFO_DELEGATED_WANCONN}, 		
	{"dns-mode", INFO_MIB, MIB_LAN_DNSV6_MODE}, 		
	{"dns-wan-conn", INFO_MIB, MIB_DNSINFO_WANCONN}, 		
#endif
	{"loid_reg_status", INFO_MIB, CWMP_USERINFO_STATUS},
	{"loid_reg_result", INFO_MIB, CWMP_USERINFO_RESULT},
	{"cwmp_conf", INFO_MIB, CWMP_CONFIGURABLE},
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


static int fnget_wpsKey(int eid, request* wp, int argc, char **argv, char *buffer) {
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
				mib_id = MIB_WLAN_WEP64_KEY1;
			else if (key == 1)
				mib_id = MIB_WLAN_WEP64_KEY2;
			else if (key == 2)
				mib_id = MIB_WLAN_WEP64_KEY3;
			else
				mib_id = MIB_WLAN_WEP64_KEY4;
			strcpy(tmp, Entry.wep64Key1);
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
				mib_id = MIB_WLAN_WEP128_KEY1;
			else if (key == 1)
				mib_id = MIB_WLAN_WEP128_KEY2;
			else if (key == 2)
				mib_id = MIB_WLAN_WEP128_KEY3;
			else
				mib_id = MIB_WLAN_WEP128_KEY4;
			strcpy(tmp, Entry.wep128Key1);
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

web_custome_cmd get_info_custom_list[] = {
	#ifdef WLAN_SUPPORT
	#ifdef CONFIG_WIFI_SIMPLE_CONFIG//WPS
	{ "wps_key", fnget_wpsKey },
	#endif
	#endif
	{ NULL, 0 }
};

int getInfo(int eid, request* wp, int argc, char **argv)
{
	char	*name;
	unsigned char buffer[256 + 1];
	int idx, ret;
	FILE *fp;

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

#ifdef E8B_NEW_DIAGNOSE
	/* willin.hou 2009-03-28 */
	if (!strncmp(name, "tr069Inform", 11)) {
		char cause[64];
		int status, ret;

		fp = fopen(INFORM_STATUS_FILE, "r");
		if (fp == NULL) {
			return boaWrite(wp, "无");
		}
		cause[0] = '\0';
		ret = fscanf(fp, "%d:%64[^\n]", &status, cause);
		fclose(fp);
		if (ret == EOF) {
			return boaWrite(wp, "无");
		}
		switch (status) {
		case NO_INFORM:
			return boaWrite(wp, "未上报（%s）", cause);
		case NO_RESPONSE:
			return boaWrite(wp, "上报无回应");
		case INFORM_BREAK:
			return boaWrite(wp, "上报过程中断");
		case INFORM_SUCCESS:
			return boaWrite(wp, "上报成功");
		case INFORM_AUTH_FAIL:
			return boaWrite(wp, "上报验证失败");
		case INFORMING:
			return boaWrite(wp, "上报中...");
		default:
			return boaWrite(wp, "无");
		}
	}

	if (!strncmp(name, "tr069Connect", 12)) {
		int status, ret;

		fp = fopen(CONNREQ_STATUS_FILE, "r");
		if (fp == NULL) {
			return boaWrite(wp, "无");
		}
		ret = fscanf(fp, "%d", &status);
		fclose(fp);
		if (ret == EOF) {
			return boaWrite(wp, "无");
		}
		switch (status) {
		case NO_REQUEST:
			return boaWrite(wp, "未收到远程连接请求");
		case REQUEST_BREAK:
			return boaWrite(wp, "ITMS+发起的远程连接过程中断");
		case REQUEST_SUCCESS:
			return boaWrite(wp, "ITMS+发起的远程连接过程成功");
		default:
			return boaWrite(wp, "无");
		}
	}

	if (!strncmp(name, "tr069Config", 12)) {
		int i;
		char action[16];
		char settingFileName[32];
		char tmpbuf[128];
		unsigned int regResult;
		pvc_status_entry entry;

		fp = fopen(REMOTE_SETSAVE_FILE, "r");
		if (fp) {
			fgets(buffer, sizeof(buffer), fp);
			fclose(fp);
			return boaWrite(wp, "业务配置完成并导致重启: %s", buffer);
		}

		mib_get(CWMP_USERINFO_RESULT, &regResult);

		buffer[0] = '\0';
		switch (regResult) {
		case NO_SET:
			return boaWrite(wp, "ITMS未下发远程业务配置状态");
		case NOW_SETTING:
			return boaWrite(wp, "正在接受ITMS的远程业务配置");
		case SET_SUCCESS:
			fp = fopen(NEW_SETTING, "r");
			if (fp) {
				fgets(settingFileName, sizeof(settingFileName), fp);
				fclose(fp);
			}
			if (!strcmp(settingFileName, PVC_FILE)) {
				for (i = 1; ; i++) {
					sprintf(tmpbuf, "%s%d", PVC_FILE, i);
					fp = fopen(tmpbuf, "rb");
					if (!fp)
						break;

					fread(&entry, sizeof(entry), 1, fp);
					fclose(fp);
					if (entry.action[0] != 0 && entry.servertype[0] != 0 && entry.wantype[0] != 0) {
						if (!strcmp(entry.action, "del") || 
						(!strcmp(entry.action, "add") && (entry.vpi != 0 || entry.vci != 0))) {
							if (!strcmp(entry.action, "del"))
								strcpy(action, "删除");
							else
								strcpy(action, "创建");
							sprintf(tmpbuf, "已成功%s%s属性%sWAN连接(PVC:%d/%d);", 
									action, entry.servertype, entry.wantype,
									entry.vpi, entry.vci);
							strcat(buffer, tmpbuf);
						}
					}
				}
			} else if (!strcmp(settingFileName, SSID_FILE)) {
				for (i = 1; i <= 5; i ++) {
					sprintf(tmpbuf, "%s%d", SSID_FILE, i);
					fp = fopen(tmpbuf, "r");
					if (fp) {
						fgets(action, sizeof(action), fp);
						fclose(fp);
						sprintf(tmpbuf, "已成功%sSSID-%d;", action, i);
						strcat(buffer, tmpbuf);
					}
				}
			} else if (!strcmp(settingFileName, USERLIMIT_FILE)) {
				fp = fopen(USERLIMIT_FILE, "r");
				if (fp) {
					fscanf(fp, "%d", &i);
					fclose(fp);
					sprintf(buffer, "已成功修改用户数限制为%d", i);
				}
			} else if (!strcmp(settingFileName, QOS_FILE)) {
				fp = fopen(QOS_FILE, "r");
				if (fp) {
					fgets(tmpbuf, sizeof(tmpbuf), fp);
					fclose(fp);
					sprintf(buffer, "已成功设置QoS模板%s", tmpbuf);
				}
			}
			if (strlen(buffer) == 0)
				sprintf(buffer, "没有成功的业务配置下发");
			return boaWrite(wp, "业务配置成功: %s", buffer);
		case SET_FAULT:
			return boaWrite(wp, "业务配置失败");
		default:
			return boaWrite(wp, "无");
		}
	}
#endif

#ifdef FIELD_TRY_SAFE_MODE
#ifdef CONFIG_DEV_xDSL
	if (!strncmp(name, "safemodenote", 12)) {
		SafeModeData vSmd;
		memset((void *)&vSmd, 0, sizeof(vSmd));
		adsl_drv_get(RLCM_GET_SAFEMODE_CTRL, (void *)&vSmd, SAFEMODE_DATA_SIZE);
		return boaWrite(wp, "%s", vSmd.SafeModeNote);
	}
#endif
#endif

	if (!strncmp(name, "wan_limit", 9))
	{
		int enable = 0, limit = 0;

		mib_get(CWMP_CT_MWBAND_MODE, &enable);
		mib_get(CWMP_CT_MWBAND_NUMBER, &limit);
		if(enable)
			return boaWrite(wp, "%d", limit);
		else
			return boaWrite(wp, "0");
	}

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
			else
			ret = boaWrite(wp, "%s", buffer);
			//fprintf(stderr, "%s = %s\n", name, buffer);
			//printf("%s = %s\n", name, buffer);
			break;
		}
	}

	if (!strncmp(name, "manufacture", 11)) {
		return boaWrite(wp, "%s", DEF_MANUFACTURER_STR);
	}

	if (!strncmp(name, "devModel", 8)) {
		return boaWrite(wp, "%s", DEVICE_MODEL_STR);
	}

	if (!strncmp(name, "devId", 5)) {
#ifdef _CWMP_MIB_
		unsigned char *bufptr;
		/*Jim 20081007 START */
#ifdef E8B_GET_OUI
		getOUIfromMAC(buffer);
#else
		strcpy(buffer, DEF_MANUFACTUREROUI_STR);
#endif
		bufptr = buffer + strlen(buffer);
		/*Jim 20081007 END */
		*bufptr = '-';
		bufptr++;
		mib_get(MIB_HW_SERIAL_NUMBER, (void *)bufptr);
		return boaWrite(wp, "%s", buffer);
#else
		return boaWrite(wp, "%s", "devId");
#endif
	}

	if (!strncmp(name, "hdVer", 5)) {
		return boaWrite(wp, "%s", HARDWARE_VERSION_STR);
	}

	if (!strncmp(name, "stVer", 5))
		return boaWrite(wp, "%s", SOFTWARE_VERSION_STR);

#ifdef CONFIG_DEV_xDSL
	if ( !strncmp(name, "linkState", 9) )
	{
		getAdslInfo(ADSL_GET_STATE, buffer, sizeof(buffer));
		if(!strncmp(buffer, "SHOWTIME", 8))
		{
			strncpy(buffer, "Up", 2);
			*(buffer+2)=0;
		}
		else
		{
			strncpy(buffer, "Down", 4);
			*(buffer+4)=0;
		}
		return boaWrite(wp, "%s", buffer);
	}

	//add end by liuxiao 2008-01-25
	//add by liuxiao 2008-01-28
	if ( !strncmp(name, "vsnrMargin", 11) ) {
		char tmp_buffer[64];
		int ret_us = 0;
		int ret_ds = 0;

		memset (tmp_buffer, 0, sizeof(tmp_buffer));
		buffer[0] = '\0';
		ret_us = getAdslInfo(ADSL_GET_SNR_US, buffer, sizeof(buffer));

		buffer[ret_us] = '/';
		ret_ds = getAdslInfo(ADSL_GET_SNR_DS, tmp_buffer, sizeof(tmp_buffer));
		if (ret_ds > 0)
			strncat(buffer, tmp_buffer, ret_ds);

		return boaWrite(wp, "上行/下行: %s", buffer);
	}
	if ( !strncmp(name, "vpmsPramD", 11) ) {
		char tmp_buffer[64];
		int ret_us = 0;
		int ret_ds = 0;

		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		buffer[0] = '\0';
		ret_us = getAdslInfo(ADSL_GET_D_US, buffer, sizeof(buffer));

		buffer[ret_us] = '/';
		ret_ds = getAdslInfo(ADSL_GET_D_DS, tmp_buffer, sizeof(tmp_buffer));
		if (ret_ds > 0)
			strncat(buffer, tmp_buffer, ret_ds);

		if(0 != ret_us || 0 != ret_ds)
			return boaWrite(wp, "上行/下行: %s", buffer);
	}

	if ( !strcmp (name, "lineProto") ) {
		short mode;
		char buf[100]={0};

		mib_get( MIB_ADSL_MODE, (void *)&mode );

		if ( mode & ADSL_MODE_GLITE )
			strcat(buf, "G.Lite ");

		if ( mode & ADSL_MODE_T1413 )
			strcat(buf, "T1.413 ");

		if ( mode & ADSL_MODE_GDMT )
			strcat(buf, "G.Dmt ");

		if ( mode & ADSL_MODE_ADSL2 )
			strcat(buf, "ADSL2 ");

		if ( mode & ADSL_MODE_ADSL2P )
			strcat(buf, "ADSL2+ ");

		boaWrite(wp,"%s",buf);
		return 0;
	}

	if ( !strncmp(name, "attenuation", 11) ) {
		char tmp_buffer[64];
		int ret_us = 0;
		int ret_ds = 0;

		memset (tmp_buffer, 0, sizeof(tmp_buffer));
		buffer[0] = '\0';
		ret_us = getAdslInfo(ADSL_GET_LPATT_US, buffer, sizeof(buffer));

		buffer[ret_us] = '/';
		ret_ds = getAdslInfo(ADSL_GET_LPATT_DS, tmp_buffer, sizeof(tmp_buffer));
		if (ret_ds > 0)
			strncat(buffer, tmp_buffer, ret_ds);

		return boaWrite(wp, "上行/下行: %s", buffer);
	}
	if ( !strncmp(name, "outputPower", 11) ) {
		char tmp_buffer[64];
		int ret_us = 0;
		int ret_ds = 0;

		memset (tmp_buffer, 0, sizeof(tmp_buffer));
		buffer[0] = '\0';
		ret_us = getAdslInfo(ADSL_GET_POWER_US, buffer, sizeof(buffer));

		buffer[ret_us] = '/';
		ret_ds = getAdslInfo(ADSL_GET_POWER_DS, tmp_buffer, sizeof(tmp_buffer));
		if (ret_ds > 0)
			strncat(buffer, tmp_buffer, ret_ds);

		return boaWrite(wp, "上行/下行: %s", buffer);
	}
	//add end by liuxiao 2008-01-28
	//add by liuxiao 2008-01-26
	if ( !strncmp(name, "pmsLatency", 10) ) {
		getAdslInfo(ADSL_GET_LATENCY, buffer, sizeof(buffer));
		return boaWrite(wp, "%s", buffer);
	}
#endif // of CONFIG_DEV_xDSL
	//add end by liuxiao 2008-01-26
	//add by liuxiao 2008-01-28 for wlan status
#ifdef WLAN_SUPPORT
	if ( !strncmp(name, "wlanState", 9) )
	{
		MIB_CE_MBSSIB_T Entry;
		wlan_getEntry(&Entry, 0);

		if (!Entry.wlanDisabled)
			return boaWrite (wp, INFO_ENABLED);
		else
			return boaWrite (wp, INFO_DISABLED);
	}

	//cathy, for e8b wlan status
	if( !strncmp(name, "wlDefChannel", 12) ) {
		struct iwreq wrq;
		int ret;
		#define RTL8185_IOCTL_GET_MIB	0x89f2
		idx= socket(AF_INET, SOCK_DGRAM, 0);
		strcpy(wrq.ifr_name, "wlan0");
		strcpy(buffer,"channel");
		wrq.u.data.pointer = (caddr_t)&buffer;
		wrq.u.data.length = 10;
		ret = ioctl(idx, RTL8185_IOCTL_GET_MIB, &wrq);
		close( idx );
		if ( ret != -1) {
			return boaWrite (wp, "%d", buffer[wrq.u.data.length-1]);
		}
		else
			return boaWrite (wp, "N/A");
	}

	if( !strncmp(name, "wlanMode", 8) ) {
		unsigned char vChar;
		const char *wlan_band[] ={0,"802.11b","802.11g","802.11 b+g" ,0
//#ifdef CONFIG_USB_RTL8192SU_SOFTAP
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
	     , 0, 0, 0,	"802.11 n", 0,	"802.11 g+n",	"802.11 b+g+n",0
#endif
			};
		MIB_CE_MBSSIB_T Entry;
		wlan_getEntry(&Entry, 0);
		return boaWrite (wp, "%s", wlan_band[Entry.wlanBand]);
	}
	if( !strncmp(name, "wlTxPower", 9) ) {
		unsigned char vChar;
		mib_get( MIB_TX_POWER, (void *)&vChar);
		if(vChar == 0)
			return boaWrite (wp, "100%%");
		else if(vChar == 1)
			return boaWrite (wp, "80%%");
		else if(vChar == 2)
			return boaWrite (wp, "50%%");
		else if(vChar == 3)
			return boaWrite (wp, "25%%");
		else
			return boaWrite (wp, "10%%");
	}
	if( !strncmp(name, "wlanBssid", 9) ) {
		unsigned char strbf[20];
		mib_get(MIB_ELAN_MAC_ADDR, (void *)buffer);
		snprintf(strbf, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
		return boaWrite(wp, "%s", (void *)strbf);
	}
	if( !strncmp(name, "wlanSsidAttr", 12) ) {
		MIB_CE_MBSSIB_T Entry;
		wlan_getEntry(&Entry, 0);
		return boaWrite (wp, (Entry.hidessid==0)?"Visual":"Hidden");
	}
	if( !strncmp(name, "ssidName", 8) ) {
		MIB_CE_MBSSIB_T Entry;
		wlan_getEntry(&Entry, 0);
		return boaWrite(wp, "%s", (void *)Entry.ssid);
	}
	if( !strncmp(name, "encryptState", 12) ) {
		MIB_CE_MBSSIB_T Entry;
		wlan_getEntry(&Entry, 0);
		return boaWrite (wp, (Entry.encrypt==0)?INFO_DISABLED:INFO_ENABLED);
	}
#endif
	//add end by liuxiao 2008-01-28 for wlan status
	if (!strcmp(name, "usbstate")) {
		int fd;
		#ifndef CONFIG_DEFAULTS_KERNEL_3_18
		char *buf = "/proc/bus/usb/devices";
		#else
		char *buf = "/sys/kernel/debug/usb/devices";
		#endif

		if (access(buf, R_OK) < 0) {
			#ifndef CONFIG_DEFAULTS_KERNEL_3_18
			va_cmd("/bin/mount", 4, 1, "-t", "usbfs", "none", "/proc/bus/usb");
			#else
			va_cmd("/bin/mount", 4, 1, "-t", "debugfs", "none", "/sys/kernel/debug/");
			#endif
		}
		if ((fd = open(buf, O_RDONLY)) == -1) {
			fprintf(stderr, "cannot open %s, %s (%d)\n", buf, strerror(errno), errno);
			return -1;
		}
		devtree_parsedevfile(fd);
		close(fd);
		devtree_processchanges();
		devtree_dump_for_web(wp);

		return 0;
	}
	/* End Magician: Copy from Realsil E8B */

	// Magician: Get primary DNS of default gateway.
	if( !strncmp(name, "wan-dns-1", 10) )
	{
		FILE *fp;
		char dns[64];
		int ret = -1;

		if (!(fp=fopen(RESOLV_BACKUP, "r"))) {
			fclose(fp);
			printf("Error: cannot open %s !!\n", RESOLV_BACKUP);
			return ret;
		}

		while( fgets(dns, sizeof(dns), fp) != NULL )
		{
			if ( (strchr(dns, '.') != NULL))
			{
				boaWrite(wp, "%s", dns);
				ret = 0;
				break;
			}
		}
		fclose(fp);
		return ret;
	}

	// Magician: Get 2nd DNS of default gateway.
	if( !strncmp(name, "wan-dns-2", 10) )
	{
		FILE *fp;
		char dns[64];
		int order=0;
		int ret = -1;

		if (!(fp=fopen(RESOLV_BACKUP, "r"))) {
			fclose(fp);
			printf("Error: cannot open %s !!\n", RESOLV_BACKUP);
			return ret;
		}

		while( fgets(dns, sizeof(dns), fp) != NULL )
		{
			if ( (strchr(dns, '.') != NULL))
			{
				order++;
				if(order == 2)
				{
					boaWrite(wp, "%s", dns);
					ret = 0;
					break;
				}
			}
		}
		fclose(fp);
		return ret;
	}

	// Mason Yu: Get 1st DNSv6 server
	if( !strncmp(name, "wan-dns6-1", 10) )
	{
		FILE *fp;
		char dns[64];
		int ret = -1;

		if (!(fp=fopen(RESOLV_BACKUP, "r"))) {
			fclose(fp);
			printf("Error: cannot open %s !!\n", RESOLV_BACKUP);
			return ret;
		}

		while( fgets(dns, sizeof(dns), fp) != NULL )
		{
			if ( (strchr(dns, ':') != NULL))
			{
				boaWrite(wp, "%s", dns);
				ret = 0;
				break;
			}
		}
		fclose(fp);
		return ret;
	}

	// Mason Yu: Get 2nd DNSv6 server
	if( !strncmp(name, "wan-dns6-2", 10) )
	{
		FILE *fp;
		char dns[64];
		int order=0;
		int ret = -1;

		if (!(fp=fopen(RESOLV_BACKUP, "r"))) {
			fclose(fp);
			printf("Error: cannot open %s !!\n", RESOLV_BACKUP);
			return ret;
		}

		while( fgets(dns, sizeof(dns), fp) != NULL )
		{
			if ( (strchr(dns, ':') != NULL))
			{
				order++;
				if(order == 2)
				{
					boaWrite(wp, "%s", dns);
					ret = 0;
					break;
				}
			}
		}
		fclose(fp);
		return ret;
	}

NEXTSTEP:
	return ret;
}

int addMenuJavaScript( request* wp,int nums,int maxchildrensize)
{
#ifdef WEB_MENU_USE_NEW
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
	boaWrite(wp,"\tif (num == 0) {\n\t\twhichEl1 = eval(el + \"Child\";\n"
		"\t\tfor(i=0;i<%d;i++)\n"),nums);
	boaWrite(wp,"\t\t{\n\t\t\twhichEl = eval(scores[i] + \"Child\";\n"
		"\t\t\tif(whichEl!=whichEl1)\n "
		"\t\t\t{\n\t\t\t\twhichEl.style.display = \"none\";\n"
		"\t\t\t\tdocument.getElementById(btns[i]).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t\t}\n\t\t}\n"));
	boaWrite(wp,"\t\twhichEl1 = eval(el + \"Child\";\n"
		"\t\tif (whichEl1.style.display == \"none\")\n "
		"\t\t{\n"
		"\t\t\twhichEl1.style.display = \"\";\n"
		"\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_open.gif\";\n"
		"\t\t}\n\t\telse {\n\t\t\twhichEl1.style.display =\"none\";\n"
		"\t\t\tdocument.getElementById(imgs).src =\"menu-images/menu_folder_closed.gif\";\n"
		"\t\t}\n\t}\n\telse {\n"));
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
#else
	boaWrite(wp,"<script type=\"text/javascript\" src=\"/admin/mtmcode.js\">\n"
	"</script>\n"
	"\n"
	"<script type=\"text/javascript\">\n"
	"    // Morten's JavaScript Tree Menu\n"
	"    // version 2.3.2-macfriendly, dated 2002-06-10\n"
	"    // http://www.treemenu.com/\n"
	"\n"
	"    // Copyright (c) 2001-2002, Morten Wang & contributors\n"
	"    // All rights reserved.\n"
	"\n"
	"    // This software is released under the BSD License which should accompany\n");
	boaWrite(wp,"    // it in the file \"COPYING\".  If you do not have this file you can access\n"
	"    // the license through the WWW at http://www.treemenu.com/license.txt\n"
	"\n"
	"    // Nearly all user-configurable options are set to their default values.\n"
	"    // Have a look at the section \"Setting options\" in the installation guide\n"
	"    // for description of each option and their possible values.\n");
	boaWrite(wp,"\n"
	"MTMDefaultTarget = \"view\";\n"
	"\n"
	"/******************************************************************************\n"
	" * User-configurable list of icons.                                            *\n"
	" ******************************************************************************/\n");
	boaWrite(wp,"\n"
	"var MTMIconList = null;\n"
	"MTMIconList = new IconList();\n"
	"MTMIconList.addIcon(new MTMIcon(\"menu_link_external.gif\", \"http://\", \"pre\"));\n"
	"MTMIconList.addIcon(new MTMIcon(\"menu_link_pdf.gif\", \".pdf\", \"post\"));\n");
	boaWrite(wp,"\n"
	"/******************************************************************************\n"
	" * User-configurable menu.                                                     *\n"
	" ******************************************************************************/\n");
	boaWrite(wp,"\n"
	"var menu = null;\n"
	"\n"
	"menu = new MTMenu();\n");
#endif
}

// Kaohj
int checkWrite(int eid, request* wp, int argc, char **argv)
{
	char *name;
	unsigned char vChar;
	unsigned short vUShort;
	unsigned int vUInt;

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
	if ( !strcmp(name, "rip-ver") ) {
		if ( !mib_get( MIB_RIP_VERSION, (void *)&vChar) )
			return -1;
		if (0==vChar) {
			boaWrite(wp, "<option selected value=0>v1</option>\n");
			boaWrite(wp, "\t<option value=1>v2</option>");
		} else {
			boaWrite(wp, "<option value=0>v1</option>\n");
			boaWrite(wp, "\t<option selected value=1>v2</option>\n");
		}
		return 0;
	}
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	if (!strcmp(name, "ripEn"))
	{
#ifdef CONFIG_USER_ROUTED_ROUTED
		if (!mib_get(MIB_RIP_ENABLE, (void *)&vChar))
			return -1;
#else
		vChar = 0;
#endif
		if (1 == vChar)
			boaWrite(wp, "1");
		else
			boaWrite(wp, "0");

		return 0;
	}
	if (!strcmp(name, "ospfEn"))
	{
		if (!mib_get(MIB_OSPF_ENABLE, (void *)&vChar))
			return -1;
		if (1 == vChar)
			boaWrite(wp, "1");
		else
			boaWrite(wp, "0");

		return 0;
	}
#endif
if (!strcmp (name, "SoftwareVersion"))
    {


      return 0;
    }

#ifdef CONFIG_DEV_xDSL
  if (!strcmp (name, "DspVersion"))
    {
      char buffer[256];
      getAdslDrvInfo ("version", buffer, 64);
	 boaWrite (wp,  "<tr bgcolor=\"#DDDDDD\">\n<td width=40%%><font size=2><b>DSP Version</b></td>"
                  "<td width=60%%><font size=2>%s</td>\n </tr>",
                 buffer);
     return 0;
    }
#endif

  if (!strcmp (name, "DnsServer"))
    {
   boaWrite (wp,
                 "<tr bgcolor=\"#EEEEEE\">\n<td width=20%%><font size=2><b>DNS Servers</b></td>");
  boaWrite (wp, " <td width=80%%><font size=2>");
      getNameServer (0, wp, 1, 0);
      boaWrite (wp, "</td>\n</tr>");
      return 0;
    }

  if (!strcmp (name, "DefaultGw"))
    {
    boaWrite (wp,
                 "<tr bgcolor=\"#DDDDDD\">\n <td width=20%% ><font size=2><b>Default Gateway</b></td>");
	  boaWrite (wp, "<td width=80%% colspan=\"6\"><font size=2>");
      getDefaultGW (0, wp, 1, 0);
      boaWrite (wp, "</td> </tr>");

      return 0;
    }

#ifdef CONFIG_DEV_xDSL
  if (!strcmp (name, "getDslSnr"))
    {
boaWrite (wp,
                 "<tr bgcolor=\"#DDDDDD\">\n <td width=40%%><font size=2><b>Upstream SNR</b></td>");
      boaWrite (wp, "<td width=60%%><font size=2>");
      char buf[256] = { 0 };
      getAdslInfo (ADSL_GET_SNR_US, buf, sizeof (buf) / sizeof (char));
      boaWrite (wp, "%sdB", buf);
      boaWrite (wp, "</td>\n </tr>");
  boaWrite (wp,
                 "<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>Downstream SNR</b></td>");
      boaWrite (wp, "<td width=60%%><font size=2>");
      memset (buf, 0, sizeof (buf) / sizeof (char));
      getAdslInfo (ADSL_GET_SNR_DS, buf, sizeof (buf) / sizeof (char));
      boaWrite (wp, "%sdB", buf);
      boaWrite (wp, "</td>\n </tr>");
      return 0;
    }

  if (!strcmp (name, "hskCount"))
    {
 boaWrite (wp,
                 "<tr bgcolor=\"#DDDDDD\">\n <td width=40%%><font size=2><b>reconnection Counts</b></td>");
      boaWrite (wp, "<td width=60%%><font size=2>");
      int intVal;

      if (adsl_drv_get(RLCM_GET_REHS_COUNT, &intVal, sizeof(int)))
        {
          boaWrite (wp, "%d", (intVal==0)?0:intVal-1);
        }

      boaWrite (wp, "</td>\n </tr>");
      return 0;

    }

  if (!strcmp (name, "dslMode"))
    {

    boaWrite (wp,
                 "<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>mode</b></td>");
      boaWrite (wp, "<td width=60%%><font size=2>");
      short mode;
      char buf[100]={0};
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_GLITE)
        strcat(buf,"G.Lite ");

      if (mode & ADSL_MODE_T1413)
        strcat(buf,"T1.413 ");

      if (mode & ADSL_MODE_GDMT)
        strcat(buf,"G.Dmt ");

      if (mode & ADSL_MODE_ADSL2)
        strcat(buf,"ADSL2 ");

      if (mode & ADSL_MODE_ADSL2P)
        strcat(buf,"ADSL2+ ");

//according ZTE's advise... show all set mode... jim
	if (mode & ADSL_MODE_ANXL)
	        strcat(buf,"AnnexL  ");
	if (mode & ADSL_MODE_ANXM)
       	 strcat(buf,"AnnexM  ");
	if (mode & ADSL_MODE_ANXB)
       	 strcat(buf,"AnnexB  ");


      boaWrite(wp,"%s",buf);

      boaWrite (wp, "</td>\n </tr>");
      return 0;

    }

  if (!strcmp (name, "adslmode-gdmt"))
    {
      short mode;
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_GDMT)
        boaWrite(wp, "checked");

      return 0;
    }
  if (!strcmp (name, "adslmode-glite"))
    {
      short mode;
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_GLITE)
        boaWrite(wp, "checked");

      return 0;
    }
  if (!strcmp (name, "adslmode-t1413"))
    {
      short mode;
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_T1413)
        boaWrite(wp, "checked");

      return 0;
    }
  if (!strcmp (name, "adslmode-adsl2"))
    {
      short mode;
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_ADSL2)
        boaWrite(wp, "checked");

      return 0;
    }
  if (!strcmp (name, "adslmode-adsl2p"))
    {
      short mode;
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_ADSL2P)
        boaWrite(wp, "checked");

      return 0;
    }
  if (!strcmp (name, "adslmode-anxl"))
    {
      short mode;
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_ANXL)
        boaWrite(wp, "checked");

      return 0;
    }
  if (!strcmp (name, "adslmode-anxm"))
    {
      short mode;
      mib_get( MIB_ADSL_MODE, (void *)&mode);

      if (mode & ADSL_MODE_ANXM)
        boaWrite(wp, "checked");

      return 0;
    }

  if (!strcmp (name, "showtime"))
    {
	boaWrite (wp,
                 "<tr bgcolor=\"#EEEEEE\">\n <td width=40%%><font size=2><b>Showtime:</b></td>");
      boaWrite (wp, "<td width=60%%><font size=2>");
      unsigned int intVal[3];
      char buffer[128]={0};

      if (adsl_drv_get(RLCM_GET_DSL_ORHERS, &intVal, 3*sizeof(int)))
        {
        //add by ramen to adjust the DSL connect time ---68/1
        // jim according paget's advise the accuracy equation is : actual time= showtime - showtime/69;
       	 intVal[0]=intVal[0]-intVal[0]/68;
          if (intVal[0]>3600)
            {
              sprintf(buffer,"%dh:%02dm",intVal[0]/3600,(intVal[0]%3600)/60);
              boaWrite (wp, "%s", buffer);
            }

          else if (intVal[0]>60)
            {
              boaWrite(wp,"%d min",intVal[0]/60);
            }

          else
            boaWrite(wp,"< 1 min");
        }

      boaWrite (wp, "</td>\n </tr>");

      return 0;

    }
#endif // of CONFIG_DEV_xDSL

	if (!strcmp (name, "wlaninfo"))
	{
#ifdef WLAN_SUPPORT
      		const char *bgColor[]={"#EEEEEE","#DDDDDD"};
      		int col_nums=0;
      		const char *wlan_band[] ={0,"802.11b","802.11g","802.11 b+g",0};
      		unsigned char vChar;
		MIB_CE_MBSSIB_T Entry;
		wlan_getEntry(&Entry, 0);
	 	boaWrite(wp,"<tr>\n <td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>Wireless Configuration</b></font></td>");
	  	boaWrite(wp,"<tr bgcolor=%s> <td width=40%%><font size=2><b>Wireless</b></td>",bgColor[col_nums++%2]);

      		//ramen--wireless enable??
      		boaWrite (wp, "<td width=60%%><font size=2>");

     		vChar = Entry.wlanDisabled;
      		if (!vChar)
        		boaWrite (wp, INFO_ENABLED);
      		else{
          		boaWrite (wp, INFO_DISABLED);
          		boaWrite (wp, "\n</td>\n</tr>\n");
          		goto wlend;
        	}

      		boaWrite (wp, "\n</td>\n</tr>\n");

      		//ramen--get the wireless band
	   	boaWrite (wp,"<tr bgcolor=%s> <td width=40%%><font size=2><b>band</b></td>",bgColor[col_nums++%2]);
      		boaWrite (wp, "<td width=60%%><font size=2>");
		vChar = Entry.wlanBand;
     	 	boaWrite (wp, "%s", wlan_band[(BAND_TYPE_T) vChar]);
      		boaWrite (wp, "\n</td>\n</tr>\n");

	  //ramen--get wireless mode
	  	{
			vChar = Entry.wlanMode;
			boaWrite (wp,  "<tr bgcolor=%s> <td width=40%%><font size=2><b>Mode</b></td>",bgColor[col_nums++%2]);
          		boaWrite (wp, "<td width=60%%><font size=2>");

          		if (vChar == AP_MODE)
            			boaWrite (wp, "AP");
          		else if (vChar == CLIENT_MODE)
            			boaWrite (wp, "Client");
          		else if (vChar == AP_WDS_MODE)// jim support wds info shown.
            			boaWrite (wp, "AP+WDS");
          		else if (vChar == WDS_MODE)
            			boaWrite (wp, "WDS");
          		boaWrite (wp, "\n</td>\n</tr>\n");
        	}

      	//ramen---broadcast SSID
	    	boaWrite (wp,  "<tr bgcolor=%s> <td width=40%%><font size=2><b>Broadcast  SSID</b></td>",bgColor[col_nums++%2]);
     		boaWrite (wp, "<td width=60%%><font size=2>");
		vChar = Entry.hidessid;
          	boaWrite (wp, (vChar!=0)?INFO_DISABLED:INFO_ENABLED);
          	boaWrite (wp, "\n</td>\n</tr>\n");

wlend:
      		boaWrite (wp, "</tr>");
#endif
      		return 0;

	}

 	if(!strcmp(name,"wlanencryptioninfo"))
  	{
  		return 0;
  	}

  	if (!strcmp(name,"wlanClient"))
    	{
#ifdef WLAN_SUPPORT
 		boaWrite(wp,"<P><table border=0 width=\"550\">"
                     "<tr> <td width=100%% colspan=\"6\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>Wireless Client List</b></font></td> </tr>"
                     "<tr bgcolor=#7f7f7f><td width=\"25%%\"><font size=2><b>MAC Address</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Tx Packet</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Rx Packet</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Tx Rate (Mbps)</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Power Saving</b></td>"
                     "<td width=\"15%%\"><font size=2><b>Expired Time (s)</b></td></tr>");
			// Mason Yu. t123
      		//wirelessClientList(0,wp,1,0);
      		boaWrite(wp,"</table>");
#endif
      		return 0;
    	}

  	if (!strcmp(name,"wlanAccessControl"))
    	{
#ifdef WLAN_SUPPORT
#ifdef WLAN_ACL
      		unsigned char vChar;
      		char *acType[]={"Disable","Allow Listed","Deny Listed"};

      		mib_get( MIB_WLAN_AC_ENABLED, (void *)&vChar);
   		boaWrite(wp,"<P><table border=0 width=550>"
                     "<tr> <td width=100%%  bgcolor=\"#008000\" colspan=\"2\"><font color=\"#FFFFFF\" size=2><b>Current Access Control List:</b></font></td> </tr>");
      		boaWrite(wp,"<tr bgcolor=\"#EEEEEE\"><td><font size=2><b>Mode</b></font></td><td align=left><font size=2>%s</font></td></tr>",acType[vChar]);

      		if (vChar){
				// Mason Yu. t123
          		//wlShowAcList(0,wp,1,0);
        	}

      		boaWrite(wp," </table>");
#endif
#endif
      		return 0;
    	}

  	if (!strcmp (name, "showpvctable0"))
    	{
   		boaWrite (wp,"<table border=\"0\" width=700><tr><font size=2><b>Current ATM VC Table:</b></font></tr>");
      		atmVcList2 (0, wp, 1, 0);
      		boaWrite (wp, "</table>");
      		return 0;
    	}

  if (!strcmp (name, "showpvctable1"))
    {
      boaWrite (wp,
                 "<table border=\"0\" width=700><tr><font size=2><b>Current ATM VC Table:</b></font></tr>");
      atmVcList2 (0, wp, 1, 0);
      boaWrite (wp, "</table>");
      return 0;
    }
	if ( !strcmp(name, "dhcpMode") ) {
 		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;
/*		if (vChar == 0) {
			boaWrite(wp, "<option selected value=\"0\">None</option>\n" );
			boaWrite(wp, "<option value=\"1\">DHCP Relay</option>\n" );
			boaWrite(wp, "<option value=\"2\">DHCP Server</option>\n" );
		}
		if (vChar == 1) {
			boaWrite(wp, "<option selected value=\"1\">DHCP Relay</option>\n" );
			boaWrite(wp, "<option value=\"0\">None</option>\n" );
			boaWrite(wp, "<option value=\"2\">DHCP Server</option>\n" );
		}
		if (vChar == 2) {
			boaWrite(wp, "<option selected value=\"2\">DHCP Server</option>\n" );
			boaWrite(wp, "<option value=\"0\">None</option>\n" );
			boaWrite(wp, "<option value=\"1\">DHCP Relay</option>\n" );
		}*/
		boaWrite(wp, "<input type=\"radio\" name=dhcpdenable value=0 onClick=\"disabledhcpd()\">None&nbsp;&nbsp;\n" );
		boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=1 onClick=\"enabledhcprelay()\">DHCP Relay&nbsp;&nbsp;\n" );
		boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=2 onClick=\"enabledhcpd()\">DHCP Server&nbsp;&nbsp;\n" );
		return 0;
	}

	if (!strcmp(name, "wan-interface-name")) {
		int mibTotal, i;
		MIB_CE_ATM_VC_T vcEntry;
		char interface_name[MAX_WAN_NAME_LEN];

		mibTotal = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<mibTotal; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vcEntry)) {
				printf("get mib_atm_vc_tbl error!\n");
			}

			if (vcEntry.cmode == CHANNEL_MODE_BRIDGE)
				continue;

			//get name
			getWanName(&vcEntry, interface_name);
			boaWrite(wp, "<option value=\"%s\">%s</option>\n",interface_name,interface_name);
		}
	}

	if ( !strcmp(name, "dhcpV6Mode") ) {
 		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;
/*		if (vChar == 0) {
			boaWrite(wp, "<option selected value=\"0\">None</option>\n" );
			boaWrite(wp, "<option value=\"1\">DHCP Relay</option>\n" );
			boaWrite(wp, "<option value=\"2\">DHCP Server</option>\n" );
		}
		if (vChar == 1) {
			boaWrite(wp, "<option selected value=\"1\">DHCP Relay</option>\n" );
			boaWrite(wp, "<option value=\"0\">None</option>\n" );
			boaWrite(wp, "<option value=\"2\">DHCP Server</option>\n" );
		}
		if (vChar == 2) {
			boaWrite(wp, "<option selected value=\"2\">DHCP Server</option>\n" );
			boaWrite(wp, "<option value=\"0\">None</option>\n" );
			boaWrite(wp, "<option value=\"1\">DHCP Relay</option>\n" );
		}*/
		boaWrite(wp, "<input type=\"radio\" name=dhcpdenable value=0 onClick=\"disabledhcpd()\">Disable&nbsp;\n" );
		//hide unneed UI for real world IPv6 usage
		boaWrite(wp, "<div id=advancedDHCPv6setting style=\"display:none\">" );
			boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=1 onClick=\"enabledhcprelay()\">Relay&nbsp;\n" );
			boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=2 onClick=\"enabledhcpd()\">Server(Manual)&nbsp;\n" );
		boaWrite(wp, "</div>" );
			boaWrite(wp, "<input type=\"radio\"name=dhcpdenable value=3 onClick=\"autodhcpd()\">Enable;\n" );
		return 0;
	}

#ifdef ADDRESS_MAPPING
#ifndef MULTI_ADDRESS_MAPPING
	if ( !strcmp(name, "addressMapType") ) {
 		if ( !mib_get( MIB_ADDRESS_MAP_TYPE, (void *)&vChar) )
			return -1;

		boaWrite(wp, "<option value=0>None</option>\n" );
		boaWrite(wp, "<option value=1>One-to-One</option>\n" );
		boaWrite(wp, "<option value=2>Many-to-One</option>\n" );
		boaWrite(wp, "<option value=3>Many-to-Many Overload</option>\n" );
		// Mason Yu on True
		boaWrite(wp, "<option value=4>One-to-Many</option>\n" );
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
	if ( !strcmp(name, "wlbandchoose") ) {
#if defined(CONFIG_WLAN0_5G_WLAN1_2G) || defined(CONFIG_RTL_92D_SUPPORT)
		boaWrite(wp, "<input type=\"radio\" name=\"select_2g5g\" onClick=\"BandSelected(0)\"> 5GHz");
		boaWrite(wp, "<input type=\"radio\" name=\"select_2g5g\" onClick=\"BandSelected(1)\"> 2.4GHz");
#elif defined (CONFIG_WLAN0_2G_WLAN1_5G)
		boaWrite(wp, "<input type=\"radio\" name=\"select_2g5g\" onClick=\"BandSelected(0)\"> 2.4GHz");
		boaWrite(wp, "<input type=\"radio\" name=\"select_2g5g\" onClick=\"BandSelected(1)\"> 5GHz");
#else
		boaWrite(wp, "<input style=\"display:none\" type=\"radio\" name=\"select_2g5g\" onClick=\"BandSelected(0)\">");
#endif
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
			boaWrite(wp, "<option value=3>5 GHz (A)</option>\n" );
			boaWrite(wp, "<option value=7>5 GHz (N)</option>\n" );
			boaWrite(wp, "<option value=11>5 GHz (A+N)</option>\n" );
#if defined (WLAN0_5G_11AC_SUPPORT) || defined(WLAN1_5G_11AC_SUPPORT)
			boaWrite(wp, "<option value=75>5 GHz (A+N+AC)</option>\n" );
#endif
		}
#endif

#if (defined (WLAN0_5G_SUPPORT) || defined(WLAN1_5G_SUPPORT)) && !defined(WLAN_DUALBAND_CONCURRENT)
		boaWrite(wp, "<option value=3>5 GHz (A)</option>\n" );
		boaWrite(wp, "<option value=7>5 GHz (N)</option>\n" );
		boaWrite(wp, "<option value=11>5 GHz (A+N)</option>\n" );
#if defined (WLAN0_5G_11AC_SUPPORT) || defined(WLAN1_5G_11AC_SUPPORT)
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
		return 0;
	}
	if ( !strcmp(name, "wlctlband") ) {
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
		boaWrite(wp, "<option value=\"0\">Upper</option>\n" );
		boaWrite(wp, "<option value=\"1\">Lower</option>\n" );
#endif
		return 0;
	}
	// Added by Mason Yu for TxPower
	if ( !strcmp(name, "txpower") ) {
//modified by xl_yue
#if 1 //def WLAN_TX_POWER_DISPLAY
			boaWrite(wp, "<option value=\"0\">100%%</option>\n" );
			boaWrite(wp, "<option value=\"1\">70%%</option>\n" );
			boaWrite(wp, "<option value=\"2\">50%%</option>\n" );
			boaWrite(wp, "<option value=\"3\">35%%</option>\n" );
			boaWrite(wp, "<option value=\"4\">15%%</option>\n" );
#else
 		if ( !mib_get( MIB_TX_POWER, (void *)&vChar) )
			return -1;

		if (vChar == 0) {
			boaWrite(wp, "<option selected value=\"0\">15 mW</option>\n" );
			boaWrite(wp, "<option value=\"1\">30 mW</option>\n" );
			boaWrite(wp, "<option value=\"2\">60 mW</option>\n" );
		}
		if (vChar == 1) {
			boaWrite(wp, "<option selected value=\"1\">30 mW</option>\n" );
			boaWrite(wp, "<option value=\"0\">15 mW</option>\n" );
			boaWrite(wp, "<option value=\"2\">60 mW</option>\n" );
		}
		if (vChar == 2) {
			boaWrite(wp, "<option selected value=\"2\">60 mW</option>\n" );
			boaWrite(wp, "<option value=\"0\">15 mW</option>\n" );
			boaWrite(wp, "<option value=\"1\">30 mW</option>\n" );
		}
#endif // of WLAN_TX_POWER_DISPLAY
		return 0;
	}
	if (!strcmp(name, "wifiSecurity")) {
		unsigned char mode = 0;
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		mode = Entry.wlanMode;
		boaWrite(wp, "<option value=%d>None</option>\n", WIFI_SEC_NONE);
		boaWrite(wp, "<option value=%d>WEP</option>\n", WIFI_SEC_WEP);
		boaWrite(wp, "<option value=%d>WPA</option>\n", WIFI_SEC_WPA);
		boaWrite(wp, "<option value=%d>WPA2</option>\n", WIFI_SEC_WPA2);
		if (mode != CLIENT_MODE)
		boaWrite(wp, "<option value=%d>WPA2 Mixed</option>\n", WIFI_SEC_WPA2_MIXED);
#ifdef CONFIG_RTL_WAPI_SUPPORT
		boaWrite(wp, "<option value=%d>WAPI</option>\n", WIFI_SEC_WAPI);
#endif
		return 0;
	}
	if (!strcmp(name, "wpaEncrypt")) {
		unsigned char band = 0;
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;

		band = Entry.wlanBand;

		boaWrite(wp, "<option value=%d>None</option>\n", ENCRYPT_DISABLED);
		boaWrite(wp, "<option value=%d>WEP</option>\n", ENCRYPT_WEP);
		if (!wl_isNband(band))
			boaWrite(wp, "<option value=%d>WPA(TKIP)</option>\n", ENCRYPT_WPA_TKIP);
#ifdef ENABLE_WPAAES_WPA2TKIP
		boaWrite(wp, "<option value=%d>WPA(AES)</option>\n", ENCRYPT_WPA_AES);
#endif
		boaWrite(wp, "<option value=%d>WPA2(AES)</option>\n", ENCRYPT_WPA2_AES);
#ifdef ENABLE_WPAAES_WPA2TKIP
		if (!wl_isNband(band))
			boaWrite(wp, "<option value=%d>WPA2(TKIP)</option>\n", ENCRYPT_WPA2_TKIP);
#endif
		boaWrite(wp, "<option value=%d>WPA2 Mixed</option>\n", ENCRYPT_WPA2_MIXED);
#ifdef CONFIG_RTL_WAPI_SUPPORT
		boaWrite(wp, "<option value=%d>WAPI</option>\n", ENCRYPT_WAPI);
#endif
	}
	#ifdef WLAN_UNIVERSAL_REPEATER
	if ( !strcmp(name, "repeaterEnabled") ) {
		mib_get( MIB_REPEATER_ENABLED1, (void *)&vChar);
		if (vChar)
			boaWrite(wp, "checked");
	}
	#endif
	if( !strcmp(name, "wlan_idx") ) {
		boaWrite(wp, "%d", wlan_idx);
		return 0;
	}
	if( !strcmp(name, "2G_ssid") ) {
		char ssid[33];
		int i, orig_wlan_idx = wlan_idx;
		MIB_CE_MBSSIB_T Entry;
#ifdef CONFIG_RTL_92D_SUPPORT
		for(i=0; i<NUM_WLAN_INTERFACE; i++) {
			wlan_idx = i;
			mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
			if(vChar == PHYBAND_2G) {
				if(!wlan_getEntry(&Entry, 0))
					return -1;
				strcpy(ssid, Entry.ssid);
				boaWrite(wp, "%s", ssid);
				break;
			}
		}
#else //CONFIG_RTL_92D_SUPPORT
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		strcpy(ssid, Entry.ssid);		
		boaWrite(wp, "%s", ssid);
#endif //CONFIG_RTL_92D_SUPPORT
		wlan_idx = orig_wlan_idx;
		return 0;
	}
	if( !strcmp(name, "5G_ssid") ) {
		char ssid[33];
		int i, orig_wlan_idx = wlan_idx;
		MIB_CE_MBSSIB_T Entry;
#ifdef CONFIG_RTL_92D_SUPPORT
		for(i=0; i<NUM_WLAN_INTERFACE; i++) {
			wlan_idx = i;
			mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
			if(vChar == PHYBAND_5G) {
				if(!wlan_getEntry(&Entry, 0))
					return -1;
				strcpy(ssid, Entry.ssid);
				boaWrite(wp, "%s", ssid);
				break;
			}
		}
#else //CONFIG_RTL_92D_SUPPORT
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		strcpy(ssid, Entry.ssid);		
		boaWrite(wp, "%s", ssid);
#endif //CONFIG_RTL_92D_SUPPORT
		wlan_idx = orig_wlan_idx;
		return 0;
	}
	if( !strcmp(name, "Band2G5GSupport") ) {
#ifdef CONFIG_RTL_92D_SUPPORT
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar);
		boaWrite(wp, "%d", vChar);
#else //CONFIG_RTL_92D_SUPPORT
		vChar = PHYBAND_2G;
		boaWrite(wp, "%d", vChar);
#endif //CONFIG_RTL_92D_SUPPORT
		return 0;
	}
#ifdef CONFIG_RTL_92D_SUPPORT
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
#endif //CONFIG_RTL_92D_SUPPORT
	if(!strcmp(name, "wlan_num")){
		boaWrite(wp, "%d", NUM_WLAN_INTERFACE);
		return 0;
	}
	if(!strcmp(name,"wlan_support_8812e")) //8812
	{
#if (defined(CONFIG_RTL_8812_SUPPORT) && !defined(CONFIG_RTL_8812AR_VN_SUPPORT)) || defined(WLAN0_5G_11AC_SUPPORT) || defined(WLAN1_5G_11AC_SUPPORT)
		boaWrite(wp, "1");
#else
		boaWrite(wp, "0");
#endif
		return 0;
	}
#endif // of WLAN_SUPPORT


	if ( !strcmp(name, "wapiScript0") ) {
		#ifdef CONFIG_RTL_WAPI_SUPPORT
		boaWrite(wp, "if ( (form.method.selectedIndex == 2 && wpaAuth[0].checked) ) {"\
			"\tdisableTextField(form.radiusPort);"\
			"\tenableTextField(form.radiusIP);"\
			"\tdisableTextField(form.radiusPass);"\
			"\tdisableTextField(document.formEncrypt.pskFormat);"\
			"\tdisableTextField(document.formEncrypt.pskValue);"\
			"} else");
		#endif
	}

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
		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;
		if (DHCP_LAN_SERVER == vChar)
			boaWrite(wp, "Enabled");
		else
			boaWrite(wp, "Disabled");
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
//#ifdef CONFIG_USER_UPNPD
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
#endif

// Mason Yu. MLD Proxy
#ifdef CONFIG_IPV6
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

#endif

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
/*	else if ( !strcmp(name, "portFwEn")) {
		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}*/
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
	if ( !strcmp(name, "portFwNum")) {
		vUInt = mib_chain_total(MIB_PORT_FW_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
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
	else if ( !strcmp(name, "radvd_enable0")) {
		if ( !mib_get( MIB_V6_RADVD_ENABLE, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "radvd_enable1")) {
		if ( !mib_get( MIB_V6_RADVD_ENABLE, (void *)&vChar) )
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
/*	else if ( !strcmp(name, "dmzEn")) {
		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		if (vChar)
			boaWrite(wp, "checked");
		return 0;
	}*/
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
	else if ( !strcmp(name, "ipFilterNum")) {
		vUInt = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
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
	else if ( !strcmp(name, "aclNum")) {
		vUInt = mib_chain_total(MIB_ACL_IP_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
	else if ( !strcmp(name, "macFilterNum")) {
		vUInt = mib_chain_total(MIB_MAC_FILTER_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
#ifdef PARENTAL_CTRL
	else if( !strcmp(name, "parentCtrlNum")) {
			return 1;  //temp
		}
/*
	else if ( !strcmp(name, "parentCtrlNum")) {
		vUInt = mib_chain_total(MIB_MAC_FILTER_TBL);
		if (0 == vUInt)
			boaWrite(wp, "disableDelButton();");
		return 0;
	}
*/
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
	else if ( !strcmp(name, "pppoeStatus") ) {
		if (0) {
			boaWrite(wp, "\n<script> setPPPConnected(); </script>\n");
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
     else if(!strcmp(name,"PostVC"))
     	{
     	   boaWrite(wp,"function postVC(vpi,vci,encap,napt,mode,username,passwd,pppType,idletime,pppoeProxyEnable,ipunnum,ipmode,ipaddr,remoteip,netmask,droute,status,enable)");
     	}
     else if(!strcmp(name,"pppoeProxyEnable"))
     	{
	boaWrite(wp,"  if(mode==\"PPPoE\""
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
   else if(!strcmp(name,"PostVC"))
     	{
     	   boaWrite(wp,"function postVC(vpi,vci,encap,napt,mode,username,passwd,pppType,idletime,ipunnum,ipmode,ipaddr,remoteip,netmask,droute,status,enable)");
     	}
     else if(!strcmp(name,"pppoeProxyEnable"))
     	{

     	}
  #endif

	else if ( !strcmp(name, "adsl-line-mode") ) {
		if ( !mib_get( MIB_ADSL_MODE, (void *)&vChar) )
			return -1;
		if (1 == vChar) {
			boaWrite(wp,"<option selected value=\"1\">T1.413</option>");
			boaWrite(wp,"<option value=\"2\">G.dmt</option>");
			boaWrite(wp,"<option value=\"3\">MultiMode</option>");
		} else if (2 == vChar) {
			boaWrite(wp,"<option value=\"1\">T1.413</option>");
			boaWrite(wp,"<option selected value=\"2\">G.dmt</option>");
			boaWrite(wp,"<option value=\"3\">MultiMode</option>");
		} else if (3 == vChar) {
			boaWrite(wp,"<option value=\"1\">T1.413</option>");
			boaWrite(wp,"<option value=\"2\">G.dmt</option>");
			boaWrite(wp,"<option selected value=\"3\">MultiMode</option>");
		}
		return 0;
	}
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

#ifdef CONFIG_BOA_WEB_E8B_CH	//cathy
#ifdef CONFIG_USB_RTL8187SU_SOFTAP
		vUShort = Entry.mlcstRate;
		boaWrite(wp, "mulrate=%d\n",vUShort);
#else
		boaWrite(wp, "mulrate=0\n");
#endif
#endif
	}

	else if ( !strcmp(name, "wl_chno")) {
		mib_get( MIB_HW_REG_DOMAIN, (void *)&vChar);
		boaWrite(wp, "regDomain=%d\n",vChar);
		mib_get( MIB_WLAN_CHAN_NUM ,(void *)&vChar);
		boaWrite(wp, "defaultChan=%d\n",vChar);
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

	//for adsl debug
	else if ( !strcmp(name, "adsldbg-cap0") ) {
   		if ( !mib_get( MIB_ADSL_DEBUG, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "adsldbg-cap1") ) {
   		if ( !mib_get( MIB_ADSL_DEBUG, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
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
	else if ( !strcmp(name, "tr069-certauth-0") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_CERT_AUTH)==0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "tr069-certauth-1") ) {
   		if ( !mib_get( CWMP_FLAG, (void *)&vChar) )
			return -1;
		if ( (vChar & CWMP_FLAG_CERT_AUTH)!=0 )
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
	else if ( !strcmp(name, "check-certca") ) {
		if(access("/var/config/cacert.pem", F_OK ) == -1)
			boaWrite(wp, "disabled");
		return 0;
	}
	
//czhu add for middleware 2015-5-4
#ifdef CONFIG_MIDDLEWARE
	else if ( !strcmp(name, "midware-disable") ) {
		if ( !mib_get(CWMP_TR069_ENABLE, (void *)&vChar) )
			return -1;
		if ( vChar == 1 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "midware-enable1") ) {
		if ( !mib_get(CWMP_TR069_ENABLE, (void *)&vChar) )
			return -1;
		if ( vChar == 0 )
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "midware-enable2") ) {
		if ( !mib_get(CWMP_TR069_ENABLE, (void *)&vChar) )
			return -1;
		if ( vChar == 2 )
			boaWrite(wp, "checked");
		return 0;
	}
#endif //end of CONFIG_MIDDLEWARE
// Mason Yu. t123
#if 0
	else if( !strcmp(name, "tr069-connReqEnable")){ // star: for e8b feature
		if ( !mib_get( CWMP_CONREQ_ENABLE, (void *)&vChar) )
			return -1;
		if (vChar == 1 )
			boaWrite(wp, "checked");
		return 0;
	}
#endif
#endif
#ifdef WLAN_SUPPORT
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
	else if (!strcmp(name, "wpsVer")) {
	#ifdef WPS20
		boaWrite(wp, "wps20 = 1;\n");
	#else
		boaWrite(wp, "wps20 = 0;\n");
	#endif
		return 0;
	}
	else if (!strcmp(name, "wscConfig-0") ) {
		MIB_CE_MBSSIB_T Entry;
		if(!wlan_getEntry(&Entry, 0))
			return -1;
		if (!Entry.wsc_configured) 
			boaWrite(wp, "checked");
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
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)		// add by yq_zhou 1.20
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
	else if (!strcmp(name,"protectionDisabled-0")){
		if (!mib_get(MIB_WLAN_PROTECTION_DISABLED,(void *)&vChar))
			return -1;
		if (!vChar)
			boaWrite(wp,"checked");
	}
	else if (!strcmp(name,"protectionDisabled-1")){
		if (!mib_get(MIB_WLAN_PROTECTION_DISABLED,(void *)&vChar))
			return -1;
		if (vChar)
			boaWrite(wp,"checked");
	}
	else if (!strcmp(name,"aggregation-0")){
		if (!mib_get(MIB_WLAN_AGGREGATION,(void *)&vChar))
			return -1;
		if (vChar)
			boaWrite(wp,"checked");
	}
	else if (!strcmp(name,"aggregation-1")){
		if (!mib_get(MIB_WLAN_AGGREGATION,(void *)&vChar))
			return -1;
		if (vChar)
			boaWrite(wp,"checked");
	}
	else if (!strcmp(name,"shortGIEnabled-0")){
		if (!mib_get(MIB_WLAN_SHORTGI_ENABLED,(void *)&vChar))
			return -1;
		if (vChar)
			boaWrite(wp,"checked");
	}
	else if (!strcmp(name,"shortGIEnabled-1")){
		if (!mib_get(MIB_WLAN_SHORTGI_ENABLED,(void *)&vChar))
			return -1;
		if (vChar)
			boaWrite(wp,"checked");
	}
#endif
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
		vChar = Entry.wsc_enc;
		switch(vChar) {
			case 0:
			case WSC_ENCRYPT_NONE: boaWrite(wp, "None"); break;
			case WSC_ENCRYPT_WEP: boaWrite(wp, "WEP"); break;
			case WSC_ENCRYPT_TKIP: boaWrite(wp, "TKIP"); break;
			case WSC_ENCRYPT_AES: boaWrite(wp, "AES"); break;
			case WSC_ENCRYPT_TKIPAES: boaWrite(wp, "TKIP+AES"); break;
			default:
				break;
		}
		return 0;
	}
#endif
#endif
	// Jenny, for RFC1577
	if ( !strcmp(name, "adslcmode") ) {
#ifdef CONFIG_ATM_CLIP
		boaWrite(wp, "<option value=\"5\">1577 Routed</option>" );
#endif
		return 0;
	}
#ifndef CONFIG_GUI_WEB
//add by ramen for create new menu
// remove for e8b
#if 0
       else if(!strcmp(name,"CreateMenu")){

	   #ifdef WEB_MENU_USE_NEW
	   createMenu(0,wp,0,0);
	   #else
	   addMenuJavaScript( wp,0,0);
	   char *argv[]={"1",0};
	   if(!strcmp(argv[0],"1"))
	                     createMenu(0,wp,2,argv);
	   boaWrite(wp,"</script>\n</head>\n");
	   boaWrite(wp,"<body onload=\"MTMStartMenu(true)\" bgcolor=\"#000033\" text=\"#ffffcc\" link=\"yellow\" vlink=\"lime\" alink=\"red\">\n");
	   if(!strcmp(argv[0],"0"))
	                     {
	                      boaWrite(wp,"<table width=100%% border=0 cellpadding=0 cellspacing=0>\n<tr><td width=20%%>&nbsp&nbsp</td><td  align=left>\n");
	                     createMenu(0,wp,2,argv);
			   boaWrite(wp,"</td></tr>\n</table>\n");
	   	     }

	   #endif
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
#endif
#else	
	else if(!strcmp(name,"naptEnable"))
	{
		boaWrite(wp,"\tif ((document.adsl.adslConnectionMode.selectedIndex == 1) ||\n"
			"\t\t(document.adsl.adslConnectionMode.selectedIndex == 2) || (document.adsl.adslConnectionMode.selectedIndex == 3))\n"
			"\t\tdocument.adsl.naptEnabled.checked = true;\n"
			"\telse\n"
			"\t\tdocument.adsl.naptEnabled.checked = false;\n");
		return 0;
	}
#endif
	else if(!strcmp(name,"qos_mode"))
	{
#if defined(CONFIG_USER_IP_QOS) && defined(_PRMT_X_CT_COM_QOS_)
		char tmpBuf[40];
   		if ( mib_get(CTQOS_MODE, (void *)&tmpBuf) ){
			boaWrite(wp,"\t%s\n",tmpBuf);
		}
#endif
		return 0;
	}
	
#ifdef CONFIG_IPV6
	else if(!strcmp(name,"IPv6vcCheck1"))
	{
		if (boaArgs(argc, argv, "%*s %s", &name) < 2) {
			boaError(wp, 400, "Insufficient args\n");
			return -1;
		}

		boaWrite(wp,"if(%s.IpProtocolType.value == 3 || %s.IpProtocolType.value == 1){\n", name, name);
		return 0;
	}
	else if(!strcmp(name,"IPv6vcCheck2"))
	{
		boaWrite(wp,"}\n");
		return 0;
	}
	else if(!strcmp(name,"IPv6vcCheck3"))
	{
		if (boaArgs(argc, argv, "%*s %s", &name) < 2) {
			boaError(wp, 400, "Insufficient args\n");
			return -1;
		}
	boaWrite(wp,"if(%s.IpProtocolType.value == 3 || %s.IpProtocolType.value == 2)\n"
			"\t{\n"
			"\t	if(%s.staticIpv6.checked) {\n"
			"\t		if(%s.itfenable.checked == false ){\n"
			"\t			if(%s.Ipv6Addr.value == \"\" ){\n"
			"\t				alert(\"Please input ipv6 address or open DHCPv6 client!\");\n"
			"\t				%s.Ipv6Addr.focus();\n"
			"\t				return false;\n"
			"\t			}\n"
			"\t		}\n", name, name, name, name, name, name);
	boaWrite(wp,"\t		if(%s.Ipv6Addr.value != \"\"{\n"
			"\t			if (! isGlobalIpv6Address( %s.Ipv6Addr.value) ){\n"
			"\t				alert(\"Invalid ipv6 address!\");\n"
			"\t				%s.Ipv6Addr.focus();\n"
			"\t				return false;\n"
			"\t			}\n", name, name, name);
	boaWrite(wp,"\t			var prefixlen= getDigit(%s.Ipv6PrefixLen.value, 1);\n"
			"\t			if (prefixlen > 128 || prefixlen <= 0) {\n"
			"\t				alert(\"Invalid ipv6 prefix length!\");\n"
			"\t				%s.Ipv6PrefixLen.focus();\n"
			"\t				return false;\n"
			"\t			}\n"
			"\t		}\n", name, name);
	boaWrite(wp,"\t		if(%s.Ipv6Gateway.value != \"\" ){\n"
			"\t			if (! isUnicastIpv6Address( %s.Ipv6Gateway.value) ){\n"
			"\t				alert(\"Invalid ipv6 gateway address!\");\n"
			"\t				%s.Ipv6Gateway.focus();\n"
			"\t				return false;\n"
			"\t			}\n"
			"\t		}\n", name, name, name);
	boaWrite(wp,"\t	}else{\n"
			"\t		%s.Ipv6Addr.value = \"\";\n"
			"\t		%s.Ipv6PrefixLen.value = \"\";\n"
			"\t		%s.Ipv6Gateway.value = \"\";\n"
			"\t	}\n"
			"\t}\n", name, name, name);

		return 0;
	}
	else if(!strcmp(name,"IPv6vcCheck9"))
	{
		boaWrite(wp,"<tr nowrap><td width=\"150px\"><input type=\"radio\" id=\"IPMode\" name=\"ipmode\" value=\"3\" onClick=\"on_ctrlupdate(this)\">Static</td><td>经ISP处配置一个静态地址</td></tr>\n");
		return 0;
	}
	else if(!strcmp(name,"IPv6ChannelMode1"))
	{
		boaWrite(wp,"ipv6SettingsEnable();\n"
			       "	document.getElementById('tbprotocol').style.display=\"block\";\n");
		return 0;
	}
	else if(!strcmp(name,"IPv6ChannelMode2"))
	{
		boaWrite(wp,"ipv6SettingsDisable();\n"
			       "		document.getElementById('tbprotocol').style.display=\"none\";\n");
		return 0;
	}
#endif
	else if(!strcmp(name,"IPv6Show"))
	{
#ifdef CONFIG_IPV6
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
	//add by ramen for zte acl default ip
	else if(!strcmp(name,"remoteClientIp"))
	{
		boaWrite(wp,"%s",wp->remote_ip_addr);
		return 0;
	}
#ifdef DEFAULT_GATEWAY_V2
	// Jenny, for PPPoE auto route
	else if ( !strcmp(name, "autort") ) {
#ifdef AUTO_PPPOE_ROUTE
		boaWrite(wp, "<option value=239>Auto</option>" );
#endif
		return 0;
	}
#endif
	else if ( !strcmp(name, "pppExist") ) {
		MIB_CE_ATM_VC_T Entry;
		unsigned int totalEntry;
		int i, isPPP=0;
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++)
			if (mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				if (Entry.enable == 1 && (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA)) {
					isPPP = 1;
					break;
				}
		if (isPPP == 0)
			boaWrite(wp,"document.pppauth.username.disabled = true;\n"
							"document.pppauth.password.disabled = true;\n"
							"document.all.Submit.disabled = true;\n");
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
		boaWrite(wp,"<font size=2>开启DNS绑定:<input type=\"checkbox\" name=\"enableDnsBind\" value=\"on\" %s onClick=\"DnsBindPvcClicked();\"></font>",
			(dnsBindPvcEnable)?"checked":"");
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
		boaWrite(wp,"<font size=2>开启DNSv6绑定:<input type=\"checkbox\" name=\"enableDnsv6Bind\" value=\"on\" %s onClick=\"Dnsv6BindPvcClicked();\"></font>",
			(dnsv6BindPvcEnable)?"checked":"");
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

	else if(!strcmp(name,"QosSpeedLimitWeb"))
		{
#ifdef QOS_SPEED_LIMIT_SUPPORT
	         boaWrite(wp,"<td>\n"
		"<input type=\"checkbox\" name= qosspeedenable onClick=\"qosSpeedClick(this)\"; > <font size=2>限速</font>\n"
		"</td>\n "
	         "<td>\n"
	         "<div id='speedlimit' style=\"display:none\">\n"
		"<table>\n"
		"<tr>\n<td>\n"
		"<input type=text name=speedLimitRank  size=6 maxlength=5 >\n"
		"</td>\n<td><font size=2>\nkBps< bytes/sec * 1024></td>\n</tr>\n</table>\n"
		"</div>\n"
		"</td>\n");
#endif
		return 0;
		}
#if defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD) || defined(CONFIG_USER_ROUTED_ROUTED)
	else if (!strcmp(name, "ospf")) {
#ifdef CONFIG_USER_ROUTED_ROUTED
		boaWrite(wp, "	<option value=\"0\">RIP</option>\n");
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
		boaWrite(wp, "	<option value=\"1\">OSPF</option>");
#endif
	}
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
#ifndef CONFIG_11N_SAGEM_WEB
//		boaWrite(wp, "<img src=\"graphics/topbar.gif\" width=900 height=90 border=0>");
		boaWrite(wp,	"<img src=\"graphics/topbar.gif\" width=900 height=60 border=0>");
#else
//		boaWrite(wp,"<img src=\"graphics/sagemlogo1.gif\" width=1350 height=90 border=0>");
		boaWrite(wp,	"<img src=\"graphics/sagemlogo1.gif\" width=1350 height=60 border=0>");
#endif
	}
	else if(!strncmp(name, "logobelow", 9))	{
#ifdef CONFIG_11N_SAGEM_WEB
//		boaWrite(wp,"<img src=\"graphics/sagemlogo2.gif\" width=180 height=90 border=0>");
		boaWrite(wp,"<img src=\"graphics/sagemlogo2.gif\" width=180 height=60 border=0>");
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
	else if ( !strcmp(name, "usb-res0") ) {
#ifdef _PRMT_USBRESTORE
   		if (!mib_get(MIB_USBRESTORE, (void *)&vChar))
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
#else
		boaWrite(wp, "checked");
#endif
		return 0;
	}
	else if ( !strcmp(name, "usb-res1") ) {
#ifdef _PRMT_USBRESTORE
   		if ( !mib_get( MIB_USBRESTORE, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
#endif
		return 0;
	}
	else if ( !strcmp(name, "sntp_if_type_voip") ) {
#ifdef VOIP_SUPPORT
		boaWrite(wp, "<option value = \"1\">VOICE</option>\n");
#endif
		return 0;
	}
#ifdef CONFIG_IPV6
	else if ( !strcmp(name, "lanipv6addr") ) {
		char tmpBuf[40];
   		if ( !mib_get( MIB_IPV6_LAN_IP_ADDR, (void *)&tmpBuf) )
			return -1;
		boaWrite(wp, "%s", tmpBuf);
		return 0;
	}
	else if ( !strcmp(name, "lanipv6prefix") ) {
		char tmpBuf[40], len;
   		if ( !mib_get( MIB_IPV6_LAN_PREFIX, (void *)&tmpBuf) )
		{
			return -1;
		}
   		if ( !mib_get( MIB_IPV6_LAN_PREFIX_LEN, (void *)&len) )
		{
			return -1;
		}
		if(tmpBuf[0] && (len!=0))
			boaWrite(wp, "%s/%d", tmpBuf,len);
		return 0;
	}
#endif
#if defined(CONFIG_USER_IP_QOS) && defined(_PRMT_X_CT_COM_QOS_)
	else if ( !strcmp(name, "enable_force_weight0")) {
		if ( !mib_get( MIB_QOS_ENABLE_FORCE_WEIGHT, (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "enable_force_weight1")) {
		if ( !mib_get( MIB_QOS_ENABLE_FORCE_WEIGHT, (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "enable_dscp_remark0")) {
		if ( !mib_get( MIB_QOS_ENABLE_DSCP_MARK , (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "enable_dscp_remark1")) {
		if ( !mib_get( MIB_QOS_ENABLE_DSCP_MARK , (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "enable_1p_remark0")) {
		if ( !mib_get( MIB_QOS_ENABLE_1P , (void *)&vChar) )
			return -1;
		if (0 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "enable_1p_remark1")) {
		if ( !mib_get( MIB_QOS_ENABLE_1P , (void *)&vChar) )
			return -1;
		if (1 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
	else if ( !strcmp(name, "enable_1p_remark2")) {
		if ( !mib_get( MIB_QOS_ENABLE_1P , (void *)&vChar) )
			return -1;
		if (2 == vChar)
			boaWrite(wp, "checked");
		return 0;
	}
#endif
	else if ( !strcmp(name, "priv") ) {
		struct user_info *pUser_info;

		pUser_info = search_login_list(wp);
		if (!pUser_info)
			return -1;

		if (!pUser_info->priv)
			boaWrite(wp, "style=\"display: none\"");

		return 0;
	}
	else if(!strcmp(name, "vlan_mapping_interface")){
		int i;
		boaWrite(wp, "[");
		for(i=1; i<SW_LAN_PORT_NUM;i++)
			boaWrite(wp, "\"LAN%d\",",i);
		boaWrite(wp, "\"LAN%d\"",i);
#ifdef WLAN_SUPPORT
		boaWrite(wp, ",\"SSID1\"");
#ifdef WLAN_MBSSID
		MIB_CE_MBSSIB_T entry;
		for (i = 0; i < 3; i++)
		{
			mib_chain_get(MIB_MBSSIB_TBL, i + 1, &entry);
			if (entry.wlanDisabled) {
				continue;
			}
			boaWrite(wp, ",\"SSID%d\"", i + 2);
		}
#endif
		boaWrite(wp,"]");
#else
		boaWrite(wp,"]");
#endif
		
		return 0;
	}
#if 0
	else if (!strcmp(name, "ctmdw_off"))
	{
		unsigned char vChar;

		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if (1 == vChar)//off
			boaWrite(wp,"checked");
	}
	else if (!strcmp(name, "ctmdw_on"))
	{
		unsigned char vChar;

		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if (1 != vChar)//on
			boaWrite(wp,"checked");
	}
#endif//end of CONFIG_MIDDLEWARE
	else if(!strcmp(name, "qos_interface")){
		int i;
		for(i=1; i<=SW_LAN_PORT_NUM;i++)
			boaWrite(wp, ", \"LAN%d\"",i);
		return 0;
	}
	else if(!strcmp(name, "wan_interface_name"))
	{
#ifdef CONFIG_DEV_xDSL
		boaWrite(wp, "DSL");
#elif defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
		boaWrite(wp, "PON");
#endif
		return 0;
	}
	else
		return -1;

	return 0;
}

void write_wladvanced(int eid, request* wp, int argc, char **argv)        //add by yq_zhou 1.20
{
#if 0
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Preamble Type:</b></td>"\
     "<td width=\"70%%\"><font size=2>"\
     "<input type=\"radio\" value=\"long\" name=\"preamble\">Long Preamble&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"preamble\" value=\"short\">Short Preamble</td></tr>"\
     "<tr><td width=\"30%%\"><font size=2><b>Broadcast SSID:</b></td>"\
     "<td width=\"70%%\"><font size=2>"\
     "<input type=\"radio\" name=\"hiddenSSID\" value=\"no\">Enabled&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"hiddenSSID\" value=\"yes\">Disabled</td></tr>");
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Relay Blocking:</b></td>"\
     "<td width=\"70%%\"><font size=2>"\
     "<input type=\"radio\" name=block value=1>Enabled&nbsp;&nbsp;"\
     "<input type=\"radio\" name=block value=0>Disabled</td></tr>");
#ifdef CONFIG_USB_RTL8192SU_SOFTAP
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Protection:</b></td>"\
     "<td width=\"70%%\"><font size=2>"\
     "<input type=\"radio\" name=\"11g_protection\" value=\"yes\">Enabled&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"11g_protection\" value=\"no\">Disabled</td></tr>");
     	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Aggregation:</b></td>"\
     "<td width=\"70%%\"><font size=2>"\
     "<input type=\"radio\" name=\"aggregation\" value=\"enable\">Enabled&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"aggregation\" value=\"disable\">Disabled</td></tr>");
       boaWrite(wp,
     "<tr id=\"ShortGi\" style=\"display:\">"\
     "<td width=\"30%%\"><font size=2><b>Short GI:</b></td>"\
     "<td width=\"70%%\"><font size=2>"\
     "<input type=\"radio\" name=\"shortGI0\" value=\"on\">Enabled&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"shortGI0\" value=\"off\">Disabled</td></tr>");
       boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>RF Output Power:</b></td>"\
     "<td width=\"70%%\"><font size=2>"\
     "<input type=\"radio\" name=\"RFPower\" value=0>100%%&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"RFPower\" value=1>70%%&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"RFPower\" value=2>50%%&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"RFPower\" value=3>35%%&nbsp;&nbsp;"\
     "<input type=\"radio\" name=\"RFPower\" value=4>15%%</td></tr>");
#endif
#endif
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Preamble Type:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" value=\"long\" name=\"preamble\">Long Preamble&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"preamble\" value=\"short\">Short Preamble</td></tr>\n");
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Broadcast SSID:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"hiddenSSID\" value=\"no\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"hiddenSSID\" value=\"yes\">Disabled</td></tr>\n");
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Relay Blocking:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=block value=1>Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=block value=0>Disabled</td></tr>\n");
	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Protection:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"protection\" value=\"yes\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"protection\" value=\"no\">Disabled</td></tr>\n");
   #if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
  	boaWrite(wp,
     "<tr><td width=\"30%%\"><font size=2><b>Aggregation:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"aggregation\" value=\"enable\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"aggregation\" value=\"disable\">Disabled</td></tr>\n");
       boaWrite(wp,
     "<tr id=\"ShortGi\" style=\"display:\">\n"
     "<td width=\"30%%\"><font size=2><b>Short GI:</b></td>\n"
     "<td width=\"70%%\"><font size=2>\n"
     "<input type=\"radio\" name=\"shortGI0\" value=\"on\">Enabled&nbsp;&nbsp;\n"
     "<input type=\"radio\" name=\"shortGI0\" value=\"off\">Disabled</td></tr>\n");
#endif
}

/* add by yq_zhou 09.2.02 add sagem logo for 11n*/
#if 0
void write_title(int eid, request* wp, int argc, char **argv)
{
	printf("%s ...............1\n",__FUNCTION__);
#ifndef CONFIG_11N_SAGEM_WEB
	boaWrite(wp,	"<img src=\"graphics/topbar.gif\" width=900 height=60 border=0>");
#else
	boaWrite(wp,	"<img src=\"graphics/sagemlogo1.gif\" width=1350 height=60 border=0>");
#endif
}

void write_logo_below(int eid, request* wp, int argc, char **argv)
{
#ifdef CONFIG_11N_SAGEM_WEB
	printf("%s ...............1\n",__FUNCTION__);
	boaWrite(wp,
	"<img src=\"graphics/sagemlogo2.gif\" width=160 height=80 border=0>");
#endif
}
#endif

// Kaohj
#if 0
int getIndex(int eid, request* wp, int argc, char **argv)
{
	char *name;
	char  buffer[100];

	unsigned char vChar;
	unsigned short vUShort;
	unsigned int vUInt;

   	if (boaArgs(argc, argv, "%s", &name) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
   		return -1;
   	}

	memset(buffer,0x00,100);
   	if ( !strcmp(name, "device-type") ) {
 		if ( !mib_get( MIB_DEVICE_TYPE, (void *)&vChar) )
			return -1;
#ifdef __uClinux__
		sprintf(buffer, "%u", vChar);
#else
		sprintf(buffer,"%u", 1);
#endif
		ejSetResult(eid, buffer);
		return 0;
	}
	if ( !strcmp(name, "dhcp-mode") ) {
 		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	if ( !strcmp(name, "adsl-line-mode") ) {
 		if ( !mib_get( MIB_ADSL_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef CONFIG_USER_ROUTED_ROUTED
   	if ( !strcmp(name, "rip-on") ) {
 		if ( !mib_get( MIB_RIP_ENABLE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	if ( !strcmp(name, "rip-ver") ) {
 		if ( !mib_get( MIB_RIP_VERSION, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif
// Commented by Mason Yu for dhcpmode
#if 0
   	if ( !strcmp(name, "lan-dhcp") ) {
 		if ( !mib_get( MIB_ADSL_LAN_DHCP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif
 	else if ( !strcmp(name, "br-stp") ) {
   		if ( !mib_get( MIB_BRCTL_STP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef CONFIG_EXT_SWITCH
 	else if ( !strcmp(name, "mp-mode") ) {
   		if ( !mib_get( MIB_MPMODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif
#ifdef CONFIG_USER_IGMPPROXY
 	else if ( !strcmp(name, "igmp-proxy") ) {
   		if ( !mib_get( MIB_IGMP_PROXY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif

	else if ( !strcmp(name, "acl-cap") ) {
   		if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}

   	else if ( !strcmp(name, "wan-dns") ) {
 		if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "portFwEnabled")) {
		if ( !mib_get( MIB_PORT_FW_ENABLE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "defaultFilterAction")) {
		if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "dmzEnabled")) {
		if ( !mib_get( MIB_DMZ_ENABLE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "vc-auto")) {
		if ( !mib_get( MIB_ATM_VC_AUTOSEARCH, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, "ippt-itf")) {
		if( !mib_get( MIB_IPPT_ITF,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, "ippt-lanacc")) {
		if( !mib_get( MIB_IPPT_LANACC,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "rconf-status")) {
		sprintf(buffer, "%d", g_remoteConfig);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "rconf-port")) {
		sprintf(buffer, "%d", g_remoteAccessPort);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, "spc-enable")) {
		if( !mib_get( MIB_SPC_ENABLE,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, "spc-iptype")) {
		if( !mib_get( MIB_SPC_IPTYPE,  (void *)&vChar))
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "ipPortFilterNum")) {
		vUInt = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "macFilterNum")) {
		vUInt = mib_chain_total(MIB_MAC_FILTER_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "portFwNum")) {
		vUInt = mib_chain_total(MIB_PORT_FW_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "atmVcNum")) {
		vUInt = mib_chain_total(MIB_ATM_VC_TBL);
		sprintf(buffer, "%u", vUInt);
		ejSetResult(eid, buffer);
		return 0;
	}
   	else if ( !strcmp(name, "wan-pppoeConnectStatus") ) {
////	check the pppoe status
		sprintf(buffer, "%d", 0);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef WLAN_SUPPORT
	else if ( !strcmp(name, "channel") ) {
		if ( !mib_get( MIB_WLAN_CHAN_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%d", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "regDomain") ) {
		if ( !mib_get( MIB_HW_REG_DOMAIN, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wep") ) {
		if ( !mib_get( MIB_WLAN_WEP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
   	    	return 0;
	}
	else if ( !strcmp(name, "defaultKeyId") ) {
		if ( !mib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar) )
			return -1;
		vChar++;
		sprintf(buffer, "%u", vChar) ;
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "keyType") ) {
		if ( !mib_get( MIB_WLAN_WEP_KEY_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		ejSetResult(eid, buffer);
		return 0;
	}
  	else if ( !strcmp(name, "authType")) {
		if ( !mib_get( MIB_WLAN_AUTH_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "operRate")) {
		if ( !mib_get( MIB_WLAN_SUPPORTED_RATE, (void *)&vUShort) )
			return -1;
		sprintf(buffer, "%u", vUShort);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "basicRate")) {
		if ( !mib_get( MIB_WLAN_BASIC_RATE, (void *)&vUShort) )
			return -1;
		sprintf(buffer, "%u", vUShort);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "preamble")) {
		if ( !mib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "hiddenSSID")) {
		if ( !mib_get( MIB_WLAN_HIDDEN_SSID, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wmFilterNum")) {
		if ( !mib_get( MIB_WLAN_AC_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wlanDisabled")) {
		if ( !mib_get( MIB_WLAN_DISABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wlanAcNum") ) {
		if ( !mib_get( MIB_WLAN_AC_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wlanAcEnabled")) {
		if ( !mib_get( MIB_WLAN_AC_ENABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "rateAdaptiveEnabled")) {
		if ( !mib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wlanMode")) {
		if ( !mib_get( MIB_WLAN_MODE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "networkType")) {
		if ( !mib_get( MIB_WLAN_NETWORK_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "iappDisabled")) {
		if ( !mib_get( MIB_WLAN_IAPP_DISABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#ifdef WLAN_WPA
	else if ( !strcmp(name, "encrypt")) {
		if ( !mib_get( MIB_WLAN_ENCRYPT, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "enable1X")) {
		if ( !mib_get( MIB_WLAN_ENABLE_1X, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "enableSuppNonWpa")) {
		if ( !mib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "suppNonWpa")) {
		if ( !mib_get( MIB_WLAN_SUPP_NONWPA, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wpaAuth")) {
		if ( !mib_get( MIB_WLAN_WPA_AUTH, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wpaCipher")) {
		if ( !mib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "pskFormat")) {
		if ( !mib_get( MIB_WLAN_WPA_PSK_FORMAT, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "enableMacAuth")) {
		if ( !mib_get( MIB_WLAN_ENABLE_MAC_AUTH, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "rsRetry") ) {
		if ( !mib_get( MIB_WLAN_RS_RETRY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
#endif

#ifdef WLAN_WDS
	else if ( !strcmp(name, "wlanWdsEnabled")) {
		if ( !mib_get( MIB_WLAN_WDS_ENABLED, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wlanWdsNum")) {
		if ( !mib_get( MIB_WLAN_WDS_NUM, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wdsWep")) {
		if ( !mib_get( MIB_WLAN_WDS_WEP, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wdsDefaultKeyId")) {
		if ( !mib_get( MIB_WLAN_WDS_WEP_DEFAULT_KEY, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", ++vChar);
		ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "wdsKeyType") ) {
		if ( !mib_get( MIB_WLAN_WDS_WEP_KEY_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		 ejSetResult(eid, buffer);
		return 0;
	}
#endif

#ifdef WLAN_8185AG
	else if ( !strcmp(name, "RFType") ) {
		if ( !mib_get( MIB_HW_RF_TYPE, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", vChar) ;
		 ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "band") ) {
		if ( !mib_get( MIB_WLAN_BAND, (void *)&vChar) )
			return -1;
		sprintf(buffer, "%u", (int)vChar) ;
		 ejSetResult(eid, buffer);
		return 0;
	}
	else if ( !strcmp(name, "fixTxRate") ) {
		if ( !mib_get( MIB_WLAN_FIX_RATE, (void *)&vUShort) )
			return -1;
		sprintf(buffer, "%u", vUShort) ;
		 ejSetResult(eid, buffer);
		return 0;
	}
#endif

#endif // of WLAN_SUPPORT

	sprintf(buffer, "%d", -1);
	ejSetResult(eid, buffer);
	return 0;

//   	return -1;
}
#endif

int isConnectPPP(void)
{
	return 0;
}

int getNameServer(int eid, request* wp, int argc, char **argv) {

	FILE *fp;
	char buffer[128], tmpbuf[64];
	int count = 0;
	//fprintf(stderr, "getNameServer %x\n", gResolvFile);
	//boaWrite(wp, "[]", tmpbuf);
	//if ((gResolvFile == NULL) ||
	if ( (fp = fopen("/var/resolv.conf", "r")) == NULL ) {
		//printf("Unable to open resolver file\n");
		return -1;
	}

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		if (sscanf(buffer, "nameserver %s", tmpbuf) != 1) {
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

int getDefaultGWMask(int eid, request* wp, int argc, char **argv)
{
	char buff[256];
	int flags, ret = -1;
	struct in_addr gw, dest, mask, inAddr;
	char ifname[16], dgw[16];
	FILE *fp;

	if (!(fp = fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return ret;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (sscanf
		    (buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flags,
		     &mask) != 5) {
			printf("Unsuported kernel route format\n");
			ret = -1;
			break;
		}
		//printf("ifname=%s, dest=%x, gw=%x, flags=%x, mask=%x\n", ifname, dest.s_addr, gw.s_addr, flags, mask.s_addr);
		if (flags & RTF_UP) {
			// default gateway
			if (getInAddr(ifname, IP_ADDR, &inAddr) == 1) {
				if (inAddr.s_addr == 0x40404040) {
					boaWrite(wp, "");
					ret = 0;
					break;
				}
			}

			if (getInAddr(ifname, SUBNET_MASK, &inAddr)) {
				boaWrite(wp, "%s", inet_ntoa(inAddr));
				ret = 0;
				break;
			}
		}
	}

	fclose(fp);
	return ret;
}

// Jenny, get default gateway information
int getDefaultGW(int eid, request* wp, int argc, char **argv)
{
	char buff[256];
	int flags, ret = -1;
	struct in_addr gw, dest, mask, inAddr;
	char ifname[MAX_WAN_NAME_LEN], dgw[16], vc_ifname[16], total_entry, i;
	FILE *fp;
	MIB_CE_ATM_VC_T entry;

	if (!(fp=fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return ret;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (sscanf(buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flags, &mask) != 5) {
			printf("Unsuported kernel route format\n");
			ret = -1;
			break;
		}

		//printf("ifname=%s, dest=%x, gw=%x, flags=%x, mask=%x\n", ifname, dest.s_addr, gw.s_addr, flags, mask.s_addr);
		if(flags & RTF_UP) {
			// default gateway
			if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1) {
				if (inAddr.s_addr == 0x40404040) {
					boaWrite(wp, "");
					ret = 0;
					break;
				}
			}
			if (dest.s_addr == 0 && mask.s_addr == 0) {
				if (gw.s_addr != 0) {
					strncpy(dgw,  inet_ntoa(gw), 16);
					boaWrite(wp, "%s", dgw);
					ret = 0;
					break;
				}
				else
				{
					total_entry = mib_chain_total(MIB_ATM_VC_TBL);

					for( i = 0; i < total_entry; i++ )
					{
						if(!mib_chain_get(MIB_ATM_VC_TBL, i, &entry))
							continue;

						if (entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA)
							snprintf(vc_ifname, 6, "ppp%u", PPP_INDEX(entry.ifIndex));
						else
							strcpy(vc_ifname, "aabbcc");
						if(entry.dgw != 1)
							continue;
						if(!strcmp(vc_ifname, ifname))
						{
							//getWanName(&entry, ifname);
							break;
						}
					}
					if (getInAddr(ifname, DST_IP_ADDR, &inAddr)) {
						boaWrite(wp, "%s", inet_ntoa(inAddr));
						ret = 0;
						break;
					}

					//boaWrite(wp, "%s", ifname);
					//ret = 0;
					//break;
				}
			}
		}
	}

	fclose(fp);
	return ret;
}

#ifdef CONFIG_IPV6
int getDefaultGW_ipv6(int eid, request* wp, int argc, char **argv)
{
	char buff[256];
	struct in6_addr addr, zero_ip = {0};
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

			if( len == 0 && (strcmp(devname, "lo") !=0) && (memcmp(&zero_ip, &addr, sizeof(struct in6_addr)) != 0)) {
				
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

