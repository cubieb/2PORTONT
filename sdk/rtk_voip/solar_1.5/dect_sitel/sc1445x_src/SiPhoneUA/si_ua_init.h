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
 * File:		si_ua_init.h
 * Purpose:		
 * Created:		23/10/2007
 * By:			YP
 * Country:		Greece
 *
 *-----------------------------------------------------------------------------
 */

#ifndef SI_UA_INIT_H
#define SI_UA_INIT_H

#define FIX_MEMORY_LEAK  

/*========================== Include files ==================================*/
#include "../exosip2/src/eXosip.h"

#include "../common/si_config_file.h"
#include <ortp/ortp.h>

#include "si_ua_api.h" 
#include "../../mcu/sc1445x_mcu_block.h" 
/*========================== Definitions ==================================*/
#define emptystr(x) ((x == 0) || (x[0]== 0))
#define invalidport(x) (x<1024) //SIPIT 25

/*========================== Global definitions =============================*/
// UA definitions
#define SITEL_PHONE_USER_AGENT_LABEL  "SiTel/1.2.0"
#define SITEL_PHONE_USER_AGENT_VERSION  "1.2.0"
#define SIPHONE_UA_DEFAULT_SIP_PORT		5060

#define DTMF_MODE_INBAND		0
#define DTMF_MODE_INFO			1
#define DTMF_MODE_RTPPAYLOAD	2


// Message headers definitions
#define SIPHONE_UA_ALLOW_HEADER "INVITE,ACK,CANCEL,BYE,REFER,SUBSCRIBE,NOTIFY,MESSAGE,OPTIONS" //UPDATE 
//FIX BROADWORKS

//#define SIPHONE_UA_ACCEPT_HEADER "application/pidf+xml, application/xpidf+xml, multipart/related,application/rlmi+xmf"
#define SIPHONE_UA_ACCEPT_HEADER "application/simple-message-summary"
#define SIPHONE_UA_EVENT_SUB_HEADER "ua-profile;"

#define SIPHONE_UA_SUPPORTED_HEADER_REGISTER 	"100rel"
   
#define SIPHONE_UA_SUPPORTED_HEADER_INVITE 	"100rel,callerid,replaces" //"replaces,norefersub,timer"
#define SIPHONE_UA_SUPPORTED_HEADER_ACK200 	SIPHONE_UA_SUPPORTED_HEADER_INVITE //"100rel, timer, callerid"

#define SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER 	"100rel,timer" //"replaces,norefersub,timer" "callerid"
#define SIPHONE_UA_SUPPORTED_HEADER_ACK200_wTIMER 	SIPHONE_UA_SUPPORTED_HEADER_INVITE_wTIMER //"100rel, timer, callerid"



// SIP Status Definitions
#define STATUS_SIP_INIT			1
#define STATUS_SIP_REGISTERING	2
#define STATUS_SIP_REGISTERED	3
 
// EXOSIP error return values
#define EXOSIP_INIT_FAILED 			-100
#define EXOSIP_LISTEN_FAILED 			-101
#define EXOSIP_SET_PARAMETER_FAILED		-102
#define EXOSIP_BUILD_REGISTER_FAILED		-103
#define EXOSIP_SEND_REGISTER_FAILED		-104
#define EXOSIP_PTHREAD_CREATE_FAILED		-105
#define EXOSIP_BUILD_INVITE_FAILED		-106
#define EXOSIP_SEND_INVITE_FAILED		-107
#define EXOSIP_HOLD_ON_FAILED			-108
#define EXOSIP_HOLD_OFF_FAILED			-109
#define EXOSIP_CALL_TERMINATE_FAILED		-110


#define SI_MESSAGE_INVITE_ON_HOLD  101
#define SI_MESSAGE_INVITE_OFF_HOLD  102
#define SI_MESSAGE_INVITE_IMAGE  103
 

#define MAX_SUPPORTED_CALLS				10

