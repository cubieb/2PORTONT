/*
 *      Web server handler routines for IP QoS
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "multilang.h"

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
#ifdef QOS_SPEED_LIMIT_SUPPORT
void mib_qos_speed_limit_check(MIB_CE_IP_QOS_Tp entry)
{
	int mibspeedindex=mib_qos_speed_limit_existed(entry->limitSpeedRank,entry->prior);
	MIB_CE_IP_QOS_SPEEDRANK_T qosSpeedEntry;
	mib_chain_get(MIB_QOS_SPEED_LIMIT,mibspeedindex,&qosSpeedEntry);
	qosSpeedEntry.count--;
	if(qosSpeedEntry.count==0)
		{
		printf("delete the entry:speed=%d,prior=%d\n",entry->limitSpeedRank,entry->prior);
		mib_chain_delete(MIB_QOS_SPEED_LIMIT,mibspeedindex);
		}
	else
		mib_chain_update(MIB_QOS_SPEED_LIMIT,&qosSpeedEntry,mibspeedindex);
}
#endif
/////////////////////////////////////////////////////////////////////////////
// Jenny, for checking duplicated IP QoS rule
int checkRule_ipqos(MIB_CE_IP_QOS_T qosEntry, unsigned char *sip, unsigned char *dip)
{
	int totalEntry, i;
	MIB_CE_IP_QOS_T Entry;
	char *temp;
	long nSip, nDip;

	totalEntry = mib_chain_total(MIB_IP_QOS_TBL);

	temp = inet_ntoa(*((struct in_addr *)sip));
	nSip = ntohl(inet_addr(temp));
	temp = inet_ntoa(*((struct in_addr *)dip));
	nDip = ntohl(inet_addr(temp));
	if (nSip == nDip && nSip != 0x0)
		return 0;

	for (i=0; i<totalEntry; i++) {
		unsigned long v1, v2, pSip, pDip;
		int m;

		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry))
			return 0;
		temp[0] = '\0';
		temp = inet_ntoa(*((struct in_addr *)Entry.sip));
		v1 = ntohl(inet_addr(temp));
		v2 = 0xFFFFFFFFL;
		for (m=32; m>Entry.smaskbit; m--) {
			v2 <<= 1;
			v2 |= 0x80000000;
		}
		pSip = v1&v2;
		temp[0] = '\0';
		temp = inet_ntoa(*((struct in_addr *)Entry.dip));
		v1 = ntohl(inet_addr(temp));
		v2 = 0xFFFFFFFFL;
		for (m=32; m>Entry.dmaskbit; m--) {
			v2 <<= 1;
			v2 |= 0x80000000;
		}
		pDip = v1&v2;

		// If all parameters of Entry are all the same as mew rule, drop this new rule.
#ifdef QOS_DIFFSERV
		if (Entry.enDiffserv == qosEntry.enDiffserv
			&& Entry.m_ipprio == qosEntry.m_ipprio && Entry.m_iptos == qosEntry.m_iptos)
			return 2;
#endif
		if (nSip == pSip && nDip == pDip &&
			Entry.sPort == qosEntry.sPort && Entry.dPort == qosEntry.dPort &&
			Entry.protoType == qosEntry.protoType && Entry.phyPort == qosEntry.phyPort
//#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
			&& Entry.qosDscp == qosEntry.qosDscp
#endif
			/* && Entry.vlan1p == qosEntry.vlan1p*/
//#endif
#ifdef QOS_DIFFSERV
			&& Entry.enDiffserv == qosEntry.enDiffserv
			&& Entry.m_ipprio == qosEntry.m_ipprio && Entry.m_iptos == qosEntry.m_iptos
			&& Entry.limitSpeed == qosEntry.limitSpeed && Entry.policing == qosEntry.policing
#endif
			)
				return 0;
	}
	return 1;
}

#ifdef NEW_IP_QOS_SUPPORT
#define PRINT_TRAFFICTL_RULE(pEntry) \
    printf("[TRAFFIC CONTROL]: entryid:%d, ifIndex:%d, srcip:%s, smaskbits:%d, dstip:%s, dmaskbits:%d,"  \
	   "sport:%d, dport%d, protoType:%d, limitspeed:%d\n",                                       \
	    pEntry->entryid, pEntry->ifIndex, inet_ntoa(*((struct in_addr*)&pEntry->srcip)),        \
	    pEntry->smaskbits, inet_ntoa(*((struct in_addr*)&pEntry->dstip)), pEntry->dmaskbits,    \
	    pEntry->sport, pEntry->dport, pEntry->protoType, pEntry->limitSpeed);

static int getWanInfName(MIB_CE_ATM_VC_T *pEntry, char* name)
{
	if(pEntry==NULL || name==NULL)
				return 0;
#ifdef _CWMP_MIB_
	if(pEntry->WanName[0])
		strcpy(name, pEntry->WanName);
	else
#endif
	{//if not set by ACS. then generate automaticly.
		generateWanInfName(pEntry, name);
	}
	return 1;
}

char * getWanNameFromIfIndex(unsigned int ifIndex, char *wanname)
{
	MIB_CE_ATM_VC_T entry;
	int i, entryNum;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry))
			continue;
		if (entry.ifIndex == ifIndex)
			break;
	}
	if (i>=entryNum) {
		printf("not find matched interface!\n");
		return NULL;
	}

	getWanInfName(&entry, wanname);

	return wanname;
}

//ql: 20081117 add traffic shaping
int initTraffictlPage(int eid, request * wp, int argc, char **argv)
{
	MIB_CE_IP_TC_T entry;
	int entryNum = 0, i=0, nBytes = 0;
	char ifname[MAX_WAN_NAME_LEN];
	char sip[20], dip[20], *p = NULL;
	unsigned int total_bandwidth = 0;
	unsigned char totalBandWidthEn = 0;

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);

	mib_get(MIB_TOTAL_BANDWIDTH, &total_bandwidth);
	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, &totalBandWidthEn);

	nBytes += boaWrite(wp, "totalBandWidthEn=%d;\n", totalBandWidthEn);
	if (totalBandWidthEn) {
		nBytes += boaWrite(wp, "totalBandwidth=%u;\n", total_bandwidth);
	} else {
		nBytes += boaWrite(wp, "totalBandwidth=0;\n");
	}

    for(;i<entryNum; i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
		    continue;
		strncpy(sip, inet_ntoa(*((struct in_addr*)&entry.srcip)), INET_ADDRSTRLEN);
		strncpy(dip, inet_ntoa(*((struct in_addr*)&entry.dstip)), INET_ADDRSTRLEN);
		if(entry.srcip && entry.smaskbits)
		{
		    p = sip + strlen(sip);
		    snprintf(p,sizeof(sip)-strlen(sip), "/%d", entry.smaskbits );
		}
		if(entry.dstip && entry.dmaskbits)
		{
		    p = dip + strlen(dip);
		    snprintf(p,sizeof(dip)-strlen(dip), "/%d", entry.dmaskbits );
		}

		if (getWanNameFromIfIndex(entry.ifIndex, ifname)==NULL) {
			printf("fatal error! didn't find such interface\n");
			continue;
		}

		nBytes += boaWrite(wp, "traffictlRules.push(new it_nr(%d, \"%s\", %d, %d, %d, \"%s\", \"%s\", %d));\n",
			      entry.entryid, ifname, entry.protoType,
			      entry.sport, entry.dport, sip, dip, entry.limitSpeed);
	}
    return nBytes;
}

#define TRAFFICTL_RULE_NUM_MAX 256
#define DELIM      '&'
#define SUBDELIM   '|'
#define SUBDELIM1  "|"

//Used to get subnet mask bit number
static int getNetMaskBit(char* netmask)
{
    unsigned int bits = 0, mask = 0;
    int i=0, flag = 0;

    if(!netmask||strlen(netmask)>15)
	return 0;
    mask = inet_network(netmask);
    for(;i<32;i++)
    {
	if(mask&(0x80000000>>i)) {
	    if(flag)
		return 0;
	    else
		bits++;
	}
	else {
	    flag = 1;
	}
    }
    return bits;
}

static int parseArgs(char* action, MIB_CE_IP_TC_Tp pEntry)
{
    char* p = NULL, *tmp = NULL;
    int i = 0;

	//ifIndex
	tmp = strstr(action, "inf=");
	tmp += strlen("inf=");
	if (!tmp || *tmp==DELIM)
		return 1;
	else {
		pEntry->ifIndex = strtol(tmp, &p,0);
		if (*p != DELIM)
			return 1;
	}

    //protocol
    tmp =strstr(action, "proto=");
    tmp += strlen("proto=");
    if(!tmp||*tmp == DELIM)//not set protocol, set it to default,none
		pEntry->protoType = 0;
    else
    {
		pEntry->protoType = strtol(tmp, &p, 0);
		if(*p != DELIM)
	    	return 1;
    }

    //source ip
    tmp = strstr(action, "srcip=");
    tmp += strlen("srcip=");
    if(!tmp||*tmp == DELIM)//noet set, set default
		memset(pEntry->srcip, 0, IP_ADDR_LEN);
    else
    {
		char sip[16]={0};
		struct in_addr addr;
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15)
			return 1;
		strncpy(sip, tmp, p-tmp);
		if(!inet_aton(sip, &addr))
			return 1;
		memcpy(pEntry->srcip, &addr.s_addr, IP_ADDR_LEN);
    }

    //source ip address netmask
    tmp = strstr(action, "srcnetmask=");
    tmp += strlen("srcnetmask=");
    if(!tmp||*tmp==DELIM)
		pEntry->smaskbits = 0;
    else
    {
		char smask[16]={0};
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15) return 1;
		strncpy(smask, tmp, p-tmp);
		pEntry->smaskbits = getNetMaskBit(smask);
    }

    //destination ip
    tmp = strstr(action, "dstip=");
    tmp += strlen("dstip=");
    if(!tmp||*tmp == DELIM)//noet set, set default
		memset(pEntry->dstip, 0, IP_ADDR_LEN);
    else
    {
		char dip[16]={0};
		struct in_addr addr;
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15)
			return 1;
		strncpy(dip, tmp, p-tmp);
		if(!inet_aton(dip, &addr))
			return 1;
		memcpy(pEntry->dstip, &addr.s_addr, IP_ADDR_LEN);
    }

    //destination ip address netmask
    tmp = strstr(action, "dstnetmask=");
    tmp += strlen("dstnetmask=");
    if(!tmp||*tmp==DELIM)
		pEntry->dmaskbits = 0;
    else
    {
		char dmask[16]={0};
		p = strchr(tmp, DELIM);
		if(p&&p-tmp>15)
			return 1;
		strncpy(dmask, tmp, p-tmp);
		pEntry->dmaskbits = getNetMaskBit(dmask);

    }

    //source port
    tmp = strstr(action, "sport=");
    tmp += strlen("sport=");
    if(!tmp||*tmp==DELIM)
		pEntry->sport = 0;
    else
    {
		pEntry->sport = strtol(tmp, &p, 0);
		if(*p != DELIM)
			return 1;
    }

    //destination port
    tmp = strstr(action, "dport=");
    tmp += strlen("dport=");
    if(!tmp||*tmp==DELIM)
		pEntry->dport = 0;
    else
    {
		pEntry->dport = strtol(tmp, &p, 0);
		if(*p != DELIM)
	return 1;
}

    //upstream rate
    tmp = strstr(action, "uprate=");
    tmp += strlen("uprate=");
    if(!tmp||*tmp=='\0')
		pEntry->limitSpeed = 0;
    else
    {
		pEntry->limitSpeed = strtol(tmp, &p, 0);
		if(*p != '\0')
			return 1;
    }

    return 0;
}

