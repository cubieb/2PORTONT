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
 * File:		 		 si_ua_events.c
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
#include <pthread.h>

#include <si_print_api.h>
 
#include "si_ua_events.h"
#include "si_ua_init.h"
#include "si_ua_hold.h"
#include "si_ua_redirect.h"
#include "si_ua_callcontrol.h"
#include "si_ua_sessiontimer.h"
#include "si_ua_dtmf.h"
#include "si_ua_transfer.h"
#include "si_ua_instantmessage.h"
#include "si_ua_registrar.h"
#include "si_ua_notify.h"

#include "../exosip2/src/eXosip2.h"

#include "../exosip2/src/eXosip.h"
#include "../common/si_phone_api.h"
#include "../CCFSM/ccfsm_api.h"

extern int uaPendingRegistration;
int si_ua_process(void)
{
	pthread_t thread;
  int i;
 
	i = pthread_create (&thread, NULL, si_ua_thread, (void *) NULL);
	pthread_detach(thread) ;

	if (i != 0)
	{
		FatalError(EXOSIP_PTHREAD_CREATE_FAILED, "Exosip_pthread_create failed"); 
  		return -1;
	}
 
 	return 0;
}

int check_sip_version(eXosip_event_t *pevent);
int check_sip_version(eXosip_event_t *pevent)
{
	char *ver=NULL;
 	osip_via_t *via=NULL;

	if (pevent==NULL) return 1;
	if (pevent->request)  {
		osip_message_get_via (pevent->request,0, &via);
  	}
	else if (pevent->response)    {
		osip_message_get_via (pevent->response,0, &via);
 	}
 	if(via)  ver= via_get_version(via);
 	if (ver && !strcmp(ver,"2.0"))
 		return 1;
 		 
   	return 0;
}

