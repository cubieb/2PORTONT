/*
 *      Web server handler routines for NET
 *      Authors:
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <semaphore.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../uClibc/include/linux/autoconf.h"
#endif
#include <config/autoconf.h>
#if defined(CONFIG_EPON_FEATURE) || defined(CONFIG_RTK_RG_INIT)
#include <rtk_rg_define.h>
#endif
#ifdef CONFIG_RTK_OMCI_V1
#include <omci_api.h>
#include <gos_type.h>
#endif

#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif

#include "../webs.h"
#include "fmdefs.h"
#include "mib.h"
#include "utility.h"
// Mason Yu. t123
#include "webform.h"
#define UBR_WITHOUT_PCR		0
#define UBR_WITH_PCR		1
#define CBR			2
#define NO_RT_VBR		3
#define RT_VBR			4

#define MAX_SRV_NUM		8
int web2mib_srv[MAX_SRV_NUM] = {
	X_CT_SRV_TR069|X_CT_SRV_INTERNET,
	X_CT_SRV_INTERNET,
	X_CT_SRV_TR069,
	X_CT_SRV_OTHER,
	X_CT_SRV_VOICE,
	X_CT_SRV_TR069|X_CT_SRV_VOICE,
	X_CT_SRV_VOICE|X_CT_SRV_INTERNET,
	X_CT_SRV_TR069|X_CT_SRV_VOICE|X_CT_SRV_INTERNET
};

typedef enum {
	CONN_DISABLED=0,
	CONN_NOT_EXIST,
	CONN_DOWN,
	CONN_UP
} CONN_T;

#ifdef DEFAULT_GATEWAY_V1
static int dr=0, pdgw=0;
#endif

static char wanif[10];
static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
static const char IF_DISABLED[] = "Disabled";
static const char IF_ENABLE[]="Enable";
static const char IF_ON[] = "On";
static const char IF_OFF[] = "Off";

#ifdef CONFIG_IPV6
int retrieveIPv6Record(request * wp, MIB_CE_ATM_VC_Tp pEntry)
{
	char *strValue;
	struct in6_addr ip6Addr;

	// IpProtocolType(ipv4/ipv6, ipv4, ipv6)
	strValue = boaGetVar(wp, "IpProtocolType", "");
	if (strValue[0]) {
		pEntry->IpProtocol = strValue[0] - '0';
	}

	strValue = boaGetVar(wp, "AddrMode", "");
	if (strValue[0]) {
		pEntry->AddrMode = (char)atoi(strValue);
	}

	pEntry->Ipv6Dhcp = 0;
	if(pEntry->AddrMode == IPV6_WAN_STATIC)
	{
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

		// IPv6 DNS 1
		strValue = boaGetVar(wp, "Ipv6Dns1", "");
		if(strValue[0]) {
			inet_pton(PF_INET6, strValue, &ip6Addr);
			memcpy(pEntry->Ipv6Dns1, &ip6Addr, sizeof(pEntry->Ipv6Dns1));
		}

		// IPv6 DNS 2
		strValue = boaGetVar(wp, "Ipv6Dns2", "");
		if(strValue[0]) {
			inet_pton(PF_INET6, strValue, &ip6Addr);
			memcpy(pEntry->Ipv6Dns2, &ip6Addr, sizeof(pEntry->Ipv6Dns2));
		}
	}
	else if(pEntry->AddrMode == IPV6_WAN_DHCP) // Enable DHCPv6 client
	{
		pEntry->Ipv6Dhcp = 1;
		// Request Address
		strValue = boaGetVar(wp, "iana", "");
		if ( !gstrcmp(strValue, "ON"))
			pEntry->Ipv6DhcpRequest |= 1;

	}

	strValue = boaGetVar(wp, "iapd", "");
	if ( !gstrcmp(strValue, "ON"))
		pEntry->Ipv6DhcpRequest |= 2;

#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
	// ds-lite enable
	if(pEntry->IpProtocol==IPVER_IPV6){
	strValue = boaGetVar(wp, "dslite_enable", "");
	if ( !gstrcmp(strValue, "ON")){
		pEntry->dslite_enable = 1;

			strValue = boaGetVar(wp, "dslite_aftr_mode", "");
			if(strValue[0])
				pEntry->dslite_aftr_mode = strValue[0] - '0';

			printf("dslite_aftr_mode=%d\n",pEntry->dslite_aftr_mode);

			if(pEntry->dslite_aftr_mode == IPV6_DSLITE_MODE_STATIC){
				strValue = boaGetVar(wp, "dslite_aftr_hostname", "");
				if(strValue[0])
					strncpy(pEntry->dslite_aftr_hostname,strValue,sizeof(pEntry->dslite_aftr_hostname));
				printf("dslite_aftr_hostname=%s\n",pEntry->dslite_aftr_hostname);
			}

		}
	}
#endif

/*
#ifdef DUAL_STACK_LITE
	// Get parameter for DS-Lite
	else if ((pEntry->AddrMode & IPV6_WAN_DSLITE) == IPV6_WAN_DSLITE)
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
*/
	return 0;
}
#endif

int atmVcList2(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
#ifdef CTC_WAN_NAME
	char ctcWanName[MAX_WAN_NAME_LEN];
#endif
	char if_display_name[16];
	char	*mode, vpi[6], vci[6], *aal5Encap;
	char	*strNapt, ipAddr[20], remoteIp[20], netmask[20], *strUnnum, *strDroute;
	char IpMask[20];
	char *strIgmp;
	char *strQos;
#ifdef CONFIG_GUI_WEB
	char	userName[P_MAX_NAME_LEN], passwd[P_MAX_NAME_LEN];
#else
	char	userName[MAX_PPP_NAME_LEN+1], passwd[MAX_NAME_LEN];
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
     char pppoeProxy[10]={0};
#endif
	const char	*pppType, *strStatus;
	char	*temp;
	CONN_T	conn_status;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
#ifdef DEFAULT_GATEWAY_V1
	dr = 0;
#endif

	nBytesSent += boaWrite(wp, "<tr><font size=2>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Select</td>\n"
	"<td align=center width=\"4%%\" bgcolor=\"#808080\"><font size=2>Inf</td>\n"
	"<td align=center width=\"7%%\" bgcolor=\"#808080\"><font size=2>Mode</td>\n"
	"<td align=center width=\"4%%\" bgcolor=\"#808080\"><font size=2>VPI</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>VCI</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Encap</td>\n"
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>NAPT</td>\n"
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>IGMP</td>\n"
#endif
#ifdef IP_QOS
	"<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>IP QoS</td>\n"
#endif
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>IP Addr</td>\n"
#ifdef DEFAULT_GATEWAY_V1
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>Remote IP</td>\n"
#endif
	"<td align=center width=\"13%%\" bgcolor=\"#808080\"><font size=2>Subnet Mask</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>User Name</td>\n");

#ifdef DEFAULT_GATEWAY_V1
	nBytesSent += boaWrite(wp, "<td align=center width=\"3%%\" bgcolor=\"#808080\"><font size=2>DRoute</td>\n");
#endif
	nBytesSent += boaWrite(wp, "<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Status</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\"><font size=2>Actions</td></font></tr>\n");

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

		snprintf(vpi, 6, "%u", Entry.vpi);
		snprintf(vci, 6, "%u", Entry.vci);

		aal5Encap = 0;
		if (Entry.encap == 0)
			aal5Encap = "VCMUX";
		else
			aal5Encap = "LLC";

		if (Entry.napt == 0)
			strNapt = (char*)IF_OFF;
		else
			strNapt = (char*)IF_ON;

#ifdef CONFIG_IGMPPROXY_MULTIWAN
		if (Entry.enableIGMP == 0)
			strIgmp = (char*)IF_OFF;
		else
			strIgmp = (char*)IF_ON;
#else
		strIgmp = (char *)IF_OFF;
#endif
#ifdef IP_QOS
		if (Entry.enableIpQos == 0)
			strQos = (char*)IF_OFF;
		else
			strQos = (char*)IF_ON;
#else
		strQos = (char *)IF_OFF;
#endif

#ifdef DEFAULT_GATEWAY_V1
		if (Entry.dgw == 0)	// Jenny, default route
			strDroute = (char*)IF_OFF;
		else
			strDroute = (char*)IF_ON;
		if (Entry.dgw && Entry.cmode != CHANNEL_MODE_BRIDGE)
			dr = 1;
#endif

		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
		if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA)
		{
			PPP_CONNECT_TYPE_T type;
#ifdef CONFIG_GUI_WEB
			strncpy(userName, Entry.pppUsername, P_MAX_NAME_LEN-1);
			userName[P_MAX_NAME_LEN-1] = '\0';
			//userName[P_MAX_NAME_LEN] = '\0';
			strncpy(passwd, Entry.pppPassword, P_MAX_NAME_LEN-1);
			passwd[P_MAX_NAME_LEN-1] = '\0';
			//passwd[P_MAX_NAME_LEN] = '\0';
#else
			strncpy(userName, Entry.pppUsername, MAX_PPP_NAME_LEN);
			userName[MAX_PPP_NAME_LEN] = '\0';
			//userName[MAX_NAME_LEN] = '\0';
			strncpy(passwd, Entry.pppPassword, MAX_NAME_LEN-1);
			passwd[MAX_NAME_LEN-1] = '\0';
			//passwd[MAX_NAME_LEN] = '\0';
#endif
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
				strStatus = IF_DISABLED;
				conn_status = CONN_DISABLED;
			}
			else
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP)
				{
//					strStatus = (char *)IF_UP;
					strStatus = IF_ENABLE;
					conn_status = CONN_UP;
				}
				else
				{
					if (find_ppp_from_conf(ifname))
					{
//						strStatus = (char *)IF_DOWN;
						strStatus = IF_ENABLE;
						conn_status = CONN_DOWN;
					}
					else
					{
//						strStatus = (char *)IF_NA;
						strStatus = IF_ENABLE;
						conn_status = CONN_NOT_EXIST;
					}
				}
			}
			else
			{
//				strStatus = (char *)IF_NA;
				strStatus = IF_ENABLE;
				conn_status = CONN_NOT_EXIST;
			}
			#ifdef CONFIG_USER_PPPOE_PROXY
			if(Entry.cmode==CHANNEL_MODE_PPPOE)
			{
				if(Entry.PPPoEProxyEnable)
					strcpy(pppoeProxy,"Enable");
				else
					strcpy(pppoeProxy,"Disabled");
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
				strIgmp = "";
				strDroute = "";
			}
			else if (Entry.cmode == CHANNEL_MODE_RT1483)
				strcpy(netmask, "");

			// set status flag
			if (Entry.enable == 0)
			{
				strStatus = IF_DISABLED;
				conn_status = CONN_DISABLED;
			}
			else
			if (getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_UP)
				{
//					strStatus = (char *)IF_UP;
					strStatus = IF_ENABLE;
					conn_status = CONN_UP;
				}
				else
				{
//					strStatus = (char *)IF_DOWN;
					strStatus = IF_ENABLE;
					conn_status = CONN_DOWN;
				}
			}
			else
			{
//				strStatus = (char *)IF_NA;
				strStatus = IF_ENABLE;
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

#ifdef CTC_WAN_NAME
		{
			memset(ctcWanName, 0, sizeof(ctcWanName));
			getWanName(&Entry, ctcWanName);
		}

#endif

#ifdef CONFIG_IPV6
		unsigned char 	Ipv6AddrStr[48], RemoteIpv6AddrStr[48];

		if ((Entry.AddrMode & IPV6_WAN_STATIC) == IPV6_WAN_STATIC)
		{
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
		} else {
			strcpy(Ipv6AddrStr, "");
			strcpy(RemoteIpv6AddrStr, "");
		}
#endif

         #ifdef CONFIG_USER_PPPOE_PROXY
 		nBytesSent += boaWrite (wp, "<tr>"
			"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><input type=\"radio\" name=\"select\""
#ifdef CONFIG_IPV6
			" value=\"s%d\" onClick=\"postVC2(%s,%s,'%s','%s','%s','%s','%s','%s',%d,%d,%d,%d,'%s','%s', '%s', %d, %d, %d"
			"%d, %d,'%s','%s', %d, %d, %d)\"></td>\n",
#else
			" value=\"s%d\" onClick=\"postVC(%s,%s,'%s','%s','%s','%s','%s','%s',%d,%d,%d,%d,'%s','%s', '%s', %d, %d, %d)\"></td>\n"),
#endif
			i, vpi, vci, aal5Encap, strNapt, mode,
			userName, passwd, pppType,
			Entry.pppIdleTime,
			Entry.PPPoEProxyEnable,
			Entry.ipunnumbered,
			Entry.ipDhcp, ipAddr,
			remoteIp,
			netmask, Entry.dgw, conn_status,
#ifdef CONFIG_IPV6
			Entry.enable,
			Entry.IpProtocol, Entry.AddrMode, Ipv6AddrStr, RemoteIpv6AddrStr, Entry.Ipv6AddrPrefixLen,  Entry.Ipv6Dhcp, Entry.Ipv6DhcpRequest);
#else
			Entry.enable);
#endif
	#else
		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"2%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><input type=\"radio\" name=\"select\""
#ifdef CONFIG_IPV6
		" value=\"s%d\" onClick=\"postVC2(%s,%s,'%s','%s',"
#else
		" value=\"s%d\" onClick=\"postVC(%s,%s,'%s','%s',"
#endif
		"'%s',"
		"'%s',"
#ifdef CONFIG_IPV6
		"'%s','%s','%s','%s',%d,%d,%d,'%s','%s', '%s', %d, %d, %d,"
		"%d, %d,'%s','%s', %d, %d, %d)\"></td>\n",
#else
		"'%s','%s','%s','%s',%d,%d,%d,'%s','%s', '%s', %d, %d, %d)\"></td>\n"),
#endif
		i,vpi,vci,aal5Encap,strNapt,
		strIgmp,
		strQos,
		mode,userName,passwd,pppType,
		Entry.pppIdleTime,Entry.ipunnumbered,Entry.ipDhcp,ipAddr,
#ifdef CONFIG_IPV6
		remoteIp, netmask, Entry.dgw, conn_status, Entry.enable,
		Entry.IpProtocol, Entry.AddrMode, Ipv6AddrStr, RemoteIpv6AddrStr, Entry.Ipv6AddrPrefixLen,  Entry.Ipv6Dhcp, Entry.Ipv6DhcpRequest);
#else
		remoteIp, netmask, Entry.dgw, conn_status, Entry.enable);
#endif

	#endif

#ifdef CTC_WAN_NAME
		nBytesSent += boaWrite(wp,
		"<td align=center width=\"14%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"9%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"6%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"19%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n",
		ctcWanName, mode,
		vpi, vci,
		aal5Encap, strNapt,
		ipAddr
		);
#else
		nBytesSent += boaWrite(wp,
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"7%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"4%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#endif
#ifdef IP_QOS
		"<td align=center width=\"3%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
#endif
		"<td align=center width=\"13%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n",
		if_display_name, mode, vpi, vci, aal5Encap, strNapt,
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		strIgmp,
#endif
#ifdef IP_QOS
		strQos,
#endif
		ipAddr);
#endif
#ifdef DEFAULT_GATEWAY_V1
		nBytesSent += boaWrite(wp,
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"
#else
		nBytesSent += boaWrite(wp,
#endif
		"<td align=center width=\"13%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"9%%\" bgcolor=\"#C0C0C0\" style=\"word-break:break-all\"><font size=\"2\"><b>%s</b></font></td>\n"

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
		strDroute,
#endif
		strStatus);
		nBytesSent += boaWrite(wp,
		"<a href=\"#?edit\" onClick=\"editClick(%d)\">"
		"<image border=0 src=\"graphics/edit.gif\" alt=\"Edit\" /></a>", i);

		nBytesSent += boaWrite(wp,
		"<a href=\"#?delete\" onClick=\"delClick(%d)\">"
		"<image border=0 src=\"graphics/del.gif\" alt=Delete /></td></tr>\n", i);
	}

	return nBytesSent;
}

#ifdef BR_ROUTE_ONEPVC
/*
 *	Set device interface of mibentry to be the same as the one of Entry's.
 *	Entry and mibentry share the same pvc. One for bridged mode and the
 *	other for routed mode.
 */
void modifyifIndex(MIB_CE_ATM_VC_Tp Entry,MIB_CE_ATM_VC_Tp mibentry)
{
	if(Entry->cmode != CHANNEL_MODE_BRIDGE && mibentry->cmode == CHANNEL_MODE_BRIDGE)
		mibentry->ifIndex = TO_IFINDEX(MEDIA_INDEX(mibentry->ifIndex), DUMMY_PPP_INDEX, VC_INDEX(Entry->ifIndex));
	if(Entry->cmode == CHANNEL_MODE_BRIDGE && (mibentry->cmode == CHANNEL_MODE_PPPOE || mibentry->cmode == CHANNEL_MODE_PPPOA))
		mibentry->ifIndex = TO_IFINDEX(MEDIA_INDEX(mibentry->ifIndex), PPP_INDEX(mibentry->ifIndex), VC_INDEX(Entry->ifIndex));
	else
		mibentry->ifIndex = TO_IFINDEX(MEDIA_INDEX(mibentry->ifIndex), DUMMY_PPP_INDEX, VC_INDEX(Entry->ifIndex));
}

/*
 *	Disable br_route_flag of pvc-Entry which share the same pvc of pEntry.
 */
void modify_Br_Rt_entry(MIB_CE_ATM_VC_Tp pEntry)
{
	int num,i;
	MIB_CE_ATM_VC_T entry;

	num=mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<num;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&entry)!=1)
			continue;
		if(entry.br_route_flag==1 && entry.vpi==pEntry->vpi&&entry.vci==pEntry->vci){
			entry.br_route_flag=0;
			mib_chain_update(MIB_ATM_VC_TBL,&entry,i);
		}
	}
}
#endif

static int mib2web(MIB_CE_ATM_VC_Tp mibentry,struct atmvc_entryx* webentry)
{
	int i;

	webentry->svtype = mibentry->svtype;
	webentry->vpi = mibentry->vpi;
	webentry->vci = mibentry->vci;
	webentry->pcr = mibentry->pcr;
	webentry->scr = mibentry->scr;
	webentry->mbs = mibentry->mbs;
	webentry->encap = (mibentry->encap==1?0:1);
	webentry->napt = mibentry->napt;
	webentry->mtu =	mibentry->mtu;
	if(mibentry->cmode==CHANNEL_MODE_PPPOE){
		if(mibentry->mtu > 1492)
			webentry->mtu = 1492;
	}
	if(mibentry->cmode==CHANNEL_MODE_PPPOA||mibentry->cmode==CHANNEL_MODE_RT1483)
		return -1;
	webentry->cmode = mibentry->cmode;
 	//webentry->brmode = mibentry->brmode;
	webentry->AddrMode = mibentry->AddrMode;
	strcpy(webentry->pppUsername,mibentry->pppUsername);
	strcpy(webentry->pppPassword, mibentry->pppPassword);
	webentry->pppAuth = mibentry->pppAuth;
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
	strcpy(webentry->pppServiceName, mibentry->pppServiceName);
#endif
/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
	strcpy(webentry->pppACName, mibentry->pppACName);
/*star:20080718 END*/
	webentry->pppCtype = mibentry->pppCtype;
	webentry->ipDhcp = mibentry->ipDhcp;
	*(unsigned long*)&(webentry->ipAddr) = ntohl(*(unsigned long*)&(mibentry->ipAddr));
	*(unsigned long*)&(webentry->remoteIpAddr) = ntohl(*(unsigned long*)&(mibentry->remoteIpAddr));
	*(unsigned long*)&(webentry->netMask) = ntohl(*(unsigned long*)&(mibentry->netMask));
	webentry->dnsMode = mibentry->dnsMode;
	*(unsigned long*)&(webentry->v4dns1) = ntohl(*(unsigned long*)&(mibentry->v4dns1));
	*(unsigned long*)&(webentry->v4dns2) = ntohl(*(unsigned long*)&(mibentry->v4dns2));
#if 1
	webentry->vlan = mibentry->vlan;
	webentry->vid = mibentry->vid;
	webentry->vprio = mibentry->vprio;
	webentry->vpass = mibentry->vpass;
#endif
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	webentry->qos = mibentry->enableIpQos;
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	webentry->PPPoEProxyEnable = mibentry->PPPoEProxyEnable;
	webentry->PPPoEProxyMaxUser = mibentry->PPPoEProxyMaxUser;
#endif
#ifdef CTC_WAN_NAME
	for (i=0; i<MAX_SRV_NUM; i++) {
		if (web2mib_srv[i] == (mibentry->applicationtype&CT_SRV_MASK))
			webentry->applicationtype = i;
	}
#endif
	webentry->dgw = mibentry->dgw;
	webentry->ifIndex = mibentry->ifIndex;
#ifdef PPPOE_PASSTHROUGH
	if(mibentry->cmode==CHANNEL_MODE_PPPOE || mibentry->cmode==CHANNEL_MODE_BRIDGE){
		if(mibentry->brmode==BRIDGE_DISABLE)
			webentry->brmode=0;
		else if(mibentry->brmode==BRIDGE_PPPOE)
			webentry->brmode=1;
		else
			webentry->brmode=0;
	}else
		webentry->brmode=0;
#endif
	if(mibentry->itfGroup==0)
		webentry->itfGroup=0;
	else{
		unsigned char vChar,vcgroup;
		int i;
		unsigned short group=0;

//now we just update the group in the web
#ifdef NEW_PORTMAPPING
		group = mibentry->itfGroup;
#else

#ifdef CONFIG_EXT_SWITCH
		MIB_CE_SW_PORT_T Port;

		vcgroup = mibentry->itfGroup;

	printf("\nitfgroup=%d\n",vcgroup);
		for(i=0;i<4;i++){
			if( mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Port) ){
				if(vcgroup == Port.itfGroup)
					group|=(1<<i);
			}
		}
#endif

#ifdef WLAN_SUPPORT
		mib_get(MIB_WLAN_ITF_GROUP, (void *)&vChar);
		if(vcgroup == vChar)
			group|=0x10;
#ifdef WLAN_MBSSID
		mib_get(MIB_WLAN_VAP0_ITF_GROUP, (void *)&vChar);
		if(vcgroup == vChar)
			group|=0x20;
		mib_get(MIB_WLAN_VAP1_ITF_GROUP, (void *)&vChar);
		if(vcgroup == vChar)
			group|=0x40;
		mib_get(MIB_WLAN_VAP2_ITF_GROUP, (void *)&vChar);
		if(vcgroup == vChar)
			group|=0x80;
		mib_get(MIB_WLAN_VAP3_ITF_GROUP, (void *)&vChar);
		if(vcgroup == vChar)
			group|=0x100;
#endif //WLAN_MBSSID
#endif //WLAN_SUPPORT

#endif// NEW_PORTMAPPING
		//the group record the finale states
		webentry->itfGroup = group;
	}
	return 0;

}

