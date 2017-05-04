/*
 *      Web server handler routines for DHCP Server stuffs
 *      Authors: Kaohj	<kaohj@realtek.com.tw>
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "multilang.h"
#include <rtk/utility.h>


#ifdef __i386__
#define _LITTLE_ENDIAN_
#endif

#define _DHCPD_PID_FILE			"/var/run/udhcpd.pid"
#define _DHCPD_LEASES_FILE		"/var/udhcpd/udhcpd.leases"

/*-- Macro declarations --*/
#ifdef _LITTLE_ENDIAN_
#define ntohdw(v) ( ((v&0xff)<<24) | (((v>>8)&0xff)<<16) | (((v>>16)&0xff)<<8) | ((v>>24)&0xff) )

#else
#define ntohdw(v) (v)
#endif

void formDhcpd(request * wp, char *path, char *query)
{
	char	*strDhcp, *submitUrl, *strIp;
	struct in_addr inIp, inMask, inGatewayIp;
#ifdef DHCPS_POOL_COMPLETE_IP
	struct in_addr inPoolStart, inPoolEnd, dhcpmask, ori_dhcpmask;
	char *str_dhcpmask;
#endif
	DHCP_T dhcp, curDhcp;
	char tmpBuf[100];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char vChar;
//star: for dhcp change
	unsigned char origvChar;
	unsigned int origInt;
	char origstr[30];
	char *origstrDomain=origstr;
	struct in_addr origGatewayIp;
	int dhcpd_changed_flag=0;
	char *strdhcpenable;
	unsigned char mode;

	char	*strdhcpRangeStart, *strdhcpRangeEnd, *strLTime, *strDomain;

	strdhcpenable = boaGetVar(wp, "dhcpdenable", "");
	mib_get( MIB_DHCP_MODE, (void *)&origvChar);

	if(strdhcpenable[0])
	{
		sscanf(strdhcpenable, "%u", &origInt);
		mode = (unsigned char)origInt;
		if(mode!=origvChar)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_DHCP_MODE, (void *)&mode)) {
  			strcpy(tmpBuf, strSetDhcpModeerror);
			goto setErr_dhcpd;
		}
	}

	// Read current DHCP setting for reference later
	// Modified by Mason Yu for dhcpmode
	//if ( !mib_get( MIB_ADSL_LAN_DHCP, (void *)&vChar) ) {
	if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) ) {
		strcpy(tmpBuf, strGetDhcpModeerror);
		goto setErr_dhcpd;
	}
	curDhcp = (DHCP_T) vChar;

	dhcp = curDhcp;

	if ( dhcp == DHCP_LAN_SERVER ) {
		// Get/Set DHCP client range
		unsigned int uVal, uLTime;
		unsigned char uStart, uEnd;

		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		strdhcpRangeStart = boaGetVar(wp, "dhcpRangeStart", "");
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			uStart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = boaGetVar(wp, "dhcpRangeEnd", "");
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			uEnd = (unsigned char)uVal;
		}
		#else
		strdhcpRangeStart = boaGetVar(wp, "dhcpRangeStart", "");
		if ( strdhcpRangeStart[0] ) {
			if ( !inet_aton(strdhcpRangeStart, &inPoolStart) ) {
				strcpy(tmpBuf, strSetStarIperror);
				goto setErr_dhcpd;
			}
		}
		strdhcpRangeEnd = boaGetVar(wp, "dhcpRangeEnd", "");
		if ( strdhcpRangeEnd[0] ) {
			if ( !inet_aton(strdhcpRangeEnd, &inPoolEnd) ) {
				strcpy(tmpBuf, strSetEndIperror);
				goto setErr_dhcpd;
			}
		}
		#endif

		strLTime = boaGetVar(wp, "ltime", "");
		if ( strLTime[0] ) {
			sscanf(strLTime, "%u", &uLTime);
		}

		strDomain = boaGetVar(wp, "dname", "");

		if(!mib_get( MIB_ADSL_LAN_IP,  (void *)&inIp)) {
			strcpy(tmpBuf, strGetIperror);
			goto setErr_dhcpd;
		}

		if(!mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&inMask)) {
			strcpy(tmpBuf, strGetMaskerror);
			goto setErr_dhcpd;
		}

		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		// update DHCP server config file
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( uEnd - uStart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (uStart&mask[3]) ||
				(ip[3]&mask[3]) != (uEnd&mask[3]) ) {
				strcpy(tmpBuf, strInvalidRange);
				goto setErr_dhcpd;
			}
		}
		#else
		// check the pool range
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			//tylo, for single-PC DHCP
			//if (inPoolStart.s_addr >= inPoolEnd.s_addr) {
			if (inPoolStart.s_addr > inPoolEnd.s_addr) {
				strcpy(tmpBuf, strInvalidRange);
				goto setErr_dhcpd;
			}
		}

		// Magician: Subnet mask for DHCP.
		str_dhcpmask = boaGetVar(wp, "dhcpSubnetMask", "");

		if(!inet_aton(str_dhcpmask, &dhcpmask))
		{
			strcpy(tmpBuf, multilang(LANG_INVALID_SUBNET_MASK_VALUE));
			goto setErr_dhcpd;
		}

		if((inPoolStart.s_addr & dhcpmask.s_addr) != (inPoolEnd.s_addr & dhcpmask.s_addr))
		{
			strcpy(tmpBuf, multilang(LANG_INVALID_DHCP_CLIENT_END_ADDRESSIT_SHOULD_BE_LOCATED_IN_THE_SAME_SUBNET_OF_CURRENT_IP_ADDRESS));
			goto setErr_dhcpd;
		}
		#endif