int si_ua_thread(void * arg)
{
	int ret;
	eXosip_event_t *pevent=NULL;
	sicall_t *pCall;
	SICORE* pCore = &siphoneCore;
 	eXosip_execute();
	eXosip_automatic_action ();

 
	if (!(pevent = eXosip_event_wait (0, 5)))
	{
 
		return 0;//continue;
	}
	pCall= findCallByIds(pevent);
 	if (pevent!=NULL)
	{	
	/*
 	  //SIPIT 25
 	  if (!check_sip_version(pevent)) 
	  {
 		eXosip_call_send_answer (pevent->tid, 505, NULL);
  		goto _exit;
	  }

	 */ 
       switch ((int)(pevent->type))
      {
 	  case EXOSIP_CALL_INVITE:
		si_ua_incomingCall(pevent);
		break;			
	  case EXOSIP_CALL_REINVITE:
			//check if is a holdon/hold off message
			//si_print(PRINT_LEVEL_DEBUG, "Call si_ua_processReinvite\n ");
			si_ua_processReinvitetoHoldOn(pevent);
			//si_print(PRINT_LEVEL_DEBUG, "Call si_ua_processReinvitetoHoldOn Ended\n ");
			break;
	  case EXOSIP_CALL_NOANSWER:
			//si_print(PRINT_LEVEL_DEBUG, "EXOSIP_CALL_NOANSWER \n ");
			si_ua_recv_callReleased(pCall,pevent);
			break;
	  case EXOSIP_CALL_PROCEEDING:
			//si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP_CALL_ PROCEEDING ");
			si_ua_recv_callProceeding(pCall,pevent);
			break;
	  case	EXOSIP_CALL_RINGING:

			si_ua_recv_ringing(pCall,pevent);
			break;
	  case	EXOSIP_CALL_ANSWERED:
			 //si_print(PRINT_LEVEL_DEBUG, "EXOSIP_CALL_ANSWERED \n");

 			eXosip_call_send_ack(pevent->did,si_ua_recv_callAnswered(pCall,pevent)); 
			//si_print(PRINT_LEVEL_DEBUG, "EXOSIP_CALL_ANSWERED 1\n");
		  break;
	  case	EXOSIP_CALL_REDIRECTED:
			si_ua_redirectcall(pCall, pevent);
			break;
	  case	EXOSIP_CALL_REQUESTFAILURE:
			if (pevent->response->status_code==422)
			{ 
				//FIX BROADWORKS
				si_ua_retry_invite(pevent, pCall);
  			}else if (!(pevent->response->status_code==401 || pevent->response->status_code==407 ||pevent->response->status_code==480 ) )
			{
				 si_print(PRINT_LEVEL_DEBUG, "EXOSIP_CALL_REQUESTFAILURE \n ");
				si_ua_recv_callReleased(pCall,pevent);							
				//si_ua_terminateCall(pCall);
			}
			break;
	  case	EXOSIP_CALL_SERVERFAILURE:
			//si_print(PRINT_LEVEL_DEBUG, "EXOSIP_CALL_SERVERFAILURE \n ");
			si_ua_recv_callReleased(pCall,pevent);							
			break;
	  case	EXOSIP_CALL_GLOBALFAILURE:
			if( pevent && pevent->response) 
		  {
			  if (!(pevent->response->status_code==401 || pevent->response->status_code==407 ||pevent->response->status_code==480) )
				{

					//si_print(PRINT_LEVEL_DEBUG, "EXOSIP_CALL_GLOBALFAILURE \n ");
					si_ua_recv_callReleased(pCall,pevent);							
				}
		  }
			break;
	  case 	EXOSIP_CALL_ACK:
			si_ua_recv_ack(pCall,pevent);
			break;
	  case	EXOSIP_CALL_CANCELLED:
			//si_print(PRINT_LEVEL_DEBUG, "\n 16 EXOSIP CALL  CANCELLED \n");
			break;
	  case	EXOSIP_CALL_TIMEOUT:
 			//si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP CALL  TIMEOUT \n");
			si_ua_recv_callReleased(pCall,pevent);
		break;
	  case	EXOSIP_CALL_CLOSED:
			//si_print(PRINT_LEVEL_DEBUG, "\n   EXOSIP CALL  CLOSED <%x>\n", (int)pCall);
			si_ua_recv_callClosed(pCall,pevent);
		break;
	  case	EXOSIP_CALL_RELEASED:
			//si_print(PRINT_LEVEL_DEBUG, "\n   EXOSIP CALL  RELEASED <%x>\n" ,(int)pCall);
			si_ua_recv_callReleased(pCall,pevent);
			si_ua_ack(pCall, pevent->tid, 200);
		break;
      ///////////////////////////////////////////////////////////////////////////////
	  case EXOSIP_CALL_MESSAGE_NEW:
			// parse incoming new message (INFO, ...)
	 		if (MSG_IS_PRACK(pevent->request)) 
			{
	 			si_ua_ack(pCall, pevent->tid, 200); 

			}else if (MSG_IS_UPDATE(pevent->request)) 
			{
   				si_sessiontimer_update(pCall, pevent->request);
				//if (ret!=0)
				si_ua_ack(pCall, pevent->tid, 200); 

			}else if (MSG_IS_REQUEST(pevent->request)) 
			{
 				// parse incoming request and send ack with the appropriate TYPE
				// otherwise return -1
 				ret = si_ua_parseNewMessageRequest(pCall, pevent);
				if (ret!=0){
					si_ua_ack(pCall, pevent->tid, 200); 
				} 
			}else{
 				si_ua_ack(pCall, pevent->tid, 200);   // YPAPA:			
			}

		   break;			
	  case EXOSIP_CALL_MESSAGE_PROCEEDING:
			//si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP_CALL_MESSAGE_PROCEEDING \n");
			break;			
	  case EXOSIP_CALL_MESSAGE_ANSWERED:
 			si_ua_recv_msgAck(pCall,pevent);
			si_sessiontimer_update(pCall,pevent->response);
  			break;		
	  case EXOSIP_CALL_MESSAGE_REDIRECTED:
			//si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP_CALL_MESSAGE_REDIRECTED \n");
			break;			
	  case EXOSIP_CALL_MESSAGE_REQUESTFAILURE:
			//si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP_CALL_MESSAGE_REQUESTFAILURE 1\n");
		  si_ua_parseMessageRequestFailure(pCall, pevent);
			//si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP_CALL_MESSAGE_REQUESTFAILURE 2\n");
			break;			
	  case EXOSIP_CALL_MESSAGE_SERVERFAILURE:
		  // REFER			
		  //si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP_CALL_MESSAGE_SERVERFAILURE \n");
			break;			
	  case EXOSIP_CALL_MESSAGE_GLOBALFAILURE:
			//si_print(PRINT_LEVEL_DEBUG, "\n EXOSIP_CALL_MESSAGE_GLOBALFAILURE \n");
			break;			
		 case EXOSIP_REGISTRATION_NEW:

			break;
		case EXOSIP_REGISTRATION_SUCCESS:
		 {
			siregistration_t *pReg;
			pReg = si_ua_get_reg_account(pevent->rid);
			if (pReg)
			{
				pReg->retries =0;
				if (pReg->state == CCFSM_UNREG_IN_PROGRESS)
				{	
 					pReg->state = CCFSM_REG_INIT;
					checkToReRegister(pCore);
				}
				else if (pReg->state == CCFSM_REG_IN_PROGRESS)
				{
					  //si_print(PRINT_LEVEL_INFO, "EXOSIP_REGISTRATION_SUCCESS (send callback)\n");
					si_ua_registrationended(pevent->rid, pevent,CCFSM_REG_SUCCESS);	
					pReg->state =CCFSM_REG_SUCCESS;
				}
			} 
		}
		break;
	case EXOSIP_REGISTRATION_FAILURE:
		{
			siregistration_t *pReg;
			pReg = si_ua_get_reg_account(pevent->rid);
			if (pReg)
			{
  				si_print(PRINT_LEVEL_ERR, "EXOSIP_REGISTRATION_FAILED    \n");
				// pReg->state = CCFSM_REG_FAILED;
 				pReg->authentication = 1;
			}
  		} 
		break;
 	case EXOSIP_REGISTRATION_TERMINATED:
		{
			siregistration_t *pReg;
			si_print(PRINT_LEVEL_INFO, "EXOSIP_REGISTRATION_TERMINATED   \n");
			pReg = si_ua_get_reg_account(pevent->rid);
			if (pReg)
			{
 				pReg->state = CCFSM_REG_INIT;
				pReg->retries ++;
				int ret= si_get_account_from_rid(pevent->rid);
				if (pReg->retries<3 && ret>-1){	
  					si_ua_register(pCore, &pCore->siregistration[ret], 1);		

				}
				else 	si_ua_registrationended(pevent->rid, pevent,CCFSM_REG_TERMINATED);	

			}
		}
		si_print(PRINT_LEVEL_ERR, "Registration terminated\n");
		break;
	case EXOSIP_MESSAGE_NEW:
		if (MSG_IS_OPTIONS(pevent->request)) 
		{
			//FIX BROADWORKS
			si_ua_ackoptions( pevent->tid, 200);

		}else if (MSG_IS_MESSAGE(pevent->request)) 
		  {
  			ret = sc1445x_im_recv(pevent); //fix April
			if (!ret)
				si_ua_ackoptions( pevent->tid, 200); 
			else
				si_ua_ackoptions( pevent->tid, 415); 
		  } 
		break;
		case EXOSIP_SUBSCRIPTION_UPDATE:
		case EXOSIP_SUBSCRIPTION_CLOSED:
 		case EXOSIP_SUBSCRIPTION_NOANSWER:
		case EXOSIP_SUBSCRIPTION_PROCEEDING:
		case EXOSIP_SUBSCRIPTION_ANSWERED:
		case EXOSIP_SUBSCRIPTION_REDIRECTED:
		case EXOSIP_SUBSCRIPTION_REQUESTFAILURE:
		case EXOSIP_SUBSCRIPTION_SERVERFAILURE:
		case EXOSIP_SUBSCRIPTION_GLOBALFAILURE:
		case EXOSIP_SUBSCRIPTION_RELEASED:
			si_print(PRINT_LEVEL_DEBUG, "Incoming Subscribe message= %d \n\n", (int)(pevent->type));
		break;
		case EXOSIP_SUBSCRIPTION_NOTIFY:
 			si_notify_MessageWaitingIndication(pCall,pevent);
  		break;
		/*
		case EXOSIP_REGISTRATION_REFRESHED  :
		case EXOSIP_CALL_MESSAGE_PROCEEDING : 
		case EXOSIP_CALL_MESSAGE_REDIRECTED : 
		case EXOSIP_CALL_MESSAGE_SERVERFAILURE :    
		case EXOSIP_CALL_MESSAGE_GLOBALFAILURE  :
		case EXOSIP_MESSAGE_PROCEEDING : 
		case EXOSIP_MESSAGE_ANSWERED : 
		case EXOSIP_MESSAGE_REDIRECTED : 
		case EXOSIP_MESSAGE_REQUESTFAILURE : 
		case EXOSIP_MESSAGE_SERVERFAILURE : 
		case EXOSIP_MESSAGE_GLOBALFAILURE : 
		case EXOSIP_IN_SUBSCRIPTION_NEW  :
		case EXOSIP_IN_SUBSCRIPTION_RELEASED  :
		case EXOSIP_NOTIFICATION_NOANSWER :
		case EXOSIP_NOTIFICATION_PROCEEDING : 
		case EXOSIP_NOTIFICATION_ANSWERED : 
		case EXOSIP_NOTIFICATION_REDIRECTED : 
		case EXOSIP_NOTIFICATION_REQUESTFAILURE  :
		case EXOSIP_NOTIFICATION_SERVERFAILURE : 
		case EXOSIP_NOTIFICATION_GLOBALFAILURE :
		case EXOSIP_EVENT_COUNT:
			*/
	default:
		eXosip_call_send_answer (pevent->tid, 501, NULL);
 	}
  
//_exit:	
	 if (pevent)  {
 		 eXosip_event_free (pevent);
 		 pevent = NULL;
  	 }
   }
	return 0;
  
}
int si_ua_parseMessageRequestFailure(sicall_t *pCall, eXosip_event_t* pEvent)//int tid, osip_message_t *req)
{	
   int ret=0;	 
  
   if (MSG_IS_INFO(pEvent->request))
   {

   }

   if (MSG_IS_REFER(pEvent->request))
   {
 		if (pEvent->request->status_code)
		{
			;
		}else  if (pEvent->response->status_code)
		{
 			if (!(pEvent->response->status_code==401 || pEvent->response->status_code==407 ||pEvent->response->status_code==480) )
			{
 				sipua_blindtransfer_indication(pCall,pEvent->response->status_code);
  				ret=si_ua_holdOff(pCall);
				if (ret) {si_ua_ack(pCall, pEvent->tid, ret); return 0;}
				else return -1;
			}else return -1;
		}
   }
   if (MSG_IS_NOTIFY(pEvent->request))
   {
	   ;
   } 
   return -1;
}	

