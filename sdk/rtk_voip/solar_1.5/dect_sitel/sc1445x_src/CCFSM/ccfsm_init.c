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
 * File:		 		 ccfsm_init.c
 * Purpose:		 		 
 * Created:		 		 30/10/2008 
 * By:		 		 	 YP
 * Country:		 		 Greece
 *
 *-----------------------------------------------------------------------------
 */

/*========================== Include files ==================================*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h> 
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/msg.h>

#include <si_print_api.h>

#include "ccfsm_init.h"
#include "sc1445x_phone_api.h"
#include "../SiPhoneUA/si_ua_server_api.h"
#include "../SiPhoneUA/si_ua_events.h"
#include "../SiPhoneUA/si_ua_callcontrol.h"

/*========================== Global Variables definitions ========================*/
#define MAX_CALLS_SUPPORTED  16
#include "../common/si_configfiles_api.h"
extern ConfigSettings m_Config_Settings;
ccfsm_call_list listOfCalls[MAX_CALLS_SUPPORTED]; 
static int isvalid_callid(  int callid);
static int find_call_id(int callnumber);
int	checkandallocate_new_outgoingcall(ccfsm_attached_entity attachedentity,ccfsm_audio_peripheral audio_peripheral_type,   int portid);
int	checkandallocate_new_incomingcall(ccfsm_new_call_cback_data  *pInCall);
static void	deallocate_callinstance(ccfsm_attached_entity attachedentity,ccfsm_audio_peripheral audio_peripheral_type,   int portid,   int callid);
static int establish_new_call(ccfsm_outgoing_call_req_data *pCallData);
static int establish_new_call_w_replaces(ccfsm_outgoing_call_w_replaces_req_data *pCallData);
static int change_desktop_state(ccfsm_audio_peripheral_status status);
static int change_dect_state(ccfsm_audio_peripheral_status status);
static int change_dect_handset_state(ccfsm_audio_peripheral_status status);
static int change_ata_state(ccfsm_audio_peripheral_status status);
int ccfsm_send_registration(ccfsm_register_req_data *pReq);
int ccfsm_update_call(ccfsm_call_answer_req_data *pReqData);
int CallControlThreadStatus =0;
int CallControlQueue = -1;
pid_t ParentThreadID ;
void ccfsm_init(pid_t parID)
{
	pthread_t CallControlThread;
	int ret;


	ParentThreadID = parID;
	CallControlQueue = msgget(IPC_PRIVATE, 0600 |  IPC_CREAT);
	if (CallControlQueue < 0) {
		return;
	}
	memset(listOfCalls, 0, sizeof(listOfCalls));
	CallControlThreadStatus=1;
	ret = pthread_create (&CallControlThread, NULL, ccfsm_polling_entry, (void *) NULL);

	pthread_detach( CallControlThread ) ;

	if (ret != 0)
	{
		CallControlThreadStatus=0;
		si_print(PRINT_LEVEL_CRIT, "Call Control Thread  failed\n"); 
	}
	return;
}

 int ccfsm_set_requests(ccfsm_req_type *msg)
{
	int ret = 0;
	callcontrol_message qBuffer;

	if ((CallControlQueue<0) || (msg==NULL)){
 		return -1;
	}

	memcpy(&qBuffer.msg[0], msg, 512);
 	qBuffer.type = CALLCONTROL_REQUEST;
 	ret = msgsnd(CallControlQueue, &qBuffer, 512,  IPC_NOWAIT);

	if (ret) si_print(PRINT_LEVEL_CRIT, "!!!!!!!!!!!!!! FATAL ERROR on ccfsm_set_requests\n");

 	return 0;
}

int KILLThread = 0;
int ccfsm_send_callbacks(ccfsm_cback_type* pCBack)
{
	int ret = 0;
	callcontrol_message qBuffer;

 	if ((CallControlQueue<0) || (pCBack==NULL))return -1;

	memcpy(&qBuffer.msg[0], pCBack, 512);
 	qBuffer.type = CALLCONTROL_CALLBACK;
 	ret = msgsnd(CallControlQueue, &qBuffer, 512,  IPC_NOWAIT);
	if (ret) si_print(PRINT_LEVEL_CRIT, "\nFATAL ERROR ON ccfsm_send_callbacks\n\n");

	if (ParentThreadID){
		ret =  kill(ParentThreadID, SIGUSR2); 
	 }

	KILLThread = 0;
	return 0;
}

static volatile int call_do_ccfsm_get_callbacks = 0 ;
void do_ccfsm_get_callbacks( void )
{
	int ret;
	callcontrol_message qBuffer;

	if (CallControlQueue<0)
		return ;

	qBuffer.type = CALLCONTROL_CALLBACK;
	do {
		ret = msgrcv(CallControlQueue, (void*)&qBuffer, 512, CALLCONTROL_CALLBACK, IPC_NOWAIT);

		if (ret>0){
			sc1445x_phoneapi_application_callback((ccfsm_cback_type*) &qBuffer.msg[0] );
		}
	} while (ret>0);
}
 
void ccfsm_get_callbacks (int args)
{
#if 0
	int ret;
	callcontrol_message qBuffer;
	if (CallControlQueue<0) return ;
 	qBuffer.type = CALLCONTROL_CALLBACK;
      do
	{
  		ret = msgrcv(CallControlQueue, (void*)&qBuffer, 512, CALLCONTROL_CALLBACK, IPC_NOWAIT);
 		if (ret>0){
 			sc1445x_phoneapi_application_callback((ccfsm_cback_type*) &qBuffer.msg[0] ); 
		}
else
 	}while (ret>0);
#else
	call_do_ccfsm_get_callbacks = 1 ;
#endif
 }

sigset_t myblock_mask;
extern struct sigaction    myactions;
extern  sigset_t myblock_mask;

