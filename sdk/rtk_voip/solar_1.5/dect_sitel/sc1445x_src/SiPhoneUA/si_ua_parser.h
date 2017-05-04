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
 * File:		si_ua_parser.h
 * Purpose:		
 * Created:		Nov 2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_PARSER_H
#define SI_UA_PARSER_H
#include "../exosip2/src/eXosip.h"
#include "si_ua_init.h"

int getRemoteSDPInfo(sdp_message_t * sdp, unsigned char* remoteaddress, int *remoteport);
int get_c_address_port(sdp_message_t * sdp, unsigned char* remoteaddress, int *remoteport);
int check_incoming_sdp(	sicall_t* pCall, sdp_message_t * sdp,int flag);

#endif //SI_UA_HOLD_H
