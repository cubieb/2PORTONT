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
 * File:		si_sc_manager.h
 * Purpose:		
 * Created:		Jan 2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_SC_MANAGER_H
#define SI_SC_MANAGER_H
 
#include "si_ua_init.h"
#include "../../mcu/sc1445x_mcu_block.h"
#include "../../mcu/sc1445x_amb_api.h"
    
int si_manager_session_addcall(	SICORE* pCore, sicall_t *pCall, sdp_message_t *pSDP) ;
int si_manager_session_closecall(sicall_t *pCall) ;
  
sc1445x_mcu_dtmf_mode si_convertDtmfMode(int mode);
int convert_ip_address(unsigned char *IP_address, char *string);
   
 #endif //SI_SC_INIT_H
