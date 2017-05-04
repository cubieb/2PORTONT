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
 * File:		ccfsm_api.h
 * Purpose:		
 * Created:		30/10/2008
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */
  
#ifndef CCFSM_API_H
#define CCFSM_API_H

/*========================== Include files ==================================*/
#include "../common/si_config_file.h"

typedef enum  
{
	CCFSM_STATE_IDLE,
	CCFSM_STATE_DIALING,
	CCFSM_STATE_ROUTE,
 	CCFSM_STATE_INCOMING,
	CCFSM_STATE_RINGING,
	CCFSM_STATE_ANSWERING,
	CCFSM_STATE_TALK,
	CCFSM_STATE_HOLD,
	CCFSM_STATE_HOLDING,
	CCFSM_STATE_HELD,
	CCFSM_STATE_INACTIVE,
	CCFSM_STATE_INACTIVATING,
	CCFSM_STATE_RESUMING_TALK,
	CCFSM_STATE_RESUMING_HELD,
	CCFSM_STATE_BLIND_TRANSFERRING,
	CCFSM_STATE_BLIND_TRANSFERED,
	CCFSM_STATE_ATTENDED_TRANSFERRING,
	CCFSM_STATE_ATTENDED_TRANSFERED,
	CCFSM_STATE_TERMINATING,
	CCFSM_STATE_TERMINATED,
	CCFSM_STATE_PARKED,
	CCFSM_STATE_CONFERENCE,
	CCFSM_STATE_CONFERENCE_HOLD,
	CCFSM_STATE_CONFERENCE_ADD_MEMBER,
 	CCFSM_STATE_NONE
}ccfsm_call_states;

#define  CCFSM_STATE_FLAG_TRANSFERPREPARE 0x00000001 
#define  CCFSM_STATE_FLAG_FORWARDPREPARE  0x00000002

#define  CCFSM_STATE_IDLE_STR "" 
#define  CCFSM_STATE_DIALING_STR "Dialing" 
#define  CCFSM_STATE_ROUTE_STR "Connecting" 
#define  CCFSM_STATE_INCOMING_STR  "New Call" 
#define  CCFSM_STATE_OUTGOING_STR  "Calling" 
#define  CCFSM_STATE_RINGING_STR "Ringing" 
#define  CCFSM_STATE_TALK_STR  "Talking" 
#define  CCFSM_STATE_HOLD_STR    "Hold" 
#define  CCFSM_STATE_HOLDING_STR   "Holding" 
#define  CCFSM_STATE_HELD_STR    "Held" 
#define  CCFSM_STATE_INACTIVE_STR  "Inactive" 
#define  CCFSM_STATE_BLIND_TRANSFERRING_STR "Transfer" 
#define  CCFSM_STATE_BLIND_TRANSFERED_STR "Transfer" 
#define  CCFSM_STATE_ATTENDED_TRANSFERRING_STR "Transfer" 
#define  CCFSM_STATE_ATTENDED_TRANSFERED_STR "Transfer" 
#define  CCFSM_STATE_TERMINATING_STR "Terminating" 
#define  CCFSM_STATE_TERMINATED_STR "Terminated" 
#define  CCFSM_STATE_PARKED_STR "Parked" 
#define  CCFSM_STATE_CONFERENCE_STR "Conference" 
 
 
#define ACTION_IDLE			0x0000
#define ACTION_SEND			0x0001
#define ACTION_EDIT			0x0002
#define ACTION_DEL			0x0004
#define ACTION_ANSWER		0x0008
#define ACTION_FORWARD		0x0010
#define ACTION_REJECT		0x0020
#define ACTION_END			0x0040
#define ACTION_HOLD			0x0080
#define ACTION_RESUME		0x0100
#define ACTION_CODECCHANGE	0x0200
#define ACTION_NEW			0x0400

#define ACTION_CONF_END		0x10000
#define ACTION_CONF_RESUME	0x20000
#define ACTION_CONF_HOLD	0x40000
#define ACTION_CONF_NEW		0x80000

