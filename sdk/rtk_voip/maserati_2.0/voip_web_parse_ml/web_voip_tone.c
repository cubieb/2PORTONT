#include <stdio.h>
#include "web_voip.h"

char tone_country[TONE_MAX][20] = {"USA", "UK", "AUSTRALIA", "HONG KONG", "JAPAN",
				   "SWEDEN", "GERMANY", "FRANCE", "TAIWAN", "BELGIUM",
#ifdef COUNTRY_TONE_RESERVED
				   "FINLAND", "ITALY", "CHINA", "RUSSIAN", "SPAIN", "EXT3", "EXT4", "RESERVE", "CUSTOMER"};
#else
				   "FINLAND", "ITALY", "CHINA", "RUSSIAN", "SPAIN", "EXT3", "EXT4", "CUSTOMER"};
#endif

char cust_tone[TONE_CUSTOMER_MAX][20] = {"Custom1", "Custom2", "Custom3", "Custom4",
					 "Custom5", "Custom6", "Custom7", "Custom8"};

char tone_type[TONE_TYPE_MAX][20] = {"ADDITIVE", "MODULATED", "SUCC", "SUCC_ADD", "FOUR_FREQ",
                                     "STEP_INC", "TWO_STEP"};
#if 0 // jwsyu 20121005 disable
#if 1
char tone_cycle[TONE_CYCLE_MAX][20] = {"CONTINUOUS", "BURST", "CADENCE"};
#else
char tone_cycle[TONE_CYCLE_MAX][20] = {"CONTINUOUS", "BURST"};
#endif
#endif

char number_of_distone[DIS_CONNECT_TONE_MAX][5] = {"0", "1", "2"};