static int web2mib(struct atmvc_entryx* webentry,MIB_CE_ATM_VC_Tp mibentry)
{
	switch(webentry->svtype){
		case UBR_WITHOUT_PCR:
		case UBR_WITH_PCR:
			mibentry->qos=ATMQOS_UBR;
			break;
		case CBR:
			mibentry->qos=ATMQOS_CBR;
			break;
		case NO_RT_VBR:
			mibentry->qos=ATMQOS_VBR_NRT;
			break;
		case RT_VBR:
			mibentry->qos=ATMQOS_VBR_RT;
			break;
		default:
			mibentry->qos=ATMQOS_UBR;
	}
	mibentry->svtype = webentry->svtype;
	mibentry->vpi = webentry->vpi;
	mibentry->vci = webentry->vci;
	mibentry->pcr = webentry->pcr;
	mibentry->scr = webentry->scr;
	mibentry->mbs = webentry->mbs;
	mibentry->encap = (webentry->encap==1?0:1);
	mibentry->napt = webentry->napt;
	mibentry->cmode = webentry->cmode;
	mibentry->mtu = webentry->mtu;
	//mibentry->brmode = webentry->brmode;
	mibentry->AddrMode = webentry->AddrMode;
	strcpy(mibentry->pppUsername,webentry->pppUsername);
	strcpy(mibentry->pppPassword, webentry->pppPassword);
	mibentry->pppAuth = webentry->pppAuth;
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
	strcpy(mibentry->pppServiceName, webentry->pppServiceName);
#endif
/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
	strcpy(mibentry->pppACName, webentry->pppACName);
/*star:20080718 END*/
	mibentry->pppCtype = webentry->pppCtype;
	mibentry->ipDhcp = webentry->ipDhcp;
	*(unsigned long*)&(mibentry->ipAddr) = htonl(*(unsigned long*)&(webentry->ipAddr));
	*(unsigned long*)&(mibentry->remoteIpAddr) = htonl(*(unsigned long*)&(webentry->remoteIpAddr));
	*(unsigned long*)&(mibentry->netMask) = htonl(*(unsigned long*)&(webentry->netMask));
#if 1//defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_8676HWNAT)
	mibentry->vlan = webentry->vlan;
	mibentry->vid = webentry->vid;
	mibentry->vprio = webentry->vprio;
	mibentry->vpass = webentry->vpass;
#endif
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
	// Kaohj -- E8 don't care, so always enabled.
	//mibentry->enableIpQos = webentry->qos;
	mibentry->enableIpQos = 1;
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	mibentry->PPPoEProxyEnable = webentry->PPPoEProxyEnable;
	mibentry->PPPoEProxyMaxUser = webentry->PPPoEProxyMaxUser;
#endif
#ifdef CTC_WAN_NAME
	if (webentry->applicationtype>=0 && webentry->applicationtype<MAX_SRV_NUM)
		mibentry->applicationtype = web2mib_srv[webentry->applicationtype];
	else
		mibentry->applicationtype = 0;
#ifdef _PRMT_X_CT_COM_WANEXT_
	mibentry->ServiceList = mibentry->applicationtype;
#endif
    if (mibentry->applicationtype&X_CT_SRV_OTHER)
    {
        /* wan that is other type need disable dhcp on lan interface binding with it */
        mibentry->disableLanDhcp = 1;
    }
#endif
	mibentry->dgw = webentry->dgw;
#ifdef PPPOE_PASSTHROUGH
	if (mibentry->cmode == CHANNEL_MODE_BRIDGE)
/*star:20090403 START to make the br connection is displayed in WANPPPConnection of tr069*/
//		mibentry->brmode = webentry->brmode;
		mibentry->brmode = BRIDGE_ETHERNET; //BRIDGE_PPPOE;
/*star:20090403 END*/
	else if(mibentry->cmode == CHANNEL_MODE_IPOE)
		mibentry->brmode = BRIDGE_DISABLE;
	else{
		if(webentry->brmode==0)
			mibentry->brmode=BRIDGE_DISABLE;
		else
			mibentry->brmode=BRIDGE_PPPOE;
	}
#endif
	//printf("\nbrmode:%d %d\n",mibentry->brmode,webentry->brmode);

#ifdef NEW_PORTMAPPING
	mibentry->itfGroup = webentry->itfGroup;
#endif

#ifdef CTC_WAN_NAME
	// no napt, dgw for TR069 and/or VOICE.
	if (!(mibentry->applicationtype&(X_CT_SRV_INTERNET|X_CT_SRV_OTHER)))
	{
		mibentry->napt=0;
		mibentry->dgw=0;
	}
#endif

	mibentry->dslite_enable = webentry->dslite_enable;
	mibentry->dslite_aftr_mode = webentry->dslite_aftr_mode;
	strcpy(mibentry->dslite_aftr_hostname,webentry->dslite_aftr_hostname);
	return 0;

}

int initdgwoption(int eid, request * wp, int argc, char ** argv)
{
#ifdef NEW_DGW_POLICY
	boaWrite(wp, "tbdgw.style.display =\"none\";\n");
#endif
	return 0;
}

static void do_wan_restart()
{
//add by ramen to take effect rip
#ifdef CONFIG_USER_ROUTED_ROUTED
	startRip();
#endif
	DEBUGPRINT;
	va_cmd(IFCONFIG, 2, 1, "imq0", "down");
	restartWAN(CONFIGALL, NULL);
}

/*****************************
** Internet连接
*/
#ifdef CONFIG_DEV_xDSL
int initPageAdsl(int eid, request * wp, int argc, char ** argv)
{
	struct atmvc_entryx	entry;
	int				pppnumleft = 5;
	int				cnt = 0;
	int				index = 0;
	unsigned char	ipAddr[16];		//IP地址
	unsigned char	remoteIpAddr[16];	//缺省网关
	unsigned char	netMask[16];	//子网掩码
	unsigned char	v4dns1[16];
	unsigned char	v4dns2[16];
	unsigned long	fstdns = 0;	//缺省DNS
	unsigned long	secdns = 0;	//可选DNS
	int				lineno = __LINE__;
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
	MIB_CE_PORT_BINDING_T pbEntry;
	int vlan_map;
#endif
	int poe_proxy;

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	/*test code*
	cnt = 1;
	pppnumleft = 7;

	memset(&entry, 0, sizeof(entry));
	entry.svtype = 0;
	entry.vpi = 0;
	entry.vci = 32;
	entry.pcr = 0;
	entry.scr = 0;
	entry.mbs = 0;
	entry.encap = 0;
	entry.napt = 0;
	entry.cmode = 2;
	entry.brmode = 1;
	strcpy(entry.pppUsername, "test");
	strcpy(entry.pppPassword, "test");
	entry.pppAuth = 0;
	strcpy(entry.pppServiceName, "test");
	entry.pppCtype = 0;
	entry.ipDhcp = 0;
	*(unsigned long*)&(entry.ipAddr) = 0x77777777;
	*(unsigned long*)&(entry.remoteIpAddr) = 0x77777777;
	*(unsigned long*)&(entry.netMask) = htonl(0xFFFFFF00);
	entry.vlan = 1;
	entry.vid = 0;
	entry.vprio = 0;
	entry.vpass = 0;
	entry.itfGroup = 0x48;
	entry.qos = 1;
	entry.PPPoEProxyEnable = 1;
	entry.PPPoEProxyMaxUser = 0;
	entry.applicationtype = 2;
	************Place your code here, do what you want to do! ************/
	MIB_CE_ATM_VC_T mibentry;
	int mibtotal,i;
	char wanname[MAX_WAN_NAME_LEN];

#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
	vlan_map = 0;
	cnt = mib_chain_total(MIB_PORT_BINDING_TBL);
	for (index=0; index<cnt; index++) {
		mib_chain_get(MIB_PORT_BINDING_TBL, index, (void*)&pbEntry);
		if (pbEntry.pb_mode)
			vlan_map |= (1<<index);
	}
	// put vlan-based port mapping
	_PUT_INT(vlan_map);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	poe_proxy = 1;
#else
	poe_proxy = 0;
#endif
	_PUT_INT(poe_proxy);
	memset(&entry,0,sizeof(entry));

	_PUT_INT(pppnumleft);
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&mibentry)!=1)
			continue;

		// Mason Yu. ITMS4
		if (MEDIA_INDEX(mibentry.ifIndex) != MEDIA_ATM)
			continue;
//		_PUT_INT(pppnumleft);
	//	_PUT_IP(fstdns);
	//	_PUT_IP(secdns);

//	for(index = 0; index < cnt; index++)
		{
			/************Place your code here, do what you want to do! ************/
			/************Place your code here, do what you want to do! ************/

			getWanName(&mibentry, wanname);
			int tmp=mib2web(&mibentry,&entry);
			if(tmp==-1)
				continue;

			strcpy(v4dns1, inet_ntoa(*((struct in_addr *)mibentry.v4dns1)));
			if (strcmp(v4dns1, "0.0.0.0")==0)
				strcpy(v4dns1, "");

			strcpy(v4dns2, inet_ntoa(*((struct in_addr *)mibentry.v4dns2)));
			if (strcmp(v4dns2, "0.0.0.0")==0)
				strcpy(v4dns2, "");

#ifdef CONFIG_IPV6
			unsigned char 	Ipv6AddrStr[48], RemoteIpv6AddrStr[48], RemoteIpv6EndPointAddrStr[48];
			char Ipv6Dns1Str[48], Ipv6Dns2Str[48];
			unsigned char	IPv6Str1[20], IPv6Str2[20];
			strcpy(Ipv6AddrStr, "");
			strcpy(RemoteIpv6AddrStr, "");
			strcpy(RemoteIpv6EndPointAddrStr, "");
			strcpy(IPv6Str1, "Ipv6Addr");
			strcpy(IPv6Str2, "Ipv6Gateway");

			if ((mibentry.AddrMode & IPV6_WAN_STATIC) == IPV6_WAN_STATIC)
			{
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Dns1, Ipv6Dns1Str, sizeof(Ipv6Dns1Str));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Dns2, Ipv6Dns2Str, sizeof(Ipv6Dns2Str));
			}
#ifdef DUAL_STACK_LITE
			else if ((mibentry.AddrMode & IPV6_WAN_DSLITE) == IPV6_WAN_DSLITE)
			{
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.RemoteIpv6EndPointAddr, RemoteIpv6EndPointAddrStr, sizeof(RemoteIpv6EndPointAddrStr));
				strcpy(IPv6Str1, "DSLiteLocalIP");
				strcpy(IPv6Str2, "DSLiteGateway");
			}
#endif
#endif

#ifndef CONFIG_IPV6
			boaWrite(wp, "push(new it_nr(\"%s\"" _PTI _PTI _PTI _PTI _PTI _PTI \
				_PTI _PTI _PTI _PTI \
				_PTS _PTS _PTI _PTS _PTS \
				_PTI _PTI \
				_PTS _PTS _PTS _PTI\
				_PTS _PTS _PTI\
				_PTI _PTI _PTI _PTI _PTI \
				_PTI _PTI _PTI _PTI"));\n",
				wanname, _PME(svtype), _PME(vpi), _PME(vci), _PME(pcr), _PME(scr), _PME(mbs),
				_PME(encap), _PME(napt), _PME(cmode), _PME(brmode),
				_PME(pppUsername), _PME(pppPassword), _PME(pppAuth),  _PME(pppServiceName),  _PME(pppACName),
				_PME(pppCtype), _PME(ipDhcp),
				_PMEIP(ipAddr), _PMEIP(remoteIpAddr), _PMEIP(netMask), _PME(dgw),
				"v4dns1", v4dns1, "v4dns2", v4dns2, _PME(dnsMode),
				_PME(vlan), _PME(vid), _PME(vprio), _PME(vpass), _PME(itfGroup),
				_PME(qos), _PME(PPPoEProxyEnable), _PME(PPPoEProxyMaxUser), _PME(applicationtype)
				);
#else
				boaWrite(wp, "push(new it_nr(\"%s\"" _PTI _PTI _PTI _PTI _PTI _PTI \
				_PTI _PTI _PTI _PTI _PTI\
				_PTS _PTS _PTI _PTS _PTS \
				_PTI _PTI \
				_PTS _PTS _PTS _PTI\
				_PTS _PTS _PTI\
				_PTI _PTI _PTI _PTI _PTI \
				_PTI _PTI _PTI _PTI "",

				wanname, _PME(svtype), _PME(vpi), _PME(vci), _PME(pcr), _PME(scr), _PME(mbs),
				_PME(encap), _PME(napt), _PME(cmode), _PME(brmode), _PME(AddrMode),
				_PME(pppUsername), _PME(pppPassword), _PME(pppAuth),  _PME(pppServiceName),  _PME(pppACName),
				_PME(pppCtype), _PME(ipDhcp),
				_PMEIP(ipAddr), _PMEIP(remoteIpAddr), _PMEIP(netMask), _PME(dgw),
				"v4dns1", v4dns1, "v4dns2", v4dns2, _PME(dnsMode),
				_PME(vlan), _PME(vid), _PME(vprio), _PME(vpass), _PME(itfGroup),
				_PME(qos), _PME(PPPoEProxyEnable), _PME(PPPoEProxyMaxUser), _PME(applicationtype)
				);

				boaWrite(wp, ""
				_PTI _PTI _PTI \
				_PTS _PTS \
				_PTS _PTS \
				_PTS \
				_PTI \
				_PTI _PTI \
				_PTI _PTI"));\n",

				"IpProtocolType", mibentry.IpProtocol , "slacc", (mibentry.AddrMode & IPV6_WAN_AUTO) == IPV6_WAN_AUTO ? 1:0, "staticIpv6", (mibentry.AddrMode & IPV6_WAN_STATIC) == IPV6_WAN_STATIC ? 1:0,
				IPv6Str1, Ipv6AddrStr, IPv6Str2, RemoteIpv6AddrStr,
				"Ipv6Dns1", Ipv6Dns1Str, "Ipv6Dns2", Ipv6Dns2Str,
				"DSLiteRemoteIP", RemoteIpv6EndPointAddrStr,
				"dslite_enable", mibentry.dslite_enable,
				"Ipv6PrefixLen", mibentry.Ipv6AddrPrefixLen, "itfenable", mibentry.Ipv6Dhcp,
				"iana", (mibentry.Ipv6DhcpRequest & 0x1) == 0x1 ? 1:0, "iapd", (mibentry.Ipv6DhcpRequest & 0x2) == 0x2 ? 1:0
				);
#endif
		}
	}

check_err:
	_TRACE_LEAVEL;
	return 0;
}

int initPageAdsl2(int eid, request * wp, int argc, char ** argv)
{
	boaWrite(wp, "var apmodes = new Array("
		"\"TR069_INTERNET\", \"INTERNET\", \"TR069\", \"Other\""
#ifdef VOIP_SUPPORT
		", \"VOICE\", \"TR069_VOICE\", \"VOICE_INTERNET\", \"TR069_VOICE_INTERNET\");\n");
#else
		");\n");
#endif
	return 0;
}

//add by chenzhuoxin
void formSetAdsl(request * wp, char *path, char *query)
{
	char *	strVal = NULL;
	short		adslMode = 0;
	int			lineno = __LINE__;
	int 		xmode;

	_TRACE_CALL;

	strVal = boaGetVar(wp, "glite", "");
	if ( 0 == strcmp(strVal,"on"))
		adslMode |= ADSL_MODE_GLITE;

	strVal = boaGetVar(wp, "gdmt", "");
	if ( 0 == strcmp(strVal,"on"))
		adslMode |= ADSL_MODE_GDMT;

	strVal = boaGetVar(wp, "t1413", "");
	if ( 0 == strcmp(strVal,"on"))
		adslMode |= ADSL_MODE_T1413;

	strVal = boaGetVar(wp, "adsl2", "");
	if ( 0 == strcmp(strVal,"on"))
		adslMode |= ADSL_MODE_ADSL2;

	strVal = boaGetVar(wp, "anxl", "");
	if ( 0 == strcmp(strVal,"on"))
		adslMode |= ADSL_MODE_ANXL;

	strVal = boaGetVar(wp, "adsl2p", "");
	if ( 0 == strcmp(strVal,"on"))
		adslMode |= ADSL_MODE_ADSL2P;

	strVal = boaGetVar(wp, "anxm", "");
	if ( 0 == strcmp(strVal,"on"))
		adslMode |= ADSL_MODE_ANXM;

	//printf("adslmode =%x\n",adslMode);

	/*
	 *add code by RTL
       */
       	if (!mib_set(MIB_ADSL_MODE, (void *)&adslMode)) {
		printf("Set ADSL mode error!");
		return;
	}

	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	//apply change
	xmode=0;
	if (adslMode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT))
		xmode |= 1;	// ADSL1
	if (adslMode & ADSL_MODE_ADSL2)
		xmode |= 2;	// ADSL2
	if (adslMode & ADSL_MODE_ADSL2P)
		xmode |= 4;	// ADSL2+
	adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);

	xmode = adslMode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT);	//  1: ansi, 2: g.dmt, 8:g.lite
	adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&xmode, 4);

	if (adslMode & ADSL_MODE_ANXL)	// Annex L
		xmode = 3; // Wide-Band & Narrow-Band Mode
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&xmode, 4);

	if (adslMode & ADSL_MODE_ANXM)	// Annex M
		xmode = 1;
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&xmode, 4);

	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	return;
}