#define ACTION_BLIND		0x1000
#define ACTION_TRANSFER		0x2000
#define ACTION_CONFERENCE	0x4000
#define ACTION_PARK			0x8000

#define  CCFSM_STATE_IDLE_ACTIONS				ACTION_IDLE
#define  CCFSM_STATE_DIALING_ACTIONS			ACTION_SEND | ACTION_EDIT | ACTION_DEL
#define  CCFSM_STATE_ROUTE_ACTIONS				ACTION_END 
#define  CCFSM_STATE_INCOMING_ACTIONS			ACTION_ANSWER | ACTION_FORWARD | ACTION_REJECT
#define  CCFSM_STATE_OUTGOING_ACTIONS		    ACTION_END    
#define  CCFSM_STATE_RINGING_ACTIONS			ACTION_END	
#define  CCFSM_STATE_TALK_ACTIONS				ACTION_END | ACTION_HOLD | ACTION_TRANSFER | ACTION_BLIND | ACTION_CONFERENCE  | ACTION_CODECCHANGE
#define  CCFSM_STATE_HOLD_ACTIONS				ACTION_END | ACTION_RESUME | ACTION_NEW  | ACTION_CONFERENCE 
#define  CCFSM_STATE_HELD_ACTIONS				ACTION_END | ACTION_HOLD  
#define  CCFSM_STATE_INACTIVE_ACTIONS			ACTION_END | ACTION_RESUME
#define  CCFSM_STATE_BLIND_TRANSFERRING_ACTIONS	ACTION_END 
#define  CCFSM_STATE_BLIND_TRANSFERED_ACTIONS	ACTION_END
#define  CCFSM_STATE_ATTENDED_TRANSFERRING_ACTIONS	ACTION_END 
#define  CCFSM_STATE_ATTENDED_TRANSFERED_ACTIONS	ACTION_END
#define  CCFSM_STATE_TERMINATING_ACTIONS		ACTION_END
#define  CCFSM_STATE_TERMINATED_ACTIONS			ACTION_END
#define  CCFSM_STATE_PARKED_ACTIONS				ACTION_END | ACTION_RESUME
#define  CCFSM_STATE_CONFERENCE_ACTIONS					ACTION_CONF_END | ACTION_CONF_HOLD	  
#define  CCFSM_STATE_CONFERENCE_ACTIONS_HOLD			ACTION_CONF_END | ACTION_CONF_RESUME  | ACTION_CONF_NEW 
#define  CCFSM_STATE_CONFERENCE_ACTIONS_ADD_MEMBER		ACTION_CONF_END | ACTION_CONF_HOLD	  | ACTION_CONF_NEW
 
#define	CCFSM_REQ_RETURN_INVALID_COMMAND		1000	
#define CCFSM_REQ_RETURN_INVALID_STATE			1001		
#define CCFSM_REQ_RETURN_INVALID_ARGUMENTS	1002 
#define CCFSM_REQ_RETURN_IVALID_CALL			  1003

#define MAX_ALLOWED_CALLS_PER_DECT_PORT   2
#define MAX_ALLOWED_DECT_PORT			        MAX_PCM_PORTS_SUPPORTED   //no zero based
#define MAX_ALLOWED_CALLS_PER_ATA_PORT    2
#define MAX_ALLOWED_ATA_PORT			        MAX_PCM_PORTS_SUPPORTED
#define MAX_ALLOWED_DECT_HANDSETS         MAX_SUPPORTED_ACCOUNTS				//6 + 1 for broadcast
#define CCFSM_DECT_BROADCAST_ID           MAX_ALLOWED_DECT_HANDSETS - 1//6 + 1 for broadcast zero based

typedef enum 
{
	CCFSM_HOLD_SENDONLY,
	CCFSM_HOLD_RECVONLY,
	CCFSM_HOLD_SENDRECV,
	CCFSM_HOLD_INACTIVE
}ccfsm_hold_flag;

typedef enum 
{
	CCFSM_DIRECTION_IN,
	CCFSM_DIRECTION_OUT,
	CCFSM_DIRECTION_IN_IPCALL,
	CCFSM_DIRECTION_OUT_IPCALL,
	CCFSM_DIRECTION_INTERNAL_CALL,
	CCFSM_DIRECTION_ENUM_SIZE
}ccfsm_call_type;

