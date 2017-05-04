#include <debug.h>

FILE *fp_debug=NULL;

int cdh_debug_init(void)
{
	fp_debug = fopen("/dev/console", "w");
	//fp_debug = fopen("/var/bftpd_debug", "w");
	if ( fp_debug == NULL )
	{
		DEBUG_PRINT("Fail to open \"/var/tty\"\n");
		return -1;
	}
	else
	{
		DEBUG_PRINT("cdh_debug_init successfully!\n");
		DEBUG_PRINT("Success to open \"/var/tty\",fp=%d\n",fp_debug);
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
