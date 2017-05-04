#ifndef CWMP_EVT_MSG_H
#define CWMP_EVT_MSG_H

#include "kernel_config.h"
#include "voip_feature.h"
#include "rtk_voip.h"
#include "mib_def.h"

//#define	MAX_PORTS			CON_CH_NUM 	///MAX channel number
#define	MAX_PORTS			3 	///MAX channel number

#define TEST_STRING_LEN					25		///< max string length


typedef enum{
    EVT_VOICEPROFILE_LINE_NONE,
	EVT_VOICEPROFILE_LINE_GET_STATUS,
	EVT_VOICEPROFILE_LINE_SET_STATUS,
	EVT_VOICEPROFILE_LINE_SET_DIAGNOSTICS
}cwmpEvt;

typedef enum{
	CALLER_NONE,
	CALLER_NO_DIALTONE,
	CALLER_OFFHOOK_RELEASE,
	CALLER_AFTERDIAL_RELEASE,
	CALLER_NO_ANSWER
}CallerFailReasonEvt;


typedef enum{
	CALLEE_NONE,
	CALLEE_NO_INCOMINGCALL,
	CALLEE_OFFHOOK_RELEASE,
	CALLEE_NO_ANSWER
}CalledFailReasonEvt;


#define IMS_SPECIAL_DIAL_TONE	(1<<0)
#define IMS_IMMEDIATE_CALLSETUP	(1<<1) // immediate hot line
#define IMS_MCID_ACTIVATE			(1<<2)
#define IMS_CONFERENCE_ACTIVATE	(1<<3)
#define IMS_CALL_WAITING_ACTIVATE	(1<<4)
#define IMS_DEFERE_CALLSETUP	(1<<5) //delay hot line
#define IMS_HOLD_SERVICE_ACTIVATE		(1<<6) // 0: support , 1:disable




typedef struct voiceProfileLineStatusObj{
	int profileID;
	int line;
	int incomingCallsReceived;
	int incomingCallsAnswered;
	int incomingCallsConnected;
	int incomingCallsFailed;
	int outgoingCallsAttempted;
	int outgoingCallsAnswered;
	int outgoingCallsConnected;
	int outgoingCallsFailed;
	int resetStatistics;
	unsigned int totalCallTime;
	unsigned long	featureFlags;
	int IADDiagnosticsState; //VoiceProfile.{i}.X_IADDiagnostics.IADDiagnosticsState ,0:None, 1:Requested, 2:Complete
	int TestServer;//VoiceProfile.{i}.X_IADDiagnostics.TestServer  ,0:None, 1:Requested, 2:Complete
	int RegistResult ;//VoiceProfile.{i}.X_IADDiagnostics.RegistResult  , register 0: ok , 1: fail
	int Reason;//VoiceProfile.{i}.X_IADDiagnostics.Reason  
	line_status_t LineStatus; //VoiceService.{i}.VoiceProfile.{i}.Line.{i}.Status
	

	/* omci add */

	int SIP_user_data_Alarm; /*9.9.2 SIP user data	, alarm number*/
	/*0: Cannot authenticate registration session (missing credentials)
	  1: SIPUA register timeout
	  2:SIPUA register fail*/
	int SIP_status; /*9.9.3 SIP agent config data-SIP status, current status of the SIP agent.*/
	/*
	0 Ok/initial
	1 Connected
	2 Failed ¡V ICMP error
	3 Failed ¡V Malformed response
	4 Failed ¡V Inadequate info response
	5 Failed ¡V Timeout
	6 Redundant, offline:
	*/
	int cid_features;	/*9.9.8 VoIP application service profile-CID features */
	/*
	bit map of caller ID features
	0x01 Calling number
	0x02 Calling name
	0x04 CID blocking (both number and name)
	0x08 CID number ¡V Permanent presentation status for number
	(0 = public, 1 = private)
	0x10 CID name ¡V Permanent presentation status for name
	(0 = public, 1 = private)
	0x20 Anonymous CID blocking (ACR). It may not be possible to support this in the ONU.
	*/

	int call_progress_features; /*9.9.8 VoIP application service profile-Call progress or transfer features*/
	/*
	0x0001 3way
	0x0002 Call transfer
	0x0004 Call hold
	0x0008 Call park
	0x0010 Do not disturb
	0x0020 Flash on emergency service call (flash is to be	processed during an emergency service call)
	0x0040 Emergency service originating hold (determines whether call clearing is to be performed on on-hook during an emergency service call)
	0x0080 6way
	*/

	int voip_codec_used; /*9.9.11 VoIP line status-Voip codec used, current codec used for a VoIP POTS port ,value define in voip_params.h RtpPayloadType, the same with omci*/  
	char codec_string[12];
	int voice_server_status; /*9.9.11 VoIP line status-Voip voice server status*/
	/*
	0 None/initial
	1 Registered
	2 In session
	*/
	int Voip_port_session_type; /*9.9.11 VoIP line status-Voip port session type,current state of fxs */
	/*
	0 Idle/none
	1 2way
	2 3way
	3 Fax
	4 Telem
	5 Conference
	*/

	int call_packet_period[2]; /*9.9.11 ,Voip call 1/2 packet period ,packet period for the  call (ms) */
	char call_dest_addr[2][16]; /* 9.9.11 , voip call dest addr: xxx.xxx.xxx.xxx */
	int call_setup_failures;/*9.9.12 ,counts call setup failures*/
	int call_setup_timer; /*9.9.12 , high water mark that records the longest duration of a single call setup */
	int call_terminate_failures; /*9.9.12,counts the number of calls that were terminated with cause */
	int analog_port_releases; /*9.9.12	analogue port releases without dialling detected (ms)*/
	int analog_port_offhook_timer; /*9.9.12 ,records the longest period of a single off-hook detected on the analogue port (ms) */
	uint32 rtp_errors;	/*9.9.13 counts RTP packet errors. */
	uint32 packet_loss; /*9.9.13 packets lost */
	int maximum_jitter;/*9.9.13 packets lost ,maximum jitter identified during the measured interval */ 
	int Maximum_time_rtcp; /*9.9.13 high water mark that represents the maximum time between RTCP packets during the measured interval*/
	int buffer_underflows; /* 9.9.13 , not support*/
	int buffer_overflows;/* 9.9.13 , not support*/
	int fail_connect_counter; /* 9.9.15 count fail to reach sip server */
	int fail_validate_counter;/* 9.9.15 SIP UA failed to validate its peer during SIP call initiations */
	int timeout_counter; /* 9.9.15 number of times the SIP UA timed out during SIP call initiations.*/
	int failure_received_counter; /* 9.9.15 number of times the SIP UA received a failure error code during SIP call initiations*/
	int fail_to_authenticate_counter; /* 9.9.15 SIP UA failed to authenticate itself during SIP call initiations */
	
}voiceProfileLineStatus;





