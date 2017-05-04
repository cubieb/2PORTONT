#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "voip_manager.h"

int main(int argc, char *argv[])
{
	if ((argc == 4) || (argc == 5))
	{
		char vmwi_on = 1, vmwi_off = 0;
		/* fsk area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
		rtk_SetFskArea(atoi(argv[1])/*chid*/, atoi(argv[2])|0x100/*area*/);
	
		//FSK_PARAM_NULL = 0,
		//FSK_PARAM_DATEnTIME = 0x01,	// Date and Time
		//FSK_PARAM_CLI = 0x02,		// Calling Line Identify (CLI)
		//FSK_PARAM_CLI_ABS = 0x04,	// Reason for absence of CLI
		//FSK_PARAM_CLI_NAME = 0x07,	// Calling Line Identify (CLI) Name
		//FSK_PARAM_CLI_NAME_ABS = 0x08,	// Reason for absence of (CLI) Name
		//FSK_PARAM_MW = 0x0b,		// Message Waiting

		TstFskClid clid;
		int size=0;
	
		clid.ch_id = atoi(argv[1]);
        clid.service_type = 0; 					//service type 1
        

#if 0	// Date_time before Message waiting
		if (argv[4])
		{
        	clid.cid_data[0].type = 0x01;			//DATE and Time
		    strcpy(clid.cid_data[0].data, argv[4]);
		
			clid.cid_data[1].type = 0x0B;			//Message Waiting
			if (atoi(argv[3]) == 1) //on
				strcpy(clid.cid_data[1].data, &vmwi_on);
			else
				strcpy(clid.cid_data[1].data, &vmwi_off);

			size = 2;
		}
		else
		{
			clid.cid_data[0].type = 0x0B;			//Message Waiting
			if (atoi(argv[3]) == 1) //on
				strcpy(clid.cid_data[0].data, &vmwi_on);
			else
				strcpy(clid.cid_data[0].data, &vmwi_off);

			size = 1;
        	
			clid.cid_data[1].type = 0;
			printf("date and time is NULL\n");
		}
        
#else	// Message waiting before  Date_time
        clid.cid_data[0].type = 0x0B;			//Message Waiting
		if (atoi(argv[3]) == 1) //on
			strcpy(clid.cid_data[0].data, &vmwi_on);
		else
			strcpy(clid.cid_data[0].data, &vmwi_off);
		size++;

		if (argv[4])
		{
        	clid.cid_data[1].type = 0x01;			//DATE and Time
	        strcpy(clid.cid_data[1].data, argv[4]);
			size++;
		}
		else
		{
        	clid.cid_data[1].type = 0;
			printf("date and time is NULL\n");
		}
#endif
	
		//Only 2 elements for VMWI data. Set other element to 0 (MUST)
        clid.cid_data[2].type = 0;
        clid.cid_data[3].type = 0;
        clid.cid_data[4].type = 0;
			
		rtk_GenFskVmwi(atoi(argv[1])/*chid*/, &clid, size);
	}
	else
	{
		printf("Usage error! Method: vmwigen chid fsk_area on(1)/off(0) date_time\n");
		printf("Note:\n");
		printf(" - fsk_area: 0 -> Bellcore, 1 -> ETSI, 2 -> BT, 3 -> NTT\n");
		printf(" - date_time is optional.\n");
		printf("Example of VMWI chid 0, area ETSI, on ,date_time 010203 (Bellcore)\n");
		printf("  vmwigen 0 1 1 010203\n"); 
	}

	return 0;
}

