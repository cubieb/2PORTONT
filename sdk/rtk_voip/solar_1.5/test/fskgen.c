#include "voip_manager.h"

int fskgen_main(int argc, char *argv[])
{
	if (argc == 7)
	{
		if (atoi(argv[2]) == 1)//type I
		{
			//DSP default is soft gen, no need to set soft gen mode
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[3])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			// API to gen FSK caller ID and auto ring by DSP
			rtk_Gen_FSK_CID(atoi(argv[1]), argv[4]/*cid*/, argv[6], argv[5]/*name*/, 0);
#elif 0
			// API to gen caller ID and auto ring by DSP
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], argv[6], argv[5]/*name*/, 0/*type1*/, 0);
#else
			//FSK_PARAM_NULL = 0,
			//FSK_PARAM_DATEnTIME = 0x01,	// Date and Time
			//FSK_PARAM_CLI = 0x02,		// Calling Line Identify (CLI)
			//FSK_PARAM_CLI_ABS = 0x04,	// Reason for absence of CLI
			//FSK_PARAM_CLI_NAME = 0x07,	// Calling Line Identify (CLI) Name
			//FSK_PARAM_CLI_NAME_ABS = 0x08,	// Reason for absence of (CLI) Name
			//FSK_PARAM_MW = 0x0b,		// Message Waiting
	
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
        		clid.service_type = 0; //service type 1
        
        		clid.cid_data[0].type = 0x01;
        		strcpy(clid.cid_data[0].data, argv[6]);//DATE

			if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') ) //Private or Out of area
				clid.cid_data[1].type = 0x04;
        		else
        			clid.cid_data[1].type = 0x02;
        		strcpy(clid.cid_data[1].data, argv[4]);	//CLI
        		
        		if ( (argv[5][0] == 'P') || (argv[5][0] == 'O') ) //Private or Out of area
        			clid.cid_data[2].type = 0x08;
        		else
               			clid.cid_data[2].type = 0x07;
        		strcpy(clid.cid_data[2].data, argv[5]);	//CLI_NAME

			//Only 3 elements for Caller ID data. Set other element to 0 (MUST)
        		clid.cid_data[3].type = 0;
        		clid.cid_data[4].type = 0;
			
			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 3);
#endif
		}
		else if (atoi(argv[2]) == 2)//type II
		{
			//DSP default is soft gen, no need to set soft gen mode
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[3])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			rtk_Gen_FSK_CID(atoi(argv[1])/*chid*/, argv[4]/*cid*/, argv[6], argv[5]/*name*/, 1);
#elif 0
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], argv[6], argv[5]/*name*/, 1/*type1*/, 0);
#else
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
        		clid.service_type = 1;  //service type 2
        
        		clid.cid_data[0].type = 0x01;
        		strcpy(clid.cid_data[0].data, argv[6]);	//DATE

        		if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') ) //Private or Out of area
				clid.cid_data[1].type = 0x04;
        		else
        			clid.cid_data[1].type = 0x02;
        		strcpy(clid.cid_data[1].data, argv[4]);	//CLI
        		
               		if ( (argv[5][0] == 'P') || (argv[5][0] == 'O') ) //Private or Out of area
        			clid.cid_data[2].type = 0x08;
        		else
               			clid.cid_data[2].type = 0x07;
        		strcpy(clid.cid_data[2].data, argv[5]);	//CLI_NAME
        
        		//Only 3 elements for Caller ID data. Set other element to 0 (MUST)
        		clid.cid_data[3].type = 0;
        		clid.cid_data[4].type = 0;
			
			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 3);
#endif
		}
		else
			printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
	}
	else if (argc == 5)// no name, date, time
	{
		if (atoi(argv[2]) == 1)//type I
		{
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[3])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			rtk_Gen_FSK_CID(atoi(argv[1]), argv[4]/*cid*/, 0/*date*/, 0/*name*/, 0);
#elif 0
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], 0/*date*/, 0/*name*/, 0/*type1*/, 0);
#else
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
        		clid.service_type = 0;  //service type 1
        
        		if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') )
				clid.cid_data[0].type = 0x04;
        		else
        			clid.cid_data[0].type = 0x02;
        		strcpy(clid.cid_data[0].data, argv[4]);	//CLI

			clid.cid_data[1].type = 0;        		
			clid.cid_data[2].type = 0;
			clid.cid_data[3].type = 0;
        		clid.cid_data[4].type = 0;

			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 1);
