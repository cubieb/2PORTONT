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
 * File:	si_callcntrl_api.h
 * Purpose: This file exports the CALL CONTROL (based on phone CALL CONTROL) 
 *			    interface to any other interface like ATA or DECT Handler.
 *          In the current implementation, both the ATA/DECT and the Call Control are 
 *          running in the same thread (task context) and therefore there is
 *          no need for inter-process (or inter-task) communication model with 
 *          a signalling and queuing mechanism. At the moment the communication
 *          between the ATA/DECT handler and the Call Control is done by a direct 
 *			    function call.
 *          Since there might be a need in the future for these two modules to
 *          run from different threads (task contexts), their interface has
 *          been implemented with signalling/queuing mechanism in mind.
 *             
 * Created:		21/05/2008
 * By:			  ED
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_CALLCNTRL_API_H
#define SI_CALLCNTRL_API_H
 
/*========================== Include files ==================================*/

/* access menuconfig options */
#include "operation_mode_defs.h" 
 
#if ((defined DECT_DESIGN) || (defined DECT_HANDSET_DESIGN)) 
#include "../SiDECT/std-def.h" 
#elif defined CONFIG_SC1445x_LEGERITY_890_SUPPORT
#include "vp_api.h"
#else
typedef signed char       int8;
typedef unsigned char     uint8;
typedef unsigned short    uint16;
typedef signed short      int16;
typedef unsigned long     uint32;
typedef signed long       int32;
typedef unsigned char     boolean;
#endif

#define CC_SERVER_PORT          17180

/*========================== Local macro definitions & typedefs =============*/

/* ATA/DECT => CALL CONTROL message identifier definition
*/
typedef enum {
  AP_CALLCNTRL_CONNECT_IND,
  AP_CALLCNTRL_DISCONNECT_IND,
  AP_CALLCNTRL_HOOK_ON_IND,
  AP_CALLCNTRL_HOOK_OFF_IND,
  AP_CALLCNTRL_CONS_HOOK_ON_IND,
  AP_CALLCNTRL_CONS_HOOK_OFF_IND,
  AP_CALLCNTRL_HOOK_FLASH_IND,
  AP_CALLCNTRL_KEY_PRESS_IND,
  AP_CALLCNTRL_STAR_CODE_IND,
  AP_CALLCNTRL_LINE_CONFIG_IND,
#ifdef DECT_DESIGN
  AP_CALLCNTRL_DECT_INFO_IND,
  AP_CALLCNTRL_DECT_CODEC_IND,
#endif
  AP_CALLCNTRL_OUTGOING_CALL_REQ,
  AP_CALLCNTRL_CALL_TRANSFER_REQ,
  AP_CALLCNTRL_BLINDTRANSFER_REQ,
  AP_CALLCNTRL_ATTENDTRANSFER_REQ,
  AP_CALLCNTRL_CALL_FORWARD_REQ,
  AP_CALLCNTRL_CALL_HOLD_REQ,
  AP_CALLCNTRL_CALL_UNHOLD_REQ,
  AP_CALLCNTRL_CONFERENCE_REQ,
  AP_CALLCNTRL_SWITCH_TO_LINE_REQ,
  AP_CALLCNTRL_ANSWER_REQ,
  AP_CALLCNTRL_TERMINATE_REQ,
  AP_CALLCNTRL_DECT_REG_INIT_REQ,
  AP_CALLCNTRL_SELECT_AUDIO_PATH_REQ,
  AP_CALLCNTRL_VOLUMEUP_REQ,
  AP_CALLCNTRL_VOLUMEDOWN_REQ,
  AP_CALLCNTRL_ID_ENUM_SIZE  /* Always last */
}ap_callcntrl_msg_id_type;

