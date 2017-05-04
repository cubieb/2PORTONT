#include <sys/msg.h>
#include "kernel_config.h"
#include "rtk_voip.h"
#include "uglib.h"	/* g_error */
#include "uihandler.h"
#include "rcm_customize.h"

#define IP_CHID			0
#define IP_SID			0
#define IP_SID_TEMP		0

static inline int IpPhoneOwnChid( chid_t chid )
{
	return ( chid == IP_CHID );
}

/* *********************************************************
 * Main UI processor 
 * ********************************************************* */
#ifdef UI_EVENTS_USE_IPC_MODE
static int qidEventSip = -1;
static int qidEventUI = -1;

static unsigned char *pszExeCmd;
static sid_t *psidExeCmd;	/* sid of linphonec_parse_command_line() */

void InitializeUIEvent( void )
{
	key_t key;
	
	/* Sip -> UI event */
	key = ftok( UI_CTRL_PATHNAME, UI_CTRL_PRJ_ID_OWNER_UI );
	
	if( ( qidEventUI = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
		g_error( "Sip: Create UI event queue fail.\n" );
	}
	
	/* Sip <- UI event */
	key = ftok( UI_CTRL_PATHNAME, UI_CTRL_PRJ_ID_OWNER_SIP );
	
	if( ( qidEventSip = msgget( key, 0666 | IPC_CREAT ) ) == -1 ) {
		g_error( "Sip: Create Sip event queue fail.\n" );
	}
}

void U2S_EvnetExecutive( unsigned char *pszCmd, sid_t *psid )
{	
	tlv_event_super_t tlv_event_buf;
	int ret;

	if( qidEventSip == -1 ) {
		g_error( "Not qid for sip.\n" );
		return;
	}
	
	/* set variables for U2S_ functions. */
	/* These variables are parameters of linphonec_parse_command_line() */
	pszExeCmd = pszCmd;
	psidExeCmd = psid;	/* Fill SID according to command. */
	
	while( 1 ) 
	{
		/* recevice a message */
		ret = msgrcv( qidEventSip, &tlv_event_buf, sizeof( tlv_event_buf ) - sizeof( long ), 0, IPC_NOWAIT );
		
		if( ret == -1 )
			break;	/* no more message */
			
		switch( tlv_event_buf.header.type ) {		
		case UI_EVENT_TYPE_INCOMING_CALL_ACCEPT_REQ:	/* accept incoming call */
			U2S_IncomingCallAcceptReq( tlv_event_buf.accept.sid );
			break;
			
		case UI_EVENT_TYPE_INCOMING_CALL_REJECT_REQ:	/* reject incoming call */
			U2S_IncomingCallRejectReq();
			break;

		case UI_EVENT_TYPE_LINE_ALLOCATION_REQ:			/* allocate a line for call */
			U2S_LineAllocationReq( tlv_event_buf.line.sid, tlv_event_buf.line.connected );
			break;
			
		case UI_EVENT_TYPE_LINE_RELEASE_REQ:			/* release a line from outgoing call */
			U2S_LineReleaseReq();			
			break;
			
#if 0
		case UI_EVENT_TYPE_SWITCH_FXO_REQ:				/* switch a line to FXO outgong call */
			U2S_FxoLineSwitchReq( tlv_event_buf.fxo.sid, tlv_event_buf.fxo.enable, tlv_event_buf.fxo.connected );
			break;
#endif

		case UI_EVENT_TYPE_OUTGOING_CALL_REQ:			/* do outgoing call */
			U2S_OutgoingCallReq( tlv_event_buf.call.sid, tlv_event_buf.call.phonenumber, tlv_event_buf.call.connected, tlv_event_buf.call.transfer, tlv_event_buf.call.fxo );
			break;
			
		case UI_EVENT_TYPE_OUTGOING_CALL_CANCEL_REQ:	/* cancel outgoing call */
			U2S_OutgoingCallCancelReq( tlv_event_buf.cancel.sid, tlv_event_buf.cancel.connected );
			break;
		
		case UI_EVENT_TYPE_LINE_SWITCHING_REQ:			/* line switching: hold/resume/conference */
			U2S_LineSwitchingReq( tlv_event_buf.lsr.lsr, tlv_event_buf.lsr.transfer, tlv_event_buf.lsr.back );
			break;
		
		case UI_EVENT_TYPE_SEND_DTMF_REQ:				/* snd DTMF */
			U2S_SendDTMFReq( tlv_event_buf.dtmf.sid, tlv_event_buf.dtmf.dtmf );
			break;
			
		case UI_EVENT_TYPE_DISCONNECT_REQ:				/* disconnect req. */
			U2S_DisconnectReq();
			break;
			
		default:
			g_error( "(SIP)Unexpected event: %d\n", tlv_event_buf.header.type );
			break;
		}
		
		/* need parser to process */
		if( pszCmd[ 0 ] )
			break;
	}
}
#endif /* UI_EVENTS_USE_IPC_MODE */

/* ============================================================= */
/* S2U / U2S events */
void U2S_LineAllocationReq( sid_t sid, unsigned int bConnected )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_LINE_ALLOCATION_REQ */
	const unsigned int chid = IP_CHID;
	SYSTEMSTSTE sysState;
	
	sysState = GetSysState( chid );
	
	if( bConnected ) {
		SetActiveSession( chid , sid , TRUE );
		SetSysState( chid , SYS_STATE_CONNECT_EDIT );
	} else {
		if( sysState != SYS_STATE_IDLE )
			g_error( "Line allocation but not SYS_STATE_IDLE.\n" );
	
		SetActiveSession( chid , 0 , TRUE );
		SetSysState( chid , SYS_STATE_EDIT );
	}

	printf( "U2S_LineAllocationReq(%lu)\n", sid );
#else
#endif
}

