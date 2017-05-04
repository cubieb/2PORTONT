/* this file only use for Simulator */
#include "rcm_tapi.h"
//[SD6, darren_shiue, rcm simulation
#include "rcm_sim_defs.h"
#ifdef RCM_SIMULATION
#include "../cmsim/rcm_sim_data.h"
#include "../cmsim/rcm_sim_common.h"
#include "../cmsim/rcm_sim_proc.h"
#include "../cmsim/rcm_sim_occur.h"

//	extern variable
extern sim_fxs_t	FXS[MAX_FXS];
#endif



#ifdef RCM_SIMULATION
//#if  0
int32 rcm_tapi_InitDSP(int ch)
{
	
		rcm_sim_polling();
		rcm_sim_log("\n");	 
	return rtk_InitDSP(ch);

}

#ifdef CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT

int32 rcm_tapi_SetDectPower( uint32 power )
{
	int32 ret=0;

	rcm_sim_polling();

	ret=rtk_SetDectPower(power);
	
	rcm_sim_log( "power=%d\n" ,power );   

	return 0;
}




int32 rcm_tapi_GetDectPower( void )
{
	int32 ret=0;
	rcm_sim_polling();

	ret=rtk_GetDectPower();
	
	rcm_sim_log("\n");   

	return ret;
}



int32 rcm_tapi_GetDectPage( void )
{
	int32 ret=0;
	
	rcm_sim_polling();

	ret=rtk_GetDectPage();

	rcm_sim_log("\n");   
	return ret;
}


int32 rcm_tapi_GetDectButtonEvent( void )
{
	int32 ret=0;

	rcm_sim_polling();

	ret=rtk_GetDectButtonEvent();

	rcm_sim_log("\n");   
	
	return ret;
}


int32 rtk_SetDectLED( int chid, char state )
{
	int32 ret=0;

	ret=rtk_SetDectLED(chid,state);
	

	rcm_sim_log("\n");   

	return ret;
}
#endif


int32 rcm_tapi_SetRtpConfig(rtp_config_t *cfg)
{
	int32 ret=0;

	rcm_sim_polling();
  

	ret=rtk_SetRtpConfig(cfg);


rcm_sim_log("chid=%d sid=%d src_ip=%x src_port=%d dst_ip=%x dsp_port=%d\n" , 
	cfg->chid, cfg->sid , cfg->remIp ,	cfg->remPort , cfg->extIp , cfg->extPort); 


	return ret;
	
}




int32 rcm_tapi_SetRtcpConfig(rtp_config_t *cfg, unsigned short rtcp_tx_interval)	//thlin+ for Rtcp
{
	int32 ret=0;

		rcm_sim_polling();

	ret= rtk_SetRtcpConfig(cfg, rtcp_tx_interval);



	rcm_sim_log("chid=%d sid=%d src_ip=%x src_port=%d dst_ip=%x dsp_port=%d\n" , 
		cfg->chid, cfg->sid , cfg->remIp ,  cfg->remPort+1 , cfg->extIp , cfg->extPort+1);   

	rcm_sim_log("chid=%d rtcp_tx_interval=%d\n" , cfg->chid, rtcp_tx_interval);   

	return ret;


}


int32 rcm_tapi_GetRtcpLogger( uint32 chid, uint32 sid, TstVoipRtcpLogger *logger )
{

	return rtk_GetRtcpLogger(chid,sid,logger);

}


int32 rcm_tapi_SetRtpPayloadType(payloadtype_config_t *cfg)
{
	int32 ret=0;
	
	rcm_sim_polling();
	
	ret= rtk_SetRtpPayloadType(cfg);

    rcm_insert_codec_event(cfg->local_pt, cfg->chid);

	rcm_sim_log("chid=%d sid=%d local_pt=%d remote_pt=%d\n", cfg->chid, cfg->sid , cfg->local_pt ,  cfg->remote_pt);   

	return ret;

}


int32 rcm_tapi_SetRtpSessionState(uint32 chid, uint32 sid, RtpSessionState state)
{
	int32 ret=0;
	
	rcm_sim_polling();

	ret=rtk_SetRtpSessionState(chid,sid,state);

	rcm_sim_log("chid=%d sid=%d state=%d\n", chid, sid , state  );   
	if(rtp_session_inactive == state ){
		rcm_sim_log("close RTP\n"); 
#ifdef SUPPORT_RTCP
		rcm_sim_log("close RTCP\n");   
#endif	
		rcm_sim_log("close CODEC\n");	
	}			


	return ret;

}


int32 rcm_tapi_SetThresholdVadCng( int32 chid, int32 mid, int32 nThresVAD, int32 nThresCNG, int32 nModeCNG, int32 nLevelCNG, int32 nGainCNG )
{
	return rtk_SetThresholdVadCng(chid,mid,nThresVAD,nThresCNG,nModeCNG,nLevelCNG,nGainCNG);	
}


int32 rcm_tapi_DisableRingFXS(int bDisable)
{
	
		rcm_sim_polling();


	return rtk_DisableRingFXS(bDisable);

}


int32 rcm_tapi_SetRingFXS(uint32 chid, uint32 bRinging)
{
	int32 ret;
	
	rcm_sim_polling();
	
	ret=rtk_SetRingFXS(chid,bRinging);


		rcm_sim_log("chid=%d ring_set=%d\n" , chid , bRinging);

	return ret;


}


