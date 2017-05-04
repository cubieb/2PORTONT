/*
 *      Web server handler routines for LED Schedule Control
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>

/*-- Local inlcude files --*/
#include "mib.h"
#include "utility.h"

/**
 * setLedStatus - set LED status
 * @status: New LED status	0 - off, 1 - on
 *
 * Interface for DBus
 * Set new LED status and apply it 
 * Returns  0  -  Success 
 *             -1  - Mib Set Error
 */
int setLedStatus(unsigned char status)
{
	if(!mib_set(MIB_LED_STATUS, (void *)&status))
	{
		return -1;	//Mib Set Error!
	}
	else
	{
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif
		if(status == 0)
		{
			system("/bin/mpctl led off");
		}
		else
		{
			system("/bin/mpctl led restore");
		}	
		return 0;
	}
}


/**
 * getLedStatus - get LED status
 * @status: restore the value of  LED status
 *
 * Interface for DBus
 * Get current LED status 
 * Returns  0   -  Success
 *             -1  - Mib Get Error
 */
int getLedStatus(unsigned char* status)
{
	if(!mib_get(MIB_LED_STATUS, (void *)status))
	{
		return -1;	//Mib Get Error!
	}
	else
	{
		return 0;
	}
}


/**
 * setLedIndicatorTimer - add a schedule to on/off LED
 * @enable: Does the schedule enable
 * @ctlCycle: DAY
 * @startHour: Start Hour for LED on
 * @startMin: Start Min for LED on
 * @endHour: End Hour for LED on
 * @endMin: End Min for LED on
 *
 * Interface for DBus
 * Set new LED schedule and apply it 
 * Returns    0    -   success; 
 *               -1   -   Chain Get Error 
 *               -2   -   Chain Add Error 
 */
int setLedIndicatorTimer(unsigned char enable, unsigned char ctlCycle,
	unsigned char startHour, unsigned char startMin, unsigned char endHour, unsigned char endMin)
{
	unsigned int totalEntry;
	MIB_CE_DAY_SCHED_T entry;

	totalEntry = mib_chain_total(MIB_LED_INDICATOR_TIMER_TBL);
	memset(&entry, 0, sizeof(entry));
	if(0 != totalEntry)
	{
		if (!mib_chain_get(MIB_LED_INDICATOR_TIMER_TBL, 0, (void *)&entry))
		{
			return -1;	//Chain Get Error
		}
	}

	entry.enable 	= enable;
	entry.ctlCycle 	= ctlCycle;
	entry.startHour = startHour;
	entry.startMin 	= startMin;
	entry.endHour 	= endHour;
	entry.endMin 	= endMin;
	
	if(0 != totalEntry)
	{
		mib_chain_update(MIB_LED_INDICATOR_TIMER_TBL, (void *)&entry, 0);
	}
	else
	{
		if(!mib_chain_add(MIB_LED_INDICATOR_TIMER_TBL, (void *)&entry))
		{
			return -2;	//Chain Add Error
		}
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	updateScheduleCrondFile("/var/spool/cron/crontabs", 0);
	return 0;
}

/*
void applyLEDTimer()
{
	unsigned int totalEntry;
	MIB_CE_DAY_SCHED_T entry;

	totalEntry = mib_chain_total(MIB_LED_INDICATOR_TIMER_TBL);

	if(0 != totalEntry)
	{
		memset(&entry, 0, sizeof(entry));
		if (!mib_chain_get(MIB_LED_INDICATOR_TIMER_TBL, 0, (void *)&entry))
		{
			return;	//Chain Get Error
		}
		if(entry.enable)
		{
			TIMEOUT(led_schedule, 0, 1, ledsched_ch);
		}
		else
		{
			UNTIMEOUT(led_schedule, 0, ledsched_ch);
			system("/bin/mpctl led restore");
		}
	}
	else
	{
		UNTIMEOUT(led_schedule, 0, ledsched_ch);
		system("/bin/mpctl led restore");
	}
}
*/
