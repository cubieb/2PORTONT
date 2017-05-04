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
 * File:		 		sc1445x_phone_api.c
 * Purpose:		 		 
 * Created:		 	Sep 2008
 * By:		 		 	YP
 * Country:		 	Greece
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>


#include <si_print_api.h>

#include "sc1445x_phone_api.h"
#include "ccfsm_api.h"
#include "ccfsm_init.h"  
#include "../common/si_call_log_api.h"
#include "../../mcu/sc1445x_mcu_api.h"
#include "../../mcu/sc1445x_amb_api.h"

#include "../SiDECT/cvmcon.h"
#include "../ConsoleIFACE/si_cc_server_api.h"
#include "../common/si_configfiles_api.h"
#include "../SiPhoneUA/si_user_agent.h"

extern ConfigSettings m_Config_Settings;
void sc1445x_phoneapi_show_mwi(ccfsm_cback_type* pCBack);
gui_conference_data_t conference_data;

#ifdef GUI_ENABLED
	#ifdef TEXT_USER_INTERFACE
		#include "../TUI/scr_main_state_machine.h"
		#include  "../TUI/sc1445x_gui_main.h"
		#include "../TUI/kbd_driver_api.h"
	#elif defined TFT_USER_INTERFACE
		#include "../TFT_GUI/scr_main_state_machine.h"
		#include  "../TFT_GUI/sc1445x_gui_main.h"
    #include "../TFT_GUI/kbd_driver_api.h"
	#else
		#include "../GUI/scr_main_state_machine.h"
		#include  "../GUI/sc1445x_gui_main.h"
    #include "../GUI/kbd_driver_api.h"
	#endif  
#endif
 
#ifdef ATA_ENABLED 
	#include "../SiATA/si_ata_int.h"  
	#include "../SiATA/si_ata_callcontrol.h"  
	#define led_set(x,y) 
	#define led_unset(x) 
#endif

#ifdef DECT_ENABLED  
	#include "../SiDECT/si_dect_callcontrol.h"   
	#define led_set(x,y) 
	#define led_unset(x)
#endif

typedef struct _CodecNameTable_t
{
	int num;
 	char *name;
}CodecNameTable_t;

CodecNameTable_t CodecNameTable[]={
 {9, "G722"},
 {0, "PCMU"},
 {8, "PCMA"},
 {18,"G729"},
 {2, "G726"},
 {98, "iLBC"},
 {99, "iLBC-20"},
 {0,NULL} 
};

char *returnCodecName(int cnum)
{
 int i;
 cnum =cnum & 0x0fffffff; 
 for (i=0;CodecNameTable[i].name;i++)
 {
  if (CodecNameTable[i].num==cnum)
  {
   return CodecNameTable[i].name;
  }
 }
 return NULL;
}

sc1445x_phone_active_calls_list m_active_calls_list[GUI_MAX_CALLS_SUPPORTED];
phone_api phone_function[CCFSM_ATTACHED_ENUM_SIZE ];
struct sigaction myactions;
sigset_t myblock_mask;
 
int sc1445x_phoneapi_init(void)
{
	memset(phone_function, 0, sizeof(phone_api)*CCFSM_ATTACHED_ENUM_SIZE);

	#ifdef GUI_ENABLED
 			phone_function[CCFSM_ATTACHED_GUI].init = gui_main;
  		phone_function[CCFSM_ATTACHED_GUI].processcalls = sc1445x_gui_process_calls;
  		phone_function[CCFSM_ATTACHED_GUI].phoneevents = sc1445x_gui_event_status;
  		phone_function[CCFSM_ATTACHED_GUI].ring = sc1445x_gui_ring;
    	phone_function[CCFSM_ATTACHED_GUI].retransmit = NULL;
     	phone_function[CCFSM_ATTACHED_GUI].assignedline = sc1445x_gui_assignedline;
     	phone_function[CCFSM_ATTACHED_GUI].improcess = gui_application_im_process;
			phone_function[CCFSM_ATTACHED_GUI].terminatecall = sc1445x_gui_terminate_call;
	#endif	
/*#ifdef DECT_HEADSET	
		phone_function[CCFSM_ATTACHED_DECT].init =  NULL;
  		phone_function[CCFSM_ATTACHED_DECT].processcalls =NULL ;
  		phone_function[CCFSM_ATTACHED_DECT].retransmit =NULL ;
	#endif
*/
  #ifdef DECT_ENABLED
		si_print(PRINT_LEVEL_INFO, "DECT_ENABLED\n ");
		phone_function[CCFSM_ATTACHED_DECT].init = NULL;
		phone_function[CCFSM_ATTACHED_DECT].processcalls = NULL;
		phone_function[CCFSM_ATTACHED_DECT].phoneevents = NULL;
		phone_function[CCFSM_ATTACHED_DECT].ring = sc1445x_dect_terminal_ring;
		phone_function[CCFSM_ATTACHED_DECT].retransmit = NULL;
		phone_function[CCFSM_ATTACHED_DECT].terminatecall = sc1445x_dect_terminate_call;
		sc1445x_phoneapi_register(0, 0, 1);
	#endif

	#ifdef ATA_ENABLED
		si_print(PRINT_LEVEL_INFO, "ATA_ENABLED\n ");
 		phone_function[CCFSM_ATTACHED_ATA].init = ata_main;
   	phone_function[CCFSM_ATTACHED_ATA].processcalls = NULL;
		phone_function[CCFSM_ATTACHED_ATA].phoneevents = NULL;
  	phone_function[CCFSM_ATTACHED_ATA].ring =sc1445x_ata_terminal_ring;
   	phone_function[CCFSM_ATTACHED_ATA].retransmit = NULL;
   	phone_function[CCFSM_ATTACHED_ATA].terminatecall = sc1445x_ata_terminate_call;
	#endif

	#ifdef USE_PCM
		sc1445x_phoneapi_init_pcm_ports();
	#endif

 	#ifdef CONSOLE_ENABLED
		phone_function[CCFSM_ATTACHED_CONSOLE].init = NULL ;
   	phone_function[CCFSM_ATTACHED_CONSOLE].processcalls = NULL;
		phone_function[CCFSM_ATTACHED_CONSOLE].phoneevents = NULL;
		phone_function[CCFSM_ATTACHED_CONSOLE].ring = NULL;
 		phone_function[CCFSM_ATTACHED_CONSOLE].retransmit = sc1445x_CONSOLE_RETRANSMIT;
	#endif

	

 	if (phone_function[CCFSM_ATTACHED_GUI].init )
		phone_function[CCFSM_ATTACHED_GUI].init();

	if (phone_function[CCFSM_ATTACHED_ATA].init )
   		phone_function[CCFSM_ATTACHED_ATA].init();

	if (phone_function[CCFSM_ATTACHED_DECT].init ){
 	  	phone_function[CCFSM_ATTACHED_DECT].init();
	}
	return 0;
}
  

