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
 * File:		 		 si_ua_dtmf.c
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
 
#include "si_ua_transfer.h"
#include "si_ua_callcontrol.h"
#include "si_ua_events.h"
#include "si_ua_hold.h"
#include "si_ua_notify.h"
#include "../CCFSM/ccfsm_init.h"
#include "../CCFSM/ccfsm_api.h"

int si_ua_rcvREFER(sicall_t* pCall, eXosip_event_t *pEvent) 
{
  	char tempRefer[256];
 	char *pos_start,*replace_pos;
	int tid = pEvent->tid;
 	int ret ;
 	osip_header_t *pReferTo;
	osip_header_t *pReferredBy;
  
	if (pCall==NULL)  goto ref_exit;

	if (pCall && (pCall->localHold || pCall->localrefer) ) 
	{
		// not allowed here
 		goto ref_exit;
	}
  	 //check the REFER packet header
	ret = osip_message_header_get_byname(pEvent->request, "Refer-To", 0, &pReferTo); 
	ret = osip_message_header_get_byname(pEvent->request, "Referred-By", 0, &pReferredBy); 
	if (pReferTo==NULL) 
 	{
  		goto ref_exit;
	}
  
	if (pReferredBy==NULL)
	{
  		goto ref_exit;
	}
	
	// get host from refer to 
	if (strlen(pReferTo->hvalue)<256)
	{
		if (pReferTo->hvalue[0]=='<')  
			strcpy(tempRefer, &pReferTo->hvalue[1]);
		else
			strcpy(tempRefer, &pReferTo->hvalue[0]);
	}else {
   		goto ref_exit;
	}
  
 	__osip_uri_unescape(tempRefer); 
 
	if ((0 != osip_strncasecmp (tempRefer, "sip", 3)
       && 0 != osip_strncasecmp (tempRefer, "sips", 4)))
    {                  
 		goto ref_exit;
	}
	pos_start = tempRefer;
	while (pos_start[0]!=';' && pos_start[0]!='?' )
	{
		if ((pos_start-tempRefer)==(strlen(tempRefer)+1))
			break;
 		pos_start++;
		
	}
 	if ((pos_start-tempRefer)<64)
	{
		// sipit 25 
		strncpy(pCall->transfer_to,tempRefer, pos_start-tempRefer+1);
		char *ttt = strstr(pCall->transfer_to,">");
		if (ttt)
		{
			ttt[0]='\0';
 		} 
		// sipit 25  - end
 		 
		si_print(PRINT_LEVEL_DEBUG, "------------[%s] TRANSFER TO [%s] \n\n",__FUNCTION__, pCall->transfer_to);
		 


 	}
	else {
 		goto ref_exit;
	}

	if (pos_start[0]==';' || pos_start[0]=='?' )
		pCall->transfer_to[strlen(pCall->transfer_to)-1]='\0' ;
   	strcpy(pCall->transfer_from , pReferredBy->hvalue);
  
  	replace_pos= strstr(tempRefer, "Replaces");
 	if (replace_pos) { 
		strcpy(pCall->replaces,&replace_pos[9]);
		if (pCall->replaces[strlen(pCall->replaces)-1]=='>')
			pCall->replaces[strlen(pCall->replaces)-1]='\0';
	}else pCall->replaces[0]='\0'; 	

  	// check if a 'REPLACES' field is contained in the incoming refer packet
 	if (replace_pos) {
 
		pCall->callTransferInProgress=SIPUA_TRANSFEREE_ATTENDED_TRYING;
		si_ua_ack(pCall, tid, 202);
	 	eXosip_execute();
  	 	eXosip_automatic_action ();
		si_ua_referNotify(pEvent->did, 100, "Trying", 1);
  		return -1;
	}
	else{
		pCall->callTransferInProgress=SIPUA_TRANSFEREE_BLINK;
		si_ua_ack(pCall, tid, 202);
		eXosip_execute();
		eXosip_automatic_action ();
		si_print(PRINT_LEVEL_DEBUG, "------------[%s] Blind Transfer from [%s] TO [%s] \n\n",__FUNCTION__, pCall->transfer_from, pCall->transfer_to);
   		create_cback_transfer(pCall, CCFSM_BLINDTRANSFER_IND, pCall->transfer_from,pCall->transfer_to, NULL);
 	}

	// pCall-> remoterefer=1;
	 return 0;
   
 ref_exit:
 
 si_ua_ack(pCall, tid, 200);   
 si_ua_referNotify(pEvent->did, 403, "Forbidden", 0);
  return 0;
} 
 
