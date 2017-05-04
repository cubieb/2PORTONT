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
 * File:		 		 si_ua_stun.c
 * Purpose:		 		 
 * Created:		 		 Sept 2009
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "si_ua_dns.h"

int si_dns_add_entry(char* host, char* ipaddress)
{
	struct eXosip_dns_cache entry;
 
	strcpy(entry.host, host); 
	strcpy(entry.ip, ipaddress); 

	// FIX BROADWORKS - DNS
	eXosip_set_option(EXOSIP_OPT_ADD_DNS_CACHE,&entry);

	return 0;
}
int si_dns_del_entry(char* host, char* ipaddress)
{

	return 0;
}
char* si_dns_find_entry_by_host(char* host)
{

	return NULL;
}
char* si_dns_find_entry_by_ipaddress(char* ipaddress)
{

	return NULL;
}
int si_dns_set_alternate(osip_transaction_t *tr )//eXosip_event_t *pevent)
{
  //	eXosip_dialog_t *jd = NULL;
 //	eXosip_call_t *jc = NULL;
// 	osip_transaction_t *tr = NULL;
 
//   _eXosip_call_transaction_find (pevent->tid, &jc, &jd, &tr);
 
  if (tr != NULL && tr->record.name[0] != '\0'
      && tr->record.srventry[0].srv[0] != '\0')
    {
      /* always choose the first here.
         if a network error occur, remove first entry and
         replace with next entries.
       */
  
          memmove (&tr->record.srventry[0], &tr->record.srventry[1],
                   9 * sizeof (osip_srv_entry_t));
          memset (&tr->record.srventry[9], 0, sizeof (osip_srv_entry_t));
 
  
          /* copy next element */
     } 
  return 0;
}


 