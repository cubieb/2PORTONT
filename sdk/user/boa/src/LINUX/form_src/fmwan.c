/*
 *      Web server handler routines for ATM
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

/* for ioctl */
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include "../defs.h"
#include "multilang.h"
#ifdef CONFIG_RTK_RG_INIT
#include "rtusr_rg_api.h"
#endif
//#define MAX_POE_PER_VC		2	// Jenny, move definition to utiltiy.h
static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
static const char IF_DISABLED[] = "Disabled";
static const char IF_ENABLE[] = "Enabled";
static const char IF_ON[] = "On";
static const char IF_OFF[] = "Off";

typedef enum {
	CONN_DISABLED=0,
	CONN_NOT_EXIST,
	CONN_DOWN,
	CONN_UP
} CONN_T;

#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
//  add for auto-pvc-search
#define MAX_PVC_SEARCH_PAIRS 16

// for auto-pvc-search
//int update_auto_pvc_disable(unsigned char enabled)
static void update_auto_pvc_disable(unsigned int enabled)
{
	MIB_T tmp_buffer;
	unsigned char vChar=0;
	if(enabled)	vChar = 1;
	mib_set( MIB_ATM_VC_AUTOSEARCH, (void *)&vChar);
}


static int autopvc_is_up(void)
{
  	MIB_T tmp_buffer;
 	unsigned char vChar;
  	int value=0;

	mib_get( MIB_ATM_VC_AUTOSEARCH, (void *)&vChar);
//	    printf("autopvc_is_up: VC_AUTOSEARCH %x\n", vChar);
	if (vChar==1)
		value=1;
	return value;
}

// action : 0=> delete PVC in auto-pvc-search tbl
//		  1=> add a pair of PVC in
static int add_auto_pvc_search_pair(request * wp, int action)
{
	char *strValue;
	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	MIB_AUTO_PVC_SEARCH_T entry;

	unsigned int VPI,VCI,entryNum,i;
	char tmpBuf[100];

	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	if(entryNum > MAX_PVC_SEARCH_PAIRS ) {
		strcpy(tmpBuf, multilang(LANG_AUTO_PVC_SEARCH_TABLE_WAS_FULL));
		goto setErr_filter;
	}

	memset(&entry, 0x00, sizeof(entry));
	/*
	strValue = boaGetVar(wp, "autopvcvpi", "");
	if(!strValue[0]) {
		printf("add_auto_pvc_search: strValue(\"autopvcvpi\") = %s\n", strValue);
	}
	*/
	strValue = boaGetVar(wp, "addVPI", "");
	if (!strValue[0]) {
		strcpy(tmpBuf, multilang(LANG_ENTER_VPI_0_255));
		goto setErr_filter;
	}
	//printf("add_auto_pvc_search: strValue(VPI) = %s\n", strValue);
	sscanf(strValue,"%u",&VPI);
	if(VPI > 255 ) {
		strcpy(tmpBuf, multilang(LANG_INVALID_VCI));
		goto setErr_filter;
	}

	strValue = boaGetVar(wp, "addVCI", "");

	if (!strValue[0]) {
		strcpy(tmpBuf, multilang(LANG_ENTER_VCI_0_65535));
		goto setErr_filter;
	}
	//printf("add_auto_pvc_search: strValue(VCI) = %s\n", strValue);
	sscanf(strValue,"%u",&VCI);
	if ( VCI>65535) {
		strcpy(tmpBuf, multilang(LANG_INVALID_VCI));
		goto setErr_filter;
	}

	if(VCI ==0 && VPI == 0) {
		strcpy(tmpBuf, multilang(LANG_INVALID_VPI_AND_VCI_00));
		goto setErr_filter;
	}

	for(i=0; i< entryNum; i++)
	{
		if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
			continue;
		if(Entry.vpi == VPI && Entry.vci == VCI) {
			if(action)
				return 0;
			else
				break;
		}
	}

	entry.vpi = VPI;	entry.vci = VCI;

	if(action) {
		printf("add_auto_pvc_search : adding (%d %d) PVC into search table\n", VPI,VCI);
		int intVal = mib_chain_add(MIB_AUTO_PVC_SEARCH_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_filter;
		}
	} else {
		printf("add_auto_pvc_search : deleting (%d %d) PVC from search table\n", VPI,VCI);
		if(mib_chain_delete(MIB_AUTO_PVC_SEARCH_TBL, i) != 1){
			strcpy(tmpBuf, Tdelete_chain_error);
			goto setErr_filter;
		}
	}

//	mib_update(CURRENT_SETTING);
	return 0;

setErr_filter:
	ERR_MSG(tmpBuf);
	return -1;

}
#endif

#ifdef CONFIG_IPV6
static int retrieveIPv6Record(request * wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char *strValue;
	struct in6_addr ip6Addr;
	char ipAddr[20];

#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
	if( pEntry->cmode == CHANNEL_MODE_6RD ) {  // 6rd

		pEntry->AddrMode |= 8;
		pEntry->IpProtocol = IPVER_IPV4_IPV6; 	   // IPv4/IPv6
		// Get parameter for 6rd
		//Board Router IPv4 address
		strValue = boaGetVar(wp, "SixrdBRv4IP", "");
		if (strValue[0]) {
			inet_aton(strValue, (struct in_addr *)pEntry->SixrdBRv4IP);
		}

		//6rd IPv4 mask len
		strValue = boaGetVar(wp, "SixrdIPv4MaskLen", "");
		if(strValue[0]) {
			pEntry->SixrdIPv4MaskLen = atoi( strValue);
		}

		//6rd prefix
		strValue = boaGetVar(wp, "SixrdPrefix", "");
		if(strValue[0]) {
			memcpy(pEntry->SixrdPrefix, strValue, sizeof(pEntry->SixrdPrefix));
		}

		//6rd prefix len
		strValue = boaGetVar(wp, "SixrdPrefixLen", "");
		if(strValue[0]) {
			pEntry->SixrdPrefixLen = atoi( strValue);
		}
		return 0;
	}
#endif

	// IpProtocolType(ipv4, ipv6, ipv4/ipv6)
	strValue = boaGetVar(wp, "IpProtocolType", "");
	if (strValue[0]) {
		pEntry->IpProtocol = strValue[0] - '0';
	}

	// Use Slaac
	strValue = boaGetVar(wp, "slacc", "");
	if ( !gstrcmp(strValue, "ON"))
		pEntry->AddrMode |= 1;

	// Use static IP
	strValue = boaGetVar(wp, "staticIpv6", "");
	if ( !gstrcmp(strValue, "ON"))
		pEntry->AddrMode |= 2;

#ifdef DUAL_STACK_LITE
	if( pEntry->cmode == CHANNEL_MODE_DSLITE ) {  // DS-Lite
		pEntry->AddrMode |= 4;
		pEntry->cmode = CHANNEL_MODE_IPOE; // DS-Lite only for MER
		pEntry->IpProtocol = IPVER_IPV6; 	// IPv6 only
	}
#endif

	// Local IPv6 IP
	strValue = boaGetVar(wp, "Ipv6Addr", "");
	if(strValue[0]) {
		inet_pton(PF_INET6, strValue, &ip6Addr);
		memcpy(pEntry->Ipv6Addr, &ip6Addr, sizeof(pEntry->Ipv6Addr));
	}

	// Local Prefix length of IPv6's IP
	strValue = boaGetVar(wp, "Ipv6PrefixLen", "");
	if(strValue[0]) {
		pEntry->Ipv6AddrPrefixLen = (char)atoi(strValue);
	}

	// Remote IPv6 IP
	strValue = boaGetVar(wp, "Ipv6Gateway", "");
	if(strValue[0]) {
		inet_pton(PF_INET6, strValue, &ip6Addr);
		memcpy(pEntry->RemoteIpv6Addr, &ip6Addr, sizeof(pEntry->RemoteIpv6Addr));
	}

	//DNS servers
	strValue = boaGetVar(wp, "Ipv6Dns1", "");
	if(strValue[0]) {
		inet_pton(PF_INET6, strValue, &ip6Addr);
		memcpy(pEntry->Ipv6Dns1, &ip6Addr, sizeof(pEntry->Ipv6Dns1));
	}

	strValue = boaGetVar(wp, "Ipv6Dns2", "");
	if(strValue[0]) {
		inet_pton(PF_INET6, strValue, &ip6Addr);
		memcpy(pEntry->Ipv6Dns2, &ip6Addr, sizeof(pEntry->Ipv6Dns2));
	}

	// Enable DHCPv6 client
	strValue = boaGetVar(wp, "itfenable", "");
	if ( !gstrcmp(strValue, "ON"))
		pEntry->Ipv6Dhcp = 1;
	else
		pEntry->Ipv6Dhcp = 0;

	// Request Address
	strValue = boaGetVar(wp, "iana", "");
	if ( !gstrcmp(strValue, "ON"))
		pEntry->Ipv6DhcpRequest |= 1;

	// Request Prefix
	strValue = boaGetVar(wp, "iapd", "");
	if ( !gstrcmp(strValue, "ON"))
		pEntry->Ipv6DhcpRequest |= 2;

#ifdef DUAL_STACK_LITE
	// Get parameter for DS-Lite
	if ((pEntry->AddrMode & 0x4) == 0x4)
	{
		// DSLiteLocalIP
		strValue = boaGetVar(wp, "DSLiteLocalIP", "");
		if(strValue[0]) {
			inet_pton(PF_INET6, strValue, &ip6Addr);
			memcpy(pEntry->Ipv6Addr, &ip6Addr, sizeof(pEntry->Ipv6Addr));
		}

		// DSLiteGateway
		strValue = boaGetVar(wp, "DSLiteGateway", "");
		if(strValue[0]) {
			inet_pton(PF_INET6, strValue, &ip6Addr);
			memcpy(pEntry->RemoteIpv6Addr, &ip6Addr, sizeof(pEntry->RemoteIpv6Addr));
		}

		// DSLiteRemoteIP
		strValue = boaGetVar(wp, "DSLiteRemoteIP", "");
		if(strValue[0]) {
			inet_pton(PF_INET6, strValue, &ip6Addr);
			memcpy(pEntry->RemoteIpv6EndPointAddr, &ip6Addr, sizeof(pEntry->RemoteIpv6EndPointAddr));
		}
	}
#endif

	return 0;
}
#endif

/// Mason Yu. enable_802_1p_090722
#if defined(ENABLE_802_1Q) || defined(CONFIG_RTL_MULTI_PVC_WAN)
static int fm1q(request * wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char *strValue;

	// VLAN
	strValue = boaGetVar(wp, "vlan", "");
	pEntry->vlan = strValue[0] - '0';

	strValue = boaGetVar(wp, "vid", "");
	pEntry->vid = atoi(strValue);
	if (pEntry->vid<=0 || pEntry->vid>=4096)
		pEntry->vid=0;


	strValue = boaGetVar(wp, "vprio", "");
	pEntry->vprio = strValue[0]-'0';
/*
	strValue = boaGetVar(wp, "vpass", "");
	if (strValue[0]=='1')
		pEntry->vpass = 1;
	else
		pEntry->vpass = 0;
*/
}
#endif

#ifdef PPPOE_PASSTHROUGH
/*
 * Deal with Ethernet WAN Brige Mode
 */
static void retrieveBrMode(request * wp, MIB_CE_ATM_VC_Tp pEntry)
{
	unsigned char brmode = BRIDGE_DISABLE;
	char *str_br = NULL;
	char *str_brmode = NULL;

	str_br = boaGetVar(wp, "br", "OFF");
	str_brmode = boaGetVar(wp, "brmode", "2");
	brmode = str_brmode[0] - '0';

	if(brmode == BRIDGE_ETHERNET || brmode == BRIDGE_PPPOE)
	{
		if(pEntry->cmode == CHANNEL_MODE_BRIDGE || strcmp(str_br, "ON") == 0)
			pEntry->brmode = brmode;
		else
			pEntry->brmode == BRIDGE_DISABLE;
	}
	else
	{
		// invalid, assige default value
		if(pEntry->cmode == CHANNEL_MODE_BRIDGE)
			pEntry->brmode = BRIDGE_ETHERNET;
		else
			pEntry->brmode = BRIDGE_DISABLE;
	}
}
#endif

#ifdef CONFIG_USER_DHCP_OPT_GUI_60
static int retrieveDHCPOpts(request * wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char *strValue;
	char tmpBuf[256] = {0};

	/* Get DHCP client option 60 settings */
	strValue = boaGetVar(wp, "enable_opt_60", "");
	if( strcmp(strValue, "1") == 0)
		pEntry->enable_opt_60 = 1;
	else
		pEntry->enable_opt_60 = 0;

	if(pEntry->enable_opt_60)
	{
		strValue = boaGetVar(wp, "opt60_val", "");
		strcpy(pEntry->opt60_val, strValue);
	}

	/* Get DHCP client option 61 settings */
	strValue = boaGetVar(wp, "enable_opt_61", "");
	if( strcmp(strValue, "1") == 0)
		pEntry->enable_opt_61 = 1;
	else
		pEntry->enable_opt_61 = 0;

	if(pEntry->enable_opt_61)
	{
		strValue = boaGetVar(wp, "iaid", "0");
		sscanf(strValue, "%u", &pEntry->iaid);

		strValue = boaGetVar(wp, "duid_type", "1");
		sscanf(strValue, "%hhu", &pEntry->duid_type);

		if(pEntry->duid_type == 2)	/* enterprise number + identifier */
		{
			strValue = boaGetVar(wp, "duid_ent_num", "");
			if(sscanf(strValue, "%u", &pEntry->duid_ent_num) != 1)
			{
				strcpy(tmpBuf, multilang(LANG_INVALID_ENTERPRISE_NUMBER));
				goto setErr_filter;
			}

			strValue = boaGetVar(wp, "duid_id", "");
			strcpy(pEntry->duid_id, strValue);
		}
	}

	/* Get DHCP client option 125 settings */
	strValue = boaGetVar(wp, "enable_opt_125", "");
	if( strcmp(strValue, "1") == 0)
		pEntry->enable_opt_125 = 1;
	else
		pEntry->enable_opt_125 = 0;

	if(pEntry->enable_opt_125)
	{
		strValue = boaGetVar(wp, "manufacturer", "");
		strcpy(pEntry->manufacturer, strValue);

		strValue = boaGetVar(wp, "product_class", "");
		strcpy(pEntry->product_class, strValue);

		strValue = boaGetVar(wp, "model_name", "");
		strcpy(pEntry->model_name, strValue);

		strValue = boaGetVar(wp, "serial_num", "");
		strcpy(pEntry->serial_num, strValue);
	}

	return 0;

setErr_filter:
	ERR_MSG(tmpBuf);
	return -1;
}
#endif

static unsigned char base64chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz0123456789+/=";

static int base64charsIndex(unsigned char c){
	int i=0;
	while(i<65){
		if(base64chars[i]==c)
			return i;
		i++;
	}
}

static void data_base64decode(unsigned char *input, unsigned char *output)
{
	unsigned char chr1, chr2, chr3;
	unsigned char enc1, enc2, enc3, enc4;
	int i=0, j=0;

	for (i = 0; i < strlen(input) - 3; i += 4) {
		enc1 = base64charsIndex(input[i+0]);
		enc2 = base64charsIndex(input[i+1]);
		enc3 = base64charsIndex(input[i+2]);
		enc4 = base64charsIndex(input[i+3]);

		output[j++] = ((enc1 << 2) | (enc2 >> 4));
		if (input[i+2] != base64chars[64])
			output[j++] = (((enc2 << 4) & 0xF0) | ((enc3 >> 2) & 0x0F));
		if (input[i+3] != base64chars[64])
			output [j++] = (((enc3 << 6) & 0xC0) | enc4);
	}
}

static void  data_base64encode(unsigned char *input, unsigned char *output)
{
	unsigned char chr1, chr2, chr3;
	unsigned char enc1, enc2, enc3, enc4;
	int i=0, j=0, len=strlen(input);


	for (i = 0; i <= len - 3; i += 3)
	{
		output[j++] = base64chars[(input[i] >> 2)];
		output[j++] = base64chars[((input[i] & 3) << 4) | (input[i+1] >> 4)];
		output[j++] = base64chars[((input[i+1] & 15) << 2) | (input[i+2] >> 6)];
		output[j++] = base64chars[input[i+2] & 63];
	}

	if (len % 3 == 2)
	{
		output[j++] = base64chars[(input[i] >> 2)];
		output[j++] = base64chars[((input[i] & 3) << 4) | (input[i+1] >> 4)];
		output[j++] = base64chars[((input[i+1] & 15) << 2)];
		output[j++] = base64chars[64];
	}
	else if (len % 3 == 1)
	{
		output[j++] = base64chars[(input[i] >> 2)];
		output[j++] = base64chars[((input[i] & 3) << 4)];
		output[j++] = base64chars[64];
		output[j++] = base64chars[64];
	}
}


// retrieve VC record from web form
// reture value:
// 0  : success
// -1 : failed
static int retrieveVcRecord(request * wp, MIB_CE_ATM_VC_Tp pEntry, MEDIA_TYPE_T mType)
{
	char tmpBuf[100];
	char *strValue, *strIp, *dns1Ip, *dns2Ip,*strMask, *strGW;
	unsigned int vUInt;
	MIB_CE_ATM_VC_T entry;

	memset(&entry, 0x00, sizeof(entry));

#ifdef CONFIG_RTL8672_SAR
	if (mType == MEDIA_ATM) {
		// VPI (0~255)
		strValue = boaGetVar(wp, "vpi", "");
		if (!strValue[0]) {
			strcpy(tmpBuf, multilang(LANG_ENTER_VPI_0_255));
			goto setErr_filter;
		}

		sscanf(strValue,"%u",&vUInt);
		if ( vUInt>255) {
			strcpy(tmpBuf, multilang(LANG_INVALID_VPI));
			goto setErr_filter;
		}
		entry.vpi = (unsigned char)vUInt;

		// VCI (0~65535)
		strValue = boaGetVar(wp, "vci", "");

		if (!strValue[0]) {
			strcpy(tmpBuf, multilang(LANG_ENTER_VCI_0_65535));
			goto setErr_filter;
		}

		sscanf(strValue,"%u",&vUInt);
		if ( vUInt>65535) {
			strcpy(tmpBuf, multilang(LANG_INVALID_VCI));
			goto setErr_filter;
		}


		entry.vci = (unsigned short)vUInt;
		// set default Qos
		entry.qos = 0;
		entry.pcr = ATM_MAX_US_PCR;

		// Encapsulation
		strValue = boaGetVar(wp, "adslEncap", "");
		if (strValue[0]) {
			entry.encap = strValue[0] - '0';
		}
	}
#endif// CONFIG_RTL8672_SAR

#ifdef CONFIG_USER_RTK_WAN_CTYPE
	// applicationtype
	strValue = boaGetVar(wp, "ctype", "");
	if (strValue[0]) {
		//entry.applicationtype = strValue[0] - '0';
		sscanf(strValue,"%u",&vUInt);
		entry.applicationtype = vUInt;
	}
#endif

	// Enable NAPT
	strValue = boaGetVar(wp, "naptEnabled", "");
	if ( !gstrcmp(strValue, "ON"))
		entry.napt = 1;
	else
		entry.napt = 0;
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	// Enable IGMP
	strValue = boaGetVar(wp, "igmpEnabled", "");
	if ( !gstrcmp(strValue, "ON"))
		entry.enableIGMP = 1;
	else
		entry.enableIGMP = 0;
#endif
#ifdef CONFIG_USER_IP_QOS
	// Enable QoS
	strValue = boaGetVar(wp, "qosEnabled", "");
	if ( !gstrcmp(strValue, "ON"))
		entry.enableIpQos = 1;
	else
		entry.enableIpQos = 0;
#endif
// Mason Yu. don't care enableIpQos
//#elif defined(CONFIG_USER_IP_QOS)
//	entry.enableIpQos = 1;
//#endif

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	if( (mType == MEDIA_ETH)
		#ifdef CONFIG_PTMWAN
		|| (mType == MEDIA_PTM)
		#endif /*CONFIG_PTMWAN*/
	)
	{
		strValue = boaGetVar(wp, "vlan", "");
		if ( !gstrcmp(strValue, "ON"))
			entry.vlan = 1;
		else
			entry.vlan = 0;

		if (entry.vlan == 1)
		{
			strValue = boaGetVar(wp, "vid", "");
			entry.vid = (unsigned int) strtol(strValue, (char**)NULL, 10);
			strValue = boaGetVar(wp, "vprio", "");
			if (strValue[0]) {
				entry.vprio = strValue[0] - '0';
			}
		}
	}
#endif
	// Enabled
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	strValue = boaGetVar(wp, "chEnable", "");
	if (strValue[0]) {
		entry.enable = strValue[0] - '0';
	}
#else
	if (mType == MEDIA_ATM) {
		strValue = boaGetVar(wp, "chEnable", "");
		if (strValue[0]) {
			entry.enable = strValue[0] - '0';
		}
	}
	else
		entry.enable = 1;
#endif

	// Connection mode
	strValue = boaGetVar(wp, "adslConnectionMode", "");
	if (strValue[0]) {
		entry.cmode = strValue[0] - '0';
	}
	strValue=boaGetVar(wp,"mtu","");
	if(strValue[0]){
		 entry.mtu=(unsigned int) strtol(strValue, (char**)NULL, 10);
	}
#ifdef DEFAULT_GATEWAY_V1
	// Default Route
	strValue = boaGetVar(wp, "droute", "");
	if (strValue[0]) {
		entry.dgw = strValue[0] - '0';
	}

#ifdef CONFIG_USER_RTK_WAN_CTYPE
	entry.dgw = 0;
	//entry.enableIGMP = 0;
	// if 'INTERNET', set as default route.
#ifndef CONFIG_RTK_RG_INIT
	if (entry.applicationtype& X_CT_SRV_INTERNET)
		entry.dgw = 1;
#endif
	// If connection type is other(IPTV), and mode is not bridge mode, enable IGMP proxy
	//if ((entry.applicationtype& X_CT_SRV_OTHER) && (entry.cmode != CHANNEL_MODE_BRIDGE))
	//	entry.enableIGMP = 1;
#endif
#else
{
	char *strIp, *strIf;
	unsigned char vChar, strIfc;
	unsigned int dgw;
	struct in_addr inGatewayIp;

	strValue = boaGetVar(wp, "droute", "");
	if (strValue[0])
		vChar = strValue[0] - '0';

	if (vChar == 0) {
		strValue = boaGetVar(wp, "gwStr", "");
		if (strValue[0]) {
			strIfc = strValue[0] - '0';
			if (strIfc == 1) {
				char ifname[16];
				strIf = boaGetVar(wp, "wanIf", "");
				dgw = (unsigned int)atoi(strIf);
			}
			else if (strIfc == 0) {
				dgw = DGW_NONE;
				strIp = boaGetVar(wp, "dstGtwy", "");
				if (strIp[0]) {
					if (!inet_aton(strIp, &inGatewayIp)) {
						strcpy(tmpBuf, strInvalidGatewayerror);
						goto setErr_filter;
					}
					if (entry.cmode == CHANNEL_MODE_IPOE || (entry.cmode == CHANNEL_MODE_RT1483 && entry.ipunnumbered == 0)) {
						if (!inet_aton(strIp, (struct in_addr *)&entry.remoteIpAddr)) {
							strcpy(tmpBuf, strIPAddresserror);
							goto setErr_filter;
						}
					}
					if (!mib_set(MIB_ADSL_WAN_DGW_IP, (void *)&inGatewayIp)) {
						strcpy(tmpBuf, strSetGatewayerror);
						goto setErr_filter;
					}
				}
			}
		}
	}
	else if (vChar == 1)
		dgw = DGW_AUTO;

	if (!mib_set(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw)) {
		strcpy(tmpBuf, strSetGatewayerror);
		goto setErr_filter;
	}
}
#endif
#ifdef CONFIG_IPV6
		strValue = boaGetVar(wp, "IpProtocolType", "");
		if (strValue[0]) {
			entry.IpProtocol = strValue[0] - '0';
		}
#endif

	entry.brmode = BRIDGE_DISABLE;

#ifdef PPPOE_PASSTHROUGH
	// 1483 bridged
	if (entry.cmode == CHANNEL_MODE_BRIDGE)
	{
		entry.brmode = 0;
	}
	else // PPP connection
#endif
	if (entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA)
	{
		// PPP user name
		strValue = boaGetVar(wp, "encodePppUserName", "");
		if ( strValue[0] ) {
			data_base64decode(strValue, entry.pppUsername);
			if ( strlen(entry.pppUsername) > MAX_PPP_NAME_LEN ) {
				strcpy(tmpBuf, strUserNametoolong);
				goto setErr_filter;
			}
			//strncpy(entry.pppUsername, strValue, MAX_PPP_NAME_LEN);
			entry.pppUsername[MAX_PPP_NAME_LEN]='\0';
		}

		// PPP password
		strValue = boaGetVar(wp, "encodePppPassword", "");

		if ( strValue[0] ) {
			data_base64decode(strValue, entry.pppPassword);
			if ( strlen(entry.pppPassword) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, strPasstoolong);
				goto setErr_filter;
			}
			//strncpy(entry.pppPassword, strValue, MAX_NAME_LEN-1);
			entry.pppPassword[MAX_NAME_LEN-1]='\0';
			//entry.pppPassword[MAX_NAME_LEN]='\0';
		}
		else{
			strValue = boaGetVar(wp, "pppPassword", "");
			if ( strValue[0] ) {
				if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
					strcpy(tmpBuf, strPasstoolong);
					goto setErr_filter;
				}
				strncpy(entry.pppPassword, strValue, MAX_NAME_LEN-1);
				entry.pppPassword[MAX_NAME_LEN-1]='\0';
				//entry.pppPassword[MAX_NAME_LEN]='\0';
			}
		}

		// PPP connection type
		strValue = boaGetVar(wp, "pppConnectType", "");

		if ( strValue[0] ) {
			PPP_CONNECT_TYPE_T type;

			if ( strValue[0] == '0' )
				type = CONTINUOUS;
			else if ( strValue[0] == '1' )
				type = CONNECT_ON_DEMAND;
			else if ( strValue[0] == '2' )
				type = MANUAL;
			else {
				strcpy(tmpBuf, strInvalPPPType);
				goto setErr_filter;
			}

			entry.pppCtype = (unsigned char)type;

			if (type != CONTINUOUS) {
				// PPP idle time
				strValue = boaGetVar(wp, "pppIdleTime", "");
				if ( strValue[0] ) {
					unsigned short time;
					time = (unsigned short) strtol(strValue, (char**)NULL, 10);
					entry.pppIdleTime = time;
				}
			}
		}

		// PPP authentication method
		strValue = boaGetVar(wp, "auth", "");
		if ( strValue[0] ) {
			entry.pppAuth = strValue[0] - '0';
		}
		// PPP AC-Name
		strValue = boaGetVar(wp, "acName", "");
		if ( strValue[0] ) {
			strncpy(entry.pppACName, strValue, MAX_NAME_LEN-1);
		}
		// PPP Service-Name
		strValue = boaGetVar(wp, "serviceName", "");
		if ( strValue[0] ) {
			strncpy(entry.pppServiceName, strValue, MAX_NAME_LEN-1);
		}
#ifdef CONFIG_USER_PPPOE_PROXY
		//pppoe_proxy
		strValue = boaGetVar(wp, "enableProxy", "");
		if(strValue[0] == '1')
			entry.PPPoEProxyEnable = 1;

		if(entry.PPPoEProxyEnable == 1){
			strValue = boaGetVar(wp, "maxProxyUser", "");
			if ( strValue[0] ) {
				unsigned short maxuser;
				maxuser = (unsigned short) strtol(strValue, (char**)NULL, 10);
				entry.PPPoEProxyMaxUser = maxuser;
			}
		}
		printf("Get from web: enable %d, maxuser %d\n",entry.PPPoEProxyEnable, entry.PPPoEProxyMaxUser);
