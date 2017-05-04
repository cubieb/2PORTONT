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
 * File:		si_ua_sessiontimer.h
 * Purpose:		
 * Created:		Apr 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_SESSIONTIMER_H
#define SI_UA_SESSIONTIMER_H
 
#include "si_ua_init.h"
 
void si_sessiontimer_find_update_keeper(sicall_t *pCall ,char *sRefresher);
void si_sessiontimer_update(sicall_t *pCall, osip_message_t *pPacket);
int	si_sessiontimer_get_min_time(sicall_t *pCall, osip_message_t *pPacket);
int	si_sessiontimer_get_max_time(sicall_t *pCall, osip_message_t *pPacket);
int	si_sessiontimer_get_refresher(sicall_t *pCall, osip_message_t *pPacket); 
int	si_sessiontimer_get_refresh_method(sicall_t *pCall, osip_message_t *pPacket) ;
void si_ua_start_session_refresh(void*);
int si_ua_stop_session_timer(sicall_t *pCall);
void si_sessiontimer_add_refresher_string(sicall_t *pCall, osip_message_t *pPacket);
int si_ua_send_sessiontimer_update(sicall_t *pCall);
int si_ua_send_sessiontimer_reinvite(sicall_t *pCall);
int si_ua_start_session_timer(sicall_t *pCall);
#endif //SI_UA_SESSIONTIMER_H