void sc1445x_phoneapi_application_callback(ccfsm_cback_type* pData)
{
	sc1445x_phone_call_info* pCall;
  
	switch(pData->cback_id)
	{
		case CCFSM_REGISTRATION_EVENT:
   			sc1445x_phoneapi_show_registration(pData);
 		break;
 		case CCFSM_NEW_ALLOCATE_CB:
   			break;
		case CCFSM_NEW_DEALLOCATE_CB:
   			break;
  	case CCFSM_NEW_CALL_ANSWERED_CB:
  	case CCFSM_NEW_CALL_CB:
 				if (!pData->ccfsm_new_call_cback.status)
    				sc1445x_phoneapi_add_call(pData);		 
				else {
					si_print(PRINT_LEVEL_ERR, "Unable to establish the new call \n");
				}
 				break;
 		case CCFSM_TERMINATING_CB:
 			sc1445x_phoneapi_modify_call_state(pData); //FIX 10082009
 			break;
     	case CCFSM_ANSWERED_CB:
 			pCall = sc1445x_phoneapi_find_call_withID(pData->ccfsm_updatestatus_cback .callid );
  			if (pCall /*&& pCall->state == CCFSM_STATE_INCOMING*/)//FIX 10082009
			{
  				sc1445x_phoneapi_ringmode(pData->ccfsm_updatestatus_cback.attachedentity, 
				STOP_RINGING, 
				pCall->portid, pCall->accountid, 
				pCall->codec_id,
		 		sc1445x_phoneapi_find_numof_calls(pCall->portid), NULL);
			}else {
  				si_print(PRINT_LEVEL_DEBUG, "CCFSM_ANSWERED_CB: INVALID CALL STATE ON PHONEAPI \n" );
			}
 			sc1445x_phoneapi_modify_call_state(pData);
			break;
		case CCFSM_RINGING_CB:
		case CCFSM_HOLD_CB:
		case CCFSM_HELD_CB:
 		case CCFSM_INACTIVE_CB:
 		case CCFSM_RESUMED_CB:
 		case CCFSM_BLINDTRANSFERRING_CB:
		case CCFSM_BLINDTRANSFERED_CB:
		case CCFSM_ATTENDEDTRANSFERRING_CB:
		case CCFSM_ATTENDEDTRANSFERED_CB:
 		case CCFSM_TRANSFER_ACCEPTED_CB:
 		case CCFSM_INCALL_CODEC_CHANGED_CB:  
 
   			sc1445x_phoneapi_modify_call_state(pData);
   			break;
		case CCFSM_TERMINATED_CB:
  			pCall = sc1445x_phoneapi_find_call_withID(pData->ccfsm_updatestatus_cback .callid );
			if (pCall && pCall->state == CCFSM_STATE_INCOMING)	
			{
  				sc1445x_phoneapi_ringmode(pCall->attached, STOP_RINGING, pCall->portid, 
				pCall->accountid, pCall->codec_id,sc1445x_phoneapi_find_numof_calls(pCall->portid), NULL);
			} 
  			sc1445x_phoneapi_del_call( pData);		 
  
   		break;
		case CCFSM_ATTENDEDTRANSFER_IND:
 		case CCFSM_BLINDTRANSFER_IND:
 			sc1445x_phoneapi_replace_call(pData);
 			break;
		case CCFSM_REJECTED_CB:
  				pCall = sc1445x_phoneapi_find_call_withID(pData->ccfsm_updatestatus_cback .callid );
				if (pCall){
   					if (pCall->state == CCFSM_STATE_INCOMING)
						sc1445x_phoneapi_ringmode(pData->ccfsm_updatestatus_cback.attachedentity, STOP_RINGING, pCall->portid, pCall->accountid, pCall->codec_id, sc1445x_phoneapi_find_numof_calls(pCall->portid), NULL); 
					strcpy(pCall->info,pData->ccfsm_updatestatus_cback.info);
 					sc1445x_phoneapi_del_call( pData);
				}			 
   			break;
		case CCFSM_REDIRECTED_CB:
			 
 			break;
  		case CCFSM_IM_RECV_IND:  
   			sc1445x_phoneapi_im_process(pData);
  			break;

  		case CCFSM_PRESENCE_NOTIFY_IND:
				 
  			sc1445x_phoneapi_presence_notify_process(pData);
  			break;
  		case CCFSM_PRESENCE_SUBSCRIBE_CB:
  			sc1445x_phoneapi_presence_subscribe_process(pData);
  		break;
		case CCFSM_EVENT_NOTIFICATION_EVENT: //FIX BROADWORKS
  			sc1445x_phoneapi_show_mwi(pData);
			break;
		default:
			break;
  }
}

#include "../common/si_configfiles_api.h"
int sc1445x_phoneapi_reset(void) //fix 07072009
{
 	ccfsm_req_type m_CallReq;
	m_CallReq.ccfsm_reset_req.req_id = CCFSM_RESET_REQ;
 	LoadAllConfigFiles(&m_Config_Settings);
   	ccfsm_set_requests(&m_CallReq);
	return 0;
}

int sc1445x_phoneapi_configure(ccfsm_attached_entity attachedentity, void* params, int size)
{
	ccfsm_req_type m_newReq;

	m_newReq.ccfsm_configure_req.req_id =CCFSM_CONFIGURE_REQ;
	m_newReq.ccfsm_configure_req.attachedentity = attachedentity;
 	m_newReq.ccfsm_configure_req.params = params;
	m_newReq.ccfsm_configure_req.size = size;
 	ccfsm_set_requests(&m_newReq);

	return 0;//fix April 29, 2009
}

int sc1445x_phoneapi_register(ccfsm_attached_entity attachedentity, int account, int flag)
{
	ccfsm_req_type	 m_req;
 
	m_req.ccfsm_register_req.req_id = CCFSM_REGISTER_REQ;
	m_req.ccfsm_register_req.attachedentity = attachedentity;
	m_req.ccfsm_register_req.account = account;
	m_req.ccfsm_register_req.status = flag;
 	ccfsm_set_requests(&m_req);
	return 0;
}





//  Request to Call Control  - Send an instant message
int sc1445x_phoneapi_send_im(ccfsm_attached_entity attachedentity, unsigned char * dialnumber, char *msg,int portid, int account)
{
	int direct; 
	ccfsm_req_type m_SendIMReq;
	
	m_SendIMReq.ccfsm_send_im_req .req_id = CCFSM_IM_SEND_REQ;
 	m_SendIMReq.ccfsm_send_im_req.attachedentity = attachedentity; 
 
 	m_SendIMReq.ccfsm_send_im_req.portid = portid;
	m_SendIMReq.ccfsm_send_im_req.accountid = account;
	if (strlen(msg) >= MAX_IM_TEXT_SIZE)
		return -1;

	strcpy(m_SendIMReq.ccfsm_send_im_req.text, msg);
	direct = sc1445x_phoneapi_CheckNumber(m_SendIMReq.ccfsm_send_im_req.dialled_num, (char *)dialnumber, m_Config_Settings.m_SIPSettings.registrar);

	if (direct==1) 
 		m_SendIMReq.ccfsm_send_im_req.calltype = CCFSM_DIRECTION_OUT_IPCALL;
 	else if (direct==0) 
 		m_SendIMReq.ccfsm_send_im_req.calltype = CCFSM_DIRECTION_OUT;
 	else  
 		return -1;
   
 	ccfsm_set_requests(&m_SendIMReq);
	return 0;
}

int sc1445x_phoneapi_create_newcall(ccfsm_attached_entity attachedentity, unsigned char * dialnumber, int portid, int account, ccfsm_codec_type codec, int reference)
{
	int direct; 
	ccfsm_req_type m_newCallReq;
	
	m_newCallReq.ccfsm_outgoing_call_req.req_id =CCFSM_OUTGOING_CALL_REQ;
	m_newCallReq.ccfsm_outgoing_call_req.audio_status =CCFSM_NO_AUDIO_INFO;
	m_newCallReq.ccfsm_outgoing_call_req.attachedentity =attachedentity; 
	m_newCallReq.ccfsm_outgoing_call_req.audio_peripheral_type = CCFSM_AUDIO_DESKTOP;  

	m_newCallReq.ccfsm_outgoing_call_req.codec = codec;
	m_newCallReq.ccfsm_outgoing_call_req.portid =portid;
	m_newCallReq.ccfsm_outgoing_call_req.accountid = account;
	m_newCallReq.ccfsm_outgoing_call_req.callid =-1;
  m_newCallReq.ccfsm_outgoing_call_req.reference=reference;
 
	direct = sc1445x_phoneapi_CheckNumber(m_newCallReq.ccfsm_outgoing_call_req.dialled_num, (char *)dialnumber, m_Config_Settings.m_SIPSettings.registrar);
	if (direct==1){
		m_newCallReq.ccfsm_outgoing_call_req.calltype = CCFSM_DIRECTION_OUT_IPCALL;
	}
	else if (direct==0){
  	m_newCallReq.ccfsm_outgoing_call_req.calltype = CCFSM_DIRECTION_OUT;
	}
	else {
		si_print(PRINT_LEVEL_CRIT, "FATAL ERROR on dial number \n ");
		return -1;
	}

 	ccfsm_set_requests(&m_newCallReq);
	return 0;
}