int32 rcm_tapi_SetPlayTone(uint32 chid, uint32 sid, DSPCODEC_TONE nTone, uint32 bFlag,
	DSPCODEC_TONEDIRECTION Path)
{
	int32 ret=0;
	
		rcm_sim_polling();



	ret= rtk_SetPlayTone(chid,sid,nTone,bFlag,Path);


	if(bFlag){
		switch( nTone ){
	        case DSPCODEC_TONE_DIAL:
	        case DSPCODEC_TONE_STUTTERDIAL:
	        case DSPCODEC_TONE_MESSAGE_WAITING:
			case DSPCODEC_TONE_CONFIRMATION:
			case DSPCODEC_TONE_RINGING:
	        case DSPCODEC_TONE_BUSY:
			case DSPCODEC_TONE_CONGESTION:
			case DSPCODEC_TONE_ROH:
			case DSPCODEC_TONE_DOUBLE_RING:
			case DSPCODEC_TONE_SIT_NOCIRCUIT:
			case DSPCODEC_TONE_SIT_INTERCEPT:
			case DSPCODEC_TONE_SIT_VACANT:
			case DSPCODEC_TONE_SIT_REORDER:
			case DSPCODEC_TONE_CALLING_CARD_WITHEVENT:
			case DSPCODEC_TONE_CALLING_CARD:
			case DSPCODEC_TONE_CALL_WAITING:
			case DSPCODEC_TONE_CALL_WAITING_2:
			case DSPCODEC_TONE_CALL_WAITING_3:
			case DSPCODEC_TONE_CALL_WAITING_4:
			case DSPCODEC_TONE_INGRESS_RINGBACK:
			case DSPCODEC_TONE_OFFHOOKWARNING:
			case DSPCODEC_TONE_HOLD:
				/*insert_exp_event(RSE_TONE_RINGING, chid);
				rcm_sim_log("insert RSE_TONE_RINGING\n");
				rcm_dp("insert RSE_TONE_RINGING\n");*/
				rcm_insert_tone_event(nTone, chid);
			default: break;
		}
	}
	rcm_sim_log("chid=%d sid=%d Tone=%d Flag=%d\n" , chid , sid , nTone , bFlag);



	return ret;

}

int32 rcm_tapi_VoIP_resource_check(uint32 chid, int payload_type)
{
	
		rcm_sim_polling();
		rcm_sim_log("TBD for resource check\n");
	return 1;



}


int32 rcm_tapi_Onhook_Reinit(uint32 chid)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Onhook_Reinit(chid);


	rcm_sim_log("chid=%d\n" , chid);

	return ret;


}



int32 rcm_tapi_GetDectEvent(uint32 chid, SIGNSTATE *pval)
{
	int32 ret=0;
	
		rcm_sim_polling();

	ret=  rtk_GetDectEvent(chid,pval);


	rcm_sim_log("rcm_tapi_GetDectEvent chid=%d\n" , chid);

	return ret;

}


int32 rcm_tapi_GetFxsEvent(uint32 chid, SIGNSTATE *pval)
{


	rcm_sim_polling();
	*pval = FXS[chid].rtk_GetFxsEvent.pval;
	if(FXS[chid].rtk_GetFxsEvent.pval){
		rcm_sim_log( "chid=%d event=%d\n" ,chid , FXS[chid].rtk_GetFxsEvent.pval );
	}
	FXS[chid].rtk_GetFxsEvent.pval = 0;

	return 0;


}


int32 rcm_tapi_GetFxoEvent(uint32 chid, SIGNSTATE *pval)
{

	rcm_sim_polling();
	*pval = FXS[chid].rtk_GetFxoEvent.pval;
	if(FXS[chid].rtk_GetFxoEvent.pval){
		rcm_sim_log("chid=%d event=%d\n" ,chid , FXS[chid].rtk_GetFxoEvent.pval);
	}
	FXS[chid].rtk_GetFxoEvent.pval = 0;
	return 0;
	

}



int32 rcm_tapi_GetRealFxoEvent(uint32 chid, FXOEVENT *pval)
{
	int32 ret=0;
	
		rcm_sim_polling();



	ret= rtk_GetRealFxoEvent(chid,pval);

	rcm_sim_log( "chid=%d event=%d\n" ,chid , *pval );

	return ret;

}



int32 rcm_tapi_SetTranSessionID(uint32 chid, uint32 sid)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret=rtk_SetTranSessionID(chid,sid);


		rcm_sim_log( "chid=%d sid=%d enable=%d\n" , chid , sid , 1 );   

	return ret;



}

int32 rcm_tapi_GetTranSessionID(uint32 chid, uint32* psid)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret= rtk_GetTranSessionID(chid,psid);

		rcm_sim_log( "rcm_tapi_GetTranSessionID chid=%d\n" , chid );   


	return ret;
}


int32 rcm_tapi_SetConference(TstVoipMgr3WayCfg *stVoipMgr3WayCfg)
{

		rcm_sim_polling();
		rcm_sim_log("\n");	 


	return rtk_SetConference(stVoipMgr3WayCfg);
}


int32 rcm_tapi_SetDTMFMODE(uint32 chid, uint32 mode)
{
	int32 ret=0;
	rcm_sim_polling();
	ret= rtk_SetDTMFMODE(chid,mode);
	rcm_sim_log( "chid=%d mode=%d\n" , chid , mode );
	return ret;

}

int32 rcm_tapi_Set_echoTail(uint32 chid, uint32 echo_tail, uint32 nlp, uint32 at_stepsize, uint32 rt_stepsize, uint32 cng_flag)
{
	int32 ret=0;
	rcm_sim_polling();
	ret= rtk_Set_echoTail(chid,echo_tail,nlp,at_stepsize,rt_stepsize,cng_flag);
	rcm_sim_log( "chid=%d echo_tail=%d \n" , chid , echo_tail );   
	return ret;
}



int32 rcm_tapi_Set_G168_LEC(uint32 chid, uint32 support_lec)	/* This function can turn on/off G168 LEC. */
{
	int32 ret=0;

			rcm_sim_polling();
		


	ret= rtk_Set_G168_LEC(chid,support_lec);
	rcm_sim_log( "chid=%d support_lec=%d\n" , chid , support_lec );   
	return ret;

}



int32 rcm_tapi_Set_GET_EC_DEBUG(TstVoipEcDebug* pstVoipEcDebug)
{
	int32 ret=0;

	rcm_sim_polling();
	rcm_sim_log( "\n" );   

	ret= rtk_Set_GET_EC_DEBUG(pstVoipEcDebug);
	return ret;
}


int32 rcm_tapi_Set_SLIC_Ring_Cadence(uint32 chid, uint16 cad_on_msec, uint16 cad_off_msec)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_SLIC_Ring_Cadence(chid,cad_on_msec,cad_off_msec);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}



int32 rcm_tapi_Set_SLIC_Ring_Freq_Amp(uint32 chid, uint8 preset)
{
	int32 ret=0;
	
		rcm_sim_polling();

	ret=rtk_Set_SLIC_Ring_Freq_Amp(chid,preset);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}


