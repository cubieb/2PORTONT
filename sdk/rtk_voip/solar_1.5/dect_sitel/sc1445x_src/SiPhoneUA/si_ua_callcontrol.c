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
 * File:		 		 si_ua_callcontrol.c
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <si_print_api.h>
 
#include "../common/si_phone_api.h"
#include "../common/si_configfiles_api.h"
#include "../common/si_timers_api.h"

#include "si_sc_manager.h"
#include "si_sdp_negotiation.h"
#include "si_ua_callcontrol.h"
#include "si_ua_hold.h"
#include "si_ua_transfer.h"
#include "si_ua_notify.h"
#include "si_ua_sessiontimer.h"
#include "si_ua_reliable.h"
#include "si_ua_stun.h"
#include "si_ua_registrar.h"
#include "si_ua_parser.h"
#include "../../mcu/sc1445x_ringplay.h"
#include "../../mcu/sc1445x_mcu_api.h"
 
#include "../exosip2/src/eXosip2.h"
#include "../common/operation_mode_defs.h" 
#include "../CCFSM/ccfsm_init.h"
  
#define INVITE_NO_SDP  1

///////////////////////////////////////////////////////////////////////////////
//				REGISTER SECTION
//
//
///////////////////////////////////////////////////////////////////////////////
extern  SICORE siphoneCore;
void si_ua_setpreferred(int accountId,int codectype, SICORE* pCore,  osip_negotiation_t* audioProfile );
 ///////////////////////////////////////////////////////////////////////////////
//				INCOMING CALL SECTION
//
//
///////////////////////////////////////////////////////////////////////////////
int UniqueCallId = 0x8000;
int gerenateCallIdentification(void)
{
	if (--UniqueCallId==0x0) UniqueCallId=0x8000;
	return UniqueCallId;
}
  
void si_ua_incomingCall(eXosip_event_t *pEvent)
{
 	osip_header_t *pReplaceHeader=NULL;
	sdp_message_t *sdpRemote=NULL;
 	incomingInfo_t from;
	sicall_t* pCall;
	int ret;
	int i;
	int action= (int)0;
 
	// check user/phone settings.
   // allocate an ID for the new incoming call. If there is no available line then reject the call //YPAPA status code????
	if (checkRequiredHeader(pEvent->request))
		return ;

    pCall = allocateCallId(pEvent);
  	if (pCall==NULL)
	{
  		eXosip_lock ();
		eXosip_call_send_answer(pEvent->tid, 486, NULL);
 		eXosip_unlock ();
		return  ;		
 	}
    pCall->callid=gerenateCallIdentification();	
      //pCall->cbackId=(int)-1;	

	pCall->rCall = 0;
	pCall->initiator=0;
	pCall->updateInterval=0;
	pCall->codectype =0;
	siphoneCore.session_version+=10000;
 	pCall->SDPEnchanged = SIPUA_OFFERANSWER_IDLE;
	pCall->session_id=siphoneCore.session_version;
	pCall->session_version=siphoneCore.session_version;
	pCall->loop=0;
  
 	if (pEvent->request->to->url->username )
	{
		pCall->accountID = findAccount(pEvent->request->to->url->username);
		if (pCall->accountID<0)
		{
			eXosip_lock ();
			eXosip_call_send_answer(pEvent->tid, 405, NULL);
 			eXosip_unlock ();
 			return ;
		}
	}
	else 
	{
		pCall->accountID=0;
		#ifdef DECT_ENABLED
			pCall->accountID = CCFSM_DECT_BROADCAST_ID;
		#endif
	}
 
	pCall->attachedentity = findEntity(pCall->accountID );
	pCall->port = findPort(pCall->accountID, pCall->attachedentity );
 
	pCall->sStreamParameters.payload=-1;
  	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
	{
		siphoneCore.Codecs[pCall->accountID][i]= siphoneCore.CodecsSupported[pCall->accountID][i];
	}
  	eXosip_sdp_negotiation_init(pCall->accountID,  &sdpNegotiationData[pCall->accountID], &siphoneCore); 
    if (pEvent && pEvent->request && pEvent->request->from->url->host)
	{
		if (!strcmp(pEvent->request->from->url->host,siphoneCore.RegistrarIPAddress))
 			pCall->directIP =0;
		else 
			pCall->directIP =1;
 	}
#ifdef DECT_ENABLED
	if (!checkDECTBroadcastState(pCall->accountID))
	{
		si_ua_rejectCall(pCall);
		return;
	}
#endif

	// Read Phone Setting from file
	// Read Phone Setting from file
	if (isAnonymousBlocked(pCall->accountID,pEvent))
	{
		si_ua_rejectCall(pCall);
		return ;
	}

	if (!isWaiting(pCall->accountID))
	{
		si_ua_rejectCall(pCall);
		return ;
	}
 	if (isBusy(pCall->accountID))
	{
  		si_ua_rejectCall(pCall);
			return;
	}
 	if  (isRedirected(pCall->accountID)) 
	{
  		si_ua_redirect(pCall, pEvent,  getRedirectionUri(pCall->accountID), getRedirectionReason(pCall->accountID));
		return;
	}
 	// retrieve the sdp info from the remote side
	sdpRemote= eXosip_get_sdp_info(pEvent->request);
	if (sdpRemote==NULL) {
		si_print(PRINT_LEVEL_ERR, "Failed to retrieve SDP from the current response \n\n ");
		sdpRemote= eXosip_get_remote_sdp_from_tid(pEvent->tid);
		if (sdpRemote==NULL) {
			sdpRemote= eXosip_get_remote_sdp(pEvent->did);
			if (sdpRemote==NULL) {
				pCall->SDPEnchanged = SIPUA_OFFERANSWER_IDLE;
				#ifndef INVITE_NO_SDP
					si_ua_terminateCall(pCall);
					si_print(PRINT_LEVEL_ERR, "Failed to retrieve SDP from the current incoming EVENT \n\n ");
					return  ;
				#endif
			}
		}
	}
 
	if (sdpRemote) {
		check_incoming_sdp(pCall, sdpRemote, 1);
		pCall->SDPEnchanged = SIPUA_OFFERANSWER_OFFER;
	}

   
	if (getNumOfCalls(pCall) || ( !getOtherActiveCall (pCall) && isAutoAnswer(pCall->accountID))) //
	{
		action = 1;
 		si_stop_ringtone(pCall );

	}else if (si_ua_setRinging( pCall, pEvent->request)) // June 20, 2008
	{
		si_print(PRINT_LEVEL_ERR, "Unable to send ringing ...\n ");
	  	si_ua_rejectCall(pCall);
		return ;
 	}
 	si_sessiontimer_update(pCall, pEvent->request);
	if (pEvent->request->from->url)
 	{
		memset(from.displayname, '\0', sizeof(from.displayname));
		memset(from.username, '\0', sizeof(from.username));
		memset(from.host, '\0', sizeof(from.host));
 		if (pEvent->request->from->displayname)
		{
   			strcpy(from.displayname, pEvent->request->from->displayname);
  		}
		if (pEvent->request->from->url->username)
		{
  			strcpy(from.username , pEvent->request->from->url->username);
  		}
 		if (pEvent->request->from->url->host)
		{
  			strcpy(from.host , pEvent->request->from->url->host);
  			strcpy(pCall->remoteHost,pEvent->request->from->url->host); 
 		}
    } 
 
	if (sdpRemote){
		 ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdpRemote, &pCall->sStreamParameters);
 
		 pCall->SDPEnchanged = SIPUA_OFFERANSWER_OFFER;
	}else pCall->sStreamParameters.payload=-1;

	ret = osip_message_header_get_byname(pEvent->request, "Replaces", 0, &pReplaceHeader);	
 	  
	if (pReplaceHeader) 
	{
		sicall_t* pOldCall;
		ret = eXosip_call_find_by_replaces(pReplaceHeader->hvalue);
		pOldCall = findCallByCid(ret);
		si_ua_terminateCall(pOldCall);
		eXosip_execute();
		eXosip_automatic_action ();
		action = 1;
	}
  	{
		ccfsm_new_call_cback_data m_cback_newcall;
		// find atatched entity 
 		if (action==0) 
		{
			m_cback_newcall.cback_id=CCFSM_NEW_CALL_CB;
			m_cback_newcall.state = CCFSM_STATE_INCOMING;
 		}
 		else 
		{
			m_cback_newcall.cback_id=CCFSM_NEW_CALL_ANSWERED_CB;
			m_cback_newcall.state = CCFSM_STATE_TALK;
 		}
		m_cback_newcall.attachedentity= pCall->attachedentity ;
  
		m_cback_newcall.accountid = pCall->accountID ;
		m_cback_newcall.callid = pCall->callid; 
 		m_cback_newcall.calltype=CCFSM_DIRECTION_IN;
		m_cback_newcall.portid =pCall->port;

 		m_cback_newcall.codec=(int)pCall->sStreamParameters.payload;
		m_cback_newcall.username_num[0]='\0';
		m_cback_newcall.host_num[0]='\0';
		m_cback_newcall.display_num[0]='\0';
 		if (from.username[0]!='\0')
		{
			strcpy(m_cback_newcall.username_num, from.username ); 
			strcpy(m_cback_newcall.host_num, from.host ); 

		}
		else 
		{
			strcpy(m_cback_newcall.username_num, from.host ); 
			strcpy(m_cback_newcall.host_num, from.host ); 
 		}
		if (from.displayname && (strlen(from.displayname)<sizeof(m_cback_newcall.display_num)))
		{	
 			strcpy(m_cback_newcall.display_num,from.displayname);  
		}
		else m_cback_newcall.display_num[0]='\0';

		m_cback_newcall.status =0;
	
 		ccfsm_callback(&m_cback_newcall);
    	}
	 if (action == 1){
			si_ua_acceptCall(pCall); 
	}else  pCall->state = CALL_STATE_NEW;


	#ifdef FIX_MEMORY_LEAK
		if (sdpRemote) 
			sdp_message_free(sdpRemote);
	#endif

 	return ;
}

int si_ua_setRinging(sicall_t* pCall, osip_message_t *pInPacket)
{
 	osip_message_t *pAnswer;
	int ret;
	eXosip_lock ();
	// check for SDP in ringing message
	//edoo

 	ret = eXosip_call_build_answer (pCall->tid, 180, &pAnswer);
 	if (ret==0)
	{
	 	osip_message_set_allow(pAnswer, SipAllowHeader);
		if (si_ua_mustbeReliableRespone(pInPacket))
		{
 		//Check if a offer must be sent
			if (pCall->SDPEnchanged==SIPUA_OFFERANSWER_IDLE)
			{
				//Add SDP info
			}
			osip_message_set_require(pAnswer, "100rel");
			{
				char RSeqStr[8];
				if (!pCall->RSeq) pCall->RSeq=1717;//YANNIS TO DO
 				sprintf(RSeqStr, "%d",pCall->RSeq);
			 	osip_message_set_header(pAnswer,"RSeq", RSeqStr);
 				pCall->RSeq++;
			}
 		} 

  		ret = eXosip_call_send_answer(pCall->tid, 180,pAnswer);
		if (ret==0)
		{	
			pCall->state = CALL_STATE_RINGING;
		}else osip_message_free (pAnswer);
 	}

	eXosip_unlock ();
	return ret;
}