#endif

	}
	else // Wan IP setting
	{
#ifdef CONFIG_IPV6
		if (entry.IpProtocol & IPVER_IPV4) {
#endif
		// Jenny, IP unnumbered

		strValue = boaGetVar(wp, "ipUnnum", "");
		if (!gstrcmp(strValue, "ON"))
			entry.ipunnumbered = 1;
		else
			entry.ipunnumbered = 0;

		// IP mode
		strValue = boaGetVar(wp, "ipMode", "");
		if (strValue[0]) {

			if(strValue[0] == '0')
				entry.ipDhcp = (char) DHCP_DISABLED;
			else if(strValue[0] == '1')
			{
				entry.ipDhcp = (char) DHCP_CLIENT;
#ifdef CONFIG_USER_DHCP_OPT_GUI_60
				// Only support PTM & Ethernet WAN currently.
				if(mType == MEDIA_PTM || mType == MEDIA_ETH)
				{
					if(retrieveDHCPOpts(wp, &entry) == -1)
						return -1;	/*Error message has been output. */
				}
#endif
			}
			else {
				strcpy(tmpBuf, strInvalDHCP);
				goto setErr_filter;
			}
		}

		// Local IP address
		strIp = boaGetVar(wp, "ip", "");
		if (strIp[0]) {
			if (!inet_aton(strIp, (struct in_addr *)&entry.ipAddr)) {
				strcpy(tmpBuf, strInvalIP);
				goto setErr_filter;
			}
		}

		// Remote IP address
		strGW = boaGetVar(wp, "remoteIp", "");
		if (strGW[0]) {
			if (!inet_aton(strGW, (struct in_addr *)&entry.remoteIpAddr)) {
				strcpy(tmpBuf, strInvalGateway);
				goto setErr_filter;
			}
		}

		// Subnet Mask, added by Jenny
		strMask = boaGetVar(wp, "netmask", "");
		if (strMask[0]) {
			if (!isValidNetmask(strMask, 1)) {
				strcpy(tmpBuf, strInvalMask);
				goto setErr_filter;
			}
			if (!inet_aton(strMask, (struct in_addr *)&entry.netMask)) {
				strcpy(tmpBuf, strInvalMask);
				goto setErr_filter;
			}
			if (!isValidHostID(strIp, strMask)) {
				strcpy(tmpBuf, multilang(LANG_INVALID_IP_SUBNET_MASK_COMBINATION));
				goto setErr_filter;
			}
		}

		strValue = boaGetVar(wp, "dnsMode", "");
		if (strValue[0]) {
			entry.dnsMode = strValue[0] - '0';
		}

		dns1Ip = boaGetVar(wp, "dns1", "");
		if (dns1Ip[0]) {
			if (!inet_aton(dns1Ip, (struct in_addr *)&entry.v4dns1)) {
				strcpy(tmpBuf, multilang(LANG_INVALID_DNSV4_1_IP_ADDRESS_VALUE));
				goto setErr_filter;
			}
		}

		dns2Ip = boaGetVar(wp, "dns2", "");
		if (dns2Ip[0]) {
			if (!inet_aton(dns2Ip, (struct in_addr *)&entry.v4dns2)) {
				strcpy(tmpBuf,  multilang(LANG_INVALID_DNSV4_2_IP_ADDRESS_VALUE));
				goto setErr_filter;
			}
		}

#ifdef DEFAULT_GATEWAY_V1
		//if (!isSameSubnet(strIp, strGW, strMask)) {
		if (entry.cmode == CHANNEL_MODE_IPOE && !isSameSubnet(strIp, strGW, strMask)) {
			strcpy(tmpBuf, multilang(LANG_INVALID_IP_ADDRESS_IT_SHOULD_BE_LOCATED_IN_THE_SAME_SUBNET));
			goto setErr_filter;
		}
#endif
#ifdef CONFIG_IPV6
		}
#endif
	}

#ifdef CONFIG_IPV6
	if (entry.cmode != CHANNEL_MODE_BRIDGE) {
		if (entry.IpProtocol & IPVER_IPV6)
			retrieveIPv6Record(wp, &entry);
	}
#endif

#ifdef NEW_PORTMAPPING
	strValue = boaGetVar(wp, "itfGroup", "");
	entry.itfGroup = atoi(strValue);

	AUG_PRT("itfGroup : 0x%x\n", entry.itfGroup);

#endif

#ifdef PPPOE_PASSTHROUGH
	if(mType == MEDIA_ETH || mType == MEDIA_PTM)
		retrieveBrMode(wp, &entry);
#endif

#if defined(CONFIG_USER_RTK_WAN_CTYPE) && defined(CONFIG_RTK_RG_INIT)
/*set up default route in RG_check_Droute*/
		entry.dgw = 0;
#endif

	memcpy(pEntry, &entry, sizeof(entry));
	return 0;

setErr_filter:
	ERR_MSG(tmpBuf);
	return -1;
}

// Mason Yu. enable_802_1p_090722
#ifdef ENABLE_802_1Q
static void write1q(request * wp, MIB_CE_ATM_VC_Tp pEntry, char *formName)
{
	// vlan mapping
	boaWrite(wp,
	"<script>\nfunction click1q()\n{\n\tif (%s.%s.vlan[0].checked){"
	"\n\t\t%s.%s.vid.disabled=true;\n\t\t%s.%s.vprio.disabled=true;}\n\telse{\n\t\t%s.%s.vid.disabled=false;\n\t\t%s.%s.vprio.disabled=false;\n\t\t%s.%s.vprio.selectedIndex=%d;}"
	"\n}\n</script>\n",
	DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName,pEntry->vprio);
	boaWrite(wp,
	"<script>\nfunction check1q(str)\n{\n\tfor (var i=0; i<str.length; i++) {"
	"\n\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9'))"
	"\n\t\t\tcontinue;"
	"\n\t\treturn false;\n\t}"
	"\n\td = parseInt(str, 10);"
	"\n\tif (d < 0 || d > 4095)"
	"\n\t\treturn false;\n\treturn true;\n}\n");
	boaWrite(wp,
	"\nfunction apply1q()\n{\n\tif (!check1q(%s.%s.vid.value)) {"
	"\n\t\talert(\"Invalid VLAN ID!\");"
	"\n\t\t%s.%s.vid.focus();"
	"\n\t\treturn false;\n\t}\n\treturn true;\n}\n</script>\n",
	DOCUMENT, formName, DOCUMENT, formName);
	boaWrite(wp,
	"<tr><th align=left><b>802.1q:</b></th>\n<td>\n"
	"<input type=radio value=0 name=\"vlan\" %s onClick=click1q()>Disable&nbsp;&nbsp;\n"
	"<input type=radio value=1 name=\"vlan\" %s onClick=click1q()>Enable\n</td></tr>\n",
	pEntry->vlan? "":"checked", pEntry->vlan? "checked":"");
	boaWrite(wp,
	"<tr><th align=left></th>\n<td>\n"
	"VLAN ID(0-4095):&nbsp;&nbsp;\n"
	"<input type=text name=vid size=6 maxlength=4 value=%d %s></td></tr>\n"
	, pEntry->vid, pEntry->vlan?"":"disabled");

	boaWrite(wp, "<tr><th align=left></th>\n<td>%s: <select style=\"WIDTH: 60px\" name=\"vprio\">\n"
			"<option value=\"0\" >  </option>\n"
			"<option value=\"1\" > 0 </option>\n"
			"<option value=\"2\" > 1 </option>\n"
			"<option value=\"3\" > 2 </option>\n"
			"<option value=\"4\" > 3 </option>\n"
			"<option value=\"5\" > 4 </option>\n"
			"<option value=\"6\" > 5 </option>\n"
			"<option value=\"7\" > 6 </option>\n"
			"<option value=\"8\" > 7 </option>\n"
			"</td></tr>", multilang(LANG_802_1P_MARK));
	boaWrite(wp, "<script>\n\t\t%s.%s.vprio.selectedIndex=%d;\n</script>\n",
	DOCUMENT, formName, pEntry->vprio);
}
#endif

#ifdef PPPOE_PASSTHROUGH
// Jenny, sync PPPoE Passthrough flag for multisession PPPoE
static void syncPPPoEPassthrough(MIB_CE_ATM_VC_T entry)
{
	MIB_CE_ATM_VC_T Entry;
	unsigned int totalEntry;
	int i;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return;
		if (Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
			Entry.brmode = entry.brmode;
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
		}
	}
}
#endif

#ifdef DEFAULT_GATEWAY_V1
static int dr=0, pdgw=0;
#endif
// Jenny, PPPoE static IP option
#ifdef CONFIG_SPPPD_STATICIP
static void writeStaticIP(request * wp, MIB_CE_ATM_VC_Tp pEntry, char *formName)
{
	char ipAddr[20];

	// static PPPoE
	boaWrite(wp,
		"<script>\nfunction clickstatic()\n{\n\tif (%s.%s.pppip[0].checked)"
		"\n\t\t%s.%s.staticip.disabled=true;\n\telse\n\t\t%s.%s.staticip.disabled=false;"
		"\n}\n</script>\n",
		DOCUMENT, formName, DOCUMENT, formName, DOCUMENT, formName);
	boaWrite(wp,
		"<tr><th align=left><b>%s:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"pppip\" %s onClick=clickstatic()>%s&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"pppip\" %s onClick=clickstatic()>%s&nbsp;&nbsp;\n",
		Tip_addr, pEntry->pppIp? "":"checked", Tdynamic_ip, pEntry->pppIp? "checked":"", Tstatic_ip);
	strcpy(ipAddr, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)));
	boaWrite(wp,
		"<input type=text name=staticip size=10 maxlength=15 value=%s %s></td></tr>\n"
		, ipAddr, pEntry->pppIp?"":"disabled");
}
#endif

static void convert_to_star_string(char *star_string, int length)
{
	int i=0;
	for(i=0;i<length;i++)
		strcat(star_string, "*");
}

void writePPPEdit(request * wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="ppp";
		char *proto;
		unsigned char pppUsername[ENC_PPP_NAME_LEN+1];
		unsigned char pppPassword[MAX_NAME_LEN];

		if (pEntry->cmode == CHANNEL_MODE_PPPOE)
			proto = "PPPoE";
		else
		if (pEntry->cmode == CHANNEL_MODE_PPPOA)
			proto = "PPPoA";

#ifdef DEFAULT_GATEWAY_V1
		pdgw=pEntry->dgw;
#endif

		// Javascript
		boaWrite(wp, "<head>\n");
		boaWrite(wp, "<script type=\"text/javascript\" src=\"../../base64_code.js\"></script>\n");
		boaWrite(wp, "<script>\nfunction pppTypeSelection()\n{\n"
		"if ( document.%s.pppConnectType.selectedIndex == 2) {\n"
		"document.%s.pppIdleTime.value = \"\";\n"
		"document.%s.pppIdleTime.disabled = true;\n}\nelse {\n"
		"if (document.%s.pppConnectType.selectedIndex == 1) {\n"
		"document.%s.pppIdleTime.value = %d;\n"
		"document.%s.pppIdleTime.disabled = false;\n}\nelse {\n"
		"document.%s.pppIdleTime.value = \"\";\n"
		"document.%s.pppIdleTime.disabled = true;\n}\n}\n}\n"
		,fmName, fmName, fmName, fmName, fmName, pEntry->pppIdleTime, fmName, fmName, fmName);

		boaWrite(wp,"function getDigit(str, num)\n{"
"  i=1;  if ( num != 1 ) {"
"  	while (i!=num && str.length!=0) {"
"		if ( str.charAt(0) == '.' ) {"
"			i++;"
"		}"
"		str = str.substring(1);"
  	"}  	if ( i!=num )"
"  		return -1;"
  "}"
  "for (i=0; i<str.length; i++) {"
"  	if ( str.charAt(i) == '.' ) {"
"		str = str.substring(0, i);"
"		break;"
"	}"
  "}");
		boaWrite(wp," if ( str.length == 0)"
"  	return -1;"
"  d = parseInt(str, 10);"
"  return d;"
"}"
"function validateKey(str)"
"{   for (var i=0; i<str.length; i++) {"
"    if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||"
"    		(str.charAt(i) == '.' ) )"
"			continue;"
"	return 0;"
" }"
  "return 1;"
"}");
		boaWrite(wp,
		"\nfunction checkString(str)\n{\n\tfor (var i=0; i<str.length; i++) {"
		"\n\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '-' ) || (str.charAt(i) == '_' ) || (str.charAt(i) == '@' ) || (str.charAt(i) == ':' ) ||"
		"\n\t\t\t(str.charAt(i) == '.' ) || (str.charAt(i) == '/' ) || (str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'z'))"
		"\n\t\t\tcontinue;"
		"\n\t\treturn 0;\n\t}"
		"\n\treturn 1;\n}\n");

		boaWrite(wp,
		"\nfunction isAllStar(str)"
		"\n{"
		"\n\tfor (var i=0; i<str.length; i++) {"
  		"\n\t\tif ( str.charAt(i) != '*' ) {"
  		"\n\t\t\treturn false;"
		"\n\t\t}"
  		"\n\t}"
  		"\n\treturn true;"
		"\n}");

		boaWrite(wp,
		"\nfunction disableUsernamePassword()"
		"\n{"
		"\n\tdocument.ppp.name.disabled=true;"
		"\n\tif(!isAllStar(document.ppp.passwd.value))"
		"\n\t\tdocument.ppp.passwd.disabled=true;"
		"\n\treturn true;"
		"\n}");

		boaWrite(wp,
		"\nfunction checkDigit(str)\n{\n\tfor (var i=0; i<str.length; i++) {"
		"\n\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9'))"
		"\n\t\t\tcontinue;"
		"\n\t\treturn false;\n\t}"
		"\n\treturn true;\n}\n");
		boaWrite(wp,
		"\nfunction applyPPP()\n{"
		"if (document.%s.pppConnectType.selectedIndex == 1) {"
		"\n\t\tif ((document.ppp.pppIdleTime.value <= 0) || (!checkDigit(document.ppp.pppIdleTime.value))) {"
		"\n\t\t\talert(\"Invalid idle time!\");"
		"\n\t\t\tdocument.ppp.pppIdleTime.focus();"
		"\n\t\t\treturn false;\n\t\t}\n\t}"
		"\n\tif (!checkString(document.ppp.name.value)) {"
		"\n\t\talert(\"Invalid username!\");"
		"\n\t\tdocument.ppp.name.focus();"
		"\n\t\treturn false;\n\t}"
		"\n\tdocument.ppp.encodename.value=encode64(document.ppp.name.value);"
		"\n\tif(!isAllStar(document.ppp.passwd.value)){"
		"\n\t\tif (!checkString(document.ppp.passwd.value)) {"
		"\n\t\t\talert(\"Invalid password!\");"
		"\n\t\t\tdocument.ppp.passwd.focus();"
		"\n\t\t\treturn false;\n\t}"
		"\n\t\tdocument.ppp.encodepasswd.value=encode64(document.ppp.passwd.value);"
		"\n\t}", fmName);
		if (pEntry->cmode == CHANNEL_MODE_PPPOE) {
			boaWrite(wp,
			"\n\tif (!checkString(document.ppp.acName.value)) {"
			"\n\t\talert(\"Invalid AC name!\");"
			"\n\t\tdocument.ppp.acName.focus();"
			"\n\t\treturn false;\n\t}");
#ifdef _CWMP_MIB_
			boaWrite(wp,
			"\n\tif (!checkString(document.ppp.serviceName.value)) {"
			"\n\t\talert(\"Invalid service name!\");"
			"\n\t\tdocument.ppp.serviceName.focus();"
			"\n\t\treturn false;\n\t}");
#endif
		}
		boaWrite(wp, "\n\tdisableUsernamePassword();");
		boaWrite(wp,"\n\treturn true;\n}\n");
boaWrite(wp,"function postcheck()"
"{if ( validateKey( document.ppp.maxusernums.value ) == 0 ) {"
"alert(\"Invalid max user number!\");"
"document.ppp.maxusernums.focus();"
"return false;"
"}"
"d1 = getDigit(document.ppp.maxusernums.value, 1);"
"if (d1 > 255 || d1 < 1) {"
"alert(\"Invalid max user number!\");"
"document.ppp.maxusernums.focus();"
"return false;}return true;}</script></head>");

		// body
		boaWrite(wp,
		"<body><blockquote><h2><font color=\"#0000FF\">PPP Interface - Modify</font></h2>\n"
		"<form action=/boaform/admin/formPPPEdit method=POST name=\"%s\">\n"
		"<table border=0 width=600 cellspacing=4 cellpadding=0>"
		"<tr>\n<th align=left><b>PPP Interface:</b></th>\n<td>ppp%d</td></tr>\n"
		"<tr>\n<th align=left><b>Protocol:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM VCC:</b></th>\n<td>%d/%d</td></tr>\n",
		fmName, PPP_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);

		boaWrite(wp,
		"<tr><th align=left><b>Status:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>Enable\n</td></tr>\n",
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");

		memset(pppUsername, 0, sizeof(pppUsername));
		data_base64encode(pEntry->pppUsername, pppUsername);
		pppUsername[ENC_PPP_NAME_LEN] = '\0';
		memset(pppPassword, 0, sizeof(pppPassword));
		convert_to_star_string(pppPassword,strlen(pEntry->pppPassword));
		pppPassword[MAX_NAME_LEN-1] = '\0';

		boaWrite(wp,
		"<tr><th align=left><b>Login Name:</b></th>\n<td>"
		"<input type=\"text\" name=\"name\" maxlength=%d></td></tr>\n"
		"<tr><th align=left><b>Password:</b></th>\n"
		"<td><input type=\"password\" name=\"passwd\" maxlength=%d value=%s></td></tr>\n"
		"<tr><th align=left><b>Authentication Method:</b></th>\n<td>"
		"<select size=1 name=\"auth\">\n"
		"<option %s value=0>AUTO</option>\n"
		"<option %s value=1>PAP</option>\n"
		"<option %s value=2>CHAP</option></select></td></tr>\n",
		MAX_PPP_NAME_LEN, MAX_NAME_LEN-1, pppPassword,
		pEntry->pppAuth==PPP_AUTH_AUTO?"selected":"",
		pEntry->pppAuth==PPP_AUTH_PAP?"selected":"",
		pEntry->pppAuth==PPP_AUTH_CHAP?"selected":"");

		boaWrite(wp,"\n<script>document.%s.name.value = decode64(\"%s\");</script>\n", fmName, pppUsername);

		boaWrite(wp,
			"\n<script>\nfunction resetUserName()"
			"\n{\ndocument.getElementsByName(\"%s\")[0].reset();\n"
			"\ndocument.%s.name.value = decode64(\"%s\");"
			"\n}\n</script>\n", fmName, fmName, pppUsername);

		boaWrite(wp,
		"<tr><th align=left><b>Connection Type:</b></th>\n<td>"
		"<select size=1 name=\"pppConnectType\" onChange=\"pppTypeSelection()\">\n"
		"<option %s value=0>Continuous</option>\n"
		"<option %s value=1>Connect on Demand</option>\n"
		"<option %s value=2>Manual</option></select></td></tr>\n"
		"<tr><th align=left><b>Idle Time:</b></th>\n<td>"
		"<input type=\"text\" name=\"pppIdleTime\" maxlength=10 value=%d %s></td></tr>",
		pEntry->pppCtype==CONTINUOUS?"selected":"",
		pEntry->pppCtype==CONNECT_ON_DEMAND?"selected":"",
		pEntry->pppCtype==MANUAL?"selected":"",
		pEntry->pppIdleTime,
		pEntry->pppCtype==CONNECT_ON_DEMAND?"":"disabled");

#ifdef _CWMP_MIB_
		boaWrite(wp,
		"<tr><th align=left><b>Auto Disconnect Time:</b></th>\n<td>"
		"<input type=\"text\" name=\"disconnectTime\" value=%d maxlength=10></td></tr>\n",
		pEntry->autoDisTime);
		boaWrite(wp,
		"<tr><th align=left><b>Warn Disconnect Delay:</b></th>\n<td>"
		"<input type=\"text\" name=\"disconnectDelay\" value=%d maxlength=10></td></tr>\n",
		pEntry->warnDisDelay);
#endif

#ifdef DEFAULT_GATEWAY_V1
		boaWrite(wp,
		"<tr><th align=left><b>Default Route:</b></th>\n<td>"
		"<input type=radio value=0 name=\"droute\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"droute\" %s>Enable\n</td></tr>\n",
		pEntry->dgw? "":"checked", pEntry->dgw? "checked":"");
#endif

		boaWrite(wp,
		"<tr><th align=left><b>MTU:</b></th>\n<td>"
		"<input type=\"text\" name=\"mru\" value=%d maxlength=4></td></tr>\n",
		pEntry->mtu);

		if (pEntry->cmode == CHANNEL_MODE_PPPOE) {
#ifdef CONFIG_SPPPD_STATICIP
			writeStaticIP(wp, pEntry, fmName);
#endif
#ifdef PPPOE_PASSTHROUGH
			#if 0
			boaWrite(wp,
			"<tr><th align=left><b>PPPoE pass-through:</b></th>\n<td>"
			"<input type=radio value=0 name=\"poe\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"poe\" %s>Enable\n</td></tr>\n",
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			(pEntry->brmode==BRIDGE_ETHERNET)? "checked":"", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"");
			#endif
			boaWrite(wp,
			"<tr><th align=left><b>Bridge:</b></th>\n<td>"
			"<input type=radio value=0 name=\"mode\" %s>Bridged Ethernet"
			" (Transparent Bridging)&nbsp;&nbsp;\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=1 name=\"mode\" %s>Bridged PPPoE"
			" (implies Bridged Ethernet)\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=2 name=\"mode\" %s>Disable Bridge"
			" \n</td></tr>\n",
			pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif

			boaWrite(wp,
			"<tr><th align=left><b>AC-Name:</b></th>\n<td>"
			"<input type=\"text\" name=\"acName\" maxlength=29 value=%s></td></tr>\n",
			pEntry->pppACName);

			boaWrite(wp,
			"<tr><th align=left><b>Service-Name:</b></th>\n<td>"
			"<input type=\"text\" name=\"serviceName\" maxlength=29 value=%s></td></tr>\n",
			pEntry->pppServiceName);

// Mason Yu. enable_802_1p_090722
#ifdef ENABLE_802_1Q
			write1q(wp, pEntry, fmName);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
                          boaWrite(wp,
			"<tr><th align=left><b>PPPoE Proxy:</b></th>\n<td>"
			"<input type=radio value=0 name=\"pproxy\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"pproxy\" %s>Enable\n</td></tr>\n",
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");

			(!pEntry->PPPoEProxyEnable)? "checked":"", (pEntry->PPPoEProxyEnable)? "checked":"");

                          boaWrite(wp,
			"<tr><th align=left><b>Max User Nums:</b></th>\n<td>"
			"<input type=\"text\" name=\"maxusernums\" maxlength=2 value=%d></td></tr>\n",
			pEntry->PPPoEProxyMaxUser);
#endif

		}
#ifdef WEB_ENABLE_PPP_DEBUG
		int pppdbg = pppdbg_get(PPP_INDEX(pEntry->ifIndex));
		boaWrite(wp,
		"<tr><th align=left><b>Debug:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"pppdebug\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"pppdebug\" %s>Enable\n</td></tr>\n",
		pppdbg? "":"checked", pppdbg? "checked":"");
#endif
		boaWrite(wp,
		"</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return applyPPP()\">\n"
		"<input type=submit value=\"Return\" name=\"return\" onClick=\"return disableUsernamePassword()\">\n"
		"<input type=button value=\"Undo\" onClick=\"resetUserName()\">\n"
		"<input type=hidden value=\"\" name=\"encodename\">\n"
		"<input type=hidden value=\"\" name=\"encodepasswd\">\n"
		"<input type=hidden value=%d name=\"item\">\n", index);
		if ( !strcmp(directory_index, "index_user.html") )
			boaWrite(wp,
				"<input type=hidden value=\"/admin/wanadsl.asp\" name=\"submit-url\">\n");
		else
			boaWrite(wp,
				"<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n");
}

MIB_CE_ATM_VC_T tmpEntry;
void writeIPEdit(request * wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="ip";
		char *proto;
		char ipAddr[20], remoteIp[20], netMask[20];

		if (pEntry->cmode == CHANNEL_MODE_IPOE)
			proto = "MER";
		else
		if (pEntry->cmode == CHANNEL_MODE_RT1483)
			proto = "1483 routed";
#ifdef CONFIG_ATM_CLIP
		else if (pEntry->cmode == CHANNEL_MODE_RT1577)
			proto = "1577 routed";
#endif

#ifdef DEFAULT_GATEWAY_V1
		pdgw=pEntry->dgw;
#endif
		memcpy(&tmpEntry, pEntry, sizeof(tmpEntry));

		// Javascript
		boaWrite(wp, "<head>\n<script>\n");
		boaWrite(wp,
		"\nfunction getDigit(str, num)\n{\n"
		"\ti = 1;\n"
		"\tif (num != 1) {\n"
		"\t\twhile (i!=num && str.length!=0) {\n"
		"\t\t\tif ( str.charAt(0) == '.' ) {\n"
		"\t\t\t\ti ++;\n\t\t\t}\n"
		"\t\t\tstr = str.substring(1);\n\t\t}\n"
		"\t\tif (i!=num)\n\t\t\treturn -1;\n\t}\n"
		"\tfor (i=0; i<str.length; i++) {\n"
		"\t\tif (str.charAt(i) == '.') {\n"
		"\t\t\tstr = str.substring(0, i);\n\t\t\tbreak;\n\t\t}\n\t}\n"
		"\tif (str.length == 0)\n\t\treturn -1;\n"
		"\td = parseInt(str, 10);\n"
		"\treturn d;\n}\n");
		boaWrite(wp,
		"\nfunction validateKey(str)\n{\n"
		"\tfor (var i=0; i<str.length; i++) {\n"
		"\t\tif ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))\n"
		"\t\t\tcontinue;\n\t\treturn 0;\n\t}\n\treturn 1;\n}\n");
		boaWrite(wp,
		"\nfunction IsInvalidIP(str)\n{\n"
		"\td = getDigit(str, 1);\n"
		"\tif (d == 127)\n"
		"\t\treturn 1;\n\treturn 0;\n}\n");
		boaWrite(wp,
		"\nfunction checkDigitRange(str, num, min, max)\n{\n"
		"\td = getDigit(str,num);\n"
		"\tif (d > max || d < min)\n"
		"\t\treturn false;\n\treturn true;\n}\n");
		boaWrite(wp,
		"\nfunction checkIP(ip)\n{\n"
		"\tif (ip.value==\"\") {\n"
		"\t\talert(\"IP address cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.\");\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (validateKey(ip.value) == 0) {\n"
		"\t\talert(\"Invalid IP address value. It should be the decimal number (0-9).\");\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (IsInvalidIP( ip.value)==1) {\n"
		"\t\talert(\"Invalid IP address value\");\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n");
		boaWrite(wp,
		"\tif (!checkDigitRange(ip.value,1,1,223)) {\n"
		"\t\talert('Invalid IP address range in 1st digit. It should be 1-223.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (!checkDigitRange(ip.value,2,0,255)) {\n"
		"\t\talert('Invalid IP address range in 2nd digit. It should be 0-255.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (!checkDigitRange(ip.value,3,0,255)) {\n"
		"\t\talert('Invalid IP address range in 3rd digit. It should be 0-255.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n"
		"\tif (!checkDigitRange(ip.value,4,0,254)) {\n"
		"\t\talert('Invalid IP address range in 4th digit. It should be 1-254.');\n"
		"\t\tip.focus();\n\t\treturn false;\n\t}\n\treturn true;\n}\n");
		boaWrite(wp,
		"\nfunction checkMask(netmask)\n{\n"
		"\tvar i, d;\n"
		"\tif (netmask.value==\"\") {\n"
		"\t\talert(\"Subnet mask cannot be empty! It should be filled with 4 digit numbers as xxx.xxx.xxx.xxx.\");\n"
		"\t\tnetmask.focus();\n\t\treturn false;\n\t}\n"
		"\tif (validateKey(netmask.value) == 0) {\n"
		"\t\talert(\"Invalid subnet mask value. It should be the decimal number (0-9).\");\n"
		"\t\tnetmask.focus();\n\t\treturn false;\n\t}\n"
		"\tfor (i=1; i<=4; i++) {\n"
		"\t\td = getDigit(netmask.value,i);\n"
		"\t\tif( !(d==0 || d==128 || d==192 || d==224 || d==240 || d==248 || d==252 || d==254 || d==255 )) {\n"
		"\t\t\talert('Invalid subnet mask digit.It should be the number of 0, 128, 192, 224, 240, 248, 252 or 254');\n"
		"\t\t\tnetmask.focus();\n\t\t\treturn false;\n\t\t}\n\t}\n"
		"\treturn true;\n}\n");
		if (pEntry->cmode == CHANNEL_MODE_IPOE)
			boaWrite(wp,
			"\nfunction ipTypeSelection()\n{\n"
			"if ( document.%s.dhcp[0].checked) {\n"
			"document.%s.ipaddr.disabled = false;\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = false;\n"
#endif
			"document.%s.netmask.disabled = false;\n}\nelse {\n"
			"document.%s.ipaddr.value = \"\";\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.value = \"\";\n"
#endif
			"document.%s.netmask.value = \"\";\n"
			"document.%s.ipaddr.disabled = true;\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = true;\n"
#endif
			"document.%s.netmask.disabled = true;\n}\n}"
#ifdef DEFAULT_GATEWAY_V1
			,  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#else
			), fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#endif
//		"\n</script>\n</head>\n"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
		if (pEntry->cmode == CHANNEL_MODE_RT1483)
			boaWrite(wp, "\nfunction ipModeSelection()\n{\n"
			"if ( document.%s.ipunnumber[1].checked) {\n"
			"document.%s.ipaddr.disabled = true;\n"
			//"document.%s.remoteip.disabled = true;\n"
			//"document.%s.netmask.disabled = true;\n}\n"
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = true;\n}\n"
#else
			"}\n"
#endif
			"else if (document.%s.ipunnumber[0].checked) {\n"
			"document.%s.ipaddr.disabled = false;\n"
			//"document.%s.remoteip.disabled = false;\n"
			//"document.%s.netmask.disabled = false;\n}\n}"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#ifdef DEFAULT_GATEWAY_V1
			"document.%s.remoteip.disabled = false;\n}\n"
			"document.%s.netmask.disabled = true;\n}",  fmName, fmName, fmName, fmName, fmName, fmName, fmName);
#else
			"}\n"
			"document.%s.netmask.disabled = true;\n}"),  fmName, fmName, fmName, fmName, fmName);
