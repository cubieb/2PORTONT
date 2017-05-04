/*
 *      Web server handler routines for IGMP proxy stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "multilang.h"

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

#ifdef CONFIG_USER_IGMPPROXY
#define RUNFILE "/var/run/igmp_pid"

///////////////////////////////////////////////////////////////////
void formIgmproxy(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
	FILE *fp;
	char * argv[8];
	char ifname[6];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char proxy;
	unsigned int proxy_if;
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	// Set IGMP proxy
	str = boaGetVar(wp, "proxy", "");
	if (str[0]) {
		if (str[0] == '0')
			proxy = 0;
		else
			proxy = 1;
		if ( !mib_set(MIB_IGMP_PROXY, (void *)&proxy)) {
			sprintf(tmpBuf, " %s (IGMP proxy).",Tset_mib_error);
			goto setErr_igmp;
		}
	}
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	setting_Igmproxy();
#else
	str = boaGetVar(wp, "proxy_if", "");
	if (str[0]) {
		proxy_if = (unsigned int)atoi(str);
		if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&proxy_if)) {
			sprintf(tmpBuf, " %s (IGMP proxy interface).",Tset_mib_error);
			goto setErr_igmp;
		}
	}

#ifdef EMBED
	startIgmproxy();
#endif
#endif // of CONFIG_IGMPPROXY_MULTIWAN

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

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

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);
	return;

setErr_igmp:
	ERR_MSG(tmpBuf);
}
#endif	// of CONFIG_USER_IGMPPROXY

// List all the available WAN side ip interface at web page.
// return: number of ip interface listed.
int ifwanList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	//char  buffer[3];
	char *name;
	int type, hasAny = 0;
	int innerHTML=0;  // Mason Yu
	unsigned char protocol = IPVER_IPV4_IPV6; // 1:IPv4, 2: IPv6, 3: Both. Mason Yu
	int hasVPN = 0;
#ifdef CONFIG_NET_IPIP
	MIB_IPIP_T ipip_Entry;
#endif

#ifdef CONFIG_USER_PPPOMODEM //for 3G card
	MIB_WAN_3G_T wan_3g_Entry,*wan_3g_p;
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	MIB_PPTP_T pptp_Entry;
#endif
#ifdef CONFIG_USER_L2TPD_L2TPD
	MIB_L2TP_T l2tp_Entry;
#endif
	char web_ifname[15];
	char wanname[IFNAMSIZ];
	unsigned int enable;

#if defined(IP_QOS) || defined(CONFIG_USER_IP_QOS_3)
	char interfaceName[16];
#endif
	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	if ( !strcmp(name, "all") )
		type = 0;
	else if ( !strcmp(name, "all2") ) {  // Mason Yu
		type = 0;
		innerHTML = 1;
	}
	else if ( !strcmp(name, "rt") ) {
		type = 1;	// route interface
		protocol = IPVER_IPV4;
	}
	else if ( !strcmp(name, "rt-any") )
		{
			type = 1; hasAny = 1;
		}
	else if ( !strcmp(name, "br") )
		type = 2;	// bridge interface
	else if ( !strcmp(name, "p2p") )
		type = 3;	// point-to-point interface
	else if ( !strcmp(name, "queueITF") )
		{
			type = 4; hasAny = 1;	// Queue interface
		}
	else if ( !strcmp(name, "queueITF-without-Any") )
		type = 4;  	// Queue interface without Any
#ifdef CONFIG_IPV6
	else if ( !strcmp(name, "rtv4") ) {  // Mason Yu
		type = 1;
		protocol = IPVER_IPV4;
	}
	else if ( !strcmp(name, "rtv6") ) {  // Mason Yu
		type = 1;
		protocol = IPVER_IPV6;
	}
#endif
#if defined(CONFIG_NET_IPIP) || defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
	else if ( !strcmp(name, "rt-any-vpn") )
		{
			type = 1; hasAny = 1; hasVPN = 1; protocol = IPVER_IPV4;
		}
	else if ( !strcmp(name, "rtv6-any-vpn") )
		{
			type = 1; hasAny = 1; hasVPN = 1; protocol = IPVER_IPV6;
		}
#endif
	else if (!strcmp(name, "adsl")) {
		type = 5;
	}
	else if (!strcmp(name, "vdsl")) {
		type = 6; //CONFIG_PTMWAN
	}
	else if (!strcmp(name, "eth")) {
		type = 7;
	}
	else if (!strcmp(name, "ptm")) {
		type = 8; //CONFIG_PTMWAN
	}
	else
		type = 1;	// default to route

	if (hasAny) {
		nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
				DUMMY_IFINDEX, multilang(LANG_ANY));
	}

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  		boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if(!isInterfaceMatch(Entry.ifIndex))
			continue;

		if (Entry.enable == 0 || !isValidMedia(Entry.ifIndex))
			continue;
 		ifGetName(Entry.ifIndex, wanname, sizeof(wanname));
		if (type == 2) {
			if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			{
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
					Entry.ifIndex, wanname);
				ifnum++;
			}
		}

#if defined(IP_QOS) || defined(CONFIG_USER_IP_QOS_3)
		else if (type == 4 && Entry.enableIpQos == 1 ) {
			getDisplayWanName(&Entry, interfaceName);
			nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
				Entry.ifIndex, interfaceName);
		}
#endif
		else if (type == 5) {
			if (MEDIA_INDEX(Entry.ifIndex) == MEDIA_ATM) {
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
						Entry.ifIndex, wanname);
			}
		}
		else if (type == 6) {
#ifdef CONFIG_PTMWAN
			if( (MEDIA_INDEX(Entry.ifIndex) == MEDIA_ATM)
			     || (MEDIA_INDEX(Entry.ifIndex) == MEDIA_PTM) ) {
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
						Entry.ifIndex, wanname);
			}
#endif /*CONFIG_PTMWAN*/
		}
		else if (type == 7) {
			if (MEDIA_INDEX(Entry.ifIndex) == MEDIA_ETH) {
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
						Entry.ifIndex, wanname);
			}
		}
		else if (type == 8) {
#ifdef CONFIG_PTMWAN
			if (MEDIA_INDEX(Entry.ifIndex) == MEDIA_PTM) {
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
						Entry.ifIndex, wanname);
			}
