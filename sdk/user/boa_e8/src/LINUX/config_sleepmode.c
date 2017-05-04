#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <signal.h>
#include "utility.h"

extern int wlan_idx;
#define WRITE_MEM32(addr, val)   (*(volatile unsigned int *)   (addr)) = (val)
#define READ_MEM32(addr)         (*(volatile unsigned int *)   (addr))

static void set_sleepmode(unsigned char enable)
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


int main (int argc, char **argv)
{
	if(!strcmp(argv[1],"0")){
		set_sleepmode(0);
	}
	else{
		set_sleepmode(1);
	}
	return 0;
}