typedef enum __ccfsm_attached_entity
{
 	CCFSM_ATTACHED_GUI,
	CCFSM_ATTACHED_ATA,
	CCFSM_ATTACHED_DECT,
	CCFSM_ATTACHED_CONSOLE, 
	CCFSM_ATTACHED_WEB, 
	CCFSM_ATTACHED_REMOTE, 
	CCFSM_ATTACHED_BROADCAST, 
	CCFSM_ATTACHED_ENUM_SIZE 
}ccfsm_attached_entity;

typedef enum 
{
	CCFSM_AUDIO_DESKTOP,
	CCFSM_AUDIO_DECT,
	CCFSM_AUDIO_DECT_HANDSET,
	CCFSM_AUDIO_ATA_FXS 	 
}ccfsm_audio_peripheral;

typedef enum  
{
 	CCFSM_AUD_HANDSET_ID,
 	CCFSM_AUD_OPENLISTEN_ID,
	CCFSM_AUD_HANDSFREE_ID,
 	CCFSM_AUD_HEADSET_ID,
  CCFSM_AUD_DECTHANDSET_ID,
	CCFSM_AUD_ANALOGPHONE_ID
}ccfsm_audio_peripheral_input;


#define CCFSM_AUDIO_STATE_IDLE				0			
#define CCFSM_AUDIO_STATE_HANDSET			1<<CCFSM_AUD_HANDSET_ID
#define CCFSM_AUDIO_STATE_EXT_SPK			1<<CCFSM_AUD_HANDSFREE_ID
#define CCFSM_AUDIO_STATE_HEADSET			1<<CCFSM_AUD_HEADSET_ID
#define CCFSM_AUDIO_STATE_DECT_HANDSET		1<<CCFSM_AUD_DECTHANDSET_ID
#define CCFSM_AUDIO_STATE_ANALOG_PHONE		1<<CCFSM_AUD_ANALOGPHONE_ID
#define CCFSM_AUDIO_STATE_OPEN_LISTEN		1<<CCFSM_AUD_OPENLISTEN_ID

typedef enum  
{
	CCFSM_NO_AUDIO_INFO,
	CCFSM_HOOK_ON,
	CCFSM_HOOK_OFF,
	CCFSM_EXT_SPK_ON,
	CCFSM_EXT_SPK_OFF,
	CCFSM_HEADSET_ON,
	CCFSM_HEADSET_OFF,
	CCFSM_DECT_HANDSET_ON,
	CCFSM_DECT_HANDSET_OFF 
}ccfsm_audio_peripheral_status;

typedef enum{
	CCFSM_CONFIG_CODEC,	
 	CCFSM_NARROWBAND_CODEC,
	CCFSM_WIDEBAND_CODEC
}ccfsm_codec_type;

typedef enum{
	CCFSM_DELETE_DECT_START,
	CCFSM_DELETE_DECT_END,
	CCFSM_DELETE_DECT_NO_DEVICE,
}ccfsm_del_result;

typedef enum{
  CCFSM_NORMAL_DECT_MODE,
  CCFSM_REG_HANDSET,
  CCFSM_DELETE_HANDSETS
}ccfsm_reg_mode;

typedef enum{
   CCFSM_REGISTRATION_FAILED,
	 CCFSM_REGISTRATION_OK,
}ccfsm_reg_result;

typedef enum _ring_mode
{
  STOP_RINGING=0,
  START_RINGING,
  OUTGOING_RINGING
}ring_mode;

//IDs of Requests and Indications to Call Control 

