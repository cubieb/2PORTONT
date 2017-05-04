/*
 *      Web server handler routines for other advanced stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <signal.h>
#include <sys/socket.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "multilang.h"

///////////////////////////////////////////////////////////////////
#ifdef IP_PASSTHROUGH
void formOthers(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char ippt_lanacc=0, old_ippt_lanacc, flag;
	unsigned int ippt_itf=DUMMY_IFINDEX, old_ippt_itf;
	unsigned int ippt_lease=0, old_ippt_lease;
	struct ippt_para para;

	// Set PVC auto-search setting
	str = boaGetVar(wp, "autosearch", "");
	if (str[0]) {
		if (str[0] == '0')
			flag = 0;
		else
			flag = 1;
		if ( !mib_set(MIB_ATM_VC_AUTOSEARCH, (void *)&flag)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_others;
		}
	}

#ifdef IP_PASSTHROUGH
	// Get old index of IPPT interface
	mib_get(MIB_IPPT_ITF, (void *)&old_ippt_itf);
	para.old_ippt_itf= old_ippt_itf;

	// Set IP PassThrough setting
	str = boaGetVar(wp, "ippt", "");
	if (str[0]) {
		ippt_itf = (unsigned int)atoi(str);
		if ( !mib_set(MIB_IPPT_ITF, (void *)&ippt_itf)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_others;
		}
	}

	// Get old IPPT Lease Time
	mib_get(MIB_IPPT_LEASE, (void *)&old_ippt_lease);
	para.old_ippt_lease= old_ippt_lease;

	str = boaGetVar(wp, "ltime", "");
	if (str[0]) {
		ippt_lease = (unsigned int)atoi(str);
		if ( !mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_others;
		}
	}

	// Get old IPPT LAN access flag
	mib_get(MIB_IPPT_LANACC, (void *)&old_ippt_lanacc);
	para.old_ippt_lanacc= old_ippt_lanacc;

	str = boaGetVar(wp, "lan_acc", "");
	if ( !gstrcmp(str, "ON"))
		ippt_lanacc = 1;
	else
		ippt_lanacc = 0;

	if ( !mib_set(MIB_IPPT_LANACC, (void *)&ippt_lanacc)) {
		strcpy(tmpBuf, Tset_mib_error);
		goto setErr_others;
	}
#endif

	str = boaGetVar(wp, "singlePC", "");
	if ( !gstrcmp(str, "ON"))
		flag = 1;
	else
		flag = 0;

	if ( !mib_set(MIB_SPC_ENABLE, (void *)&flag)) {
		strcpy(tmpBuf, Tset_mib_error);
		goto setErr_others;
	}

	str = boaGetVar(wp, "IPtype", "");
	if (str[0]) {
		if (str[0] == '0')
			flag = 0;
		else
			flag = 1;
		if ( !mib_set(MIB_SPC_IPTYPE, (void *)&flag)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_others;
		}
	}

	// Mason Yu
#if defined(APPLY_CHANGE)
	para.new_ippt_itf= ippt_itf;
	para.new_ippt_lease= ippt_lease;
	para.new_ippt_lanacc= ippt_lanacc;

	restartIPPT(para);
#endif

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

setErr_others:
	ERR_MSG(tmpBuf);
}
#endif

