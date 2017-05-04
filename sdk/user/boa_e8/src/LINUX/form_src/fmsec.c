/*
 *      Web server handler routines for SEC
 *      Authors:
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <semaphore.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../uClibc/include/linux/autoconf.h"
#endif

/* for ioctl */
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#include "../webs.h"
#include "fmdefs.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include "../defs.h"

/*****************************
** 安全级
*/
int initPageFirewall(int eid, request *wp, int argc, char **argv)
{
	//防火墙等级:
	unsigned char 	filterLevel=1;	// 1- 低;  2- 中;  3- 高
	int				lineno = __LINE__;

	_TRACE_CALL;

	if (!mib_get(MIB_FW_GRADE, (void *)&filterLevel)) {
		printf("get fw grade fail\n");
	}

	_PUT_INT(filterLevel);

check_err:
	_TRACE_LEAVEL;
	return 0;
}

void formFirewall(request * wp, char *path, char *query)
{
	//防火墙等级:
	unsigned char 	filterLevel;	//1- 低;  2- 中;  3- 高
	char*			stemp = "";
	int				lineno = __LINE__;

	_TRACE_CALL;

	_GET_INT(filterLevel, _NEED);
	if(filterLevel > 3)	// out of range
	{
		lineno = __LINE__; goto check_err;
	}

	if (!mib_set(MIB_FW_GRADE, (void *)&filterLevel)) {
		printf("set firewall grade fail\n");
		goto check_err;
	}

	//write to flash
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	//take effect
	changeFwGrade(filterLevel);

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	return;
}

/*****************************
** 攻击保护设置
*/
int initPageDos(int eid, request *wp, int argc, char **argv)
{
	//使能:
	unsigned int		dosEnble;	// 1- 使能;  0- 禁用
	int				lineno = __LINE__;

	_TRACE_CALL;

	if (!mib_get(MIB_DOS_ENABLED, (void *)&dosEnble)) {
		printf("get DOS fail\n");
	}

	_PUT_BOOL(dosEnble);

check_err:
	_TRACE_LEAVEL;
	return 0;
}

void formDos(request * wp, char *path, char *query)
{
	//使能:
	unsigned int		dosEnble;	//1- 使能;  0- 禁用
	char*			stemp = "";
	int				lineno = __LINE__;
	unsigned int lastStatus;

	_TRACE_CALL;

	_GET_BOOL(dosEnble, _NEED);

	//if (!mib_get(MIB_DOS_ENABLED, (void *)&lastStatus))
	//	printf("get DOS failed!\n");

	if (!mib_set(MIB_DOS_ENABLED, (void *)&dosEnble)) {
		printf("set DOS failed!\n");
		goto check_err;
	}

	//write to flash
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	//take effect
	//if (lastStatus != dosEnble)
#ifdef CONFIG_RTK_RG_INIT
	RTK_RG_FLUSH_DOS_FILTER_RULE();
#endif
	if (dosEnble)
		setupDos();

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	return;
}

void changeMacFormat(char *str, char s, char d)
{
    int i=0;
    int length=strlen(str);
    for (i=0;i<length;i++)
    {
        if (str[i]==s) str[i]=d;
    }
}

int brgMacFilterList(int eid, request *wp, int argc, char **argv)
{
	unsigned char 		macFilterEnble = 1;
	unsigned char 		macFilterMode = 0;
	//struct brgmac_entry	entry = {1, 0, 0, 1, 0,"00-11-22-33-44-55", "11-11-22-33-44-55" };
	char ifname[1024] = "INTERNET_R_0_0_32";
	int cnt = 2;
	int index = 0;
	int lineno = __LINE__;
	int i;
	char entry_lost;

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	cnt=mib_chain_total(MIB_MAC_FILTER_EBTABLES_TBL);
	mib_get(MIB_MAC_FILTER_EBTABLES_ENABLE, &macFilterEnble);
	mib_get(MIB_MAC_FILTER_EBTABLES_MODE, &macFilterMode);
	/************Place your code here, do what you want to do! ************/

	_PUT_BOOL(macFilterEnble);
	_PUT_BOOL(macFilterMode);
	for(index = 0; index < cnt; index++)
	{
		memset(ifname, 0, sizeof(ifname));
		entry_lost = 0;

		/************Place your code here, do what you want to do! ************/
		struct brgmac_entry entry;
		mib_chain_get(MIB_MAC_FILTER_EBTABLES_TBL, index, &entry);

		for(i=0;i<entry.portNum;i++)
		{
			char name[256]={0};
			MIB_CE_ATM_VC_T pvc_entry;
			{
				if(getWanEntrybyindex(&pvc_entry, entry.ifIndex[i]|0xff00) == -1)
				{
					printf("%s:%d: This entry is not found, deleting...\n", __FILE__, __LINE__);
					mib_chain_delete(MIB_MAC_FILTER_EBTABLES_TBL, index);
					entry_lost = 1;
					break;
				}

				if(getWanName(&pvc_entry, name) != 1)
					printf("%s:%d: Get Wan name failed!\n", __FILE__, __LINE__);

				strcat(ifname,name);
				if(i!=entry.portNum-1)
					strcat(ifname,"/");
			}
		}

		if(entry_lost)
			continue;

		changeMacFormat(entry.dmac,':','-');
		changeMacFormat(entry.smac,':','-');
		/************Place your code here, do what you want to do! ************/

		boaWrite(wp, "push(new it_nr(\"%d\"" _PTS _PTS _PTI _PTI _PTI _PTI _PTS "));\n",
			index, _PME(dmac), _PME(smac), _PME(protoType), _PME(direction),
			_PME(allPort), _PME(portNum), "ifname",ifname);
	}

check_err:
	_TRACE_LEAVEL;
	return 0;
}

