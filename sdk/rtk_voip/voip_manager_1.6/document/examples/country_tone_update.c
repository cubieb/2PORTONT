#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "voip_manager.h"

// Guatemala
void SetCustomToneGuatemala(void);
void ToneKeyGuatemala(int chid , SIGNSTATE val);

// Romania
void SetCustomToneRomania(void);
void ToneKeyRomania(int chid , SIGNSTATE val);

// Country Descriptor
typedef struct CountryToneDesc_s{
	char *country_name;
	void (*SetCustomTone)(void);
	void (*ToneKey)(int chid , SIGNSTATE val);
} CountryToneDesc_t;

CountryToneDesc_t CountryToneDesc[] = {
	{ "Guatemala" , SetCustomToneGuatemala , ToneKeyGuatemala } ,
	{ "Romania"   , SetCustomToneRomania   , ToneKeyRomania   }
	};


int main(int argc , char *argv[])
{
	int i;
	SIGNSTATE val;	
	int country_index;
	int country_num;
	
	if( argc != 2 ){
		printf("Usage: %s country_index\n" , argv[0]);
		printf("example:\n");
		printf("  %s 0 # Use country index 0 tone\n" , argv[0]);
		printf("  %s 1 # Use country index 1 tone\n" , argv[0]);
		return 0;
	}
	
	country_index = atoi( argv[1] );
	country_num   = sizeof( CountryToneDesc ) / sizeof( CountryToneDesc[0] );

	if( country_index >= country_num ){
		printf("Wrong country_index\n");
		printf("country_index=%d country_num=%d\n" , country_index , country_num);
		return 0;
	}		
	
	printf("Country %s\n" , CountryToneDesc[country_index].country_name);
	
	CountryToneDesc[country_index].SetCustomTone();
		
	for (i=0; i<CON_CH_NUM; i++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;
		
		rtk_SetVoiceGain(i, 0, 0);
		rtk_SetFlashHookTime(i, 0, 300);
		rtk_SetFlushFifo(i);					// flush kernel fifo before app run
	}

main_loop:

	for (i=0; i<CON_CH_NUM; i++)
	{
		if( !RTK_VOIP_IS_SLIC_CH( i, g_VoIP_Feature ) )
			continue;
			
		rtk_GetFxsEvent(i, &val);

		CountryToneDesc[country_index].ToneKey(i , val);

		usleep(100000 / CON_CH_NUM); // 100 ms
	}

	goto main_loop;

	return 0;
}
          

