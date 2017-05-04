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
#include <config/autoconf.h>
#else
#include "../../../../include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

#if defined(CONFIG_IPV6) && defined(IP_PORT_FILTER)
int ipPortFilterListV6(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_V6_IP_PORT_FILTER_T Entry;
	char *dir, *ract;
	char	*type, *ip;
	unsigned char ip6StartStr[INET6_ADDRSTRLEN], ip6EndStr[INET6_ADDRSTRLEN];
	char	ipaddr[100], portRange[20];

	entryNum = mib_chain_total(MIB_V6_IP_PORT_FILTER_TBL);
#ifdef CONFIG_RTK_RG_INIT
	nBytesSent += boaWrite(wp, "<tr>"
	"<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s %s</b></font></td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s %s</b></font></td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
	multilang(LANG_SELECT), multilang(LANG_PROTOCOL),
#else
	nBytesSent += boaWrite(wp, "<tr>"
    "<td align=center width=\"10%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
    "<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
    "<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
    "<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s %s</b></font></td>\n"
    "<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
    "<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s %s</b></font></td>\n"
    "<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
    "<td align=center width=\"15%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
	multilang(LANG_SELECT), multilang(LANG_DIRECTION), multilang(LANG_PROTOCOL),
#endif
#ifdef CONFIG_IPV6_OLD_FILTER
	multilang(LANG_SOURCE), multilang(LANG_IP_ADDRESS), 
#else
	multilang(LANG_SOURCE), multilang(LANG_INTERFACE_ID), 
#endif
	multilang(LANG_SOURCE_PORT),
#ifdef CONFIG_IPV6_OLD_FILTER
	multilang(LANG_DESTINATION), multilang(LANG_IP_ADDRESS), 
#else
	multilang(LANG_DESTINATION), multilang(LANG_INTERFACE_ID), 
#endif
	multilang(LANG_DESTINATION_PORT),
	multilang(LANG_RULE_ACTION));
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_V6_IP_PORT_FILTER_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (Entry.dir == DIR_OUT)
			dir = Toutgoing_ippfilter;
		else
			dir = Tincoming_ippfilter;

		// Modified by Mason Yu for Block ICMP packet
		if ( Entry.protoType == PROTO_ICMP )
		{
			type = (char *)ARG_ICMPV6;
		}
		else if ( Entry.protoType == PROTO_TCP )
			type = (char *)ARG_TCP;
		else
			type = (char *)ARG_UDP;
#if 0
		ip = inet_ntoa(*((struct in_addr *)Entry.srcIp));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(ipaddr, 20, "%s", ip);
			else
				snprintf(ipaddr, 20, "%s/%d", ip, Entry.smaskbit);
			ip = ipaddr;
		}
#endif

#ifdef CONFIG_IPV6_OLD_FILTER
		inet_ntop(PF_INET6, (struct in6_addr *)Entry.sip6Start, ip6StartStr, sizeof(ip6StartStr));
		if ( !strcmp(ip6StartStr, "::"))
			ip = (char *)BLANK;
		else {
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.sip6End, ip6EndStr, sizeof(ip6EndStr));
			if ( !strcmp(ip6EndStr, "::")) {
				snprintf(ipaddr, 100, "%s/%d", ip6StartStr, Entry.sip6PrefixLen);
			}
			else {
				snprintf(ipaddr, 100, "%s-%s/%d", ip6StartStr, ip6EndStr, Entry.sip6PrefixLen);
			}
			ip = ipaddr;
		}
#else
		inet_ntop(PF_INET6, (struct in6_addr *)Entry.sIfId6Start, ip6StartStr, sizeof(ip6StartStr));
		if ( !strcmp(ip6StartStr, "::"))
			ip = (char *)BLANK;
		else {
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.sIfId6End, ip6EndStr, sizeof(ip6EndStr));
			if ( !strcmp(ip6EndStr, "::")) {
				snprintf(ipaddr, 100, "%s", ip6StartStr+2);
			}
			else {
				snprintf(ipaddr, 100, "%s-%s", ip6StartStr+2, ip6EndStr+2);
			}
			ip = ipaddr;
		}
