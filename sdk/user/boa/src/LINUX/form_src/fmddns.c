/*
 *      Web server handler routines for ACL stuffs
 *
 */



/*-- System inlcude files --*/
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "multilang.h"

#define	DDNS_ADD	0
#define DDNS_MODIFY	1

#ifdef CONFIG_USER_DDNS
static void getEntry(request * wp, MIB_CE_DDNS_Tp pEntry)
{
	char	*str;
	unsigned long ddns_if, if_num;	// Mason Yu. Specify IP Address
	char ifname[IFNAMSIZ];
	
	memset(pEntry, 0, sizeof(MIB_CE_DDNS_T));

	str = boaGetVar(wp, "ddnsProv", "");
	if ( str[0]=='0' ) {
		strcpy(pEntry->provider, "dyndns");
	} else if ( str[0]=='1') {
		strcpy(pEntry->provider, "tzo");
	} else if ( str[0]=='2') {
		strcpy(pEntry->provider, "noip");
	} else
		printf("Updatedd not support this provider!!\n");
	//printf("pEntry->provider = %s\n",pEntry->provider);

	str = boaGetVar(wp, "hostname", "");
	if (str[0]) {
		strcpy(pEntry->hostname, str);
	}
	//printf("pEntry->hostname = %s\n", pEntry->hostname);

	str = boaGetVar(wp, "interface", "");
	if (str[0]) {
		// Mason Yu. Specify IP Address
		ddns_if = (unsigned long)atoi(str);

		if ( ddns_if == 100 ) {
			strcpy(pEntry->interface, BRIF);
		} else if ( ddns_if == DUMMY_IFINDEX ) {
			strcpy(pEntry->interface, "Any");
		} else {			
			ifGetName( ddns_if, ifname, sizeof(ifname));
			strcpy(pEntry->interface, ifname);			
		}

		// Mason Yu. Specify IP Address
		//strcpy(pEntry->interface, str);
		//printf("pEntry->interface= %s\n", pEntry->interface);
	}

	if ( strcmp(pEntry->provider, "dyndns") == 0 || strcmp(pEntry->provider, "noip") == 0 ) {
		str = boaGetVar(wp, "username", "");
		if (str[0]) {
			strcpy(pEntry->username, str);
		}
		//printf("pEntry->username = %s\n", pEntry->username);

		str = boaGetVar(wp, "password", "");
		if (str[0]) {
			strcpy(pEntry->password, str);
		}
		//printf("pEntry->password = %s\n", pEntry->password);

	} else if ( strcmp(pEntry->provider, "tzo") == 0 ) {
		str = boaGetVar(wp, "email", "");
		if (str[0]) {
			//strcpy(pEntry->email, str);
			strcpy(pEntry->username, str);
		}
		//printf("email = %s\n", pEntry->username);

		str = boaGetVar(wp, "key", "");
		if (str[0]) {
			//strcpy(pEntry->key, str);
			strcpy(pEntry->password, str);
		}
		//printf("key = %s\n", pEntry->password);

	} else
		printf("Please choose the correct provider!!!\n");

	str = boaGetVar(wp, "enable", "");
	if ( str && str[0] ) {
		pEntry->Enabled = 1;
	}
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
static int checkEntry(MIB_CE_DDNS_Tp pEntry, int type, char *pMsg, int selected)
{
	MIB_CE_DDNS_T tmpEntry;
	int num, i;

	num = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */
	// check duplication
	for (i=0; i<num; i++) {
		mib_chain_get(MIB_DDNS_TBL, i, (void *)&tmpEntry);
		if (type == DDNS_MODIFY) { // modify
			if (selected == i)
				continue;
		}
		// Mason Yu
		if (gstrcmp(pEntry->provider, tmpEntry.provider))
			continue;
		if (gstrcmp(pEntry->interface, tmpEntry.interface))
			continue;
		if (gstrcmp(pEntry->hostname, tmpEntry.hostname))
			continue;
		
		// entry duplication
		strcpy(pMsg, multilang(LANG_ENTRY_ALREADY_EXISTS));
		return -1;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////
void formDDNS(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100], ifname[6];
	unsigned int totalEntry, selected, i, idx;
	MIB_CE_DDNS_T entry;
//	unsigned char ddns_if, if_num;
#ifndef NO_ACTION
	int pid;
#endif

	// Mason Yu. Support ddns status file.
	remove_ddns_status();
	stop_all_ddns();
	
	totalEntry = mib_chain_total(MIB_DDNS_TBL); /* get chain record size */

	// Remove
	str = boaGetVar(wp, "delacc", "");
	if (str[0]) {
		str = boaGetVar(wp, "select", "");
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);
				//printf("tmpBuf(select) = %s idx=%d\n", tmpBuf, idx);

				if ( !gstrcmp(str, tmpBuf) ) {
					// delete from chain record
					if(mib_chain_delete(MIB_DDNS_TBL, idx) != 1) {
						strcpy(tmpBuf, Tdelete_chain_error);
						goto setErr_route;
					}
				}
			} // end of for
		}
		goto setOk_route;
	}

	// Add
	str = boaGetVar(wp, "addacc", "");
	if (str[0]) {
		MIB_CE_DDNS_T tmpEntry;
		int intVal;

		getEntry(wp, &entry);
		if (checkEntry(&entry, DDNS_ADD, &tmpBuf[0], 0xffff) == -1)
			goto setErr_route;

		intVal = mib_chain_add(MIB_DDNS_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//boaWrite(wp, "%s", "Error: Add chain(MIB_DDNS_TBL) record.");
			//return;
			sprintf(tmpBuf, "%s (MIB_DDNS_TBL)",Tadd_chain_error);
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_route;
		}

	}

	// Modify
	str = boaGetVar(wp, "modify", "");
	if (str[0]) {
		selected = -1;
		str = boaGetVar(wp, "select", "");
		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);

				if ( !gstrcmp(str, tmpBuf) ) {
					selected = idx;
					break;
				}
			}
			if (selected >= 0) {
				getEntry(wp, &entry);
				if (checkEntry(&entry, DDNS_MODIFY, &tmpBuf[0], selected) == -1)
					goto setErr_route;

				mib_chain_update(MIB_DDNS_TBL, (void *)&entry, selected);
			}
		}
	}