/* CALL CONTROL => ATA/DECT message identifier definition
*/
typedef enum {
  CALLCNTRL_AP_INCOMING_CALL_IND,
  CALLCNTRL_AP_OUTGOING_CALL_RINGING_IND,
  CALLCNTRL_AP_OUTGOING_CALL_ANSWERED_IND,
  CALLCNTRL_AP_CALL_TERMINATED_IND,
  CALLCNTRL_AP_CALL_TRANSFERED_IND,
  CALLCNTRL_AP_MESSAGE_WAITING_IND,
#ifdef DECT_DESIGN
  CALLCNTRL_AP_CODEC_INFO_IND,
#endif
#if ((defined DECT_DESIGN) || (defined DECT_HANDSET_DESIGN))
  CALLCNTRL_AP_REG_INIT_IND,
#endif
  CALLCNTRL_AP_ID_ENUM_SIZE  /* Always last */
}callcntrl_ap_msg_id_type;

/* Definitions for Direct IP call and PSTN called. To be used
** from ATA/DECT to indicate to CALL CONTROL the type of the outgoing call
*/
typedef enum {
  AP_CALL_DIRECT_IP,
  AP_CALL_PSTN,
  AP_CALL_USER_ID_IP	/* i.e. 200@192.100.10.30 */ 
}ap_call_type;

/* IDs of all supported star codes 
*/
typedef enum{  
  SC_ADD_TO_CONF_CALL,    /* Add another caller to the conf call */ 
  SC_BLIND_TRANSFER,
  SC_CALLER_ID_BLOCK,
  SC_CALLER_ID_SEND,
  SC_CALL_WAITING_OFF,
  SC_CALL_WAITING_ON,
  SC_CALL_FRWD_UNCOND_ON,
  SC_CALL_FRWD_UNCOND_OFF,
  SC_CALL_FWRD_BUSY_ON,
  SC_CALL_FWRD_BUSY_OFF,
  SC_CALL_FRWD_DELAYED_ON,
  SC_CALL_FRWD_DELAYED_OFF,
  SC_INTERNAL_CALL,      /* Next outgoing call will be an internal call */
  SC_CODEC_SELECTION,
  SC_USER_ID_IP_CALL,     /* Next outgoing call will be IP call with User ID */
  SC_ENUM_SIZE            /*Always last */
}ap_star_code_id_type;

/* ATA line configurations.
*/
typedef enum{  
  ATA_1_FXS_NO_FXO,
  ATA_2_FXS_NO_FXO,
  ATA_3_FXS_NO_FXO,
  ATA_4_FXS_NO_FXO,
  ATA_1_FXS_1_FXO,
  ATA_2_FXS_1_FXO,
  ATA_2_FXS_2_FXO,
  ATA_1_FXS_1_DECT,
  ATA_2_FXS_1_DECT
}ap_line_conf_type;

/* Supported codecs for selection via star codes
*/
typedef enum{ 
  SC_PCMU,
  SC_G_726,
  SC_G_723,
  SC_PCMA,
  SC_G_722,
  SC_G_729
}ap_codec_id_type;

#ifdef DECT_DESIGN
/* This enum defines the possible CODEC types supported by a DECT device.
** This enumeration is paired with ApiCodecIdType in api-inf.h. Do not change !! */
typedef enum
{
  DECT_NONE             = 0x00, /*  */
  DECT_USER_SPECIFIC_32 = 0x01, /* User specific, information transfer rate 32 kbit/s */
  DECT_G726             = 0x02, /* G.726 ADPCM, information transfer rate 32 kbit/s */
  DECT_G722             = 0x03, /* G.722, , information transfer rate 64 kbit/s */
  DECT_G711A            = 0x04, /* G.711 A-law PCM, information transfer rate 64 kbit/s */
  DECT_G711U            = 0x05, /* G.711 u-law PCM, information transfer rate 64 kbit/s */
  DECT_G7291            = 0x06, /* G.729.1, information transfer rate 32 kbit/s */
  DECT_MP4_32           = 0x07, /* MPEG-4 ER AAC-LD, information transfer rate 32 kbit/s */
  DECT_MP4_64           = 0x08, /* MPEG-4 ER AAC-LD, information transfer rate 64 kbit/s */
  DECT_USER_SPECIFIC_64 = 0x09, /* User specific, information transfer rate 64 kbit/s */
  DECT_INVALID          = 0x0A  /* [0x0A; 0xFF]is reserved */
} ap_dect_codec_id_type;

