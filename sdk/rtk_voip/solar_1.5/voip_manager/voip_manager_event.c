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

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT

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

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT

pthread_mutex_t         semDectEventFifoSem;

#define MUTEXOBTAIN_semDectEventFifoSem pthread_mutex_lock( &semDectEventFifoSem );
#define MUTEXRELEASE_semDectEventFifoSem pthread_mutex_unlock( &semDectEventFifoSem );

void dect_event_mutex_init(void)
{
	pthread_mutex_init(&semDectEventFifoSem,0);
}

#define DECT_EVENT_FIFO_SIZE 20
static char dect_event_fifo[4][DECT_EVENT_FIFO_SIZE];
static int dect_event_wp[4], dect_event_rp[4];

int dect_event_in(uint32 ch_id, char input)
{

	//printf("dect_event_in, ch%d\n", ch_id);
	
	MUTEXOBTAIN_semDectEventFifoSem;
	
        if ((dect_event_wp[ch_id]+1)%DECT_EVENT_FIFO_SIZE != dect_event_rp[ch_id])
	{
	 	dect_event_fifo[ch_id][dect_event_wp[ch_id]] = input;
                dect_event_wp[ch_id] = (dect_event_wp[ch_id]+1) % DECT_EVENT_FIFO_SIZE;
	  	//printf("dect_event_wp=%d\n", dect_event_wp[ch_id]);
	}
	else
	{
		printf("dect_event FIFO overflow,(%d)\n", ch_id);
	}
	
	MUTEXRELEASE_semDectEventFifoSem;
	
	return 0;
}

char dect_event_out(uint32 ch_id)
{
	char output;

	MUTEXOBTAIN_semDectEventFifoSem;
	
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
#endif

#endif

int32 rtk_GetFxsEvent(uint32 chid, SIGNSTATE *pval)
{

	TstVoipSlicHook stVoipSlicHook;
	TstVoipCfg stVoipCfg;
	TstVoipValue stVoipValue;

	*pval = SIGN_NONE;

#ifdef NO_SLIC
return 0;
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT	

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


	if (chid < RTK_VOIP_DECT_NUM(g_VoIP_Feature))	//DECT Channel
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
			else if (stVoipValue.value == 1)
			{
				CMBS_Api_OutboundAnswerCall(chid);
				
				*pval = SIGN_OFFHOOK;
				/* when phone offhook, enable pcm */
				stVoipCfg.ch_id = chid;
				stVoipCfg.enable = 1;
	
				SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
			}
			else if (stVoipValue.value == 2)
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
	else
#endif
	//SLIC Channel
	{
	
	stVoipSlicHook.ch_id = chid;
	SETSOCKOPT(VOIP_MGR_SLIC_HOOK, &stVoipSlicHook, TstVoipSlicHook, 1);
	//printf("***** %d ******\n", stVoipSlicHook.hook_status);
	if (stVoipSlicHook.hook_status == 4) /* PHONE_STILL_OFF_HOOK */
	{
		// detect DTMF
		stVoipValue.ch_id = chid;
		stVoipValue.value = 'X';
		SETSOCKOPT(VOIP_MGR_DTMF_GET, &stVoipValue, TstVoipValue, 1);

		if (stVoipValue.value >= '1' && stVoipValue.value <= '9')
			*pval = SIGN_KEY1 + stVoipValue.value - '1';
		else if (stVoipValue.value == '0')
			*pval = SIGN_KEY0;
		else if (stVoipValue.value == '*')
			*pval = SIGN_STAR;
		else if (stVoipValue.value == '#')
		    *pval = SIGN_HASH;
		else
			*pval = SIGN_OFFHOOK_2;

		return 0;
	}
	else if (stVoipSlicHook.hook_status == 1) /* PHONE_OFF_HOOK */
	{
		*pval = SIGN_OFFHOOK; // off-hook
#if 1
		/* when phone offhook, enable pcm and DTMF detection */
		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = stVoipSlicHook.hook_status;

		SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
		
		if (stVoipCfg.ret_val != 0)
		{
			return stVoipCfg.ret_val;
		}

#if 0
		if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(NULL))
		{
			no_resource_flag[chid] = 1;
			*pval = 0;
			usleep(500000);//after phone off-hook, wait for a second,and then play IVR.
#if 1
			char text[]={IVR_TEXT_ID_NO_RESOURCE, '\0'};
			printf("play ivr (%d)...\n", chid);
			rtk_IvrStartPlaying( chid, IVR_DIR_LOCAL, text );
#else
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
			return 0;
		}
#endif

		SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
		if (stVoipCfg.ret_val != 0)
		{
			return stVoipCfg.ret_val;
		}
		
		stVoipCfg.enable = 1;
		/* when phone offhook, enable fax detection */
		SETSOCKOPT(VOIP_MGR_FAX_OFFHOOK, &stVoipCfg, TstVoipCfg, 1);

		return stVoipCfg.ret_val;

#endif
	}
	else if (stVoipSlicHook.hook_status == 0) /* PHONE_ON_HOOK */
	{
		*pval = SIGN_ONHOOK;	// on-hook
#if 1
		/* when phone onhook, stop play tone, disable pcm and DTMF detection */
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		usleep(100000); // [Important] sleep >= 100ms. MUST add delay for ACMW to stop tone!
		
		stVoipCfg.ch_id = chid;
		stVoipCfg.enable = stVoipSlicHook.hook_status;
		SETSOCKOPT(VOIP_MGR_PCM_CFG, &stVoipCfg, TstVoipCfg, 1);
		if (stVoipCfg.ret_val != 0)
		{
			return stVoipCfg.ret_val;
		}

#if 0
		if (no_resource_flag[chid] == 1)
		{
			no_resource_flag[chid] = 0;
			*pval = 0;
#if 1
			printf("stop play ivr(%d)...\n", chid);
			rtk_IvrStopPlaying(chid);
#else
			rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 0, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
			return 0;
		}
#endif

		SETSOCKOPT(VOIP_MGR_DTMF_CFG, &stVoipCfg, TstVoipCfg, 1);
		if (stVoipCfg.ret_val != 0)
		{
			return stVoipCfg.ret_val;
		}

		/* when phone onhook, re-init CED detection */
		SETSOCKOPT(VOIP_MGR_ON_HOOK_RE_INIT, &stVoipCfg, TstVoipCfg, 1);
		return stVoipCfg.ret_val;
	
#endif
	}
	else if (stVoipSlicHook.hook_status == 2) /* PHONE_FLASH_HOOK */
	{
		*pval = SIGN_FLASHHOOK;
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 3) /* PHONE_STILL_ON_HOOK */
	{
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 11) /* FXO_RING_ON */
	{
		*pval = SIGN_RING_ON;
		return 0;
	}
	else if (stVoipSlicHook.hook_status == 12)  /* FXO_RING_OFF */
	{
		*pval = SIGN_RING_OFF;
		return 0;
	}
	else
	{
		printf("==>UNKOWN SLIC STATUS (%d)\n", chid);   /* HOOK FIFO empty also belong to this case */
		return -1;
	}
}
}

