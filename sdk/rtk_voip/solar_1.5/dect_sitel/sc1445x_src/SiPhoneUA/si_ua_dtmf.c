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
 * File:		 		 si_ua_dtmf.c
 * Purpose:		 		 
 * Created:		 		 Nov 2007
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
 
#include "si_ua_dtmf.h"
#include "si_ua_notify.h"

int si_ua_rcvDTMF(osip_message_t *req)
{
	char data[64];	
	osip_body_t *pData;
	
	if (req->content_type)
	{
		if (strcmp(req->content_type->type,"application") || strcmp(req->content_type->subtype,"dtmf-relay"))
		  return 415;		
	}
 
	osip_message_get_body(req, 0, &pData);
	if (pData->body)
	{
	 	strcpy(data, pData->body);
		
  	si_notify_dtmfDetect( pData->body);
  }else return 415;
 
	return 200;
}

int si_ua_sendDtmfInfo(sicall_t* pCall, unsigned char number, unsigned char duration)
{
	osip_message_t *info;
	char dtmf_relay[100];
	int ret;
  	eXosip_lock();
 	ret= eXosip_call_build_info(pCall->did, &info);
	if (ret==0)
	{
  	  snprintf(dtmf_relay, 63,"Signal=%c\r\nDuration=%d\r\n",(char)number,(int)duration);	
 	  osip_message_set_content_type (info, "application/dtmf-relay");
 	  osip_message_set_body (info,dtmf_relay, strlen (dtmf_relay));
	  ret= eXosip_call_send_request(pCall->did, info);
	  if (ret) 	osip_message_free (info);

 	} 
 
  	eXosip_unlock();
	return 0;
}