int ccfsm_polling_entry(void * args)
{
  int ret = 0;
	callcontrol_message qBuffer;
	
	memset(&myactions, 0, sizeof(myactions));
  sigemptyset(&myactions.sa_mask);

  sigfillset(&myblock_mask);
	sigemptyset(&myblock_mask);
  
	sigaddset (&myblock_mask, SIGUSR2);
 	myactions.sa_mask = myblock_mask;
  myactions.sa_handler = ccfsm_get_callbacks; 
 	myactions.sa_flags = 0;
  sigaction(SIGUSR2,&myactions,NULL);

	 /* Set up the mask of signals to temporarily block. */ 
  
  	while (CallControlThreadStatus)
	{
 		usleep(10000);
  	qBuffer.type = CALLCONTROL_REQUEST;
   	ret = msgrcv(CallControlQueue, (void*)&qBuffer, 512, CALLCONTROL_REQUEST, IPC_NOWAIT);
		if (ret>0) 
			ccfsm_parse_requests((ccfsm_req_type*)(&qBuffer.msg[0]));
		if( call_do_ccfsm_get_callbacks ) {
			call_do_ccfsm_get_callbacks = 0 ;
			do_ccfsm_get_callbacks() ;
		}
  	 	 si_ua_thread(NULL);
       }

	return 0;
}
 
int ccfsm_parse_requests(ccfsm_req_type *pReq)
{
	int err = 0;
     switch(pReq->req_id)
	{
 		case CCFSM_REGISTER_REQ:
  			ccfsm_send_registration(&pReq->ccfsm_register_req);
			break;
		case CCFSM_CONFIGURE_REQ:
  			ccfsm_configure(&pReq->ccfsm_configure_req);
			break;
		case CCFSM_CONNECT_REQ:
			break;
		case CCFSM_DISCONNECT_REQ:
			break;
 		case  CCFSM_ALLOCATE_CALL_REQ:
 			ccfsm_enterdialingstate(&pReq->ccfsm_allocate_newcall_req);
  		break;
		case  CCFSM_DEALLOCATE_CALL_REQ:

		break;
		case  CCFSM_OUTGOING_CALL_REQ:
   			err= ccfsm_create_newcall(&pReq->ccfsm_outgoing_call_req);
 		break;
		case  CCFSM_INTERNAL_CALL_REQ:
   			err= ccfsm_create_internalcall(&pReq->ccfsm_outgoing_call_req);
 		break;
		case CCFSM_ANSWER_REQ:
 			// check if any call in talk staste and send a hold on 
			// before answer current call
			ccfsm_update_call(&pReq->ccfsm_call_answer_req );
      {
			ccfsm_call_list *pCall=NULL;
			do{
			 	pCall = ccfsm_get_next_call(pCall, pReq->ccfsm_call_answer_req.portid);

				if (pCall && pCall->state==CCFSM_STATE_TALK )
				{
					ccfsm_req_type m_CallReq;
					m_CallReq.ccfsm_call_hold_req.req_id = CCFSM_CALL_HOLD_REQ;
					m_CallReq.ccfsm_call_hold_req.callid = pCall->callid;
					err = ccfsm_hold_call(&m_CallReq.ccfsm_call_hold_req);
					 
					usleep(1000);
				}
 			}while (pCall);
			err= ccfsm_answer_call(&pReq->ccfsm_call_answer_req );
			if (err) si_print(PRINT_LEVEL_ERR, "INAVLID STATE ON ANSWER ......... ");
      }	
			break;
 		case CCFSM_CALL_HOLD_REQ:
			err= ccfsm_hold_call(&pReq->ccfsm_call_hold_req );
 			break;	
		case CCFSM_CALL_RESUME_REQ:
			err= ccfsm_resume_call(&pReq->ccfsm_call_resume_req );
 			break;	
		case CCFSM_BLINDTRANSFER_REQ:
			err= ccfsm_blind_transfer(&pReq->ccfsm_blindtransfer_req );
 			break;	
		case CCFSM_ATTENDEDTRANSFER_REQ:
				err= ccfsm_attended_transfer(&pReq->ccfsm_attendedtransfer_req );
 			break;	
  		case CCFSM_TERMINATE_REQ:
  			err= ccfsm_terminate_call(&pReq->ccfsm_call_terminate_req);
			break;
  		case CCFSM_RESET_REQ:
  			ccfsm_reset();
			break;
  		case CCFSM_DTMF_REQ:
  			ccfsm_dtmf(&pReq->ccfsm_dtmf_req);
			break;
  		case CCFSM_FAX_REQ:
  			ccfsm_fax(&pReq->ccfsm_fax_req);
			break;
 		case CCFSM_INCALL_CODEC_CHANGE_REQ:
 			ccfsm_incall_codec_change(&pReq->ccfsm_incall_codec_change_req);
			break;
		case CCFSM_IM_SEND_REQ:
				ccfsm_send_im(&pReq->ccfsm_send_im_req);	
			break;
		case CCFSM_PRERENCE_SUBSCRIBE_REQ:
			ccfsm_send_presence_subscribe(&pReq->ccfsm_send_presence_subscribe_req );
			break;
		case CCFSM_PRERENCE_PUBLISH_REQ:
			ccfsm_send_presence_publish(&pReq->ccfsm_send_presence_publish_req);
			break;
		default:
			si_print(PRINT_LEVEL_ERR,"INVALID REQUEST \n ");
	}
 
	if (err>0)
		 si_print(PRINT_LEVEL_ERR, "Error issuing a request [%d] errornumber=%d.......\n", pReq->req_id, err);

   	return 0;
}