typedef enum{
  DECT_NARROWBAND_CODEC,
  DECT_WIDEBAND_CODEC
}ap_dect_codec_type;

typedef enum{
  CODEC_SUPPORTED,
  CODEC_NOT_SUPPORTED
}ap_dect_codec_supported;
#endif

/* --------------------------------------------------------------------------**
** ATA/DECT => CALL CONTROL indication/request message type definitions
** --------------------------------------------------------------------------*/

/* AP_CALLCNTRL_CONNECT_IND message type definition. 
** Send this message to CC to initiate connection
*/
typedef struct _ap_callcntrl_connect_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_CONNECT_IND */
  char                      ClientName[16]; /*Application name */
  uint16                    PortToSendAnswer; /* Server side needs that in order to have the port for callbacks */
  uint8                     line;			/* ATA/DECT Line id */
}ap_callcntrl_connect_ind_type;

/* AP_CALLCNTRL_DISCONNECT_IND message type definition. 
** Send this message to CC to disable connection
*/
typedef struct _ap_callcntrl_disconnect_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_DISCONNECT_IND */
  char                      ClientName[15]; /*Application name */
  uint8                     line;			/* ATA/DECT Line id */
}ap_callcntrl_disconnect_ind_type;


/* AP_CALLCNTRL_HOOK_ON_IND message type definition. 
** Indicates ATA/DECT device HOOK-ON event to CALL CONTROL
*/
typedef struct _ap_callcntrl_hook_on_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_HOOK_ON_IND */
  uint8                     line;			/* ATA/DECT Line id */
}ap_callcntrl_hook_on_ind_type;


/* AP_CALLCNTRL_HOOK_OFF_IND message type definition. 
** Indicates ATA/DECT device HOOK-OFF event to CALL CONTROL
*/
typedef struct _ap_callcntrl_hook_off_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_HOOK_OFF_IND */
  uint8                     line;			/* ATA/DECT Line id */
  uint8                     pcm_port;
}ap_callcntrl_hook_off_ind_type;

/* AP_CALLCNTRL_CONS_HOOK_ON_IND message type definition. 
** Indicates ATA/DECT device HOOK-ON event to CALL CONTROL
*/
typedef struct _ap_callcntrl_cons_hook_on_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_CONS_HOOK_ON_IND */
  uint8                     line;			/* ATA/DECT Line id */
}ap_callcntrl_cons_hook_on_ind_type;


/* AP_CALLCNTRL_CONS_HOOK_OFF_IND message type definition. 
** Indicates ATA/DECT device HOOK-OFF event to CALL CONTROL
*/
typedef struct _ap_callcntrl_cons_hook_off_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_CONS_HOOK_OFF_IND */
  uint8                     line;			/* ATA/DECT Line id */
}ap_callcntrl_cons_hook_off_ind_type;


/* AP_CALLCNTRL_HOOK_FLASH_IND message type definition. 
** Indicates ATA/DECT device HOOK-FLASH event to CALL CONTROL
*/
typedef struct _ap_callcntrl_hook_flash_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_HOOK_FLASH_IND */
  uint8                     line;			/* ATA/DECT Line id */
}ap_callcntrl_hook_flash_ind_type;

/* AP_CALLCNTRL_KEY_PRESS_IND message type definition. 
** Indicates to CALL CONTROL that a key press has been detected
*/
typedef struct _ap_callcntrl_key_press_ind_type {
  ap_callcntrl_msg_id_type  msg_id;   /* AP_CALLCNTRL_KEY_PRESS_IND */
  uint8                     line;			/* ATA/DECT Line id */
}ap_callcntrl_key_press_ind_type;

/* AP_CALLCNTRL_STAR_CODE_IND message type definition. 
** Indicates to CALL CONTROL that a Star Code has been detected
*/
typedef struct _ap_callcntrl_star_code_ind_type {
  ap_callcntrl_msg_id_type   msg_id;    /* AP_CALLCNTRL_STAR_CODE_IND */
  uint8                      line;		  /* ATA/DECT Line id */
  ap_star_code_id_type       star_code; /* Star Code ID */
  uint8                      intern_star_code_data; /* Internal star code data */
}ap_callcntrl_star_code_ind_type;