void formAdsl(request * wp, char *path, char *query)
{
	struct atmvc_entryx	entry;
	char*			pifname = NULL;
	char*			stemp = "";
	char 			*submitUrl;
	unsigned long	fstdns = 0;	//缺省DNS
	unsigned long	secdns = 0;	//可选DNS
	int				ival = 0;
#ifdef CONFIG_USER_PPPOE_PROXY
	int				pppnummax = 5;//default value
#endif
	int				lineno = __LINE__;

	MIB_CE_ATM_VC_T mibentry,Entry;
	int totalEntry,i;
	char tmpBuf[100];

	unsigned int ifMap;
	char* strValue;
	char *dns1Ip, *dns2Ip;
	unsigned char mode;

	char act[10];
	MEDIA_TYPE_T mType;
/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
	int acflag=0;
	FETCH_INVALID_OPT(stemp, "acnameflag", _NEED);
	if(strcmp(stemp,"have")==0)
		acflag=1;
/*star:20080718 END*/
	_TRACE_CALL;
	FETCH_INVALID_OPT(stemp, "action", _NEED);
	strncpy(act,stemp,10);

	if(strcmp(stemp, "rm") == 0)	//remove
	{
		int idx=-1;
		char webwanname[MAX_WAN_NAME_LEN];
		char mibwanname[MAX_WAN_NAME_LEN];

		_ENTRY_INT(vpi, _NEED);
		_ENTRY_INT(vci, _NEED);

		/************Place your code here, do what you want to do! ************/
		/*use 'stemp' as 'link name' to match 'atmvc_entry' entry and remove relevant entry from MIB */
		/************Place your code here, do what you want to do! ************/

		FETCH_INVALID_OPT(stemp, "lst", _NEED);
		strncpy(webwanname,stemp,MAX_WAN_NAME_LEN-1);

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		for(i=0;i<totalEntry;i++){
			if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
				continue;
			getWanName(&Entry,mibwanname);
			if(Entry.vpi==entry.vpi&&Entry.vci==entry.vci&&(!strcmp(mibwanname,webwanname))){
				idx=i;
				break;
			}
		}

		if(idx!=-1){
			resolveServiceDependency(idx);

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

#ifdef CONFIG_IPV6
#ifdef DNSV6_BIND_PVC_SUPPORT
			MIB_CE_ATM_VC_T dnsv6PvcEntry;
			if(mib_chain_get(MIB_ATM_VC_TBL,idx,&dnsv6PvcEntry)&&(dnsv6PvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
			{
				int tempi=0;
				unsigned int pvcifIdx=0;
				for(tempi=0;tempi<3;tempi++)
				{
					mib_get(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					if(pvcifIdx==dnsv6PvcEntry.ifIndex)//I get it
					{
						pvcifIdx = DUMMY_IFINDEX;
						mib_set(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif
#endif

//#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
			{
				MIB_CE_ATM_VC_T cwmp_entry;
				if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&cwmp_entry))
				{
					// todo: check e8b
					#if 0
					delBrgMacFilterRule(cwmp_entry.ifIndex);
					delIpRouteTbl(cwmp_entry.ifIndex);
				#ifdef CONFIG_USER_DDNS
					delDDNSinter(cwmp_entry.ifIndex);
				#endif
				#ifdef BR_ROUTE_ONEPVC
					modify_Br_Rt_entry(&cwmp_entry);
				#endif
					#endif
				}
			}
//#endif
			// Mason Yu. ITMS4
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
					goto setErr_restart;
			}


//ql add: check if it is necessary to delete a group of interface
#ifdef ITF_GROUP
			{
				int wanPortNum;
				unsigned int swNum, vcNum;
				MIB_CE_SW_PORT_T Entry;
				MIB_CE_ATM_VC_T pvcEntry;
				int j, grpnum;
				char mygroup;
				int enable_portmap =0;

				vcNum = mib_chain_total(MIB_ATM_VC_TBL);
				for (grpnum=1; grpnum<=4; grpnum++) {
					wanPortNum = 0;

					for (j=0; j<vcNum; j++) {
						if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
						{
							//boaError(wp, 400, "Get chain record error!\n");
							printf("Get chain record error!\n");
							return;
						}
						if(pvcEntry.itfGroup!=0)
							enable_portmap++;
						if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
							continue;

						if (pvcEntry.applicationtype & (X_CT_SRV_INTERNET|X_CT_SRV_OTHER))
							wanPortNum++;

					}
					//printf("\nwanPortNum=%d\n",wanPortNum);
					if (0 == wanPortNum) {
						//printf("delete port mapping group %d\n", grpnum);
						//release LAN ports
						swNum = mib_chain_total(MIB_SW_PORT_TBL);
						for (j=swNum; j>0; j--) {
							if (!mib_chain_get(MIB_SW_PORT_TBL, j-1, (void *)&Entry))
								return;
							if (Entry.itfGroup == grpnum) {
								Entry.itfGroup = 0;
								mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, j-1);
							}
						}
#ifdef WLAN_SUPPORT
						//release wlan0
						mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
						if (mygroup == grpnum) {
							mygroup = 0;
							mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
						}
#endif
#ifdef WLAN_MBSSID
						//release MBSSID
						for (j=1; j<5; j++) {
							mib_get(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
							if (mygroup == grpnum) {
								mygroup = 0;
								mib_set(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
							}
						}
#endif
						for (j=0; j<vcNum; j++) {
							if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
							{
								//boaError(wp, 400, "Get chain record error!\n");
								printf("Get chain record error!\n");
								return;
							}
							if(pvcEntry.itfGroup==grpnum){
								printf("\nmodify tr069 portmapping!\n");
								pvcEntry.itfGroup=0;
								mib_chain_update(MIB_ATM_VC_TBL,(void *)&pvcEntry,j);
							}
						}
						//setgroup("", grpnum, lowPrio);
						setgroup("", grpnum);

					}
				}

				if(!enable_portmap)
				{
				    printf("\nstop portmapping!\n");

			            mib_get(MIB_MPMODE, (void *)&mode);
			             mode &= 0xfe;
			            mib_set(MIB_MPMODE, (void *)&mode);
				}
			}//end
#endif

		}else
		{
			strcpy(tmpBuf, strSelectvc);
			goto setErr_nochange;
		}

		// Mason Yu. ITMS4
		restartWAN(CONFIGONE, NULL);
		goto setOk_filter;

	}
	else if(strcmp(stemp, "sv") == 0)	//add or modify
		{
		memset(&entry, 0, sizeof(entry));
		memset(&mibentry, 0, sizeof(mibentry));

		//_GET_PSTR(ifname, _OPT);
		//(fstdns, _OPT);
		//_GET_IP(secdns, _OPT);
		// Mason Yu. for IPV6
#ifdef CONFIG_IPV6
		strValue = boaGetVar(wp, "IpProtocolType", "");
		if (strValue[0]) {
			mibentry.IpProtocol = strValue[0] - '0';
		}
#endif

		_ENTRY_INT(svtype, _NEED);
		if(entry.svtype > 4){lineno = __LINE__; goto check_err;}

		_ENTRY_INT(vpi, _NEED);

		_ENTRY_INT(vci, _NEED);
		if(entry.vci < 32){lineno = __LINE__; goto check_err;}
		entry.pcr=6000; //set default value
		if(entry.svtype > 0)
		{
			_ENTRY_INT(pcr, _NEED);
			if(entry.pcr < 1){lineno = __LINE__; goto check_err;}
			if(entry.svtype > 2)
			{
				_ENTRY_INT(scr, _NEED);
				if(entry.scr < 1 || entry.scr > entry.pcr){lineno = __LINE__; goto check_err;}
				_ENTRY_INT(mbs, _NEED);
				if(entry.mbs < 1){lineno = __LINE__; goto check_err;}
			}
		}

		_ENTRY_INT(encap, _NEED);
		if(entry.encap > 1){lineno = __LINE__; goto check_err;}

		_ENTRY_BOOL(napt, _NEED);

		_ENTRY_INT(cmode, _NEED);
		if(entry.cmode > 2){lineno = __LINE__; goto check_err;}

		switch(entry.cmode)
		{
		case CHANNEL_MODE_BRIDGE://bridge
			{
			//	_ENTRY_BOOL(brmode, _NEED);
			}break;
		case CHANNEL_MODE_IPOE://route
			{
#ifdef CONFIG_IPV6
				if (mibentry.IpProtocol & IPVER_IPV4) {
#endif
				_ENTRY_INT(ipDhcp, _NEED);
				if(entry.ipDhcp > 1){lineno = __LINE__; goto check_err;}
				if(entry.cmode == 1)//static ip
				{
					//_ENTRY_IP(ipAddr, _NEED);
					FETCH_INVALID_OPT(stemp, "ipAddr", _NEED);
					//printf("\nweb ip %s\n",stemp);
					*(unsigned long*)&(entry.ipAddr) = inet_addr(stemp);
					//printf("\nentry ip:%x\n",*(unsigned long*)&(entry.ipAddr));
					_ENTRY_IP(remoteIpAddr, _NEED);
					_ENTRY_IP(netMask, _NEED);
				}
#ifdef CONFIG_IPV6
				}
#endif
			}break;
		case CHANNEL_MODE_PPPOE://pppoe
			{
/*star:20090302 START ppp username and password can be empty*/
				//_ENTRY_STR(pppUsername, _NEED);
				//_ENTRY_STR(pppPassword, _NEED);
				_ENTRY_STR(pppUsername, _OPT);
				_ENTRY_STR(pppPassword, _OPT);
/*star:20090302 END*/

				_ENTRY_INT(pppCtype, _NEED);
				if(entry.pppCtype > 1){lineno = __LINE__; goto check_err;}
				_ENTRY_STR(pppServiceName, _OPT);
/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
				if(acflag==1)
					_ENTRY_STR(pppACName, _OPT);
/*star:20080718 END*/
#ifdef CONFIG_USER_PPPOE_PROXY
				_ENTRY_BOOL(PPPoEProxyEnable, _NEED);
				if(entry.PPPoEProxyEnable)
				{
					_ENTRY_INT(PPPoEProxyMaxUser, _NEED);
					if(entry.PPPoEProxyMaxUser < 0 || entry.PPPoEProxyMaxUser > (unsigned)pppnummax){lineno = __LINE__; goto check_err;}
				}
				_ENTRY_BOOL(brmode, _NEED);
#endif
			}break;
		}

		_ENTRY_BOOL(vlan, _NEED);
		if(entry.vlan)
		{
			_ENTRY_INT(vid, _NEED);
			if(entry.vid > 4095){lineno = __LINE__; goto check_err;}

			_ENTRY_INT(vprio, _NEED);// entry.vprio == (无)
			if(entry.vprio > 8){lineno = __LINE__; goto check_err;}
		}

		_ENTRY_INT(vpass, _OPT);

		_ENTRY_INT(itfGroup, _NEED);

		_ENTRY_BOOL(qos, _NEED);

		_ENTRY_INT(applicationtype, _NEED);
		web2mib(&entry,&mibentry);

		if (mibentry.cmode == CHANNEL_MODE_IPOE)
		{
			strValue = boaGetVar(wp, "dnsMode", "");
			if (strValue[0]) {
				mibentry.dnsMode = strValue[0] - '0';
			}

			dns1Ip = boaGetVar(wp, "v4dns1", "");
			if (dns1Ip[0]) {
				if (!inet_aton(dns1Ip, (struct in_addr *)&mibentry.v4dns1)) {
					strcpy(tmpBuf, "Invalid dnsv4 1 IP-address value!");
					goto setErr_nochange;
				}
			}

			dns2Ip = boaGetVar(wp, "v4dns2", "");
			if (dns2Ip[0]) {
				if (!inet_aton(dns2Ip, (struct in_addr *)&mibentry.v4dns2)) {
					strcpy(tmpBuf, "Invalid dnsv4 2 IP-address value!");
					goto setErr_nochange;
				}
			}
		}
		else
			mibentry.dnsMode = 1; // default is enable dnsMode

		if(mibentry.applicationtype & ~CT_SRV_MASK)
			{lineno = __LINE__; goto check_err;}

		// E8B: if 'INTERNET', set as default route.
		if (mibentry.applicationtype & X_CT_SRV_INTERNET)
			mibentry.dgw = 1;
		else
			mibentry.dgw = 0;
		//_ENTRY_BOOL(dgw,_NEED);

		/************Place your code here, do what you want to do! ************/
		/************Place your code here, do what you want to do! ************/

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		mibentry.enable=1;//always enable

#ifdef CONFIG_IPV6
		if (mibentry.cmode != CHANNEL_MODE_BRIDGE) {
			if (mibentry.IpProtocol & IPVER_IPV6)
				retrieveIPv6Record(wp, &mibentry);
			else
				mibentry.AddrMode = IPV6_WAN_NONE;
		}
#endif

		FETCH_INVALID_OPT(stemp, "lkname", _NEED);
		if(strcmp(stemp,"new")==0){   //add
			int cnt, pIdx;
	#ifdef BR_ROUTE_ONEPVC
			int br_route_idx=-1;
	#endif
			unsigned char vcIdx;
			int intVal;

			if (totalEntry >= MAX_VC_NUM)
			{
				strcpy(tmpBuf, strMaxVc);
				goto setErr_nochange;
			}
			// check if connection exists
			ifMap = 0;
			cnt=0;

			for (i=0; i<totalEntry; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				{
					boaError(wp, 400, strGetChainerror);
					return;
				}
				mType = MEDIA_INDEX(Entry.ifIndex);
#ifdef BR_ROUTE_ONEPVC
				if (mType == MEDIA_ATM && Entry.vpi == mibentry.vpi && Entry.vci == mibentry.vci )
				{
					cnt++;
					pIdx = i;	// Jenny, for multisession PPPoE, record entry index instead of atmvc entry pointer
					//pmyEntry = &Entry;
					if(Entry.cmode!=mibentry.cmode)
						br_route_idx = i;
				}
#else
				if (mType == MEDIA_ATM && Entry.vpi == mibentry.vpi && Entry.vci == mibentry.vci) {
					cnt++;
					pIdx = i;	// Jenny, for multisession PPPoE, record entry index instead of atmvc entry pointer
					//pmyEntry = &Entry;
				}
#endif

				if (mType == MEDIA_ATM)
					ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
				ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex); // PPP map

				//AUG_PRT("The ifMap is 0x%x\n", ifMap);
			}

			if (cnt == 0)	// pvc not exists
			{

				mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
				//AUG_PRT("The mibentry.ifIndex is 0x%x\n", mibentry.ifIndex);
				if (mibentry.ifIndex == NA_VC) {
					strcpy(tmpBuf, strMaxVc);
					goto setErr_nochange;
				}
				else if (mibentry.ifIndex == NA_PPP) {
					strcpy(tmpBuf, strMaxNumPPPoE);
					goto setErr_nochange;
				}

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
				mibentry.ConDevInstNum = 1 + findMaxConDevInstNum(MEDIA_INDEX(mibentry.ifIndex));
				if( (mibentry.cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
				    ((mibentry.cmode==CHANNEL_MODE_BRIDGE)&&(mibentry.brmode==BRIDGE_PPPOE)) ||
#endif
				    (mibentry.cmode==CHANNEL_MODE_PPPOA) )
					mibentry.ConPPPInstNum = 1;
				else
					mibentry.ConIPInstNum = 1;
				//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, mibentry.ConDevInstNum, mibentry.ConPPPInstNum, mibentry.ConIPInstNum );
#endif
			}else{
				if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&Entry)) {	// Jenny, for multisession PPPoE, get existed pvc config
					strcpy(tmpBuf, errGetEntry);
					goto setErr_nochange;
				}
				if (Entry.cmode == CHANNEL_MODE_PPPOE && mibentry.cmode == CHANNEL_MODE_PPPOE
#ifdef BR_ROUTE_ONEPVC
					&& br_route_idx<0
#endif
				)
				{
					if (cnt<MAX_POE_PER_VC)
					{	// get the pvc info.
						mibentry.qos = Entry.qos;
						mibentry.pcr = Entry.pcr;
						mibentry.encap = Entry.encap;
						ifMap &= 0xffff0000; // don't care the vc part
						mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
						if (mibentry.ifIndex == NA_PPP) {
							strcpy(tmpBuf, strMaxNumPPPoE);
							goto setErr_nochange;
						}
						mibentry.ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(mibentry.ifIndex), VC_INDEX(mibentry.ifIndex));
#ifdef PPPOE_PASSTHROUGH
						mibentry.brmode = Entry.brmode;	// Jenny, for multisession PPPoE
#endif

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
						mibentry.ConDevInstNum = Entry.ConDevInstNum;
						mibentry.ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(Entry.ifIndex), Entry.ConDevInstNum);
						//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, mibentry.ConDevInstNum, mibentry.ConPPPInstNum, mibentry.ConIPInstNum );
#endif
					}
					else
					{
						strcpy(tmpBuf, strMaxNumPPPoE);
						goto setErr_nochange;
					}
				}
		#ifdef BR_ROUTE_ONEPVC
				else if(cnt==1 && ((Entry.cmode != CHANNEL_MODE_BRIDGE && mibentry.cmode == CHANNEL_MODE_BRIDGE)||(Entry.cmode == CHANNEL_MODE_BRIDGE && mibentry.cmode != CHANNEL_MODE_BRIDGE))){
					modifyifIndex(&Entry,&mibentry);
					Entry.br_route_flag=1;
					mibentry.br_route_flag=1;
					//printf("\nmibentry.ifIndex=%x,Entry.ifIndex=%x\n",mibentry.ifIndex, Entry.ifIndex);
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
					mibentry.ConDevInstNum = Entry.ConDevInstNum;
					if(mibentry.cmode == CHANNEL_MODE_PPPOE || mibentry.cmode == CHANNEL_MODE_PPPOA){
						mibentry.ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(Entry.ifIndex), Entry.ConDevInstNum);
						mibentry.ConIPInstNum = 0;
					}else{
						mibentry.ConIPInstNum = 1 + findMaxIPConInstNum(MEDIA_INDEX(Entry.ifIndex), Entry.ConDevInstNum );
						mibentry.ConPPPInstNum = 0;
					}
					//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, mibentry.ConDevInstNum, mibentry.ConPPPInstNum, mibentry.ConIPInstNum );
#endif
					mib_chain_update(MIB_ATM_VC_TBL,&Entry,br_route_idx);
				}
		#endif
				else
				{
					strcpy(tmpBuf, strConnectExist);
					goto setErr_nochange;
				}
			}

			// set default
			if (mibentry.cmode == CHANNEL_MODE_PPPOE)
			{
				mibentry.mtu = 1492;
/*
#ifdef CONFIG_USER_PPPOE_PROXY
				mibentry.PPPoEProxyMaxUser=4;
				mibentry.PPPoEProxyEnable=0;
#endif
*///set by web
			}
			else
				mibentry.mtu = 1500;
/*
#ifdef CONFIG_EXT_SWITCH
			// VLAN
			mibentry.vlan = 0;	// disable
			mibentry.vid = 0; // VLAN tag
			mibentry.vprio = 0; // priority bits (0 ~ 7)
			mibentry.vpass = 0; // no pass-through
#endif
*///set by web
// todo: check e8b

#ifdef NEW_PORTMAPPING
			check_itfGroup(&mibentry, 0);
#endif

/*			if(mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&mibentry) != 1){
				strcpy(tmpBuf, strAddChainerror);
				goto setErr_filter;
			}
*/
/*star:20090302 START wen INTERNET pvc start, igmp proxy open auto*/
			mibentry.enableIGMP=0;
			if(mibentry.cmode!=CHANNEL_MODE_BRIDGE)
				if (mibentry.applicationtype&X_CT_SRV_INTERNET)
					mibentry.enableIGMP=1;
/*star:20090302 END*/
			// Mason Yu. ITMS4
			deleteConnection(CONFIGONE, NULL);  // Add
			intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&mibentry);
			if (intVal == 0) {
				strcpy(tmpBuf, strAddChainerror);
				goto setErr_restart;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, strTableFull);
				goto setErr_restart;
			}
			// Kaohj -- Queue LEN table not supportted.
			#if 0
#ifndef QOS_SETUP_IMQ
			//ql_xu: add qos queue
			if (mibentry.enableIpQos)
				addIpQosQueue(mibentry.ifIndex);
#endif
			#endif
			// Mason Yu. ITMS4
			restartWAN(CONFIGONE, &mibentry);    // Add
			goto setOk_filter;

		}else{   //modify
			int cnt=0, pIdx;
			int selected=-1;
			int itsMe;
			MIB_CE_ATM_VC_T myEntry;
	#ifdef BR_ROUTE_ONEPVC
			int br_route_idx=-1;
			MIB_CE_ATM_VC_T  br_rt_Entry;
	#endif

			char webwanname[MAX_WAN_NAME_LEN];
			char mibwanname[MAX_WAN_NAME_LEN];

			ifMap=0;
			FETCH_INVALID_OPT(stemp, "lst", _NEED);
			strncpy(webwanname,stemp,MAX_WAN_NAME_LEN-1);

			for (i=0; i<totalEntry; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				{
					boaError(wp, 400, strGetChainerror);
					return;
				}

				getWanName(&Entry,mibwanname);
				//printf("\nmibname=%s\n",mibwanname);
		DEBUGPRINT;
				mType = MEDIA_INDEX(Entry.ifIndex);
				itsMe = 0;
				if (!strcmp(webwanname,mibwanname)) {
					itsMe = 1;
					if (selected == -1)
						selected = i;
					else {
						strcpy(tmpBuf, strConnectExist);
						goto setErr_nochange;
					}
						printf("Error: Duplicate WAN name!\n");
				}
				if (mType == MEDIA_ATM &&
					Entry.vpi == mibentry.vpi && Entry.vci == mibentry.vci &&
					!itsMe)
					cnt++; // count the number of entry with the same pvc as the modified one

				if (!itsMe) {
		DEBUGPRINT;
					if (mType == MEDIA_ATM)
						ifMap |= 1 << VC_INDEX(Entry.ifIndex);	// vc map
					ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex); // PPP map
				}
			}

			if(cnt > 0) {
				//Make sure there is no mismatch mode
				for (i=0; i<totalEntry; i++) {
		DEBUGPRINT;
					if(i==selected)
						continue;
					if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)){
						boaError(wp, 400, strGetChainerror);
						return;
					}
		DEBUGPRINT;
					mType = MEDIA_INDEX(Entry.ifIndex);
					if (mType == MEDIA_ATM && Entry.vpi == mibentry.vpi && Entry.vci == mibentry.vci) {
		DEBUGPRINT;
						if (mibentry.cmode == CHANNEL_MODE_PPPOE)		// Jenny, for multisession PPPoE support
							pIdx = i;
						if(Entry.cmode != mibentry.cmode) {
		#ifdef BR_ROUTE_ONEPVC
							if((Entry.cmode != CHANNEL_MODE_BRIDGE && mibentry.cmode == CHANNEL_MODE_BRIDGE)||
									(Entry.cmode == CHANNEL_MODE_BRIDGE && mibentry.cmode != CHANNEL_MODE_BRIDGE)) {
								br_route_idx=i;
								if (cnt > 1) {
									strcpy(tmpBuf, strConnectExist);
									goto setErr_nochange;
								}
							}
							else {
		#endif
								strcpy(tmpBuf, strConnectExist);
								goto setErr_nochange;
		#ifdef BR_ROUTE_ONEPVC
							}
		#endif
						}
					}
				}
		DEBUGPRINT;
				//Max. 2 PPPoE connections
				//if(entry.cmode == CHANNEL_MODE_PPPOE && cnt==2) {
				if(mibentry.cmode == CHANNEL_MODE_PPPOE && cnt==MAX_POE_PER_VC) {	// Jenny, multisession PPPoE support
					strcpy(tmpBuf, strMaxNumPPPoE);
					goto setErr_nochange;
				//Max. 1 connection except PPPoE
				} else if(mibentry.cmode != CHANNEL_MODE_PPPOE&& cnt>0 ) {
	#ifdef BR_ROUTE_ONEPVC
					if(br_route_idx<0)
	#endif
					{
						strcpy(tmpBuf, strConnectExist);
						goto setErr_nochange;
					}
				}
				if (mibentry.cmode == CHANNEL_MODE_PPPOE && cnt>0)		// Jenny, for multisession PPPoE, get existed PPPoE config for further ifindex use
					if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&myEntry)) {
						strcpy(tmpBuf, errGetEntry);
						goto setErr_nochange;
					}
	#ifdef BR_ROUTE_ONEPVC
				if(br_route_idx>=0)
					if (!mib_chain_get(MIB_ATM_VC_TBL, br_route_idx, (void *)&br_rt_Entry)) {
						strcpy(tmpBuf, errGetEntry);
						goto setErr_nochange;
					}
	#endif
			}
		DEBUGPRINT;
			if (!mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_nochange;
			}

			// restore stuff not posted in this form
			if (mibentry.enable
				&& ((CHANNEL_MODE_IPOE == mibentry.cmode)
				|| (CHANNEL_MODE_PPPOA == mibentry.cmode)
				|| (CHANNEL_MODE_PPPOE == mibentry.cmode)
				|| (CHANNEL_MODE_RT1483 == mibentry.cmode)))
			{
				/* restore igmp-proxy setting */
				mibentry.enableIGMP = Entry.enableIGMP;
			}

			if (mibentry.applicationtype == Entry.applicationtype)
			{
				/* application type not changed, reserve the dhcp setting on lan interface */
				mibentry.disableLanDhcp = Entry.disableLanDhcp;
			}

			if (mibentry.cmode == CHANNEL_MODE_PPPOE)
				if (cnt > 0) {		// Jenny, for multisession PPPoE, ifIndex(VC device) must refer to existed PPPoE connection
			#ifdef BR_ROUTE_ONEPVC
					if(br_route_idx>=0)
						mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
					else
			#endif
					{
						ifMap &= 0xffff0000; // don't care the vc part
						mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
						mibentry.ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(mibentry.ifIndex), VC_INDEX(myEntry.ifIndex));
					}
				}
				else {
		DEBUGPRINT;
					mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
/*
#ifdef PPPOE_PASSTHROUGH
					if (mibentry.cmode == Entry.cmode)
						mibentry.brmode = Entry.brmode;
#endif
*/
				}
			else{
		#ifdef BR_ROUTE_ONEPVC
				if(Entry.br_route_flag==1)
					mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
				else
		#endif
				mibentry.ifIndex = Entry.ifIndex;
			}
	DEBUGPRINT;
/*
			entry.qos = Entry.qos;
			entry.pcr = Entry.pcr;
			entry.scr = Entry.scr;
			entry.mbs = Entry.mbs;
			entry.cdvt = Entry.cdvt;
*///set by web
			mibentry.pppAuth = Entry.pppAuth;
			mibentry.rip = Entry.rip;
			//entry.dgw = Entry.dgw;
			mibentry.mtu = Entry.mtu;

#ifdef CONFIG_EXT_SWITCH
			//ql: when pvc is modified, interface group don't changed???
			//mibentry.itfGroup = Entry.itfGroup;
#endif

	DEBUGPRINT;
#ifdef CONFIG_SPPPD_STATICIP
			if(mibentry.cmode == CHANNEL_MODE_PPPOE)
			{
				mibentry.pppIp = Entry.pppIp;
				strcpy( mibentry.ipAddr, Entry.ipAddr);
			}
#endif
	DEBUGPRINT;