int sc1445x_phoneapi_create_internalcall(ccfsm_attached_entity attachedentity, unsigned char *dialnumber, int portid, int account, ccfsm_codec_type codec, int reference)
{
	ccfsm_req_type m_newCallReq;

	m_newCallReq.ccfsm_outgoing_call_req.req_id = CCFSM_INTERNAL_CALL_REQ;
	m_newCallReq.ccfsm_outgoing_call_req.audio_status = CCFSM_NO_AUDIO_INFO;
	m_newCallReq.ccfsm_outgoing_call_req.attachedentity = attachedentity; 
	m_newCallReq.ccfsm_outgoing_call_req.audio_peripheral_type = CCFSM_AUDIO_DESKTOP;  
	m_newCallReq.ccfsm_outgoing_call_req.codec = codec;
	m_newCallReq.ccfsm_outgoing_call_req.portid = portid;
	m_newCallReq.ccfsm_outgoing_call_req.accountid = account;

	m_newCallReq.ccfsm_outgoing_call_req.callid = -1;
  m_newCallReq.ccfsm_outgoing_call_req.reference = reference;
	strcpy(m_newCallReq.ccfsm_outgoing_call_req.dialled_num, (char *)dialnumber);

	m_newCallReq.ccfsm_outgoing_call_req.calltype = CCFSM_DIRECTION_INTERNAL_CALL;

 	ccfsm_set_requests(&m_newCallReq);
	return 0;
}
 
int	sc1445x_phoneapi_attendedtransfer( int fromid, int toid)  
{
 	ccfsm_req_type m_CallAttendedTransferReq;
  	 
	m_CallAttendedTransferReq.ccfsm_attendedtransfer_req.req_id =CCFSM_ATTENDEDTRANSFER_REQ;
	m_CallAttendedTransferReq.ccfsm_attendedtransfer_req.callid  = toid;
	m_CallAttendedTransferReq.ccfsm_attendedtransfer_req.calltotransfer= fromid;

 	ccfsm_set_requests(&m_CallAttendedTransferReq);
 	return 0;
}

int	sc1445x_phoneapi_blindtransfer(int callid,  char *number)
{
 	ccfsm_req_type m_CallBlindTransferReq;

	m_CallBlindTransferReq.ccfsm_blindtransfer_req.req_id = CCFSM_BLINDTRANSFER_REQ;
	m_CallBlindTransferReq.ccfsm_blindtransfer_req.callid = callid;
 	strcpy((char*)m_CallBlindTransferReq.ccfsm_blindtransfer_req.number, (char*)number);
 	ccfsm_set_requests(&m_CallBlindTransferReq);

 	return 0;
}

int sc1445x_phoneapi_call_answer(int callid, ccfsm_attached_entity attached, int port, int codec, int accountid)
{
  ccfsm_req_type m_CallAnswerReq;
 
  m_CallAnswerReq.ccfsm_call_answer_req.req_id = CCFSM_ANSWER_REQ;
  m_CallAnswerReq.ccfsm_call_answer_req.callid  = callid;
  m_CallAnswerReq.ccfsm_call_answer_req.portid  = port;
	m_CallAnswerReq.ccfsm_call_answer_req.accountid  = accountid;
  m_CallAnswerReq.ccfsm_call_answer_req.attachedentity = attached;
  m_CallAnswerReq.ccfsm_call_answer_req.codec =  codec;
  sc1445x_phoneapi_update_port(callid, port);
	sc1445x_phoneapi_update_accountid(callid, accountid);//for dect broadcast update
  sc1445x_phoneapi_setled(callid, 1);
  ccfsm_set_requests(&m_CallAnswerReq);
 
  return 0;
}

int sc1445x_phoneapi_call_terminate(int callid, ccfsm_attached_entity attached)
{
 	ccfsm_req_type m_CallTerminateReq;
   	m_CallTerminateReq.ccfsm_call_terminate_req.req_id =CCFSM_TERMINATE_REQ;
	m_CallTerminateReq.ccfsm_call_terminate_req.callid = callid;
	m_CallTerminateReq.ccfsm_call_terminate_req.attachedentity = attached;
	sc1445x_phoneapi_setled(callid, 0);
 	ccfsm_set_requests(&m_CallTerminateReq);
 	return 0;
}
    
int sc1445x_phoneapi_incall_codec_change(int callid, int codec)
{
	ccfsm_req_type m_CallReq;
 	m_CallReq.ccfsm_incall_codec_change_req.req_id = CCFSM_INCALL_CODEC_CHANGE_REQ;
	m_CallReq.ccfsm_incall_codec_change_req.callid = callid;
 	ccfsm_set_requests(&m_CallReq);
	return 0;
}
  
int sc1445x_phoneapi_call_hold(int callid)
{
 	ccfsm_req_type m_CallReq;
	m_CallReq.ccfsm_call_hold_req.req_id = CCFSM_CALL_HOLD_REQ;
	m_CallReq.ccfsm_call_hold_req.callid = callid;
 	ccfsm_set_requests(&m_CallReq);
	sc1445x_phoneapi_setled(callid, 2);
	return 0;
}

int sc1445x_phoneapi_call_resume(int callid)
{
 	ccfsm_req_type m_CallReq;
	m_CallReq.ccfsm_call_resume_req.req_id = CCFSM_CALL_RESUME_REQ;
	m_CallReq.ccfsm_call_resume_req.callid = callid;
	sc1445x_phoneapi_setled(callid, 1);

 	ccfsm_set_requests(&m_CallReq);
	return 0;
}

int sc1445x_phoneapi_dtmf_stop(void)
{
  sc1445x_mcu_set_tone(MCU_TONE_NA_BT, (sc1445x_mcu_key)0,(unsigned short)0);
	return 0;
}

int sc1445x_phoneapi_dtmf_start(char key, int port)
{
 	ccfsm_req_type m_CallReq;

 	if((key >= 0) && (key <= 8))
		key=key+'1';
	else if(key == 9)
		key=0x2a;
 	else if(key == 10)
		key='0';
 	else if(key == 11)
		key=0x23;

	sc1445x_mcu_set_tone(MCU_TONE_DT_BT, (sc1445x_mcu_key)(key), (unsigned short)port);
 	m_CallReq.ccfsm_dtmf_req.req_id = CCFSM_DTMF_REQ;
 	m_CallReq.ccfsm_dtmf_req.key = key;
 	ccfsm_set_requests(&m_CallReq);
 	return 0;
}
 
void sc1445x_phoneapi_show_registration(ccfsm_cback_type* pCBack) 		 
{
 	if (phone_function[pCBack->ccfsm_registration_cback.attachedentity].phoneevents)
	  	phone_function[pCBack->ccfsm_registration_cback.attachedentity].phoneevents(CCFSM_REGISTRATION_EVENT, pCBack->ccfsm_registration_cback.accountid, pCBack->ccfsm_registration_cback.status , NULL);

   sc1445x_phoneapi_retransmit(pCBack->ccfsm_registration_cback.attachedentity,(void*)pCBack,sizeof(ccfsm_cback_type));
  
}
 
