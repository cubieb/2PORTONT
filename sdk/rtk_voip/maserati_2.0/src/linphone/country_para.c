#include "uglib.h"
#include "voip_manager.h"
#include "rcm_customize.h"


void customer_country_guatemala(){

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

static st_ToneCfgParam dial_tone             = {0, 0, 1, 2000,    0,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
static st_ToneCfgParam ring_tone             = {0, 0, 1, 1000, 4000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
static st_ToneCfgParam busy_tone             = {0, 0, 1,  500,  500,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
static st_ToneCfgParam congestion_tone       = {0, 0, 1,  200,  200,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
static st_ToneCfgParam call_waiting_tone     = {0, 0, 2,  200,  200, 200, 10000, 0, 0, 0, 0, 0, 1, 425, 425, 0, 0,  7,  7, 0, 0};  //2 cadances
static st_ToneCfgParam call_forwarding_tone  = {0, 0, 1, 1000, 4000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
static st_ToneCfgParam messagge_wating_tone  = {0, 0, 1, 1000, 1000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	static int country_slic_impedance=-1; //use default 600 ohms


	if(country_slic_impedance >0)
		rtk_SetCountryImpedance(country_slic_impedance);

	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_DIAL,			&dial_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_RINGING, 		&ring_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_BUSY,			&busy_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CONGESTION,		&congestion_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CALL_WAITING,	&call_waiting_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CUSTOM_TONE1,	&call_forwarding_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_MESSAGE_WAITING, &messagge_wating_tone); 


}

void customer_country_brazil(){
	//BRAZIL tone
	/*
							 Freq	 on    off	  on	off
	dial tone				 425   continue
	ring tone				 425	  1 	 4 
	busy tone				 425	0.25	0.25
	congestion tone 		 425	0.75	0.25  0.25	0.25
	waiting call tone		 425	0.05	1	
	call forwarding tone	 425	  1 	 4
	tone message waiting	 425	  1 	 1
	*/
	
	
	static st_ToneCfgParam dial_tone			 = {0, 0, 1, 2000,	  0,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam ring_tone			 = {0, 0, 1, 1000, 4000,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam busy_tone			 = {0, 0, 1,  250,	250,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam congestion_tone		 = {0, 0, 2,  750,	250,  250,	250, 0, 0, 0, 0, 0, 1, 425, 425, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam call_waiting_tone	 = {0, 0, 1,  50,  1000,	0,	  0, 0, 0, 0, 0, 0, 1, 425, 0, 0, 0,  7,  7, 0, 0};
	static st_ToneCfgParam call_forwarding_tone  = {0, 0, 1, 1000, 4000,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam messagge_wating_tone  = {0, 0, 1, 1000, 1000,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static int country_slic_impedance=-1; //use default 600 ohms
	
	if(country_slic_impedance >0)
			rtk_SetCountryImpedance(country_slic_impedance);
		
		rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_DIAL,			&dial_tone);
		rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_RINGING, 		&ring_tone);
		rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_BUSY,			&busy_tone);
		rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CONGESTION,		&congestion_tone);
		rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CALL_WAITING,	&call_waiting_tone);
		rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CUSTOM_TONE1,	&call_forwarding_tone);
		rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_MESSAGE_WAITING, &messagge_wating_tone); 

	
}

void customer_country_russian(){
	/*
	russian
	dial tone                425   continue
	ring tone                425      1      4
	busy tone 		 415     0.35   0.35 

	other need modify

	*/
	static st_ToneCfgParam dial_tone             = {0, 0, 1, 2000,    0,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam ring_tone             = {0, 0, 1, 1000, 4000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam busy_tone             = {0, 0, 1,  350,  350,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam congestion_tone       = {0, 0, 1,  200,  200,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam call_waiting_tone     = {0, 0, 2,  200,  200, 200, 10000, 0, 0, 0, 0, 0, 1, 425, 425, 0, 0,  7,  7, 0, 0};  //2 cadances
	static st_ToneCfgParam call_forwarding_tone  = {0, 0, 1, 1000, 4000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam messagge_wating_tone  = {0, 0, 1, 1000, 1000,   0,     0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,  7,  0, 0, 0};
	static int country_slic_impedance=-1;


	if(country_slic_impedance >0)
			rtk_SetCountryImpedance(country_slic_impedance);
		
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_DIAL,			&dial_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_RINGING, 		&ring_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_BUSY,			&busy_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CONGESTION,		&congestion_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CALL_WAITING,	&call_waiting_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CUSTOM_TONE1,	&call_forwarding_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_MESSAGE_WAITING, &messagge_wating_tone); 
}


void customer_country_spain(){
	/*
	SPAIN
	dial tone  , 425 countinue
	ringback tone , 425 , 1.5s on , 3s 0ff
	busy tone , 425 , 0.2s on , 0.2s off
	congestion tone , 425,	0.2s on , 0.2s off, 0.2s on , 0.2s off , 0.2s on ,0.6s off
	call waiting tone , 425, 0.175s on , 0.175s off, 0.175s on ,3.5s off.
	call forward tone , 425, 1s on ,0.1 off.
	
	*/
	
	static st_ToneCfgParam dial_tone			 = {0, 0, 1, 2000,	  0,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam ring_tone			 = {0, 0, 1, 1500, 3000,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam busy_tone			 = {0, 0, 1,  200,	200,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam congestion_tone		 = {0, 0, 3,  200,	200,   200,    200, 200, 600, 0, 0, 0, 1, 425,	 0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam call_waiting_tone	 = {0, 0, 2,  175,	175, 175, 3500, 0, 0, 0, 0, 0, 1, 425, 425, 0, 0,  7,  7, 0, 0};  //2 cadances
	static st_ToneCfgParam call_forwarding_tone  = {0, 0, 1, 1000, 100,   0,	 0, 0, 0, 0, 0, 0, 1, 425,	 0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam messagge_wating_tone  = {0, 0, 1, 1000, 1000,   0,	  0, 0, 0, 0, 0, 0, 1, 425,   0, 0, 0,	7,	0, 0, 0};
	static int country_slic_impedance=9; // 270 + (750 || 150nF) 
	

	if(country_slic_impedance >0)
			rtk_SetCountryImpedance(country_slic_impedance);

	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_DIAL,            &dial_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_RINGING,         &ring_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_BUSY,            &busy_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CONGESTION,      &congestion_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CALL_WAITING,    &call_waiting_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CUSTOM_TONE1,    &call_forwarding_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_MESSAGE_WAITING, &messagge_wating_tone);	


}



void customer_country_NEW_Zealand(){
	/*
	New Zealand
	dial tone  , 400 countinue
	ringback tone , 400+450 ,  0.4on 0.4 off,  0.4on ,2.0 off
	busy tone , 400 , 0.5s on , 0.5s off
	congestion tone , 400,	0.25s on , 0.25s off
	call waiting tone , 400, 0.2s on , 3s off, 0.2 on ,3s off. 0.2s on , 3s off

	*/
	
	static st_ToneCfgParam dial_tone			 = {0, 0, 1, 2000,	  0,   0,	  0, 0, 0, 0, 0, 0, 1, 400,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam ring_tone			 = {0, 0, 2, 400, 200,   400,	  2000, 0, 0, 0, 0, 0, 2, 400,   450, 0, 0,	7,	7, 0, 0};
	static st_ToneCfgParam busy_tone			 = {0, 0, 1,  500,	500,   0,	  0, 0, 0, 0, 0, 0, 1, 400,   0, 0, 0,	7,	0, 0, 0};
	static st_ToneCfgParam congestion_tone		 = {0, 0, 1,  250,	250,   0,    0, 0,0, 0, 0, 0, 1, 400,	 0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam call_waiting_tone	 = {0, 0, 3,  200,	3000, 200, 3000, 200, 3000, 0, 0, 0, 1, 400, 0, 0, 0,  7,  0, 0, 0}; 
	static st_ToneCfgParam call_forwarding_tone  = {0, 0, 1, 1000, 100,   0,	 0, 0, 0, 0, 0, 0, 1, 400,	 0, 0, 0,  7,  0, 0, 0};
	static st_ToneCfgParam messagge_wating_tone  = {0, 0, 1, 1000, 1000,   0,	  0, 0, 0, 0, 0, 0, 1, 400,   0, 0, 0,	7,	0, 0, 0};
	static int country_slic_impedance=20;//  370R+620R//310nF 
	
	if(country_slic_impedance >0)
			rtk_SetCountryImpedance(country_slic_impedance);

	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_DIAL,            &dial_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_RINGING,         &ring_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_BUSY,            &busy_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CONGESTION,      &congestion_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CALL_WAITING,    &call_waiting_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_CUSTOM_TONE1,    &call_forwarding_tone);
	rtk_SetUpdateTone(TONE_EXT1, DSPCODEC_TONE_MESSAGE_WAITING, &messagge_wating_tone);	


}



int rcm_country_para_update(voipCfgParam_t * voip_ptr)
{
	int ret =0;
	
	//always set use EXT1 country
	rtk_SetCountryTone(TONE_EXT1);
	// update tone
	g_message("rcm_country_para_update ,index is %d\n",voip_ptr->tone_of_country);
	//need also check web_voip_tone.c
	switch(voip_ptr->tone_of_country){
//		case 13: //guatemala
//			customer_country_guatemala();
//			break;
//		case 14: //brazil
//			customer_country_brazil();
//			break;
		case 13://russian
			customer_country_russian();
			break;
		case 14://spain
			customer_country_spain();
			break;
	
		default:
			rtk_SetCountryTone(voip_ptr->tone_of_country);
			g_message("rcm_country_para_update use other country ,index is %d\n",voip_ptr->tone_of_country);	
			break;
	}
		
	return ret;
}

