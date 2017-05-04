#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"
#include "voip_params.h"

#define LED_TEST	1
#define TEST_TIME 3000000

/*
 *  LED control sample code.
 *  1. Please killall solar_monitor before running this sample code.
 *	2. led_ctrl chid &
 *
 */

int main(int argc, char *argv[])
{
	int chid = 0;
	if(argc < 2)
	{
		printf("usage: %s chid \n", argv[0]);
		return 0;
	}

//	sscanf(argv[1], "%d", &chid);
	chid = atoi(argv[1]);
	printf("chid = [%d]\n", chid);
	rtk_OffHookAction(chid);

	return 0;
}