void sc1445x_phoneapi_show_mwi(ccfsm_cback_type* pCBack) 		 
{
	if (phone_function[pCBack->ccfsm_notify_indication_cback.attachedentity].phoneevents)
		phone_function[pCBack->ccfsm_notify_indication_cback.attachedentity].phoneevents(CCFSM_EVENT_NOTIFICATION_EVENT, pCBack->ccfsm_notify_indication_cback.accountid, pCBack->ccfsm_notify_indication_cback.status ,NULL);
//   sc1445x_phoneapi_retransmit(pCBack->ccfsm_notify_indication_cback.attachedentity,(void*)pCBack,sizeof(ccfsm_cback_type));
}

 void sc1445x_phoneapi_add_call(ccfsm_cback_type* pData) 		 
{
	sc1445x_phone_call_info *call_info_ptr=NULL;
	int i;
	 
	for(i = 0 ; i < GUI_MAX_CALLS_SUPPORTED ; i++) {
		if(m_active_calls_list[i].call_info.state == CCFSM_STATE_IDLE) {
			call_info_ptr = &m_active_calls_list[i].call_info;
	  	call_info_ptr->attached =pData->ccfsm_new_call_cback.attachedentity; 
 			call_info_ptr->state = pData->ccfsm_new_call_cback.state;  
			call_info_ptr->reference = pData->ccfsm_new_call_cback.reference ;  
			call_info_ptr->calltype = pData->ccfsm_new_call_cback.calltype;  
			call_info_ptr->callid   = pData->ccfsm_new_call_cback.callid;  
			call_info_ptr->portid  = pData->ccfsm_new_call_cback.portid ;  
			call_info_ptr->encryption = 0;
			call_info_ptr->accountid = pData->ccfsm_new_call_cback.accountid; 
			call_info_ptr->inConference = 0;//FIX 05082009
			strcpy(call_info_ptr->user_name,pData->ccfsm_new_call_cback.username_num); 

 			if ( strlen(call_info_ptr->user_name) <1) 
				strcpy(call_info_ptr->user_name,pData->ccfsm_new_call_cback.host_num); 

			strcpy(call_info_ptr->host_name ,pData->ccfsm_new_call_cback.host_num); 
			strcpy(call_info_ptr->display_name, pData->ccfsm_new_call_cback.display_num); 
 
			call_info_ptr->codec_id = pData->ccfsm_new_call_cback.codec  ;
  
			call_info_ptr->lineid = sc1445x_phoneapi_assignedline(m_active_calls_list[i].call_info.attached, call_info_ptr->callid ) ;
			 
			if (pData->ccfsm_new_call_cback.state == CCFSM_STATE_INCOMING)
			{
	 		  // send a ring event to application
   				sc1445x_phoneapi_ringmode(pData->ccfsm_new_call_cback.attachedentity, START_RINGING, 
				pData->ccfsm_new_call_cback.portid, pData->ccfsm_new_call_cback.accountid, 
				pData->ccfsm_new_call_cback.codec, sc1445x_phoneapi_find_numof_calls( pData->ccfsm_new_call_cback.portid), pData->ccfsm_new_call_cback.username_num);
			}else{
 				sc1445x_phoneapi_ringmode(pData->ccfsm_new_call_cback.attachedentity, OUTGOING_RINGING, pData->ccfsm_new_call_cback.portid, pData->ccfsm_new_call_cback.accountid, 
				pData->ccfsm_new_call_cback.codec, sc1445x_phoneapi_find_numof_calls( pData->ccfsm_new_call_cback.portid), pData->ccfsm_new_call_cback.username_num);
			}

 			sc1445x_phoneapi_processcalls(m_active_calls_list[i].call_info.attached) ;
			sc1445x_phoneapi_retransmit(m_active_calls_list[i].call_info.attached,(void*)pData,sizeof(ccfsm_cback_type));
  
			return ;
		}
	}
  	 
}

int sc1445x_phoneapi_assignedline(ccfsm_attached_entity attachedentity, int callid)  
{
  	if (phone_function[attachedentity].assignedline)
   		return phone_function[attachedentity].assignedline(callid);
 
	return -1;
}

void sc1445x_phoneapi_ringmode(ccfsm_attached_entity attachedentity, ring_mode mode, int port, int accountid, int codec, int numofcalls, char* display) 
{
 	if (attachedentity==CCFSM_ATTACHED_BROADCAST)
		{
			//TO DO
  		if (phone_function[CCFSM_ATTACHED_GUI].ring)
				phone_function[CCFSM_ATTACHED_GUI].ring((int)mode, port, accountid, codec, numofcalls, display);

			if (phone_function[CCFSM_ATTACHED_DECT].ring)
				phone_function[CCFSM_ATTACHED_DECT].ring((int)mode, port, accountid, codec, numofcalls, display);
		  
			if (phone_function[CCFSM_ATTACHED_ATA].ring)
				phone_function[CCFSM_ATTACHED_ATA].ring((int)mode, port, accountid,  codec,numofcalls, display);
					
	}else
	{ 	 
 			if (phone_function[attachedentity].ring)
			{
  				phone_function[attachedentity].ring((int)mode, port, accountid,   codec,numofcalls, display);
			}
	}
}
 
void sc1445x_phoneapi_retransmit(ccfsm_attached_entity attachedentity, void* pData, int size) 
{
 
	if (phone_function[CCFSM_ATTACHED_CONSOLE].retransmit)
		phone_function[CCFSM_ATTACHED_CONSOLE].retransmit((void*) pData,  size);
		
 }

void sc1445x_phoneapi_processcalls(ccfsm_attached_entity attachedentity) 
{
 	if (attachedentity==CCFSM_ATTACHED_BROADCAST)
	{
		if (phone_function[CCFSM_ATTACHED_GUI].processcalls)
			phone_function[CCFSM_ATTACHED_GUI].processcalls();

		if (phone_function[CCFSM_ATTACHED_DECT].processcalls)
			phone_function[CCFSM_ATTACHED_DECT].processcalls();
	  
		if (phone_function[CCFSM_ATTACHED_ATA].processcalls)
			phone_function[CCFSM_ATTACHED_ATA].processcalls();
		
	}else
	{
  	 
		if (phone_function[attachedentity].processcalls)
			phone_function[attachedentity].processcalls(); 
	}
 

}

void sc1445x_phoneapi_terminatecall(ccfsm_attached_entity attachedentity, int port, int callid, int numofcalls, int term_reason) 
{
	if (attachedentity==CCFSM_ATTACHED_BROADCAST)
	{
	  if (phone_function[CCFSM_ATTACHED_GUI].terminatecall)
	    phone_function[CCFSM_ATTACHED_GUI].terminatecall(port, callid, numofcalls, term_reason);

	  if (phone_function[CCFSM_ATTACHED_DECT].terminatecall)
	    phone_function[CCFSM_ATTACHED_DECT].terminatecall(port, callid, numofcalls, term_reason);
	  
    if (phone_function[CCFSM_ATTACHED_ATA].terminatecall){
			phone_function[CCFSM_ATTACHED_ATA].terminatecall(port, callid, numofcalls, term_reason);
    }
		
	}else
	{
	  if (phone_function[attachedentity].terminatecall)
	    phone_function[attachedentity].terminatecall(port, callid, numofcalls, term_reason);
	}
}
  
void sc1445x_phoneapi_del_call(ccfsm_cback_type* pData) 		 
{
 	int i;
 
  	if (pData==NULL) return;
 
  	for(i = 0 ; i < GUI_MAX_CALLS_SUPPORTED ; i++) {
		if (m_active_calls_list[i].call_info.callid == pData->ccfsm_updatestatus_cback.callid) {
			 
			//show reason TO DO 
			// pData->ccfsm_updatestatus_cback.info
			{
			 time_t tim=time(NULL);
			 struct tm *now=localtime(&tim);
  			if (now)
			{
					if (!m_active_calls_list[i].call_info.CallRecord) 
					 cmd_calllog_add_record(m_active_calls_list[i].call_info.user_name,  now, MISSED);

			} 
			}
			 
 
			if(!(pData->ccfsm_updatestatus_cback.info=='\0')){
 
		        strcpy(m_active_calls_list[i].call_info.info,pData->ccfsm_updatestatus_cback.info);
			}
			else{
 
				strcpy(m_active_calls_list[i].call_info.info,"End of call");
			}
			 

		  if (!m_active_calls_list[i].call_info.inConference )//FIX 11082009
		  {
 
			if (m_active_calls_list[i].call_info.lineid==1)
				led_unset(LED_LINE1);
			else if (m_active_calls_list[i].call_info.lineid==2) 
				led_unset(LED_LINE2);
		  }
			 

			{
				sc1445x_phone_call_info* pCall;		
				pCall = sc1445x_phoneapi_find_call_withID(pData->ccfsm_updatestatus_cback.callid );
						 

				if (pCall && pCall->inConference) {
					 conference_data.numofmembers--;
					 if (conference_data.numofmembers<0) 
					 conference_data.numofmembers=0;
				}
							 

				sc1445x_phoneapi_terminatecall(m_active_calls_list[i].call_info.attached, 
					m_active_calls_list[i].call_info.portid, m_active_calls_list[i].call_info.callid,
					sc1445x_phoneapi_find_numof_calls(m_active_calls_list[i].call_info.portid),
					pData->ccfsm_updatestatus_cback.reasoncode  );
			 

				m_active_calls_list[i].call_info.state = pData->ccfsm_updatestatus_cback.state;
 		 		sc1445x_phoneapi_processcalls(m_active_calls_list[i].call_info.attached);
				sc1445x_phoneapi_retransmit(m_active_calls_list[i].call_info.attached,(void*)pData,sizeof(ccfsm_cback_type));
 				memset(&m_active_calls_list[i].call_info, 0, sizeof(sc1445x_phone_call_info));
 			}
 			break;
		}
	} 
}