int si_ua_referNotify(int did, int status, const char* msg, int active)
{
   int ret;
   char msgstatus[128];
   osip_message_t *pNotify;
    sprintf(msgstatus,  "SIP/2.0 %d %s\r\n", status, msg );	
   eXosip_lock();
    	
   ret = eXosip_call_build_notify(did,active ?EXOSIP_SUBCRSTATE_ACTIVE :EXOSIP_SUBCRSTATE_TERMINATED ,&pNotify);	   			
   if (ret) 
   {
      si_print(PRINT_LEVEL_ERR, "FATAL ERROR sending refer notify / request 1 \n\n ");
      eXosip_unlock();
      return -1;
   }
   osip_message_set_content_type(pNotify, "message/sipfrag;version=2.0");
   osip_message_set_header(pNotify, "Event","refer");
   osip_message_set_body(pNotify, msgstatus,strlen(msgstatus));	

   ret = eXosip_call_send_request(did, pNotify);	   			
   if (ret) 
   {
		osip_message_free (pNotify);

      si_print(PRINT_LEVEL_ERR, "FATAL ERROR sending refer notify / request \n\n ");
      eXosip_unlock();
      return -1;
   }	
   eXosip_unlock();
  
   return 0;
}

int si_ua_rcvSIPFlag(eXosip_event_t *pEvent)//id, osip_message_t *req)
{
	char data[512];	
	osip_body_t *pData;
	sicall_t* pCall;
	osip_message_t *req= pEvent->request;
	int tid = pEvent->tid;
 
	if (req==NULL) return -1;
	pCall = findCallByIds(pEvent); 
	if (pCall==NULL) return -1;
 
	if (req->content_type)
	{
		if (strcmp(req->content_type->type,"message") || strcmp(req->content_type->subtype,"sipfrag"))
		{
		    si_ua_ack(pCall, tid, 415); 
		    return 0;
		}
	}
 
	osip_message_get_body(req, 0, &pData);
	if (pData->body)
	{
		int status =0;
		char* statusstr ;
	 	strcpy(data, pData->body);
   		si_notify_BlindTransferProgress(  pData->body);
		statusstr = strchr(pData->body,' ' );
		if (statusstr)
			status = atoi(statusstr);
		if (status==180)
 		{
			//si_print(PRINT_LEVEL_DEBUG, " ....RINGING ");
		}else if  (status<200 && status >=100)
		{ 
 			//si_print(PRINT_LEVEL_DEBUG, " ....IN PROGRESS ");
		 }else if  (status>=200 && status<300)
		 {
  			si_ua_ack(pCall, tid, 200);
			if (pCall)
			{
  			  // YPAPA set semaphore
 			  si_ua_terminateCall(pCall);
			}else si_print(PRINT_LEVEL_ERR, "FATAL ERROR: CALL NOT FOUND \n\n ");
			return 0;
 		}else{
 	 		 si_ua_ack(pCall, tid, 200);
  			if (pCall)
			{
 			   // YPAPA set semaphore
 			   si_ua_terminateCall(pCall);
  			}else si_print(PRINT_LEVEL_ERR, "FATAL ERROR: CALL NOT FOUND \n\n ");
  			return 0; 
 		}
   	}else{
	 	si_ua_ack(pCall, tid, 415); 
		return 0;
	}
 	si_ua_ack(pCall, tid, 200); 
	return 0;
}

