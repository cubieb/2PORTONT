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
 * (C) Copyright SiTel Semiconductor BV, unpublished work.
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		si_ua_events.h
 * Purpose:		
 * Created:		Nov 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_EVENTS_H
#define SI_UA_EVENTS_H
#include "../exosip2/src/eXosip.h"
#include "si_ua_init.h"

int si_ua_process(void);
int si_ua_thread(void * arg);
int si_ua_parseNewMessageRequest(sicall_t* pCall, eXosip_event_t* pEvent);
int si_ua_parseMessageRequestFailure(sicall_t *pCall, eXosip_event_t* pEvent);

int si_ua_ack(sicall_t*, int tid, int code);
int si_ua_ackoptions( int tid, int code);
int si_ua_retry_invite(eXosip_event_t *pevent,sicall_t *pCall);
 


 
#endif //SI_UA_EVENTS_H
