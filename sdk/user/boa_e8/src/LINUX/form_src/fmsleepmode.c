/*
 *      Web server handler routines for URL stuffs
 */
#include "options.h"

/*-- System inlcude files --*/
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "fmdefs.h"

void formSleepMode(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *stemp = "";
	unsigned char timerEnable, onoffEnable;
	int total = 0, act_idx = -1;
	int	lineno = __LINE__;
	
	MIB_CE_RG_SLEEPMODE_SCHED_T entry;

	memset(&entry, 0, sizeof(MIB_CE_RG_SLEEPMODE_SCHED_T));
	
	FETCH_INVALID_OPT(stemp, "action", _NEED);
	if(strcmp(stemp, "add") == 0)	//add
	{
		FETCH_INVALID_OPT(stemp, "day", _NEED);
		entry.day = atoi(stemp);

		if(entry.day != 0)
		{
			FETCH_INVALID_OPT(stemp, "hour", _NEED);
			entry.hour = atoi(stemp);
		
			FETCH_INVALID_OPT(stemp, "minute", _NEED);
			entry.minute = atoi(stemp);	
		}

		_GET_BOOL(timerEnable, _NEED);
		entry.enable = timerEnable;

		_GET_BOOL(onoffEnable, _NEED);
		entry.onoff = onoffEnable;

		if(!mib_chain_add(MIB_SLEEP_MODE_SCHED_TBL, (void *)&entry))
		{
			//Chain Add Error
			goto check_err;
		}
	}
	else if(strcmp(stemp, "modify") == 0)//modify
	{
		FETCH_INVALID_OPT(stemp, "idx", _NEED);
		act_idx = atoi(stemp);
			
		FETCH_INVALID_OPT(stemp, "day", _NEED);
		entry.day = atoi(stemp);

		if(entry.day != 0)
		{
			FETCH_INVALID_OPT(stemp, "hour", _NEED);
			entry.hour = atoi(stemp);

			FETCH_INVALID_OPT(stemp, "minute", _NEED);
			entry.minute = atoi(stemp);
		}
		_GET_BOOL(timerEnable, _NEED);
		entry.enable = timerEnable;

		_GET_BOOL(onoffEnable, _NEED);
		entry.onoff = onoffEnable;

		total = mib_chain_total(MIB_SLEEP_MODE_SCHED_TBL);
		if(total>act_idx)
		{
			mib_chain_update(MIB_SLEEP_MODE_SCHED_TBL, (void*)&entry, act_idx);
		}
	}
	else if (strcmp(stemp, "del") == 0)//delete
	{
		FETCH_INVALID_OPT(stemp, "idx", _NEED);
		act_idx = atoi(stemp);

		total = mib_chain_total(MIB_SLEEP_MODE_SCHED_TBL);
		if(total>act_idx)
		{
			mib_chain_delete(MIB_SLEEP_MODE_SCHED_TBL, act_idx);
		}
	}
	else{
		;
	}


#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	updateScheduleCrondFile("/var/spool/cron/crontabs", 0);

check_err:
	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;

}

int initPageSleepModeRule(int eid, request * wp, int argc, char ** argv)
{
	int total = 0;
	int idx = 0;
	int lineno = __LINE__;
	MIB_CE_RG_SLEEPMODE_SCHED_T entry;

	_TRACE_CALL;

	total = mib_chain_total(MIB_SLEEP_MODE_SCHED_TBL);
	for(idx = 0; idx < total; idx++)
	{
		if (!mib_chain_get(MIB_SLEEP_MODE_SCHED_TBL, idx, (void *)&entry)) {
			printf("get Sleep Mode chain error!\n");
			goto check_err;
		}
		boaWrite (wp, "push(new it_nr(\"%d\""_PTI _PTI _PTI _PTI _PTI"));\n", idx, "day", entry.day, "hour", entry.hour, "minute", entry.minute, "enable", entry.enable, "onoff", entry.onoff);
	}
check_err:
	_TRACE_LEAVEL;
	return 0;
}
