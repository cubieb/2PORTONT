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
#include "utility.h"


int fm3g_checkWrite(int eid, request * wp, int argc, char **argv)
{
	char *name;
	MIB_WAN_3G_T entry,*p;

	//printf( "%s: enter\n", __FUNCTION__ );
   	if (boaArgs(argc, argv, "%s", &name) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
		//printf( "%s: error, line=%d\n", __FUNCTION__, __LINE__ );
   		return -1;
   	}
	//printf( "%s: name=%s\n", __FUNCTION__, name );

	p=&entry;
	if(!mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p))
	{
		//printf( "%s: error mib_chain_get(), line=%d\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	if(!strcmp(name, "fm3g-enable-dis")) {
		if(p->enable==0) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-enable-en")) {
		if(p->enable) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-pin")) {
		if( p->pin!=NO_PINCODE ) boaWrite(wp, "%04u", p->pin);
		return 0;
	}else if(!strcmp(name, "fm3g-apn")) {
		boaWrite(wp, "%s", p->apn);
		return 0;
	}else if(!strcmp(name, "fm3g-dial")) {
		boaWrite(wp, "%s", p->dial);
		return 0;
	}else if(!strcmp(name, "fm3g-auth-none")) {
		if(p->auth==PPP_AUTH_NONE) boaWrite(wp, "selected");
		return 0;
	}else if(!strcmp(name, "fm3g-auth-auto")) {
		if(p->auth==PPP_AUTH_AUTO) boaWrite(wp, "selected");
		return 0;
	}else if(!strcmp(name, "fm3g-auth-pap")) {
		if(p->auth==PPP_AUTH_PAP) boaWrite(wp, "selected");
		return 0;
	}else if(!strcmp(name, "fm3g-auth-chap")) {
		if(p->auth==PPP_AUTH_CHAP) boaWrite(wp, "selected");
		return 0;
	}else if(!strcmp(name, "fm3g-username")) {
		boaWrite(wp, "%s", p->username);
		return 0;
	}else if(!strcmp(name, "fm3g-username-dis")) {
		if(p->auth==PPP_AUTH_NONE) boaWrite(wp, "disabled");
		return 0;
	}else if(!strcmp(name, "fm3g-password")) {
		boaWrite(wp, "%s", p->password);
		return 0;
	}else if(!strcmp(name, "fm3g-password-dis")) {
		if(p->auth==PPP_AUTH_NONE) boaWrite(wp, "disabled");
		return 0;
	}else if(!strcmp(name, "fm3g-ctype-cont")) {
		if(p->ctype==CONTINUOUS) boaWrite(wp, "selected");
		return 0;
	}else if(!strcmp(name, "fm3g-ctype-demand")) {
		if(p->ctype==CONNECT_ON_DEMAND) boaWrite(wp, "selected");
		return 0;
	}else if(!strcmp(name, "fm3g-ctype-manual")) {
		if(p->ctype==MANUAL) boaWrite(wp, "selected");
		return 0;
	}else if(!strcmp(name, "fm3g-idletime")) {
		boaWrite(wp, "%u", p->idletime);
		return 0;
	}else if(!strcmp(name, "fm3g-idletime-dis")) {
		if(p->ctype!=CONNECT_ON_DEMAND) boaWrite(wp, "disabled");
		return 0;
	}else if(!strcmp(name, "fm3g-napt-dis")) {
		if(!p->napt) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-napt-en")) {
		if(p->napt) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-droute-dis")) {
		if(!p->dgw) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-droute-en")) {
		if(p->dgw) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-mtu")) {
		boaWrite(wp, "%u", p->mtu);
		return 0;
	//paula, 3g backup PPP
	}else if(!strcmp(name, "fm3g-backup-dis")) {
		if(!p->backup) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-backup-en")) {
		if(p->backup) boaWrite(wp, "checked");
		return 0;
	}else if(!strcmp(name, "fm3g-backup_timer")) {
		boaWrite(wp, "%u", p->backup_timer);
		return 0;
	}else if(!strcmp(name, "fm3g-backup_timer-dis")) {
		if(!p->backup) boaWrite(wp, "disabled");
		return 0;
	}else if(!strcmp(name, "fm3g-droute-bu")) {
		if(p->backup) boaWrite(wp, "disabled");
		return 0;
	}else if(!strcmp(name, "fm3g-ctype-bu")) {
		if(p->backup) boaWrite(wp, "disabled");
		return 0;
	//paula, 3g backup PPP end
	}else{
		//printf( "%s: error, line=%d\n", __FUNCTION__, __LINE__ );
		return -1;
	}

	return 0;
}