void formQosShape(request * wp, char *path, char *query)
{
	char *action = NULL, *url = NULL;
	char *act1="applybandwidth";
	char *act2="applysetting";
	char *act3="addsetting";
	int entryNum = 0;

	entryNum = mib_chain_total(MIB_IP_QOS_TC_TBL);
	action = boaGetVar(wp, "lst", "");

	if(action[0])
	{
		if( !strncmp(action, act1, strlen(act1)) )
		{//set total bandwidth
			unsigned int totalbandwidth = 0;
			unsigned char totalbandwidthEn=0;
			char* strbandwidth = NULL;
			strbandwidth = strstr(action, "bandwidth=");
			strbandwidth += strlen("bandwidth=");
			if(strbandwidth)//found it
			{
				totalbandwidth = strtoul(strbandwidth, NULL, 0);

				if (totalbandwidth) {
					totalbandwidthEn = 1;
				} else {
					totalbandwidthEn = 0;
				}
			}
			else {
				totalbandwidthEn = 0;
				totalbandwidth = 0;
			}

			mib_set(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalbandwidthEn);
			mib_set(MIB_TOTAL_BANDWIDTH, &totalbandwidth);

			//take effect
			take_qos_effect();
		}
		else if( !strncmp(action, act2, strlen(act2)) )
		{//delete some
			int idlst[TRAFFICTL_RULE_NUM_MAX+1] = {0};
			char stridlst[256],err_msg[256], *p = NULL;
			MIB_CE_IP_TC_T entry;
			int  i=0, index=1;

			p = strstr(action, "id=");
			p += strlen("id=");
			if(*p == '\0') {//delete none
				goto done;
			}

			stridlst[0] = '\0';
			strncpy(stridlst, p, 256);

			//convert the id list, store them in idlst,
			//you can delete most 10 rules at one time
			p = strtok(stridlst, SUBDELIM1);
			if(p) index = atoi(p);
			if(index>0&&index<=TRAFFICTL_RULE_NUM_MAX) idlst[index]=1;
			while((p = strtok(NULL, SUBDELIM1)) != NULL)
			{
				index = atoi(p);
				idlst[index]=1;
				if(index > TRAFFICTL_RULE_NUM_MAX )
				    break;
			}

			for(i=entryNum-1; i>=0; i--)
			{
				if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
				    continue;

				if( 1 == idlst[entry.entryid]) //delete it
				{
		            //delete rules of  tc and iptables
				    if(1 != mib_chain_delete(MIB_IP_QOS_TC_TBL, i)) {
						snprintf(err_msg, 256, multilang(LANG_ERROR_HAPPENED_WHEN_DELETING_RULE_D), entry.entryid);
						ERR_MSG(err_msg);
						return;
				    }
				}
			}

done:
			take_qos_effect();
		}
		else if ( !strncmp(action, act3, strlen(act3)) )
		{
			MIB_CE_IP_TC_T entry;
			unsigned char map[TRAFFICTL_RULE_NUM_MAX+1]={0};
			int entryid = 1;
			int i = 0;

			if(entryNum>=TRAFFICTL_RULE_NUM_MAX)
			{
				ERR_MSG(multilang(LANG_TRAFFIC_CONTROLLING_QUEUE_IS_FULL_YOU_MUST_DELETE_SOME_ONE));
				return;
			}

			//allocate a free rule id for new entry
			for(;i<entryNum;i++)
			{
				if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry))
				    continue;

				map[entry.entryid] = 1;
			}
			for(i=1;i<=TRAFFICTL_RULE_NUM_MAX;i++)
			{
				if(!map[i])
				{
					entryid = i;
					break;
				}
			}

			memset(&entry, 0, sizeof(MIB_CE_IP_TC_T));
			entry.entryid = entryid;

			if(parseArgs(action, &entry))
			{//some arguments are wrong
				ERR_MSG(multilang(LANG_WRONG_SETTING_IS_FOUND));
				return;
			}

			PRINT_TRAFFICTL_RULE((&entry));

			if(mib_chain_add(MIB_IP_QOS_TC_TBL, &entry) < 1)
			{//adding mib setting is wrong
				ERR_MSG(Tadd_chain_error);
				return;
			}
		}
	}
	//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	//well, go back
	url = boaGetVar(wp, "submit-url", "");
	if(url[0])
		boaRedirect(wp, url);

	return;
}
#endif