#endif /*CONFIG_PTMWAN*/
		}
		else { // rt or all (1 or 0)
#if defined(IP_QOS) || defined(CONFIG_USER_IP_QOS_3)
			if ( type == 4)
				continue;
#endif
			if ((type == 1 || type == 3) && Entry.cmode == CHANNEL_MODE_BRIDGE)
				continue;

			// check for p-2-p link
//			if (type == 3 && Entry.cmode == CHANNEL_MODE_IPOE)
			if (type == 3 && (Entry.cmode == CHANNEL_MODE_IPOE || Entry.ipunnumbered==1))	// Jenny
				continue;

			// Mason Yu. for IPv6
#ifdef CONFIG_IPV6
			if ( protocol == IPVER_IPV4_IPV6 || Entry.IpProtocol == IPVER_IPV4_IPV6 || protocol == Entry.IpProtocol ) {
				if ( innerHTML != 1 ) {
					nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
						Entry.ifIndex, wanname);
				} else {
					nBytesSent += boaWrite(wp, "<option value=%u>%s</option>",
						Entry.ifIndex, wanname);
				}
			}
#else
			if ( innerHTML != 1 ) {
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
					Entry.ifIndex, wanname);
			} else {
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>",
					Entry.ifIndex, wanname);
			}
#endif
			ifnum++;
		}
	}