void asp_voip_ToneSet(request * wp, char *path, char *query)
{
	char *ptr;
	int i, cust_flag = 0, cust_idx, idx;
	voipCfgParam_t *pCfg;

	if (web_flash_get(&pCfg) != 0)
		return;


	ptr = boaGetVar(wp, "Country", "");
	if (strcmp(ptr, "Apply") == 0)
	{
		/* select country */
		idx = atoi(boaGetVar(wp, "tone_country", ""));
		if (idx < 0 || idx >= TONE_MAX)
			idx = 0;
			
		pCfg->tone_of_country = idx;
		if (idx == TONE_CUSTOMER)
			cust_flag = 1;

		
		if (cust_flag)
		{
			/* select dial */
			ptr = boaGetVar(wp, "dial", "");
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_1;

			pCfg->tone_of_custdial = i;
		
			/* select ring */
			ptr = boaGetVar(wp, "ring", "");
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_2;

			pCfg->tone_of_custring = i;
			
			/* select busy */
			ptr = boaGetVar(wp, "busy", "");
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_3;

			pCfg->tone_of_custbusy = i;
			
			/* select waiting */
			ptr = boaGetVar(wp, "waiting", "");
			
			for(i=0; i < TONE_CUSTOMER_MAX; i++)
			{
				if (!gstrcmp(ptr, cust_tone[i]))
					break;
			}
			if (i == TONE_CUSTOMER_MAX)
				i = TONE_CUSTOMER_4;
				
			pCfg->tone_of_custwaiting = i;
		}
	}
	
	/* Select Custom Tone */
	ptr = boaGetVar(wp, "selfItem", "");
	for(i=0; i < TONE_CUSTOMER_MAX; i++)
	{
		if (!gstrcmp(ptr, cust_tone[i]))
			break;
	}
	if (i == TONE_CUSTOMER_MAX)
		i = TONE_CUSTOMER_1;
	
	pCfg->tone_of_customize = i;	
	
	/* Tone Parameters */
	ptr = boaGetVar(wp, "Tone", "");
	if (strcmp(ptr, "Apply") == 0)
	{
		// Custom Tone Parameters Set
		cust_idx = pCfg->tone_of_customize;
		
		ptr = boaGetVar(wp, "type", "");
		
		for(i=0; i < TONE_TYPE_MAX; i++)
		{
			if (!gstrcmp(ptr, tone_type[i]))
				break;
		}
		if (i == TONE_TYPE_MAX)
			i = TONE_TYPE_ADDITIVE;
		
		pCfg->cust_tone_para[cust_idx].toneType = i;

		///////////////////////////////////////////
#if 0 // jwsyu 20121005 disable
		ptr = boaGetVar(wp, "cycle", "");
		
		for(i=0; i < TONE_CYCLE_MAX; i++)
		{
			if (!gstrcmp(ptr, tone_cycle[i]))
				break;
		}
		if (i == TONE_CYCLE_MAX)
			i = TONE_CYCLE_CONTINUOUS;
			
		pCfg->cust_tone_para[cust_idx].cycle = i;
#endif
		pCfg->cust_tone_para[cust_idx].cycle = atoi(boaGetVar(wp, "cycle", ""));
		pCfg->cust_tone_para[cust_idx].cadNUM = atoi(boaGetVar(wp, "cadNUM", ""));
		pCfg->cust_tone_para[cust_idx].CadOn0 = atoi(boaGetVar(wp, "CadOn0", ""));
		pCfg->cust_tone_para[cust_idx].CadOn1 = atoi(boaGetVar(wp, "CadOn1", ""));
		pCfg->cust_tone_para[cust_idx].CadOn2 = atoi(boaGetVar(wp, "CadOn2", ""));
		pCfg->cust_tone_para[cust_idx].CadOn3 = atoi(boaGetVar(wp, "CadOn3", ""));
		pCfg->cust_tone_para[cust_idx].CadOff0 = atoi(boaGetVar(wp, "CadOff0", ""));
		pCfg->cust_tone_para[cust_idx].CadOff1 = atoi(boaGetVar(wp, "CadOff1", ""));
		pCfg->cust_tone_para[cust_idx].CadOff2 = atoi(boaGetVar(wp, "CadOff2", ""));
		pCfg->cust_tone_para[cust_idx].CadOff3 = atoi(boaGetVar(wp, "CadOff3", ""));
		pCfg->cust_tone_para[cust_idx].PatternOff = atoi(boaGetVar(wp, "PatternOff", ""));
		pCfg->cust_tone_para[cust_idx].ToneNUM = atoi(boaGetVar(wp, "ToneNUM", ""));
		pCfg->cust_tone_para[cust_idx].Freq0 = atoi(boaGetVar(wp, "Freq0", ""));
		pCfg->cust_tone_para[cust_idx].Freq1 = atoi(boaGetVar(wp, "Freq1", ""));
		pCfg->cust_tone_para[cust_idx].Freq2 = atoi(boaGetVar(wp, "Freq2", ""));
		pCfg->cust_tone_para[cust_idx].Freq3 = atoi(boaGetVar(wp, "Freq3", ""));
		pCfg->cust_tone_para[cust_idx].Gain0 = atoi(boaGetVar(wp, "Gain0", ""));
		pCfg->cust_tone_para[cust_idx].Gain1 = atoi(boaGetVar(wp, "Gain1", ""));
		pCfg->cust_tone_para[cust_idx].Gain2 = atoi(boaGetVar(wp, "Gain2", ""));
		pCfg->cust_tone_para[cust_idx].Gain3 = atoi(boaGetVar(wp, "Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C1_Freq0 = atoi(boaGetVar(wp, "C1_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C1_Freq1 = atoi(boaGetVar(wp, "C1_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C1_Freq2 = atoi(boaGetVar(wp, "C1_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C1_Freq3 = atoi(boaGetVar(wp, "C1_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C1_Gain0 = atoi(boaGetVar(wp, "C1_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C1_Gain1 = atoi(boaGetVar(wp, "C1_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C1_Gain2 = atoi(boaGetVar(wp, "C1_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C1_Gain3 = atoi(boaGetVar(wp, "C1_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C2_Freq0 = atoi(boaGetVar(wp, "C2_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C2_Freq1 = atoi(boaGetVar(wp, "C2_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C2_Freq2 = atoi(boaGetVar(wp, "C2_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C2_Freq3 = atoi(boaGetVar(wp, "C2_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C2_Gain0 = atoi(boaGetVar(wp, "C2_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C2_Gain1 = atoi(boaGetVar(wp, "C2_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C2_Gain2 = atoi(boaGetVar(wp, "C2_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C2_Gain3 = atoi(boaGetVar(wp, "C2_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C3_Freq0 = atoi(boaGetVar(wp, "C3_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C3_Freq1 = atoi(boaGetVar(wp, "C3_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C3_Freq2 = atoi(boaGetVar(wp, "C3_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C3_Freq3 = atoi(boaGetVar(wp, "C3_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C3_Gain0 = atoi(boaGetVar(wp, "C3_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C3_Gain1 = atoi(boaGetVar(wp, "C3_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C3_Gain2 = atoi(boaGetVar(wp, "C3_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C3_Gain3 = atoi(boaGetVar(wp, "C3_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].CadOn4 = atoi(boaGetVar(wp, "CadOn4", ""));
		pCfg->cust_tone_para[cust_idx].CadOff4 = atoi(boaGetVar(wp, "CadOff4", ""));
		pCfg->cust_tone_para[cust_idx].CadOn5 = atoi(boaGetVar(wp, "CadOn5", ""));
		pCfg->cust_tone_para[cust_idx].CadOff5 = atoi(boaGetVar(wp, "CadOff5", ""));
		pCfg->cust_tone_para[cust_idx].CadOn6 = atoi(boaGetVar(wp, "CadOn6", ""));
		pCfg->cust_tone_para[cust_idx].CadOff6 = atoi(boaGetVar(wp, "CadOff6", ""));
		pCfg->cust_tone_para[cust_idx].CadOn7 = atoi(boaGetVar(wp, "CadOn7", ""));
		pCfg->cust_tone_para[cust_idx].CadOff7 = atoi(boaGetVar(wp, "CadOff7", ""));
		pCfg->cust_tone_para[cust_idx].C4_Freq0 = atoi(boaGetVar(wp, "C4_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C4_Freq1 = atoi(boaGetVar(wp, "C4_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C4_Freq2 = atoi(boaGetVar(wp, "C4_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C4_Freq3 = atoi(boaGetVar(wp, "C4_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C4_Gain0 = atoi(boaGetVar(wp, "C4_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C4_Gain1 = atoi(boaGetVar(wp, "C4_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C4_Gain2 = atoi(boaGetVar(wp, "C4_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C4_Gain3 = atoi(boaGetVar(wp, "C4_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C5_Freq0 = atoi(boaGetVar(wp, "C5_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C5_Freq1 = atoi(boaGetVar(wp, "C5_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C5_Freq2 = atoi(boaGetVar(wp, "C5_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C5_Freq3 = atoi(boaGetVar(wp, "C5_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C5_Gain0 = atoi(boaGetVar(wp, "C5_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C5_Gain1 = atoi(boaGetVar(wp, "C5_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C5_Gain2 = atoi(boaGetVar(wp, "C5_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C5_Gain3 = atoi(boaGetVar(wp, "C5_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C6_Freq0 = atoi(boaGetVar(wp, "C6_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C6_Freq1 = atoi(boaGetVar(wp, "C6_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C6_Freq2 = atoi(boaGetVar(wp, "C6_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C6_Freq3 = atoi(boaGetVar(wp, "C6_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C6_Gain0 = atoi(boaGetVar(wp, "C6_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C6_Gain1 = atoi(boaGetVar(wp, "C6_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C6_Gain2 = atoi(boaGetVar(wp, "C6_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C6_Gain3 = atoi(boaGetVar(wp, "C6_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C7_Freq0 = atoi(boaGetVar(wp, "C7_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C7_Freq1 = atoi(boaGetVar(wp, "C7_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C7_Freq2 = atoi(boaGetVar(wp, "C7_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C7_Freq3 = atoi(boaGetVar(wp, "C7_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C7_Gain0 = atoi(boaGetVar(wp, "C7_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C7_Gain1 = atoi(boaGetVar(wp, "C7_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C7_Gain2 = atoi(boaGetVar(wp, "C7_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C7_Gain3 = atoi(boaGetVar(wp, "C7_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].CadOn8 = atoi(boaGetVar(wp, "CadOn8", ""));
		pCfg->cust_tone_para[cust_idx].CadOff8 = atoi(boaGetVar(wp, "CadOff8", ""));
		pCfg->cust_tone_para[cust_idx].CadOn9 = atoi(boaGetVar(wp, "CadOn9", ""));
		pCfg->cust_tone_para[cust_idx].CadOff9 = atoi(boaGetVar(wp, "CadOff9", ""));
		pCfg->cust_tone_para[cust_idx].CadOn10 = atoi(boaGetVar(wp, "CadOn10", ""));
		pCfg->cust_tone_para[cust_idx].CadOff10 = atoi(boaGetVar(wp, "CadOff10", ""));
		pCfg->cust_tone_para[cust_idx].CadOn11 = atoi(boaGetVar(wp, "CadOn11", ""));
		pCfg->cust_tone_para[cust_idx].CadOff11 = atoi(boaGetVar(wp, "CadOff11", ""));
		pCfg->cust_tone_para[cust_idx].C8_Freq0 = atoi(boaGetVar(wp, "C8_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C8_Freq1 = atoi(boaGetVar(wp, "C8_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C8_Freq2 = atoi(boaGetVar(wp, "C8_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C8_Freq3 = atoi(boaGetVar(wp, "C8_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C8_Gain0 = atoi(boaGetVar(wp, "C8_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C8_Gain1 = atoi(boaGetVar(wp, "C8_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C8_Gain2 = atoi(boaGetVar(wp, "C8_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C8_Gain3 = atoi(boaGetVar(wp, "C8_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C9_Freq0 = atoi(boaGetVar(wp, "C9_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C9_Freq1 = atoi(boaGetVar(wp, "C9_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C9_Freq2 = atoi(boaGetVar(wp, "C9_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C9_Freq3 = atoi(boaGetVar(wp, "C9_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C9_Gain0 = atoi(boaGetVar(wp, "C9_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C9_Gain1 = atoi(boaGetVar(wp, "C9_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C9_Gain2 = atoi(boaGetVar(wp, "C9_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C9_Gain3 = atoi(boaGetVar(wp, "C9_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C10_Freq0 = atoi(boaGetVar(wp, "C10_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C10_Freq1 = atoi(boaGetVar(wp, "C10_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C10_Freq2 = atoi(boaGetVar(wp, "C10_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C10_Freq3 = atoi(boaGetVar(wp, "C10_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C10_Gain0 = atoi(boaGetVar(wp, "C10_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C10_Gain1 = atoi(boaGetVar(wp, "C10_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C10_Gain2 = atoi(boaGetVar(wp, "C10_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C10_Gain3 = atoi(boaGetVar(wp, "C10_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C11_Freq0 = atoi(boaGetVar(wp, "C11_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C11_Freq1 = atoi(boaGetVar(wp, "C11_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C11_Freq2 = atoi(boaGetVar(wp, "C11_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C11_Freq3 = atoi(boaGetVar(wp, "C11_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C11_Gain0 = atoi(boaGetVar(wp, "C11_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C11_Gain1 = atoi(boaGetVar(wp, "C11_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C11_Gain2 = atoi(boaGetVar(wp, "C11_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C11_Gain3 = atoi(boaGetVar(wp, "C11_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].CadOn12 = atoi(boaGetVar(wp, "CadOn12", ""));
		pCfg->cust_tone_para[cust_idx].CadOff12 = atoi(boaGetVar(wp, "CadOff12", ""));
		pCfg->cust_tone_para[cust_idx].CadOn13 = atoi(boaGetVar(wp, "CadOn13", ""));
		pCfg->cust_tone_para[cust_idx].CadOff13 = atoi(boaGetVar(wp, "CadOff13", ""));
		pCfg->cust_tone_para[cust_idx].CadOn14 = atoi(boaGetVar(wp, "CadOn14", ""));
		pCfg->cust_tone_para[cust_idx].CadOff14 = atoi(boaGetVar(wp, "CadOff14", ""));
		pCfg->cust_tone_para[cust_idx].CadOn15 = atoi(boaGetVar(wp, "CadOn15", ""));
		pCfg->cust_tone_para[cust_idx].CadOff15 = atoi(boaGetVar(wp, "CadOff15", ""));
		pCfg->cust_tone_para[cust_idx].C12_Freq0 = atoi(boaGetVar(wp, "C12_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C12_Freq1 = atoi(boaGetVar(wp, "C12_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C12_Freq2 = atoi(boaGetVar(wp, "C12_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C12_Freq3 = atoi(boaGetVar(wp, "C12_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C12_Gain0 = atoi(boaGetVar(wp, "C12_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C12_Gain1 = atoi(boaGetVar(wp, "C12_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C12_Gain2 = atoi(boaGetVar(wp, "C12_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C12_Gain3 = atoi(boaGetVar(wp, "C12_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C13_Freq0 = atoi(boaGetVar(wp, "C13_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C13_Freq1 = atoi(boaGetVar(wp, "C13_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C13_Freq2 = atoi(boaGetVar(wp, "C13_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C13_Freq3 = atoi(boaGetVar(wp, "C13_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C13_Gain0 = atoi(boaGetVar(wp, "C13_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C13_Gain1 = atoi(boaGetVar(wp, "C13_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C13_Gain2 = atoi(boaGetVar(wp, "C13_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C13_Gain3 = atoi(boaGetVar(wp, "C13_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C14_Freq0 = atoi(boaGetVar(wp, "C14_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C14_Freq1 = atoi(boaGetVar(wp, "C14_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C14_Freq2 = atoi(boaGetVar(wp, "C14_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C14_Freq3 = atoi(boaGetVar(wp, "C14_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C14_Gain0 = atoi(boaGetVar(wp, "C14_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C14_Gain1 = atoi(boaGetVar(wp, "C14_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C14_Gain2 = atoi(boaGetVar(wp, "C14_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C14_Gain3 = atoi(boaGetVar(wp, "C14_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C15_Freq0 = atoi(boaGetVar(wp, "C15_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C15_Freq1 = atoi(boaGetVar(wp, "C15_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C15_Freq2 = atoi(boaGetVar(wp, "C15_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C15_Freq3 = atoi(boaGetVar(wp, "C15_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C15_Gain0 = atoi(boaGetVar(wp, "C15_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C15_Gain1 = atoi(boaGetVar(wp, "C15_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C15_Gain2 = atoi(boaGetVar(wp, "C15_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C15_Gain3 = atoi(boaGetVar(wp, "C15_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].CadOn16 = atoi(boaGetVar(wp, "CadOn16", ""));
		pCfg->cust_tone_para[cust_idx].CadOff16 = atoi(boaGetVar(wp, "CadOff16", ""));
		pCfg->cust_tone_para[cust_idx].CadOn17 = atoi(boaGetVar(wp, "CadOn17", ""));
		pCfg->cust_tone_para[cust_idx].CadOff17 = atoi(boaGetVar(wp, "CadOff17", ""));
		pCfg->cust_tone_para[cust_idx].CadOn18 = atoi(boaGetVar(wp, "CadOn18", ""));
		pCfg->cust_tone_para[cust_idx].CadOff18 = atoi(boaGetVar(wp, "CadOff18", ""));
		pCfg->cust_tone_para[cust_idx].CadOn19 = atoi(boaGetVar(wp, "CadOn19", ""));
		pCfg->cust_tone_para[cust_idx].CadOff19 = atoi(boaGetVar(wp, "CadOff19", ""));
		pCfg->cust_tone_para[cust_idx].C16_Freq0 = atoi(boaGetVar(wp, "C16_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C16_Freq1 = atoi(boaGetVar(wp, "C16_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C16_Freq2 = atoi(boaGetVar(wp, "C16_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C16_Freq3 = atoi(boaGetVar(wp, "C16_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C16_Gain0 = atoi(boaGetVar(wp, "C16_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C16_Gain1 = atoi(boaGetVar(wp, "C16_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C16_Gain2 = atoi(boaGetVar(wp, "C16_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C16_Gain3 = atoi(boaGetVar(wp, "C16_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C17_Freq0 = atoi(boaGetVar(wp, "C17_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C17_Freq1 = atoi(boaGetVar(wp, "C17_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C17_Freq2 = atoi(boaGetVar(wp, "C17_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C17_Freq3 = atoi(boaGetVar(wp, "C17_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C17_Gain0 = atoi(boaGetVar(wp, "C17_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C17_Gain1 = atoi(boaGetVar(wp, "C17_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C17_Gain2 = atoi(boaGetVar(wp, "C17_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C17_Gain3 = atoi(boaGetVar(wp, "C17_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C18_Freq0 = atoi(boaGetVar(wp, "C18_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C18_Freq1 = atoi(boaGetVar(wp, "C18_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C18_Freq2 = atoi(boaGetVar(wp, "C18_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C18_Freq3 = atoi(boaGetVar(wp, "C18_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C18_Gain0 = atoi(boaGetVar(wp, "C18_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C18_Gain1 = atoi(boaGetVar(wp, "C18_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C18_Gain2 = atoi(boaGetVar(wp, "C18_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C18_Gain3 = atoi(boaGetVar(wp, "C18_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C19_Freq0 = atoi(boaGetVar(wp, "C19_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C19_Freq1 = atoi(boaGetVar(wp, "C19_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C19_Freq2 = atoi(boaGetVar(wp, "C19_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C19_Freq3 = atoi(boaGetVar(wp, "C19_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C19_Gain0 = atoi(boaGetVar(wp, "C19_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C19_Gain1 = atoi(boaGetVar(wp, "C19_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C19_Gain2 = atoi(boaGetVar(wp, "C19_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C19_Gain3 = atoi(boaGetVar(wp, "C19_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].CadOn20 = atoi(boaGetVar(wp, "CadOn20", ""));
		pCfg->cust_tone_para[cust_idx].CadOff20 = atoi(boaGetVar(wp, "CadOff20", ""));
		pCfg->cust_tone_para[cust_idx].CadOn21 = atoi(boaGetVar(wp, "CadOn21", ""));
		pCfg->cust_tone_para[cust_idx].CadOff21 = atoi(boaGetVar(wp, "CadOff21", ""));
		pCfg->cust_tone_para[cust_idx].CadOn22 = atoi(boaGetVar(wp, "CadOn22", ""));
		pCfg->cust_tone_para[cust_idx].CadOff22 = atoi(boaGetVar(wp, "CadOff22", ""));
		pCfg->cust_tone_para[cust_idx].CadOn23 = atoi(boaGetVar(wp, "CadOn23", ""));
		pCfg->cust_tone_para[cust_idx].CadOff23 = atoi(boaGetVar(wp, "CadOff23", ""));
		pCfg->cust_tone_para[cust_idx].C20_Freq0 = atoi(boaGetVar(wp, "C20_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C20_Freq1 = atoi(boaGetVar(wp, "C20_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C20_Freq2 = atoi(boaGetVar(wp, "C20_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C20_Freq3 = atoi(boaGetVar(wp, "C20_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C20_Gain0 = atoi(boaGetVar(wp, "C20_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C20_Gain1 = atoi(boaGetVar(wp, "C20_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C20_Gain2 = atoi(boaGetVar(wp, "C20_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C20_Gain3 = atoi(boaGetVar(wp, "C20_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C21_Freq0 = atoi(boaGetVar(wp, "C21_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C21_Freq1 = atoi(boaGetVar(wp, "C21_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C21_Freq2 = atoi(boaGetVar(wp, "C21_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C21_Freq3 = atoi(boaGetVar(wp, "C21_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C21_Gain0 = atoi(boaGetVar(wp, "C21_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C21_Gain1 = atoi(boaGetVar(wp, "C21_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C21_Gain2 = atoi(boaGetVar(wp, "C21_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C21_Gain3 = atoi(boaGetVar(wp, "C21_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C22_Freq0 = atoi(boaGetVar(wp, "C22_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C22_Freq1 = atoi(boaGetVar(wp, "C22_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C22_Freq2 = atoi(boaGetVar(wp, "C22_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C22_Freq3 = atoi(boaGetVar(wp, "C22_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C22_Gain0 = atoi(boaGetVar(wp, "C22_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C22_Gain1 = atoi(boaGetVar(wp, "C22_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C22_Gain2 = atoi(boaGetVar(wp, "C22_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C22_Gain3 = atoi(boaGetVar(wp, "C22_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C23_Freq0 = atoi(boaGetVar(wp, "C23_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C23_Freq1 = atoi(boaGetVar(wp, "C23_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C23_Freq2 = atoi(boaGetVar(wp, "C23_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C23_Freq3 = atoi(boaGetVar(wp, "C23_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C23_Gain0 = atoi(boaGetVar(wp, "C23_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C23_Gain1 = atoi(boaGetVar(wp, "C23_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C23_Gain2 = atoi(boaGetVar(wp, "C23_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C23_Gain3 = atoi(boaGetVar(wp, "C23_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].CadOn24 = atoi(boaGetVar(wp, "CadOn24", ""));
		pCfg->cust_tone_para[cust_idx].CadOff24 = atoi(boaGetVar(wp, "CadOff24", ""));
		pCfg->cust_tone_para[cust_idx].CadOn25 = atoi(boaGetVar(wp, "CadOn25", ""));
		pCfg->cust_tone_para[cust_idx].CadOff25 = atoi(boaGetVar(wp, "CadOff25", ""));
		pCfg->cust_tone_para[cust_idx].CadOn26 = atoi(boaGetVar(wp, "CadOn26", ""));
		pCfg->cust_tone_para[cust_idx].CadOff26 = atoi(boaGetVar(wp, "CadOff26", ""));
		pCfg->cust_tone_para[cust_idx].CadOn27 = atoi(boaGetVar(wp, "CadOn27", ""));
		pCfg->cust_tone_para[cust_idx].CadOff27 = atoi(boaGetVar(wp, "CadOff27", ""));
		pCfg->cust_tone_para[cust_idx].C24_Freq0 = atoi(boaGetVar(wp, "C24_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C24_Freq1 = atoi(boaGetVar(wp, "C24_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C24_Freq2 = atoi(boaGetVar(wp, "C24_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C24_Freq3 = atoi(boaGetVar(wp, "C24_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C24_Gain0 = atoi(boaGetVar(wp, "C24_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C24_Gain1 = atoi(boaGetVar(wp, "C24_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C24_Gain2 = atoi(boaGetVar(wp, "C24_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C24_Gain3 = atoi(boaGetVar(wp, "C24_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C25_Freq0 = atoi(boaGetVar(wp, "C25_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C25_Freq1 = atoi(boaGetVar(wp, "C25_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C25_Freq2 = atoi(boaGetVar(wp, "C25_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C25_Freq3 = atoi(boaGetVar(wp, "C25_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C25_Gain0 = atoi(boaGetVar(wp, "C25_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C25_Gain1 = atoi(boaGetVar(wp, "C25_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C25_Gain2 = atoi(boaGetVar(wp, "C25_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C25_Gain3 = atoi(boaGetVar(wp, "C25_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C26_Freq0 = atoi(boaGetVar(wp, "C26_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C26_Freq1 = atoi(boaGetVar(wp, "C26_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C26_Freq2 = atoi(boaGetVar(wp, "C26_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C26_Freq3 = atoi(boaGetVar(wp, "C26_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C26_Gain0 = atoi(boaGetVar(wp, "C26_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C26_Gain1 = atoi(boaGetVar(wp, "C26_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C26_Gain2 = atoi(boaGetVar(wp, "C26_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C26_Gain3 = atoi(boaGetVar(wp, "C26_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C27_Freq0 = atoi(boaGetVar(wp, "C27_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C27_Freq1 = atoi(boaGetVar(wp, "C27_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C27_Freq2 = atoi(boaGetVar(wp, "C27_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C27_Freq3 = atoi(boaGetVar(wp, "C27_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C27_Gain0 = atoi(boaGetVar(wp, "C27_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C27_Gain1 = atoi(boaGetVar(wp, "C27_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C27_Gain2 = atoi(boaGetVar(wp, "C27_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C27_Gain3 = atoi(boaGetVar(wp, "C27_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].CadOn28 = atoi(boaGetVar(wp, "CadOn28", ""));
		pCfg->cust_tone_para[cust_idx].CadOff28 = atoi(boaGetVar(wp, "CadOff28", ""));
		pCfg->cust_tone_para[cust_idx].CadOn29 = atoi(boaGetVar(wp, "CadOn29", ""));
		pCfg->cust_tone_para[cust_idx].CadOff29 = atoi(boaGetVar(wp, "CadOff29", ""));
		pCfg->cust_tone_para[cust_idx].CadOn30 = atoi(boaGetVar(wp, "CadOn30", ""));
		pCfg->cust_tone_para[cust_idx].CadOff30 = atoi(boaGetVar(wp, "CadOff30", ""));
		pCfg->cust_tone_para[cust_idx].CadOn31 = atoi(boaGetVar(wp, "CadOn31", ""));
		pCfg->cust_tone_para[cust_idx].CadOff31 = atoi(boaGetVar(wp, "CadOff31", ""));
		pCfg->cust_tone_para[cust_idx].C28_Freq0 = atoi(boaGetVar(wp, "C28_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C28_Freq1 = atoi(boaGetVar(wp, "C28_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C28_Freq2 = atoi(boaGetVar(wp, "C28_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C28_Freq3 = atoi(boaGetVar(wp, "C28_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C28_Gain0 = atoi(boaGetVar(wp, "C28_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C28_Gain1 = atoi(boaGetVar(wp, "C28_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C28_Gain2 = atoi(boaGetVar(wp, "C28_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C28_Gain3 = atoi(boaGetVar(wp, "C28_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C29_Freq0 = atoi(boaGetVar(wp, "C29_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C29_Freq1 = atoi(boaGetVar(wp, "C29_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C29_Freq2 = atoi(boaGetVar(wp, "C29_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C29_Freq3 = atoi(boaGetVar(wp, "C29_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C29_Gain0 = atoi(boaGetVar(wp, "C29_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C29_Gain1 = atoi(boaGetVar(wp, "C29_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C29_Gain2 = atoi(boaGetVar(wp, "C29_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C29_Gain3 = atoi(boaGetVar(wp, "C29_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C30_Freq0 = atoi(boaGetVar(wp, "C30_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C30_Freq1 = atoi(boaGetVar(wp, "C30_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C30_Freq2 = atoi(boaGetVar(wp, "C30_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C30_Freq3 = atoi(boaGetVar(wp, "C30_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C30_Gain0 = atoi(boaGetVar(wp, "C30_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C30_Gain1 = atoi(boaGetVar(wp, "C30_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C30_Gain2 = atoi(boaGetVar(wp, "C30_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C30_Gain3 = atoi(boaGetVar(wp, "C30_Gain3", ""));	
		pCfg->cust_tone_para[cust_idx].C31_Freq0 = atoi(boaGetVar(wp, "C31_Freq0", ""));
		pCfg->cust_tone_para[cust_idx].C31_Freq1 = atoi(boaGetVar(wp, "C31_Freq1", ""));
		pCfg->cust_tone_para[cust_idx].C31_Freq2 = atoi(boaGetVar(wp, "C31_Freq2", ""));
		pCfg->cust_tone_para[cust_idx].C31_Freq3 = atoi(boaGetVar(wp, "C31_Freq3", ""));
		pCfg->cust_tone_para[cust_idx].C31_Gain0 = atoi(boaGetVar(wp, "C31_Gain0", ""));
		pCfg->cust_tone_para[cust_idx].C31_Gain1 = atoi(boaGetVar(wp, "C31_Gain1", ""));
		pCfg->cust_tone_para[cust_idx].C31_Gain2 = atoi(boaGetVar(wp, "C31_Gain2", ""));
		pCfg->cust_tone_para[cust_idx].C31_Gain3 = atoi(boaGetVar(wp, "C31_Gain3", ""));	
	}
	
	/* Disconnect tone det config */
	ptr = boaGetVar(wp, "Distone", "");
	if (strcmp(ptr, "Apply") == 0)
	{
		/* select disconnect tone num */
		ptr = boaGetVar(wp, "distone_num", "");
		for(i=0; i < DIS_CONNECT_TONE_MAX; i++)
		{
			if (!gstrcmp(ptr, number_of_distone[i]))
				break;
		}
		if (i == DIS_CONNECT_TONE_MAX)
			i = 2;
		pCfg->distone_num = i;

		ptr = boaGetVar(wp, "d1freqnum", "");
		for(i=1; i < DIS_CONNECT_TONE_FREQ_MAX; i++)
		{
			if (!gstrcmp(ptr, number_of_distone[i]))
				break;
		}
		if (i == DIS_CONNECT_TONE_FREQ_MAX)
			i = 2;
		pCfg->d1freqnum = i;


		pCfg->d1Freq1 = atoi(boaGetVar(wp, "d1Freq1", ""));
		pCfg->d1Freq2 = atoi(boaGetVar(wp, "d1Freq2", ""));
		pCfg->d1Accur = atoi(boaGetVar(wp, "d1Accur", ""));
		pCfg->d1Level = atoi(boaGetVar(wp, "d1Level", ""));
		pCfg->d1ONup = atoi(boaGetVar(wp, "d1ONup", ""));
		pCfg->d1ONlow = atoi(boaGetVar(wp, "d1ONlow", ""));
		pCfg->d1OFFup = atoi(boaGetVar(wp, "d1OFFup", ""));
		pCfg->d1OFFlow = atoi(boaGetVar(wp, "d1OFFlow", ""));

		ptr = boaGetVar(wp, "d2freqnum", "");
		for(i=1; i < DIS_CONNECT_TONE_FREQ_MAX; i++)
		{
			if (!gstrcmp(ptr, number_of_distone[i]))
				break;
		}
		if (i == DIS_CONNECT_TONE_FREQ_MAX)
			i = 2;
		pCfg->d2freqnum = i;

		pCfg->d2Freq1 = atoi(boaGetVar(wp, "d2Freq1", ""));
		pCfg->d2Freq2 = atoi(boaGetVar(wp, "d2Freq2", ""));
		pCfg->d2Accur = atoi(boaGetVar(wp, "d2Accur", ""));
		pCfg->d2Level = atoi(boaGetVar(wp, "d2Level", ""));
		pCfg->d2ONup = atoi(boaGetVar(wp, "d2ONup", ""));
		pCfg->d2ONlow = atoi(boaGetVar(wp, "d2ONlow", ""));
		pCfg->d2OFFup = atoi(boaGetVar(wp, "d2OFFup", ""));
		pCfg->d2OFFlow = atoi(boaGetVar(wp, "d2OFFlow", ""));

	}



	web_flash_set(pCfg);
	
#ifdef REBOOT_CHECK
	OK_MSG("/voip_tone.asp");
#else
	web_restart_solar();

	boaRedirect(wp, "/voip_tone.asp");
#endif

}


#ifdef CONFIG_APP_BOA
int asp_voip_ToneGet(request * wp, int argc, char **argv)
#else
int asp_voip_ToneGet(int ejid, request * wp, int argc, char **argv)
#endif
{
	int i, cust_idx;
	voipCfgParam_t *pCfg;

	if (web_flash_get(&pCfg) != 0)
		return -1;

	cust_idx = pCfg->tone_of_customize;
	
	if (strcmp(argv[0], "tone_country")==0)
	{
		for (i=0; i < TONE_MAX ;i++)
		{
			if (i == pCfg->tone_of_country)
				boaWrite(wp, "<option value=%d selected>%s</option>", i, tone_country[i]);
			else
				boaWrite(wp, "<option value=%d>%s</option>", i, tone_country[i]);
		}
	}
	else if (strcmp(argv[0], "dial")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custdial) // dial
				boaWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				boaWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	else if (strcmp(argv[0], "ring")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custring) // ring
				boaWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				boaWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	else if (strcmp(argv[0], "busy")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custbusy) // busy
				boaWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				boaWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	else if (strcmp(argv[0], "waiting")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_custwaiting) // waiting
				boaWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				boaWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	// Get Custome Tone 
	else if (strcmp(argv[0], "selfItem")==0)
	{
		for (i=0; i < TONE_CUSTOMER_MAX ;i++)
		{
			if (i == pCfg->tone_of_customize)
				boaWrite(wp, "<option selected>%s</option>", cust_tone[i]);
			else
				boaWrite(wp, "<option>%s</option>", cust_tone[i]);
		}
	}
	// Get Custom Tone Parameters	
	else if (strcmp(argv[0], "type")==0)
	{
		for (i=0; i < TONE_TYPE_MAX ;i++)
		{
			if (i == pCfg->cust_tone_para[cust_idx].toneType)
				boaWrite(wp, "<option selected>%s</option>", tone_type[i]);
			else
				boaWrite(wp, "<option>%s</option>", tone_type[i]);
		}
	}
	else if (strcmp(argv[0], "cycle")==0)
	{
#if 0 // jwsyu 20121005 disable
		for (i=0; i < TONE_CYCLE_MAX ;i++)
		{
			if (i == pCfg->cust_tone_para[cust_idx].cycle)
				boaWrite(wp, "<option selected>%s</option>", tone_cycle[i]);
			else
				boaWrite(wp, "<option>%s</option>", tone_cycle[i]);
		}
#endif
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].cycle);
	}	
	else if (strcmp(argv[0], "cadNUM")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].cadNUM);
	else if (strcmp(argv[0], "CadOn0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn0);
	else if (strcmp(argv[0], "CadOn1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn1);
	else if (strcmp(argv[0], "CadOn2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn2);
	else if (strcmp(argv[0], "CadOn3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn3);
	else if (strcmp(argv[0], "CadOff0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff0);
	else if (strcmp(argv[0], "CadOff1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff1);
	else if (strcmp(argv[0], "CadOff2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff2);
	else if (strcmp(argv[0], "CadOff3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff3);
	else if (strcmp(argv[0], "PatternOff")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].PatternOff);
	else if (strcmp(argv[0], "ToneNUM")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].ToneNUM);
	else if (strcmp(argv[0], "Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq0);
	else if (strcmp(argv[0], "Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq1);
	else if (strcmp(argv[0], "Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq2);
	else if (strcmp(argv[0], "Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq3);
	else if (strcmp(argv[0], "Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain0);
	else if (strcmp(argv[0], "Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain1);
	else if (strcmp(argv[0], "Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain2);
	else if (strcmp(argv[0], "Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain3);
	else if (strcmp(argv[0], "C1_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Freq0);
	else if (strcmp(argv[0], "C1_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Freq1);
	else if (strcmp(argv[0], "C1_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Freq2);
	else if (strcmp(argv[0], "C1_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Freq3);
	else if (strcmp(argv[0], "C1_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Gain0);
	else if (strcmp(argv[0], "C1_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Gain1);
	else if (strcmp(argv[0], "C1_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Gain2);
	else if (strcmp(argv[0], "C1_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C1_Gain3);
	else if (strcmp(argv[0], "C2_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Freq0);
	else if (strcmp(argv[0], "C2_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Freq1);
	else if (strcmp(argv[0], "C2_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Freq2);
	else if (strcmp(argv[0], "C2_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Freq3);
	else if (strcmp(argv[0], "C2_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Gain0);
	else if (strcmp(argv[0], "C2_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Gain1);
	else if (strcmp(argv[0], "C2_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Gain2);
	else if (strcmp(argv[0], "C2_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C2_Gain3);
	else if (strcmp(argv[0], "C3_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Freq0);
	else if (strcmp(argv[0], "C3_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Freq1);
	else if (strcmp(argv[0], "C3_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Freq2);
	else if (strcmp(argv[0], "C3_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Freq3);
	else if (strcmp(argv[0], "C3_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Gain0);
	else if (strcmp(argv[0], "C3_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Gain1);
	else if (strcmp(argv[0], "C3_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Gain2);
	else if (strcmp(argv[0], "C3_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C3_Gain3);
	else if (strcmp(argv[0], "CadOn4")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn4);
	else if (strcmp(argv[0], "CadOn5")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn5);
	else if (strcmp(argv[0], "CadOn6")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn6);
	else if (strcmp(argv[0], "CadOn7")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn7);
	else if (strcmp(argv[0], "CadOff4")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff4);
	else if (strcmp(argv[0], "CadOff5")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff5);
	else if (strcmp(argv[0], "CadOff6")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff6);
	else if (strcmp(argv[0], "CadOff7")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff7);
	else if (strcmp(argv[0], "C4_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Freq0);
	else if (strcmp(argv[0], "C4_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Freq1);
	else if (strcmp(argv[0], "C4_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Freq2);
	else if (strcmp(argv[0], "C4_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Freq3);
	else if (strcmp(argv[0], "C4_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Gain0);
	else if (strcmp(argv[0], "C4_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Gain1);
	else if (strcmp(argv[0], "C4_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Gain2);
	else if (strcmp(argv[0], "C4_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C4_Gain3);
	else if (strcmp(argv[0], "C5_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Freq0);
	else if (strcmp(argv[0], "C5_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Freq1);
	else if (strcmp(argv[0], "C5_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Freq2);
	else if (strcmp(argv[0], "C5_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Freq3);
	else if (strcmp(argv[0], "C5_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Gain0);
	else if (strcmp(argv[0], "C5_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Gain1);
	else if (strcmp(argv[0], "C5_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Gain2);
	else if (strcmp(argv[0], "C5_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C5_Gain3);
	else if (strcmp(argv[0], "C6_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Freq0);
	else if (strcmp(argv[0], "C6_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Freq1);
	else if (strcmp(argv[0], "C6_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Freq2);
	else if (strcmp(argv[0], "C6_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Freq3);
	else if (strcmp(argv[0], "C6_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Gain0);
	else if (strcmp(argv[0], "C6_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Gain1);
	else if (strcmp(argv[0], "C6_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Gain2);
	else if (strcmp(argv[0], "C6_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C6_Gain3);
	else if (strcmp(argv[0], "C7_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Freq0);
	else if (strcmp(argv[0], "C7_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Freq1);
	else if (strcmp(argv[0], "C7_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Freq2);
	else if (strcmp(argv[0], "C7_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Freq3);
	else if (strcmp(argv[0], "C7_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Gain0);
	else if (strcmp(argv[0], "C7_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Gain1);
	else if (strcmp(argv[0], "C7_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Gain2);
	else if (strcmp(argv[0], "C7_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C7_Gain3);
	else if (strcmp(argv[0], "CadOn8")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn8);
	else if (strcmp(argv[0], "CadOn9")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn9);
	else if (strcmp(argv[0], "CadOn10")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn10);
	else if (strcmp(argv[0], "CadOn11")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn11);
	else if (strcmp(argv[0], "CadOff8")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff8);
	else if (strcmp(argv[0], "CadOff9")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff9);
	else if (strcmp(argv[0], "CadOff10")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff10);
	else if (strcmp(argv[0], "CadOff11")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff11);
	else if (strcmp(argv[0], "C8_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Freq0);
	else if (strcmp(argv[0], "C8_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Freq1);
	else if (strcmp(argv[0], "C8_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Freq2);
	else if (strcmp(argv[0], "C8_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Freq3);
	else if (strcmp(argv[0], "C8_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Gain0);
	else if (strcmp(argv[0], "C8_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Gain1);
	else if (strcmp(argv[0], "C8_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Gain2);
	else if (strcmp(argv[0], "C8_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C8_Gain3);
	else if (strcmp(argv[0], "C9_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Freq0);
	else if (strcmp(argv[0], "C9_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Freq1);
	else if (strcmp(argv[0], "C9_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Freq2);
	else if (strcmp(argv[0], "C9_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Freq3);
	else if (strcmp(argv[0], "C9_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Gain0);
	else if (strcmp(argv[0], "C9_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Gain1);
	else if (strcmp(argv[0], "C9_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Gain2);
	else if (strcmp(argv[0], "C9_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C9_Gain3);
	else if (strcmp(argv[0], "C10_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Freq0);
	else if (strcmp(argv[0], "C10_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Freq1);
	else if (strcmp(argv[0], "C10_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Freq2);
	else if (strcmp(argv[0], "C10_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Freq3);
	else if (strcmp(argv[0], "C10_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Gain0);
	else if (strcmp(argv[0], "C10_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Gain1);
	else if (strcmp(argv[0], "C10_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Gain2);
	else if (strcmp(argv[0], "C10_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C10_Gain3);
	else if (strcmp(argv[0], "C11_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Freq0);
	else if (strcmp(argv[0], "C11_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Freq1);
	else if (strcmp(argv[0], "C11_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Freq2);
	else if (strcmp(argv[0], "C11_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Freq3);
	else if (strcmp(argv[0], "C11_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Gain0);
	else if (strcmp(argv[0], "C11_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Gain1);
	else if (strcmp(argv[0], "C11_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Gain2);
	else if (strcmp(argv[0], "C11_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C11_Gain3);
	else if (strcmp(argv[0], "CadOn12")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn12);
	else if (strcmp(argv[0], "CadOn13")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn13);
	else if (strcmp(argv[0], "CadOn14")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn14);
	else if (strcmp(argv[0], "CadOn15")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn15);
	else if (strcmp(argv[0], "CadOff12")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff12);
	else if (strcmp(argv[0], "CadOff13")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff13);
	else if (strcmp(argv[0], "CadOff14")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff14);
	else if (strcmp(argv[0], "CadOff15")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff15);
	else if (strcmp(argv[0], "C12_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Freq0);
	else if (strcmp(argv[0], "C12_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Freq1);
	else if (strcmp(argv[0], "C12_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Freq2);
	else if (strcmp(argv[0], "C12_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Freq3);
	else if (strcmp(argv[0], "C12_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Gain0);
	else if (strcmp(argv[0], "C12_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Gain1);
	else if (strcmp(argv[0], "C12_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Gain2);
	else if (strcmp(argv[0], "C12_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C12_Gain3);
	else if (strcmp(argv[0], "C13_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Freq0);
	else if (strcmp(argv[0], "C13_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Freq1);
	else if (strcmp(argv[0], "C13_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Freq2);
	else if (strcmp(argv[0], "C13_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Freq3);
	else if (strcmp(argv[0], "C13_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Gain0);
	else if (strcmp(argv[0], "C13_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Gain1);
	else if (strcmp(argv[0], "C13_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Gain2);
	else if (strcmp(argv[0], "C13_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C13_Gain3);
	else if (strcmp(argv[0], "C14_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Freq0);
	else if (strcmp(argv[0], "C14_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Freq1);
	else if (strcmp(argv[0], "C14_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Freq2);
	else if (strcmp(argv[0], "C14_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Freq3);
	else if (strcmp(argv[0], "C14_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Gain0);
	else if (strcmp(argv[0], "C14_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Gain1);
	else if (strcmp(argv[0], "C14_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Gain2);
	else if (strcmp(argv[0], "C14_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C14_Gain3);
	else if (strcmp(argv[0], "C15_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Freq0);
	else if (strcmp(argv[0], "C15_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Freq1);
	else if (strcmp(argv[0], "C15_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Freq2);
	else if (strcmp(argv[0], "C15_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Freq3);
	else if (strcmp(argv[0], "C15_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Gain0);
	else if (strcmp(argv[0], "C15_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Gain1);
	else if (strcmp(argv[0], "C15_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Gain2);
	else if (strcmp(argv[0], "C15_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C15_Gain3);
	else if (strcmp(argv[0], "CadOn16")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn16);
	else if (strcmp(argv[0], "CadOn17")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn17);
	else if (strcmp(argv[0], "CadOn18")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn18);
	else if (strcmp(argv[0], "CadOn19")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn19);
	else if (strcmp(argv[0], "CadOff16")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff16);
	else if (strcmp(argv[0], "CadOff17")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff17);
	else if (strcmp(argv[0], "CadOff18")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff18);
	else if (strcmp(argv[0], "CadOff19")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff19);
	else if (strcmp(argv[0], "C16_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Freq0);
	else if (strcmp(argv[0], "C16_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Freq1);
	else if (strcmp(argv[0], "C16_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Freq2);
	else if (strcmp(argv[0], "C16_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Freq3);
	else if (strcmp(argv[0], "C16_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Gain0);
	else if (strcmp(argv[0], "C16_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Gain1);
	else if (strcmp(argv[0], "C16_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Gain2);
	else if (strcmp(argv[0], "C16_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C16_Gain3);
	else if (strcmp(argv[0], "C17_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Freq0);
	else if (strcmp(argv[0], "C17_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Freq1);
	else if (strcmp(argv[0], "C17_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Freq2);
	else if (strcmp(argv[0], "C17_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Freq3);
	else if (strcmp(argv[0], "C17_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Gain0);
	else if (strcmp(argv[0], "C17_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Gain1);
	else if (strcmp(argv[0], "C17_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Gain2);
	else if (strcmp(argv[0], "C17_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C17_Gain3);
	else if (strcmp(argv[0], "C18_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Freq0);
	else if (strcmp(argv[0], "C18_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Freq1);
	else if (strcmp(argv[0], "C18_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Freq2);
	else if (strcmp(argv[0], "C18_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Freq3);
	else if (strcmp(argv[0], "C18_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Gain0);
	else if (strcmp(argv[0], "C18_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Gain1);
	else if (strcmp(argv[0], "C18_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Gain2);
	else if (strcmp(argv[0], "C18_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C18_Gain3);
	else if (strcmp(argv[0], "C19_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Freq0);
	else if (strcmp(argv[0], "C19_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Freq1);
	else if (strcmp(argv[0], "C19_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Freq2);
	else if (strcmp(argv[0], "C19_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Freq3);
	else if (strcmp(argv[0], "C19_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Gain0);
	else if (strcmp(argv[0], "C19_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Gain1);
	else if (strcmp(argv[0], "C19_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Gain2);
	else if (strcmp(argv[0], "C19_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C19_Gain3);
	else if (strcmp(argv[0], "CadOn20")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn20);
	else if (strcmp(argv[0], "CadOn21")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn21);
	else if (strcmp(argv[0], "CadOn22")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn22);
	else if (strcmp(argv[0], "CadOn23")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn23);
	else if (strcmp(argv[0], "CadOff20")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff20);
	else if (strcmp(argv[0], "CadOff21")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff21);
	else if (strcmp(argv[0], "CadOff22")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff22);
	else if (strcmp(argv[0], "CadOff23")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff23);
	else if (strcmp(argv[0], "C20_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Freq0);
	else if (strcmp(argv[0], "C20_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Freq1);
	else if (strcmp(argv[0], "C20_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Freq2);
	else if (strcmp(argv[0], "C20_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Freq3);
	else if (strcmp(argv[0], "C20_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Gain0);
	else if (strcmp(argv[0], "C20_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Gain1);
	else if (strcmp(argv[0], "C20_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Gain2);
	else if (strcmp(argv[0], "C20_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C20_Gain3);
	else if (strcmp(argv[0], "C21_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Freq0);
	else if (strcmp(argv[0], "C21_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Freq1);
	else if (strcmp(argv[0], "C21_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Freq2);
	else if (strcmp(argv[0], "C21_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Freq3);
	else if (strcmp(argv[0], "C21_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Gain0);
	else if (strcmp(argv[0], "C21_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Gain1);
	else if (strcmp(argv[0], "C21_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Gain2);
	else if (strcmp(argv[0], "C21_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C21_Gain3);
	else if (strcmp(argv[0], "C22_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Freq0);
	else if (strcmp(argv[0], "C22_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Freq1);
	else if (strcmp(argv[0], "C22_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Freq2);
	else if (strcmp(argv[0], "C22_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Freq3);
	else if (strcmp(argv[0], "C22_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Gain0);
	else if (strcmp(argv[0], "C22_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Gain1);
	else if (strcmp(argv[0], "C22_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Gain2);
	else if (strcmp(argv[0], "C22_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C22_Gain3);
	else if (strcmp(argv[0], "C23_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Freq0);
	else if (strcmp(argv[0], "C23_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Freq1);
	else if (strcmp(argv[0], "C23_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Freq2);
	else if (strcmp(argv[0], "C23_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Freq3);
	else if (strcmp(argv[0], "C23_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Gain0);
	else if (strcmp(argv[0], "C23_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Gain1);
	else if (strcmp(argv[0], "C23_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Gain2);
	else if (strcmp(argv[0], "C23_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C23_Gain3);
	else if (strcmp(argv[0], "CadOn24")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn24);
	else if (strcmp(argv[0], "CadOn25")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn25);
	else if (strcmp(argv[0], "CadOn26")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn26);
	else if (strcmp(argv[0], "CadOn27")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn27);
	else if (strcmp(argv[0], "CadOff24")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff24);
	else if (strcmp(argv[0], "CadOff25")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff25);
	else if (strcmp(argv[0], "CadOff26")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff26);
	else if (strcmp(argv[0], "CadOff27")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff27);
	else if (strcmp(argv[0], "C24_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Freq0);
	else if (strcmp(argv[0], "C24_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Freq1);
	else if (strcmp(argv[0], "C24_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Freq2);
	else if (strcmp(argv[0], "C24_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Freq3);
	else if (strcmp(argv[0], "C24_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Gain0);
	else if (strcmp(argv[0], "C24_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Gain1);
	else if (strcmp(argv[0], "C24_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Gain2);
	else if (strcmp(argv[0], "C24_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C24_Gain3);
	else if (strcmp(argv[0], "C25_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Freq0);
	else if (strcmp(argv[0], "C25_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Freq1);
	else if (strcmp(argv[0], "C25_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Freq2);
	else if (strcmp(argv[0], "C25_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Freq3);
	else if (strcmp(argv[0], "C25_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Gain0);
	else if (strcmp(argv[0], "C25_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Gain1);
	else if (strcmp(argv[0], "C25_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Gain2);
	else if (strcmp(argv[0], "C25_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C25_Gain3);
	else if (strcmp(argv[0], "C26_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Freq0);
	else if (strcmp(argv[0], "C26_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Freq1);
	else if (strcmp(argv[0], "C26_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Freq2);
	else if (strcmp(argv[0], "C26_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Freq3);
	else if (strcmp(argv[0], "C26_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Gain0);
	else if (strcmp(argv[0], "C26_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Gain1);
	else if (strcmp(argv[0], "C26_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Gain2);
	else if (strcmp(argv[0], "C26_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C26_Gain3);
	else if (strcmp(argv[0], "C27_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Freq0);
	else if (strcmp(argv[0], "C27_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Freq1);
	else if (strcmp(argv[0], "C27_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Freq2);
	else if (strcmp(argv[0], "C27_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Freq3);
	else if (strcmp(argv[0], "C27_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Gain0);
	else if (strcmp(argv[0], "C27_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Gain1);
	else if (strcmp(argv[0], "C27_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Gain2);
	else if (strcmp(argv[0], "C27_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C27_Gain3);
	else if (strcmp(argv[0], "CadOn28")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn28);
	else if (strcmp(argv[0], "CadOn29")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn29);
	else if (strcmp(argv[0], "CadOn30")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn30);
	else if (strcmp(argv[0], "CadOn31")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOn31);
	else if (strcmp(argv[0], "CadOff28")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff28);
	else if (strcmp(argv[0], "CadOff29")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff29);
	else if (strcmp(argv[0], "CadOff30")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff30);
	else if (strcmp(argv[0], "CadOff31")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].CadOff31);
	else if (strcmp(argv[0], "C28_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Freq0);
	else if (strcmp(argv[0], "C28_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Freq1);
	else if (strcmp(argv[0], "C28_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Freq2);
	else if (strcmp(argv[0], "C28_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Freq3);
	else if (strcmp(argv[0], "C28_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Gain0);
	else if (strcmp(argv[0], "C28_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Gain1);
	else if (strcmp(argv[0], "C28_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Gain2);
	else if (strcmp(argv[0], "C28_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C28_Gain3);
	else if (strcmp(argv[0], "C29_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Freq0);
	else if (strcmp(argv[0], "C29_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Freq1);
	else if (strcmp(argv[0], "C29_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Freq2);
	else if (strcmp(argv[0], "C29_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Freq3);
	else if (strcmp(argv[0], "C29_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Gain0);
	else if (strcmp(argv[0], "C29_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Gain1);
	else if (strcmp(argv[0], "C29_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Gain2);
	else if (strcmp(argv[0], "C29_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C29_Gain3);
	else if (strcmp(argv[0], "C30_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Freq0);
	else if (strcmp(argv[0], "C30_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Freq1);
	else if (strcmp(argv[0], "C30_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Freq2);
	else if (strcmp(argv[0], "C30_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Freq3);
	else if (strcmp(argv[0], "C30_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Gain0);
	else if (strcmp(argv[0], "C30_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Gain1);
	else if (strcmp(argv[0], "C30_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Gain2);
	else if (strcmp(argv[0], "C30_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C30_Gain3);
	else if (strcmp(argv[0], "C31_Freq0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Freq0);
	else if (strcmp(argv[0], "C31_Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Freq1);
	else if (strcmp(argv[0], "C31_Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Freq2);
	else if (strcmp(argv[0], "C31_Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Freq3);
	else if (strcmp(argv[0], "C31_Gain0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Gain0);
	else if (strcmp(argv[0], "C31_Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Gain1);
	else if (strcmp(argv[0], "C31_Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Gain2);
	else if (strcmp(argv[0], "C31_Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].C31_Gain3);
	else if (strcmp(argv[0], "display")==0)
	{
		if (pCfg->tone_of_country == TONE_CUSTOMER)
			boaWrite(wp, "style=\"display:online\"");		
		else
			boaWrite(wp, "style=\"display:none\"");		
	}
#if defined(CONFIG_RTK_VOIP_DRIVERS_SI3050) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316)
	else if (strcmp(argv[0], "display_distone")==0)
		boaWrite(wp, "%s", "");
#else
	else if (strcmp(argv[0], "display_distone")==0)
		boaWrite(wp, "%s", "style=\"display:none\"");
#endif
	else if (strcmp(argv[0], "distone_num")==0)
	{
		for (i=0; i < DIS_CONNECT_TONE_MAX ;i++)
		{
			if (i == pCfg->distone_num)
				boaWrite(wp, "<option value=%d selected>%s</option>", i, number_of_distone[i]);
			else
				boaWrite(wp, "<option value=%d >%s</option>", i, number_of_distone[i]);
		}
	}
	else if (strcmp(argv[0], "d1display")==0)
	{
		if (pCfg->distone_num > 0)
#if defined(CONFIG_RTK_VOIP_DRIVERS_SI3050) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_LE89316)
			boaWrite(wp, "style=\"display:online\"");		
#else
			boaWrite(wp, "style=\"display:none\"");
#endif
		else
			boaWrite(wp, "style=\"display:none\"");		
	}
	else if (strcmp(argv[0], "d1freqnum")==0)
	{
		for (i=1; i < DIS_CONNECT_TONE_FREQ_MAX ;i++)
		{
			if (i == pCfg->d1freqnum)
				boaWrite(wp, "<option selected>%s</option>", number_of_distone[i]);
			else
				boaWrite(wp, "<option>%s</option>", number_of_distone[i]);
		}
	}
	else if (strcmp(argv[0], "d1Freq1")==0)
		boaWrite(wp, "%d", pCfg->d1Freq1);
	else if (strcmp(argv[0], "d1Freq2")==0)
		boaWrite(wp, "%d", pCfg->d1Freq2);
	else if (strcmp(argv[0], "d1Accur")==0)
		boaWrite(wp, "%d", pCfg->d1Accur);
	else if (strcmp(argv[0], "d1Level")==0)
		boaWrite(wp, "%d", pCfg->d1Level);
	else if (strcmp(argv[0], "d1ONup")==0)
		boaWrite(wp, "%d", pCfg->d1ONup);
	else if (strcmp(argv[0], "d1ONlow")==0)
		boaWrite(wp, "%d", pCfg->d1ONlow);
	else if (strcmp(argv[0], "d1OFFup")==0)
		boaWrite(wp, "%d", pCfg->d1OFFup);
	else if (strcmp(argv[0], "d1OFFlow")==0)
		boaWrite(wp, "%d", pCfg->d1OFFlow);
	else if (strcmp(argv[0], "d2display")==0)
	{
		if (pCfg->distone_num > 1)
			boaWrite(wp, "style=\"display:online\"");		
		else
			boaWrite(wp, "style=\"display:none\"");		
	}
	else if (strcmp(argv[0], "d2freqnum")==0)
	{
		for (i=1; i < DIS_CONNECT_TONE_FREQ_MAX ;i++)
		{
			if (i == pCfg->d2freqnum)
				boaWrite(wp, "<option selected>%s</option>", number_of_distone[i]);
			else
				boaWrite(wp, "<option>%s</option>", number_of_distone[i]);
		}
	}
	else if (strcmp(argv[0], "d2Freq1")==0)
		boaWrite(wp, "%d", pCfg->d2Freq1);
	else if (strcmp(argv[0], "d2Freq2")==0)
		boaWrite(wp, "%d", pCfg->d2Freq2);
	else if (strcmp(argv[0], "d2Accur")==0)
		boaWrite(wp, "%d", pCfg->d2Accur);
	else if (strcmp(argv[0], "d2Level")==0)
		boaWrite(wp, "%d", pCfg->d2Level);
	else if (strcmp(argv[0], "d2ONup")==0)
		boaWrite(wp, "%d", pCfg->d2ONup);
	else if (strcmp(argv[0], "d2ONlow")==0)
		boaWrite(wp, "%d", pCfg->d2ONlow);
	else if (strcmp(argv[0], "d2OFFup")==0)
		boaWrite(wp, "%d", pCfg->d2OFFup);
	else if (strcmp(argv[0], "d2OFFlow")==0)
		boaWrite(wp, "%d", pCfg->d2OFFlow);



	

	return 0;
}


