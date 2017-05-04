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
 * File:		si_ua_registrar.h
 * Purpose:		
 * Created:		May 2009
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_REGISTRAR_H
#define SI_UA_REGISTRAR_H

#include "si_ua_init.h"
#include "../CCFSM/ccfsm_api.h"

 //REGISTRATION SECTION
int registration_start(int account, int status);
int checkToReRegister(SICORE* pCore);
int si_ua_unregister(SICORE* pCore, siregistration_t *pRegisterInfo);
siregistration_t *si_ua_get_reg_account(int rid);

int si_ua_register(SICORE* pCore,  siregistration_t *pRegisterInfo, int regEnable);
int si_ua_registrationended(int rid, eXosip_event_t *pEvent, int flag);
int si_ua_registration_init_notification(int account, int flag);
int si_get_account_from_rid(int rid);
//SUBSCRIPTION SECTION
int si_mwi_subscribe(ccfsm_attached_entity attachedentity, 
					 int accountID, 
					 char *subAccount, 
					 char *subRealm, 
					 int expires  );

#endif //SI_UA_REGISTRAR_H