int32 rcm_tapi_Set_SLIC_Line_Voltage(uint32 chid, uint8 flag)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Set_SLIC_Line_Voltage(chid,flag);


	rcm_sim_log( "chid=%d\n" , chid );   


}



int32 rcm_tapi_Gen_SLIC_CPC(uint32 chid, uint32 cpc_ms)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret= rtk_Gen_SLIC_CPC(chid,cpc_ms);

		rcm_sim_log( "chid=%d\n", chid );   

	return ret;
}


int32 rcm_tapi_Set_FXO_Ring_Detection(uint16 ring_on_msec, uint16 first_ringoff_msec, uint16 ring_off_msec)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret=rtk_Set_FXO_Ring_Detection(ring_on_msec,first_ringoff_msec,ring_off_msec);

		rcm_sim_log( "\n" );   

	return ret;

}



int32 rcm_tapi_Set_DAA_Tx_Gain(uint32 gain)
{
	int32 ret=0;
	
			rcm_sim_polling();

	ret= rtk_Set_DAA_Tx_Gain(gain);

	rcm_sim_log( "gain=%d\n" , gain );   

	return ret;
}


int32 rcm_tapi_Set_DAA_Rx_Gain(uint32 gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_DAA_Rx_Gain(gain);

		rcm_sim_log( "gain=%d\n" , gain );   

	return ret;

}


int32 rcm_tapi_Set_Flash_Hook_Time(uint32 chid, uint32 min_time, uint32 time)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Set_Flash_Hook_Time(chid,min_time,time);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}


int32 rcm_tapi_SetRFC2833SendByAP(uint32 chid, uint32 config)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_SetRFC2833SendByAP(chid,config);


	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}


int32 rcm_tapi_SetRFC2833TxConfig(uint32 chid, uint32 volume, uint32 tx_mode) //tx_mode: 0: DSP mode, 1: AP mode
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_SetRFC2833TxConfig(chid,volume,tx_mode);

	rcm_sim_log( "chid=%d volume=%d tx_mode=%d\n" , chid , volume , tx_mode);   

	return ret;

}


int32 rcm_tapi_SetRTPRFC2833(uint32 chid, uint32 sid, uint32 event, unsigned int duration)
{
	int32 ret;
	
		rcm_sim_polling();


	ret= rtk_SetRTPRFC2833(chid,sid,event,duration);

	rcm_sim_log( "chid=%d sid=%d digit=%d duration=%d\n" , chid , sid , event , duration );   

	return ret;

}

int32 rcm_tapi_LimitMaxRfc2833DtmfDuration(uint32 chid, uint32 duration_in_ms, uint8 bEnable)
{

		rcm_sim_polling();
		rcm_sim_log( "chid=%d\n" , chid );	 


	return rtk_LimitMaxRfc2833DtmfDuration(chid,duration_in_ms,bEnable);
}




int32 rcm_tapi_GetRfc2833RxEvent(uint32 chid, uint32 mid, RFC2833_EVENT *pent)
{
    rcm_sim_polling();
    *pent = FXS[chid].rtk_GetRfc2833RxEvent.pent;
    if(*pent >= ENT_DTMF_0 && *pent <= ENT_DTMF_FLASH){
	    rcm_sim_log( "chid=%d dtmf event=%d\n" ,chid , FXS[chid].rtk_GetRfc2833RxEvent.pent );
        rcm_dp( "chid=%d rfc2833 dtmf event=%d\n" ,chid , FXS[chid].rtk_GetRfc2833RxEvent.pent );
    }
    FXS[chid].rtk_GetRfc2833RxEvent.pent = -1;
	
	return 0;
}

int32 rcm_tapi_SetFaxModemRfc2833(uint32 chid, uint32 relay_flag, uint32 removal_flag, uint32 tone_gen_flag)
{

	return rtk_SetFaxModemRfc2833(chid,relay_flag,removal_flag,tone_gen_flag);
}




int32 rcm_tapi_GetFaxModemEvent(uint32 chid, uint32 *pval, uint32 flush)
{

	rcm_sim_polling();
	*pval = FXS[chid].rtk_GetFaxModemEvent.pval;
	if(*pval){
		rcm_sim_log("FaxModemEvent pavl=%d\n" , *pval);
        rcm_dp("FaxModemEvent pavl=%d\n" , *pval);
    }
	FXS[chid].rtk_GetFaxModemEvent.pval = 0;

	return 0;//stVoipCfg.ret_val;
}


int32 rcm_tapi_GetFaxEndDetect(uint32 chid, uint32 *pval)
{
	rcm_sim_polling();
	*pval = FXS[chid].rtk_GetFaxEndDetect.pval;
	if(*pval){
		rcm_sim_log("FaxEnd pavl=%d\n" , *pval);
        rcm_dp("FaxEnd pavl=%d\n" , *pval);
    }
	FXS[chid].rtk_GetFaxEndDetect.pval = 0;
    
	return 0;//stVoipCfg.ret_val;
}



int32 rcm_tapi_GetFaxDisDetect(uint32 chid, uint32 *pval)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret = rtk_GetFaxDisDetect(chid,pval);


	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}


int32 rcm_tapi_GetFaxDisTxDetect(uint32 chid, uint32 *pval)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret= rtk_GetFaxDisTxDetect(chid,pval);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;
}


int32 rcm_tapi_GetFaxDisRxDetect(uint32 chid, uint32 *pval)
{
	int32 ret=0;
	
			rcm_sim_polling();

	ret= rtk_GetFaxDisRxDetect(chid,pval);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}



int32 rcm_tapi_GetFxoLineVoltage(uint32 chid, uint32 *pval)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret= rtk_GetFxoLineVoltage(chid,pval);

	rcm_sim_log( "chid=%d\n" , chid );   


	return ret;
}


//internal usage
int32 rcm_tapi_eanblePCM(uint32 chid, uint32 val)
{
	int32 ret=0;
	
		rcm_sim_polling();



	ret= rtk_eanblePCM(chid,val);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}




int32 rcm_tapi_Gen_Dtmf_CID(uint32 chid, char *str_cid)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Gen_Dtmf_CID(chid,str_cid);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;
}