void sc1445x_phoneapi_replace_call(ccfsm_cback_type* pData) 		 
{
	int i;
	sc1445x_phone_call_info *call_info_ptr=NULL;
	 
	for(i = 0 ; i < GUI_MAX_CALLS_SUPPORTED ; i++) {
		if (m_active_calls_list[i].call_info.callid == pData->ccfsm_transfer_indication_cback.callid) {
 			if (pData->ccfsm_transfer_indication_cback.state == CCFSM_STATE_ROUTE)
				call_info_ptr = &m_active_calls_list[i].call_info;
			else
			{
				si_print(PRINT_LEVEL_ERR, "UNABLE TO ESTABLISH NEW CALL \n");
				return ;
			}

			//call_info_ptr->attached = pData->attachedentity; 
 			call_info_ptr->state = pData->ccfsm_transfer_indication_cback.state;  

  			call_info_ptr->calltype = pData->ccfsm_transfer_indication_cback.calltype;  
			call_info_ptr->callid = pData->ccfsm_transfer_indication_cback.newcallid;  
			call_info_ptr->accountid = pData->ccfsm_transfer_indication_cback.accountid; 
			strcpy(call_info_ptr->user_name,pData->ccfsm_transfer_indication_cback.transfer_to); 
			strcpy(call_info_ptr->display_name,pData->ccfsm_transfer_indication_cback.transfer_to); 
			strcpy(call_info_ptr->host_name,pData->ccfsm_transfer_indication_cback.transfer_to); 

			sc1445x_phoneapi_processcalls(m_active_calls_list[i].call_info.attached) ;
			sc1445x_phoneapi_retransmit(m_active_calls_list[i].call_info.attached,(void*)pData,sizeof(ccfsm_cback_type));
 
 		}
	}
	if (call_info_ptr==NULL) {
		si_print(PRINT_LEVEL_ERR, "FATAL ERROR : callid not found \n");
		return ;
	}
  	sc1445x_phoneapi_processcalls(m_active_calls_list[i].call_info.attached) ;
	sc1445x_phoneapi_retransmit(m_active_calls_list[i].call_info.attached,(void*)pData,sizeof(ccfsm_cback_type));

 }
 
void sc1445x_phoneapi_start_fax(ccfsm_attached_entity attached,int callid, int port, int faxevent)
{
  ccfsm_req_type m_CallFaxReq;
 
  m_CallFaxReq.ccfsm_fax_req.req_id = CCFSM_FAX_REQ;
  m_CallFaxReq.ccfsm_fax_req.callid  = callid;
  m_CallFaxReq.ccfsm_fax_req.portid  = port;
  m_CallFaxReq.ccfsm_fax_req.attachedentity = attached;
  m_CallFaxReq.ccfsm_fax_req.event   = faxevent;
 
  ccfsm_set_requests(&m_CallFaxReq);
}

sc1445x_phone_call_info* sc1445x_phoneapi_modify_call_state(ccfsm_cback_type* pData) 		 
{
	int i;  
 
	for(i = 0 ; i < GUI_MAX_CALLS_SUPPORTED ; i++) {
		if (m_active_calls_list[i].call_info.callid == pData->ccfsm_updatestatus_cback.callid) {
 			m_active_calls_list[i].call_info.state = pData->ccfsm_updatestatus_cback.state;
			m_active_calls_list[i].call_info.reference  =pData->ccfsm_updatestatus_cback.reference;
 			if ((pData->cback_id == CCFSM_ANSWERED_CB) || (pData->cback_id == CCFSM_INCALL_CODEC_CHANGED_CB) ||(pData->cback_id == CCFSM_RESUMED_CB))
			{
 				if (pData->ccfsm_updatestatus_cback.parameter & 0x80000000)
					m_active_calls_list[i].call_info.encryption = 1;
				else 
					m_active_calls_list[i].call_info.encryption = 0;

				m_active_calls_list[i].call_info.codec_id = pData->ccfsm_updatestatus_cback.parameter & 0x0fffffff ;
			}
			// Keep a Call Log 
			if (pData->cback_id==CCFSM_ANSWERED_CB)	
			{
				m_active_calls_list[i].call_info.CallRecord=1;
				{
		 		 time_t tim=time(NULL);
				 struct tm *now=localtime(&tim);
				 if (now)
				 {
 					 if ((m_active_calls_list[i].call_info.calltype 	== CCFSM_DIRECTION_OUT_IPCALL) || (m_active_calls_list[i].call_info.calltype == CCFSM_DIRECTION_OUT))
						 cmd_calllog_add_record(m_active_calls_list[i].call_info.user_name,  now, DIALED);
					 if ((m_active_calls_list[i].call_info.calltype 	== CCFSM_DIRECTION_IN_IPCALL) || (m_active_calls_list[i].call_info.calltype == CCFSM_DIRECTION_IN))
						 cmd_calllog_add_record(m_active_calls_list[i].call_info.user_name,  now, RECEIVED);
				 } 
				}
			}

			 sc1445x_phoneapi_processcalls(m_active_calls_list[i].call_info.attached) ;
 			 sc1445x_phoneapi_retransmit(m_active_calls_list[i].call_info.attached,(void*)pData,sizeof(ccfsm_cback_type));
  			return (sc1445x_phone_call_info *)&m_active_calls_list[i].call_info;
  		}
	}
	return (sc1445x_phone_call_info *)NULL;
}
  

#define  CCFSM_STATE_IDLE_str 

char* sc1445x_phoneapi_getDiplayStatusString( ccfsm_call_states status) 
{
 	switch(status)	
	{
 		case CCFSM_STATE_DIALING: return CCFSM_STATE_DIALING_STR;
		case CCFSM_STATE_ROUTE: return  CCFSM_STATE_ROUTE_STR;
 		case CCFSM_STATE_INCOMING: return  CCFSM_STATE_INCOMING_STR;
		case CCFSM_STATE_RINGING: return  CCFSM_STATE_RINGING_STR;
		case CCFSM_STATE_TALK: return  CCFSM_STATE_TALK_STR;
		case CCFSM_STATE_HOLD: return   CCFSM_STATE_HOLD_STR;
		case CCFSM_STATE_HELD: return  CCFSM_STATE_HELD_STR;
		case CCFSM_STATE_INACTIVE: return  CCFSM_STATE_INACTIVE_STR;
		case CCFSM_STATE_BLIND_TRANSFERRING: return  CCFSM_STATE_BLIND_TRANSFERRING_STR;
		case CCFSM_STATE_ATTENDED_TRANSFERRING: return  CCFSM_STATE_ATTENDED_TRANSFERRING_STR;
 		case CCFSM_STATE_TERMINATING: return  CCFSM_STATE_TERMINATING_STR;
		case CCFSM_STATE_TERMINATED: return  CCFSM_STATE_TERMINATED_STR;
		case CCFSM_STATE_PARKED: return  CCFSM_STATE_PARKED_STR;
		case CCFSM_STATE_CONFERENCE: return  CCFSM_STATE_CONFERENCE_STR;
		default: break;
 	}
	return  CCFSM_STATE_IDLE_STR;
}
char*  sc1445x_phoneapi_getUsername(char *userstring)
{
 
		if (strstr(userstring, "sip:"))
			return &userstring[4];
			else return userstring;
}		