int si_ua_acceptCall( sicall_t* pCall)
{
	int ret, i;
	osip_message_t *pAnswer;
	char *sdpbody=NULL;
	SICORE* pCore = &siphoneCore;
   	char LocalAudioBuffer[16];
	char PublicAudioBuffer[16];

	if (pCall==NULL) return -1;
 
	si_ua_setpreferred(pCall->accountID ,pCall->codectype, pCore  ,&sdpNegotiationData[pCall->accountID]);
	eXosip_lock ();
	//fix 31 March 2009  
	pCall->state = CALL_STATE_ANSWERING;
	ret = eXosip_call_build_answer (pCall->tid, 200, &pAnswer);

	if (ret != 0)
	{
		pCall->state = CALL_STATE_REJECTING;
		eXosip_call_send_answer (pCall->tid, 400, NULL); //YPAPA
		eXosip_unlock ();
		return -1;
	}
	if (pCall->SDPEnchanged == SIPUA_OFFERANSWER_OFFER) //SIPIT23
	{
 
		sdpbody = build_sdp_answer(pCore, pCall);
		 
		pCall->SDPEnchanged = SIPUA_OFFERANSWER_ANSWER;
	}else{

		for (i=0;i<MAX_SUPPORTED_CODECS;i++)
			siphoneCore.Codecs[pCall->accountID][i]= siphoneCore.CodecsSupported[pCall->accountID][i];

		eXosip_sdp_negotiation_init(pCall->accountID,  &sdpNegotiationData[pCall->accountID], &siphoneCore); 
   		//Select and audio port for the outgoing call (local audio port or public audio port)
		select_audio_port(pCore, pCall, LocalAudioBuffer, PublicAudioBuffer) ;
		si_ua_setpreferred(pCall->accountID ,pCall->codectype, pCore  ,&sdpNegotiationData[pCall->accountID]);
 		sdpbody = eXosip_create_sdpbody_offer(pCall, NULL, &sdpNegotiationData[pCall->accountID], LocalAudioBuffer,PublicAudioBuffer,0);
  		pCall->SDPEnchanged = SIPUA_OFFERANSWER_OFFER;

   	}

	if (sdpbody /*|| pCall->SDPEnchanged == SIPUA_OFFERANSWER_ANSWER*/) //SIPIT23
	{
		if (sdpbody)
		 osip_message_set_body (pAnswer, sdpbody, strlen (sdpbody));

		osip_message_set_content_type (pAnswer, "application/sdp");
		osip_message_set_allow(pAnswer, SipAllowHeader);

		if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported(pAnswer,SIPHONE_UA_SUPPORTED_HEADER_ACK200_wTIMER );
			// SESSION TIMER 
			if (pCall->updateKeeper && pCall->initiator) 
				osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uac");
			else  if (pCall->updateKeeper && !pCall->initiator) 
				osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uas");
			else if (!pCall->updateKeeper && pCall->initiator) 
				osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uas");
			else osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uac");

			osip_message_set_header(pAnswer, "Min-SE", "300");
		}else  osip_message_set_supported(pAnswer,SIPHONE_UA_SUPPORTED_HEADER_ACK200 );

  eXosip_call_send_answer (pCall->tid, 200, pAnswer);
	if (pCall->SDPEnchanged == SIPUA_OFFERANSWER_ANSWER)
	    openRTPSocketOnCallAccept(pCall, NULL); //v1.0.1.1
  eXosip_unlock ();
  #ifdef FIX_MEMORY_LEAK
  if (sdpbody) 
   osip_free(sdpbody);
  #endif
       return 0;
  }
  else
 {
  osip_message_free (pAnswer);
  eXosip_call_send_answer (pCall->tid, 488, NULL); //TO CHECK status code 488, 415
  eXosip_unlock ();
  pCall->state = CALL_STATE_REJECTING;
 
   return 488;
  }
 
 return 0;
}
//v1.0.1.1
int	openRTPSocketOnCallAccept(sicall_t* pCall,eXosip_event_t *pEvent)
{
  	int ret;
	sdp_message_t *pSDP=NULL;

 	if (pEvent){
 		pSDP= eXosip_get_sdp_info(pEvent->ack );
	}
	else{
	 	pSDP = eXosip_get_remote_sdp_from_tid(pCall->tid);
	}
  	if (pSDP == NULL) 
	{
         si_ua_terminateCall(pCall);
		return -1;
	}
	check_incoming_sdp(pCall, pSDP, 2);

 	//fix 10 Sept (MCU)
   	ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],pSDP, &pCall->sStreamParameters);
   	pCall->state=CALL_STATE_CONNECTED;
    ret=si_manager_session_addcall( &siphoneCore, pCall,  pSDP); 	
	if (ret) //SIPIt 25
	{
         si_ua_terminateCall(pCall);
		return -1;
 	}
   	create_updatestatus_callback( CCFSM_ANSWERED_CB, pCall->accountID, pCall->callid, (int)(isSRTPEnabled(pCall) | (int)pCall->sStreamParameters.payload), 0, "", CCFSM_STATE_TALK);
    sdp_message_free( pSDP ) ;
	return 0;
}

int	modifyCallParameters(sicall_t* pCall,eXosip_event_t *pEvent);
int	modifyCallParameters(sicall_t* pCall,eXosip_event_t *pEvent)
{
  	int ret;
	unsigned char remoteaddress[16];
	int remoteport;
	sdp_message_t *pSDP=NULL;
 
 	if (pEvent)	pSDP= eXosip_get_sdp_info(pEvent->ack );
   	if (pSDP == NULL) return -1;
  
  	ret = get_c_address_port(pSDP, remoteaddress,&remoteport);
	if (ret) {si_print(PRINT_LEVEL_ERR, "Failed to analyze SDP(modifyCallParameters) 1\n\n ");return -1;}
  	ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],pSDP, &pCall->sStreamParameters);
 	if (ret) {si_print(PRINT_LEVEL_ERR, "Failed to analyze SDP(modifyCallParameters) \n\n ");return -1;}
   	ret = si_ua_checkSDPForChanges(pSDP, pCall,&pCall->sStreamParameters, 1, remoteaddress,remoteport);
   	sdp_message_free( pSDP ) ;
	return 0;
}

#define D1  '"'
 
// fix 31 March 2009
#include <arpa/inet.h> 
int si_ua_redirect(sicall_t *pCall, eXosip_event_t *pEvent, char *redirectionid, char* reason)
{
   osip_message_t *pAnswer;
 osip_to_t *prevContact;
 int tid= pCall->tid;
 char tmpString[256];
 char tmpString1[256];
  int ret;
   SICORE* pCore = &siphoneCore;

   eXosip_lock ();
    ret = eXosip_call_build_answer (tid, 302, &pAnswer);
 if (ret!=0){
    eXosip_call_send_answer (tid, 400, NULL); //YPAPA
    eXosip_unlock ();
    return -1;
    }
 pCall->state=CALL_STATE_REDIRECTING;
   prevContact=osip_message_get_to(pEvent->request);
  
 //YPAPA  
    if (prevContact->url->port)
   sprintf(tmpString1,"<%s:%s@%s:%s>;reason=%c%s%c",prevContact->url->scheme,prevContact->url->username,prevContact->url->host,prevContact->url->port,D1,"no-answer",D1);
   else 
   sprintf(tmpString1,"<%s:%s@%s>;reason=%c%s%c",prevContact->url->scheme,prevContact->url->username,prevContact->url->host,D1,reason,D1);

//  ret = osip_message_set_contact (pAnswer, redirectionid);
  if (strstr(redirectionid, pCore->RegistrarIPAddress))
 {
  if (pCore->remoteRegistrarPort>0)
   sprintf(tmpString,"<sip:%s:%d>",redirectionid,pCore->remoteRegistrarPort); //format user@server
  else 
   sprintf(tmpString,"<sip:%s",redirectionid);  
  }else{
  if (inet_addr(redirectionid)==-1)   //format user
  {
   if (pCore->remoteRegistrarPort>0)
    sprintf(tmpString,"<sip:%s@%s:%d>",redirectionid, pCore->RegistrarIPAddress,pCore->remoteRegistrarPort);
   else
    sprintf(tmpString,"<sip:%s@%s>",redirectionid, pCore->RegistrarIPAddress);
  }
  else{
   if (pCore->remoteRegistrarPort>0)
    sprintf(tmpString,"<sip:%s:%d>",redirectionid, pCore->remoteRegistrarPort); //format ipaddress
   else
    sprintf(tmpString,"<sip:%s>",redirectionid);
   }
  }
   // ret = osip_message_set_contact (pAnswer, tmpString);
    ret = osip_message_set_header (pAnswer,"Contact" ,tmpString);
   osip_message_set_header (pAnswer, "Diversion", tmpString1);
 
   ret = eXosip_call_send_answer(tid, 302, pAnswer);
   eXosip_unlock ();
   create_updatestatus_callback( CCFSM_REDIRECTED_CB, pCall->accountID,pCall->callid, 0,0,"", CCFSM_STATE_NONE);
   return ret;
}

void create_updatestatus_callback(int id, int accountid, int callid, int parameter, int reason, char*info, int state)
{
 	ccfsm_updatestatus_cback_data m_cback_updatestatus ; 

	m_cback_updatestatus.attachedentity = findEntity(accountid);
	m_cback_updatestatus.cback_id = id;
	m_cback_updatestatus.accountid = accountid; 
	m_cback_updatestatus.callid = callid; 
	m_cback_updatestatus.state = state;
	m_cback_updatestatus.parameter  = parameter;
 	m_cback_updatestatus.reasoncode = reason;
	if (info)
		strcpy(m_cback_updatestatus.info , info);
	else m_cback_updatestatus.info[0]=0;
	ccfsm_callback(&m_cback_updatestatus);
}

int si_ua_rejectCall(sicall_t* pCall)
{
	int ret;
 	eXosip_lock ();
	ret = eXosip_call_send_answer(pCall->tid, 486, NULL);
	/*if (ret==0)
	{
 		//si_print(PRINT_LEVEL_DEBUG, "\n\n CALL_STATE_REJECTING; 2 \n\n ");
  		//pCall->state = CALL_STATE_REJECTING;
 	}
	*/
 	InitiateCall(pCall);
	eXosip_unlock ();
	return ret;		
}
///////////////////////////////////////////////////////////////////////////////
//				OUTGOINING CALL SECTION
//
//
///////////////////////////////////////////////////////////////////////////////

