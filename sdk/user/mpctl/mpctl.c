#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <config/autoconf.h>

#ifdef CONFIG_USER_RTK_VOIP
#include "voip_manager.h"
#include "voip_params.h"
#endif

void usage(void);
int main(int argc, char *argv[])
{
	if (argc<=1)
		goto arg_err_rtn;
	if (!strcmp(argv[1], "led"))
	{	
		if(!strcmp(argv[2], "on")){
			//printf("%s(%d) rtk_SetLedDisplay ch0 LED_ON\n" , __FUNCTION__ , __LINE__);
			#ifdef CONFIG_USER_RTK_VOIP
			rtk_SetLedDisplay( 0, 0, LED_ON );
			rtk_SetLedDisplay( 1, 0, LED_ON );
			#endif
			system("echo 1 > /proc/mptest");
			#ifdef CONFIG_USER_WIRELESS_TOOLS
			system("echo 1 > /proc/wlan0/led");
			system("echo 1 > /proc/wlan1/led");
			system("echo 1 >> /proc/wlan1/led");
			#endif

		}else if(!strcmp(argv[2], "off")){
			//printf("%s(%d) rtk_SetLedDisplay ch0 LED_OFF\n" , __FUNCTION__ , __LINE__);
			#ifdef CONFIG_USER_RTK_VOIP
			rtk_SetLedDisplay( 0, 0, LED_OFF );
			rtk_SetLedDisplay( 1, 0, LED_OFF );
			#endif
			system("echo 0 > /proc/mptest");
			#ifdef CONFIG_USER_WIRELESS_TOOLS
			system("echo 0 > /proc/wlan0/led");
			system("echo 0 > /proc/wlan1/led");
			system("echo 0 >> /proc/wlan1/led");
			#endif

		}else if(!strcmp(argv[2], "restore")){
			#ifdef CONFIG_USER_RTK_VOIP
			system("echo L > /var/run/solar_control.fifo");
			#endif
			#ifdef CONFIG_USER_WIRELESS_TOOLS
			system("echo 2 > /proc/wlan0/led");
			system("echo 2 > /proc/wlan1/led");
			system("echo 2 >> /proc/wlan1/led");
			#endif
			system("echo 2 > /proc/mptest");
		}else{
			goto arg_err_rtn;
		}
	}

	return 0;
	arg_err_rtn:
	usage();
	exit(1);

}

void usage(void)
{
	printf("mpctl led on\n");
	printf("mpctl led off\n");
	printf("mpctl led restore\n");
}