void formBrgMacFilter(request * wp, char *path, char *query)
{
	unsigned char macFilterEnble = 0;
	unsigned char macFilterMode = 0;
	MIB_CE_BRGMAC_T entry;
	MIB_CE_BRGMAC_T curEntry;
	char ifname[1024];
	char *stemp = "";
	char *send = NULL;
	char * stoken = NULL;
	char ** ppifs = NULL;
	int index = 0;
	int lineno = __LINE__;
	int entrynum, i;
	_BC_USE;
	memset(&entry,0,sizeof(struct brgmac_entry));
	FETCH_INVALID_OPT(stemp, "action", _NEED);
	if(strcmp(stemp, "sw") == 0)	//switch
	{
		_GET_BOOL(macFilterEnble, _NEED);
		/************Place your code here, do what you want to do! ************/
		printf("macFilterEnble=%d\n",macFilterEnble);
		mib_set(MIB_MAC_FILTER_EBTABLES_ENABLE,&macFilterEnble);
		//macFilterMode=0;//default is black list
		//mib_set(MIB_MAC_FILTER_EBTABLES_MODE,&macFilterMode);
		/************Place your code here, do what you want to do! ************/
	}else if(strcmp(stemp,"modesw")==0){
		_GET_BOOL(macFilterMode, _NEED);
		mib_chain_clear(MIB_MAC_FILTER_EBTABLES_TBL);
		mib_set(MIB_MAC_FILTER_EBTABLES_MODE,&macFilterMode);
	}
	else if(strcmp(stemp, "rm") == 0)	//remove
	{
		_BC_INIT("bcdata");
		while(_BC_NEXT())
		{
			memset(ifname, 0, sizeof(ifname));

			_BC_ENTRY_STR(dmac, _OPT);
			_BC_ENTRY_STR(smac, _OPT);
			_BC_ENTRY_INT(protoType, _OPT);
			_BC_ENTRY_INT(direction, _OPT);
			_BC_ENTRY_INT(allPort, _OPT);
			_BC_ENTRY_INT(portNum, _OPT);

			/************Place your code here, do what you want to do! ************/
			/*remove*/
			char *itf;
			itf=(char *)bc_gets(bc, "ifname");
			strncpy(ifname, itf, sizeof(ifname));
			stoken = &ifname[0];
			send = strchr(stoken, '/');
			index = 0;
			while(stoken && index < entry.portNum)
			{
				if(send)*send = '\0';

				//ppifs[index] = stoken;
				entry.ifIndex[index]=getifIndexByWanName(stoken);
				printf("entry[%d]=%d\n",index,entry.ifIndex[index]);
				if(send == NULL)break;
				stoken = send + 1;
				send = strchr(stoken, '/');
				index++;
			}
			/************Place your code here, do what you want to do! ************/
			int t=0;
			char *bugentry=(char*)&entry;
			changeMacFormat(entry.smac,'-',':');
			changeMacFormat(entry.dmac,'-',':');
			MIB_CHAIN_DELETE(MIB_MAC_FILTER_EBTABLES_TBL, struct brgmac_entry, entry)
		}
	}
	else if(strcmp(stemp, "ad") == 0)	//add
	{
		memset(ifname, 0, sizeof(ifname));
		if(mib_chain_total(MIB_MAC_FILTER_EBTABLES_TBL)>=MAC_FILTER_BRIDGE_RULES)
		{
			ERR_MSG("对不起,规则数已达最大限制!");
			goto check_err;
		}
		_ENTRY_STR(dmac, _OPT);
		_ENTRY_STR(smac, _OPT);
		_ENTRY_INT(protoType, _OPT);
		if(entry.protoType > 7) {lineno = __LINE__; goto check_err;}
		_ENTRY_INT(direction, _OPT);
		if(entry.direction > 2) {lineno = __LINE__; goto check_err;}
		_ENTRY_INT(allPort, _OPT);
		if(entry.allPort > 1) {lineno = __LINE__; goto check_err;}
		if(!entry.allPort)
		{
			_ENTRY_INT(portNum, _NEED);
			FETCH_INVALID_OPT(stemp, "ifname", _OPT);
			strncpy(ifname, stemp, sizeof(ifname));
			//if(ppifs) {free(ppifs); ppifs = NULL;}
			//ppifs = (char**)malloc((entry.portNum + 1) * sizeof(char*));
			stoken = &ifname[0];
			send = strchr(stoken, ';');
			index = 0;
			while(stoken && index < entry.portNum)
			{
				if(send)*send = '\0';

				//ppifs[index] = stoken;
				entry.ifIndex[index]=atoi(stoken);
				printf("ifIndex[%d]=%d\n", index, entry.ifIndex[index]);
				if(send == NULL) break;
				stoken = send + 1;
				send = strchr(stoken, ';');
				index++;
			}
		}

		/************Place your code here, do what you want to do! ************/
		//fix the mac format form xx-xx-xx-xx-xx-xx to xx:xx:xx:xx:xx
		changeMacFormat(entry.smac,'-',':');
		changeMacFormat(entry.dmac,'-',':');

		entrynum = mib_chain_total(MIB_MAC_FILTER_EBTABLES_TBL);
		for( i = 0; i < entrynum; i++ )
		{
			if(!mib_chain_get(MIB_MAC_FILTER_EBTABLES_TBL, i, (void *)&curEntry))
				continue;

			if(!strcmp(entry.smac, curEntry.smac) && !strcmp(entry.dmac, curEntry.dmac) && entry.protoType == curEntry.protoType &&
				entry.direction == curEntry.direction && !memcmp(entry.ifIndex, curEntry.ifIndex, 8*sizeof(int)))
				goto Dup_ERR;
		}

		mib_chain_add(MIB_MAC_FILTER_EBTABLES_TBL, &entry);
		/************Place your code here, do what you want to do! ************/
	}
	else {lineno = __LINE__; goto check_err;}

	restart_IPFilter_DMZ_MACFilter();

	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	_COND_REDIRECT;
check_err:
	_BC_FREE();
	if(ppifs) {free(ppifs); ppifs = NULL;}
	_TRACE_LEAVEL;
	return;
Dup_ERR:
	_BC_FREE();
	if(ppifs) {free(ppifs); ppifs = NULL;}
	_TRACE_LEAVEL;
	ERR_MSG("This rule already existed!");
	return;
}