int si_ua_parseNewMessageRequest(sicall_t *pCall, eXosip_event_t* pEvent)//int tid, osip_message_t *req)
{	
   int ret;	 
  
   if (MSG_IS_INFO(pEvent->request)){
  	 ret= si_ua_rcvDTMF( pEvent->request);
	 if (ret) {si_ua_ack(pCall, pEvent->tid, ret); return 0;}
	 else return -1;
   }

 
   if (MSG_IS_REFER(pEvent->request)){
	 	 ret= si_ua_rcvREFER(pCall, pEvent);
	// if (ret) {si_ua_ack(pCall, pEvent->tid, ret); return 0;}
	   return 0;
   }

 
   if (MSG_IS_NOTIFY(pEvent->request))
   {
 	   	if (pEvent->request->content_type)
		{
			if (!strcmp(pEvent->request->content_type->type,"message") && !strcmp(pEvent->request->content_type->subtype,"sipfrag"))
			   	si_ua_rcvSIPFlag(pEvent);//tid, req);	
			else si_notify_MessageWaitingIndication(pCall,pEvent);
		}
    } 

    return -1;
}	

int si_ua_ack(sicall_t *pCall, int tid, int code)
{
	int ret;
	osip_message_t *pAnswer;

	if (pCall == NULL) return -1;
  	eXosip_lock ();
	ret = eXosip_call_build_answer (tid, code, &pAnswer);
	if (ret==0)
	{ 
		if (siphoneCore.SessionTimerEnable)
		{
			osip_message_set_supported(pAnswer,SIPHONE_UA_SUPPORTED_HEADER_ACK200_wTIMER );
			// SESSION TIMER 
			 
			if (pCall->updateKeeper && pCall->initiator)	
				osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uac");
  			else 	if (pCall->updateKeeper && !pCall->initiator)	
				osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uas");
 			else if (!pCall->updateKeeper && pCall->initiator)	
				osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uas");
 			else osip_message_set_header(pAnswer, "Session-Expires", "300;refresher=uac");
 
			osip_message_set_header(pAnswer, "Min-SE", "300");
			osip_message_set_require(pAnswer, "timer");
  		}else
		{
			osip_message_set_supported(pAnswer,SIPHONE_UA_SUPPORTED_HEADER_ACK200);
 		}

		osip_message_set_header(pAnswer,"Server", "Sitel SIP UA" );
 	    eXosip_call_send_answer (tid, code, pAnswer);
	}
 	eXosip_unlock ();
	return 0;
}

  
int si_ua_ackoptions( int tid, int code)
{
	int ret;
	osip_message_t *pAnswer;
    	eXosip_lock ();
	ret = eXosip_options_build_answer(tid, code, &pAnswer);
 
	if (ret==0)
	{ 
  		osip_message_set_header(pAnswer,"Server", "Sitel SIP UA" );
  		eXosip_options_send_answer(tid, code, pAnswer);
	} 
  	eXosip_unlock ();
	return 0;
}
 
