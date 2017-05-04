#include <stdio.h>
#include "web_voip.h"

char ring_group[RING_GROUP_MAX][20] = {"Group1", "Group2","Group3","Group4"};

char cad[RING_CADENCE_MAX][20] = {"Cadence1", "Cadence2", "Cadence3", "Cadence4",
				  "Cadence5", "Cadence6", "Cadence7", "Cadence8"};

char ring_cad[RING_CADENCE_MAX+1][20] = {"Cadence1", "Cadence2", "Cadence3", "Cadence4",
				    	 "Cadence5", "Cadence6", "Cadence7", "Cadence8", "Default"};


void asp_voip_RingSet(request * wp, char *path, char *query)
{
	char *ptr;
	voipCfgParam_t *pCfg;
	int i, group_idx, cad_idx;

	if (web_flash_get(&pCfg) != 0)
		return;

	/* Ring Cadence */
	ptr = boaGetVar(wp, "ring_cad", "");

	for(i=0; i < RING_CADENCE_MAX+1; i++)
	{
		if (!gstrcmp(ptr, ring_cad[i]))
			break;
	}
	if (i == RING_CADENCE_MAX+1)
		i = RING_CADENCE_1;

	pCfg->ring_cad = i;


	/* Group */
	ptr = boaGetVar(wp, "group", "");

	for(i=0; i < RING_GROUP_MAX; i++)
	{
		if (!gstrcmp(ptr, ring_group[i]))
			break;
	}
	if (i == RING_GROUP_MAX)
		i = RING_GROUP_1;

	pCfg->ring_group = group_idx = i;

	/* Cadence Select */
	ptr = boaGetVar(wp, "cadence_sel", "");

	for(i=0; i < RING_CADENCE_MAX; i++)
	{
		if (!gstrcmp(ptr, cad[i]))
			break;
	}
	if (i == RING_CADENCE_MAX)
		i = RING_CADENCE_1;

	pCfg->ring_cadence_sel = cad_idx = i;


	/* Ring_Cad Apply */
	ptr = boaGetVar(wp, "Ring_Cad", "");
	if (strcmp(ptr, "Apply") == 0)
	{
		/* Ring Cadence for All */
		ptr = boaGetVar(wp, "ring_cad", "");

		for(i=0; i < RING_CADENCE_MAX+1; i++)
		{
			if (!gstrcmp(ptr, ring_cad[i]))
				break;
		}
		if (i == RING_CADENCE_MAX+1)
			i = RING_CADENCE_1;

		pCfg->ring_cad = i;
	}

	/* Ring_Group Apply */
	ptr = boaGetVar(wp, "Ring_Group", "");
	if (strcmp(ptr, "Apply") == 0)
	{
		/* Phone Number */
		pCfg->ring_phone_num[group_idx] = atoi(boaGetVar(wp, "phonenumber", ""));

		/* Cadence Use*/
		ptr = boaGetVar(wp, "cadence_use", "");

		for(i=0; i < RING_CADENCE_MAX; i++)
		{
			if (!gstrcmp(ptr, cad[i]))
				break;
		}
		if (i == RING_CADENCE_MAX)
			i = RING_CADENCE_1;

		pCfg->ring_cadence_use[group_idx] = i;
	}

	/* Ring_Cadence Apply */
	ptr = boaGetVar(wp, "Ring_Cadence", "");
	if (strcmp(ptr, "Apply") == 0)
	{
		/* Cadence ON/OFF */
		pCfg->ring_cadon[cad_idx] = atoi(boaGetVar(wp, "cad_on", ""));
		pCfg->ring_cadoff[cad_idx] = atoi(boaGetVar(wp, "cad_off", ""));
	}



	web_flash_set(pCfg);

#ifdef REBOOT_CHECK
	OK_MSG("/voip_ring.asp");
#else
	web_restart_solar();

	boaRedirect(wp, "/voip_ring.asp");
#endif
}


#ifdef CONFIG_APP_BOA
int asp_voip_RingGet(request * wp, int argc, char **argv)
#else
int asp_voip_RingGet(int ejid, request * wp, int argc, char **argv)
#endif
{
	int i, group_idx, cad_idx;
	voipCfgParam_t *pCfg;

	if (web_flash_get(&pCfg) != 0)
		return -1;

	group_idx = pCfg->ring_group;
	cad_idx = pCfg->ring_cadence_sel;

	/* Cadence */
	if (strcmp(argv[0], "ring_cad")==0)
	{
		for (i=0; i < RING_CADENCE_MAX+1 ;i++)
		{
			if (i == pCfg->ring_cad)
				boaWrite(wp, "<option selected>%s</option>", ring_cad[i]);
			else
				boaWrite(wp, "<option>%s</option>", ring_cad[i]);
		}
	}
	/* Group */
	else if (strcmp(argv[0], "group")==0)
	{
		for (i=0; i < RING_GROUP_MAX ;i++)
		{
			if (i == pCfg->ring_group)
				boaWrite(wp, "<option selected>%s</option>", ring_group[i]);
			else
				boaWrite(wp, "<option>%s</option>", ring_group[i]);
		}
	}
	/* Phone Number */
	else if (strcmp(argv[0], "phonenumber")==0)
		boaWrite(wp, "%d", pCfg->ring_phone_num[group_idx]);
	/* Cadence Use */
	else if (strcmp(argv[0], "cadence_use")==0)
	{
		for (i=0; i < RING_CADENCE_MAX ;i++)
		{
			if (i == pCfg->ring_cadence_use[group_idx])
				boaWrite(wp, "<option selected>%s</option>", cad[i]);
			else
				boaWrite(wp, "<option>%s</option>", cad[i]);
		}
	}
	/* Cadence Select */
	else if (strcmp(argv[0], "cadence_sel")==0)
	{
		for (i=0; i < RING_CADENCE_MAX ;i++)
		{
			if (i == pCfg->ring_cadence_sel)
				boaWrite(wp, "<option selected>%s</option>", cad[i]);
			else
				boaWrite(wp, "<option>%s</option>", cad[i]);
		}
	}
	/* Cadence ON/OFF */
	else if(strcmp(argv[0], "cad_on")==0)
		boaWrite(wp, "%d", pCfg->ring_cadon[cad_idx]);
	else if(strcmp(argv[0], "cad_off")==0)
		boaWrite(wp, "%d", pCfg->ring_cadoff[cad_idx]);

	return 0;
}