int ccfsm_callback(ccfsm_cback_type* pCBack)
{
	int message_delivery = 0;
	int call_id;
	int err;
  	 
  	switch(pCBack->cback_id)
	{
		case CCFSM_REGISTRATION_EVENT:
  		message_delivery = 1;
 			break;
		case CCFSM_NEW_ALLOCATE_CB:
  			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
			if (!isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid)) 
 				si_print(PRINT_LEVEL_CRIT, "ERROR CALL BACK CCFSM_NEW_ALLOCATE_CB \n ");
 	 			message_delivery = 1;
			break;
		case CCFSM_NEW_DEALLOCATE_CB:
 			message_delivery = 1;
			break;
		case CCFSM_NEW_CALL_CB:
		case CCFSM_NEW_CALL_ANSWERED_CB:
			 // check if acceptable
			 // check if it is incoming or outgoing call
			 
 			if ((pCBack ->ccfsm_new_call_cback.calltype == CCFSM_DIRECTION_IN ) || 
			(pCBack ->ccfsm_new_call_cback.calltype == CCFSM_DIRECTION_IN_IPCALL))
			{
 				// lock
			    call_id= checkandallocate_new_incomingcall(&pCBack ->ccfsm_new_call_cback);
				if  (call_id<0)		
				{
					 si_print(PRINT_LEVEL_CRIT, "Unable to allocate new call \n" );//terminateCall() // TODO
					 break;
				}
			 
				if (pCBack->cback_id==CCFSM_NEW_CALL_ANSWERED_CB){
			 		listOfCalls[call_id].state = CCFSM_STATE_TALK;
					pCBack->ccfsm_new_call_cback.state  = CCFSM_STATE_TALK;
				}
				// unlock
 			}
			message_delivery = 1;
  			break;
  		case CCFSM_RINGING_CB:
			 // check if acceptable
  			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid)) 
			{
 				listOfCalls[call_id].state = CCFSM_STATE_RINGING;
				pCBack->ccfsm_updatestatus_cback.state = listOfCalls[call_id].state ;
				pCBack->ccfsm_updatestatus_cback.reference = listOfCalls[call_id].reference ;
				message_delivery = 1;
 			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");
			break;
		case CCFSM_ANSWERED_CB:
			 // check if acceptable
			// change state
			// TODO
			//listOfCalls[pCBack->ccfsm_updatestatus_cback.callid].display
			//listOfCalls[pCBack->ccfsm_updatestatus_cback.callid].host
 			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid)) 
			{
				listOfCalls[call_id].state = CCFSM_STATE_TALK;
				pCBack->ccfsm_updatestatus_cback.reference = listOfCalls[call_id].reference ;
				pCBack->ccfsm_updatestatus_cback.state =  	listOfCalls[call_id].state ;
				message_delivery = 1;
  			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");
			break;
		case CCFSM_HOLD_CB:
   			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid)) 
			{
				if (listOfCalls[call_id].state == CCFSM_STATE_TALK)  
					listOfCalls[call_id].state = CCFSM_STATE_HOLD;
				else if (listOfCalls[call_id].state == CCFSM_STATE_HOLDING)
					listOfCalls[call_id].state = CCFSM_STATE_HOLD;
				else if (listOfCalls[call_id].state == CCFSM_STATE_INACTIVATING)
					listOfCalls[call_id].state = CCFSM_STATE_INACTIVE;

				pCBack->ccfsm_updatestatus_cback.state =  	listOfCalls[call_id].state ;
				pCBack->ccfsm_updatestatus_cback.reference = listOfCalls[call_id].reference ;

				message_delivery = 1;
 			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");
			break;
		case CCFSM_HELD_CB:
  
  			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid)) 
			{
				if (listOfCalls[call_id].state == CCFSM_STATE_TALK)  
					listOfCalls[call_id].state = CCFSM_STATE_HELD;
				else if (listOfCalls[call_id].state == CCFSM_STATE_HOLD)
					listOfCalls[call_id].state = CCFSM_STATE_INACTIVE;
				else if (listOfCalls[call_id].state == CCFSM_STATE_HOLDING)
					listOfCalls[call_id].state = CCFSM_STATE_INACTIVATING;
				else if (listOfCalls[call_id].state == CCFSM_STATE_INACTIVATING)
					listOfCalls[call_id].state = CCFSM_STATE_INACTIVE;

				pCBack->ccfsm_updatestatus_cback.reference = listOfCalls[call_id].reference ;
				pCBack->ccfsm_updatestatus_cback.state =  	listOfCalls[call_id].state ;
				message_delivery = 1;
 			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");
			break;
		case CCFSM_INACTIVE_CB:
 			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid)) 
			{
 				listOfCalls[call_id].state = CCFSM_STATE_INACTIVE;
				pCBack->ccfsm_updatestatus_cback.state =  	listOfCalls[call_id].state ;
 				message_delivery = 1;
  			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");
			break;
		case CCFSM_RESUMED_CB:
			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
  			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
			{
				if (listOfCalls[call_id].state == CCFSM_STATE_RESUMING_HELD) 
					listOfCalls[call_id].state = CCFSM_STATE_HELD;
				else if (listOfCalls[call_id].state == CCFSM_STATE_RESUMING_TALK) 
					listOfCalls[call_id].state = CCFSM_STATE_TALK;
				else if (listOfCalls[call_id].state == CCFSM_STATE_HELD) 
					listOfCalls[call_id].state = CCFSM_STATE_TALK;
				else if (listOfCalls[call_id].state == CCFSM_STATE_INACTIVE) 
					listOfCalls[call_id].state = CCFSM_STATE_HOLD;
 
				pCBack->ccfsm_updatestatus_cback.state =  	listOfCalls[call_id].state ;
				message_delivery = 1;
 
			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");
			break;
		case CCFSM_TERMINATING_CB:
			 // check if acceptable
   			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
 			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
			{
				listOfCalls[call_id].state = CCFSM_STATE_TERMINATING;
				pCBack->ccfsm_updatestatus_cback.state =  	listOfCalls[call_id].state ;
 				message_delivery = 1;
 			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID (CCFSM_TERMINATING_CB)\n");

			break;
		case CCFSM_REJECTED_CB:
		case CCFSM_TERMINATED_CB:
 			// check if acceptable
 			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
			{
	  			listOfCalls[call_id].state = CCFSM_STATE_TERMINATED;
				pCBack->ccfsm_updatestatus_cback.state = listOfCalls[call_id].state ;
				pCBack->ccfsm_updatestatus_cback.reference = listOfCalls[call_id].reference ;
				message_delivery = 1;

				//YANNIS - 12Jan
				deallocate_callinstance(listOfCalls[call_id].attachedentity,listOfCalls[call_id].audio_peripheral_type,listOfCalls[call_id].portid,listOfCalls[call_id].callid);

 			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID (CCFSM_TERMINATED_CB)\n");

			break;
 		case CCFSM_REDIRECTED_CB:
			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
 			if (!isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
 			 si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");
			else{
				pCBack->ccfsm_updatestatus_cback.reference = listOfCalls[call_id].reference ;
 			}
			break;
		case CCFSM_BLINDTRANSFERRING_CB:
			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
 			if (!isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
 			 si_print(PRINT_LEVEL_ERR, "Invalid Call ID \n");

 		case CCFSM_BLINDTRANSFERED_CB:
	 		call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
 			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
			{
 				pCBack->ccfsm_updatestatus_cback.state =  	listOfCalls[call_id].state ;
  				message_delivery = 1;
 			}else si_print(PRINT_LEVEL_ERR,"Invalid Call ID \n");
			break;
		case CCFSM_ATTENDEDTRANSFERED_CB:
			break;
		case CCFSM_TRANSFER_ACCEPTED_CB:
			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
 			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
			{
				listOfCalls[call_id].state = CCFSM_STATE_TALK;
 				message_delivery = 1;
 			}
			break;
		case CCFSM_BLINDTRANSFER_IND:
		case CCFSM_ATTENDEDTRANSFER_IND:
			call_id = find_call_id(pCBack->ccfsm_transfer_indication_cback.callid);
   			if (isvalid_callid(pCBack->ccfsm_transfer_indication_cback.callid))
			{
  				if ((listOfCalls[call_id].state == CCFSM_STATE_HELD) || (listOfCalls[call_id].state == CCFSM_STATE_INACTIVE))
				{
					ccfsm_outgoing_call_w_replaces_req_data m_CallData_w_replaces;
					if (pCBack->cback_id==CCFSM_ATTENDEDTRANSFER_IND)
					{
						 
						listOfCalls[call_id].state = CCFSM_STATE_ATTENDED_TRANSFERRING;
					}
					else 
					{
						 
 						listOfCalls[call_id].state = CCFSM_STATE_BLIND_TRANSFERRING;
					}
 					m_CallData_w_replaces.transferred_callid=pCBack->ccfsm_transfer_indication_cback.callid;  
 					m_CallData_w_replaces.accountid = pCBack->ccfsm_transfer_indication_cback.accountid ;
					m_CallData_w_replaces.attachedentity =listOfCalls[call_id].attachedentity;
 					m_CallData_w_replaces.req_id =CCFSM_OUTGOING_CALL_REQ;
					m_CallData_w_replaces.calltype=listOfCalls[call_id].calltype; 
					m_CallData_w_replaces.portid =listOfCalls[call_id].portid; 
					if (pCBack->cback_id==CCFSM_ATTENDEDTRANSFER_IND)
						strcpy(m_CallData_w_replaces.replaces,pCBack->ccfsm_transfer_indication_cback .replaces);
					else 
						m_CallData_w_replaces.replaces[0]=0;

					strcpy(m_CallData_w_replaces.transfer_from,pCBack->ccfsm_transfer_indication_cback.transfer_from );
					strcpy(m_CallData_w_replaces.transfer_to,pCBack->ccfsm_transfer_indication_cback.transfer_to );
  					{	
						int newCallId;
						newCallId = checkandallocate_new_outgoingcall(m_CallData_w_replaces.attachedentity,m_CallData_w_replaces.audio_peripheral_type,m_CallData_w_replaces.portid);
						if (newCallId<0) 
						{
							//TODO 				
						}else{
							m_CallData_w_replaces.callid=newCallId;
							pCBack->ccfsm_transfer_indication_cback.newcallid = newCallId;
							pCBack->ccfsm_transfer_indication_cback.attachedentity  = listOfCalls[call_id].attachedentity ;
							pCBack->ccfsm_transfer_indication_cback.calltype = listOfCalls[call_id].calltype;
							message_delivery = 1;
 							err = establish_new_call_w_replaces(&m_CallData_w_replaces);
							pCBack->ccfsm_transfer_indication_cback.state  = CCFSM_STATE_ROUTE;
 							if (err)
							{
								deallocate_callinstance(m_CallData_w_replaces.attachedentity,m_CallData_w_replaces.audio_peripheral_type,m_CallData_w_replaces.portid,m_CallData_w_replaces.callid);
 							}

  						}
 					}
   				}
  			}else si_print(PRINT_LEVEL_ERR, "Invalid Call ID transfer ind [%d][%d]\n",pCBack->ccfsm_transfer_indication_cback.callid,call_id);
		break;
  		case CCFSM_INCALL_CODEC_CHANGED_CB:  
 			call_id = find_call_id(pCBack->ccfsm_updatestatus_cback.callid);
 			if (isvalid_callid(pCBack->ccfsm_updatestatus_cback.callid))
			{
				listOfCalls[call_id].state = CCFSM_STATE_TALK;
 				message_delivery = 1;
 			}
 			break;
   		case CCFSM_IM_RECV_IND:  
   		case CCFSM_PRESENCE_NOTIFY_IND:
   		case CCFSM_PRESENCE_SUBSCRIBE_CB:
		case CCFSM_EVENT_NOTIFICATION_EVENT://FIX BROADWORKS
			message_delivery = 1;
		break;
		default:
			break;
	}
 
	if (message_delivery)
		ccfsm_send_callbacks(pCBack);

//	si_print(PRINT_LEVEL_DEBUG,"ccfsm_callback :end cbackid [%d] state = %d\n", pCBack->cback_id,listOfCalls[call_id].state);

 	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
void ccfsm_reset(void)
{
	sip_reset();
}
void ccfsm_dtmf(ccfsm_dtmf_req_data *pCallData)
{
	sip_dtmf_start( pCallData->key ); 
}

void ccfsm_fax(ccfsm_fax_req_data *pCallData)
{
	sip_fax_start( pCallData->callid, pCallData->event); 
}
 
void ccfsm_incall_codec_change(ccfsm_incall_codec_change_req_data *pCallData) 
{
 	sip_incall_codec_change(pCallData->callid, pCallData->codec) ; 
}

int ccfsm_change_audio_peripheral_state(ccfsm_audio_peripheral peripheral, ccfsm_audio_peripheral_status status)
{
	switch (peripheral)
	{
		case CCFSM_AUDIO_DESKTOP:
			change_desktop_state( status);
			break;
		case CCFSM_AUDIO_DECT:
			change_dect_state( status);
			break;
		case CCFSM_AUDIO_DECT_HANDSET:
			change_dect_handset_state( status);
			break;
		case CCFSM_AUDIO_ATA_FXS:
			change_ata_state( status);
			break;
 	}
	return 0;
}
void ccfsm_send_im(ccfsm_send_im_req_data *pIMData) 
{
 	sip_send_instant_message(pIMData->text, pIMData->accountid,pIMData->dialled_num,pIMData->calltype ) ; 
}

int ccfsm_create_newcall(ccfsm_outgoing_call_req_data *pCallData)
{
	int err=-1;
 	//check call id
 	if (!isvalid_callid(pCallData->callid)) 
	{		 
 		  pCallData->callid = checkandallocate_new_outgoingcall(pCallData->attachedentity, pCallData->audio_peripheral_type,pCallData->portid);
		
 		  if (pCallData->callid<0){ return err;}
 		  err = establish_new_call(pCallData);
		 if (err)
		   deallocate_callinstance(pCallData->attachedentity,pCallData->audio_peripheral_type,pCallData->portid,pCallData->callid);
	}else{
   		err = establish_new_call(pCallData);
	}
 	// send a response to attached entity to inform it about the call status 
	ccfsm_create_newoutcall_cback(pCallData, err);

    return  err;
}
int ccfsm_create_internalcall(ccfsm_outgoing_call_req_data *pCallData)
{
	int err=-1;
	ccfsm_new_call_cback_data m_cback_newcall;

	//check call id
 	if (!isvalid_callid(pCallData->callid)) 
	{		 
 		  pCallData->callid = checkandallocate_new_outgoingcall(pCallData->attachedentity, pCallData->audio_peripheral_type,pCallData->portid);
 		  if (pCallData->callid<0){ return err;}
  	} 
 	// send a response to attached entity to inform it about the call status 
	ccfsm_create_newoutcall_cback(pCallData, err);

 
	m_cback_newcall.cback_id=CCFSM_NEW_CALL_CB;
	m_cback_newcall.state = CCFSM_STATE_INCOMING;
	m_cback_newcall.attachedentity= pCallData->attachedentity;
	 m_cback_newcall.accountid = 0 ;
	m_cback_newcall.callid = pCallData->callid+0x40000000 ; 
 	m_cback_newcall.calltype = CCFSM_DIRECTION_INTERNAL_CALL;
	m_cback_newcall.portid = atoi(pCallData->dialled_num);
  
 	m_cback_newcall.status = 0;
	ccfsm_send_callbacks(&m_cback_newcall);
   
    return  err;
}

int ccfsm_create_newcall_w_replaces(ccfsm_outgoing_call_w_replaces_req_data *pCallData)
{
	int err=-1;
 	//check call id
	  pCallData->callid = checkandallocate_new_outgoingcall(pCallData->attachedentity, pCallData->audio_peripheral_type,pCallData->portid);
	  if (pCallData->callid<0){ return err;}
	  err = establish_new_call_w_replaces(pCallData);
	 if (err)
	   deallocate_callinstance(pCallData->attachedentity,pCallData->audio_peripheral_type,pCallData->portid,pCallData->callid);
  	 
     return  err;
}

void ccfsm_create_newoutcall_cback(ccfsm_outgoing_call_req_data *pCallData, int status)
{			
 	ccfsm_new_call_cback_data m_cback_newcall;
 
 	m_cback_newcall.cback_id =CCFSM_NEW_CALL_CB;
	m_cback_newcall.attachedentity =pCallData->attachedentity;
	m_cback_newcall.reference =pCallData->reference ;
	m_cback_newcall.accountid =pCallData->accountid ;
	m_cback_newcall.callid = pCallData->callid;
    m_cback_newcall.portid = pCallData->portid ;
	m_cback_newcall. calltype=pCallData->calltype;
 
	m_cback_newcall.display_num[0]='\0'; 
	m_cback_newcall.host_num [0]='\0'; 
	m_cback_newcall.username_num[0]='\0'; 
	strcpy(m_cback_newcall.username_num,pCallData->dialled_num) ;
  	m_cback_newcall.codec=pCallData->codec; 
	m_cback_newcall.state = CCFSM_STATE_ROUTE;
	
	m_cback_newcall.status = status;
	ccfsm_send_callbacks(&m_cback_newcall);
//		sc1445x_phoneapi_application_callback(&m_cback_newcall);
//	ccfsm_callback((ccfsm_cback_type*)&m_cback_newcall);
	 
}
 
void ccfsm_create_newincall_cback(ccfsm_incoming_call_ind_data *pNewInCall)
{			
	ccfsm_new_call_cback_data m_cback_newcall;

 	m_cback_newcall.cback_id =CCFSM_NEW_CALL_CB;
	m_cback_newcall.accountid = pNewInCall->accountid;
	m_cback_newcall.callid = pNewInCall->callid;
 	m_cback_newcall.calltype=pNewInCall->calltype;
	strcpy( m_cback_newcall.username_num,pNewInCall->username ) ;
	strcpy( m_cback_newcall.display_num ,pNewInCall->display  ) ;
	strcpy(m_cback_newcall.host_num,pNewInCall->host ) ;
  m_cback_newcall.codec=pNewInCall->codec; 
	m_cback_newcall.state = CCFSM_STATE_INCOMING;
	m_cback_newcall.status =0;
	ccfsm_callback(&m_cback_newcall);

}

  
int ccfsm_answer_call(ccfsm_call_answer_req_data *pReqData) 
{
 int callid = find_call_id(pReqData->callid);
 int currentstate;

 // check if callid is a valid call 
 if (!pReqData) return CCFSM_REQ_RETURN_INVALID_ARGUMENTS;
 if (callid<0) return CCFSM_REQ_RETURN_INVALID_COMMAND;
  
 //check if answer request is allowed in currect state  
 currentstate  = listOfCalls[callid].state;

 if (currentstate == CCFSM_STATE_INCOMING){
   listOfCalls[callid].state = CCFSM_STATE_ANSWERING;
 }else {
   return CCFSM_REQ_RETURN_INVALID_STATE;
 }
  // starttimeout(currentstate, pReqData->callid,CCFSM_TOUT_HOLDING);
 sip_answer_call(pReqData->callid, pReqData->codec, pReqData->portid, pReqData->accountid );
 return 0;
}


int ccfsm_hold_call(ccfsm_call_hold_req_data *pReqData) 
{
	int callid = find_call_id(pReqData->callid);
	ccfsm_hold_flag flag = 0;
	int currentstate;
  // check if callid is a valid call	
 
	if (!pReqData) return CCFSM_REQ_RETURN_INVALID_ARGUMENTS;
	if (callid<0) return CCFSM_REQ_RETURN_INVALID_COMMAND;
	//check if the currect state allow an answer command
	currentstate  = listOfCalls[callid].state;
 
	if ((currentstate == CCFSM_STATE_TALK) || (currentstate == CCFSM_STATE_HOLDING)){
		listOfCalls[callid].state = CCFSM_STATE_HOLDING;
		flag =	CCFSM_HOLD_SENDONLY;
	}else if (currentstate == CCFSM_STATE_HELD)  //TODO
	{
		listOfCalls[callid].state = CCFSM_STATE_INACTIVATING;
		flag = CCFSM_HOLD_INACTIVE;
	}else if (currentstate == CCFSM_STATE_HOLD)  //TODO
 		return CCFSM_REQ_RETURN_INVALID_STATE;//?TODO
 
//	starttimeout(currentstate, pReqData->callid,CCFSM_TOUT_HOLDING); 
  	sip_hold_call(pReqData->callid,flag);
	return 0;
}

int ccfsm_resume_call(ccfsm_call_resume_req_data *pReqData) 
{
	int callid = find_call_id(pReqData->callid);
	ccfsm_hold_flag flag;
	int currentstate;
  	// check if callid is a valid call	
  	if (!pReqData) return CCFSM_REQ_RETURN_INVALID_ARGUMENTS;
	if (callid<0) return CCFSM_REQ_RETURN_INVALID_COMMAND;
	//check if the currect state allow an answer command
	currentstate  = listOfCalls[callid].state;
	if (currentstate == CCFSM_STATE_HOLD){
		listOfCalls[callid].state = CCFSM_STATE_RESUMING_TALK;
		flag =	CCFSM_HOLD_SENDRECV;
	}else if (currentstate == CCFSM_STATE_INACTIVE) {
		listOfCalls[callid].state = CCFSM_STATE_RESUMING_HELD;
		flag = CCFSM_HOLD_RECVONLY;
	}else return CCFSM_REQ_RETURN_INVALID_STATE;

//	starttimeout(currentstate, pReqData->callid,CCFSM_TOUT_RESUMING); 
  	sip_resume_call(pReqData->callid,flag);
	return 0;
}

int ccfsm_blind_transfer(ccfsm_blindtransfer_req_data *pReqData)  
{
	int callid = find_call_id(pReqData->callid);
 	int currentstate;
  	// check if callid is a valid call	
   	if (!pReqData) return CCFSM_REQ_RETURN_INVALID_ARGUMENTS;
 	if (callid<0) return CCFSM_REQ_RETURN_INVALID_COMMAND;
	//check if the currect state allow an answer command
	currentstate  = listOfCalls[callid].state;
  	if ((currentstate == CCFSM_STATE_TALK) || (currentstate == CCFSM_STATE_HOLD) || (currentstate == CCFSM_STATE_INACTIVE)){
		listOfCalls[callid].state = CCFSM_STATE_BLIND_TRANSFERRING;
 	}else return CCFSM_REQ_RETURN_INVALID_STATE;

 	sip_blindtransfer_call(pReqData->callid, (char*)pReqData->number);
 	//	starttimeout(currentstate, pReqData->callid,CCFSM_TOUT_TRANSFERRING); 
	return 0;
}
	  
int ccfsm_attended_transfer(ccfsm_attendedtransfer_req_data *pReqData) 
 {
	int callid = find_call_id(pReqData->callid);
 	int currentstate;
	 
  	// check if callid is a valid call	
  	if (!pReqData) return CCFSM_REQ_RETURN_INVALID_ARGUMENTS;
	if (callid<0) return CCFSM_REQ_RETURN_INVALID_COMMAND;
	//check if the currect state allow an answer command
 	currentstate  = listOfCalls[callid].state;
	if (currentstate == CCFSM_STATE_TALK){
		listOfCalls[callid].state = CCFSM_STATE_ATTENDED_TRANSFERRING;
 	} else return CCFSM_REQ_RETURN_INVALID_STATE;

	 sip_attendedtransfer_call(pReqData->calltotransfer, pReqData->callid); 
//	starttimeout(currentstate, pReqData->callid,CCFSM_TOUT_TRANSFERRING); 
  	return 0;
}

int ccfsm_terminate_call(ccfsm_call_terminate_req_data	*pReqData) 
{
	int callpos = find_call_id(pReqData->callid);
	int currentstate;
 
	// check if callid is a valid call	
  	if (!pReqData) return CCFSM_REQ_RETURN_INVALID_ARGUMENTS;
	if (callpos<0) return CCFSM_REQ_RETURN_INVALID_COMMAND;
	//check if a terminate rerquest is allowed in currect state  
 	currentstate  = listOfCalls[callpos].state;
	if (currentstate == CCFSM_STATE_DIALING) 
	{
 		//exit dialing state
		listOfCalls[callpos].state = CCFSM_STATE_IDLE;
  		return 0;
	}
 	if (currentstate != CCFSM_STATE_IDLE){
		listOfCalls[callpos].state = CCFSM_STATE_TERMINATING;
	}
 	// check in the call with callid is member of a conference
	// TO DO	
 
  	sip_terminate_call(pReqData->callid);
 	// send a response to attached entity to inform it about the call status 
	//ccfsm_create_terminate_cback(pCallData, pCallData->callid, err);
	return 0;
}


int ccfsm_terminate_call_cback(ccfsm_call_terminate_req_data *pReqData) 
{

return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int change_desktop_state(ccfsm_audio_peripheral_status status)
{
/*	switch(status)
	{
		case CCFSM_HOOK_ON:
 			break;
		case CCFSM_HOOK_OFF:
			break;
		case CCFSM_EXT_SPK_ON:
			break;
		case CCFSM_EXT_SPK_OFF:
			break;
		case CCFSM_DECT_HANDSET_ON:
			break;
		case CCFSM_DECT_HANDSET_OFF:
			break;
		case CCFSM_HEADSET_ON:
			break;
		case CCFSM_HEADSET_OFF:
			break;
  	}*/
	 return 0;
}

static int change_dect_state(ccfsm_audio_peripheral_status status)
{
/*	switch(status)
	{
		case CCFSM_HOOK_ON:
			// terminate_all_active_calls();
			break;
		case CCFSM_HOOK_OFF:
			break;
 		case CCFSM_EXT_SPK_ON:
 		case CCFSM_EXT_SPK_OFF:
 		case CCFSM_DECT_HANDSET_ON:
 		case CCFSM_DECT_HANDSET_OFF:
 		case CCFSM_HEADSET_ON:
 		case CCFSM_HEADSET_OFF:
		case CCFSM_NO_AUDIO_INFO:
		break;

	}
	*/
	return 0;
}
static int change_dect_handset_state(ccfsm_audio_peripheral_status status)
{
	/*
	switch(status)
	{
		case CCFSM_HOOK_ON:
			break;
		case CCFSM_HOOK_OFF:
			break;
		case CCFSM_EXT_SPK_ON:
			break;
		case CCFSM_EXT_SPK_OFF:
			break;
		case CCFSM_DECT_HANDSET_ON:
			break;
		case CCFSM_DECT_HANDSET_OFF:
			break;
		case CCFSM_HEADSET_ON:
			break;
		case CCFSM_HEADSET_OFF:
			break;
		case CCFSM_NO_AUDIO_INFO:
			break;
	}
	*/
	return 0;
}
static int change_ata_state(ccfsm_audio_peripheral_status status)
{
/*	switch(status)
	{
		case CCFSM_HOOK_ON:
			// terminate_all_active_calls();
			break;
		case CCFSM_HOOK_OFF:
			
			break;
 		case CCFSM_EXT_SPK_ON:
 		case CCFSM_EXT_SPK_OFF:
 		case CCFSM_DECT_HANDSET_ON:
 		case CCFSM_DECT_HANDSET_OFF:
 		case CCFSM_HEADSET_ON:
 		case CCFSM_HEADSET_OFF:
		case CCFSM_NO_AUDIO_INFO:
		break;
 
	}
	*/
	return 0;
}

void ccfsm_configure(ccfsm_configure_req_data *pCallData)
{
 	si_ua_configure(pCallData->attachedentity, pCallData->params, pCallData->size);
}
void ccfsm_enterdialingstate(ccfsm_allocate_newcall_req_data *pCallData)
{	
	ccfsm_allocate_new_call_cback_data m_cback_newalloc;
	int err;
 	err =checkandallocate_new_outgoingcall(pCallData->attachedentity,pCallData->audio_peripheral_type,pCallData->portid);

	m_cback_newalloc.cback_id = CCFSM_NEW_ALLOCATE_CB;
	m_cback_newalloc.accountid =pCallData->accountid ;
	m_cback_newalloc.callid = pCallData->callid=err;
    m_cback_newalloc.portid = pCallData->portid ;
 
	ccfsm_callback(&m_cback_newalloc);
}

 void ccfsm_exitdialingstate(ccfsm_allocate_newcall_req_data *pCallData)
{
	ccfsm_allocate_new_call_cback_data m_cback_newalloc;

	deallocate_callinstance(pCallData->attachedentity,pCallData->audio_peripheral_type,pCallData->portid,pCallData->callid);

	m_cback_newalloc.cback_id =CCFSM_NEW_DEALLOCATE_CB;
	m_cback_newalloc.accountid =pCallData->accountid ;
	m_cback_newalloc.callid = pCallData->callid;
    m_cback_newalloc.portid = pCallData->portid ;
 
	ccfsm_callback(&m_cback_newalloc);

}
   
int	checkandallocate_new_outgoingcall(ccfsm_attached_entity attachedentity,ccfsm_audio_peripheral audio_peripheral_type,   int portid)
{
	int i;
 	for (i=0;i<MAX_CALLS_SUPPORTED;i++)
	{
		if (listOfCalls[i].state == CCFSM_STATE_DIALING)
		{
 			if ((listOfCalls[i].audio_peripheral_type == audio_peripheral_type) && (listOfCalls[i].portid==portid) && (listOfCalls[i].attachedentity==attachedentity))
			{
				 
				return 	listOfCalls[i].callid; 
 			}
		}
	}
 
	for (i=0;i<MAX_CALLS_SUPPORTED;i++)
	{
		if (listOfCalls[i].state == CCFSM_STATE_IDLE )
		{
 
			 listOfCalls[i].audio_peripheral_type=audio_peripheral_type;
			 listOfCalls[i].portid=portid;
 
			 listOfCalls[i].attachedentity=attachedentity;
			 listOfCalls[i].state  = CCFSM_STATE_DIALING;
			 listOfCalls[i].callid = 0x100+i;
 			  
			 si_print(PRINT_LEVEL_DEBUG, "RETURN A NEW CALL ID %d[%d- %d] \n", attachedentity,i,listOfCalls[i].callid);
			 return  0x100 + i;
 		}
	}
 
	return -1;
}

int checkandallocate_new_incomingcall(ccfsm_new_call_cback_data  *pInCall) 
{
	int i;
	
	for (i=0;i<MAX_CALLS_SUPPORTED;i++)
	{
		if (listOfCalls[i].state == CCFSM_STATE_IDLE )
		{
			listOfCalls[i].callid = pInCall->callid;
   			strcpy(listOfCalls[i].username,pInCall->username_num); 
			strcpy(listOfCalls[i].display,pInCall->display_num ); 
			strcpy(listOfCalls[i].host,pInCall->host_num ); 
 			listOfCalls[i].calltype = pInCall->calltype ;
			listOfCalls[i].accountid = pInCall->accountid ;
   			listOfCalls[i].state  = CCFSM_STATE_INCOMING;
  	 		return  i;
 		}
	}
  	return -1;
}

static void	deallocate_callinstance(ccfsm_attached_entity attachedentity,ccfsm_audio_peripheral audio_peripheral_type,  int portid, int callno)
{
	int i;
	int callid= find_call_id(callno);
	if (isvalid_callid(callno)) 
	{
		//if (listOfCalls[callid].state  == CCFSM_STATE_DIALING)
			memset((char*)&listOfCalls[callid].state, 0, sizeof(ccfsm_call_list));
 		//else si_print(PRINT_LEVEL_ERR, "Invalid state on deallocate_callinstance \n");
		return ;
	}

 	for (i=0;i<MAX_CALLS_SUPPORTED;i++)
	{
		if (listOfCalls[i].state == CCFSM_STATE_DIALING)
		{
			if ((listOfCalls[i].audio_peripheral_type == audio_peripheral_type) && (listOfCalls[i].portid==portid) && (listOfCalls[i].attachedentity==attachedentity))
			{
				 memset((char*)&listOfCalls[i].state, 0, sizeof(ccfsm_call_list));
				 return ;
			}
		}
	}
	si_print(PRINT_LEVEL_ERR, "Invalid state on deallocate_callinstance \n");
}

static int isvalid_callid(  int callnumber)
{

	if (find_call_id(callnumber)<0) return 0;
	else return 1;  
}
  
int find_call_id(int callnumber)
{
	int i;
	for(i = 0 ; i < MAX_CALLS_SUPPORTED ; i++)  
	{
  		if (listOfCalls[i].callid == callnumber) 
		{
 			return i;
		}
 	}
	return -1;
}

static int establish_new_call(ccfsm_outgoing_call_req_data *pCallData)
{
	int err;
	int callid =  find_call_id(pCallData->callid);

 	if (!pCallData) { return -1;}
	if (!isvalid_callid(pCallData->callid))    {  return -1; }

	/* if (listOfCalls[callid].state == CCFSM_STATE_DIALING )
	{
		if (listOfCalls[callid].attachedentity !=pCallData->attachedentity)  		
		{
			si_print(PRINT_LEVEL_ERR, "Invalid state on establish_new_call \n");
			return -1;
		}
	} //TODO else return -1;*/

	if (strlen(pCallData->dialled_num)>sizeof(listOfCalls[callid].username )) return -1;
  	
	listOfCalls[callid].accountid  =pCallData->accountid;
	listOfCalls[callid].reference   =pCallData->reference ;
	listOfCalls[callid].calltype = pCallData->calltype;
	listOfCalls[callid].portid = pCallData->portid ;

	strcpy(listOfCalls[callid].username, pCallData->dialled_num); 
	strcpy(listOfCalls[callid].display, ""); 
	strcpy(listOfCalls[callid].host, ""); 
 	listOfCalls[callid].state = CCFSM_STATE_ROUTE;	

	//set timeout
 	//issue a call establisment command
	err = si_ua_callEstablish( pCallData->accountid, pCallData->portid , pCallData->callid, pCallData->codec , NULL, pCallData->dialled_num  , pCallData->calltype , NULL);
	return err;		
}

static int establish_new_call_w_replaces(ccfsm_outgoing_call_w_replaces_req_data *pCallData)
{
	int err, direct;
	int callid = find_call_id(pCallData->callid);
  	if (!pCallData) { return -1;}
	if (!isvalid_callid(pCallData->callid))    {  return -1; }

    listOfCalls[callid].accountid  =pCallData->accountid;
	listOfCalls[callid].calltype = pCallData->calltype;

  	strcpy(listOfCalls[callid].username, pCallData->transfer_to); 
	strcpy(listOfCalls[callid].display, ""); 
	strcpy(listOfCalls[callid].host, ""); 
 	listOfCalls[callid].state = CCFSM_STATE_ROUTE;	
 
	//set timeout
 	//issue a call establisment command
	 

	if (strstr(pCallData->transfer_to,m_Config_Settings.m_SIPSettings.registrar)) 
		direct =1;
	else 
		direct =0;
	//direct = sc1445x_phoneapi_CheckNumber(pCallData->transfer_to,pCallData->transfer_to,m_Config_Settings.m_SIPSettings.registrar);
	 

 	if (direct==1){
 		pCallData->calltype  = CCFSM_DIRECTION_OUT_IPCALL;
	}
	else if (direct==0){
 		pCallData->calltype = CCFSM_DIRECTION_OUT;
	}
	else {
		si_print(PRINT_LEVEL_CRIT, "FATAL ERROR on dial number while transferring \n ");
		return -1;
	}
 	err= sip_invite(pCallData);
  
	return err;		
}
  
ccfsm_call_list*  ccfsm_find_call_per_state(ccfsm_call_states state, int portid)
{
  ccfsm_call_list *pCallItem;

 	for(pCallItem = listOfCalls; pCallItem<((int)listOfCalls + (sizeof(listOfCalls))); pCallItem++)  
	{
  	if ((pCallItem->state == state)  && (pCallItem->portid == portid) )
		{
 			return pCallItem;
		} 
	}
	return NULL;
}

ccfsm_call_list * ccfsm_get_next_call(ccfsm_call_list *pCall,int portid)
{
 	ccfsm_call_list *pCallItemStart = listOfCalls;
 	ccfsm_call_list *pCallItem ;
	if (pCall)
	{
		for(pCallItem = pCallItemStart; (int)pCallItem<((int)listOfCalls+(sizeof(listOfCalls))); pCallItem++)  
		{	
 			if ((pCallItem == pCall)  && (pCallItem->portid  ==portid))
			{
 				pCallItem++;
				pCallItemStart =pCallItem ;
 
			}
		}
	}
 	for(pCallItem = pCallItemStart; pCallItem<((int)listOfCalls+(sizeof(listOfCalls))); pCallItem++)  
	{
 		if (  pCallItem->portid==portid )
		{
  			return pCallItem ;
		}
	}
 	return NULL;
}

int ccfsm_update_call(ccfsm_call_answer_req_data *pReqData) 
{
 int callid = find_call_id(pReqData->callid);
 
 // check if callid is a valid call 
 if (!pReqData) return CCFSM_REQ_RETURN_INVALID_ARGUMENTS;
 if (callid<0) return CCFSM_REQ_RETURN_INVALID_COMMAND;
  
 listOfCalls[callid].portid = pReqData->portid ;

 return 0;
}
int ccfsm_send_presence_subscribe(ccfsm_send_presence_subscribe_req_data *pPRData) 
{
	sip_presence_subscribe (pPRData->attachedentity, pPRData->accountid, pPRData->subAccount, pPRData->subRealm, pPRData->expires);
	return 0;
}			
int ccfsm_send_presence_publish(ccfsm_send_presence_publish_req_data *pPRData) 
{
	sip_presence_publish (pPRData->accountid, pPRData->pres, pPRData->note, pPRData->expires);
	return 0;
}

int ccfsm_send_registration(ccfsm_register_req_data *pReq)
{
	sip_registration(pReq->account, pReq->status);
	return 0;
}