/*****************************
** 路由MAC过滤
*/
int rteMacFilterList(int eid, request * wp, int argc, char ** argv)
{
	unsigned char macFilterEnble;
	struct routemac_entry	entry = {"00-11-22-33-44-55", "INTERNET_R_0_0_32"};
	int cnt = 2;
	int index = 0;
	int lineno = __LINE__;

	_TRACE_CALL;

	mib_get(MIB_MAC_FILTER_SRC_ENABLE, &macFilterEnble);

	_PUT_BOOL(macFilterEnble);

	if(macFilterEnble){
		/************Place your code here, do what you want to do! ************/
		cnt=mib_chain_total(MIB_MAC_FILTER_ROUTER_TBL);
		/************Place your code here, do what you want to do! ************/

		for(index = 0; index < cnt; index++)
		{
			/************Place your code here, do what you want to do! ************/
			memset(&entry,0,sizeof(struct routemac_entry));
			mib_chain_get(MIB_MAC_FILTER_ROUTER_TBL,index,&entry);
			/************Place your code here, do what you want to do! ************/
			changeMacFormat(entry.mac,':','-');
			boaWrite(wp, "push(new it_nr(\"%d\"" _PTS _PTS "));\n",
				index, _PME(devname), _PME(mac));
		}
	}
	_TRACE_LEAVEL;
	return 0;
}

void formRteMacFilter(request *wp, char *path, char *query)
{
	struct routemac_entry	entry;
	char *stemp = "";
	int index = 0;
	int lineno = __LINE__;
	unsigned char 		macFilterEnble = 0;
	_BC_USE;

	_TRACE_CALL;
	FETCH_INVALID_OPT(stemp, "action", _NEED);

	if(strcmp(stemp, "sw") == 0)	//switch
	{
		_GET_BOOL(macFilterEnble, _NEED);
		printf("macFilterEnble=%d\n",macFilterEnble);
		mib_set(MIB_MAC_FILTER_SRC_ENABLE,&macFilterEnble);
	}
	else if(strcmp(stemp, "rm") == 0)	//remove
	{
		_BC_INIT("bcdata");
		while(_BC_NEXT())
		{
			memset(&entry, 0, sizeof(struct routemac_entry));
			//_BC_ENTRY_STR(devname, _OPT);
			_BC_ENTRY_STR(mac, _OPT);
			changeMacFormat(entry.mac,'-',':');
			/************Place your code here, do what you want to do! ************/
			/*remove*/
			MIB_CHAIN_DELETE(MIB_MAC_FILTER_ROUTER_TBL,struct routemac_entry,entry);
			/************Place your code here, do what you want to do! ************/
		}
	}
	else if(strcmp(stemp, "ad") == 0)	//add
	{
		if(mib_chain_total(MIB_MAC_FILTER_ROUTER_TBL)>=MAC_FILTER_ROUTER_RULES)
		{
			ERR_MSG("对不起,规则数已达最大限制!");
			goto check_err;
		}
		memset(&entry, 0, sizeof(struct routemac_entry));
		//_ENTRY_STR(devname, _NEED);
		_ENTRY_STR(mac, _NEED);

		/************Place your code here, do what you want to do! ************/
		changeMacFormat(entry.mac,'-',':');
		mib_chain_add(MIB_MAC_FILTER_ROUTER_TBL,&entry);
		/************Place your code here, do what you want to do! ************/
	}
	else {lineno = __LINE__; goto check_err;}

	restart_IPFilter_DMZ_MACFilter();

	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	_COND_REDIRECT;
check_err:
	_BC_FREE();
	_TRACE_LEAVEL;
	return;
}

/*****************************
** 端口过滤
*/
int ipPortFilterConfig(int eid, request * wp, int argc, char ** argv)
{
	//IP地址过滤启用:
	unsigned char ipfilterEnable = 0;	//1- 启用;  0- 禁用
	int lineno = __LINE__;

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	mib_get(MIB_IPFILTER_ON_OFF, (void*)&ipfilterEnable);
	/************Place your code here, do what you want to do! ************/

	_PUT_BOOL(ipfilterEnable);

check_err:
	_TRACE_LEAVEL;
	return 0;
}