int sc1445x_phoneapi_getAllowedUserActions(ccfsm_call_states status) 
{
	switch(status)
	{
 		case CCFSM_STATE_DIALING: return CCFSM_STATE_DIALING_ACTIONS;
		case CCFSM_STATE_ROUTE: return  CCFSM_STATE_ROUTE_ACTIONS;
 		case CCFSM_STATE_INCOMING: return  CCFSM_STATE_INCOMING_ACTIONS;
		case CCFSM_STATE_RINGING: return  CCFSM_STATE_RINGING_ACTIONS;
		case CCFSM_STATE_TALK: return  CCFSM_STATE_TALK_ACTIONS;
		case CCFSM_STATE_HOLD: return   CCFSM_STATE_HOLD_ACTIONS;
		case CCFSM_STATE_HELD: return  CCFSM_STATE_HELD_ACTIONS;
		case CCFSM_STATE_INACTIVE: return  CCFSM_STATE_INACTIVE_ACTIONS;
		case CCFSM_STATE_BLIND_TRANSFERRING: return  CCFSM_STATE_BLIND_TRANSFERRING_ACTIONS;
		case CCFSM_STATE_BLIND_TRANSFERED: return  CCFSM_STATE_BLIND_TRANSFERED_ACTIONS;
		case CCFSM_STATE_TERMINATING: return  CCFSM_STATE_TERMINATING_ACTIONS;
		case CCFSM_STATE_TERMINATED: return  CCFSM_STATE_TERMINATED_ACTIONS;
		case CCFSM_STATE_PARKED: return  CCFSM_STATE_PARKED_ACTIONS;
		case CCFSM_STATE_CONFERENCE: return  CCFSM_STATE_CONFERENCE_ACTIONS;			
		case CCFSM_STATE_CONFERENCE_HOLD: return  CCFSM_STATE_CONFERENCE_ACTIONS_HOLD;//FIX 11082009
		case CCFSM_STATE_CONFERENCE_ADD_MEMBER: return  CCFSM_STATE_CONFERENCE_ACTIONS_ADD_MEMBER;	//FIX 11082009	
		default: break;
	}
	return  CCFSM_STATE_IDLE_ACTIONS;
}
 
  
sc1445x_phone_call_info *  sc1445x_phoneapi_find_call_per_state(ccfsm_call_states state,int portid)
{
  sc1445x_phone_active_calls_list *pCallItem;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.state == state)  && (pCallItem->call_info.portid  == portid))
		{
 			return &pCallItem->call_info;
			}
	}
	return NULL;
}

sc1445x_phone_call_info *  sc1445x_phoneapi_find_call_per_state_ofportid(ccfsm_call_states state, int port)
{
  sc1445x_phone_active_calls_list *pCallItem;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
    if ((pCallItem->call_info.state == state)  && (pCallItem->call_info.portid == port))
		{
 			return &pCallItem->call_info;
		}
	}
	return NULL;
}


ccfsm_call_states sc1445x_phoneapi_find_state_per_callid(int callid)
{
  sc1445x_phone_active_calls_list *pCallItem;
	int i=1;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		i++;
		if (pCallItem->call_info.callid == callid)
		{
			return pCallItem->call_info.state;
		}
	}
	return CCFSM_STATE_IDLE;
}


sc1445x_phone_call_info *  sc1445x_phoneapi_find_call_per_state_ofaccountid(ccfsm_call_states state, int accountid)
{
  sc1445x_phone_active_calls_list *pCallItem;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
    if ((pCallItem->call_info.state == state)  && (pCallItem->call_info.accountid  == accountid))
		{
 			return &pCallItem->call_info;
		}
	}
	return NULL;
}

sc1445x_phone_call_info * sc1445x_phoneapi_find_call(int portid)
{
  sc1445x_phone_active_calls_list *pCallItem;

  for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
  {
	  //to check for fax 
   if ( (pCallItem->call_info.state != CCFSM_STATE_IDLE) &&  (pCallItem->call_info.portid == portid))
   {
	    return &pCallItem->call_info ;
   }
  }
 return NULL;
}

sc1445x_phone_call_info * sc1445x_phoneapi_find_call_per_accountid(int accountid)
{
 	sc1445x_phone_active_calls_list *pCallItem;
  	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
	   if ((pCallItem->call_info.state != CCFSM_STATE_IDLE) && (pCallItem->call_info.accountid == accountid))
		{
 			return &pCallItem->call_info ;
		}
	}
	return NULL;
}
//to check for fax

sc1445x_phone_call_info * sc1445x_phoneapi_find_call_withID(int callid)
{
 	sc1445x_phone_active_calls_list *pCallItem;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.callid == callid) && (pCallItem->call_info.state != CCFSM_STATE_IDLE))
		{
 			return &pCallItem->call_info ;
		}
	}
	return NULL;
}

sc1445x_phone_call_info * sc1445x_phoneapi_find_call_withline(int lineid)
{
 	sc1445x_phone_active_calls_list *pCallItem;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.state != CCFSM_STATE_IDLE) && (pCallItem->call_info.lineid ==lineid))
		{
 			return &pCallItem->call_info ;
		}
	}
	return NULL;
}


sc1445x_phone_call_info * sc1445x_phoneapi_get_next_call_per_state(sc1445x_phone_call_info *pCall, ccfsm_call_states state, int portid)
{
 	sc1445x_phone_active_calls_list *pCallItemStart = m_active_calls_list;
 	sc1445x_phone_active_calls_list *pCallItem ;
	if (pCall)
	{
		for(pCallItem = pCallItemStart; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
		{	
			if ((&pCallItem->call_info == pCall) && (pCallItem->call_info.portid == portid) && (pCallItem->call_info.state == state))
			{
 				pCallItem++;
				pCallItemStart =pCallItem ;
 
			}
		}
	}
 	for(pCallItem = pCallItemStart; pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.portid == portid) && (pCallItem->call_info.state == state))
		{
 			return &pCallItem->call_info ;
		}
	}
	return NULL;
}


sc1445x_phone_call_info * sc1445x_phoneapi_get_next_call(sc1445x_phone_call_info *pCall,int portid)
{
 	sc1445x_phone_active_calls_list *pCallItemStart = m_active_calls_list;
 	sc1445x_phone_active_calls_list *pCallItem ;
	if (pCall)
	{
		for(pCallItem = pCallItemStart; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
		{	
			if ((&pCallItem->call_info == pCall)  && (pCallItem->call_info.portid == portid) && (!(pCallItem->call_info.state == CCFSM_STATE_IDLE)))
			{
 				pCallItem++;
				pCallItemStart =pCallItem ;
 
			}
		}
	}
 	for(pCallItem = pCallItemStart; pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.portid == portid) && (!(pCallItem->call_info.state == CCFSM_STATE_IDLE)))
		{
 			return &pCallItem->call_info ;
		}
	}
	return NULL;
}

int sc1445x_phoneapi_find_numof_calls(int portid)
{
	int found =0;
 	sc1445x_phone_active_calls_list *pCallItem;
     
	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
 		if ((pCallItem->call_info.portid == portid) && (pCallItem->call_info.state != CCFSM_STATE_IDLE))
		{
 			found++;
		}
	}
	return found;
}

int sc1445x_phoneapi_find_numof_calls_notinconference(int portid)
{
	int found =0;
 	sc1445x_phone_active_calls_list *pCallItem;
     
	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
 		if (!pCallItem->call_info.inConference && (pCallItem->call_info.portid == portid) && (pCallItem->call_info.state != CCFSM_STATE_IDLE))
		{
 			found++;
		}
	}
	return found;
}


int sc1445x_phoneapi_find_numof_calls_per_port(ccfsm_call_states state, int port)
{
	int found =0;
 	sc1445x_phone_active_calls_list *pCallItem;
     
	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
 		if ((pCallItem->call_info.portid == port) && (pCallItem->call_info.state == state))
		{
 			found++;
		}
	}
	return found;
}

int sc1445x_phoneapi_find_numof_calls_peraccountid(int accountid)
{
	int found =0;
 	sc1445x_phone_active_calls_list *pCallItem;
     
	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
    if ((pCallItem->call_info.accountid == accountid) &&( pCallItem->call_info.state !=CCFSM_STATE_IDLE ))
		{
 			found++;
		}
	}
	return found;
}

int sc1445x_phoneapi_find_numof_calls_per_state(ccfsm_call_states state, int portid)
{
	int found =0;
 	sc1445x_phone_active_calls_list *pCallItem;
     
	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
 		if (  (pCallItem->call_info.portid == portid ) &&( pCallItem->call_info.state ==state ))
		{
 			found++;
		}
	}
	return found;
}