int si_ua_callEstablish( int accountID, int port, int vline, int codec, char *fromURI, char*toURI, int IPCall, sicall_t *rCall)
{
	int i;
	char LocalAudioBuffer[16];
	char PublicAudioBuffer[16];
	int ret;	
	char *route=NULL;
	char proxy[64];
	char from[64];
	char to[64];
	char *sdpbody;
	sicall_t *pCall;
	SICORE* pCore = &siphoneCore;
	osip_message_t *invite =NULL;
 
  if (IPCall!=CCFSM_DIRECTION_OUT_IPCALL) 
	{ 
 		if (!pCore->siregistration[accountID].state) 
  		{	
  			sprintf(proxy, "sip:%s@%s", toURI, pCore->RegistrarIPAddress);
		 	route = proxy;//pCore->RegistrarIPAddress; (to check sipit23) 
 		} 
 	}
     	
	if (toURI==NULL ) return -1; 
 	pCall = allocateCallId(NULL);
  	if (pCall == NULL) return -1;
 	pCall->callid = vline;
  	if (IPCall!=CCFSM_DIRECTION_OUT_IPCALL) 
		pCall->directIP = 0;
	else 
		pCall->directIP = 1;

 	pCall->initiator =  1;
	pCall->updateInterval=0;
 	pCall->accountID =  accountID;
	pCall->port =  port;
	pCall->codectype = codec;
	pCall->sStreamParameters.payload=-1; 
	pCall->SDPEnchanged = SIPUA_OFFERANSWER_IDLE;
 	pCall->session_id=pCore->session_version;
	pCall->session_version=pCore->session_version;
	pCall->state = CALL_STATE_NEW; //FIX 10082009
	pCore->session_version+=10000;
	
	if (rCall)
	{
  		pCall->rCall=(void*)rCall;
		rCall->tCall= (void*)pCall;
	}else pCall->rCall=0;
 
	eXosip_lock();
	sprintf(to,"%s",toURI);
 //FIX YPAPA
  // build a registration packet
  if (IPCall!=CCFSM_DIRECTION_OUT_IPCALL)  
	{
		if (!pCore->IPv6)
		{
	 		if (fromURI)	
	 	 		sprintf(from,"sip:%s@%s",fromURI, pCore->RegistrarIPAddress); 
	 		else 
				sprintf(from,"sip:%s@%s",&siphoneCore.regusername[accountID][0], pCore->RegistrarIPAddress);	

			if (pCore->remoteRegistrarPort>0)
				sprintf(to,"%s:%d",toURI,  pCore->remoteRegistrarPort); 
		}else{

	 		if (fromURI)	
	 	 		sprintf(from,"sip:%s@[%s]",fromURI, pCore->RegistrarIPAddress); 
	 		else 
				sprintf(from,"sip:%s@[%s]",&siphoneCore.regusername[accountID][0], pCore->RegistrarIPAddress);	

			if (pCore->remoteRegistrarPort>0)
				sprintf(to,"%s:[%d]",toURI,  pCore->remoteRegistrarPort); 
		}
	 }else{
		if (!pCore->IPv6)
		{
			if (!pCore->hideid[accountID])
			{
	 			if (fromURI)
		 			sprintf(from,"sip:%s@%s",fromURI, siphoneCore.IPAddress); 
				else 
					sprintf(from,"sip:%s@%s",&pCore->regusername[accountID][0], siphoneCore.IPAddress); 
			}else
				 sprintf(from,"sip:%s","anonymous@anonymous.invalid"); 

 			if (pCore->remoteRegistrarPort>0)
				sprintf(to,"%s:%d",toURI, pCore->remoteRegistrarPort); 
		}else{
			if (!pCore->hideid[accountID])
			{
	 			if (fromURI)
		 			sprintf(from,"sip:%s@[%s]",fromURI, siphoneCore.IPAddress); 
				else 
					sprintf(from,"sip:%s@[%s]",&pCore->regusername[accountID][0], siphoneCore.IPAddress); 
			}else
				 sprintf(from,"sip:%s","anonymous@anonymous.invalid"); 

 			if (pCore->remoteRegistrarPort>0)
				sprintf(to,"%s:%d",toURI, pCore->remoteRegistrarPort); 

		  }
   	}

	  ret = eXosip_call_build_initial_invite(&invite, to, from, route, NULL);

	  if (ret!=0)
	  { 
		 FatalError(EXOSIP_BUILD_INVITE_FAILED, "eXosip_call_build_invite"); 
		 eXosip_unlock();
		 pCall->cid=-1;
		 return -1;
	  } 
 	 //fix 31 March 2009	
  	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
 		siphoneCore.Codecs[pCall->accountID][i]= siphoneCore.CodecsSupported[pCall->accountID][i];
   	eXosip_sdp_negotiation_init(pCall->accountID,  &sdpNegotiationData[pCall->accountID], &siphoneCore); 
 	// 	eXosip_guess_localip (AF_INET, localip, 64);
	//Select and audio port for the outgoing call (local audio port or public audio port)
#if 1
	select_audio_port(pCore, pCall, LocalAudioBuffer, PublicAudioBuffer) ;
 	si_ua_setpreferred(pCall->accountID ,pCall->codectype, pCore  ,&sdpNegotiationData[pCall->accountID]);
 	sdpbody = eXosip_create_sdpbody_offer(pCall, invite, &sdpNegotiationData[pCall->accountID], LocalAudioBuffer,PublicAudioBuffer,1);
 	if (sdpbody)
	{
		pCall->SDPEnchanged = SIPUA_OFFERANSWER_OFFER;
		if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER); 
			char refresher[24];
			char minse[8];
			// SESSION TIMER 
			sprintf(refresher, "%d;refresher=%s",  siphoneCore.SessionExpireTimer,"uas");
 			osip_message_set_header(invite, "Session-Expires", refresher); 
 			sprintf(minse, "%d",  siphoneCore.MinSessionExpireTimer);
			osip_message_set_header(invite, "Min-SE", minse);
			pCall->maxSessionExpireTime = siphoneCore.SessionExpireTimer;
			pCall->minSessionExpireTime = siphoneCore.MinSessionExpireTimer;
			//??????? require
 		}else {
 			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE);
		}
		osip_message_set_content_type (invite, "application/sdp");
		osip_message_set_allow (invite, SipAllowHeader);
		osip_message_set_body (invite,sdpbody, strlen (sdpbody));
		if (pCore->hideid[accountID])
		{
			char Identity[64];
			if (fromURI)	
		 	 	sprintf(Identity,"sip:%s@%s",fromURI, pCore->RegistrarIPAddress); 
		 	else 
				sprintf(Identity,"sip:%s@%s",&siphoneCore.regusername[accountID][0], pCore->RegistrarIPAddress);	
	 		osip_message_set_header(invite, "Privacy", "id");						 
			osip_message_set_header(invite, "P-Asserted-Identity", Identity);						 
			
		}
		osip_free(sdpbody);
   	}else
	{
		FatalError(EXOSIP_SEND_INVITE_FAILED, "eXosip_call_send_invite"); 
		eXosip_unlock();
		pCall->cid=-1;
	   return -1;
 	}