#ifdef IP_BASED_CLIENT_TYPE
		unsigned char pcstart,pcend,cmrstart,cmrend,stbstart,stbend,phnstart,phnend;

		//PC
		strdhcpRangeStart = boaGetVar(wp, "dhcppcRangeStart", "");
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			pcstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = boaGetVar(wp, "dhcppcRangeEnd", "");
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			pcend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( pcend - pcstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (pcstart&mask[3]) ||
				(ip[3]&mask[3]) != (pcend&mask[3]) ) {
				strcpy(tmpBuf, strInvalidRangepc);
				goto setErr_dhcpd;
			}
		}
		//CMR
		strdhcpRangeStart = boaGetVar(wp, "dhcpcmrRangeStart", "");
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			cmrstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = boaGetVar(wp, "dhcpcmrRangeEnd", "");
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			cmrend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( cmrend - cmrstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (cmrstart&mask[3]) ||
				(ip[3]&mask[3]) != (cmrend&mask[3]) ) {
				strcpy(tmpBuf, strInvalidRangecmr);
				goto setErr_dhcpd;
			}
		}
		//STB
		strdhcpRangeStart = boaGetVar(wp, "dhcpstbRangeStart", "");
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			stbstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = boaGetVar(wp, "dhcpstbRangeEnd", "");
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			stbend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( stbend - stbstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (stbstart&mask[3]) ||
				(ip[3]&mask[3]) != (stbend&mask[3]) ) {
				strcpy(tmpBuf, strInvalidRangestb);
				goto setErr_dhcpd;
			}
		}
		//PHN
		strdhcpRangeStart = boaGetVar(wp, "dhcpphnRangeStart", "");
		if ( strdhcpRangeStart[0] ) {
			sscanf(strdhcpRangeStart, "%u", &uVal);
			phnstart = (unsigned char)uVal;
		}
		strdhcpRangeEnd = boaGetVar(wp, "dhcpphnRangeEnd", "");
		if ( strdhcpRangeEnd[0] ) {
			sscanf(strdhcpRangeEnd, "%u", &uVal);
			phnend = (unsigned char)uVal;
		}
		if ( strdhcpRangeStart[0] && strdhcpRangeEnd[0] ) {
			unsigned char *ip, *mask;
			int diff;

			diff = (int) ( phnend - phnstart );
			ip = (unsigned char *)&inIp;
			mask = (unsigned char *)&inMask;
			if (diff <= 0 ||
				(ip[3]&mask[3]) != (phnstart&mask[3]) ||
				(ip[3]&mask[3]) != (phnend&mask[3]) ) {
				strcpy(tmpBuf, strInvalidRangephn);
				goto setErr_dhcpd;
			}
		}
		//check if the type ip pool out of ip pool range
		if((pcstart<uStart)||(cmrstart<uStart)||(stbstart<uStart)||(phnstart<uStart)
			||(pcend>uEnd)||(cmrend>uEnd)||(stbend>uEnd)||(phnend>uEnd)){
				strcpy(tmpBuf, strInvalidTypeRange);
				goto setErr_dhcpd;
		}
		//check if the type ip pool overlap
		unsigned char ippool[4][2]={{pcstart,pcend},{cmrstart,cmrend},{stbstart,stbend},{phnstart,phnend}};
		unsigned char tmp1,tmp2;
		int i,j,min;
		for(i=0;i<4;i++)
		{
			min = i;
			for(j=i;j<4;j++)
			{
				if(ippool[j][0] < ippool[min][0])
					min = j;
			}
			if(min!=i){
				tmp1=ippool[i][0];
				tmp2=ippool[i][1];
				ippool[i][0]=ippool[min][0];
				ippool[i][1]=ippool[min][1];
				ippool[min][0]=tmp1;
				ippool[min][1]=tmp2;
			}
		}

		for(i=0;i<3;i++)
		{
			if(ippool[i][1]>=ippool[i+1][0]){
				strcpy(tmpBuf, strOverlapRange);
				goto setErr_dhcpd;
			}
		}

		//set the type ip pool
		mib_get(CWMP_CT_PC_MINADDR, (void *)&origvChar);
		if(origvChar != pcstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PC_MINADDR, (void *)&pcstart)) {
			strcpy(tmpBuf, strSetPcStartIperror);
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_PC_MAXADDR, (void *)&origvChar);
		if(origvChar != pcend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PC_MAXADDR, (void *)&pcend)) {
			strcpy(tmpBuf, strSetPcEndIperror);
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_CMR_MINADDR, (void *)&origvChar);
		if(origvChar != cmrstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_CMR_MINADDR, (void *)&cmrstart)) {
			strcpy(tmpBuf, strSetCmrStartIperror);
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_CMR_MAXADDR, (void *)&origvChar);
		if(origvChar != cmrend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_CMR_MAXADDR, (void *)&cmrend)) {
			strcpy(tmpBuf, strSetCmrEndIperror);
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_STB_MINADDR, (void *)&origvChar);
		if(origvChar != stbstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_STB_MINADDR, (void *)&stbstart)) {
			strcpy(tmpBuf, strSetStbStartIperror);
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_STB_MAXADDR, (void *)&origvChar);
		if(origvChar != stbend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_STB_MAXADDR, (void *)&stbend)) {
			strcpy(tmpBuf, strSetStbEndIperror);
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_PHN_MINADDR, (void *)&origvChar);
		if(origvChar != phnstart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PHN_MINADDR, (void *)&phnstart)) {
			strcpy(tmpBuf, strSetPhnStartIperror);
			goto setErr_dhcpd;
		}

		mib_get(CWMP_CT_PHN_MAXADDR, (void *)&origvChar);
		if(origvChar != phnend)
			dhcpd_changed_flag = 1;
		if ( !mib_set(CWMP_CT_PHN_MAXADDR, (void *)&phnend)) {
			strcpy(tmpBuf, strSetPhnEndIperror);
			goto setErr_dhcpd;
		}

