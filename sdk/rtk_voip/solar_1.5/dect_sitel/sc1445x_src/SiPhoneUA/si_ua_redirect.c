 
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
 * File:		 		 si_ua_redirect.c
 * Purpose:		 		 
 * Created:		 		 Dec 2007
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */
/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <si_print_api.h>
 
#include "../common/si_phone_api.h"
#include "si_ua_notify.h"
#include "si_ua_redirect.h"
 
int si_ua_redirectcall(sicall_t *pCall, eXosip_event_t *pEvent)
{
	char to[64];
    int ret;
    osip_contact_t *co_redirect=NULL;
 
	ret = osip_message_get_contact(pEvent->response , 0, &co_redirect);
    if (ret<0 || co_redirect==NULL || co_redirect->url==NULL)
    {
		si_print(PRINT_LEVEL_ERR, "\nInvalid Contact \n");
        return -1;
    }
 
	if (co_redirect->url->username)		sprintf(to,"sip:%s@",co_redirect->url->username); 
 	if (co_redirect->url->host)			sprintf(&to[strlen(to)],"%s",co_redirect->url->host); 
	if (co_redirect->url->port)			sprintf(&to[strlen(to)],":%s",co_redirect->url->port);  	
//TO ADD, TO DO 
// CCFSM
//	si_cb_callback(pCall->cbackId,-1, 214, pCall->callid,pCall->accountID,pEvent->response->status_code, to);
 	return 0;
 }

