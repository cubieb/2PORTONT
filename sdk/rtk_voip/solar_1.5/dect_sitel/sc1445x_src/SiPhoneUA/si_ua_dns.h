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
 * File:		si_ua_dns.h
 * Purpose:		
 * Created:		Sept 2009
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_DNS_H
#define SI_UA_DNS_H
#include <si_print_api.h>
#include "../exosip2/src/eXosip.h"
#include "../exosip2/src/eXosip2.h"
 

#include "si_ua_init.h"

int si_dns_add_entry(char* host, char* ipaddress);
int si_dns_del_entry(char* host, char* ipaddress);
char* si_dns_find_entry_by_host(char* host);
char* si_dns_find_entry_by_ipaddress(char* ipaddress);

//int si_dns_set_alternate(eXosip_event_t *pevent);
int si_dns_set_alternate(osip_transaction_t *tr );//eXosip_event_t *pevent)
  
#endif //SI_UA_DNS_H