//FIX BROADWORKS
int si_ua_retry_invite(eXosip_event_t *pevent,sicall_t *pCall)
{

	osip_header_t *pInMinSEHeader=NULL;
	osip_header_t *pMinSEHeader=NULL;
	osip_header_t *pSessionExpiresHeader=NULL;
	eXosip_dialog_t *jd = NULL;
	eXosip_call_t *jc = NULL;
	osip_transaction_t *tr = NULL;

   _eXosip_call_transaction_find (pCall->tid, &jc, &jd, &tr);
	if (tr)
	{
		osip_message_header_get_byname(pevent->response, "Min-SE", 0, &pInMinSEHeader);
		if (pInMinSEHeader && pInMinSEHeader->hvalue!=NULL)
		{
			osip_message_header_get_byname(tr->orig_request, "Session-Expires", 0, &pSessionExpiresHeader);
			osip_message_header_get_byname(tr->orig_request, "Min-SE", 0, &pMinSEHeader);
			if (pSessionExpiresHeader && pSessionExpiresHeader->hvalue!=NULL)
			{
				char refresher[24];
				sprintf(refresher, "%d;refresher=%s",atoi(pInMinSEHeader->hvalue),"uas");
 				osip_free(pSessionExpiresHeader->hvalue);
				pSessionExpiresHeader->hvalue = osip_strdup(refresher);
 				pCall->maxSessionExpireTime = atoi(pMinSEHeader->hvalue);
 			} 
			if (pMinSEHeader && pMinSEHeader->hvalue!=NULL)
			{
				char minse[8];
				sprintf(minse, "%s",pInMinSEHeader->hvalue);
 				osip_free(pMinSEHeader->hvalue);
				pMinSEHeader->hvalue = osip_strdup(minse);
				pCall->minSessionExpireTime = atoi(pMinSEHeader->hvalue);
 			} 
			if (pMinSEHeader && pSessionExpiresHeader)
				eXosip_default_action(pevent);
		}
	} 
	return 0;   
			
}