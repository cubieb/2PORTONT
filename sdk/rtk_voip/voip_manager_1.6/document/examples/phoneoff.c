#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"
#include "voip_control.h"

int main(int argc, char* argv[]){
	int opt = 0;
	int chid = 0,time = 0;
	

	
	if ( argc != 5 )
	{
		printf("Please input channel id and off hook time\n");
		printf("\t-c : channel id = 0 or 1\n");
		printf("\t-t : \n");
		printf("\t\t time(ms)\n");
		return 0;
	}
	
	while (( opt = getopt(argc, argv, "c:t:")) != -1 )
	{
		switch (opt)
		{
			case 'c':
				sscanf(optarg, "%d", &chid);
				break;				
			case 't':
				sscanf(optarg, "%d", &time);
				break;								
			case ':':
				printf("need to more parameters\n");
				break;
		}
	}

	printf("chid = [%d], time = [%d]\n", chid, time);
	rtk_SetOffHookTime(chid, time);
	return 0;
}