#endif


#ifdef CONFIG_USER_WT_146
	if( (pEntry->cmode==CHANNEL_MODE_IPOE) || (pEntry->cmode==CHANNEL_MODE_RT1483) )
	{
		boaWrite(wp,
			"\n\nfunction checkDigit(str)\n"
			"{\n"
			"	for (var i=0; i<str.length; i++) {\n"
			"		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9'))\n"
			"			continue;\n"
			"		return 0;\n"
			"	}\n"
			"	return 1;\n"
			"}\n" );

		boaWrite(wp,
			"function checkHex(str)\n"
			"{\n"
			"	for (var i=0; i<str.length; i++) {\n"
			"		if ( (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||\n"
			"		     (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') ||\n"
			"		     (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') )\n"
			"			continue;\n"
			"		return 0;\n"
			"	}\n"
			"	return 1;\n"
			"}\n" );

		boaWrite(wp,
			"function bfdauthtype_select()\n"
			"{\n"
			"	if( document.ip.bfdauthtype.selectedIndex==0 )\n"
			"	{\n"
			"		document.ip.bfdkeyid.disabled=true;\n"
			"		document.ip.bfdkey.disabled=true;\n"
			"	}else{\n"
			"		document.ip.bfdkeyid.disabled=false;\n"
			"		document.ip.bfdkey.disabled=false;\n"
			"	}\n"
			"}\n" );

		boaWrite(wp,
			"function checkbfdinfo()\n"
			"{\n" );
		boaWrite(wp,
			"	if( (document.ip.bfdmult.value==\"\") ||\n"
			"		!checkDigit(document.ip.bfdmult.value) ||\n"
			"		(parseInt(document.ip.bfdmult.value, 10)<1) ||\n"
			"		(parseInt(document.ip.bfdmult.value, 10)>255) )\n"
			"	{\n"
			"		alert('Invalid Detect Mult value. It should be 1-255.');\n"
			"		document.ip.bfdmult.focus();\n"
			"		return false;\n"
			"	}\n"
			"	if( (document.ip.bfdtxint.value==\"\") ||\n"
			"		!checkDigit(document.ip.bfdtxint.value) ||\n"
			"		(parseInt(document.ip.bfdtxint.value, 10)<0) ||\n"
			"		(parseInt(document.ip.bfdtxint.value, 10)>4294967295) )\n"
			"	{\n"
			"		alert('Invalid Min Tx Interval value. It should be 0-4294967295.');\n"
			"		document.ip.bfdtxint.focus();\n"
			"		return false;\n"
			"	}\n" );
		boaWrite(wp,
			"	if( (document.ip.bfdrxint.value==\"\") ||\n"
			"		!checkDigit(document.ip.bfdrxint.value) ||\n"
			"		(parseInt(document.ip.bfdrxint.value, 10)<0) ||\n"
			"		(parseInt(document.ip.bfdrxint.value, 10)>4294967295) )\n"
			"	{\n"
			"		alert('Invalid Min Rx Interval value. It should be 0-4294967295.');\n"
			"		document.ip.bfdrxint.focus();\n"
			"		return false;\n"
			"	}\n"
			"	if( (document.ip.bfdechorxint.value==\"\") ||\n"
			"		!checkDigit(document.ip.bfdechorxint.value) ||\n"
			"		(parseInt(document.ip.bfdechorxint.value, 10)<0) ||\n"
			"		(parseInt(document.ip.bfdechorxint.value, 10)>4294967295) )\n"
			"	{\n"
			"		alert('Invalid Min Echo Rx Interval value. It should be 0-4294967295.');\n"
			"		document.ip.bfdechorxint.focus();\n"
			"		return false;\n"
			"	}\n" );
		boaWrite(wp,
			"	if( document.ip.bfdauthtype.selectedIndex!=0 )\n"
			"	{\n"
			"		if( (document.ip.bfdkeyid.value==\"\") ||\n"
			"			!checkDigit(document.ip.bfdkeyid.value) ||\n"
			"			(parseInt(document.ip.bfdkeyid.value, 10)<0) ||\n"
			"			(parseInt(document.ip.bfdkeyid.value, 10)>255) )\n"
			"		{\n"
			"			alert('Invalid Auth Key ID value. It should be 0-255.');\n"
			"			document.ip.bfdkeyid.focus();\n"
			"			return false;\n"
			"		}\n"
			"		if( (document.ip.bfdauthtype.selectedIndex==1)&&\n"
			"			((document.ip.bfdkey.value.length&0x1)||(document.ip.bfdkey.value.length<2)||(document.ip.bfdkey.value.length>32)) )\n"
			"		{\n"
			"			alert('Invalid Auth Key length for Simple Password. It should be an even length between 2-32.');\n"
			"			document.ip.bfdkey.focus();\n"
			"			return false;\n"
			"		}\n" );
		boaWrite(wp,
			"		if( ((document.ip.bfdauthtype.selectedIndex==2)||(document.ip.bfdauthtype.selectedIndex==3))&&\n"
			"			(document.ip.bfdkey.value.length!=32) )\n"
			"		{\n"
			"			alert('Invalid Auth Key length for (Meticulous) Keyed MD5 (32 in length).');\n"
			"			document.ip.bfdkey.focus();\n"
			"			return false;\n"
			"		}\n"
			"		if( ((document.ip.bfdauthtype.selectedIndex==4)||(document.ip.bfdauthtype.selectedIndex==5))&&\n"
			"			(document.ip.bfdkey.value.length!=40) )\n"
			"		{\n"
			"			alert('Invalid Auth Key length for (Meticulous) Keyed SHA1 (40 in length).');\n"
			"			document.ip.bfdkey.focus();\n"
			"			return false;\n"
			"		}\n"
			"		if( !checkHex(document.ip.bfdkey.value) )\n"
			"		{\n"
			"			alert('Invalid Auth Key. It should be hexadecimal (0-9,a-f,A-F).');\n"
			"			document.ip.bfdkey.focus();\n"
			"			return false;\n"
			"		}\n"
			"	}\n" );
		boaWrite(wp,
			"	if( (document.ip.bfddscp.value==\"\") ||\n"
			"		!checkDigit(document.ip.bfddscp.value) ||\n"
			"		(parseInt(document.ip.bfddscp.value, 10)<0) ||\n"
			"		(parseInt(document.ip.bfddscp.value, 10)>63) )\n"
			"	{\n"
			"		alert('Invalid DSCP value. It should be 0-63.');\n"
			"		document.ip.bfddscp.focus();\n"
			"		return false;\n"
			"	}\n" );
		if(pEntry->cmode==CHANNEL_MODE_IPOE)
		{
		  boaWrite(wp,
			"	if( (document.ip.bfdethprio.value==\"\") ||\n"
			"		!checkDigit(document.ip.bfdethprio.value) ||\n"
			"		(parseInt(document.ip.bfdethprio.value, 10)<0) ||\n"
			"		(parseInt(document.ip.bfdethprio.value, 10)>7) )\n"
			"	{\n"
			"		alert('Invalid Ethernet Priority value. It should be 0-7.');\n"
			"		document.ip.bfdethprio.focus();\n"
			"		return false;\n"
			"	}\n" );
		}
		boaWrite(wp,
			"	return true;\n"
			"}\n" );
	}
#endif //CONFIG_USER_WT_146


		boaWrite(wp, "\nfunction applyIP()\n{");
		if (pEntry->cmode == CHANNEL_MODE_IPOE) {
			boaWrite(wp,
			"\n\tif (document.%s.dhcp[0].checked) {"
			"\n\t\tif (!checkIP(document.%s.ipaddr))\n\t\treturn false;"
#ifdef DEFAULT_GATEWAY_V1
			"\n\t\tif (!checkIP(document.%s.remoteip))\n\t\treturn false;"
			"\n\t\tif (!checkMask(document.%s.netmask))\n\t\treturn false;"
			, fmName, fmName, fmName, fmName);
#else
			"\n\t\tif (!checkMask(document.%s.netmask))\n\t\treturn false;"
			), fmName, fmName, fmName);
#endif
		}
		if (pEntry->cmode == CHANNEL_MODE_RT1483)
			boaWrite(wp,
			"\n\tif (document.%s.ipunnumber[0].checked) {"
			"\n\t\tif (!checkIP(document.%s.ipaddr))\n\t\t\treturn false;"
#ifdef DEFAULT_GATEWAY_V1
			"\n\t\tif (!checkIP(document.%s.remoteip))\n\t\t\treturn false;"
			, fmName, fmName, fmName);
#else
			), fmName, fmName);
#endif
			//"\n\t\tif (!checkMask(document.%s.netmask))\n\t\t\treturn false;"
			//), fmName, fmName, fmName, fmName);
		boaWrite(wp, "\n\t}\n\treturn true;\n}\n");
/*		"document.%s.dhcp[0].disabled = true;\n"
		"document.%s.dhcp[1].disabled = true;\n}\nelse {\n"
		"document.%s.dhcp[0].disabled = false;\n"
		"document.%s.dhcp[1].disabled = false;\n}\n}"
		"ipTypeSelection();\n}\n}"*/
		boaWrite(wp, "\n</script>\n</head>\n");
//		"\n</script>\n</head>\n"),  fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName, fmName);

		// body
		boaWrite(wp,
		"<body><blockquote><h2><font color=\"#0000FF\">IP Interface - Modify</font></h2>\n"
		"<form action=/boaform/admin/formIPEdit method=POST name=\"%s\">\n"
		"<table border=0 width=600 cellspacing=4 cellpadding=0>"
#ifdef CONFIG_RTL_MULTI_PVC_WAN
		"<tr>\n<th align=left><b>IP Interface:</b></th>\n<td>vc%d_%d</td></tr>\n"
#else
		"<tr>\n<th align=left><b>IP Interface:</b></th>\n<td>vc%d</td></tr>\n"
#endif
		"<tr>\n<th align=left><b>Protocol:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM VCC:</b></th>\n<td>%d/%d</td></tr>\n",
#ifdef CONFIG_RTL_MULTI_PVC_WAN
		fmName, VC_MAJOR_INDEX(pEntry->ifIndex), VC_MINOR_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
#else
		fmName, VC_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
#endif

		boaWrite(wp,
		"<tr><th align=left><b>Status:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>Enable\n</td></tr>\n",
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");

		if (pEntry->cmode == CHANNEL_MODE_RT1483) {
			boaWrite(wp,
			"<tr><th align=left><b>Unnumbered:</b></th>\n<td>\n"
			"<input type=radio value=0 name=\"ipunnumber\" %s onClick=\"ipModeSelection()\">Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"ipunnumber\" %s onClick=\"ipModeSelection()\">Enable\n</td></tr>\n",
			pEntry->ipunnumbered? "":"checked", pEntry->ipunnumbered? "checked":"");
		}

		if (pEntry->cmode == CHANNEL_MODE_IPOE) {
			boaWrite(wp,
			"<tr><th align=left><b>Use DHCP:</b></th>\n<td>\n"
			"<input type=radio value=0 name=\"dhcp\" %s onClick=\"ipTypeSelection()\">Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"dhcp\" %s onClick=\"ipTypeSelection()\">Enable\n</td></tr>\n",
			pEntry->ipDhcp? "":"checked", pEntry->ipDhcp? "checked":"");
		}

		strcpy(ipAddr, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)));
#ifdef DEFAULT_GATEWAY_V1
		strcpy(remoteIp, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)));
#endif
		if (pEntry->cmode == CHANNEL_MODE_IPOE)
			strcpy(netMask, inet_ntoa(*((struct in_addr *)pEntry->netMask)));
		else
			strcpy(netMask, "");

		boaWrite(wp,
		"<tr><th align=left><b>Local IP Address:</b></th>\n<td>"
		"<input type=\"text\" name=\"ipaddr\" maxlength=15 value=\"%s\" %s></td></tr>\n"
#ifdef DEFAULT_GATEWAY_V1
		"<tr><th align=left><b>Remote IP Address:</b></th>\n"
		"<td><input type=\"text\" name=\"remoteip\" maxlength=15 value=\"%s\" %s></td></tr>\n"
#endif
		"<tr><th align=left><b>Subnet Mask:</b></th>\n"
		"<td><input type=\"text\" name=\"netmask\" maxlength=15 value=\"%s\" %s></td></tr>\n",
		ipAddr, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"",
#ifdef DEFAULT_GATEWAY_V1
		remoteIp, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"",
#endif
		netMask, (pEntry->ipDhcp || pEntry->cmode == CHANNEL_MODE_RT1483)?"disabled":"");
		//netMask, (pEntry->ipDhcp || pEntry->ipunnumbered==1)?"disabled":"");

#ifdef DEFAULT_GATEWAY_V1
		boaWrite(wp,
		"<tr><th align=left><b>Default Route:</b></th>\n<td>"
		"<input type=radio value=0 name=\"droute\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"droute\" %s>Enable\n</td></tr>\n",
		pEntry->dgw? "":"checked", pEntry->dgw? "checked":"");
#endif

//		boaWrite(wp,
//		"<tr><th align=left><b>MTU:</b></th>\n<td>"
//		"<input type=\"text\" name=\"mtu\" value=%d maxlength=4></td></tr>\n",
//		pEntry->mtu);

		if (pEntry->cmode == CHANNEL_MODE_IPOE) {
#ifdef PPPOE_PASSTHROUGH
			#if 0
			boaWrite(wp,
			"<tr><th align=left><b>PPPoE pass-through:</b></th>\n<td>"
			"<input type=radio value=0 name=\"poe\" %s>Disable&nbsp;&nbsp;\n"
			"<input type=radio value=1 name=\"poe\" %s>Enable\n</td></tr>\n",
			//pEntry->brmode? "":"checked", pEntry->brmode? "checked":"");
			(pEntry->brmode==BRIDGE_ETHERNET)? "checked":"", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"");
			#endif
			boaWrite(wp,
			"<tr><th align=left><b>Bridge:</b></th>\n<td>"
			"<input type=radio value=0 name=\"mode\" %s>Bridged Ethernet"
			" (Transparent Bridging)&nbsp;&nbsp;\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=1 name=\"mode\" %s>Bridged PPPoE"
			" (implies Bridged Ethernet)\n</td></tr>\n"
			"<tr><th></th>\n<td>\n"
			"<input type=radio value=2 name=\"mode\" %s>Disable Bridge"
			" \n</td></tr>\n",
			pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif

// Mason Yu. enable_802_1p_090722
#ifdef ENABLE_802_1Q
			write1q(wp, pEntry, fmName);
#endif
		}

		// Mason Yu. Set MTU
		boaWrite(wp,
		"<tr><th align=left><b>MTU:</b></th>\n<td>"
		"<input type=\"text\" name=\"mru\" value=%d maxlength=4></td></tr>\n",
		pEntry->mtu);

		boaWrite(wp,
		"</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\" onClick=\"return applyIP()\">\n"
		"<input type=submit value=\"Return\" name=\"return\">\n"
		"<input type=reset value=\"Undo\" name=\"reset\">\n"
		"<input type=hidden value=%d name=\"item\">\n", index);
		if ( !strcmp(directory_index, "index_user.html") )
			boaWrite(wp,
				"<input type=hidden value=\"/admin/wanadsl.asp\" name=\"submit-url\">\n");
		else
			boaWrite(wp,
				"<input type=hidden value=\"/wanadsl.asp\" name=\"submit-url\">\n");
		if (pEntry->cmode == CHANNEL_MODE_RT1483)
			boaWrite(wp,"<script>ipModeSelection();</script>");


#ifdef CONFIG_USER_WT_146
	if( (pEntry->cmode==CHANNEL_MODE_IPOE) || (pEntry->cmode==CHANNEL_MODE_RT1483) )
	{
		boaWrite(wp,
			"\n\n<table border=0 width=600 cellspacing=4 cellpadding=0>\n" );

		boaWrite(wp,
			"<tr>\n"
			"	<td colspan=\"2\"><hr size=1 noshade align=top></td>\n"
			"</tr>\n" );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>BFD Setting:</b></th>\n"
			"	<td></td>\n"
			"</tr>\n" );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>BFD:</b></th>\n"
			"	<td>\n"
			"		<input type=radio value=0 name=\"bfdenable\" %s>Disable&nbsp;&nbsp;\n"
			"		<input type=radio value=1 name=\"bfdenable\" %s>Enable\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_enable?"":"checked",
			pEntry->bfd_enable?"checked":"" );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Mode:</b></th>\n"
			"	<td>\n"
			"		<input type=radio value=0 name=\"bfdopmode\" %s>Asynchronous&nbsp;&nbsp;\n"
			"		<input type=radio value=1 name=\"bfdopmode\" %s>Demand\n"
			"	</td>\n"
			"</tr>\n",
			(pEntry->bfd_opmode==BFD_ASYNC_MODE)?"checked":"",
			(pEntry->bfd_opmode==BFD_DEMAND_MODE)?"checked":"" );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Role:</b></th>\n"
			"	<td>\n"
			"		<input type=radio value=0 name=\"bfdrole\" %s>Active&nbsp;&nbsp;\n"
			"		<input type=radio value=1 name=\"bfdrole\" %s>Passive\n"
			"	</td>\n"
			"</tr>\n",
			(pEntry->bfd_role==BFD_ACTIVE_ROLE)?"checked":"",
			(pEntry->bfd_role==BFD_PASSIVE_ROLE)?"checked":"" );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Detect Mult:</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfdmult\" value=%u maxlength=3>\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_detectmult );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Min Tx Interval:</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfdtxint\" value=%u maxlength=10> microseconds\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_mintxint );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Min Rx Interval:</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfdrxint\" value=%u maxlength=10> microseconds\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_minrxint );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Min Echo Rx Interval:</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfdechorxint\" value=%u maxlength=10> microseconds\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_minechorxint );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Auth Type:</b></th>\n"
			"	<td>\n"
			"		<select size=\"1\" name=\"bfdauthtype\" onChange=\"bfdauthtype_select()\">\n"
			"		<option %s value=\"0\">None</option>\n"
			"		<option %s value=\"1\">Simple Password</option>\n"
			"		<option %s value=\"2\">Keyed MD5</option>\n"
			"		<option %s value=\"3\">Meticulous Keyed MD5</option>\n"
			"		<option %s value=\"4\">Keyed SHA1</option>\n"
			"		<option %s value=\"5\">Meticulous Keyed SHA1</option>\n"
			"		</select>\n"
			"	</td>\n"
			"</tr>\n",
			(pEntry->bfd_authtype==BFD_AUTH_NONE)?"selected":"",
			(pEntry->bfd_authtype==BFD_AUTH_PASSWORD)?"selected":"",
			(pEntry->bfd_authtype==BFD_AUTH_MD5)?"selected":"",
			(pEntry->bfd_authtype==BFD_AUTH_METI_MD5)?"selected":"",
			(pEntry->bfd_authtype==BFD_AUTH_SHA1)?"selected":"",
			(pEntry->bfd_authtype==BFD_AUTH_METI_SHA1)?"selected":"" );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Auth Key ID:</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfdkeyid\" value=%u maxlength=3 %s>\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_authkeyid,
			(pEntry->bfd_authtype==BFD_AUTH_NONE)?"disabled":""  );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Auth Key (Hex):</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfdkey\"" );
			if(pEntry->bfd_authkeylen)
			{
				unsigned char bfdauthkeylen=0;
				boaWrite(wp, " value=" );
				while( bfdauthkeylen<pEntry->bfd_authkeylen )
				{
					boaWrite(wp,"%02x", pEntry->bfd_authkey[bfdauthkeylen] );
					bfdauthkeylen++;
				}
			}
			boaWrite(wp,
				" maxlength=40 %s>\n"
				"	</td>\n"
				"</tr>\n",
				(pEntry->bfd_authtype==BFD_AUTH_NONE)?"disabled":"" );

		boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>DSCP:</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfddscp\" value=%u maxlength=2>\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_dscp );

		if(pEntry->cmode==CHANNEL_MODE_IPOE)
		{
		  boaWrite(wp,
			"<tr>\n"
			"	<th align=left><b>Ethernet Priority:</b></th>\n"
			"	<td>\n"
			"		<input type=\"text\" name=\"bfdethprio\" value=%u maxlength=1>\n"
			"	</td>\n"
			"</tr>\n",
			pEntry->bfd_ethprio );
		}

		boaWrite(wp,
			"</table>\n"
			"<br><input type=submit value=\"Apply Changes\" name=\"bfdsave\" onClick=\"return checkbfdinfo()\">\n\n" );
		}
#endif //CONFIG_USER_WT_146


		boaWrite(wp,
		"</form></blockquote></body>");
//		"</form></blockquote></body>"), index);
}


void writeBrEdit(request * wp, MIB_CE_ATM_VC_Tp pEntry, int index)
{
		char *fmName="brif";
		char *proto;

		proto = "ENET";

		// head
		boaWrite(wp,"<head>\n</head>\n");

		// body
		boaWrite(wp,
		"<body><blockquote><h2><font color=\"#0000FF\">Bridged Interface - Modify</font></h2>\n"
		"<form action=/boaform/admin/formBrEdit method=POST name=\"%s\">\n"
		"<table border=0 width=500 cellspacing=4 cellpadding=0>"
#ifdef CONFIG_RTL_MULTI_PVC_WAN
		"<tr>\n<th align=left><b>Bridged Interface:</b></th>\n<td>vc%d_%d</td></tr>\n"
#else
		"<tr>\n<th align=left><b>Bridged Interface:</b></th>\n<td>vc%d</td></tr>\n"
#endif
		"<tr>\n<th align=left><b>Protocol:</b></th>\n<td>%s</td></tr>\n"
		"<tr>\n<th align=left><b>ATM VCC:</b></th>\n<td>%d/%d</td></tr>\n",
#ifdef CONFIG_RTL_MULTI_PVC_WAN
		fmName, VC_MAJOR_INDEX(pEntry->ifIndex), VC_MINOR_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
#else
		fmName, VC_INDEX(pEntry->ifIndex), proto, pEntry->vpi, pEntry->vci);
#endif

		boaWrite(wp,
		"<tr><th align=left><b>Status:</b></th>\n<td>\n"
		"<input type=radio value=0 name=\"status\" %s>Disable&nbsp;&nbsp;\n"
		"<input type=radio value=1 name=\"status\" %s>Enable\n</td></tr>\n",
		pEntry->enable? "":"checked", pEntry->enable? "checked":"");

#ifdef PPPOE_PASSTHROUGH
		boaWrite(wp,
		"<tr><th align=left><b>Mode:</b></th>\n<td>"
		"<input type=radio value=0 name=\"mode\" %s>Bridged Ethernet"
		" (Transparent Bridging)&nbsp;&nbsp;\n</td></tr>\n"
		"<tr><th></th>\n<td>\n"
		"<input type=radio value=1 name=\"mode\" %s>Bridged PPPoE"
		" (implies Bridged Ethernet)\n</td></tr>\n"
		"<tr><th></th>\n<td>\n"
		"<input type=radio value=2 name=\"mode\" %s>Disable Bridge"
		" \n</td></tr>\n",
		pEntry->brmode? "":"checked", (pEntry->brmode==BRIDGE_PPPOE)? "checked":"", (pEntry->brmode==BRIDGE_DISABLE)? "checked":"");
#endif

// Mason Yu. enable_802_1p_090722
#ifdef ENABLE_802_1Q
		write1q(wp, pEntry, fmName);
#endif
		boaWrite(wp,
		"</table>\n<br><input type=submit value=\"Apply Changes\" name=\"save\">\n"
		"<input type=submit value=\"Return\" name=\"return\">\n"
		"<input type=reset value=\"Undo\" name=\"reset\">\n"
		"<input type=hidden value=%d name=\"item\">\n", index);
		if ( !strcmp(directory_index, "index_user.html") )
			boaWrite(wp,
				"<input type=hidden value=\"/admin/wanadsl.asp\" name=\"submit-url\">\n");
		else
			boaWrite(wp,
				"<input type=hidden value=\"/admin/wanadsl.asp\" name=\"submit-url\">\n");
		boaWrite(wp, "</form></blockquote></body>");
}

/*--------------------------------------------------------------
 *	Check if user do some action in this page
 *	Return:
 *		0 :	no action
 *		1 :	do action
 *		-1:	error with message errMsg
 *-------------------------------------------------------------*/
int checkAction(request * wp, char *errMsg)
{
	char tmpBuf[100];
	char *strSubmit, *strValue;
	//MIB_CE_ATM_VC_Tp pEntry;
	MIB_CE_ATM_VC_T Entry;
	int action, index;

	strSubmit = boaGetVar(wp, "action", "");
	action = -1;

	if (strSubmit[0]) {
		action = strSubmit[0] - '0';
		strSubmit = boaGetVar(wp, "idx", "");
		index = strSubmit[0] - '0';
	}
	else
		return 0;	// no action

	if (action == 0) {	// delete
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(errMsg, errGetEntry);
			return -1;
		}
		deleteConnection(CONFIGONE, &Entry);

		resolveServiceDependency(index);
		if(mib_chain_delete(MIB_ATM_VC_TBL, index) != 1) {
			strcpy(errMsg, strDelChainerror);
			return -1;
		}
		restartWAN(CONFIGONE, NULL);
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
	}
	else
	if (action == 1) {	// modify
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(errMsg, errGetEntry);
			return -1;
		}

		boaHeader(wp);

		if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA)
			writePPPEdit(wp, &Entry, index);
		else
#ifdef CONFIG_ATM_CLIP
		if (Entry.cmode == CHANNEL_MODE_IPOE || Entry.cmode == CHANNEL_MODE_RT1483 || Entry.cmode == CHANNEL_MODE_RT1577)
#else
		if (Entry.cmode == CHANNEL_MODE_IPOE || Entry.cmode == CHANNEL_MODE_RT1483)
#endif
			writeIPEdit(wp, &Entry, index);
		else
		if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			writeBrEdit(wp, &Entry, index);

		boaFooter(wp);
	}

	strSubmit = boaGetVar(wp, "submit-url", "");   // hidden page
	if (strSubmit[0] && action != 1)
		boaRedirect(wp, strSubmit);
	else
		boaDone(wp, 200);

  	return 1;
}

static int check_vlan_id(MIB_CE_ATM_VC_Tp pEntry, char *tmpBuf)
{
/* We have only 1 bridge WAN currently */
/* So we let MIB_LAN_VLAN_ID1 be the same value with the first bridge WAN */
#if 0 //def CONFIG_RTK_RG_INIT
	unsigned char mbtd;
	int vlan_id;

	if(pEntry->cmode == CHANNEL_MODE_BRIDGE && pEntry->vlan == 1)
	{
		mib_get(MIB_MAC_BASED_TAG_DECISION, (void *)&mbtd);

		if(mbtd == 0)
		{
			mib_get(MIB_LAN_VLAN_ID1, (void *)&vlan_id);

			if(pEntry->vid != vlan_id)
			{
				sprintf(tmpBuf, "VLAN ID does not match LAN VLAN ID %d!", vlan_id);
				return -1;
			}
		}
	}
#endif
#ifndef CONFIG_RTK_RG_INIT
	if(pEntry->vlan == 1 && pEntry->vid >= 0 && pEntry->vid <= 9)
	{
		strcpy(tmpBuf, multilang(LANG_VLAN_ID_0__9_IS_RESERVED));
		return -1;
	}
#endif

	return 0;
}

void SaveAndReboot(request * wp)
{
	boaHeader(wp);
	//--- Add timer countdown
	boaWrite(wp, "<head><style>\n" \
		"#cntdwn{ border-color: white;border-width: 0px;font-size: 16pt;color: red;text-align:left; font-family: Calibri;}\n" \
		"</style><script language=javascript>\n" \
		"var h=60;\n" \
		"function stop() { clearTimeout(id); }\n"\
		"function start() { h--; if (h >= 0) { frm.time.value = h; frm.textname.value='System is rebooting, please wait ...'; id=setTimeout(\"start()\",1000); }\n" \
		"if (h == 0) { window.open(\"/\", \"_top\"); }}\n" \
		"</script></head>");
	boaWrite(wp,
		"<body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\" style=\"font-family: Calibri;\"><blockquote>" \
		"<form name=frm><font color=red><INPUT TYPE=text NAME=textname size=40 id=\"cntdwn\">\n" \
		"<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></font></form></blockquote></body>" );
	//--- End of timer countdown
	boaWrite(wp, "<body><blockquote>\n");
	boaWrite(wp, "%s<br><br>\n", rebootWord1);
	boaWrite(wp, "%s\n", rebootWord2);
	boaWrite(wp, "</blockquote></body>");
	boaFooter(wp);
	boaDone(wp, 200);

	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	cmd_reboot();
	return;
}

