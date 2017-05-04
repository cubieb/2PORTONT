#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include "adsl_drv.h"	
#include <sys/types.h>
#include <unistd.h>

#include "utility.h"
#define RUNFILE "/var/run/ShowStatus_pid"

void StartShowStatusThread (void)
{
	char strbuf[64]={0};

	while(1) {
#ifdef CONFIG_DEV_xDSL
		getAdslInfo(ADSL_GET_STATE, strbuf, sizeof(strbuf));
#endif
		printf("DSL Operational Status : %s\n", strbuf);
		
		sleep(2);	
//		sync();
	}
}
/*
void cancelShowThread(void)
{
	if(pShowd)
		pthread_cancel(pShowd);
}

void startShowThread(void)
{
	if (pthread_create( &pShowd, 0, &StartShowStatusThread, 0 )!=0) {
		printf("Create show status pthread failed!\n");
	}

}
*/
int main(int argc, char *argv[])
{
	pid_t s_pid;
	FILE *filep;
	
	s_pid   = getpid();
	filep = fopen(RUNFILE,"w+");

	fprintf(filep, "%d", s_pid);
	fclose(filep);
	
	StartShowStatusThread();

	return 0;
}