void formPortFilter(request * wp, char *path, char *query)
{
	//IP地址过滤启用:
	unsigned char ipfilterEnable = 0;		//1- 启用;  0- 禁用
	char *stemp = "";
	int lineno = __LINE__;
	unsigned char original_state = 0;
	_TRACE_CALL;

	_GET_BOOL(ipfilterEnable, _NEED);

	/************Place your code here, do what you want to do! ************/
	mib_get(MIB_IPFILTER_ON_OFF, (void*)&original_state);
	if(ipfilterEnable)
		ipfilterEnable=1;
	//printf("new=%d, old=%d, new^old=%d\n",ipfilterEnable, original_state, (ipfilterEnable^original_state));
	if(ipfilterEnable^original_state)
	{//mib set only if ipfitler switch changed...
		//printf("###%d###\n", ipfilterEnable);
		mib_set(MIB_IPFILTER_ON_OFF, (void*)&ipfilterEnable);
		//take effect immediate...
		//setupFirewall();
		restart_IPFilter_DMZ_MACFilter();
		//Write to flash, take effect forever
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
	/************Place your code here, do what you want to do! ************/

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	return;
}

//DIR_OUT 0
//DIR_IN    1
int parseIpFilterInfo2Entry(struct ipfilter_blacklist_entry *entry, MIB_CE_IP_PORT_FILTER_T *filterEntry, int direction)
{
	unsigned long mask, mbit;
#ifdef CONFIG_IPV6
	unsigned char o_ipProtocol;
	o_ipProtocol = filterEntry->IpProtocol;
#endif
	memset(filterEntry, 0x00, sizeof(MIB_CE_IP_PORT_FILTER_T));
#ifdef CONFIG_IPV6
	filterEntry->IpProtocol = o_ipProtocol;
#endif

	strncpy(filterEntry->name, entry->filterName, 32);
	switch(entry->protoType)
	{
		case 0:
			filterEntry->protoType=PROTO_NONE;
			break;
		case 1:
			filterEntry->protoType=PROTO_UDPTCP;
			break;
		case 2:
			filterEntry->protoType=PROTO_TCP;
			break;
		case 3:
			filterEntry->protoType=PROTO_UDP;
			break;
		case 4:
			filterEntry->protoType=PROTO_ICMP;
			break;
	}

#ifdef CONFIG_IPV6
	// If it is a IPv4 rule.
	if ( filterEntry->IpProtocol == IPVER_IPV4 ) {
#endif
	memcpy(filterEntry->srcIp, &entry->sipStart, 4);
	memcpy(filterEntry->srcIp2, &entry->sipEnd, 4);
	memcpy(filterEntry->dstIp, &entry->dipStart, 4);
	memcpy(filterEntry->dstIp2, &entry->dipEnd, 4);
#ifdef CONFIG_IPV6
	}
#endif

	filterEntry->srcPortFrom=entry->sportStart;
	filterEntry->srcPortTo=entry->sportEnd;
	filterEntry->dstPortFrom=entry->dportStart;
	filterEntry->dstPortTo=entry->dportEnd;
	filterEntry->dir=direction;

#ifdef CONFIG_IPV6
	// If it is a IPv4 rule.
	if ( filterEntry->IpProtocol == IPVER_IPV4 ) {
#endif
	mask=entry->smask;
	mbit=0;
	while (1) {
		if (mask&0x80000000) {
			mbit++;
			mask <<= 1;
		}
		else
			break;
	}
	filterEntry->smaskbit = mbit;
	mask=entry->dmask;
	mbit=0;
	while (1) {
		if (mask&0x80000000) {
			mbit++;
			mask <<= 1;
		}
		else
			break;
	}
	filterEntry->dmaskbit = mbit;
#ifdef CONFIG_IPV6
	}
#endif

	if(direction == DIR_OUT)
		filterEntry->action=0; //deny
	else
		filterEntry->action=1; //allow
	return 0;
}

// caculate the total entry numbers at one direction (incoming or outgoing)
int getTotalIPFilterNumOneDirection(int dir)
{
	int entrynum=0;
	int totalentrynum=0;
	int index;
	MIB_CE_IP_PORT_FILTER_T Entry;
	totalentrynum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
	for(index = 0; index < totalentrynum; index++)
	{
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, index, (void *)&Entry))
		{
			return 0;
		}
		if (Entry.dir == dir)
		{
			entrynum++;
		}
	}
	return entrynum;
}
//dir DIR_IN    1
//     DIR_OUT 0
// pIndex tell caller the found entry's index in table, if can.
// ret   1: the entry found , otherwise not found...
int findIPFilterEntrybyNameDirection(char *name, int dir, MIB_CE_IP_PORT_FILTER_T *pEntry, int *pIndex)
{
	int entrynum=0;
	int index;
	MIB_CE_IP_PORT_FILTER_T Entry;
	entrynum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
	for(index = 0; index < entrynum; index++)
	{
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, index, (void *)&Entry))
		{
			return 0;
		}
		if (Entry.dir != dir) //bypass the reversed direction rules...
			continue;
		if(!strncmp(Entry.name, name , sizeof(Entry.name)))
		{
			if(pEntry)
				memcpy(pEntry, &Entry, sizeof(MIB_CE_IP_PORT_FILTER_T));
			if(pIndex)
				*pIndex=index;
			//found the entry by name and direction
			return 1;
		}
	}
	//not found the wanted entry....
	return 0;
}

int ipPortFilterBlacklist (int eid, request * wp, int argc, char ** argv)
{
	//IP地址过滤启用:
	struct ipfilter_blacklist_entry	entry = {"test", 1, 0x10101010, 0x11101010, 0x00FFFFFF, 80, 81, 0x10101010, 0x11101010, 0x00FFFFFF, 80, 81};
	char sipStart[16];
	char sipEnd[16];
	char smask[16];
	char dipStart[16];
	char dipEnd[16];
	char dmask[16];
	int cnt = 2;
	int index = 0;
	int lineno = __LINE__;
#ifdef CONFIG_IPV6
	unsigned char 	sip6StartStr[48], dip6StartStr[48], sip6EndStr[48], dip6EndStr[48];
#endif

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	MIB_CE_IP_PORT_FILTER_T Mib_Entry;
	cnt = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
	/************Place your code here, do what you want to do! ************/

	for(index = 0; index < cnt; index++)
	{
		/************Place your code here, do what you want to do! ************/
		memset(&entry, 0, sizeof(entry));
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, index, (void *)&Mib_Entry))
		{
 			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}
		if (Mib_Entry.dir == DIR_IN) //incoming rules, which belong to white list...
		{
			continue;
		}
		memcpy(entry.filterName, Mib_Entry.name, sizeof(entry.filterName));
		switch(Mib_Entry.protoType)
		{
			case PROTO_NONE:
				entry.protoType=0;
				break;
			case PROTO_UDPTCP:
				entry.protoType=1;
				break;
			case PROTO_TCP:
				entry.protoType=2;
				break;
			case PROTO_UDP:
				entry.protoType=3;
				break;
			case PROTO_ICMP:
				entry.protoType=4;
				break;
		}
		memcpy(&entry.sipStart, Mib_Entry.srcIp, 4);
		memcpy(&entry.sipEnd, Mib_Entry.srcIp2, 4);
		entry.smask=0xFFFFFFFF;
		if(Mib_Entry.smaskbit !=0)
			entry.smask =entry.smask<<(32-Mib_Entry.smaskbit);
		else
			entry.smask=0;
		memcpy(&entry.dipStart, Mib_Entry.dstIp, 4);
		memcpy(&entry.dipEnd, Mib_Entry.dstIp2, 4);
		entry.dmask=0xFFFFFFFF;
		if(Mib_Entry.dmaskbit !=0)
			entry.dmask =(entry.dmask<<(32-Mib_Entry.dmaskbit));
		else
			entry.dmask=0;
		entry.sportStart=Mib_Entry.srcPortFrom;
		entry.sportEnd=Mib_Entry.srcPortTo;
		entry.dportStart=Mib_Entry.dstPortFrom;
		entry.dportEnd=Mib_Entry.dstPortTo;

