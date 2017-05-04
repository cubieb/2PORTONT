#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"

int main(int argc, char* argv[]){
	int opt = 0;
	int chid = 0, hz = 0, onTime = 0, offTime = 0, numMeters = 0;

	if ( argc != 11 )
	{
		printf("Please input channel id, hz(12/16)k, onTime, offTime, numMeters\n");
		printf("\t-c : channel id = 0 or 1\n");
		printf("\t-h : hz = 12 or 16\n");
		printf("\t-o : onTime 1 unit is 10ms, so if you assign 10, it is 100ms\n");
		printf("\t-f : offTime 1 unit is 10ms, so if you assign 10, it is 100ms\n");
		printf("\t-n : numMeters = cyle number\n");
		return 0;
	}
	
	while (( opt = getopt(argc, argv, "c:h:o:f:n:")) != -1 )
	{
		switch (opt)
		{
			case 'c':
				sscanf(optarg, "%d", &chid);
				break;
			case 'h':
				sscanf(optarg, "%d", &hz);
				break;
			case 'o':
				sscanf(optarg, "%d", &onTime);
				break;
			case 'f':
				sscanf(optarg, "%d", &offTime);
				break;
			case 'n':
				sscanf(optarg, "%d", &numMeters);
				break;					
			case ':':
				printf("need to more parameters\n");
				break;
		}
	}

	printf("chid = [%d], hz = [%d], onTime = [%d], offTime = [%d], numMeters = [%d]\n", chid, hz, onTime, offTime, numMeters);

	rtk_GenMeteringPulse(chid, hz, onTime, offTime, numMeters);
	
	return 0;
}

