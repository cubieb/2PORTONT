/*
 * API in this files will associated with DECT API, which use pthread
 * functions, so we move them to an individual file. 
 * The advantages is that most of applications expecting to SIP don't 
 * need to link with pthread library. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
// fsk date & time sync
#include <time.h>
#include <sys/time.h>
// fsk date & time sync
#include "voip_manager.h"

#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
#include "si_dect_api.h"
#endif
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
#include "cmbs_api.h"
#include "appcall.h"
#include "dect_test_scenario.h"
#endif

#define SETSOCKOPT(optid, varptr, vartype, qty) \
        { \
                int     sockfd; \
                if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) { \
                        return -1; \
                } \
                if (setsockopt(sockfd, IPPROTO_IP, optid, (void *)varptr, sizeof(vartype)*qty) != 0) { \
                        close(sockfd); \
                        return -2; \
                } \
                close(sockfd); \
        }

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT		// enclose this file 


pthread_mutex_t         semDectEventFifoSem;

#define MUTEXOBTAIN_semDectEventFifoSem pthread_mutex_lock( &semDectEventFifoSem );
#define MUTEXRELEASE_semDectEventFifoSem pthread_mutex_unlock( &semDectEventFifoSem );

void dect_event_mutex_init(void)
{
	pthread_mutex_init(&semDectEventFifoSem,0);
}

typedef enum
{
	HS_OCCUPY_FAIL = 0,
	HS_OCCUPY_SUCC = 1,
	HS_OCCUPY_IDLE = 2
}
HS_ENT_ST;

#define DECT_EVENT_FIFO_SIZE 20
#define MAX_DECT_CH_SIZE 8
#define MAX_LINE_SUPPORT 4
#define MAX_HS_SUPPORT 5
#define NONE -1
static char dect_event_fifo[MAX_DECT_CH_SIZE][DECT_EVENT_FIFO_SIZE];
static int dect_event_wp[MAX_DECT_CH_SIZE]={0}, dect_event_rp[MAX_DECT_CH_SIZE]={0};
static int LineOccupyByHS[MAX_LINE_SUPPORT] = {[0 ... MAX_LINE_SUPPORT-1] = NONE};
static HS_ENT_ST HsEntSt[MAX_HS_SUPPORT] = {[0 ... MAX_HS_SUPPORT-1] = HS_OCCUPY_IDLE};

static int dect_event_init(void)
{
	int i;
	
	for (i=0; i<MAX_DECT_CH_SIZE; i++)
	{
		dect_event_wp[i] = 0;
		dect_event_rp[i] = 0;
	}
	
	for (i=0; i<MAX_LINE_SUPPORT; i++)
		LineOccupyByHS[i] = NONE;

	for (i=0; i<MAX_HS_SUPPORT; i++)
		HsEntSt[i] = HS_OCCUPY_IDLE;
	
	return 0;
}

int dect_event_in(uint32 line_id, uint32 hs_id, char input)
{
	//printf("dect_event_in, ch%d\n", line_id);
	uint32 ch_id;
	int key;
	
	MUTEXOBTAIN_semDectEventFifoSem;
	
	if ( LineOccupyByHS[line_id] == NONE ) // Line is IDLE
	{
		if (HsEntSt[hs_id] == HS_OCCUPY_IDLE)
		{
			if (input == 1) //off-hook
			{
				LineOccupyByHS[line_id] = hs_id;
				HsEntSt[hs_id] = HS_OCCUPY_SUCC; //IDLE-->SUCC
				printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_SUCC\n.", line_id, LineOccupyByHS[line_id], hs_id);
			}
		}
		else if (HsEntSt[hs_id] == HS_OCCUPY_FAIL)
		{
			if (input ==0) //on-hook
			{
				HsEntSt[hs_id] = HS_OCCUPY_IDLE;
				input += 100;
			}
			else if (input == 1) //off-hook
				printf("Error! Shout not go to here: %s, line%d\n", __FUNCTION__, __LINE__);
				
			printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_IDLE\n.", line_id, LineOccupyByHS[line_id], hs_id);
		}
		else if (HsEntSt[hs_id] == HS_OCCUPY_SUCC)
		{
			printf("Error! Shout not go to here: %s, line%d\n", __FUNCTION__, __LINE__);
		}
	}
	else if ( LineOccupyByHS[line_id] != NONE ) // Line is Occupy
	{
		// Line has been used by other HS in the same line group.
		if ( LineOccupyByHS[line_id] != hs_id )
		{
			if (input == 1) //off-hook
			{
				HsEntSt[hs_id] = HS_OCCUPY_FAIL; //IDLE-->FAIL			
				printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_FAIL\n.", line_id, LineOccupyByHS[line_id], hs_id);
			}
			else if (input ==0) //on-hook
			{
				HsEntSt[hs_id] = HS_OCCUPY_IDLE; //FAIL-->IDLE	
				printf("[DECT]: line%d occupy by HS%d, HS%d =HS_OCCUPY_IDLE\n.", line_id, LineOccupyByHS[line_id], hs_id);
			}
			
			input += 100;
				
		}
		else
		{
			if (input == 0) //on-hook
			{
				LineOccupyByHS[line_id] = NONE;
				HsEntSt[hs_id] = HS_OCCUPY_IDLE;//SUCC-->IDLE
				printf("[DECT]: line%d is IDLE, HS%d =HS_OCCUPY_IDLE\n.", line_id, hs_id);
			}
		}
	}
	
	ch_id = line_id;
	
        if ((dect_event_wp[ch_id]+1)%DECT_EVENT_FIFO_SIZE != dect_event_rp[ch_id])
	{
	 	dect_event_fifo[ch_id][dect_event_wp[ch_id]] = input;
                dect_event_wp[ch_id] = (dect_event_wp[ch_id]+1) % DECT_EVENT_FIFO_SIZE;
	  	//printf("dect_event_wp=%d\n", dect_event_wp[ch_id]);

#if 1 // play DTMF tone by VoIP DSP
		if (input >= '0' && input <= '9')
		{
			key = input - '0';
			rtk_SetPlayTone(ch_id, 0, key, 1, 0);
		}
		else if (input == '*')
		{
			key = 10;//DSPCODEC_TONE_STARSIGN;
			rtk_SetPlayTone(ch_id, 0, key, 1, 0);
		}
		else if (input == '#')
		{
			key = 11;//DSPCODEC_TONE_HASHSIGN;
			rtk_SetPlayTone(ch_id, 0, key, 1, 0);
		}
#endif
	}
	else
	{
		printf("dect_event FIFO overflow,(%d)\n", ch_id);
	}
	
	MUTEXRELEASE_semDectEventFifoSem;
	
	return 0;
}

char dect_event_out(uint32 line_id)
{
	char output;
	uint32 ch_id;

	MUTEXOBTAIN_semDectEventFifoSem;
	
	ch_id = line_id;
	
	if ( dect_event_wp[ch_id] == dect_event_rp[ch_id]) // FIFO empty
	{
		output = 'Z';
		//printf("output = %d\n", output);
	}
	else
	{
		output = dect_event_fifo[ch_id][dect_event_rp[ch_id]];
                dect_event_rp[ch_id] = (dect_event_rp[ch_id]+1) % DECT_EVENT_FIFO_SIZE;
		//printf("dect_event_rp=%d\n", dect_event_rp[ch_id]);
		//printf("output = %d\n", output);
	}
	
	MUTEXRELEASE_semDectEventFifoSem;
	
	return output;
}

int32 rtk_GetLineOccupyHS(uint32 line_id)
{
	return LineOccupyByHS[line_id];
}

/*
 * @ingroup VOIP_DECT
 * @brief Set DECT power 
 * @param power DECT power. 0: active, 1: inactive.
 * @retval 0 Success
 */
