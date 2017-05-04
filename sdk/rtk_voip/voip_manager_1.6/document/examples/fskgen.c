#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "voip_manager.h"

unsigned int Gain2Val(int gain);
int Val2Gain(unsigned int val);



typedef enum
{
	EXAMPLE_FSK_PARAM_NULL = 0,				///< 
	EXAMPLE_FSK_PARAM_DATEnTIME = 0x01,			///< Date and Time
	EXAMPLE_FSK_PARAM_CLI = 0x02,				///< Calling Line Identify (CLI)
	EXAMPLE_FSK_PARAM_CLI_ABS = 0x04,			///< Reason for absence of CLI
	EXAMPLE_FSK_PARAM_CLI_NAME = 0x07,			///< Calling Line Identify (CLI) Name
	EXAMPLE_FSK_PARAM_CLI_NAME_ABS = 0x08,			///< Reason for absence of (CLI) Name
	EXAMPLE_FSK_PARAM_MW = 0x0b,				///< Message Waiting

}EXAMPLE_TfskParaType;

void test_P(void)
{
	TstVoipFskPara para;
	TstFskClid clid;

	// Get the Current setting
	para.ch_id = 0;
	para.area = 1; // Bellcore
	rtk_Get_FSK_CLID_Para(&para);

	// Change the params
	para.ch_id = 0;
	para.area = 1; // Bellcore
	para.CS_cnt = 300;
	para.mark_cnt = 80;
	para.mark_gain = 0xE;
	para.space_gain = 0xE;
			
	rtk_Set_FSK_CLID_Para(&para);


	// Gen FSK Caller ID
	clid.ch_id = 0;
        clid.service_type = 0; //service type 1

	clid.cid_data[0].type = EXAMPLE_FSK_PARAM_CLI_ABS;
	strcpy(clid.cid_data[0].data, "P");	//CLI
	
	// Set other element to 0 (MUST)
	clid.cid_data[1].type = 0;        		
	clid.cid_data[2].type = 0;
	clid.cid_data[3].type = 0;
        clid.cid_data[4].type = 0;

	rtk_GenMdmfFskCid(0, &clid, 1);
        
}

void test_O(void)
{
	TstVoipFskPara para;
	TstFskClid clid;

	// Get the Current setting
	para.ch_id = 0;
	para.area = 1; // Bellcore
	rtk_Get_FSK_CLID_Para(&para);

	// Change the params
	para.ch_id = 0;
	para.area = 1; // Bellcore
	para.CS_cnt = 300;
	para.mark_cnt = 80;
	para.mark_gain = 0xE;
	para.space_gain = 0xE;
			
	rtk_Set_FSK_CLID_Para(&para);


	// Gen FSK Caller ID
	clid.ch_id = 0;
        clid.service_type = 0; //service type 1

	clid.cid_data[0].type = EXAMPLE_FSK_PARAM_CLI_ABS;
	strcpy(clid.cid_data[0].data, "O");	//CLI
        
	clid.cid_data[1].type = EXAMPLE_FSK_PARAM_DATEnTIME;		// DATE_TIME
        strcpy(clid.cid_data[1].data, "01020304");

	// Set other element to 0 (MUST)
	//clid.cid_data[1].type = 0;        		
	clid.cid_data[2].type = 0;
	clid.cid_data[3].type = 0;
       clid.cid_data[4].type = 0;

	rtk_GenMdmfFskCid(0, &clid, 1);
        
}

