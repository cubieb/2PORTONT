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

#ifndef SI_UA_STUN_H
#define SI_UA_STUN_H
#include "../exosip2/src/eXosip.h"
#include "si_ua_init.h"

#define  SIPHONE_IPADDR_SIZE 64
int stun_get_nattype(SICORE* pCore);

int select_audio_port(SICORE* pCore,sicall_t *pCall, char *LocalAudioBuffer,char *PublicAudioBuffer);

int getPublicPort(SICORE* pCore, char *local_voice_port, char *public_voice_port );
void get_local_audio_sdp_port(SICORE* pCore, char *buf);

int stun_get_localip(SICORE* pCore, char *result, int *);

#endif //SI_UA_STUN_H