setOk_route:

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	restart_ddns();

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

setErr_route:
	ERR_MSG(tmpBuf);
}

int showDNSTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum, i;
	MIB_CE_DDNS_T Entry;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16];
	int status;
	char *statusStr;
	FILE *fp;
	unsigned char filename[100];
	unsigned int ifIndex;
	
	entryNum = mib_chain_total(MIB_DDNS_TBL);

	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">%s</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s%s</td>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\">%s</td>"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\">%s</td>"
//	"<td align=center width=\"20%%\" bgcolor=\"#808080\">Interface</td>"
	"</font></tr>\n", multilang(LANG_SELECT), multilang(LANG_STATE),
	multilang(LANG_HOSTNAME), multilang(LANG_USER), multilang(LANG_NAME), multilang(LANG_SERVICE), multilang(LANG_STATUS));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_DDNS_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return;
		}

		// open a status file
		status = LINK_DOWN;
		sprintf(filename, "/var/%s.%s.%s.txt", Entry.provider, Entry.username, Entry.password);					
		fp=fopen(filename,"r"); 
		if (fp) {
			fscanf(fp,"%d\n",&status);			
			fclose(fp);
		}
		
		if (status == SUCCESSFULLY_UPDATED)
			statusStr = "Successfully updated";
		else if (status == CONNECTION_ERROR)
			statusStr = "Connection error";
		else if (status == AUTH_FAILURE)
			statusStr = "Authentication failure";
		else if (status == WRONG_OPTION)
			statusStr = "Wrong option";			
		else if (status == HANDLING)
			statusStr = "Handling DDNS request packet";
		else if (status == LINK_DOWN)
			statusStr = "Cannot connecting to provider";		
		
		ifIndex = getIfIndexByName(Entry.interface);
		if (ifIndex == -1)
			ifIndex = DUMMY_IFINDEX;
		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
		" value=\"s%d\" onClick=postEntry(%d,'%s','%s','%s','%s',%d)></td>\n",
		i, Entry.Enabled, Entry.provider, Entry.hostname, Entry.username, Entry.password, ifIndex);
		nBytesSent += boaWrite(wp,
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
//		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>"
		"</tr>\n",
		//Entry.hostname, strcmp(Entry.username, "")==0?Entry.email:Entry.username, Entry.provider, Entry.interface);
//		Entry.Enabled ? "Enable" : "Disable", Entry.hostname, Entry.username, Entry.provider, Entry.interface);
		multilang(Entry.Enabled ? LANG_ENABLE : LANG_DISABLE), Entry.hostname, Entry.username, Entry.provider, statusStr);

	}
	return 0;
}
#endif