//CCFSM_AUDIO_CONFIF,  // LINE_CONFIG_IND,
typedef enum {
  CCFSM_CONNECT_REQ,
  CCFSM_DISCONNECT_REQ,
  CCFSM_CONFIGURE_REQ,
  CCFSM_HOOKON_REQ,
  CCFSM_HOOKOFF_REQ,
  CCFSM_SELECT_AUDIO_PATH_REQ,
  CCFSM_REGISTER_REQ,
  CCFSM_OUTGOING_CALL_REQ,
  CCFSM_INTERNAL_CALL_REQ,
  CCFSM_FLASH_IND,
  CCFSM_KEY_PRESS_IND,
  CCFSM_STAR_CODE_REQ,
  CCFSM_DECT_SAVE_REQ, //????
  CCFSM_DECT_CODEC_IND, // ???
  CCFSM_ALLOCATE_CALL_REQ,
  CCFSM_DEALLOCATE_CALL_REQ,
  CCFSM_ANSWER_REQ,
  CCFSM_BLINDTRANSFER_REQ,
  CCFSM_ATTENDEDTRANSFER_REQ,
  CCFSM_CALL_FORWARD_REQ,
  CCFSM_CALL_HOLD_REQ,
  CCFSM_CALL_RESUME_REQ,
  CCFSM_CONFERENCE_REQ,
  CCFSM_SWITCH_TO_LINE_REQ,
  CCFSM_TERMINATE_REQ,
  CCFSM_VOLUMEUP_REQ,
  CCFSM_VOLUMEDOWN_REQ,
  CCFSM_INCALL_CODEC_CHANGE_REQ,
  CCFSM_DTMF_REQ,
  CCFSM_RESET_REQ,
  CCFSM_FAX_REQ,
  CCFSM_IM_SEND_REQ,
  CCFSM_PRERENCE_PUBLISH_REQ,
  CCFSM_PRERENCE_SUBSCRIBE_REQ,
  CCFSM_PRESENCE_UNSUBSCRIBE_REQ,
  CCFSM_ID_ENUM_SIZE  /* Always last */
 
}ccfsm_req_id;
typedef enum
{
	STATUS_OPN,
	STATUS_CLS
}pr_status_basic;

typedef enum
{
	STATUS_ONLINE,
	STATUS_AWAY,
	STATUS_DND
}pr_note;

typedef enum{
	CCFSM_PRESENCE_SUBSCRIBE_IDLE,
	CCFSM_PRESENCE_SUBSCRIBE_INIT,
	CCFSM_PRESENCE_SUBSCRIBE_FAILED,
	CCFSM_PRESENCE_SUBSCRIBE_SUCCESS,
	CCFSM_PRESENCE_SUBSCRIBE_IN_PROGRESS,
	CCFSM_PRESENCE_UNSUBSCRIBE_IN_PROGRESS,
	CCFSM_PRESENCE_SUBSCRIBE_TERMINATED
}ccfsm_presence_subscription_status;

typedef struct _ccfsm_dect_save_req_data //CCFSM_DECT_SAVE_REQ
{
  ccfsm_req_id	req_id ;
  ccfsm_attached_entity	 attachedentity;
  int	status;
  int	reg_id;
  int	accountid;
}ccfsm_dect_save_req_data;
 
  
typedef struct _ccfsm_disconnect_iface_req_data {
  ccfsm_req_id						req_id;   /* AP_CALLCNTRL_DISCONNECT_IND */
  ccfsm_attached_entity				attachedentity;			// attached entity 
  ccfsm_audio_peripheral			audio_peripheral_type;
  char								ClientName[16]; /*Application name */
  int			                    line;			/* ATA/DECT Line id */
 
}ccfsm_disconnect_iface_req_data;

  
typedef struct _ccfsm_reset_req_data {
  ccfsm_req_id						req_id;			        //    
  ccfsm_attached_entity				attachedentity;			// attached entity 
}ccfsm_reset_req_data;

typedef struct _ccfsm_dtmf_req_data {
  ccfsm_req_id						req_id;			        //    
  int								callid;
  char								key;
}ccfsm_dtmf_req_data;
  

typedef struct _ccfsm_connect_iface_req_data {
  ccfsm_req_id						req_id;			        //    
  ccfsm_attached_entity				attachedentity;			// attached entity 
  ccfsm_audio_peripheral			audio_peripheral_type;
  char								ClientName[16]; /*Application name */
  unsigned short					PortToSendAnswer; /* Server side needs that in order to have the port for callbacks */
  unsigned char						line;			/* ATA/DECT Line id */
}ccfsm_connect_iface_req_data;