static int g_bDisableRingFXS = 0;

int32 rtk_DisableRingFXS(int bDisable)
{
	g_bDisableRingFXS = bDisable;
	return 0;
}

int32 rtk_SetRingFXS(uint32 chid, uint32 bRinging)
{
    TstVoipSlicRing stVoipSlicRing;

	if (g_bDisableRingFXS)
	{
		// quiet mode
		return 0;
	}

#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT	
	if (chid < RTK_VOIP_DECT_NUM(g_VoIP_Feature))	//DECT Channel
	{
	
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
		stVoipSlicRing.ret_val = 0;
	}
	else
#endif
	//SLIC Channel
	{
    stVoipSlicRing.ch_id = chid;
    stVoipSlicRing.ring_set = bRinging;
    SETSOCKOPT(VOIP_MGR_SLIC_RING, &stVoipSlicRing, TstVoipSlicRing, 1);
    	}
    
	return stVoipSlicRing.ret_val;
}

int32 rtk_Gen_CID_And_FXS_Ring(uint32 chid, char cid_mode, char *str_cid, char *str_date, char *str_cid_name, char fsk_type, uint32 bRinging)
{

	TstVoipSlicRing stVoipSlicRing;
    int32 ret;
	
#ifdef CONFIG_RTK_VOIP_DRIVERS_ATA_DECT	
	if (chid < RTK_VOIP_DECT_NUM(g_VoIP_Feature))	//DECT Channel
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
	else
#endif
	//SLIC Channel
	{
	if (str_cid[0] != 0)//Send CID and Ring FXS by DSP
	{
		if (cid_mode == 0)	// DTMF CID
		{
			ret = rtk_Gen_Dtmf_CID(chid, str_cid);
		}
		else	// FSK CID
		{
			ret = rtk_Gen_FSK_CID(chid, str_cid, str_date, str_cid_name, fsk_type);
		}
		
		return ret;
	}
	else	// Just Ring FXS
	{
		if (g_bDisableRingFXS)
		{
			// quiet mode
			return 0;
		}
		
		stVoipSlicRing.ch_id = chid;
		stVoipSlicRing.ring_set = bRinging;
		SETSOCKOPT(VOIP_MGR_SLIC_RING, &stVoipSlicRing, TstVoipSlicRing, 1);
	    
		return stVoipSlicRing.ret_val;
	}
}
}


