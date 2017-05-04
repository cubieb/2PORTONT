
#include <stdio.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
//#include "../boa/src/LINUX/mib.h"
//#include "../boa/src/LINUX/utility.h"
#include <rtk/sysconfig.h>
#include <config/autoconf.h>

char g_InterfaceName[10];

int main (int argc, char** argv)	
{	   
	unsigned int entryNum, i;
	char InterfaceName[10];
	char account[70];
	MIB_CE_DDNS_T Entry;	
	int ipversion;
	
	//printf("updateddctrl start .....\n");	
   	
	if (argc != 3)
   	{
      		printf("Usage: updateddctrl InterfaceName IPVersion>\n");
			printf("IPVersion: \n");
			printf("1:  IPv4 only\n");
			printf("2:  IPv6 only\n");
			printf("3:  IPv4 and IPv6\n");			
      		printf("Example: updateddctrl ppp0 1\n");
      		//printf("Example: updateddctrl br0\n");
      		exit(0);
   	}
	
	// Save the interface names	
	strcpy(InterfaceName, argv[1]);		
	
	// Save IPVersion
	ipversion = atoi(argv[2]);
	
	//init. mib
	/*
	if ( mib_init() == 0 ) {
		printf("[upnpctrl] Initialize MIB failed!\n");
		return;
	}
	*/	
	
	//printf("InterfaceName is %s. ipversion is %d\n", InterfaceName, ipversion);
	cmd_ddnsctrl(InterfaceName, ipversion);			
	return 1;
}
