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
 * File:		 		 si_ua_sessiontimer.c
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
#include "si_ua_sessiontimer.h"
#include "si_ua_callcontrol.h"
#include "../common/si_timers_api.h" 

extern  SICORE siphoneCore;
int si_ua_stop_session_timer(sicall_t *pCall)
{
	if (pCall==NULL) return -1;
	si_canceltimerByIndex(pCall->selectedSessionTimer);
	pCall->currentSessionExpireTime=0;
	return 0;
}
int si_ua_start_session_timer(sicall_t *pCall)
{
	//start session timer	
 	if (!siphoneCore.SessionTimerEnable || !pCall) return 0;
	if (pCall->updateKeeper)
	{
		if (pCall->selectedSessionTimer<1){
			si_print(PRINT_LEVEL_DEBUG, "[%s] Session Timer Started ............\n", __FUNCTION__);
	 		pCall->selectedSessionTimer = si_addNonStopTimer(&si_ua_start_session_refresh,((pCall->minSessionExpireTime/3)*1000000),TIMER_SOURCE_UA, (void*)pCall);
 		}
  }
	pCall->currentSessionExpireTime=0;
	return 0;
}

void si_ua_start_session_refresh(void* parameter)
{
	sicall_t *pCall = (sicall_t *)parameter;
	if (pCall==NULL) return ;
	pCall->currentSessionExpireTime+=pCall->minSessionExpireTime/3;
	if (pCall->currentSessionExpireTime>pCall->maxSessionExpireTime)
	{
		si_ua_stop_session_timer(pCall);
		si_ua_terminateCall(pCall);
		return ;
	}

	if (pCall->updateKeeper)
	{
 		switch(pCall->refreshMethod)
		{
			case SI_UA_SESSION_TIMER_REFRESH_METHOD_REINVITE:
				si_ua_send_sessiontimer_reinvite(pCall);
				break;
			case SI_UA_SESSION_TIMER_REFRESH_METHOD_UPDATE:
				si_ua_send_sessiontimer_update(pCall);
				break;
			default:
				si_ua_send_sessiontimer_reinvite(pCall);
		}

	}
}

int si_ua_send_sessiontimer_reinvite(sicall_t *pCall)
{
   if (!siphoneCore.SessionTimerEnable ) return -1;
   if (!pCall || pCall->refreshMethod != SI_UA_SESSION_TIMER_REFRESH_METHOD_REINVITE) return -1;
   si_ua_reInvite(pCall);
	
	 return 0;
}  

int si_ua_send_sessiontimer_update(sicall_t *pCall)
{
   int ret;
   osip_message_t *pUpdate;
   
   if (!siphoneCore.SessionTimerEnable ) return -1;
   if (!pCall || pCall->refreshMethod != SI_UA_SESSION_TIMER_REFRESH_METHOD_UPDATE) return -1;

   eXosip_lock();
    
   ret = eXosip_call_build_update(pCall->did, &pUpdate);
   if (ret) 
   {
      si_print(PRINT_LEVEL_ERR, "FATAL ERROR building update packet \n\n ");
      eXosip_unlock();
      return -1;
   }
 	osip_message_set_supported (pUpdate, SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER);
	// SESSION TIMER 
	if (pCall->updateKeeper && pCall->initiator)	
		osip_message_set_header(pUpdate, "Session-Expires", "180;refresher=uac");
  	else 	if (pCall->updateKeeper && !pCall->initiator)	
		osip_message_set_header(pUpdate, "Session-Expires", "180;refresher=uas");
 	else if (!pCall->updateKeeper && pCall->initiator)	
		osip_message_set_header(pUpdate, "Session-Expires", "180;refresher=uas");
 	else osip_message_set_header(pUpdate, "Session-Expires", "180;refresher=uac");
 
 	osip_message_set_header(pUpdate, "Min-SE", "180");
	osip_message_set_require(pUpdate, "timer");
	ret = eXosip_call_send_request(pCall->did, pUpdate);	   			
   if (ret) 
   {
 	  osip_message_free (pUpdate);
    si_print(PRINT_LEVEL_ERR, "FATAL ERROR sending update packet \n\n ");
    eXosip_unlock();
    return -1;
   }	
   eXosip_unlock();
   return 0;
}