#endif
/*	int k,length;
	char *message;
    osip_message_to_str (invite, &message, &length);
	printf("OSIP MESSAFE");
	for (k=0;k<length;k++)
		printf("%c", message[k]);
*/
	ret = eXosip_call_send_initial_invite(invite);
  
	if (ret<0)
	{
 	   FatalError(EXOSIP_SEND_INVITE_FAILED, "eXosip_call_send_invite"); 
 	   eXosip_unlock();
	   pCall->cid=-1;
	   return -1;
	} 
 	pCall->cid= ret;
 	pCall->did= 0;
 	pCall->tid= 0;

  	//int setTimeout(int duration, char* reason);
	//setTimeout(1000,"CALL_STATE_CALLING");
	pCall->state=CALL_STATE_CALLING;

	// 	sprintf(reference,"line-%d", vline);
	//  	eXosip_call_set_reference(ret,reference);//reference);
    eXosip_unlock();
	 
    return 0;
}
 
 
int si_ua_Invite_w_Replace( int accountID, int port, int vline,int codec ,char *fromURI, char*toURI, int IPCall, sicall_t *rCall,char *referredBy, char *replaces)
{
	int i;
	char LocalAudioBuffer[16];
	char PublicAudioBuffer[16];
 	char reference[16];
	int ret;	
	char *route=NULL;
	char proxy[64];
	char from[64];
	char to[64];
	char *sdpbody;
 	char localip[64];
	sicall_t *pCall;
	SICORE* pCore = &siphoneCore;
	osip_message_t *invite =NULL;

  if (IPCall!=CCFSM_DIRECTION_OUT_IPCALL) 
	{ 
 		if (!pCore->siregistration[accountID].state) 
  		{	
   			sprintf(proxy, "sip:%s@%s", toURI, pCore->RegistrarIPAddress);
		 	route = proxy;//pCore->RegistrarIPAddress;
 		} 
 	}
  
	if (toURI==NULL ) return -1; 
	pCall = allocateCallId(NULL);
 	if (pCall == NULL) return -1;   
	pCall->callid = vline;
	if (IPCall!=CCFSM_DIRECTION_OUT_IPCALL) 
		pCall->directIP = 0;
	else 
		pCall->directIP = 1;

	pCall->initiator =  1;
	pCall->updateInterval=0;
	pCall->accountID =  accountID;
	pCall->port =  port;
	pCall->sStreamParameters.payload=-1; 
		pCall->SDPEnchanged = SIPUA_OFFERANSWER_IDLE;

	pCall->state = CALL_STATE_NEW; //FIX 10082009
	pCall->codectype = codec;
	pCall->session_id=pCore->session_version;
	pCall->session_version=pCore->session_version;
	pCore->session_version++;

	if (rCall)
	{
 
  		pCall->rCall=(void*)rCall;
		rCall->tCall= (void*)pCall;
	}else pCall->rCall=0;
  
	eXosip_lock();
    // build a registration packet
  	  strcpy(to,toURI); 

   	if (IPCall!=CCFSM_DIRECTION_OUT_IPCALL) 
	{
 	 	if (fromURI)
	 	 	sprintf(from,"sip:%s@%s",fromURI, pCore->RegistrarIPAddress); 
	 	else 
			sprintf(from,"sip:%s@%s",&siphoneCore.regusername[accountID][0], pCore->RegistrarIPAddress); 
  
		si_print(PRINT_LEVEL_DEBUG, ">>>>>>>>>>>>>>>>> INVITE with Replase = %d \n ", toURI);
	 	//sprintf(to,"%s@%s",toURI,  pCore->RegistrarIPAddress); 
 	} else
	{
 		if (!pCore->hideid[accountID])
		{
			if (fromURI)
		 		sprintf(from,"sip:%s@%s",fromURI, siphoneCore.IPAddress); 
			else 
				sprintf(from,"sip:%s@%s",&pCore->regusername[accountID][0], siphoneCore.IPAddress); 
		}else
			 sprintf(from,"sip:%s","anonymous@anonymous.invalid"); 
   }

 	si_print(PRINT_LEVEL_DEBUG, "=============[%s]IPCall=[%d] to = [%s], from = [%s], toURI=[%s] REPLACES = [%s]\n\n", __FUNCTION__, IPCall,to, from, toURI, replaces);	
  	ret = eXosip_call_build_initial_invite(&invite,to,from, route, NULL);
	if (ret!=0)
	{ 
 	   FatalError(EXOSIP_BUILD_INVITE_FAILED, "eXosip_call_build_invite"); 
 	   eXosip_unlock();
	   pCall->cid=-1;
	   return -1;
	} 
	
 	//fix 31 March 2009	
  	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
 		siphoneCore.Codecs[pCall->accountID][i]= siphoneCore.CodecsSupported[pCall->accountID][i];
   	eXosip_sdp_negotiation_init(pCall->accountID,  &sdpNegotiationData[pCall->accountID], &siphoneCore); 
  

 	eXosip_guess_localip (AF_INET, localip, 64);
 	//Select and audio port for the outgoing call (local audio port or public audio port)
	select_audio_port(pCore, pCall, LocalAudioBuffer, PublicAudioBuffer) ;
 	si_ua_setpreferred(pCall->accountID ,pCall->codectype, pCore  ,&sdpNegotiationData[pCall->accountID]);
    sdpbody = eXosip_create_sdpbody_offer(pCall, invite, &sdpNegotiationData[pCall->accountID], LocalAudioBuffer,PublicAudioBuffer,1);
  	if (sdpbody)
	{
		pCall->SDPEnchanged = SIPUA_OFFERANSWER_OFFER;
 		osip_message_set_header(invite, "Replaces", replaces);
		osip_message_set_header(invite, "Referred-By",referredBy);
		osip_message_set_header(invite, "Require", "replaces");
  	 	osip_message_set_allow (invite, SipAllowHeader);
	 
		if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER); 
			// SESSION TIMER 
			char refresher[24];
			char minse[8];
			// SESSION TIMER 
			sprintf(refresher, "%d;refresher=%s",  siphoneCore.SessionExpireTimer,"uas");
 			osip_message_set_header(invite, "Session-Expires", refresher); 
 			sprintf(minse, "%d",  siphoneCore.MinSessionExpireTimer);
			osip_message_set_header(invite, "Min-SE", minse);
  		}else {
			osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE);
		}
		osip_message_set_header(invite, "Content-Type", "application/sdp");
		osip_message_set_body (invite,sdpbody, strlen (sdpbody));	
    osip_free(sdpbody);
  	}else
	{
  	 FatalError(EXOSIP_SEND_INVITE_FAILED, "eXosip_call_send_invite"); 
 	   eXosip_unlock();
	   pCall->cid=-1;
	   return -1;
 	}
  	ret = eXosip_call_send_initial_invite(invite);
	if (ret<0)
	{
 	   FatalError(EXOSIP_SEND_INVITE_FAILED, "eXosip_call_send_invite"); 
 	   eXosip_unlock();
	   pCall->cid=-1;
	   return -1;
	} 
 	pCall->cid= ret;
 	pCall->did= 0;
 	pCall->tid= 0;
  	sprintf(reference,"line-%d", vline);
  	eXosip_call_set_reference(ret,reference);//reference);
	pCall->state=CALL_STATE_CALLING;//fix 0607098
    eXosip_unlock();
 	return 0;
 }
int  si_ua_recv_callProceeding(sicall_t *pCall, eXosip_event_t *pEvent)
{
  	if (!pCall) {
		si_print(PRINT_LEVEL_ERR, "NULL CALL while receiving %s", pEvent->textinfo);		
		return -1;
	}
  	if (pCall->state == CALL_STATE_CALLING) 
	{
	 	pCall->cid=pEvent->cid;
		pCall->tid=pEvent->tid;
		pCall->did=pEvent->did;
  		//inform user for current call state
 //  	    create_updatestatus_callback( CCFSM_PROCEEDING_CB, pCall->accountID,pCall->callid, 0, pEvent->response->status_code,pEvent->textinfo, CCFSM_STATE_NONE);
    } 

  	return 0;
}
int si_ua_recv_ringing(sicall_t *pCall, eXosip_event_t *pEvent)
{
 	int ret;
	if (!pCall) {
 		return -1;
	}
	pCall->cid=pEvent->cid;
	pCall->tid=pEvent->tid;
	pCall->did=pEvent->did;

 	//inform user for current call state

	
	if (pCall->state == CALL_STATE_CALLING) 
	{
		if (pEvent->response->status_code==183) 
		{			
			sdp_message_t *sdpRemote=NULL;
 			si_alsa_stop_tone(pCall);
   			pCall->state = CALL_STATE_CONNECTED;
 			if (pEvent->request->from->url)
 			{
  				if (pEvent->request->from->url->host)
				{
 				 	strcpy(pCall->remoteHost,pEvent->request->from->url->host); 
 				}
   			} 
 			sdpRemote= eXosip_get_sdp_info(pEvent->response);
 			if (sdpRemote==NULL) {
				si_ua_terminateCall(pCall);
				si_print(PRINT_LEVEL_ERR, "Failed to retrieve SDP from the current incoming EVENT (%d) \n\n ",pEvent->did);
				return -1;
			}
 			//fix 10 Sept (MSU)
  			ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdpRemote, &pCall->sStreamParameters);
   			ret  = si_manager_session_addcall( &siphoneCore, pCall,  sdpRemote); 	//calling (183)
			if (ret) //SIPIt 25
			{
		        si_ua_terminateCall(pCall);
				return -1;
		 	}

		 	// fix: Memory Leak (06052008)
		  	if (sdpRemote) sdp_message_free(sdpRemote);
 			 // change FIX BROADWORKS
			checkForReliableResponse(pCall, pEvent->response);
  			create_updatestatus_callback( CCFSM_ANSWERED_CB, pCall->accountID,pCall->callid,  0, pEvent->response->status_code, pEvent->textinfo,CCFSM_STATE_RINGING); //fix 31 March 2009
		
			return 0;
		}
	  	else  {
			/* fix April 14, 2009
			sdp_message_t *sdpRemote=NULL;
 			sdpRemote= eXosip_get_sdp_info(pEvent->response);
 			if (sdpRemote) // SIPIT23 // Invite noSDP
			{
		  		sdp_message_free(sdpRemote);
 			}*/
 	 	 	pCall->ToneInProgress =(int)1;
	 	 	si_start_tone_ringing(pCall);
	 	}  
	}else if (pCall->state == CALL_STATE_CONNECTED) 
	{
		if ((pEvent->response->status_code==180) || (pEvent->response->status_code==183))
		{
			sdp_message_t *sdpRemote=NULL;
			sdpRemote= eXosip_get_sdp_info(pEvent->response);
			if (sdpRemote)	
			{
			  audio_stream_parameters tempStreamParameters;
				int remoteport=-1;
				unsigned char remoteaddress[16];

				int ret;
				ret= getRemoteSDPInfo(sdpRemote, remoteaddress, &remoteport);
 				ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdpRemote, &tempStreamParameters);
				if (ret) {si_print(PRINT_LEVEL_ERR, "Failed to get SDP \n\n ");return ret;}
 				si_ua_checkSDPForChanges(sdpRemote, pCall,&tempStreamParameters, 0, remoteaddress, remoteport);
  				sdp_message_free(sdpRemote);
 				checkForReliableResponse(pCall, pEvent->response);
				create_updatestatus_callback( CCFSM_ANSWERED_CB, pCall->accountID,pCall->callid,  0, pEvent->response->status_code, pEvent->textinfo,CCFSM_STATE_RINGING); //fix 31 March 2009
 			}
 		}
 	}
 
	// change FIX BROADWORKS
  	checkForReliableResponse(pCall,pEvent->response); 			
  	create_updatestatus_callback( CCFSM_RINGING_CB, pCall->accountID,pCall->callid,  0, 0,"",CCFSM_STATE_RINGING);
	return 0;
}

osip_message_t * si_ua_build_reply_on_ACK200(sicall_t *pCall)
{
   	osip_message_t *pAck;
	char *sdpbody=NULL;
	SICORE* pCore = &siphoneCore;
	int i;

	 
 
	i = eXosip_call_build_ack (pCall->did, &pAck);
	if (i != 0)
	{
 	  return NULL;
	}

	sdpbody = build_sdp_answer(pCore, pCall);
 	
	if (sdpbody)
	 	osip_message_set_body (pAck, sdpbody, strlen (sdpbody));
	else 
		return NULL;

   osip_free(sdpbody);
 
 	osip_message_set_content_type (pAck, "application/sdp");
 	osip_message_set_allow(pAck, SipAllowHeader);
	return pAck;
}

