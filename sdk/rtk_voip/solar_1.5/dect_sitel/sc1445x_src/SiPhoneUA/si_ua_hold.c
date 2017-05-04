/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 

 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 		 si_ua_hold.c
 * Purpose:		 		 
 * Created:		 		 Nov 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <si_print_api.h>

#include "../common/si_phone_api.h"
#include "../common/si_timers_api.h"
#include "../../mcu/sc1445x_mcu_api.h"
#include "../CCFSM/ccfsm_init.h"

#include "si_sdp_negotiation.h"
#include "si_ua_hold.h"
#include "si_ua_notify.h"
#include "si_ua_callcontrol.h"
#include "si_ua_sessiontimer.h"
#include "si_ua_parser.h"
#include "si_ua_fax.h"

extern  SICORE siphoneCore;
osip_message_t *origInvite=NULL;
 

void holdOnTimerCallBack(void)
{
	int i;
  	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		if (SiCall[i].localHold ==2)
		{
			si_ua_terminateCall(&SiCall[i]);
  		si_print(PRINT_LEVEL_DEBUG, "holdOnTimerCallBack\n\n ");
			return ;			
 		}
	}
}
void holdOffTimerCallBack(void)
{
	int i;
  	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		if (SiCall[i].localHold ==3)
		{
			si_ua_terminateCall(&SiCall[i]);
  		si_print(PRINT_LEVEL_DEBUG, "holdOffTimerCallBack\n\n ");
			return ;			
 		}
	}
}
int si_ua_reInvite(sicall_t* pCall) 
{
  sdp_message_t *sdp=NULL;
	osip_message_t *invite;
	char *sdpbody;
	int ret;

	if (pCall==NULL) return -1;
 
  si_print(PRINT_LEVEL_DEBUG, "send REINVITE .............\n");
 	sdp=eXosip_get_local_sdp_from_tid(pCall->tid);     
     
	if (sdp==NULL)
	{
	  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to get local sdp \n"); 
	  eXosip_unlock();
	  return -1;
	}
 
	 ret=eXosip_call_build_request(pCall->did, "INVITE", &invite); 
	 if (ret) 
	 {
		  sdp_message_free(sdp);
		  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold 5"); 
		  eXosip_unlock();
		  return -1;
	 } 
	  
  	ret = sdp_message_to_str(sdp, &sdpbody);

	if (sdpbody)
	{
		if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER); 
			si_sessiontimer_add_refresher_string(pCall, invite);
				//??????? require
 		}else {
 			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE);
		}
		 
		osip_message_set_content_type (invite, "application/sdp");
		osip_message_set_allow (invite, SipAllowHeader);
		osip_message_set_body (invite,sdpbody, strlen (sdpbody));

  	 }else
	 {
	  sdp_message_free(sdp);
	  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold"); 
	  eXosip_unlock();
	  return -1;
	  }
	  
	 ret = eXosip_call_send_request(pCall->did, invite);
	 if (ret!=0) 
	 {
		  sdp_message_free(sdp);
		  //YPAPA SOS
		   osip_message_free (invite);
		   FatalError(EXOSIP_HOLD_ON_FAILED, "failed to send reinvite message "); 
		  eXosip_unlock();
		  return -1;
	 } 

 		if (sdp) sdp_message_free(sdp);
		if (sdpbody) osip_free(sdpbody);
		eXosip_unlock();
 
 		return 0;
}
 