#ifdef WLAN_WISP
void setWirelessWanEntryMode(unsigned int wanmode, int idx)
{
	int i;
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;


	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);
		if(MEDIA_INDEX(Entry.ifIndex) == MEDIA_WLAN && ETH_INDEX(Entry.ifIndex)==idx)
			break;

	}
	if(wanmode & MODE_Wlan)
		Entry.enable = 1;
	else
		Entry.enable = 0;
	mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
}
#endif

void formWanMode(request * wp, char *path, char *query)
{
	char *strSubmit, *strValue, *submitUrl;
	char tmpBuf[100];
	unsigned int wanmode;
#ifdef WLAN_WISP
	int i;
#endif
	strSubmit = boaGetVar(wp, "submitwan", "");

	if(strSubmit[0])
	{
		if(!(strValue = boaGetVar(wp, "wan_mode", "")))
		{
			strcpy(tmpBuf, multilang(LANG_INTERNAL_ERROR_SET_WAN_MODE_FAILED));
			goto setErr_filter;
		}

		wanmode = atoi(strValue);
		wanmode = wanmode >> 5? 0: wanmode;

		if(wanmode)
		{
#ifdef WLAN_WISP
			if(!(wanmode&MODE_Wlan)) //disable wireless wan entry in ATM_VC_TBL
				for(i=0;i<NUM_WLAN_INTERFACE;i++)
					setWirelessWanEntryMode(wanmode, i);
#endif

			printf("WAN mode is set to ATM=%d, Ethernet=%d, PTM=%d, Wireless=%d\n",
				wanmode&MODE_ATM?1:0, wanmode&MODE_Ethernet?1:0, wanmode&MODE_PTM?1:0, wanmode&MODE_Wlan?1:0);

			mib_set(MIB_WAN_MODE, (void *)&wanmode);
			SaveAndReboot(wp);
			return;
		}
		else
		{
			strcpy(tmpBuf, multilang(LANG_INTERNAL_ERROR_SET_WAN_MODE_FAILED));
			goto setErr_filter;
		}
	}

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);

	return;

setErr_filter:
	ERR_MSG(tmpBuf);
#ifdef CONFIG_GUI_WEB
	writeFlash();
#endif
	restartWAN(CONFIGALL, NULL);
}

static inline int isAllStar(char *data)
{
	int i;
	for (i=0; i<strlen(data); i++) {
		if (data[i] != '*')
			return 0;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
static char wanif[10];
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
void formEth(request * wp, char *path, char *query)
{
	char *strSubmit, *strValue, *strMode;
	char *strConn, *strDisconn;
	char *submitUrl;
	char tmpBuf[100], macaddr[MAC_ADDR_LEN];
	int dns_changed=0;
	unsigned int vUInt;
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int i, selected=-1;
	int havePPP=0;
	char ifname[6];
	char buff[256];
	unsigned int ifMap; // high half for PPP bitmap, low half for vc bitmap
	struct data_to_pass_st msg;
	char qosParms[32];
	int drflag=0;	// Jenny, check if default route exists
	char disabled;	// for auto-pvc-search
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	unsigned int modify=0;
	unsigned int ifindex;
#endif
	MEDIA_TYPE_T mType;
#ifdef CONFIG_RTK_RG_INIT
	unsigned char mbtd;
	int vlan_id;
	int remained=-1;
#endif
#ifdef WLAN_WISP
	unsigned char wlanset=0;
	int need_restart_wlan=0;
	char vChar;
#endif

	if(strncmp(wanif,"ptm",3)==0)
		mType = MEDIA_PTM;
	else
		mType = MEDIA_ETH;


#ifdef WLAN_WISP
	strSubmit = boaGetVar(wp, "wlan_submit", "");
	if (strSubmit[0]) {
		wlanset=1;
		mType = MEDIA_WLAN;
	}
#endif
	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */

	strSubmit = boaGetVar(wp, "apply", "");
	// apply
	if (strSubmit[0]) {
		MIB_CE_ATM_VC_T entry, myEntry;
		int cnt=0, pIdx;

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
		strValue = boaGetVar(wp, "lkname", "");
		//printf("%s strValue=%s\n", __func__, strValue);
		if (strcmp(strValue, "new")) {
			modify = 1;
		}

		if (modify) {
			strValue = boaGetVar(wp, "lst", "");
			if (!strncmp(strValue, ALIASNAME_MWNAS, strlen(ALIASNAME_MWNAS))) {
				sscanf(strValue, ALIASNAME_MWNAS"%d", &ifindex);
				ifindex = TO_IFINDEX(MEDIA_ETH, DUMMY_PPP_INDEX, ETH_INDEX(ifindex));
				printf("%s %d ifindex=0x%x\n", __func__, __LINE__, ifindex);
			}
			#ifdef CONFIG_PTMWAN
			else if (!strncmp(strValue, ALIASNAME_MWPTM, strlen(ALIASNAME_MWPTM) )) {
				sscanf(strValue, ALIASNAME_MWPTM"%d", &ifindex);
				ifindex = TO_IFINDEX(MEDIA_PTM, DUMMY_PPP_INDEX, PTM_INDEX(ifindex));
				printf("%s %d ifindex=0x%x\n", __func__, __LINE__, ifindex);
			}
			#endif /*CONFIG_PTMWAN*/
			#ifdef WLAN_WISP
			else if (!strncmp(strValue, "wlan", 4 )) {
				sscanf(strValue, "wlan%d-vxd", &ifindex);
				ifindex = TO_IFINDEX(MEDIA_WLAN, DUMMY_PPP_INDEX, ifindex);
				printf("%s %d ifindex=0x%x\n", __func__, __LINE__, ifindex);
			}
			#endif /*WLAN_WISP*/
			else{
				strcpy(tmpBuf, strSelectvc);
				goto setErr_filter;
			}
		}
#endif
		memset(&entry,0,sizeof(entry));
		ifMap = 0;

		if (retrieveVcRecord(wp, &entry, mType) != 0) {
			return;
		}
#ifdef NEW_PORTMAPPING
		AUG_PRT("entry->enableIGMP=%d\n",entry.enableIGMP);
#endif
		if(check_vlan_id(&entry, tmpBuf) != 0)
			goto setErr_filter;

		//printf("%s %d vlan=%d vid=%d\n", __func__, __LINE__, entry.vlan, entry.vid);
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
		if (modify) {
			int itsMe;
#endif
			for (i=0; i<totalEntry; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				{
					//boaError(wp, 400, strGetChainerror);
					//return;
					strcpy(tmpBuf, strGetChainerror);
					goto setErr_filter;
				}
				//printf("%s %d VC entry %d ifindex=0x%x\n", __func__, __LINE__, i, Entry.ifIndex);
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
				itsMe = 0;
				if ((selected==-1) && ((mType == MEDIA_INDEX(Entry.ifIndex)) && (PTM_INDEX(Entry.ifIndex)==PTM_INDEX(ifindex)))) {
					selected = i;
					itsMe = 1;
				}
				if (!itsMe && mType == MEDIA_INDEX(Entry.ifIndex))
					ifMap |= 1 << PTM_INDEX(Entry.ifIndex);	// vc map

				if (!itsMe)
#endif
				if ((Entry.cmode == CHANNEL_MODE_PPPOE) || (Entry.cmode == CHANNEL_MODE_PPPOA))
					ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map

#ifdef DEFAULT_GATEWAY_V1
				if (Entry.cmode != CHANNEL_MODE_BRIDGE)
					if (Entry.dgw)
						drflag = 1;
#ifdef CONFIG_RTK_RG_INIT
						drflag = 0;
#endif
#endif
			}

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
			if ((selected == -1) || !mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry))
			{
				printf("no entry (%d) matched.\n", selected);
				strcpy(tmpBuf, strGetChainerror);
				goto setErr_filter;
			}
#else
			selected = getWanEntrybyMedia(&Entry, MEDIA_ETH);

			if (selected < 0) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_filter;
			}
#endif

			//stopConnection(&Entry);

			//entry.pppAuth = Entry.pppAuth;
			entry.rip = Entry.rip;
	//		entry.dgw = Entry.dgw;
	//		entry.mtu = Entry.mtu;
#ifdef CONFIG_USER_BRIDGE_GROUPING
			entry.itfGroup = Entry.itfGroup;
			//printf("Enable Bridge Grouping, itfGroup =%d\n",entry.itfGroup);
			entry.itfGroupNum = Entry.itfGroupNum;
#endif

#ifdef CONFIG_SPPPD_STATICIP
			if(entry.cmode == CHANNEL_MODE_PPPOE)
			{
				entry.pppIp = Entry.pppIp;
				memcpy(entry.ipAddr, Entry.ipAddr, sizeof(Entry.ipAddr));
			}
#endif

#ifdef CONFIG_USER_WT_146
			wt146_copy_config( &entry, &Entry );
#endif //CONFIG_USER_WT_146

			//strcpy(entry.pppACName, Entry.pppACName);
			if (entry.cmode != Entry.cmode)
				resolveServiceDependency(selected);

#ifdef DEFAULT_GATEWAY_V1
			// set default route flag
			if (entry.cmode != CHANNEL_MODE_BRIDGE)
				if (!entry.dgw && Entry.dgw)
					drflag =0;
			else
				if (Entry.dgw)
					drflag = 0;
#endif
			// find the ifIndex
			if (entry.cmode != Entry.cmode)
			{
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
				entry.ifIndex = TO_IFINDEX(mType, PPP_INDEX(entry.ifIndex), PTM_INDEX(entry.ifIndex));
#else
				entry.ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(entry.ifIndex), 0);
#endif
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
				if ((PPP_INDEX(entry.ifIndex) != DUMMY_PPP_INDEX) && (PPP_INDEX(entry.ifIndex) > 7))
#else
				if (entry.ifIndex == NA_PPP)
#endif
				{
					strcpy(tmpBuf, strMaxNumPPPoE);
					goto setErr_filter;
				}
#if 0
				// mode changed, restore to default
				if (entry.cmode == CHANNEL_MODE_PPPOE) {
					entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
					//entry.PPPoEProxyMaxUser=4;
#endif
					//entry.pppAuth = 0;
				}
				else {
#ifdef CONFIG_USER_PPPOE_PROXY
					entry.PPPoEProxyMaxUser=0;
#endif
	//				entry.dgw = 1;
					entry.mtu = 1500;
				}
#endif // endif if 0
	//			entry.dgw = 1;
			}
			else{
				entry.ifIndex = Entry.ifIndex;
			}

#ifdef _CWMP_MIB_
			/*start use_fun_call_for_wan_instnum*/
			resetWanInstNum(&entry);
			entry.ConDevInstNum = Entry.ConDevInstNum;
			entry.ConIPInstNum = Entry.ConIPInstNum;
			entry.ConPPPInstNum = Entry.ConPPPInstNum;
			updateWanInstNum(&entry);
			dumpWanInstNum(&entry, "new");
			/*end use_fun_call_for_wan_instnum*/

			entry.autoDisTime = Entry.autoDisTime;
			entry.warnDisDelay = Entry.warnDisDelay;
			//strcpy( entry.pppServiceName, Entry.pppServiceName );
			strcpy( entry.WanName, Entry.WanName );
#ifdef _PRMT_TR143_
			entry.TR143UDPEchoItf = Entry.TR143UDPEchoItf;
#endif //_PRMT_TR143_
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyEnable = Entry.PPPoEProxyEnable;
			entry.PPPoEProxyMaxUser = Entry.PPPoEProxyMaxUser;
#endif //CONFIG_USER_PPPOE_PROXY
#ifdef _PRMT_X_CT_COM_WANEXT_
			entry.ServiceList = Entry.ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#ifdef CONFIG_RTK_RG_INIT
			entry.rg_wan_idx = Entry.rg_wan_idx;
#endif
#endif /*_CWMP_MIB_*/

#ifndef CONFIG_USER_RTK_WAN_CTYPE
#ifdef DEFAULT_GATEWAY_V1
			if (entry.cmode != CHANNEL_MODE_BRIDGE)
			{
				if (drflag && entry.dgw && !Entry.dgw)
				{
					strcpy(tmpBuf, strDrouteExist);
					goto setErr_filter;
				}
				if (entry.dgw && !Entry.dgw)
					drflag = 1;
			}
#endif
#endif

#ifdef WLAN_WISP
			if(!wlanset)
#endif
			if(check_vlan_conflict(&entry, selected, tmpBuf) != 0)
				goto setErr_filter;

			/*ql:20080926 START: delete MIB_DHCP_CLIENT_OPTION_TBL entry to this pvc*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
			if ((Entry.cmode == CHANNEL_MODE_IPOE) && (Entry.ipDhcp == DHCP_CLIENT))
				delDhcpcOption(Entry.ifIndex);
#endif
			/*ql:20080926 END*/

			if( entry.ifIndex!=Entry.ifIndex )
			{
#ifdef PORT_FORWARD_GENERAL
				updatePortForwarding( Entry.ifIndex, entry.ifIndex );
#endif
#ifdef ROUTING
				updateRoutingTable( Entry.ifIndex, entry.ifIndex );
#endif
				//ql 20081125 add
#ifdef NEW_IP_QOS_SUPPORT
				delIpQosTcRule(&Entry);
#endif
			}
#ifdef NEW_IP_QOS_SUPPORT
			//ql 20081125 Add else
			else if (entry.cmode != Entry.cmode)
			{
				delIpQosTcRule(&Entry);
			}
#endif

#ifdef CONFIG_ATM_CLIP
			if (entry.cmode == CHANNEL_MODE_RT1577)
				entry.encap = 1;	// LLC
#endif

	//add by ramen for DNS bind pvc
#ifdef DNS_BIND_PVC_SUPPORT
			MIB_CE_ATM_VC_T dnsPvcEntry;
			if(mib_chain_get(MIB_ATM_VC_TBL,selected,&dnsPvcEntry)&&(dnsPvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
			{
				int tempi=0;
				unsigned int pvcifIdx=0;
				for(tempi=0;tempi<3;tempi++)
				{
					mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
					if(pvcifIdx==dnsPvcEntry.ifIndex) //I get it
					{
						if(entry.cmode==CHANNEL_MODE_BRIDGE) pvcifIdx=DUMMY_IFINDEX;
						else pvcifIdx=entry.ifIndex;
						mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif
#ifdef NEW_PORTMAPPING
		check_itfGroup(&entry, &Entry);
#endif


	if(entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA){
		if(isAllStar(entry.pppPassword))
			strncpy( entry.pppPassword, Entry.pppPassword , MAX_NAME_LEN);
	}

#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
			memcpy(entry.MacAddr, Entry.MacAddr, MAC_ADDR_LEN);
			/* Magician: Auto generate MAC address for every WAN interface. */
			mib_get(MIB_ELAN_MAC_ADDR, (void *)macaddr);
			macaddr[MAC_ADDR_LEN-1] += WAN_HW_ETHER_START_BASE + ETH_INDEX(entry.ifIndex);
			memcpy(entry.MacAddr, macaddr, MAC_ADDR_LEN);
			/* End Majgician */
#endif
			//jim garbage action...
			//memcpy(&Entry, &entry, sizeof(entry));
			// log message
			//printf("%s %d vlan %d vid %d\n", __func__, __LINE__, entry.vlan, entry.vid);
			deleteConnection(CONFIGONE, &Entry);

#if 0 //def WLAN_WISP
			if(wlanset){
				struct sockaddr hwaddr;
				char itf_name[IFNAMSIZ];
				snprintf(itf_name, 16, "wlan%d-vxd", ETH_INDEX(entry.ifIndex));
				getInAddr(itf_name, HW_ADDR, (void *)&hwaddr);
				//getInAddr("wlan0", HW_ADDR, (void *)&hwaddr);
				memcpy(entry.MacAddr, hwaddr.sa_data, MAC_ADDR_LEN);
			}
#endif
#ifdef WLAN_WISP
			if(entry.enable!=Entry.enable)
				need_restart_wlan = 1;
#endif
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, selected);
			restartWAN(CONFIGONE, &entry);
#ifdef WLAN_WISP
			if(need_restart_wlan)
				config_WLAN(ACT_RESTART);
#endif
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
		}
		else {//add
#ifdef CONFIG_RTK_RG_INIT
			remained = Check_RG_Intf_Count();
			if(remained == 0){
				/*Table FULL*/
				strcpy(tmpBuf, strTableFull);
				goto setErr_filter;
			}
#endif
			int intVal;

			for (i=0; i<totalEntry; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				{
					strcpy(tmpBuf, strGetChainerror);
					goto setErr_filter;
				}

				if(mType == MEDIA_INDEX(Entry.ifIndex))
					ifMap |= 1 << PTM_INDEX(Entry.ifIndex);	// vc map

				if ((Entry.cmode == CHANNEL_MODE_PPPOE) || (Entry.cmode == CHANNEL_MODE_PPPOA))
					ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex); // PPP map

#ifdef DEFAULT_GATEWAY_V1
				if (Entry.cmode != CHANNEL_MODE_BRIDGE)
					if (Entry.dgw)
						drflag = 1;
#ifdef CONFIG_RTK_RG_INIT
						drflag = 0;
#endif
#endif
			}

			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			entry.ifIndex = TO_IFINDEX(mType, PPP_INDEX(entry.ifIndex), PTM_INDEX(entry.ifIndex));

			if ((PPP_INDEX(entry.ifIndex) != DUMMY_PPP_INDEX) && (PPP_INDEX(entry.ifIndex) > 7))
			{
				strcpy(tmpBuf, strMaxNumPPPoE);
				goto setErr_filter;
			}

#ifdef _CWMP_MIB_
			/*start use_fun_call_for_wan_instnum*/
			resetWanInstNum(&entry);
			updateWanInstNum(&entry);
			dumpWanInstNum(&entry, "new");
			/*end use_fun_call_for_wan_instnum*/
#endif /*_CWMP_MIB_*/
#if 0
			// mode changed, restore to default
			if (entry.cmode == CHANNEL_MODE_PPPOE) {
				entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
				//entry.PPPoEProxyMaxUser=4;
#endif
				//entry.pppAuth = 0;
			}
			else {
#ifdef CONFIG_USER_PPPOE_PROXY
				//entry.PPPoEProxyMaxUser=0;
#endif
				entry.mtu = 1500;
			}
#endif // end of #if 0
#ifndef CONFIG_USER_RTK_WAN_CTYPE
#ifdef DEFAULT_GATEWAY_V1
			if (entry.cmode != CHANNEL_MODE_BRIDGE)
			{
				if (drflag && entry.dgw)
				{
					strcpy(tmpBuf, strDrouteExist);
					goto setErr_filter;
				}
			}
#endif
#endif
			if(check_vlan_conflict(&entry, -1, tmpBuf) != 0)
				goto setErr_filter;

#ifdef CONFIG_ATM_CLIP
			if (entry.cmode == CHANNEL_MODE_RT1577)
				entry.encap = 1;	// LLC
#endif

#ifdef NEW_PORTMAPPING
			check_itfGroup(&entry, 0);
#endif
#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
			/* Magician: Auto generate MAC address for every WAN interface. */
			mib_get(MIB_ELAN_MAC_ADDR, (void *)macaddr);
			macaddr[MAC_ADDR_LEN-1] += WAN_HW_ETHER_START_BASE + ETH_INDEX(entry.ifIndex);
			memcpy(entry.MacAddr, macaddr, MAC_ADDR_LEN);
			/* End Majgician */
#endif
			intVal = mib_chain_add(MIB_ATM_VC_TBL, (void *)&entry);
			if (intVal == 0) {
				strcpy(tmpBuf, strAddChainerror);
				goto setErr_filter;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, strTableFull);
				goto setErr_filter;
			}
			restartWAN(CONFIGONE, &entry);
		}
#endif
		goto setOk_filter;
	}
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
	else {//delete
		strSubmit = boaGetVar(wp, "delete", "");
		if (strSubmit[0]) {
			strValue = boaGetVar(wp, "lst", "");
			printf("to delete interface %s\n", strValue);
			if (!strncmp(strValue, ALIASNAME_MWNAS, strlen(ALIASNAME_MWNAS) )) {
				sscanf(strValue, ALIASNAME_MWNAS"%d", &ifindex);
				ifindex = TO_IFINDEX(MEDIA_ETH, DUMMY_PPP_INDEX, ifindex);
			}
			#ifdef CONFIG_PTMWAN
			else if (!strncmp(strValue, ALIASNAME_MWPTM, strlen(ALIASNAME_MWPTM) )) {
				sscanf(strValue, ALIASNAME_MWPTM"%d", &ifindex);
				ifindex = TO_IFINDEX(MEDIA_PTM, DUMMY_PPP_INDEX, ifindex);
			}
			#endif /*CONFIG_PTMWAN*/
			else{
				printf("interface %s not exist.\n", strValue);
				strcpy(tmpBuf, strSelectvc);
				goto setErr_filter;
			}
		}

		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
				strcpy(tmpBuf, strGetChainerror);
				goto setErr_filter;
			}

			if (MEDIA_ATM == MEDIA_INDEX(Entry.ifIndex))
				continue;

			if(mType == MEDIA_INDEX(Entry.ifIndex) && (PTM_INDEX(Entry.ifIndex) == PTM_INDEX(ifindex))) {
				selected = i;
				break;
			}
		}

		if (selected != -1)
		{
			/*ql:20080926 START: delete MIB_DHCP_CLIENT_OPTION_TBL entry to this pvc*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
			if ((Entry.cmode == CHANNEL_MODE_IPOE) && (Entry.ipDhcp == DHCP_CLIENT))
				delDhcpcOption(Entry.ifIndex);
#endif
			/*ql:20080926 END*/

#ifdef NEW_IP_QOS_SUPPORT
			delIpQosTcRule(&Entry);
#endif
			//stopConnection(&Entry);
			deleteConnection(CONFIGONE, &Entry);
			resolveServiceDependency(selected);

	//add by ramen for DNS bind pvc
#ifdef DNS_BIND_PVC_SUPPORT
			MIB_CE_ATM_VC_T dnsPvcEntry;
			if(mib_chain_get(MIB_ATM_VC_TBL,selected,&dnsPvcEntry)&&(dnsPvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
			{
				int tempi=0;
				unsigned int pvcifIdx=0;
				for(tempi=0;tempi<3;tempi++)
				{
					mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
					if(pvcifIdx==dnsPvcEntry.ifIndex) //I get it
					{
						if(Entry.cmode==CHANNEL_MODE_BRIDGE) pvcifIdx=DUMMY_IFINDEX;
						else pvcifIdx=Entry.ifIndex;
						mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif
			if (mib_chain_delete(MIB_ATM_VC_TBL, selected) != 1) {
				strcpy(tmpBuf, strDelChainerror);
				goto setErr_filter;
			}

			restartWAN(CONFIGONE, NULL);
			goto setOk_filter;
		}
		else
		{
			printf("interface %s not found.\n", strValue);
			strcpy(tmpBuf, strSelectvc);
			goto setErr_filter;
		}
	}
#endif

	strSubmit = boaGetVar(wp, "refresh", "");
	// Refresh
	if (strSubmit[0]) {
		//goto setOk_filter;
		goto setOk_nofilter;
	}
	goto setOk_nofilter;

setOk_filter:

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef WLAN_WISP
	if(need_restart_wlan)
		config_WLAN(ACT_RESTART);
#endif

setOk_nofilter:

#ifndef NO_ACTION
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
	}
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);

//	if (submitUrl[0])
//		boaRedirect(wp, submitUrl);
//	else
//		boaDone(wp, 200);

	return;

setErr_filter:
	ERR_MSG(tmpBuf);
}
#endif // CONFIG_ETHWAN

#ifdef CONFIG_RTL8672_SAR
void formAdsl(request * wp, char *path, char *query)
{
	char *strSubmit, *strValue, *strMode;
	char *strConn, *strDisconn;
	char *submitUrl;
	char tmpBuf[100];
	int dns_changed=0;
	unsigned int vUInt;
	unsigned int totalEntry;
	MIB_CE_ATM_VC_T Entry;
	int i, selected;
	int havePPP=0;
	char ifname[6];
	char buff[256];
	unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
	struct data_to_pass_st msg;
	char qosParms[32];
	int drflag=0;	// Jenny, check if default route exists
	char disabled;	// for auto-pvc-search
	MEDIA_TYPE_T mType;

	i = checkAction(wp, tmpBuf);
	if (i == 1)	// do action
		return;
	else if (i == -1) // error
		goto setErr_filter;
	// else no action (i == 0) -> continue ...

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */

	strSubmit = boaGetVar(wp, "delvc", "");

	// Delete
	if (strSubmit[0]) {
		unsigned int i;
		unsigned int idx;

		strValue = boaGetVar(wp, "select", "");

		if (strValue[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);

				if ( !gstrcmp(strValue, tmpBuf) )
				{
					resolveServiceDependency(idx);
//add by ramen to check whether the deleted pvc bind a dns!
#ifdef DNS_BIND_PVC_SUPPORT
					MIB_CE_ATM_VC_T dnsPvcEntry;
					if(mib_chain_get(MIB_ATM_VC_TBL,idx,&dnsPvcEntry)&&(dnsPvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
					{
						int tempi=0;
						unsigned int pvcifIdx=0;
						for(tempi=0;tempi<3;tempi++)
						{
							mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
							if(pvcifIdx==dnsPvcEntry.ifIndex)//I get it
							{
								pvcifIdx=DUMMY_IFINDEX;
								mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
							}
						}
					}
#endif
/*ql:20080926 START: delete MIB_DHCP_CLIENT_OPTION_TBL entry to this pvc*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
					{
						MIB_CE_ATM_VC_T dhcp_entry;
						if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&dhcp_entry))
						{
							if ((dhcp_entry.cmode == CHANNEL_MODE_IPOE) && (dhcp_entry.ipDhcp == DHCP_CLIENT))
								delDhcpcOption(dhcp_entry.ifIndex);
						}
					}
#endif
/*ql:20080926 END*/
					{
						MIB_CE_ATM_VC_T vcEntry;
						if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&vcEntry))
						{

#ifdef NEW_IP_QOS_SUPPORT//ql 20081125
							delIpQosTcRule(&vcEntry);
#endif
							deleteConnection(CONFIGONE, &vcEntry);
						}
					}

					if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1) {
						strcpy(tmpBuf, strDelChainerror);
						goto setErr_filter;
					}
					break;
				}
			}
		}
		else
		{
			strcpy(tmpBuf, strSelectvc);
			goto setErr_filter;
		}
		restartWAN(CONFIGONE, NULL);
		goto setOk_filter;
	}

	strSubmit = boaGetVar(wp, "modify", "");

	// Modify
	if (strSubmit[0]) {
		MIB_CE_ATM_VC_T entry, myEntry;
		int cnt=0, pIdx;

		memset(&entry,0,sizeof(entry));
		strValue = boaGetVar(wp, "select", "");
		selected = -1;
		ifMap = 0;

		if (retrieveVcRecord(wp, &entry, MEDIA_ATM) != 0) {
			return;
		}

		for (i=0; i<totalEntry; i++) {
			int isMe=0;
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
				//boaError(wp, 400, strGetChainerror);
				//return;
				strcpy(tmpBuf, strGetChainerror);
				printf( "%s: error, line=%d\n", __FUNCTION__, __LINE__ );
				goto setErr_filter;
			}
			mType = MEDIA_INDEX(Entry.ifIndex);
			if (mType == MEDIA_ATM && Entry.vpi == entry.vpi && Entry.vci == entry.vci )
				cnt++;
			snprintf(tmpBuf, 4, "s%d", i);
			if ( (selected == -1) && !gstrcmp(strValue, tmpBuf) )
				selected = i;
			else
			{
#ifdef CONFIG_RTL_MULTI_PVC_WAN
				if (mType == MEDIA_ATM && Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
					ifMap |= 1 << VC_MINOR_INDEX(Entry.ifIndex);	// vc map(minior)
					isMe = 1;
				}
				else {
					if (mType == MEDIA_ATM) {
						if (!isMe)
							ifMap |= (1 << 8) << VC_MAJOR_INDEX(Entry.ifIndex);	// vc map(major)
					}
				}
#else
				if (mType == MEDIA_ATM)
					ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
#endif
				ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
			}

#ifdef DEFAULT_GATEWAY_V1
			if (Entry.cmode != CHANNEL_MODE_BRIDGE)
				if (Entry.dgw)
					drflag = 1;
#endif
		}

		if (selected == -1)
		{
			strcpy(tmpBuf, strSelectvc);
			goto setErr_filter;
		}

		if(cnt > 0) {
			//Make sure there is no mismatch mode
			for (i=0, cnt=0; i<totalEntry; i++) {
				if(i==selected)
					continue;
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)){
	  			//boaError(wp, 400, strGetChainerror);
					//return;
					strcpy(tmpBuf, strGetChainerror);
					goto setErr_filter;
				}
				mType = MEDIA_INDEX(Entry.ifIndex);
				if (mType == MEDIA_ATM && Entry.vpi == entry.vpi && Entry.vci == entry.vci){
					// Mason Yu. set two vlan on a PVC
					#if !defined(CONFIG_RTL_MULTI_PVC_WAN)
					cnt++;
					if(Entry.cmode != entry.cmode){
						strcpy(tmpBuf, strConnectExist);
						goto setErr_filter;
					}
					#endif
					if (entry.cmode == CHANNEL_MODE_PPPOE) {	// Jenny, for multisession PPPoE support
						pIdx = i;
						// Mason Yu. set two vlan on a PVC
						#if defined(CONFIG_RTL_MULTI_PVC_WAN)
						cnt++;
						#endif
					}
				}
			}
			//Max. 2 PPPoE connections
			//if(entry.cmode == CHANNEL_MODE_PPPOE && cnt==2) {
			if(entry.cmode == CHANNEL_MODE_PPPOE && cnt==MAX_POE_PER_VC) {	// Jenny, multisession PPPoE support
				strcpy(tmpBuf, strMaxNumPPPoE);
				goto setErr_filter;
			//Max. 1 connection except PPPoE
			} else if(entry.cmode != CHANNEL_MODE_PPPOE&& cnt>0 ) {
				strcpy(tmpBuf, strConnectExist);
				goto setErr_filter;
			}
			if (entry.cmode == CHANNEL_MODE_PPPOE && cnt>0)		// Jenny, for multisession PPPoE, get existed PPPoE config for further ifindex use
				if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&myEntry)) {
					strcpy(tmpBuf, errGetEntry);
					goto setErr_filter;
				}
		}
		if (!mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_filter;
		}

		// restore stuff not posted in this form
		if (entry.cmode == CHANNEL_MODE_PPPOE)
			if (cnt > 0) {		// Jenny, for multisession PPPoE, ifIndex(VC device) must refer to existed PPPoE connection
#ifdef CONFIG_RTL_MULTI_PVC_WAN
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
#else
				ifMap &= 0xffff0000; // don't care the vc part
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
				entry.ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(entry.ifIndex), VC_INDEX(myEntry.ifIndex));