void formQos(request * wp, char *path, char *query)
{
	char *submitUrl, *strAddQos, *strDelQos, *strVal, *strDelAllQos;
	char *strSAddr, *strDAddr, *strSport, *strDport;
	char tmpBuf[100];
	int intVal;
	unsigned int entryNum, totalEntry, i;
	MIB_CE_IP_QOS_T entry;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3, mask, mbit;
	unsigned char vChar, vChar_mode;
	int itfid, itfdomain;
#ifndef NO_ACTION
	int pid;
#endif
#ifdef CONFIG_8021P_PRIO
	char *strSet1p,*strSet1p_prio;
	const char *nset1ptbl;
#ifdef NEW_IP_QOS_SUPPORT
	char *strSetPred, *strSetPred_prio;
	const char *nsetPredtbl;
#endif
	unsigned char m;
	unsigned char value[8];
#endif
	int queueIndex;
	MIB_CE_IP_QOS_QUEUE_T Entry_queue;
	unsigned char mode=0;
	mib_get(MIB_MPMODE, (void *)&mode);

#ifdef IP_QOS//ql 20081117 START when NEW_IP_QOS_SUPPORT enable, dont need to stop IP QoS firstly.
	if (mode&MP_IPQ_MASK) stopIPQ();
#endif
	strVal = boaGetVar(wp, "admin", "");

	// enable/disable Qos
	if (strVal[0]) {
		// bitmap for virtual lan port function
		// Port Mapping: bit-0
		// QoS : bit-1

		//ql 20081119 START get qos policy
#ifdef NEW_IP_QOS_SUPPORT
		unsigned char policy;
		strVal = boaGetVar(wp, "qosPolicy", "");
		if (strVal[0] == '1')
			policy = 1;
		else
			policy = 0;
		mib_set(MIB_QOS_POLICY, (void *)&policy);
#endif

		mib_get(MIB_MPMODE, (void *)&vChar_mode);
		strVal = boaGetVar(wp, "qosen", "");
		if ( strVal[0] == '1' ) {
			vChar_mode |= MP_IPQ_MASK;
#ifdef QOS_DIFFSERV
			vChar = 0;
			mib_set(MIB_QOS_DIFFSERV, (void *)&vChar);
#endif
		}
		else
			vChar_mode &= ~MP_IPQ_MASK;
		mib_set(MIB_MPMODE, (void *)&vChar_mode);

#ifndef CONFIG_RTL8672NIC
		if ( strVal[0] == '1' ) {
			setupIPQoSflag(1);
		}
		else
			setupIPQoSflag(0);
#endif

//if define re8305 cannot user 802.1p prio
#ifndef CONFIG_RE8305
		if ( strVal[0] == '1' ) {
			strVal = boaGetVar(wp, "qosdmn", "");
			vChar = strVal[0] - '0';
			mib_set(MIB_QOS_DOMAIN, (void *)&vChar);
		}
#endif
	//Kevin, check does it need to enable/disable IP fastpath status
    UpdateIpFastpathStatus();
#if defined(APPLY_CHANGE)
#ifdef IP_QOS
		if (vChar_mode&MP_IPQ_MASK)
			setupIPQ();
#else
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
#endif
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
		submitUrl = boaGetVar(wp, "submit-url", "");
		OK_MSG(submitUrl);
		return;
	}
#ifdef CONFIG_8021P_PRIO
	strSet1p= boaGetVar(wp, "set1p", "");
	if (strSet1p[0]){
		if(mib_get(MIB_8021P_PRIO, (void *)value)== 0)
		{
			strcpy(tmpBuf, Tget_mib_error);
			goto setErr_qos;
		}

		for(i=0;i<IPQOS_NUM_PKT_PRIO;i++)
		{
			nset1ptbl =set1ptable[i];
		  strSet1p_prio=boaGetVar(wp, nset1ptbl, "");
		  if(strSet1p_prio[0]){
		  vChar = strSet1p_prio[0] - '0';
			value[i]=vChar;
		  }
		}
		mib_set(MIB_8021P_PRIO, (void *)value);
#if defined(APPLY_CHANGE)
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
		submitUrl = boaGetVar(wp, "submit-url", "");
		OK_MSG(submitUrl);
		return;
	}

#ifdef NEW_IP_QOS_SUPPORT
	strSetPred = boaGetVar(wp, "setpred", "");
	if (strSetPred[0]){
		if (mib_get(MIB_PRED_PRIO, (void *)value) == 0)
		{
			strcpy(tmpBuf, Tget_mib_error);
			goto setErr_qos;
		}

		for (i=0; i<IPQOS_NUM_PKT_PRIO; i++)
		{
			nsetPredtbl = setpredtable[i];
			strSetPred_prio = boaGetVar(wp, nsetPredtbl, "");
			if (strSetPred_prio[0]){
				vChar = strSetPred_prio[0] - '0';
				value[i] = vChar;
			}
		}
		mib_set(MIB_PRED_PRIO, (void *)value);
#if defined(APPLY_CHANGE)
#ifdef NEW_IP_QOS_SUPPORT
		take_qos_effect();
#endif
#endif
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
		submitUrl = boaGetVar(wp, "submit-url", "");
		OK_MSG(submitUrl);
		return;
	}
#endif
#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
	char *bandwidthctrl=boaGetVar(wp,"bandwidthctrl","");
	if(bandwidthctrl[0]){
		char * upbandwidth=boaGetVar(wp,"upbandwidth","");
		printf("upbandwidth=%s\n",upbandwidth);
		if ( !string_to_dec(upbandwidth, &intVal) ) {
			strcpy(tmpBuf, Tinvalid_pvc_bandwidth);
			goto setErr_qos;
		}
		unsigned short shortVal=(unsigned short)intVal;
		if(!mib_set(MIB_PVC_TOTAL_BANDWIDTH,&shortVal))
			goto setErr_qos;
		else
			goto setOk_qos;
	}
#endif
	strAddQos = boaGetVar(wp, "addqos", "");
	strDelQos = boaGetVar(wp, "delSel", "");
	strDelAllQos = boaGetVar(wp, "delAll", "");

	memset(&entry, 0, sizeof(entry));
	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */

	/* Add new qos entry */
	if (strAddQos[0]) {
		int intVal;
		unsigned char smask[4], dmask[4], sip[4], dip[4];
		memset(sip, 0x0, 4);
		memset(dip, 0x0, 4);
		if (totalEntry >= MAX_QOS_RULE)
		{
			strcpy(tmpBuf, Texceed_max_rules);
			goto setErr_qos;
		}

#ifdef _CWMP_MIB_
		// enable
		entry.enable = 1;
#endif
#ifdef NEW_IP_QOS_SUPPORT
		//strVal = boaGetVar(wp, "s_m1p", "");
		//entry.vlan1p = strVal[0] - '0';
		entry.vlan1p = 0;

#ifdef QOS_DSCP_MATCH
		strVal = boaGetVar(wp, "s_dscp", "");
		if (strVal[0]) {
			entry.qosDscp = strtol(strVal, (char**)NULL, 0);
		} else
#endif
			entry.qosDscp = 0;
#endif
		// protocol type
		strVal = boaGetVar(wp, "prot", "");
		entry.protoType = strVal[0] - '0';

		strSAddr = boaGetVar(wp, "sip", "");
		strDAddr = boaGetVar(wp, "dip", "");

		// Source address
		if (strSAddr[0]) {
			inet_aton(strSAddr, (struct in_addr *)&entry.sip);
			if ((entry.sip[0]=='\0') &&(entry.sip[1]=='\0') && (entry.sip[2]=='\0') && (entry.sip[3]=='\0')) {
				strcpy(tmpBuf, Tinvalid_source_ip);
				goto setErr_qos;
			}

			strSAddr = boaGetVar(wp, "smask", "");
			if (strSAddr[0]) {
				if (!isValidNetmask(strSAddr, 1)) {
					strcpy(tmpBuf, Tinvalid_source_netmask);
					goto setErr_qos;
				}
				inet_aton(strSAddr, (struct in_addr *)smask);
				inet_aton(strSAddr, (struct in_addr *)&mask);
				if (mask==0) {
					strcpy(tmpBuf, Tinvalid_source_netmask);
					goto setErr_qos;
				}
				mask = htonl(mask);
				mbit=0; intVal=0;
				for (i=0; i<32; i++) {
					if (mask&0x80000000) {
						if (intVal) {
							strcpy(tmpBuf, Tinvalid_source_netmask);
							goto setErr_qos;
						}
						mbit++;
					}
					else
						intVal=1;
					mask <<= 1;
				}
				entry.smaskbit = mbit;
			}
			else {
				entry.smaskbit = 32;
				inet_aton(ARG_255x4, (struct in_addr *)smask);
			}

			// Jenny, for checking duplicated source address
			sip[0] = entry.sip[0] & smask[0];
			sip[1] = entry.sip[1] & smask[1];
			sip[2] = entry.sip[2] & smask[2];
			sip[3] = entry.sip[3] & smask[3];
		}

		// source port
		if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
			strSport = boaGetVar(wp, "sport", "");

			if (strSport[0]) {
				if ( !string_to_dec(strSport, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, Tinvalid_source_port);
					goto setErr_qos;
				}
				entry.sPort = (unsigned short)intVal;
			}
		}

		// Destination address
		if (strDAddr[0]) {
			inet_aton(strDAddr, (struct in_addr *)&entry.dip);
			if ((entry.dip[0]=='\0') && (entry.dip[1]=='\0') && (entry.dip[2]=='\0') && (entry.dip[3]=='\0')) {
				strcpy(tmpBuf, Tinvalid_destination_ip);
				goto setErr_qos;
			}

			strDAddr = boaGetVar(wp, "dmask", "");
			if (strDAddr[0]) {
				if (!isValidNetmask(strDAddr, 1)) {
					strcpy(tmpBuf, Tinvalid_destination_netmask);
					goto setErr_qos;
				}
				inet_aton(strDAddr, (struct in_addr *)dmask);
				inet_aton(strDAddr, (struct in_addr *)&mask);
				if (mask==0) {
					strcpy(tmpBuf, Tinvalid_destination_netmask);
					goto setErr_qos;
				}
				mask = htonl(mask);
				mbit=0; intVal=0;
				for (i=0; i<32; i++) {
					if (mask&0x80000000) {
						if (intVal) {
							strcpy(tmpBuf, Tinvalid_destination_netmask);
							goto setErr_qos;
						}
						mbit++;
					}
					else
						intVal=1;
					mask <<= 1;
				}
				entry.dmaskbit = mbit;
			}
			else {
				entry.dmaskbit = 32;
				inet_aton(ARG_255x4, (struct in_addr *)dmask);
			}

			// Jenny, for checking duplicated destination address
			dip[0] = entry.dip[0] & dmask[0];
			dip[1] = entry.dip[1] & dmask[1];
			dip[2] = entry.dip[2] & dmask[2];
			dip[3] = entry.dip[3] & dmask[3];
		}

		// destination port
		if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
			strDport = boaGetVar(wp, "dport", "");

			if (strDport[0]) {
				if ( !string_to_dec(strDport, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, Tinvalid_destination_port);
					goto setErr_qos;
				}
				entry.dPort = (unsigned short)intVal;
			}
		}

		// physical port
		strVal = boaGetVar(wp, "phyport", "");
		entry.phyPort = 0xff;
		itfid = atoi(strVal);
		if (itfid != 0) {
			itfdomain = IF_DOMAIN(itfid);
			itfid = itfid&0x0ff;
			if (itfdomain == DOMAIN_ELAN)
				entry.phyPort = itfid;
#ifdef WLAN_SUPPORT
			else if (itfdomain == DOMAIN_WLAN)
#ifdef WLAN_MBSSID
			{
				entry.phyPort=5+itfid;
			}
#else
				entry.phyPort = 5;	// wlan0
#endif
#endif
#ifdef CONFIG_USB_ETH
			else if (itfdomain == DOMAIN_ULAN)
				entry.phyPort = IFUSBETH_PHYNUM;	// usb0
#endif //CONFIG_USB_ETH
		}

		// outbound interface
		strVal = boaGetVar(wp, "out_if", "");
		if (strVal[0]) {
			entry.outif = (unsigned int)atoi(strVal);
		}

		/*
		 *	Assign/Mark priority
		 */
		#if 0
		// outbound priority
		strVal = boaGetVar(wp, "prio", "");
		entry.prior = (unsigned char)(strVal[0]-'0');
		#else
		strVal = boaGetVar(wp, "queuekey", "");
		if (strVal[0]) {
			queueIndex = (unsigned char)atoi(strVal);

			if ( queueIndex != 255 ) {
				if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, queueIndex, (void *)&Entry_queue))
				{
					boaError(wp, 400, "Get chain record error!\n");
					return;
				}

				// outbound interface
				entry.outif = Entry_queue.outif;

				// outbound priority
				entry.prior =  Entry_queue.prior;
			}

		}
		#endif

#ifdef QOS_DSCP
		// DSCP enable
		strVal = boaGetVar(wp, "dscpenable", "");
		entry.dscp = (unsigned char)(strVal[0]-'0');
		if(entry.dscp == 0) {
#endif

		// IP precedence
		strVal = boaGetVar(wp, "ipprio", "");
		entry.m_ipprio = (unsigned char)(strVal[0]-'0');

		// IP Type of Service
		strVal = boaGetVar(wp, "tos", "");
		entry.m_iptos = atoi(strVal);

#ifdef QOS_DSCP
		}
		else if (entry.dscp == 1) {
			int dscp;
			// DSCP
			strVal = boaGetVar(wp, "dscp", "");
			dscp = atoi(strVal);
#ifdef NEW_IP_QOS_SUPPORT
			entry.m_ipprio = 0;
			entry.m_iptos = 255;
			entry.m_dscp = dscp;
#else
			entry.m_ipprio = dscp >> 3;
			entry.m_iptos = (dscp & 0x07) >> 1;
#endif
		}
#endif

		// 802.1p
		strVal = boaGetVar(wp, "m1p", "");
		entry.m_1p = (unsigned char)(strVal[0]-'0');

		// Check if this qos entry exists
		// ...
		#ifdef QOS_SPEED_LIMIT_SUPPORT
		//check whether limit speed

		strVal = boaGetVar(wp,"qosspeedenable","");
		if(!strcmp(strVal,"on"))
		{
			entry.limitSpeedEnabled=1;
			printf("entry.limitSpeedEnabled=%d\n",entry.limitSpeedEnabled);
		if(entry.limitSpeedEnabled)
		{
			//get limit speed rank
			strVal = boaGetVar(wp,"speedLimitRank","");
			if ( !string_to_dec(strVal, &intVal) || intVal<1 || intVal>1024) {
				strcpy(tmpBuf, Tinvalid_speed);
				goto setErr_qos;
			}
			entry.limitSpeedRank= (unsigned char)intVal;
			MIB_CE_IP_QOS_SPEEDRANK_T qosSpeedMib;
			qosSpeedMib.speed=(unsigned char)intVal;
			qosSpeedMib.prior=entry.prior;
			int mibspeedindex=mib_qos_speed_limit_existed(qosSpeedMib.speed,qosSpeedMib.prior);
			if(mibspeedindex==-1)//not existed
			{
				qosSpeedMib.count=1;
				qosSpeedMib.index=mib_chain_total(MIB_QOS_SPEED_LIMIT);
				intVal = mib_chain_add(MIB_QOS_SPEED_LIMIT, (unsigned char*)&qosSpeedMib);
				if (intVal == 0) {
					strcpy(tmpBuf, Tadd_chain_error);
					printf("chain_add MIB_QOS_SPEED_LIMIT error!\n");
					goto setErr_qos;
				}
				else if (intVal == -1) {
					strcpy(tmpBuf, strTableFull);
					printf("chain_add MIB_QOS_SPEED_LIMIT table full!\n");
					goto setErr_qos;
				}
			}
			else//existed
				{
					mib_chain_get(MIB_QOS_SPEED_LIMIT,mibspeedindex,&qosSpeedMib);
					qosSpeedMib.count++;
					mib_chain_update(MIB_QOS_SPEED_LIMIT,&qosSpeedMib,mibspeedindex);
				}
			}
		}
		else entry.limitSpeedEnabled=0;
		#endif

		if (!checkRule_ipqos(entry, sip, dip)) {	// Jenny
			strcpy(tmpBuf, Tinvalid_rule);
			goto setErr_qos;
		}

		intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, Tadd_chain_error);
			goto setErr_qos;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_qos;
		}
	}

	/* Delete entry */
	if (strDelQos[0])
	{
		unsigned int i;
		unsigned int idx;
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "s%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if (strVal[0] == '1') {
				deleted ++;
#ifdef QOS_SPEED_LIMIT_SUPPORT
				MIB_CE_IP_QOS_T qosEntry;
				mib_chain_get(MIB_IP_QOS_TBL,idx,&qosEntry);
				mib_qos_speed_limit_check(&qosEntry);
#endif
				if(mib_chain_delete(MIB_IP_QOS_TBL, idx) != 1) {
					strcpy(tmpBuf, Tdelete_chain_error);
					goto setErr_qos;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, multilang(LANG_THERE_IS_NO_ITEM_SELECTED_TO_DELETE));
			goto setErr_qos;
		}

		goto setOk_qos;
	}
	/* Delete all entry */
	if ( strDelAllQos[0]) {
		mib_chain_clear(MIB_IP_QOS_TBL); /* clear chain record */
		#ifdef QOS_SPEED_LIMIT_SUPPORT
		mib_chain_clear(MIB_QOS_SPEED_LIMIT); /* clear chain record */
		#endif
		goto setOk_qos;
	}

