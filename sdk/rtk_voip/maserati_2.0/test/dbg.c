#include "voip_manager.h"

TstVoipCadence RingCad;
TstVoipCadence* pRingCad = &RingCad;


int dbg_main(int argc, char *argv[])
{
	if (argc == 2 || argc == 3)
	{
		int dbg;
		int watchdog;

		dbg = atoi(argv[1]);
		watchdog = ( argc == 3 ? atoi(argv[2]) : 0xff );
		rtk_debug_with_watchdog(dbg, watchdog);
		printf("set dbg flag = %d\n", dbg);
	}
	else if (argc == 4)
	{
		if (atoi(argv[1]) == 1)
		{
			// priori 1st ring, auto AC, auto ring, digit duration = 80ms, inter digit pause = 80ms
			if (atoi(argv[3]) == 0)
				rtk_SetDtmfCidParam(atoi(argv[2]), DTMF_DIGIT_A, DTMF_DIGIT_C, 0xF, 80, 80, 300, 300);	// ETSI start/end
			else if (atoi(argv[3]) == 1)
				rtk_SetDtmfCidParam(atoi(argv[2]), DTMF_DIGIT_A, DTMF_DIGIT_HASH, 0xF, 80, 80, 300, 300); //Denmark start/end
			else if (atoi(argv[3]) == 2)
				rtk_SetDtmfCidParam(atoi(argv[2]), DTMF_DIGIT_STAR, DTMF_DIGIT_HASH, 0xF, 80, 80, 300, 300); // Just for test
			else if (atoi(argv[3]) == 3)
				rtk_SetDtmfCidParam(atoi(argv[2]), DTMF_DIGIT_1, DTMF_DIGIT_2, 0xF, 80, 80, 300, 300); // Just for test, auto 1 start, 2 end
			else if (atoi(argv[3]) == 4)
				rtk_SetDtmfCidParam(atoi(argv[2]), DTMF_DIGIT_C, DTMF_DIGIT_8, 0xF, 80, 80, 300, 300); // Just for test, auto C start, 8 end
		}
		else if(atoi(argv[1]) == 2)	
		{
			// after 1st ring, non-auto AC, auto ring, digit duration = 120ms, inter digit pause = 120ms
			rtk_SetDtmfCidParam(atoi(argv[2]), DTMF_DIGIT_A, DTMF_DIGIT_C, 0x8, 120, 120, 300, 300); 
		}
		else if(atoi(argv[1]) == 3)	
		{
			rtk_GenDtmfCid(atoi(argv[2]), "654321");
		}
		else if(atoi(argv[1]) == 4)	
		{
			if (atoi(argv[3]) == 0)
				rtk_GenDtmfCid(atoi(argv[2]), "A3939080C"); // ETSI start/end
			else if (atoi(argv[3]) == 1)
				rtk_GenDtmfCid(atoi(argv[2]), "A3939080#"); //Denmark start/end
			else if (atoi(argv[3]) == 2)
				rtk_GenDtmfCid(atoi(argv[2]), "*3939080#"); // Just for test
			else if (atoi(argv[3]) == 3)
				rtk_GenDtmfCid(atoi(argv[2]), "139390802"); // Just for test, 1 start, 2 end
			else if (atoi(argv[3]) == 4)
				rtk_GenDtmfCid(atoi(argv[2]), "A39390803"); // Just for test, A start, 3 end

		}
		else if(atoi(argv[1]) == 5)	
		{
			if (atoi(argv[3]) == 0)
				rtk_GenDtmfCid(atoi(argv[2]), "D1#");	//Denmark 
			else if (atoi(argv[3]) == 1)
				rtk_GenDtmfCid(atoi(argv[2]), "D2#"); //Denmark
			else if (atoi(argv[3]) == 2)
				rtk_GenDtmfCid(atoi(argv[2]), "D3#"); //Denmark
		}
		else if (atoi(argv[1]) == 6)
		{
			rtk_SetImpedance(atoi(argv[2]));
		}
		else if (atoi(argv[1]) == 7)
		{
			rtk_GenSlicCpc(atoi(argv[2]), atoi(argv[3]));
		}
		else if (atoi(argv[1]) == 8)
		{
			rtk_SetRingFxs(atoi(argv[2]), atoi(argv[3]));
		}

		else if (atoi(argv[1]) == 9)
		{
			rtk_SetDaaHybrid(atoi(argv[2]), atoi(argv[3]));
		}
		else if (atoi(argv[1]) == 10)
		{
			rtk_SetFxoTune(atoi(argv[2]), atoi(argv[3]));
		}
	}
#if 1	// Test plan need these cmd for testing, do NOT chagne or remove it.
	else if (argc == 5)
	{
		// dbg cmd ch cmd X
		switch (atoi(argv[1]))
		{
			case 0:
				rtk_SetSlicRingCadence(atoi(argv[2]), 2000, 4000);
				printf("Set SLIC Ring Cadence for ch%d, %dms on - %dms off\n", atoi(argv[2]), 2000, 4000);
				break;				
			case 1:
				pRingCad->cadon1 = 2000; pRingCad->cadoff1 = 4000;
				pRingCad->cadon2 = 0; pRingCad->cadoff2 = 0;
				pRingCad->cadon3 = 0; pRingCad->cadoff3 = 0;
				pRingCad->cadon4 = 0; pRingCad->cadoff4 = 0;
				break;				
			case 2:
				pRingCad->cadon1 = 200; pRingCad->cadoff1 = 400;
				pRingCad->cadon2 = 200; pRingCad->cadoff2 = 2000;
				pRingCad->cadon3 = 0; pRingCad->cadoff3 = 0;
				pRingCad->cadon4 = 0; pRingCad->cadoff4 = 0;
				break;
			case 3:
				pRingCad->cadon1 = 200; pRingCad->cadoff1 = 400;
				//pRingCad->cadon1 = 200; pRingCad->cadoff1 = 600;
				pRingCad->cadon2 = 400; pRingCad->cadoff2 = 600;
				pRingCad->cadon3 = 600; pRingCad->cadoff3 = 3000;
				pRingCad->cadon4 = 0; pRingCad->cadoff4 = 0;
				break;
			case 4:
				pRingCad->cadon1 = 200; pRingCad->cadoff1 = 400;
				pRingCad->cadon2 = 400; pRingCad->cadoff2 = 600;
				//pRingCad->cadon3 = 600; pRingCad->cadoff3 = 600;
				pRingCad->cadon3 = 600; pRingCad->cadoff3 = 800;
				pRingCad->cadon4 = 800; pRingCad->cadoff4 = 4000;
				break;
			case 5:
				pRingCad->cadon1 = 200; pRingCad->cadoff1 = 2000;
				pRingCad->cadon2 = 200; pRingCad->cadoff2 = 400;
				pRingCad->cadon3 = 0; pRingCad->cadoff3 = 0;
				pRingCad->cadon4 = 0; pRingCad->cadoff4 = 0;
				break;
			case 6:
				pRingCad->cadon1 = 200; pRingCad->cadoff1 = 400;
				pRingCad->cadon2 = 400; pRingCad->cadoff2 = 3000;
				pRingCad->cadon3 = 600; pRingCad->cadoff3 = 800;
				pRingCad->cadon4 = 0; pRingCad->cadoff4 = 0;
				break;
			case 7:
				pRingCad->cadon1 = 200; pRingCad->cadoff1 = 400;
				pRingCad->cadon2 = 400; pRingCad->cadoff2 = 600;
				pRingCad->cadon3 = 600; pRingCad->cadoff3 = 4000;
				pRingCad->cadon4 = 800; pRingCad->cadoff4 = 800;
				break;
			case 8:
				rtk_SetRingFxs(atoi(argv[2]), atoi(argv[3]));
				printf("Ringing %s for ch%d\n", atoi(argv[3]) ? "On" : "Off", atoi(argv[2]));
				break;
			case 9:
				rtk_SetSlicRingFreqAmp(atoi(argv[2])/*chid*/, atoi(argv[3]));
				break;
			
		}
		
		if (((atoi(argv[1])) != 0) && ((atoi(argv[1])) != 8) && ((atoi(argv[1])) != 9))
		{
			printf("Set Multi-Ring Cadence for ch%d, (%d-%d) (%d-%d) (%d-%d) (%d-%d)\n", atoi(argv[2]), 
					pRingCad->cadon1, pRingCad->cadoff1,
					pRingCad->cadon2, pRingCad->cadoff2,
					pRingCad->cadon3, pRingCad->cadoff3,
					pRingCad->cadon4, pRingCad->cadoff4);
			rtk_SetMultiRingCadence(atoi(argv[2]), pRingCad);
			rtk_SetRingFxs(atoi(argv[2]), 1);
		}
	}
#endif
	else if (argc == 6)
	{
		rtk_Set_RTP_PT_checker(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
	}
	else if(argc == 7){
		printf("Multi Ring long term testing begine\n");
		int test_cycle = 100;	
		int sleep_time = 0;
		int ch_num = 0;
		int cad_num = 0;
		int on_time = 0;
		int i = 0;
		while(test_cycle >= 0 ){
			sleep_time = (rand()%10) + 1;
			ch_num = (rand()%2);
			cad_num = (rand()%4) + 1;
			on_time = (rand()%800) + 100;
			switch(cad_num){
				case 1:
					pRingCad->cadon1 = on_time; pRingCad->cadoff1 = on_time;
					printf("cad 1 : %d\n",on_time);
					break;
				case 2:
					printf("cad 2 : %d",on_time);
					pRingCad->cadon1 = on_time; pRingCad->cadoff1 = on_time;
					on_time = (rand()%800) + 100;
					printf(" %d\n",on_time);
					pRingCad->cadon2 = on_time; pRingCad->cadoff2 = on_time;
					break;
				case 3:
					printf("cad 3 : %d",on_time);
					pRingCad->cadon1 = on_time; pRingCad->cadoff1 = on_time;
					on_time = (rand()%800) + 100;
					printf(" %d",on_time);
					pRingCad->cadon2 = on_time; pRingCad->cadoff2 = on_time;
					on_time = (rand()%800) + 100;
					printf(" %d\n",on_time);
					pRingCad->cadon3 = on_time; pRingCad->cadoff3 = on_time;
                                        break;
				case 4:
					printf("cad 4 : %d",on_time);
					pRingCad->cadon1 = on_time; pRingCad->cadoff1 = on_time;
					on_time = (rand()%800) + 100;
					printf(" %d",on_time);
					pRingCad->cadon2 = on_time; pRingCad->cadoff2 = on_time;
					on_time = (rand()%800) + 100;
					printf(" %d",on_time);
					pRingCad->cadon3 = on_time; pRingCad->cadoff3 = on_time;
					on_time = (rand()%800) + 100;
					printf(" %d\n",on_time);
					pRingCad->cadon4 = on_time; pRingCad->cadoff4 = on_time;
                                        break;
				default :
					printf("cad 4 : %d",on_time);
                                        pRingCad->cadon1 = on_time; pRingCad->cadoff1 = on_time;
                                        on_time = (rand()%800) + 100;
                                        printf(" %d",on_time);
                                        pRingCad->cadon2 = on_time; pRingCad->cadoff2 = on_time;
                                        on_time = (rand()%800) + 100;
                                        printf(" %d",on_time);
                                        pRingCad->cadon3 = on_time; pRingCad->cadoff3 = on_time;
                                        on_time = (rand()%800) + 100;
                                        printf(" %d\n",on_time);
                                        pRingCad->cadon4 = on_time; pRingCad->cadoff4 = on_time;
					break;
			}		
			rtk_SetMultiRingCadence(ch_num, pRingCad);
                        sleep(1);
                        rtk_SetRingFxs(ch_num, 1);
			sleep(sleep_time);
			rtk_SetRingFxs(ch_num, 0);
			test_cycle--;
		}
		while(1){
			pRingCad->cadon1 = 200; pRingCad->cadoff1 = 200;
                        pRingCad->cadon2 = 200; pRingCad->cadoff2 = 200;
                        pRingCad->cadon3 = 200; pRingCad->cadoff3 = 200;
                        pRingCad->cadon4 = 200; pRingCad->cadoff4 = 200;	
			rtk_SetMultiRingCadence(0, pRingCad);
                        sleep(1);
                        rtk_SetRingFxs(0, 1);
                        sleep(3);
                        rtk_SetRingFxs(0, 0);
			rtk_SetMultiRingCadence(1, pRingCad);
                        sleep(1);
                        rtk_SetRingFxs(1, 1);
                        sleep(3);
                        rtk_SetRingFxs(1, 0);
			printf("final\n");
		}
	}
	else
	{
		printf("use: %s dbg_flag [watchdog]\n", argv[0]);
	}

	return 0;
}

