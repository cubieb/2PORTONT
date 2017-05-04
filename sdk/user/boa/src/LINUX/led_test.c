#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

static const char LEDTEST_PID[] = "/var/run/ledtest.pid";

int writepidfile(const char * fname, int pid)
{
	char pidstring[16];
	int pidstringlen;
	int pidfile;

	if(!fname || (strlen(fname) == 0))
		return -1;
	
	if( (pidfile = open(fname, O_WRONLY|O_CREAT, 0644)) < 0)
	{
		printf("Unable to open pidfile for writing %s\n", fname);
		return -1;
	}

	pidstringlen = snprintf(pidstring, sizeof(pidstring), "%d\n", pid);
	if(pidstringlen <= 0)
	{
		printf( "Unable to write to pidfile %s: snprintf(): FAILED\n", fname);
		close(pidfile);
		return -1;
	}
	else
	{
		if(write(pidfile, pidstring, pidstringlen) < 0)
			printf("Unable to write to pidfile %s\n", fname);
	}

	close(pidfile);

	return 0;
}


int main(int argc, char** argv)
{
	int pid;
	FILE * fpled;
	char buf[50];
	int hit = 0;

	//printf("led test get start\n");

	pid = getpid();
	writepidfile(LEDTEST_PID,pid);
	
	while(1)
	{
		sleep(1);
		fpled = fopen("/proc/led_test","r");
		if(fpled)
		{
			while (!feof(fpled))
			{
				memset(buf, 0, sizeof(buf));
				fgets(buf, sizeof(buf), fpled);
				
				if((strncmp(buf,"RESET", 5) == 0)
					|| (strncmp(buf,"WPS", 3) == 0)
					|| (strncmp(buf,"WLAN", 4) == 0))
				{				
					printf(buf);
					hit = 1;
				}
			}
			fclose(fpled);
			if(hit)
			{
				//printf("echo clean > /proc/led_test\n");
				system("echo clean > /proc/led_test");
				hit = 0;
			}
		}		
	}
	return 0;
}