int si_ua_holdOn( sicall_t* pCall, int action) 
{
	osip_negotiation_t sdpInfo;
	int did;
	int i;
	int tid;
	sdp_message_t *sdp=NULL;
	osip_message_t *invite;
	char *addr_null;
	char *sdpbody;
	int ret;
	int type;
 	char *rtpport;   
 
	if (pCall==NULL) return -1;
  	if (action) type =1;
	else type =(int)pCall->remoteHold;
 	si_print(PRINT_LEVEL_DEBUG, "PUT a CALL on HOLD .............\n");
 	eXosip_sdp_negotiation_init(pCall->accountID,  &sdpInfo, &siphoneCore); 
 	did=pCall->did;
	tid=pCall->tid;
  	eXosip_lock();
	rtpport = eXosip_sdp_get_audio_port(pCall);
 
  	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
	{
		if (atoi(sdpInfo.audio_codec[i].payload) == pCall->sStreamParameters.payload) 
		{
			memcpy(&sdpInfo.audio_codec[0], &sdpInfo.audio_codec[i], sizeof(audio_profile));
			sdpInfo.audio_codec[0].valid = 1;
			break;
		}
	}
    for (i=1;i<MAX_SUPPORTED_CODECS;i++)
			sdpInfo.audio_codec[i].valid = 0;
 
	sdp = eXosip_create_sdp_offer(pCall, invite, &sdpInfo,rtpport,rtpport,0);
    
	  if (sdp==NULL)
	 {
	  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold 1"); 
	  eXosip_unlock();
	  return -1;
	 }
	   ret=create_sdp_hold_on(sdp, type);
	  if (ret) 
	 {
		  sdp_message_free(sdp);

		  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold 2"); 
		  eXosip_unlock();
		  return -1;
	 }
 
	 ret=eXosip_call_build_request(did, "INVITE", &invite);//int did, const char *method, osip_message_t **request);
	 if (ret) 
	 {
		  sdp_message_free(sdp);
		  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold 5"); 
		  eXosip_unlock();
		  return -1;
	 } 
 
	 if (sdp->c_connection && sdp->c_connection->c_addr)
	 {
	  // fix: Memory Leak (06052008)
		osip_free(sdp->c_connection->c_addr);    
		addr_null=osip_strdup("0.0.0.0"); 
	  if (addr_null==NULL)
	  {
	   sdp_message_free(sdp);
	   FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold 4"); 
	   eXosip_unlock();
	   return -1;
	   }
	   // fix: Memory Leak (06052008)
	   sdp->c_connection->c_addr = addr_null;
	 }
	  ret = sdp_message_to_str(sdp, &sdpbody);

	  if (sdpbody)
	 {
		if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER); 
			si_sessiontimer_add_refresher_string(pCall, invite);
			//??????? require
 		}else {
 			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE);
		}
	 
	  osip_message_set_content_type (invite, "application/sdp");
	  osip_message_set_allow (invite, SipAllowHeader);
	  osip_message_set_body (invite,sdpbody, strlen (sdpbody));

	 }else
	 {
	  sdp_message_free(sdp);
	  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold"); 
	  eXosip_unlock();
	  return -1;
	  }
 
		 ret = eXosip_call_send_request(did, invite);
		 if (ret!=0) 
		 {
			  sdp_message_free(sdp);
			  //YPAPA SOS
			   osip_message_free (invite);
			   FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold"); 
			  eXosip_unlock();
			  return -1;
		 } 
		 pCall->localHold=2;
		 origInvite = invite;
		// pthread_mutex_lock(&mTimerMutex);
		  si_addtimerList(&holdOnTimerCallBack,32000000,TIMER_SOURCE_UA, NULL);
		// pthread_mutex_unlock(&mTimerMutex);

		if (sdp) sdp_message_free(sdp);
		if (sdpbody) osip_free(sdpbody);
		eXosip_unlock();
		si_audio_stream_suspend( pCall);  
		si_print(PRINT_LEVEL_DEBUG, "============  SUSPEND CALL = %x MCU = %x \n ", (int)pCall,pCall->pMcuHandler );
 		return 0;
}

