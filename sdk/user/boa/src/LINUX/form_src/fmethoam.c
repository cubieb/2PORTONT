/*
 *      Web server handler routines for other advanced stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <signal.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

///////////////////////////////////////////////////////////////////
void formY1731(request * wp, char *path, char *query)
{
	char *str, *submitUrl;
	//char tmpBuf[32];
	unsigned char u8Val;
	unsigned short u16Val;
#ifndef NO_ACTION
	int pid;
#endif
	str = boaGetVar(wp, "oamMode", "");	
	u8Val = (str && str[0]) ? 1 : 0;
	mib_set(Y1731_MODE, (void *)&u8Val);
	
	str = boaGetVar(wp, "meglevel", "7");	
	u8Val = atoi(str);
	mib_set(Y1731_MEGLEVEL, (void *)&u8Val);
	
	str = boaGetVar(wp, "myid", "1");	
	u16Val = atoi(str);
	mib_set(Y1731_MYID, (void *)&u16Val);
	
	str = boaGetVar(wp, "megid", "rtkIgd");		
	mib_set(Y1731_MEGID, (void *)str);
	
	str = boaGetVar(wp, "loglevel", "medium");		
	mib_set(Y1731_LOGLEVEL, (void *)str);	
	
	str = boaGetVar(wp, "ccminterval", "0");	
	u8Val = atoi(str);
	mib_set(Y1731_CCM_INTERVAL, (void *)&u8Val);
		
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif	
	
	Y1731_start(0);
	
	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);
	
	return;

//setErr_others:
//	ERR_MSG(tmpBuf);
}