void U2S_LineReleaseReq( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_LINE_RELEASE_REQ */
	const unsigned int chid = IP_CHID;
	int i;
	
	SetSysState( chid , SYS_STATE_IDLE ) ;
	
	for( i = 0 ; i < MAX_SS ; i++ ) {			
		rcm_tapi_SetRtpSessionState( chid , i , FALSE ) ;
		SetSessionState( chid , i , SS_STATE_IDLE ) ;
		SetActiveSession( chid , i , FALSE ) ;
	}

	/* copy command for executive */	
	strcpy( pszExeCmd, "release" );

	printf( "U2S_LineReleaseReq\n" );
#else
#endif
}

#if 0
void U2S_FxoLineSwitchReq( sid_t sid, unsigned int bEnable, unsigned int bConnected )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_SWITCH_FXO_REQ */
	
	/* copy command for executive */
	sprintf( pszExeCmd, "fxo %u", bEnable );
	
	//sprintf( psidExeCmd, "fxo %d %u", sid, bConnected );
	
	*psidExeCmd = sid;

	printf( "U2S_FxoLineSwitchReq\n" );
#else
#endif
}
#endif

void S2U_IncomingCallInd( chid_t chid, sid_t sid, unsigned int reason, const unsigned char *pszPhonenumber )
{
	/* reasons below 3 have been assigned. */
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_INCOMING_CALL_IND */
	tlv_event_phone_call_t tlv_event;
	//const unsigned char *pszPhonenumber = "5217777";
	
	if( !IpPhoneOwnChid( chid ) )
		return;
	
	tlv_event.type = UI_EVENT_TYPE_INCOMING_CALL_IND;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_phone_call_t );
	tlv_event.sid = sid;
	tlv_event.fxo = 0;	/* UNUSED!! */
	strcpy( tlv_event.phonenumber, pszPhonenumber );
	
	if( msgsnd( qidEventUI, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		g_error( "Fail to send incoming call ind.\n" );
		return;
	}
	
	printf( "S2U_IncomingCallInd(%lu):%d\n", sid, reason );
#else
#endif
}

void U2S_IncomingCallAcceptReq( sid_t sid )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_INCOMING_CALL_ACCEPT_REQ */

	/* copy command for executive */
	strcpy( pszExeCmd, "accept" );
	
	*psidExeCmd = sid;

	printf( "U2S_IncomingCallAcceptReq\n" );
#else
#endif	
}

void U2S_IncomingCallRejectReq( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_INCOMING_CALL_REJECT_REQ */

	/* copy command for executive */
	strcpy( pszExeCmd, "reject" );
			
	printf( "U2S_IncomingCallRejectReq\n" );
#else
#endif
}

void U2S_OutgoingCallReq( sid_t sid, const unsigned char *pszPhonenumber, 
						  unsigned long bConnected, unsigned int bTransfer,
						  unsigned int bFxo )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_OUTGOING_CALL_REQ */
	const unsigned int chid = IP_CHID;
	//const unsigned int sid = IP_SID;
	
	/* FXO is a special case */
	if( bFxo ) {
		if( bTransfer )
			strcpy( pszExeCmd, "transferfxo " );
		else
			strcpy( pszExeCmd, "callfxo " );
		
		goto label_fill_command_field_done;
	}
	
	/* Normal case */
	// FIXME: use a simple rule to catenate 
	if( strstr( pszPhonenumber, "." ) == NULL ) {
		/* use proxy call */
		if( bTransfer )
			strcpy( pszExeCmd, "transfer " );
		else 
			strcpy( pszExeCmd, "call " );
	} else {
		/* use direct ip call */
		if( bTransfer )
			strcpy( pszExeCmd, "transfer sip:" );
		else
			strcpy( pszExeCmd, "call sip:" );
	}