#endif
			}
			else {
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
#ifdef PPPOE_PASSTHROUGH
				if (entry.cmode == Entry.cmode)
					entry.brmode = Entry.brmode;
#endif
			}
		else
			entry.ifIndex = Entry.ifIndex;

		entry.qos = Entry.qos;
		entry.pcr = Entry.pcr;
		entry.scr = Entry.scr;
		entry.mbs = Entry.mbs;
		entry.cdvt = Entry.cdvt;
		entry.pppAuth = Entry.pppAuth;
		entry.rip = Entry.rip;
//		entry.dgw = Entry.dgw;
		entry.mtu = Entry.mtu;
#ifdef CONFIG_USER_BRIDGE_GROUPING
			entry.itfGroup = Entry.itfGroup;
			//printf("Enable Bridge Grouping, itfGroup =%d\n",entry.itfGroup);
			entry.itfGroupNum = Entry.itfGroupNum;
#endif
#ifdef CONFIG_SPPPD_STATICIP
		if(entry.cmode == CHANNEL_MODE_PPPOE)
		{
			entry.pppIp = Entry.pppIp;
			memcpy(entry.ipAddr, Entry.ipAddr, sizeof(Entry.ipAddr));
		}
#endif
#ifdef PPPOE_PASSTHROUGH
		if (entry.cmode != CHANNEL_MODE_PPPOE)
			if (entry.cmode == Entry.cmode)
				entry.brmode = Entry.brmode;
#endif

#ifdef CONFIG_RTL_MULTI_PVC_WAN
		fm1q(wp, &entry);
#endif

#ifdef CONFIG_USER_WT_146
		wt146_copy_config( &entry, &Entry );
#endif //CONFIG_USER_WT_146

#ifdef _CWMP_MIB_
		/*start use_fun_call_for_wan_instnum*/
		resetWanInstNum(&entry);

		findConDevInstNumByPVC( entry.vpi, entry.vci );
		if (entry.ConDevInstNum == 0)
		{
			entry.ConDevInstNum = Entry.ConDevInstNum;
			entry.ConIPInstNum = Entry.ConIPInstNum;
			entry.ConPPPInstNum = Entry.ConPPPInstNum;
		}

		updateWanInstNum(&entry);
		dumpWanInstNum(&entry, "new");
		/*end use_fun_call_for_wan_instnum*/

		entry.autoDisTime = Entry.autoDisTime;
		entry.warnDisDelay = Entry.warnDisDelay;
		strcpy( entry.WanName, Entry.WanName );
#ifdef _PRMT_TR143_
		entry.TR143UDPEchoItf = Entry.TR143UDPEchoItf;
#endif //_PRMT_TR143_
#ifdef CONFIG_USER_PPPOE_PROXY
		entry.PPPoEProxyEnable = Entry.PPPoEProxyEnable;
		entry.PPPoEProxyMaxUser = Entry.PPPoEProxyMaxUser;
#endif //CONFIG_USER_PPPOE_PROXY
#ifdef _PRMT_X_CT_COM_WANEXT_
		entry.ServiceList = Entry.ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#endif /*_CWMP_MIB_*/
		strcpy(entry.pppACName, Entry.pppACName);
		strcpy( entry.pppServiceName, Entry.pppServiceName );

		// channel mode changed, resolve dependency
		if (entry.cmode != Entry.cmode)
			resolveServiceDependency(selected);

#ifdef DEFAULT_GATEWAY_V1
		// set default route flag
		if (entry.cmode != CHANNEL_MODE_BRIDGE)
			if (!entry.dgw && Entry.dgw)
				drflag =0;
		else
			if (Entry.dgw)
				drflag = 0;
#endif

		// find the ifIndex
		if (entry.cmode != Entry.cmode)
		{
			if (!(entry.cmode == CHANNEL_MODE_PPPOE && cnt>0))	// Jenny, entries except multisession PPPoE
				entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == NA_VC)
			{
				strcpy(tmpBuf, strMaxVc);
				goto setErr_filter;
			}
			else if (entry.ifIndex == NA_PPP)
			{
				strcpy(tmpBuf, strMaxNumPPPoE);
				goto setErr_filter;
			}

			// mode changed, restore to default
			if (entry.cmode == CHANNEL_MODE_PPPOE) {
				entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
				//entry.PPPoEProxyMaxUser=4;
#endif
				entry.pppAuth = 0;
			}
			else {
#ifdef CONFIG_USER_PPPOE_PROXY
				//entry.PPPoEProxyMaxUser=0;
#endif
//				entry.dgw = 1;
				entry.mtu = 1500;
			}

//			entry.dgw = 1;
		}

#ifndef CONFIG_USER_RTK_WAN_CTYPE
#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != CHANNEL_MODE_BRIDGE)
		{
			if (drflag && entry.dgw && !Entry.dgw)
			{
				strcpy(tmpBuf, strDrouteExist);
				goto setErr_filter;
			}
			if (entry.dgw && !Entry.dgw)
				drflag = 1;
		}
#endif
#endif

		if(check_vlan_conflict(&entry, selected, tmpBuf) != 0)
			goto setErr_filter;

		/*ql:20080926 START: delete MIB_DHCP_CLIENT_OPTION_TBL entry to this pvc*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
		if ((Entry.cmode == CHANNEL_MODE_IPOE) && (Entry.ipDhcp == DHCP_CLIENT))
			delDhcpcOption(Entry.ifIndex);
#endif
		/*ql:20080926 END*/

		if( entry.ifIndex!=Entry.ifIndex )
		{
			//ql 20081125 add
#ifdef NEW_IP_QOS_SUPPORT
			delIpQosTcRule(&Entry);
#endif
		}
#ifdef NEW_IP_QOS_SUPPORT
		//ql 20081125 Add else
		else if (entry.cmode != Entry.cmode)
		{
			delIpQosTcRule(&Entry);
		}
#endif

#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == CHANNEL_MODE_RT1577)
			entry.encap = 1;	// LLC
#endif

//add by ramen for DNS bind pvc
#ifdef DNS_BIND_PVC_SUPPORT
		MIB_CE_ATM_VC_T dnsPvcEntry;
		if(mib_chain_get(MIB_ATM_VC_TBL,selected,&dnsPvcEntry)&&(dnsPvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
		{
			int tempi=0;
			unsigned int pvcifIdx=0;
			for(tempi=0;tempi<3;tempi++)
			{
				mib_get(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
				if(pvcifIdx==dnsPvcEntry.ifIndex) //I get it
				{
					if(entry.cmode==CHANNEL_MODE_BRIDGE) pvcifIdx=DUMMY_IFINDEX;
					else pvcifIdx=entry.ifIndex;
					mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
				}
			}
		}
#endif
		//jim garbage action...
		//memcpy(&Entry, &entry, sizeof(entry));
		// log message

#ifdef NEW_PORTMAPPING
		AUG_PRT("Modify ; entry.itfGroup : 0x%x\n", entry.itfGroup);

		check_itfGroup(&entry, &Entry);
#endif
		if(entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA){
			if(isAllStar(entry.pppPassword))
				strncpy( entry.pppPassword, Entry.pppPassword , MAX_NAME_LEN);
		}
		deleteConnection(CONFIGONE, &Entry);
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&entry, selected);
		restartWAN(CONFIGONE, &entry);
		goto setOk_filter;
	}

	strSubmit = boaGetVar(wp, "add", "");

	// Add
	if (strSubmit[0]) {
		MIB_CE_ATM_VC_T entry;
		int cnt, pIdx, intVal;

		memset(&entry,0,sizeof(entry));

		if (totalEntry >= MAX_VC_NUM)
		{
			strcpy(tmpBuf, strMaxVc);
			goto setErr_filter;
		}

		if (retrieveVcRecord(wp, &entry, MEDIA_ATM) != 0) {
			return;
		}

		// check if connection exists
		ifMap = 0;
		cnt=0;

#ifndef CONFIG_RTL_MULTI_PVC_WAN	// Mason Yu. multi pvc wan
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
				//boaError(wp, 400, strGetChainerror);
				//return;
				strcpy(tmpBuf, strGetChainerror);
				goto setErr_filter;
			}

			mType = MEDIA_INDEX(Entry.ifIndex);
			if (mType == MEDIA_ATM && Entry.vpi == entry.vpi && Entry.vci == entry.vci)
			{
				cnt++;
				pIdx = i;	// Jenny, for multisession PPPoE, record entry index instead of atmvc entry pointer
//				pmyEntry = &Entry;
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
#else	//#ifndef CONFIG_RTL_MULTI_PVC_WAN
		for (i=0; i<totalEntry; i++) {
			int isMe=0, j;
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
				strcpy(tmpBuf, strGetChainerror);
				goto setErr_filter;
			}

			mType = MEDIA_INDEX(Entry.ifIndex);
			if (mType == MEDIA_ATM && Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
				ifMap |= 1 << VC_MINOR_INDEX(Entry.ifIndex);	// vc map(minior)

				if (!isMe) {
					// When we find the major vc index, Fixed the ifmap of major vc index
					for (j=0; j <VC_MAJOR_INDEX(Entry.ifIndex); j++)
						ifMap |= ((1 << 8) << j);	// vc map(major)
				}
				isMe = 1;

				if ( (entry.cmode==CHANNEL_MODE_PPPOE) &&
					(Entry.cmode==CHANNEL_MODE_PPPOE) ) {
					cnt++;
					pIdx = i;	// Jenny, for multisession PPPoE, record entry index instead of atmvc entry pointer
				}
			}
			else {
				if (mType == MEDIA_ATM) {
					if (!isMe)
						ifMap |= (1 << 8) << VC_MAJOR_INDEX(Entry.ifIndex);	// vc map(major)
				}
			}
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map

#ifdef DEFAULT_GATEWAY_V1
			if (Entry.cmode != CHANNEL_MODE_BRIDGE)
				if (Entry.dgw)
					drflag = 1;
#endif
		}
#endif	//#ifndef CONFIG_RTL_MULTI_PVC_WAN

		if (cnt == 0)	// pvc(PPP) not exists
		{
			entry.ifIndex = if_find_index(entry.cmode, ifMap);
			if (entry.ifIndex == NA_VC)
			{
				strcpy(tmpBuf, strMaxVc);
				goto setErr_filter;
			}
			else if (entry.ifIndex == NA_PPP)
			{
				strcpy(tmpBuf, strMaxNumPPPoE);
				goto setErr_filter;
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
			if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&Entry)) {	// Jenny, for multisession PPPoE, get existed pvc config
				strcpy(tmpBuf, errGetEntry);
				goto setErr_filter;
			}
			if (Entry.cmode == CHANNEL_MODE_PPPOE && entry.cmode == CHANNEL_MODE_PPPOE)
			{
				if (cnt<MAX_POE_PER_VC)
				{	// get the pvc info.
					entry.qos = Entry.qos;
					entry.pcr = Entry.pcr;
					entry.encap = Entry.encap;
#ifdef CONFIG_RTL_MULTI_PVC_WAN
					entry.ifIndex = if_find_index(entry.cmode, ifMap);
#else
					ifMap &= 0xffff0000; // don't care the vc part
					entry.ifIndex = if_find_index(entry.cmode, ifMap);
					if (entry.ifIndex == NA_PPP)
					{
						strcpy(tmpBuf, strMaxNumPPPoE);
						goto setErr_filter;
					}
					entry.ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(entry.ifIndex), VC_INDEX(Entry.ifIndex));
#endif
#ifdef PPPOE_PASSTHROUGH
					entry.brmode = Entry.brmode;	// Jenny, for multisession PPPoE
#endif

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
					strcpy(tmpBuf, strMaxNumPPPoE);
					goto setErr_filter;
				}
			}
			else
			{
				strcpy(tmpBuf, strConnectExist);
				goto setErr_filter;
			}
		}

#ifndef CONFIG_USER_RTK_WAN_CTYPE
#ifdef DEFAULT_GATEWAY_V1
		if (entry.cmode != CHANNEL_MODE_BRIDGE)
			if (drflag && entry.dgw)
			{
				strcpy(tmpBuf, strDrouteExist);
				goto setErr_filter;
			}
#endif
#endif

#ifdef CONFIG_ATM_CLIP
		if (entry.cmode == CHANNEL_MODE_RT1577)
			entry.encap = 1;	// LLC
#endif

		// set default
//		entry.dgw = 1;
		if (entry.cmode == CHANNEL_MODE_PPPOE)
			{
			entry.mtu = 1492;
#ifdef CONFIG_USER_PPPOE_PROXY
			//entry.PPPoEProxyMaxUser=4;
			//entry.PPPoEProxyEnable=0;
#endif
			}
		else
			entry.mtu = 1500;

#ifdef CONFIG_RTL_MULTI_PVC_WAN
		fm1q(wp, &entry);
#endif

#ifdef CONFIG_USER_WT_146
		wt146_set_default_config( &entry );
#endif //CONFIG_USER_WT_146

#ifdef NEW_PORTMAPPING
		check_itfGroup(&entry, 0);
#endif
                if(check_vlan_conflict(&entry, -1, tmpBuf) != 0)
			goto setErr_filter;

		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_filter;
		}
		restartWAN(CONFIGONE, &entry);
		goto setOk_filter;
	}

	// auto-pvc-search Apply
	strSubmit = boaGetVar(wp, "enablepvc", "");
	if ( strSubmit[0] ) {
		//printf("formAdsl: got disabled %x %s\n", atoi(strSubmit), strSubmit);
		update_auto_pvc_disable(atoi(strSubmit));
	}

	//auto-pvc-search add PVC
	strSubmit = boaGetVar(wp, "autopvcadd", "");
	if ( strSubmit[0] ) {
		//printf("formAdsl(autopvcadd): got strSubmit %s\n", strSubmit);
		if(add_auto_pvc_search_pair(wp, 1) != 0)
			return;
	}
	//auto-pvc-search delete PVC
	strSubmit = boaGetVar(wp, "autopvcdel", "");
	if ( strSubmit[0] ) {
		//printf("formAdsl(autopvcdel): got strSubmit %s\n", strSubmit);
		if(add_auto_pvc_search_pair(wp, 0) != 0)
			return;
	}

	strSubmit = boaGetVar(wp, "refresh", "");
	// Refresh
	if (strSubmit[0]) {
		//goto setOk_filter;
		goto setOk_nofilter;
	}
	goto setOk_nofilter;

setOk_filter:
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

setOk_nofilter:

#ifndef NO_ACTION
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
	}
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);

	return;

setErr_filter:
	ERR_MSG(tmpBuf);
}
#endif // CONFIG_RTL8672_SAR

/////////////////////////////////////////////////////////////////////////////
void formAtmVc(request * wp, char *path, char *query)
{
	char *strVal, *submitUrl;
	MIB_CE_ATM_VC_T entry;
	MIB_CE_ATM_VC_T Entry;
	char tmpBuf[100];
	memset(tmpBuf,0x00,100);

	strVal = boaGetVar(wp, "changeAtmVc", "");

	deleteConnection(CONFIGALL, NULL);
	if (strVal[0] ) {
		unsigned int i, k;
		unsigned int totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
		unsigned int vUInt;

		strVal = boaGetVar(wp, "select", "");
		if (!strVal[0]) {
			strcpy(tmpBuf, strSelectvc);
			goto setErr_filter;
		}

		sscanf(strVal,"s%u", &i);

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
			strcpy(tmpBuf, strGetChainerror);
			goto setErr_filter;
		}

		memcpy(&entry, &Entry, sizeof(entry));

		strVal = boaGetVar(wp, "qos", "");

		if (!strVal[0]) {
			strcpy(tmpBuf, multilang(LANG_INVALID_QOS));
			goto setErr_filter;
		}

		entry.qos = strVal[0] - '0';

		strVal = boaGetVar(wp, "pcr", "");

		if (!strVal[0]) {
			strcpy(tmpBuf, multilang(LANG_INVALID_PCR));
			goto setErr_filter;
		}

		sscanf(strVal,"%u",&vUInt);
		if ( vUInt>65535) {
			strcpy(tmpBuf, multilang(LANG_INVALID_PCR));
			goto setErr_filter;
		}
		entry.pcr = (unsigned short)vUInt;

		strVal = boaGetVar(wp, "cdvt", "");

		if (!strVal[0]) {
			strcpy(tmpBuf, multilang(LANG_INVALID_CDVT));
			goto setErr_filter;
		}

		sscanf(strVal,"%u",&entry.cdvt);

		if (entry.qos == 2 || entry.qos == 3)
		{
			strVal = boaGetVar(wp, "scr", "");

			if (!strVal[0]) {
				strcpy(tmpBuf, multilang(LANG_INVALID_SCR));
				goto setErr_filter;
			}

			sscanf(strVal,"%u",&vUInt);
			if ( vUInt>65535) {
				strcpy(tmpBuf, multilang(LANG_INVALID_SCR));
				goto setErr_filter;
			}
			entry.scr = (unsigned short)vUInt;

			strVal = boaGetVar(wp, "mbs", "");

			if (!strVal[0]) {
				strcpy(tmpBuf, multilang(LANG_INVALID_MBS));
				goto setErr_filter;
			}

			sscanf(strVal,"%u",&vUInt);
			if ( vUInt>65535) {
				strcpy(tmpBuf, multilang(LANG_INVALID_MBS));
				goto setErr_filter;
			}
			entry.mbs = (unsigned short)vUInt;
		}

		memcpy(&Entry, &entry, sizeof(entry));
		// log message
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);

		// synchronize this vc across all interfaces
		for (k=i+1; k<totalEntry; k++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&Entry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_filter;
			}
			if (Entry.vpi == entry.vpi && Entry.vci == entry.vci) {
				Entry.qos = entry.qos;
				Entry.pcr = entry.pcr;
				Entry.cdvt = entry.cdvt;
				Entry.scr = entry.scr;
				Entry.mbs = entry.mbs;
				// log message
				mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, k);
			}
		}

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
	}


setOk_filter:
//	mib_update(CURRENT_SETTING);

#ifndef NO_ACTION
	pid = fork();
        if (pid) {
	      	waitpid(pid, NULL, 0);
	}
        else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
               	exit(1);
        }
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	restartWAN(CONFIGALL, NULL);
	return;
setErr_filter:
	ERR_MSG(tmpBuf);
	restartWAN(CONFIGALL, NULL);
}

/////////////////////////////////////////////////////////////////////////////

int atmVcList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i, k;
	MIB_CE_ATM_VC_T Entry;
	char	vpi[6], vci[6], *qos, pcr[6], scr[6], mbs[6];
	char cdvt[12];
	char *temp;
	int vcList[MAX_VC_NUM], found;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	memset(vcList, 0, MAX_VC_NUM*4);

	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">VPI</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">VCI</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">QoS</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">PCR</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">CDVT</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">SCR</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">MBS</td></font></tr>\n",
	multilang(LANG_SELECT));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (MEDIA_INDEX(Entry.ifIndex) != MEDIA_ATM)
			continue;
		// skip duplicate vc display
		found = 0;
#ifdef CONFIG_RTL_MULTI_PVC_WAN
		for (k=0; k<MAX_VC_NUM && vcList[k] != 0; k++) {
			if (vcList[k] == VC_MAJOR_INDEX(Entry.ifIndex)+1) {
				found = 1;
				break;
			}
#else
		for (k=0; k<MAX_VC_NUM && vcList[k] != 0; k++) {
			if (vcList[k] == VC_INDEX(Entry.ifIndex)) {
				found = 1;
				break;
			}
#endif
		}
		if (found)
			continue;
		else
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			vcList[k] = VC_MAJOR_INDEX(Entry.ifIndex) + 1;
#else
			vcList[k] = VC_INDEX(Entry.ifIndex);
#endif

		snprintf(vpi, 6, "%u", Entry.vpi);
		snprintf(vci, 6, "%u", Entry.vci);
		snprintf(pcr, 6, "%u", Entry.pcr);
		snprintf(cdvt, 12, "%u", Entry.cdvt);

		if ( Entry.qos == ATMQOS_UBR )
			qos = "UBR";
		else if ( Entry.qos == ATMQOS_CBR )
			qos = "CBR";
		else if ( Entry.qos == ATMQOS_VBR_NRT )
			qos = "nrt-VBR";
		else if ( Entry.qos == ATMQOS_VBR_RT )
			qos = "rt-VBR";

		if(Entry.qos > 1) {
			snprintf(scr, 6, "%u", Entry.scr);
			snprintf(mbs, 6, "%u", Entry.mbs);
		} else {
			strcpy(scr, "---");
			strcpy(mbs, "---");
		}

		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=\"postVC(%s,%s,%d,%d,%d,%d,%d)\"></td>\n",
		i,vpi,vci,Entry.qos,Entry.pcr,Entry.cdvt,Entry.scr,Entry.mbs);
		nBytesSent += boaWrite(wp,
		      	"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		      	"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		      	"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
				vpi, vci, qos, pcr, cdvt, scr, mbs);
	}

	return nBytesSent;
}

// Jenny, PPPoE static IP option
#ifdef CONFIG_SPPPD_STATICIP
static int fmStaticIP(request * wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char *strValue;
	char tmpBuf[100];

	// static PPPoE
	strValue = boaGetVar(wp, "pppip", "");
	pEntry->pppIp = strValue[0] - '0';

	strValue = boaGetVar(wp, "staticip", "");
	if (strValue[0])
		if (!inet_aton(strValue, (struct in_addr *)pEntry->ipAddr)) {
			strcpy(tmpBuf, Tinvalid_ip);
			ERR_MSG(tmpBuf);
		}
}
#endif

/////////////////////////////////////////////////////////////////////////////
void formPPPEdit(request * wp, char *path, char *query)
{
	char *strSubmit, *strValue;
	char *submitUrl;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry,old_entry;
	int index,intVal;

	strSubmit = boaGetVar(wp, "save", "");
	if (strSubmit[0]) {
		strSubmit = boaGetVar(wp, "item", "");
		index = strSubmit[0] - '0';
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_ppp;
		}

		memcpy(&old_entry,&Entry,sizeof(old_entry));

		// status
		strValue = boaGetVar(wp, "status", "");
		Entry.enable = strValue[0] - '0';

		// user name
		strValue = boaGetVar(wp, "encodename", "");
		if ( strValue[0] ) {
			memset(Entry.pppUsername, 0, sizeof(Entry.pppUsername));
			data_base64decode(strValue, Entry.pppUsername);
			if ( strlen(Entry.pppUsername) > MAX_PPP_NAME_LEN ) {
				strcpy(tmpBuf, strUserNametoolong);
				goto setErr_ppp;
			}
			//strncpy(Entry.pppUsername, strValue, MAX_PPP_NAME_LEN);
			Entry.pppUsername[MAX_PPP_NAME_LEN]='\0';
		}
		else {
			strcpy(tmpBuf, strUserNameempty);
			goto setErr_ppp;
		}


		// password
		strValue = boaGetVar(wp, "encodepasswd", "");

		if ( strValue[0] ) {
			memset(Entry.pppPassword, 0, sizeof(Entry.pppPassword));
			data_base64decode(strValue, Entry.pppPassword);
			if ( strlen(Entry.pppPassword) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, strPasstoolong);
				goto setErr_ppp;
			}
			//strncpy(Entry.pppPassword, strValue, MAX_NAME_LEN-1);
			Entry.pppPassword[MAX_NAME_LEN-1]='\0';
			//Entry.pppPassword[MAX_NAME_LEN]='\0';
		}
		else {
			// get all stars
			// password
		#if 0
			strValue = boaGetVar(wp, "passwd", "");
			if ( strValue[0] ) {
				if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
					strcpy(tmpBuf, strPasstoolong);
					goto setErr_ppp;
				}
				strncpy(Entry.pppPassword, strValue, MAX_NAME_LEN-1);
				Entry.pppPassword[MAX_NAME_LEN-1]='\0';
				//Entry.pppPassword[MAX_NAME_LEN]='\0';
			}
			else{
				strcpy(tmpBuf, strPassempty);
				goto setErr_ppp;
			}
		#endif
		}

		// authentication method
		strValue = boaGetVar(wp, "auth", "");

		if ( strValue[0] ) {
			PPP_AUTH_T method;

			method = (PPP_AUTH_T)(strValue[0] - '0');
			Entry.pppAuth = (unsigned char)method;
		}

		// connection type
		strValue = boaGetVar(wp, "pppConnectType", "");

		if ( strValue[0] ) {
			PPP_CONNECT_TYPE_T type;

			if ( strValue[0] == '0' )
				type = CONTINUOUS;
			else if ( strValue[0] == '1' )
				type = CONNECT_ON_DEMAND;
			else if ( strValue[0] == '2' )
				type = MANUAL;
			else {
				strcpy(tmpBuf, strInvalPPPType);
				goto setErr_ppp;
			}

			Entry.pppCtype = (unsigned char)type;

			if (type == CONNECT_ON_DEMAND) {
				// PPP idle time
				strValue = boaGetVar(wp, "pppIdleTime", "");
				if ( strValue[0] ) {
					unsigned short time;
					time = (unsigned short) strtol(strValue, (char**)NULL, 10);
					if (time > 0)
						Entry.pppIdleTime = time;
					else
						Entry.pppIdleTime = 36000;	// Jenny, default PPP idle time
				}
				else
					Entry.pppIdleTime = 36000;	// Jenny, default PPP idle time
			}
		}

#ifndef CONFIG_USER_RTK_WAN_CTYPE
#ifdef DEFAULT_GATEWAY_V1
		// default route
		strValue = boaGetVar(wp, "droute", "");
		Entry.dgw = strValue[0] - '0';
		if (dr && !pdgw && Entry.dgw)
		{
			Entry.dgw = 0;	// set to disable
			strcpy(tmpBuf, strDrouteExist);
			goto setErr_ppp;
		}
#endif
#endif
		// mtu
		strValue = boaGetVar(wp, "mru", "");
		if (strValue[0]){
			intVal = strtol(strValue, (char**)NULL, 10);
			if(intVal <  65 || intVal > (Entry.cmode == CHANNEL_MODE_PPPOE ? 1492 : 1500)){
				strcpy(tmpBuf, strMruErr);
				goto setErr_ppp;
			}
			Entry.mtu = intVal;
		}
		else {
			if (Entry.cmode == CHANNEL_MODE_PPPOE)
				Entry.mtu = 1492;		// Jenny, default PPPoE MRU
			else
				Entry.mtu = 1500;		// Jenny, default PPPoA MRU
		}

#ifdef CONFIG_SPPPD_STATICIP
		fmStaticIP(wp, &Entry);