#endif
		
		if ( Entry.srcPortFrom == 0)
			strcpy(portRange, BLANK);
		else if ( Entry.srcPortFrom == Entry.srcPortTo )
			snprintf(portRange, 20, "%d", Entry.srcPortFrom);
		else
			snprintf(portRange, 20, "%d-%d", Entry.srcPortFrom, Entry.srcPortTo);
#ifdef CONFIG_RTK_RG_INIT
	nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",
			i, type, ip, portRange);
#else
		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n",
      			i, dir, type, ip, portRange);
#endif
#if 0
		ip = inet_ntoa(*((struct in_addr *)Entry.dstIp));
		if ( !strcmp(ip, "0.0.0.0"))
			ip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(ipaddr, 20, "%s", ip);
			else
				snprintf(ipaddr, 20, "%s/%d", ip, Entry.dmaskbit);
			ip = ipaddr;
		}
#endif
		
#ifdef CONFIG_IPV6_OLD_FILTER
		inet_ntop(PF_INET6, (struct in6_addr *)Entry.dip6Start, ip6StartStr, sizeof(ip6StartStr));
		if ( !strcmp(ip6StartStr, "::"))
			ip = (char *)BLANK;
		else {
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.dip6End, ip6EndStr, sizeof(ip6EndStr));
			if ( !strcmp(ip6EndStr, "::")) {
				snprintf(ipaddr, 100, "%s/%d", ip6StartStr, Entry.dip6PrefixLen);
			}
			else {
				snprintf(ipaddr, 100, "%s-%s/%d", ip6StartStr, ip6EndStr, Entry.dip6PrefixLen);
			}
			ip = ipaddr;
		}
#else
		inet_ntop(PF_INET6, (struct in6_addr *)Entry.dIfId6Start, ip6StartStr, sizeof(ip6StartStr));
		if ( !strcmp(ip6StartStr, "::"))
			ip = (char *)BLANK;
		else {
			inet_ntop(PF_INET6, (struct in6_addr *)Entry.dIfId6End, ip6EndStr, sizeof(ip6EndStr));
			if ( !strcmp(ip6EndStr, "::")) {
				snprintf(ipaddr, 100, "%s", ip6StartStr+2);
			}
			else {
				snprintf(ipaddr, 100, "%s-%s", ip6StartStr+2, ip6EndStr+2);
			}
			ip = ipaddr;
		}

#endif
		
		if ( Entry.dstPortFrom == 0)
			strcpy(portRange, BLANK);
		else if ( Entry.dstPortFrom == Entry.dstPortTo )
			snprintf(portRange, 20, "%d", Entry.dstPortFrom);
		else
			snprintf(portRange, 20, "%d-%d", Entry.dstPortFrom, Entry.dstPortTo);

		if ( Entry.action == 0 )
			ract = Tdeny_ippfilter;
		else
			ract = Tallow_ippfilter;
		nBytesSent += boaWrite(wp,
			"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
      			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n",
//      			"<td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n"),
				ip, portRange, ract);
	}

	return nBytesSent;
}