int32 rcm_tapi_Gen_FSK_CID(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Gen_FSK_CID(chid,str_cid,str_date,str_cid_name,mode);

		rcm_sim_log( "chid=%d\n" , chid );	 


	return ret;
}


int32 rcm_tapi_Gen_MDMF_FSK_CID(uint32 chid, TstFskClid* pClid, uint32 num_clid_element)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Gen_MDMF_FSK_CID(chid,pClid,num_clid_element);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}


int32 rcm_tapi_Gen_CID_And_FXS_Ring(uint32 chid, char cid_mode, char *str_cid, char *str_date, char *str_cid_name, char fsk_type, uint32 bRinging)
{

		rcm_sim_polling();
		rcm_insert_tone_event(DSPCODEC_TONE_RING, chid);
		if(str_cid[0] != 0){
			rcm_sim_log("Incoming Call with Caller ID %s\n", str_cid);
			rcm_dp("Incoming Call with Caller ID %s\n", str_cid);
		}
		else{
			rcm_sim_log("Incoming Call without Caller ID\n");
			rcm_dp("Incoming Call without Caller ID \n");
		}


	return rtk_Gen_CID_And_FXS_Ring(chid,cid_mode,str_cid,str_date,str_cid_name,fsk_type,bRinging);

}


int32 rcm_tapi_Gen_FSK_VMWI(uint32 chid, char *state, char mode)  /* state:	point to the address of value to set VMWI state. 0 : off; 1 : on*/
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Gen_FSK_VMWI(chid,state,mode);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}



int32 rcm_tapi_Set_FSK_Area(uint32 chid, char area)   /* area -> 0:Bellcore 1:ETSI 2:BT 3:NTT */
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_FSK_Area(chid,area);


	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}


int32 rcm_tapi_Set_FSK_CLID_Para(TstVoipFskPara* para)
{
	int32 ret=0;
		rcm_sim_polling();


	ret=rtk_Set_FSK_CLID_Para(para);


	rcm_sim_log( "\n" );   

	return ret;

}


int32 rcm_tapi_Get_FSK_CLID_Para(TstVoipFskPara* para)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Get_FSK_CLID_Para(para);

	rcm_sim_log( "\n" );   

	return ret;
}


int32 rcm_tapi_Hold_Rtp(uint32 chid, uint32 sid, uint32 enable)
{
	int32 ret=0;
	
	rcm_sim_polling();

	ret= rtk_Hold_Rtp(chid,sid,enable);

	rcm_sim_log( "chid=%d sid=%d enable=%d\n" , chid , sid , !enable );   

	return ret;

}


int32 rcm_tapi_enable_pcm(uint32 chid, int32 bEnable)
{
	int32 ret=0;
	
		rcm_sim_polling();

	ret= rtk_enable_pcm(chid,bEnable);

	rcm_sim_log( "chid=%d enable=%d\n" , chid , bEnable );   

	return ret;

}


int32 rcm_tapi_Set_DTMF_CFG(uint32 chid, int32 bEnable, uint32 dir)
{
	return rtk_Set_DTMF_CFG(chid,bEnable,dir);
}



uint32 rcm_tapi_Get_SLIC_Reg_Val(uint32 chid, uint32 reg, uint8 *regdata)
{
	uint32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Get_SLIC_Reg_Val(chid,reg,regdata);

	rcm_sim_log( "chid=%d reg=%d\n" , chid , reg );   

	return ret;



	
}



int rcm_tapi_Get_DAA_Used_By_Which_SLIC(uint32 chid)
{
	int ret=0;
	
		rcm_sim_polling();

	ret= rtk_Get_DAA_Used_By_Which_SLIC(chid);


	rcm_sim_log( "chid=%d \n" , chid );   

	return ret;

}



int rcm_tapi_DAA_on_hook(uint32 chid)// for virtual DAA on-hook(channel ID is FXS channel)
{
	int ret=0;
	
		rcm_sim_polling();

	ret= rtk_DAA_on_hook(chid);

	rcm_sim_log( "chid=%d \n" , chid );   

	return ret;

}


int rcm_tapi_DAA_off_hook(uint32 chid)// for virtual DAA off-hook(channel ID is FXS channel)
{

	int enable;
	rcm_sim_polling();
	if ((g_VoIP_Feature & DAA_TYPE_MASK) != NO_DAA )
	{
		enable = FXS[chid].rtk_DAA_off_hook.enable;
		rcm_sim_log( "enable=%d\n" ,  FXS[chid].rtk_DAA_off_hook.enable );
		FXS[chid].rtk_DAA_off_hook.enable = 0;
		return enable;
	}
	else
	{	
		printf("API rtk_DAA_off_hook usage error.\n");
		return 0xff;
	}

}


int rcm_tapi_DAA_ring(uint32 chid)
{
	
		rcm_sim_polling();
		rcm_sim_log( "chid=%d \n" , chid );   


	return rtk_DAA_ring(chid);
}


int32 rcm_tapi_Set_Country(voipCfgParam_t *voip_ptr)
{
	int32 ret=0;

	char country;


  	country = voip_ptr->tone_of_country;
	rcm_sim_polling();


	ret= rtk_Set_Country(voip_ptr);

	rcm_sim_log( "country=%d \n" , country );   

	return ret;

}


int32 rcm_tapi_Set_Country_Impedance(_COUNTRY_ country)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_Country_Impedance(country);

	rcm_sim_log( "country=%d \n" , country );   

	return ret;

}


int32 rcm_tapi_Set_Country_Tone(_COUNTRY_ country)
{
	int32 ret=0;
	
		rcm_sim_polling();

	ret=rtk_Set_Country_Tone(country);
	

	rcm_sim_log( "country=%d \n" , country );   


}


int32 rcm_tapi_Set_Impedance(uint16 preset)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_Impedance(preset);

	rcm_sim_log( "country=%d \n" , preset );   

	return ret;

}



int32 rcm_tapi_Set_Dis_Tone_Para(voipCfgParam_t *voip_ptr)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_Dis_Tone_Para(voip_ptr);

	rcm_sim_log( "\n" );   

	return ret;

}



