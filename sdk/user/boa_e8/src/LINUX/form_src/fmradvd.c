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

#ifdef CONFIG_IPV6
static const char RADVD_NEW_CONF[] ="/var/radvd2.conf";
static const char KEYWORD1[]= "AdvManagedFlag";
static const char KEYWORD2[]= "AdvOtherConfigFlag";
static const char KEYWORD3[]= "MaxRtrAdvInterval";
static const char KEYWORD4[]= "MinRtrAdvInterval";

void  radvd_modify_mo_flag(){

	FILE *fp=NULL;
	FILE *nfp=NULL;
	char buf[512];
	int radvdpid;
	unsigned char m_flag,o_flag;
	unsigned char max_interval_str[MAX_RADVD_CONF_PREFIX_LEN],
				  min_interval_str[MAX_RADVD_CONF_PREFIX_LEN];

	if ( !mib_get(MIB_V6_MANAGEDFLAG, (void *)&m_flag)){
		printf("ERROR: Get MIB_V6_MANAGEDFLAG failed!\n");
		return;
	}

	if ( !mib_get(MIB_V6_OTHERCONFIGFLAG, (void *)&o_flag)){
		printf("ERROR: Get MIB_V6_OTHERCONFIGFLAG failed!\n");
		return;
	}

	if ( !mib_get(MIB_V6_MAXRTRADVINTERVAL, (void *)max_interval_str)){
		printf("ERROR: MIB_V6_MAXRTRADVINTERVAL failed!\n");
		return;
	}

	if ( !mib_get(MIB_V6_MINRTRADVINTERVAL, (void *)min_interval_str)){
		printf("ERROR: Get MIB_V6_MINRTRADVINTERVAL failed!\n");
		return;
	}

	if ((fp = fopen(RADVD_CONF, "r")) == NULL)
	{
		printf("Open file %s Error!\n", RADVD_CONF);
		return;
	}

	if ((nfp = fopen(RADVD_NEW_CONF, "w")) == NULL)
	{
		printf("Open file %s Error!\n", RADVD_NEW_CONF);
		fclose(fp);
		return;
	}

	while(fgets(buf,sizeof(buf),fp)>0)
	{
		if(strstr(buf,KEYWORD1))
			fprintf(nfp,"\t%s %s;\n",KEYWORD1,(m_flag==1)?"on":"off");
		else if (strstr(buf,KEYWORD2))
			fprintf(nfp,"\t%s %s;\n",KEYWORD2,(o_flag==1)?"on":"off");
		else if(strstr(buf,KEYWORD3))
			fprintf(nfp,"\t%s %s;\n",KEYWORD3,max_interval_str);
		else if(strstr(buf,KEYWORD4))
			fprintf(nfp,"\t%s %s;\n",KEYWORD4,min_interval_str);
		else
			fprintf(nfp,"%s",buf);

	}

	fclose(fp);
	fclose(nfp);

	sprintf(buf,"cp %s %s\n",RADVD_NEW_CONF, RADVD_CONF);
	system(buf);
}

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
			strcpy(tmpBuf, "MaxRtrAdvIntervalAct mib 设定错误!"); //Set MaxRtrAdvIntervalAct mib error!
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "MinRtrAdvIntervalAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_MINRTRADVINTERVAL, (void *)str)) {
			strcpy(tmpBuf, "MinRtrAdvIntervalAct mib 设定错误!"); //Set MinRtrAdvIntervalAct mib error!
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvCurHopLimitAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVCURHOPLIMIT, (void *)str)) {
			strcpy(tmpBuf, "AdvCurHopLimitAct mib 设定错误!"); //Set AdvCurHopLimitAct mib error!
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvDefaultLifetimeAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVDEFAULTLIFETIME, (void *)str)) {
			strcpy(tmpBuf, "AdvDefaultLifetime mib 设定错误!"); //Set AdvDefaultLifetime mib error!
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvReachableTimeAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVREACHABLETIME, (void *)str)) {
			strcpy(tmpBuf, "AdvReachableTime mib 设定错误!"); //Set AdvReachableTime mib error!
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvRetransTimerAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVRETRANSTIMER, (void *)str)) {
			strcpy(tmpBuf, "AdvRetransTimer mib 设定错误!"); //Set AdvRetransTimer mib error!
			goto setErr_radvd;
		}
	//}

	str = boaGetVar(wp, "AdvLinkMTUAct", "");
	//if (str[0]) {
		if ( !mib_set(MIB_V6_ADVLINKMTU, (void *)str)) {
			strcpy(tmpBuf, "AdvLinkMTU mib 设定错误!"); //Set AdvLinkMTU mib error!
			goto setErr_radvd;
		}
	//}

	strVal = boaGetVar(wp, "AdvSendAdvertAct", "");
	if ( strVal[0] ) {
		vChar = strVal[0] - '0';
		mib_set( MIB_V6_SENDADVERT, (void *)&vChar);
	}

	strVal = boaGetVar(wp, "radvd_enable", "");
	if ( strVal[0] ) {
		vChar = strVal[0] - '0';
		mib_set( MIB_V6_RADVD_ENABLE, (void *)&vChar);
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
			strcpy(tmpBuf, "ULA prefix mib 设定错误!"); //Set ULA prefix mib error!
			goto setErr_radvd;
		}
	}

	str = boaGetVar(wp, "ULAPrefixlen", "");
	if (str[0]) {
		if ( !mib_set(MIB_V6_ULAPREFIX_LEN, (void *)str)) {
			strcpy(tmpBuf, "ULA prefix len mib 设定错误!"); //Set ULA prefix len mib error!
			goto setErr_radvd;
		}
	}
	str = boaGetVar(wp, "ULAPrefixValidTime", "");
	if (str[0]) {
		if ( !mib_set(MIB_V6_ULAPREFIX_VALID_TIME, (void *)str)) {
			strcpy(tmpBuf, "ULA prefix valid time mib 设定错误!"); //Set ULA prefix valid time mib error!
			goto setErr_radvd;
		}
	}
	
	str = boaGetVar(wp, "ULAPrefixPreferedTime", "");
	if (str[0]) {
		if ( !mib_set(MIB_V6_ULAPREFIX_PREFER_TIME, (void *)str)) {
			strcpy(tmpBuf, "ULA prefix prefered time mib 设定错误!"); //Set ULA prefix prefered time mib error!
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
					strcpy(tmpBuf, "prefix_ip mib 设定错误!"); //Set prefix_ip mib error!
					goto setErr_radvd;
				}
			}

			str = boaGetVar(wp, "prefix_len", "");
			if (str[0]) {
				if ( !mib_set(MIB_V6_PREFIX_LEN, (void *)str)) {
					strcpy(tmpBuf, "prefix_len mib 设定错误!"); //Set prefix_len mib error!
					goto setErr_radvd;
				}
			}

			str = boaGetVar(wp, "AdvValidLifetimeAct", "");
			//if (str[0]) {
				if ( !mib_set(MIB_V6_VALIDLIFETIME, (void *)str)) {
					strcpy(tmpBuf, "AdvValidLifetime mib 设定错误!"); //Set AdvValidLifetime mib error!
					goto setErr_radvd;
				}
			//}

			str = boaGetVar(wp, "AdvPreferredLifetimeAct", "");
			//if (str[0]) {
				if ( !mib_set(MIB_V6_PREFERREDLIFETIME, (void *)str)) {
					strcpy(tmpBuf, "AdvPreferredLifetime mib 设定错误!"); //Set AdvPreferredLifetime mib error!
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

	mib_get(MIB_V6_RADVD_ENABLE, (void *)&vChar);
	radvdpid = read_pid((char *)RADVD_PID);
	if ( vChar==0 ){ //disable RADVD
		if(radvdpid>0){
			kill(radvdpid, SIGTERM);
			unlink(RADVD_PID);
		}
	}
	else{ //RADVD is enabled
		restartRadvd();
	}


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
