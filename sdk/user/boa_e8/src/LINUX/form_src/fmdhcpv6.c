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

#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
void formDhcpv6(request * wp, char *path, char *query)
{
	char *submitUrl, *str, *str_prefix, *str_min, *str_max;
	static char tmpBuf[100];
	unsigned char origvChar,vChar;
	unsigned int origInt;
	int dhcpd_changed_flag=0;
	char *strdhcpenable;
	unsigned char mode;
	DHCP_T curDhcp;
	unsigned char prefixLen=0;
	struct in6_addr ip6Addr_prefix, ip6Addr_start, ip6Addr_end;
	int dhcpd6pid;

	// stop dhcpd
	start_dhcpv6(0);
	strdhcpenable = boaGetVar(wp, "dhcpdenable", "");
	mib_get( MIB_DHCPV6_MODE, (void *)&origvChar);

	if(strdhcpenable[0])
	{
		sscanf(strdhcpenable, "%u", &origInt);
		mode = (unsigned char)origInt;
		if(mode!=origvChar)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_DHCPV6_MODE, (void *)&mode)) {
  			strcpy(tmpBuf, strSetDhcpModeerror);
			goto setErr_dhcpv6;
		}
	}

	if ( !mib_get( MIB_DHCPV6_MODE, (void *)&origvChar) ) {
		strcpy(tmpBuf, strGetDhcpModeerror);
		goto setErr_dhcpv6;
	}
	curDhcp = (DHCP_T) origvChar;

	if ( curDhcp == DHCP_LAN_SERVER_AUTO ) {
		str = boaGetVar(wp, "save", "");
		if (str[0]) {
			str_min = boaGetVar(wp, "dhcpRangeStart", "");
			str_max = boaGetVar(wp, "dhcpRangeEnd", "");

			printf("[%s:%d] min %s, max %s\n",__func__,__LINE__,str_min,str_max);
			//TODO: check if the start, end IP range could be valid IPv6 Address
			mib_set(MIB_DHCPV6S_MIN_ADDRESS, (void *)str_min);
			mib_set(MIB_DHCPV6S_MAX_ADDRESS, (void *)str_max);

		} // of save
	}