#ifdef PPPOE_PASSTHROUGH
			//printf("\nentry.cmode=%d,Entry.cmode=%d,Entry.brmode=%d\n",mibentry.cmode,Entry.cmode,Entry.brmode);
			//if (mibentry.cmode != CHANNEL_MODE_PPPOE && mibentry.cmode != CHANNEL_MODE_BRIDGE)
			if (mibentry.cmode != CHANNEL_MODE_PPPOE)
				if (mibentry.cmode == Entry.cmode)
					mibentry.brmode = Entry.brmode;
#endif
DEBUGPRINT;
#ifdef CONFIG_EXT_SWITCH
/*
			// VLAN
			entry.vlan = Entry.vlan;
			entry.vid = Entry.vid;
			entry.vprio = Entry.vprio;
			entry.vpass = Entry.vpass;
*///set by web
#ifdef _PRMT_X_CT_COM_ETHLINK_
			//mibentry.pmark = Entry.pmark;
#endif
#endif
	DEBUGPRINT;
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
			mibentry.connDisable = 0;
			if (Entry.vpi == mibentry.vpi && Entry.vci == mibentry.vci)
			{
				mibentry.ConDevInstNum = Entry.ConDevInstNum;
				if( (mibentry.cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
				    ((mibentry.cmode==CHANNEL_MODE_BRIDGE)&&(mibentry.brmode==BRIDGE_PPPOE)) ||
#endif
				    (mibentry.cmode==CHANNEL_MODE_PPPOA) )
				{
					if( Entry.ConPPPInstNum!=0 )
						mibentry.ConPPPInstNum = Entry.ConPPPInstNum;
					else
						mibentry.ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum);
					mibentry.ConIPInstNum = 0;
				}else{
					mibentry.ConPPPInstNum = 0;
					if(Entry.ConIPInstNum!=0)
						mibentry.ConIPInstNum = Entry.ConIPInstNum;
					else
						mibentry.ConIPInstNum = 1 + findMaxIPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum );
				}
			}else{
	DEBUGPRINT;
				unsigned int  instnum=0;
				instnum = findConDevInstNumByPVC( mibentry.vpi, mibentry.vci );
				if(instnum==0)
					instnum = 1 + findMaxConDevInstNum(MEDIA_INDEX(mibentry.ifIndex));
				mibentry.ConDevInstNum = instnum;
				if( (mibentry.cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
				    ((mibentry.cmode==CHANNEL_MODE_BRIDGE)&&(mibentry.brmode==BRIDGE_PPPOE)) ||
#endif
				    (mibentry.cmode==CHANNEL_MODE_PPPOA) )
				{
					mibentry.ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum);
					mibentry.ConIPInstNum = 0;
				}else{
					mibentry.ConPPPInstNum = 0;
					mibentry.ConIPInstNum = 1 + findMaxIPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum );
				}
	DEBUGPRINT;
			}
			//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, mibentry.ConDevInstNum, mibentry.ConPPPInstNum, mibentry.ConIPInstNum );

			mibentry.autoDisTime = Entry.autoDisTime;
			mibentry.warnDisDelay = Entry.warnDisDelay;
			//strcpy( entry.pppServiceName, Entry.pppServiceName );
			strcpy( mibentry.WanName, Entry.WanName );
#ifdef CONFIG_USER_PPPOE_PROXY
			entry.PPPoEProxyEnable = Entry.PPPoEProxyEnable;
			entry.PPPoEProxyMaxUser = Entry.PPPoEProxyMaxUser;
#endif
#ifdef _PRMT_X_CT_COM_WANEXT_
//			mibentry.ServiceList = Entry.ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#ifdef _PRMT_X_CT_COM_DHCP_
			memcpy(mibentry.dhcpv6_opt16_enable, Entry.dhcpv6_opt16_enable, sizeof(Entry.dhcpv6_opt16_enable));
			memcpy(mibentry.dhcpv6_opt16_type, Entry.dhcpv6_opt16_type, sizeof(Entry.dhcpv6_opt16_type));
			memcpy(mibentry.dhcpv6_opt16_value_mode, Entry.dhcpv6_opt16_value_mode, sizeof(Entry.dhcpv6_opt16_value_mode));
			memcpy(mibentry.dhcpv6_opt16_value, Entry.dhcpv6_opt16_value, 4 * 80);
			memcpy(mibentry.dhcpv6_opt17_enable, Entry.dhcpv6_opt17_enable, sizeof(Entry.dhcpv6_opt17_enable));
			memcpy(mibentry.dhcpv6_opt17_type, Entry.dhcpv6_opt17_type, sizeof(Entry.dhcpv6_opt17_type));
			memcpy(mibentry.dhcpv6_opt17_sub_code, Entry.dhcpv6_opt17_sub_code, sizeof(Entry.dhcpv6_opt17_sub_code));
			memcpy(mibentry.dhcpv6_opt17_sub_data, Entry.dhcpv6_opt17_sub_data, 4 * 36);
			memcpy(mibentry.dhcpv6_opt17_value, Entry.dhcpv6_opt17_value, 4 * 36);

			memcpy(mibentry.dhcp_opt60_enable, Entry.dhcp_opt60_enable, sizeof(Entry.dhcp_opt60_enable));
			memcpy(mibentry.dhcp_opt60_type, Entry.dhcp_opt60_type, sizeof(Entry.dhcp_opt60_type));
			memcpy(mibentry.dhcp_opt60_value_mode, Entry.dhcp_opt60_value_mode, sizeof(Entry.dhcp_opt60_value_mode));
			memcpy(mibentry.dhcp_opt60_value, Entry.dhcp_opt60_value, 4 * 80);
			memcpy(mibentry.dhcp_opt125_enable, Entry.dhcp_opt125_enable, sizeof(Entry.dhcp_opt125_enable));
			memcpy(mibentry.dhcp_opt125_type, Entry.dhcp_opt125_type, sizeof(Entry.dhcp_opt125_type));
			memcpy(mibentry.dhcp_opt125_sub_code, Entry.dhcp_opt125_sub_code, sizeof(Entry.dhcp_opt125_sub_code));
			memcpy(mibentry.dhcp_opt125_sub_data, Entry.dhcp_opt125_sub_data, 4 * 36);
			memcpy(mibentry.dhcp_opt125_value, Entry.dhcp_opt125_value, 4 * 36);
#endif
#endif //_CWMP_MIB_
/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
			if(acflag==0)
/*star:20080718 END*/
				strcpy(mibentry.pppACName, Entry.pppACName);

			// find the ifIndex
			if (mibentry.cmode != Entry.cmode)
			{
				if (!(mibentry.cmode == CHANNEL_MODE_PPPOE && cnt>0))	// Jenny, entries except multisession PPPoE
					mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
				if (mibentry.ifIndex == NA_VC) {
					strcpy(tmpBuf, strMaxVc);
					goto setErr_nochange;
				}
				else if (mibentry.ifIndex == NA_PPP) {
					strcpy(tmpBuf, strMaxNumPPPoE);
					goto setErr_nochange;
				}
	DEBUGPRINT;
				// mode changed, restore to default
				if (mibentry.cmode == CHANNEL_MODE_PPPOE) {
					mibentry.mtu = 1492;
/*
#ifdef CONFIG_USER_PPPOE_PROXY
					entry.PPPoEProxyMaxUser=4;
#endif
					entry.pppAuth = 0;
*///set by web
				}
				else {
/*
#ifdef CONFIG_USER_PPPOE_PROXY
					entry.PPPoEProxyMaxUser=0;
#endif
*/
	//				entry.dgw = 1;
					mibentry.mtu = 1500;
				}
	DEBUGPRINT;
	//			entry.dgw = 1;
/*
#ifdef CONFIG_EXT_SWITCH
				// VLAN
				entry.vlan = 0;	// disable
				entry.vid = 0; // VLAN tag
				entry.vprio = 0; // priority bits (0 ~ 7)
				entry.vpass = 0; // no pass-through
#endif
*///set by web
			}

	#ifdef BR_ROUTE_ONEPVC
			if(br_route_idx>=0){
					modifyifIndex(&br_rt_Entry,&mibentry);
					br_rt_Entry.br_route_flag=1;
					mibentry.br_route_flag=1;
					//printf("\nmibentry.ifIndex=%x,Entry.ifIndex=%x\n",mibentry.ifIndex, br_rt_Entry.ifIndex);
					mib_chain_update(MIB_ATM_VC_TBL,&br_rt_Entry,br_route_idx);
			}
			if(br_route_idx<0 && Entry.br_route_flag==1)
				modify_Br_Rt_entry(&Entry);
	#endif


DEBUGPRINT;
			if( mibentry.ifIndex!=Entry.ifIndex ||
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
				mibentry.enableIpQos != Entry.enableIpQos ||
#endif
				mibentry.cmode != Entry.cmode)
				resolveServiceDependency(selected);

			if( mibentry.ifIndex!=Entry.ifIndex)
			{
				// todo: check e8b
				#if 0
		#ifdef CONFIG_USER_DDNS
				delDDNSinter(Entry.ifIndex);
		#endif
				delBrgMacFilterRule(Entry.ifIndex);
				delIpRouteTbl(Entry.ifIndex);
				#endif
			}
			// todo: check e8b
			#if 0
			if (mibentry.cmode != CHANNEL_MODE_BRIDGE)
			{
				delBrgMacFilterRule(mibentry.ifIndex);
			}
			#endif
DEBUGPRINT;

// todo: check e8b
#ifdef NEW_PORTMAPPING
			//mibentry record current web record;
			//Entry record old record;
			AUG_PRT("^O^ %s:%d. The mibentry.itfgroup is 0x%x, The Entry's itfgroup is 0x%x, the apptype is %d\n", __FILE__,
				__LINE__, mibentry.itfGroup, Entry.itfGroup, Entry.applicationtype);
			//do not need more check work!
			check_itfGroup(&mibentry, &Entry);
#endif

/*star:20090302 START wen INTERNET pvc start, igmp proxy open auto*/
			mibentry.enableIGMP=0;
			if(mibentry.cmode!=CHANNEL_MODE_BRIDGE)
				if (mibentry.applicationtype&X_CT_SRV_INTERNET)
					mibentry.enableIGMP=1;
/*star:20090302 END*/


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
					if(pvcifIdx==dnsPvcEntry.ifIndex)//I get it
					{
						if(mibentry.cmode==CHANNEL_MODE_BRIDGE)
							pvcifIdx = DUMMY_IFINDEX;
						else
							pvcifIdx=mibentry.ifIndex;
						mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif

#ifdef CONFIG_IPV6
#ifdef DNSV6_BIND_PVC_SUPPORT
			MIB_CE_ATM_VC_T dnsv6PvcEntry;
			if(mib_chain_get(MIB_ATM_VC_TBL,selected,&dnsv6PvcEntry)&&(dnsv6PvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
			{
				int tempi=0;
				unsigned int pvcifIdx=0;
				for(tempi=0;tempi<3;tempi++)
				{
					mib_get(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					if(pvcifIdx==dnsv6PvcEntry.ifIndex)//I get it
					{
						if(mibentry.cmode==CHANNEL_MODE_BRIDGE)
							pvcifIdx = DUMMY_IFINDEX;
						else
							pvcifIdx=mibentry.ifIndex;
						mib_set(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif
#endif
	        //jim garbage action...
			//memcpy(&Entry, &entry, sizeof(entry));
			// log message
			// Mason Yu

			// Mason Yu. ITMS4
			deleteConnection(CONFIGONE, &Entry);		// Modify
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&mibentry, selected);
			restartWAN(CONFIGONE, &mibentry);			// Modify
	// todo: check e8b
	#if 0
#ifndef QOS_SETUP_IMQ
			//ql_xu: add IP QoS queue
			if (mibentry.enableIpQos)
				addIpQosQueue(mibentry.ifIndex);
#endif
	#endif
//			restart_dnsrelay(); //star
			DEBUGPRINT;

			goto setOk_filter;
		}
	}
	else {lineno = __LINE__; goto check_err;}

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	strcpy(tmpBuf, "参数错误");
	goto setErr_nochange;
	return;

setOk_filter:

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	// Mason Yu. ITMS4
	//do_wan_restart();
/*star:20081205 START when change wan by web, tr069 associated notify entry should be updated*/
	// todo: check e8b
	//writeWanChangeFile();
/*star:20081205 END*/
	DEBUGPRINT;//ql_xu
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
 	return;

setErr_restart:
	// Mason Yu. ITMS4
	//do_wan_restart();
setErr_nochange:
	ERR_MSG(tmpBuf);
}
#endif // of CONFIG_DEV_xDSL

#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
#define CHECK_CONNECTION_MODE(cmode1, cmode2) (((cmode1 == CHANNEL_MODE_BRIDGE) && (cmode2 == CHANNEL_MODE_BRIDGE))\
						|| ((cmode1 > CHANNEL_MODE_BRIDGE) && (cmode2 > CHANNEL_MODE_BRIDGE)\
							&& cmode1 != CHANNEL_MODE_PPPOE && cmode2 != CHANNEL_MODE_PPPOE\
							&& cmode2 != CHANNEL_MODE_PPPOA && cmode2 != CHANNEL_MODE_PPPOA))
/*****************************
** Internet连接
*/
int initPageEth(int eid, request * wp, int argc, char ** argv)
{
	struct atmvc_entryx	entry;
	int				pppnumleft = 5;
	int				cnt = 0;
	int				index = 0;
	unsigned char	ipAddr[16];		//IP地址
	unsigned char	remoteIpAddr[16];	//缺省网关
	unsigned char	netMask[16];	//子网掩码
	unsigned char	v4dns1[16];
	unsigned char	v4dns2[16];
	unsigned long	fstdns = 0;	//缺省DNS
	unsigned long	secdns = 0;	//可选DNS
	int				lineno = __LINE__;
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
	MIB_CE_PORT_BINDING_T pbEntry;
	int vlan_map;
#endif
	int poe_proxy;

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	/*test code*
	cnt = 1;
	pppnumleft = 7;

	memset(&entry, 0, sizeof(entry));
	entry.svtype = 0;
	entry.vpi = 0;
	entry.vci = 32;
	entry.pcr = 0;
	entry.scr = 0;
	entry.mbs = 0;
	entry.encap = 0;
	entry.napt = 0;
	entry.cmode = 2;
	entry.brmode = 1;
	strcpy(entry.pppUsername, "test");
	strcpy(entry.pppPassword, "test");
	entry.pppAuth = 0;
	strcpy(entry.pppServiceName, "test");
	entry.pppCtype = 0;
	entry.ipDhcp = 0;
	*(unsigned long*)&(entry.ipAddr) = 0x77777777;
	*(unsigned long*)&(entry.remoteIpAddr) = 0x77777777;
	*(unsigned long*)&(entry.netMask) = htonl(0xFFFFFF00);
	entry.vlan = 1;
	entry.vid = 0;
	entry.vprio = 0;
	entry.vpass = 0;
	entry.itfGroup = 0x48;
	entry.qos = 1;
	entry.PPPoEProxyEnable = 1;
	entry.PPPoEProxyMaxUser = 0;
	entry.applicationtype = 2;
	************Place your code here, do what you want to do! ************/
	MIB_CE_ATM_VC_T mibentry;
	int mibtotal,i;
	char wanname[MAX_WAN_NAME_LEN];
#ifdef CONFIG_PTMWAN
	unsigned char ptmset=0;
#endif /*CONFIG_PTMWAN*/
	unsigned int upmodes = 0;

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	mib_get(MIB_PON_MODE, (void *)&upmodes);
#else // ETHWAN or PTMWAN
	upmodes = 0;
#endif

#ifdef CONFIG_PTMWAN
	if(strncmp(wanif,"ptm",3)==0)
			ptmset=1;
#endif /*CONFIG_PTMWAN*/

#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
	vlan_map = 0;
	cnt = mib_chain_total(MIB_PORT_BINDING_TBL);
	for (index=0; index<cnt; index++) {
		mib_chain_get(MIB_PORT_BINDING_TBL, index, (void*)&pbEntry);
		if (pbEntry.pb_mode)
			vlan_map |= (1<<index);
	}
	// put vlan-based port mapping
	_PUT_INT(vlan_map);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
	poe_proxy = 1;
#else
	poe_proxy = 0;
#endif
	_PUT_INT(poe_proxy);
	memset(&entry,0,sizeof(entry));

	_PUT_INT(pppnumleft);
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&mibentry)!=1)
			continue;

#ifdef CONFIG_PTMWAN
		if(ptmset)
		{
			if( MEDIA_INDEX(mibentry.ifIndex) != MEDIA_PTM )
				continue;
		}else
#endif /*CONFIG_PTMWAN*/
		{
			if( MEDIA_INDEX(mibentry.ifIndex) != MEDIA_ETH )
				continue;
		}

//		_PUT_INT(pppnumleft);
	//	_PUT_IP(fstdns);
	//	_PUT_IP(secdns);

//	for(index = 0; index < cnt; index++)
		{
			/************Place your code here, do what you want to do! ************/
			/************Place your code here, do what you want to do! ************/

			getWanName(&mibentry, wanname);
			int tmp=mib2web(&mibentry,&entry);
			if(tmp==-1)
				continue;

			strcpy(v4dns1, inet_ntoa(*((struct in_addr *)mibentry.v4dns1)));
			if (strcmp(v4dns1, "0.0.0.0")==0)
				strcpy(v4dns1, "");

			strcpy(v4dns2, inet_ntoa(*((struct in_addr *)mibentry.v4dns2)));
			if (strcmp(v4dns2, "0.0.0.0")==0)
				strcpy(v4dns2, "");

#ifdef CONFIG_IPV6
			unsigned char 	Ipv6AddrStr[48]={0}, RemoteIpv6AddrStr[48]={0}, RemoteIpv6EndPointAddrStr[48]={0};
			char Ipv6Dns1Str[48]={0}, Ipv6Dns2Str[48]={0};
			unsigned char zeroIpv6Dns[IP6_ADDR_LEN]={0};
			unsigned char	IPv6Str1[40], IPv6Str2[40];
			unsigned char prefixLenStr[5]={0};

			strcpy(Ipv6AddrStr, "");
			strcpy(RemoteIpv6AddrStr, "");
			strcpy(RemoteIpv6EndPointAddrStr, "");
			strcpy(IPv6Str1, "Ipv6Addr");
			strcpy(IPv6Str2, "Ipv6Gateway");
			if(mibentry.Ipv6AddrPrefixLen!=0)
				sprintf(prefixLenStr,"%d",mibentry.Ipv6AddrPrefixLen);

			if ((mibentry.AddrMode & IPV6_WAN_STATIC) == IPV6_WAN_STATIC)
			{
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
				if(memcmp(zeroIpv6Dns, mibentry.Ipv6Dns1, sizeof(zeroIpv6Dns)))
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Dns1, Ipv6Dns1Str, sizeof(Ipv6Dns1Str));
				if(memcmp(zeroIpv6Dns, mibentry.Ipv6Dns2, sizeof(zeroIpv6Dns)))
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Dns2, Ipv6Dns2Str, sizeof(Ipv6Dns2Str));

			}
#ifdef DUAL_STACK_LITE
			else if ((mibentry.AddrMode & IPV6_WAN_DSLITE) == IPV6_WAN_DSLITE)
			{
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)mibentry.RemoteIpv6EndPointAddr, RemoteIpv6EndPointAddrStr, sizeof(RemoteIpv6EndPointAddrStr));
				strcpy(IPv6Str1, "DSLiteLocalIP");
				strcpy(IPv6Str2, "DSLiteGateway");
			}
#endif
#endif
#ifndef CONFIG_IPV6
			boaWrite(wp, "push(new it_nr(\"%s\"" _PTI \
				_PTI _PTI _PTI \
				_PTS _PTS _PTI _PTS _PTS \
				_PTI _PTI \
				_PTS _PTS _PTS _PTI\
				_PTS _PTS _PTI\
				_PTI _PTI _PTI _PTI _PTI _PTI \
				_PTI _PTI _PTI _PTI"));\n",
				wanname, "upmode", upmodes,
				_PME(napt), _PME(cmode), _PME(brmode),
				_PME(pppUsername), _PME(pppPassword), _PME(pppAuth),  _PME(pppServiceName),  _PME(pppACName),
				_PME(pppCtype), _PME(ipDhcp),
				_PMEIP(ipAddr), _PMEIP(remoteIpAddr), _PMEIP(netMask), _PME(dgw),
				"v4dns1", v4dns1, "v4dns2", v4dns2, _PME(dnsMode),
				_PME(vlan), _PME(vid), _PME(mtu), _PME(vprio), _PME(vpass), _PME(itfGroup),
				_PME(qos), _PME(PPPoEProxyEnable), _PME(PPPoEProxyMaxUser), _PME(applicationtype)
				);
#else
			boaWrite(wp, "push(new it_nr(\"%s\"" _PTI  \
				_PTI _PTI _PTI _PTI \
				_PTS _PTS _PTI _PTS _PTS \
				_PTI _PTI \
				_PTS _PTS _PTS _PTI\
				_PTS _PTS _PTI\
				_PTI _PTI _PTI _PTI _PTI _PTI \
				_PTI  _PTI _PTI  _PTI \
				_PTI _PTI _PTI \
				_PTS _PTS \
				_PTS _PTS \
				_PTS \
				_PTI \
				_PTS _PTI \
				_PTI _PTI \
				_PTI _PTS"));\n",
				wanname, "upmode", upmodes,
				_PME(napt), _PME(cmode), _PME(brmode), _PME(AddrMode),
				_PME(pppUsername), _PME(pppPassword), _PME(pppAuth),  _PME(pppServiceName),  _PME(pppACName),
				_PME(pppCtype), _PME(ipDhcp),
				_PMEIP(ipAddr), _PMEIP(remoteIpAddr), _PMEIP(netMask), _PME(dgw),
				"v4dns1", v4dns1, "v4dns2", v4dns2, _PME(dnsMode),
				_PME(vlan), _PME(vid), _PME(mtu), _PME(vprio), _PME(vpass), _PME(itfGroup),
				_PME(qos), _PME(PPPoEProxyEnable), _PME(PPPoEProxyMaxUser), _PME(applicationtype),
				"IpProtocolType", mibentry.IpProtocol , "slacc", (mibentry.AddrMode & IPV6_WAN_AUTO) == IPV6_WAN_AUTO ? 1:0, "staticIpv6", (mibentry.AddrMode & IPV6_WAN_STATIC) == IPV6_WAN_STATIC ? 1:0,
				IPv6Str1, Ipv6AddrStr, IPv6Str2, RemoteIpv6AddrStr,
				"Ipv6Dns1", Ipv6Dns1Str, "Ipv6Dns2", Ipv6Dns2Str,
				"DSLiteRemoteIP", RemoteIpv6EndPointAddrStr,
				"dslite_enable", mibentry.dslite_enable,
				"Ipv6PrefixLen", prefixLenStr, "itfenable", mibentry.Ipv6Dhcp,
				"iana", (mibentry.Ipv6DhcpRequest & 0x1) == 0x1 ? 1:0, "iapd", (mibentry.Ipv6DhcpRequest & 0x2) == 0x2 ? 1:0,
				"dslite_aftr_mode", mibentry.dslite_aftr_mode,
				"dslite_aftr_hostname", mibentry.dslite_aftr_hostname
				);
