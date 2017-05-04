#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <signal.h>
#include "utility.h"
#define FON_TEST

void startFonCoovaChilli(unsigned char start)
{
	unsigned char vChar;
	if(mib_get(MIB_FON_ONOFF, (void *)&vChar)!=0){
		if(vChar != start){
			vChar = start;
			mib_set(MIB_FON_ONOFF, (void *)&vChar);
			config_WLAN(ACT_RESTART);
#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif // of #if COMMIT_IMMEDIATELY
		}
	}
}

int main (int argc, char **argv)
{
	if(!strcmp(argv[1],"0")){
		startFonCoovaChilli(0);
	}
	else{
		startFonCoovaChilli(1);
	}
	return 0;
}
