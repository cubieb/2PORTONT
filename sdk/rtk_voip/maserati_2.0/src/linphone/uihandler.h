#ifndef __UI_HANDLER_H__
#define __UI_HANDLER_H__

#include "linphonecore.h"	/* SESSIONSTATE */

#include "../../ui/include/ui_event_def.h"

typedef unsigned long	chid_t;

#define IP_TEMP_CHID	0
#define IP_TEMP_SID		0

#ifdef UI_EVENTS_USE_IPC_MODE
extern void InitializeUIEvent( void );
extern void U2S_EvnetExecutive( unsigned char *pszCmd, sid_t *psid );
#endif /* UI_EVENTS_USE_IPC_MODE */

/* ============================================================= */
/* macro */
static inline int ChidIsOwnedByIpPhone( chid_t chid )
{
	return ( chid == IP_TEMP_CHID );
}

/* ============================================================= */
/* Command queue for linphone */
#if 0
typedef struct {
	unsigned int bChangeState;
	SESSIONSTATE ss_state;
	unsigned char command[ LINE_MAX_LEN ];
} command_queue_t;

extern int U2S_CopyInputFromCommandQueueAndSetState( unsigned int chid, 
											  unsigned int sid, 
											  unsigned char *pszCmd );
#endif

/* ============================================================= */
/* S2U / U2S events */

extern void S2U_IncomingCallInd( chid_t chid, sid_t sid, unsigned int reason, const unsigned char *pszPhonenumber );
/* UI_EVENT_TYPE_INCOMING_CALL_IND */

extern void U2S_IncomingCallAcceptReq( sid_t sid );
/* UI_EVENT_TYPE_INCOMING_CALL_ACCEPT_REQ */

extern void U2S_IncomingCallRejectReq( void );
/* UI_EVENT_TYPE_INCOMING_CALL_REJECT_REQ */

extern void U2S_LineAllocationReq( sid_t sid, unsigned int bConnected );
/* UI_EVENT_TYPE_LINE_ALLOCATION_REQ */

extern void U2S_LineReleaseReq( void );
/* UI_EVENT_TYPE_LINE_RELEASE_REQ */

//extern void U2S_FxoLineSwitchReq( sid_t sid, unsigned int bEnable, unsigned int bConnected );
/* UI_EVENT_TYPE_SWITCH_FXO_REQ */
	
extern void U2S_OutgoingCallReq( sid_t sid, const unsigned char *pszPhonenumber, unsigned long bConnected, unsigned int bTransfer, unsigned int bFxo );
/* UI_EVENT_TYPE_OUTGOING_CALL_REQ */

extern void U2S_OutgoingCallCancelReq( sid_t sid, unsigned int bConnected );
/* UI_EVENT_TYPE_OUTGOING_CALL_CANCEL_REQ */

extern void S2U_ConnectionEstInd( chid_t chid, sid_t sid, unsigned int reason );
/* UI_EVENT_TYPE_CONNECTION_EST_IND */

extern void U2S_LineSwitchingReq( lsr_t lsr, unsigned int transfer, unsigned int back );
/* UI_EVENT_TYPE_LINE_SWITCHING_REQ */

#if 0
extern void U2S_ConferenceCallReq( void );
/* UI_EVENT_TYPE_CONN_CONFERENCE_REQ */

extern void U2S_CallWaitingAcceptReq( sid_t sid );
/* UI_EVENT_TYPE_CALL_WAITING_ACCEPT_REQ */

extern void U2S_SessionHoldReq( sid_t sid );
/* UI_EVENT_TYPE_HOLD_REQ */
#endif

extern void S2U_SessionHeldInd( chid_t chid, sid_t sid, unsigned int reason );
/* UI_EVENT_TYPE_HELD_IND */

extern void S2U_SessionResumeInd( chid_t chid, sid_t sid, unsigned int reason );
/* UI_EVENT_TYPE_RESUME_IND */

extern void U2S_SendDTMFReq( sid_t sid, unsigned char dtmf );
/* UI_EVENT_TYPE_SEND_DTMF_REQ */

extern void U2S_DisconnectReq( void );
/* UI_EVENT_TYPE_DISCONNECT_REQ */

extern void S2U_DisconnectInd( chid_t chid, sid_t sid, unsigned int reason );
/* UI_EVENT_TYPE_DISCONNECT_IND */

extern void S2U_SolarRestartInd( unsigned int reason );
/* UI_EVENT_TYPE_SOLAR_RESTART_IND */

extern void S2U_SipRegisterInd( chid_t chid, register_status_t status );
/* UI_EVENT_TYPE_SIP_REGISTER_IND */

#endif /* __UI_HANDLER_H__ */

