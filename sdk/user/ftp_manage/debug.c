#include <stdio.h>
#include "debug.h"

FILE *fp_debug=NULL;

int cdh_debug_init(void)
{
	fp_debug = fopen("/dev/console", "w");
	//fp_debug = fopen("/var/ftp_manage_debug", "w");
	if ( fp_debug == NULL )
	{
		printf("Fail to open \"/dev/console\"\n");
		return -1;
	}
	else
	{
		DEBUG_PRINT("cdh_debug_init successfully!\n");
		printf("Success to open \"/dev/console\"\n");
		return 0;
	}
}

void cdh_debug_cleanup(void)
{
	if(fp_debug){
		fclose(fp_debug);
		fp_debug = NULL;
	}
}