int si_ua_holdOff(sicall_t* pCall)
{
	osip_negotiation_t sdpInfo;
	int i;
	sdp_message_t *sdp=NULL;
	osip_message_t *invite;
	int did;
	int tid;
	char *sdpbody;
	int ret;
	char *rtpport;
	int type =(int)pCall->remoteHold;

	eXosip_sdp_negotiation_init(pCall->accountID,  &sdpInfo, &siphoneCore); 

	if (pCall==NULL) return -1;
	did=pCall->did;
	tid=pCall->tid ;
	eXosip_lock();

	rtpport = eXosip_sdp_get_audio_port(pCall);
  
	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
	{
		if (atoi(sdpInfo.audio_codec[i].payload) == pCall->sStreamParameters.payload) 
		{
			memcpy(&sdpInfo.audio_codec[0], &sdpInfo.audio_codec[i], sizeof(audio_profile));
			sdpInfo.audio_codec[0].valid = 1;
			break;
		}
	 }
	for (i=1;i<MAX_SUPPORTED_CODECS;i++)
		sdpInfo.audio_codec[i].valid = 0;

 
	sdp = eXosip_create_sdp_offer(pCall, invite, &sdpInfo,rtpport,rtpport,0);
  
	{
		sdp_connection_t *pConnection;
		int pos;
		pos =-1;

		pConnection = sdp_message_connection_get(sdp,pos,0);
		while (pConnection!=NULL)
		{
			 pos++;
			if (pConnection->c_addr)
			{
				osip_free(pConnection->c_addr);
				pConnection->c_addr = osip_strdup((char*)siphoneCore.IPAddress);//pCall->remoteHost);
			} 
			pConnection = sdp_message_connection_get(sdp,0,0);
		}
	}


	if (sdp==NULL)
	{
		FatalError(EXOSIP_HOLD_OFF_FAILED, "failed to put call off hold"); 
		eXosip_unlock();
		return -1;
	}

	ret=create_sdp_hold_off(sdp, type);
	if (ret) 
	{
		sdp_message_free(sdp);
		FatalError(EXOSIP_HOLD_OFF_FAILED, "failed to put call off hold"); 
		eXosip_unlock();
		return -1;
	}

	ret=eXosip_call_build_request(did, "INVITE", &invite);//int did, const char *method, osip_message_t **request);
	if (ret) 
	{
		sdp_message_free(sdp);
		FatalError(EXOSIP_HOLD_OFF_FAILED, "failed to put call off hold"); 
		eXosip_unlock();
		return -1;
	} 

	ret = sdp_message_to_str(sdp, &sdpbody);

	if (sdpbody)
	{
			if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER); 
 			 
 			si_sessiontimer_add_refresher_string(pCall, invite);
   			//??????? require
 		}else {
 			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE);
		}
		osip_message_set_content_type (invite, "application/sdp");
		osip_message_set_allow (invite, SipAllowHeader);
		osip_message_set_body (invite,sdpbody, strlen (sdpbody));
	}else
	{
		sdp_message_free(sdp);
		FatalError(EXOSIP_HOLD_OFF_FAILED, "failed to put call off hold"); 
		eXosip_unlock();
		return -1;
	}

	ret  = eXosip_call_send_request(did, invite);
	if (ret!=0) 
	{
		sdp_message_free(sdp);
		//  osip_message_free (invite);
		FatalError(EXOSIP_HOLD_OFF_FAILED, "failed to put call off hold"); 
		eXosip_unlock();
		return -1;
	} 
  
	//YPAPA
	//int setTimeout(int duration, char* reason);
	//setTimeout(1000,"Holding Off");
	pCall->localHold=3;
	// pthread_mutex_lock(&mTimerMutex);
	si_addtimerList(&holdOffTimerCallBack,32000000,TIMER_SOURCE_UA, NULL);
	// pthread_mutex_unlock(&mTimerMutex);
	// fix: Memory Leak (06052008)
	if (sdp) sdp_message_free(sdp);
	if (sdpbody) osip_free(sdpbody);
//	printf("Hold off ...  resuming call \n ");
	si_audio_stream_resume(pCall);
	eXosip_unlock();
  
    return 0;
} 	