/* AP_CALLCNTRL_LINE_CONFIG_IND message type definition. 
** Indicates to CALL CONTROL which lines have been activated
** and if a line is an FXS or FXO line.
*/
typedef struct _ap_callcntrl_line_config_ind_type {
  ap_callcntrl_msg_id_type   msg_id;      /* AP_CALLCNTRL_LINE_CONFIG_IND */
  ap_line_conf_type          line_config; /* ATA Line configurations */
}ap_callcntrl_line_config_ind_type;

#ifdef DECT_DESIGN
/* AP_CALLCNTRL_DECT_INFO_IND message type definition. 
** Inticates to CALL CONTROL DECT device registration info 
*/
typedef struct _ap_callcntrl_dect_info_ind_type {
  ap_callcntrl_msg_id_type  msg_id;          /* AP_CALLCNTRL_DECT_INFO_IND */
  uint8                     line;	           /* ATA/DECT Line id */
  uint8                     codec_list_len; /* The length of the CODEC list.*/
  uint8                     codecs[10];       /* CodecListLength number of bytes
                                            holding the CODEC list. */
}ap_callcntrl_dect_info_ind_type;

/* AP_CALLCNTRL_DECT_CODEC_IND message type definition. 
** Inticates to CALL CONTROL DECT device if the codec requested is supported 
*/
typedef struct _ap_callcntrl_dect_codec_ind_type {
  ap_callcntrl_msg_id_type  msg_id;           /* AP_CALLCNTRL_DECT_CODEC_IND */
  uint8                     line;	            /* ATA/DECT Line id */
  ap_dect_codec_supported   codec_supported;  /* Codec supported or not */

}ap_callcntrl_dect_codec_ind_type;
#endif /* DECT_DESIGN */

/* AP_CALLCNTRL_OUTGOING_CALL_REQ message type definition. 
** Outgoing call request to CALL CONTROL
*/
typedef struct _ap_callcntrl_outgoing_call_req_type {
  ap_callcntrl_msg_id_type  msg_id;			        /* AP_CALLCNTRL_OUTGOING_CALL_REQ */
  char                      dialled_num[64];		/* Dialled Number  */
  char                      displayname[64];		/* displayname */
  char                      username[64]; 			/* username */
  char                      host[64];      			/* host */
  uint8                     vline;              /* Virtual line */
  uint8                     line;					      /* ATA/DECT Line id */
  ap_call_type			        call_type;				  /* PSTN or Direct IP */
#ifdef DECT_DESIGN
  ap_dect_codec_type        codec;				      /* DECT codec type */
#endif
}ap_callcntrl_outgoing_call_req_type;

/* AP_CALLCNTRL_CALL_BLINDTRANSFER_REQ message type definition. 
** Call Transfer request to CALL CONTROL
*/
typedef struct _ap_callcntrl_blindtransfer_req_type {
  ap_callcntrl_msg_id_type  msg_id;			        /* AP_CALLCNTRL_BLINDTRANSFER_REQ */
  char                      dialled_num[64];		/* Dialled Number  */
  uint8                     vline;					    /* Which vline to transfer */
  uint8                     line;					      /* ATA/DECT Line id */
}ap_callcntrl_blindtransfer_req_type;

/* AP_CALLCNTRL_CALL_ATTENDTRANSFER_REQ message type definition. 
** Call Transfer request to CALL CONTROL
*/
typedef struct _ap_callcntrl_attendtransfer_req_type {
  ap_callcntrl_msg_id_type  msg_id;			        /* AP_CALLCNTRL_ATTENDTRANSFER_REQ */
  char                      dialled_num[64];		/* Dialled Number  */
  uint8                     vline;					    /* Which vline to transfer */
  uint8                     line;					      /* ATA/DECT Line id */
}ap_callcntrl_attendtransfer_req_type;