#if 0 //Iulian Modify for E8 request 
	if ( curDhcp == DHCP_LAN_SERVER ) {
		unsigned int DLTime, PFTime, RNTime, RBTime;

		str = boaGetVar(wp, "save", "");
		if (str[0]) {
			str = boaGetVar(wp, "dhcpRangeStart", "");
			if (!inet_pton(PF_INET6, str, &ip6Addr_start)) {
				strcpy(tmpBuf, Tinvalid_start_ip);
				goto setErr_dhcpv6;
			}

			str = boaGetVar(wp, "dhcpRangeEnd", "");
			if (!inet_pton(PF_INET6, str, &ip6Addr_end)) {
				strcpy(tmpBuf, Tinvalid_end_ip);
				goto setErr_dhcpv6;
			}

			str = boaGetVar(wp, "prefix_len", "");
			prefixLen = (char)atoi(str);

			str = boaGetVar(wp, "Dltime", "");
			sscanf(str, "%u", &DLTime);

			str = boaGetVar(wp, "PFtime", "");
			sscanf(str, "%u", &PFTime);

			str = boaGetVar(wp, "RNtime", "");
			sscanf(str, "%u", &RNTime);

			str = boaGetVar(wp, "RBtime", "");
			sscanf(str, "%u", &RBTime);

			str = boaGetVar(wp, "clientID", "");

			// Everything is ok, so set it.
			mib_set(MIB_DHCPV6S_RANGE_START, (void *)&ip6Addr_start);
			mib_set(MIB_DHCPV6S_RANGE_END, (void *)&ip6Addr_end);
			mib_set(MIB_DHCPV6S_PREFIX_LENGTH, (char *)&prefixLen);
			mib_set(MIB_DHCPV6S_DEFAULT_LEASE, (void *)&DLTime);
			mib_set(MIB_DHCPV6S_PREFERRED_LIFETIME, (void *)&PFTime);
			mib_set(MIB_DHCPV6S_RENEW_TIME, (void *)&RNTime);
			mib_set(MIB_DHCPV6S_REBIND_TIME, (void *)&RBTime);
			mib_set(MIB_DHCPV6S_CLIENT_DUID, (void *)str);
		} // of save

		// Delete all Name Server
		str = boaGetVar(wp, "delAllNameServer", "");
		if (str[0]) {
			mib_chain_clear(MIB_DHCPV6S_NAME_SERVER_TBL); /* clear chain record */
			goto setOk_dhcpv6;
		}

		/* Delete selected Name Server */
		str = boaGetVar(wp, "delNameServer", "");
		if (str[0]) {
			unsigned int i;
			unsigned int idx;
			unsigned int totalEntry = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL); /* get chain record size */
			unsigned int deleted = 0;

			for (i=0; i<totalEntry; i++) {

				idx = totalEntry-i-1;
				snprintf(tmpBuf, 20, "select%d", idx);
				strVal = boaGetVar(wp, tmpBuf, "");

				if ( !gstrcmp(strVal, "ON") ) {
					deleted ++;
					if(mib_chain_delete(MIB_DHCPV6S_NAME_SERVER_TBL, idx) != 1) {
						strcpy(tmpBuf, "删除失败!"); //Delete chain record error!
						goto setErr_dhcpv6;
					}
				}
			}
			if (deleted <= 0) {
				strcpy(tmpBuf, "没有选择删除的项目(MIB_DHCPV6S_NAME_SERVER_TBL)!"); //There is no item selected to delete
				goto setErr_dhcpv6;
			}

			goto setOk_dhcpv6;
		}

		// Add Name server
		str = boaGetVar(wp, "addNameServer", "");
		if (str[0]) {
			MIB_DHCPV6S_NAME_SERVER_T entry;
			int i, intVal;
			unsigned int totalEntry = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL); /* get chain record size */

			str = boaGetVar(wp, "nameServerIP", "");
			//printf("formDOMAINBLK:(Add) str = %s\n", str);
			// Jenny, check duplicated rule
			for (i = 0; i< totalEntry; i++) {
				if (!mib_chain_get(MIB_DHCPV6S_NAME_SERVER_TBL, i, (void *)&entry)) {
					strcpy(tmpBuf, errGetEntry);
					goto setErr_dhcpv6;
				}
				if (!strcmp(entry.nameServer, str)) {
					strcpy(tmpBuf, strMACInList );
					goto setErr_dhcpv6;
				}
			}

			// add into configuration (chain record)
			strcpy(entry.nameServer, str);

			intVal = mib_chain_add(MIB_DHCPV6S_NAME_SERVER_TBL, (unsigned char*)&entry);
			if (intVal == 0) {
				//boaWrite(wp, "%s", "Error: Add Domain Blocking chain record.");
				//return;
				strcpy(tmpBuf, "错误: 加入 Name Server chain record for DHCPv6."); //Error: Add Name Server chain record for DHCPv6.
				goto setErr_dhcpv6;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, strTableFull);
				goto setErr_dhcpv6;
			}

		}

		// Delete all Domain
		str = boaGetVar(wp, "delAllDomain", "");
		if (str[0]) {
			mib_chain_clear(MIB_DHCPV6S_DOMAIN_SEARCH_TBL); /* clear chain record */
			goto setOk_dhcpv6;
		}

		/* Delete selected Domain */
		str = boaGetVar(wp, "delDomain", "");
		if (str[0]) {
			unsigned int i;
			unsigned int idx;
			unsigned int totalEntry = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL); /* get chain record size */
			unsigned int deleted = 0;

			for (i=0; i<totalEntry; i++) {

				idx = totalEntry-i-1;
				snprintf(tmpBuf, 20, "select%d", idx);
				strVal = boaGetVar(wp, tmpBuf, "");

				if ( !gstrcmp(strVal, "ON") ) {
					deleted ++;
					if(mib_chain_delete(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, idx) != 1) {
						strcpy(tmpBuf, "删除失败!"); //Delete chain record error!
						goto setErr_dhcpv6;
					}
				}
			}
			if (deleted <= 0) {
				strcpy(tmpBuf, "没有选择删除的项目(MIB_DHCPV6S_DOMAIN_SEARCH_TBL)!"); //There is no item selected to delete
				goto setErr_dhcpv6;
			}

			goto setOk_dhcpv6;
		}

		// Add doamin
		str = boaGetVar(wp, "addDomain", "");
		if (str[0]) {
			MIB_DHCPV6S_DOMAIN_SEARCH_T entry;
			int i, intVal;
			unsigned int totalEntry = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL); /* get chain record size */

			str = boaGetVar(wp, "domainStr", "");
			//printf("formDOMAINBLK:(Add) str = %s\n", str);
			// Jenny, check duplicated rule
			for (i = 0; i< totalEntry; i++) {
				if (!mib_chain_get(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, i, (void *)&entry)) {
					strcpy(tmpBuf, errGetEntry);
					goto setErr_dhcpv6;
				}
				if (!strcmp(entry.domain, str)) {
					strcpy(tmpBuf, strMACInList );
					goto setErr_dhcpv6;
				}
			}

			// add into configuration (chain record)
			strcpy(entry.domain, str);

			intVal = mib_chain_add(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, (unsigned char*)&entry);
			if (intVal == 0) {
				//boaWrite(wp, "%s", "Error: Add Domain Blocking chain record.");
				//return;
				strcpy(tmpBuf, "错误: 加入 Domain chain record for DHCPv6."); //Error: Add Domain chain record for DHCPv6.
				goto setErr_dhcpv6;
			}
			else if (intVal == -1) {
				strcpy(tmpBuf, strTableFull);
				goto setErr_dhcpv6;
			}

		}

	}
	else if( curDhcp == DHCP_LAN_RELAY ){
		unsigned int upper_if;

		str = boaGetVar(wp, "upper_if", "");
		upper_if = (unsigned int)atoi(str);

		if ( !mib_set(MIB_DHCPV6R_UPPER_IFINDEX, (void *)&upper_if)) {
			strcpy(tmpBuf, "设定 MIB_DHCPV6R_UPPER_IFINDEX mib 错误!"); //Set MIB_DHCPV6R_UPPER_IFINDEX mib error!
			goto setErr_dhcpv6;
		}

	}