void si_sessiontimer_update(sicall_t *pCall, osip_message_t *pPacket)
{
 	if (!siphoneCore.SessionTimerEnable || !pCall || !pPacket) return;
 	si_sessiontimer_get_min_time(pCall, pPacket);
	si_sessiontimer_get_max_time(pCall, pPacket);
	si_sessiontimer_get_refresher(pCall, pPacket);
	si_sessiontimer_get_refresh_method(pCall, pPacket);

 	si_ua_start_session_timer(pCall);
}

  
void si_sessiontimer_find_update_keeper(sicall_t *pCall ,char *sRefresher)
{
	int initiator = (int)pCall->initiator;
 	if (!siphoneCore.SessionTimerEnable || !pCall) return;
	if (sRefresher==NULL )
	{
		if (!initiator)	pCall->updateKeeper = (int)1;

	}else if (initiator) //we are uac
 	{
		if (!strncmp(sRefresher,"uac",3)) 	pCall->updateKeeper = (int)1;
 		else  pCall->updateKeeper = (int)0;
 	}else {
 		if (!strncmp(sRefresher,"uas",3)) pCall->updateKeeper = (int)1;
 		else  pCall->updateKeeper = (int)0;
  	}
}

int	si_sessiontimer_get_min_time(sicall_t *pCall, osip_message_t *pPacket) 
{
   	osip_header_t *pMinSessionTime;
 	if (pPacket==NULL || pCall==NULL) return -1;
 	osip_message_header_get_byname(pPacket, "Min-SE", 0, &pMinSessionTime);	
  	if (pMinSessionTime && pMinSessionTime->hvalue)
	{
		pCall->minSessionExpireTime= atoi(pMinSessionTime->hvalue);
		if (pCall->minSessionExpireTime<90)
			pCall->minSessionExpireTime=90;
 	}
	return 0;
}
int	si_sessiontimer_get_max_time(sicall_t *pCall, osip_message_t *pPacket) 
{
	char sTimeout[8];
	char *tmp;
   	osip_header_t *pMaxSessionTime;
 	if (pPacket==NULL || pCall==NULL) return -1;
 	osip_message_header_get_byname(pPacket, "Session-Expires", 0, &pMaxSessionTime);	
  	if (pMaxSessionTime && pMaxSessionTime->hvalue)
	{
		tmp = strstr(pMaxSessionTime->hvalue,";");
		if (tmp==NULL) return -1;
		strncpy(sTimeout, pMaxSessionTime->hvalue, (int)tmp-(int)pMaxSessionTime->hvalue);
 		pCall->maxSessionExpireTime= atoi(sTimeout);
   	}
	return 0;

}
int	si_sessiontimer_get_refresher(sicall_t *pCall, osip_message_t *pPacket) 
{
	osip_header_t *pRefresher;
  if (pPacket==NULL || pCall==NULL) return -1;
  	osip_message_header_get_byname(pPacket, "Session-Expires", 0, &pRefresher);	
  if (pRefresher && pRefresher->hvalue)
	{
  	if (strstr(pRefresher->hvalue,"uac")) si_sessiontimer_find_update_keeper(pCall ,"uac" );
		else if (strstr(pRefresher->hvalue,"uas")) si_sessiontimer_find_update_keeper(pCall ,"uas" );
		else si_sessiontimer_find_update_keeper(pCall ,NULL);
  }

	return 0;
}

int	si_sessiontimer_get_refresh_method(sicall_t *pCall, osip_message_t *pPacket) 
{	
	osip_allow_t *allow;
	int pos=0;
	int ret;


  if (pPacket==NULL || pCall==NULL) return -1;

  osip_message_get_allow(pPacket, 0, &allow);	
	for (;;){
	   ret = osip_message_get_allow (pPacket , pos,&allow);
	   if (ret==-1) break;
	   if (allow && allow->value)
	   {
			if (!strcmp(allow->value,"UPDATE")) 
			{ 
				pCall->refreshMethod = SI_UA_SESSION_TIMER_REFRESH_METHOD_UPDATE;
				return 0;
			}
	   } 
 	   pos++;
	}

 	pCall->refreshMethod = SI_UA_SESSION_TIMER_REFRESH_METHOD_REINVITE;

	return 0;
}
void si_sessiontimer_add_refresher_string(sicall_t *pCall, osip_message_t *pPacket)
{
	char minse[8];
	char refresher[24];
	if ((pCall==NULL) || (pPacket==NULL)) return ;	

	sprintf(minse, "%d",  pCall->minSessionExpireTime);
 	
	if (pCall->updateKeeper && pCall->initiator)	
		sprintf(refresher, "%d;refresher=%s",  pCall->minSessionExpireTime,"uac");
	else 	if (pCall->updateKeeper && !pCall->initiator)	
		sprintf(refresher, "%d;refresher=%s",  pCall->minSessionExpireTime,"uas");
	else if (!pCall->updateKeeper && pCall->initiator)	
		sprintf(refresher, "%d;refresher=%s",  pCall->minSessionExpireTime,"uas");
	else sprintf(refresher, "%d;refresher=%s",  pCall->minSessionExpireTime,"uac");

	osip_message_set_header(pPacket, "Session-Expires", refresher);
	osip_message_set_header(pPacket, "Min-SE", minse);
}