typedef struct _ccfsm_select_audio_path_req_data {
  ccfsm_req_id						req_id;					// AP_CALLCNTRL_SELECT_AUDIO_PATH_REQ 
  ccfsm_attached_entity				attachedentity;			// attached entity 
  ccfsm_audio_peripheral			audio_peripheral_type;
  unsigned char                     path;					//  
  unsigned char                     line;					// ???????
}ccfsm_select_audio_path_req_data;

typedef struct _ccfsm_hookon_req_data {
  ccfsm_req_id						req_id;		  
  ccfsm_attached_entity				attachedentity;			// attached entity 
  ccfsm_audio_peripheral			audio_peripheral_type;
  int								line;					// ATA/DECT Line id
  int								port;					// PCM Port

}ccfsm_hookon_req_data;

typedef struct _ccfsm_hookoff_req_data {
  ccfsm_req_id						req_id;					//  AP_CALLCNTRL_SELECT_AUDIO_PATH_REQ 
  ccfsm_attached_entity				attachedentity;			// attached entity 
  ccfsm_audio_peripheral			audio_peripheral_type;
  int								line;					// ATA/DECT Line id 
  int								port;					// PCM Port
}ccfsm_hookoff_req_data;

typedef struct _ccfsm_allocate_newcall_req_data {
  ccfsm_req_id						req_id;			        //    
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_audio_peripheral			audio_peripheral_type;
  int								callid;
  int								portid;
  int								accountid; 
}ccfsm_allocate_newcall_req_data;

typedef struct _ccfsm_configure_req_data {
  ccfsm_req_id						req_id;			        //    
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_audio_peripheral			audio_peripheral_type;
  void*								params;
  int								size;
 }ccfsm_configure_req_data;

typedef struct _ccfsm_register_req_data {
  ccfsm_req_id						req_id;		  
  ccfsm_attached_entity				attachedentity;			// attached entity 
  ccfsm_audio_peripheral			audio_peripheral_type;
  int								account;					 
  int								status;
}ccfsm_register_req_data;


//  Request to Call Control  - Create a new outgoing call 
typedef struct _ccfsm_outgoing_call_req_data {
  ccfsm_req_id						req_id;			        // CCFSM_OUTGOING_CALL_REQ  
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_audio_peripheral			audio_peripheral_type;
  ccfsm_audio_peripheral_status		audio_status;
  ccfsm_call_type					calltype;
  ccfsm_codec_type 					codec;				    // DECT codec type   
  int								reference;
  int								callid;
  int								portid;
  int								accountid;
  char								dialled_num[64];		// Dialled Number   
}ccfsm_outgoing_call_req_data;

//  Request to Call Control  - Send an instant message
#define MAX_IM_TEXT_SIZE 64
enum {
	ID_IM_TYPE_PLAIN_TEXT,
	ID_IM_TYPE_PLAIN_HTML,
	ID_IM_TYPE_PLAIN_UNDEFINED
};
typedef struct _ccfsm_send_im_req_data {
  ccfsm_req_id						req_id;			        // CCFSM_SEND_SEND_REQ  
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_call_type					calltype;
  int								portid;
  int								accountid;
  char								dialled_num[64];		// Dialled Number   
  char								text[MAX_IM_TEXT_SIZE];				// Intant Message
}ccfsm_send_im_req_data;

typedef struct _ccfsm_send_presence_publish_req_data {
  ccfsm_req_id						req_id;			        // CCFSM_SEND_SEND_REQ
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_call_type					calltype;
  int								portid;
  int								accountid;
  pr_status_basic 					pres;
  pr_note 							note;
  int 								expires;
}ccfsm_send_presence_publish_req_data;

typedef struct _ccfsm_send_presence_subscribe_req_data {
  ccfsm_req_id						req_id;			        // CCFSM_SEND_SEND_REQ
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_call_type					calltype;
  int								portid;
  int								accountid;
  char 								subAccount [64];
  char								subRealm [64];
  int 								expires;
}ccfsm_send_presence_subscribe_req_data;