#endif
setOk_dhcpv6:
	// start dhcpd

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	mib_get(MIB_DHCPV6_MODE, (void *)&vChar);
	dhcpd6pid = read_pid((char *) DHCPSERVER6PID);
	if ( vChar==DHCP_LAN_NONE ){ //disable DHCPv6 server
		if(dhcpd6pid>0){
			kill(dhcpd6pid, SIGTERM);
			unlink(DHCPSERVER6PID);
		}
	}
	else
		restartDHCPV6Server();

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
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  return;

setErr_dhcpv6:
	ERR_MSG(tmpBuf);
}


int showDhcpv6SNameServerTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_DHCPV6S_NAME_SERVER_T Entry;
	unsigned long int d,g,m;
	unsigned char sdest[MAX_V6_IP_LEN];

	entryNum = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL);

	nBytesSent += boaWrite(wp, "'<tr><font size=1>'+"
	"'<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>'+\n"
	"'<td align=center width=\"35%%\" bgcolor=\"#808080\">Name Server</td></font></tr>'+\n");


	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_DHCPV6S_NAME_SERVER_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "读取Name Server chain record错误!\n"); //Get Name Server chain record error!
			return;
		}

		strncpy(sdest, Entry.nameServer, strlen(Entry.nameServer));
		sdest[strlen(Entry.nameServer)] = '\0';

		nBytesSent += boaWrite(wp, "'<tr>'+"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\"></td>\n"
		"'<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>'+\n"
		"'<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>'+\n",
		i, sdest);
	}

	return 0;
}

int showDhcpv6SDOMAINTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_DHCPV6S_DOMAIN_SEARCH_T Entry;
	unsigned long int d,g,m;
	unsigned char sdest[MAX_DOMAIN_LENGTH];

	entryNum = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL);

	nBytesSent += boaWrite(wp, "'<tr><font size=1>'+"
	"'<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>'+\n"
	"'<td align=center width=\"35%%\" bgcolor=\"#808080\">Domain</td></font></tr>'+\n");


	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "读取Domain search chain record错误!\n"); //Get Domain search chain record error!
			return;
		}

		strncpy(sdest, Entry.domain, strlen(Entry.domain));
		sdest[strlen(Entry.domain)] = '\0';

		nBytesSent += boaWrite(wp, "'<tr>'+"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\"></td>\n"
		"'<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>'+\n"
		"'<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>'+\n",
		i, sdest);
	}

	return 0;
}
#endif
#endif  //#ifdef CONFIG_IPV6