int32 rcm_tapi_Set_Custom_Tone(uint8 custom, st_ToneCfgParam *pstToneCfgParam)
{
	int32 ret=0;

	
		rcm_sim_polling();


	ret= rtk_Set_Custom_Tone(custom,pstToneCfgParam);


	rcm_sim_log( "custom=%d\n" , custom );   

	return ret;

}



int32 rcm_tapi_Use_Custom_Tone(uint8 dial, uint8 ringback, uint8 busy, uint8 waiting)
{
	
		rcm_sim_polling();


	return rtk_Use_Custom_Tone(dial,ringback,busy,waiting);
}


int rcm_tapi_Set_SLIC_Reg_Val(int chid, int reg, int len, char *regdata)
{
	
		rcm_sim_polling();



	return rtk_Set_SLIC_Reg_Val(chid,reg,len,regdata);
}



int rcm_tapi_reset_slic(int chid, unsigned int law)
{
	
		rcm_sim_polling();


	return rtk_reset_slic(chid,law);
}


int rcm_tapi_Set_PCM_Loop_Mode(char group, char mode, char main_ch, char mate_ch) //mode: 0- Not loop mode, 1- loop mode, 2- loop mode with VoIP
{
	
		rcm_sim_polling();


	return rtk_Set_PCM_Loop_Mode(group,mode,main_ch,mate_ch);
}



int rcm_tapi_Set_FXS_FXO_Loopback(unsigned int chid, unsigned int enable)
{
	
		rcm_sim_polling();


	return rtk_Set_FXS_FXO_Loopback(chid,enable);
}


int rcm_tapi_Set_FXS_OnHook_Trans_PCM_ON(unsigned int chid)
{
	
		rcm_sim_polling();


	return rtk_Set_FXS_OnHook_Trans_PCM_ON(chid);
}

int rcm_tapi_debug_with_watchdog(int dbg_flag, int watchdog)
{
	
		rcm_sim_polling();


	return rtk_debug_with_watchdog(dbg_flag,watchdog);
}

int rcm_tapi_debug(int dbg_flag)
{
	return rtk_debug(dbg_flag);

}


#ifdef CONFIG_RTK_VOIP_WAN_VLAN

int rcm_tapi_switch_wan_3_vlan(voipCfgParam_t *voip_ptr)
{

	return rtk_switch_wan_3_vlan(voip_ptr);
}


int rcm_tapi_switch_wan_2_vlan(voipCfgParam_t *voip_ptr)
{

	return rtk_switch_wan_2_vlan(voip_ptr);
}


int rcm_tapi_switch_wan_vlan(voipCfgParam_t *voip_ptr)
{

	return rtk_switch_wan_vlan(voip_ptr);
}

#endif // CONFIG_RTK_VOIP_WAN_VLAN



#ifdef CONFIG_RTK_VOIP_IVR
int rcm_tapi_IvrStartPlaying( unsigned int chid,unsigned int sid, IvrPlayDir_t dir, unsigned char *pszText2Speech )
{
	return rtk_IvrStartPlaying(chid,sid,dir,pszText2Speech);
}

int rcm_tapi_IvrStartPlayG72363( unsigned int chid, unsigned int sid, unsigned int nFrameCount, const unsigned char *pData )
{
	return rtk_IvrStartPlayG72363(chid, sid,nFrameCount,pData);
}

int rcm_tapi_IvrStartPlayG729( unsigned int chid,unsigned int sid, unsigned int nFrameCount, const unsigned char *pData )
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_IvrStartPlayG729(chid,sid,nFrameCount,pData);

	rcm_sim_log( "chid=%d\n", chid );   
	return ret;


}


int rcm_tapi_IvrPollPlaying( unsigned int chid ,int sid)
{
	
	int bPlaying;
	rcm_sim_polling();
	bPlaying = FXS[chid].rtk_IvrPollPlaying.bPlaying;
	rcm_sim_log( "bPlaying=%d\n", FXS[chid].rtk_IvrPollPlaying.bPlaying );
	FXS[chid].rtk_IvrPollPlaying.bPlaying=0;
	return bPlaying;
	
}


int rcm_tapi_IvrStopPlaying( unsigned int chid , unsigned int sid)
{
	int ret=0;
	
		rcm_sim_polling();


	ret=rtk_IvrStopPlaying(chid,sid);

		rcm_sim_log( "chid=%d\n", chid );	


	return ret;
}
#endif /* CONFIG_RTK_VOIP_IVR */

int rcm_tapi_sip_register(unsigned int chid, unsigned int isOK)
{
	int ret=0;	
	rcm_sim_polling();


	ret= rtk_sip_register(chid,isOK);

	rcm_sim_log( "chid=%d\n", chid );   

	return ret;


}

int32 rcm_tapi_Set_CID_DTMF_MODE(uint32 chid, char area, char cid_dtmf_mode)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_CID_DTMF_MODE(chid,area,cid_dtmf_mode);


	rcm_sim_log( "chid=%d\n", chid );   

	return ret;

}


int32 rcm_tapi_SIP_INFO_play_tone(unsigned int chid, unsigned int ssid, DSPCODEC_TONE tone, unsigned int duration)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_SIP_INFO_play_tone(chid,ssid,tone,duration);


	switch( tone ){
        case DSPCODEC_TONE_0:
        case DSPCODEC_TONE_1:   
        case DSPCODEC_TONE_2:
        case DSPCODEC_TONE_3:
        case DSPCODEC_TONE_4:
        case DSPCODEC_TONE_5:
        case DSPCODEC_TONE_6:
        case DSPCODEC_TONE_7:
        case DSPCODEC_TONE_8:
        case DSPCODEC_TONE_9:
        case DSPCODEC_TONE_STARSIGN:
        case DSPCODEC_TONE_HASHSIGN:
            rcm_insert_tone_event(tone, chid);
            default: break;
   }

	rcm_sim_log( "chid=%d ssid=%d\n", chid , ssid);   

	return ret;

}


int32 rcm_tapi_Set_SPK_AGC(uint32 chid, uint32 support_gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_SPK_AGC(chid,support_gain);

	rcm_sim_log( "chid=%d\n", chid );   

	return ret;

}