///////////////////////////////////////////////////////////////////
void form3GConf(request * wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	MIB_WAN_3G_T entry,*p;
	MIB_CE_ATM_VC_T vc_Entry;

#ifdef APPLY_CHANGE
	//wan3g_stop();	
	vc_Entry.ifIndex = TO_IFINDEX(MEDIA_3G,  MODEM_PPPIDX_FROM, 0);
	deleteConnection(CONFIGONE, &vc_Entry);	
#endif //APPLY_CHANGE

	p=&entry;
	if(!mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p))
	{
		//printf( "%s: error mib_chain_get(), line=%d\n", __FUNCTION__, __LINE__ );
		strcpy(tmpBuf, Tget_mib_error);
		goto setErr_3g;
	}

	strData = boaGetVar(wp, "enable3g", "");
	if ( strData[0] )
	{
		p->enable= (strData[0]=='0')? 0:1;
	}

	strData = boaGetVar(wp, "pin", "");
	if ( strData[0] )
	{
		if( (strlen(strData)!=4) ||
		    isdigit(strData[0])==0 ||
		    isdigit(strData[1])==0 ||
		    isdigit(strData[2])==0 ||
		    isdigit(strData[3])==0 )
		{
			strcpy(tmpBuf, strInvaidPinCode);
			goto setErr_3g;
		}
		p->pin= (unsigned short) atoi( strData );
	}else //no pin code
		p->pin= NO_PINCODE;

	strData = boaGetVar(wp, "apn", "");
	//if ( strData[0] )
	{
		strncpy( p->apn, strData, sizeof(p->apn)-1 );
		p->apn[sizeof(p->apn)-1]=0;
	}

	strData = boaGetVar(wp, "dial", "");
	if ( strData[0] )
	{
		strncpy( p->dial, strData, sizeof(p->dial)-1 );
		p->dial[sizeof(p->dial)-1]=0;
	}

	strData = boaGetVar(wp, "auth", "");
	if ( strData[0] )
	{
		p->auth= (unsigned char) atoi( strData );
	}

	strData = boaGetVar(wp, "username", "");
	//if ( strData[0] )
	{
		strncpy( p->username, strData, sizeof(p->username)-1 );
		p->username[sizeof(p->username)-1]=0;
	}

	strData = boaGetVar(wp, "password", "");
	//if ( strData[0] )
	{
		strncpy( p->password, strData, sizeof(p->password)-1 );
		p->password[sizeof(p->password)-1]=0;
	}

	strData = boaGetVar(wp, "ctype", "");
	if ( strData[0] )
	{
		p->ctype= (unsigned char) atoi( strData );
	}

	strData = boaGetVar(wp, "idletime", "");
	if ( strData[0] )
	{
		p->idletime= (unsigned short) atoi( strData );
	}

	strData = boaGetVar(wp, "napt", "");
	if ( strData[0] )
	{
		p->napt= (strData[0]=='0')? 0:1;
	}

	strData = boaGetVar(wp, "droute", "");
	if ( strData[0] )
	{
		p->dgw= (strData[0]=='0')? 0:1;
	}

	strData = boaGetVar(wp, "mtu", "");
	if ( strData[0] )
	{
		p->mtu= (unsigned short) atoi( strData );
		if( p->mtu<65 || p->mtu>1500 )
		{
			sprintf(tmpBuf, "%s : MTU", strInvalidValue);
			goto setErr_3g;
		}
	}

	//paula, 3g backup PPP
	strData = boaGetVar(wp, "backup", "");
	if ( strData[0] )
	{
		p->backup= (strData[0]=='0')? 0:1;
		if(p->backup) p->dgw = 1;
	}
	strData = boaGetVar(wp, "backup_timer", "");
	if ( strData[0] )
	{
		p->backup_timer= (unsigned short) atoi( strData );
		if( p->backup_timer>600 ) //0-10min
		{
			strcpy(tmpBuf, strInvaidBackupTimer);
			goto setErr_3g;
		}
	}
	if(!mib_chain_update( MIB_WAN_3G_TBL, (void*)p, 0))
	{
		//printf( "%s: error mib_chain_get(), line=%d\n", __FUNCTION__, __LINE__ );
		strcpy(tmpBuf, Tupdate_chain_error);
		goto setErr_3g;
	}

#ifdef APPLY_CHANGE
	//wan3g_start();
	restartWAN(CONFIGONE, &vc_Entry);
#endif //APPLY_CHANGE

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	strData = boaGetVar(wp, "submit-url", "");
	OK_MSG(strData);
	return;



setErr_3g:

#ifdef APPLY_CHANGE
	//wan3g_start();	
	restartWAN(CONFIGONE, &vc_Entry);
#endif //APPLY_CHANGE

	//ERR_MSG(tmpBuf);
	strData = boaGetVar(wp, "submit-url", "");
	OK_MSG1(tmpBuf, strData);

	return;
}

