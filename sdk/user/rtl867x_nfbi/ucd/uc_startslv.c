#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "utility.h"


#define UCRELAY_RUNFILE	"/var/run/ucrelay.pid"
static void uc_notify_ucrelay(void)
{
	int p;

	p=read_pid( UCRELAY_RUNFILE );
	if(p>0)
		kill( p, SIGUSR1);
	else
		printf( "%s(): can't find %s\n", __FUNCTION__, UCRELAY_RUNFILE );
	return;
}

int uc_startslv(void)
{
	int  sysret, ret=0;
	char sysbuf[128];
	int verify_image=0;
	
	//nfbi fw_w_boot 0 /etc/fw/boot.bin /etc/fw/linux.bin
	sprintf( sysbuf, "/bin/nfbi fw_w_boot %d %s %s",
			verify_image,		//verify image or not
			"/etc/fw/boot.bin", //bootloader
			"/etc/fw/linux.bin" //image
			);

	printf( "%s: system=%s\n", __FUNCTION__, sysbuf );
	sysret=system( sysbuf );
	if( WEXITSTATUS(sysret)!=0 )
	{
		printf( "call /bin/nfbi to init slave firmware failed!\n" );
		ret=-1;
	}else{
		//notify ucrelay
		uc_notify_ucrelay();	
		if(sleep(1)) sleep(1);

		printf( "%s: init slave firmware OK!\n", __FUNCTION__ );
		mib_slv_sync_all();
		printf( "%s: sync slave mib\n", __FUNCTION__ );
		sys_slv_init( UC_STR_DSL_INIT );
		printf( "%s: init slave dsl\n", __FUNCTION__ );
	}

	return ret;
}


int main(int argc, char **argv)
{
	int ret;
	ret=uc_startslv();
	return ret;
}

#if 0 /*use udp channel for message from startup.c*/
int startSlv(void)
{
	int  sysret, ret=0;
	char sysbuf[128], value[16], sysip[16], sysip4slv[16], sysnetmask[16];

	mib_get(MIB_ADSL_LAN_IP, (void *)value);
	strncpy(sysip, inet_ntoa(*((struct in_addr *)value)), 16);
	sysip[15] = '\0';

	mib_get(MIB_ADSL_LAN_SUBNET, (void *)value);
	strncpy(sysnetmask, inet_ntoa(*((struct in_addr *)value)), 16);
	sysnetmask[15] = '\0';

	mib_get(MIB_ADSL_SLV_LAN_IP, (void *)value);
	strncpy(sysip4slv, inet_ntoa(*((struct in_addr *)value)), 16);
	sysip4slv[15] = '\0';

#ifdef _UC_USE_RELAY_
	sprintf( sysbuf, "/bin/ucrelay %s", sysip4slv );
	printf( "system(): %s\n", sysbuf );
	sysret=system( sysbuf );
	if( WEXITSTATUS(sysret)!=0 )
		printf( "exec ucrelay failed!\n" );
#endif /*_UC_USE_RELAY_*/
	
	//va_cmd() can't get the return status
#if 1
	sprintf( sysbuf, "/bin/nfbi tftp_fw_w_boot %02x%02x%02x%02x%02x%02x %s %s %s %s %s",
			mac4slave[0],mac4slave[1],mac4slave[2],mac4slave[3],mac4slave[4],mac4slave[5], //slave mac
			sysip4slv,	//slave ip
			sysnetmask, //subnet mask
			sysip,		//tftp server ip
			"/etc/fw/boot.bin", //bootloader
			"/etc/fw/linux.bin" //image
			);
#else
	sprintf( sysbuf, "/bin/nfbi tftp_fw_w_boot %02x%02x%02x%02x%02x%02x %s %s %s %s %s",
			mac4slave[0],mac4slave[1],mac4slave[2],mac4slave[3],mac4slave[4],mac4slave[5], //slave mac
			sysip4slv,	//slave ip
			sysnetmask, //subnet mask
			"192.168.1.50", 	//tftp server ip
			"/etc/fw/boot.bin", //bootloader
			"vm.img" 			//image
			); 
#endif
	printf( "system(): %s\n", sysbuf );
	sysret=system( sysbuf );
	if( WEXITSTATUS(sysret)!=0 )
	{
		printf( "init slave firmware failed!\n" );
		//ret=-1;
	}else{
		if(sleep(1)) sleep(1);
		printf( "init slave firmware OK!\n" );
		printf( "sync mib and init dsl\n");
		mib_slv_sync();
		sys_slv_init( UC_STR_DSL_INIT );
	}

	return ret;
}
#endif