#endif

#ifdef PPPOE_PASSTHROUGH
		#if 0
		// PPPoE pass-through
		strValue = boaGetVar(wp, "poe", "");
		Entry.brmode = strValue[0] - '0';
		#endif
		// bridged mode
		strValue = boaGetVar(wp, "mode", "");
		Entry.brmode = strValue[0] - '0';
		syncPPPoEPassthrough(Entry);	// Jenny, for multisession PPPoE
#endif

		// Access concentrator name
		strValue = boaGetVar(wp, "acName", "");
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, strACName);
				goto setErr_ppp;
			}
			strncpy(Entry.pppACName, strValue, MAX_NAME_LEN-1);
			Entry.pppACName[MAX_NAME_LEN-1]='\0';
		}
		else
			Entry.pppACName[0] = '\0';	// Jenny, AC name could be empty

		// Service name
		strValue = boaGetVar(wp, "serviceName", "");
		if ( strValue[0] ) {
			if ( strlen(strValue) > MAX_NAME_LEN-1 ) {
				strcpy(tmpBuf, strServerName);
				goto setErr_ppp;
			}
			strncpy(Entry.pppServiceName, strValue, MAX_NAME_LEN-1);
			Entry.pppServiceName[MAX_NAME_LEN-1]='\0';
		}
		else
			Entry.pppServiceName[0] = '\0';	// Jenny, service name could be empty
// Mason Yu. enable_802_1p_090722
#ifdef ENABLE_802_1Q
		fm1q(wp, &Entry);
#endif

#ifdef _CWMP_MIB_
		// auto disconnect time
		strValue = boaGetVar(wp, "disconnectTime", "");
		if (strValue[0])
			Entry.autoDisTime = (unsigned short)strtol(strValue, (char**)NULL, 10);
		else
			Entry.autoDisTime = 0;
		// warn disconnect delay
		strValue = boaGetVar(wp, "disconnectDelay", "");
		if (strValue[0])
			Entry.warnDisDelay = (unsigned short)strtol(strValue, (char**)NULL, 10);
		else
			Entry.warnDisDelay = 0;
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
            //pppoe proxy
            strValue = boaGetVar(wp, "pproxy", "");
	    Entry.PPPoEProxyEnable = strValue[0] - '0';
		printf("set pppoeproxy enable %d \n",Entry.PPPoEProxyEnable);
	  //max user nums
	  strValue = boaGetVar(wp, "maxusernums", "");
		if ( strValue[0] ) {
			Entry.PPPoEProxyMaxUser=atoi(strValue);
		}
		else
			Entry.PPPoEProxyMaxUser = 4;



#endif
#ifdef WEB_ENABLE_PPP_DEBUG
	// PPP debug
	{
		unsigned char pppdbg;
		struct data_to_pass_st msg;
		strValue = boaGetVar(wp, "pppdebug", "");
		pppdbg = strValue[0] - '0';
		snprintf(msg.data, BUF_SIZE, "spppctl dbg %d debug %d", PPP_INDEX(Entry.ifIndex), pppdbg);
		write_to_pppd(&msg);
	}
#endif
		// log message
		deleteConnection(CONFIGONE, &old_entry);
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, index);

#ifdef APPLY_CHANGE
	// Mason Yu.
	restartWAN(CONFIGONE, &Entry);
#endif
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
		goto setOk_ppp;
	}

	strSubmit = boaGetVar(wp, "return", "");
	if (strSubmit[0]) {
		goto setOk_ppp;
	}

setOk_ppp:

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_ppp:
	ERR_MSG(tmpBuf);

}

/////////////////////////////////////////////////////////////////////////////
void formIPEdit(request * wp, char *path, char *query)
{
	char *strSubmit, *strValue, *strIp, *strMask, *strGW;
	char *submitUrl;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry,old_entry;
	int index;

	strSubmit = boaGetVar(wp, "save", "");
	if (strSubmit[0]) {
		strSubmit = boaGetVar(wp, "item", "");
		index = strSubmit[0] - '0';
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_ip;
		}

		memcpy(&old_entry,&Entry,sizeof(old_entry));
		// status
		strValue = boaGetVar(wp, "status", "");
		Entry.enable = strValue[0] - '0';

		// Jenny, IP unnumbered
		if (Entry.cmode == CHANNEL_MODE_RT1483) {
			strValue = boaGetVar(wp, "ipunnumber", "");
			Entry.ipunnumbered = strValue[0] - '0';

		}

		// DHCP
		strValue = boaGetVar(wp, "dhcp", "");
		if (strValue[0]) {
			if(strValue[0] == '0')
				Entry.ipDhcp = (char) DHCP_DISABLED;
			else if(strValue[0] == '1')
				Entry.ipDhcp = (char) DHCP_CLIENT;
			else {
				strcpy(tmpBuf, strInvalDHCP);
				goto setErr_ip;
			}
		}

		if (Entry.ipDhcp == (char) DHCP_DISABLED) {
			// Local IP address
			strIp = boaGetVar(wp, "ipaddr", "");
			if (strIp[0]) {
				if (!inet_aton(strIp, (struct in_addr *)&Entry.ipAddr)) {
					strcpy(tmpBuf, strInvalIP);
					goto setErr_ip;
				}
			}
			else
				if (Entry.ipunnumbered!=1) {
					strcpy(tmpBuf, strIPAddresserror);
					goto setErr_ip;
				}


			// Remote IP address
			strGW = boaGetVar(wp, "remoteip", "");
			if (strGW[0]) {
				if (!inet_aton(strGW, (struct in_addr *)&Entry.remoteIpAddr)) {
					strcpy(tmpBuf, strInvalGateway);
					goto setErr_ip;
				}
			}
			else
				if (Entry.ipunnumbered!=1) {
					strcpy(tmpBuf, strGatewayIpempty);
					goto setErr_ip;
				}

			// Subnet mask
			strMask = boaGetVar(wp, "netmask", "");
			if (strMask[0]) {
				if (!isValidNetmask(strMask, 1)) {
					strcpy(tmpBuf, strInvalMask);
					goto setErr_ip;
				}
				if (!inet_aton(strMask, (struct in_addr *)&Entry.netMask)) {
					strcpy(tmpBuf, strInvalMask);
					goto setErr_ip;
				}
				if (!isValidHostID(strIp, strMask)) {
					strcpy(tmpBuf, multilang(LANG_INVALID_IP_SUBNET_MASK_COMBINATION));
					goto setErr_ip;
				}
#ifdef DEFAULT_GATEWAY_V1
				if (!isSameSubnet(strIp, strGW, strMask)) {
					strcpy(tmpBuf, multilang(LANG_INVALID_IP_ADDRESS_IT_SHOULD_BE_LOCATED_IN_THE_SAME_SUBNET));
					goto setErr_ip;
				}
#endif
			}
			/*else
				if (Entry.ipunnumbered!=1) {
					strcpy(tmpBuf, strMaskempty);
					goto setErr_ip;
				}*/
		}

#ifndef CONFIG_USER_RTK_WAN_CTYPE
#ifdef DEFAULT_GATEWAY_V1
		// default route
		strValue = boaGetVar(wp, "droute", "");
		Entry.dgw = strValue[0] - '0';
		if (dr && !pdgw && Entry.dgw)
		{
			Entry.dgw = 0;	// set to disable
			strcpy(tmpBuf, strDrouteExist);
			goto setErr_ip;
		}
#endif
#endif
		// mtu
//		strValue = boaGetVar(wp, "mtu", "");
//		Entry.mtu = strtol(strValue, (char**)NULL, 10);
		// Masob Yu. Set mtu
		strValue = boaGetVar(wp, "mru", "");
		if (strValue[0]) {
			//Entry.mtu = strtol(strValue, (char**)NULL, 10);
			int intVal = strtol(strValue, (char**)NULL, 10);
			if (intVal <  65 || intVal > 1500){
				strcpy(tmpBuf, strMruErr);
				goto setErr_ip;
			}
			Entry.mtu = intVal;
		}
		else {
			Entry.mtu = 1500;		// Mason Yu, default MTU
		}

#ifdef PPPOE_PASSTHROUGH
		#if 0
		// PPPoE pass-through
		strValue = boaGetVar(wp, "poe", "");
		Entry.brmode = strValue[0] - '0';
		#endif
		// bridge mode
		strValue = boaGetVar(wp, "mode", "");
		Entry.brmode = strValue[0] - '0';
#endif

// Mason Yu. enable_802_1p_090722
#ifdef ENABLE_802_1Q
		fm1q(wp, &Entry);
#endif

		// log message
		deleteConnection(CONFIGONE, &old_entry);
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, index);

#ifdef APPLY_CHANGE
	restartWAN(CONFIGONE, &Entry);
#endif
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

		goto setOk_ip;
	}

	strSubmit = boaGetVar(wp, "return", "");
	if (strSubmit[0]) {
		goto setOk_ip;
	}


#ifdef CONFIG_USER_WT_146
	strSubmit = boaGetVar(wp, "bfdsave", "");
	if(strSubmit[0])
	{
		strSubmit = boaGetVar(wp, "item", "");
		index = strSubmit[0] - '0';
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_ip;
		}
		memcpy(&old_entry,&Entry,sizeof(old_entry));

		strValue = boaGetVar(wp, "bfdenable", "");
		if( strValue[0] )
		{
			Entry.bfd_enable = strValue[0] - '0';
		}

		strValue = boaGetVar(wp, "bfdopmode", "");
		if( strValue[0] )
		{
			Entry.bfd_opmode = strValue[0] - '0';
		}

		strValue = boaGetVar(wp, "bfdrole", "");
		if( strValue[0] )
		{
			Entry.bfd_role = strValue[0] - '0';
		}

		strValue = boaGetVar(wp, "bfdmult", "");
		if( strValue[0] )
		{
			int intVal = strtol(strValue, (char**)NULL, 10);
			if (intVal <  1 || intVal > 255 ){
				strcpy(tmpBuf, multilang(LANG_INVALID_DETECT_MULT_1_255));
				goto setErr_ip;
			}
			Entry.bfd_detectmult = (unsigned char)intVal;
		}

		strValue = boaGetVar(wp, "bfdtxint", "");
		if( strValue[0] )
		{
			unsigned int uintVal = strtoul(strValue, (char**)NULL, 10);
			if( (uintVal==ULONG_MAX)&&(errno==ERANGE) )
			{
				strcpy(tmpBuf, multilang(LANG_INVALID_MIN_TX_INTERVAL));
				goto setErr_ip;
			}
			Entry.bfd_mintxint = uintVal;
		}

		strValue = boaGetVar(wp, "bfdrxint", "");
		if( strValue[0] )
		{
			unsigned int uintVal = strtoul(strValue, (char**)NULL, 10);
			if( (uintVal==ULONG_MAX)&&(errno==ERANGE) )
			{
				strcpy(tmpBuf, multilang(LANG_INVALID_MIN_RX_INTERVAL));
				goto setErr_ip;
			}
			Entry.bfd_minrxint = uintVal;
		}

		strValue = boaGetVar(wp, "bfdechorxint", "");
		if( strValue[0] )
		{
			unsigned int uintVal = strtoul(strValue, (char**)NULL, 10);
			if( (uintVal==ULONG_MAX)&&(errno==ERANGE) )
			{
				strcpy(tmpBuf, multilang(LANG_INVALID_MIN_ECHO_RX_INTERVAL));
				goto setErr_ip;
			}
			Entry.bfd_minechorxint = uintVal;
		}

		strValue = boaGetVar(wp, "bfdauthtype", "");
		if( strValue[0] )
		{
			int intVal = strtol(strValue, (char**)NULL, 10);
			if (intVal <  0 || intVal > 5 ){
				strcpy(tmpBuf, multilang(LANG_INVALID_AUTH_TYPE));
				goto setErr_ip;
			}
			Entry.bfd_authtype = (unsigned char)intVal;
		}

		if( Entry.bfd_authtype!=BFD_AUTH_NONE )
		{
			strValue = boaGetVar(wp, "bfdkeyid", "");
			if( strValue[0] )
			{
				int intVal = strtol(strValue, (char**)NULL, 10);
				if (intVal <  0 || intVal > 255 ){
					strcpy(tmpBuf, multilang(LANG_INVALID_AUTH_KEY_ID_0_255));
					goto setErr_ip;
				}
				Entry.bfd_authkeyid = (unsigned char)intVal;
			}

			strValue = boaGetVar(wp, "bfdkey", "");
			if( strValue[0] )
			{
				int bfdstrlen;
				int bfd_i, bfd_idx;
				unsigned char bfdtmpbuf[4];

				//check len
				bfdstrlen=strlen( strValue );
				strcpy(tmpBuf, multilang(LANG_INVALID_AUTH_KEY_LENGTH));
				if(Entry.bfd_authtype==BFD_AUTH_PASSWORD)
				{
					if( (bfdstrlen&0x1) ||
						(bfdstrlen<(BFD_AUTH_PASS_MINKEYLEN*2)) ||
						(bfdstrlen>(BFD_AUTH_PASS_MAXKEYLEN*2)) )
						goto setErr_ip;
				}else if( (Entry.bfd_authtype==BFD_AUTH_MD5) ||
					 (Entry.bfd_authtype==BFD_AUTH_METI_MD5) )
				{
					if( bfdstrlen!=(BFD_AUTH_MD5_KEYLEN*2) )
						goto setErr_ip;
				}else if( (Entry.bfd_authtype==BFD_AUTH_SHA1) ||
					 (Entry.bfd_authtype==BFD_AUTH_METI_SHA1) )
				{
					if( bfdstrlen!=(BFD_AUTH_SHA1_KEYLEN*2) )
						goto setErr_ip;
				}else{
					goto setErr_ip;
				}
				//check key
				for( bfd_idx=0; bfd_idx<bfdstrlen; bfd_idx++ )
				{

					if(!(	((strValue[bfd_idx]>='0')&&(strValue[bfd_idx]<='9')) ||
						((strValue[bfd_idx]>='A')&&(strValue[bfd_idx]<='F')) ||
						((strValue[bfd_idx]>='a')&&(strValue[bfd_idx]<='f')) ))
					{
						strcpy(tmpBuf, multilang(LANG_INVALID_AUTH_KEY_VALUE_0_9A_FA_F));
							goto setErr_ip;
					}
				}
				bfd_i=0;
				for( bfd_idx=0; bfd_idx<bfdstrlen; bfd_idx+=2 )
				{
					bfdtmpbuf[0]=strValue[bfd_idx];
					bfdtmpbuf[1]=strValue[bfd_idx+1];
					bfdtmpbuf[2]=0;
					Entry.bfd_authkey[bfd_i] = (unsigned char) strtol(bfdtmpbuf, (char**)NULL, 16);
					bfd_i++;
				}
				Entry.bfd_authkeylen=bfd_i;
			}
		}

		strValue = boaGetVar(wp, "bfddscp", "");
		if( strValue[0] )
		{
			int intVal = strtol(strValue, (char**)NULL, 10);
			if (intVal <  0 || intVal > 63 ){
				strcpy(tmpBuf, multilang(LANG_INVALID_DSCP_0_63));
				goto setErr_ip;
			}
			Entry.bfd_dscp = (unsigned char)intVal;
		}

		if(Entry.cmode==CHANNEL_MODE_IPOE)
		{
			strValue = boaGetVar(wp, "bfdethprio", "");
			if( strValue[0] )
			{
				int intVal = strtol(strValue, (char**)NULL, 10);
				if (intVal <  0 || intVal > 7 ){
					strcpy(tmpBuf, multilang(LANG_INVALID_ETHERNET_PRIORITY_0_7));
					goto setErr_ip;
				}
				Entry.bfd_ethprio = (unsigned char)intVal;
			}
		}

		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, index);
#ifdef APPLY_CHANGE
		wt146_del_wan(&old_entry);
		wt146_create_wan(&Entry, 1);
#endif
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif

		goto setOk_ip;
	}
#endif //CONFIG_USER_WT_146


setOk_ip:

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_ip:
	//memcpy(pEntry, &tmpEntry, sizeof(tmpEntry));
	ERR_MSG(tmpBuf);

}

/////////////////////////////////////////////////////////////////////////////
void formBrEdit(request * wp, char *path, char *query)
{
	char *strSubmit, *strValue;
	char *submitUrl;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry,old_entry;
	int index;

	strSubmit = boaGetVar(wp, "save", "");
	if (strSubmit[0]) {
		strSubmit = boaGetVar(wp, "item", "");
		index = strSubmit[0] - '0';
		if (!mib_chain_get(MIB_ATM_VC_TBL, index, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_br;
		}

		memcpy(&old_entry,&Entry,sizeof(old_entry));

		// status
		strValue = boaGetVar(wp, "status", "");
		Entry.enable = strValue[0] - '0';

#ifdef PPPOE_PASSTHROUGH
		// bridged mode
		strValue = boaGetVar(wp, "mode", "");
		Entry.brmode = strValue[0] - '0';
#ifdef _CWMP_MIB_
		/*start use_fun_call_for_wan_instnum*/
		updateWanInstNum(&Entry);
		dumpWanInstNum(&Entry, "new");
		/*end use_fun_call_for_wan_instnum*/
#endif /*_CWMP_MIB_*/
#endif /*PPPOE_PASSTHROUGH*/

// Mason Yu. enable_802_1p_090722
#ifdef ENABLE_802_1Q
		fm1q(wp, &Entry);
#endif
		// log message
		deleteConnection(CONFIGONE, &old_entry);
		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, index);

#ifdef APPLY_CHANGE
	restartWAN(CONFIGONE, &Entry);
#endif
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
		goto setOk_br;
	}

	strSubmit = boaGetVar(wp, "return", "");
	if (strSubmit[0]) {
		goto setOk_br;
	}

setOk_br:

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_br:
	ERR_MSG(tmpBuf);

}

int atmVcList2(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	int intNapt, intIgmp, intQos, intDroute, intStatus;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
	char if_display_name[16];
	char	*mode, vpi[6], vci[6], *aal5Encap, *ctype;
	char	*strNapt, ipAddr[20], remoteIp[20], netmask[20], dns1Addr[20], dns2Addr[20], *strUnnum, *strDroute;
	char IpMask[20];
	char *strIgmp;
	char *strQos;
	char	userName[ENC_PPP_NAME_LEN+1], passwd[MAX_NAME_LEN];
#ifdef CONFIG_USER_PPPOE_PROXY
     char pppoeProxy[16]={0};
#endif
	const char *pppType, *strStatus;
	char *temp;
	CONN_T	conn_status;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
#ifdef DEFAULT_GATEWAY_V1
	dr = 0;
#endif

	nBytesSent += boaWrite(wp, "<tr><font size=2>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
	"<td align=center width=\"4%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
	"<td align=center width=\"7%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
	"<td align=center width=\"4%%\" bgcolor=\"#808080\"><font size=2>VPI</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>VCI</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>NAPT</td>\n"
#ifdef CONFIG_USER_RTK_WAN_CTYPE
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>Connection Type</td>\n"
#endif
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>IGMP</td>\n"
#endif
#ifdef IP_QOS
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>IP QoS</td>\n"
#endif
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
#ifdef DEFAULT_GATEWAY_V1
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>%s IP</td>\n"
#endif
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>%s%s</td>\n",
	multilang(LANG_SELECT), multilang(LANG_INTERFACE), multilang(LANG_MODE),
	multilang(LANG_ENCAPSULATION), multilang(LANG_IP_ADDRESS),
#ifdef DEFAULT_GATEWAY_V1
	multilang(LANG_REMOTE),
#endif
	multilang(LANG_SUBNET_MASK), multilang(LANG_USER), multilang(LANG_NAME));

#ifdef DEFAULT_GATEWAY_V1
	nBytesSent += boaWrite(wp, "<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n", multilang(LANG_DEFAULT_ROUTE));
#endif
	nBytesSent += boaWrite(wp, "<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Actions</td></font></tr>\n", multilang(LANG_STATUS));

	for (i=0; i<entryNum; i++) {
		struct in_addr inAddr;
		int flags;

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (MEDIA_INDEX(Entry.ifIndex) != MEDIA_ATM)
			continue;

		mode = 0;

		if (Entry.cmode == CHANNEL_MODE_PPPOE)
			mode = "PPPoE";
		else if (Entry.cmode == CHANNEL_MODE_PPPOA)
			mode = "PPPoA";
		else if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			mode = "br1483";
		else if (Entry.cmode == CHANNEL_MODE_IPOE)
			mode = "mer1483";
		else if (Entry.cmode == CHANNEL_MODE_RT1483)
			mode = "rt1483";
#ifdef CONFIG_ATM_CLIP
		else if (Entry.cmode == CHANNEL_MODE_RT1577)
			mode = "rt1577";
#endif
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
		else if (Entry.cmode == CHANNEL_MODE_6RD)
			mode = "6rd";
#endif
#ifdef DUAL_STACK_LITE
		if ((Entry.AddrMode & 0x4) == 0x4 )
			mode = "DS-Lite-mer";
#endif

		snprintf(vpi, 6, "%u", Entry.vpi);
		snprintf(vci, 6, "%u", Entry.vci);

		aal5Encap = 0;
		if (Entry.encap == 0)
			aal5Encap = "VCMUX";
		else
			aal5Encap = "LLC";

		if (Entry.napt == 0){
			strNapt = (char*)IF_OFF;
			intNapt = LANG_OFF;
		}else{
			strNapt = (char*)IF_ON;
			intNapt = LANG_ON;
		}

		ctype = 0;
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		if (Entry.applicationtype == X_CT_SRV_TR069)
			ctype = "TR069";
		else if (Entry.applicationtype == X_CT_SRV_INTERNET)
			ctype = "INTERNET";
		else if (Entry.applicationtype == X_CT_SRV_OTHER)
			ctype = "OTHER";
		else if (Entry.applicationtype == X_CT_SRV_VOICE)
			ctype = "VOICE";
		else if (Entry.applicationtype == X_CT_SRV_INTERNET+X_CT_SRV_TR069)
			ctype = "INTERNET_TR069";
		else if (Entry.applicationtype == X_CT_SRV_VOICE+X_CT_SRV_TR069)
			ctype = "VOICE_TR069";
		else if (Entry.applicationtype == X_CT_SRV_VOICE+X_CT_SRV_INTERNET)
			ctype = "VOICE_INTERNET";
		else if (Entry.applicationtype == X_CT_SRV_VOICE+X_CT_SRV_INTERNET+X_CT_SRV_TR069)
			ctype = "VOICE_INTERNET_TR069";
		else
			ctype = "None";
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
		if (Entry.enableIGMP == 0){
			strIgmp = (char*)IF_OFF;
			intIgmp = LANG_OFF;
		}else{
			strIgmp = (char*)IF_ON;
			intIgmp = LANG_ON;
		}
#else
		strIgmp = (char *)IF_OFF;
		intIgmp = LANG_OFF;
#endif

#ifdef CONFIG_USER_IP_QOS
		if (Entry.enableIpQos == 0){
			strQos = (char*)IF_OFF;
			intQos = LANG_OFF;
		}else{
			strQos = (char*)IF_ON;
			intQos = LANG_ON;
		}
#else
		strQos = (char *)IF_OFF;
		intQos = LANG_OFF;
#endif

#ifdef DEFAULT_GATEWAY_V1
		if (Entry.dgw == 0){	// Jenny, default route
			strDroute = (char*)IF_OFF;
			intDroute = LANG_OFF;
		}else{
			strDroute = (char*)IF_ON;
			intDroute = LANG_ON;
		}
		if (Entry.dgw && Entry.cmode != CHANNEL_MODE_BRIDGE)
			dr = 1;
#endif

		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
		if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA)
		{
			PPP_CONNECT_TYPE_T type;
			//strncpy(userName, Entry.pppUsername, MAX_PPP_NAME_LEN);
			memset(userName, 0, sizeof(userName));
			data_base64encode(Entry.pppUsername, userName);
			userName[ENC_PPP_NAME_LEN]='\0';
			//userName[MAX_PPP_NAME_LEN] = '\0';
			//userName[MAX_NAME_LEN] = '\0';
			//strncpy(passwd, Entry.pppPassword, MAX_NAME_LEN-1);
			memset(passwd, 0, sizeof(passwd));
			convert_to_star_string(passwd,strlen(Entry.pppPassword));
			passwd[MAX_NAME_LEN-1] = '\0';
			//passwd[MAX_NAME_LEN] = '\0';
			type = Entry.pppCtype;

			if (type == CONTINUOUS)
				pppType = "conti";
			else if (type == CONNECT_ON_DEMAND)
				pppType = "demand";
			else
				pppType = "manual";

#ifdef CONFIG_SPPPD_STATICIP
			if (Entry.cmode == CHANNEL_MODE_PPPOE && Entry.pppIp) {
				temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
				strcpy(ipAddr, temp);
				strcpy(IpMask, temp);
			}
			else {
				strcpy(ipAddr, "");
				strcpy(IpMask, "");
			}
#else
			strcpy(ipAddr, "");
			strcpy(IpMask, "");
#endif
				strcpy(remoteIp, "");
				strcpy(netmask, "");

			// set status flag
			if (Entry.enable == 0)
			{
				intStatus = LANG_DISABLED;
				conn_status = CONN_DISABLED;
			}
			else
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP)
				{
//					strStatus = (char *)IF_UP;
					intStatus = LANG_ENABLED;
					conn_status = CONN_UP;
				}
				else
				{
#ifdef CONFIG_PPP
					if (find_ppp_from_conf(ifname))
					{
//						strStatus = (char *)IF_DOWN;
						intStatus = LANG_ENABLED;
						conn_status = CONN_DOWN;
					}
					else
#endif
					{
//						strStatus = (char *)IF_NA;
						intStatus = LANG_ENABLED;
						conn_status = CONN_NOT_EXIST;
					}
				}
			}
			else
			{
//				strStatus = (char *)IF_NA;
				intStatus = LANG_ENABLED;
				conn_status = CONN_NOT_EXIST;
			}
			#ifdef CONFIG_USER_PPPOE_PROXY
			if(Entry.cmode==CHANNEL_MODE_PPPOE)
			{
				if(Entry.PPPoEProxyEnable)
					strcpy(pppoeProxy, multilang(LANG_ENABLED));
				else
					strcpy(pppoeProxy, multilang(LANG_DISABLED));
			}
			#endif
		}
		else
		{
			if (Entry.ipDhcp == (char)DHCP_DISABLED)
			{
				// static IP address
				temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
				strcpy(ipAddr, temp);

				temp = inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr));
				strcpy(remoteIp, temp);

				temp = inet_ntoa(*((struct in_addr *)Entry.netMask));	// Jenny, subnet mask
				strcpy(netmask, temp);
			}
			else
			{
				// DHCP enabled
					strcpy(ipAddr, "");
					strcpy(IpMask, "");
					strcpy(remoteIp, "");
					strcpy(netmask, "");
			}

			if (Entry.ipunnumbered)
			{
				strcpy(ipAddr, "");
				strcpy(IpMask, "");
				strcpy(netmask, "");
				strcpy(remoteIp, "");
			}

			if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			{
				strcpy(ipAddr, "");
				strcpy(IpMask, "");
				strcpy(netmask, "");
				strcpy(remoteIp, "");
				strNapt = "";
				intNapt = LANG_STR_NULL;
				strIgmp = "";
				intIgmp = LANG_STR_NULL;
				strDroute = "";
				intDroute = LANG_STR_NULL;
			}
			else if (Entry.cmode == CHANNEL_MODE_RT1483)
				strcpy(netmask, "");

			temp = inet_ntoa(*((struct in_addr *)Entry.v4dns1));
			if (strcmp(temp, "0.0.0.0"))
				strcpy(dns1Addr, temp);
			else
				strcpy(dns1Addr, "");

			temp = inet_ntoa(*((struct in_addr *)Entry.v4dns2));
			if (strcmp(temp, "0.0.0.0"))
				strcpy(dns2Addr, temp);
			else
				strcpy(dns2Addr, "");

			// set status flag
			if (Entry.enable == 0)
			{
				intStatus = LANG_DISABLED;
				conn_status = CONN_DISABLED;
			}
			else
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP)
				{
//					strStatus = (char *)IF_UP;
					intStatus = LANG_ENABLED;
					conn_status = CONN_UP;
				}
				else
				{
//					strStatus = (char *)IF_DOWN;
					intStatus = LANG_ENABLED;
					conn_status = CONN_DOWN;
				}
			}
			else
			{
//				strStatus = (char *)IF_NA;
				intStatus = LANG_ENABLED;
				conn_status = CONN_NOT_EXIST;
			}

			strcpy(userName, "");
			passwd[0]='\0';
			pppType = BLANK;
		}
		getDisplayWanName(&Entry, if_display_name);
		#ifdef CONFIG_USER_PPPOE_PROXY
		if(Entry.cmode != CHANNEL_MODE_PPPOE)
		{
			strcpy(pppoeProxy,"----");
		}
		#endif