int32 rcm_tapi_Set_SPK_AGC_LVL(uint32 chid, uint32 level)
{
	int32 ret=0;
	
		rcm_sim_polling();



	ret= rtk_Set_SPK_AGC_LVL(chid,level);

	rcm_sim_log( "chid=%d level=%d\n", chid , level);   

	return ret;

}


int32 rcm_tapi_Set_SPK_AGC_GUP(uint32 chid, uint32 gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Set_SPK_AGC_GUP(chid,gain);


	rcm_sim_log( "chid=%d gain=%d\n", chid , gain );   

	return ret;

}


int32 rcm_tapi_Set_SPK_AGC_GDOWN(uint32 chid, uint32 gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_SPK_AGC_GDOWN(chid,gain);

	rcm_sim_log( "chid=%d gain=%d\n", chid , gain );   

	return ret;
}


int32 rcm_tapi_Set_MIC_AGC(uint32 chid, uint32 support_gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_MIC_AGC(chid,support_gain);

	rcm_sim_log( "chid=%d gain=%d\n", chid , support_gain );   

	return ret;


}

int32 rcm_tapi_Set_MIC_AGC_LVL(uint32 chid, uint32 level)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret=rtk_Set_MIC_AGC_LVL(chid,level);


	rcm_sim_log( "chid=%d level=%d\n", chid , level );   

	return ret;

}

int32 rcm_tapi_Set_MIC_AGC_GUP(uint32 chid, uint32 gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_MIC_AGC_GUP(chid,gain);

	rcm_sim_log( "chid=%d gain=%d\n", chid , gain );   

	return ret;

}


int32 rcm_tapi_Set_MIC_AGC_GDOWN(uint32 chid, uint32 gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_MIC_AGC_GDOWN(chid,gain);


	rcm_sim_log( "chid=%d gain=%d\n", chid , gain );   

	return ret;

}


int32 rcm_tapi_Dial_PSTN_Call_Forward(uint32 chid, uint32 sid, char *cf_no_str)
{
	int32 ret=0;
	
	rcm_sim_polling();

	
	ret= rtk_Dial_PSTN_Call_Forward(chid,sid,cf_no_str);

	rcm_sim_log( "chid=%d sid=%d\n", chid , sid );   

	return ret;


}




int32 rcm_tapi_Get_DAA_CallerID(uint32 chid, char *str_cid, char *str_date, char *str_name)
{
		TstVoipCID stCIDstr;
	
		rcm_sim_polling();

		
		strncpy( str_cid  ,  FXS[chid].rtk_Get_DAA_CallerID.str_cid  , sizeof(stCIDstr.string) );
		strncpy( str_date ,  FXS[chid].rtk_Get_DAA_CallerID.str_date , sizeof(stCIDstr.string2) );
		strncpy( str_name ,  FXS[chid].rtk_Get_DAA_CallerID.str_name , sizeof(stCIDstr.cid_name) );
		rcm_sim_log( "chid=%d str_cid=%s str_date=%s str_name=%s\n", chid , str_cid , str_date , str_name );   
	
		strcpy( FXS[chid].rtk_Get_DAA_CallerID.str_cid	, "" );
		strcpy( FXS[chid].rtk_Get_DAA_CallerID.str_date , "" );
		strcpy( FXS[chid].rtk_Get_DAA_CallerID.str_name , "" );
		return 0;

}




int32 rcm_tapi_Get_VoIP_Feature(void)
{
		TstVoipCfg stVoipCfg;
	
		rcm_sim_polling();
	
		printf( "VOIP_MGR_GET_FEATURE=%d\n", VOIP_MGR_GET_FEATURE );
		
		stVoipCfg.cfg = ( RTK_VOIP_FEATURE & 0xFFFFFFFFULL );
		stVoipCfg.enable = ( ( uint64 )RTK_VOIP_FEATURE >> 32 );	// store high 32bits in .enable 
	
		g_VoIP_Feature = stVoipCfg.cfg;
		g_VoIP_Feature |= ( uint64 )stVoipCfg.enable << 32;
	
		printf("rtk_Get_VoIP_Feature: 0x%llx \n", g_VoIP_Feature);
		

		printf( "\tRTK_VOIP_SLIC_NUM=%d\n", RTK_VOIP_SLIC_NUM( g_VoIP_Feature ) );
		printf( "\tRTK_VOIP_DAA_NUM=%d\n", RTK_VOIP_DAA_NUM( g_VoIP_Feature ) );
		printf( "\tRTK_VOIP_DECT_NUM=%d\n", RTK_VOIP_DECT_NUM( g_VoIP_Feature ) );
		printf( "\tRTK_VOIP_CH_NUM=%ud\n", RTK_VOIP_CH_NUM( g_VoIP_Feature ) );
		if( RTK_VOIP_PLATFORM_CHK_IS8672( g_VoIP_Feature ) )
			printf( "\tPLATFORM 8672\n" );
		else if( RTK_VOIP_PLATFORM_CHK_IS865xC( g_VoIP_Feature ) )
			printf( "\tPLATFORM 865xC\n" );
		else if( RTK_VOIP_PLATFORM_CHK_IS8972B( g_VoIP_Feature ) )
			printf( "\tPLATFORM 8972B\n" );
		else if( RTK_VOIP_PLATFORM_CHK_IS89xxC( g_VoIP_Feature ) )
			printf( "\tPLATFORM 89xxC\n" );
		else
			printf( "\tPLATFORM unknown: %llX\n", ((g_VoIP_Feature) & RTK_VOIP_PLATFORM_MASK) );

	

		rcm_sim_log( "\n" );   

	
		return 0;

}


int32 rcm_tapi_Set_CID_Det_Mode(uint32 chid, int auto_det, int cid_det_mode)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_CID_Det_Mode(chid,auto_det,cid_det_mode);

	rcm_sim_log( "chid=%d auto_det=%d cid_det_mode=%d\n" , chid , auto_det , cid_det_mode );   

	return ret;

}


int32 rcm_tapi_GetFskCIDState(uint32 chid, uint32 *cid_state)
{

	rcm_sim_polling();
	*cid_state = FXS[chid].rtk_GetFskCIDState.cid_state;
	rcm_sim_log( "chid=%d cid_stat=%d\n" , chid , FXS[chid].rtk_GetFskCIDState.cid_state );   
	FXS[chid].rtk_GetFskCIDState.cid_state = 0;
	return 0;//stCIDstr.ret_val;


}