sc1445x_phone_call_info * sc1445x_phoneapi_find_call_per_flag(int flag, int portid, int account)
{
  sc1445x_phone_active_calls_list *pCallItem;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.flags & flag) && (pCallItem->call_info.accountid ==account) && (pCallItem->call_info.portid  ==portid))
		{
 			 return &pCallItem->call_info;
		}
	}
	return NULL;
}

int sc1445x_phoneapi_update_port(int callid, int portid)
{
  sc1445x_phone_active_calls_list *pCallItem;

  for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
  {
   if ((pCallItem->call_info.callid ==callid))
   {
    pCallItem->call_info.portid = portid ;
    return 1;
   }
  }
  return 0;
}
 
int sc1445x_phoneapi_update_accountid(int callid, int accountid)
{
  sc1445x_phone_active_calls_list *pCallItem;

  for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
  {
   if ((pCallItem->call_info.callid ==callid))
   {
    pCallItem->call_info.accountid = accountid;
    return 1;
   }
  }
  return 0;
}

int sc1445x_phoneapi_get_call_flags(int portid, int account)
{
  	sc1445x_phone_active_calls_list *pCallItem;

 	for(pCallItem = m_active_calls_list; (int)pCallItem<((int)m_active_calls_list+(sizeof(m_active_calls_list))); pCallItem++)  
	{
		if ((pCallItem->call_info.accountid ==account) && (pCallItem->call_info.portid  ==portid))
		{
 			 return pCallItem->call_info.flags ; 
		}
	}
	return -1;
}
#ifdef GUI_ENABLED
 	void sc1445x_phoneapi_setled(int callid, int flag)
	{
		sc1445x_phone_call_info * pCall= sc1445x_phoneapi_find_call_withID(callid);
		unsigned short line =1 ;
		if (pCall==NULL) return ;

		if (pCall->lineid ==1)
			line =LED_LINE1 ;
		else if (pCall->lineid ==2)
			line =LED_LINE2 ;
		else return ;
		 
	 
		if (!flag) led_unset(line);
		else if (flag==1) led_set(line,0);
		else   led_set(line,1);

	 }
#else 
 	void sc1445x_phoneapi_setled(int callid, int flag)
	{
		return;
	}
#endif

/* This function creates the approprite calling string */
/* It uses the string number to check if */
/* the registrar field must be added to the end of the number */
/* The result is returned to the URI string. */
/* Returns 0 for SIP call, 1 for DirectIP call and -1 when wrong IP occurs */
	/*
int sc1445x_phoneapi_CheckNumber(char *URI, char *number, char *registrar)
{
  char *pos;
  char num[64];
  char to[64];
  unsigned char tmpip[4];
  int check_ip=0;

  if(number==NULL) return -1;
  strcpy(num, number);
 
  if(!strcmp(num,""))
	  return -1;
	check_ip = sc1445x_phoneapi_extract_address(tmpip, num);
	// Check if it is an IP only number 
	if (check_ip == 0){
		sprintf(URI,"sip:%s",num); 
		// Check if this IP is the registrar 
		if (!strcmp(registrar, num)){
 		  return 0;
		 }else{
 		  return 1;
		}
		// Check if is a number 
	}else {
 		// Check if is there is IP inside the number  
		pos=strchr(num, '@') ;
		if (pos==NULL){
			sprintf(to, "sip:%s@%s", num, registrar); 
			strcpy(URI, to);
			si_print(PRINT_LEVEL_INFO, "[%s] URI %s \n ", __FUNCTION__, URI);
 			return 0;
	    }else {  
			pos++;
			// Check if IP exists is the registrar
			if (!strcmp(registrar, pos)){
 				sprintf(URI,"sip:%s",num); 
				return 0;
		  }else{
					// Check if IP is not the registrar 
					// Check if it is a valid IP 
		      check_ip = sc1445x_phoneapi_extract_address(tmpip, pos);
  				if (check_ip == -1) {
			        si_print(PRINT_LEVEL_INFO, "Number to call has an invalid IP address field\n");
			        return -1;
 		       }else{
		          sprintf(URI,"sip:%s",num); 
				  return 1;
				}
			}
		}
  }
  return 0;
}
*/

#include "../exosip2/src/eXosip.h"
#include <netdb.h>
int sc1445x_phoneapi_CheckNumber(char *URI, char *number, char *registrar)
{
 osip_uri_t *dialnumber;
 int ret;
 int direct = 0;
 char *pos;
 char localnumber[64];
 
 if(number==NULL) return -1;
 
 if (!strncmp(number,"sip",3))
  strcpy(localnumber,number);
 else{
  sprintf(localnumber,"sip:%s", number );
 }

 osip_uri_init(&dialnumber);
 ret = osip_uri_parse(dialnumber, localnumber);
 if (ret != OSIP_SUCCESS) return -1;
 
  if (dialnumber && dialnumber->username)
 {
  if (dialnumber && dialnumber->host)
		sprintf(URI,"sip:%s@%s",dialnumber->username,dialnumber->host);
	else{
   if (registrar)
    sprintf(URI,"sip:%s@%s",dialnumber->username,registrar);
  }
  }else{
   if (dialnumber && dialnumber->host) 
  {
   if (INADDR_NONE == inet_addr (dialnumber->host))
    sprintf(URI,"sip:%s@%s",dialnumber->host,registrar);
   else 
    sprintf(URI,"sip:%s",dialnumber->host);
  }
  else return -1;
 }
 if (dialnumber && dialnumber->port)
 {
  sprintf(&URI[strlen(URI)],":%s",dialnumber->port);
  }
 if (dialnumber && dialnumber->host)
 {
  if (registrar==NULL) direct=1;
  else if (strncmp(dialnumber->host,registrar, strlen(registrar)))
   direct=1;
 }
  
  if (dialnumber) osip_uri_free(dialnumber);
		return direct;
}


int sc1445x_phoneapi_extract_address(unsigned char *IP_address, char *string)
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
		else if(!isdigit(*ptr))
			return -1;

		ptr++;
	}

	if(num_of_dots != 3)
		return -1;

	ptr = string_copy_ptr;	
	for(i = 0 ; i < 3 ; i++) {
		while(*ptr) {
			if(*ptr == '.') {
				if(ptr - string_copy_ptr) {
					*ptr = 0;
					sscanf(string_copy_ptr, "%u", &temp[i]);

					if(temp[i] > 255)
						return -1;

					ptr++;					
					string_copy_ptr = ptr;
					break;
				}else
					return -1;
			}

			ptr++;
		}
	}

	if(strlen(string_copy_ptr)) {
		sscanf(string_copy_ptr, "%u", &temp[3]);

		if(temp[3] > 255)
			return -1;
	}else
		return -1;

	for(i = 0 ; i < 4 ; i++)
		IP_address[i] = (unsigned char)temp[i];

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//Translates a number with five dots x.x.x.x.x.x to a number with:
//x@x.x.x.x. 
//The @ field is treated as a username
//Result is returned to the newnumber string
////////////////////////////////////////////////////////////////////////////////
void sc1445x_phoneapi_fix_calling_username(unsigned char *newnumber, unsigned char *dialnumber)
{
  int i = 0, cntdots = 0;

  strcpy((char *)newnumber, (char *)dialnumber);
  for(i=0;i<strlen((char *)dialnumber);i++){
    if (dialnumber[i] == '.'){
      cntdots++;
    }
  }

  if (cntdots == 4){//username.x.x.x.x
    for(i=0; i<strlen((char *)dialnumber); i++){
      if (dialnumber[i] == '.'){                     
        newnumber[i] = '@';
        break;
      }
    }
  }
  cntdots=0;                  
}
    
////////////////////////////////////////////////////////////////////////////////////////
//
//  Set the pcm line type at:
// /* ATA/DECT line type */
//
// SC1445x_AE_LINE_TYPE_ATA = 0,			/* ATA */
//
// SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW,		/* narrowband */
//							/* CVM DECT */
//
// SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_8KHZ_ALAW,	/* wideband CVM DECT */
//							/* a-law compressed */
//							/* to 8kHz */
//
//	SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_8KHZ_ULAW,	/* wideband CVM DECT */
//							/* u-law compressed */
//							/* to 8kHz */
//
//	SC1445x_AE_LINE_TYPE_CVM_DECT_WIDE_16KHZ,	/* wideband CVM DECT */
//							/* at 16kHz */
//
//	SC1445x_AE_LINE_TYPE_NATIVE_DECT_NARROW,	/* narrowband */
//							/* native DECT */
//
//	SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE,		/* wideband */
//							/* native DECT */
//
//	SC1445x_AE_LINE_TYPE_INVALID
//
////////////////////////////////////////////////////////////////////////////////////////