#ifdef CONFIG_IPV6
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.sip6Start, sip6StartStr, sizeof(sip6StartStr));
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.sip6End, sip6EndStr, sizeof(sip6EndStr));
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.dip6Start, dip6StartStr, sizeof(dip6StartStr));
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.dip6End, dip6EndStr, sizeof(dip6EndStr));
#endif
		/************Place your code here, do what you want to do! ************/

		boaWrite (wp, "push(new it_nr(\"%d\"" _PTS _PTI _PTS _PTS _PTS _PTS _PTS _PTS _PTI _PTI _PTI _PTI
#ifdef CONFIG_IPV6
		", \nnew it(\"IpProtocolType\",%d),\n"  	//ipv4 or ipv6
		"new it(\"sip6Start\",  \"%s\"),\n" 	//source ip6Start
		"new it(\"sip6End\",  \"%s\"),\n" 		//source ip6End
		"new it(\"dip6Start\",  \"%s\"),\n" 	//dst ip6Start
		"new it(\"dip6End\",  \"%s\"),\n" 		//dst ip6End
		"new it(\"sip6PrefixLen\",%d),\n"  	 	//source ip6 Prefix Len
		"new it(\"dip6PrefixLen\",%d)\n"  		//dst ip6 Prefix Len
#endif
		"));\n",
		index, _PME(filterName), _PME(protoType),
		_PMEIP(sipStart), _PMEIP(sipEnd), _PMEIP(smask), _PMEIP(dipStart), _PMEIP(dipEnd), _PMEIP(dmask),
		_PME(sportStart), _PME(sportEnd), _PME(dportStart), _PME(dportEnd)
#ifdef CONFIG_IPV6
		, Mib_Entry.IpProtocol, sip6StartStr, sip6EndStr, dip6StartStr, dip6EndStr, Mib_Entry.sip6PrefixLen, Mib_Entry.dip6PrefixLen
#endif
		);
	}

check_err:
	_TRACE_LEAVEL;
	return 0;
}

int ipPortFilterWhitelist (int eid, request * wp, int argc, char ** argv)
{
	//IP地址过滤启用:
	struct ipfilter_whitelist_entry	entry = {"test", 1, 0x10101010, 0x11101010, 0x00FFFFFF, 80, 81, 0x10101010, 0x11101010, 0x00FFFFFF, 80, 81, 0, 2, NULL};
	char sipStart[16];
	char sipEnd[16];
	char smask[16];
	char dipStart[16];
	char dipEnd[16];
	char dmask[16];
	char ifname[1024] = "INTERNET_R_0_0_32;INTERNET_R_0_8_35";
	int cnt = 2;
	int index = 0;
	int lineno = __LINE__;
#ifdef CONFIG_IPV6
	unsigned char 	sip6StartStr[48], dip6StartStr[48], sip6EndStr[48], dip6EndStr[48];
#endif

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	MIB_CE_IP_PORT_FILTER_T Mib_Entry;
	cnt = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
	/************Place your code here, do what you want to do! ************/

	for(index = 0; index < cnt; index++)
	{
		/************Place your code here, do what you want to do! ************/
		memset(&entry, 0, sizeof(entry));
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, index, (void *)&Mib_Entry))
		{
  		boaError(wp, 400, "读取chain record错误!\n"); //Get chain record error!
			return -1;
		}
		if (Mib_Entry.dir == DIR_OUT) //outgoing rules, which belong to black list...
		{
			continue;
		}
		memcpy(entry.filterName, Mib_Entry.name, sizeof(entry.filterName));
		switch(Mib_Entry.protoType)
		{
			case PROTO_NONE:
				entry.protoType=0;
				break;
			case PROTO_UDPTCP:
				entry.protoType=1;
				break;
			case PROTO_TCP:
				entry.protoType=2;
				break;
			case PROTO_UDP:
				entry.protoType=3;
				break;
			case PROTO_ICMP:
				entry.protoType=4;
				break;
		}
		memcpy(&entry.sipStart, Mib_Entry.srcIp, 4);
		memcpy(&entry.sipEnd, Mib_Entry.srcIp2, 4);
		entry.smask=0xFFFFFFFF;
		if(Mib_Entry.smaskbit !=0)
			entry.smask =entry.smask<<(32-Mib_Entry.smaskbit);
		else
			entry.smask=0;
		memcpy(&entry.dipStart, Mib_Entry.dstIp, 4);
		memcpy(&entry.dipEnd, Mib_Entry.dstIp2, 4);
		entry.dmask=0xFFFFFFFF;
		if(Mib_Entry.dmaskbit !=0)
			entry.dmask =(entry.dmask<<(32-Mib_Entry.dmaskbit));
		else
			entry.dmask=0;
		entry.sportStart=Mib_Entry.srcPortFrom;
		entry.sportEnd=Mib_Entry.srcPortTo;
		entry.dportStart=Mib_Entry.dstPortFrom;
		entry.dportEnd=Mib_Entry.dstPortTo;