label_fill_command_field_done:	
	/* set sys state if necessary */
	if( !bTransfer ) {	/* first call only */
		if( GetSysState( chid ) == SYS_STATE_IDLE ) {
			SetActiveSession( chid , 0 , TRUE );
			SetSysState( chid , SYS_STATE_EDIT );
		}
	}
	
	/* copy command for executive */
	strcat( pszExeCmd, pszPhonenumber );
	*psidExeCmd = sid;
	
	SetSessionState( chid, sid, SS_STATE_CALLOUT );

	if( bTransfer )
		printf( "Transfer call(%lu):%s\n", sid, pszPhonenumber );
	else
		printf( "Outgoing call(%lu):%s\n", sid, pszPhonenumber );
#else
#endif
}

void U2S_OutgoingCallCancelReq( sid_t sid, unsigned int bConnected )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_OUTGOING_CALL_CANCEL_REQ */
	const unsigned int chid = IP_CHID;
	int i;
	
	if( !bConnected ) {
		/* cancel 1st outgoing call */
		SetSysState( chid , SYS_STATE_IDLE ) ;
		
		for( i = 0 ; i < MAX_SS ; i++ ) {
			SetSessionState( chid , i , SS_STATE_IDLE ) ;
			SetActiveSession( chid , i , FALSE ) ;
		}
		
		/* copy command for executive */
		strcpy( pszExeCmd, "cancel" );
	} else {
		// TODO: cancel 2nd outgoing call ?? Use LineSwitch 
		
	}
			
	printf( "U2S_OutgoingCallCancelReq\n" );
#else
#endif
}

void S2U_ConnectionEstInd( chid_t chid, sid_t sid, unsigned int reason )
{
	/* reasons below 14 have been assigned. */
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_CONNECTION_EST_IND */
	tlv_event_sid_t tlv_event;
	
	if( !IpPhoneOwnChid( chid ) )
		return;
	
	tlv_event.type = UI_EVENT_TYPE_CONNECTION_EST_IND;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_sid_t );
	tlv_event.sid = sid;
	tlv_event.value = 0;	/* ignore */
	
	if( msgsnd( qidEventUI, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		g_error( "Fail to send connectin est ind.\n" );
		return;
	}
	
	printf( "S2U_ConnectionEstInd(%lu):%d\n", sid, reason );
#else
#endif
}

void U2S_LineSwitchingReq( lsr_t lsr, unsigned int transfer, unsigned int back )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_LINE_SWITCHING_REQ */
	//const unsigned int chid = IP_CHID;
	
	// TODO: I assume that UI check it possible to do conference call. 
	
	/* copy command for executive */
	if( back ) {
		sprintf( pszExeCmd, "back %u", lsr );		
	} else {
		sprintf( pszExeCmd, "switch %u %u", lsr, transfer );
	}
			
	printf( "U2S_LineSwitchingReq(%u)\n", lsr );
#else
#endif
}

#if 0
void U2S_ConferenceCallReq( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_CONN_CONFERENCE_REQ */
	
	// TODO: I assume that UI check it possible to do conference call. 
	/* copy command for executive */
	strcpy( pszExeCmd, "conference" );
			
	printf( "U2S_ConferenceCallReq\n" );
#else
#endif
}

void U2S_CallWaitingAcceptReq( sid_t sid )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_CALL_WAITING_ACCEPT_REQ */
	/* copy command for executive */
	sprintf( pszExeCmd, "accept %lu", sid );
			
	printf( "U2S_CallWaitingAcceptReq(%lu)\n", sid );
#else
#endif
}

void U2S_SessionHoldReq( sid_t sid )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_HOLD_REQ */
	
#else
#endif
}
#endif

void S2U_SessionHeldInd( chid_t chid, sid_t sid, unsigned int reason )
{
	/* reasons below 4 have been assigned. */	
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_HELD_IND */
	tlv_event_sid_t tlv_event;
	
	if( !IpPhoneOwnChid( chid ) )
		return;
	
	tlv_event.type = UI_EVENT_TYPE_HELD_IND;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_sid_t );
	tlv_event.sid = sid;
	tlv_event.value = 0;	/* ignore */
	
	if( msgsnd( qidEventUI, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		g_error( "Fail to send session held ind.\n" );
		return;
	}
	
	printf( "S2U_SessionHeldInd(%lu):%d\n", sid, reason );
#else
#endif
}