int32 rtk_SetDectPower( uint32 power )
{
	TstVoipSingleValue stVoipSingleValue;
	
	stVoipSingleValue.value = power;
	
	SETSOCKOPT(VOIP_MGR_DECT_SET_POWER, &stVoipSingleValue, TstVoipSingleValue, 1);
	
	dect_event_init();
	
	return 0;
}

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT power 
 * @retval DECT power. 0: active, 1: inactive.
 */
int32 rtk_GetDectPower( void )
{
	TstVoipSingleValue stVoipSingleValue;
	
	stVoipSingleValue.value = 0;
	
	SETSOCKOPT(VOIP_MGR_DECT_GET_POWER, &stVoipSingleValue, TstVoipSingleValue, 1);
	
	return stVoipSingleValue.value;
}

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT page button 
 * @retval DECT page. 0: active, 1: inactive.
 */
int32 rtk_GetDectPage( void )
{
	TstVoipSingleValue stVoipSingleValue;
	
	stVoipSingleValue.value = 0;
	
	SETSOCKOPT(VOIP_MGR_DECT_GET_PAGE, &stVoipSingleValue, TstVoipSingleValue, 1);
	
	return stVoipSingleValue.value;
}

/*
 * @ingroup VOIP_DECT
 * @brief Get DECT button event
 * @retval DECT button event. -1: no event, 0x1: page, 0x2: registration mode, 0x3: delete registered HS, 0xff: not defined
 */
int32 rtk_GetDectButtonEvent( void )
{
	TstVoipValue stVoipValue;
	
	stVoipValue.value = 0;
	
	SETSOCKOPT(VOIP_MGR_DECT_BUTTON_GET, &stVoipValue, stVoipValue, 1);

	//printf("rtk_GetDectButtonEvent : %d\n", stVoipValue.value);
	
	return stVoipValue.value;
}

int32 rtk_SetRingFXS(uint32 chid, uint32 bRinging)
{
	extern int g_bDisableRingFXS;

	if (g_bDisableRingFXS)
	{
		// quiet mode
		return 0;
	}
	
#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT

	if( bRinging == 1 ) {
		dect_api_S2R_call_setup( 0, chid, "12345", "noname" );
	} else {
		if( dect_api_S2R_check_handset_ringing( chid ) )
			dect_api_S2R_call_release( chid );
	}
#endif

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT

	if (bRinging == 1)
	{
		CMBS_Api_InBound_Ring_CallerID(chid, NULL, NULL);
	}
	else
	{	
		if(CMBS_Api_LineStateGet(chid) == 2)//ringing
		{
			CMBS_Api_CallRelease(chid, 0);
		}
	}
#endif

	return 0;
}