int32 rcm_tapi_Set_CID_FSK_GEN_MODE(unsigned int chid, unsigned int isOK)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_CID_FSK_GEN_MODE(chid,isOK);

	rcm_sim_log( "chid=%d \n" , chid  );   

	return ret;

}

int32 rcm_tapi_Set_Voice_Gain(uint32 chid, int spk_gain, int mic_gain)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_Voice_Gain(chid,spk_gain,mic_gain);

	rcm_sim_log( "chid=%d \n" , chid  );   

	return ret;


}

int32 rcm_tapi_Set_AnswerTone_Det(uint32 chid, uint32 config)
{
	return rtk_Set_AnswerTone_Det(chid,config);
}


int32 rcm_tapi_Gen_FSK_CID_VMWI(uint32 chid, char *str_cid, char *str_date, char *str_cid_name, char mode, char msg_type)
{
	
		rcm_sim_polling();
		rcm_sim_log( "chid=%d \n" , chid  );   


	return rtk_Gen_FSK_CID_VMWI(chid,str_cid,str_date,str_cid_name,mode,msg_type);
}


int32 rcm_tapi_GetPhoneState(TstVoipCfg* pstVoipCfg)
{
	
		rcm_sim_polling();

		pstVoipCfg->cfg = FXS[pstVoipCfg->ch_id].rtk_Set_GetPhoneStat.pstVoipCfg.cfg;
		rcm_sim_log( "cfg=%d\n" , FXS[pstVoipCfg->ch_id].rtk_Set_GetPhoneStat.pstVoipCfg.cfg );   
		FXS[pstVoipCfg->ch_id].rtk_Set_GetPhoneStat.pstVoipCfg.cfg = 0;
	
		return 0;//pstVoipCfg->ret_val;

}


int32 rcm_tapi_Set_GETDATA_Mode(TstVoipdataget_o* pstVoipdataget_o)
{

		rcm_sim_polling();
		rcm_sim_log( "\n" );   


	return rtk_Set_GETDATA_Mode(pstVoipdataget_o);
}

int32 rcm_tapi_Set_Voice_Play(TstVoipdataput_o* pstVoipdataput_o)
{

		rcm_sim_polling();
		rcm_sim_log( "\n" );   


	return rtk_Set_Voice_Play(pstVoipdataput_o);
}

int32 rcm_tapi_Get_Rtp_Statistics( uint32 chid,  uint32 mid,uint32 bReset, TstRtpRtcpStatistics *pstRtpRtcpStatistics )
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_GetRtpRtcpStatistics(chid,mid,bReset,pstRtpRtcpStatistics);

	rcm_sim_log( "chid=%d bReset=%d\n" , chid , bReset );   

	return 0;

}

int32 rcm_tapi_Get_Session_Statistics( uint32 chid, uint32 sid, uint32 bReset, TstVoipSessionStatistics *pstVoipSessionStatistics )
{
	int32 ret=0;
	
		rcm_sim_polling();

	ret= rtk_Get_Session_Statistics(chid,sid,bReset,pstVoipSessionStatistics);

	rcm_sim_log( "chid=%d bReset=%d\n" , chid , bReset );   

	return ret;


}

int32 rcm_tapi_qos_set_dscp_priority(int32 dscp, int32 priority)
{
	int32 ret=0;
	
		rcm_sim_polling();

	ret= rtk_qos_set_dscp_priority(dscp,priority);

	rcm_sim_log( "dscp=%d priority=%d\n" , dscp , priority );   

	return ret;


}

int32 rcm_tapi_qos_reset_dscp_priority()
{

		rcm_sim_polling();
		rcm_sim_log( "\n" );   


	return rtk_qos_reset_dscp_priority();
}
int32 rcm_tapi_Set_Rtp_Tos(int32 rtp_tos)
{

		rcm_sim_polling();
		rcm_sim_log( "rtp_tos=%d\n" , rtp_tos );   


	return rtk_Set_Rtp_Tos(rtp_tos);
}

int32 rcm_tapi_Set_Rtp_Dscp(int32 rtp_dscp)
{

		rcm_sim_polling();
		rcm_sim_log( "rtp_dscp=%d\n" , rtp_dscp );	 


	return rtk_Set_Rtp_Dscp(rtp_dscp);
}


int32 rcm_tapi_Set_Sip_Tos(int32 sip_tos)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_Sip_Tos(sip_tos);

	rcm_sim_log( "sip_tos=%d\n" , sip_tos );   
  

	return ret;


}

int32 rcm_tapi_Set_Sip_Dscp(int32 sip_dscp)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_Sip_Dscp(sip_dscp);

	rcm_sim_log( "rcm_tapi_Set_Sip_Dscp=%d\n" , sip_dscp );   
  



}

#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
int rcm_tapi_Set_IPhone(unsigned int function_type, unsigned int reg, unsigned int value)
{

		rcm_sim_log( "\n" );   


	return rtk_Set_IPhone(function_type,reg,value);
}
#endif

/******************** New Add for AudioCodes Solution ****************/
int32 rcm_tapi_Onhook_Action(uint32 chid)
{
	int32 ret=0;
	
			rcm_sim_polling();


	ret= rtk_Onhook_Action(chid);

		rcm_sim_log( "\n" );   

	return ret;
}

int32 rcm_tapi_Offhook_Action(uint32 chid)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_Offhook_Action(chid);


		rcm_sim_log( "\n" );   

	return ret;
}

/**********************************************************************/

#ifdef CONFIG_RTK_VOIP_IP_PHONE
int rcm_tapi_GetIPPhoneHookStatus( uint32 *pHookStatus )
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_GetIPPhoneHookStatus(pHookStatus);

		rcm_sim_log( "\n" );   


	return ret;
}
#endif /* CONFIG_RTK_VOIP_IP_PHONE */

int rcm_tapi_Set_flush_fifo(uint32 chid)
{
	int ret=0;
	
		rcm_sim_polling();

	ret= rtk_Set_flush_fifo(chid);

		rcm_sim_log( "chid=%d\n" , chid );	 


	return ret;
}