#endif

		// Kaohj
		#ifndef DHCPS_POOL_COMPLETE_IP
		mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&origvChar);
		if(origvChar != uStart)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_CLIENT_START, (void *)&uStart)) {
			strcpy(tmpBuf, strSetStarIperror);
			goto setErr_dhcpd;
		}

		mib_get(MIB_ADSL_LAN_CLIENT_END, (void *)&origvChar);
		if(origvChar != uEnd)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_CLIENT_END, (void *)&uEnd)) {
			strcpy(tmpBuf, strSetEndIperror);
			goto setErr_dhcpd;
		}
		#else
		mib_get(MIB_DHCP_POOL_START, (void *)&inIp);
		if(inIp.s_addr != inPoolStart.s_addr)
			dhcpd_changed_flag = 1;
		if ( !mib_set( MIB_DHCP_POOL_START, (void *)&inPoolStart)) {
			strcpy(tmpBuf, strSetStarIperror);
			goto setErr_dhcpd;
		}
		mib_get(MIB_DHCP_POOL_END, (void *)&inIp);
		if(inIp.s_addr != inPoolEnd.s_addr)
			dhcpd_changed_flag = 1;
		if ( !mib_set( MIB_DHCP_POOL_END, (void *)&inPoolEnd)) {
			strcpy(tmpBuf, strSetEndIperror);
			goto setErr_dhcpd;
		}

		// Magician: Subnet mask for DHCP.
		mib_get(MIB_DHCP_SUBNET_MASK, (void *)&ori_dhcpmask);
		if( ori_dhcpmask.s_addr != dhcpmask.s_addr )
			dhcpd_changed_flag = 1;

		if( !mib_set(MIB_DHCP_SUBNET_MASK, (void *)&dhcpmask))
		{
			sprintf(tmpBuf, " %s (DHCP subnetmask).",Tset_mib_error);
			goto setErr_dhcpd;
		}
		#endif

		mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void *)&origInt);
		if(origInt != uLTime)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime)) {
			strcpy(tmpBuf, strSetLeaseTimeerror);
			goto setErr_dhcpd;
		}

		mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)origstrDomain);
		if(strcmp(origstrDomain, strDomain)!=0)
			dhcpd_changed_flag = 1;
		if ( !mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)strDomain)) {
			strcpy(tmpBuf, strSetDomainNameerror);
			goto setErr_dhcpd;
		}

		// Added by Mason Yu for DHCP Server Gateway Address
		// Set Gateway address
		strIp = boaGetVar(wp, "ip", "");
		if ( strIp[0] ) {
			if ( !inet_aton(strIp, &inGatewayIp) ) {
				strcpy(tmpBuf, strInvalidGatewayerror);
				goto setErr_dhcpd;
			}
			mib_get(MIB_ADSL_LAN_DHCP_GATEWAY,(void*)&origGatewayIp);
			if(origGatewayIp.s_addr != inGatewayIp.s_addr)
				dhcpd_changed_flag = 1;
			if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&inGatewayIp)) {
				strcpy(tmpBuf, strSetGatewayerror);
				goto setErr_dhcpd;
			}
		}
		// Kaohj