#ifdef CONFIG_IPV6
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.sip6Start, sip6StartStr, sizeof(sip6StartStr));
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.sip6End, sip6EndStr, sizeof(sip6EndStr));
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.dip6Start, dip6StartStr, sizeof(dip6StartStr));
		inet_ntop(PF_INET6, (struct in6_addr *)Mib_Entry.dip6End, dip6EndStr, sizeof(dip6EndStr));
#endif
		/************Place your code here, do what you want to do! ************/

		boaWrite (wp, "push(new it_nr(\"%d\"" _PTS _PTI _PTS _PTS _PTS _PTS _PTS _PTS _PTI _PTI _PTI _PTI _PTI _PTI _PTS
#ifdef CONFIG_IPV6
		", \nnew it(\"IpProtocolType\",%d),\n"  //ipv4 or ipv6
		"new it(\"sip6Start\",  \"%s\"),\n" 	//source ip6Start
		"new it(\"sip6End\",  \"%s\"),\n" 		//source ip6End
		"new it(\"dip6Start\",  \"%s\"),\n" 	//dst ip6Start
		"new it(\"dip6End\",  \"%s\"),\n" 		//dst ip6End
		"new it(\"sip6PrefixLen\",%d),\n"  	 	//source ip6 Prefix Len
		"new it(\"dip6PrefixLen\",%d)\n"  		//dst ip6 Prefix Len
#endif
		"));\n",
		index, _PME(filterName), _PME(protoType),
		_PMEIP(sipStart), _PMEIP(sipEnd), _PMEIP(smask), _PMEIP(dipStart), _PMEIP(dipEnd), _PMEIP(dmask),
		_PME(sportStart), _PME(sportEnd), _PME(dportStart), _PME(dportEnd), _PME(allport), _PME(portnum), _PMEX(ifname)
#ifdef CONFIG_IPV6
		, Mib_Entry.IpProtocol, sip6StartStr, sip6EndStr, dip6StartStr, dip6EndStr, Mib_Entry.sip6PrefixLen, Mib_Entry.dip6PrefixLen
#endif
		);
	}
	_TRACE_LEAVEL;
	return 0;
}