typedef struct _ccfsm_send_presence_unsubscribe_req_data {
  ccfsm_req_id						req_id;			        // CCFSM_SEND_SEND_REQ
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_call_type					calltype;
  int								portid;
  int								sid;
}ccfsm_send_presence_unsubscribe_req_data;

typedef struct _ccfsm_outgoing_call_w_replaces_req_data {
  ccfsm_req_id						req_id;			        // CCFSM_OUTGOING_CALL_REQ  
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_audio_peripheral			audio_peripheral_type;
  ccfsm_audio_peripheral_status	audio_status;
  ccfsm_codec_type 					codec;				    // DECT codec type   
  int								portid;
  ccfsm_call_type					calltype;
  int								callid;
  int								transferred_callid;
  int								accountid;
  char								transfer_from[64];		 
  char								transfer_to[64];		 
  char								replaces[256];		 

}ccfsm_outgoing_call_w_replaces_req_data;
 //  Indication to Call Control  - Received a new outgoing call 
typedef struct _ccfsm_incoming_call_ind_data {
  ccfsm_req_id						ind_id;			        // CCFSM_OUTGOING_CALL_REQ  
  ccfsm_attached_entity				attachedentity;			// attached entity
  ccfsm_audio_peripheral			audio_peripheral_type;
  ccfsm_call_type					calltype;
  int								portid;
  int								callid;
  int								accountid;
  char								username[64];		// Dialled Number   
  char								display[64];		// Dialled Number   
  char								host[64];		// Dialled Number   
  int			 					codec;		   // DECT codec type   
}ccfsm_incoming_call_ind_data;
 
typedef struct _ccfsm_call_answer_req_data
{
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;			// attached entity
  int								portid;
  int								callid;
  int								accountid;
  ccfsm_codec_type 					codec;				     
}ccfsm_call_answer_req_data; 


typedef struct _ccfsm_call_hold_req_data
{
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;			// attached entity
  int								portid;
  int								callid;
  int								accountid;
}ccfsm_call_hold_req_data; 

typedef struct _ccfsm_blindtransfer_req_data
{
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;			// attached entity
  int								portid;
  int								callid;
  int								accountid;
  unsigned char						number[64];
  ccfsm_codec_type 					codec;				     
}ccfsm_blindtransfer_req_data; 

 typedef struct _ccfsm_attendedtransfer_req_data
{
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;			// attached entity
  int								portid;
  int								callid;
  int								accountid;
  int								calltotransfer; 

}ccfsm_attendedtransfer_req_data;
 
typedef struct _ccfsm_call_terminate_req_data
{
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;			 
  int								portid;
  int								callid;
  int								accountid;
  ccfsm_codec_type 					codec;				     
}ccfsm_call_terminate_req_data; 

 
typedef struct _ccfsm_call_resume_req_data
{
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;			 
  int								portid;
  int								callid;
  int								accountid;
  ccfsm_codec_type 					codec;				     
}ccfsm_call_resume_req_data; 

typedef struct _ccfsm_fax_req_data {
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;
  int								portid;
  int								callid;
  int								accountid;
  int               event;
 
}ccfsm_fax_req_data;


typedef struct _ccfsm_incall_codec_change_req_data
{
  ccfsm_req_id						req_id;			         
  ccfsm_attached_entity				attachedentity;			 
  int								portid;
  int								callid;
  int								accountid;
  ccfsm_codec_type 					codec;				     
}ccfsm_incall_codec_change_req_data; 


