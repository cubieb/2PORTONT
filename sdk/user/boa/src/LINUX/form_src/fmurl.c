/*
 *      Web server handler routines for URL stuffs
 */
#include "options.h"
#ifdef URL_BLOCKING_SUPPORT

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
#define  URL_MAX_ENTRY  500
#define  KEY_MAX_ENTRY  500
///////////////////////////////////////////////////////////////////

#ifdef URL_BLOCKING_SUPPORT
void formURL(request * wp, char *path, char *query)
{
	char	*str, *submitUrl, *strVal;
	char tmpBuf[100];
	unsigned char urlcap;
#ifndef NO_ACTION
	int pid;
#endif

	// Set URL Capability
	str = boaGetVar(wp, "apply", "");
	if (str[0]) {
		str = boaGetVar(wp, "urlcap", "");
		if (str[0]) {
			if (str[0] == '0')
				urlcap = 0;
			else if(str[0] == '1')
				{
				urlcap = 1;
				}
#ifdef  URL_ALLOWING_SUPPORT
			else if(str[0]=='2')
				{
				   urlcap=2 ;
				}
#endif
			if ( !mib_set(MIB_URL_CAPABILITY, (void *)&urlcap)) {
				strcpy(tmpBuf, multilang(LANG_SET_URL_CAPABILITY_ERROR));
				goto setErr_route;
			}
		}
		goto  setOk_route;
 	}

	// Delete all FQDN
	str = boaGetVar(wp, "delFAllQDN", "");
	if (str[0]) {
		mib_chain_clear(MIB_URL_FQDN_TBL); /* clear chain record */
		goto setOk_route;
	}

	/* Delete selected FQDN */
	str = boaGetVar(wp, "delFQDN", "");
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_URL_FQDN_TBL); /* get chain record size */
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {
				deleted ++;
				if(mib_chain_delete(MIB_URL_FQDN_TBL, idx) != 1) {
					strcpy(tmpBuf, Tdelete_chain_error);
					goto setErr_route;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, multilang(LANG_PLEASE_SELECT_AN_ENTRY_TO_DELETE));
			goto setErr_route;
		}

		goto setOk_route;
	}

#if 0
	// Delete FQDN
	str = boaGetVar(wp, "delFQDN", "");
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_URL_FQDN_TBL); /* get chain record size */

		str = boaGetVar(wp, "select", "");

		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);

				if ( !gstrcmp(str, tmpBuf) ) {
					// delete from chain record
					if(mib_chain_delete(MIB_URL_FQDN_TBL, idx) != 1) {
						strcpy(tmpBuf, "Delete URL chain record error!");
						goto setErr_route;
					}
				}
			} // end of for
		}
		goto setOk_route;
	}
#endif

	// Add FQDN
	str = boaGetVar(wp, "addFQDN", "");
	if (str[0]) {
		MIB_CE_URL_FQDN_T entry;
		int i, intVal;
		unsigned int totalEntry = mib_chain_total(MIB_URL_FQDN_TBL); /* get chain record size */
		if((totalEntry+1)>(URL_MAX_ENTRY))
		{
		   strcpy(tmpBuf, TMaxUrl);
		   goto setErr_route;
		}
		str = boaGetVar(wp, "urlFQDN", "");
//		printf("str = %s\n", str);
		for (i = 0 ; i< totalEntry;i++)	{
			if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&entry)){
				strcpy(tmpBuf, errGetEntry);
				goto setErr_route;
			}
			if(!strcmp(entry.fqdn,str)){
				strcpy(tmpBuf, TstrUrlExist );
				goto setErr_route;
			}
		}

		// add into configuration (chain record)
		strcpy(entry.fqdn, str);

		intVal = mib_chain_add(MIB_URL_FQDN_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//boaWrite(wp, "%s", "Error: Add URL chain record.");
			//return;
			strcpy(tmpBuf, multilang(LANG_ERROR_ADD_URL_CHAIN_RECORD));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_route;
		}
		goto setOk_route;
	}
#ifdef URL_ALLOWING_SUPPORT
	//add allow fqdn
	str = boaGetVar(wp, "addallowFQDN", "");
	if (str[0]) {
		MIB_CE_URL_ALLOW_FQDN_T entry;
		int i, intVal ;
		unsigned int totalEntry = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL); /* get chain record size */
		if((totalEntry+1)>(URL_MAX_ENTRY))
		{
			strcpy(tmpBuf, TMaxUrl);
			goto setErr_route;
		}
		str = boaGetVar(wp, "urlFQDNALLOW", "");

		for (i = 0 ; i< totalEntry;i++)	{
			if (!mib_chain_get(MIB_URL_ALLOW_FQDN_TBL, i, (void *)&entry)){
				strcpy(tmpBuf, errGetEntry);
				goto setErr_route;
			}
			if(!strcmp(entry.fqdn,str)){
				strcpy(tmpBuf, TstrUrlExist );
				goto setErr_route;
			}
		}

		// add into configuration (chain record)
		strcpy(entry.fqdn, str);

		intVal = mib_chain_add(MIB_URL_ALLOW_FQDN_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//boaWrite(wp, "%s", "Error: Add URL chain record.");
			//return;
			strcpy(tmpBuf, multilang(LANG_ERROR_ADD_URL_CHAIN_RECORD));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_route;
		}
	     goto setOk_route;
	}

       // Delete allowFQDN
	str = boaGetVar(wp, "delallowFQDN", "");
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL); /* get chain record size */

		str = boaGetVar(wp, "selectallow", "");

		if (str[0]) {
			printf("delallowFQDN\n");
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);

				if ( !gstrcmp(str, tmpBuf) ) {
					// delete from chain record
					if(mib_chain_delete(MIB_URL_ALLOW_FQDN_TBL, idx) != 1) {
						strcpy(tmpBuf, multilang(LANG_DELETE_URL_CHAIN_RECORD_ERROR));
						goto setErr_route;
					}
				}
			} // end of for
		}
		goto setOk_route;
	}

