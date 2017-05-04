#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "uc_mib.h"

#define UC_ELANIF		"eth0"
#define UC_ELANMTU		1500

static void uc_sys_system(char *cmd)
{
	if(cmd)
	{
		int sysret;

		printf( "%s(): %s\n", __FUNCTION__, cmd );
		sysret=system( cmd );
		if(sysret==-1)
			printf( "%s(): exec failed(%d)!\n", __FUNCTION__, sysret );
	}
}

int uc_sys_eth_init(void)
{
	char buf[128];
	unsigned char mac[6],ip[4],mask[4];

	uc_mib_get( UC_MIB_ELAN_MAC_ADDR, mac );
	uc_mib_get( UC_MIB_ADSL_LAN_IP, ip );
	uc_mib_get( UC_MIB_ADSL_LAN_SUBNET, mask );

	sprintf( buf, "ifconfig %s down", UC_ELANIF );
	uc_sys_system( buf );

	sprintf( buf, "ifconfig %s hw ether %02x%02x%02x%02x%02x%02x",
					UC_ELANIF, mac[0],mac[1],mac[2],mac[3],mac[4],mac[5] );
	uc_sys_system( buf );

	sprintf( buf, "ifconfig %s %u.%u.%u.%u netmask %u.%u.%u.%u mtu %d",
					UC_ELANIF, ip[0],ip[1],ip[2],ip[3],mask[0],mask[1],mask[2],mask[3], UC_ELANMTU );
	uc_sys_system( buf );

	sprintf( buf, "ifconfig %s up", UC_ELANIF );
	uc_sys_system( buf );

	return 0;
}

int uc_sys_eth_down(void)
{
	char buf[128];

	sprintf( buf, "ifconfig %s down", UC_ELANIF );
	uc_sys_system( buf );

	return 0;
}

