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
 * File:		 		 si_ua_reliable.c
 * Purpose:		 		 
 * Created:		 		 Apr 2008
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
 
#include "si_ua_reliable.h"

void checkForReliableResponse(sicall_t *pCall, osip_message_t *pPacket)
{
	char *ret_ptr; 
	int ret = 0;
 	osip_header_t *pRequire;
 	osip_header_t *pRSeq;

	if (pPacket==NULL) return ;
	eXosip_lock();
	ret = osip_message_header_get_byname(pPacket, "Require", 0, &pRequire);	
	if (pRequire)
	{
	 	ret_ptr = strstr(pRequire->hvalue, "100rel");
		if (ret_ptr)
		{
			ret = osip_message_header_get_byname(pPacket, "RSeq", 0, &pRSeq);	
			if (pRSeq)
			{
 				 si_ua_sendPRACK(pCall);
			}
		}
	}
  	eXosip_unlock();
}
int si_ua_mustbeReliableRespone(osip_message_t *pInPacket)
{
 	osip_header_t *pRequire;
	osip_header_t *pSupported;
 	char *var1 = NULL;
	int pos = 0;

	//Reliability of provisional response in SIP (RFC 3262)
	// check for an option tag "100rel" in "required" header 
 	osip_message_header_get_byname(pInPacket, "Require", pos, &pRequire);	
 
 	while (pRequire) {
 
 		if (pRequire)   
			var1 = strstr(pRequire->hvalue,"100rel");
 		if (var1) { return 1;}
		pos++;
		osip_message_header_get_byname(pInPacket, "Require", pos, &pRequire);	
	} 
 

	// check for an option tag "100rel" in "supported" header 
	pos = 0;
 	osip_message_header_get_byname(pInPacket, "Supported", pos, &pSupported);	
 

 	while (pSupported) {
 
 		if (pSupported)   
			var1 = strstr(pSupported->hvalue,"100rel");
 		if (var1) { return 1;}
		pos++;
		osip_message_header_get_byname(pInPacket, "Supported", pos, &pSupported);	
	} 

 	return 0;
}

int si_ua_sendPRACK(sicall_t *pCall)
{
	osip_message_t *prack;
	int ret;

	eXosip_lock();
	ret= eXosip_call_build_prack(pCall->tid, &prack);
	if (ret) return -1;
	osip_message_set_supported(prack,SIPHONE_UA_SUPPORTED_HEADER_ACK200_wTIMER );

	ret= eXosip_call_send_prack(pCall->tid, prack);
	if (ret) osip_message_free (prack);
  	eXosip_unlock();
	return 0;
}


