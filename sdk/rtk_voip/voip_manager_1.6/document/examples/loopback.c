#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"
#include "voip_control.h"

#define BUF_SIZE 128

int main(int argc, char* argv[]){
	int i;//, opt = 0;
	//int c0txts = 0, c0rxts = 0, c1txts = 0, c1rxts = 0;
	//char buffer[BUF_SIZE];
	
	//memset(buffer, 0x0, BUF_SIZE);
	//USAGE
	printf("PCM_Loop fxs 0 and fxs 1\n");
#if 0
	if ( argc != 9 )
	{
		printf("Please input channel id\n");
		printf("\t-t : ch 0 tx timeslot\n");
		printf("\t-r : ch 0 rx timeslot\n");
		printf("\t-x : ch 1 tx timeslot\n");
		printf("\t-f : ch 1 rx timeslot\n");		
		return 0;
	}
	
	while (( opt = getopt(argc, argv, "t:r:x:f:")) != -1 )
	{
		switch (opt)
		{		
			case 't':
				sscanf(optarg, "%d", &c0txts);
				break;								
			case 'r':
				sscanf(optarg, "%d", &c0rxts);
				break;												
			case 'x':
				sscanf(optarg, "%d", &c1txts);
				break;								
			case 'f':
				sscanf(optarg, "%d", &c1rxts);
				break;												
			case ':':
				printf("need to more parameters\n");
				break;
		}
	}	
#endif	
//	rtk_debug_with_watchdog(4, 0xff);

	// init 
	for (i=0; i<CON_CH_NUM; i++)
	{
		// init dsp
		rtk_InitDsp(i);

		rtk_SetBusFormat(i, AP_BUSDATFMT_PCM_LINEAR);
		//rtk_SetBusFormat(i, AP_BUSDATFMT_PCM_WIDEBAND_LINEAR);

		// init phone
		rtk_SetVoiceGain(i, 0, 0); // 0db


		rtk_OnHookReinit(i);
		rtk_EnablePcm(i,1);
		//rtk_EnablePcm(i,2);
	}
	rtk_Set_PCM_Loop_Mode(0, 1, 0, 1);	
	
#if 0
	// modify ch0 slic timeslot
	sprintf(buffer, "echo \"0 %d %d\" > /proc/slictimeslot", c0txts, c0rxts);
	system(buffer);
	
	//modify ch0 pcm timeslot
	rtk_SetPcmTimeslot(0, c0txts, c0rxts+16);
	usleep(3 * 1000);
	
	// modify ch1 slic timeslot
	sprintf(buffer, "echo \"1 %d %d\" > /proc/slictimeslot", c1txts, c1rxts);
	system(buffer);
	
	// modify ch1 pcm timeslot
	rtk_SetPcmTimeslot(1, c1txts, c1rxts+16);
#endif
	return 0;
	
}