osip_message_t * si_ua_recv_callAnswered(sicall_t *pCall, eXosip_event_t *pEvent)
{
	osip_message_t *replyOnACK200=NULL;
 
 	int did;
	int ret;
  sdp_message_t *sdpRemote = NULL;

  if (!pCall) {
		si_print(PRINT_LEVEL_ERR, "NULL CALL while receiving %s", pEvent->textinfo);		
		return NULL;
	}
  	 	
	pCall->cid=(int)pEvent->cid;
	pCall->tid=(int)pEvent->tid;
	did = pCall->did;
	pCall->did=(int)pEvent->did;

	sdpRemote= eXosip_get_sdp_info(pEvent->response);
	if (sdpRemote)
		check_incoming_sdp(pCall, sdpRemote, 3);

  	//inform user for current call state
 	switch(pCall->state)
 	{		
		case CALL_STATE_CALLING:
        	pCall->state = CALL_STATE_CONNECTED;
 			si_alsa_stop_tone(pCall);
  			si_sessiontimer_update(pCall,pEvent->response);
  			if (pEvent->response->to->url)
			{
				if (pEvent->response->to->url->host)
				{
   				 	if (strlen(pEvent->response->to->url->host)>15)
					{
						strncpy(pCall->remoteHost,pEvent->response->to->url->host, 15); 
						pCall->remoteHost[15]='\0';
					}
					else
						strcpy(pCall->remoteHost,pEvent->response->to->url->host); 
  				}
   			} 
  			if (pCall->rCall)
			{
			   sicall_t *referCall=pCall->rCall;
				if (referCall->callTransferInProgress)
				{
 					switch(referCall->callTransferInProgress)
					{
					case SIPUA_TRANSFEREE_BLINK:
   					   si_ua_referNotify(referCall->did, 200, "OK", 1);
					   referCall->tCall=NULL;
 					   si_ua_terminateCall(referCall);
 					   pCall->rCall = NULL;
					   referCall->callTransferInProgress = 0;
					   //TOCHECK
					   create_updatestatus_callback(CCFSM_TRANSFER_ACCEPTED_CB, pCall->accountID, pCall->callid, 0, 0,"", CCFSM_STATE_TALK);
					   break;
					case SIPUA_TRANSFEREE_ATTENDED_TRANSFERRING:
						referCall->callTransferInProgress = 0;
						pCall->rCall = NULL;
  						si_ua_referNotify(referCall->did, 200, "OK", 1);
 						//TOCHECK
  						create_updatestatus_callback(CCFSM_TRANSFER_ACCEPTED_CB, pCall->accountID, pCall->callid, 0, 0,"", CCFSM_STATE_TALK);
						break;
       				}
 				}
			}
 			sdpRemote= eXosip_get_sdp_info(pEvent->response);
 			if (sdpRemote==NULL) {
				si_print(PRINT_LEVEL_ERR, "Failed to retrieve SDP from the current response \n\n ");
  				sdpRemote= eXosip_get_remote_sdp_from_tid(pEvent->tid);
 				if (sdpRemote==NULL) {
	 				sdpRemote= eXosip_get_remote_sdp(pEvent->did);
					if (sdpRemote==NULL) {
 						si_ua_terminateCall(pCall);
						si_print(PRINT_LEVEL_ERR, "Failed to retrieve SDP from the current incoming EVENT \n\n ");
						return NULL;
					}
				}
			}else check_incoming_sdp(pCall, sdpRemote, 4);
 			if (pCall->SDPEnchanged == SIPUA_OFFERANSWER_IDLE) 
			{
				replyOnACK200 = si_ua_build_reply_on_ACK200(pCall);
 				if (replyOnACK200==NULL)	
				{
					//fix 04082009
 					 si_ua_terminateCall(pCall);
  					 return NULL;//fix 04082009
				}
			  }
				//fix 10 Sept (MSU)
				ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdpRemote, &pCall->sStreamParameters);
				sdp_get_first_crypto_option(&sdpNegotiationData[pCall->accountID],sdpRemote,&pCall->crypto_parameters);
				create_updatestatus_callback(CCFSM_ANSWERED_CB, pCall->accountID, pCall->callid,  (int)( isSRTPEnabled(pCall) | pCall->sStreamParameters.payload),pEvent->response->status_code,pEvent->textinfo,CCFSM_STATE_TALK);
				ret = si_manager_session_addcall( &siphoneCore, pCall,  sdpRemote); 	//calling (180)
				if (ret) //SIPIt 25
				{
					si_ua_terminateCall(pCall);
					return NULL;
	 			}

				// create_updatestatus_callback(CCFSM_ANSWERED_CB, pCall->accountID, pCall->callid,  (int)pCall->sStreamParameters.payload,pEvent->response->status_code,pEvent->textinfo,CCFSM_STATE_TALK);
				// fix: Memory Leak (06052008)
				if (sdpRemote) sdp_message_free(sdpRemote);
				return replyOnACK200;
		break;
		case CALL_STATE_CONNECTED:
			 si_sessiontimer_update(pCall,pEvent->response);
//			if (pCall->codecChange) //fix 10 Sept (MCU)
			 // TOFIX , TOCHECK
			if (!pCall->faxsupport && (pCall->localHold!=(int)2) && (pCall->localHold!=(int)3)) 
 			{
				sdpRemote= eXosip_get_sdp_info(pEvent->response);
				if (sdpRemote)	
				{
			  		audio_stream_parameters tempStreamParameters;
					int remoteport=-1;
					unsigned char remoteaddress[16];
					
					int ret;
					ret= getRemoteSDPInfo(sdpRemote, remoteaddress, &remoteport);
 					ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],sdpRemote, &tempStreamParameters);
					if (ret) {si_print(PRINT_LEVEL_ERR, "Failed to get SDP \n\n ");return NULL;}
 					si_ua_checkSDPForChanges(sdpRemote, pCall,&tempStreamParameters, 1, remoteaddress, remoteport);
  					sdp_message_free(sdpRemote);
 					checkForReliableResponse(pCall, pEvent->response);
	 
		   			pCall->codecChange=(int)0; 
				}
			}
 
  			if (pCall->localrefer)
			{ 
  			    if (pCall->localHold==(int)2)
			    {
					si_canceltimerByFunction(&holdOnTimerCallBack);
 					 pCall->localHold=1;
					 si_ua_transmit_Xrefer(pCall);
			     }else 	if (pCall->localHold==(int)3)
			    {
					si_canceltimerByFunction(&holdOffTimerCallBack);
  					si_audio_stream_resume( pCall);
   					pCall->localHold=0;
					pCall->localrefer = 0;

			     }
  			}else if (pCall->callTransferInProgress)
			{
	 
				switch(pCall->callTransferInProgress)
				{
				case SIPUA_TRANSFEREE_BLINK:
   					if (pCall->localHold==(int)2)
					{
 					  si_canceltimerByFunction(&holdOnTimerCallBack);
 					  pCall->localHold=1;
			 		  // call transfer in progress
			  		  //memcpy(from.host,pCall->transfer_to, sizeof(from.host));  
					  si_print(PRINT_LEVEL_DEBUG, "SIPUA_TRANSFEREE_BLINK %s \n",pCall->transfer_to);
  					  //si_cb_callback1(pCall->cbackId,(int)pCall->sStreamParameters.payload,SI_CALL_BACK_RELEASE_OLD_AND_TRANSFER,pCall->callid,pCall->accountID,0,  &from );
			 		  //create_updatestatus_callback(CCFSM_ANSWERED_CB, pCall->accountID, pCall->callid,  (int)pCall->sStreamParameters.payload,pEvent->response->status_code,pEvent->textinfo,CCFSM_STATE_TALK);
 					}					
					break;
 				}
 
			}else  {
				if (pCall->localHold==(int)2){
					si_canceltimerByFunction(&holdOnTimerCallBack);
 					pCall->localHold=1;
     				 // si_audio_stream_suspend( pCall);
  					create_updatestatus_callback(CCFSM_HOLD_CB, pCall->accountID, pCall->callid,  0,0,"",CCFSM_STATE_HOLD);
  				}else if (pCall->localHold==(int)3){
					si_canceltimerByFunction(&holdOffTimerCallBack);
					 
  					 // si_audio_stream_resume( pCall);
					//si_cb_callback(pCall->cbackId,(int)pCall->sStreamParameters.payload,SI_CALL_BACK_RESUMED,pCall->callid,pCall->accountID,pEvent->response->status_code, "CALL resumed");
  					pCall->localHold=0;
					if (pCall->remoteHold)
						create_updatestatus_callback(CCFSM_RESUMED_CB, pCall->accountID, pCall->callid,  (int)pCall->sStreamParameters.payload,0,"",CCFSM_STATE_HELD);
					else
						create_updatestatus_callback(CCFSM_RESUMED_CB, pCall->accountID, pCall->callid,  (int)(isSRTPEnabled(pCall) | (int)pCall->sStreamParameters.payload),0,"",CCFSM_STATE_TALK);
 				}else if (pCall->faxsupport==2) 
				{
 					ata_t38_sdp_params_type fax_sdp_params;
				  	sdp_message_t *sdpRemote=NULL;

					sdpRemote= eXosip_get_sdp_info(pEvent->response);
  			  		if (sdpRemote==NULL || sdp_get_fax_params(&fax_sdp_params, sdpRemote, &pCall->sStreamParameters))
 					{
						si_print(PRINT_LEVEL_ERR, "Failed to negotiate sdp/fax params \n ");
						si_ua_terminateCall(pCall);
						return NULL;
					}
  					sc1445x_fax_switch_mode(pCall);
				}

			}
 
   			break;
 		default:
 
 			if (pCall->localHold==(int)2){
				 
					si_canceltimerByFunction(&holdOnTimerCallBack);
					pCall->localHold=1;
					//SOS
  					//si_audio_stream_suspend(pCall);
					create_updatestatus_callback( CCFSM_HOLD_CB, pCall->accountID,pCall->callid,(int)pCall->sStreamParameters.payload,  pEvent->response->status_code,"CALL on hold",CCFSM_STATE_HOLD);

 				}
			if (pCall->localHold==(int)3){
					si_canceltimerByFunction(&holdOffTimerCallBack);
					//SOS
 					//si_audio_stream_resume( pCall);
 					pCall->localHold=0;
					if (pCall->remoteHold)
						create_updatestatus_callback( CCFSM_RESUMED_CB, pCall->accountID,pCall->callid,(int)pCall->sStreamParameters.payload,  pEvent->response->status_code,"CALL resumed",CCFSM_STATE_HELD);
					else
						create_updatestatus_callback( CCFSM_RESUMED_CB, pCall->accountID,pCall->callid, (int)(isSRTPEnabled(pCall) | (int)pCall->sStreamParameters.payload), pEvent->response->status_code,"CALL resumed",CCFSM_STATE_TALK);

			}
 	}
 	//si_print(PRINT_LEVEL_DEBUG, "si_ua_recv_callAnswered (return)<%x>\n", (int)pCall);
  return NULL;
}


///////////////////////////////////////////////////////////////////////////////
//				MANAGEMENT CALL SECTION
//
//
///////////////////////////////////////////////////////////////////////////////
int si_ua_transfer(int line, char *tUri)
{
	sicall_t *pCall;
   
	pCall = findCallByLine(line);
 	if (pCall==NULL) return -1;
	return 0;
}
 
int si_ua_attendedTransfer(sicall_t *pCall, sicall_t *pCallToTransfer)  
{
   	eXosip_call_get_referto(pCallToTransfer->did,pCall->transfer_to, 256  );

	if (!pCall->directIP)    
	{
		if (!siphoneCore.hideid[pCall->accountID])
	 		sprintf(pCall->transfer_from,"<sip:%s@%s>",&siphoneCore.regusername[pCall->accountID ][0], siphoneCore.RegistrarIPAddress );
 		else
			sprintf(pCall->transfer_from,"sip:%s","anonymous@anonymous.invalid"); 
	}
	else 
		sprintf(pCall->transfer_from,"<sip:%s>",siphoneCore.IPAddress );

 	si_ua_transmit_Xrefer(pCall);
	return 0;
 }


