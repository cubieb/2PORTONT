#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "voip_manager.h"

//extern int rtk_SetRFC2833EnableABCDSignal(int bEnable);
//extern int rtk_SetRFC2833GenerateABCDSignal(uint32 chid, unsigned char signal);

int main(int argc, char* argv[]){
	int eventtype = 0;
	int opt = 0;
	int chid = 0, setting = 0;
#if 0
	if ( argc != 2 )
	{
		printf("Please input event id\n");
		return 0;
	}
#endif
#if 0
	int i = 0;
	for ( i = 0 ; i < 2 ; i ++ ){
		rtk_EnableABCDSignal(0, i);
		rtk_EnableABCDSignal(1, i);
	}
#endif	
	
	
	while (( opt = getopt(argc, argv, "c:s:e:")) != -1 )
	{
		switch (opt)
		{
			case 'c':
				sscanf(optarg, "%d", &chid);
				break;
			case 's':
				sscanf(optarg, "%d", &setting);
				break;
			case 'e':
				sscanf(optarg, "%d", &eventtype);
				break;
			case ':':
				printf("need to more parameters\n");
				break;
		}
	}

	printf("chid = [%d], setting = [%d], eventtype = [%d]\n", chid, setting, eventtype);

	rtk_EnableABCDSignal(chid, setting);
	
	rtk_GenerateABCDSignal(chid, eventtype);
	
#if 0	
	sscanf(argv[1], "%d", &eventtype);
	if ( (eventtype == 0) || (eventtype == 1) )
	{
		rtk_EnableABCDSignal(0, eventtype);
	}
	else
	{
		rtk_GenerateABCDSignal(0, eventtype);
	}
#endif	
	return 0;
}