#endif
		}
	}

check_err:
	_TRACE_LEAVEL;
	return 0;
}

int initPageEth2(int eid, request * wp, int argc, char ** argv)
{
	unsigned char cwmp_configurable = 0;
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	unsigned int upmodes;
	static char *str_upmodes[] = {"LAN", "GPON", "EPON"};

	mib_get(MIB_PON_MODE, (void *)&upmodes);
	boaWrite(wp, "var upmodes = new Array(\"%s\");\n", str_upmodes[upmodes]);
#else
	boaWrite(wp, "var upmodes = new Array(\"LAN\");\n");
#endif

	mib_get(CWMP_CONFIGURABLE, &cwmp_configurable);

	boaWrite(wp, "var apmodes = new Array("
		"\"TR069_INTERNET\", \"INTERNET\", \"TR069\", \"Other\""
#ifdef VOIP_SUPPORT
		", \"VOICE\", \"TR069_VOICE\", \"VOICE_INTERNET\", \"TR069_VOICE_INTERNET\");\n");
#else
		");\n");
#endif
	boaWrite(wp, "var cwmp_configurable = %d;\n", cwmp_configurable);

	return 0;
}

int initVlanRange(int eid, request * wp, int argc, char ** argv)
{
	unsigned int untag_wan_vid, fwdvlan_cpu, fwdvlan_proto_block, fwdvlan_bind_internet, fwdvlan_bind_other;
	unsigned int lan_vlan_id1, lan_vlan_id2;
#ifdef CONFIG_RTK_RG_INIT
	mib_get(MIB_FWD_CPU_VLAN_ID, (void *)&fwdvlan_cpu);
	mib_get(MIB_FWD_PROTO_BLOCK_VLAN_ID, (void *)&fwdvlan_proto_block);
	mib_get(MIB_FWD_BIND_INTERNET_VLAN_ID, (void *)&fwdvlan_bind_internet);
	mib_get(MIB_FWD_BIND_OTHER_VLAN_ID, (void *)&fwdvlan_bind_other);
	mib_get(MIB_UNTAG_WAN_VLAN_ID, (void *)&untag_wan_vid);	
	mib_get(MIB_LAN_VLAN_ID1, (void *)&lan_vlan_id1);	
	mib_get(MIB_LAN_VLAN_ID2, (void *)&lan_vlan_id2);	

	boaWrite(wp, "var reservedVlanA = [%d, %d, %d, %d, %d, %d, %d, %d];\n", 0, fwdvlan_cpu,lan_vlan_id1, untag_wan_vid ,lan_vlan_id2 , fwdvlan_proto_block,  fwdvlan_bind_internet,4095);
	boaWrite(wp, "var otherVlanStart = %d;\n",fwdvlan_bind_other);
	boaWrite(wp, "var otherVlanEnd = %d;\n",fwdvlan_bind_other+DEFAULT_BIND_LAN_OFFSET);
	boaWrite(wp, "var alertVlanStr = \"%d, %d, %d, %d, %d, %d, %d, %d ~ %d, %d\";\n",0, fwdvlan_cpu,lan_vlan_id1, untag_wan_vid ,lan_vlan_id2 , fwdvlan_proto_block,  fwdvlan_bind_internet,fwdvlan_bind_other,fwdvlan_bind_other+DEFAULT_BIND_LAN_OFFSET,4095);
#else
	/*For no RG project, you must set the reserved vlan here, 
	or the web page would have problem*/
	unsigned int bind_other_offset=10;
	fwdvlan_bind_other = 4000
	boaWrite(wp, "var reservedVlanA = [%d, %d, %d];\n", 0, lan_vlan_id1 ,4095);
	boaWrite(wp, "var otherVlanStart = %d;\n",fwdvlan_bind_other);
	boaWrite(wp, "var otherVlanEnd = %d;\n",fwdvlan_bind_other+bind_other_offset);
	boaWrite(wp, "var alertVlanStr = \"%d, %d, %d ~ %d, %d\";\n",0, lan_vlan_id1, fwdvlan_bind_other,fwdvlan_bind_other+bind_other_offset,4095);
#endif
	//printf("initVlanRange:done\n");

	return 0;
}


int initPageQoSAPP(int eid, request * wp, int argc, char ** argv)
{
#ifdef VOIP_SUPPORT
	boaWrite(wp, "var appNames = new Array(\"\", \"VOIP\", \"TR069\");\n");
#else
	boaWrite(wp, "var appNames = new Array(\"\",  \"TR069\");\n");
#endif
	return 0;
}

#ifdef CONFIG_IPV6
void clear_delegated_default_wanconn(MIB_CE_ATM_VC_Tp mibentry_p)
{
	//If use this WAN as default conn in prefix delegated, clear this value.

	unsigned char lanIPv6PrefixMode;
	unsigned int old_wan_conn=0;

	if(((mibentry_p->cmode!=CHANNEL_MODE_BRIDGE) && (mibentry_p->IpProtocol&IPVER_IPV6)) && (mibentry_p->applicationtype & X_CT_SRV_INTERNET)){
		if (!mib_get(MIB_PREFIXINFO_PREFIX_MODE, (void *)&lanIPv6PrefixMode))
			printf("Error! Fail to et MIB_PREFIXINFO_PREFIX_MODE!\n");
		if(lanIPv6PrefixMode == IPV6_PREFIX_DELEGATION){
			if (!mib_get(MIB_PREFIXINFO_DELEGATED_WANCONN, (void *)&old_wan_conn))
				printf("Error! Fail to get MIB_PREFIXINFO_DELEGATED_WANCONN!\n");
			if(old_wan_conn && (old_wan_conn==mibentry_p->ifIndex)){
				printf("Prefix Mode is WANDelegated and using this  WAN Connection %x, now clear this!, %x\n",mibentry_p->ifIndex);
				old_wan_conn = 0;
				if (!mib_set(MIB_PREFIXINFO_DELEGATED_WANCONN, (void *)&old_wan_conn))
					printf("Error! Fail to set MIB_PREFIXINFO_DELEGATED_WANCONN!\n");
			}
		}
	}

}

void setup_delegated_default_wanconn(MIB_CE_ATM_VC_Tp mibentry_p)
{
	//In Spec , if prefix mode WANDelegated, and the default WANN conn
	//			is the one have INTERNET connection type

	unsigned char lanIPv6PrefixMode;
	unsigned int old_wan_conn=0;

	if(((mibentry_p->cmode!=CHANNEL_MODE_BRIDGE) && (mibentry_p->IpProtocol&IPVER_IPV6)) && (mibentry_p->applicationtype & X_CT_SRV_INTERNET)){
		if (!mib_get(MIB_PREFIXINFO_PREFIX_MODE, (void *)&lanIPv6PrefixMode))
			printf("Error! Fail to et MIB_PREFIXINFO_PREFIX_MODE!\n");
		if(lanIPv6PrefixMode == IPV6_PREFIX_DELEGATION){
			if (!mib_get(MIB_PREFIXINFO_DELEGATED_WANCONN, (void *)&old_wan_conn))
				printf("Error! Fail to get MIB_PREFIXINFO_DELEGATED_WANCONN!\n");

			if(old_wan_conn==0){
				printf("Prefix Mode is WANDelegated but not set WAN Connection yet, now use this WAN, %x\n",mibentry_p->ifIndex);
				if (!mib_set(MIB_PREFIXINFO_DELEGATED_WANCONN, (void *)&mibentry_p->ifIndex))
					printf("Error! Fail to set MIB_PREFIXINFO_DELEGATED_WANCONN!\n");
			}
		}
	}
}
#endif

void formEthernet(request * wp, char *path, char *query)
{
	struct atmvc_entryx	entry;
	char*			pifname = NULL;
	char*			stemp = "";
	char 			*submitUrl;
	unsigned long	fstdns = 0;	//缺省DNS
	unsigned long	secdns = 0;	//可选DNS
	int				ival = 0;
#ifdef CONFIG_USER_PPPOE_PROXY
	int				pppnummax = 5;//default value
#endif
	int				lineno = __LINE__;

	MIB_CE_ATM_VC_T mibentry,Entry;
	int totalEntry,i;
	char tmpBuf[100];
	int remained=0;
	unsigned int ifMap;
	char* strValue;
	char *dns1Ip, *dns2Ip;
	unsigned char mode;
#ifdef CONFIG_PTMWAN
	unsigned char ptmset=0;
#endif /*CONFIG_PTMWAN*/


	char act[10];
	MEDIA_TYPE_T mType;
/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
	int acflag=0;
	FETCH_INVALID_OPT(stemp, "acnameflag", _NEED);
	if(strcmp(stemp,"have")==0)
		acflag=1;
/*star:20080718 END*/
	_TRACE_CALL;
	FETCH_INVALID_OPT(stemp, "action", _NEED);
	strncpy(act,stemp,10);

#ifdef CONFIG_PTMWAN
	if(strncmp(wanif,"ptm",3)==0)
		ptmset=1;
#endif /*CONFIG_PTMWAN*/

	if(strcmp(stemp, "rm") == 0)	//remove
	{
		int idx=-1;
		char webwanname[MAX_WAN_NAME_LEN];
		char mibwanname[MAX_WAN_NAME_LEN];

		/************Place your code here, do what you want to do! ************/
		/*use 'stemp' as 'link name' to match 'atmvc_entry' entry and remove relevant entry from MIB */
		/************Place your code here, do what you want to do! ************/

		FETCH_INVALID_OPT(stemp, "lst", _NEED);
		strncpy(webwanname,stemp,MAX_WAN_NAME_LEN-1);

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		for(i=0;i<totalEntry;i++){
			if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
				continue;
			getWanName(&Entry,mibwanname);
			if((!strcmp(mibwanname,webwanname))){
				idx=i;
				break;
			}
		}

		//Delete QoS rule if CTQOS_MODE has string INTERNET and this WAN is with type INTERNET
		if((Entry.applicationtype & X_CT_SRV_INTERNET)){
			char qos_mode[MAX_NAME_LEN]={0};

			if(mib_get(CTQOS_MODE, qos_mode)){
				if(strcasestr(qos_mode,"INTERNET")){
					printf("delete MIB for QoS Mode INTERNET\n");
					delQoSRuleByMode("INTERNET");
				}
			}
		}

		if(idx!=-1){
			resolveServiceDependency(idx);

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

#ifdef CONFIG_IPV6
#ifdef DNSV6_BIND_PVC_SUPPORT
			MIB_CE_ATM_VC_T dnsv6PvcEntry;
			if(mib_chain_get(MIB_ATM_VC_TBL,idx,&dnsv6PvcEntry)&&(dnsv6PvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
			{
				int tempi=0;
				unsigned int pvcifIdx=0;
				for(tempi=0;tempi<3;tempi++)
				{
					mib_get(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					if(pvcifIdx==dnsv6PvcEntry.ifIndex)//I get it
					{
						pvcifIdx = DUMMY_IFINDEX;
						mib_set(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif
#endif

//#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
			{
				MIB_CE_ATM_VC_T cwmp_entry;
				if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&cwmp_entry))
				{
					// todo: check e8b
					#if 0
					delBrgMacFilterRule(cwmp_entry.ifIndex);
					delIpRouteTbl(cwmp_entry.ifIndex);
				#ifdef CONFIG_USER_DDNS
					delDDNSinter(cwmp_entry.ifIndex);
				#endif
				#ifdef BR_ROUTE_ONEPVC
					modify_Br_Rt_entry(&cwmp_entry);
				#endif
					#endif
				}
			}
//#endif
			// Mason Yu. ITMS4
			{
				MIB_CE_ATM_VC_T vcEntry;
				if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&vcEntry))
				{
#ifdef CONFIG_IPV6
					clear_delegated_default_wanconn(&vcEntry);
#endif

#ifdef NEW_IP_QOS_SUPPORT//ql 20081125
					delIpQosTcRule(&vcEntry);
#endif
					deleteConnection(CONFIGONE, &vcEntry);
				}
			}

			if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1) {
					strcpy(tmpBuf, strDelChainerror);
					goto setErr_restart;
			}


//ql add: check if it is necessary to delete a group of interface
#ifdef ITF_GROUP
			{
				int wanPortNum;
				unsigned int swNum, vcNum;
				MIB_CE_SW_PORT_T Entry;
				MIB_CE_ATM_VC_T pvcEntry;
				int j, grpnum;
				char mygroup;
				int enable_portmap =0;

				vcNum = mib_chain_total(MIB_ATM_VC_TBL);
				for (grpnum=1; grpnum<=4; grpnum++) {
					wanPortNum = 0;

					for (j=0; j<vcNum; j++) {
						if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
						{
							//boaError(wp, 400, "Get chain record error!\n");
							printf("Get chain record error!\n");
							return;
						}
						if(pvcEntry.itfGroup!=0)
							enable_portmap++;
						if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
							continue;

						if (pvcEntry.applicationtype & (X_CT_SRV_INTERNET|X_CT_SRV_OTHER))
							wanPortNum++;

					}
					//printf("\nwanPortNum=%d\n",wanPortNum);
					if (0 == wanPortNum) {
						//printf("delete port mapping group %d\n", grpnum);
						//release LAN ports
						swNum = mib_chain_total(MIB_SW_PORT_TBL);
						for (j=swNum; j>0; j--) {
							if (!mib_chain_get(MIB_SW_PORT_TBL, j-1, (void *)&Entry))
								return;
							if (Entry.itfGroup == grpnum) {
								Entry.itfGroup = 0;
								mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, j-1);
							}
						}
#ifdef WLAN_SUPPORT
						//release wlan0
						mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
						if (mygroup == grpnum) {
							mygroup = 0;
							mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
						}
#endif
#ifdef WLAN_MBSSID
						//release MBSSID
						for (j=1; j<5; j++) {
							mib_get(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
							if (mygroup == grpnum) {
								mygroup = 0;
								mib_set(MIB_WLAN_VAP0_ITF_GROUP+j-1, (void *)&mygroup);
							}
						}
#endif
						for (j=0; j<vcNum; j++) {
							if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
							{
								//boaError(wp, 400, "Get chain record error!\n");
								printf("Get chain record error!\n");
								return;
							}
							if(pvcEntry.itfGroup==grpnum){
								printf("\nmodify tr069 portmapping!\n");
								pvcEntry.itfGroup=0;
								mib_chain_update(MIB_ATM_VC_TBL,(void *)&pvcEntry,j);
							}
						}
						//setgroup("", grpnum, lowPrio);
						setgroup("", grpnum);

					}
				}

				if(!enable_portmap)
				{
				    printf("\nstop portmapping!\n");

			            mib_get(MIB_MPMODE, (void *)&mode);
			             mode &= 0xfe;
			            mib_set(MIB_MPMODE, (void *)&mode);
				}
			}//end
#endif
		}else
		{
			strcpy(tmpBuf, strSelectvc);
			goto setErr_nochange;
		}

		// Mason Yu. ITMS4
		restartWAN(CONFIGONE, NULL);
		goto setOk_filter;

	}
	else if(strcmp(stemp, "sv") == 0)	//add or modify
		{
		memset(&entry, 0, sizeof(entry));
		memset(&mibentry, 0, sizeof(mibentry));

		//_GET_PSTR(ifname, _OPT);
		//(fstdns, _OPT);
		//_GET_IP(secdns, _OPT);
		// Mason Yu. for IPV6
#ifdef CONFIG_IPV6
		strValue = boaGetVar(wp, "IpProtocolType", "");
		if (strValue[0]) {
			mibentry.IpProtocol = strValue[0] - '0';
		}

#endif

		_ENTRY_BOOL(napt, _NEED);

		_ENTRY_INT(cmode, _NEED);
		if(entry.cmode > 2){lineno = __LINE__; goto check_err;}

		switch(entry.cmode)
		{
		case CHANNEL_MODE_BRIDGE://bridge
			{
			//	_ENTRY_BOOL(brmode, _NEED);
			}break;
		case CHANNEL_MODE_IPOE://route
			{
#ifdef CONFIG_IPV6
				if (mibentry.IpProtocol & IPVER_IPV4) {
#endif
				_ENTRY_INT(ipDhcp, _NEED);
				if(entry.ipDhcp > 1){lineno = __LINE__; goto check_err;}
				if(entry.cmode == 1)//static ip
				{
					//_ENTRY_IP(ipAddr, _NEED);
					FETCH_INVALID_OPT(stemp, "ipAddr", _NEED);
					//printf("\nweb ip %s\n",stemp);
					*(unsigned long*)&(entry.ipAddr) = inet_addr(stemp);
					//printf("\nentry ip:%x\n",*(unsigned long*)&(entry.ipAddr));
					_ENTRY_IP(remoteIpAddr, _NEED);
					_ENTRY_IP(netMask, _NEED);
				}
#ifdef CONFIG_IPV6
				}
#endif
			}break;
		case CHANNEL_MODE_PPPOE://pppoe
			{
/*star:20090302 START ppp username and password can be empty*/
				//_ENTRY_STR(pppUsername, _NEED);
				//_ENTRY_STR(pppPassword, _NEED);
				_ENTRY_STR(pppUsername, _OPT);
				_ENTRY_STR(pppPassword, _OPT);
/*star:20090302 END*/

				_ENTRY_INT(pppCtype, _NEED);
				if(entry.pppCtype > 1){lineno = __LINE__; goto check_err;}
				_ENTRY_STR(pppServiceName, _OPT);
/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
				if(acflag==1)
					_ENTRY_STR(pppACName, _OPT);
/*star:20080718 END*/
#ifdef CONFIG_USER_PPPOE_PROXY
				_ENTRY_BOOL(PPPoEProxyEnable, _NEED);
				if(entry.PPPoEProxyEnable)
				{
					_ENTRY_INT(PPPoEProxyMaxUser, _NEED);
					if(entry.PPPoEProxyMaxUser < 0 || entry.PPPoEProxyMaxUser > (unsigned)pppnummax){lineno = __LINE__; goto check_err;}
				}
				_ENTRY_BOOL(brmode, _NEED);
#endif
			}break;
		}

		_ENTRY_BOOL(vlan, _NEED);
		if(entry.vlan)
		{
			_ENTRY_INT(vid, _NEED);
			if(entry.vid > 4095){lineno = __LINE__; goto check_err;}

			_ENTRY_INT(vprio, _NEED);// entry.vprio == (无)
			if(entry.vprio > 8){lineno = __LINE__; goto check_err;}
		}
		_ENTRY_INT(mtu, _NEED);
		if(entry.mtu > 1500 || entry.mtu < 576){lineno = __LINE__; goto check_err;}
		_ENTRY_INT(vpass, _OPT);

		_ENTRY_INT(itfGroup, _NEED);


		_ENTRY_BOOL(qos, _NEED);

		_ENTRY_INT(applicationtype, _NEED);
		web2mib(&entry,&mibentry);
#ifdef CONFIG_USER_PPPOE_PROXY
		if(entry.PPPoEProxyEnable){
			if(mibentry.itfGroup > 0){
				system("echo 1 > /proc/rg/pppoe_proxy_only_for_binding_packet");
				printf("echo 1 > /proc/rg/pppoe_proxy_only_for_binding_packet\n");
			}else{
				system("echo 0 > /proc/rg/pppoe_proxy_only_for_binding_packet");
				printf("echo 0 > /proc/rg/pppoe_proxy_only_for_binding_packet\n");
			}
		}
#endif
		if (mibentry.cmode == CHANNEL_MODE_IPOE)
		{
			strValue = boaGetVar(wp, "dnsMode", "");
			if (strValue[0]) {
				mibentry.dnsMode = strValue[0] - '0';
			}

			dns1Ip = boaGetVar(wp, "v4dns1", "");
			if (dns1Ip[0]) {
				if (!inet_aton(dns1Ip, (struct in_addr *)&mibentry.v4dns1)) {
					strcpy(tmpBuf, "不合法的dnsv4 1 IP地址!"); //Invalid dnsv4 1 IP-address value!
					goto setErr_nochange;
				}
			}

			dns2Ip = boaGetVar(wp, "v4dns2", "");
			if (dns2Ip[0]) {
				if (!inet_aton(dns2Ip, (struct in_addr *)&mibentry.v4dns2)) {
					strcpy(tmpBuf, "不合法的dnsv4 2 IP地址!"); //Invalid dnsv4 2 IP-address value!
					goto setErr_nochange;
				}
			}
		}
		else
			mibentry.dnsMode = 1; // default is enable dnsMode

		if(mibentry.applicationtype & ~CT_SRV_MASK)
			{lineno = __LINE__; goto check_err;}

		// E8B: if 'INTERNET', set as default route.
#ifndef CONFIG_RTK_RG_INIT
		if((mibentry.applicationtype & X_CT_SRV_INTERNET) && entry.cmode != CHANNEL_MODE_BRIDGE)
			mibentry.dgw = 1;
		else
			mibentry.dgw = 0;
#endif
		//_ENTRY_BOOL(dgw,_NEED);

		/************Place your code here, do what you want to do! ************/
		/************Place your code here, do what you want to do! ************/

		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		mibentry.enable=1;//always enable

#ifdef CONFIG_IPV6
		if (mibentry.cmode != CHANNEL_MODE_BRIDGE) {
			if (mibentry.IpProtocol & IPVER_IPV6)
				retrieveIPv6Record(wp, &mibentry);
		}
#endif

		FETCH_INVALID_OPT(stemp, "lkname", _NEED);
		if(strcmp(stemp,"new")==0){   //add
			int cnt, pIdx;
			unsigned char vcIdx;
			int intVal;

			if (totalEntry >= MAX_VC_NUM)
			{
				strcpy(tmpBuf, strMaxVc);
				goto setErr_nochange;
			}
			// check if connection exists
			ifMap = 0;
			cnt=0;
#ifdef CONFIG_RTK_RG_INIT
			remained = Check_RG_Intf_Count();
			if(remained == 0){
				/*Table FULL*/
				strcpy(tmpBuf, strTableFull);
				goto setErr_nochange;
			}
#endif
			if(entry.vlan==1){
				for (i=0; i<totalEntry; i++) {
					if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					{
						boaError(wp, 400, strGetChainerror);
						return;
					}
					mType = MEDIA_INDEX(Entry.ifIndex);

					// Mason Yu. ITMS4
					#ifdef CONFIG_PTMWAN
					if(ptmset)
					{
						if (mType == MEDIA_PTM && Entry.vlan==1 && Entry.vid == entry.vid && CHECK_CONNECTION_MODE(Entry.cmode, entry.cmode)) {
							strcpy(tmpBuf, strConnectExist);
							goto setErr_nochange;
						}
					}else
					#endif
					{
						if (mType == MEDIA_ETH && Entry.vlan==1 && Entry.vid == entry.vid && CHECK_CONNECTION_MODE(Entry.cmode, entry.cmode)) {
							strcpy(tmpBuf, strConnectExist);
							goto setErr_nochange;
						}
					}

					// Mason Yu. ITMS4
					#ifdef CONFIG_PTMWAN
					if(ptmset)
					{
						if (mType == MEDIA_PTM)
							ifMap |= 1 << PTM_INDEX(Entry.ifIndex);	// vc map
					}else
					#endif
					{
						if (mType == MEDIA_ETH)
							ifMap |= 1 << ETH_INDEX(Entry.ifIndex);	// vc map
					}
					ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex); // PPP map
				}
				mibentry.vlan = entry.vlan;
				mibentry.vid = entry.vid;
				mibentry.vprio = entry.vprio;
			}
			else{
				for (i=0; i<totalEntry; i++) {
					if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					{
						boaError(wp, 400, strGetChainerror);
						return;
					}
					mType = MEDIA_INDEX(Entry.ifIndex);

					// Mason Yu. ITMS4
					#ifdef CONFIG_PTMWAN
					if(ptmset)
					{
						if (mType == MEDIA_PTM && Entry.vlan==0 && CHECK_CONNECTION_MODE(Entry.cmode, entry.cmode)) {
							strcpy(tmpBuf, strConnectExist);
							goto setErr_nochange;
						}
					}else
					#endif
					{
						if (mType == MEDIA_ETH && Entry.vlan==0 && CHECK_CONNECTION_MODE(Entry.cmode, entry.cmode)) {
							strcpy(tmpBuf, strConnectExist);
							goto setErr_nochange;
						}
					}

					// Mason Yu. ITMS4
					#ifdef CONFIG_PTMWAN
					if(ptmset)
					{
						if (mType == MEDIA_PTM)
							ifMap |= 1 << PTM_INDEX(Entry.ifIndex);	// vc map
					}else
					#endif
					{
						if (mType == MEDIA_ETH)
							ifMap |= 1 << ETH_INDEX(Entry.ifIndex);	// vc map
					}
					ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex); // PPP map
				}
				mibentry.vlan = 0;
				mibentry.vid = 0;
				mibentry.vprio = 0;
			}

			if (cnt == 0)	// pvc not exists
			{
				mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
				// Mason Yu. ITMS4
				#ifdef CONFIG_PTMWAN
				if(ptmset)
					mibentry.ifIndex = TO_IFINDEX(MEDIA_PTM, PPP_INDEX(mibentry.ifIndex), ETH_INDEX(mibentry.ifIndex));
				else
				#endif
					mibentry.ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(mibentry.ifIndex), ETH_INDEX(mibentry.ifIndex));
				AUG_PRT("The mibentry.ifIndex is 0x%x\n", mibentry.ifIndex);
				if (mibentry.ifIndex == NA_VC) {
					strcpy(tmpBuf, strMaxVc);
					goto setErr_nochange;
				}
				else if (mibentry.ifIndex == NA_PPP) {
					strcpy(tmpBuf, strMaxNumPPPoE);
					goto setErr_nochange;
				}

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
				mibentry.ConDevInstNum = 1 + findMaxConDevInstNum(MEDIA_INDEX(mibentry.ifIndex));
				if( (mibentry.cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
				    ((mibentry.cmode==CHANNEL_MODE_BRIDGE)&&(mibentry.brmode==BRIDGE_PPPOE)) ||
#endif
				    (mibentry.cmode==CHANNEL_MODE_PPPOA) )
					mibentry.ConPPPInstNum = 1;
				else
					mibentry.ConIPInstNum = 1;
				//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, mibentry.ConDevInstNum, mibentry.ConPPPInstNum, mibentry.ConIPInstNum );
#endif
			}

			// set default
			if (mibentry.cmode == CHANNEL_MODE_PPPOE)
			{
				if(mibentry.mtu > 1492)
					mibentry.mtu = 1492;
			}
#if 0
			if (mibentry.cmode == CHANNEL_MODE_PPPOE)
			{
				mibentry.mtu = 1492;
/*
#ifdef CONFIG_USER_PPPOE_PROXY
				mibentry.PPPoEProxyMaxUser=4;
				mibentry.PPPoEProxyEnable=0;
#endif
*///set by web
			}
			else
				mibentry.mtu = 1500;
#endif
/*
#ifdef CONFIG_EXT_SWITCH
			// VLAN
			mibentry.vlan = 0;	// disable
			mibentry.vid = 0; // VLAN tag
			mibentry.vprio = 0; // priority bits (0 ~ 7)
			mibentry.vpass = 0; // no pass-through
#endif
*///set by web
// todo: check e8b

#ifdef NEW_PORTMAPPING
			check_itfGroup(&mibentry, 0);
#endif

/*			if(mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&mibentry) != 1){
				strcpy(tmpBuf, strAddChainerror);
				goto setErr_filter;
			}
*/
/*star:20090302 START wen INTERNET pvc start, igmp proxy open auto*/
			mibentry.enableIGMP=0;
			if(mibentry.cmode!=CHANNEL_MODE_BRIDGE)
				if (mibentry.applicationtype&X_CT_SRV_INTERNET)
					mibentry.enableIGMP=1;
/*star:20090302 END*/
			// Mason Yu. ITMS4
#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
			{
//AUG_PRT("================>>>>>>>%s-%d mibentry.ifIndex=0x%x\n",__func__,__LINE__,mibentry.ifIndex);
			char macaddr[MAC_ADDR_LEN];
			mib_get(MIB_ELAN_MAC_ADDR, (void *)macaddr);
			macaddr[MAC_ADDR_LEN-1] += WAN_HW_ETHER_START_BASE + ETH_INDEX(mibentry.ifIndex);
			memcpy(mibentry.MacAddr, macaddr, MAC_ADDR_LEN);
//AUG_PRT("================>>>>>>>%s-%d %02X:%02X:%02X:%02X:%02X:%02X\n",__func__,__LINE__,macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
			}
#endif

#ifdef CONFIG_IPV6
			setup_delegated_default_wanconn(&mibentry);
#endif

			intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&mibentry);
			if (intVal == 0) {
				strcpy(tmpBuf, strAddChainerror);
				goto setErr_restart;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, strTableFull);
				goto setErr_restart;
			}
			// Kaohj -- Queue LEN table not supportted.
			#if 0
#ifndef QOS_SETUP_IMQ
			//ql_xu: add qos queue
			if (mibentry.enableIpQos)
				addIpQosQueue(mibentry.ifIndex);
#endif
			#endif

			//Update QoS rule if mib CTQOS_MODE has string INTERNET and this new added WAN is with type INTERNET
			if((mibentry.applicationtype & X_CT_SRV_INTERNET)){
				char qos_mode[MAX_NAME_LEN]={0};

				if(mib_get(CTQOS_MODE, qos_mode)){
					if(strcasestr(qos_mode,"INTERNET")){
						printf("update MIB for QoS Mode %s\n",qos_mode);
						mib_get(CTQOS_MODE, (void *)qos_mode);
						updateMIBforQosMode(qos_mode);
					}
				}
			}

			// Mason Yu. ITMS4
			restartWAN(CONFIGONE, &mibentry);    // Add
			goto setOk_filter;

		}else{   //modify
			int cnt=0, pIdx;
			int selected=-1;
			int itsMe;
			MIB_CE_ATM_VC_T myEntry;
			char webwanname[MAX_WAN_NAME_LEN];
			char mibwanname[MAX_WAN_NAME_LEN];

			ifMap=0;
			FETCH_INVALID_OPT(stemp, "lst", _NEED);
			strncpy(webwanname,stemp,MAX_WAN_NAME_LEN-1);

			for (i=0; i<totalEntry; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				{
					boaError(wp, 400, strGetChainerror);
					return;
				}
				getWanName(&Entry,mibwanname);
				//printf("\nmibname=%s\n",mibwanname);
		DEBUGPRINT;
				mType = MEDIA_INDEX(Entry.ifIndex);
				itsMe = 0;
				if (!strcmp(webwanname,mibwanname)) {
					itsMe = 1;
					if (selected == -1)
						selected = i;
					else{
						strcpy(tmpBuf, strConnectExist);
						goto setErr_nochange;
					}
				}
				// Mason Yu. ITMS4
				#ifdef CONFIG_PTMWAN
				if(ptmset)
				{
					if ((mType == MEDIA_PTM) &&
						(Entry.vlan == mibentry.vlan) && (Entry.vid == mibentry.vid) && CHECK_CONNECTION_MODE(Entry.cmode, mibentry.cmode) &&
						!itsMe){
						strcpy(tmpBuf, strConnectExist);
						goto setErr_nochange;
					}
				}else
				#endif
				{
					if ((mType == MEDIA_ETH) &&
						(Entry.vlan == mibentry.vlan) && (Entry.vid == mibentry.vid) && CHECK_CONNECTION_MODE(Entry.cmode, mibentry.cmode) &&
						!itsMe){
						strcpy(tmpBuf, strConnectExist);
						goto setErr_nochange;
					}
				}

				if (!itsMe) {
		DEBUGPRINT;
					// Mason Yu. ITMS4
					#ifdef CONFIG_PTMWAN
					if(ptmset)
					{
						if (mType == MEDIA_PTM)
							ifMap |= 1 << PTM_INDEX(Entry.ifIndex);	// vc map
					}else
					#endif
					{
						if (mType == MEDIA_ETH)
							ifMap |= 1 << ETH_INDEX(Entry.ifIndex);	// vc map
					}
					ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex); // PPP map
				}
			}

		DEBUGPRINT;
			if (!mib_chain_get(MIB_ATM_VC_TBL, selected, (void *)&Entry)) {
				strcpy(tmpBuf, errGetEntry);
				goto setErr_nochange;
			}
			// restore stuff not posted in this form
			if (mibentry.enable
				&& ((CHANNEL_MODE_IPOE == mibentry.cmode)
				|| (CHANNEL_MODE_PPPOA == mibentry.cmode)
				|| (CHANNEL_MODE_PPPOE == mibentry.cmode)
				|| (CHANNEL_MODE_RT1483 == mibentry.cmode)))
			{
				/* restore igmp-proxy setting */
				mibentry.enableIGMP = Entry.enableIGMP;
			}

			if (mibentry.applicationtype == Entry.applicationtype)
			{
				/* application type not changed, reserve the dhcp setting on lan interface */
				mibentry.disableLanDhcp = Entry.disableLanDhcp;
			}

			if (mibentry.cmode == CHANNEL_MODE_PPPOE)
				if (cnt > 0) {		// Jenny, for multisession PPPoE, ifIndex(VC device) must refer to existed PPPoE connection
					{
						ifMap &= 0xffff0000; // don't care the vc part
						mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
						// Mason Yu. ITMS4
						#ifdef CONFIG_PTMWAN
						if(ptmset)
							mibentry.ifIndex = TO_IFINDEX(MEDIA_PTM, PPP_INDEX(mibentry.ifIndex), PTM_INDEX(myEntry.ifIndex));
						else
						#endif
							mibentry.ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(mibentry.ifIndex), ETH_INDEX(myEntry.ifIndex));
					}
				}
				else {
		DEBUGPRINT;
					mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
					// Mason Yu. ITMS4
					#ifdef CONFIG_PTMWAN
					if(ptmset)
						mibentry.ifIndex = TO_IFINDEX(MEDIA_PTM, PPP_INDEX(mibentry.ifIndex), PTM_INDEX(mibentry.ifIndex));
					else
					#endif
						mibentry.ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(mibentry.ifIndex), ETH_INDEX(mibentry.ifIndex));