int si_ua_blindTransfer(sicall_t *pCall, char *remote)
{
	int num_of_dots = 0;
 	char* ptr = remote;
	int isIPAddress=1;
	while(*ptr) {
		if(*ptr == '.')
			num_of_dots++;
		else if(!isdigit(*ptr))
		{isIPAddress=0;break;}
 		ptr++;
	}
 	if(num_of_dots != 3) isIPAddress=0;  

	if (pCall==NULL) return -1;
 
	if (!isIPAddress){  //fix 11082009
 		
		if (!siphoneCore.hideid[pCall->accountID])
	 		sprintf(pCall->transfer_from,"<sip:%s@%s>",&siphoneCore.regusername[pCall->accountID ][0], siphoneCore.RegistrarIPAddress );
 		else
			sprintf(pCall->transfer_from,"sip:%s","anonymous@anonymous.invalid"); 
  
   		sprintf(pCall->transfer_to,"sip:%s@%s",remote, siphoneCore.RegistrarIPAddress );
   		sprintf(pCall->transfer_refer_to,"sip:%s@%s",remote, siphoneCore.RegistrarIPAddress );

  }else {
		if (!siphoneCore.hideid[pCall->accountID])
	 		sprintf(pCall->transfer_from,"<sip:%s>",siphoneCore.IPAddress );
 		else
			sprintf(pCall->transfer_from,"sip:%s","anonymous@anonymous.invalid"); 
 
		sprintf(pCall->transfer_refer_to,"sip:%s",remote);
		sprintf(pCall->transfer_to,"sip:%s",remote);
	}		
  
  	pCall->localrefer =1;
   	if (pCall->localHold==1) 
	{
		si_ua_transmit_Xrefer(pCall);
	}else if (si_ua_holdOn(pCall, 1)) 
	{
		return -1;
	} 
 
 return 0;
}


int si_ua_transmit_Xrefer(sicall_t *pCall)
{ 
  osip_message_t *refer;
 int ret;
 char temp_transfer_to[256];
 
  eXosip_lock();
   sprintf(temp_transfer_to, "<%s>", pCall->transfer_to);
   ret = eXosip_call_build_refer(pCall->did, temp_transfer_to, &refer); 
    if (ret) {
	  pCall->localrefer=0; //YPAPA to unhold
	  eXosip_unlock();return -1;
	 }

	if (strlen(pCall->transfer_from)>0){
	  osip_message_set_header (refer, "Referred-By",pCall->transfer_from);
	}
  //osip_message_set_header (refer, "Refer-To",pCall->transfer_to);
 
 ret= eXosip_call_send_request (pCall->did, refer);
  if (ret) {
		osip_message_free (refer);
		pCall->localrefer=0;//YPAPA to unhold
		eXosip_unlock();return -1;
 }
 //YPAPA change state
  eXosip_unlock();
 return 0;
 }


int si_ua_recv_ack(sicall_t* pCall, eXosip_event_t *pEvent)
{
  	sdp_message_t *sdpRemote=NULL;
 
   	if (!pCall) return -1;
 	sdpRemote= eXosip_get_sdp_info(pEvent->response);
	if (sdpRemote)
		check_incoming_sdp(pCall, sdpRemote, 5);
  
 
  	switch (pCall->state)
	{
   		case CALL_STATE_ANSWERING:
 			openRTPSocketOnCallAccept(pCall, pEvent); 
     		break;
		case CALL_STATE_CONNECTED:
 
			modifyCallParameters(pCall, pEvent); 
			break;
   	}	
	return 0;
}
 
int si_ua_recv_callClosed(sicall_t* pCall, eXosip_event_t *pEvent)
{
	if (pCall==NULL) return -1;
 
 //	si_stop_ringtone(pCall );
   	si_ua_recv_callReleased(pCall, pEvent);
  	return 0;
}

int si_ua_recv_callReleased(sicall_t* pCall, eXosip_event_t *pEvent)
{	
   	int rStatus = 0;
 	incomingInfo_t from;
  	int releaseReason = 0;
	  
	if (!pCall) {
		return -1;}
   
  	if (pCall->state==CALL_STATE_IDLE)return 0;
   	if (pCall->state == CALL_STATE_RELEASING) 
	{
		InitiateCall(pCall); 
		return 0;
	}
  
 	si_alsa_stop_tone(pCall);
  	if (pEvent->response) rStatus = osip_message_get_status_code(pEvent->response);
  	if (pCall->rCall) //close the original close (or re-establish it)
	{
 		sicall_t *rCall;
	  
		rCall = (sicall_t *)pCall->rCall;
		//FIX 10082009
		if ((pCall->callTransferInProgress==SIPUA_TRANSFEREE_BLINK) || (rCall->callTransferInProgress==SIPUA_TRANSFEREE_BLINK))
		{
	 		si_ua_referNotify(rCall->did,rStatus, "Closed", 1 );
 			si_ua_terminateCall(rCall);	
		}else if (pCall->callTransferInProgress==SIPUA_TRANSFEREE_ATTENDED_TRANSFERRING)
		{
			 ;
		}
 		//si_ua_holdOff(rCall->line);
	   pCall->rCall = NULL;
	}else  if (pCall->tCall) // 
	{
  	   sicall_t *tCall;

 
  	   tCall = (sicall_t *)pCall->tCall;
 	   if (tCall->rCall) 	
	   {
 		tCall->rCall=NULL;
	   }
 //	   si_ua_terminateCall(tCall);
	   pCall->tCall = NULL;
	}else if (pCall->callTransferInProgress) 
	{
	  
 		if (pCall->localHold==(int)2)	
		{
  			si_canceltimerByFunction(&holdOnTimerCallBack);
   			pCall->localHold=0;
			pCall->rCall = 0 ;
			//si_ua_referNotify(pEvent->did, 100, "Trying", 1);

			// call transfer in progress but the origin call is terminated
		  // call new destination in any case
			releaseReason = 1;
   		}
	}
	// notify MMI for the current event and call state
  	if (releaseReason == 0)
	{
  		if(pCall->callid!=-1)
		{
 			    sc1445x_mcu_configure(pCall->pMcuHandler,MCU_MCTRL_STOP_ID,NULL);
				usleep(1000);
 				if (pEvent->response) 
				{
 					create_updatestatus_callback( CCFSM_TERMINATED_CB, pCall->accountID,pCall->callid, 0,pEvent->response->status_code,pEvent->response->reason_phrase, CCFSM_STATE_TERMINATED);
 				}
				 else 
 				  	create_updatestatus_callback( CCFSM_TERMINATED_CB, pCall->accountID,pCall->callid, 0, 0,"release", CCFSM_STATE_TERMINATED);
				InitiateCall(pCall);
	   			if (pCall->state>0) {
					if ((pCall->state==CALL_STATE_CALLING)) InitiateCall(pCall);
					else pCall->state = CALL_STATE_RELEASING;
 				}
				return 0;
  		}
 	}else {
  			memcpy(from.host,pCall->transfer_to, sizeof (from.host));  
			si_print(PRINT_LEVEL_DEBUG, "\nTO DO SI_CALL_BACK_RELEASE_OLD_AND_TRANSFER\n");
  	}

   	if (pCall->state>0) {
		if ((pCall->state==CALL_STATE_CALLING)) InitiateCall(pCall);
		else pCall->state = CALL_STATE_RELEASING;

	}
  si_manager_session_closecall(pCall); 
	return 0;
}
 
int si_ua_terminateCall(sicall_t *pCall)
{	
  	int ret;
 	int var1, var2; 
	if (pCall==NULL) return -1;
	var1 =  pCall->accountID;
	var2= pCall->callid;
// 	si_alsa_stop_tone(pCall);
  	eXosip_lock ();
 
	//FIX 10082009
   if (pCall->rCall )  eXosip_call_terminate(((sicall_t *)(pCall->rCall))->cid,((sicall_t *)(pCall->rCall))->did);
    if (pCall->tCall )  eXosip_call_terminate(((sicall_t *)(pCall->tCall))->cid,((sicall_t *)(pCall->tCall))->did);

  	ret = eXosip_call_terminate(pCall->cid,pCall->did);
   	if (ret!=0)
	{
 	   FatalError(EXOSIP_CALL_TERMINATE_FAILED, "eXosip_call_terminate_failed"); 
 	}
   	eXosip_unlock ();
   	si_manager_session_closecall(pCall) ;
	// fix 17 April 2009
	 create_updatestatus_callback( CCFSM_TERMINATED_CB,var1, var2, 0,0,"normal",CCFSM_STATE_TERMINATED);
 
	return ret;
}

int si_ua_callTerminated(eXosip_event_t *pEvent)
{
 	return 0;
}

///////////////////////////////////////////////////////////////////////////////
//				BASIC CALL FUNCTIONs
//
//
///////////////////////////////////////////////////////////////////////////////
void releaseCallId(sicall_t* pCall)
{
	if (pCall==NULL) return ;
 
	if (pCall->state == CALL_STATE_IDLE) return;
	// memset ((char*)pCall, 0, sizeof (sicall_t));
 	//pCall->cbackId=-1;
  
}

int current_call_position =0;
sicall_t* allocateCallId(eXosip_event_t* pEvent)
 {
	int j;
	if (pEvent==NULL) //new outgoing call
	{
		for (j=0;j<MAX_SUPPORTED_CALLS;j++)
		{
			current_call_position++;
			if (current_call_position==MAX_SUPPORTED_CALLS) current_call_position = 0;
 			if (SiCall[current_call_position].state == CALL_STATE_IDLE)
			{
			//	SiCall[current_call_position].cbackId=-1;
				return (sicall_t*)&SiCall[current_call_position];
			} 
		}
		return NULL;
 	}

	for (j=0;j<MAX_SUPPORTED_CALLS;j++)
	{
		current_call_position++;
		if (current_call_position==MAX_SUPPORTED_CALLS) current_call_position = 0;

		if ((SiCall[current_call_position].state == CALL_STATE_IDLE) || (SiCall[current_call_position].cid==pEvent->cid))
		{		
			memset((char*)&SiCall[current_call_position], 0, sizeof(sicall_t));
			 
 			SiCall[current_call_position].cid=pEvent->cid;
			SiCall[current_call_position].tid=pEvent->tid;
			SiCall[current_call_position].did=pEvent->did;
			SiCall[current_call_position].port=(int)-1;
	/* if (pEvent->remote_sdp_audio_ip[0])
			{
	 			strncpy(SiCall[i].remote_sdp_audio_ip, pEvent->remote_sdp_audio_ip, sizeof(SiCall[i].remote_sdp_audio_ip));
	 			SiCall[i].remote_sdp_audio_port = pEvent->remote_sdp_audio_port;
	 			strncpy(SiCall[i].audio_payload_name, pEvent->payload_name, sizeof(SiCall[i].audio_payload_name));
	 			SiCall[i].audio_payload = pEvent->payload;
				si_print(PRINT_LEVEL_DEBUG, "\n NEW CALL:ip[%s]payload[%s]port[%d]payload[%d] \n\n ",SiCall[i].remote_sdp_audio_ip,strncpy(SiCall[i].audio_payload_name,SiCall[i].remote_sdp_audio_port,SiCall[i].audio_payload);
			}
	*/
			//si_print(PRINT_LEVEL_DEBUG, "Return Call Current State %d %d\n\n ",current_call_position, SiCall[current_call_position].state);
			SiCall[current_call_position].state = CALL_STATE_IDLE;
			return &SiCall[current_call_position];
		} //else si_print(PRINT_LEVEL_DEBUG, "Call Current State %d %d\n\n ",current_call_position, SiCall[current_call_position].state);
	 }
	return NULL;
}

