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
 * File:		 		 si_sc_manager.c
 * Purpose:		 		 
 * Created:		 		 Jan 2008
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

#include "si_sc_manager.h"
#include "si_ua_callcontrol.h"
#include "si_ua_sessiontimer.h"
#include "si_sdp_negotiation.h"
#include "../../mcu/sc1445x_mcu_api.h"
#include "../../mcu/sc1445x_media.h"
#include "../../mcu/sc1445x_srtp.h"
#include "../common/operation_mode_defs.h"

 //FIX OCTOBER 
int si_manager_session_addcall(	SICORE* pCore, sicall_t *pCall, sdp_message_t *pSDP ) 
{
 	int ret;
	int ptime;
	int payload;  
	int cr_id;
  	if (pCall==NULL) return -1;
   	if ((pCall->localHold==1) || (pCall->remoteHold==1) ) return 0;

	// check if audio is enabled reading the sdp of the answer
  	ret = sdp_get_first_supported(&sdpNegotiationData[pCall->accountID],pSDP, &pCall->sStreamParameters);
   	if (ret) {si_print(PRINT_LEVEL_ERR, "ADD CALL - Failed to get SDP \n\n ");return ret;}
	if (invalidport(pCall->sStreamParameters.remote_port) || (invalidport(pCall->public_sdp_audio_port)))
		return -1;
		
	payload=pCall->sStreamParameters.payload;

	if ((ptime = sc1445x_media_get_ptime(payload))==0) 	 
	{
  		ret = sdp_message_get_ptime(pSDP); //v1.0.1.1
		if (validatePtimeValue(ret))
			ptime=ret;
	   	else  ptime = pCore->ptime[pCall->accountID];
	} 
 	{ // Prepare a MCU command
		unsigned short handler;
//  		sc1445x_mcu_media_open(handler,MCU_MTYPE_AUDIO_ID) ;
   		ret = sc1445x_mcu_media_open(&handler,MCU_MTYPE_AUDIO_ID) ;
		if (!ret){
			if (handler)
				pCall->pMcuHandler	=(void*)sc1445x_get_mcu_instance(handler);
			else 			
				si_print(PRINT_LEVEL_ERR, "------------UNABLE TO ALLOCATE NEW INSTANCE 1...... %d \n",handler );
		}else {
			si_print(PRINT_LEVEL_ERR, "------------UNABLE TO ALLOCATE NEW INSTANCE ...... %d \n", ret);
			return -1;
		}
		if (pCall->loop==CALL_REMOTE_LOOPBACK){
			si_print(PRINT_LEVEL_INFO, "ADD A LOOPBACK connection\n ");
		 	pCall->mcuMediaStream.mediaAttr= MCU_MATTR_RLOOP_ID;
 		}
		else
	 		pCall->mcuMediaStream.mediaAttr = MCU_MATTR_BOTH_ID;

		pCall->mcuMediaStream.mediaFormat=MCU_MFMT_RTPAVP_ID;
		pCall->mcuMediaStream.mediaType =MCU_MTYPE_AUDIO_ID;
		pCall->mcuMediaStream.mediaParams.dtmfMode = si_convertDtmfMode(pCore->DtmfMode[pCall->accountID]);
		pCall->mcuMediaStream.mediaParams.dtmfPtype=101;
		pCall->mcuMediaStream.mediaParams.lport =  pCall->public_sdp_audio_port;
 		pCall->mcuMediaStream.mediaParams.rtp_ptype = (unsigned char)pCall->sStreamParameters.d_payload ;
 		pCall->mcuMediaStream.mediaParams.ptype = (unsigned char)pCall->sStreamParameters.payload  ;
		pCall->mcuMediaStream.mediaParams.tos  = pCore->tos[pCall->accountID];
#ifndef SIP_IPv6 
		convert_ip_address((unsigned char*)pCall->mcuMediaStream.mediaParams.rAddress, (char*)pCall->sStreamParameters.remote_address);
#else
		strcpy((unsigned char*)pCall->mcuMediaStream.mediaParams.rAddress, (char*)pCall->sStreamParameters.remote_address);
		printf("CALL MANAGER REMOTE ADDRESS = %s\n\n ",pCall->mcuMediaStream.mediaParams.rAddress );
		printf("\n\n 4++++++++++++++ OPEN LOCAL RTP PORT : [%d] \n\n\n\n ",pCall->mcuMediaStream.mediaParams.lport);
#endif
 
 		pCall->mcuMediaStream.mediaParams.rport = (unsigned int)pCall->sStreamParameters.remote_port ;

#ifdef USE_PCM
 		pCall->mcuMediaStream.mediaParams.fxsLine = pCall->port | 0x80000000; 
#else 
		pCall->mcuMediaStream.mediaParams.fxsLine = pCall->port; 
#endif

 		if (payload==0 || payload==8 ||  payload==18)
			pCall->mcuMediaStream.mediaParams.vad  = pCore->VAD[pCall->accountID]; 
		else 
			pCall->mcuMediaStream.mediaParams.vad =0;
		pCall->mcuMediaStream.mediaParams.Jitter = pCore->jitter [pCall->accountID];

 		pCall->mcuMediaStream.mediaParams.rxRate  =(unsigned short) ptime;
		pCall->mcuMediaStream.mediaParams.txRate  =(unsigned short) ptime;

	// add crypto params
		cr_id = pCall->crypto_parameters.tx_encryption_suite_id;
 		if ( cr_id>=0 && pCall->crypto_parameters.tx_encryption_suite[cr_id].enabled && pCall->crypto_parameters.rx_encryption_suite.enabled) 
		{
 			pCall->mcuMediaStream.mediaParams.srtp_params.cr_au_flag =pCall->crypto_parameters.tx_encryption_suite[cr_id].flag; 
 			pCall->mcuMediaStream.mediaParams.srtp_params.tx_key_len = pCall->crypto_parameters.tx_encryption_suite[cr_id].key_len;
			pCall->mcuMediaStream.mediaParams.srtp_params.tx_suite_len  = pCall->crypto_parameters.tx_encryption_suite[cr_id].suite_len ;
  
 			memcpy(pCall->mcuMediaStream.mediaParams.srtp_params.tx_key,
				pCall->crypto_parameters.tx_encryption_suite[cr_id].key,
				pCall->crypto_parameters.tx_encryption_suite[cr_id].key_len); 

			memcpy(pCall->mcuMediaStream.mediaParams.srtp_params.tx_suite ,
				pCall->crypto_parameters.tx_encryption_suite[cr_id].suite ,
				pCall->crypto_parameters.tx_encryption_suite[cr_id].suite_len ); 
	 		
 
 			pCall->mcuMediaStream.mediaParams.srtp_params.rx_key_len = pCall->crypto_parameters.rx_encryption_suite.key_len  ;
			pCall->mcuMediaStream.mediaParams.srtp_params.rx_suite_len  = pCall->crypto_parameters.rx_encryption_suite.suite_len ;
 
			memcpy(pCall->mcuMediaStream.mediaParams.srtp_params.rx_key,
				pCall->crypto_parameters.rx_encryption_suite .key,
				pCall->mcuMediaStream.mediaParams.srtp_params.rx_key_len ); 
			memcpy(pCall->mcuMediaStream.mediaParams.srtp_params.rx_suite ,
				pCall->crypto_parameters.rx_encryption_suite.suite,
				pCall->mcuMediaStream.mediaParams.srtp_params.rx_suite_len);    
		}else {
			pCall->mcuMediaStream.mediaParams.srtp_params.cr_au_flag=0;
		}


		ret = sc1445x_mcu_configure(pCall->pMcuHandler,MCU_MCTRL_START_ID,&pCall->mcuMediaStream); 	 
		if (ret)
		{
			si_print(PRINT_LEVEL_ERR, "sc1445x_mcu_configure: FATAL ERROR \n");
		}
 		 si_ua_start_session_timer(pCall);
 	}
   	return 0;
}