void formFilterV6(request * wp, char *path, char *query)
{	
	char *strSetDefaultAction;
	char *strAddIpPort, *strDelIpPort;
	char *strDelAllIpPort, *strVal, *submitUrl, *strComment;	
	unsigned char vChar;
	char tmpBuf[100];	
	unsigned int totalEntry;
	
	memset(tmpBuf,0x00,100);
	
	strSetDefaultAction = boaGetVar(wp, "setDefaultAction", "");
	strAddIpPort = boaGetVar(wp, "addFilterIpPort", "");
	strDelIpPort = boaGetVar(wp, "deleteSelFilterIpPort", "");
	strDelAllIpPort = boaGetVar(wp, "deleteAllFilterIpPort", "");

	totalEntry = mib_chain_total(MIB_V6_IP_PORT_FILTER_TBL); /* get chain record size */

	// delete ALL
	if(strDelAllIpPort[0])
	{
		mib_chain_clear(MIB_V6_IP_PORT_FILTER_TBL); /* clear chain record */
		goto setOk_filter;
	}

	// delete select
	if(strDelIpPort[0])
	{
		unsigned int i;
		unsigned int idx;
		unsigned int deleted = 0;
		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {
				deleted ++;
				if(mib_chain_delete(MIB_V6_IP_PORT_FILTER_TBL, idx) != 1) {
					strcpy(tmpBuf, Tdelete_chain_error);
					goto setErr_filter;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, strNoItemSelectedToDelete);
			goto setErr_filter;
		}
		goto setOk_filter;
	}
	
	// Set IP filtering default action
	if (strSetDefaultAction[0])
	{
		strVal = boaGetVar(wp, "outAct", "");
		if ( strVal[0] ) {
			vChar = strVal[0] - '0';
			mib_set( MIB_V6_IPF_OUT_ACTION, (void *)&vChar);
		}

		strVal = boaGetVar(wp, "inAct", "");
		if ( strVal[0] ) {
			vChar = strVal[0] - '0';
			mib_set( MIB_V6_IPF_IN_ACTION, (void *)&vChar);
		}
		goto setOk_filter;
	}
	
	if (totalEntry >= MAX_FILTER_NUM)
	{
		strcpy(tmpBuf, Texceed_max_rules);
		goto setErr_filter;
	}

	// Add a new rule
	if (strAddIpPort[0] ) {		// IP/Port FILTER
		MIB_CE_V6_IP_PORT_FILTER_T filterEntry;		
		char *strFrom, *strTo;
		int intVal;
		unsigned int totalEntry;
		MIB_CE_V6_IP_PORT_FILTER_T Entry;
		int i;		

		memset(&filterEntry, 0x00, sizeof(filterEntry));	

		// protocol
		strVal = boaGetVar(wp, "protocol", "");

		if (!strVal[0]) {
			strcpy(tmpBuf, Tprotocol_empty);
			goto setErr_filter;
		}

		filterEntry.protoType = strVal[0] - '0';
		
		// source port
		strFrom = boaGetVar(wp, "sfromPort", "");
		strTo = boaGetVar(wp, "stoPort", "");		
		
		if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP){
			strFrom = 0;
		}
		
		if(strFrom!= NULL && strFrom[0])
		{
			int intVal;
			if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, Tinvalid_source_port);
				goto setErr_filter;
			}
			filterEntry.srcPortFrom = (unsigned short)intVal;


			if ( !strTo[0] )
				filterEntry.srcPortTo = filterEntry.srcPortFrom;
			else {
				if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, Tinvalid_source_port);
					goto setErr_filter;
				}
				filterEntry.srcPortTo = (unsigned short)intVal;
			}

			if ( filterEntry.srcPortFrom  > filterEntry.srcPortTo ) {
				strcpy(tmpBuf, Tinvalid_port_range);
				goto setErr_filter;
			}
		}
		
		// destination port
		strFrom = boaGetVar(wp, "dfromPort", "");
		strTo = boaGetVar(wp, "dtoPort", "");
		
		if (filterEntry.protoType != PROTO_TCP && filterEntry.protoType != PROTO_UDP){
			strFrom = 0;
		}
		
		if(strFrom!= NULL && strFrom[0])
		{
			int intVal;
			if ( !string_to_dec(strFrom, &intVal) || intVal<1 || intVal>65535) {
				strcpy(tmpBuf, Tinvalid_destination_port);
				goto setErr_filter;
			}
			filterEntry.dstPortFrom = (unsigned short)intVal;


			if ( !strTo[0] )
				filterEntry.dstPortTo = filterEntry.dstPortFrom;
			else {
				if ( !string_to_dec(strTo, &intVal) || intVal<1 || intVal>65535) {
					strcpy(tmpBuf, Tinvalid_destination_port);
					goto setErr_filter;
				}
				filterEntry.dstPortTo = (unsigned short)intVal;
			}

			if ( filterEntry.dstPortFrom  > filterEntry.dstPortTo ) {
				strcpy(tmpBuf, Tinvalid_port_range);
				goto setErr_filter;
			}
		}
		