/*
#ifdef PPPOE_PASSTHROUGH
					if (mibentry.cmode == Entry.cmode)
						mibentry.brmode = Entry.brmode;
#endif
*/
				}
			else{
				mibentry.ifIndex = Entry.ifIndex;
				// Mason Yu. ITMS4
				#ifdef CONFIG_PTMWAN
				if(ptmset)
					mibentry.ifIndex = TO_IFINDEX(MEDIA_PTM, PPP_INDEX(mibentry.ifIndex), PTM_INDEX(mibentry.ifIndex));
				else
				#endif
					mibentry.ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(mibentry.ifIndex), ETH_INDEX(mibentry.ifIndex));
			}
	DEBUGPRINT;
/*
			entry.qos = Entry.qos;
			entry.pcr = Entry.pcr;
			entry.scr = Entry.scr;
			entry.mbs = Entry.mbs;
			entry.cdvt = Entry.cdvt;
*///set by web
			mibentry.pppAuth = Entry.pppAuth;
			mibentry.rip = Entry.rip;
			//entry.dgw = Entry.dgw;
			//mibentry.mtu = Entry.mtu;
			if (mibentry.cmode == CHANNEL_MODE_PPPOE){
				if(mibentry.mtu > 1492)
					mibentry.mtu = 1492;
			}

#ifdef CONFIG_EXT_SWITCH
			//ql: when pvc is modified, interface group don't changed???
			//mibentry.itfGroup = Entry.itfGroup;
#endif

	DEBUGPRINT;
#ifdef CONFIG_SPPPD_STATICIP
			if(mibentry.cmode == CHANNEL_MODE_PPPOE)
			{
				mibentry.pppIp = Entry.pppIp;
				strcpy( mibentry.ipAddr, Entry.ipAddr);
			}
#endif
	DEBUGPRINT;

#ifdef PPPOE_PASSTHROUGH
			//printf("\nentry.cmode=%d,Entry.cmode=%d,Entry.brmode=%d\n",mibentry.cmode,Entry.cmode,Entry.brmode);
			//if (mibentry.cmode != CHANNEL_MODE_PPPOE && mibentry.cmode != CHANNEL_MODE_BRIDGE)
			if (mibentry.cmode != CHANNEL_MODE_PPPOE)
				if (mibentry.cmode == Entry.cmode)
					mibentry.brmode = Entry.brmode;
#endif
DEBUGPRINT;
#ifdef CONFIG_EXT_SWITCH
/*
			// VLAN
			entry.vlan = Entry.vlan;
			entry.vid = Entry.vid;
			entry.vprio = Entry.vprio;
			entry.vpass = Entry.vpass;
*///set by web
#ifdef _PRMT_X_CT_COM_ETHLINK_
			//mibentry.pmark = Entry.pmark;
#endif
#endif
	DEBUGPRINT;
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
			mibentry.connDisable = 0;
			if (Entry.vpi == mibentry.vpi && Entry.vci == mibentry.vci)
			{
				mibentry.ConDevInstNum = Entry.ConDevInstNum;
				if( (mibentry.cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
				    ((mibentry.cmode==CHANNEL_MODE_BRIDGE)&&(mibentry.brmode==BRIDGE_PPPOE)) ||
#endif
				    (mibentry.cmode==CHANNEL_MODE_PPPOA) )
				{
					if( Entry.ConPPPInstNum!=0 )
						mibentry.ConPPPInstNum = Entry.ConPPPInstNum;
					else
						mibentry.ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum);
					mibentry.ConIPInstNum = 0;
				}else{
					mibentry.ConPPPInstNum = 0;
					if(Entry.ConIPInstNum!=0)
						mibentry.ConIPInstNum = Entry.ConIPInstNum;
					else
						mibentry.ConIPInstNum = 1 + findMaxIPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum );
				}
			}else{
	DEBUGPRINT;
				unsigned int  instnum=0;
				instnum = findConDevInstNumByPVC( mibentry.vpi, mibentry.vci );
				if(instnum==0)
					instnum = 1 + findMaxConDevInstNum(MEDIA_INDEX(mibentry.ifIndex));
				mibentry.ConDevInstNum = instnum;
				if( (mibentry.cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
				    ((mibentry.cmode==CHANNEL_MODE_BRIDGE)&&(mibentry.brmode==BRIDGE_PPPOE)) ||
#endif
				    (mibentry.cmode==CHANNEL_MODE_PPPOA) )
				{
					mibentry.ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum);
					mibentry.ConIPInstNum = 0;
				}else{
					mibentry.ConPPPInstNum = 0;
					mibentry.ConIPInstNum = 1 + findMaxIPConInstNum(MEDIA_INDEX(mibentry.ifIndex), mibentry.ConDevInstNum );
				}
	DEBUGPRINT;
			}
			//fprintf( stderr, "<%s:%d>NewInstNum=>ConDev:%u, PPPCon:%u, IPCon:%u\n", __FILE__, __LINE__, mibentry.ConDevInstNum, mibentry.ConPPPInstNum, mibentry.ConIPInstNum );

			mibentry.autoDisTime = Entry.autoDisTime;
			mibentry.warnDisDelay = Entry.warnDisDelay;
			//strcpy( entry.pppServiceName, Entry.pppServiceName );
			strcpy( mibentry.WanName, Entry.WanName );
/*
#ifdef _PRMT_X_CT_COM_PPPOEv2_
			entry.PPPoEProxyEnable = Entry.PPPoEProxyEnable;
			entry.PPPoEProxyMaxUser = Entry.PPPoEProxyMaxUser;
#endif //_PRMT_X_CT_COM_PPPOEv2_
*///set by web
#ifdef _PRMT_X_CT_COM_WANEXT_
//			mibentry.ServiceList = Entry.ServiceList;
#endif //_PRMT_X_CT_COM_WANEXT_
#ifdef _PRMT_X_CT_COM_DHCP_
			memcpy(mibentry.dhcpv6_opt16_enable, Entry.dhcpv6_opt16_enable, sizeof(Entry.dhcpv6_opt16_enable));
			memcpy(mibentry.dhcpv6_opt16_type, Entry.dhcpv6_opt16_type, sizeof(Entry.dhcpv6_opt16_type));
			memcpy(mibentry.dhcpv6_opt16_value_mode, Entry.dhcpv6_opt16_value_mode, sizeof(Entry.dhcpv6_opt16_value_mode));
			memcpy(mibentry.dhcpv6_opt16_value, Entry.dhcpv6_opt16_value, 4 * 80);
			memcpy(mibentry.dhcpv6_opt17_enable, Entry.dhcpv6_opt17_enable, sizeof(Entry.dhcpv6_opt17_enable));
			memcpy(mibentry.dhcpv6_opt17_type, Entry.dhcpv6_opt17_type, sizeof(Entry.dhcpv6_opt17_type));
			memcpy(mibentry.dhcpv6_opt17_sub_code, Entry.dhcpv6_opt17_sub_code, sizeof(Entry.dhcpv6_opt17_sub_code));
			memcpy(mibentry.dhcpv6_opt17_sub_data, Entry.dhcpv6_opt17_sub_data, 4 * 36);
			memcpy(mibentry.dhcpv6_opt17_value, Entry.dhcpv6_opt17_value, 4 * 36);

			memcpy(mibentry.dhcp_opt60_enable, Entry.dhcp_opt60_enable, sizeof(Entry.dhcp_opt60_enable));
			memcpy(mibentry.dhcp_opt60_type, Entry.dhcp_opt60_type, sizeof(Entry.dhcp_opt60_type));
			memcpy(mibentry.dhcp_opt60_value_mode, Entry.dhcp_opt60_value_mode, sizeof(Entry.dhcp_opt60_value_mode));
			memcpy(mibentry.dhcp_opt60_value, Entry.dhcp_opt60_value, 4 * 80);
			memcpy(mibentry.dhcp_opt125_enable, Entry.dhcp_opt125_enable, sizeof(Entry.dhcp_opt125_enable));
			memcpy(mibentry.dhcp_opt125_type, Entry.dhcp_opt125_type, sizeof(Entry.dhcp_opt125_type));
			memcpy(mibentry.dhcp_opt125_sub_code, Entry.dhcp_opt125_sub_code, sizeof(Entry.dhcp_opt125_sub_code));
			memcpy(mibentry.dhcp_opt125_sub_data, Entry.dhcp_opt125_sub_data, 4 * 36);
			memcpy(mibentry.dhcp_opt125_value, Entry.dhcp_opt125_value, 4 * 36);
#endif
#endif //_CWMP_MIB_

#ifdef CONFIG_RTK_RG_INIT
			mibentry.rg_wan_idx = Entry.rg_wan_idx;
#endif
#ifdef CONFIG_MCAST_VLAN
			mibentry.mVid = Entry.mVid;
			//AUG_PRT("%s-%d mVlan=%d %d\n",__func__,__LINE__,mibentry.mVid, Entry.mVid);
#endif

/*star:20080718 START add for set acname by net_adsl_links_acname.asp*/
			if(acflag==0)
/*star:20080718 END*/
				strcpy(mibentry.pppACName, Entry.pppACName);

			// find the ifIndex
			if (mibentry.cmode != Entry.cmode)
			{
				if (!(mibentry.cmode == CHANNEL_MODE_PPPOE && cnt>0)){	// Jenny, entries except multisession PPPoE
					mibentry.ifIndex = if_find_index(mibentry.cmode, ifMap);
					// Mason Yu. ITMS4
					#ifdef CONFIG_PTMWAN
					if(ptmset)
						mibentry.ifIndex = TO_IFINDEX(MEDIA_PTM, PPP_INDEX(mibentry.ifIndex), PTM_INDEX(mibentry.ifIndex));
					else
					#endif
						mibentry.ifIndex = TO_IFINDEX(MEDIA_ETH, PPP_INDEX(mibentry.ifIndex), ETH_INDEX(mibentry.ifIndex));
				}
				if (mibentry.ifIndex == NA_VC) {
					strcpy(tmpBuf, strMaxVc);
					goto setErr_nochange;
				}
				else if (mibentry.ifIndex == NA_PPP) {
					strcpy(tmpBuf, strMaxNumPPPoE);
					goto setErr_nochange;
				}
	DEBUGPRINT;
				// mode changed, restore to default
				if (mibentry.cmode == CHANNEL_MODE_PPPOE) {
					if(mibentry.mtu < 1492) //set by web
						mibentry.mtu = 1492;
/*
#ifdef CONFIG_USER_PPPOE_PROXY
					entry.PPPoEProxyMaxUser=4;
#endif
					entry.pppAuth = 0;
*///set by web
				}
//				else {
/*
#ifdef CONFIG_USER_PPPOE_PROXY
					entry.PPPoEProxyMaxUser=0;
#endif
*/
	//				entry.dgw = 1;
//					mibentry.mtu = 1500;
//				}
	DEBUGPRINT;
	//			entry.dgw = 1;
/*
#ifdef CONFIG_EXT_SWITCH
				// VLAN
				entry.vlan = 0;	// disable
				entry.vid = 0; // VLAN tag
				entry.vprio = 0; // priority bits (0 ~ 7)
				entry.vpass = 0; // no pass-through
#endif
*///set by web
			}