sc1445x_mcu_dtmf_mode si_convertDtmfMode(int mode)
{
	switch (mode) 
	{
	case DTMF_MODE_INBAND:
		return	MCU_DTMF_INBAND_ID;
	case DTMF_MODE_INFO:
		return MCU_DTMF_OUTBAND_ID;
	case DTMF_MODE_RTPPAYLOAD:
		return MCU_DTMF_2833_ID;
	}
		return	MCU_DTMF_INBAND_ID;
}
 #include "si_ua_sessiontimer.h"
int si_manager_session_closecall(sicall_t *pCall) 
{
 	if ((pCall==NULL) || (pCall->pMcuHandler==NULL))return 0;
   	sc1445x_mcu_configure(pCall->pMcuHandler,MCU_MCTRL_STOP_ID,NULL);
   // sc1445x_mcu_terminate(pCall->pMcuHandler);
	si_ua_stop_session_timer(pCall);
	InitiateCall(pCall);
  	return 0;
}

int convert_ip_address(unsigned char *IP_address, char *string)
{
	char *ptr;
	int num_of_dots = 0;
	int i;
	unsigned int temp[4];
	char string_copy[20];
	char *string_copy_ptr;
	
	strncpy(string_copy, string, 20);
	string_copy[19] = 0;
	string_copy_ptr = string_copy;

	ptr = string_copy_ptr;

	while(*ptr) {
		if(*ptr == '.')
			num_of_dots++;
		else if(!isdigit(*ptr)){
 			return -1;
		}
 		ptr++;
	}

	if(num_of_dots != 3){
 		return -1;
	}
 	ptr = string_copy_ptr;	
	for(i = 0 ; i < 3 ; i++) {
		while(*ptr) {
			if(*ptr == '.') {
				if(ptr - string_copy_ptr) {
					*ptr = 0;
					sscanf(string_copy_ptr, "%u", &temp[i]);
					if(temp[i] > 255){
 						return -1;
					}
 					ptr++;					
					string_copy_ptr = ptr;
					break;
				}else{
 					return -1;
				}
			}
 			ptr++;
		}
	}

	if(strlen(string_copy_ptr)) {
		sscanf(string_copy_ptr, "%u", &temp[3]);
		if(temp[3] > 255){
 			return -1;
		}
	}else{
  		return -1;
	}
	for(i = 0 ; i < 4 ; i++)
		IP_address[i] = (unsigned char)temp[i];

 	return 0;
}