/* AP_CALLCNTRL_CALL_FORWARD_REQ message type definition. 
** Call Forward request to CALL CONTROL
*/
typedef struct _ap_callcntrl_call_fwd_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_CALL_FORWARD_REQ */
  char                      dialled_num[64];	/* Dialled Number  */
  ap_call_type		          call_type;				/* PSTN or Direct IP */
  uint8                     line;					    /* ATA/DECT Line id */
}ap_callcntrl_call_fwd_req_type;

/* AP_CALLCNTRL_CALL_HOLD_REQ message type definition. 
** Call Hold request to CALL CONTROL
*/
typedef struct _ap_callcntrl_call_hold_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_CALL_HOLD_REQ */
  uint8                     vline;		  /* Which vline to hold */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_call_hold_req_type;

/* AP_CALLCNTRL_CALL_UNHOLD_REQ message type definition. 
** Call Unhold request to CALL CONTROL
*/
typedef struct _ap_callcntrl_call_unhold_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_CALL_UNHOLD_REQ */
  uint8                     vline;		  /* Which vline to unhold */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_call_unhold_req_type;

/* AP_CALLCNTRL_CONFERENCE_REQ message type definition. 
** Call conference request to CALL CONTROL
*/
typedef struct _ap_callcntrl_conference_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_CONFERENCE_REQ */
  uint8                     fvline;     /* First vline for conference */
  uint8                     svline;     /* Second vline for conference */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_conference_req_type;

/* AP_CALLCNTRL_SWITCH_TO_LINE_REQ message type definition. 
** Switch to a particular talking line request to CALL CONTROL
*/
typedef struct _ap_callcntrl_switch_to_line_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_SWITCH_TO_LINE_REQ */
  uint8                     vline;      /* Selected vline (call) */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_switch_to_line_req_type;

/* AP_CALLCNTRL_ANSWER_REQ message type definition. 
** Answer call request to CALL CONTROL
*/
typedef struct _ap_callcntrl_answer_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_ANSWER_REQ */
  uint8                     vline;      /* Vline to answer */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_answer_req_type;

/* AP_CALLCNTRL_TERMINATE_REQ message type definition. 
** Terminate call request to CALL CONTROL
*/
typedef struct _ap_callcntrl_terminate_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_TERMINATE_REQ */
  uint8                     vline;      /* Vline to terminate */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_terminate_req_type;

/* AP_CALLCNTRL_SELECT_AUDIO_PATH_REQ message type definition. 
** Audio path change request to CALL CONTROL
*/
typedef struct _ap_callcntrl_select_audio_path_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_SELECT_AUDIO_PATH_REQ */
  uint8                     path;       /* Audio path selection*/
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_select_audio_path_req_type;

/* AP_CALLCNTRL_VOLUMEUP_REQ  message type definition. 
** Volume increase request to CALL CONTROL
*/
typedef struct _ap_callcntrl_volumeup_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_VOLUMEUP_REQ */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_volumeup_req_type;

/* AP_CALLCNTRL_VOLUMEDOWN_REQ  message type definition. 
** Volume decrease request to CALL CONTROL
*/
typedef struct _ap_callcntrl_volumedown_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_VOLUMEDOWN_REQ */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_volumedown_req_type;

/* AP_CALLCNTRL_DECT_REG_INIT_REQ  message type definition. 
** Volume decrease request to CALL CONTROL
*/
typedef struct _ap_callcntrl_dect_reg_init_req_type {
  ap_callcntrl_msg_id_type  msg_id;			/* AP_CALLCNTRL_DECT_REG_INIT_REQ */
  uint8                     line;			  /* ATA/DECT Line id */
}ap_callcntrl_dect_reg_init_req_type;