#ifdef CONFIG_IPV6
		unsigned char 	Ipv6AddrStr[48] = {0}, RemoteIpv6AddrStr[48] = {0}, RemoteIpv6EndPointAddrStr[48] = {0};
		unsigned char Ipv6Dns1Str[48] = {0}, Ipv6Dns2Str[48] = {0};
	    unsigned char	SixrdBRv4IP[INET_ADDRSTRLEN] = {0};

		if ((Entry.AddrMode & 0x2) == 0x2)
		{
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.Ipv6Dns1, Ipv6Dns1Str, sizeof(Ipv6Dns1Str));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.Ipv6Dns2, Ipv6Dns2Str, sizeof(Ipv6Dns2Str));
		}
#ifdef DUAL_STACK_LITE
		else if ((Entry.AddrMode & 0x4) == 0x4)
		{
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.RemoteIpv6EndPointAddr, RemoteIpv6EndPointAddrStr, sizeof(RemoteIpv6EndPointAddrStr));
		}
#endif
#ifdef CONFIG_IPV6_SIT_6RD
		else if ((Entry.AddrMode & 0x8) == 0x8)
		{
			inet_ntop(PF_INET, (struct in_addr *)Entry.SixrdBRv4IP, SixrdBRv4IP, sizeof(SixrdBRv4IP));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.RemoteIpv6EndPointAddr, RemoteIpv6EndPointAddrStr, sizeof(RemoteIpv6EndPointAddrStr));
		}
#endif
#endif

         #ifdef CONFIG_USER_PPPOE_PROXY
 		nBytesSent += boaWrite (wp, "<tr>"
			"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><input type=\"radio\" name=\"select\""
#ifdef CONFIG_IPV6
			" value=\"s%d\" onClick=\"postVC2(%s,%s,'%s','%s','%s',%d, %d, %d,'%s',"
			"'%s','%s','%s',"
			"%d,%d,%d,%d,"
			"'%s','%s', '%s',"
			"%d,'%s','%s',"
			"%d, %d,"
			"%d,"
			"%d, %d,'%s','%s',"
			"'%s','%s',"
			"'%s', %d, %d, %d,"
			"'%s',%d,'%s',%d,'%s','%s')\"></td>\n",
#else
			" value=\"s%d\" onClick=\"postVC(%s,%s,'%s','%s','%s',%d, %d, %d,'%s',"
			"'%s','%s','%s',"
			"%d,%d,%d,%d,"
			"'%s','%s', '%s',"
			"%d,'%s','%s',"
			"%d, %d,"
			"%d\"></td>\n"),
#endif
			i, vpi, vci, aal5Encap, strNapt, ctype, Entry.vlan, Entry.vid, Entry.vprio, mode,
			userName, passwd, pppType,
			Entry.pppIdleTime,	Entry.PPPoEProxyEnable, Entry.ipunnumbered, Entry.ipDhcp,
			ipAddr, remoteIp, netmask,
			Entry.dnsMode, dns1Addr, dns2Addr,
			Entry.dgw, conn_status,
#ifdef CONFIG_IPV6
			Entry.enable,
			Entry.IpProtocol, Entry.AddrMode, Ipv6AddrStr, RemoteIpv6AddrStr,
			Ipv6Dns1Str, Ipv6Dns2Str,
			RemoteIpv6EndPointAddrStr, Entry.Ipv6AddrPrefixLen,  Entry.Ipv6Dhcp, Entry.Ipv6DhcpRequest,
            SixrdBRv4IP, Entry.SixrdIPv4MaskLen, Entry.SixrdPrefix, Entry.SixrdPrefixLen, Ipv6Dns1Str, Ipv6Dns2Str);
#else
			Entry.enable);
#endif
	#else

		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"2%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><input type=\"radio\" name=\"select\""
#ifdef CONFIG_IPV6
		" value=\"s%d\" onClick=\"postVC2(%s,%s,'%s','%s','%s',%d, %d, %d,"
#else
		" value=\"s%d\" onClick=\"postVC(%s,%s,'%s','%s','%s',%d, %d, %d,"
#endif
		"'%s',"
		"'%s',"
#ifdef CONFIG_IPV6
		"'%s','%s','%s','%s',"
		"%d,%d,%d,"
		"'%s',"
		"'%s', '%s',"
		"%d,'%s','%s',"
		"%d, %d, %d, %d,"
		"%d, %d,'%s','%s',"
		"'%s','%s',"
		"'%s', %d, %d, %d,"
		"'%s',%d,'%s',%d)\"></td>\n",
#else
		"'%s','%s','%s','%s',"
		"%d,%d,%d,"
		"'%s',"
		"'%s', '%s',"
		"%d,'%s','%s',"
		"%d, %d, %d, %d)\"></td>\n",
#endif
		i,vpi,vci,aal5Encap,strNapt, ctype, Entry.vlan, Entry.vid, Entry.vprio,
		strIgmp,
		strQos,
		mode, userName, passwd, pppType,
		Entry.pppIdleTime, Entry.ipunnumbered, Entry.ipDhcp,
		ipAddr,
#ifdef CONFIG_IPV6
		remoteIp, netmask,
		Entry.dnsMode, dns1Addr, dns2Addr,
		Entry.dgw, conn_status, Entry.enable, Entry.itfGroup,
		Entry.IpProtocol, Entry.AddrMode, Ipv6AddrStr, RemoteIpv6AddrStr,
		Ipv6Dns1Str, Ipv6Dns2Str,
		RemoteIpv6EndPointAddrStr, Entry.Ipv6AddrPrefixLen,  Entry.Ipv6Dhcp, Entry.Ipv6DhcpRequest,
        SixrdBRv4IP, Entry.SixrdIPv4MaskLen, Entry.SixrdPrefix, Entry.SixrdPrefixLen);
#else
		remoteIp, netmask,
		Entry.dnsMode, dns1Addr, dns2Addr,
		Entry.dgw, conn_status, Entry.enable, Entry.itfGroup);
#endif

	#endif

		nBytesSent += boaWrite(wp,
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#endif
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#endif
#ifdef IP_QOS
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#endif
		"<td align=center width=\"13%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n",
		if_display_name, mode, vpi, vci, aal5Encap, multilang(intNapt),
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		ctype,
#endif
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		multilang(intIgmp),
#endif
#ifdef IP_QOS
		multilang(intQos),
#endif
		ipAddr);
#ifdef DEFAULT_GATEWAY_V1
		nBytesSent += boaWrite(wp,
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
#else
		nBytesSent += boaWrite(wp,
#endif
		"<td align=center width=\"13%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"9%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>"
		"<script>document.write(decode64(\"%s\"))</script></b></font></td>\n"

#ifdef DEFAULT_GATEWAY_V1
		"<td align=center width=\"6%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
#endif
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\">",
#ifdef DEFAULT_GATEWAY_V1
		remoteIp,
#endif
		netmask,
		userName,

#ifdef DEFAULT_GATEWAY_V1
		multilang(intDroute),
#endif
		multilang(intStatus));
		nBytesSent += boaWrite(wp,
		"<a href=\"#?edit\" onClick=\"applyClick();editClick(%d);\">"
		"<image border=0 src=\"graphics/edit.gif\" alt=\"Edit\" /></a>", i);

		nBytesSent += boaWrite(wp,
		"<a href=\"#?delete\" onClick=\"applyClick();delClick(%d);\">"
		"<image border=0 src=\"graphics/del.gif\" alt=Delete /></td></tr>\n", i);
	}

	return nBytesSent;
}
#if 0
// add for auto-pvc search
void showPVCList(int eid, request * wp, int argc, char **argv)
{
	int i, entryNum;
	MIB_AUTO_PVC_SEARCH_T	Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);

//	boaWrite(wp, "<table border=2 width=\"500\" cellspacing=4 cellpadding=0><tr>"
	boaWrite(wp, "<tr>"
	"<td bgcolor=\"#808080\"><font size=2><b>PVC</b></td>\n"
 	"<td bgcolor=\"#808080\"><font size=2><b>VPI</b></td>\n"
	"<td bgcolor=\"#808080\"><font size=2><b>VCI</b></td>\n"
// 	"<td bgcolor=\"#808080\"><font size=2><b>QoS</b></td>\n"
//	"<td bgcolor=\"#808080\"><font size=2><b>Frame</b></td>\n"
// 	"<td bgcolor=\"#808080\"><font size=2><b>Status</b></td>\n"
//	"<td bgcolor=\"#808080\"><font size=2><b>Action</b></td></font></tr>\n");
	"</font></tr>\n");
	if(autopvc_is_up()) {

			for(i=0;i<entryNum;i++)
			{
				if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
				{
  					boaError(wp, 400, "Get chain record error!\n");
					break;
				}

				boaWrite(wp, "<tr>"
				"<td><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
//			 	"<td><font size=2><b>ubr</b></td>\n"
//		 		"<td><font size=2><b>br1483</b></td>\n"
//			 	"<td><font size=2><b>active</b></td>\n"
//			 	"<td><input type=\"submit\" value=\"Delete\" name=\"deletePVC\"></td></tr>\n",
				"</td></tr>\n"),

			 	i, Entry.vpi, Entry.vci);
			}

	}

}
#endif

void ShowAutoPVC(int eid, request * wp, int argc, char **argv)
{
//#if AutoPvcSearch
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
	int i, entryNum;
	MIB_AUTO_PVC_SEARCH_T	Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);

	boaWrite(wp, "<br>"
//	"<input type=\"hidden\" name=\"enablepvc\" value=0>"
	"<input type=\"hidden\" name=\"enablepvc\" value=%d>"
	"<input type=\"hidden\" name=\"addVPI\" value=0>  "
	"<input type=\"hidden\" name=\"addVCI\" value=0>      	"
	"<table>"
	"  <tr>"
	"<td width=\"70%%\" colspan=2><font size=2><b>       "
   	"<input type=\"checkbox\" name=\"autopvc\" value="
   	, autopvc_is_up());

	if (autopvc_is_up()) {
		boaWrite(wp, "\"ON\" checked enabled");
	} else {
		boaWrite(wp, "\"OFF\" enabled");
	}

//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	boaWrite(wp,
	" ONCLICK=updatepvcState() >&nbsp;&nbsp;%s</b>"
     	"</td>", multilang(LANG_ENABLE_AUTO_PVC_SEARCH)
//#endif

//#if defined(AUTO_PVC_SEARCH_TR068_OAMPING)
#else
	boaWrite(wp,
		" ONCLICK=updatepvcState2() >&nbsp;&nbsp;%s</b>"
		"</td>", multilang(LANG_ENABLE_AUTO_PVC_SEARCH)
#endif
		);

	boaWrite(wp,
		"<td width=\"30%%\" colspan=2><font size=2><b>"
   	"<input type=\"submit\" name=\"autopvcapply\" onClick=\"return autopvcEnableClick()\" value= \"%s\" >"
		"</td>"
		"</tr>"

//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	"<tr>"
	"<td><font size=2><b>VPI: </b>"
	"<input type=\"text\" name=\"autopvcvpi\" size=\"4\" maxlength=\"3\" value=0>&nbsp;&nbsp;"
	"</td>"

	"<td><b>VCI: </b>"
	"<input type=\"text\" name=\"autopvcvci\" size=\"6\" maxlength=\"5\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
	"</td>"

	"<td>"
	"<input type=\"submit\" value=\"%s\" name=\"autopvcadd\" onClick=\"return autopvcCheckClick()\">			"
	"</td>"

	"<td>"
	"<input type=\"submit\" value=\"%s\" name=\"autopvcdel\" onClick=\"return autopvcCheckClick()\">			"
	"</td>"
	"</tr>"
#endif
	"</table>"

//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	"<br>  "
	"<table border=\"1\" width=\"30%%\">"
	"<tr><font size=2><b>%s:</b></font></tr> "
#endif
	, multilang(LANG_APPLY)
#ifdef AUTO_PVC_SEARCH_TABLE
	, multilang(LANG_ADD), multilang(LANG_DELETE), multilang(LANG_CURRENT_AUTO_PVC_TABLE)
#endif
	);

//#if defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
#ifdef AUTO_PVC_SEARCH_TABLE
	boaWrite(wp, "<tr>"
	"<td bgcolor=\"#808080\"><font size=2><b>PVC</b></td>\n"
 	"<td bgcolor=\"#808080\"><font size=2><b>VPI</b></td>\n"
	"<td bgcolor=\"#808080\"><font size=2><b>VCI</b></td>\n"
	"</font></tr>\n");
	if(autopvc_is_up()) {

			for(i=0;i<entryNum;i++)
			{
				if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
				{
  					boaError(wp, 400, "Get chain record error!\n");
					break;
				}

				boaWrite(wp, "<tr>"
				"<td><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
			 	"<td><font size=2><b>%d</b></td>\n"
				"</td></tr>\n",

			 	i, Entry.vpi, Entry.vci);
			}

	}

	boaWrite(wp, "</table>");
#endif
#endif
}

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN) || defined(WLAN_WISP)
#define _PTS			", new it(\"%s\", \"%s\")"
#define _PTI			", new it(\"%s\", %d)"
#define _PTUL			", new it(\"%s\", %lu)"
#define _PME(name)		#name, Entry.name
#define _PMEIP(name)	#name, strcpy(name, inet_ntoa(*(struct in_addr*)&(Entry.name)))

void initPageWaneth(int eid, request * wp, int argc, char ** argv)
{
	MIB_CE_ATM_VC_T Entry, *pEntry;
	char wanname[MAX_NAME_LEN];
	unsigned char	ipAddr[16];		//IP?∞Â?
	unsigned char	remoteIpAddr[16];	//Áº∫Á?ÁΩëÂÖ≥
	unsigned char	netMask[16];	//Â≠êÁ??©Á?
	unsigned char	v4dns1[16], v4dns2[16];
#ifdef CONFIG_IPV6
	unsigned char	Ipv6AddrStr[48]={0};
	unsigned char Ipv6Dns1Str[48] = {0};
	unsigned char Ipv6Dns2Str[48] = {0};
	unsigned char	RemoteIpv6AddrStr[48]={0};
	unsigned char	RemoteIpv6EndPointAddrStr[48]={0};
#ifdef CONFIG_IPV6_SIT_6RD
	unsigned char	SixrdBRv4IP[INET_ADDRSTRLEN]={0};
#endif
#endif
	int mibtotal, i;
	MEDIA_TYPE_T mType;
#ifdef	WLAN_WISP
	char *wlanname;
	unsigned char wlanset=0;
#endif
	unsigned char pppUsername[ENC_PPP_NAME_LEN+1];
	unsigned char pppPassword[MAX_NAME_LEN];

	if(strncmp(wanif,"ptm",3)==0)
		mType = MEDIA_PTM;
	else
		mType = MEDIA_ETH;

#ifdef	WLAN_WISP
	if (boaArgs(argc, argv, "%s", &wlanname)==1)
	{
		if ( !strcmp(wlanname, "wlan") ){
				mType = MEDIA_WLAN;
				wlanset = 1;
		}
	}
#endif

	pEntry = &Entry;
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0; i<mibtotal; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)pEntry))
			continue;

		if( MEDIA_INDEX(pEntry->ifIndex) != mType )
			continue;

#ifdef CONFIG_TR142_MODULE
		if(pEntry->omci_configured)
			continue;
#endif

		//generate wan name
		if (MEDIA_INDEX(pEntry->ifIndex) == MEDIA_ETH)
			sprintf(wanname, "%s%d", ALIASNAME_MWNAS, VC_INDEX(pEntry->ifIndex));
#ifdef CONFIG_PTMWAN
		else if (MEDIA_INDEX(pEntry->ifIndex) == MEDIA_PTM)
			sprintf(wanname, "%s%d", ALIASNAME_MWPTM, VC_INDEX(pEntry->ifIndex));
#endif /*CONFIG_PTMWAN*/
#ifdef WLAN_WISP
		else if(MEDIA_INDEX(pEntry->ifIndex) == MEDIA_WLAN)
			sprintf( wanname, "wlan%d-vxd", ETH_INDEX(pEntry->ifIndex));
#endif


#ifdef CONFIG_IPV6
		if ((pEntry->AddrMode & 0x2) == 0x2)
		{
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Dns1, Ipv6Dns1Str, sizeof(Ipv6Dns1Str));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Dns2, Ipv6Dns2Str, sizeof(Ipv6Dns2Str));
		}
#ifdef DUAL_STACK_LITE
		else if ((pEntry->AddrMode & 0x4) == 0x4)
		{
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6EndPointAddr, RemoteIpv6EndPointAddrStr, sizeof(RemoteIpv6EndPointAddrStr));
		}
#endif
#ifdef CONFIG_IPV6_SIT_6RD
		else if ((pEntry->AddrMode & 0x8) == 0x8)
		{
			inet_ntop(PF_INET, (struct in_addr *)pEntry->SixrdBRv4IP, SixrdBRv4IP, sizeof(SixrdBRv4IP));
		}
#endif
#endif

		strcpy(v4dns1, inet_ntoa(*((struct in_addr *)pEntry->v4dns1)));
		if (strcmp(v4dns1, "0.0.0.0")==0)
			strcpy(v4dns1, "");

		strcpy(v4dns2, inet_ntoa(*((struct in_addr *)pEntry->v4dns2)));
		if (strcmp(v4dns2, "0.0.0.0")==0)
			strcpy(v4dns2, "");

		memset(pppUsername, 0, sizeof(pppUsername));
		data_base64encode(pEntry->pppUsername, pppUsername);
		pppUsername[ENC_PPP_NAME_LEN]='\0';
		memset(pppPassword, 0, sizeof(pppPassword));
		convert_to_star_string(pppPassword,strlen(pEntry->pppPassword));
		pppPassword[MAX_NAME_LEN-1]='\0';

		boaWrite(wp, "push(new it_nr(\"%s\" "_PTI _PTI _PTI _PTI _PTI "",
			wanname, _PME(applicationtype), _PME(cmode), _PME(brmode), _PME(napt), _PME(enableIGMP));
#ifdef WLAN_WISP
if(!wlanset)
#endif
#ifdef CONFIG_USER_IP_QOS
			boaWrite(wp, _PTI, _PME(enableIpQos));
#else
			boaWrite(wp, _PTI, "enableIpQos", 0);
#endif
#ifdef WLAN_WISP
	if(wlanset)
			boaWrite(wp, _PTI _PTI, _PME(dgw), _PME(enable));
	else
#endif
			boaWrite(wp, _PTI _PTI _PTI _PTI _PTI _PTI, _PME(dgw), _PME(vlan), _PME(vid), _PME(vprio), _PME(enable), _PME(mtu));
#ifdef CONFIG_IPV6
		boaWrite(wp, _PTI, _PME(IpProtocol));
#endif
		boaWrite(wp, _PTI _PTS _PTS _PTS,
			_PME(ipDhcp), _PMEIP(ipAddr), _PMEIP(remoteIpAddr), _PMEIP(netMask));

		boaWrite(wp, _PTI _PTS _PTS, _PME(dnsMode), "v4dns1", v4dns1, "v4dns2", v4dns2);

#ifdef CONFIG_USER_DHCP_OPT_GUI_60
		boaWrite(wp, _PTI _PTS
					_PTI _PTI _PTI _PTI _PTS
					_PTI _PTS _PTS _PTS _PTS
			, _PME(enable_opt_60), _PME(opt60_val)
			, _PME(enable_opt_61), _PME(iaid), _PME(duid_type), _PME(duid_ent_num), _PME(duid_id)
			, _PME(enable_opt_125), _PME(manufacturer), _PME(product_class), _PME(model_name), _PME(serial_num));
#endif
		boaWrite(wp, _PTS _PTS _PTI _PTI,
			"pppUsername", pppUsername, "pppPassword", pppPassword, _PME(pppCtype), _PME(pppIdleTime));
		boaWrite(wp, _PTI _PTS _PTS,
			_PME(pppAuth), _PME(pppACName), _PME(pppServiceName));
#ifdef CONFIG_IPV6
		boaWrite(wp, _PTI _PTS _PTS _PTS _PTS _PTS _PTI _PTI _PTI,
			_PME(AddrMode), "Ipv6Addr", Ipv6AddrStr,
			"RemoteIpv6Addr", RemoteIpv6AddrStr,
			"RemoteIpv6EndPointAddr", RemoteIpv6EndPointAddrStr,
			"Ipv6Dns1", Ipv6Dns1Str, "Ipv6Dns2", Ipv6Dns2Str,
			_PME(Ipv6AddrPrefixLen), _PME(Ipv6Dhcp), _PME(Ipv6DhcpRequest));
#endif
#ifdef WLAN_WISP
	if(!wlanset)
#endif
		boaWrite(wp, _PTI, _PME(itfGroup));

#ifdef WLAN_WISP
		if(wlanset){
			boaWrite(wp, _PTI, "wlwan", ETH_INDEX(pEntry->ifIndex));
		}
#endif

#ifdef CONFIG_IPV6_SIT_6RD
		boaWrite(wp, _PTI _PTI _PTS _PTS,
			_PME(SixrdIPv4MaskLen), _PME(SixrdPrefixLen),
			_PME(SixrdPrefix), "SixrdBRv4IP", SixrdBRv4IP);
#endif
		boaWrite(wp, "));\n");

	}
}
#endif

void ShowChannelMode(int eid, request * wp, int argc, char **argv)
{
	char *name;

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return;
	}
#ifdef BRIDGE_ONLY_ON_WEB

	boaWrite(wp, "<font size=2>"
	" <b>%s:</b>"
	"<select size=\"1\" name=\"adslConnectionMode\" >\n"
	"	  <option selected value=\"0\">1483 Bridged</option>\n",
	multilang(LANG_CHANNEL_MODE));
//	checkWrite(eid, wp, argc, argv);
	boaWrite(wp, "</select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
#else
	#ifdef CONFIG_RTL8672_SAR
	if ( !strcmp(name, "adslcmode") ) {
		boaWrite(wp, "<font size=2>"
		" <b>%s:</b>"
		"<select size=\"1\" name=\"adslConnectionMode\" onChange=\"adslConnectionModeSelection()\">\n"
		"	  <option selected value=\"0\">1483 Bridged</option>\n"
		"	  <option value=\"1\">1483 MER</option>\n"
#ifdef CONFIG_PPP
		"	  <option value=\"2\">PPPoE</option>\n"
#endif
		, multilang(LANG_CHANNEL_MODE));
		boaWrite(wp,
#ifdef CONFIG_PPP
		"	  <option value=\"3\">PPPoA</option>\n"
#endif
		"	  <option value=\"4\">1483 Routed</option>\n"
		);
	}
	#endif
	#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
	if ( !strcmp(name, "ethcmode") ) {
		boaWrite(wp, "<font size=2>"
		" <b>%s:</b>"
		"<select size=\"1\" name=\"adslConnectionMode\" onChange=\"adslConnectionModeSelection(false)\">\n"
		"	  <option selected value=\"0\">Bridged</option>\n"
		"	  <option value=\"1\">IPoE</option>\n"
#ifdef CONFIG_PPP
		"	  <option value=\"2\">PPPoE</option>\n"
#endif
		, multilang(LANG_CHANNEL_MODE));
	}
	#endif
	#if defined (WLAN_WISP)
	if ( !strcmp(name, "wlancmode") ) {
		boaWrite(wp, "<font size=2>"
		" <b>%s:</b>"
		"<select size=\"1\" name=\"adslConnectionMode\" onChange=\"adslConnectionModeSelection(false)\">\n"
		"	  <option selected value=\"1\">IPoE</option>\n"
#ifdef CONFIG_PPP
		"	  <option value=\"2\">PPPoE</option>\n"
#endif
		, multilang(LANG_CHANNEL_MODE));
	}
	#endif
	checkWrite(eid, wp, argc, argv);
	boaWrite(wp, "</select></font>\n");
#endif
}

void ShowBridgeMode(int eid, request * wp, int argc, char **argv)
{
#ifdef PPPOE_PASSTHROUGH
	boaWrite(wp, "<tr id=\"br_row\"><td><font size=2><b>%s: </b><input type=\"checkbox\" name=\"br\" size=\"2\" maxlength=\"2\" value=\"ON\" onClick=brClicked()></font></td></tr>"
	"<tr><td colspan=2><font size=2>\n"
	" <b>%s:</b>\n"
	"<select size=\"1\" name=\"brmode\" id=\"brmode\">\n"
	"	<option value=\"0\">Bridged Ethernet (Transparent Bridging)</option>\n"
	"	<option value=\"1\">Bridged PPPoE(implies Bridged Ethernet)</option>\n"
	"</select></font></td></tr>"
	, multilang(LANG_ENABLE_BRIDGE), multilang(LANG_BRIDGE_MODE));
#endif //PPPOE_PASSTHROUGH
}


void ShowNAPTSetting(int eid, request * wp, int argc, char **argv)
{
#ifdef BRIDGE_ONLY_ON_WEB
	boaWrite(wp, "<input type=\"hidden\"  name=\"naptEnabled\">\n");
	return;
#else
	boaWrite(wp, "<font size=2><b>%s: </b><input type=\"checkbox\" name=\"naptEnabled\"\n"
			"size=\"2\" maxlength=\"2\" value=\"ON\" onClick=naptClicked()>",
			multilang(LANG_ENABLE_NAPT));
#endif
}

void ShowIGMPSetting(int eid, request * wp, int argc, char **argv)
{
#if !defined(CONFIG_IGMPPROXY_MULTIWAN) || defined(BRIDGE_ONLY_ON_WEB)
	boaWrite(wp, "<td><input type=\"hidden\"  name=\"igmpEnabled\"></td>\n");
#else
	boaWrite(wp, "<td><font size=2><b>%s: </b><input type=\"checkbox\" name=\"igmpEnabled\"\n"
			"size=\"2\" maxlength=\"2\" value=\"ON\"></td>", multilang(LANG_ENABLE_IGMP_PROXY));
#endif
}