#ifdef DHCPS_DNS_OPTIONS
		strDhcp = boaGetVar(wp, "dhcpdns", "");
		strDhcp[0] -= '0';
		mib_get(MIB_DHCP_DNS_OPTION, (void *)&origvChar);
		if (origvChar != strDhcp[0])
			dhcpd_changed_flag = 1;
		mib_set(MIB_DHCP_DNS_OPTION, (void *)strDhcp);
		if (strDhcp[0] == 1) { // set manually
			strIp = boaGetVar(wp, "dns1", "");
			if ( !inet_aton(strIp, &inIp) ) {
				strcpy(tmpBuf, Tinvalid_dns);
				goto setErr_dhcpd;
			}
			mib_get(MIB_DHCPS_DNS1, (void *)&origGatewayIp);
			if (origGatewayIp.s_addr != inIp.s_addr)
				dhcpd_changed_flag = 1;
			mib_set(MIB_DHCPS_DNS1, (void *)&inIp);
			inIp.s_addr = INADDR_NONE;
			strIp = boaGetVar(wp, "dns2", "");
			if (strIp[0]) {
				if ( !inet_aton(strIp, &inIp) ) {
					strcpy(tmpBuf, Tinvalid_dns);
					goto setErr_dhcpd;
				}
				mib_get(MIB_DHCPS_DNS2, (void *)&origGatewayIp);
				if (origGatewayIp.s_addr != inIp.s_addr)
					dhcpd_changed_flag = 1;
				mib_set(MIB_DHCPS_DNS2, (void *)&inIp);
				inIp.s_addr = INADDR_NONE;
				strIp = boaGetVar(wp, "dns3", "");
				if (strIp[0]) {
					if ( !inet_aton(strIp, &inIp) ) {
						strcpy(tmpBuf, Tinvalid_dns);
						goto setErr_dhcpd;
					}
				}
				mib_get(MIB_DHCPS_DNS3, (void *)&origGatewayIp);
				if (origGatewayIp.s_addr != inIp.s_addr)
					dhcpd_changed_flag = 1;
				mib_set(MIB_DHCPS_DNS3, (void *)&inIp);
			}
			else {
				mib_get(MIB_DHCPS_DNS2, (void *)&origGatewayIp);
				if (origGatewayIp.s_addr != inIp.s_addr)
					dhcpd_changed_flag = 1;
				mib_set(MIB_DHCPS_DNS2, (void *)&inIp);
				mib_set(MIB_DHCPS_DNS3, (void *)&inIp);
			}
		}