int32 rtk_Gen_CID_And_FXS_Ring(uint32 chid, char cid_mode, char *str_cid, char *str_date, char *str_cid_name, char fsk_type, uint32 bRinging)
{
#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT 
	if (str_cid[0] != 0)//Send CID and Ring FXS by DSP
	{
		dect_api_S2R_call_setup( 0, chid, str_cid, str_cid_name );
	}
	else
	{
		if( dect_api_S2R_check_handset_ringing( chid ) )
			dect_api_S2R_call_release( chid );
	}
#endif

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	extern int g_bDisableRingFXS;

	if (str_cid[0] != 0)//Send CID and Ring FXS by DSP
	{
		if (0 == (strcmp(str_cid, "-")))
		{
			//strcpy(str_cid, "P");
			CMBS_Api_InBound_Ring_CallerID(chid, NULL, str_cid_name);
		}
		else
			CMBS_Api_InBound_Ring_CallerID(chid, str_cid, str_cid_name);

	}
	else	// Just Ring FXS
	{
		if (g_bDisableRingFXS)
		{
			// quiet mode
			return 0;
		}
		
		if (bRinging == 1)
		{
			CMBS_Api_InBound_Ring_CallerID(chid, NULL, NULL);
		}
		else
		{
			if(CMBS_Api_LineStateGet(chid) == 2)//ringing
			{
				CMBS_Api_CallRelease(chid, 0);
			}
		}
	}
#endif
	return 0;
}

int32 rtk_GetFxsEvent(uint32 chid, SIGNSTATE *pval)
{
	TstVoipSlicHook stVoipSlicHook;
	TstVoipCfg stVoipCfg;
	TstVoipValue stVoipValue;
	char event;
	
	*pval = SIGN_NONE;
	
	stVoipSlicHook.ch_id = chid;

#if 1
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
	stVoipValue.value = rtk_GetDectButtonEvent();

	if (stVoipValue.value == 0x1)
	{
		CMBS_Api_HandsetPage("all");
		//printf("page all HS\n");
	}
	else if (stVoipValue.value == 0x2)
	{
		CMBS_Api_RegistrationOpen();
		//printf("registration mode open\n");
	}
	else if (stVoipValue.value == 0x3)
	{
		CMBS_Api_HandsetDelet("all");
		//printf("delete all registed HS\n");
	}
#endif

	if (chid < SLIC_CH_NUM)//(chid == 0)
	{


		stVoipValue.value = dect_event_out(chid);

		
		if ('Z' != stVoipValue.value)
		{
			
			// HS hook event
			if (stVoipValue.value == 0)
			{
				*pval = SIGN_ONHOOK;
				/* when phone onhook, stop play tone, disable pcm and DTMF detection */
				rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 0, DSPCODEC_TONEDIRECTION_LOCAL);
				usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!
				
				stVoipCfg.ch_id = chid;
				stVoipCfg.enable = 0;
				SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
				/* when phone onhook, re-init DSP */
				SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
			}
			else if (stVoipValue.value == 1 || stVoipValue.value == 2)
			{
				
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
				CMBS_Api_OutboundAnswerCall(chid);
#endif
				
				*pval = SIGN_OFFHOOK;
				/* when phone offhook, enable pcm */
				stVoipCfg.ch_id = chid;
				stVoipCfg.enable = stVoipValue.value;
	
				SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
			}
			else if (stVoipValue.value == 3)	// pkshih: change 2 to 3 
			{
				*pval = SIGN_FLASHHOOK;					
			}				
			// HS key event
			else if (stVoipValue.value == '*')
			{
				*pval = SIGN_STAR;
			}
			else if (stVoipValue.value == '#')
			{
				*pval = SIGN_HASH;
			}
			else if (stVoipValue.value == '0')
			{
				*pval = SIGN_KEY0;
			}
			else if (stVoipValue.value >= '1' && stVoipValue.value <= '9')
			{
				*pval = SIGN_KEY1 + stVoipValue.value - '1';
			}
			else if (stVoipValue.value == 100)
			{
				//DECT on-hook of the HS which not occupy line
				*pval = SIGN_NONE;
				printf("DECT on-hook of the HS which not occupy line\n");
			}
			else if (stVoipValue.value == 101)
			{
				//DECT off-hook of the HS which not occupy line
#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT
				CMBS_Api_OutboundAnswerCall(chid);
#endif
				*pval = SIGN_NONE;
				printf("DECT off-hook of the HS which not occupy line\n");
			}
			else
			{
				*pval = SIGN_NONE;
			}
			
					
			return 0;
			
		}
		else
		{
			//printf("==>UNKOWN SLIC STATUS (%d)\n", chid);   /* HOOK FIFO empty also belong to this case */
			return -1;
		}
	}
	else	// only support ch0
	{
		return -1;
	}
	
	return 0;
}


#endif // CONFIG_RTK_VOIP_DRIVERS_ATA_DECT