int  getNumOfCalls(sicall_t* curCall )
{
	int i;
	int numofcalls = 0;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
 		if (&SiCall[i]!= curCall)
		{
			if (SiCall[i].state!=CALL_STATE_IDLE) 
				numofcalls++;
		}
	}
#ifdef REMOTE_LOOPBACK_TEST 
	 if (numofcalls>=2) 
	{
		curCall->loop=CALL_REMOTE_LOOPBACK;
		return 1;
	}
#endif
	return 0;

}
 sicall_t*  getOtherActiveCall(sicall_t* curCall )
{
	int i;
 
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
 		if (&SiCall[i]!= curCall)
		{
			if ((SiCall[i].state==CALL_STATE_IDLE)|| (SiCall[i].state==CALL_STATE_RELEASING))
			{
				;
			}else return &SiCall[i];
		}
	}
	return NULL;
}

int  incallCodecChecking(int codecType)
{
	int i;
	int found=0;
 	#ifdef DECT_ENABLED
 		return 1;
	#endif

	if (codecType ==101) return 1; 
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
 		if ((SiCall[i].state == CALL_STATE_CONNECTED) || (SiCall[i].state == CALL_STATE_ANSWERING) || (SiCall[i].state == CALL_STATE_NEW) ) //FIX 10082009
			found++;
	}

	if (found<2) return 1;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		if (SiCall[i].sStreamParameters.payload==-1) continue;
		if ((SiCall[i].state == CALL_STATE_CONNECTED) && !SiCall[i].codecChange)//CALL_STATE_IDLE) && (SiCall[i].state!=CALL_STATE_RELEASING) )
		{
			if ((codecType==9) && (SiCall[i].sStreamParameters.payload!=9))
				return 0;
			if ((codecType!=9) && (SiCall[i].sStreamParameters.payload==9))
				return 0;
		} 
	}
   return 1;
}
 
sicall_t*  getFirstActiveCall(void)
{
	int i;
//	if (siConference.ConferenceState) return NULL;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
 		if (SiCall[i].state!=CALL_STATE_IDLE)
		{
			return &SiCall[i];
		}
	}
	return NULL;
}
sicall_t*  findCallByLine(int line)
{
	int i;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		if (SiCall[i].callid==line)
		{
			return &SiCall[i];
		}
	}
	return NULL;
}

 
sicall_t*  findCallByIds( eXosip_event_t* pEvent)
{
 int i;
    for (i=MAX_SUPPORTED_CALLS-1;i>-1;i--)
 {
  if ((SiCall[i].cid>0) && (SiCall[i].cid==pEvent->cid))
  {
    return &SiCall[i];
  }
  if ((SiCall[i].did>0) && (SiCall[i].did==pEvent->did))
  { 
    return &SiCall[i];
  }
  if ((SiCall[i].tid>0) && (SiCall[i].tid==pEvent->tid))
  {
    return &SiCall[i];
  }

 }
   return (sicall_t*)NULL;
}


sicall_t*  findCallByCid(int cid)
{
	int i;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		if (SiCall[i].cid==cid)
		{
			return &SiCall[i];
		}
	}
	return NULL;
}

sicall_t*  findCallByTid(int tid)
{
	int i;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		if (SiCall[i].tid==tid)
		{
			return &SiCall[i];
		}
	}
	return NULL;
}

int findAccount(char *userName )
{
	int i;
	for ( i=0;i<MAX_SUPPORTED_ACCOUNTS;i++)
	{
		if ( strlen(&siphoneCore.regusername[i][0])==strlen(userName))
		{
 			if (!strncmp(userName,&siphoneCore.regusername[i][0],strlen(&siphoneCore.regusername[i][0])))
				return i;
		}
 	}
 	return -1;
}

int findPort(int accountID, int entity)  //fix 31 March 2009
{
 	if ((accountID<0) || (accountID>=MAX_SUPPORTED_ACCOUNTS)) return 0; //fix 31 March 2009

	if ((entity == CCFSM_ATTACHED_GUI)|| (entity == CCFSM_ATTACHED_DECT)) return 0;
	else return accountID;
}

ccfsm_attached_entity findEntity(int accountID)
{
 if ((accountID<0) || (accountID>MAX_SUPPORTED_ACCOUNTS)) return -1;//fix 31 March 2009

	#if (!(defined ATA_ENABLED) && (!(defined DECT_ENABLED)))
	  return CCFSM_ATTACHED_GUI;

	#elif (defined ATA_ENABLED) && (!(defined DECT_ENABLED))
	  return CCFSM_ATTACHED_ATA;

	#elif (defined ATA_ENABLED) && (defined DECT_ENABLED)//CVM or NATALIE with ATA
	#if ((defined CONFIG_ATA_1_FXS_NO_FXO_1_CVM) || (defined CONFIG_LMX4180_DECT_SUPPORT))
	  if(accountID>0) return CCFSM_ATTACHED_DECT;
	  else if (accountID==0) return CCFSM_ATTACHED_ATA;
    else return CCFSM_ATTACHED_DECT; //any other is broadcast 
	#elif ((defined CONFIG_ATA_2_FXS_NO_FXO_1_CVM) || (defined CONFIG_LMX4180_DECT_SUPPORT))
	  if(accountID>1) return CCFSM_ATTACHED_DECT;
	  else if ((accountID==0)||(accountID==1)) return CCFSM_ATTACHED_ATA;
	  else return CCFSM_ATTACHED_DECT; //any other is broadcast 
	#endif

	#elif (!(defined ATA_ENABLED) && (defined DECT_ENABLED)) //CVM or NATALIE ONLY    
	  return CCFSM_ATTACHED_DECT;
	#endif
}

int checkDECTBroadcastState(int accountID)
{
	int isSecondCall = 0;
	int i;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
		//If Br account is not idle talking) and the new incoming is for the Br account or if Br account is ringing (CALL_STATE_NEW)(that means that all handsets are ringning)
 		if(((!(SiCall[i].state==CALL_STATE_IDLE)) && (SiCall[i].accountID==CCFSM_DECT_BROADCAST_ID) && (accountID==CCFSM_DECT_BROADCAST_ID))
			||((SiCall[i].state==CALL_STATE_NEW) && (SiCall[i].accountID==CCFSM_DECT_BROADCAST_ID)))
		{
		 si_print(PRINT_LEVEL_DEBUG, "\nDect is busy \n");
		 isSecondCall++; 
		}
	}
	// DECT always disable call waiting //
	if (isSecondCall>0){
		return (int)0;
	}

	return (int)1;
}

//fix April 28, 2009
int isAnonymousBlocked(int accountID, eXosip_event_t *pEvent)
{
	if (!siphoneCore.rejectanonymous[accountID]) return 0;
 	if (pEvent && pEvent->request && pEvent->request->from)
	 {
 		if (pEvent->request->from->displayname)
		{
  			if (!strcmp("anonymous", tolower(pEvent->request->from->displayname)))
				return 1;
  		}
		if (pEvent->request->from->url->username)
		{
  			if (!strcmp("anonymous", tolower(pEvent->request->from->url->username)))
				return 1;
   		}
   	 }
 	 return 0;
 }

int linebusyflag = 0;
int isWaiting(int accountID)
{
	int isSecondCall=0;
	int i;
	for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	{
 	 if ((!(SiCall[i].state==CALL_STATE_IDLE)) && (SiCall[i].accountID==accountID))
		{
		 isSecondCall++; 
		}
	}
 	if (linebusyflag) isSecondCall++; //v1.0.1.1
 	if ((isSecondCall>0) && !siphoneCore.callwaiting[accountID])
 		return (int)0;
#ifdef DECT_ENABLED
	// DECT always disable call waiting //
	if (isSecondCall>0){
		return (int)0;}
#endif

	return (int)1;
 }

 int isBusy(int accountID)
 {
 	return siphoneCore.busyFlag[accountID];
 }

 int isAutoAnswer(int accountID)
 {
	
	return siphoneCore.autoanswer[accountID];
 }

 //fix April 28, 2009
int isRedirected(int accountID)
{
	int i;
	int reason = siphoneCore.redirectFlag[accountID]; 
  
	if(reason == NEVER) return 0;
 	else if (reason == ALWAYS) return 1;
	else if (reason == ON_BUSY)
	{
		for (i=0;i<MAX_SUPPORTED_CALLS;i++)
	 	 if ((!(SiCall[i].state==CALL_STATE_IDLE)) && (SiCall[i].accountID==accountID))
			return 1;
 		return 0;
	}else if (reason == ON_NO_ANSWER)  
	{
		
	}
	return 0;
}
 
 char* getRedirectionUri(int accountID)
 {
	if (siphoneCore.redirectFlag) return &siphoneCore.redirectURI[accountID][0];
	return NULL;
 }

 char* getRedirectionReason(int accountID)
 {
	if (siphoneCore.redirectFlag) return &siphoneCore.redirectReason[accountID][0]; 
	return NULL;
 }

void InitiateCall(sicall_t *pCall)
{
	memset ((char*)pCall, (char)0, sizeof (sicall_t));
	pCall->tid =  0xffffffff;
 	pCall->cid =  0xffffffff;
  	pCall->did =  0xffffffff;
 	pCall->accountID = 0;
	pCall->callid  = 0xffffffff;
  
}

int si_alsa_stop_tone(sicall_t *pCall) 
{
 	if (pCall->ToneInProgress)
	{
		pCall->ToneInProgress =0;
 		return sc1445x_mcu_set_tone(MCU_TONE_NA_BT, 0,(unsigned short)pCall->port);
	}
	return 0;
}


