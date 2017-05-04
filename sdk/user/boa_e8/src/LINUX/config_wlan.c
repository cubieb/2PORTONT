#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <signal.h>
#include "utility.h"


static void start_wifi(unsigned char start)
{
	unsigned char vChar;
	unsigned char wlanDisabled = start? 0:1;
	MIB_CE_MBSSIB_T Entry;
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	int orig_idx = wlan_idx;
#endif
	int i, j;
	for(i=0; i<NUM_WLAN_INTERFACE; i++){
		wlan_idx = i;
		mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
		if(wlanDisabled == Entry.wlanDisabled)
			continue;
		Entry.wlanDisabled = wlanDisabled;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, 0);
#if 0
		for(j=0; j<NUM_VWLAN_INTERFACE; j++){
			mib_chain_get(MIB_MBSSIB_TBL, j+1, (void *)&Entry);
			if(wlanDisabled == Entry.wlanDisabled)
				continue;
			Entry.wlanDisabled = wlanDisabled;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, j+1);
		}
#endif

#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
		update_wps_configured(0);
#endif
	}
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = orig_idx;
#endif

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY

	if(start==0)
		config_WLAN(ACT_STOP);
	else
		config_WLAN(ACT_START);

}

int main (int argc, char **argv)
{
	if(!strcmp(argv[1],"0")){
		start_wifi(0);
	}
	else{
		start_wifi(1);
	}
	return 0;
}