//ericchung for E8C tr104 auto test
typedef struct _LinphoneTR104Test
{
	
//VoiceService.{i}.PhyInterface.{i}.Tests.X_SimulateTest.

	int enable; //0 : disable , 1: enable  
	int TestType;	//0: Caller  or 1: Callee   
	char callednumber[TEST_STRING_LEN]; //phonenumber
	int DailDTMFConfirmEnable; //need play dtmf 
	int dtmfindex; //play dtmf index
	char DailDTMFConfirmNumber[TEST_STRING_LEN]; //for send dtmf
	int receive_dtmf_index; //receive dtmf index
	char ReceiveDTMFNumber[TEST_STRING_LEN]; //for receive dtmf
	int DailDTMFConfirmResult; //dtmf compare result

	
	int status;		// 0:not start, need offhook , 1:send dtmf  , internal linphone usage.
	
	char Simulate_Status[TEST_STRING_LEN]; // idle,Off-hook,Dialtone ,Receiving,ReceiveEnd,Ringing-back,Connected,Testend
	int Conclusion;	//Simulate success or not. 0:success, 1: fail
	CallerFailReasonEvt CallerFailReason;
	CalledFailReasonEvt CalledFailReason;
	int FailedResponseCode;

	int TestStatus;//VoiceService.{i}.PhyInterface.{i}.Tests.TestState , 0:none, 1:Requested  , 2:Complete
	int TestSelector;////VoiceService.{i}.PhyInterface.{i}.Tests.TestSelector  0: PhoneConnectivityTest,1: X_SimulateTest
	int PhoneConnectivity; //VoiceService.{i}.PhyInterface.{i}.Tests.TestState , 0:non
	
} LinphoneTR104Test_t;




typedef struct cwmpEvtMsgObj{
	cwmpEvt event;
	voiceProfileLineStatus voiceProfileLineStatusMsg[MAX_PORTS];	
	LinphoneTR104Test_t e8c_autotest; /* currently only support fxs 1 */
}cwmpEvtMsg;

/*new the evt msg*/
cwmpEvtMsg *cwmpEvtMsgNew(void);

/* free the evt msg */
void cwmpEvtMsgFree(cwmpEvtMsg *msg);

/*set and get the msg event*/
void cwmpEvtMsgSetEvent(cwmpEvtMsg *msg, cwmpEvt event);
cwmpEvt cwmpEvtMsgGetEvent(cwmpEvtMsg *msg);

/* set and get the voiceProfileLineStatus */
void cwmpEvtMsgSetVPLineStatus(cwmpEvtMsg *msg, voiceProfileLineStatus *VPLineStatus, int nPort);
voiceProfileLineStatus *cwmpEvtMsgGetVPLineStatus(cwmpEvtMsg *msg, int nPort);

/* the size of the evt msg */
int cwmpEvtMsgSizeof(void);

#endif /*CWMP_EVT_MSG_H*/
