#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"


void test_O(void)
{
	TstFskClid clid;
	uint32 tmp;

	/*disable bit 8 , auto ring */
	rtk_SetFskArea(0,0x89|0x100);
	/* change slic mode to OHT */
	rtk_SetSlicLineVoltage (0,1);
 
	// Gen FSK Caller ID
	clid.ch_id = 0;
	clid.service_type = 0; //service type 1
 
	// clid.cid_data[0].type = 0x04;
	// strcpy(clid.cid_data[0].data, ""); //CLI
        
	clid.cid_data[0].type = 0x01;  // DATE_TIME
	strcpy(clid.cid_data[0].data, "05040302");
 
	// Set other element to 0 (MUST)
	clid.cid_data[1].type = 0;          
	clid.cid_data[2].type = 0;
	clid.cid_data[3].type = 0;
	clid.cid_data[4].type = 0;
 
	rtk_GenMdmfFskCid(0, &clid, 1);
 
	rtk_GetFskCidState(0,&tmp);

	/*check cid is send finish */
	while(tmp!=0){
		sleep( 1 );
		rtk_GetFskCidState(0,&tmp);
	}
 
	rtk_SetFskArea(0,0x89|0x100); //reset fsk area to enable bit 8     
}

int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		if (argv[1][0] == 'O')
			test_O();
	}
	else
	{
		printf("Usage:\n");
		printf("fsk_date O\n");
	}	

	return 0;
}