setOk_qos:
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

#if defined(APPLY_CHANGE)
	if (mode&MP_IPQ_MASK) {
#ifdef IP_QOS
		//stopIPQ();
		setupIPQ();
#elif defined(NEW_IP_QOS_SUPPORT)
		take_qos_effect();
#endif
	}
#endif

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

setErr_qos:
	ERR_MSG(tmpBuf);
}
#endif	// of IP_QOS

// List all the available LAN side interface at web page.
// return: number of interface listed.
int iflanList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	int ifnum=0;
	unsigned int i;
	char *name;
	struct itfInfo itfs[16];
	int type;

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	type = 0;
	if ( !strcmp(name, "all") )
#ifdef CONFIG_USB_ETH
		type = (DOMAIN_ELAN|DOMAIN_WLAN|DOMAIN_ULAN);
#else
		type = (DOMAIN_ELAN|DOMAIN_WLAN);
#endif //CONFIG_USB_ETH
	else if ( !strcmp(name, "eth") )
		type = DOMAIN_ELAN;	// eth interface
	else if ( !strcmp(name, "wlan") )
		type = DOMAIN_WLAN;	// wireless interface
#ifdef CONFIG_USB_ETH
	else if ( !strcmp(name, "usb") )
		type = DOMAIN_ULAN;	// usb eth interface
#endif //CONFIG_USB_ETH
	else
		type = DOMAIN_ELAN;	// default to eth

	ifnum = get_domain_ifinfo(itfs, 16, type);
	if (ifnum==0)
		return 0;
	for (i=0; i<ifnum; i++) {
		nBytesSent += boaWrite(wp, "<option value=%u>%s</option>\n",
		IF_ID(itfs[i].ifdomain, itfs[i].ifid), itfs[i].name);
	}
	return nBytesSent;
}

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
/////////////////////////////////////////////////////////////////////////////
int default_qos(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
//alex
#ifdef CONFIG_RE8305
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += boaWrite(wp, "<td><font size=2>Default QoS:&nbsp;</td>\n");
	nBytesSent += boaWrite(wp, "<td><select name=qosdmn style=\"width:100px \">\n");
#else
	nBytesSent += boaWrite(wp, "<td><font size=2><b>Default QoS:&nbsp;</b></td>\n");
	nBytesSent += boaWrite(wp, "<td><select name=qosdmn>\n");
#endif
	nBytesSent += boaWrite(wp, "<option value=%d>IP Pred</option>"
			"</select>\n</td>\n", PRIO_IP);
	return nBytesSent;
#endif

#ifdef CONFIG_8021P_PRIO
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += boaWrite(wp, "<td><font size=2>Default QoS:&nbsp;&nbsp;</td>\n");
//	nBytesSent += boaWrite(wp, "<td><select name=qosdmn>\n");
	nBytesSent += boaWrite(wp, "<td><select name=qosdmn style=\"width:100px \" onClick=\"return enable8021psetting()\">\n");
#else
	nBytesSent += boaWrite(wp, "<td><font size=2><b>Default QoS:&nbsp;&nbsp;</b></td>\n");
//	nBytesSent += boaWrite(wp, "<td><select name=qosdmn>\n");
	nBytesSent += boaWrite(wp, "<td><select name=qosdmn onClick=\"return enable8021psetting()\">\n");
#endif
	nBytesSent += boaWrite(wp, "<option value=%d>IP Pred</option>\n",PRIO_IP);
	nBytesSent += boaWrite(wp, "<option value=%d>802.1p</option>\n",PRIO_802_1p);
	nBytesSent +=  boaWrite(wp,"</select>\n</td>\n");

	return nBytesSent;
#else
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += boaWrite(wp, "<td><font size=2>Default QoS:&nbsp;</td>\n");
	nBytesSent += boaWrite(wp, "<td><select name=qosdmn style=\"width:100px \">\n");
#else
	nBytesSent += boaWrite(wp, "<td><font size=2><b>Default QoS:&nbsp;</b></td>\n");
	nBytesSent += boaWrite(wp, "<td><select name=qosdmn>\n");
#endif
	nBytesSent += boaWrite(wp, "<option value=%d>IP Pred</option>"
			"<option value=%d>802.1p</option>\n</select>\n</td>\n", PRIO_IP, PRIO_802_1p);
	return nBytesSent;
#endif
}

/////////////////////////////////////////////////////////////////////////////
int qosList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i, k;
	MIB_CE_IP_QOS_T Entry;
	const char *type;
	char	*psip, *pdip, sip[20], dip[20];
	char sport[10], dport[10];
	const char *pPrio, *pIPrio, *pTos, *p1p;
#ifdef QOS_DSCP
	char pDscp[5];
#endif
#ifdef NEW_IP_QOS_SUPPORT
	char qosDscp[5], vlan1p[5];
#endif
	char pItf[16];
	MIB_CE_ATM_VC_T  vc_entry;
	unsigned int mask, smask, dmask;
	char strPhy[]="LAN0", *pPhy;
	char wanif[IFNAMSIZ];
	//char  buffer[3];

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	nBytesSent += boaWrite(wp, "<tr>"
#ifdef _CWMP_MIB_
	"<th bgcolor=\"#808080\" colspan=2><font size=\"2\"></font></th>\n"
#else
	"<th bgcolor=\"#808080\" colspan=1><font size=\"2\"></font></th>\n"
#endif
#ifndef QOS_DSCP_MATCH
	"<th bgcolor=\"#808080\" colspan=6><font size=\"2\">%s</font></th>\n"
#else
	"<th bgcolor=\"#808080\" colspan=7><font size=\"2\">%s</font></th>\n"
#endif
#ifndef NEW_IP_QOS_SUPPORT
	"<th bgcolor=\"#808080\" colspan=8><font size=\"2\">%s</font></th>\n"
	//"<th bgcolor=\"#808080\" colspan=7><font size=\"2\">Mark</font></th>\n"));
#else
#ifndef QOS_DSCP
	"<th bgcolor=\"#808080\" colspan=5><font size=\"2\">%s</font></th>\n"
#else
	"<th bgcolor=\"#808080\" colspan=6><font size=\"2\">%s</font></th>\n")
#endif
#endif
	, multilang(LANG_CLASSIFICATION_RULES)
#ifndef NEW_IP_QOS_SUPPORT
	, multilang(LANG_CLASSIFICATION_RESULTS)
#else
	, multilang(LANG_MARK)
#endif
	);
//	"<th bgcolor=\"#808080\" colspan=2><font size=\"2\"></font></th>\n"));
	nBytesSent += boaWrite(wp, "<tr>"
      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>\n"
#ifdef _CWMP_MIB_
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Status</b></font></td>\n"
#endif
#ifdef NEW_IP_QOS_SUPPORT
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src IP</b></font></td>\n"
#else
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src IP</b></font></td>\n"
#endif
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Src Port</b></font></td>\n"
      	"<td align=center width=\"25%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Dst IP</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Dst Port</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Lan Port</b></font></td>\n"
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>DSCP</b></font></td>\n"
#endif
      	//"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>802.1p</b></font></td>\n"
#endif
		);
      	nBytesSent += boaWrite(wp,
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Interface</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Priority</b></font></td>\n"
      	#ifdef QOS_SPEED_LIMIT_SUPPORT
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Speed  Limit</b></font></td>\n"
	#endif
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP Precd</b></font></td>\n"
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP ToS</b></font></td>\n"
#ifdef QOS_DSCP
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>IP DSCP</b></font></td>\n"
#endif
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>802.1p</b></font></td>\n"
#ifdef IP_POLICY_ROUTING
      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Wan IF</b></font></td>\n"
#endif
//#ifdef _CWMP_MIB_
//      	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Enable</b></font></td>\n"
//#endif
//      	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td></tr>\n"));
      	"</tr>\n");
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry))
		{
			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

#ifdef QOS_DIFFSERV
		if (Entry.enDiffserv == 1) // Diffserv entry
			continue;
#endif

		// source ip
		psip = inet_ntoa(*((struct in_addr *)Entry.sip));
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbit);
			psip = sip;
		}
		//snprintf(sip, 20, "%s/%d", inet_ntoa(*((struct in_addr *)Entry.sip)), Entry.smaskbit);
		// source port
		if (Entry.sPort == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sPort);

		// destination ip
		pdip = inet_ntoa(*((struct in_addr *)Entry.dip));
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbit);
			pdip = dip;
		}
		//snprintf(dip, 20, "%s/%d", inet_ntoa(*((struct in_addr *)Entry.dip)), Entry.dmaskbit);
		// destination port
		if (Entry.dPort == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dPort);


		if ( Entry.protoType == PROTO_TCP )
			type = ARG_TCP;
		else if ( Entry.protoType == PROTO_UDP )
			type = ARG_UDP;
		else if ( Entry.protoType == PROTO_ICMP )
			type = ARG_ICMP;
		else
			type = BLANK;

#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
		//DSCP
		if (Entry.qosDscp == 0)
			strcpy(qosDscp, BLANK);
		else
			snprintf(qosDscp, 5, "%d", Entry.qosDscp>>2);
#endif

		//802.1p
		if (Entry.vlan1p == 0)
			strcpy(vlan1p, BLANK);
		else
			snprintf(vlan1p, 5, "%d", Entry.vlan1p-1);
#endif

		// Assign outif
		if (Entry.outif == DUMMY_IFINDEX)
			strcpy(pItf, BLANK);
		else
		{
			if(getATMVCEntryByIfIndex(Entry.outif, &vc_entry))
			{
				getDisplayWanName(&vc_entry, pItf);
			}
			else
				strcpy(pItf, BLANK);
		}

		// Assign Priority
		if (Entry.prior <= (IPQOS_NUM_PRIOQ-1))
			pPrio = prioLevel[Entry.prior];
		else
			// should not be here !!
			pPrio = prioLevel[2];

