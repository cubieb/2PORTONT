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
 * File:		 		 si_ua_fax.c
 * Purpose:		 		 
 * Created:		 		 feb 2009
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

#include "si_ua_fax.h"
#include "../../mcu/sc1445x_mcu_api.h"
#include "si_sdp_negotiation.h"
#include "si_sc_manager.h"
   
extern  SICORE siphoneCore;


ata_t38_sdp_params_type *fax_sdp_params;

int sc1445x_fax_params(void *params, int size ) 
{
  	fax_sdp_params = (ata_t38_sdp_params_type *) params;
		return 0;//fix April 29, 2009
}

int sc1445x_fax_switch_mode(sicall_t* pCall)
{
	if( pCall->faxsupport != 3 )
    {
       si_print(PRINT_LEVEL_ERR, "[%s] Received Re-invite without FAX Detection");
	  //	calltermination();	
    }
 	pCall->faxsupport = 1;
	pCall->mcuMediaStream.mediaFormat= MCU_MFMT_T38_ID;
	pCall->mcuMediaStream.mediaType = MCU_MTYPE_FAX_ID;
	pCall->mcuMediaStream.mediaParams.lport =  pCall->public_sdp_audio_port;
	pCall->mcuMediaStream.mediaParams.rport =   pCall->sStreamParameters.remote_port;
	convert_ip_address((unsigned char*)pCall->mcuMediaStream.mediaParams.rAddress, 
		(char*)pCall->sStreamParameters.remote_address);

	sc1445x_mcu_configure(pCall->pMcuHandler,MCU_MCTRL_MODIFY_ID,&pCall->mcuMediaStream);   	 
	return 0;
}

int sc1445x_fax_open_connection(sicall_t* pCall) 
{
 	osip_negotiation_t sdpInfo;
  sdp_message_t *sdp=NULL;
	osip_message_t *invite;
	char *sdpbody;
	int ret;
  char *rtpport;   

	if (pCall==NULL) return -1;
	if (pCall->faxsupport) return -1;
	pCall->faxsupport =2;  
	eXosip_sdp_negotiation_init(pCall->accountID,  &sdpInfo, &siphoneCore); 
 	eXosip_lock();
	rtpport = eXosip_sdp_get_image_port(pCall);
 
	// set faxsupport field and prepare a sdp body for fax
 	sdp = eXosip_create_sdp_offer(pCall, invite, &sdpInfo,rtpport,rtpport,0);
    
	if (sdp==NULL)
	{
	  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to open a fax connection 1\n"); 
	  eXosip_unlock();
	  return -1;
	}
 	 
	 ret=eXosip_call_build_request(pCall->did, "INVITE", &invite); 
	 if (ret) 
	 {
		sdp_message_free(sdp);
		FatalError(EXOSIP_HOLD_ON_FAILED, "failed to open a fax connection 2\n"); 
		eXosip_unlock();
		return -1;
	 } 
	  
 	 ret = sdp_message_to_str(sdp, &sdpbody);

	 if (sdpbody)
	 {
		  osip_message_set_supported (invite, SIPHONE_UA_SUPPORTED_HEADER_INVITE);
		  osip_message_set_content_type (invite, "application/sdp");
		  osip_message_set_allow (invite, SipAllowHeader);
		  osip_message_set_body (invite,sdpbody, strlen (sdpbody));
 	 }else
	 {
		  sdp_message_free(sdp);
		  FatalError(EXOSIP_HOLD_ON_FAILED, "failed to open a fax connection 3"); 
		  eXosip_unlock();
		  return -1;
	}
	  
	ret = eXosip_call_send_request(pCall->did, invite);
	if (ret!=0) 
	{
	  sdp_message_free(sdp);
	  //YPAPA SOS
	   osip_message_free (invite);
	   FatalError(EXOSIP_HOLD_ON_FAILED, "failed to open a fax connection 4"); 
	  eXosip_unlock();
	  return -1;
	} 


	if (sdp) sdp_message_free(sdp);
	if (sdpbody) osip_free(sdpbody);
	eXosip_unlock();
	return 0;
}