#ifdef CONFIG_IPV6_OLD_FILTER
		// Source IP
		strVal = boaGetVar(wp, "sip6Start", "");
		if (strVal[0]) {
			if (!inet_pton(PF_INET6, strVal, &filterEntry.sip6Start)) {
				sprintf(tmpBuf, "%s (sip6start)", strInvalidValue);
				goto setErr_filter;
			}
		}
		
		strVal = boaGetVar(wp, "sip6End", "");
		if (strVal[0]) {
			if (!inet_pton(PF_INET6, strVal, &filterEntry.sip6End)) {
				sprintf(tmpBuf, "%s (sip6end)", strInvalidValue);
				goto setErr_filter;
			}
		}
		
		// destination IP
		strVal = boaGetVar(wp, "dip6Start", "");
		if (strVal[0]) {
			if (!inet_pton(PF_INET6, strVal, &filterEntry.dip6Start)) {
				sprintf(tmpBuf, "%s (dip6start)", strInvalidValue);
				goto setErr_filter;
			}
		}
		
		strVal = boaGetVar(wp, "dip6End", "");
		if (strVal[0]) {
			if (!inet_pton(PF_INET6, strVal, &filterEntry.dip6End)) {
				sprintf(tmpBuf, "%s (dip6end)", strInvalidValue);
				goto setErr_filter;
			}
		}
		
		// source PrefixLen
		strVal = boaGetVar(wp, "sip6PrefixLen", "");
		filterEntry.sip6PrefixLen = (char)atoi(strVal);
		
		// destination PrefixLen
		strVal = boaGetVar(wp, "dip6PrefixLen", "");
		filterEntry.dip6PrefixLen = (char)atoi(strVal);
#else

		// Source IfId
		strVal = boaGetVar(wp, "sIfId6Start", "");
		if (strVal[0]) {
			snprintf(tmpBuf,sizeof(tmpBuf),"::%s",strVal);
			if (!inet_pton(PF_INET6, tmpBuf, &filterEntry.sIfId6Start)) {
				sprintf(tmpBuf, "%s (dip6start)", strInvalidValue);
				goto setErr_filter;
			}
		}
		
		strVal = boaGetVar(wp, "sIfId6End", "");
		if (strVal[0]) {
			snprintf(tmpBuf,sizeof(tmpBuf),"::%s",strVal);
			if (!inet_pton(PF_INET6, tmpBuf, &filterEntry.sIfId6End)) {
				sprintf(tmpBuf, "%s (dip6end)", strInvalidValue);
				goto setErr_filter;
			}
		}
		
		// destination IfId
		strVal = boaGetVar(wp, "dIfId6Start", "");
		if (strVal[0]) {
			snprintf(tmpBuf,sizeof(tmpBuf),"::%s",strVal);
			if (!inet_pton(PF_INET6, tmpBuf, &filterEntry.dIfId6Start)) {
				sprintf(tmpBuf, "%s (dip6start)", strInvalidValue);
				goto setErr_filter;
			}
		}
		
		strVal = boaGetVar(wp, "dIfId6End", "");
		if (strVal[0]) {
			snprintf(tmpBuf,sizeof(tmpBuf),"::%s",strVal);
			if (!inet_pton(PF_INET6, tmpBuf, &filterEntry.dIfId6End)) {
				sprintf(tmpBuf, "%s (dip6end)", strInvalidValue);
				goto setErr_filter;
			}
		}
#endif
				
		strVal = boaGetVar(wp, "filterMode", "");
		if ( strVal[0] ) {
			if (!strcmp(strVal, "Deny"))
				filterEntry.action = 0;
			else if (!strcmp(strVal, "Allow"))
				filterEntry.action = 1;
			else {
				strcpy(tmpBuf, Tinvalid_rule_action);
				goto setErr_filter;
			}
		}

		strVal = boaGetVar(wp, "dir", "");
		if(strVal[0])
			filterEntry.dir = strVal[0]-'0';		

		intVal = mib_chain_add(MIB_V6_IP_PORT_FILTER_TBL, (unsigned char*)&filterEntry);
		if (intVal == 0) {
			strcpy(tmpBuf, Tadd_chain_error);
			goto setErr_filter;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_filter;
		}

	}
	
setOk_filter:
//	Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif	
	
	restart_IPFilter_DMZ_MACFilter();
	
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

#endif