int si_ua_processReinvitetoHoldOn(eXosip_event_t* pEvent)
{
	int cb_required =1;
	int remoteport=-1;
	unsigned char remoteaddress[16];
	int switchtofax=0;
 	ccfsm_updatestatus_cback_data m_cback_updatestatus ; 
	int tid = pEvent->tid;
  	sdp_message_t *sdp=NULL;
  	sdp_message_t *sdpRemote=NULL;
	osip_message_t *pAnswer;
 	audio_stream_parameters tempStreamParameters;
	sicall_t *pCall;
	char *addr_null;
	char *sdpbody;
 	int ret,i;
	 
	remoteaddress[0]='\0';

	pCall = (sicall_t*)findCallByIds(pEvent);
	if (pCall==NULL) return -1;
	m_cback_updatestatus.accountid = pCall->accountID; 
	m_cback_updatestatus.callid = pCall->callid; 
  	eXosip_lock();
	si_sessiontimer_update(pCall, pEvent->request ); 
 	sdp = eXosip_get_remote_sdp_from_tid(tid);
  	if (sdp==NULL)
	{
  	    char* rtpport = eXosip_sdp_get_audio_port(pCall);
   		sdp = eXosip_create_sdp_offer(pCall, NULL, &sdpNegotiationData[pCall->accountID],rtpport,rtpport,0);
		 
  		goto aaa;
	}else{
 	 	//check if the incoming packet contains information for holdon/holdoff
		//FIX BROADWORKS - TEST PLAN 7
		if (check_incoming_sdp(pCall, sdp, 0))
		{
			if (pCall->session_version>0)
				pCall->session_version--;
	 		// to take care if there is a call state transition although a session version is the same with the old 
			// transaction
			cb_required=0;
			 
		}
	 

	  ret= getRemoteSDPInfo(sdp, remoteaddress, &remoteport);
	 
		sdp=create_sdp_answer_4invite_on_hold(pCall,  &sdpNegotiationData[pCall->accountID], &siphoneCore, sdp );
 	}
  
 	if ((ret==SI_MESSAGE_INVITE_ON_HOLD) && (cb_required))
	{
	 	 
   		pCall->remoteHold =(int) 1;
 		si_audio_stream_suspend( pCall);
   		m_cback_updatestatus.cback_id = CCFSM_HELD_CB;
		m_cback_updatestatus.state = CCFSM_STATE_HELD;
 		ccfsm_callback(&m_cback_updatestatus);

	}else if ((ret==SI_MESSAGE_INVITE_OFF_HOLD) && (pCall->localHold || pCall->remoteHold ))
	{
		cb_required=3;
		 

 	   pCall->remoteHold=(int)0;
 		for (i=0;i<MAX_SUPPORTED_CODECS;i++)
 			siphoneCore.Codecs[pCall->accountID][i]= siphoneCore.CodecsSupported[pCall->accountID][i];
   		ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdp, &tempStreamParameters);
    	if (ret) {si_print(PRINT_LEVEL_ERR, "Failed to get SDP \n\n ");return ret;}

  		ret = si_ua_checkSDPForChanges(sdp, pCall, &tempStreamParameters, 0, remoteaddress, remoteport);
		if (!ret)
		{
			si_print(PRINT_LEVEL_DEBUG, "===========   execute resume from REMOTE HELD \n");
			si_audio_stream_resume(pCall);
 		}
  		if (pCall->localHold)
			create_updatestatus_callback(CCFSM_RESUMED_CB, pCall->accountID, pCall->callid,  (int)pCall->sStreamParameters.payload,0,"",CCFSM_STATE_HOLD);
		else
			create_updatestatus_callback(CCFSM_RESUMED_CB, pCall->accountID, pCall->callid,  (int)pCall->sStreamParameters.payload,0,"",CCFSM_STATE_TALK);
  
   	} else if (ret!=SI_MESSAGE_INVITE_IMAGE) 
	{
		 
	 	//possible codec change
  	 	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
 			siphoneCore.Codecs[pCall->accountID][i]= siphoneCore.CodecsSupported[pCall->accountID][i];
 
 	  	ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdp, &tempStreamParameters);
   		if (ret) {si_print(PRINT_LEVEL_ERR, "Failed to get SDP \n\n ");return ret;}
 		ret = si_ua_checkSDPForChanges(sdp, pCall,&tempStreamParameters, 0, remoteaddress, remoteport);

		if (ret>0 && cb_required)
		{
  			create_updatestatus_callback(CCFSM_INCALL_CODEC_CHANGED_CB, pCall->accountID, pCall->callid,  (int)pCall->sStreamParameters.payload,0,"",CCFSM_STATE_TALK);
 		}
	}else  if (ret==SI_MESSAGE_INVITE_IMAGE) 
	{
		ata_t38_sdp_params_type fax_sdp_params;
		sdpRemote= eXosip_get_sdp_info(pEvent->request);
		if (sdpRemote==NULL || sdp_get_fax_params(&fax_sdp_params, sdpRemote, &pCall->sStreamParameters))
		{
			si_print(PRINT_LEVEL_ERR, "Failed to negotiate sdp/fax params \n ");
			si_ua_terminateCall(pCall);
			return 0;
		}
		switchtofax=1;
	} 

	 
   	if (sdp==NULL) 
	{
 		eXosip_call_send_answer (tid, 500, NULL); //TO CHECK status code 488, 415
		FatalError(EXOSIP_HOLD_ON_FAILED, "failed to process reinvite on hold"); 
		eXosip_unlock();
		return -1;
	}
  

 	if (pCall->remoteHold==1)
	{
	  		if (sdp->c_connection && sdp->c_connection->c_addr) osip_free(sdp->c_connection->c_addr);  		
			// fix: Memory Leak (06052008)
  			addr_null=osip_strdup("0.0.0.0"); 
 			if (addr_null==NULL)
			{
				if (sdp) sdp_message_free(sdp);
				sdp  = NULL;
				eXosip_call_send_answer (tid, 500, NULL); //TO CHECK status code 488, 415
				FatalError(EXOSIP_HOLD_ON_FAILED, "failed to process reinvite on hold"); 
				eXosip_unlock();
				return -1;
 			}
 			// fix: Memory Leak (06052008)
  			sdp->c_connection->c_addr = addr_null;
  	}