int main(int argc, char *argv[])
{
	if (argc == 7)
	{
		if (atoi(argv[3]) == 1)//type I
		{
			//DSP default is soft gen, no need to set soft gen mode
			rtk_SetFskArea(atoi(argv[1])/*chid*/, atoi(argv[2])|0X100/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
	

			//=====Set the FSK Caller ID Parameters. 
			TstFskClid clid;
			memset(&clid, 0, sizeof(TstFskClid));
			clid.ch_id = atoi(argv[1]);

			//=====Set the FSK Caller ID Parameters-service type 1
        		clid.service_type = 0; 


			//=====Set the FSK Caller ID Parameters-DATE
        		clid.cid_data[0].type = EXAMPLE_FSK_PARAM_DATEnTIME;
        		strcpy(clid.cid_data[0].data, argv[6]);


			//=====Set the FSK Caller ID Parameters-Calling Line Identify (CLI)
        		clid.cid_data[1].type = EXAMPLE_FSK_PARAM_CLI;
        		strcpy(clid.cid_data[1].data, argv[4]);	


			//=====Set the FSK Caller ID Parameters-Calling Line Identify (CLI) Name
			clid.cid_data[2].type = EXAMPLE_FSK_PARAM_CLI_NAME;
			strcpy(clid.cid_data[2].data, argv[5]);	
			

			//Only 3 elements for Caller ID data. Set other element to 0 (MUST)
	        	clid.cid_data[3].type = 0;
	        	clid.cid_data[4].type = 0;

			
			//=====gen CID
			rtk_GenMdmfFskCid(atoi(argv[1])/*chid*/, &clid, 3);

		}
		else if (atoi(argv[3]) == 2)//type II
		{
			//DSP default is soft gen, no need to set soft gen mode
			rtk_SetFskArea(atoi(argv[1])/*chid*/, atoi(argv[2])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */


			//=====Set the FSK Caller ID Parameters. 
			TstFskClid clid;
			memset(&clid, 0, sizeof(TstFskClid));
			clid.ch_id = atoi(argv[1]);
			
			//=====Set the FSK Caller ID Parameters-service type 2
			clid.service_type = 1; 

			
			//=====Set the FSK Caller ID Parameters-DATE
			clid.cid_data[0].type = EXAMPLE_FSK_PARAM_DATEnTIME;
			strcpy(clid.cid_data[0].data, argv[6]);				


			//=====Set the FSK Caller ID Parameters-Calling Line Identify (CLI)
			clid.cid_data[1].type = EXAMPLE_FSK_PARAM_CLI;
			strcpy(clid.cid_data[1].data, argv[4]);	

			

			//=====Set the FSK Caller ID Parameters-Calling Line Identify (CLI) Name
			clid.cid_data[2].type = EXAMPLE_FSK_PARAM_CLI_NAME;
			strcpy(clid.cid_data[2].data, argv[5]);

			
	
			//Only 3 elements for Caller ID data. Set other element to 0 (MUST)
			clid.cid_data[3].type = 0;
			clid.cid_data[4].type = 0;

			
			/*before generating TYPE 2 FSK CID ,  phone need OFF-HOOK, enable PCM, enable DtmfDet */
			rtk_EnablePcm(atoi(argv[1]), 1);
			rtk_SetDtmfCfg(atoi(argv[1]), 1, 0);

			//=====gen CID
			rtk_GenMdmfFskCid(atoi(argv[1])/*chid*/, &clid, 3);
			uint32 tmp=-1;
			do
			{
				if(rtk_GetFskCidState(0, &tmp) != 0)
					break;
				usleep(50000);  // 50ms

			}while (tmp);
			/*send finish, disable */
			rtk_SetDtmfCfg(atoi(argv[1]), 0, 0);
			rtk_EnablePcm(atoi(argv[1]), 0);

		}
		else
			printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
	}
	else if (argc == 5)// no name, date, time
	{
		if (atoi(argv[3]) == 1)//type I
		{
			rtk_SetFskArea(atoi(argv[1])/*chid*/, atoi(argv[2])|0X100/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */

			//=====Set the FSK Caller ID Parameters. 
			TstFskClid clid;
			memset(&clid, 0, sizeof(TstFskClid));
			clid.ch_id = atoi(argv[1]);

			//=====Set the FSK Caller ID Parameters-service type 1
       		clid.service_type = 0;  

			
        		//=====Set the FSK Caller ID Parameters-Calling Line Identify (CLI)
			clid.cid_data[0].type = EXAMPLE_FSK_PARAM_CLI;
       		strcpy(clid.cid_data[0].data, argv[4]);	


			//Only 1 elements for Caller ID data. Set other element to 0 (MUST)
			clid.cid_data[1].type = 0;        		
			clid.cid_data[2].type = 0;
			clid.cid_data[3].type = 0;
        		clid.cid_data[4].type = 0;

			//=====gen CID
			rtk_GenMdmfFskCid(atoi(argv[1])/*chid*/, &clid, 1);

		}
		else if (atoi(argv[3]) == 2)//type II
		{
			rtk_SetFskArea(atoi(argv[1])/*chid*/, atoi(argv[2])/*area*/);   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */

			
			//=====Set the FSK Caller ID Parameters. 
			TstFskClid clid;
			memset(&clid, 0, sizeof(TstFskClid));
			clid.ch_id = atoi(argv[1]);


			//=====Set the FSK Caller ID Parameters-service type 2
	        	clid.service_type = 1;	 

		
			//=====Set the FSK Caller ID Parameters-Calling Line Identify (CLI)
	        	clid.cid_data[0].type = EXAMPLE_FSK_PARAM_CLI;
	        	strcpy(clid.cid_data[0].data, argv[4]);


			//Only 1 elements for Caller ID data. Set other element to 0 (MUST)
        		clid.cid_data[1].type = 0;        		
			clid.cid_data[2].type = 0;
			clid.cid_data[3].type = 0;
        		clid.cid_data[4].type = 0;


			/*before generating TYPE 2 FSK CID ,  phone need OFF-HOOK, enable PCM, enable DtmfDet */	
			rtk_EnablePcm(atoi(argv[1]), 1);
			rtk_SetDtmfCfg(atoi(argv[1]), 1, 0);

			//=====gen CID
			rtk_GenMdmfFskCid(atoi(argv[1])/*chid*/, &clid, 1);
			uint32 tmp=-1;
			do
			{
				if(rtk_GetFskCidState(0, &tmp) != 0)
					break;
				usleep(50000);  // 50ms

			}while (tmp);
				
			/*send finish, disable */
			rtk_SetDtmfCfg(atoi(argv[1]), 0, 0);
			rtk_EnablePcm(atoi(argv[1]), 0);

		}
		else
			printf("wrong fsk type: should be type-I(1) or type-II(2)\n");
	}
	else if ( ( argc == 19 ) || ( argc == 21 ) ) /* set */
	{
		TstVoipFskPara para;
		int mark_gain;
		int space_gain;
		
		if (argv[1][0] == 's') //set para
		{
			/* check Gain range */
			mark_gain = atoi(argv[6]);
			space_gain = atoi(argv[7]);

			if (mark_gain<-16 || mark_gain>8) {
				printf("\ninvalid mark_gain  : %d\n",mark_gain);
				printf("Range of mark_gain : -16dB ~ +8dB\n\n");
				return -1;
			}

			if (space_gain<-16 || space_gain>8) {
				printf("\ninvalid space_gain  : %d\n",space_gain);
				printf("Range of space_gain : -16dB ~ +8dB\n\n");
				return -1;
			}

			para.ch_id = atoi(argv[2]);
			para.area = atoi(argv[3]);
			para.CS_cnt = atoi(argv[4]);
			para.mark_cnt = atoi(argv[5]);
			para.mark_gain = Gain2Val(atoi(argv[6]));
			para.space_gain = Gain2Val(atoi(argv[7]));
			para.type2_expected_ack_tone = argv[8][0];
			para.delay_after_1st_ring = atoi(argv[9]);
			para.delay_before_2nd_ring = atoi(argv[10]);
			para.silence_before_sas = atoi(argv[11]);
			para.sas_time = atoi(argv[12]);
			para.delay_after_sas = atoi(argv[13]);
			para.cas_time = atoi(argv[14]);
			para.type1_delay_after_cas = atoi(argv[15]);
			para.ack_waiting_time = atoi(argv[16]);
			para.delay_after_ack_recv = atoi(argv[17]);
			para.delay_after_type2_fsk = atoi(argv[18]);
			if( para.area == FSK_ETSI )
			{
				para.RPAS_Duration = atoi(argv[19]);
				para.RPAS2FSK_Period = atoi(argv[20]);
			}
			else
			{			
				para.RPAS_Duration = 250;
				para.RPAS2FSK_Period = 650;
			}
			
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
			
			printf("FSK parameters of chid %d\n",para.ch_id);
			printf("=======================================\n");

			switch(para.area) {
				case FSK_Bellcore: 
					printf(" - fsk_area = Bellcore/Telcordia FSK\n");
				break;
				case FSK_ETSI: 
					printf(" - fsk_area = ETSI FSK\n");
				break;
				case FSK_BT: 
					printf(" - fsk_area = BT FSK\n");
				break;
				case FSK_NTT: 
					printf(" - fsk_area = NTT FSK\n");
				break;
				default:
					printf(" - fsk_area = %d\n",para.area);
				break;
			}

			printf(" - ch seizure cnt           = %4d bits\n", para.CS_cnt);
			printf(" - mark cnt                 = %4d bits\n", para.mark_cnt);
			//printf(" - mark value               = %4d\n", para.mark_gain);
			printf(" - mark  (Logic 1) +/-gain  = %4d dB\n", Val2Gain(para.mark_gain));
			//printf(" - space value              = %4d\n", para.space_gain);
			printf(" - space (Logic 0) +/-gain  = %4d dB\n", Val2Gain(para.space_gain));
			printf(" - type2_expected_ack_tone  = %4C\n", para.type2_expected_ack_tone);
			printf(" - delay_after_1st_ring     = %4d ms\n", para.delay_after_1st_ring);
			printf(" - delay_before_2nd_ring    = %4d ms\n", para.delay_before_2nd_ring);
			printf(" - silence_before_sas       = %4d ms\n", para.silence_before_sas);
			printf(" - sas_time                 = %4d ms\n", para.sas_time);
			printf(" - delay_after_sas          = %4d ms\n", para.delay_after_sas);
			printf(" - cas_time                 = %4d ms\n", para.cas_time);
			printf(" - type1_delay_after_cas    = %4d ms\n", para.type1_delay_after_cas);
			printf(" - ack_waiting_time         = %4d ms\n", para.ack_waiting_time);
			printf(" - delay_after_ack_recv     = %4d ms\n", para.delay_after_ack_recv);
			printf(" - delay_after_type2_fsk    = %4d ms\n", para.delay_after_type2_fsk);
			if( para.area == FSK_ETSI )
			{
				printf(" - RP-AS duration           = %4d ms\n", para.RPAS_Duration);
				printf(" - RP-AS to FSK data period = %4d ms\n", para.RPAS2FSK_Period);
			}
			printf("=======================================\n");

			printf("\nExample Command of FSK param set and gen:\n\n");
			if( para.area == FSK_ETSI )
			{
				printf("%s set %d %d %d %d %d %d %C %d %d %d %d %d %d %d %d %d %d %d %d\n",argv[0], para.ch_id, para.area, para.CS_cnt, 
				para.mark_cnt, Val2Gain(para.mark_gain), Val2Gain(para.space_gain),
				para.type2_expected_ack_tone, para.delay_after_1st_ring, 
				para.delay_before_2nd_ring, para.silence_before_sas,
				para.sas_time, para.delay_after_sas,
				para.cas_time, para.type1_delay_after_cas, para.ack_waiting_time,
				para.delay_after_ack_recv, para.delay_after_type2_fsk, para.RPAS_Duration, para.RPAS2FSK_Period);
			}else
			{
				printf("%s set %d %d %d %d %d %d %C %d %d %d %d %d %d %d %d %d %d\n",argv[0], para.ch_id, para.area, para.CS_cnt, 
				para.mark_cnt, Val2Gain(para.mark_gain), Val2Gain(para.space_gain),
				para.type2_expected_ack_tone, para.delay_after_1st_ring, 
				para.delay_before_2nd_ring, para.silence_before_sas,
				para.sas_time, para.delay_after_sas,
				para.cas_time, para.type1_delay_after_cas, para.ack_waiting_time,
				para.delay_after_ack_recv, para.delay_after_type2_fsk);
			}

			printf("%s %d %d 1 1955-2011 Steve.Jobs 10060700\n\n",argv[0], para.ch_id, para.area);
		}
	}
	else if (argc == 2)
	{
		if (argv[1][0] == 'P')
			test_P();
		else if (argv[1][0] == 'O')
			test_O();
	}
	else
	{
		printf("*****Usage*****\n");
		
		printf("-----To get parameters:------------------------------------\n");
		printf(" fskgen get <chid> <fsk_area>\n\n");
		printf("Example of getting ch0 Bellcore parameters:\n");
		printf("  fskgen get 0 0\n\n");
		
		printf("-----To set parameters for a specific FSK standard:--------------\n");
		printf("  fskgen set <chid> <fsk_area> <ch seizure cnt> <mark cnt>\n");
		printf("             <mark gain> <space gain> <type-2 expected ack tone>\n");
		printf("             <delay_after_1st_ring> <delay_before_2nd_ring>\n");
		printf("             <silence_before_sas> <sas_time>\n");
		printf("             <delay_after_sas> <cas_time> <type1_delay_after_cas>\n");
		printf("             <ack_waiting_time> <delay_after_ack_recv>\n");
		printf("             <delay_after_type2_fsk>\n");
		printf("             If fsk_area=1 (ETSI): <RPAS_Duration> <RPAS2FSK_Period>\n\n");	
		printf("Example of setting ch0 Bellcore FSK parameters:\n");
		printf("  fskgen set 0 0 300 180 0 0 D 150 1000 0 330 50 80 80 160 80 150\n\n");
		
		printf("-----To generate Caller ID:----------------------------------\n");
		printf("  fskgen <chid> <fsk_area> <type> <caller_id> <name> <date_time>\n");
		printf(" - fsk_area: 0 -> Bellcore, 1 -> ETSI, 2 -> BT, 3 -> NTT\n\n");
		
		/*type 1 example */
		printf("Example of type 1 CID displaying number 035780211 and name, date, and time.(Bellcore)\n");
		printf("  fskgen 0 0 1 035780211 tester_B 01020304\n"); 
		printf("Example of type 1 CID displaying number 035780211 and name, date, and time.(ETSI)\n");
		printf("  fskgen 0 1 1 035780211 tester_E 04030201\n"); 
		printf("Example of type 1 CID displaying number 035780211 and name, date, and time.(NTT)\n");
		printf("  fskgen 0 3 1 035780211 tester_N 06020602\n");
		printf("Example of type 1 CID displaying number 035780211 without name, date, and time.(Bellcore)\n");
		printf("  fskgen 0 0 1 035780211\n"); 
		printf("Example of type 1 CID displaying number 035780211 without name, date, and time.(ETSI)\n");
		printf("  fskgen 0 1 1 035780211\n");
		printf("Example of type 1 CID displaying number 035780211 without name, date, and time.(NTT)\n");
		printf("  fskgen 0 3 1 035780211\n\n");

		/*type 2 example, before send CID, OFF-hook first */
		printf("Example of type 2 CID displaying number 035780211 and name, date, and time.(Bellcore)\n");
		printf("  fskgen 0 0 2 035780211 tester_B 01020304\n"); 
		printf("Example of type 2 CID displaying number 035780211 without name, date, and time.(Bellcore)\n");
		printf("  fskgen 0 0 2 035780211\n");
		printf("[Note: Before send type 2 CID, OFF-hook first]\n\n");

		/*Reason for absence of CLI */
		printf("Example of Reason for absence of CLI [private]\n");
		printf("  fskgen P\n");
		printf("Example of Reason for absence of CLI [Out of area]\n");
		printf("  fskgen O\n");
		
	}
		

	return 0;
}

/*******************************************************
 * Convert Gain dB to value
 *
 * val  0 =   8 dB
 * val  1 =   7 dB
 * val  2 =   6 dB
 * val  3 =   5 dB
 * val  4 =   4 dB
 * val  5 =   3 dB
 * val  6 =   2 dB
 * val  7 =   1 dB
 * val  8 =   0 dB
 * val  9 =  -1 dB
 * val 10 =  -2 dB
 * val 11 =  -3 dB
 * val 12 =  -4 dB
 * val 13 =  -5 dB
 * val 14 =  -6 dB
 * val 15 =  -7 dB
 * val 16 =  -8 dB
 * val 17 =  -9 dB
 * val 18 = -10 dB
 * val 19 = -11 dB
 * val 20 = -12 dB
 * val 21 = -13 dB
 * val 22 = -14 dB
 * val 23 = -15 dB
 * val 24 = -16 dB
 ******************************************************/
unsigned int Gain2Val(int gain)
{
	int inGain = gain;

	/* valid gain: -16dB ~ 8dB */
    if (gain < -16) inGain = -16;
    if (gain > 8) 	inGain = 8;
    
    return (8-inGain);
}


int Val2Gain(unsigned int val)
{
	int inVal = val;

	/* valid val 0 ~ 24 */
    if (val>24) inVal = 24;

    return (8-inVal);
}

/*
		   - fsk_area[2:0] => 0 BELLCORE, 1: ETSI, 2: BT, 3: NTT\n" \
		"  - fsk_area[bit8]=> Auto SLIC Ringing\n" \
		"  - fsk_area[bit7]=> FSK date & time sync\n" \
		"  - fsk_area[bit6]=> reverse polarity before caller id (For FSK)\n" \
		"  - fsk_area[bit5]=> short ring before caller id (For FSK)\n" \
		"  - fsk_area[bit4]=> dual alert tone before caller id (For FSK)\n" \
		"  - fsk_area[bit3]=> caller id Prior Ring (FSK & DTMF)\n"
*/