/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// CALL BACK DATA
/////////////////////////////////////////////////////////////////////////////////////////////////////
typedef enum {
	CCFSM_REGISTRATION_EVENT=0,
	CCFSM_EVENT_NOTIFICATION_EVENT, //FIX BROADWORKS
	CCFSM_NEW_CALL_CB,
	CCFSM_NEW_CALL_ANSWERED_CB,
 	CCFSM_PROCEEDING_CB,
 	CCFSM_RINGING_CB,
	CCFSM_ANSWERED_CB,
	CCFSM_ANSWERING_CB,
	CCFSM_HOLD_CB,
	CCFSM_HOLDING_CB,
	CCFSM_HELD_CB,
	CCFSM_INACTIVE_CB,
	CCFSM_INACTIVATING_CB,
	CCFSM_RESUMED_CB,
	CCFSM_RESUMING_CB,
	CCFSM_TERMINATING_CB,
	CCFSM_TERMINATED_CB,
	CCFSM_REJECTED_CB,
	CCFSM_REDIRECTED_CB,
	CCFSM_BLINDTRANSFERRING_CB,
	CCFSM_BLINDTRANSFERED_CB,
	CCFSM_ATTENDEDTRANSFERRING_CB,
	CCFSM_ATTENDEDTRANSFERED_CB,
	CCFSM_TRANSFER_ACCEPTED_CB,
 	CCFSM_INCALL_CODEC_CHANGED_CB,
	CCFSM_COMMAND_STATUS_CB,
	CCFSM_NEW_ALLOCATE_CB,
	CCFSM_NEW_DEALLOCATE_CB,
 	CCFSM_ATTENDEDTRANSFER_IND,
	CCFSM_BLINDTRANSFER_IND,
	CCFSM_DECT_REGISTRATION_IND, 
	CCFSM_IM_RECV_IND,  
	CCFSM_PRESENCE_NOTIFY_IND,
	CCFSM_PRESENCE_SUBSCRIBE_CB
	
 
}ccfsm_cback_id;

 typedef enum{
	CCFSM_REG_IDLE,
	CCFSM_REG_INIT,
	CCFSM_REG_FAILED,
	CCFSM_REG_SUCCESS,
	CCFSM_REG_IN_PROGRESS,
	CCFSM_UNREG_IN_PROGRESS,
	CCFSM_REG_TERMINATED
}ccfsm_registration_status;
  
typedef struct _ccfsm_registration_cback_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	 attachedentity;
  ccfsm_registration_status	status;
  int accountid;
}ccfsm_registration_cback_data;
 
typedef struct _ccfsm_notify_indication_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	 attachedentity;
  int   status;
  int	accountid;
  
}ccfsm_notify_indication_data;


typedef struct _ccfsm_dect_registration_indication_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	 attachedentity;
  int	regid;
  int	accountid;
}ccfsm_dect_registration_indication_data;
 
 
typedef struct _ccfsm_allocate_new_call_cback_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	attachedentity;
  ccfsm_call_states state;
  int accountid;
  int callid;
  int portid;
}ccfsm_allocate_new_call_cback_data;

typedef struct _ccfsm_im_recv_indication_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	 attachedentity;
  int	accountid;
  char	username[64]; // Dialled Number   
  char	display[64]; // Dialled Number   
  char	host[64]; // Dialled Number   
  int callid;
  int im_type;
  void* im_text;
  int   im_size;

}ccfsm_im_recv_indication_data;
 
typedef struct _ccfsm_presence_notify_indication_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	 attachedentity;
  int	accountid;
  char	username[64]; // Dialled Number
  char	display[64]; // Dialled Number
  char	host[64]; // Dialled Number
  int callid;
  char not_type [64];
  char not_subtype [64];
  void* not_text;
  int   not_size;
}ccfsm_presence_notify_indication_data;

 typedef struct _ccfsm_presence_subscribe_cback_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	 attachedentity;
  ccfsm_presence_subscription_status	status;
  int accountid;
  char subAccount[64];
  char subRealm[64];
  int sid;
}ccfsm_presence_subscribe_cback_data;

 
 typedef struct _ccfsm_new_call_cback_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	attachedentity;
  ccfsm_call_states state;
  int accountid;
  int	reference;
  int callid;
  int portid;
  ccfsm_call_type calltype;
  char	username_num[64]; // Dialled Number   
  char	display_num[64]; // Dialled Number   
  char	host_num[64]; // Dialled Number   
  ccfsm_codec_type 	codec; // DECT codec type   
  int status;

}ccfsm_new_call_cback_data;