#ifdef CONFIG_USER_PPPOMODEM
		wan_3g_p=&wan_3g_Entry;
		if( mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)wan_3g_p) && wan_3g_p->enable ){
			memset(web_ifname, 0, 15);
			snprintf( web_ifname,15, "ppp%d", MODEM_PPPIDX_FROM );
			//printf("web_ifname is %s\n",web_ifname);
			nBytesSent += boaWrite(wp, "<option value=%u>%s</option>",TO_IFINDEX(MEDIA_3G, MODEM_PPPIDX_FROM, 0), web_ifname);
		}
#endif

#ifdef CONFIG_USER_ZM8620_ECMMODE
		/*ericchung: it is hardcopy , need modify it, 4g inteface always use usb0 interface, value use 404040 */

		memset(web_ifname, 0, 15);
		snprintf( web_ifname,15, "usb%d", 0 );
		printf("web_ifname is %s\n",web_ifname);
		nBytesSent += boaWrite(wp, "<option value=%u>%s</option>",404040, web_ifname);

#endif




#if defined(CONFIG_NET_IPIP) || defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
	if (hasVPN) {
#ifdef CONFIG_NET_IPIP
		// (1) IPIP Tunnel
		if ( !mib_get(MIB_IPIP_ENABLE, (void *)&enable) ) {
			boaError(wp, 400, "Get MIB_IPIP_ENABLE chain record error!\n");
			return -1;
		}

		if ( enable) {
			entryNum = mib_chain_total(MIB_IPIP_TBL);
			for (i=0; i<entryNum; i++)
			{
				if (!mib_chain_get(MIB_IPIP_TBL, i, (void *)&ipip_Entry))
				{
					boaError(wp, 400, "Get MIB_IPIP_TBL chain record error!\n");
					return -1;
				}

				ifGetName(ipip_Entry.ifIndex, wanname, sizeof(wanname));
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
					ipip_Entry.ifIndex, wanname);
			}
		}
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
		// (2) PPTP Tunnel
		if ( !mib_get(MIB_PPTP_ENABLE, (void *)&enable) ) {
			boaError(wp, 400, "Get MIB_PPTP_ENABLE chain record error!\n");
			return -1;
		}

		if ( enable) {
			entryNum = mib_chain_total(MIB_PPTP_TBL);
			for (i=0; i<entryNum; i++)
			{
				if (!mib_chain_get(MIB_PPTP_TBL, i, (void *)&pptp_Entry))
				{
					boaError(wp, 400, "Get MIB_PPTP_TBL chain record error!\n");
					return -1;
				}

				ifGetName(pptp_Entry.ifIndex, wanname, sizeof(wanname));
				snprintf(web_ifname, 15, "pptp%d(%s)", pptp_Entry.idx, wanname);
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
					pptp_Entry.ifIndex, web_ifname);
			}
		}
#endif

#ifdef CONFIG_USER_L2TPD_L2TPD
		// (3) L2TP Tunnel
		if ( !mib_get(MIB_L2TP_ENABLE, (void *)&enable) ) {
			boaError(wp, 400, "Get MIB_L2TP_ENABLE chain record error!\n");
			return -1;
		}

		if ( enable) {
			entryNum = mib_chain_total(MIB_L2TP_TBL);
			for (i=0; i<entryNum; i++)
			{
				if (!mib_chain_get(MIB_L2TP_TBL, i, (void *)&l2tp_Entry))
				{
					boaError(wp, 400, "Get MIB_L2TP_TBL chain record error!\n");
					return -1;
				}

				ifGetName(l2tp_Entry.ifIndex, wanname, sizeof(wanname));
				snprintf(web_ifname, 15, "l2tp%d(%s)", l2tp_Entry.idx, wanname);
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
					l2tp_Entry.ifIndex, web_ifname);
			}
		}
#endif
	}
#endif
	//snprintf(buffer, 3, "%u", ifnum);
	//ejSetResult(eid, buffer);
	return nBytesSent;
}
