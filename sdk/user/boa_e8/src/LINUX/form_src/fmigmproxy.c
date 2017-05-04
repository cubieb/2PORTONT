/*
 *      Web server handler routines for IGMP proxy stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <signal.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "fmdefs.h"    // Mason Yu. IGMP Proxy for e8b

#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

extern int startIgmproxy();

#ifdef CONFIG_USER_IGMPPROXY
#define RUNFILE "/var/run/igmp_pid"
///////////////////////////////////////////////////////////////////
void formIgmproxy (request * wp, char *path, char *query)
{
	struct igmproxy_entry	entry;
	char*			stemp = "";
	int				index = 0;
	int				lineno = __LINE__;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char wanname[MAX_WAN_NAME_LEN];
	
	_BC_USE;
	
	_TRACE_CALL;

	_BC_INIT("bcdata");

	  while(_BC_NEXT())
	 {
		memset(&entry, 0, sizeof(struct igmproxy_entry));
		_BC_ENTRY_STR(ifName, _BC_NEED);
		_BC_ENTRY_INT(enable, _BC_NEED);
		if(entry.enable > 1){lineno = __LINE__; continue;}

		/************Place your code here, do what you want to do! ************/
		/************Place your code here, do what you want to do! ************/
               entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		
		
	       
	        for (i=0; i<entryNum; i++) 
	       {
	      
	              if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		      {
  			 boaError(wp, 400, "¶ÁÈ¡chain record´íÎó!\n"); //Get chain record error!
			 return;
		       }
                      getWanName(&Entry, wanname);

		      if(!strcmp(entry.ifName,wanname)){
                        if(entry.enable){

				 Entry.enableIGMP =1;
						  
                       	}else 
			  Entry.enableIGMP =0;
			   
		       mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry,i);
		       break;
		       }
		 		
		 }
			
	}
		   
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	setting_Igmproxy();
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif	 
	
	_COND_REDIRECT;
check_err:
	_BC_FREE();
	_TRACE_LEAVEL;
	return;
}

#if 0
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
			strcpy(tmpBuf, "Set IGMP proxy error!");
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
			strcpy(tmpBuf, "Set IGMP proxy interface index error!");
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
#endif

// Mason Yu. IGMP Proxy for e8b
int igmproxyList(int eid, request * wp, int argc, char ** argv)
{
	struct igmproxy_entry	entry ;
	int				cnt = 2;
	int				index = 0;
	int				lineno = __LINE__;

    unsigned int entryNum, i;
		
	MIB_CE_ATM_VC_T Entry;
        char wanname[MAX_WAN_NAME_LEN];
        unsigned char igmp_proxy;
	_TRACE_CALL;
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) 
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}
		
		if (Entry.enable == 0)
			continue;
		if((Entry.cmode == CHANNEL_MODE_IPOE||Entry.cmode == CHANNEL_MODE_PPPOA||Entry.cmode == CHANNEL_MODE_PPPOE||Entry.cmode == CHANNEL_MODE_RT1483))
		{
			if(Entry.IpProtocol & IPVER_IPV4){
			memset(&entry, 0, sizeof(struct igmproxy_entry));
			memset(wanname, 0, sizeof(wanname));
			getWanName(&Entry, wanname);
			strcpy(entry.ifName,wanname);
			entry.enable= Entry.enableIGMP;     // Mason Yu. IGMP Proxy for e8b
                       
			boaWrite(wp, "push(new it_nr(\"%d\"" _PTS _PTI "));\n", 
			entryNum, _PME(ifName), _PME(enable));
			}
		}	
		
	}
		
check_err:
	_TRACE_LEAVEL;
	return 0;
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
	int protocol = IPVER_IPV4_IPV6; // 1:IPv4, 2: IPv6, 3: both. Mason Yu
	int hasVPN = 0;
#ifdef CONFIG_NET_IPIP
	MIB_IPIP_T ipip_Entry;
#endif
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	MIB_PPTP_T pptp_Entry;
#endif
#ifdef CONFIG_USER_L2TPD_L2TPD
	MIB_L2TP_T l2tp_Entry;
#endif
	char web_ifname[15];
	char wanname[MAX_WAN_NAME_LEN];
	unsigned int enable;	
	
#ifdef IP_QOS
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
		type = 4;  	// Queue interface
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
			type = 1; hasAny = 1; hasVPN = 1;
		}
#endif
	else if (!strcmp(name, "adsl")) {
		type = 5;
	}
	else if (!strcmp(name, "vdsl")) {
		type = 6;
	}
	else if (!strcmp(name, "eth")) {
		type = 7;
	}
	else
		type = 1;	// default to route

	if (hasAny) {
		nBytesSent += boaWrite(wp, "<option value=%u>Any</option>\n", DUMMY_IFINDEX);
	}

#ifdef IP_QOS
	if ( type == 4 ) {
		nBytesSent += boaWrite(wp, "<option value=%d>(Click to Select)</option>\n", DUMMY_IFINDEX);
	}
#endif

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0 || !isValidMedia(Entry.ifIndex))
			continue;
 		//ifGetName(Entry.ifIndex, wanname, sizeof(wanname));
		getWanName(&Entry, wanname);
		if (type == 2) {
			if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			{
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
					Entry.ifIndex, wanname);
				ifnum++;
			}
		}
#ifdef IP_QOS
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
		/*
		else if (type == 6) {
		}
		*/
		else if (type == 7) {
			if (MEDIA_INDEX(Entry.ifIndex) == MEDIA_ETH) {
				nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
						Entry.ifIndex, wanname);
			}
		}
		else { // rt or all (1 or 0)
#ifdef IP_QOS
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
			if ( protocol == IPVER_IPV4_IPV6 || Entry.IpProtocol == IPVER_IPV4_IPV6 ) {
				if ( innerHTML != 1 ) {
					nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
						Entry.ifIndex, wanname);
				} else {
					nBytesSent += boaWrite(wp, "<option value=%u>%s</option>",
						Entry.ifIndex, wanname);
				}
			} else if ( protocol == Entry.IpProtocol ) {
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

// Mason Yu. IGMP snooping for e8b
void formIgmpSnooping(request * wp, char *path, char *query)
{
	char	*str, *submitUrl, *strSnoop;
	char tmpBuf[100], mode;	
#ifndef NO_ACTION
	int pid;
#endif	
	
#if defined(CONFIG_RTL_IGMP_SNOOPING)
	char origmode = 0;
	strSnoop = boaGetVar(wp, "snoop", "");
	if ( strSnoop[0] ) {
		// bitmap for virtual lan port function
		// Port Mapping: bit-0
		// QoS : bit-1
		// IGMP snooping: bit-2		
		mib_get(MIB_MPMODE, (void *)&mode);
		origmode = mode;
		strSnoop = boaGetVar(wp, "snoop", "");		
		if ( strSnoop[0] == '1' ) {
			mode |= 0x04;
			//if(origmode != mode)
			//	igmp_changed_flag = 1;
			// take effect immediately
			__dev_setupIGMPSnoop(1);
		}
		else {
			mode &= 0xfb;
			//if(origmode != mode)
			//	igmp_changed_flag = 1;
			__dev_setupIGMPSnoop(0);
		}		
		mib_set(MIB_MPMODE, (void *)&mode);
	}
#endif
#if defined(CONFIG_MCAST_VLAN) && defined(CONFIG_RTK_RG_INIT)
RTK_RG_ACL_Flush_mVlan();
RTK_RG_ACL_Add_mVlan();
#endif	
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif	
		
	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;	
}