#ifdef QOS_DSCP
		if(Entry.dscp==0) {
			strcpy(pDscp, BLANK);
#endif
		if (Entry.m_ipprio == 0)
			pIPrio = BLANK;
		else
			pIPrio = n0to7[Entry.m_ipprio];

		if (Entry.m_iptos == 0xff)
			pTos = BLANK;
		else {
			int mask, i;

			mask = i = 1;
			while (i<=5) {
				if (Entry.m_iptos & mask)
					break;
				else {
					i++;
					mask<<=1;
				}
			}
			if (i>=6)
				i = 1;
			pTos = ipTos[i];
		}
#ifdef QOS_DSCP
		}
		else if (Entry.dscp == 1) {
			pIPrio = BLANK;
			pTos = BLANK;
#ifndef NEW_IP_QOS_SUPPORT
			if (Entry.m_ipprio == 0 && Entry.m_iptos == 0)
				strcpy(pDscp, "BE");
			else if (Entry.m_ipprio == 5 && Entry.m_iptos == 3)
				strcpy(pDscp, "EF");
			else
				sprintf(pDscp, "AF%d%d", Entry.m_ipprio, Entry.m_iptos);
#else
			if (Entry.m_dscp == 0)
				strcpy(pDscp, BLANK);
			else
				snprintf(pDscp, 5, "%d", Entry.m_dscp>>2);
#endif
		}
#endif

		if (Entry.m_1p == 0)
			p1p = BLANK;
		else
			p1p = n0to7[Entry.m_1p];

		pPhy = strPhy;
		if (Entry.phyPort == 0xff)
			pPhy = (char *)BLANK;
#ifdef IP_QOS_VPORT
		else if (Entry.phyPort < SW_LAN_PORT_NUM)
			strPhy[3] = '0' + virt2user[Entry.phyPort];
#else
		else if (Entry.phyPort == 0)
			strPhy[3] = '0';
#endif
#ifdef CONFIG_USB_ETH
		else if (Entry.phyPort == IFUSBETH_PHYNUM)
			pPhy = (char *)USBETHIF;
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		else
		{
			#ifdef WLAN_MBSSID
			if(Entry.phyPort==5) //wlan0
				pPhy = (char *)WLANIF[0];
			else 	// 6,7,8,9 vap0,vap1 vap2 vap3
			{
				strncpy(strPhy, "vap0", 4);
				strPhy[3]='0'+ (Entry.phyPort-6);
			}
			#else
			pPhy = (char *)WLANIF[0];
			#endif
		}

#endif
#ifdef QOS_SPEED_LIMIT_SUPPORT
		char qosSpeedLimit[8]={0};
		if(Entry.limitSpeedEnabled) sprintf(qosSpeedLimit,"%d",Entry.limitSpeedRank);
		else sprintf(qosSpeedLimit,"none");
#endif

		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"s%d\" value=1></td>\n"
#ifdef _CWMP_MIB_
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif
		"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"25%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif
     			//"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif
     			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
	#ifdef QOS_SPEED_LIMIT_SUPPORT
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
	#endif
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
	#ifdef QOS_DSCP
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
	#endif
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",
//			psip, sport, pdip, dport, type, pPhy, pPrio,
			i,
#ifdef _CWMP_MIB_
			(Entry.enable==0)?"Disable":"Enable",
#endif
			psip, sport, pdip, dport, type, pPhy,
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP_MATCH
			qosDscp,
#endif
			/*vlan1p, */
#endif
			pItf,
			pPrio,
		#ifdef QOS_SPEED_LIMIT_SUPPORT
			qosSpeedLimit,
		#endif
//			pIPrio, pTos, p1p);
			pIPrio, pTos,
		#ifdef QOS_DSCP
			pDscp,
		#endif
			p1p);

#ifdef IP_POLICY_ROUTING
		if (Entry.outif == DUMMY_IFINDEX) {
			wanif[0] = ' ';
			wanif[1] = 0;
		}
		else {

			ifGetName(Entry.outif, wanif, sizeof(wanif));

		}

		nBytesSent += boaWrite(wp,
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n", wanif);
#endif
		nBytesSent += boaWrite(wp, "</tr>\n");

	}

	return nBytesSent;
}

#define IP_QOS_QUEUE_LIST 0x1
#define IP_QOS_QUEUE_RULE_LIST 0x2
#define IP_QOS_QUEUE_BUTTON 0x04
#define IP_QOS_RULE_SUMMARY 0x1
#define IP_QOS_RULE_DETAIL 0x2
/////////////////////////////////////////////////////////////////////////////
void formQueueAdd(request * wp, char *path, char *query)
{
	char *submitUrl, *strVal, *str;
	char tmpBuf[100];
	unsigned int totalEntry;
	MIB_CE_IP_QOS_QUEUE_T entry;
	unsigned char vChar;

	int itfid, i, num;
	int order=0, old_itfid= -1;
	char *arg0, *token;

#ifndef NO_ACTION
	int pid;
#endif

#if defined(APPLY_CHANGE)
#ifdef IP_QOS
	mib_get(MIB_MPMODE, (void *)&vChar);
	if (vChar&MP_IPQ_MASK)
		stopIPQ();
#endif
#endif
	strVal = boaGetVar(wp, "save", "");

	// enable/disable Qos
	if (strVal[0]) {
		memset(&entry, 0, sizeof(MIB_CE_IP_QOS_QUEUE_T));

		strVal = boaGetVar(wp, "queuedesc", "");
		strncpy(entry.desc,strVal,MAX_QUEUE_DESC_LEN-1);
		entry.desc[MAX_QUEUE_DESC_LEN-1] = 0;

		strVal = boaGetVar(wp, "queueenbl", "");
		if ( strVal[0] == '1' )
			entry.enable = 0;   // disable
		else
			entry.enable = 1;   // enable

		strVal = boaGetVar(wp, "queueintf", "");
		if (strVal[0]) {
			entry.outif = (unsigned int)atoi(strVal);
		}

		// outbound priority
		strVal = boaGetVar(wp, "queuepriority", "");
		entry.prior = (unsigned char)(strVal[0]-'1');

		if (findQosQueue(&entry)) {
			strcpy(tmpBuf, multilang(LANG_ENTRY_ALREADY_EXISTS));
			goto setErr_queueadd;
		}
/*ping_zhang:20081201 START:update Qos for Dlink project*/
#ifdef _CWMP_MIB_ /*for cwmp-tr069*/
		entry.QueueInstNum = findUnusedQueueInstNum();
#endif
/*ping_zhang:20081201 END*/

		if(mib_chain_add(MIB_IP_QOS_QUEUE_TBL, (unsigned char*)&entry) < 1){
			strcpy(tmpBuf, Tadd_chain_error);
			goto setErr_queueadd;
		}
	}

	strVal = boaGetVar(wp, "RemoveQueue", "");
	// Remove Queue
	if (strVal[0]) {
		str = boaGetVar(wp, "removeQueueList", "");

		// Clean the end ',' character.
		if ( strlen(str) > 1)
			str[strlen(str)-1]='\0';

		arg0=str;
		while ((token=strtok(arg0,","))!=NULL) {
			order++;
			itfid = atoi(token);
			itfid = itfid - order;

			// delete from chain record
			if(mib_chain_delete(MIB_IP_QOS_QUEUE_TBL, itfid) != 1) {
				strcpy(tmpBuf, Tdelete_chain_error);
				goto setErr_queueadd;
			}
			arg0=0;
		}
	}

	strVal = boaGetVar(wp, "SaveAndReboot", "");
	// Save Queue
	if (strVal[0]) {
		str = boaGetVar(wp, "eblQueueList", "");

		// Clean the end ',' character.
		if ( strlen(str) > 1)
			str[strlen(str)-1]='\0';
		//printf("str for enabled Queue List=%s\n", str);

		arg0=str;
		while ((token=strtok(arg0,","))!=NULL) {
			itfid = atoi(token);
			// enable the saerched
			itfid = itfid-1;

			// assign the max queue index that need to be disable queue entry.
			i = itfid - 1;

			//printf("itfid=%d i=%d old_itfid=%d\n", itfid, i, old_itfid);
			// disable all the IP Qos queue that index is less than this searched index.
			for ( i; (i > old_itfid && i >= 0); i--) {
				//printf("i=%d\n", i);
				if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void *)&entry)) {
					strcpy(tmpBuf, multilang(LANG_GET_CHAIN_RECORD_ERROR));
					goto setErr_queueadd;
				}
				entry.enable = 0;
				mib_chain_update(MIB_IP_QOS_QUEUE_TBL, (void *)&entry, i);
			}

			// enable this saerched index for IP Qos Queue
			if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, itfid, (void *)&entry)){
				strcpy(tmpBuf, multilang(LANG_GET_CHAIN_RECORD_ERROR));
				goto setErr_queueadd;
			}
			entry.enable = 1;
			mib_chain_update(MIB_IP_QOS_QUEUE_TBL, (void *)&entry, itfid);

			arg0=0;
			old_itfid = itfid;
		}

		// disable all queue entry that index is less than the max index of queue entry but is bigger than the max index that need to be enabled.
		num = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
		if ( (old_itfid) != num && num > 0 ) {
			i = num - 1;
			for ( i; (i > old_itfid && i >= 0); i--) {
				//printf("ii=%d\n", i);
				if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void *)&entry)) {
					strcpy(tmpBuf, multilang(LANG_GET_CHAIN_RECORD_ERROR));
					goto setErr_queueadd;
				}
				entry.enable = 0;
				mib_chain_update(MIB_IP_QOS_QUEUE_TBL, (void *)&entry, i);
			}
		}
	}
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #ifdef COMMIT_IMMEDIATELY

#if defined(APPLY_CHANGE)
#ifdef IP_QOS
	if (vChar&0x02)
		setupIPQ();
#endif
#endif
	update_qos_tbl();
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

setErr_queueadd:
#if defined(APPLY_CHANGE)
#ifdef IP_QOS
	if (vChar&0x02)
		setupIPQ();
#endif
#endif
	ERR_MSG(tmpBuf);
}

