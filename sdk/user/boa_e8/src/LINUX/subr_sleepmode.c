/*
 *      Web server handler routines for LED Control and Timer
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>

/*-- Local inlcude files --*/
#include "mib.h"
#include "utility.h"

extern int wlan_idx;
#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))

/**
 * config_powersave - enable or diasble rg to powersave mode
 * @enable: enable the powersave mode or not. 0 - disable; 1 - enable
 *
 * Set rg to powersave mode or wake up it
 * enable == 1: enable powersave mode, disable Wifi/Storage/LAN/LED
 * enable == 0: disable powersave mode, enable Wifi/Storage/LAN/LED
 */
void config_sleepmode(int enable)
{
	unsigned char vChar;
	unsigned char wlanDisabled = enable;
	MIB_CE_MBSSIB_T Entry;
	int i, j;
	unsigned int value;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	int orig_idx = wlan_idx;
#endif
	
	for(i=0; i<NUM_WLAN_INTERFACE; i++)
	{
		wlan_idx = i;
		mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
		if(wlanDisabled == Entry.wlanDisabled)
			continue;
		Entry.wlanDisabled = wlanDisabled;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
		update_wps_configured(0);
#endif
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	if(enable==1)
	{
		//LED
		system("/bin/mpctl led off");
		//LAN
		system("/bin/diag port set phy-force-power-down port 0-3 state enable");
		//Storage, Disable IP for USB
		//0xb8000600, bit[3:5]
		value = READ_MEM32(0xb8000600);
		value &= (~0x38);
		WRITE_MEM32(0xb8000600, value);
		//WiFi
		for(i=0; i<NUM_WLAN_INTERFACE; i++)
		{
			wlan_idx = i;
			config_WLAN(ACT_STOP);
		}
	}
	else
	{
		//LED
		system("/bin/mpctl led restore");
		//LAN
		system("/bin/diag port set phy-force-power-down port 0-3 state disable");
		//Storage, Enable IP for USB
		//0xb8000600, bit[3:5]
		value = READ_MEM32(0xb8000600);
		value |= 0x38;
		WRITE_MEM32(0xb8000600, value);
		//WiFi
		for(i=0; i<NUM_WLAN_INTERFACE; i++)
		{
			wlan_idx = i;
			config_WLAN(ACT_START);
		}
	}

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = orig_idx;
#endif

}


/**
 * setPSModeSchedRule - set new config rule to powersave mode schedule
 * @action: 0 - add rule, 1 - delete rule
 * @weekday: 0: active right now, 1:Sunday; 2: Monday; ...
 * @startHour: schedule hour
 * @startMin: schedule min
 * @active: 0 - leave sleep mode, 1 - enter sleep mode
 * @enable: whether the rule enable or not
 *
 * Interface for DBus(SET_SLEEP_STATUS)
 * Clear old rules and set new rules
 * Returns    0    -   success; 
 *               -1   -   Rule num beyond the limit 
 *               -2   -   Chain add fail
 */
int setSleepModeSchedRule(unsigned char action, unsigned char weekday, unsigned char startHour, 
	unsigned char startMin, unsigned char active, unsigned char enable)
{
	int i, total;
	MIB_CE_RG_SLEEPMODE_SCHED_T rule;
	int ret;

	total = mib_chain_total(MIB_SLEEP_MODE_SCHED_TBL);
	if((100 == total)&&(0 == action))
	{
		//Table full, can not add yet.
		return -1;
	}

	if(0 == action)
	{
		//add new rules
		memset(&rule, 0, sizeof(rule));
		rule.enable = enable;
		rule.onoff = active;
		rule.hour = startHour;
		rule.minute = startMin;
		rule.day = weekday;

		if(!mib_chain_add(MIB_SLEEP_MODE_SCHED_TBL, (void *)(&rule)))
		{
			//Chain Add Error
			return -2;
		}
	}
	else
	{
		//delete rule
		for(i = 0; i < total; i++)
		{
			memset(&rule, 0, sizeof(rule));
			mib_chain_get(MIB_SLEEP_MODE_SCHED_TBL, i, &rule);
			if((rule.onoff==active)
				&&(rule.hour==startHour)
				&&(rule.minute==startMin)
				&&(rule.day==weekday))
			{
				//exist
				mib_chain_delete(MIB_SLEEP_MODE_SCHED_TBL, i);
				break;
			}
		}
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	updateScheduleCrondFile("/var/spool/cron/crontabs", 0);
	return 0;
}


/**
 * getPSModeSchedRule - get config rule of powersave mode schedule
 * @rule: base addr of point of powersave entry array.
 * @count: the length of array
 *
 * Interface for DBus(GET_SLEEP_STATUS)
 * Get current rules
 * Returns    0    -   success; 
 *               -1   -   patameter is NULL pointer 
 *               -2   -   Chain Get fail
 */
int getSleepModeSchedRule(MIB_CE_RG_SLEEPMODE_SCHED_T** pPEntry, int* count)
{
	int i, totalEntry;
	MIB_CE_RG_SLEEPMODE_SCHED_T* pSleepModeEntry;

	if((pPEntry == NULL) || (count == NULL))
	{
		//Null pointer.
		return -1;
	}
	
	totalEntry = mib_chain_total(MIB_SLEEP_MODE_SCHED_TBL);
	*count = totalEntry;
	pSleepModeEntry = (MIB_CE_RG_SLEEPMODE_SCHED_T*) malloc(sizeof(MIB_CE_RG_SLEEPMODE_SCHED_T)*totalEntry);
	for(i = 0; i < totalEntry; i++)
	{
		memset(pSleepModeEntry+i, 0, sizeof(MIB_CE_RG_SLEEPMODE_SCHED_T));
		if(!mib_chain_get(MIB_SLEEP_MODE_SCHED_TBL, i, (void *)(pSleepModeEntry+i)))
		{
			//Chain Get Error
			free(pSleepModeEntry);
			return -2;
		}
	}
	*pPEntry = pSleepModeEntry;
	return 0;
}