#endif
		}
		else if (atoi(argv[2]) == 2)//type II
		{
			rtk_Set_FSK_Area(atoi(argv[1])/*chid*/, atoi(argv[3])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
#if 0
			rtk_Gen_FSK_CID(atoi(argv[1])/*chid*/, argv[4]/*cid*/, 0/*date*/, 0/*name*/, 1);
#elif 0
			rtk_Gen_CID_And_FXS_Ring(atoi(argv[1]), 1/*fsk*/, argv[4], 0/*date*/, 0/*name*/, 1/*type1*/, 0);
#else
			TstFskClid clid;
	
			clid.ch_id = atoi(argv[1]);
        		clid.service_type = 1;	 //service type 2
        
        		if ( (argv[4][0] == 'P') || (argv[4][0] == 'O') )
				clid.cid_data[0].type = 0x04;
        		else
        			clid.cid_data[0].type = 0x02;
        		strcpy(clid.cid_data[0].data, argv[4]);
        		
        		clid.cid_data[1].type = 0;        		
			clid.cid_data[2].type = 0;
			clid.cid_data[3].type = 0;
        		clid.cid_data[4].type = 0;

			rtk_Gen_MDMF_FSK_CID(atoi(argv[1])/*chid*/, &clid, 1);
#endif
		}
		else
			printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
	}
	else if (argc == 11) 
	{
		TstVoipFskPara para;
		
		if (argv[1][0] == 's') //set para
		{
			para.ch_id = atoi(argv[2]);
			para.area = atoi(argv[3]);
			para.CS_cnt = atoi(argv[4]);
			para.mark_cnt = atoi(argv[5]);
			para.mark_gain = atoi(argv[6]);
			para.space_gain = atoi(argv[7]);
			para.type2_expected_ack_tone = argv[8][0];
			para.delay_after_1st_ring = atoi(argv[9]);
			para.delay_before_2nd_ring = atoi(argv[10]);
			
			rtk_Set_FSK_CLID_Para(&para);
		}
	}
	else if (argc == 4) 
	{
		TstVoipFskPara para;
		
		if (argv[1][0] == 'g') //get para
		{
			para.ch_id = atoi(argv[2]);
			para.area = atoi(argv[3]);
			
			rtk_Get_FSK_CLID_Para(&para);
			
			printf("Get parameters:\n");
			printf("=================================\n");
			printf(" - ch seizure cnt = %d\n", para.CS_cnt);
			printf(" - mark cnt = %d\n", para.mark_cnt);
			printf(" - mark gain = %d\n", para.mark_gain);
			printf(" - space gain = %d\n", para.space_gain);
			printf(" - type2_expected_ack_tone = %c\n", para.type2_expected_ack_tone);
			printf(" - delay_after_1st_ring = %d ms\n", para.delay_after_1st_ring);
			printf(" - delay_before_2nd_ring = %d ms\n", para.delay_before_2nd_ring);
			printf("=================================\n");
		}
	}
	else
	{
		printf("Error! Usage:\n");
		printf("To send Caller ID ==> fskgen 'chid' 'fsk_type' 'fsk_area' 'caller_id' 'name' 'date_time'\n");
		printf("To set parameters ==> fskgen set 'chid' 'fsk_area' 'ch seizure cnt' 'mark cnt' 'mark gain' 'space gain' 'type-2 expected ack tone' 'delay_after_1st_ring' 'delay_before_2nd_ring'\n");
		printf("To set parameters ==> fskgen get 'chid' 'fsk_area'\n");
		
		printf("Example for Type 1:\n");
		printf("'fskgen 0 1 128 035780211 tester_B 01020304' will display number 035780211 and name, date, and time.(Bellcore)\n");
		printf("'fskgen 0 1 129 035780211 tester_E 04030201' will display number 035780211 and name, date, and time.(ETSI)\n");
		printf("'fskgen 0 1 131 035780211 tester_N 06020602' will display number 035780211 and name, date, and time.(NTT)\n");
		printf("'fskgen 0 1 0 035780211' will just display number 035780211 without name, date, and time.(Bellcore)\n");
		printf("'fskgen 0 1 1 035780211' will just display number 035780211 without name, date, and time.(ETSI)\n");
		printf("'fskgen 0 1 3 035780211' will just display number 035780211 without name, date, and time.(NTT)\n");
		
		printf("Example for set Bellcore parameters:\n");
		printf("  fskgen set 0 0 300 180 8 8 D 150 1000\n");
		printf("Example for get ETSI parameters:\n");
		printf("  fskgen get 0 1\n\n\n");
		
		printf("Note:\n");
		printf(" - fsk_type: 1 -> type1, 2 -> type2\n");
		printf(" - fsk_area: 0 -> Bellcore, 1 -> ETSI, 2 -> BT, 3 -> NTT\n");
		printf(" - gain 0 ~ gain 8: 8dB ~ 0dB\n");
		printf(" - gain 9 ~ gain 24: -1dB ~ -16dB\n");
		printf(" - type-2 expexted ack tone: character A or B or C or D\n");
		
	}
		

	return 0;
}

/*
		   - fsk_area[2:0] => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n" \
		"  - fsk_area[bit7]=> FSK date & time sync\n" \
		"  - fsk_area[bit6]=> reverse polarity before caller id (For FSK)\n" \
		"  - fsk_area[bit5]=> short ring before caller id (For FSK)\n" \
		"  - fsk_area[bit4]=> dual alert tone before caller id (For FSK)\n" \
		"  - fsk_area[bit3]=> caller id Prior Ring (FSK & DTMF)\n"
*/
