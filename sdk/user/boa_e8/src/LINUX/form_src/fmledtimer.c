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
#include <linux/if.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "fmdefs.h"

#define  URL_MAX_ENTRY  500
#define  KEY_MAX_ENTRY  500

void formLedTimer(request * wp, char *path, char *query)
{
	int i, totalEntry, index;
	char *strVal, *action;
	unsigned char status, enable;
	int ctlCycle, startHour, startMin, endHour, endMin;
	MIB_CE_DAY_SCHED_T entry;
	_TRACE_CALL;

	strVal = boaGetVar(wp, "ledSwitch", "");
	if(strVal[0])
	{
		//switch led state
		strVal = boaGetVar(wp, "ledstatus", "");
		if (strVal[0]=='1')
		{
			status = 1;
		}
		else
		{
			status = 0;
		}
		setLedStatus(status);

		goto apply_ok;
	}

	action = boaGetVar(wp, "action", "");
	if(action[0])
	{
		if(action[0]=='0')
		{
			//delete
			strVal = boaGetVar(wp, "if_index", "");
			if(strVal[0])
			{
				index = strVal[0]-'0';
			}
			//now index always be 0.
			mib_chain_delete(MIB_LED_INDICATOR_TIMER_TBL, 0);
		}
		else if((action[0]=='1') || (action[0]=='2'))
		{
			//add or modify
			strVal = boaGetVar(wp, "if_index", "");
			if(strVal[0])
			{
				index = strVal[0]-'0';
			}

			totalEntry = mib_chain_total(MIB_LED_INDICATOR_TIMER_TBL);
			memset(&entry, 0, sizeof(entry));
			if(0 != totalEntry)
			{
				if (!mib_chain_get(MIB_LED_INDICATOR_TIMER_TBL, 0, (void *)&entry))
				{
					printf("[%s %d]apply_failed\n", __func__, __LINE__);
					goto apply_failed;
				}
			}

			strVal = boaGetVar(wp, "Fnt_Active", "");
			/*
			if (!strVal[0]) 
			{
				printf("[%s %d]apply_failed\n", __func__, __LINE__);
				goto apply_failed;
			}
			*/
			if ( !gstrcmp(strVal, "ON") )
				entry.enable = 1;
			else
				entry.enable = 0;

			strVal = boaGetVar(wp, "startHour", "");
			if (!strVal[0]) 
			{
				printf("[%s %d]apply_failed\n", __func__, __LINE__);
				goto apply_failed;
			}
			string_to_dec(strVal, &startHour);
			entry.startHour = (unsigned char)startHour;

			strVal = boaGetVar(wp, "startMin", "");
			if (!strVal[0]) 
			{
				printf("[%s %d]apply_failed\n", __func__, __LINE__);
				goto apply_failed;
			}
			string_to_dec(strVal, &startMin);
			entry.startMin = (unsigned char)startMin;

			strVal = boaGetVar(wp, "endHour", "");
			if (!strVal[0]) 
			{
				printf("[%s %d]apply_failed\n", __func__, __LINE__);
				goto apply_failed;
			}
			string_to_dec(strVal, &endHour);
			entry.endHour = (unsigned char)endHour;

			strVal = boaGetVar(wp, "endMin", "");
			if (!strVal[0]) 
			{
				printf("[%s %d]apply_failed\n", __func__, __LINE__);
				goto apply_failed;
			}
			string_to_dec(strVal, &endMin);
			entry.endMin = (unsigned char)endMin;

			strVal = boaGetVar(wp, "controlCycle", "");
			if (!strVal[0]) 
			{
				printf("[%s %d]apply_failed\n", __func__, __LINE__);
				goto apply_failed;
			}
			string_to_dec(strVal, &ctlCycle);
			entry.ctlCycle = (unsigned char)ctlCycle;
			
			if(0 != totalEntry)
			{
				mib_chain_update(MIB_LED_INDICATOR_TIMER_TBL, (void *)&entry, 0);
			}
			else
			{
				if(!mib_chain_add(MIB_LED_INDICATOR_TIMER_TBL, (void *)&entry))
				{
					printf("[%s %d]apply_failed\n", __func__, __LINE__);
					goto apply_failed;
				}
			}
		}
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	updateScheduleCrondFile("/var/spool/cron/crontabs", 0);
apply_ok:
	_COND_REDIRECT;
	return;

apply_failed:
	ERR_MSG("设置失败，请重新设置!");
	return;
}
#endif
