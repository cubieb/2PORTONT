#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* for ioctl */
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>

#include <sys/signal.h>

#include "adsl_drv.h"	
#define VERSION_FILE "/etc/version"
#define HW_VERSION "8671"

#define RUNFILE "/var/run/ShowStatus_pid"

#define QC_USAGE 	\
"Usage:\tqc <test_item>\n\n\t \
<test_item>\n\t\t \
enable <status>    : Enable displaying DSL operational status\n\t\t \
disable <status> : Disable displaying DSL operation status\n\t\t \
datalinkspeed : Display upstream and downstream speed\n\t\t \
crc           : Display the number of CRC error packet\n\t\t \
swversion     : Display the Software Version\n\t\t \
hwversion     : Display the Hardware version\n\n" 

int main(int argc, char** argv)
{
	char strbuf[64], *strBld;
	FILE *fp;
	int argNum=1, s_pid;
	
	if ( argc > 1 ) 
	{

		if( !(strcmp( argv[argNum], "enable")) ) {
			if( !strcmp( argv[argNum+1], "status")) {
				system("ShowStatus &");
				return 0;
			} else {
				printf("%s", QC_USAGE); 	
				return 0;
			}
		}

		if( !(strcmp( argv[argNum], "disable")) ) {
			if( !strcmp( argv[argNum+1], "status")) {
				/* terminate displaying thread */
				s_pid = read_pid(RUNFILE);
				if(s_pid >=1 ) {
					if(kill(s_pid, SIGTERM) != 0) 
						printf("Cound not kill pid %d (ShowStatus)\n", s_pid);
				} else {
					printf("Can not find ShowStatus pid\n");
				}
			}
			return 0;
		}
		
#ifdef CONFIG_DEV_xDSL
		if ( !(strcmp( argv[argNum], "datalinkspeed")) ){
			getAdslInfo(ADSL_GET_RATE_US, strbuf, sizeof(strbuf));
			printf("Upstream Speed: %d kbps\n", atoi(strbuf));
			
			getAdslInfo(ADSL_GET_RATE_DS, strbuf, sizeof(strbuf));
			printf("Downstream Speed: %d kbps\n", atoi(strbuf));			
			return 0;
		} 
		else if ( !(strcmp( argv[argNum], "crc")) ){
			getAdslInfo(ADSL_GET_CRC_DS, strbuf, sizeof(strbuf));
			printf("Downtream CRC error number: %d\n",  atoi(strbuf));

			getAdslInfo(ADSL_GET_CRC_US, strbuf, sizeof(strbuf));
			printf("Upstream CRC error number: %d\n",  atoi(strbuf));
			return 0;
		} 
#endif
		else if ( !(strcmp( argv[argNum], "swversion")) ){
			strbuf[0]=0;
			strBld = 0;
			fp = fopen(VERSION_FILE, "r");
			if (fp!=NULL) {
				fgets(strbuf, sizeof(strbuf), fp);  //main version
				fclose(fp);
				strBld = strchr(strbuf, ' ');
				*strBld=0;
			}
			printf( "Software Version: %s\n", strbuf);
			return 0;
		}	
		else if ( !(strcmp( argv[argNum], "hwversion")) ){
			printf( "Hardware Version: %s\n", HW_VERSION);
			return 0;
		}	

		else {
			printf("%s", QC_USAGE); 
		}	

	}
		printf("%s", QC_USAGE); 
	return 0;
}