#endif // of DHCPS_DNS_OPTIONS
	}
	else if( dhcp == DHCP_LAN_RELAY ){
		struct in_addr dhcps,origdhcps;
		char *str;

		str = boaGetVar(wp, "dhcps", "");
		if ( str[0] ) {
			if ( !inet_aton(str, &dhcps) ) {
				strcpy(tmpBuf, strInvalDhcpsAddress);
				goto setErr_dhcpd;
			}
			mib_get(MIB_ADSL_WAN_DHCPS, (void*)&origdhcps);
			if(origdhcps.s_addr != dhcps.s_addr)
				dhcpd_changed_flag = 1;
			if ( !mib_set(MIB_ADSL_WAN_DHCPS, (void *)&dhcps)) {
	  			strcpy(tmpBuf, strSetDhcpserror);
				goto setErr_dhcpd;
			}
		}
	}
	vChar = (unsigned char) dhcp;
	// Modify by Mason Yu for dhcpmode
	//if ( !mib_set(MIB_ADSL_LAN_DHCP, (void *)&vChar)) {
	if ( !mib_set(MIB_DHCP_MODE, (void *)&vChar)) {
  		strcpy(tmpBuf, strSetDhcpModeerror);
		goto setErr_dhcpd;
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

	if(dhcpd_changed_flag == 1)
	{
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif

		restart_dhcp();
		submitUrl = boaGetVar(wp, "submit-url", "");
		OK_MSG(submitUrl);
	}
	else
	{
		submitUrl = boaGetVar(wp, "submit-url", "");
		if (submitUrl[0])
			boaRedirect(wp, submitUrl);
		else
			boaDone(wp, 200);
	}

	return;

setErr_dhcpd:
	ERR_MSG(tmpBuf);
}

/////////////////////////////////////////////////////////////////////////////
int dhcpClientList(int eid, request * wp, int argc, char **argv)
{
#ifdef EMBED
	int pid;
	struct stat status;
	int nBytesSent=0;
	int element=0, ret;
	char ipAddr[40], macAddr[40], liveTime[80], *buf=NULL, *ptr;
	FILE *fp;
	int fsize;

	// siganl DHCP server to update lease file
	pid = read_pid(_DHCPD_PID_FILE);
	if ( pid > 0)
		kill(pid, SIGUSR1);
	usleep(1000);

	if ( stat(_DHCPD_LEASES_FILE, &status) < 0 )
		goto err;

	// read DHCP server lease file
	buf = malloc(status.st_size);
	if ( buf == NULL ) goto err;
	fp = fopen(_DHCPD_LEASES_FILE, "r");
	if ( fp == NULL )goto err;
	fread(buf, 1, status.st_size, fp);
	fclose(fp);

	ptr = buf;
	fsize = (int)status.st_size;
	while (1) {
		ret = getOneDhcpClient(&ptr, &fsize, ipAddr, macAddr, liveTime);

		if (ret < 0)
			break;
		if (ret == 0)
			continue;
		nBytesSent += boaWrite(wp,
			"<tr bgcolor=#b7b7b7><td><font size=2>%s</td><td><font size=2>%s</td><td><font size=2>%s</td></tr>",
			ipAddr, macAddr, liveTime);
		element++;
	}

err:
	if (element == 0) {
		nBytesSent += boaWrite(wp,
			"<tr bgcolor=#b7b7b7><td><font size=2>%s</td><td><font size=2>----</td><td><font size=2>----</td></tr>", multilang(LANG_NONE));
	}
	if (buf)
		free(buf);

	return nBytesSent;
#else
	return 0;
#endif
}

/////////////////////////////////////////////////////////////////////////////
void formReflashClientTbl(request * wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
}

//////////////////////////////////////////////////////////////////////////////
int isDhcpClientExist(char *name)
{
/*
	char tmpBuf[100];
	struct in_addr intaddr;

	if ( getInAddr(name, IP_ADDR, (void *)&intaddr ) ) {
		snprintf(tmpBuf, 100, "%s/%s-%s.pid", _DHCPC_PID_PATH, _DHCPC_PROG_NAME, name);
		if ( getPid(tmpBuf) > 0)
			return 1;
	}
*/
	return 0;
}