/*	for FXS:
	0: Phone dis-connect, 
	1: Phone connect, 
	2: Phone off-hook, 
	3: Check time out ( may connect too many phone set => view as connect),
	4: Can not check, Linefeed should be set to active state first.

	for FXO:
	0: PSTN Line connect, 
	1: PSTN Line not connect, 
	2: PSTN Line busy 
*/

int rcm_tapi_line_check(uint32 chid)
{
	rcm_sim_polling();
	int value = FXS[chid].rtk_line_check.value;
	rcm_sim_log("value=%d\n" ,  FXS[chid].rtk_line_check.value);   
	FXS[chid].rtk_line_check.value = 0;
 
	return value;
}


int rcm_tapi_FXO_offhook(uint32 chid)// for real DAA off-hook(channel is FXO channel)
{
	
		rcm_sim_polling();
		rcm_sim_log( "chid=%d\n" , chid );	 


	return rtk_FxoOffHook(chid);

}

int rcm_tapi_FXO_onhook(uint32 chid)// for real DAA on-hook(channel is FXO channel)
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_FXO_onhook(chid);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}

int rcm_tapi_FXO_RingOn(uint32 chid)
{
	int ret=0;
	
	rcm_sim_polling();
	
	ret=rtk_FXO_RingOn(chid);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}

int rcm_tapi_FXO_Busy(uint32 chid)
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_FXO_Busy(chid);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;

}

int rcm_tapi_gpio(unsigned long action, unsigned long pid, unsigned long value, unsigned long *ret_value)
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_gpio(action,pid,value,ret_value);

	rcm_sim_log( "\n" );   

	return ret;

}

int rcm_tapi_Set_LED_Display( uint32 chid, uint32 LED_ID, LedDisplayMode mode )
{
	return rtk_Set_LED_Display(chid,LED_ID,mode);
}

int rcm_tapi_Set_SLIC_Relay( uint32 chid, uint32 close1 )
{
	return rtk_Set_SLIC_Relay(chid,close1);
}

int rcm_tapi_Set_Pulse_Digit_Det(uint32 chid, uint32 enable, uint32 pause_time, uint32 min_break_ths, uint32 max_break_ths) /* 0: disable 1: enable Pulse Digit Detection */
{
	int ret=0;
	
		rcm_sim_polling();

	ret= rtk_Set_Pulse_Digit_Det(chid,enable,pause_time,min_break_ths,max_break_ths);

	rcm_sim_log( "chid=%d enable=%d\n" , chid , enable );	

	return ret;
}

int rcm_tapi_Set_Dail_Mode(uint32 chid, uint32 mode) /* 0: disable 1: enable Pulse dial */
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_Dail_Mode(chid,mode);


	rcm_sim_log( "chid=%d mode=%d\n" , chid , mode );   


	return ret;

}

int rcm_tapi_Get_Dail_Mode(uint32 chid) /* 0: disable 1: enable Pulse dial */
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_Get_Dail_Mode(chid);

	rcm_sim_log( "chid=%d\n" , chid );   

	return ret;
}

int rcm_tapi_PulseDial_Gen_Cfg(char pps, short make_duration, short interdigit_duration)
{
	int ret=0;
	
		rcm_sim_polling();

	ret=rtk_PulseDial_Gen_Cfg(pps,make_duration,interdigit_duration);

	rcm_sim_log( "\n" );   

	return ret;


}

int rcm_tapi_Gen_Pulse_Dial(uint32 chid, char digit) /* digit: 0 ~ 9 */
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_Gen_Pulse_Dial(chid,digit);

	rcm_sim_log( "chid=%d digit=%d\n", chid , digit );   

	return ret;

}

uint32 rcm_tapi_Get_SLIC_Ram_Val(uint8 chid, uint16 reg)
{
	uint32 ret=0;
	
	rcm_sim_polling();

	ret= rtk_Get_SLIC_Ram_Val(chid,reg);

		rcm_sim_log( "chid=%d reg=%d\n", chid , reg );	 


	return ret;
}

int rcm_tapi_SLIC_Ram_Val(uint8 chid, uint16 reg, uint32 value)
{
	int ret=0;
	
		rcm_sim_polling();


	ret= rtk_Set_SLIC_Ram_Val(chid,reg,value);

	rcm_sim_log( "chid=%d reg=%d value=%d\n", chid , reg , value);   

	return ret;

}

int32 rcm_tapi_SetFaxModemDet(uint32 chid, uint32 mode)
{
	int32 ret=0;
	
		rcm_sim_polling();


	ret= rtk_SetFaxModemDet(chid,mode);

	rcm_sim_log( "chid=%d mode=%d\n", chid , mode );   

	return ret;

}


int32 rcm_tapi_GetPortLinkStatus( uint32 *pstatus )
{
	uint32 status;
	rcm_sim_polling();
	rcm_sim_log( "TBD return link up tempararily\n" );   

	status = PORT_LINK_LAN_ALL | PORT_LINK_WAN;
	*pstatus = status;

	return 0;
}



int32 rcm_tapi_WTD_Reboot(int reboot)
{
	int32 ret=0;
	ret=rtk_WTD_Reboot(reboot);
	
	rcm_sim_polling();
	rcm_sim_log( "\n" );   

	return ret;
}

int rcm_tapi_print(int level, char *module, char *msg)
{
	int ret;
	
		rcm_sim_polling();

	ret= rtk_print(level,module,msg);
	rcm_sim_log( "%s %s\n" , module , msg );

	return ret;

}

int rcm_tapi_cp3_measure(st_CP3_VoIP_param* cp3_voip_param)
{
	return rtk_cp3_measure(cp3_voip_param);
}

int32 rcm_tapi_SetDspIdToDsp(unsigned char dsp_id)
{
   return rtk_SetDspIdToDsp(dsp_id);
}

int32 rcm_tapi_SetDspPhyId(unsigned char phy_id)
{
    return rtk_SetDspPhyId(phy_id);
}

uint8 rcm_tapi_CheckDspAllSoftwareReady(unsigned char phy_id)
{
   return rtk_CheckDspAllSoftwareReady(phy_id);
}
#endif

