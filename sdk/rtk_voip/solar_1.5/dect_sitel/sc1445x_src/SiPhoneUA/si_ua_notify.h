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
 * File:		si_ua_notify.h
 * Purpose:		
 * Created:		Nov 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_NOTIFY_H
#define SI_UA_NOTIFY_H
#include "../exosip2/src/eXosip.h"
#include "si_ua_init.h"
int create_cback_notify(sicall_t* pCall, int type,int status, void *info);

void si_cb_callback(int clientId, int codecType, int cbackId, int vline, int accountId,int reason,char* info) ;
void si_cb_callback1(int clientId,int codecType, int cbackId, int vline, int accountId,int reason,incomingInfo_t* info) ;
int si_cb_event(int cbackId, char *pEvent);

int si_notify_MessageWaitingIndication(sicall_t* pCall, eXosip_event_t *pEvent);
int si_notify_dtmfDetect(char *dtmfBody);
int si_notify_BlindTransferProgress(char* Progress);
int si_ua_init_SIPKeepAlive(int did); 

#endif //SI_UA_NOTIFY_H
