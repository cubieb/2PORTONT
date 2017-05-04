/*
 *      Web server handler routines for ACL stuffs
 *
 */

/*-- System inlcude files --*/
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "fmdefs.h"

#ifdef CONFIG_USER_DDNS
#define	DDNS_ADD	0
#define DDNS_MODIFY	1
/*****************************
**DDNS配置
*/
int showDNSTable(int eid, request * wp, int argc, char **argv)
{
	//启用DDNS服务:
	unsigned char	ddnsEnable = 1;	//1- 启用;  0- 禁用
	struct ddns_entryx entry = {"pro", "hostname", "INTERNET_R_0_0_32", "user", "user"};
	int				cnt = 2;
	int				index = 0;
	int				lineno = __LINE__;
	MIB_CE_DDNS_T Entry;
	int totalEntry,i;

	_TRACE_CALL;

	/************Place your code here, do what you want to do! ************/
	/************Place your code here, do what you want to do! ************/

	mib_get(MIB_DDNS_ENABLE,(void*)&ddnsEnable);

	_PUT_BOOL(ddnsEnable);

	memset(&entry,0,sizeof(entry));

	totalEntry=mib_chain_total(MIB_DDNS_TBL);

	for(i = 0; i < totalEntry; i++)
	{
		/************Place your code here, do what you want to do! ************/
		/************Place your code here, do what you want to do! ************/
		if(mib_chain_get(MIB_DDNS_TBL,i,(void*)&Entry)!=1)
			continue;

		strncpy(entry.provider,Entry.provider,10);
		strncpy(entry.username,Entry.username,35);
		strncpy(entry.hostname,Entry.hostname,35);
		strncpy(entry.ifname,Entry.ifname,32);

		boaWrite(wp, "push(new it_nr(\"%d\"" _PTS _PTS _PTS _PTS "));\n",
			index, _PME(provider), _PME(hostname), _PME(ifname), _PME(username));
	}


check_err:
	_TRACE_LEAVEL;
	return 0;
}

/*
 *	type:
 *		DDNS_ADD(0):	entry to add
 *		DDNS_MODIFY(1):	entry to modify
 *	Return value:
 *	-1	: fail
 *	0	: successful
 *	pMsg	: error message
 */
static int checkEntry(MIB_CE_DDNS_Tp pEntry, int type, char *pMsg)
{
	MIB_CE_DDNS_T tmpEntry;
	int num, i;

	num = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
	// check duplication
	for (i=0; i<num; i++) {
		mib_chain_get(MIB_DDNS_TBL, i, (void *)&tmpEntry);
		if (type == DDNS_MODIFY) { // modify
			if (pEntry->Enabled != tmpEntry.Enabled)
				continue;
		}
		if (gstrcmp(pEntry->provider, tmpEntry.provider))
			continue;
		if (gstrcmp(pEntry->hostname, tmpEntry.hostname))
			continue;
		if (gstrcmp(pEntry->interface, tmpEntry.interface))
			continue;
		if (gstrcmp(pEntry->username, tmpEntry.username))
			continue;
		if (gstrcmp(pEntry->password, tmpEntry.password))
			continue;
		if (pEntry->ServicePort != tmpEntry.ServicePort)
			continue;
		// entry duplication
		strcpy(pMsg, "条目已存在"); //Entry already exists!
		return -1;
	}
	return 0;
}

static int  getifname(char *wanif, char *wanname, unsigned int ifidx)
{
	MIB_CE_ATM_VC_T Entry;
	int total,i;

	total = mib_chain_total(MIB_ATM_VC_TBL);

	for(i=0;i<total;i++){
		if(!mib_chain_get(MIB_ATM_VC_TBL,i,&Entry))
			continue;
		//printf("\nEntry.ifindex=%d,ifidx=%d\n",Entry.ifIndex,ifidx);
		if(ifidx==Entry.ifIndex){
			ifGetName(ifidx, wanif, 10);
			getWanName(&Entry,wanname);
			printf("\nname=%s %s\n",wanif,wanname);
			return 0;
		}
	}
	return -1;
}