/* access menuconfig options */
#ifdef USE_PCM
#include "../common/operation_mode_defs.h" 

int sc1445x_phoneapi_init_pcm_ports(void)
{

#if ((defined ATA_ENABLED) && !(defined DECT_ENABLED)) //ATA_ONLY

  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_ATA);
#ifdef SUPPORT_4_CHANNELS
	sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_ATA);
#endif

#elif defined CONFIG_ATA_1_FXS_NO_FXO_1_CVM 

  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
#ifdef SUPPORT_4_CHANNELS
	sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
#endif

#elif defined CONFIG_ATA_2_FXS_NO_FXO_1_CVM 

  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
#ifdef SUPPORT_4_CHANNELS
	sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
#endif

#elif (!(defined CONFIG_SC1445x_LEGERITY_890_SUPPORT) && (defined CONFIG_CVM480_DECT_SUPPORT)) //DECT ONLY

  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
#ifdef SUPPORT_4_CHANNELS
	sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_CVM_DECT_NARROW);
#endif

#elif (!(defined CONFIG_SC1445x_LEGERITY_890_SUPPORT) && (defined CONFIG_LMX4180_DECT_SUPPORT)) //NATALIE ONLY
  
  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
#ifdef SUPPORT_4_CHANNELS
	sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
#endif

#elif ((defined CONFIG_SC1445x_LEGERITY_890_SUPPORT) && (defined CONFIG_LMX4180_DECT_SUPPORT)) //NATALIE+ATA 
#if( defined CONFIG_ATA_1_FXS_NO_FXO )
  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);

#elif( defined CONFIG_ATA_2_FXS_NO_FXO )
  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(4, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);

#elif( defined CONFIG_ATA_3_FXS_NO_FXO )
  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(4, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(5, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);

#elif( defined CONFIG_ATA_4_FXS_NO_FXO )
  sc1445x_amb_init_pcm_line_type(0, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(1, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(2, SC1445x_AE_LINE_TYPE_ATA);
	sc1445x_amb_init_pcm_line_type(3, SC1445x_AE_LINE_TYPE_ATA);
  sc1445x_amb_init_pcm_line_type(4, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);
  sc1445x_amb_init_pcm_line_type(5, SC1445x_AE_LINE_TYPE_NATIVE_DECT_WIDE);

#endif
#endif
   return 0;
}

#endif
int sc1445x_phoneapi_im_process(ccfsm_cback_type* pData)
{
  	if (phone_function[CCFSM_ATTACHED_GUI].improcess)
		phone_function[CCFSM_ATTACHED_GUI].improcess(pData->ccfsm_im_recv_indication.host, 
		pData->ccfsm_im_recv_indication.username, pData->ccfsm_im_recv_indication.display, 
		pData->ccfsm_im_recv_indication.im_text, (char) pData->ccfsm_im_recv_indication.im_size, 
		pData->ccfsm_im_recv_indication.im_type);

	return 0;
}

int sc1445x_phoneapi_presence_notify_process(ccfsm_cback_type* pData)
{
	//TODO send to attached entity
	//build
 	if (phone_function[pData->ccfsm_presence_notify_indication.attachedentity].phoneevents)
	  	phone_function[pData->ccfsm_presence_notify_indication.attachedentity].phoneevents(CCFSM_PRESENCE_NOTIFY_IND, 
		pData->ccfsm_presence_notify_indication.accountid, 1 , (void*)pData->ccfsm_presence_notify_indication.not_text);


 	return 0;
}



int sc1445x_phoneapi_presence_subscribe_process (ccfsm_cback_type* pData)
{
  //if (phone_function[pData->ccfdm_pr_sub_cback.attachedentity].improcess)
	//phone_function[CCFSM_ATTACHED_GUI].improcess (pData->ccfsm_im_recv_indication.username, pData->ccfsm_im_recv_indication.display, pData->ccfsm_im_recv_indication.im_text, pData->ccfsm_im_recv_indication.im_size, pData->ccfsm_im_recv_indication.im_type);

	if (pData->ccfsm_presence_subscribe_cback.status==CCFSM_PRESENCE_SUBSCRIBE_SUCCESS)
		si_print(PRINT_LEVEL_INFO, "Subscription to %s@%s %s \n", pData->ccfsm_presence_subscribe_cback.subAccount,\
											pData->ccfsm_presence_subscribe_cback.subRealm, "Success");
	else
		if (pData->ccfsm_presence_subscribe_cback.status==CCFSM_PRESENCE_SUBSCRIBE_TERMINATED)
			si_print(PRINT_LEVEL_INFO, "Subscription to %s@%s %s \n", pData->ccfsm_presence_subscribe_cback.subAccount,\
														pData->ccfsm_presence_subscribe_cback.subRealm, "Terminated");
		else
		si_print(PRINT_LEVEL_INFO, "Subscription to %s@%s %s \n", pData->ccfsm_presence_subscribe_cback.subAccount,\
													pData->ccfsm_presence_subscribe_cback.subRealm, "Failed");
	return 0;
}

int sc1445x_phoneapi_presence_subscribe(ccfsm_attached_entity attachedentity, int portid, int account, int expires, char * subAccount, char * subRealm)
{
	ccfsm_req_type m_SendPRReq;

	m_SendPRReq.ccfsm_send_presence_subscribe_req.req_id  =  CCFSM_PRERENCE_SUBSCRIBE_REQ;
	m_SendPRReq.ccfsm_send_presence_subscribe_req.attachedentity =attachedentity;
 	m_SendPRReq.ccfsm_send_presence_subscribe_req.portid =portid;
	m_SendPRReq.ccfsm_send_presence_subscribe_req.accountid = account;
	m_SendPRReq.ccfsm_send_presence_subscribe_req.expires=expires;
	if (subAccount)
 		strcpy(m_SendPRReq.ccfsm_send_presence_subscribe_req.subAccount, subAccount);
	if (subRealm)
		strcpy(m_SendPRReq.ccfsm_send_presence_subscribe_req.subRealm, subRealm);
  	ccfsm_set_requests(&m_SendPRReq);
	return 0;
}

int sc1445x_phoneapi_prerence_unsubscribe (ccfsm_attached_entity attachedentity, int sid, int portid)
{
	ccfsm_req_type m_SendPRReq;

	m_SendPRReq.ccfsm_send_presence_unsubscribe_req.req_id =CCFSM_PRESENCE_UNSUBSCRIBE_REQ;
	m_SendPRReq.ccfsm_send_presence_unsubscribe_req.attachedentity =attachedentity;

	m_SendPRReq.ccfsm_send_presence_unsubscribe_req.portid =portid;
	m_SendPRReq.ccfsm_send_presence_unsubscribe_req.sid = sid;

	ccfsm_set_requests(&m_SendPRReq);
	return 0;
}


//  Request to Call Control  - To publish the state of a user

int sc1445x_phoneapi_presence_publish(ccfsm_attached_entity attachedentity, pr_status_basic pres, pr_note note, int portid, int account, int expires)
{
	ccfsm_req_type m_SendPRReq;

	m_SendPRReq.ccfsm_send_presence_publish_req .req_id =CCFSM_PRERENCE_PUBLISH_REQ;
	m_SendPRReq.ccfsm_send_presence_publish_req.attachedentity =attachedentity;

	m_SendPRReq.ccfsm_send_presence_publish_req.portid =portid;
	m_SendPRReq.ccfsm_send_presence_publish_req.accountid = account;
	m_SendPRReq.ccfsm_send_presence_publish_req.expires=expires;

	m_SendPRReq.ccfsm_send_presence_publish_req.pres=pres;
	m_SendPRReq.ccfsm_send_presence_publish_req.note=note;

 	ccfsm_set_requests(&m_SendPRReq);
	return 0;
}