#define CALL_STATE_IDLE					0
#define CALL_STATE_NEW					1
#define CALL_STATE_RINGING				2
#define CALL_STATE_REDIRECTING			3
#define CALL_STATE_REJECTING			4
#define CALL_STATE_ANSWERING			5
#define CALL_STATE_CONNECTED			6
#define CALL_STATE_CALLING				7
#define CALL_STATE_TRANSFERING			8
#define CALL_STATE_RELEASING			9
#define CALL_STATE_INCOMINGNEW			10

#define CALL_REMOTE_LOOPBACK			1
//FIX OCTOBER
typedef struct _audio_stream_parameters
{
	int payload;
	int d_payload;

//	char reserved1[64];//reserved SOS YPAPA
	char remote_address[64];
//	char reserved2[64];//reserved SOS YPAPA
	int remote_port;
//	char reserved3[64];//reserved SOS YPAPA
}audio_stream_parameters;

#define SI_MAX_AUDIO_HEADER				2
#define SI_MAX_PTIME					6
#define SI_MAX_AUDIO_PACKET				80 
#define SI_MAX_ALLOWED_PARTICIPANTS		3
#define MAX_SUPPORTED_SESSIONS			MAX_SUPPORTED_CALLS
  
typedef enum _sc1445x_CallTransferStates
{
	SIPUA_CALLTRANSFER_IDLE,
	SIPUA_TRANSFEROR_BLINK,
	SIPUA_TRANSFEROR_ATTENDED,
	SIPUA_TRANSFEREE_BLINK,
	SIPUA_TRANSFEREE_ATTENDED_TRYING,
	SIPUA_TRANSFEREE_ATTENDED_TRANSFERRING,
 	SIPUA_TARGER_BLINK,
	SIPUA_TARGER_ATTENDED
}sc1445x_CallTransferStates;

typedef enum _sc1445x_OFFER_ANSWER // SIPIT23
{
	SIPUA_OFFERANSWER_IDLE,
	SIPUA_OFFERANSWER_OFFER,
	SIPUA_OFFERANSWER_ANSWER 
}sc1445x_OFFER_ANSWER;

typedef struct si_ua_str_t
{
     char    *data;
     size_t  slen;
} si_ua_str_t;

typedef enum _accept_options
{
	ID_ACCEPT_NO,
	ID_ACCEPT_POSSIBLE,
	ID_ACCEPT_YES 
 }accept_options;

typedef struct _crypto_parameters_t
{
	int keyGerenated;
	int tx_encryption_suite_id;
	int tag;
 	crypto_profile rx_encryption_suite ;
	crypto_profile tx_encryption_suite[MAX_ENCRYPTION_SUITES];
}crypto_parameters_t;  

#define SI_UA_SESSION_TIMER_REFRESH_METHOD_NONE			0
#define SI_UA_SESSION_TIMER_REFRESH_METHOD_REINVITE		1
#define SI_UA_SESSION_TIMER_REFRESH_METHOD_UPDATE		2

struct _sicall {
	int callid;
	int state;

	int ToneInProgress;
	int SDPEnchanged; //SIPIT23
	int port;
	int accountID;
	int attachedentity;
	int codectype;

 	char remoteHost[64];
 	char replaces[256];
  	int localHold;
	int remoteHold;

	int codecChange;
    int faxsupport;	

	int directIP;
	int loop;
	int tid;
	int cid;
 	int did;
	int session_version;
    int session_id;
	char in_session_version[16];
    char in_session_id[16];

	int local_sdp_audio_port;
	unsigned int public_sdp_audio_port;
    int  localresume;
 	int  localrefer;
 	int callTransferInProgress;
	void *rCall;	
	void *tCall;	
 	char transfer_refer_to[256];
  	char transfer_from[256];
 	char transfer_to[256];
  	unsigned int initiator;
	int updateInterval;
	int updateKeeper;
	int refresher;
	int maxSessionExpireTime;
	int minSessionExpireTime;
	int refreshMethod; 
	int selectedSessionTimer;
	int currentSessionExpireTime;

