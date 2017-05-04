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
 * File:		si_ua_hold.h
 * Purpose:		
 * Created:		Nov 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_HOLD_H
#define SI_UA_HOLD_H
#include "../exosip2/src/eXosip.h"
#include "si_ua_init.h"

int si_ua_processReinvitetoHoldOn(eXosip_event_t* pEvent);
int si_ua_holdOff( sicall_t* pCall);
int si_ua_holdOn(sicall_t* pCall, int type);
void holdOffTimerCallBack(void);
void holdOnTimerCallBack(void);
int si_ua_checkSDPForChanges(sdp_message_t *sdp, sicall_t *pCall, audio_stream_parameters *pStreamParams, int, unsigned char* , int);
void si_remoteCodecChanged(sicall_t *pCall,int newpayload );
int si_ua_codecChange( sicall_t* pCall );  
void si_audio_stream_resume(sicall_t* pCall );
void si_audio_stream_suspend(sicall_t* pCall );

#endif //SI_UA_HOLD_H