int ipQosQueueList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, entryNumAtmVC, i, j;
	MIB_CE_IP_QOS_QUEUE_T Entry;
	MIB_CE_ATM_VC_T Entry_atmVC;
	char *name;
	int type;
	char ifName[16];

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}

	type = 0;
	if ( !strcmp(name, "queueList") )
		type = IP_QOS_QUEUE_LIST;
	else if ( !strcmp(name, "ruleList") )
		type = IP_QOS_QUEUE_RULE_LIST;
	else if ( !strcmp(name, "QueueButton") )
		type = IP_QOS_QUEUE_BUTTON;
	else
		type = IP_QOS_QUEUE_LIST;

	entryNum = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
	entryNumAtmVC = mib_chain_total(MIB_ATM_VC_TBL);

	if ( type == IP_QOS_QUEUE_BUTTON ) {
		if ( entryNum > 0 ) {
			nBytesSent += boaWrite(wp,
				"<input type='submit' name=\"%s\" onClick=removeClick(this.form.removeQ) value='Remove'>\n"
				"<input type='submit' name=\"%s\" onClick='savRebootClick(this.form.enableQ)' value='Save'></p>\n",
				multilang(LANG_REMOVEQUEUE), multilang(LANG_SAVEANDREBOOT));
		}
		return nBytesSent;
	}

	if ( type == IP_QOS_QUEUE_LIST ) {
		nBytesSent += boaWrite(wp, "<tr><font size=2>"
      		"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
      		"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
      		"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
      		"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
      		"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
      		"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=2>%s</td></tr>\n",
      		multilang(LANG_INTERFACE), multilang(LANG_DESCRIPTION),
		multilang(LANG_PRECEDENCE), multilang(LANG_QUEUE_KEY),
		multilang(LANG_ENABLE), multilang(LANG_REMOVE));
	}

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (!isValidMedia(Entry.outif))
			continue;
		for (j=0; j<entryNumAtmVC; j++) {
			/*ping_zhang:20081201 START:update Qos for Dlink project*/
			if(Entry.outif == DUMMY_IFINDEX) /*the entry is new added by tr069*/
			{
				memset( &Entry_atmVC, 0, sizeof( MIB_CE_ATM_VC_T ) );
				break;
			}
			/*ping_zhang:20081201 END*/
			if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&Entry_atmVC))
			{
  				boaError(wp, 400, "Get chain record error!\n");
				return -1;
			}

			if ( Entry_atmVC.ifIndex == Entry.outif )
				break;
		}

		if (j==entryNumAtmVC) // not found
			continue;
		//snprintf(vpi, 6, "%u", Entry_atmVC.vpi);
		//snprintf(vci, 6, "%u", Entry_atmVC.vci);
		if ( type == IP_QOS_QUEUE_LIST ) {
			getDisplayWanName(&Entry_atmVC, ifName);
			nBytesSent += boaWrite(wp, "<tr>"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      				"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%u</td>\n"
      				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%u</td>\n"
      				"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"enableQ\" value=\"%u\" %s></td>\n"
      				"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"removeQ\" value=\"%u\"></td></tr>\n",
				ifName, Entry.desc, Entry.prior, i+1, i+1, Entry.enable==1? "checked":"", i+1);
		}

		if ( type == IP_QOS_QUEUE_RULE_LIST ) {
			getDisplayWanName(&Entry_atmVC, ifName);
			//nBytesSent += boaWrite(wp,"<option  value=\"%d\">PVC %u/%u&Prece %d&Queue %d</option>",i ,Entry_atmVC.vpi, Entry_atmVC.vci, Entry.prior, i+1);
			nBytesSent += boaWrite(wp,"<option  value=\"%d\">%s_p%d</option>",ifName,Entry.prior);
		}

	}
	return nBytesSent;
}

int policy_route_outif(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char wanif[IFNAMSIZ];

#ifdef IP_POLICY_ROUTING
	nBytesSent += boaWrite(wp, "<tr>\n");
#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += boaWrite(wp, "\t<td align=left><font size=2>%s:</font></td>\n",
			multilang(LANG_OUTBOUND_ITERFACE));
#else
	nBytesSent += boaWrite(wp, "\t<td align=left><font size=2><b>%s:</b></font></td>\n",
			multilang(LANG_OUTBOUND_ITERFACE));
#endif
	nBytesSent += boaWrite(wp, "\t<td align=left>\n\t\t<select name=out_if>\n");
	nBytesSent += boaWrite(wp, "\t\t<option value=255></option>\n");
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0 || !isValidMedia(Entry.ifIndex))
			continue;

		if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			continue;

		ifGetName(Entry.ifIndex, wanif, sizeof(wanif));
		nBytesSent += boaWrite(wp, "\t\t<option value=%u>%s</option>\n",
			Entry.ifIndex, wanif);
	}

	nBytesSent += boaWrite(wp, "\t\t</select>\n\t</td>\n</tr>\n");
#endif
	return nBytesSent;
}
#endif	// of IP_QOS

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
int priority_outif(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, entryNumAtmVC, i, j;
	MIB_CE_IP_QOS_QUEUE_T Entry;
	MIB_CE_ATM_VC_T Entry_atmVC;
	char ifName[16];


	entryNum = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
	entryNumAtmVC = mib_chain_total(MIB_ATM_VC_TBL);

#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += boaWrite(wp, " \t<select name=queuekey style=\"width:150px \">\n");
#else
	nBytesSent += boaWrite(wp, " \t<select name=queuekey>\n");
#endif
	nBytesSent += boaWrite(wp, "\t\t<option  value=\"-1\" selected>(Click to Select)</option>");
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_QOS_QUEUE_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (!isValidMedia(Entry.outif))
			continue;
		for (j=0; j<entryNumAtmVC; j++) {
			/*ping_zhang:20081201 START:update Qos for Dlink project*/
			if(Entry.outif == DUMMY_IFINDEX) /*the entry is new added by tr069*/
			{
				memset( &Entry_atmVC, 0, sizeof( MIB_CE_ATM_VC_T ) );
				break;
			}
			/*ping_zhang:20081201 END*/
			if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&Entry_atmVC))
			{
  				boaError(wp, 400, "Get chain record error!\n");
				return -1;
			}

			if ( Entry_atmVC.ifIndex == Entry.outif )
				break;
		}
		if (j==entryNumAtmVC) // not found
			continue;
		getDisplayWanName(&Entry_atmVC, ifName);
		nBytesSent += boaWrite(wp,"\t\t<option  value=\"%d\">%s_p%d</option>",i,ifName,Entry.prior);
	}
	nBytesSent += boaWrite(wp, "  \t</select>\n");
	return nBytesSent;
}

int confDscp(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

      nBytesSent += boaWrite(wp,  "<tr>\n");
#ifdef NEW_IP_QOS_SUPPORT
#ifdef QOS_DSCP
      nBytesSent += boaWrite(wp,  "\t<td colspan=2><font size=2><input type=\"radio\" name=dscpenable value=0 onClick=\"return dscpClick()\">%s&nbsp;&nbsp;</td></tr>\n",
		      multilang(LANG_TYPE_OF_SERVIE_TOS));
#endif
      nBytesSent += boaWrite(wp,  "\t<tr><td><font size=2>%s:&nbsp;&nbsp;</td><td><select name=ipprio style=\"width:150px \">\n",
		      multilang(LANG_PRECEDENCE));
#else
#ifdef QOS_DSCP
      nBytesSent += boaWrite(wp,  "\t<td colspan=2><font size=2><b><input type=\"radio\" name=dscpenable value=0 onClick=\"return dscpClick()\">%s&nbsp;&nbsp;</b></td>\n",
		      multilang(LANG_TYPE_OF_SERVIE_TOS));
#endif
      nBytesSent += boaWrite(wp,  "\t<td><font size=2><b>IP.Pred_Mark:&nbsp;&nbsp;</b></td><td><select name=ipprio>\n");
#endif
      nBytesSent += boaWrite(wp,  "\t\t<option value=0></option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=1>0</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=2>1</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=3>2</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=4>3</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=5>4</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=6>5</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=7>6</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=8>7</option>\n");
#ifdef NEW_IP_QOS_SUPPORT
      nBytesSent += boaWrite(wp,  "\t</select></td></tr>\n");
      nBytesSent += boaWrite(wp,  "\t<tr><td><font size=2>TOS:&nbsp;&nbsp;</td><td><select name=tos style=\"width:150px \">\n");
#else
      nBytesSent += boaWrite(wp,  "\t</select></td>\n");
      nBytesSent += boaWrite(wp,  "\t<td><font size=2><b>TOS_Mark:&nbsp;&nbsp;</b></td><td><select name=tos>\n");
#endif
      nBytesSent += boaWrite(wp,  "\t\t<option value=255></option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=0>Normal Service</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=2>Minimize Cost</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=4>Maximize Reliability</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=8>Maximize Throughput</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=16>Minimize Delay</option>\n");
      nBytesSent += boaWrite(wp,  "\t</select></td>\n");
      nBytesSent += boaWrite(wp,  "</tr>\n");
#ifdef QOS_DSCP
      nBytesSent += boaWrite(wp,  "<tr>\n");
#ifdef NEW_IP_QOS_SUPPORT
      nBytesSent += boaWrite(wp,  "\t<td colspan=2><font size=2><input type=\"radio\" name=dscpenable value=1 onClick=\"return dscpClick()\" checked>Diffserv Codepoint (DSCP)&nbsp;&nbsp;</td></tr>\n");
      nBytesSent += boaWrite(wp,  "\t<tr><td><font size=2>DSCP tag:&nbsp;&nbsp;</td><td><select name=dscp style=\"width:150px \">\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=0> </option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=1>default(000000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=57>AF13(001110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=49>AF12(001100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=41>AF11(001010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=33>CS1(001000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=89>AF23(010110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=81>AF22(010100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=73>AF21(010010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=65>CS2(010000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=121>AF33(011110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=113>AF32(011100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=105>AF31(011010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=97>CS3(011000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=153>AF43(100110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=145>AF42(100100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=137>AF41(100010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=129>CS4(100000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=185>EF(101110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=161>CS5(101000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=193>CS6(110000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=225>CS7(111000)</option>\n");
#else
      nBytesSent += boaWrite(wp,  "\t<td colspan=2><font size=2><b><input type=\"radio\" name=dscpenable value=1 onClick=\"return dscpClick()\" checked>Diffserv Codepoint (DSCP)&nbsp;&nbsp;</b></td>\n");
      nBytesSent += boaWrite(wp,  "\t<td><font size=2><b>Value:&nbsp;&nbsp;</b><select name=dscp>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=0>BE PHB (000000)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=10>AF11 PHB (001010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=12>AF12 PHB (001100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=14>AF13 PHB (001110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=18>AF21 PHB (010010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=20>AF22 PHB (010100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=22>AF23 PHB (010110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=26>AF31 PHB (011010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=28>AF32 PHB (011100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=30>AF33 PHB (011110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=34>AF41 PHB (100010)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=36>AF42 PHB (100100)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=38>AF43 PHB (100110)</option>\n");
      nBytesSent += boaWrite(wp,  "\t\t<option value=46>EF PHB (101110)</option>\n");
#endif
      nBytesSent += boaWrite(wp,  "\t</select></td>\n");
      nBytesSent += boaWrite(wp,  "</tr>\n");
#else
      nBytesSent += boaWrite(wp,  "<tr ID='dscphidden' style=\"display:none\">\n");
      nBytesSent += boaWrite(wp,  "\t<td><input type=radio name=dscpenable checked></td>\n");
      nBytesSent += boaWrite(wp,  "\t<td><input type=radio name=dscpenable ></td>\n");
      nBytesSent += boaWrite(wp,  "\t<td><input type=hidden name=dscp></td>\n");
      nBytesSent += boaWrite(wp,  "</tr>\n");
      #if 0
      nBytesSent += boaWrite(wp,  "<tr>\n");
      nBytesSent += boaWrite(wp,  "\t<td><input type=\"hidden\" value=\"0\" name=dscpenable></td>\n");
      nBytesSent += boaWrite(wp,  "\t<td><input type=\"hidden\" value=\"0\" name=dscp></td>\n");
      nBytesSent += boaWrite(wp,  "</tr>\n");
      #endif
#endif
	return nBytesSent;
}