	int RSeq;
	void* pMcuHandler;
	//fix: Memory Leak
 	RtpSession *pRtpSession;
  	audio_stream_parameters sStreamParameters;
	crypto_parameters_t crypto_parameters;
 	sc1445x_mcu_audio_media_stream_t mcuMediaStream;
  };

typedef struct _sicall sicall_t;
  
/////////////////////////////////////////////////
//Call Log
typedef enum _SiPhoneLogStatus { 
	SiPhoneLogStatusSuccess,
	SiPhoneLogStatusAborted,
	SiPhoneLogStatusMissed
} SiPhoneLogStatus;

typedef enum _LinphoneCallDir {
	SiPhoneLogStatusOutgoing, 
	SiPhoneLogStatusIncoming
} SiPhoneLogStatusDir;

typedef struct _silogs 
{
	int fdlog;
	SiPhoneLogStatusDir dir;
	SiPhoneLogStatus status;
	char *from;
	char *to;
	char start_date[128];
	int duration;
}silog_t;

 extern sicall_t SiCall[];
extern SICORE siphoneCore;
 
 
/*========================== Local function prototypes ======================*/
int si_ua_startup(ConfigSettings *pConfig); //SICONFIG *pConfig, SICORE* pCore);
void si_ua_reset(void) ;
void si_ua_getAddress4(unsigned char*);
void si_ua_getAddress6(unsigned char*);
int si_ua_init(SICORE* pCore); 
 
/*===========================================================================*/

void FatalError(int error, const char*);
/*===========================================================================*/

typedef struct __UAStatistics_t
{
	int rtpRecv;
	int rtpSnd;
	int rtpHaveMore;
	int rtpNoPkt;
	int rtpNumOfPoll;
	int alsaRecv;
	int alsaSnd;
 	int alsaNoPkt;
	int alsaNumOfPoll;
}UAStatistics_t;


#define SESSION_IDLE_STATE 		0
#define SESSION_NEW_CALL_STATE 		1
#define SESSION_RINGING_STATE 		2
#define SESSION_TALK_STATE 		3
#define SESSION_RELEASE_STATE 		4
#define SESSION_CONFERENCE_STATE	5

#define MAX_ALLOWED_SESSIONS 		6
typedef struct _DefSessionTable
{
 int   tid;
 char  status;
 char  line;
 char tmp1;
 char tmp2;
}DefSessionTable;


typedef struct _session_struct
{
	int index;
	int conferenceFlag; //0: single call 1: conference
	int numOfParticipants; //1: in single call 2 or 3 in conference
	int sessionState;  // 0:Idle, 1: Active, 2: On Hold, 3:Releasing

 	sicall_t *pCall[SI_MAX_ALLOWED_PARTICIPANTS];
	int participantState[SI_MAX_ALLOWED_PARTICIPANTS];


// 	unsigned short pktSequenceNumber[SI_MAX_ALLOWED_PARTICIPANTS];
//	unsigned short pktTimestamp[SI_MAX_ALLOWED_PARTICIPANTS];
	unsigned int keepalive[SI_MAX_ALLOWED_PARTICIPANTS];
	
	// thread releated parameters
       
} session_struct_t;

struct _siconference
{
	int ConferenceState;
	int NumOfParticipants;
	session_struct_t* pSession;
	sicall_t *pCall1;
	sicall_t *pCall2;
};
typedef struct _siconference siconference_t;



// fill the call structure for the incoming call
/*   
   for (i=0;i<MAX_ALLOWED_SESSIONS;i++)
   {
     if (SessionTable[i].status == SESSION_IDLE_STATE)
	{
	 SessionTable[i].tid = pEvent->tid;
         SessionTable[i].status = SESSION_NEW_CALL_STATE;	
	 break;
	}
  }
*/
extern char SipAllowHeader[];
int si_net_startup(unsigned char *IPAddress) ;
int initSIPparameters(SICORE* pCore);
int si_net_init(unsigned char *IPAddress) ;
int si_ua_readMACAddress(unsigned char *MACAddress);
 
int si_ua_start_ntpclient(void);
int si_ua_reInvite(sicall_t* pCall); 
#endif //SIPHONE_UA_INIT_H
