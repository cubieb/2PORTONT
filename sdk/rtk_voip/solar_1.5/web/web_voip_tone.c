#include <stdio.h>
#include "web_voip.h"

char tone_country[TONE_MAX][20] = {"USA", "UK", "AUSTRALIA", "HONG KONG", "JAPAN",
				   "SWEDEN", "GERMANY", "FRANCE", "TAIWAN", "BELGIUM",
				   "FINLAND", "ITALY", "CHINA", "CUSTOMER"};

char cust_tone[TONE_CUSTOMER_MAX][20] = {"Custom1", "Custom2", "Custom3", "Custom4",
					 "Custom5", "Custom6", "Custom7", "Custom8"};

char tone_type[TONE_TYPE_MAX][20] = {"ADDITIVE", "MODULATED", "SUCC"};

#if 1
char tone_cycle[TONE_CYCLE_MAX][20] = {"CONTINUOUS", "BURST", "CADENCE"};
#else
char tone_cycle[TONE_CYCLE_MAX][20] = {"CONTINUOUS", "BURST"};
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

		ptr = boaGetVar(wp, "cycle", "");

		for(i=0; i < TONE_CYCLE_MAX; i++)
		{
			if (!gstrcmp(ptr, tone_cycle[i]))
				break;
		}
		if (i == TONE_CYCLE_MAX)
			i = TONE_CYCLE_CONTINUOUS;

		pCfg->cust_tone_para[cust_idx].cycle = i;

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
		pCfg->cust_tone_para[cust_idx].Freq1 = atoi(boaGetVar(wp, "Freq1", ""));
		pCfg->cust_tone_para[cust_idx].Freq2 = atoi(boaGetVar(wp, "Freq2", ""));
		pCfg->cust_tone_para[cust_idx].Freq3 = atoi(boaGetVar(wp, "Freq3", ""));
		pCfg->cust_tone_para[cust_idx].Freq4 = atoi(boaGetVar(wp, "Freq4", ""));
		pCfg->cust_tone_para[cust_idx].Gain1 = atoi(boaGetVar(wp, "Gain1", ""));
		pCfg->cust_tone_para[cust_idx].Gain2 = atoi(boaGetVar(wp, "Gain2", ""));
		pCfg->cust_tone_para[cust_idx].Gain3 = atoi(boaGetVar(wp, "Gain3", ""));
		pCfg->cust_tone_para[cust_idx].Gain4 = atoi(boaGetVar(wp, "Gain4", ""));
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


#if CONFIG_RTK_VOIP_PACKAGE_865X
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
		for (i=0; i < TONE_CYCLE_MAX ;i++)
		{
			if (i == pCfg->cust_tone_para[cust_idx].cycle)
				boaWrite(wp, "<option selected>%s</option>", tone_cycle[i]);
			else
				boaWrite(wp, "<option>%s</option>", tone_cycle[i]);
		}
	}
	else if (strcmp(argv[0], "cadNUM")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].cadNUM);
	else if (strcmp(argv[0], "CadOn0")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn0);
	else if (strcmp(argv[0], "CadOn1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx] .CadOn1);
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
	else if (strcmp(argv[0], "Freq1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq1);
	else if (strcmp(argv[0], "Freq2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq2);
	else if (strcmp(argv[0], "Freq3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq3);
	else if (strcmp(argv[0], "Freq4")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Freq4);
	else if (strcmp(argv[0], "Gain1")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain1);
	else if (strcmp(argv[0], "Gain2")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain2);
	else if (strcmp(argv[0], "Gain3")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain3);
	else if (strcmp(argv[0], "Gain4")==0)
		boaWrite(wp, "%d", pCfg->cust_tone_para[cust_idx].Gain4);
	else if (strcmp(argv[0], "display")==0)
	{
		if (pCfg->tone_of_country == TONE_CUSTOMER)
			boaWrite(wp, "style=\"display:online\"");
		else
			boaWrite(wp, "style=\"display:none\"");
	}
#ifdef CONFIG_RTK_VOIP_DRIVERS_SI3050
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
			boaWrite(wp, "style=\"display:online\"");
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