DEBUGPRINT;
			if( mibentry.ifIndex!=Entry.ifIndex ||
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
				mibentry.enableIpQos != Entry.enableIpQos ||
#endif
				mibentry.cmode != Entry.cmode)
				resolveServiceDependency(selected);

			if( mibentry.ifIndex!=Entry.ifIndex)
			{
				// todo: check e8b
				#if 0
		#ifdef CONFIG_USER_DDNS
				delDDNSinter(Entry.ifIndex);
		#endif
				delBrgMacFilterRule(Entry.ifIndex);
				delIpRouteTbl(Entry.ifIndex);
				#endif
			}
			// todo: check e8b
			#if 0
			if (mibentry.cmode != CHANNEL_MODE_BRIDGE)
			{
				delBrgMacFilterRule(mibentry.ifIndex);
			}
			#endif
DEBUGPRINT;
			mibentry.vlan = entry.vlan;
			mibentry.vid = entry.vid;
			mibentry.vprio = entry.vprio;
// todo: check e8b
#ifdef NEW_PORTMAPPING
			//mibentry record current web record;
			//Entry record old record;
			AUG_PRT("^O^ %s:%d. The mibentry.itfgroup is 0x%x, The Entry's itfgroup is 0x%x, the apptype is %d\n", __FILE__,
				__LINE__, mibentry.itfGroup, Entry.itfGroup, Entry.applicationtype);
			//do not need more check work!
			check_itfGroup(&mibentry, &Entry);
#endif

/*star:20090302 START wen INTERNET pvc start, igmp proxy open auto*/
			mibentry.enableIGMP=0;
			if(mibentry.cmode!=CHANNEL_MODE_BRIDGE)
				if (mibentry.applicationtype&X_CT_SRV_INTERNET)
					mibentry.enableIGMP=1;
/*star:20090302 END*/


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
					if(pvcifIdx==dnsPvcEntry.ifIndex)//I get it
					{
						if(mibentry.cmode==CHANNEL_MODE_BRIDGE)
							pvcifIdx = DUMMY_IFINDEX;
						else
							pvcifIdx=mibentry.ifIndex;
						mib_set(MIB_DNS_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif

#ifdef CONFIG_IPV6
#ifdef DNSV6_BIND_PVC_SUPPORT
			MIB_CE_ATM_VC_T dnsv6PvcEntry;
			if(mib_chain_get(MIB_ATM_VC_TBL,selected,&dnsv6PvcEntry)&&(dnsv6PvcEntry.cmode!=CHANNEL_MODE_BRIDGE))
			{
				int tempi=0;
				unsigned int pvcifIdx=0;
				for(tempi=0;tempi<3;tempi++)
				{
					mib_get(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					if(pvcifIdx==dnsv6PvcEntry.ifIndex)//I get it
					{
						if(mibentry.cmode==CHANNEL_MODE_BRIDGE)
							pvcifIdx = DUMMY_IFINDEX;
						else
							pvcifIdx=mibentry.ifIndex;
						mib_set(MIB_DNSV6_BIND_PVC1+tempi,(void*)&pvcifIdx);
					}
				}
			}
#endif
#endif
#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
			{
			char macaddr[MAC_ADDR_LEN];
			/* Magician: Auto generate MAC address for every WAN interface. */
			mib_get(MIB_ELAN_MAC_ADDR, (void *)macaddr);
			macaddr[MAC_ADDR_LEN-1] += WAN_HW_ETHER_START_BASE + ETH_INDEX(mibentry.ifIndex);
			memcpy(mibentry.MacAddr, macaddr, MAC_ADDR_LEN);
			/* End Majgician */
			}
#endif
	        //jim garbage action...
			//memcpy(&Entry, &entry, sizeof(entry));
			// log message
			// Mason Yu

#ifdef CONFIG_IPV6
			setup_delegated_default_wanconn(&mibentry);
#endif

			// Mason Yu. ITMS4
			deleteConnection(CONFIGONE, &Entry);		// Modify
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&mibentry, selected);
			restartWAN(CONFIGONE, &mibentry);			// Modify
	// todo: check e8b
	#if 0
#ifndef QOS_SETUP_IMQ
			//ql_xu: add IP QoS queue
			if (mibentry.enableIpQos)
				addIpQosQueue(mibentry.ifIndex);
#endif
	#endif
//			restart_dnsrelay(); //star
			DEBUGPRINT;

			goto setOk_filter;
		}
	}
	else {lineno = __LINE__; goto check_err;}

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	strcpy(tmpBuf, "参数错误");
	goto setErr_nochange;
	return;

setOk_filter:

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	// Mason Yu. ITMS4
	//do_wan_restart();
/*star:20081205 START when change wan by web, tr069 associated notify entry should be updated*/
	// todo: check e8b
	//writeWanChangeFile();
/*star:20081205 END*/
#ifdef VOIP_SUPPORT
	web_restart_solar();
#endif

	DEBUGPRINT;//ql_xu
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
 	return;

setErr_restart:
	// Mason Yu. ITMS4
	//do_wan_restart();
setErr_nochange:
	ERR_MSG(tmpBuf);
	//startWan(BOOT_LAST);
}

#endif

/*****************************
** wan if list
*/
int listWanif(int eid, request * wp, int argc, char ** argv)
{
#define _LK_FLAG_RT		0x01
#define _LK_FLAG_BR		0x02
	struct atmvc_entryx	entry;
	char* type = NULL;
	int cnt = 1;
	int index = 0;
	int flag = 0;
	int lineno = __LINE__;
	unsigned char	ipAddr[16];		//IP地址
	unsigned char	remoteIpAddr[16];	//缺省网关
	unsigned char	netMask[16];	//子网掩码

	MIB_CE_ATM_VC_T mibentry;
	int mibtotal,i;

	char wanname[MAX_WAN_NAME_LEN];

	_TRACE_CALL;

	if (boaArgs(argc, argv, "%s", &type) < 1) {
		flag = _LK_FLAG_RT | _LK_FLAG_BR;
	}
	else if (strcmp(type, "rt") == 0) {
		flag = _LK_FLAG_RT;
	}
	else if (strcmp(type, "br") == 0) {
		flag = _LK_FLAG_BR;
	}
	else if (strcmp(type, "all") == 0) {
		flag = _LK_FLAG_BR;
	}
	else return -1;

	/************Place your code here, do what you want to do! ************/
	/************Place your code here, do what you want to do! ************/

	memset(&entry,0,sizeof(entry));

	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < mibtotal; i ++) {
		if (mib_chain_get(MIB_ATM_VC_TBL,i,&mibentry)!=1)
			continue;
		/************Place your code here, do what you want to do! ************/
		/************Place your code here, do what you want to do! ************/
		getWanName(&mibentry, wanname);
		int tmp = mib2web(&mibentry, &entry);
		if (tmp ==-1)
			continue;

		if (entry.cmode == 0 && !(flag & _LK_FLAG_BR))
			continue;
		else if (entry.cmode != 0 && !(flag & _LK_FLAG_RT))
			continue;

		_TRACE_POINT;
		boaWrite(wp, "push(new it_nr(\"%s\"" _PTI _PTI _PTI _PTI _PTI _PTI \
			_PTI _PTI _PTI _PTI  _PTI \
			_PTS _PTI _PTS _PTS\
			_PTI _PTI _PTS _PTS _PTS _PTI _PTI _PTI _PTI _PTI _PTI _PTI _PTI _PTI "));\n",
			wanname, _PME(vpi), _PME(qos), _PME(vci), _PME(pcr), _PME(scr), _PME(mbs),
			_PME(encap), _PME(napt), _PME(cmode), _PME(brmode), _PME(AddrMode),
			_PME(pppUsername)/*, _PME(pppPassword)*/, _PME(pppAuth), _PME(pppServiceName), _PME(pppACName),
			_PME(pppCtype), _PME(ipDhcp),
			_PMEIP(ipAddr), _PMEIP(remoteIpAddr), _PMEIP(netMask),
			_PME(vlan), _PME(vid), _PME(vprio), _PME(vpass), _PME(itfGroup),
			_PME(PPPoEProxyEnable), _PME(PPPoEProxyMaxUser), _PME(applicationtype), _PME(ifIndex)
			);
	}

check_err:
	_TRACE_LEAVEL;
	return 0;
}

#ifdef _PRMT_X_CT_COM_USERINFO_
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#define USERINFO_LINE				"光纤"
#define USERINFO_LINE_LED			"光信号灯已处于熄灭"
#elif defined(CONFIG_DEV_xDSL)
#define USERINFO_LINE				"网络"
#ifdef CONFIG_VDSL
#define USERINFO_LINE_PORT			"V"
#else
#define USERINFO_LINE_PORT			"A"
#endif
#define USERINFO_LINE_LED			"“"USERINFO_LINE USERINFO_LINE_PORT"”灯处于常亮"
#else
#define USERINFO_LINE				"网络"
#define USERINFO_LINE_PORT			"L"
#define USERINFO_LINE_LED			"“"USERINFO_LINE USERINFO_LINE_PORT"”灯处于常亮或闪烁"
#endif

#define OLT_ACCOUNT_REG_FAIL			"在OLT上注册失败，请检查光信号灯是否处于熄灭状态、逻辑ID和密码是否正确或联系客户经理或拨打10000"
#define OLT_ACCOUNT_REG_ING			"正在注册OLT"
#define OLT_ACCOUNT_REG_SUCC			"注册OLT成功，正在获取管理IP"
#define E8CLIENT_ACCOUNT_REG			"正在注册，请稍候……"
#define E8CLIENT_TR069_READY			"已获得管理IP，正在连接ITMS"
#define E8CLIENT_ITMS_NOT_REACHABLE		"到ITMS的通道不通，请联系客户经理或拨打10000"
#define E8CLIENT_ACCOUNT_REG_FAIL		"注册失败！请重试"

#define E8CLIENT_ACCOUNT_REG_SUCC		"注册ITMS成功，等待ITMS平台下发业务数据"
#define E8CLIENT_ACCOUNT_REMOTE_SETTING0	"注册ITMS成功，正在下发业务，请勿断电或拨"USERINFO_LINE
#define E8CLIENT_ACCOUNT_REMOTE_SETTING1	"ITMS平台正在下发 %s 业务数据，请勿断电或拨"USERINFO_LINE
#ifdef CONFIG_YUEME
#define E8CLIENT_ACCOUNT_REMOTE_SETTING_SUCCESS	"ITMS平台业务数据下发成功，共下发了 %s%d 个业务，欢迎使用悅me"
#define E8CLIENT_ACCOUNT_REMOTE_SETTING_REBOOT	"ITMS平台业务数据下发成功，共下发了 %s%d 个业务，悅me网关需要重启，请等待…"
#else
#define E8CLIENT_ACCOUNT_REMOTE_SETTING_SUCCESS	"ITMS平台业务数据下发成功，共下发了 %s%d 个业务，欢迎使用业务"
#define E8CLIENT_ACCOUNT_REMOTE_SETTING_REBOOT	"ITMS平台业务数据下发成功，共下发了 %s%d 个业务，家庭网关需要重启，请等待…"
#endif
#define E8CLIENT_ACCOUNT_REMOTE_SETTING_FAIL	"ITMS下发业务异常！请联系客户经理或拨打10000"
#define E8CLIENT_ACCOUNT_REG_FAIL1_2_3		"在ITMS上注册失败！请检查逻辑ID和密码是否正确或联系客户经理或拨打10000"
#define E8CLIENT_ACCOUNT_REG_FAIL1_2_3_OVER	"在ITMS上注册失败！请3分钟后重试或联系客户经理或拨打10000"
#define E8CLIENT_ACCOUNT_REG_FAIL4		"注册超时！请检查线路后重试或联系客户经理或拨打10000"
#define E8CLIENT_ACCOUNT_REG_FAIL5		"已经在ITMS注册成功，无需再注册"


/*star:20090302 START when "正在注册中...", there is no OK button*/
#define E8CLIENT_REG_HAVE_OK(s) {\
	boaWrite(wp, "<form style=\"border:0; padding:0; \">\n");\
	boaWrite(wp, "<table cellspacing=\"0\" cellpadding=\"0\" width=\"300\" align=\"center\" border=\"0\">\n");\
	boaWrite(wp, "<tr><td><font size=4>%s</td></tr></table>\n", s);\
	boaWrite(wp, "</form>\n");\
	boaWrite(wp, "<br><input type=\"button\" value=\"确定\" onClick=\"window.location.href='/ehomeclient/e8clientusereg.asp';\" style=\"width:80px; border-style:groove; font-weight:bold \">");\
}

#define E8CLIENT_REG_NO_OK(s) {\
	boaWrite(wp, "<form style=\"border:0; padding:0; \">\n");\
	boaWrite(wp, "<table cellspacing=\"0\" cellpadding=\"0\" width=\"300\" align=\"center\" border=\"0\">\n");\
	boaWrite(wp, "<tr><td><font size=4>%s</td></tr></table>\n", s);\
	boaWrite(wp, "</form>\n");\
}

#define E8CLIENT_REG_HAVE_OK_AUTORUN(s) {\
	boaWrite(wp, "<form style=\"border:0; padding:0; \">\n");\
	boaWrite(wp, "<table cellspacing=\"0\" cellpadding=\"0\" width=\"300\" align=\"center\" border=\"0\">\n");\
	boaWrite(wp, "<tr><td><font size=4>%s</td></tr></table>\n", s);\
	boaWrite(wp, "</form>\n");\
	boaWrite(wp, "<br><input type=\"button\" value=\"确定\" onClick=\"window.location.href='/autorun/accreg.asp';\" style=\"width:80px; border-style:groove; font-weight:bold \">");\
}
/*star:20090302 END*/

/*star:20080827 START add for reg timeout*/
static int ctregcount = 0;
/*star:20080827 END*/

int initE8clientUserRegPage(int eid, request * wp, int argc, char ** argv)
{
	MIB_CE_ATM_VC_T entry;
	unsigned int i, num;
	unsigned char over = 0;
	unsigned char loid[MAX_NAME_LEN];
	unsigned char password[MAX_NAME_LEN];
	unsigned int regLimit;
	unsigned int regTimes;

	mib_get(CWMP_USERINFO_LIMIT, &regLimit);
	mib_get(CWMP_USERINFO_TIMES, &regTimes);
	over = regTimes >= regLimit;

	mib_get(MIB_LOID, loid);
	mib_get(MIB_LOID_PASSWD, password);

	boaWrite(wp, "over = %hhu;\n", over);
	boaWrite(wp, "loid = \"%s\";\n", loid);
	boaWrite(wp, "password = \"%s\";\n", password);

	return 0;
}

int e8clientAccountRegResult(int eid, request * wp, int argc, char **argv)
{
	unsigned int regStatus;
	unsigned int regLimit;
	unsigned int regTimes;
	unsigned char regInformStatus;

	mib_get(CWMP_USERINFO_STATUS, &regStatus);
	mib_get(CWMP_USERINFO_LIMIT, &regLimit);
	mib_get(CWMP_USERINFO_TIMES, &regTimes);
	mib_get(CWMP_REG_INFORM_STATUS, &regInformStatus);

	if (regTimes >= regLimit) {
		E8CLIENT_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL1_2_3_OVER);
		return 0;
	}

	if (regInformStatus != CWMP_REG_RESPONSED) {	//ACS not returned result
/*star:20080827 START add for reg timeout*/
		if (ctregcount >= 4) {
			E8CLIENT_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL4);
		} else {
			ctregcount++;
			E8CLIENT_REG_NO_OK(E8CLIENT_ACCOUNT_REG);
		}
/*star:20080827 END*/
	} else {
		if (regStatus == 0) {
			E8CLIENT_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_SUCC);
		} else {
			E8CLIENT_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL);
		}
	}

	return 0;
}

/*star: 20090302 START add for autorun*/
int e8clientAutorunAccountRegResult(int eid, request * wp, int argc, char **argv)
{
	unsigned int regStatus;
	unsigned int regLimit;
	unsigned int regTimes;
	unsigned char regInformStatus;

	mib_get(CWMP_USERINFO_STATUS, &regStatus);
	mib_get(CWMP_USERINFO_LIMIT, &regLimit);
	mib_get(CWMP_USERINFO_TIMES, &regTimes);
	mib_get(CWMP_REG_INFORM_STATUS, &regInformStatus);

	if (regTimes >= regLimit) {
		E8CLIENT_REG_HAVE_OK_AUTORUN(E8CLIENT_ACCOUNT_REG_FAIL1_2_3_OVER);
		return 0;
	}

	if (regInformStatus != CWMP_REG_RESPONSED) {	//ACS not returned result
/*star:20080827 START add for reg timeout*/
		if (ctregcount >= 4) {
			E8CLIENT_REG_HAVE_OK_AUTORUN(E8CLIENT_ACCOUNT_REG_FAIL4);
		} else {
			ctregcount++;
			E8CLIENT_REG_NO_OK(E8CLIENT_ACCOUNT_REG);
		}
/*star:20080827 END*/
	} else {
		if (regStatus == 0) {
			E8CLIENT_REG_HAVE_OK_AUTORUN(E8CLIENT_ACCOUNT_REG_SUCC);
		} else {
			E8CLIENT_REG_HAVE_OK_AUTORUN(E8CLIENT_ACCOUNT_REG_FAIL);
		}
	}

	return 0;
}
/*star: 20090302 END*/

void formAccountReg(request * wp, char *path, char *query)
{
	char *stemp;
	unsigned char vChar;
	unsigned int regLimit;
	unsigned int regTimes;
	unsigned lineno;

	_TRACE_CALL;

	mib_get(CWMP_USERINFO_LIMIT, &regLimit);
	mib_get(CWMP_USERINFO_TIMES, &regTimes);
	if (regTimes >= regLimit) {
		vChar = CWMP_REG_IDLE;
		mib_set(CWMP_REG_INFORM_STATUS, &vChar);
		goto FINISH;
	}

	//_ENTRY_STR(auth, _NEED);
	//_ENTRY_STR(user, _NEED);
	stemp = boaGetVar(wp, "broadbandusername", "");
	if (stemp[0])
		mib_set(MIB_LOID, stemp);
	else {
		fprintf(stderr, "get broadband username error!\n");
		goto check_err;
	}

	stemp = boaGetVar(wp, "customer+ID", "");
	if (stemp[0])
		mib_set(MIB_LOID_PASSWD, stemp);
	else {
		fprintf(stderr, "get customer ID error!\n");
		goto check_err;
	}

/*xl_yue:20081225 record the inform status to avoid acs responses twice for only once informing*/
	vChar = CWMP_REG_REQUESTED;
	mib_set(CWMP_REG_INFORM_STATUS, &vChar);
/*xl_yue:20081225 END*/

#ifdef CONFIG_MIDDLEWARE
	mib_get(CWMP_TR069_ENABLE, &vChar);
	if (!vChar) {
		//Martin_ZHU: send CTEVENT_BIND to MidProcess
		vChar = CTEVENT_BIND;
		sendInformEventMsg2MidProcess( vChar );
	} else
#endif
	{
		pid_t cwmp_pid;

		// send signal to tr069
		cwmp_pid = read_pid("/var/run/cwmp.pid");
		if (cwmp_pid > 0) {
#ifdef CONFIG_MIDDLEWARE
			vChar = CTEVENT_BIND;
			mib_set(MIB_MIDWARE_INFORM_EVENT, &vChar);
			kill(cwmp_pid, SIGUSR1);	//SIGUSR2 is used by midware
#else
			kill(cwmp_pid, SIGUSR2);
#endif
		}
	}

/*star:20080827 START add for reg timeout*/
	ctregcount = 0;
/*star:20080827 END*/

FINISH:
	//web redirect
	_COND_REDIRECT;

check_err:
	_TRACE_LEAVEL;
	return;
}

#define USER_REG_HAVE_OK(s) {\
	boaWrite(wp, "%s\n", s);\
	boaWrite(wp, "<div id=\"progress-boader\"></div>");\
	boaWrite(wp, "<input id=\"ok\" type=\"button\" value=\"确定\"  onclick=\"location.href='/usereg.asp';\" >");\
}

#define USER_REG_NO_OK(s) {\
	boaWrite(wp, "%s\n", s);\
	boaWrite(wp, "<div id=\"progress-boader\"></div>");\
}

#define USER_REG_REBOOT() {\
	boaWrite(wp, "<script>\n");\
	boaWrite(wp, "window.opener=null \n");\
	boaWrite(wp, "window.close();\n");\
	boaWrite(wp, "</script> \n");\
}

#define NPROGRESS_SET(N) {\
	boaWrite(wp, "<script language=\"javascript\">NProgress.set(%f);</script>", N);\
}

#define NPROGRESS_DONE() {\
	boaWrite(wp, "<script language=\"javascript\">NProgress.done();</script>");\
}

void SaveLOIDReg()
{
	unsigned char loid[MAX_NAME_LEN];
	unsigned char password[MAX_NAME_LEN];
	unsigned char old_loid[MAX_NAME_LEN];
	unsigned char old_password[MAX_NAME_LEN];
	int changed = 0;

	mib_get(MIB_LOID, loid);
	mib_get(MIB_LOID_OLD, old_loid);
	if(strcmp(loid, old_loid) != 0)
	{
		mib_set(MIB_LOID_OLD, loid);
		changed = 1;
	}
	mib_get(MIB_LOID_PASSWD, password);
	mib_get(MIB_LOID_PASSWD_OLD, old_password);
	if(strcmp(password, old_password) != 0)
	{
		mib_set(MIB_LOID_PASSWD_OLD, password);
		changed = 1;
	}

#ifdef COMMIT_IMMEDIATELY
	if(changed)
		Commit();
#endif
}


