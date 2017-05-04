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
 * File:		ccfsm_init.h
 * Purpose:		
 * Created:		30/10/2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef CCFSM_INIT_H
#define CCFSM_INIT_H

/*========================== Include files ==================================*/
 
#include "ccfsm_api.h"
#include <sys/types.h>
   
typedef struct _ccfsm_call_list
{
	ccfsm_call_states		state;
 	ccfsm_call_type			calltype;
 	ccfsm_attached_entity	attachedentity;
	ccfsm_audio_peripheral		audio_peripheral_type;
  	int portid;				// 0x80:DECT HANDSET, 0x40:DECT, 0x20:ATA, 0x10:, 0x08:, 0x04:, 0x02:,0x01:ExtSPK, 0x0:Handset
	int reference;
	int callid;
	int accountid;
 	char username[64];
 	char display[64];
 	char host[64];
	char referto[64];
	char referby[64];
	char replace[64];
	int invitewithreplace;
	int timeout;
//  audio_stream_parameters sStreamParameters;
//	sc14450_mcu_audio_media_stream_t mcuMediaStream;
}ccfsm_call_list;

///////////////////////////////////////////////////////////////////////////////////////////////
int ccfsm_set_requests(ccfsm_req_type *msg);
int ccfsm_callback(ccfsm_cback_type* pCBack);
//int ccfsm_indication(ccfsm_callcontrol_indication_type* pInd);

void ccfsm_init(pid_t parID);
void ccfsm_reset(void);
void ccfsm_dtmf(ccfsm_dtmf_req_data *pCallData);
void ccfsm_fax(ccfsm_fax_req_data *pCallData);

void ccfsm_incall_codec_change(ccfsm_incall_codec_change_req_data *pCallData) ;

int ccfsm_change_audio_peripheral_state(ccfsm_audio_peripheral port, ccfsm_audio_peripheral_status status);
void ccfsm_configure(ccfsm_configure_req_data *pCallData);
 
void ccfsm_enterdialingstate(ccfsm_allocate_newcall_req_data *pCallData);
void ccfsm_exitdialingstate(ccfsm_allocate_newcall_req_data *pCallData);
void ccfsm_send_im(ccfsm_send_im_req_data *pIMData); 
int ccfsm_create_newcall(ccfsm_outgoing_call_req_data *pCallData);
int ccfsm_create_internalcall(ccfsm_outgoing_call_req_data *pCallData);
int ccfsm_terminate_call(ccfsm_call_terminate_req_data	*pReqData) ;

int ccfsm_create_newcall_w_replaces(ccfsm_outgoing_call_w_replaces_req_data *pCallData);

void ccfsm_create_newoutcall_cback(ccfsm_outgoing_call_req_data *pCallData, int status);
void ccfsm_create_newincall_cback(ccfsm_incoming_call_ind_data *pNewInCall);
int ccfsm_answer_call(ccfsm_call_answer_req_data	*pReqData) ;
int ccfsm_hold_call(ccfsm_call_hold_req_data *pReqData) ;
int ccfsm_resume_call(ccfsm_call_resume_req_data *pReqData) ;

int ccfsm_attended_transfer(ccfsm_attendedtransfer_req_data *pReqData) ;
int ccfsm_blind_transfer(ccfsm_blindtransfer_req_data *pReqData) ;
ccfsm_call_list*  ccfsm_find_call_per_state(ccfsm_call_states state,int portid);
ccfsm_call_list * ccfsm_get_next_call(ccfsm_call_list *pCall,int portid);

int ccfsm_parse_requests(ccfsm_req_type *pReq);
 
int ccfsm_polling_entry(void * args);
void ccfsm_get_callbacks (int args);
int ccfsm_send_callbacks(ccfsm_cback_type* pCBack);
int ccfsm_send_presence_subscribe(ccfsm_send_presence_subscribe_req_data *pPRData); 
int ccfsm_send_presence_publish(ccfsm_send_presence_publish_req_data *pPRData);

#endif







    