/* Union of all ATA/DECT handler to CALL CONTROL messages 
*/
typedef union {
  ap_callcntrl_msg_id_type                  msg_id;
  ap_callcntrl_connect_ind_type             ap_callcntrl_connect_ind;
  ap_callcntrl_disconnect_ind_type          ap_callcntrl_disconnect_ind;
  ap_callcntrl_hook_on_ind_type             ap_callcntrl_hook_on_ind;
  ap_callcntrl_hook_off_ind_type            ap_callcntrl_hook_off_ind;
  ap_callcntrl_cons_hook_on_ind_type        ap_callcntrl_cons_hook_on_ind;
  ap_callcntrl_cons_hook_off_ind_type       ap_callcntrl_cons_hook_off_ind;
  ap_callcntrl_hook_flash_ind_type          ap_callcntrl_hook_flash_ind;
  ap_callcntrl_key_press_ind_type           ap_callcntrl_key_press_ind;
  ap_callcntrl_star_code_ind_type           ap_callcntrl_star_code_ind;
  ap_callcntrl_line_config_ind_type         ap_callcntrl_line_config_ind;
#ifdef DECT_DESIGN
  ap_callcntrl_dect_info_ind_type           ap_callcntrl_dect_info_ind;
  ap_callcntrl_dect_codec_ind_type          ap_callcntrl_dect_codec_ind;
#endif
  ap_callcntrl_outgoing_call_req_type       ap_callcntrl_outgoing_call_req;
  ap_callcntrl_blindtransfer_req_type       ap_callcntrl_blindtransfer_req;
  ap_callcntrl_attendtransfer_req_type      ap_callcntrl_attendtransfer_req;
  ap_callcntrl_call_fwd_req_type            ap_callcntrl_call_fwd_req;
  ap_callcntrl_call_hold_req_type           ap_callcntrl_call_hold_req;
  ap_callcntrl_call_unhold_req_type         ap_callcntrl_call_unhold_req;
  ap_callcntrl_conference_req_type          ap_callcntrl_conference_req;
  ap_callcntrl_switch_to_line_req_type      ap_callcntrl_switch_to_line_req;
  ap_callcntrl_answer_req_type              ap_callcntrl_answer_req;
  ap_callcntrl_terminate_req_type           ap_callcntrl_terminate_req;
  ap_callcntrl_select_audio_path_req_type   ap_callcntrl_select_audio_path_req;
  ap_callcntrl_volumeup_req_type            ap_callcntrl_volumeup_req;
  ap_callcntrl_volumedown_req_type          ap_callcntrl_volumedown_req;
  ap_callcntrl_dect_reg_init_req_type       ap_callcntrl_dect_reg_init_req;  

} ap_callcntrl_msg_u_type;

#if 0   
/* To be used when the ATA/DECT and the CALL CONTROL module run on a different thread 
** and communicate via queues.
*/

/* Queueable union of all ATA/DECT to CALL CONTROL messages */
typedef struct
{
  q_link_type               link;
  ap_callcntrl_msg_u_type   mes;

} ap_callcntrl_msg_type;

#endif

/* --------------------------------------------------------------------------**
** CALL CONTROL => ATA/DECT indication/request message type definitions
** --------------------------------------------------------------------------*/

/* CALLCNTRL_AP_INCOMING_CALL_IND message type definition. 
** Indicates new incoming call to ATA/DECT.
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;        /* CALLCNTRL_AP_INCOMING_CALL_IND */
  char                      dialled_num[64];	/* Dialled Number  */
  char                      caller_id[64];		/* Caller ID */
  uint8                     line;				      /* ATA/DECT Line id */
#ifdef DECT_DESIGN
  ap_dect_codec_type        codec;					  /* DECT codec type */
#endif
}callcntrl_ap_incoming_call_ind_type;


/* CALLCNTRL_AP_OUTGOING_CALL_RINGING_IND message type definition. 
** Indicates to ATA/DECT device that the outgoing call is ringing.
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;   /* CALLCNTRL_AP_OUTGOING_CALL_RINGING_IND */
  uint8                     line;	    /* ATA/DECT Line id */
}callcntrl_ap_ougoing_call_ringing_ind_type;


/* CALLCNTRL_AP_OUTGOING_CALL_ANSWERED_IND message type definition. 
** Indicates to ATA/DECT device that the outgoing call has been answered.
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;   /* CALLCNTRL_AP_OUTGOING_CALL_ANSWERED_IND */
  uint8                     line; 	  /* ATA/DECT  Line id */
#ifdef DECT_DESIGN
 ap_dect_codec_type        codec;     /* Codec type */
