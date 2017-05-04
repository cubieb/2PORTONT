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

#ifdef CONFIG_IPV6
static const char RADVD_NEW_CONF[] ="/var/radvd2.conf";
static const char KEYWORD1[]= "AdvManagedFlag";
static const char KEYWORD2[]= "AdvOtherConfigFlag";
static const char KEYWORD3[]= "MaxRtrAdvInterval";
static const char KEYWORD4[]= "MinRtrAdvInterval";

//#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
void formRadvdSetup(request * wp, char *path, char *query)
{
	char *str, *submitUrl, *strVal, *pStr;
	static char tmpBuf[100];
	unsigned char vChar;
	char mode;
	int radvdpid;

#ifndef NO_ACTION
	int pid;
#endif

	str = boaGetVar(wp, "MaxRtrAdvIntervalAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_MAXRTRADVINTERVAL, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "MinRtrAdvIntervalAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_MINRTRADVINTERVAL, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvCurHopLimitAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVCURHOPLIMIT, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvDefaultLifetimeAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVDEFAULTLIFETIME, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvReachableTimeAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVREACHABLETIME, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvRetransTimerAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVRETRANSTIMER, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvLinkMTUAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVLINKMTU, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	//}

	strVal = boaGetVar(wp, "AdvSendAdvertAct", "");
	if ( strVal[0] ) {
		vChar = strVal[0] - '0';
		mib_set( MIB_V6_SENDADVERT, (void *)&vChar);
	}

	strVal = boaGetVar(wp, "AdvManagedFlagAct", "");
	if ( strVal[0] ) {
		vChar = strVal[0] - '0';
		mib_set( MIB_V6_MANAGEDFLAG, (void *)&vChar);
	}

	strVal = boaGetVar(wp, "AdvOtherConfigFlagAct", "");
	if ( strVal[0] ) {
		vChar = strVal[0] - '0';
		mib_set( MIB_V6_OTHERCONFIGFLAG, (void *)&vChar);
	}
	
	strVal = boaGetVar(wp, "EnableULA", "");
	if ( strVal[0] ) {
		vChar = strVal[0] - '0';
		mib_set( MIB_V6_ULAPREFIX_ENABLE, (void *)&vChar);
	}

	str = boaGetVar(wp, "ULAPrefix", "");
	if (str[0]) {
		if ( !mib_set(MIB_V6_ULAPREFIX, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	}

	str = boaGetVar(wp, "ULAPrefixlen", "");
	if (str[0]) {
		if ( !mib_set(MIB_V6_ULAPREFIX_LEN, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	}
	str = boaGetVar(wp, "ULAPrefixValidTime", "");
	if (str[0]) {
		if ( !mib_set(MIB_V6_ULAPREFIX_VALID_TIME, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	}
	
	str = boaGetVar(wp, "ULAPrefixPreferedTime", "");
	if (str[0]) {
		if ( !mib_set(MIB_V6_ULAPREFIX_PREFER_TIME, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_radvd;
		}
	}

	strVal = boaGetVar(wp, "RDNSS1", "");
	mib_set( MIB_V6_RDNSS1, (void *)strVal);

	strVal = boaGetVar(wp, "RDNSS2", "");
	mib_set( MIB_V6_RDNSS2, (void *)strVal);

	strVal = boaGetVar(wp, "PrefixEnable", "");
	if ( strVal[0] ) {
		vChar = strVal[0] - '0';
		mib_set( MIB_V6_PREFIX_ENABLE, (void *)&vChar);
	}
	pStr = boaGetVar(wp, "PrefixMode", "");
	if (pStr[0]) {
		if (pStr[0] == '1') {
			mode = 1;    // Manual

			str = boaGetVar(wp, "prefix_ip", "");
			if (str[0]) {
				if ( !mib_set(MIB_V6_PREFIX_IP, (void *)str)) {
					strcpy(tmpBuf, Tset_mib_error);
					goto setErr_radvd;
				}
			}

			str = boaGetVar(wp, "prefix_len", "");
			if (str[0]) {
				if ( !mib_set(MIB_V6_PREFIX_LEN, (void *)str)) {
					strcpy(tmpBuf, Tset_mib_error);
					goto setErr_radvd;
				}
			}

			str = boaGetVar(wp, "AdvValidLifetimeAct", "");
			//if (str[0]) {
				if ( !mib_set(MIB_V6_VALIDLIFETIME, (void *)str)) {
					strcpy(tmpBuf, Tset_mib_error);
					goto setErr_radvd;
				}
			//}

			str = boaGetVar(wp, "AdvPreferredLifetimeAct", "");
			//if (str[0]) {
				if ( !mib_set(MIB_V6_PREFERREDLIFETIME, (void *)str)) {
					strcpy(tmpBuf, Tset_mib_error);
					goto setErr_radvd;
				}
			//}

			strVal = boaGetVar(wp, "AdvOnLinkAct", "");
			if ( strVal[0] ) {
				vChar = strVal[0] - '0';
				mib_set( MIB_V6_ONLINK, (void *)&vChar);
			}

			strVal = boaGetVar(wp, "AdvAutonomousAct", "");
			if ( strVal[0] ) {
				vChar = strVal[0] - '0';
				mib_set( MIB_V6_AUTONOMOUS, (void *)&vChar);
			}

	   	} else {
	   		mode = 0;     // Auto
	   	}
	} else {
		mode = 0;            // Auto
	}
	mib_set(MIB_V6_PREFIX_MODE, (void *)&mode);

	// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	radvdpid = read_pid((char *)RADVD_PID);
	restartRadvd();


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

setErr_radvd:
	ERR_MSG(tmpBuf);
}
//#endif
#endif // #ifdef CONFIG_IPV6
