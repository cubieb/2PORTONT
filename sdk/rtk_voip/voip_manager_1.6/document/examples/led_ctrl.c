#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"
#include "voip_params.h"

#define TEST_TIME 3000000

/*
 *  LED control sample code.
 *  1. Please killall solar_monitor before running this sample code.
 *	2. led_ctrl chid &
 *
 */

int main(int argc, char *argv[])
{	
	LedDisplayMode status = 0;
	printf("%s(%d) rtk_SetLedDisplay ch0 LED_ON\n" , __FUNCTION__ , __LINE__);
	rtk_SetLedDisplay( 0, 0, LED_ON );
	usleep(TEST_TIME); 

	rtk_GetLEDStatus(0, 0, &status);
	printf("%s(%d) rtk_SetLedDisplay ch0 status %d\n" , __FUNCTION__ , __LINE__, status);
	
	//===============================================================================
	printf("%s(%d) rtk_SetLedDisplay ch0 LED_BLINKING\n" , __FUNCTION__ , __LINE__);
	rtk_SetLedDisplay( 0, 0, LED_BLINKING );
	usleep(TEST_TIME); 

	rtk_GetLEDStatus(0, 0, &status);
	printf("%s(%d) rtk_SetLedDisplay ch0 status %d\n" , __FUNCTION__ , __LINE__, status);
	
	//===============================================================================
	printf("%s(%d) rtk_SetLedDisplay ch0 LED_OFF\n" , __FUNCTION__ , __LINE__);
	rtk_SetLedDisplay( 0, 0, LED_OFF );
	usleep(TEST_TIME); 

	rtk_GetLEDStatus(0, 0, &status);
	printf("%s(%d) rtk_SetLedDisplay ch0 status %d\n" , __FUNCTION__ , __LINE__, status);
	
	//===============================================================================	
	printf("%s(%d) rtk_SetLedDisplay ch1 LED_ON\n" , __FUNCTION__ , __LINE__);
	rtk_SetLedDisplay( 1, 0, LED_ON );
	usleep(TEST_TIME); 

	rtk_GetLEDStatus(1, 0, &status);
	printf("%s(%d) rtk_SetLedDisplay ch0 status %d\n" , __FUNCTION__ , __LINE__, status);
	
	//===============================================================================	
	printf("%s(%d) rtk_SetLedDisplay ch1 LED_BLINKING\n" , __FUNCTION__ , __LINE__);
	rtk_SetLedDisplay( 1, 0, LED_BLINKING );
	usleep(TEST_TIME); 		
	
	rtk_GetLEDStatus(1, 0, &status);
	printf("%s(%d) rtk_SetLedDisplay ch0 status %d\n" , __FUNCTION__ , __LINE__, status);
	
	//===============================================================================
	printf("%s(%d) rtk_SetLedDisplay ch1 LED_OFF\n" , __FUNCTION__ , __LINE__);
	rtk_SetLedDisplay( 1, 0, LED_OFF );
	usleep(TEST_TIME); 
	
	rtk_GetLEDStatus(1, 0, &status);
	printf("%s(%d) rtk_SetLedDisplay ch0 status %d\n" , __FUNCTION__ , __LINE__, status);
	
	//===============================================================================
	return 0;
}