#endif
}callcntrl_ap_ougoing_call_answered_ind_type;


/* CALLCNTRL_AP_CALL_TERMINATED_IND message type definition. 
** Indicates to ATA/DECT device that the outgoing call has been terminated.
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;   /* CALLCNTRL_AP_CALL_TERMINATED_IND */
  uint8                     line;	    /* ATA/DECT Line id */
}callcntrl_ap_call_terminated_ind_type;


/* CALLCNTRL_AP_CALL_TRANSFERED_IND message type definition. 
** Indicates to ATA/DECT device that the outgoing call has been transfered.
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;   /* CALLCNTRL_AP_CALL_TRANSFERED_IND */
  uint8                     line;		  /* ATA/DECT Line id */
}callcntrl_ap_call_transfered_ind_type;


/* CALLCNTRL_AP_MESSAGE_WAITING_IND message type definition. 
** Indicates to ATA/DECT device that there is a message for this extension.
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;   /* CALLCNTRL_AP_MESSAGE_WAITING_IND */
  uint8                     line;	    /* ATA/DECT Line id */
}callcntrl_ap_massage_waiting_ind_type;

#ifdef DECT_DESIGN
/* CALLCNTRL_AP_CODEC_INFO_IND message type definition. 
** Indicates to DECT device what type of codec to use.
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;   /* CALLCNTRL_AP_CODEC_INFO_IND */
  uint8                     line;	    /* ATA/DECT Line id */
  ap_dect_codec_type        codec;    /* Codec type */
}callcntrl_ap_codec_info_ind_type;
#endif

#if ((defined DECT_DESIGN) || (defined DECT_HANDSET_DESIGN))
/* CALLCNTRL_AP_REG_INIT_IND message type definition. 
** Initiates the registration process for the DECT device
*/
typedef struct {
  callcntrl_ap_msg_id_type  msg_id;   /* CALLCNTRL_AP_REG_INIT_IND*/
  uint8                     line;	    /* ATA/DECT Line id */
}callcntrl_ap_reg_init_ind_type;
#endif


/* Union of all ATA/DECT to CALL CONRTROL messages 
*/
typedef union {
  callcntrl_ap_msg_id_type                     msg_id;
  callcntrl_ap_incoming_call_ind_type          callcntrl_ap_incoming_call_ind;
  callcntrl_ap_ougoing_call_ringing_ind_type   callcntrl_ap_ougoing_call_ringing_ind;
  callcntrl_ap_ougoing_call_answered_ind_type  callcntrl_ap_ougoing_call_answered_ind;
  callcntrl_ap_call_terminated_ind_type        callcntrl_ap_call_terminated_ind;
  callcntrl_ap_call_transfered_ind_type        callcntrl_ap_call_transfered_ind;
  callcntrl_ap_massage_waiting_ind_type        callcntrl_ap_massage_waiting_ind;
#ifdef DECT_DESIGN
  callcntrl_ap_codec_info_ind_type             callcntrl_ap_codec_info_ind;
#endif
#if ((defined DECT_DESIGN) || (defined DECT_HANDSET_DESIGN))
  callcntrl_ap_reg_init_ind_type               callcntrl_ap_reg_init_ind;              
#endif

} callcntrl_ap_msg_u_type;

#if 0   
/* To be used when the ATA/DECT and the CALL CONTROL module run on a different thread 
** and communicate via queues.
*/

/* Queueable union of all CALL CONTROL to ATA/DECT messages */
typedef struct
{
  q_link_type                 link;
  callcntrl_ap_msg_u_type     mes;

} callcntrl_ap_msg_type;

#endif

/*========================== Global variables ===============================*/


/*========================== Global function prototypes =====================*/

/* Function to be called from the CALL CONTROL module to send a message to ATA/DECT */
extern void callcntrl_send_msg_to_ap( callcntrl_ap_msg_u_type* msg );

/* Function to be called from the ATA/DECT module to send a message to CALL CONTROL */
extern void ap_send_msg_to_callcntrl( ap_callcntrl_msg_u_type* msg );


#endif /* SI_CALLCNTRL_API_H */
