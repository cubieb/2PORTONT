/*
 *      Web server handler routines for Ethernet Link Mode stuffs
 *
 */


/*-- System inlcude files --*/
#include <net/if.h>
#include <signal.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif
#include "options.h"

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
#ifdef ELAN_LINK_MODE
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

///////////////////////////////////////////////////////////////////
void formLink(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
	char p_str[]="p0";
	int k, mode;
	MIB_CE_SW_PORT_T Entry;

#ifndef NO_ACTION
	int pid;
#endif
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	for (k=0; k<SW_LAN_PORT_NUM; k++) {
		p_str[1] = '0' + k;
		str = boaGetVar(wp, p_str, "");
		mode = str[0] - '0';
		if (mode < LINK_10HALF || mode > LINK_AUTO) {
			strcpy(tmpBuf, multilang(LANG_INVALID_LINK_MODE_VALUE));
			goto setErr_link;
		}

		if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Entry)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_link;
		}
		Entry.linkMode = mode;
		// log message
		mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, k);
	}
#if defined(APPLY_CHANGE)
	setupLinkMode();
#endif

	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

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

setErr_link:
	ERR_MSG(tmpBuf);
}

int show_lanport(int eid, request * wp, int argc, char **argv)
{
	int k;

	for (k=0; k<SW_LAN_PORT_NUM; k++) {
		boaWrite(wp,"\t<tr>\n\t<td><b>LAN%d:</b></td>\n", virt2user[k]);
		boaWrite(wp,"\t<td>\n\t<select name=p%d>\n\t<script>\n\tlinkOption();"
			"\n\t</script>\n\t</select>\n\t</td>\n\t</tr>\n", k);
	}
	return 0;
}
#endif	// of ELAN_LINK_MODE

#else  // CONFIG_RTL_MULTI_LAN_DEV
#ifdef ELAN_LINK_MODE_INTRENAL_PHY
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
///////////////////////////////////////////////////////////////////
void formLink(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
	char p_str[]="p0";
	unsigned char mode;
	//MIB_CE_SW_PORT_T Entry;

#ifndef NO_ACTION
	int pid;
#endif
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	//for (k=0; k<SW_LAN_PORT_NUM; k++) {
		p_str[1] = '0';
		str = boaGetVar(wp, p_str, "");
		mode = str[0] - '0';
		if (mode < LINK_10HALF || mode > LINK_AUTO) {
			strcpy(tmpBuf, multilang(LANG_INVALID_LINK_MODE_VALUE));
			goto setErr_link;
		}

		if (!mib_set(MIB_ETH_MODE, &mode)) {
			strcpy(tmpBuf, errGetEntry);
			goto setErr_link;
		}

	//}

	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

	restart_ethernet(1);
	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);
  	return;

setErr_link:
	ERR_MSG(tmpBuf);
}

#endif
#endif // CONFIG_RTL_MULTI_LAN_DEV