void S2U_SessionResumeInd( chid_t chid, sid_t sid, unsigned int reason )
{
	/* reasons below 5 have been assigned. */	
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_RESUME_IND */
	tlv_event_sid_t tlv_event;
	
	if( !IpPhoneOwnChid( chid ) )
		return;
		
	tlv_event.type = UI_EVENT_TYPE_RESUME_IND;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_sid_t );
	tlv_event.sid = sid;
	tlv_event.value = 0;	/* ignore */
	
	if( msgsnd( qidEventUI, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		g_error( "Fail to send session held ind.\n" );
		return;
	}
	
	printf( "S2U_SessionResumeInd(%lu):%d\n", sid, reason );
#else
#endif
}

void U2S_SendDTMFReq( sid_t sid, unsigned char dtmf )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_SEND_DTMF_REQ */
	
	/* copy command for executive */
#if 1
	sprintf( pszExeCmd, "dtmf %c", dtmf );
#else
	pszExeCmd[ 0 ] = dtmf;
	pszExeCmd[ 1 ] = '\x0';
#endif

	*psidExeCmd = sid;

	printf( "DTMF(%lu):%c\n", sid, dtmf );
#else
#endif
}

void U2S_DisconnectReq( void )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_DISCONNECT_REQ */

	/* copy command for executive */
	strcpy( pszExeCmd, "disconnect" );
			
	printf( "U2S_DisconnectReq\n" );
#else
#endif
}

void S2U_DisconnectInd( chid_t chid, sid_t sid, unsigned int reason )
{
	/* reasons below 27 have been assigned. */	
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_DISCONNECT_IND */
	tlv_event_disc_ind_t tlv_event;
	
	if( !IpPhoneOwnChid( chid ) )
		return;
	
	tlv_event.type = UI_EVENT_TYPE_DISCONNECT_IND;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_disc_ind_t );
	tlv_event.sid = sid;
	
	if( msgsnd( qidEventUI, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		g_error( "Fail to send disconnect ind.\n" );
		return;
	}
	
	printf( "S2U_DisconnectInd(%lu):%d\n", sid, reason );
#else
#endif
}

void S2U_SolarRestartInd( unsigned int reason )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_SOLAR_RESTART_IND */
	tlv_event_solar_restart_t tlv_event;
	
	tlv_event.type = UI_EVENT_TYPE_SOLAR_RESTART_IND;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_solar_restart_t );
	tlv_event.reason = reason;
	
	if( msgsnd( qidEventUI, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		g_error( "Fail to send soalr restart ind.\n" );
		return;
	}
	
	printf( "S2U_SolarRestartInd:%d\n", reason );
#else
#endif
}

void S2U_SipRegisterInd( chid_t chid, register_status_t status )
{
#ifdef UI_EVENTS_USE_IPC_MODE
	/* UI_EVENT_TYPE_SIP_REGISTER_IND */
	tlv_event_sip_register_t tlv_event;
	
	if( !IpPhoneOwnChid( chid ) )
		return;
	
	tlv_event.type = UI_EVENT_TYPE_SIP_REGISTER_IND;
	tlv_event.len = TLV_EVENT_PAYLOAD_SIZE( tlv_event_sip_register_t );
	tlv_event.status = status;
	
	if( msgsnd( qidEventUI, &tlv_event, tlv_event.len + sizeof( tlv_event.len ), 0 ) == -1 ) {
		//g_error( "Fail to send sip register ind.\n" );
		// In case of SIP proxy is filled, sip send this event is very early. 
		// At that time, ui doesn't start yet. 
		g_warning( "Fail to send sip register ind.\n" );
		return;
	}
	
	printf( "S2U_SipRegisterInd:%d\n", status );
#else
#endif
}

/*
 * A happy day scenario of outgoing call: 
 *  1) call out
 *  2) callee off-hook --> S2U_ConnectionEstInd( 0 )
 *  3) callee on-hook  --> S2U_DisconnectInd( 4 )
 */

/*
 * A happy day scenario of incoming call: 
 *  1) incoming call  --> S2U_IncomingCallInd( 0 )
 *  2) callee off-hook to accept
 *  3) connection ind --> S2U_ConnectionEstInd( 2 )
 *  4) caller on-hook --> S2U_DisconnectInd( 4 )
 */

/*
 * Reject incoming call 
 *  1) incoming call       --> S2U_IncomingCallInd( 0 )
 *  2) press 'c' to reject --> wait for disconnect ind.
 *  3) disconnect-ind      --> S2U_DisconnectInd( 3 )
 */