int si_start_tone_busy(sicall_t *pCall)
{
	pCall->ToneInProgress =1;
	return sc1445x_mcu_set_tone(MCU_TONE_BZ_BT, 0,(unsigned short)pCall->port);
}

int si_start_tone_congestion(sicall_t *pCall)
{
	pCall->ToneInProgress =1;
	return sc1445x_amb_start_callprogress_tone(SC1445x_AE_STD_TONE_CONGESTION, (unsigned short)pCall->port);

}
int si_start_tone_callwaiting(sicall_t *pCall)
{
	pCall->ToneInProgress =1;
	return sc1445x_amb_start_dynamic_tone(SC1445x_AE_TONE_F425, (unsigned short)pCall->port);
}

int si_start_tone_dial(unsigned short line)
{
  return sc1445x_mcu_set_tone(MCU_TONE_DL_BT, 0,(unsigned short)line); 
}

int si_stop_tone_dial(unsigned short line)
{
  return sc1445x_mcu_set_tone(MCU_TONE_NA_BT, 0,(unsigned short)line); 
}

int si_start_tone_disconnect1(sicall_t *pCall)
{
	pCall->ToneInProgress =1;
	return sc1445x_amb_start_callprogress_tone(SC1445x_AE_STD_TONE_DISCONNECT1, (unsigned short)pCall->port);
}

int si_start_tone_disconnect2(sicall_t *pCall)
{
	pCall->ToneInProgress =1;
	return sc1445x_amb_start_callprogress_tone(SC1445x_AE_STD_TONE_DISCONNECT2, (unsigned short)pCall->port);
}
int si_start_tone_ringing(sicall_t *pCall)
{
	pCall->ToneInProgress =1;
	return sc1445x_mcu_set_tone(MCU_TONE_RG_BT, 0,(unsigned short)pCall->port);
}
 
int si_start_tone_speeddial(sicall_t *pCall)
{
	pCall->ToneInProgress =1;
	return sc1445x_amb_start_callprogress_tone(SC1445x_AE_STD_TONE_SPECIAL_DIAL, (unsigned short)pCall->port);
}

int si_start_tone_waiting(sicall_t *pCall)
{
 
	pCall->ToneInProgress =1;
	return sc1445x_mcu_set_tone(MCU_TONE_CW_BT, 0,(unsigned short)pCall->port); 	
}
void si_play_ringtone(sicall_t *pCall, char* filename )
{
	if (filename)  ua_playring(filename);
	else si_start_tone_ringing(pCall);
}

 
void si_stop_ringtone(sicall_t *pCall )
{
   	if (!ua_stopringplay()) return ;
	 si_alsa_stop_tone(pCall);	 
}

void si_ua_setconference(sicall_t* pCall1, sicall_t* pCall2)
{
	int i;
  
	for (i=0;i<MAX_SUPPORTED_CALLS	;i++)
	{
  	     if (SiCall[i].state == CALL_STATE_CONNECTED)
	     {	
			 if (pCall1==NULL) {
				 pCall1=&SiCall[i];
  			 }else  if (pCall2==NULL) 
				 pCall2=&SiCall[i];
				 break;
		 }
	 }
	if (pCall1) {
		pCall1->mcuMediaStream.mediaAttr = MCU_MATTR_BOTH_ID;
		sc1445x_mcu_configure(pCall1->pMcuHandler,MCU_MCTRL_MODIFY_ID	,&pCall1->mcuMediaStream);
	}
	if (pCall2) {
		pCall2->mcuMediaStream.mediaAttr = MCU_MATTR_BOTH_ID;
 		sc1445x_mcu_configure(pCall2->pMcuHandler,MCU_MCTRL_MODIFY_ID	,&pCall2->mcuMediaStream);
	}
}
  
int si_ua_recv_msgAck(sicall_t *pCall, eXosip_event_t *pEvent)
{
  	if (!pCall) 	return -1;
  	if (pCall->state != CALL_STATE_CONNECTED) return -1;
  	switch(pCall->callTransferInProgress)
 	{		
		case SIPUA_TRANSFEREE_ATTENDED_TRYING:
 		{
    			 pCall->callTransferInProgress = SIPUA_TRANSFEREE_ATTENDED_TRANSFERRING;
  		     create_cback_transfer(pCall, CCFSM_ATTENDEDTRANSFER_IND,pCall->transfer_from, pCall->transfer_to, pCall->replaces);
     	}
		break;
 	}
    return 0;
}

int create_cback_transfer(sicall_t* pCall, int flag, char *from, char *to, char *replaces)
{
	
  ccfsm_cback_type	m_transferInd;	 

  if (pCall==NULL) {si_print(PRINT_LEVEL_ERR, "Invalid call \n");return -1;}
 	if ((to==NULL) || (pCall==NULL)) return -1;

	m_transferInd.ccfsm_transfer_indication_cback.cback_id=flag;
	m_transferInd.ccfsm_transfer_indication_cback .accountid = pCall->accountID ;
	m_transferInd.ccfsm_transfer_indication_cback .callid = pCall->callid ;

	m_transferInd.ccfsm_transfer_indication_cback .transfermode=flag;  //ATTENDED TRANSFER, BLIND TRANSFER
	strcpy( m_transferInd.ccfsm_transfer_indication_cback .transfer_to,to); 
	if (from)
		strcpy( m_transferInd.ccfsm_transfer_indication_cback.transfer_from,from);

	if (replaces)
		strcpy( m_transferInd.ccfsm_transfer_indication_cback .replaces,replaces);	
	else 
		m_transferInd.ccfsm_transfer_indication_cback .replaces[0]=0;

 
	 ccfsm_callback(&m_transferInd);
 
	 return 0;
}

int checkRequiredHeader(osip_message_t *pPacket)
{
	int ret ; 
 	osip_header_t *pRequire;
 
	if (pPacket==NULL) return 0;
	eXosip_lock();
	ret = osip_message_header_get_byname(pPacket, "Require", 0, &pRequire);	
	if (pRequire)
	{

 	}
  	eXosip_unlock();
	return 0;
}

int sipua_blindtransfer_indication(sicall_t *pCall, int reason)
{ 
   	if (pCall->remoteHold)
	 	create_updatestatus_callback( CCFSM_BLINDTRANSFERED_CB, pCall->accountID,pCall->callid, 0, reason,"", CCFSM_STATE_HELD);
 	else
	 	create_updatestatus_callback( CCFSM_BLINDTRANSFERED_CB, pCall->accountID,pCall->callid, 0, reason,"", CCFSM_STATE_TALK);
   	return 0;
}
   

/*
for (i=0;i<MAX_SUPPORTED_CODECS;i++)
    {
	   char  tmp[32];
	   char  num[8];
	   char  *pfmtp;
	   if (pCore->Codecs[accountId][i]<0) {
	     sdpInfo->audio_codec[i].valid=0;
		}else{
		  sprintf(num, "%d", pCore->Codecs[accountId][i]);
		  sprintf(tmp, "%d %s/%s", pCore->Codecs[accountId][i], GetCodecName(pCore->Codecs[accountId][i]),GetCodecRate(pCore->Codecs[accountId][i]));
 		  strcpy(sdpInfo->audio_codec[i].payload, num);//< supported audio codec 
		  strcpy(sdpInfo->audio_codec[i].proto, "RTP/AVP");					 
		  strcpy(sdpInfo->audio_codec[i].a_rtpmap, tmp);
		  sdpInfo->audio_codec[i].valid=1;
		  pfmtp = GetFmtpAttr(pCore->Codecs[accountId][i]);
		  if (pfmtp){
			strcpy(sdpInfo->audio_codec[i].a_fmtp,pfmtp);
			sdpInfo->audio_codec[i].isfmtp=1;
 		 }else sdpInfo->audio_codec[i].isfmtp=0;
 	   }
   }*/

void si_ua_setpreferred(int accountId,int codectype, SICORE* pCore,  osip_negotiation_t  * sdpInfo )
{
	int i, k;
	int newCodecs[16]; 
 	 
	memset(newCodecs, -1, (sizeof (int))*16);

	switch(codectype)
	{
		case 1://CCFSM_NARROWBAND_CODEC :
			 
			k=0;
			for (i=0;i<MAX_SUPPORTED_CODECS;i++)
			{
				if (pCore->Codecs[accountId][i]!=9) {
					newCodecs[k] = pCore->Codecs[accountId][i];	 
					k++;
				}
			}
 		break;
		case 2://CCFSM_WIDEBAND_CODEC:
			 
 			newCodecs[0]=9;
			k=1;
 			for (i=0;i<MAX_SUPPORTED_CODECS;i++)
			{
				if (pCore->Codecs[accountId][i]!=9) {
					newCodecs[k]=pCore->Codecs[accountId][i];	 
					k++;
				}
			}
 			break;
		default:
			k=0;
			for (i=0;i<MAX_SUPPORTED_CODECS;i++)
			{
  				int flag= incallCodecChecking(pCore->Codecs[accountId][i]);
 
 				if (flag){
 	 				newCodecs[k]=pCore->Codecs[accountId][i];	 
					k++;
				}

 			}
   	}
 
	for (i=0;i<MAX_SUPPORTED_CODECS;i++)
	{
		char  tmp[32];
		char  num[8];
		char  *pfmtp;
		if (newCodecs[i]<0) {
			sdpInfo->audio_codec[i].valid=0;
		}else
		{
			sprintf(num, "%d", newCodecs[i]);
			sprintf(tmp, "%d %s/%s", newCodecs[i], GetCodecName(newCodecs[i]),GetCodecRate(newCodecs[i]));
			strcpy(sdpInfo->audio_codec[i].payload, num);//< supported audio codec 

			if (sdpInfo->crypto_level==ID_CRYPTO_DISABLE)
				strcpy(sdpInfo->audio_codec[i].proto, "RTP/AVP");					 
			else
				strcpy(sdpInfo->audio_codec[i].proto, "RTP/SAVP");					 

			strcpy(sdpInfo->audio_codec[i].a_rtpmap, tmp);
			sdpInfo->audio_codec[i].valid=1;
			pfmtp = GetFmtpAttr(newCodecs[i]);
			if (pfmtp){
				strcpy(sdpInfo->audio_codec[i].a_fmtp,pfmtp);
				sdpInfo->audio_codec[i].isfmtp=1;
			}else sdpInfo->audio_codec[i].isfmtp=0;
		}
	}	
}

int isSRTPEnabled(sicall_t *pCall)
{
	int cr_id = pCall->crypto_parameters.tx_encryption_suite_id;
	if (cr_id>=0 && pCall->crypto_parameters.tx_encryption_suite[cr_id].enabled && pCall->crypto_parameters.rx_encryption_suite.enabled) {
		return 0x80000000;  
	}
	return 0;
}