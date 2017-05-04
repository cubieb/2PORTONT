#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <signal.h>
#include "utility.h"


static void set_led(unsigned char status)
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


int main (int argc, char **argv)
{
	if(!strcmp(argv[1],"0")){
		set_led(0);
	}
	else{
		set_led(1);
	}
	return 0;
}


