
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
 * File:		 		 si_ua_notify.c
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
#include "../common/si_timers_api.h" 
#include "si_ua_notify.h"
#include "si_ua_server_api.h"
#include "si_ua_instantmessage.h"
#include "../common/si_phone_api.h"
#include "../CCFSM/ccfsm_init.h"
#include "../exosip2/src/eXosip2.h"

int si_get_mwi_message_number(char *body, int length);

//FIX BROADWORKS - MWI FIX No 2
int si_notify_MessageWaitingIndication(sicall_t* pCall, eXosip_event_t *pEvent)
{
	int ret=0;
	osip_message_t *req= pEvent->request;
	osip_header_t *eventDest=NULL;

	osip_message_header_get_byname (req,"Event",0,&eventDest);
 	if (eventDest)
	{	
		if (strstr(eventDest->hvalue,"presence"))
		{
			sc1445x_pr_recv(pEvent);
			si_print(PRINT_LEVEL_DEBUG, " INCOMING PRESENCE EVENT\n\n\n ");
			return 0;
		}
	}

 	if (req->content_type && req->content_type->type && req->content_type->subtype)
	{
		if (!strcmp(req->content_type->type,"application") && !strcmp(req->content_type->subtype,"simple-message-summary"))
		{
			if (!strcmp(req->content_type->type,"application") && !strcmp(req->content_type->subtype,"simple-message-summary"))
			{
				osip_header_t *sub_state;
				osip_message_header_get_byname (req, "subscription-state", 0, &sub_state);
				if (sub_state && sub_state->hvalue) 
				{
					if (strstr(sub_state->hvalue, "active")){
							osip_body_t *dest=NULL;
							osip_message_get_body (req, 0,&dest);
							if (dest)
								ret = si_get_mwi_message_number(dest->body, dest->length);
					}else if (strstr(sub_state->hvalue, "terminated")){
						si_print(PRINT_LEVEL_DEBUG, "SUB STATE = TERMINATED ..........................\n");

					}else if (strstr(sub_state->hvalue, "pending")){
						si_print(PRINT_LEVEL_DEBUG, "SUB STATE = PENDING ..........................\n");
					}
				}
				if (ret)
					create_cback_notify(pCall, 1,1, NULL);
			}
			return 0;
		} 
	} 

	return 0;
}
int si_get_mwi_message_number(char *body, int length)
{
	char *element;
	char *value;
	int messages =0;
	if (!body) return -1;
	if (length<1) return -1;

	element = strstr(body, "Messages-Waiting");
	if (element) 
	{
		value=strstr(element,"yes" );
		if (value) 
 			messages=1;
  	}
 	element= strstr(body, "voice-message");
 	element= strstr(body, "Voicemail");

  return messages;
}

int si_notify_dtmfDetect( char *dtmfBody)
{
 	return 0;
}

int si_notify_BlindTransferProgress( char* Progress)
{
	 
	return 0;
}

int create_cback_notify(sicall_t* pCall, int type,int status, void *info)
{
   ccfsm_cback_type	m_notifyInd;	 
 
	m_notifyInd.ccfsm_notify_indication_cback.attachedentity =0;
 	 
	m_notifyInd.ccfsm_notify_indication_cback.cback_id=CCFSM_EVENT_NOTIFICATION_EVENT;
	if (pCall)
		m_notifyInd.ccfsm_notify_indication_cback .accountid = pCall->accountID ;
	else
		m_notifyInd.ccfsm_notify_indication_cback .accountid = 0 ;
 
//	m_notifyInd.ccfsm_notify_indication_cback.eventtype=type;   
	m_notifyInd.ccfsm_notify_indication_cback.status=status;   

/*	if (info)
	  strcpy( m_notifyInd.ccfsm_notify_indication_cback.info,info); 
	else
		m_notifyInd.ccfsm_notify_indication_cback.info[0]="\0";
  */
	 ccfsm_callback(&m_notifyInd);
 
	 return 0;
}

int si_ua_send_SIPKeepAlive(int did);
void si_ua_timer_SIPKeepAlive(void);
 
int si_ua_init_SIPKeepAlive(int did)
{
	int value =10000;
	eXosip_set_option(EXOSIP_OPT_UDP_KEEP_ALIVE, &value);  
	// si_addtimerList(&si_ua_timer_SIPKeepAlive,10000000,TIMER_SOURCE_UA, NULL);
	return 0;
}

void si_ua_timer_SIPKeepAlive(void)
{ 
	udp_tl_keepalive();
	si_addtimerList(&si_ua_timer_SIPKeepAlive,10000000,TIMER_SOURCE_UA, NULL);	
}

int si_ua_send_SIPKeepAlive(int did)
{
	int ret;
	osip_message_t *OptionsMsg;

	eXosip_lock();

	ret = eXosip_call_build_options(did, &OptionsMsg);
	if (ret) 
	{
		si_print(PRINT_LEVEL_ERR, "FATAL ERROR creating options msg (ret=%d)\n\n ", ret);
		eXosip_unlock();
		return -1;
	}
	ret = eXosip_call_send_request(did, OptionsMsg);	   			
	if (ret) 
	{
		osip_message_free (OptionsMsg);
		si_print(PRINT_LEVEL_ERR, "FATAL ERROR sending options msg(ret=%d)\n\n ", ret);
		eXosip_unlock();
		return -1;
	}	
	eXosip_unlock();
	return 0;
}