void formPortFilterBlack(request * wp, char *path, char *query)
{
	struct ipfilter_blacklist_entry entry;
	char* stemp = "";
	int lineno = __LINE__;
	int index = 0;
	MIB_CE_IP_PORT_FILTER_T filterEntry;
	char ret;
	_BC_USE;
#ifdef CONFIG_IPV6
	char *str, ipv6_protoType=0;
#endif

	_TRACE_CALL;

	FETCH_INVALID_OPT(stemp, "action", _NEED);

	if(strcmp(stemp, "rm") == 0)	//remove
	{
		_BC_INIT("bcdata");
		while(_BC_NEXT())
		{
			memset(&entry, 0, sizeof(entry));
			_BC_ENTRY_STR(filterName, _OPT);
#if 0
			_BC_ENTRY_INT(protoType, _OPT);
			_BC_ENTRY_IP(sipStart, _OPT);
			_BC_ENTRY_IP(sipEnd, _OPT);
			_BC_ENTRY_IP(smask, _OPT);
			_BC_ENTRY_INT(sportStart, _OPT);
			_BC_ENTRY_INT(sportEnd, _OPT);
			_BC_ENTRY_IP(dipStart, _OPT);
			_BC_ENTRY_IP(dipEnd, _OPT);
			_BC_ENTRY_IP(dmask, _OPT);
			_BC_ENTRY_INT(dportStart, _OPT);
			_BC_ENTRY_INT(dportEnd, _OPT);
#endif
			/************Place your code here, do what you want to do! ************/
			/*remove*/
			if(findIPFilterEntrybyNameDirection(entry.filterName, DIR_OUT, NULL, &index))
				mib_chain_delete(MIB_IP_PORT_FILTER_TBL, index);

			/************Place your code here, do what you want to do! ************/
		}
		//take effect immediately...
		//setupFirewall();
		// Mason Yu. Take effect in real time; // Magician: Merge restart MAC filter into restart IPFilter and DMZ
		restart_IPFilter_DMZ_MACFilter();
	}
	else if(strcmp(stemp, "ad") == 0)	//add
	{
		memset(&entry, 0, sizeof(entry));

		_ENTRY_STR(filterName, _NEED);
		_ENTRY_INT(protoType, _NEED);
		//if(entry.protoType > 4){lineno = __LINE__; goto check_err;}
		_ENTRY_IP(sipStart, _OPT);
		_ENTRY_IP(sipEnd, _OPT);
		_ENTRY_IP(smask, _OPT);
		_ENTRY_IP(dipStart, _OPT);
		_ENTRY_IP(dipEnd, _OPT);
		_ENTRY_IP(dmask, _OPT);
		_ENTRY_INT(sportStart, _OPT);
		_ENTRY_INT(sportEnd, _OPT);

		if(entry.sportEnd == 0)
			entry.sportEnd = entry.sportStart;
		
		if(stemp && *stemp != 0)
		{
			if(entry.sportEnd < entry.sportStart){lineno = __LINE__; goto check_err;}
		}

		_ENTRY_INT(dportStart, _OPT);
		_ENTRY_INT(dportEnd, _OPT);
		
		if(entry.dportEnd == 0)
			entry.dportEnd = entry.dportStart;
		
		if(stemp && *stemp != 0)
		{
			if(entry.dportEnd < entry.dportStart){lineno = __LINE__; goto check_err;}
		}

		/************Place your code here, do what you want to do! ************/
		/*add new*/
		//jim double check the parameters validity.
		if(!entry.sipStart)
		{
			entry.smask=0;
			entry.sipEnd=0;
		}
		if(!entry.dipStart)
		{
			entry.dmask=0;
			entry.dipEnd=0;
		}
#ifdef CONFIG_IPV6
		str = boaGetVar(wp, "protoTypeV6", "");
		if (str[0]) {
			ipv6_protoType = (char)atoi(str);
		}

		if(entry.protoType == 0 && ipv6_protoType == 0) //None proto... don't care ,, all.
#else
		if(entry.protoType == 0) //None proto... don't care ,, all.
#endif
		{    //mask off port info...
			entry.sportStart=0;
			entry.sportEnd=0;
			entry.dportStart=0;
			entry.dportEnd=0;
		}
		{		// IP/Port FILTER
			MIB_CE_IP_PORT_FILTER_T filterEntry;
			// if the exist entry with the same name, abandon the add action...
			if(findIPFilterEntrybyNameDirection(entry.filterName, DIR_OUT, NULL, NULL) \
				|| getTotalIPFilterNumOneDirection(DIR_OUT) > MAX_OUTGOING_IPFILTER_RULE_NUM)
				goto check_err;
#ifdef CONFIG_IPV6
			str = boaGetVar(wp, "IpProtocolType", "");
			filterEntry.IpProtocol = (char)atoi(str);
			// If it is a IPv6 rule. Save the protoType into  ipfilter_whitelist data base.
			// Because the parseIpFilterInfo2Entry() will clean the MIB's ipportfilter_entry.
			if ( filterEntry.IpProtocol == IPVER_IPV6 )
				entry.protoType = ipv6_protoType;
#endif
			parseIpFilterInfo2Entry(&entry, &filterEntry, DIR_OUT);

#ifdef CONFIG_IPV6
			// If it is a IPv6 rule.
			if ( filterEntry.IpProtocol == IPVER_IPV6 )
			{
				str = boaGetVar(wp, "sip6Start", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.sip6Start)) {
						printf("Invalid sip6Start for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "sip6End", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.sip6End)) {
						printf("Invalid sip6End for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "dip6Start", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.dip6Start)) {
						printf("Invalid dip6Start for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "dip6End", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.dip6End)) {
						printf("Invalid dip6End for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "sip6PrefixLen", "");
				filterEntry.sip6PrefixLen = (char)atoi(str);

				str = boaGetVar(wp, "dip6PrefixLen", "");
				filterEntry.dip6PrefixLen = (char)atoi(str);
			}
#endif
			ret = mib_chain_add(MIB_IP_PORT_FILTER_TBL, (unsigned char*)&filterEntry);
			if(ret == -1)
				goto Max_Size_Reached;
			else if( ret == 0 )
				goto check_err;

			//take effect immediate
			restart_IPFilter_DMZ_MACFilter();
		}
		/************Place your code here, do what you want to do! ************/
	}
	else {lineno = __LINE__; goto check_err;}
	//Write to flash, take effect forever
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	_COND_REDIRECT;
Max_Size_Reached:
	_BC_FREE();
	_TRACE_LEAVEL;
	ERR_MSG("规则数已达最大限制!"); //Max number of rules reached!
	return;
check_err:
	_BC_FREE();
	_TRACE_LEAVEL;
	ERR_MSG("设定错误!");
	return;
}

void formPortFilterWhite(request * wp, char *path, char *query)
{
	struct ipfilter_whitelist_entry	entry;
	char				isblack = 0;
	char*				slist = NULL;
	char*				stemp = "";
	char*				send = NULL;
	char*				stoken = NULL;
	int					lineno = __LINE__;
	int					index = 0;
	char				iffs[20][20];	//接口列表
	MIB_CE_IP_PORT_FILTER_T filterEntry;
	int j, ret;
#ifdef CONFIG_IPV6
	char *str, ipv6_protoType=0;
	MIB_CE_IP_PORT_FILTER_T IpEntry;
#endif

	_BC_USE;

	_TRACE_CALL;

	FETCH_INVALID_OPT(stemp, "action", _NEED);

	if(strcmp(stemp, "rm") == 0)	//remove
	{
		_BC_INIT("bcdata");
		while(_BC_NEXT())
		{
			memset(&entry, 0, sizeof(entry));
			_BC_ENTRY_STR(filterName, _OPT);
#if 0
			_BC_ENTRY_INT(protoType, _OPT);
			_BC_ENTRY_IP(sipStart, _OPT);
			_BC_ENTRY_IP(sipEnd, _OPT);
			_BC_ENTRY_IP(smask, _OPT);
			_BC_ENTRY_INT(sportStart, _OPT);
			_BC_ENTRY_INT(sportEnd, _OPT);
			_BC_ENTRY_IP(dipStart, _OPT);
			_BC_ENTRY_IP(dipEnd, _OPT);
			_BC_ENTRY_IP(dmask, _OPT);
			_BC_ENTRY_INT(dportStart, _OPT);
			_BC_ENTRY_INT(dportEnd, _OPT);
#endif
			/************Place your code here, do what you want to do! ************/
			/*remove*/
			if(findIPFilterEntrybyNameDirection(entry.filterName, DIR_IN, NULL, &index))
			{
				mib_chain_delete(MIB_IP_PORT_FILTER_TBL, index);
			}
			/************Place your code here, do what you want to do! ************/
		}
		//take effect immediately
		//setupFirewall();
		restart_IPFilter_DMZ_MACFilter();
	}
	else if(strcmp(stemp, "ad") == 0)	//add
	{
		memset(&entry, 0, sizeof(entry));

		_ENTRY_STR(filterName, _NEED);
		_ENTRY_INT(protoType, _NEED);
		//if(entry.protoType > 4){lineno = __LINE__; goto check_err;}
		_ENTRY_IP(sipStart, _OPT);
		_ENTRY_IP(sipEnd, _OPT);
		_ENTRY_IP(smask, _OPT);
		_ENTRY_IP(dipStart, _OPT);
		_ENTRY_IP(dipEnd, _OPT);
		_ENTRY_IP(dmask, _OPT);
		_ENTRY_INT(sportStart, _OPT);
		_ENTRY_INT(sportEnd, _OPT);
		
		if(entry.sportEnd == 0)
			entry.sportEnd = entry.sportStart;
		
		if(stemp && *stemp != 0)
		{
			if(entry.sportEnd < entry.sportStart){lineno = __LINE__; goto check_err;}
		}

		_ENTRY_INT(dportStart, _OPT);
		_ENTRY_INT(dportEnd, _OPT);

		if(entry.dportEnd == 0)
			entry.dportEnd = entry.dportStart;
		
		if(stemp && *stemp != 0)
		{
			if(entry.dportEnd < entry.dportStart){lineno = __LINE__; goto check_err;}
		}

/*
	//closed by jim ,we current do not support ip filter bound with interface...
		_ENTRY_BOOL(allport, _NEED);
		if(entry.allport < 0 || entry.allport > 1){lineno = __LINE__; goto check_err;}
		if(entry.allport == 0)
		{
			_ENTRY_INT(portnum, _NEED);
			if(entry.portnum < 0 || entry.portnum >= 20){lineno = __LINE__; goto check_err;}
			if(entry.portnum){FETCH_INVALID_OPT(stemp, "ifname", _NEED); stoken = stemp;}
			for(index = 0; stemp && index < entry.portnum; index++)
			{
				stemp = strchr(stoken, ';');
				if(stemp)*stemp = 0;
				strncpy(iffs[index], stoken, 20);
				stoken = stemp ? stemp + 1: NULL;
			}
		}
*/
		//jim double check the parameters validity.
		if(!entry.sipStart)
		{
			entry.smask=0;
			entry.sipEnd=0;
		}
		if(!entry.dipStart)
		{
			entry.dmask=0;
			entry.dipEnd=0;
		}
#ifdef CONFIG_IPV6
		str = boaGetVar(wp, "protoTypeV6", "");
		if (str[0]) {
			ipv6_protoType = (char)atoi(str);
		}

		if(entry.protoType == 0 && ipv6_protoType == 0) //None proto... don't care ,, all.
#else
		if(entry.protoType == 0) //None proto... don't care ,, all.
#endif
		{    //mask off port info...
			entry.sportStart=0;
			entry.sportEnd=0;
			entry.dportStart=0;
			entry.dportEnd=0;
		}
		/************Place your code here, do what you want to do! ************/
		{		// IP/Port FILTER
			MIB_CE_IP_PORT_FILTER_T filterEntry;
			// if the exist entry with the same name, abandon the add action...
			if(findIPFilterEntrybyNameDirection(entry.filterName, DIR_IN, NULL, NULL) \
				|| getTotalIPFilterNumOneDirection(DIR_IN) > MAX_INCOMING_IPFILTER_RULE_NUM)
				goto check_err;
#ifdef CONFIG_IPV6
			str = boaGetVar(wp, "IpProtocolType", "");
			filterEntry.IpProtocol = (char)atoi(str);
			// If it is a IPv6 rule. Save the protoType into  ipfilter_whitelist data base.
			// Because the parseIpFilterInfo2Entry() will clean the MIB's ipportfilter_entry.
			if ( filterEntry.IpProtocol == IPVER_IPV6 )
				entry.protoType = ipv6_protoType;
#endif
			parseIpFilterInfo2Entry(&entry, &filterEntry, DIR_IN);

#ifdef CONFIG_IPV6
			// If it is a IPv6 rule.
			if ( filterEntry.IpProtocol == IPVER_IPV6 )
			{
				str = boaGetVar(wp, "sip6Start", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.sip6Start)) {
						printf("Invalid sip6Start for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "sip6End", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.sip6End)) {
						printf("Invalid sip6End for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "dip6Start", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.dip6Start)) {
						printf("Invalid dip6Start for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "dip6End", "");
				if (str[0]) {
					if (!inet_pton(PF_INET6, str, &filterEntry.dip6End)) {
						printf("Invalid dip6End for ipport filter!");
						goto check_err;
					}
				}

				str = boaGetVar(wp, "sip6PrefixLen", "");
				filterEntry.sip6PrefixLen = (char)atoi(str);

				str = boaGetVar(wp, "dip6PrefixLen", "");
				filterEntry.dip6PrefixLen = (char)atoi(str);
			}
#endif

			ret = mib_chain_add(MIB_IP_PORT_FILTER_TBL, (unsigned char*)&filterEntry);
			if( ret == -1 )
				goto Max_Size_Reached;
			else if( ret == 0 )
				goto check_err;

			//take effect immediate
			restart_IPFilter_DMZ_MACFilter();
		}
		/************Place your code here, do what you want to do! ************/
	}
	else {lineno = __LINE__; goto check_err;}
		//Write to flash, take effect forever
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	_COND_REDIRECT;
Max_Size_Reached:
	_BC_FREE();
	_TRACE_LEAVEL;
	ERR_MSG("规则数已达最大限制!"); //Max number of rules reached!
	return;
check_err:
	_BC_FREE();
	_TRACE_LEAVEL;
	ERR_MSG("设定错误!");
	return;
}