void formDDNS(request * wp, char *path, char *query)
{
	//启用DDNS服务:
	unsigned char	ddnsEnable = 1;	//1- 启用;  0- 禁用
	struct ddns_entryx	entry;
	char*			stemp = "";
	int				index = 0;
	int				lineno = __LINE__;
	_BC_USE;

	MIB_CE_DDNS_T Entry;
	int totalEntry,i, total;
	char tmpBuf[100];
	char wan_lan_if[IFNAMSIZ];
	char wanname[MAX_WAN_NAME_LEN];
	MIB_CE_ATM_VC_T tmpEntry;
	unsigned int ifidx;

	_TRACE_CALL;

	FETCH_INVALID_OPT(stemp, "action", _NEED);

	memset(&entry,0,sizeof(entry));
	memset(&Entry,0,sizeof(Entry));

	if(strcmp(stemp, "sw") == 0)	//switch
	{
		_GET_BOOL(ddnsEnable, _NEED);

		/************Place your code here, do what you want to do! ************/
		/************Place your code here, do what you want to do! ************/
		if(!mib_set(MIB_DDNS_ENABLE,(void*)&ddnsEnable)){
			strcpy(tmpBuf, "设置DDNS使能错误!");
			goto setErr_ddns;
		}
	}
	else if(strcmp(stemp, "rm") == 0)	//remove
	{
		_BC_INIT("bcdata");
		while(_BC_NEXT())
		{
			_BC_ENTRY_STR(provider, _NEED);
			_BC_ENTRY_STR(hostname, _NEED);
			_BC_ENTRY_STR(ifname, _NEED);
			_BC_ENTRY_STR(username, _NEED);


			/************Place your code here, do what you want to do! ************/
			/*please remove this entry from MIB */

			totalEntry=mib_chain_total(MIB_DDNS_TBL);

			for(i = 0; i < totalEntry; i++)
			{
				if(mib_chain_get(MIB_DDNS_TBL,i,(void*)&Entry)!=1)
					continue;

				if(!strcmp(entry.provider,Entry.provider)&&
					!strcmp(entry.username,Entry.username)&&
					!strcmp(entry.hostname,Entry.hostname)&&
					!strcmp(entry.ifname,Entry.ifname)){
					if(mib_chain_delete(MIB_DDNS_TBL, i) != 1) {
						strcpy(tmpBuf, strDelChainerror);
						goto setErr_ddns;
					}
					break;
				}
			}
		}
	}
	else if(strcmp(stemp, "ad") == 0)	//ad
	{
		_ENTRY_STR(provider, _NEED);
		_ENTRY_STR(hostname, _NEED);
		_ENTRY_STR(ifname, _NEED);

		/************Place your code here, do what you want to do! ************/
		strncpy(Entry.provider,entry.provider,10);
		strncpy(Entry.hostname,entry.hostname,35);

		if(!strcmp(entry.ifname,"LAN")){
				snprintf(wan_lan_if, sizeof(wan_lan_if), "%s", "br0");
				strncpy(wanname,entry.ifname,MAX_WAN_NAME_LEN);
		}
		else{
				sscanf(entry.ifname,"%u",&ifidx);

				// Mason Yu. Get E8B name
				//int tmp=getifname(wan_lan_if,wanname,ifidx);
				// Get system dev name(such as nas0_0)
				ifGetName(ifidx, wan_lan_if, sizeof(wan_lan_if));

				// Get dev name for E8B format
				total = mib_chain_total(MIB_ATM_VC_TBL);
				for(i=0; i< total; i++)
				{
					mib_chain_get(MIB_ATM_VC_TBL, i, &tmpEntry);
					if(tmpEntry.ifIndex == ifidx)
						break;
				}
				getWanName(&tmpEntry, wanname);
				//printf("\nifidx=%d,ifname=%s, ifname_e8b=%s\n",ifidx,wan_lan_if, wanname);
		}
		strncpy(Entry.interface,wan_lan_if,IFNAMSIZ);
		strncpy(Entry.ifname,wanname,MAX_WAN_NAME_LEN);

		if(!strcmp(Entry.provider,"oray")){
			FETCH_INVALID_OPT(stemp, "orayusername", _NEED);
			strncpy(Entry.username,stemp,35);
			FETCH_INVALID_OPT(stemp, "oraypassword", _NEED);
			strncpy(Entry.password,stemp,35);
		}else if(!strcmp(Entry.provider,"dyndns")){
			FETCH_INVALID_OPT(stemp, "dynusername", _NEED);
			strncpy(Entry.username,stemp,35);
			FETCH_INVALID_OPT(stemp, "dynpassword", _NEED);
			strncpy(Entry.password,stemp,35);
		}else if(!strcmp(Entry.provider,"gnudip")){
			FETCH_INVALID_OPT(stemp, "gnudipusername", _NEED);
			strncpy(Entry.username,stemp,35);
			FETCH_INVALID_OPT(stemp, "gnudippassword", _NEED);
			strncpy(Entry.password,stemp,35);
		}else{
			FETCH_INVALID_OPT(stemp, "tzoEmail", _NEED);
			strncpy(Entry.username,stemp,35);
			FETCH_INVALID_OPT(stemp, "tzoKey", _NEED);
			strncpy(Entry.password,stemp,35);
		}

		if (checkEntry(&Entry, DDNS_ADD, &tmpBuf[0]) == -1)
			goto setErr_ddns;

		Entry.Enabled=1;

		if(mib_chain_add(MIB_DDNS_TBL, (unsigned char*)&Entry) < 1){
			strcpy(tmpBuf, "添加AAAADDNS记录错误!");
			goto setErr_ddns;
		}
	}
	//else {lineno = __LINE__; goto check_err;}

//#ifdef WRITE_TO_FLASH
//	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
//#endif
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	restart_ddns();

	_COND_REDIRECT;
	return;
check_err:
	_BC_FREE();
	_TRACE_LEAVEL;
	return;
setErr_ddns:
	ERR_MSG(tmpBuf);
}

#endif