aaa:
	ret = eXosip_call_build_answer (tid, 200, &pAnswer);

	if (ret) 
	{
		if (sdp)
			sdp_message_free(sdp);
		eXosip_call_send_answer (tid, 500, NULL); //TO CHECK status code 488, 415
		FatalError(EXOSIP_HOLD_ON_FAILED, "failed to process reinvite on hold"); 
		eXosip_unlock();
		return -1;
	} 
   
	ret = sdp_message_to_str(sdp, &sdpbody);
  	if (sdpbody){
  	 	osip_message_set_body (pAnswer, sdpbody, strlen (sdpbody));
	 	osip_message_set_content_type (pAnswer, "application/sdp");
		osip_message_set_allow(pAnswer, SipAllowHeader);
		if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported(pAnswer,SIPHONE_UA_SUPPORTED_HEADER_ACK200_wTIMER );
 			// SESSION TIMER 
 
			si_sessiontimer_add_refresher_string(pCall, pAnswer);
 
		}else
		{
			osip_message_set_supported(pAnswer,SIPHONE_UA_SUPPORTED_HEADER_ACK200 );
 		}

		// fix: Memory Leak (06052008)
		if (sdp){ 	
 				sdp_message_free(sdp);
				sdp  = NULL;
		}
		if (sdpbody){
  			osip_free(sdpbody);
			sdpbody=NULL;
		}
 		eXosip_call_send_answer (tid, 200, pAnswer);
 		eXosip_unlock ();
			eXosip_execute();
			eXosip_automatic_action ();
  usleep(100);
  

  if (switchtofax)
   sc1445x_fax_switch_mode(pCall);

    return 0;
  }
  else
 {
 

  // fix: Memory Leak (06052008)
  if (sdp){  
     sdp_message_free(sdp);
    sdp  = NULL;
  }
  eXosip_call_send_answer (tid, 500, NULL); //TO CHECK status code 488, 415
  // YPAPA SOS
   osip_message_free (pAnswer);
  eXosip_unlock ();
    return -1;
  }
}
//FIX OCTOBER
#include "si_sc_manager.h"
int si_ua_checkSDPForChanges(sdp_message_t *sdp, sicall_t *pCall, audio_stream_parameters *pStreamParams, int flag, unsigned char* remoteIP, int remotePort)
{
	int ret;
	int ptime;
 	int modifyCallData =0;	
	unsigned char remoteAddress[16];
 	if (remoteIP && remoteIP[0]!='\0' && remotePort>0)
	{
  		sprintf((char*)remoteAddress, "%d.%d.%d.%d", pCall->mcuMediaStream.mediaParams.rAddress[0],pCall->mcuMediaStream.mediaParams.rAddress[1],pCall->mcuMediaStream.mediaParams.rAddress[2],pCall->mcuMediaStream.mediaParams.rAddress[3]);
  		if ((pCall->mcuMediaStream.mediaParams.rport!= remotePort) || strcmp((char*) remoteAddress, (char*)remoteIP))
		{
   			convert_ip_address((unsigned char*)pCall->mcuMediaStream.mediaParams.rAddress, (char*)remoteIP);		
			pCall->mcuMediaStream.mediaParams.rport= remotePort;
	 		modifyCallData=1;
		}
  	}
 
	

  	if (modifyCallData || (pCall->sStreamParameters.payload  != pStreamParams->payload))
	{
		modifyCallData =1;
   		pCall->sStreamParameters.payload = pStreamParams->payload ;
   		pCall->sStreamParameters.d_payload  = pStreamParams->d_payload  ;
		
		pCall->mcuMediaStream.mediaParams.ptype = (unsigned char)pCall->sStreamParameters.payload;
		pCall->mcuMediaStream.mediaParams.rtp_ptype  = (unsigned char)pCall->sStreamParameters.d_payload ;
 
		if ((ptime = sc1445x_media_get_ptime(pStreamParams->payload))==0) 	 
		{
			ptime = sdp_message_get_ptime(sdp); 
		}

		//ypapa sos
 		if (ptime>0){
			pCall->mcuMediaStream.mediaParams.rxRate  =(unsigned short) (ptime/10) *10;
			pCall->mcuMediaStream.mediaParams.txRate  =(unsigned short) (ptime/10) *10;
 		}
		else  {
 			pCall->mcuMediaStream.mediaParams.rxRate  =(unsigned short) 20;
			pCall->mcuMediaStream.mediaParams.txRate  =(unsigned short) 20;
		}
 		if (pCall->mcuMediaStream.mediaAttr == MCU_MATTR_INA_ID)
		{
			pCall->mcuMediaStream.mediaAttr = MCU_MATTR_BOTH_ID;
		}
	}
	ret = getRemoteSDPInfo(sdp, NULL,NULL);
	if ((ret==SI_MESSAGE_INVITE_OFF_HOLD) && (pCall->localHold || pCall->remoteHold ))
	{
		modifyCallData =1;
		printf("RESUME FROM HOLD .......................\n");
		pCall->mcuMediaStream.mediaAttr = MCU_MATTR_BOTH_ID;
	}

 	if (modifyCallData)
	{
  		sc1445x_mcu_configure(pCall->pMcuHandler,MCU_MCTRL_MODIFY_ID,&pCall->mcuMediaStream);   	 
		if (flag)
			create_updatestatus_callback(CCFSM_INCALL_CODEC_CHANGED_CB, pCall->accountID, pCall->callid,  (int)pCall->sStreamParameters.payload,0,"",CCFSM_STATE_TALK);
		return 1;
	}
  	return 0;
}

 int si_ua_codecChange( sicall_t* pCall ) 
{
	char *rtpport;
 	osip_message_t *invite;
	char *sdpbody;
 	int ret;	 
	if (pCall==NULL) return -1; 
	eXosip_lock();
	ret=eXosip_call_build_request(pCall->did, "INVITE", &invite);//int did, const char *method, osip_message_t **request);
	if (ret) 
	{
		FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold 5"); 
		eXosip_unlock();
		return -1;
	} 
    rtpport = eXosip_sdp_get_audio_port(pCall);
     
	sdpbody = eXosip_create_sdpbody_offer(pCall, invite, &sdpNegotiationData[pCall->accountID],rtpport,rtpport,0);
   	if (sdpbody)
	{
 	 	osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE);
	 	osip_message_set_content_type (invite, "application/sdp");
 
		  
		 osip_message_set_allow (invite, SipAllowHeader);
	 	osip_message_set_body (invite,sdpbody, strlen (sdpbody));
	
	}else
	{
 		FatalError(EXOSIP_HOLD_ON_FAILED, "failed to change codec"); 
		eXosip_unlock();
		return -1;
 	}
  
	ret = eXosip_call_send_request(pCall->did, invite);
	if (ret!=0) 
	{
 		//YPAPA SOS
	 	osip_message_free (invite);
 		FatalError(EXOSIP_HOLD_ON_FAILED, "failed to put call on hold 11"); 
    if (sdpbody) osip_free(sdpbody);
		eXosip_unlock();
		return -1;
	} 
  //   	si_addtimerList(&holdOnTimerCallBack,32000000,TIMER_SOURCE_UA, NULL);
  	
   	if (sdpbody) osip_free(sdpbody);
  	pCall->codecChange=1;
	eXosip_unlock();
  
    return 0;
}
 
void si_audio_stream_suspend(sicall_t* pCall )
{
	si_print(PRINT_LEVEL_DEBUG, "[%s] \n", __FUNCTION__);
	pCall->mcuMediaStream.mediaAttr = MCU_MATTR_INA_ID;
	sc1445x_mcu_configure(pCall->pMcuHandler,MCU_MCTRL_MODIFY_ID,&pCall->mcuMediaStream);

}
void si_audio_stream_resume(sicall_t* pCall )
{
	si_print(PRINT_LEVEL_DEBUG, "[%s] \n", __FUNCTION__);
	pCall->mcuMediaStream.mediaAttr = MCU_MATTR_BOTH_ID;
	sc1445x_mcu_configure(pCall->pMcuHandler,MCU_MCTRL_MODIFY_ID,&pCall->mcuMediaStream);   	 
}