int UserAccountRegResult(int eid, request * wp, int argc, char **argv)
{
	static int rebootTime = 0;
	int i, total, ret;
	MIB_CE_ATM_VC_T entry;
	struct in_addr inAddr;
	FILE *fp;
	char buf[256], serviceName[32];
	unsigned int regStatus;
	unsigned int regLimit;
	unsigned int regTimes;
	unsigned char regInformStatus;
	unsigned int regResult;
	unsigned char needReboot;
	int serviceNum;
#ifdef CONFIG_RTK_OMCI_V1
	PON_OMCI_CMD_T msg;
#endif
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	unsigned int pon_mode, pon_state;
	mib_get(MIB_PON_MODE, &pon_mode);
#endif

	mib_get(CWMP_USERINFO_STATUS, &regStatus);
	mib_get(CWMP_USERINFO_LIMIT, &regLimit);
	mib_get(CWMP_USERINFO_TIMES, &regTimes);
	mib_get(CWMP_REG_INFORM_STATUS, &regInformStatus);
	mib_get(CWMP_USERINFO_RESULT, &regResult);
	mib_get(CWMP_USERINFO_NEED_REBOOT, &needReboot);
	mib_get(CWMP_USERINFO_SERV_NUM, &serviceNum);

	if (regInformStatus != CWMP_REG_RESPONSED) {	//ACS not returned result

		total = mib_chain_total(MIB_ATM_VC_TBL);

		for (i = 0; i < total; i++) {
			if (mib_chain_get(MIB_ATM_VC_TBL, i, &entry) == 0)
				continue;

			if ((entry.applicationtype & X_CT_SRV_TR069) &&
					ifGetName(entry.ifIndex, buf, sizeof(buf)) &&
					getInFlags(buf, &ret) &&
					(ret & IFF_UP) &&
					getInAddr(buf, IP_ADDR, &inAddr))
				break;
		}


		if(pon_mode == 1)
		{
#ifdef CONFIG_RTK_OMCI_V1		
			/* During deactivate, the IP may not be cleared in a small period of time.*/
			/* So check gpon state first. */
			memset(&msg, 0, sizeof(msg));
			msg.cmd = PON_OMCI_CMD_LOIDAUTH_GET_RSP;
			ret = omci_SendCmdAndGet(&msg);

			if (ret != GOS_OK || (msg.state != 0 && msg.state != 1)) {
				USER_REG_HAVE_OK(OLT_ACCOUNT_REG_FAIL);
				NPROGRESS_DONE();
				return 0;
			}
			pon_state = msg.state;
#endif			
		}
		else if(pon_mode == 2)
		{
			rtk_epon_llid_entry_t  llidEntry;
			
			memset(&llidEntry, 0, sizeof(rtk_epon_llid_entry_t));
			llidEntry.llidIdx = 0;
#ifdef CONFIG_RTK_L34_ENABLE	
			rtk_rg_epon_llid_entry_get(&llidEntry);
#else
			rtk_epon_llid_entry_get(&llidEntry);
#endif
			pon_state = llidEntry.valid;//OLT Register successful
		}

		if (ctregcount >= 24)
		{
			/* 120 seconds, timeout */
			if (i == total) {
				/* The interface for TR069 is not ready */
				USER_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL4);
			} else {
				USER_REG_HAVE_OK(E8CLIENT_ITMS_NOT_REACHABLE);
			}
			NPROGRESS_DONE();
		} else {
			ctregcount++;

			if (pon_state == 0) {
				USER_REG_NO_OK(OLT_ACCOUNT_REG_ING);
				NPROGRESS_SET(0.2);
				return 0;
			}
			if (i == total) {
				/* The interface for TR069 is not ready */
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)				
				if(pon_state == 1)
				{
					USER_REG_NO_OK(OLT_ACCOUNT_REG_SUCC);
					SaveLOIDReg();
					NPROGRESS_SET(0.3);
				}
#else
				USER_REG_NO_OK(E8CLIENT_ACCOUNT_REG);
				NPROGRESS_SET(0.3);
#endif
			} else {
				USER_REG_NO_OK(E8CLIENT_TR069_READY);
				SaveLOIDReg();
				NPROGRESS_SET(0.4);
			}
		}
/*star:20080827 END*/
	} else {
		if (regStatus == 0) {
			switch (regResult) {
			case NO_SET:
				USER_REG_NO_OK(E8CLIENT_ACCOUNT_REG_SUCC);
				NPROGRESS_SET(0.5);
				unlink(REBOOT_DELAY_FILE);
				break;
			case NOW_SETTING:
				mib_get(CWMP_USERINFO_SERV_NAME, serviceName);
				if (!strstr(serviceName, "IPTV") &&
					!strstr(serviceName, "INTERNET") &&
					!strstr(serviceName, "VOIP")
				   ) {
					USER_REG_NO_OK(E8CLIENT_ACCOUNT_REMOTE_SETTING0);
				} else {
					sprintf(buf, E8CLIENT_ACCOUNT_REMOTE_SETTING1, serviceName);
					USER_REG_NO_OK(buf);
				}
				mib_get(CWMP_USERINFO_SERV_NUM_DONE, &i);

				if(serviceNum > 0)
				{
					NPROGRESS_SET(0.6 + 0.4 * i / serviceNum);
				}
				else
				{
					NPROGRESS_SET(0.6);
				}
				break;
			case SET_SUCCESS:
				if (needReboot) {
					sprintf(buf, E8CLIENT_ACCOUNT_REMOTE_SETTING_REBOOT, serviceName, serviceNum);
					USER_REG_HAVE_OK(buf);
					unlink(REBOOT_DELAY_FILE);
				} else {
					sprintf(buf, E8CLIENT_ACCOUNT_REMOTE_SETTING_SUCCESS, serviceName, serviceNum);
					USER_REG_HAVE_OK(buf);
					unlink(REBOOT_DELAY_FILE);
				}
				NPROGRESS_SET(1.0);
				break;
			case SET_FAULT:
				USER_REG_HAVE_OK(E8CLIENT_ACCOUNT_REMOTE_SETTING_FAIL);
				NPROGRESS_DONE();
				break;
			}
		} else if (regStatus >= 1 && regStatus <= 3) {
			if (regTimes >= regLimit) {
				USER_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL1_2_3);
				NPROGRESS_DONE();
			} else {
				USER_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL1_2_3_OVER);
				NPROGRESS_DONE();
			}
		} else if (regStatus == 4) {
			USER_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL4);
			NPROGRESS_DONE();
		} else if (regStatus == 5) {
			USER_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL5);
			NPROGRESS_DONE();
		} else {
			USER_REG_HAVE_OK(E8CLIENT_ACCOUNT_REG_FAIL1_2_3_OVER);
			NPROGRESS_DONE();
		}
	}

	return 0;
}

void formUserReg(request * wp, char *path, char *query)
{
	char *loid, *password;
	unsigned char vChar;
	unsigned int regLimit;
	unsigned int regTimes;
	unsigned int lineno;
	pid_t cwmp_pid;
	int num_done;
#if defined(CONFIG_GPON_FEATURE)
	int i=0;
#endif
#if defined(CONFIG_EPON_FEATURE)
	int index, entryNum;
	char cmdBuf[64] = {0};
#endif

	int sleep_time = 3;
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	unsigned int pon_mode;
	mib_get(MIB_PON_MODE, &pon_mode);
#endif

	_TRACE_CALL;

	mib_get(CWMP_USERINFO_LIMIT, &regLimit);
	mib_get(CWMP_USERINFO_TIMES, &regTimes);
	if (regTimes >= regLimit) {
		vChar = CWMP_REG_IDLE;
		mib_set(CWMP_REG_INFORM_STATUS, &vChar);
		goto FINISH;
	}

	loid = boaGetVar(wp, "loid", "");
	if (loid[0]) {
		mib_set(MIB_LOID, loid);
	} else {
		fprintf(stderr, "get LOID error!\n");
		goto check_err;
	}

	password = boaGetVar(wp, "password", "");
	if (password[0]) {
		mib_set(MIB_LOID_PASSWD, password);
	} else {
		fprintf(stderr, "No LOID Password exist!\n");
		//goto check_err;
	}

/*xl_yue:20081225 record the inform status to avoid acs responses twice for only once informing*/
	vChar = CWMP_REG_REQUESTED;
	mib_set(CWMP_REG_INFORM_STATUS, &vChar);
	/* reset to zero */
	num_done = 0;
	mib_set(CWMP_USERINFO_SERV_NUM_DONE, &num_done);
	mib_set(CWMP_USERINFO_SERV_NAME_DONE, "");
/*xl_yue:20081225 END*/

#if defined(CONFIG_GPON_FEATURE)
	// Deactive GPON
	// do not use rtk_rg_gpon_deActivate() becuase it does not send link down event.
	if(pon_mode == 1)
	{
		system("diag gpon reg-set page 1 offset 0x10 value 0x1");

		system("omcicli mib reset");

		if (password[0]) {
			va_cmd("/bin/omcicli", 4, 1, "set", "loid", loid, password);
		} else {
			va_cmd("/bin/omcicli", 3, 1, "set", "loid", loid);
		}

		while(i++ < 10)
			system("diag gpon reg-set page 1 offset 0x10 value 0x3");
	}
#endif

#if defined(CONFIG_EPON_FEATURE)
	if(pon_mode == 2)
	{
#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_epon_llidEntryNum_get(&entryNum);
#else
		rtk_epon_llidEntryNum_get(&entryNum);
#endif
		for (index = 0; index < entryNum; index++) {

			memset(cmdBuf, 0, sizeof(cmdBuf));

			if (password[0]) {
				sprintf(cmdBuf, "/bin/oamcli set ctc loid %d %s %s\n",index, loid, password);
			} else {
				sprintf(cmdBuf, "/bin/oamcli set ctc loid %d %s %s\n",index, loid);
			}
			system(cmdBuf);
		}
	}//if(pon_mode == 2)
#endif

	// Purposes:
	// 1. Wait for PON driver ready.
	// 2. Wait for old IP release.
	while(sleep_time)
		sleep_time = sleep(sleep_time);

#ifdef CONFIG_MIDDLEWARE
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(!vChar)
		{// Martin_ZHU:send CTEVENT_BIND to MidProcess
			vChar = CTEVENT_BIND;
			sendInformEventMsg2MidProcess( vChar );
		}else
#endif
		{
			{
				pid_t tr069_pid;

				// send signal to tr069
				tr069_pid = read_pid("/var/run/cwmp.pid");
				if ( tr069_pid > 0){
#ifdef CONFIG_MIDDLEWARE
					vChar = CTEVENT_BIND;
					mib_set(MIB_MIDWARE_INFORM_EVENT,(void*)&vChar);
					kill(tr069_pid, SIGUSR1);	//SIGUSR2 is used by midware
#else
					kill(tr069_pid, SIGUSR2);
#endif
				}
			}
		}

/*star:20080827 START add for reg timeout*/
	ctregcount = 0;
/*star:20080827 END*/

FINISH:
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	//web redirect
	_COND_REDIRECT;

check_err:
	_TRACE_LEAVEL;
}

int UserRegMsg(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;
	char line[128];
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	unsigned int pon_mode;
#endif

	strcpy(line, USERINFO_LINE);

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	mib_get(MIB_PON_MODE, &pon_mode);
	if (pon_mode == GPON_MODE) {
		strcat(line, "G");
	} else if (pon_mode == EPON_MODE) {
		strcat(line, "E");
	}
#else
	strcat(line, USERINFO_LINE_PORT);
#endif

	nBytesSent += boaWrite(wp, "%s%s%s", "请插紧“", line,
					"”接口的" USERINFO_LINE "，检查并确认"
					USERINFO_LINE_LED "状态<br>"
					"准确输入“逻辑ID”和“密码”，点击“确定”进行注册<br>"
					"在注册及业务下发过程中（10分钟内）不要断电、不要拨"USERINFO_LINE"<br>"
					"本注册功能仅用于新设备的认证及业务下发，已正常在用设备请勿重新注册<br>");

	return nBytesSent;
}

void formUserReg_inside_menu(request * wp, char *path, char *query)
{
	return formUserReg(wp,path,query);
}
#endif

#ifdef E8B_NEW_DIAGNOSE
int dumpPingInfo(int eid, request *wp, int argc, char **argv)
{
	int nBytesSent = 0;
	struct stat st;
	FILE *pf;
	char line[512];

	if (system("/bin/pidof ping > /dev/null") == 0) {
		nBytesSent += boaWrite(wp, "%s", "\t<div align=\"left\"><b>请稍候</b>\n\t<br><br>\n\t</div>\n");
	} else if (stat("/tmp/ping.tmp", &st) || st.st_size == 0) {
		nBytesSent += boaWrite(wp, "%s", "\t<div align=\"left\"><b>PING测试失败</b>\n\t<br><br>\n\t</div>\n");

		return nBytesSent;
	} else {
		nBytesSent += boaWrite(wp, "%s", "\t<div align=\"left\"><b>完成</b>\n\t<br><br>\n\t</div>\n");
	}

	pf = fopen("/tmp/ping.tmp", "r");
	if (!pf) {
		return nBytesSent;
	}

	nBytesSent += boaWrite(wp, "%s", "\t<pre>\n");
	while (fgets(line, sizeof(line), pf)) {
		nBytesSent += boaWrite(wp, "%s", line);
	}
	nBytesSent += boaWrite(wp, "%s", "\t</pre>\n");

	fclose(pf);

	return nBytesSent;
}

void formPing(request * wp, char *path, char *query)
{
	char *target_addr, *waninf;
	char cmd[256], outInf[IFNAMSIZ], wanname[MAX_WAN_NAME_LEN];
	int entries_num, i;
	MIB_CE_ATM_VC_T entry;
	char *proto ="";

	va_cmd("/bin/killall", 1, 0, "ping");
	unlink("/tmp/ping.tmp");

	target_addr = boaGetVar(wp, "target_addr", "");
	waninf = boaGetVar(wp, "waninf", "");

	if (!target_addr[0] || !waninf[0]) {
		ERR_MSG("目标地址或者WAN接口不正确!");
		return;
	}

	printf("starting ping(target: %s, interface: %s)...\n", target_addr, waninf);

	memset(&entry, 0, sizeof(MIB_CE_ATM_VC_T));

	entries_num = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entries_num; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			ERR_MSG("没有找到相应的WAN接口!");
			return;
		}

		getWanName(&entry, wanname);
		if (strcmp(waninf, wanname) == 0)
			break;
	}
	ifGetName(entry.ifIndex, outInf, sizeof(outInf));
	if(entry.IpProtocol == IPVER_IPV4)
		proto = "-4";
	else if(entry.IpProtocol == IPVER_IPV6)
		proto = "-6";

	snprintf(cmd, sizeof(cmd), "ping %s -c 4 -I %s -w 5 %s > /tmp/ping.tmp", proto, outInf, target_addr);

	va_cmd("/bin/sh", 2, 0, "-c", cmd);

	boaRedirect(wp, "/diag_ping_admin_result.asp");

	return;
}

int dumpTraceInfo(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;
	struct stat st;
	FILE *pf;
	char line[512];

	if (system("/bin/pidof traceroute > /dev/null") == 0) {
		nBytesSent += boaWrite(wp, "%s", "\t<div align=\"left\"><b>请稍候</b>\n\t<br><br>\n\t</div>\n");
	} else if (stat("/tmp/tracert.tmp", &st) || st.st_size == 0) {
		nBytesSent += boaWrite(wp, "%s", "\t<div align=\"left\"><b>Tracert测试失败</b>\n\t<br><br>\n\t</div>\n");

		return nBytesSent;
	} else {
		nBytesSent += boaWrite(wp, "%s", "\t<div align=\"left\"><b>完成</b>\n\t<br><br>\n\t</div>\n");
	}

	pf = fopen("/tmp/tracert.tmp", "r");
	if (!pf) {
		return nBytesSent;
	}

	nBytesSent += boaWrite(wp, "%s", "\t<pre>\n");
	while (fgets(line, sizeof(line), pf)) {
		nBytesSent += boaWrite(wp, "%s", line);
	}
	nBytesSent += boaWrite(wp, "%s", "\t</pre>\n");

	fclose(pf);

	return nBytesSent;
}

void formTracert(request * wp, char *path, char *query)
{
	char *target_addr, *wanInf, *proto = "";
	char line[512] = {0}, cmd[512] = {0}, outInf[20] = {0};
	FILE *pf = NULL;
	int entries_num = 0, i = 0;
	MIB_CE_ATM_VC_T entry;

	va_cmd("/bin/killall", 1, 0, "traceroute");

	unlink("/tmp/tracert.tmp");

	target_addr = boaGetVar(wp, "target_addr", "");
	wanInf = boaGetVar(wp, "waninf", "");

	printf("target: %s, wanInf: %s\n", target_addr, wanInf);

	if (!target_addr[0]) {
		ERR_MSG("地址不正确!");
		return;
	}

	entries_num = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entries_num; i ++) {
		char wanname[MAX_WAN_NAME_LEN] = {0};
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			ERR_MSG("没有找到相应的WAN接口!");
			return;
		}

		getWanName(&entry, wanname);
		if(strcmp(wanInf, wanname) == 0)
			break;
	}

	ifGetName(entry.ifIndex, outInf,sizeof(outInf));
	if(entry.IpProtocol == IPVER_IPV4)
		proto = "-4";
	else if(entry.IpProtocol == IPVER_IPV6)
		proto = "-6";

	//snprintf(cmd, sizeof(cmd), "traceroute -q 1 -w 2 %s > /tmp/tracert.tmp 2>&1", ip);
	//cxy 2015-1-14: traceroute use icmp  and specify out interface
	snprintf(cmd, sizeof(cmd), "traceroute %s -I -i %s %s > /tmp/tracert.tmp", proto, outInf, target_addr);

#if 1
	va_cmd("/bin/sh", 2, 0, "-c", cmd);

	boaRedirect(wp, "/diag_tracert_admin_result.asp");
#else
	if (outInf[0] != 0)
		temp_route_modify(ip, outInf, 1);
	va_cmd("/bin/sh", 2, 1, "-c", cmd);
	if (outInf[0] != 0)
		temp_route_modify(ip, outInf, 0);

	pf = fopen("/tmp/tracert.tmp", "r");
	if (pf) {
		boaHeader(wp);
   		boaWrite(wp, "<body><blockquote><pre>\n");
		while (fgets(line, sizeof(line), pf)) {
			printf("%s", line);
			boaWrite(wp, "%s", line);
		}
		boaWrite(wp, "</pre><form><input type=button value=\"  %s  \" OnClick=window.location.replace(\"/diag_tracert_admin.asp\")></form></blockquote></body>", IDS_RESULT_OK);
		boaFooter(wp);
		boaDone(wp, 200);
		fclose(pf);
	}
	//system("rm -rf /tmp/tracert.tmp");
	unlink("/tmp/tracert.tmp");
#endif
	return;
}
#endif

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

void ShowDefaultGateway(int eid, request * wp, int argc, char **argv)
{
#ifdef DEFAULT_GATEWAY_V2
	boaWrite(wp, "	<td colspan=4><input type=\"radio\" name=\"droute\" value=\"1\" onClick='autoDGWclicked()'>"
	"<font size=2><b>&nbsp;&nbsp;Obtain default gateway automatically</b></td>\n</tr>\n"
	"<tr><th></th>\n	<td colspan=4><input type=\"radio\" name=\"droute\" value=\"0\" onClick='autoDGWclicked()'>"
	"<font size=2><b>&nbsp;&nbsp;Use the following default gateway:</b></td>\n</tr>\n");
	boaWrite(wp, "<div id='gwInfo'>\n"
	"<tr><th></th>\n	<td>&nbsp;</td>\n"
	"	<td colspan=2><font size=2><input type=\"radio\" name='gwStr' value=\"0\" onClick='gwStrClick()'><b>&nbsp;Use Remote WAN IP Address:&nbsp;&nbsp;</b></td>\n"
	"	<td><div id='id_dfltgwy'><font size=2><input type='text' name='dstGtwy' maxlength=\"15\" size=\"10\"></div></td>\n</tr>\n"
	"<tr><th></th>\n	<td>&nbsp;</td>\n"
	"	<td colspan=2><font size=2><input type=\"radio\" name='gwStr' value=\"1\" onClick='gwStrClick()'><b>&nbsp;Use WAN Interface:&nbsp;&nbsp;</b></td>\n"
	"	<td><div id='id_wanIf'><font size=2><select name='wanIf'>");
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

void ShowPortMapping(int eid, request * wp, int argc, char **argv)
{
	int i;

	boaWrite(wp, "<table id=\"tbbind\" cellpadding=\"0px\" cellspacing=\"2px\">\n"
			"<tr class=\"sep\"><td colspan=\"2\"><hr align=\"left\" class=\"sep\" size=\"1\" width=\"100%%\"></td></tr>\n"
			"<tr nowrap><td width=\"150px\">绑定端口：</td><td>&nbsp;</td></tr>\n");
	for (i=PMAP_ETH0_SW0; i<=PMAP_ETH0_SW3; i++) {
		if (i < SW_LAN_PORT_NUM) {
			if (!(i&0x1))
				boaWrite(wp, "<tr nowrap>");
			boaWrite(wp, "<td><input type=checkbox name=chkpt>端口_%d</font></td>", i+1);
			if ((i&0x1) || (i+1) == SW_LAN_PORT_NUM)
				boaWrite(wp, "</tr>\n");
		}
		else
			boaWrite(wp, "<input type=hidden name=chkpt>\n");
	}

#ifdef WLAN_SUPPORT
	boaWrite(wp, "<tr nowrap><td><input type=\"checkbox\" name=\"chkpt\">无线(SSID1)</td>");
#ifdef WLAN_MBSSID
	int showNum = 0;
	MIB_CE_MBSSIB_T entry;
	for (i = 0; i < 3; i++)
	{
		mib_chain_get(MIB_MBSSIB_TBL, i + 1, &entry);
		if (entry.wlanDisabled) {
			continue;
		}

		showNum++;

		if (!(showNum & 0x1))
			boaWrite(wp, "<tr nowrap>");

		boaWrite(wp, "<td><input type=\"checkbox\" name=\"chkpt\">无线(SSID%d)</td>", i + 2);

		if ((showNum & 0x1))
			boaWrite(wp,  "</tr>\n");
	}

	if (!(showNum & 0x1))
		boaWrite(wp,  "</tr>\n");

	for (i = 0; i < (3 - showNum); i++)
		boaWrite(wp, "<input type=hidden name=chkpt>\n");
#else
	boaWrite(wp, "</tr>\n");
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
#endif
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
#else
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
	boaWrite(wp, "<input type=hidden name=chkpt>\n");
#endif

	boaWrite(wp, "</table>\n");
}