#endif

	// Delete all Keyword
	str = boaGetVar(wp, "delAllKeywd", "");
	if (str[0]) {
		mib_chain_clear(MIB_KEYWD_FILTER_TBL); /* clear chain record */
		goto setOk_route;
	}

	/* Delete selected Keyword */
	str = boaGetVar(wp, "delKeywd", "");
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_KEYWD_FILTER_TBL); /* get chain record size */
		unsigned int deleted = 0;

		for (i=0; i<totalEntry; i++) {

			idx = totalEntry-i-1;
			snprintf(tmpBuf, 20, "select%d", idx);
			strVal = boaGetVar(wp, tmpBuf, "");

			if ( !gstrcmp(strVal, "ON") ) {
				deleted ++;
				if(mib_chain_delete(MIB_KEYWD_FILTER_TBL, idx) != 1) {
					strcpy(tmpBuf, multilang(LANG_DELETE_URL_CHAIN_RECORD_ERROR));
					goto setErr_route;
				}
			}
		}
		if (deleted <= 0) {
			strcpy(tmpBuf, multilang(LANG_PLEASE_SELECT_AN_ENTRY_TO_DELETE));
			goto setErr_route;
		}

		goto setOk_route;
	}
#if 0
	// Delete Keyword
	str = boaGetVar(wp, "delKeywd", "");
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		unsigned int totalEntry = mib_chain_total(MIB_KEYWD_FILTER_TBL); /* get chain record size */

		str = boaGetVar(wp, "select", "");

		if (str[0]) {
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry-i-1;
				snprintf(tmpBuf, 4, "s%d", idx);

				if ( !gstrcmp(str, tmpBuf) ) {
					// delete from chain record
					if(mib_chain_delete(MIB_KEYWD_FILTER_TBL, idx) != 1) {
						strcpy(tmpBuf, "Delete Keyword filter chain record error!");
						goto setErr_route;
					}
				}
			} // end of for
		}
		goto setOk_route;
	}
#endif

	// Add keyword
	str = boaGetVar(wp, "addKeywd", "");
	if (str[0]) {
		MIB_CE_KEYWD_FILTER_T entry;
		int i, intVal;
		unsigned int totalEntry = mib_chain_total(MIB_KEYWD_FILTER_TBL); /* get chain record size */
		if((totalEntry+1)>(KEY_MAX_ENTRY))
		{
			strcpy(tmpBuf, TMaxKey);
			goto setErr_route;
		}
		str = boaGetVar(wp, "Keywd", "");
		//	printf("str = %s\n", str);
		for (i = 0 ; i< totalEntry;i++)	{
			if (!mib_chain_get(MIB_KEYWD_FILTER_TBL,i, (void *)&entry)){
				strcpy(tmpBuf, errGetEntry);
				goto setErr_route;
			}
			if(!strcmp(entry.keyword, str)){
				strcpy(tmpBuf, TstrKeyExist);
				goto setErr_route;
			}
		}

		// add into configuration (chain record)
		strcpy(entry.keyword, str);

		intVal = mib_chain_add(MIB_KEYWD_FILTER_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			//boaWrite(wp, "%s", "Error: Add Keyword filtering chain record.");
			//return;
			strcpy(tmpBuf, multilang(LANG_ERROR_ADD_KEYWORD_FILTERING_CHAIN_RECORD));
			goto setErr_route;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_route;
		}
	}
setOk_route:
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef URL_ALLOWING_SUPPORT
	restart_url();
#else
	restart_urlblocking();
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
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_route:
	ERR_MSG(tmpBuf);
}

int showURLTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_URL_FQDN_T Entry;

	entryNum = mib_chain_total(MIB_URL_FQDN_TBL);
	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">%s</td></font></tr>\n",
	multilang(LANG_SELECT), multilang(LANG_FQDN));


	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get URL chain record error!\n");
			return;
		}

		nBytesSent += boaWrite(wp, "<tr>"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\"></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
		"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
		i, Entry.fqdn);
	}

	return 0;
}
#endif

#ifdef URL_ALLOWING_SUPPORT
int showURLALLOWTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_URL_ALLOW_FQDN_T Entry;


	entryNum = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL);
	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">Select</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">FQDN</td></font></tr>\n");


	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_URL_ALLOW_FQDN_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get URL chain record error!\n");
			return;
		}

		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"selectallow\""
		" value=\"s%d\"></td>\n"
		"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
		i, Entry.fqdn);
	}

	return 0;
}

#endif

int showKeywdTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_KEYWD_FILTER_T Entry;

	entryNum = mib_chain_total(MIB_KEYWD_FILTER_TBL);
	nBytesSent += boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"5%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"35%%\" bgcolor=\"#808080\">%s</td></font></tr>\n",
	multilang(LANG_SELECT), multilang(LANG_FILTERED_KEYWORD));

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_KEYWD_FILTER_TBL, i, (void *)&Entry))
		{
			boaError(wp, 400, "Get Keyword filter chain record error!\n");
			return;
		}

		nBytesSent += boaWrite(wp, "<tr>"
//		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\"></td>\n"
		"<td align=center width=\"5%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td>\n"
		"<td align=center width=\"35%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
		i, Entry.keyword);
	}

	return 0;
}
#endif