void ShowPPPIPSettings(int eid, request * wp, int argc, char **argv)
{
	char *key;
	int onATM;

	if (boaArgs(argc, argv, "%s", &key)==1 && !strcmp(key, "atm"))
		onATM = 1;
	else
		onATM = 0;

#ifdef BRIDGE_ONLY_ON_WEB
	boaWrite(wp, "<input type=\"hidden\"  name=\"pppUserName\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"pppPassword\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"pppConnectType\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"pppIdleTime\">\n");

	boaWrite(wp, "<input type=\"hidden\"  name=\"ipMode\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"ipMode\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"ip\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"remoteIp\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"netmask\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"ipUnnum\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"droute\">\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"droute\">\n");
#else
	boaWrite(wp, "<table id=tbl_ppp border=0 width=800 cellspacing=4 cellpadding=0>\n"
		"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
		"<tr><th align=\"left\"><font size=2><b>%s:</b></th>\n"
		"	<td><font size=2><b>%s%s:</b></td>\n"
		"	<td><font size=2><input type=\"text\" name=\"pppUserName\" size=\"16\" maxlength=\"%d\"></td>\n"
		"	<td><font size=2><b>%s:</b></td>\n"
		"	<td><font size=2><input type=\"password\" name=\"pppPassword\" size=\"10\" maxlength=\"%d\"></td>\n"
		"</tr>\n"
		"<tr><th></th>\n"
		"	<td><font size=2><b>%s:</b></td>\n"
		"	<td><font size=2><select size=\"1\" name=\"pppConnectType\" onChange=\"pppTypeSelection()\">\n"
		"		<option selected value=\"0\">Continuous</option>\n"
		"		<option value=\"1\">Connect on Demand</option>\n"
		"		<option value=\"2\">Manual</option>\n"
		"		</select>\n"
		"	</td>\n"
		"	<td><font size=2><b>%s:</b></td>\n"
		"	<td><font size=2><input type=\"text\" name=\"pppIdleTime\" size=\"10\" maxlength=\"10\"></td>\n",
		multilang(LANG_PPP_SETTINGS), multilang(LANG_USER), multilang(LANG_NAME),
		MAX_PPP_NAME_LEN, multilang(LANG_PASSWORD), MAX_NAME_LEN - 1,
		multilang(LANG_TYPE), multilang(LANG_IDLE_TIME_SEC)
	);

	if (!onATM) {
		boaWrite(wp, "</tr>\n"
			"<tr><th></th>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><select size=\"1\" name=\"auth\">\n"
			"		<option selected value=\"0\">AUTO</option>\n"
			"		<option value=\"1\">PAP</option>\n"
			"		<option value=\"2\">CHAP</option>\n"
			"		</select>\n"
			"	</td>\n",
			multilang(LANG_AUTHENTICATION_METHOD));
		boaWrite(wp, "</tr>\n"
			"<tr><th></th>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"text\" name=\"acName\" size=\"16\" maxlength=\"%d\"></td>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"password\" name=\"serviceName\" size=\"10\" maxlength=\"%d\"></td>\n",
			multilang(LANG_AC_NAME), MAX_NAME_LEN, multilang(LANG_SERVICE_NAME), MAX_NAME_LEN);
	}
#ifdef CONFIG_USER_PPPOE_PROXY
	boaWrite(wp, "</tr>\n"
		"<tr><th></th>\n"
		"	<td><font size=2><b>%s:</b></td>\n"
		"	<td><font size=2><input type=\"checkbox\" name=\"enableProxy\" value=\"1\">\n"
		"	</td>\n"
		"	<td><font size=2><b>%s:</b></td>\n"
		"	<td><font size=2><input type=\"text\" name=\"maxProxyUser\" size=\"10\" maxlength=\"3\"></td>\n",
		multilang(LANG_ENABLE_PPPOE_PROXY), multilang(LANG_MAX_PROXY_USER));
#endif

	boaWrite(wp, "</tr>\n</table>\n"
		"<table id=tbl_ip border=0 width=800 cellspacing=4 cellpadding=0>\n"
		"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
		"<tr><th align=\"left\"><font size=2><b>%s:</b></th>\n"
		"\n"
		"	<td><font size=2><b>%s:</b></td>\n"
		"	<td><font size=2>\n"
		"	<input type=\"radio\" value=\"0\" name=\"ipMode\" checked onClick=\"ipTypeSelection(0)\">Fixed IP\n"
		"	<font size=2>\n"
		"	<input type=\"radio\" value=\"1\" name=\"ipMode\" onClick=\"ipTypeSelection(0)\">DHCP</td>\n"
#ifdef DEFAULT_GATEWAY_V2
		"	<td><font size=2><b>%s</b>\n"
		"		<input type=\"checkbox\" name=\"ipUnnum\" size=\"2\" maxlength=\"2\" value=\"ON\"  onClick=\"ipModeSelection()\"></td>\n"
#endif
		"</tr>\n"
		"<tr><th></th>\n"
		"	<td><font size=2><b>%s %s:</b></td>\n"
		"	<td><font size=2><input type=\"text\" name=\"ip\" size=\"10\" maxlength=\"15\"></td>\n"
#ifdef DEFAULT_GATEWAY_V1
		"	<td><font size=2><b>%s %s:</b></td>\n"
		"	<td><font size=2><input type=\"text\" name=\"remoteIp\" size=\"10\" maxlength=\"15\"></td>\n"
		"</tr>\n"
		"<tr><th></th>\n"
#endif
		"	<td><font size=2><b>%s:</b></td>\n"
		"	<td><font size=2><input type=\"text\" name=\"netmask\" size=\"10\" maxlength=\"15\"></td>\n"
#ifdef DEFAULT_GATEWAY_V1
		"	<td><font size=2><b>%s</b>\n"
		"		<input type=\"checkbox\" name=\"ipUnnum\" size=\"2\" maxlength=\"2\" value=\"ON\"  onClick=\"ipModeSelection()\"></td>\n"
#endif
		"</tr>\n", multilang(LANG_WAN_IP_SETTINGS), multilang(LANG_TYPE)
#ifdef DEFAULT_GATEWAY_V2
		, multilang(LANG_UNNUMBERED)
#endif
		, multilang(LANG_LOCAL), multilang(LANG_IP_ADDRESS)
#ifdef DEFAULT_GATEWAY_V1
		, multilang(LANG_REMOTE), multilang(LANG_IP_ADDRESS)
#endif
		, multilang(LANG_SUBNET_MASK)
#ifdef DEFAULT_GATEWAY_V1
		, multilang(LANG_UNNUMBERED)
#endif
		);
	boaWrite(wp,
	"<tr><th></th>\n"
       "	<td><font size=2><b>Request DNS:</b>\n"
	"		<input type=\"radio\" value=\"1\" name=\"dnsMode\" onClick='dnsModeClicked()'>Enable\n"
	"		<input type=\"radio\" value=\"0\" name=\"dnsMode\" checked onClick='dnsModeClicked()'>Disable\n"
	"	</td>\n"
       "</tr>\n"
       "<tr><th></th>\n"
	"     <td><font size=2><b>Primary DNS Server:</b></td>\n"
	"     <td><font size=2><input type=\"text\" name=\"dns1\" size=\"18\" maxlength=\"15\" value=></td>\n"
	"</tr>\n"
	"<tr><th></th>\n"
	"     <td><font size=2><b>Secondary DNS Server:</b></td>\n"
	"     <td><font size=2><input type=\"text\" name=\"dns2\" size=\"18\" maxlength=\"15\" value=></td>\n"
	"</tr>\n"
	);
	boaWrite(wp, "</table>\n");

#ifdef CONFIG_USER_DHCP_OPT_GUI_60
	// DHCP client send option function only available on PTM & ETHWAN currently.
	if(!onATM)
	{
		// Option table header
		boaWrite(wp,
			"<table id=tbl_dhcp_opt border=0 width=800 cellspacing=4 cellpadding=0>\n"
			"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
			"<tr><th colspan=5 align=\"left\"><font size=2><b>%s:</b></th></tr>\n"
			, multilang(LANG_DHCP_OPTION_SETTINGS) );

		//Option 60
		boaWrite(wp,
			"<tr><td colspan=3><input type=\"checkbox\" name=\"enable_opt_60\" value=\"1\">"
			"	<font size=2><b>%s: </b>\n"
			"</tr>\n"
			"<tr><td width=50></td>\n"
			"	<td width=150><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"text\" name=\"opt60_val\" maxlength=\"32\"></td>\n"
			"</tr>\n"
			, multilang(LANG_ENABLE_DHCP_OPTION_60), multilang(LANG_VENDOR_ID));

		//Option 61
		boaWrite(wp,
			"<tr><td colspan=3><input type=\"checkbox\" name=\"enable_opt_61\" value=\"1\">"
			"	<font size=2><b>%s: </b>\n"
			"</tr>\n"
			"<tr><td></td>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"text\" name=\"iaid\" maxlength=\"32\"></td>\n"
			"</tr>\n"
			"<tr><td></td>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"radio\" value=\"1\" name=\"duid_type\" onClick=\"showDuidType2(0)\" >%s</td>\n"
			"</tr>\n"
			"<tr><td></td><td></td><td><font size=2><input type=\"radio\" value=\"2\" name=\"duid_type\" onclick=\"showDuidType2(1)\" >%s</td></tr>\n"
			"<tr id=\"duid_t2_ent\"><td></td><td align=\"right\"><font size=2>%s:</td><td><input type=\"text\" name=\"duid_ent_num\"></td></tr>\n"
			"<tr id=\"duid_t2_id\"><td></td><td align=\"right\"><font size=2>%s:</td><td><input type=\"text\" name=\"duid_id\"></td></tr>\n"
			"<tr><td></td><td></td><td><font size=2><input type=\"radio\" value=\"3\" name=\"duid_type\" onclick=\"showDuidType2(0)\">%s</td></tr>\n"
			, multilang(LANG_ENABLE_DHCP_OPTION_61), multilang(LANG_IAID), multilang(LANG_DUID)
			, multilang(LANG_LINK_LAYER_ADDRESS_PLUSE_TIME), multilang(LANG_ENTERPRISE_NUMBER_AND_IDENTIFIER)
			, multilang(LANG_ENTERPRISE_NUMBER), multilang(LANG_IDENTIFIER)
			, multilang(LANG_LINKLAYER_ADDRESS));

		//Option 125
		boaWrite(wp,
			"<tr><td colspan=3><input type=\"checkbox\" name=\"enable_opt_125\" value=\"1\">"
			"	<font size=2><b>%s: </b>\n"
			"</tr>\n"
			"<tr><td></td>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"text\" name=\"manufacturer\" maxlength=\"32\"></td>\n"
			"</tr>\n"
			"<tr><td></td>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"text\" name=\"product_class\" maxlength=\"32\"></td>\n"
			"</tr>\n"
			"<tr><td></td>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"text\" name=\"model_name\" maxlength=\"32\"></td>\n"
			"</tr>\n"
			"<tr><td></td>\n"
			"	<td><font size=2><b>%s:</b></td>\n"
			"	<td><font size=2><input type=\"text\" name=\"serial_num\" maxlength=\"32\"></td>\n"
			"</tr>\n"
			, multilang(LANG_ENABLE_DHCP_OPTION_125), multilang(LANG_MANUFACTURER_OUI)
			, multilang(LANG_PRODUCT_CLASS), multilang(LANG_MODEL_NAME), multilang(LANG_SERIAL_NUMBER)
			);
		boaWrite(wp, "</table>\n");
	}
#endif //CONFIG_USER_DHCP_OPT_GUI_60
#endif //BRIDGE_ONLY_ON_WEB
}

void ShowDefaultGateway(int eid, request * wp, int argc, char **argv)
{
#ifdef DEFAULT_GATEWAY_V2
	boaWrite(wp, "	<td colspan=4><input type=\"radio\" name=\"droute\" value=\"1\" onClick='autoDGWclicked()'>"
	"<font size=2><b>&nbsp;&nbsp;%s</b></td>\n</tr>\n"
	"<tr><th></th>\n	<td colspan=4><input type=\"radio\" name=\"droute\" value=\"0\" onClick='autoDGWclicked()'>"
	"<font size=2><b>&nbsp;&nbsp;%s:</b></td>\n</tr>\n",
	multilang(LANG_OBTAIN_DEFAULT_GATEWAY_AUTOMATICALLY),
	multilang(LANG_USE_THE_FOLLOWING_DEFAULT_GATEWAY));
	boaWrite(wp, "<div id='gwInfo'>\n"
	"<tr><th></th>\n	<td>&nbsp;</td>\n"
	"	<td colspan=2><font size=2><input type=\"radio\" name='gwStr' value=\"0\" onClick='gwStrClick()'><b>&nbsp;%s:&nbsp;&nbsp;</b></td>\n"
	"	<td><div id='id_dfltgwy'><font size=2><input type='text' name='dstGtwy' maxlength=\"15\" size=\"10\"></div></td>\n</tr>\n"
	"<tr><th></th>\n	<td>&nbsp;</td>\n"
	"	<td colspan=2><font size=2><input type=\"radio\" name='gwStr' value=\"1\" onClick='gwStrClick()'><b>&nbsp;%s:&nbsp;&nbsp;</b></td>\n"
	"	<td><div id='id_wanIf'><font size=2><select name='wanIf'>",
	multilang(LANG_USE_REMOTE_WAN_IP_ADDRESS),
	multilang(LANG_USE_WAN_INTERFACE));
	ifwanList(eid, wp, argc, argv);
	boaWrite(wp, "</select></div></td>\n</tr>\n</div>\n</table>\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"remoteIp\">\n");
#else
	boaWrite(wp, "<div id='gwInfo'>\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"gwStr\">\n");
	boaWrite(wp, "<div id='id_dfltgwy'>\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"dstGtwy\"></div>\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"gwStr\">\n");
	boaWrite(wp, "<div id='id_wanIf'>\n");
	boaWrite(wp, "<input type=\"hidden\"  name=\"wanIf\"></div>\n</div>\n");
#endif
}

void ShowConnectionType(int eid, request * wp, int argc, char **argv)
{
#ifdef CONFIG_USER_RTK_WAN_CTYPE
		boaWrite(wp, "<tr>\n"
					"		<td><font size=2><b>%s:</b>\n"
					"			<select size=1 name=\"ctype\">\n"
					"				<option  value=4>%s</option>\n"
					"				<option  value=1>TR069</option>\n"
					"				<option  value=2>INTERNET</option>\n"
					"				<option  value=3>INTERNET_TR069</option>\n"
					"				<option  value=8>VOICE</option>\n"
					"				<option  value=9>VOICE_TR069</option>\n"
					"				<option  value=10>VOICE_INTERNET</option>\n"
					"				<option  value=11>VOICE_INTERNET_TR069</option>\n"
					"			</select>\n"
					"		</td>\n"
					"	</tr>\n" ,multilang(LANG_CONNECTION_TYPE), multilang(LANG_OTHER));
#endif
}

void ShowIpProtocolType(int eid, request * wp, int argc, char **argv)
{
#ifdef CONFIG_IPV6
	char vChar=-1;

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
	if (vChar == 0)
		boaWrite(wp, "<table id=\"tbprotocol\"  border=0 width=\"800\" cellspacing=4 cellpadding=0>\n"
				"	<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
				"	<tr nowrap id=TrIpProtocolType>\n"
				"		<td width=\"120px\"><font size=2><b>IP %s:</b></td>\n"
				"		<td><select id=\"IpProtocolType\" style=\"WIDTH: 130px\" onChange=\"protocolChange()\" name=\"IpProtocolType\">\n"
				"			<option value=\"1\" > IPv4</option>\n"
				"			</select>\n"
				"		</td>\n"
				"	</tr>\n"
				"</table>\n", multilang(LANG_PROTOCOL));
	else
		boaWrite(wp, "<table id=\"tbprotocol\"  border=0 width=\"800\" cellspacing=4 cellpadding=0>\n"
				"	<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
				"	<tr nowrap id=TrIpProtocolType>\n"
				"		<td width=\"120px\"><font size=2><b>IP %s:</b></td>\n"
				"		<td><select id=\"IpProtocolType\" style=\"WIDTH: 130px\" onChange=\"protocolChange()\" name=\"IpProtocolType\">\n"
				"			<option value=\"1\" > IPv4</option>\n"
				"			<option value=\"2\" > IPv6</option>\n"
				"			<option value=\"3\" > IPv4/IPv6</option>\n"
				"			</select>\n"
				"		</td>\n"
				"	</tr>\n"
				"</table>\n", multilang(LANG_PROTOCOL));
#endif
}

void ShowIPV6Settings(int eid, request * wp, int argc, char **argv)
{
#ifdef CONFIG_IPV6
	char vChar=-1;

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
	if (vChar == 0)
		boaWrite(wp, "<div id=IPV6_wan_setting style=\"display:none\">\n");
	else
		boaWrite(wp, "<div id=IPV6_wan_setting style=\"display:block\">\n");

		boaWrite(wp, "<table id=\"tbipv6wan\" border=0 width=\"800\" cellspacing=4 cellpadding=0>\n"
				"	<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
				"	<tr><th align=\"left\"><font size=2><b>IPv6 %s:</b></th></tr>\n"
				"	<tr nowrap id=TrIpv6AddrType>\n"
				"		<td width=\"120px\"><font size=2><b>%s:</b></td>\n"
				"		<td>\n"
				"			<input type=\"checkbox\" value=\"ON\" name=\"slacc\" id=\"send3\"><font size=2><b>Slaac</b>\n"
				"	        </td>\n"
				"	        <td>\n"
				"			<input type=\"checkbox\" value=\"ON\" name=\"staticIpv6\" id=\"send4\" onclick=\"ipv6StaticUpdate()\"><font size=2><b>Static</b>\n"
				"	        </td>\n"
				"	</tr>\n"
				"</table>\n", multilang(LANG_WAN_SETTING), multilang(LANG_ADDRESS_MODE));

		boaWrite(wp, "<div id=secIPv6Div style=\"display:none\">\n"
				"<table border=0 cellspacing=4 cellpadding=0>\n"
				"	<tr id=TrIpv6Addr>\n"
				"		<td width=\"120px\"><font size=2><b>%s:</b></td>\n"
				"		<td><font size=2><input  id=Ipv6Addr maxLength=39 size=36 name=Ipv6Addr>\n"
				"		/\n"
				"		<font size=2><input id=Ipv6PrefixLen maxLength=3 size=3 name=Ipv6PrefixLen>\n"
                          	"		</td>\n"
				"	</tr>\n"
				"	<tr id=TrIpv6Gateway>\n"
				"		<td width=\"120px\"><font size=2><b>IPv6 %s:</b></td>\n"
				"		<td><font size=2><input  id=Ipv6Gateway  maxLength=39 size=36 name=Ipv6Gateway></td>\n"
				"	</tr>\n"
				"	<tr>\n"
				"		<td width=\"120px\"><font size=2><b>%s IPv6 DNS:</b></td>\n"
				"		<td><font size=2><input  maxLength=39 size=36 name=Ipv6Dns1></td>\n"
				"	</tr>\n"
				"		<td width=\"120px\"><font size=2><b>%s IPv6 DNS:</b></td>\n"
				"		<td><font size=2><input  maxLength=39 size=36 name=Ipv6Dns2></td>\n"
				"	</tr>\n"
				"</table>\n"
				"</div>\n", multilang(LANG_IPV6_ADDRESS), multilang(LANG_GATEWAY), multilang(LANG_PRIMARY), multilang(LANG_SECONDARY));

		boaWrite(wp, "<br>\n"
				"<div  id=\"dhcp6c_ctrlblock\"  style=\"display:block\">\n"
				"<table id=\"tbdhcpv6\" border=0 cellspacing=4 cellpadding=0>\n"
				"	<tr nowrap><td width=\"120px\"><font size=2><b>%s:</b></td>\n"
				"	<td><input type=\"checkbox\" value=\"ON\" name=\"itfenable\" id=\"itfenable\" onclick=\"dhcp6cEnable()\" ></td>\n"
				"	</tr>\n"
				"</table>\n", multilang(LANG_ENABLE_DHCPV6_CLIENT));

		boaWrite(wp, "	<div  id=\"dhcp6c_block\"  style=\"display:none\">\n"
				"	<table  border=0 cellspacing=4 cellpadding=0>\n"
				"	  <tr nowrap>\n"
				"	      <td width=\"150px\"><font size=2><b>%s:</b></td>\n"
				"	      <td ></td>\n"
				"	  </tr>\n"
				"	  <tr nowrap>\n"
				"	     <td width=\"150px\"><font size=2><b>&nbsp;</b></td>\n"
				"	      <td>\n"
				"			<input type=\"checkbox\" value=\"ON\" name=\"iana\" id=\"send1\"><font size=2><b>%s</b>\n"
				"	      </td>\n"
				"	  </tr>\n", multilang(LANG_REQUEST_OPTIONS),
				multilang(LANG_REQUEST_ADDRESS));
		boaWrite(wp, "	   <tr>\n"
				"	     <td width=\"150px\"><font size=2><b>&nbsp;</b></td>\n"
				"	      <td>\n"
				"			<input type=\"checkbox\" value=\"ON\" name=\"iapd\" id=\"send2\"><font size=2><b>%s</b>\n"
				"	      </td>\n"
				"	  </tr>\n"
				"	 </table>\n"
				"</table>\n"
				"</div>\n"
				"</div>\n", multilang(LANG_REQUEST_PREFIX));
		boaWrite(wp, "</div>\n");
#endif
}

void ShowDSLiteSetting(int eid, request * wp, int argc, char **argv)
{
#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
		boaWrite(wp, "<div id=DSLiteDiv style=\"display:none\">\n"
				"<table border=0 width=\"800\" cellpadding=\"0px\" cellspacing=\"4px\">\n"
				"   <tr><td colspan=5><hr size=2 align=top></td></tr>\n"
				"	<tr><td width=\"300px\"><font size=2><b>%s:</td></tr>\n"
				"	<tr>\n"
				"		<td width=\"300px\"><font size=2>%s:</td>\n"
				"		<td><input  id=\"DSLiteLocalIP\" maxLength=39 size=36 name=\"DSLiteLocalIP\" style=\"width:150px \"></td>\n"
				"	</tr>\n", multilang(LANG_DS_LITE_WAN_CONFIG), multilang(LANG_LOCAL_IPV6_ADDRESS));
		boaWrite(wp, "	<tr>\n"
				"		<td width=\"300px\"><font size=2>%s:</td>\n"
				"		<td><input  id=\"DSLiteRemoteIP\"  maxLength=39 size=36 name=\"DSLiteRemoteIP\" style=\"width:150px \"></td>\n"
				"	</tr>\n"
				"	<tr>\n"
				"		<td width=\"300px\"><font size=2>%s:</td>\n"
				"		<td><input  id=\"DSLiteGateway\"  maxLength=39 size=36 name=\"DSLiteGateway\" style=\"width:150px \"></td>\n"
				"	</tr>\n"
				"</table>\n"
				"</div>\n", multilang(LANG_REMOTE_IPV6_END_POINT_ADDRESS), multilang(LANG_GATEWAY_IPV6_ADDRESS));
#endif
}

static void ShowWlanPortCheck(request * wp)
{
#ifdef WLAN_SUPPORT
	int i;
	if(wlan_en[0]){
		boaWrite(wp, "<tr nowrap><td><font size=2>");
		boaWrite(wp, "<input type=checkbox name=chkpt>%s</font></td></tr>\n", wlan_itf[0]);
	}
	else
		boaWrite(wp, "<input type=hidden name=chkpt>\n");

	for (i=0; i<=(PMAP_WLAN0_VAP3-PMAP_WLAN0_VAP0); i++) {
		if(wlan_en[i+1]==0)
			boaWrite(wp, "<input type=hidden name=chkpt>\n");
		else{
			if (!(i&0x1))
				boaWrite(wp, "<tr nowrap>");
			boaWrite(wp, "<td><font size=2><input type=checkbox name=chkpt>%s</font></td>\n", wlan_itf[i+1]);
			if ((i&0x1) || (i+1) == WLAN_MBSSID_NUM)
					boaWrite(wp, "</tr>\n");
		}
	}

	if(wlan_en[PMAP_WLAN1-PMAP_WLAN0]){
		boaWrite(wp, "<tr nowrap><td><font size=2>");
		boaWrite(wp, "<input type=checkbox name=chkpt>%s</font></td></tr>\n", wlan_itf[PMAP_WLAN1-PMAP_WLAN0]);
	}
	else
		boaWrite(wp, "<input type=hidden name=chkpt>\n");

	for (i=0; i<=(PMAP_WLAN1_VAP3-PMAP_WLAN1_VAP0); i++) {
		if(wlan_en[i+PMAP_WLAN1_VAP0-PMAP_WLAN0]==0)
			boaWrite(wp, "<input type=hidden name=chkpt>\n");
		else{
			if (!(i&0x1))
				boaWrite(wp, "<tr nowrap>");
			boaWrite(wp, "<td><font size=2><input type=checkbox name=chkpt>%s</font></td>\n", wlan_itf[i+PMAP_WLAN1_VAP0-PMAP_WLAN0]);
			if ((i&0x1) || (i+1) == WLAN_MBSSID_NUM)
					boaWrite(wp, "</tr>\n");
		}
	}
#endif // of WLAN_SUPPORT
}

void ShowPortMapping(int eid, request * wp, int argc, char **argv)
{
#ifdef NEW_PORTMAPPING
	int i;

	boaWrite(wp, "<div id=div_pmap>\n<table id=tbl_pmap border=0 width=800 cellspacing=4 cellpadding=0>\n"
			"<tr><td colspan=5><hr size=2 align=top></td></tr>\n"
			"<tr nowrap><td width=150px><font size=2><b>%s</b></font>"
			"</td><td>&nbsp;</td></tr>\n", multilang(LANG_PORT_MAPPING));
	boaWrite(wp, "<tr nowrap>");
	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3; i++) {
		if (i < SW_LAN_PORT_NUM) {
			if (!(i&0x1))
				boaWrite(wp, "<tr nowrap>");
			boaWrite(wp, "<td><font size=2><input type=checkbox name=chkpt>LAN_%d</font></td>\n", i+1);
			if ((i&0x1) || (i+1) == SW_LAN_PORT_NUM)
				boaWrite(wp, "</tr>\n");
		}
		else
			boaWrite(wp, "<input type=hidden name=chkpt>\n");
	}

	ShowWlanPortCheck(wp);

	boaWrite(wp, "</table>\n</div>\n");
#endif
}

void ShowPortBaseFiltering(int eid, request * wp, int argc, char **argv)
{
	int i;

	boaWrite(wp, "<div id=div_pmap>\n<table id=tbl_pmap border=0 width=800 cellspacing=4 cellpadding=0>\n"
			"<tr nowrap><td width=350px><font size=2><b>%s</b></font>"
			"</td><td>&nbsp;</td></tr>\n", multilang(LANG_FILTER_DHCP_DISCOVER_PACKET));
	boaWrite(wp, "<tr nowrap>");
	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3; i++) {
		if (i < SW_LAN_PORT_NUM) {
			if (!(i&0x1))
				boaWrite(wp, "<tr nowrap>");
			boaWrite(wp, "<td><font size=2><input type=checkbox name=chkpt>LAN_%d</font></td>\n", i+1);
			if ((i&0x1) || (i+1) == SW_LAN_PORT_NUM)
				boaWrite(wp, "</tr>\n");
		}
		else
			boaWrite(wp, "<input type=hidden name=chkpt>\n");
	}

	ShowWlanPortCheck(wp);

	boaWrite(wp, "</table>\n</div>\n");
}

void Show6rdSetting(int eid, request * wp, int argc, char **argv)
{
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
		boaWrite(wp, "<div id=6rdDiv style=\"display:none\">\n"
				"<table border=0 width=\"800\" cellpadding=\"0px\" cellspacing=\"4px\">\n"
				"   <tr><td colspan=5><hr size=2 align=top></td></tr>\n"
				"	<tr><td width=\"300px\"><font size=2><b>%s:</td></tr>\n"
				"	<tr>\n"
				"		<td width=\"300px\"><font size=2>%s:</td>\n"
				"		<td><input  id=\"SixrdBRv4IP\" maxLength=39 size=36 name=\"SixrdBRv4IP\" style=\"width:150px \"></td>\n"
				"	</tr>\n", multilang(LANG_6RD_CONFIG), multilang(LANG_BOARD_ROUTER_V4_ADDRESS));

		boaWrite(wp, "	<tr>\n"
				"		<td width=\"300px\"><font size=2>%s:</td>\n"
				"		<td><input  id=\"SixrdIPv4MaskLen\"  maxLength=39 size=36 name=\"SixrdIPv4MaskLen\" style=\"width:150px \"></td>\n"
				"	</tr>\n"
				, multilang(LANG_6RD_IPV4_MASK_LEN));

		boaWrite(wp, "	<tr>\n"
				"		<td width=\"300px\"><font size=2>%s:</td>\n"
				"		<td><input  id=\"SixrdPrefix\"  maxLength=39 size=36 name=\"SixrdPrefix\" style=\"width:150px \"></td>\n"
				"	</tr>\n"
				"	<tr>\n"
				"		<td width=\"300px\"><font size=2>%s:</td>\n"
				"		<td><input  id=\"SixrdPrefixLen\"  maxLength=39 size=36 name=\"SixrdPrefixLen\" style=\"width:150px \"></td>\n"
				"	</tr>\n"
				"</table>\n"
				"</div>\n", multilang(LANG_6RD_PREFIX_EX), multilang(LANG_6RD_PREFIX_LENGTH));
#endif
}

int getWanIfDisplay(int eid, request * wp, int argc, char **argv)
{
	if(strncmp(wanif,"eth",3)==0)
		boaWrite(wp, "Ethernet");
	else if(strncmp(wanif,"ptm",3)==0)
		boaWrite(wp, "PTM");
	else if(strncmp(wanif,"pon",3)==0)
		boaWrite(wp, "PON");
	return 0;
}

void formWanRedirect(request * wp, char *path, char *query)
{
	char *redirectUrl;
	char *strWanIf;

	redirectUrl= boaGetVar(wp, "redirect-url", "");
	strWanIf= boaGetVar(wp, "if", "");
	if(strWanIf[0]){
		strcpy(wanif,strWanIf);
	}

	if(redirectUrl[0])
		boaRedirectTemp(wp,redirectUrl);
}

#ifdef WLAN_WISP
void ShowWispWanItf(int eid, request * wp, int argc, char **argv)
{
	char wlan_mode, rptEnabled;
	int idx, orig_idx;

	orig_idx = wlan_idx;

	boaWrite(wp, "<font size=2>"
		"<select size=\"1\" name=\"wispItf\" >\n");

	for(idx = 0; idx<NUM_WLAN_INTERFACE; idx++){
		wlan_idx = idx;

		mib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
#ifdef WLAN_UNIVERSAL_REPEATER
		mib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
#else
		rptEnabled = 0;
#endif
		if( (wlan_mode==AP_MODE || wlan_mode==AP_WDS_MODE) && rptEnabled)
			boaWrite(wp, "	  <option value=\"%d\">wlan%d-vxd</option>\n", idx, idx);

	}
	boaWrite(wp, "</select></font>\n");
	wlan_idx = orig_idx;
}
#endif
#ifdef WLAN_WISP
void initWispWanItfStatus(int eid, request * wp, int argc, char **argv)
{
	char wlan_mode, rptEnabled;
	int idx, orig_idx;
	MIB_CE_MBSSIB_T Entry;

	orig_idx = wlan_idx;

	for (idx=0; idx<NUM_WLAN_INTERFACE; idx++) {
		wlan_idx = idx;
		wlan_getEntry(&Entry, idx);

		boaWrite(wp, "\twlanMode[%d]=%d;\n", idx, Entry.wlanMode);
		mib_get(MIB_REPEATER_ENABLED1, (void *)&rptEnabled);
		boaWrite(wp, "\trptEnabled[%d]=%d;\n", idx, rptEnabled);
	}

	wlan_idx = orig_idx;
}
#endif