#ifdef QOS_DIFFSERV
void formDiffServ(request * wp, char *path, char *query)
{
	char *submitUrl, *strAddQos, *strVal, *strList, *strDelQos, *strPHBClass, *strHTBRate;
	char *strSAddr, *strDAddr, *strSport, *strDport, *strInf, *strBandwidth, *strLatency, *strDSCP, *strRate, *strPolicing, *strProt;
	char tmpBuf[100];
	int intVal, ret;
	unsigned int totalEntry, i, clearIdx;
	MIB_CE_IP_QOS_T entry;
	unsigned long mask, mbit;
	unsigned char vChar, vChar_mode = 0, vChar_class = 0, pClass;
#ifndef NO_ACTION
	int pid;
#endif

#ifdef IP_QOS
	mib_get(MIB_MPMODE, (void *)&vChar_mode);
	if (vChar_mode&MP_IPQ_MASK)
		stopIPQ();
#endif

	mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&pClass);
	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */
	for (i = 0; i < totalEntry; i ++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&entry))
			continue;
		if (entry.enDiffserv == 0) // non-Diffserv entry
			continue;
		if (pClass == entry.m_ipprio) {
			clearIdx = i;
			break;
		}
	}

	strDelQos = boaGetVar(wp, "delSel", "");
	/* Delete entry */
	if (strDelQos[0]) {
		unsigned int idx;
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {
			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "s%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");
			if (strVal[0] == '1') {
				deleted ++;
				cleanupDiffservRule(idx);
				if(mib_chain_delete(MIB_IP_QOS_TBL, idx) != 1) {
					strcpy(tmpBuf, Tdelete_chain_error);
					goto setErr_qos;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, multilang(LANG_THERE_IS_NO_ITEM_SELECTED_TO_DELETE));
			goto setErr_qos;
		}
		goto setOk_qos;
	}

	strAddQos = boaGetVar(wp, "addqos", "");
	if (strAddQos[0]) {
		// enable/disable DiffServ
		strVal = boaGetVar(wp, "qoscap", "");
		vChar = strVal[0] - '0';
	}

	strPHBClass = boaGetVar(wp, "phbclass", "");
	if (strPHBClass[0]) {
		// PHB Class
		vChar_class = strPHBClass[0] - '0';
	}

	if (vChar == 1) {
		unsigned char smask[4], dmask[4], sip[4], dip[4];

		//if (deleteDiffservEntry()) {
		//	strcpy(tmpBuf, Tdelete_chain_error);
		//	goto setErr_qos;
		//}

		// WAN interface
		strInf = boaGetVar(wp, "interface", "");

		// Total Bandwidth Limit
		strBandwidth = boaGetVar(wp, "totalbandwidth", "");

		// Class Rate Limit
		strHTBRate = boaGetVar(wp, "htbrate", "");

		// Max Desired Latency
		strLatency = boaGetVar(wp, "latency", "");

		strList = boaGetVar(wp, "phblst", "");
		if (strList[0]) {
			char *token, *phblist;

			phblist = strdup(strList);
			while ((token = strtok(phblist, ",")) != NULL) {
				int phb;
				memset(&entry, 0, sizeof(entry));
				if (strInf[0])
					entry.ifIndex = (unsigned int)atoi(strInf);

				/* Add new qos entry */
				memset(sip, 0x0, 4);
				memset(dip, 0x0, 4);
				if (totalEntry >= MAX_QOS_RULE) {
					strcpy(tmpBuf, Texceed_max_rules);
					free(phblist);
					goto setErr_qos;
				}
				// Diffserv entry flag
				entry.enDiffserv = 1;
				// phb
				phb = atoi(token);
				entry.m_ipprio = phb >> 3;
				entry.m_iptos = (phb & 0x07) >> 1;
				if (strBandwidth[0])
					entry.totalBandwidth = strtol(strBandwidth, (char**)NULL, 0);
				if (strHTBRate[0])
					entry.htbRate = strtol(strHTBRate, (char**)NULL, 0);
				if (strLatency[0])
					entry.latency = strtol(strLatency, (char**)NULL, 0);

				// DSCP
#ifdef QOS_DSCP_MATCH
				snprintf(tmpBuf, 100, "m_dscp%s", token);
				strDSCP = boaGetVar(wp, tmpBuf, "");
				if (strDSCP[0])
					entry.qosDscp = strtol(strDSCP, (char**)NULL, 0);
				else
#endif
					entry.qosDscp = 0;
				snprintf(tmpBuf, 100, "rate%s", token);
				strRate = boaGetVar(wp, tmpBuf, "");
				// Police Rate
				if (strRate[0])
					entry.limitSpeed = strtol(strRate, (char**)NULL, 0);
				else
					entry.limitSpeed = 0;
				// Policing: drop/continue
				snprintf(tmpBuf, 100, "bhv%s", token);
				strPolicing = boaGetVar(wp, tmpBuf, "");
				if (strPolicing[0])
					entry.policing = strPolicing[0] - '0';
				// protocol type
				snprintf(tmpBuf, 100, "prot%s", token);
				strProt = boaGetVar(wp, tmpBuf, "");
				if (strProt[0])
					entry.protoType = strProt[0] - '0';

				snprintf(tmpBuf, 100, "sip%s", token);
				strSAddr = boaGetVar(wp, tmpBuf, "");
				snprintf(tmpBuf, 100, "dip%s", token);
				strDAddr = boaGetVar(wp, tmpBuf, "");
				// Source address
				if (strSAddr[0]) {
					inet_aton(strSAddr, (struct in_addr *)&entry.sip);
					if ((entry.sip[0]=='\0') &&(entry.sip[1]=='\0') && (entry.sip[2]=='\0') && (entry.sip[3]=='\0')) {
						strcpy(tmpBuf, Tinvalid_source_ip);
						free(phblist);
						goto setErr_qos;
					}
					snprintf(tmpBuf, 100, "smask%s", token);
					strSAddr = boaGetVar(wp, tmpBuf, "");
					if (strSAddr[0]) {
						if (!isValidNetmask(strSAddr, 1)) {
							strcpy(tmpBuf, Tinvalid_source_netmask);
							free(phblist);
							goto setErr_qos;
						}
						inet_aton(strSAddr, (struct in_addr *)smask);
						inet_aton(strSAddr, (struct in_addr *)&mask);
						if (mask==0) {
							strcpy(tmpBuf, Tinvalid_source_netmask);
							free(phblist);
							goto setErr_qos;
						}
						mask = htonl(mask);
						mbit=0; intVal=0;
						for (i=0; i<32; i++) {
							if (mask&0x80000000) {
								if (intVal) {
									strcpy(tmpBuf, Tinvalid_source_netmask);
									free(phblist);
									goto setErr_qos;
								}
								mbit++;
							}
							else
								intVal=1;
							mask <<= 1;
						}
						entry.smaskbit = mbit;
					}
					else {
						entry.smaskbit = 32;
						inet_aton(ARG_255x4, (struct in_addr *)smask);
					}

					// Jenny, for checking duplicated source address
					sip[0] = entry.sip[0] & smask[0];
					sip[1] = entry.sip[1] & smask[1];
					sip[2] = entry.sip[2] & smask[2];
					sip[3] = entry.sip[3] & smask[3];
				}

				// source port
				if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
					snprintf(tmpBuf, 100, "sport%s", token);
					strSport = boaGetVar(wp, tmpBuf, "");
					if (strSport[0]) {
						if ( !string_to_dec(strSport, &intVal) || intVal<1 || intVal>65535) {
							strcpy(tmpBuf, Tinvalid_source_port);
							free(phblist);
							goto setErr_qos;
						}
						entry.sPort = (unsigned short)intVal;
					}
				}

				// Destination address
				if (strDAddr[0]) {
					inet_aton(strDAddr, (struct in_addr *)&entry.dip);
					if ((entry.dip[0]=='\0') && (entry.dip[1]=='\0') && (entry.dip[2]=='\0') && (entry.dip[3]=='\0')) {
						strcpy(tmpBuf, Tinvalid_destination_ip);
						free(phblist);
						goto setErr_qos;
					}
					snprintf(tmpBuf, 100, "dmask%s", token);
					strDAddr = boaGetVar(wp, tmpBuf, "");
					if (strDAddr[0]) {
						if (!isValidNetmask(strDAddr, 1)) {
							strcpy(tmpBuf, Tinvalid_destination_netmask);
							free(phblist);
							goto setErr_qos;
						}
						inet_aton(strDAddr, (struct in_addr *)dmask);
						inet_aton(strDAddr, (struct in_addr *)&mask);
						if (mask==0) {
							strcpy(tmpBuf, Tinvalid_destination_netmask);
							free(phblist);
							goto setErr_qos;
						}
						mask = htonl(mask);
						mbit=0; intVal=0;
						for (i=0; i<32; i++) {
							if (mask&0x80000000) {
								if (intVal) {
									strcpy(tmpBuf, Tinvalid_destination_netmask);
									free(phblist);
									goto setErr_qos;
								}
								mbit++;
							}
							else
								intVal=1;
							mask <<= 1;
						}
						entry.dmaskbit = mbit;
					}
					else {
						entry.dmaskbit = 32;
						inet_aton(ARG_255x4, (struct in_addr *)dmask);
					}
					// Jenny, for checking duplicated destination address
					dip[0] = entry.dip[0] & dmask[0];
					dip[1] = entry.dip[1] & dmask[1];
					dip[2] = entry.dip[2] & dmask[2];
					dip[3] = entry.dip[3] & dmask[3];
				}

				// destination port
				if (entry.protoType == PROTO_TCP || entry.protoType == PROTO_UDP) {
					snprintf(tmpBuf, 100, "dport%s", token);
					strDport = boaGetVar(wp, tmpBuf, "");
					if (strDport[0]) {
						if ( !string_to_dec(strDport, &intVal) || intVal<1 || intVal>65535) {
							strcpy(tmpBuf, Tinvalid_destination_port);
							free(phblist);
							goto setErr_qos;
						}
						entry.dPort = (unsigned short)intVal;
					}
				}

				// Check if this qos entry exists
				ret = checkRule_ipqos(entry, sip, dip);
				if (ret == 0) {
					strcpy(tmpBuf, Tinvalid_rule);
					free(phblist);
					goto setErr_qos;
				}
				else if (ret == 2) {
					if (vChar_class == 5)
						snprintf(tmpBuf, 100, multilang(LANG_EF_CLASS_EXISTED_PLEASE_DELETE_EXISTED_ENTRY_FIRST));
					else
						snprintf(tmpBuf, 100, multilang(LANG_AF_CLASS_D_EXISTED_PLEASE_DELETE_EXISTED_ENTRY_FIRST), vChar_class);
					free(phblist);
					goto setErr_qos;
				}
				mib_set(MIB_DIFFSERV_PHBCLASS, (void *)&vChar_class);
				intVal = mib_chain_add(MIB_IP_QOS_TBL, (unsigned char*)&entry);
				if (intVal == 0) {
					strcpy(tmpBuf, Tadd_chain_error);
					free(phblist);
					goto setErr_qos;
				}
				else if (intVal == -1) {
					strcpy(tmpBuf, strTableFull);
					free(phblist);
					goto setErr_qos;
				}
				phblist = 0;
			}
			free(phblist);
		}
		mib_set(MIB_QOS_DIFFSERV, (void *)&vChar);