typedef struct _ccfsm_updatestatus_cback_data
{
  ccfsm_cback_id	cback_id ;
  ccfsm_attached_entity	attachedentity;
  ccfsm_call_states state;
  int accountid;
  int reference;
  int callid;
  int parameter;//state specific parameter
  int reasoncode; 
  char info[256];    

}ccfsm_updatestatus_cback_data;
   
typedef struct _ccfsm_transfer_indication_data
{
  ccfsm_cback_id cback_id ;
  ccfsm_attached_entity	attachedentity;
  ccfsm_call_states state;
  ccfsm_call_type calltype;
  int accountid;
  int callid;
  int transfermode;  //ATTENDED TRANSFER, BLIND TRANSFER
  int newcallid;
  char transfer_to[64];     
  char transfer_from[64];
  char replaces[256];	
}ccfsm_transfer_indication_data;

   
typedef union {
	ccfsm_cback_id						cback_id;
	ccfsm_attached_entity				attachedentity;
	ccfsm_registration_cback_data		ccfsm_registration_cback;
	ccfsm_allocate_new_call_cback_data  ccfsm_allocate_new_call_cback;
	ccfsm_new_call_cback_data			ccfsm_new_call_cback;	
	ccfsm_updatestatus_cback_data		ccfsm_updatestatus_cback;	
	ccfsm_transfer_indication_data		ccfsm_transfer_indication_cback;
	ccfsm_dect_registration_indication_data	ccfsm_dect_registration_indication;
	ccfsm_im_recv_indication_data		ccfsm_im_recv_indication;
	ccfsm_presence_notify_indication_data   ccfsm_presence_notify_indication;
	ccfsm_presence_subscribe_cback_data		ccfsm_presence_subscribe_cback;
	ccfsm_notify_indication_data		ccfsm_notify_indication_cback;

}ccfsm_cback_type;

typedef union {
	ccfsm_req_id								req_id;
	ccfsm_reset_req_data						ccfsm_reset_req;
	ccfsm_allocate_newcall_req_data				ccfsm_allocate_newcall_req;		
	ccfsm_outgoing_call_w_replaces_req_data		ccfsm_outgoing_call_w_replaces_req;
	ccfsm_call_answer_req_data					ccfsm_call_answer_req;
 	ccfsm_call_terminate_req_data				ccfsm_call_terminate_req;
	ccfsm_call_hold_req_data					ccfsm_call_hold_req;
	ccfsm_call_resume_req_data					ccfsm_call_resume_req;
	ccfsm_blindtransfer_req_data				ccfsm_blindtransfer_req;
	ccfsm_attendedtransfer_req_data				ccfsm_attendedtransfer_req;
 	ccfsm_connect_iface_req_data				ccfsm_connect_iface_req;
	ccfsm_disconnect_iface_req_data				ccfsm_disconnect_iface_req;
	ccfsm_select_audio_path_req_data			ccfsm_select_audio_path_req;
 	ccfsm_hookon_req_data						ccfsm_hookon_req;
	ccfsm_hookoff_req_data						ccfsm_hookoff_req;
	ccfsm_outgoing_call_req_data				ccfsm_outgoing_call_req;
	ccfsm_dect_save_req_data					ccfsm_dect_save_req;
	ccfsm_dtmf_req_data							ccfsm_dtmf_req;
	ccfsm_fax_req_data							ccfsm_fax_req;
	ccfsm_incall_codec_change_req_data			ccfsm_incall_codec_change_req;
	ccfsm_configure_req_data					ccfsm_configure_req; 
	ccfsm_register_req_data						ccfsm_register_req;
	ccfsm_send_im_req_data						ccfsm_send_im_req;
	ccfsm_send_presence_publish_req_data		ccfsm_send_presence_publish_req;
	ccfsm_send_presence_subscribe_req_data		ccfsm_send_presence_subscribe_req;
	ccfsm_send_presence_unsubscribe_req_data	ccfsm_send_presence_unsubscribe_req;

}ccfsm_req_type;

#define CALLCONTROL_REQUEST 1
#define CALLCONTROL_CALLBACK 2
typedef struct _callcontrol_message
{	
	int type;
	unsigned char msg[512];
}callcontrol_message;


#endif





    