//Guatemala
void SetCustomToneGuatemala(void)
{
	// Guatemala tone
	/*
					         Freq    on    off    on    off
	dial tone                425   continue
	ring tone                425      1      4 
	busy tone                425    0.5    0.5
	congestion tone          425    0.2    0.2
	waiting call tone        425    0.2    0.2    0.2   10
	call forwarding tone     425      1      4
	tone message waiting     425      1      1
	*/
	
	st_ToneCfgParam dial_tone             = {0, 0, 1, 2000,    0,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	ring_tone             = {0, 0, 1, 1000, 4000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	busy_tone             = {0, 0, 1,  500,  500,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	congestion_tone       = {0, 0, 1,  200,  200,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	call_waiting_tone     = {0, 0, 2,  200,  200, 200, 10000, 0, 0, 0, 0, 0, 1, 425, 425, 0, 0,  7,  7, 0, 0};  //2 cadances
	//st_ToneCfgParam	call_waiting_tone     = {0, 0, 1,  200,  200, 200, 10000, 0, 0, 0, 0, 0, 1, 425, 425, 0, 0,  7,  7, 0, 0};  //1 cadance
	st_ToneCfgParam	call_forwarding_tone  = {0, 0, 1, 1000, 4000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	messagge_wating_tone  = {0, 0, 1, 1000, 1000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};


	voipCfgParam_t VoIPCfg;

	VoIPCfg.tone_of_country = TONE_EXT1; //EXT1

	rtk_SetCountry(&VoIPCfg);
	
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_DIAL,            &dial_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_RINGING,         &ring_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_BUSY,            &busy_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CONGESTION,      &congestion_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CALL_WAITING,    &call_waiting_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CUSTOM_TONE1,    &call_forwarding_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_MESSAGE_WAITING, &messagge_wating_tone);	
}

void ToneKeyGuatemala(int chid , SIGNSTATE val)
{
	switch (val)
	{
	case SIGN_KEY1:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_DIAL");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY2:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_RINGING");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_RINGING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY3:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_BUSY");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY4:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_CALL_WAITING");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CALL_WAITING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY5:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_CONGESTION");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CONGESTION, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY6:
		printf("(%d) chid=%u %s call forwarding tone use custom_tone_1\n" , __LINE__ , chid , "DSPCODEC_TONE_CUSTOM_TONE1");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CUSTOM_TONE1, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY7:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_MESSAGE_WAITING");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_MESSAGE_WAITING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_ONHOOK:
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_OnHookAction(chid);
		break;
	case SIGN_OFFHOOK:
		rtk_OffHookAction(chid);
		break;
	case SIGN_FLASHHOOK:
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_NONE:
		break;
	case SIGN_OFFHOOK_2:
		break;
	default:
		printf("unknown(%d)\n", val);
		break;
	}
}

//Romania
void SetCustomToneRomania(void)
{
	// Romania tone
	/*
					         Freq    on    off    on    off
	dial tone                450   continue
	ring tone                450    1.5    3.5 
	busy tone                450    0.168  0.168
	congestion tone          450    0.5    0.5
	waiting call tone        450    0.2    0.2    0.2   9400
	special dial tone        450    0.4    0.04
	*/
	
	st_ToneCfgParam dial_tone             = {0, 0, 1, 2000,    0,   0,     0, 0, 0, 0, 0, 0, 1, 450,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	ring_tone             = {0, 0, 1, 1500, 3500,   0,     0, 0, 0, 0, 0, 0, 1, 450,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	busy_tone             = {0, 0, 1,  168,  168,   0,     0, 0, 0, 0, 0, 0, 1, 450,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	congestion_tone       = {0, 0, 1,  500,  500,   0,     0, 0, 0, 0, 0, 0, 1, 450,   0, 0, 0,  7,  0, 0, 0};
	st_ToneCfgParam	call_waiting_tone     = {0, 0, 2,  200,  200, 200,  9400, 0, 0, 0, 0, 0, 1, 450, 450, 0, 0,  7,  7, 0, 0};  //2 cadances
	st_ToneCfgParam	special_dial_tone     = {0, 0, 1,  400,  400,   0,     0, 0, 0, 0, 0, 0, 1, 450,   0, 0, 0,  7,  0, 0, 0};


	voipCfgParam_t VoIPCfg;

	VoIPCfg.tone_of_country = TONE_EXT1; //EXT1

	rtk_SetCountry(&VoIPCfg);
	
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_DIAL,            &dial_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_RINGING,         &ring_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_BUSY,            &busy_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CONGESTION,      &congestion_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CALL_WAITING,    &call_waiting_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CUSTOM_TONE1,    &special_dial_tone);
}


void ToneKeyRomania(int chid , SIGNSTATE val)
{
	switch (val)
	{
	case SIGN_KEY1:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_DIAL");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_DIAL, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY2:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_RINGING");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_RINGING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY3:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_BUSY");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_BUSY, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY4:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_CALL_WAITING");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CALL_WAITING, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY5:
		printf("(%d) chid=%u %s\n" , __LINE__ , chid , "DSPCODEC_TONE_CONGESTION");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CONGESTION, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_KEY6:
		printf("(%d) chid=%u %s special_dial_tone use custom_tone_1\n" , __LINE__ , chid , "DSPCODEC_TONE_CUSTOM_TONE1");
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_CUSTOM_TONE1, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_ONHOOK:
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 0, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_OnHookAction(chid);
		break;
	case SIGN_OFFHOOK:
		rtk_OffHookAction(chid);
		break;
	case SIGN_FLASHHOOK:
		rtk_SetPlayTone(chid, 0, DSPCODEC_TONE_HOLD, 1, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case SIGN_NONE:
		break;
	case SIGN_OFFHOOK_2:
		break;
	default:
		printf("unknown(%d)\n", val);
		break;
	}
}