#if defined(APPLY_CHANGE)
	cleanupDiffservRule(clearIdx);
	setupDiffServ();
#endif
	}
	else if (vChar == 0) {
		mib_set(MIB_QOS_DIFFSERV, (void *)&vChar);
		cleanupDiffservRule(clearIdx);
	}

setOk_qos:
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
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_qos:
	ERR_MSG(tmpBuf);
}

int diffservList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_IP_QOS_T Entry;
	const char *type;
	char phb[5], policing[10], rate[6];
	char	*psip, *pdip, sip[20], dip[20];
	char sport[10], dport[10];
	unsigned int mask, smask, dmask;
	char wanif[IFNAMSIZ];
#ifdef QOS_DSCP_MATCH
	char qosDscp[5];
#endif
	//unsigned char phbclass;

	//mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&phbclass);
	nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Select</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>PHB</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Class Rate</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Inf</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>BW</b></font></td>\n");
	nBytesSent += boaWrite(wp,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Latency</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Src IP</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Src Port</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Dst IP</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Dst Port</b></font></td>\n");
	nBytesSent += boaWrite(wp,
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Protocol</b></font></td>\n"
#ifdef QOS_DSCP_MATCH
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>DSCP</b></font></td>\n"
#endif
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Rate</b></font></td>\n"
		"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>Police</b></font></td>\n"
		"</tr>\n"
		);

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry)) {
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (Entry.enDiffserv == 0) // IP QoS entry
			continue;

		//if (Entry.m_ipprio != phbclass) // only show active PHB class
		//	continue;

		// PHB Group
		if (Entry.m_ipprio == 5 && Entry.m_iptos == 3)
			strcpy(phb, "EF");
		else
			sprintf(phb, "AF%d%d", Entry.m_ipprio, Entry.m_iptos);

		ifGetName(Entry.ifIndex, wanif, sizeof(wanif));

		// source ip
		psip = inet_ntoa(*((struct in_addr *)Entry.sip));
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbit);
			psip = sip;
		}
		// source port
		if (Entry.sPort == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sPort);

		// destination ip
		pdip = inet_ntoa(*((struct in_addr *)Entry.dip));
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbit);
			pdip = dip;
		}
		// destination port
		if (Entry.dPort == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dPort);

		if (Entry.protoType == PROTO_TCP)
			type = ARG_TCP;
		else if (Entry.protoType == PROTO_UDP)
			type = ARG_UDP;
		else if (Entry.protoType == PROTO_ICMP)
			type = ARG_ICMP;
		else
			type = BLANK;

#ifdef QOS_DSCP_MATCH
		//DSCP
		if (Entry.qosDscp == 0)
			strcpy(qosDscp, BLANK);
		else
			snprintf(qosDscp, 5, "%d", Entry.qosDscp>>2);
#endif

		if (Entry.limitSpeed == 0) {
			strcpy(rate, BLANK);
			strcpy(policing, BLANK);
		}
		else {
			snprintf(rate, 6, "%d", Entry.limitSpeed);
			if (Entry.policing == 1)
				strcpy(policing, "Drop");
			else if (Entry.policing == 2)
				strcpy(policing, "Continue");
		}

		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"s%d\" value=1></td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n",
			i, phb, Entry.htbRate, wanif, Entry.totalBandwidth);
		nBytesSent += boaWrite(wp,
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",
			Entry.latency, psip, sport, pdip, dport);
		nBytesSent += boaWrite(wp,
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#ifdef QOS_DSCP_MATCH
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
#endif
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"</tr>\n",
			type
#ifdef QOS_DSCP_MATCH
			, qosDscp
#endif
			, rate, policing);
	}
	return nBytesSent;
}
#endif	// of QOS_DIFFSERV
#endif	// of IP_QOS

#ifdef QOS_DSCP_MATCH
int match_dscp(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	nBytesSent += boaWrite(wp,  "<td>DSCP:</td>\n");
	nBytesSent += boaWrite(wp,  "\t<td><select name=\"s_dscp\" size=\"1\" style=\"width:150px \" onChange=\"onchange_sel1();\">\n");
	nBytesSent += boaWrite(wp,  "\t\t<script>writeDscpList();</script>\n");
	nBytesSent += boaWrite(wp,  "\t\t</select>\n");
	nBytesSent += boaWrite(wp,  "\t</td>\n");

	return nBytesSent;
}
#endif

//alex for 802.1p
#ifdef CONFIG_8021P_PRIO
int  setting_1ppriority(int eid, request * wp, int argc, char **argv)
{
   int nBytesSent=0, i,j,m=0;
   const char *pPrio;
   const char *nset1ptbl;
   unsigned char value[8];

#ifdef NEW_IP_QOS_SUPPORT
	nBytesSent += boaWrite(wp, "<tr>"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">802.1p  priority</font></th>\n"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">transmit priority</font></th>\n</tr>\n"
		);
#else
   nBytesSent += boaWrite(wp, "<tr>"
	"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">802.1p ‰ºòÂ?Á∫?/font></th>\n"
	"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">?ëÈÄÅ‰??àÁ∫ß</font></th>\n</tr>\n"
	);
#endif

    if(mib_get(MIB_8021P_PRIO, (void *)value)== 0)
    {
   	  printf("Get 8021P_PROI  error!\n");
	  return 0;
    }

   for(i=0;i<IPQOS_NUM_PKT_PRIO;i++)
   {

        m= value[i];
	nset1ptbl =set1ptable[i];

      nBytesSent += boaWrite(wp,  "<tr>");
      nBytesSent += boaWrite(wp,  "<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n",i);
      nBytesSent += boaWrite(wp, "  <td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><select name=\"%s\">\n",nset1ptbl);

      pPrio = prioLevel[IPQOS_NUM_PRIOQ-1];
#ifdef NEW_IP_QOS_SUPPORT
		if(m==IPQOS_NUM_PRIOQ-1)
			nBytesSent += boaWrite(wp, " \t\t<option selected value=%d>%s(lowest)</option>\n",  IPQOS_NUM_PRIOQ-1, pPrio);
		else
			nBytesSent += boaWrite(wp, " \t\t<option  value=%d>%s(lowest)</option>\n",  IPQOS_NUM_PRIOQ-1, pPrio);
#else
	if(m==IPQOS_NUM_PRIOQ-1)
	      nBytesSent += boaWrite(wp, " \t\t<option selected value=%d>%s(?Ä‰Ω?</option>\n",  IPQOS_NUM_PRIOQ-1, pPrio);
	else
	       nBytesSent += boaWrite(wp, " \t\t<option  value=%d>%s(?Ä‰Ω?</option>\n",  IPQOS_NUM_PRIOQ-1, pPrio);
#endif
	for (j=IPQOS_NUM_PRIOQ-2; j>=1; j--) {
		pPrio = prioLevel[j];
	   if(m==j)
		nBytesSent += boaWrite(wp, "  \t\t<option selected value=%d>%s</option>\n", j, pPrio);
	   else
	   	nBytesSent += boaWrite(wp, "  \t\t<option  value=%d>%s</option>\n", j, pPrio);
	}
	pPrio = prioLevel[0];
#ifdef NEW_IP_QOS_SUPPORT
		if(m==0)
			nBytesSent += boaWrite(wp, "  \t\t<option selected value=0>%s(highest)</option>\n", pPrio);
		else
			nBytesSent += boaWrite(wp, "  \t\t<option  value=0>%s(highest)</option>\n", pPrio);
#else
	if(m==0)
	  nBytesSent += boaWrite(wp, "  \t\t<option selected value=0>%s(?ÄÈ´?</option>\n", pPrio);
	else
	   nBytesSent += boaWrite(wp, "  \t\t<option  value=0>%s(?ÄÈ´?</option>\n", pPrio);
#endif
	nBytesSent += boaWrite(wp, "  '\t</select>\n");
      nBytesSent += boaWrite(wp,  "</tr>");
   }

   return nBytesSent;
}

#ifdef NEW_IP_QOS_SUPPORT
int setting_predprio(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0, i,j,m=0;
	const char *pPrio;
	const char *nset1ptbl;
	unsigned char value[8];

	nBytesSent += boaWrite(wp, "<tr>"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">IP Precedence</font></th>\n"
		"<th bgcolor=\"#808080\" colspan=1><font size=\"2\">transmit priority</font></th>\n</tr>\n"
		);

	if(mib_get(MIB_PRED_PRIO, (void *)value)== 0)
	{
		printf("Get PRED_PRIO  error!\n");
		return 0;
	}

	for(i=0;i<IPQOS_NUM_PKT_PRIO;i++)
	{
		m= value[i];
		nset1ptbl =setpredtable[i];

		nBytesSent += boaWrite(wp,  "<tr>");
		nBytesSent += boaWrite(wp,  "<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%d</td>\n",i);
		nBytesSent += boaWrite(wp, "  <td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><select name=\"%s\">\n",nset1ptbl);

		pPrio = prioLevel[IPQOS_NUM_PRIOQ-1];
		if(m==IPQOS_NUM_PRIOQ-1)
			nBytesSent += boaWrite(wp, " \t\t<option selected value=%d>%s(lowest)</option>\n",  IPQOS_NUM_PRIOQ-1, pPrio);
		else
			nBytesSent += boaWrite(wp, " \t\t<option  value=%d>%s(lowest)</option>\n",  IPQOS_NUM_PRIOQ-1, pPrio);

		for (j=IPQOS_NUM_PRIOQ-2; j>=1; j--) {
			pPrio = prioLevel[j];
			if(m==j)
				nBytesSent += boaWrite(wp, "  \t\t<option selected value=%d>%s</option>\n", j, pPrio);
			else
				nBytesSent += boaWrite(wp, "  \t\t<option  value=%d>%s</option>\n", j, pPrio);
		}
		pPrio = prioLevel[0];
		if(m==0)
			nBytesSent += boaWrite(wp, "  \t\t<option selected value=0>%s(highest)</option>\n", pPrio);
		else
			nBytesSent += boaWrite(wp, "  \t\t<option  value=0>%s(highest)</option>\n", pPrio);
		nBytesSent += boaWrite(wp, "  '\t</select>\n");
		nBytesSent += boaWrite(wp,  "</tr>");
	}

	return nBytesSent;
}
#endif
#endif


