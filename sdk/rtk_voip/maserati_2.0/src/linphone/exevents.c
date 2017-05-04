/*
linphone
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <ctype.h>
#include "eXosip2/eXosip.h"
#include "eXosip2.h"
#include "linphonecore.h"
#include "misc.h"
#include "uglib.h"
#include "payloadtype.h"
#include "exevents.h"
#include "alarm.h"
#ifdef CONFIG_RTK_VOIP_TR104
#include <sys/un.h>
#include "cwmpevt.h"
#endif /*CONFIG_RTK_VOIP_TR104*/

#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "uihandler.h"
#endif
#include "rcm_customize.h"
#define CALLER_ID_NAME_LENGTH 51

extern void linphone_register_authentication_required(LinphoneCore *lc,eXosip_event_t *ev);
extern void linphone_invite_authentication_required(LinphoneCore *lc, int sid);
extern int linphone_call_authentication_required(LinphoneCore *lc, LinphoneCall *lcall, eXosip_event_t *ev);
extern void linphone_subscription_new(LinphoneCore *lc,int did, int sid, char *from, char* contact);
extern void linphone_notify_recv(LinphoneCore *lc,char *from,int ss_status);
extern LinphoneProxyConfig *linphone_core_get_proxy_config_from_rid(LinphoneCore *lc, int rid);
void linphone_subscription_answered(LinphoneCore *lc,char *from, int sid);
void linphone_subscription_closed(LinphoneCore *lc,char *from, int did);
//[20110805, SD6, ericchung, rcm integration
void rcm_linphone_process_sipinfo(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev);
void rcm_linphone_process_refer(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev);
void rcm_process_other_request(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev);
int GetChIDfromOptionsTid(TAPP *pApp, eXosip_event_t *ev);
void rcm_linphone_notify_rec(LinphoneCore *lc, guint32 ssid,eXosip_event_t *ev);


//]

/* these are useful messages for the status */
extern gchar ready[] ;
extern gchar end[] ;
extern gchar contacting[] ;
extern gchar connected[] ;
extern gchar cancel[] ;
extern gchar contacted[] ;

//[SD6, bohungwu, exosip 3.5 integration 
extern eXosip_t eXosip;

#ifdef CONFIG_RTK_VOIP_TR104 //for e8c and tr104 status
extern cwmpEvtMsg rcm_tr104_st; 
#endif


//]

//#define VMWI_TEST	/* thlin+ for test VMWI */

//======================== Callee side CID only =========================
char caller_id[21];	// hc+ 1130 for get caller ID from SIP message header
char caller_id_name[CALLER_ID_NAME_LENGTH];	// display the name of caller id.
char fsk_dsil_chk[4]={0};	// hc+ 1215 for FSK CID
char caller_id_mode[4]={0};	// CID mode, 1:FSK, 0:DTMF
char call_waiting_cid[4]={0, 0, 0, 0};// call waiting caller ID gen, 1: enable 0: disable
//===================================================================================

#if 1 /* CLIP */
typedef struct caller_id_map {
	const char *original; 
	const char *modify  ;
	const char *txt_log ;
} caller_id_map_t ;
/*for customer , need check ISP provider */
caller_id_map_t caller_id_map_list[] =  /* smaller index, higher priority */
{ /* original , modify , txt_log */
	{ "+49"  , "0"  , "Caller ID Replace" },
	{ "49"   , "0"  , "Caller ID Replace" },
	{ "+"    , "00" , "Caller ID Replace" },
};

static int 
replace_caller_id( char *cid )
{
	int i = 0;
	char *loc = NULL;
	char tmp_caller_id[21] = {0};

	caller_id_map_t *cid_map = &caller_id_map_list[0];
	const int cid_map_len = sizeof(caller_id_map_list) / sizeof(caller_id_map_t);
	
	for( i = 0 ; i < cid_map_len ; i++ ){
		loc = strstr( cid , cid_map[i].original );
		if(( loc != NULL ) && ( loc == (char *)caller_id )) {
			g_message( "before replace cid caller_id = %s\n" , caller_id );
			//printf("%s(%d) before replace caller_id = %s %p loc = %s %p\n" ,  
			//	__FUNCTION__ , __LINE__ , caller_id , caller_id , loc , loc);

			strncpy( tmp_caller_id , cid_map[i].modify , strlen( cid_map[i].modify ));
			cid += strlen( cid_map[i].original );
			strncat( tmp_caller_id , cid , strlen( cid ));
			strncpy( caller_id , tmp_caller_id , strlen( tmp_caller_id ));
			caller_id[ strlen(cid_map[i].modify)+strlen(cid) ] = '\0';
			
			g_message( "after replace cid caller_id = %s\n" , caller_id );
			//printf("%s(%d) after replace caller_id = %s\n" ,  
			//	__FUNCTION__ , __LINE__ , caller_id);

			break;
		}
	}

	return 0;
}
#endif

//[SD6, ericchung, exosip 3.5 integration
int rcm_exosip_call_ack(LinphoneCore *lc, uint32 ssid,eXosip_event_t *ev)
{

	osip_message_t *msg=NULL;
	
	g_message("rcm_exosip_call_ack , lc->chid %d\n",lc->chid);
	if (lc->call[ssid] == NULL)
	{
		g_warning("Call null[%d][%d]\n", lc->chid, ssid);
		g_warning("rcm_exosip_call_ack Call null[%d][%d]\n", lc->chid, ssid);
	
		return -1;
	}
	
	
	

	//eric: build ack for 200ok
	eXosip_call_build_ack(ev->did,&msg);
	if(lc->call[ssid]->proxy!=NULL){
		g_message("lc->call[ssid]->proxy contact is %s\n",lc->call[ssid]->proxy->contact);
	_osip_list_set_empty(&msg->contacts,(void (*)(void*))osip_contact_free);
	osip_message_set_contact(msg,lc->call[ssid]->proxy->contact);
	}

	eXosip_call_send_ack(ev->did,msg);

	return 0;
}


//make out-going call , receive peer 200ok , run call_accepted.
int linphone_call_accepted(LinphoneCore *lc, uint32 ssid, eXosip_event_t *ev)
{
//	StreamParams *audio_params;
	sdp_message_t *sdp_msg;
	LinphoneCall *call=lc->call[ssid];
	uint32 chid=lc->chid;
	int another_ssid=0;

	g_message(" linphone_call_accepted \n");
	if( call == NULL )
	{
		g_warning( "No call to accept.\n" ) ;
		return 0 ;
	}

//	audio_params = &call->audio_params ;
	call->auth_failed = FALSE ;

	another_ssid=(ssid == 0 ? 1 : 0);

	sdp_msg=eXosip_get_sdp_info(ev->response);

	if( sdp_msg==NULL)
	{
		g_message( "No sdp body in 200ok.\n" ) ;
		/* merge from claro:some sip server use 183 with sdp to set rtp , so 200ok no sdp . */
		//return 0 ;
	}else{
		sdp_message_free(sdp_msg);

	}
	g_message("\r\n call->state %d \n",call->state);

	switch (call->state)
	{
	case LCStateHold:
		// receive hold response
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		S2U_SessionHeldInd( chid, ssid, 0 /* hold */ );
#endif
		//call->state = LCStateHoldACK;
		SetCallState(  chid , ssid ,&call->state, LCStateHoldACK) ;

		if((lc->call[another_ssid])&&(lc->call[another_ssid]->state==LCStateBeResume)){
			g_message(" linphone_call_accepted another ssid %d call-state  is %d\n",another_ssid,lc->call[another_ssid]->state);

			if (linphone_core_offhood_event(lc, another_ssid) != 0) {
				// offhood failed, user need try again
				g_message("\r\n offhold ssid %d fail\n",another_ssid);
			}
		}
		SetActiveSession(chid, ssid == 0 ? 1 : 0, TRUE);
//		rcm_tapi_SetRtpSessionState(chid, ssid, rtp_session_sendonly);
		linphone_core_stop_media_streams(lc, ssid);
		if (GetSessionState(lc->chid, ssid == 0 ? 1 : 0) == SS_STATE_IDLE){
			//add check country
			if(g_pVoIPCfg->tone_of_country==TONE_GERMANY)
				rcm_tapi_SetPlayTone(lc->chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_STUTTERDIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
			else
			rcm_tapi_SetPlayTone(lc->chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

		}

		break;
	case LCStateResume:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		S2U_SessionResumeInd( chid, ssid, 0 );
#endif
		//call->state = LCStateAVRunning;
		SetCallState(  chid , ssid ,&call->state, LCStateAVRunning);
		g_message("\r\nlinphone linphone_call_accepted chid[%d],ssid[%d], status %d\n",chid,ssid,call->state );
		SetActiveSession(chid, ssid, TRUE);
//		rcm_tapi_SetRtpSessionState(chid, ssid, rtp_session_sendrecv);
		rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);

		/*merge from claro: some sip server un-held 200 ok with sdp , need read again */
		rcm_sdp_context_read_answer_by_ev(lc->call[ssid]->sdpctx, ev);
		linphone_core_stop_media_streams(lc, ssid);

	#ifdef CHECK_RESOURCE
		if (linphone_core_start_media_streams(lc, ssid, call) == -1)
		{
			linphone_core_terminate_dialog(lc, ssid, NULL);
			return -1;
		}
	#else
		linphone_core_start_media_streams(lc, ssid, call);
	#endif
		if (GetSysState(chid) == SYS_STATE_IN3WAY)
		{
			int i;
			TstVoipMgr3WayCfg stVoipMgr3WayCfg;

			memset(&stVoipMgr3WayCfg, 0, sizeof(stVoipMgr3WayCfg));
			stVoipMgr3WayCfg.ch_id = chid;
			stVoipMgr3WayCfg.enable = TRUE;
			for (i=0; i<MAX_SS; i++)
			{
				GetRtpCfg(&stVoipMgr3WayCfg.rtp_cfg[i], lc, i);
			}
			rcm_tapi_SetConference(&stVoipMgr3WayCfg);
		}
		break;
	case LCStateHoldACK:
		// shouldn't occur
		printf("receive OK on HoldACK!?\n");
		break;
	case LCStateAVRunning:
		// shouldn't occur
		printf("receive OK on AVRnning...\n");
		if(ev->type!=EXOSIP_CALL_RINGING){
			SetSessionState( chid , ssid , SS_STATE_CALLER ) ;
		}		
		rcm_sdp_context_read_answer_by_ev(lc->call[ssid]->sdpctx, ev);
		linphone_core_stop_media_streams(lc, ssid);
	#ifdef CHECK_RESOURCE
		if (linphone_core_start_media_streams(lc, ssid, call) == -1)
		{
			linphone_core_terminate_dialog(lc, ssid, NULL);
			return -1;
		}
	#else
		linphone_core_start_media_streams(lc, ssid, call);
	#endif


/*bugfix:
when make outgoing call to B , 
and receive incoming call from C can't run 3way conf.
*/
			g_message("get status is %d\n",GetSysState(chid));
			switch (GetSysState(chid))
			{
			case SYS_STATE_CONNECT_EDIT:
				if(ev->type!=EXOSIP_CALL_RINGING){
					SetSysState(chid, SYS_STATE_IN2CALLS_OUT);
				}
				break;
#ifdef ATTENDED_TRANSFER
			case SYS_STATE_TRANSFER_EDIT:
#ifdef PLANET_TRANSFER
			case SYS_STATE_TRANSFER_ATTENDED:  //go through
#endif /*PLANET_TRANSFER*/

				SetSysState(chid, SYS_STATE_TRANSFER_CONNECT);
				break;
#endif
			default:

				SetSysState(chid, SYS_STATE_CONNECT);
				break;
			}


	
		break;
	case LCStateInit: // if no ringing
	case LCStateRinging:

		g_message("\r\n case LCStateInit or LCStateRinging \n");

		SetActiveSession( chid , ssid , TRUE ) ;
		rcm_sdp_context_read_answer_by_ev(lc->call[ssid]->sdpctx, ev);
		if(ev->type!=EXOSIP_CALL_RINGING){
		linphonec_display_something(lc, "Connected.\n");
		}

		if( lc->ringstream[ssid] )
		{
			lc->ringstream[ssid] = FALSE ;
			rcm_tapi_SetRingFXS( chid , FALSE ) ;
			rcm_tapi_SetPlayTone( chid , ssid , DSPCODEC_TONE_RINGING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
		}

		g_message("get status is %d\n",GetSysState(chid));
		switch (GetSysState(chid))
		{
		case SYS_STATE_CONNECT_EDIT:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
			S2U_ConnectionEstInd( chid, ssid, 11 );
#endif
			if(ev->type!=EXOSIP_CALL_RINGING){
			SetSysState(chid, SYS_STATE_IN2CALLS_OUT);
			}
			break;
#ifdef ATTENDED_TRANSFER
		case SYS_STATE_TRANSFER_EDIT:
#ifdef PLANET_TRANSFER
		case SYS_STATE_TRANSFER_ATTENDED:  //go through
#endif /*PLANET_TRANSFER*/
#if 0
			SetSysState(chid, SYS_STATE_IN2CALLS_OUT);
#else
			SetSysState(chid, SYS_STATE_TRANSFER_CONNECT);
#endif
			break;
#endif
		default:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
			S2U_ConnectionEstInd( chid, ssid, 0 );
#endif
			SetSysState(chid, SYS_STATE_CONNECT);
			break;
		}

		//if (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
		if( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature ) )
		{
			if (g_pVoIPCfg->ports[chid].hotline_enable &&
				g_pVoIPCfg->ports[chid].hotline_number[0] == 0)
			{
				// rcm_tapi_FXO_offhook has done
			}
			else
			{
				rcm_tapi_FXO_offhook(chid);
			}
		}
		if(ev->type!=EXOSIP_CALL_RINGING){
		SetSessionState( chid , ssid , SS_STATE_CALLER ) ;
		}
		linphone_core_stop_media_streams(lc, ssid);
	#ifdef CHECK_RESOURCE
		if (linphone_core_start_media_streams(lc, ssid, call) == -1)
		{
			linphone_core_terminate_dialog(lc, ssid, NULL);
			return -1;
		}
	#else
		linphone_core_start_media_streams(lc, ssid, call);
	#endif
		break;
	case LCStateFax: // fax re-invite
		rcm_sdp_context_read_answer_by_ev(lc->call[ssid]->sdpctx, ev);
		linphone_core_stop_media_streams(lc, ssid);
	#ifdef CHECK_RESOURCE
		if (linphone_core_start_media_streams(lc, ssid, call) == -1)
		{
			linphone_core_terminate_dialog(lc, ssid, NULL);
			return -1;
		}
	#else
		linphone_core_start_media_streams(lc, ssid, call);
	#endif
		break;
	case LCStateUpdate: // update
		rcm_sdp_context_read_answer_by_ev(lc->call[ssid]->sdpctx, ev);
	#ifdef CHECK_RESOURCE
		if (linphone_core_start_media_streams(lc, ssid, call) == -1)
		{
			linphone_core_terminate_dialog(lc, ssid, NULL);
			return -1;
		}
	#else
		linphone_core_start_media_streams(lc, ssid, call);
	#endif
		break;
	default:
		// do what?
		break;
	}

	return 0 ;
}

int linphone_call_terminated(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev)
{
	int err;
	lc->ringstream[ssid] = FALSE ;

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	S2U_DisconnectInd( lc ->chid, ssid, 4 );
#endif

	// avoid t38 not close problem
	if (lc->call[ssid]->faxflag == FMS_FAX_RUN &&
		lc->call[ssid]->t38_params.initialized)
	{
		int i;
		int fax_flag;

		for (i=0; i<10; i++)
		{
			rtk_GetFaxEndDetect(lc->chid, &fax_flag);
			if (fax_flag)
			{
				// Flush FIFO
				rtk_GetFaxModemEvent(lc->chid, &fax_flag, NULL, 1);
				lc->call[ssid]->faxflag = FMS_IDLE;
				printf("SIP go FAX_IDLE in call close\n");
				break;
			}
			usleep(100000); // 100ms
		}
	}

	//[SD6, ericchung, exosip 3.5 integration
	eXosip_lock();
	err=eXosip_call_terminate(ev->cid,ev->did);
	eXosip_unlock();
	//]
	linphone_core_stop_media_streams(lc, ssid);
	linphonec_display_something(lc, end);
	linphone_call_destroy(lc->call[ssid]);
	lc->call[ssid] = NULL ;

	return 0 ;
}


int linphone_call_released(LinphoneCore *lc, guint32 ssid, int cid)
{
	LinphoneCall * call = lc->call[ssid];

	if( call != NULL && call->cid == cid )
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		S2U_DisconnectInd( lc ->chid, ssid, 6 );
#endif

		linphone_call_destroy(lc->call[ssid]);
		lc->call[ssid] = NULL;
		linphonec_display_warning(lc, _("Could not reach destination.\n"));
#if 1
        // rock: transport error
        rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		SetSessionState(lc->chid, ssid, SS_STATE_IDLE);
		switch (GetSysState(lc->chid))
		{
		case SYS_STATE_EDIT:
		case SYS_STATE_CONNECT_EDIT:
		case SYS_STATE_TRANSFER_EDIT:
#ifdef PLANET_TRANSFER
		case SYS_STATE_TRANSFER_ATTENDED:
#endif /*PLANET_TRANSFER*/
			SetSysState(lc->chid, GetSysState(lc->chid) + 1);
			break;
		default:
			// do nothing
			break;
		}
#endif
	}

	return 0 ;
}

int linphone_call_failure(LinphoneCore *lc, guint32 ssid, int code, eXosip_event_t *ev)
{
	const char * reason ;
	char * msg486 = _( "User is busy." ) ;
	char * msg480 = _( "User is temporarily unavailable." ) ;
	char * msg487 = _( "Request Cancelled." ) ;
	/*++added by Jack Chan 13/02/07 for T.38++*/
	char * msg488=_( "Not Acceptable Here" );
	/*--end--*/
	char * msg600 = _( "User does not want to be disturbed." ) ;
	char * msg603 = _( "Call declined." ) ;
	char * tmpmsg = msg486 ;
	int bTransferring, hs; // hs: hold session

	hs = 0;
	bTransferring = 0;
	g_message("linphone_call_failure code is %d,ssid %d\n",code,ssid);
	if (GetSysState(lc->chid) == SYS_STATE_TRANSFERRING)
	{
		hs = (ssid == 0) ? 1 : 0;
		if (lc->call[hs] && lc->call[hs]->state != LCStateInit)
			bTransferring = 1;
	}

	switch( code )
	{
		case 422: /* add 422 session timer to small support */
			linphonec_display_something(lc, _("422 session to small,retry"));
			return 0; //exosip will retry it.
			break;
		case 401 :
		case 407 :
			if (linphone_call_authentication_required(lc, lc->call[ssid],ev) == 0)
				return 0; // call retry
			break ;
		case 400 :
			linphonec_display_something(lc, _("Bad request"));
			if (bTransferring)
				//[SD6, bohungwu, exosip 3.5 integration
				//eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 400 Bad Request");
				rcm_eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 400 Bad Request");
				//]	
			break ;
		case 404 :
			linphonec_display_something(lc, _("User cannot be found at given address.\n"));

#if defined (CONFIG_RTK_VOIP_TR104) && defined (CONFIG_E8B)

						/* E8C TEST Fail*/
			
						if(rcm_tr104_st.e8c_autotest.enable==1) {
							rcm_tr104_st.e8c_autotest.enable=0;
							rcm_tr104_st.e8c_autotest.TestStatus=2; /*Complete*/
							rcm_tr104_st.e8c_autotest.CallerFailReason=CALLER_NO_ANSWER;
							rcm_tr104_st.e8c_autotest.Conclusion=1;/*fail*/
			
						}
#endif	
			
			if (bTransferring)
				//[SD6, bohungwu, exosip 3.5 integration
				//eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 404 Not Found");
				rcm_eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 404 Not Found");
				//]
			break ;
		case 415 :
			linphonec_display_something(lc, _("Remote user cannot support any of proposed codecs."));
			if (bTransferring)
				//[SD6, bohungwu, exosip 3.5 integration
				//eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 415 Unsupported Media Type");
				rcm_eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 415 Unsupported Media Type");
				//]
			break ;
		case 480 :
			tmpmsg = msg480 ;
		case 486 :
			linphonec_display_something(lc, tmpmsg);
			if (bTransferring)
				//[SD6, bohungwu, exosip 3.5 integration
				//eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 486 Busy Here");
				rcm_eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 486 Busy Here");
				//]
			break ;
		case 487 :
			linphonec_display_something(lc, msg487);
			if (bTransferring)
				//[SD6, bohungwu, exosip 3.5 integration
				//eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 487 Request Cancelled");
				rcm_eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 487 Request Cancelled");
				//]
			break ;	
		/*++added by Jack Chan 13/02/07 for T.38++*/
		case 488 :
			linphonec_display_something(lc, msg488);
			if (ssid == -1 || lc->call[ssid] == NULL)
				break;

			g_message("+++++DEBUG:lc flag is %d, call->state is %d +++++\n", 
				lc->call[ssid]->faxflag,
				lc->call[ssid]->state);

			// t.38 failed handling => change to t.30
			if (lc->call[ssid]->faxflag == FMS_FAX_RUN &&	// check fax or modem 
				lc->call[ssid]->state == LCStateFax && 
				lc->call[ssid]->t38_params.initialized)
			{
				g_message("\r\n *** linphone_call_failure , 488 FAX must implement \n");

				lc->call[ssid]->sdpctx->version++;
				lc->call[ssid]->t38_params.initialized = 0;		//reset the t38 parameters
				rcm_linphone_eXosip_on_fax(lc,ssid,0,FMS_FAX_RUN);

				return 0;
			}
			break;
		/*--end--*/
		case 600 :
			linphonec_display_something(lc, msg600);
			break ;
		case 603 :
			linphonec_display_something(lc, msg603);
			break ;
		case -110 :	/* time out, call leg is lost */
			linphonec_display_something(lc, _("Timeout."));
			break ;
		case -111 :
			linphonec_display_something(lc, _("Remote host was found but refused connection."));
			break ;
		default :
			if( code > 0 )
			{
				reason = osip_message_get_reason( code ) ;
				linphonec_display_something(lc, (char *) reason);
			}
			else
				g_warning( "failure_cb unknown code=%i\n" , code ) ;
	}

	switch (GetSysState(lc->chid))
	{
	case SYS_STATE_CONNECT_EDIT:
		if (lc->call[ssid] && lc->call[ssid]->state == LCStateHold)
		{
			int ssid2;

			// hold failed, recover it
			ssid2 = ssid == 0 ?  1: 0;
			switch (GetSessionState(lc->chid, ssid2))
			{
			case SS_STATE_IDLE:
				rcm_tapi_SetPlayTone(lc->chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				SetSysState(lc->chid, SYS_STATE_CONNECT);
				SetActiveSession(lc->chid, ssid, TRUE);
				//lc->call[ssid]->state = LCStateAVRunning;
				SetCallState(lc->chid,ssid,&(lc->call[ssid]->state),LCStateAVRunning);
				g_message("\r\nlinphone linphone_call_failure chid[%d],ssid[ssid], status %d\n",lc->chid,ssid,lc->call[ssid]->state );
			
				return 0;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	if( lc->ringstream[ssid])
		lc->ringstream[ssid] = FALSE ;

	rcm_tapi_SetRingFXS(lc->chid, FALSE);

	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
		rcm_tapi_FXO_Busy(lc->chid);
	else{
		/* play busy tone */
		rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		/* start timer to play wrong tone , if user forget offhook phone */
		rcm_timerLaunch(lc->offhook_timer,((g_pVoIPCfg->BusyToneTimer)*1000)); //play wrong tone
		rcm_timerLaunch(lc->stop_alltone_timer,((g_pVoIPCfg->BusyToneTimer+g_pVoIPCfg->HangingReminderToneTimer)*1000)); 


	}

	SetSessionState(lc->chid, ssid, SS_STATE_IDLE);

	// rock: stop if early media failed
	linphone_core_stop_media_streams(lc, ssid); 

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	S2U_DisconnectInd( lc ->chid, ssid, 7 );
#endif

	if( lc->call[ssid] != NULL )
		linphone_call_destroy( lc->call[ssid]);
	lc->call[ssid] = NULL ;

	if (GetActiveSession(lc->chid) == -1)
	{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		//S2U_DisconnectInd( 0 );
#endif
		SetSysState(lc->chid, SYS_STATE_IDLE);
	}
	else if (bTransferring)
	{
		SetActiveSession(lc->chid, ssid == 0 ? 1 : 0, TRUE);
		SetSysState(lc->chid, SYS_STATE_CONNECT);
	}
	else
	{
		switch (GetSysState(lc->chid))
		{
		case SYS_STATE_CONNECT:
			// rock: if early media failed
			SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
			break;
		case SYS_STATE_EDIT:
		case SYS_STATE_CONNECT_EDIT:
		case SYS_STATE_TRANSFER_EDIT:
#ifdef PLANET_TRANSFER
		case SYS_STATE_TRANSFER_ATTENDED:
#endif /*PLANET_TRANSFER*/
			SetSysState(lc->chid, GetSysState(lc->chid) + 1);
			break;
		default:
			// do nothing
			break;
		}
	}

	return 0 ;
}

int linphone_inc_new_call(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev)
{
	char * barmesg;
	int status = 200 ;
	osip_message_t *answermsg;
	sdp_context_t * ctx = NULL ;
	osip_from_t * from_url = NULL ;
	osip_header_t *history_info=NULL;
	osip_header_t *diversion=NULL;
	int cid = ev->cid ;
	int did = ev->did ;
	//[SD6, bohungwu, exoisp 3.5 integration
	int tid = ev->tid ;
	//char * from = ev->remote_uri ;
	//char * sdp_body = ev->sdp_body ;
	char * from=NULL;
	char * sdp_body=NULL;
	sdp_message_t *sdp_msg;
	//]
	int i;
	uint32 chid = lc->chid;
	char *displayname;
	//[SD6, bohungwu, exoisp 3.5 integration
	//char *to = ev->local_uri;
	char *to=NULL;
	//]
	osip_to_t *to_url = NULL;
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	char one_stage_dial_number[FW_LEN];
	LinphoneProxyConfig *used_proxy;
	/* call transfer */
	int match_cid=-1;
	osip_replaces_t *replaces=NULL;
	char *replaces_str=NULL;
	guint32 another_ssid;
	int pos = 0;
	osip_header_t *_header = NULL;	
	char * to_username =NULL;

	//[SD6, bohungwu, eXosip 3.5 integration
	//osip_from_init( &from_url ) ;
	//osip_from_parse( from_url , from ) ;

	//osip_to_init(&to_url);
	//osip_to_parse(to_url, to);

	g_message("linphone_inc_new_call \n");

	from_url=osip_message_get_from(ev->request);
	to_url=osip_message_get_to(ev->request);
	osip_from_to_str(from_url,&from);
	osip_from_to_str(to_url,&to);
	g_message("linphone_inc_new_call to_url is %s \n",to);
	g_message("linphone_inc_new_call from_url is %s \n",from);

	
	/* to support Call transfer */

	//check new invite replaces head

	replaces=osip_message_get_replaces(ev->request);
	osip_replaces_to_str(replaces,&replaces_str);

	if(replaces)
	{
		g_message("linphone_inc_new_call replaces=%s\n",replaces_str);
		match_cid=eXosip_call_find_by_replaces(replaces_str);
		g_message("linphone_inc_new_call match_cid=%d\n",match_cid);
		int hs;
		if (match_cid>=0)//match 
		{
			hs = (ssid == 0) ? 1 : 0;
			linphone_core_terminate_dialog(lc, hs, NULL);
		}else{//if not match any call , need respones ?
			g_message("linphone_inc_new_call replace not match any call\n");
		}

			
	}

	//Retrieve SDP data structure and conver it to a string as requested by Linphone
	sdp_msg = eXosip_get_sdp_info(ev->request);
	
	if(sdp_msg == NULL) {
		g_message( "%s:sdp_msg is null\n", __FUNCTION__);
	}

#if 0
	/* first check if we can answer successfully to this invite */
	if( lc->presence_mode[chid] != LINPHONE_STATUS_ONLINE )
	{
		g_message( "Not present !! presence mode[%d] : %d!!\n" , chid , lc->presence_mode[chid] ) ;
		eXosip_lock() ;

		if( lc->presence_mode[chid] == LINPHONE_STATUS_BUSY )
		{
			g_warning( "The call[%d][%d] is BUSY now !!" , chid , ssid ) ;
			eXosip_answer_call( did , 486 , NULL ) ;
		}
		else if( lc->presence_mode[chid] == LINPHONE_STATUS_AWAY ||
				lc->presence_mode[chid] == LINPHONE_STATUS_BERIGHTBACK ||
				lc->presence_mode[chid] == LINPHONE_STATUS_ONTHEPHONE ||
				lc->presence_mode[chid] == LINPHONE_STATUS_OUTTOLUNCH ||
				lc->presence_mode[chid] == LINPHONE_STATUS_OFFLINE ||
				lc->presence_mode[chid] == LINPHONE_STATUS_NOT_DISTURB )
		{
			g_warning( "Temporarily Unavailable [%d]!!" , chid ) ;
			eXosip_answer_call( did , 480 , NULL ) ;
		}
		else if( lc->alt_contact != NULL && lc->presence_mode[chid] == LINPHONE_STATUS_MOVED )
		{
			g_warning( "Moved Temporarily [%d][%d]!!" , chid , ssid ) ;
			eXosip_set_redirection_address( did , lc->alt_contact ) ;
			eXosip_answer_call( did , 302 , NULL ) ;
			eXosip_set_redirection_address( did , NULL ) ;
		}
		else if( lc->alt_contact != NULL && lc->presence_mode[chid] == LINPHONE_STATUS_ALT_SERVICE )
		{
			g_warning( "Alternative Service [%d][%d]!!" , chid , ssid ) ;
			eXosip_set_redirection_address( did , lc->alt_contact ) ;
			eXosip_answer_call( did , 380 , NULL ) ;
			eXosip_set_redirection_address( did , NULL ) ;
		}
		else
		{
			g_warning( "Busy Here [%d]!!" , chid  ) ;
			eXosip_answer_call( did , 486 , NULL ) ;
		}

		eXosip_unlock() ;
		goto end ;
	}
#endif

	/* check fxo line */
    //if ( (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature)) && (rcm_tapi_line_check(lc->chid) != 0) )
    if ( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature) && (rcm_tapi_line_check(lc->chid) != 0) )
	{
		g_warning("The line%d is not connected now !!\n", chid);
		eXosip_lock() ;
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_answer_call( did , 486 , NULL ) ;
		eXosip_call_send_answer( tid , 486 , NULL ) ;
		//]
		eXosip_unlock() ;
		goto end ;
	}

	/* check busy */
	if (lc->call[ssid] != NULL)
	{
		g_warning("The call[%d] is BUSY now !!\n", chid);
		eXosip_lock() ;
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_answer_call( did , 486 , NULL ) ;
		eXosip_call_send_answer( tid , 486 , NULL ) ;
		//]
		eXosip_unlock() ;
		goto end ;
	}

	/* global error */
	//eric: support delay offer , if no sdp , use 180ring +sdp(offer) send to offer
#if 0 

	//if( strlen( sdp_body ) == 0 )
	if(sdp_msg == NULL)
	{
		g_warning("No sdp body [%d]!!\n", chid);
		eXosip_lock() ;
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_answer_call( did , 603 , NULL ) ;
		eXosip_call_send_answer( tid , 603 , NULL ) ;
		//]
		eXosip_unlock() ;
		goto end ;
	}
#endif	

	used_proxy = NULL;
	
#ifdef MULTI_PROXY
	// check request come from which proxy
	if (&to_url->url.host[0] != '\0')
	{
		for (i=0; i<MAX_PROXY; i++)
		{
			// TODO: consider DNS ?
			if (lc->proxies[i] &&
				strcmp(to_url->url.host, &lc->proxies[i]->reg_proxy[4]) == 0)
			{
				// record used proxy in call
				g_message("record used proxy in call index %d\n",i);
				used_proxy = lc->proxies[i];
				break;
			}
		}
	}
#else
	if (lc->default_proxy &&
		strcmp(to_url->url->host, &lc->default_proxy->reg_proxy[4]) == 0)
	{
		used_proxy = lc->default_proxy;
	}
#endif



	// rock: check VoIP to PSTN
	one_stage_dial_number[0] = 0;

#if 0	 //eric mark,
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
	{
		// is 1st session ?
		if (GetSysState(chid) != SYS_STATE_IDLE)
		{
			g_warning("call-waiting in DAA!?\n");
		}

		// rock: check Unconditional PSTN forward
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	//	if (!local_call_pstn && voip_ptr->uc_forward_enable == 2)
		if ( voip_ptr->uc_forward_enable == 2)
			strcpy(one_stage_dial_number, voip_ptr->uc_forward);
#else
		if (!local_call && lc->sip_conf.uc_forward_enable == 2)
			strcpy(one_stage_dial_number, lc->sip_conf.uc_forward);
#endif
		// rock: check one stage dial
		// pkshih: add "sip:number@127.0.0.1" to pass PSTN routing trick
		else if (&to_url->url.username[0] != '\0')
		{
			if (!local_call)
			{
				if (used_proxy)
				{
					if (strcmp(to_url->url.username, voip_ptr->proxies[used_proxy->index].number) != 0)
						one_stage_dial_number[0] = 1; // one stage dial via proxy call
					else
						; // two stage dial via proxy call
				}
				else
				{
					for (i=0; i<MAX_PROXY; i++)
						if (strcmp(to_url->url.username, voip_ptr->proxies[i].number) == 0)
							break;

					if (i == MAX_PROXY)
						one_stage_dial_number[0] = 1; // one stage dial via direct ip call
					else
						; // two stage dial via direct ip call
				}

				if (one_stage_dial_number[0] == 1 &&
					!g_pVoIPCfg->one_stage_dial)
				{
					g_warning("reject one stage dial\n");
					eXosip_lock();
					//[SD6, bohungwu, exosip 3.5 integration
					//eXosip_answer_call(did, 603, NULL);
					eXosip_call_send_answer(tid, 603, NULL);
					//]
					eXosip_unlock();
					goto end;
				}
			}

			if (local_call || one_stage_dial_number[0] == 1)
			{
				strncpy(one_stage_dial_number, to_url->url.username, sizeof(one_stage_dial_number) - 1);
				one_stage_dial_number[sizeof(one_stage_dial_number) - 1] = 0;
			}
		}
		else
		{
			// two stage dial
			// 	- local_call => sip:127.0.0.1 (use *0)
			// 	- sip:ip
			// 	- sip:proxy
		}

		// rock: check two stage dial
		if (!local_call &&
			one_stage_dial_number[0] == 0 &&
			g_pVoIPCfg->two_stage_dial == 0)
		{
			g_warning("reject two stage dial\n");
			eXosip_lock();
			//[SD6, bohungwu, exosip 3.5 integration
			//eXosip_answer_call(did, 603, NULL);
			eXosip_call_send_answer(tid, 603, NULL);
			//]
			eXosip_unlock();
			goto end;
		}
	}
#endif

	//check this call is to fxo ? 
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
	{
		if(strcmp(to_url->url.username,"fxo")==0){
			/* case1: 
				from_url is fxstofxo@ip
				to_url is fxo@ip , local call (use *0) , do nothing */
		}else{
			/* case2: 
				from_url is fxstofxo@ip
				to_url is number@ip , copy number to ons_stage_dialnumber for fxo use */
			strncpy(one_stage_dial_number, to_url->url.username, sizeof(one_stage_dial_number) - 1);
			one_stage_dial_number[sizeof(one_stage_dial_number) - 1] = 0;
		}
	}

	//[SD6, bohungwu, eXosip 3.5 integration
	//lc->call[ssid] = linphone_call_new_incoming(lc, ssid, from, ev->local_uri, cid, did);
	lc->call[ssid] = linphone_call_new_incoming(lc, ssid, from, to, from_url,to_url,cid, did);
	//]
	if (lc->call[ssid] == NULL)
	{
		g_warning("%s: create call failed\n", __FUNCTION__);
		eXosip_lock();
		//[SD6, bohungwu, exosip 3.5 integration
		//eXosip_answer_call(did, 500, NULL);
		eXosip_call_send_answer(tid, 500, NULL);
		//]
		eXosip_unlock();
		goto end ;
	}

	
	/*support delay offer */	
	if(sdp_msg == NULL)
	{
		lc->call[ssid]->delay_offer_enable=1;
	}else{
		lc->call[ssid]->delay_offer_enable=0;
	}
	
#ifdef MULTI_PROXY
	if(used_proxy == NULL){
		printf("used_proxy is null\n");
		lc->call[ssid]->proxy =lc->default_proxy;
	}else{
	lc->call[ssid]->proxy = used_proxy;
	}
#endif

	caller_id[0] = 0;
	caller_id_name[0] = 0;

	// username -> caller id
	if (&from_url->url.username[0] != '\0')
	{
		/* At least 3 digits */
		/* 2 digits may be a control signal in some cases */
		if (strlen(from_url->url.username) >=3)
		{
			strncpy(caller_id, from_url->url.username, sizeof(caller_id) - 1);

			caller_id[strlen(caller_id)] = 0;

#if 1  /* customize caller_id */  /* CLIP */
			replace_caller_id( caller_id );
#endif

			/* DTMF call id need all digit, but FSK. */
		    /* or private caller id (start with '-') */
			if ((lc->caller_id_mode&7) == CID_DTMF)
			{
				for (i=0; caller_id[i]; i++) 
					if (!isdigit(caller_id[i]))
					{
						caller_id[0]=0; /* reset illegal cid */
						break;
					}
			}
		}
	}

	// displayname -> caller id name
	if (from_url->displayname)
	{
		// use display name as caller id name
		displayname = osip_strdup_without_quote(from_url->displayname);
		strncpy(caller_id_name, displayname, sizeof(caller_id_name) - 1);
		caller_id_name[sizeof(caller_id_name) - 1] = 0;
		osip_free(displayname);
	}

	/* if caller_id is empty */	
	/* use caller_id_name as caller id */
	if (caller_id[0] == 0)
	{
		/* At least 3 digits */
		/* 2 digits may be a control signal in some cases */
		if (strlen(caller_id_name)>=3)
		{
			strncpy(caller_id, caller_id_name, sizeof(caller_id) - 1);
			caller_id[strlen(caller_id)] = 0;

			/* DTMF call id need all digit, but FSK. */
		    /* or private caller id (start with '-') */
			if ((lc->caller_id_mode&7) == CID_DTMF)	 
			{
				for (i=0; caller_id[i]; i++) 
					if (!isdigit(caller_id[i]))
					{
						caller_id[0]=0; /* reset illegal cid */
						break;
					}
			}
		}

		// for direct ip call: from not inculde number, always display "0123456789"
		to_username=osip_uri_get_username(&ev->request->req_uri);
		//printf("\r\n to_username is %s\n",to_username);
		if((to_username==NULL)|| (strlen(to_username)==0)){
			if (!(voip_ptr->fxsport_flag & FXS_REJECT_DIRECT_IP_CALL))
				strcpy(caller_id, "0123456789");
		}
		
		// if error use private caller id & name
		if (strlen(caller_id) == 0)
		{
			strcpy(caller_id, "P");
			strcpy(caller_id_name, "P");
		}
	}
	else if (strlen(caller_id_name) == 0)
	{
		// if error use private caller id name
		strcpy(caller_id_name, "P");
	}

	ctx = lc->call[ssid]->sdpctx ;
	/* get the result of the negociation */
	if (lc->call[ssid]->delay_offer_enable==0){		
	rcm_sdp_context_get_answer_by_ev( ctx , ev ) ;
	status = sdp_context_get_status( ctx ) ;
	if( GetSessionState( chid , ssid ) == SS_STATE_IDLE )
	{
		// avoid callee use hold function problem
		ctx->offer = sdp_context_generate_template( ctx ) ;
		// set version the same with current anwser
		sprintf(ctx->offer->o_sess_version, "%.10d", ctx->version);
	}

	}else{
		if (ctx->localip==NULL) {
			rcm_get_localip_for_voip(solar_ptr->current_voip_interface,&ctx->localip);
		}
		status=200;
	}
	
	eXosip_call_set_reference(did, (void *) lc->call[ssid]);

	lc->call[ssid]->cid=ev->cid;
	lc->call[ssid]->did=ev->did;
	lc->call[ssid]->tid=ev->tid;


	switch( status )
	{
		case 200 :
			{
				char * tmp ;
				from_2char_without_params( from_url , &tmp ) ;
				barmesg = g_strdup_printf( "%s is calling you.\n" , tmp) ;
				linphonec_display_something(lc, barmesg);
				g_free( barmesg ) ;
				osip_free( tmp ) ;
			}
			
			{
				eXosip_call_t *jc = NULL;
				eXosip_dialog_t *jd = NULL;

				// record dtmf mode for handling telephone-event in SDP
      			eXosip_call_dialog_find(did, &jc, &jd);
				if (jc && jc->c_ctx)
				{
					jc->c_ctx->dtmf_mode = lc->sip_conf.dtmf_type;
				}
			}
#ifdef ATTENDED_TRANSFER
			if (ev->rc != NULL||match_cid>=0)//match replace accept this invite, dont send 180, only respone 200 ok
			{
				SetSysState(chid, SYS_STATE_RING);
				SetSessionState(chid, ssid, SS_STATE_CALLIN);
				//linphone_call_set_state(lc->call[ssid], LCStateRinging);
				SetCallState(chid,ssid,&(lc->call[ssid]->state),LCStateRinging);
				linphone_core_accept_dialog(lc, ssid, NULL);
				goto end;
			}
#endif

			// check VoIP to PSTN
			//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
			if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
			{
				// is 1st session ?
				if (GetSysState(chid) != SYS_STATE_IDLE)
					g_warning("call-waiting in DAA!?\n");

				SetSysState(chid, SYS_STATE_RING);

				if (one_stage_dial_number[0])
					strcpy(lc->call[ssid]->one_stage_dial_number, one_stage_dial_number);

				// disable 180 ringing if:
				//  - local pstn routing ( use *0 or PSTN routing prefix )
				//  - one stage dial ( use pstn forward or number assigned via "To" filed )
				
				if (lc->call[ssid]->one_stage_dial_number[0] == 0)	// one stage dial
				{
					eXosip_lock();
					eXosip_call_send_answer(tid, 180, NULL);
					eXosip_unlock();
					lc->ringstream[ssid] = TRUE;
				}

				SetSessionState(chid, ssid, SS_STATE_CALLIN);
				rcm_tapi_FXO_RingOn(lc->chid);
				//linphone_call_set_state(lc->call[ssid], LCStateRinging);
				SetCallState(chid,ssid,&(lc->call[ssid]->state),LCStateRinging);
				
				break;
			}
			else if (lc->parent->bAutoAnswer)
			{
				SetSysState(chid, SYS_STATE_RING);
				SetSessionState(chid, ssid, SS_STATE_CALLIN);
				//linphone_call_set_state(lc->call[ssid], LCStateRinging);
				SetCallState(chid,ssid,&(lc->call[ssid]->state),LCStateRinging);
				
				linphone_core_accept_dialog(lc, ssid, NULL);
				goto end;
			}

			/*normal case , incoming call return 180ring */
			eXosip_call_build_answer(tid,180,&answermsg);
			/* check history-info header */
			osip_message_get_history_info(ev->request,0,&history_info);
			if (history_info){
				osip_message_set_history_info(answermsg,history_info->hvalue);
			}
			/* check Diversion header */
			osip_message_get_diversion(ev->request,0,&diversion);
			if (diversion){
					osip_message_set_diversion(answermsg,diversion->hvalue);
			}
  			
			/*check incoming INVITE message support PRACK  */
			if(!(g_pVoIPCfg->rfc_flags & SIP_DISABLE_PRACK)){
				pos = osip_message_get_supported(ev->request, pos, &_header);
				if((pos>=0)&&(_header!=NULL)&&(_header->hvalue!=NULL)){
					 if (0==osip_strncasecmp(_header->hvalue, "100rel", 6)){
							/* if include support 100rel , and we enable prack , send 100rel */ 			
				osip_message_set_require(answermsg,"100rel");
				osip_message_set_header(answermsg, "Rseq", "1"); //ericchung: add Rseq for PRACK 
			}
				}						
			}
		
			eXosip_lock() ;
			eXosip_call_send_answer( tid , 180 , answermsg ) ;
			eXosip_unlock() ;

			if(lc->no_Answer_timeout !=0)
			{
				g_message("launch no answer timer timer after %d sec",lc->no_Answer_timeout);
				rcm_timerLaunch (lc->no_Answer_timer_id, lc->no_Answer_timeout*1000);
			}	
			
			if (GetSysState(chid) == SYS_STATE_IDLE)
			{
				// is 1st session
				SetSysState(chid, SYS_STATE_RING);

				//============= Caller ID Generation =========// //thlin+
				caller_id_mode[chid] = lc->caller_id_mode;
#ifndef CONFIG_RTK_VOIP_IP_PHONE
				//  for DTMF CID
				if ((caller_id_mode[chid]&7) == CID_DTMF)	 /* 0:Bellcore 1:ETSI 2:BT 3:NTT 4:DTMF */
				{
					//rcm_tapi_Gen_Dtmf_CID(chid, caller_id);
					rcm_tapi_Gen_CID_And_FXS_Ring(chid, 0/*dtmf*/, caller_id, NULL, NULL, 0, 0);
				} 
				else
				{
					//rcm_tapi_Gen_FSK_CID( chid, caller_id, NULL, caller_id_name, 0);
//fsk send caller id ,not need send caller name.
					rcm_tapi_Gen_CID_And_FXS_Ring(chid, 1/*fsk*/, caller_id, NULL, caller_id, 0/*type1*/, 0);
				}
#endif /* !CONFIG_RTK_VOIP_IP_PHONE */
			
#ifdef VMWI_TEST 			
				char vmwi_on = 1, vmwi_off = 0;	
				for(i=0; i<6000000; i++);
				
				/* Set VMWI message on in on_hook mode */
				rcm_tapi_Gen_FSK_VMWI( chid, &vmwi_on, 0);
				for(i=0; i<6000000; i++);
				
				/* Set VMWI message off in on_hook mode */
				rcm_tapi_Gen_FSK_VMWI( chid, &vmwi_off, 0);
				for(i=0; i<6000000; i++);
				rcm_tapi_Gen_FSK_VMWI( chid, &vmwi_off, 0);
#endif
			
				//==============================================//

#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_IncomingCallInd( chid, ssid, 0 /* normal call */, ( const unsigned char * )caller_id );
#endif
			}
			else
			{
				// call waiting caller ID (FSK Type II)
#ifndef CONFIG_RTK_VOIP_IP_PHONE
				if (  (call_waiting_cid[chid])
				    &&((caller_id_mode[chid]&7) != CID_DTMF)  ){ //add for disable waiting id
					//rcm_tapi_Gen_FSK_CID( chid, caller_id, NULL, caller_id_name, 1);
					rcm_tapi_Gen_CID_And_FXS_Ring(chid, 1/*fsk*/, caller_id, NULL, caller_id_name, 1/*type2*/, 0);

				// wait fsk caller id done, then send call waiting tone
				int32 tmp=-1;
				do
				{
					if(rcm_tapi_GetFskCIDState(chid, &tmp) != 0)
						break;
					usleep(50000);  // 50ms
				}
				while (tmp);
				}

#endif /* !CONFIG_RTK_VOIP_IP_PHONE */

				// call waiting tone
				rcm_tapi_SetPlayTone( chid , ssid ? 0 : 1, DSPCODEC_TONE_CALL_WAITING , TRUE , DSPCODEC_TONEDIRECTION_LOCAL ) ;

#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_IncomingCallInd( chid, ssid, 1 /* call waiting */, ( const unsigned char * )caller_id );
#endif
			}

			/* play the ring */
			lc->ringstream[ssid] = TRUE;
			SetSessionState( chid , ssid , SS_STATE_CALLIN ) ;
//			if (lc->call[ssid == 0 ? 1 : 0] == NULL)
			if (GetSessionState(chid, ssid == 0 ? 1 : 0) == SS_STATE_IDLE) {
				StopAlarmRinging( lc );
				//rcm_tapi_SetRingFXS( chid , TRUE ) ;
			}


			SetCallState(chid,ssid,&(lc->call[ssid]->state),LCStateRinging);

			// add no answer check
#ifdef CONFIG_RTK_VOIP_DIALPLAN
			osip_gettimeofday(&lc->call[ssid]->no_answer_start, NULL);
#else
			osip_gettimeofday(&lc->call[ssid]->no_answer_start, NULL);
#endif /* CONFIG_RTK_VOIP_DIALPLAN */

			break ;
		default :	
			if( status == -1 )
				status = 415 ;
			g_warning( "Error during sdp negociation. status : %d\n" , status ) ;
			eXosip_lock() ;
			//[SD6, bohungwu, exosip 3.5 integration
			//eXosip_answer_call( did , status , NULL ) ;
			eXosip_call_send_answer(tid, status, NULL);
			//]
			eXosip_unlock() ;
#ifdef CONFIG_RTK_VOIP_IP_PHONE
			S2U_DisconnectInd( chid, ssid, 8 );
#endif
			linphone_call_destroy(lc->call[ssid]);
			lc->call[ssid] = NULL;
			break;
	}

end :
g_message("\r\n linphone_inc_new call end !! \n");
	if(sdp_msg!=NULL)
	sdp_message_free(sdp_msg);

	osip_free(from);
	osip_free(to);

	return 0 ;
}

void linphone_do_automatic_redirect(LinphoneCore *lc, uint32 ssid, const char *contact)
{
	char * msg = g_strdup_printf( _( "Redirected to %s..." ) , contact ) ;

	linphonec_display_something(lc, msg);
	g_free( msg ) ;

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	S2U_DisconnectInd( lc ->chid, ssid, 9 );
#endif

	if (lc->call[ssid] != NULL)
		linphone_call_destroy(lc->call[ssid]);

	lc->call[ssid] = NULL;

	if ((strcmp(contact, "<sip:fxo_redial@127.0.0.1>") == 0) &&
		//lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
		RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
	{
		lc->ringstream[ssid] = FALSE;
		SetActiveSession(lc->chid, ssid, TRUE);
#ifdef CONFIG_RTK_VOIP_IVR
		rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
		rcm_tapi_FXO_offhook(lc->chid);
		if (g_pVoIPCfg->ports[lc->chid].offhook_passwd[0])
		{
			char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};
			SetSysState(lc->chid, SYS_STATE_AUTH);
			rcm_tapi_IvrStartPlaying(lc->chid, ssid,IVR_DIR_LOCAL, text);
		}
		else
		{
			char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};
			SetSysState(lc->chid, SYS_STATE_EDIT);
			rcm_tapi_IvrStartPlaying(lc->chid, ssid,IVR_DIR_LOCAL, text);
		}
		lc->bPlayIVR = 1;
#else
		SetSysState(lc->chid, SYS_STATE_EDIT);
		rcm_tapi_FXO_offhook(lc->chid);
		rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
#endif
		digitcode_init(lc);
		osip_gettimeofday(&lc->off_hook_start, NULL);
		SetSessionState(lc->chid, ssid, SS_STATE_IDLE);
	}
	else
	{
		linphone_core_invite(lc, ssid, contact);
	}
}

void linphone_call_redirected(LinphoneCore *lc, uint32 ssid, int cid, int code, const char *contact)
{
	switch( code )
	{
		case 380 :
			linphonec_display_url(lc, _("User is not reachable at the moment but he invites you\n" \
				"to contact him using the following alternate resource:"), (char *) contact);
			if (lc->call[ssid] != NULL)
			{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_DisconnectInd( lc ->chid, ssid, 10 );
#endif

				linphone_call_destroy(lc->call[ssid]);
				lc->call[ssid] = NULL;
			}
			break ;
		case 302 :
			linphone_do_automatic_redirect(lc, ssid, contact);
			break ;
	}
}

#ifdef SUPPORT_G7111
char *rtk_g7111_modes_precedence_to_str(unsigned char *precedence)
{
	int i, idx;
	static char g7111_mode_str[256];

	g7111_mode_str[0] = 0;
	for (i=0, idx=0; i < G7111_MODES; i++)
	{
		if (precedence[i])
		{
			if (idx)
				idx += sprintf(g7111_mode_str + idx, ",%d", precedence[i]);
			else
				idx += sprintf(g7111_mode_str + idx, "mode-set=%d",
					precedence[i]);
		}
	}

	return g7111_mode_str;
}

int rtk_g7111_modes_str_to_precedence(char *modes_str, 
	unsigned char *desired_precedence, unsigned char *precedence)
{
	int i, j;
	char *modes;
	char *str1, *token, *saveptr1;
	char *str2, *subtoken, *saveptr2;
	unsigned char g7111_mode;

	memset(precedence, 0, sizeof(precedence[0]) * G7111_MODES);

	i = 0;
	if (modes_str)
	{
		for (str1 = modes_str; ; str1 = NULL)
		{
			token = strtok_r(str1, " ;", &saveptr1);
			if (token == NULL)
				break;

			modes = strstr(token, "mode-set=");
			if (modes == NULL)
				continue;

			modes += strlen("mode-set=");
			for (str2 = modes; i<G7111_MODES; str2 = NULL)
			{
				subtoken = strtok_r(str2, ",", &saveptr2);
				if (subtoken == NULL)
					break;

				g7111_mode = atoi(subtoken);
#if 0
				// check mode is support in local
				for (j = 0; i<G7111_MODES && j<G7111_MODES; j++)
				{
					if (desired_precedence[j] == g7111_mode)
					{
						precedence[i++] = g7111_mode;
						break;
					}
				}
#else
				// follow remote always
				precedence[i++] = g7111_mode;
#endif
			}
		}
	}

	// use desired precedence if modes string is unspecified or unknown
	if (i == 0)
	{
		memcpy(precedence, desired_precedence,
			sizeof(precedence[0]) * G7111_MODES);
	}

	return 0;
}
#endif

//extern int enable_using_port( int chid ) ;

/* these are the SdpHandler callbacks: we are called in to be aware of the content
of the SDP messages exchanged */
/* outgoing call audio setting --> set_audio_codecs */
int linphone_set_audio_offer(sdp_context_t *ctx)
{
	LinphoneCall *call=(LinphoneCall*)sdp_context_get_user_pointer(ctx);
	LinphoneCore *lc=call->core;
	PayloadType *codec;
	GList *elem;
	sdp_payload_t payload;
	sdp_message_t *sdp=ctx->offer;
	char *port=NULL;
	int i, s_id=0;
#ifdef SUPPORT_RTP_REDUNDANT_APP
	char buff[ 16 ];
#endif
	int static_pt;

	for( i = 0; i < MAX_SS; i++){
		if( call == lc->call[i] ){
			s_id = i;
			break;
		}
	}

#if 0 // def SUPPORT_RTP_REDUNDANT_APP
	if( lc->sip_conf.rtp_redundant_payload_type ) {
		
		sdp_payload_init(&payload);
		payload.pt = lc->sip_conf.rtp_redundant_payload_type;
		payload.localport = lc->rtp_conf.audio_rtp_port  + s_id *2;
		payload.a_rtpmap = "red/8000/1";
		
		sprintf( buff, "%d/%d", lc->sip_conf.rtp_redundant_codec_type, 
								lc->sip_conf.rtp_redundant_codec_type );
		payload.a_fmtp = buff;
		
		sdp_context_add_audio_payload(ctx,&payload);
	}
#endif

/* try the codec change here!! eric wu in 10.22 */
	elem=lc->codecs_conf.audio_codecs;
	while(elem!=NULL){
		codec=(PayloadType*) elem->data;
		if (payload_type_usable(codec) && payload_type_enabled(codec)){
			sdp_payload_init(&payload);

			if ((strcasecmp(codec->mime_type, "red") == 0)  ) {
				if( lc->sip_conf.rtp_redundant_payload_type ) {
					payload.a_rtpmap = g_strdup_printf("%s/%i/1",codec->mime_type,codec->clock_rate);
					payload.a_fmtp = g_strdup_printf("%d/%d",lc->sip_conf.rtp_redundant_codec_type, 
										lc->sip_conf.rtp_redundant_codec_type );
				}else{
					elem=g_list_next(elem);
					continue;
				}
			}else
				payload.a_rtpmap=g_strdup_printf("%s/%i",codec->mime_type,codec->clock_rate);
#ifdef CONFIG_RTK_VOIP_SRTP
			if( 1 == lc->sip_conf.security_enable ){
				payload.proto="RTP/SAVP";
			}
#endif /* CONFIG_RTK_VOIP_SRTP */
#ifdef DYNAMIC_PAYLOAD
			if (strcasecmp(codec->mime_type, "iLBC") == 0 &&
				lc->rtp_conf.iLBC_mode == ILBC_20MS)
			{
				payload.a_fmtp = "mode=20";
			}
			else if (strcasecmp(codec->mime_type, "SPEEX") == 0)
			{
				switch (lc->rtp_conf.speex_nb_rate)
				{
					case SPEEX_RATE2P15:
						payload.a_fmtp = "mode=1";
						break;

					case SPEEX_RATE5P95:
						payload.a_fmtp = "mode=2";
						break;

					case SPEEX_RATE8:
						payload.a_fmtp = "mode=3";
						break;

					case SPEEX_RATE11:
						payload.a_fmtp = "mode=4";
						break;

					case SPEEX_RATE15:
						payload.a_fmtp = "mode=5";
						break;

					case SPEEX_RATE18P2:
						payload.a_fmtp = "mode=6";
						break;

					case SPEEX_RATE24P6:
						payload.a_fmtp = "mode=7";
						break;

					case SPEEX_RATE3P95:
						payload.a_fmtp = "mode=8";
						break;
				}
			}
#ifdef SUPPORT_G7111
			else if (strcasecmp(codec->mime_type, "PCMU-WB") == 0 ||
					(strcasecmp(codec->mime_type, "PCMA-WB") == 0))
			{
				payload.a_fmtp = rtk_g7111_modes_precedence_to_str(
					lc->rtp_conf.g7111_precedence);
			}
#endif // SUPPORT_G7111
#endif // DYNAMIC_PAYLOAD
			static_pt=rtp_profile_get_payload_number_from_rtpmap(
				lc->local_profile,payload.a_rtpmap);
		#ifdef DYNAMIC_PAYLOAD
			payload.pt = dynamic_pt_get(static_pt);
			if (payload.pt == -1)
				payload.pt = static_pt;
		#else
			payload.pt = static_pt;
		#endif
			payload.localport = lc->rtp_conf.audio_rtp_port  + s_id *2;
		#ifdef AUTO_RESOURCE
			if (VOIP_RESOURCE_UNAVAILABLE == rcm_tapi_VoIP_resource_check(lc->chid, static_pt))
				g_warning("no resource for payload %s.\n", codec->mime_type);
			else
				sdp_context_add_audio_payload(ctx,&payload);
		#else
			sdp_context_add_audio_payload(ctx,&payload);
		#endif
			g_free(payload.a_rtpmap);
		}
		elem=g_list_next(elem);
	}
	// add srtp attribute if needed
#ifdef CONFIG_RTK_VOIP_SRTP
	if( 1 == lc->sip_conf.security_enable ){
		unsigned char localKey[30];	//local srtp key

		sdp_generate_sdes_key(localKey, 30);
		base64Encode(localKey, payload.localSDPKey);
		payload.localCryptAlg = HMAC_SHA1;
		sdp_message_a_attribute_add
								(ctx->offer, 0, osip_strdup("crypto"),
								g_strdup_printf("1 AES_CM_128_HMAC_SHA1_80 inline:%s",payload.localSDPKey));	
	}
#endif /* CONFIG_RTK_VOIP_SRTP */
	/* Mandy add for stun support */
#if 0 // not support STUN now 
	if (lc->net_conf.nat_address!=NULL && lc->net_conf.use_nat){
		sdp_connection_t *conn = NULL;

		// rock: get global c_connection in session
		conn = sdp_message_connection_get(sdp, -1, 0);
		// rock: check c_connection in first media line
		if (conn == NULL)
			conn = sdp_message_connection_get(sdp, 0, 0);
		// rock: shouldn't occur
		if (conn == NULL)
			g_error("conn is null?\n");

		osip_free(conn->c_addr);
		conn->c_addr=osip_strdup(lc->net_conf.nat_address);
		port = (char*)osip_malloc(10);
		snprintf(port, 9, "%i", lc->net_conf.nat_voice_port[s_id]);
		sdp_message_m_port_set(sdp, 0, port);
		g_message("Set audio offer: The external port is %d.\n", lc->net_conf.nat_voice_port[s_id]);
	}
#endif
	if (lc->sip_conf.dtmf_type == DTMF_RFC2833)		// need telephone-event only if DTMF_RFC2833
	{
		/* add telephone-event payload*/
		sdp_payload_init(&payload);
		payload.pt = lc->sip_conf.rfc2833_payload_type;
		payload.a_rtpmap="telephone-event/8000";
		payload.a_fmtp="0-15";
		sdp_context_add_audio_payload(ctx,&payload);
	}

#ifdef SUPPORT_V152_VBD	
	if( lc->sip_conf.v152_payload_type ) {
		/* add gpmd:96 vbd=yes */
		sdp_payload_init(&payload);
		payload.pt = lc->sip_conf.v152_payload_type;
		
		switch( lc->sip_conf.v152_codec_type ) {
		case rtpPayloadPCMA:
			payload.a_rtpmap = "PCMA/8000";
			break;
			
		case rtpPayloadPCMU:
		default:
			payload.a_rtpmap = "PCMU/8000";
			break;
		}
		
		payload.a_gpmd = "vbd=yes";
		sdp_context_add_audio_payload(ctx,&payload);
	}
#endif

	// add sendrecv attribute
	sdp_message_a_attribute_add(ctx->offer, 0, osip_strdup ("sendrecv"), NULL);

	return 0;
}

#ifdef SUPPORT_RTP_REDUNDANT_APP
static int get_supported_rtp_redundant_payload_type( const sdp_payload_t *payload )
{
	int pt_red, t;
	const char *p = payload ->a_fmtp;
	
	if( p == NULL )
		return -1;
	
	pt_red = atoi( payload ->a_fmtp );
	
	// all redundant codec should be the same 
	while( *p ) {
		
		if( *p == '/' ) {
			t = atoi( p + 1 );
			
			if( pt_red != t )
				return -1;
		}
		
		p ++;
	}
	
	// check valid codec (u-law, a-law, G.729)
	switch( pt_red ) {
	case rtpPayloadPCMU:
	case rtpPayloadPCMA:
	case rtpPayloadG729:
		break;
		
	default:
		return -1;
	}
		
	return pt_red;
}
#endif

SupportLevel linphone_payload_is_supported(sdp_payload_t *payload, RtpProfile *local_profile, 
	RtpProfile *dialog_profile)
{
	int localpt;
	SupportLevel ret;

	if (payload->a_rtpmap!=NULL){
		localpt=rtp_profile_get_payload_number_from_rtpmap(local_profile,payload->a_rtpmap);
		// check static payload type
		if (payload->pt >= 0 && 
			payload->pt < 96 &&
			localpt != payload->pt)
		{
			g_warning("invalid static payload type (%s)\n", payload->a_rtpmap);
			return Unsupported;
		}
	}else{
		// check dynamic payload type
		if (payload->pt >= 96 && 
			payload->pt < RTP_PROFILE_MAX_PAYLOADS)
		{
			g_warning("no rtpmap in dynamic payload type\n");
			return Unsupported;
		}
		localpt=payload->pt;
		g_warning("payload has no rtpmap.\n");
	}

	if (localpt >= 0 && localpt < RTP_PROFILE_MAX_PAYLOADS){
		/* this payload is understood, but does the user want to use it ?? */
		PayloadType *rtppayload;
		rtppayload=rtp_profile_get_payload(local_profile,localpt);
		if (rtppayload==NULL) {
			g_warning("strange error !!\n");
			return Unsupported;
		}

		if (strcmp(rtppayload->mime_type,"telephone-event") == 0)
		{
			ret = Supported;
		}
#ifdef SUPPORT_V152_VBD
		else if( payload ->a_gpmd && strstr( payload ->a_gpmd, "vbd=yes" ) )
		{
			ret = Supported;
		}
#endif
#ifdef SUPPORT_RTP_REDUNDANT_APP
		else if( payload ->a_rtpmap && 
				 strncmp( payload ->a_rtpmap, "red", 3 ) == 0 )
		{
			if( ( payload ->pt_red = get_supported_rtp_redundant_payload_type( payload ) )
				== -1 )
			{
				ret = Unsupported;
			} else
				ret = SupportedAndValid;
		}
#endif
		else
		{
			if (!payload_type_usable(rtppayload) ){
				g_warning("payload %s is not usable\n",rtppayload->mime_type);
				return Unsupported;
			}
			if ( !payload_type_enabled(rtppayload)) {
				g_warning("payload %s is not enabled.\n",rtppayload->mime_type);
				return Unsupported;
			}
			ret = SupportedAndValid;
		}

		if (dialog_profile!=NULL){
			/* this payload is supported in our local rtp profile, so add it to the dialog rtp
			profile */
			rtppayload=payload_type_clone(rtppayload);
			rtp_profile_set_payload(dialog_profile,payload->pt,rtppayload);
			/* add to the rtp payload type some other parameters (bandwidth) */
			if (payload->b_as_bandwidth!=0)
			{
				rtppayload->normal_bitrate=payload->b_as_bandwidth*1000;
			}
			if (payload->a_fmtp!=NULL)
			{
				rtppayload->fmtp=g_strdup(payload->a_fmtp);
			}
			else if (strcasecmp(rtppayload->mime_type,"iLBC")==0){
				/*default to 30 ms mode */
				payload->a_fmtp="ptime=30";
				rtppayload->fmtp=g_strdup(payload->a_fmtp);
			}
		}
		return ret;
	}
	return Unsupported;
}

#ifdef SUPPORT_V152_VBD
static int linphone_fill_vbd_audio_param( StreamParams *params, sdp_payload_t *payload )
{
	if( payload ->a_gpmd && strstr( payload ->a_gpmd, "vbd=yes" ) )
	{
		int pt_vbd = -1;
	
		if( payload ->a_rtpmap ) {
			if( strstr( payload ->a_rtpmap, "PCMU/8000" ) )
				pt_vbd = rtpPayloadPCMU;
			else if( strstr( payload ->a_rtpmap, "PCMA/8000" ) )
				pt_vbd = rtpPayloadPCMA;
			else
				return -1;
		}
		
		if( pt_vbd != -1 ) {
			params ->initialized_vbd = 1;
			params ->static_pt_vbd = pt_vbd;
			params ->local_pt_vbd = payload ->pt;
			params ->remote_pt_vbd = payload ->pt;
		}
	}
	
	return 1;
}
#endif

#ifdef SUPPORT_RTP_REDUNDANT_APP
static int linphone_fill_rtp_redudant_audio_param( LinphoneCore *lc, StreamParams *params, sdp_payload_t *payload )
{
	if( payload ->a_rtpmap &&
		strncmp( payload ->a_rtpmap, "red", 3 ) == 0)
	{
		if( lc->sip_conf.rtp_redundant_payload_type == 0 )
			return -1;	// disable on web 
		
		// fill rtp redundant payload type 
		params->initialized_rtp_redundant = 1;
		params->local_pt_rtp_redundant = payload ->pt;
		params->remote_pt_rtp_redundant = payload ->pt;
		
		return 1;
	}
	
	return 0;
}
#endif

/* incoming call audio setting --> accept_audio_codecs */
int linphone_accept_audio_offer(sdp_context_t *ctx,sdp_payload_t *payload)
{
	StreamParams *params;
	int supported;
	int s_id=0, i;
	LinphoneCall *call=(LinphoneCall*)sdp_context_get_user_pointer(ctx);
	LinphoneCore *lc=call->core;
#ifdef SUPPORT_RTP_REDUNDANT_APP
	int rtp_redundant_case = 0;
#endif
	
	/* get the session number */
	for( i = 0; i < MAX_SS; i++){
		if( call == lc->call[i] ){
			s_id = i;
			break;
		}
	}

	supported = linphone_payload_is_supported(payload, lc->local_profile, NULL);
	if (supported == Unsupported) 
	{
		g_message("Refusing audio codec %i (%s)\n",payload->pt,payload->a_rtpmap);
		return Unsupported;
	}

	if (supported == SupportedAndValid)	
	{
	#if 0//def CHECK_LOOPBACK	
		if (ctx->loopback == 1 && 
			payload->pt != rtpPayloadPCMU)
		{
			// Support PCMU only if Loopback
			return Unsupported;
		}
	#endif
		params=&call->audio_params;
		if (params->initialized==0){

#ifdef SUPPORT_RTP_REDUNDANT_APP
			if( ( rtp_redundant_case =
					linphone_fill_rtp_redudant_audio_param( lc, params, payload ) )
					== -1 )
			{
				return Unsupported;	// disable on web 
			}
#endif
			
			/* this is the first codec we may accept*/
			params->localport = payload->localport = lc->rtp_conf.audio_rtp_port + s_id *2;
			params->remoteport=payload->remoteport;
			params->line=payload->line;
#ifdef CONFIG_RTK_VOIP_SRTP
			if(1 == lc->sip_conf.security_enable){
				memcpy(&params->remoteSDPKey, payload->remoteSDPKey, SDP_KEY_LEN);
				memcpy(&params->localSDPKey, payload->localSDPKey, SDP_KEY_LEN);
				params->remoteCryptAlg=payload->remoteCryptAlg;
			}
#endif /* CONFIG_RTK_VOIP_SRTP */
#ifdef DYNAMIC_PAYLOAD
			/* remember local pt (the same with remote pt always if we are callee) */
			params->local_pt=payload->pt;
			/* remember remote pt */
			params->remote_pt=payload->pt; 
			
			/* remember static pt for rtk dsp */
			if (payload->pt >= 0 && payload->pt < 96) 
				params->static_pt = payload->pt;
			else
				params->static_pt = rtp_profile_get_payload_number_from_rtpmap(lc->local_profile,payload->a_rtpmap);
				
#ifdef SUPPORT_RTP_REDUNDANT_APP
			if( rtp_redundant_case ) {
				// it has to be 0, 8 or 18 
				params->local_pt = params->remote_pt = params->static_pt = payload->pt_red;
			}
#endif
						
			/* check iLBC mode */
			if (params->static_pt == rtpPayload_iLBC &&
				payload->a_fmtp &&
				strcmp(payload->a_fmtp, "mode=20") == 0)
			{
				params->static_pt = rtpPayload_iLBC_20ms;		
			}
			/* check SPEEX NB bitrate */
			else if (params->static_pt == rtpPayload_SPEEX_NB_RATE8 &&
				payload->a_fmtp )
			{
					params->static_pt = rtpPayload_SPEEX_NB_RATE8;
				if (strcmp(payload->a_fmtp, "mode=1") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE2P15;
				if (strcmp(payload->a_fmtp, "mode=2") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE5P95;
				if (strcmp(payload->a_fmtp, "mode=3") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE8;
				if (strcmp(payload->a_fmtp, "mode=4") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE11;
				if (strcmp(payload->a_fmtp, "mode=5") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE15;
				if (strcmp(payload->a_fmtp, "mode=6") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE18P2;
				if (strcmp(payload->a_fmtp, "mode=7") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE24P6;
				if (strcmp(payload->a_fmtp, "mode=8") == 0)
					params->static_pt = rtpPayload_SPEEX_NB_RATE3P95;
			}
#ifdef SUPPORT_G7111
			else if (params->static_pt == rtpPayloadPCMU_WB ||
				params->static_pt == rtpPayloadPCMA_WB)
			{
				rtk_g7111_modes_str_to_precedence(payload->a_fmtp,
					lc->rtp_conf.g7111_precedence, params->g7111_precedence);

				payload->a_fmtp = rtk_g7111_modes_precedence_to_str(
					params->g7111_precedence);
			}
#endif

#ifdef AUTO_RESOURCE
			if (VOIP_RESOURCE_UNAVAILABLE == rcm_tapi_VoIP_resource_check(lc->chid, params->static_pt))
			{
				g_warning("no resource for payload %s.\n", payload->a_rtpmap);
				return Unsupported;
			}
#endif

#else
			params->pt=payload->pt; /* remember the first payload accepted */
		#endif
//			params->remoteaddr=payload->c_addr;
			bzero(&params->remote_addr, sizeof(params->remote_addr));
			strcpy(params->remote_addr,payload->c_addr);
			params->remote_ip=inet_addr(payload->c_addr);
			
			params->initialized=1;
		}else{
			/* refuse all other audio lines*/
			if(params->line!=payload->line) return -1;
		}
	}
#ifdef SUPPORT_V152_VBD
	else if (supported == Supported && payload ->a_gpmd)
	{
		if( lc->sip_conf.v152_codec_type == rtpPayloadUndefined )
			return Unsupported;	// disable on web 
		
		if( linphone_fill_vbd_audio_param( &call->audio_params, payload ) == -1 )
			return Unsupported;
	}
#endif

	return supported;
}

/* --> get_audio_codecs */
int linphone_read_audio_answer( sdp_context_t * ctx , sdp_payload_t * payload )
{
	LinphoneCall * call = ( LinphoneCall * )sdp_context_get_user_pointer( ctx ) ;
	LinphoneCore * lc = call->core ;
	StreamParams * params ;
	SupportLevel supported;
	int s_id=0, i;
#ifdef SUPPORT_RTP_REDUNDANT_APP
	int rtp_redundant_case = 0;
#endif

	supported = linphone_payload_is_supported(payload, lc->local_profile, NULL);

	for( i = 0; i < MAX_SS; i++){
		if( call == lc->call[i] ){
			s_id = i;
			break;
		}
	}
	
	if (supported == Unsupported)
	{
		g_warning("This remote sip phone did not answer properly to my sdp offer!\n");
		return 0;
	}

	if (supported == SupportedAndValid) 
	{
		params = &call->audio_params;
		if (params->initialized == 0)
		{
#ifdef DYNAMIC_PAYLOAD
			char *offer_fmtp = NULL;
#endif

#ifdef SUPPORT_RTP_REDUNDANT_APP
			if( ( rtp_redundant_case =
					linphone_fill_rtp_redudant_audio_param( lc, params, payload ) )
					== -1 )
			{
				return Unsupported;	// disable on web 
			}
#endif

			/* this is the first codec we may accept */
			params->localport = lc->rtp_conf.audio_rtp_port + s_id *2;
			params->remoteport = payload->remoteport ;
			params->line = payload->line ;
#ifdef CONFIG_RTK_VOIP_SRTP
			if(1 == lc->sip_conf.security_enable){
				memcpy(&params->remoteSDPKey, payload->remoteSDPKey, SDP_KEY_LEN);
				memcpy(&params->localSDPKey, payload->localSDPKey, SDP_KEY_LEN);
				params->remoteCryptAlg=payload->remoteCryptAlg; 
			}
#endif /* CONFIG_RTK_VOIP_SRTP */
#ifdef DYNAMIC_PAYLOAD
			/* remember local pt */
			if (payload->pt >= 0 && payload->pt < 96)
			{
				// static payload type
				params->local_pt = payload->pt;
			}
			else 
			{
				// dynamic payload type
				int j, pt;
				char *pt_str, *rtpmap;

				/* for each m= line */
				for (i = 0; !sdp_message_endof_media(ctx->offer, i); i++)
				{
					for (j = 0; ((pt_str = sdp_message_m_payload_get(ctx->offer, i , j)) != NULL); j++)
					{
						pt = osip_atoi(pt_str);
						if (payload->pt >= 0 && payload->pt < 96)
							continue;

						rtpmap = sdp_message_a_attr_value_get_with_pt(ctx->offer, i, pt, "rtpmap");
						if (strcmp(rtpmap, payload->a_rtpmap) == 0)
						{
							params->local_pt = pt; // found!
							offer_fmtp = sdp_message_a_attr_value_get_with_pt(ctx->offer, i, pt, "fmtp");
							break;
						}
					}

					if (pt_str != NULL) // found!
						break;
				}

				if (sdp_message_endof_media(ctx->offer, i)) // not found!?
				{
					// shouldn't occur
					g_warning("unknonw rtpmap = %s\n", payload->a_rtpmap);
					return -1;
				}
			}
			/* remember remote pt */
			params->remote_pt=payload->pt; 
			/* remember static pt for rtk dsp */
			if (payload->pt >= 0 && payload->pt < 96)
				params->static_pt = payload->pt;
			else
				params->static_pt = rtp_profile_get_payload_number_from_rtpmap(lc->local_profile,payload->a_rtpmap);
			
#ifdef SUPPORT_RTP_REDUNDANT_APP
			if( rtp_redundant_case ) {
				// it has to be 0, 8 or 18 
				params->local_pt = params->remote_pt = params->static_pt = payload->pt_red;
			}
#endif
			
#ifdef SUPPORT_G7111
			if (params->static_pt == rtpPayloadPCMU_WB ||
				params->static_pt == rtpPayloadPCMA_WB)
			{
				rtk_g7111_modes_str_to_precedence(payload->a_fmtp,
					lc->rtp_conf.g7111_precedence, params->g7111_precedence);
			}
			else
#endif
			/* check iLBC mode */
			if (params->static_pt == rtpPayload_iLBC &&
				payload->a_fmtp &&
				strcmp(offer_fmtp, "mode=20") == 0 &&
				strcmp(payload->a_fmtp, "mode=20") == 0)
			{
				params->static_pt = rtpPayload_iLBC_20ms;		
			}
#else
			params->pt=payload->pt; /* remember the first payload accepted */
#endif
//			params->remoteaddr = payload->c_addr ;
			bzero(&params->remote_addr, sizeof(params->remote_addr));
			strcpy(params->remote_addr,payload->c_addr);
			params->remote_ip=inet_addr(payload->c_addr);

			params->initialized = 1 ;
		}
	}
#ifdef SUPPORT_V152_VBD
	else if (supported == Supported && payload ->a_gpmd)
	{
		if( lc->sip_conf.v152_codec_type == rtpPayloadUndefined )
			return Unsupported;	// disable on web 
		
		if( linphone_fill_vbd_audio_param( &call->audio_params, payload ) == -1 )
			return Unsupported;
	}
#endif

	return 0 ;
}

/*++added by Jack Chan 02/02/07 for t.38++*/
int linphone_read_t38_answer( sdp_context_t * ctx , sdp_payload_t * payload )
{
	LinphoneCall * call = ( LinphoneCall * )sdp_context_get_user_pointer( ctx ) ;
	LinphoneCore * lc = call->core ;
	StreamParams * params ;

	params = &call->t38_params ;
	if( params->initialized == 0 )
	{
		/* this is the first codec we may accept */
		params->localport = lc->sip_conf.T38_prot;
		params->remoteport = payload->remoteport ;
		params->line = payload->line ;
	#ifdef DYNAMIC_PAYLOAD
		params->local_pt=rtpPayloadT38_Virtual;
		params->remote_pt=rtpPayloadT38_Virtual;
		params->static_pt=rtpPayloadT38_Virtual;
	#else
		params->pt=payload->pt; /* remember the first payload accepted */
	#endif
//		params->remoteaddr = payload->c_addr ;	
		bzero(&params->remote_addr, sizeof(params->remote_addr));
		strcpy(params->remote_addr,payload->c_addr);
		params->remote_ip=inet_addr(payload->c_addr);
	
		params->initialized = 1 ;
	}
	return 0 ;
}
/*--end--*/

void linphone_call_proceeding(LinphoneCore *lc, uint32 ssid, int cid, int did)
{
	if (lc->call[ssid] == NULL)
	{
		g_warning( "Bug in call_proceeding()\n" ) ;
		return ;
	}

	lc->call[ssid]->cid = cid;
	lc->call[ssid]->did = did;

}
/*peer side ringing */
/*receive 180ring */
void linphone_call_ringing(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev)
{
	/* 20110711, SD6, bohungwu, exosip 3.5 integration */
	/* Retrieve SDP data structure and conver it to a string as requested by Linphone */
	sdp_message_t *sdp_msg;
	char *sdp_body_str;
	osip_header_t *require_header=NULL;
	osip_message_t *prack_msg=NULL;

		
		
	osip_message_header_get_byname(ev->response, "require",
						   0, &require_header);	
	/*check PRACK requrie first */		

	if (require_header==NULL || require_header->hvalue==NULL){
		/* UNCOMPLIANT UA without a require header */
	}else if (0==osip_strncasecmp(require_header->hvalue, "100rel", 6)){
		g_message("ev->tid %d, send PRACK \n",ev->tid);
		/* need return PRACK */

			eXosip_lock();
			eXosip_call_build_prack(ev->tid,&prack_msg);
			eXosip_call_send_prack(ev->tid,prack_msg);
			eXosip_unlock();

		}


	if (lc->audiostream[ssid])
	{
		return ;
	}
	

	linphone_call_proceeding(lc, ssid, ev->cid, ev->did);
	lc->call[ssid]->tid = ev->tid;

	sdp_msg = eXosip_get_sdp_info(ev->response);

	if(sdp_msg==NULL)
		g_message("this ev not have sdp\n ");
	
	if( sdp_msg!=NULL )
	{
		/*	somebody reported a server who uses a 18x answer to establish mediastreams and play
			an informationnal message */
		//linphone_call_accepted(lc, ssid, ev->did, ev->sdp_body);
		sdp_message_free(sdp_msg);	

		linphone_call_accepted(lc, ssid, ev);
	}
	//else if( ev->status_code == 180 )
	else if( osip_message_get_status_code(ev->response))
	{
		
		if( lc->ringstream[ssid] )
		{
			g_message( "Already ringing !\n" ) ;
			return ;
		}

		g_message( "Remote ringing [%d][%d]...\n" , lc->chid , ssid ) ;
		if (lc->ringstream[ssid] == FALSE)
		{
			lc->ringstream[ssid] = TRUE;
			rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		}
	}
}

void linphone_update_and_display_registration_status( LinphoneCore *lc, LinphoneProxyConfig *cfg, ProxyRegStatus newStatus )
{
	FILE *fh;
	char buf[MAX_VOIP_PORTS * MAX_PROXY];	
	char buf_status;
	const char * pszPrompt;
	
	g_message( "SET REG status from %d to %d(%d:%d).\n", cfg ->reg_status, newStatus, lc ->chid, cfg ->index );
	
	// set new status 
	if( cfg ->reg_status == newStatus )
		return;
	
	cfg ->reg_status = newStatus;
	
	// decide display status and warning string 
	switch( newStatus ) {
	case REGSTATUS_REG_ING:					/* registering */

		buf_status = '2';
		pszPrompt = "registration_ongoing";
#ifdef CONFIG_RTK_VOIP_IP_PHONE
  		S2U_SipRegisterInd( lc ->chid, SIP_REGISTERING );
#endif
		break;
		
	case REGSTATUS_REG_SUCCESS:				/* register success */
	case REGSTATUS_REG_SUCCESS_EXPIRED:		/* register success but expired, so it is registering */
		buf_status = '1';
		pszPrompt = "registration_success";
#ifdef CONFIG_RTK_VOIP_IP_PHONE
  		S2U_SipRegisterInd( lc ->chid, SIP_REGISTERED );
#endif
		break;
		
	case REGSTATUS_INIT:
		buf_status = '0';
		pszPrompt = "no_register";
		break;
	case REGSTATUS_RESTART:
		buf_status = '4';
		break;
	case REGSTATUS_REG_FAIL_RETRY_S:		/* register failure (retry in short delay) */		
		buf_status = '5';
		break;
	case REGSTATUS_REG_FAIL:				/* register failure (don't retry) */
	case REGSTATUS_REG_FAIL_RETRY_L:		/* register failure (retry in long delay) */
	default:
		buf_status = '3';
		pszPrompt = "registration_faillure";
#ifdef CONFIG_RTK_VOIP_IP_PHONE
  		S2U_SipRegisterInd( lc ->chid, SIP_NOT_REGISTER );
#endif
		break;
	}
	
	// update status for web 
	fh = fopen(_PATH_TMP_STATUS, "r+");
	if (!fh) {
		printf("Warning: cannot open %s. Limited output.\n", _PATH_TMP_STATUS);
		printf("\nerrno=%d\n", errno);
	}
	else {
		if (fread(buf, sizeof(buf), 1, fh) == 0) {
			printf("%s(%d): The content of /tmp/status is NULL!!\n", pszPrompt, newStatus);
			printf("\nerrno=%d\n", errno);
		}
		else {
			buf[lc->chid * MAX_PROXY + cfg->index] = buf_status;
			fseek(fh, 0L, SEEK_SET);
			fwrite(buf, sizeof(buf), 1, fh);
		}
		fclose(fh);
	}	
}

void linphone_registration_faillure(LinphoneCore *lc,eXosip_event_t *ev)
{
	char *msg;
//printf("\n.........linphone_registration_faillure..........\n");
	int status_code = 0;
	const char *reason=NULL;
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	voipCfgProxy_t *proxy_cfg;
	proxy_cfg = &voip_ptr->proxies[0];

#ifdef FIX_REGISTER_TIME
	eXosip_reg_t *reg;
	osip_message_t *resp;
	osip_header_t *min_expire_header;
	int min_expire;
#endif
	LinphoneProxyConfig *cfg;
	int i;
	ProxyRegStatus regstatus = REGSTATUS_REG_FAIL;


	g_message("\n.........linphone_registration_faillure...chid %d .ev->rid %d ....\n",lc->chid,ev->rid);

	cfg = linphone_core_get_proxy_config_from_rid(lc, ev->rid);
	if(cfg==NULL){
		g_message("cfg is null \n");
		return;
	}
	g_return_if_fail(cfg != NULL);

	cfg->registered = FALSE;

	if (ev->response){
		status_code=osip_message_get_status_code(ev->response);
		reason=osip_message_get_reason_phrase(ev->response);
		g_message("linphone_registration_faillure status_code is %d\n",status_code);
	}



	//[SD6, bohungwu, exosip 3.5 integration */
	//switch(ev->status_code){
	switch( status_code ) {
	//]
		case 401:
		case 407:
			linphone_register_authentication_required(lc,ev);
			regstatus = ( cfg->auth_failed ? REGSTATUS_REG_FAIL : 
										REGSTATUS_REG_FAIL_RETRY_S );	
			break;
#ifdef FIX_REGISTER_TIME
		case 423: // Interval too brief
			eXosip_reg_find_id(&reg, ev->rid);
			if (reg == NULL)
				goto registration_faillure_display;
	
			resp = reg->r_last_tr->last_response;
			if (resp == NULL)
				goto registration_faillure_display;

			osip_message_header_get_byname(resp, MIN_EXPIRES, 0, &min_expire_header);
			if (min_expire_header == NULL || min_expire_header->hvalue == NULL)
				goto registration_faillure_display;

			min_expire = atoi(min_expire_header->hvalue);
			cfg->expires = min_expire;					// update expires by min-expires
			regstatus = REGSTATUS_REG_FAIL_RETRY_S;
			break;
#endif			

	//	case 403:	// Forbidden  
		case 405:	// Method Not Allowed 
		/*ericchung: when receive 403, disable to retry REGISTER */
//			regstatus = REGSTATUS_REG_FAIL_RETRY_L;
			regstatus= REGSTATUS_REG_FAIL;	
			rcm_timerCancel(lc->register_retry_timer);
			eXosip_register_remove(ev->rid);
			linphone_proxy_config_enableregister(lc->proxies[cfg->index],0); //disable proxy conf
			break;
			
		case 403:	// Forbidden  	//eric for some sip server, response 403, need re-try		
		default:
			regstatus = REGSTATUS_REG_FAIL;


			if(reason==NULL){
				g_message("Register no response timeout , proxy index %d\n",cfg->index);
				g_message("\r\n disable register bit\n");
				linphone_proxy_config_enableregister(cfg,0); //disable current proxy conf
				eXosip_register_remove(ev->rid);

				if(cfg->index==0){ 
					//first/main proxy register fail. check second proxy for register.
					if(( lc->proxies[1])&& (lc->proxies[1]->registered != TRUE) ){
						if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){
							/*case 1 ,if enable SIP_SERVER_REDUNDANCY, 
							  	when main proxy register fail.
								if second proxy have proxy setting and not register , try use second proxy						
							*/
							g_message("try to second proxy \n");
							linphone_core_set_default_proxy(lc, lc->proxies[1]);
							linphone_proxy_config_enableregister(lc->proxies[1],1); //enable proxy conf
							linphone_proxy_config_register(lc->proxies[1]);
						}else{
							/* if no enable SIP_SERVER_REDUNDANCY , just retry register to proxy */
							rcm_timerLaunch(lc->register_retry_timer,proxy_cfg->RegisterRetryInterval*1000);
						}

					}else{
						//case 2. second proxy already registered or no second proxy, only retry 1st proxy.

						g_message("\r\n retry main proxy after 60s\n");
						rcm_timerLaunch(lc->register_retry_timer,proxy_cfg->RegisterRetryInterval*1000);	//90sec

					}

				}else{
					/* second/backup proxy fail   */				
					g_message("\r\n retry main proxy after 60s\n");
					rcm_timerLaunch(lc->register_retry_timer,proxy_cfg->RegisterRetryInterval*1000);  //90sec			
				}								
			}			
//			linphonec_display_something(lc, msg);
			break;
	}

registration_faillure_display:

	// update and display regitration status 
	linphone_update_and_display_registration_status( lc, cfg, regstatus );
	UpdateLedDisplayMode( lc );

	return; 

#ifdef MULTI_PROXY
  #if 1	/* pkshih: new default proxy policy */
#if 0
  	if( lc->default_proxy && lc->default_proxy->index == cfg ->index )
  	{	/* registration failure is default proxy */
  	
		// choose default proxy (registered proxy) 
		for( i = 0; i < MAX_PROXY; i ++ )
			if( lc->proxies[i] && lc->proxies[i]->registered )
				break;
	
		if( i != MAX_PROXY ) {
			// adopt a registered proxy 
			linphone_core_set_default_proxy(lc, lc->proxies[i]);
		} else {
			// For LED
			rcm_tapi_sip_register(lc->chid, 0);
			linphone_core_set_default_proxy(lc, NULL);
UpdateLedDisplayMode( lc );
		}
  	}  	
#endif
  #else
	// check first active proxy
	for (i=0; i<MAX_PROXY; i++)
	{
		if (lc->proxies[i] && !lc->proxies[i]->auth_failed)
			break;
	}

	if (i == MAX_PROXY) // all proxy failed 
	{
		// For LED
		rcm_tapi_sip_register(lc->chid, 0);
UpdateLedDisplayMode( lc );
		linphone_core_set_default_proxy(lc, NULL);
	}
	else if (cfg->index == lc->default_proxy->index)
	{		
		// change to first active proxy (not registered proxy) 
		linphone_core_set_default_proxy(lc, lc->proxies[i]);
	}
  #endif
#else
	// For LED
	if (cfg->auth_failed)
		rcm_tapi_sip_register(lc->chid, 0);
#endif
UpdateLedDisplayMode( lc );
}

void linphone_registration_success( LinphoneCore * lc , eXosip_event_t * ev )
{
	LinphoneProxyConfig * cfg ;
	//[SD6, bohungwu, exosip 3.5 integration 
	//gchar * msg = g_strdup_printf( _( "Registration on %s sucessful." ) , ev->req_uri ) ;
	gchar * msg;
	osip_to_t *to;
	char *req_uri;
	osip_header_t *expire_header=NULL;
	osip_service_route_t * service_route=NULL;


//[SD6, ericchung, rcm integration
#if 0
//]
	to = osip_message_get_to(ev->request);
	osip_to_to_str(to, &req_uri);	
	msg = g_strdup_printf( _( "Registration on %s sucessful." ) , req_uri ) ;
	osip_free(req_uri);
	osip_to_free(to);
	//]

	linphonec_display_something(lc, msg);
	g_free( msg ) ;
//[SD6, ericchung, exosip 3.5 integration
#endif 
//]

	cfg = linphone_core_get_proxy_config_from_rid( lc , ev->rid ) ;
	g_return_if_fail(cfg != NULL);
	int i=0;
	if(ev->response!=NULL)
	{
		i=osip_message_get_service_route(ev->response,0,&service_route);
		if(service_route!=NULL)
		{
			//printf("osip_message_get_service_route=%d service_route->hvalue=%s\n", i, service_route->value);
			osip_list_init(&cfg->service_routes);
			i = osip_list_clone(&(ev->response)->service_routes,&cfg->service_routes,
						(int (*)(void *, void **)) &osip_service_route_clone);


		}
	}
	osip_message_get_expires(ev->request,0,&expire_header);

	if (expire_header!=NULL && atoi(expire_header->hvalue)!=0){

	cfg->auth_failed = FALSE ;
	cfg->registered = TRUE ;
#ifdef BYPASS_RELAY
	cfg->timeout_count = 0;
#endif

	// update and display regitration status 
	linphone_update_and_display_registration_status( lc, cfg, REGSTATUS_REG_SUCCESS );
	
#ifdef FIX_REGISTER_TIME
	//printf("%s ev->server_expires=%d\n", __FUNCTION__, ev->server_expires);
	if ( (ev->server_expires !=0) && (ev->server_expires != cfg->expires) ) 
	{
		//printf("%s: Change cfg->expires from %d to %d\n", __FUNCTION__, cfg->expires, ev->server_expires);
		cfg->expires = ev->server_expires;
	}
#endif	

	//linphone_core_set_default_proxy(lc, cfg);
	
	if(cfg->index==1){
		g_message("\r\n this is backup proxy \n");
		//ericchung: not need retry main proxy , follow e8c spec , 
		//if need retry main proxy, should enable the code as below:
		if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){
			rcm_timerLaunch(lc->register_retry_timer,60*1000);	//60sec
		}
		
	}else{//main proxy register success
		
		if(g_pVoIPCfg->rfc_flags & SIP_SERVER_REDUNDANCY){	
			if((lc->proxies[1]!=NULL) && (lc->proxies[1]->registered==TRUE)){
				//unregister backup proxy.
				linphone_proxy_send_unregister(lc->proxies[1]);
			}
		}
		/* cancel retry register timer */
		rcm_timerCancel(lc->register_retry_timer);
			
	}

	
	if( lc->default_proxy_index == cfg ->index || lc->default_proxy == NULL )
	{
		/* 1. current is defalt proxy in flash setting */
		/* 2. no default proxy */
		linphone_core_set_default_proxy(lc, cfg);
	}

	/* if register success, send subscript, option to sip server */
	rcm_linphone_message_to_proxy(lc,cfg);
	

//#ifdef MULTI_PROXY
#if 0 //ericchung : for E8C  case , remark it  ,TBD
  #if 1	/* pkshih: new default proxy policy */
	if( lc->default_proxy_index == cfg ->index || lc->default_proxy == NULL )
	{
		/* 1. current is defalt proxy in flash setting */
		/* 2. no default proxy */
		linphone_core_set_default_proxy(lc, cfg);
	}
  #else
	if (lc->default_proxy == NULL)
	{
		linphone_core_set_default_proxy(lc, cfg);
	}
	else if (lc->default_proxy->index == lc->default_proxy_index)
	{
		if( lc->default_proxy->registered == TRUE ) {
			// default proxy in flash is first priority
		} else {
			// default proxy is not registered, so use current one temporarily. 
			linphone_core_set_default_proxy(lc, cfg);
		}
	}
	else if (cfg->index != lc->default_proxy->index)
	{
		// change to first active proxy 
		linphone_core_set_default_proxy(lc, cfg);
	}
  #endif
#endif
	// For LED
	    rcm_tapi_sip_register(lc->chid, 1);
	}else{/*expire is 0 , for unregister */
	
		cfg->registered = FALSE ;

		linphone_update_and_display_registration_status( lc, cfg, REGSTATUS_INIT );
		rcm_tapi_sip_register(lc->chid, 0);


	}
	
	UpdateLedDisplayMode( lc );
}

int linphone_find_chid_ssid(TAPP *pApp, int cid, int *p_chid, int *p_ssid)
{
	int i, j;
	LinphoneCore *lc;

	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		lc = &pApp->ports[i];
		for (j=0; j<MAX_SS; j++)
		{
			if (lc->call[j] &&
				lc->call[j]->cid == cid)
			{
				*p_chid = i;
				*p_ssid = j;
				return 0;
			}
		}
	}

	return -1;
}

int linphone_find_chid_ssid_terminated(TAPP *pApp, int cid, int *p_chid, int *p_ssid)
{
	int i, j;
	LinphoneCore *lc;

	for (i=0; i<g_MaxVoIPPorts; i++)
	{
		lc = &pApp->ports[i];
		for (j=0; j<MAX_SS; j++)
		{
			if (lc->call_terminated[j] &&
				lc->call_terminated[j]->cid == cid)
			{
				*p_chid = i;
				*p_ssid = j;
				return 0;
			}
		}
	}

	return -1;
}

int GetChannelFromRequest(TAPP *pApp, eXosip_event_t *ev)
{
	// TODO: check request URI
	LinphoneCore *lc;
	int i;
	GList *elem;
	osip_from_t *from,*to;
	char *tmp;
	char *to_username=NULL;
	voipCfgPortParam_t *voip_ptr;
	voipCfgProxy_t *proxy;


	
	from=osip_message_get_from(ev->request);
	to=osip_message_get_to(ev->request);
	
	osip_from_to_str(from,&tmp);
	//printf("get request from is %s\n",tmp);
	osip_free(tmp);

	osip_from_to_str(to,&tmp);
	//printf("get request to is %s\n",tmp);
	osip_free(tmp);

	//username=osip_uri_get_username(ev->request->to->url);
//	printf("\r\n aa get username is %s\n",username);
		
	to_username=osip_uri_get_username(&ev->request->req_uri);
	//printf("\r\n to_username is %s\n",to_username);
	if((to_username==NULL)|| (strlen(to_username)==0)){
		voip_ptr = &g_pVoIPCfg->ports[0];
		if ((voip_ptr->fxsport_flag & FXS_REJECT_DIRECT_IP_CALL))
//		if (voip_ptr->direct_ip_call == 0)
		{
			g_warning("reject direct ip call\n");
			eXosip_lock();
			//[SD6, bohungwu, exosip 3.5 integration
			//eXosip_answer_call(did, 603, NULL);
			eXosip_call_send_answer(ev->tid, 603, NULL);
			//]
			eXosip_unlock();
			return -1;
		}
		
		return 0; //always use chid 0 , for direct ip call
	}


/* check internal call */
/*
case 1: fxs internall call
      from_url: fxs1@ip
      to_url : fxs2@ip

case 2: fxs dial to fxo (2 stage)
       from_url: fxstofxo@ip
       to_url: fxo@ip

case 3: fxs dial pstn_route_prefix+number to fxo (1 stage)
        from_url: fxstofxo@ip
        to_url: number@ip

*/

	 if (strcmp(to_username, "fxs2") == 0) {
	 	g_message("\n match fxs2, dial to chid 1\n");
		//return chid 1 => fxs 2
	 	return 1;

	 }else if(strcmp(to_username, "fxs1") == 0) {
		 g_message("\n match fxs1, dial to chid 0\n");
		return 0;
	 }
	 else if(strcmp(from->url.username,"fxstofxo")==0){
		g_message(" dial to fxo \n");
		return RTK_VOIP_DAA_CH_OFFSET(g_VoIP_Feature);
	 }

	for (i=0; i<g_MaxVoIPPorts; i++){
			
		lc=&pApp->ports[i];
		
		for(elem=linphone_core_get_proxy_config_list(lc);elem!=NULL;elem=g_list_next(elem)){
			
			LinphoneProxyConfig *cfg=(LinphoneProxyConfig*) elem->data;
			
			if (cfg->registered)//already register 
			{//match
				voip_ptr = &g_pVoIPCfg->ports[cfg->lc->chid];
				proxy = &voip_ptr->proxies[cfg->index];
				//	printf("check chid %d,proxy index %d\n",cfg->lc->chid,cfg->index);
					
				//printf("check  proxy->number %s\n",proxy->number);
				//printf("check  proxy->login_id %s\n",proxy->login_id);
		
				//printf("check proxy->password %s\n",proxy->password);

				if (0 == strcmp(proxy->number, to_username)){
						g_message("match  proxy->number %s chid %d\n",proxy->number,cfg->lc->chid);
						return cfg->lc->chid;

				}
//				return (LinphoneProxyConfig*)elem->data;
			}
		}




	}

	g_message("not match any number \n");
	return -1;



	
	//printf("*******GetChannelFromRequest nodify modify \n");

}

int GetChIDfromOptionsTid(TAPP *pApp, eXosip_event_t *ev)
{
	LinphoneCore *lc;
	int i;

	for (i=0; i<g_MaxVoIPPorts; i++){	
		lc=&pApp->ports[i];
		if(lc->options_tid==ev->tid){
			return lc->chid;
		}			
	}

//	printf("GetChIDfromOptionsTid no match any tid \n");
	return -1;

}
#ifdef TIMECMP
#error "TIMECMP has defined!?"
#endif
#define TIMECMP(h1, m1, cmp, h2, m2) ((h1) == (h2) ? (m1) cmp (m2) : (h1) cmp (h2))
int CheckDND(int chid)
{
	if (g_pVoIPCfg->ports[chid].dnd_mode == 2)	// dnd always
	{
		return 1;
	}
	else if (g_pVoIPCfg->ports[chid].dnd_mode == 1)	// dnd period
	{
		time_t cur_time;
		struct tm *cur_tm;

		time(&cur_time);
		cur_tm = localtime(&cur_time);
		if (TIMECMP(
			g_pVoIPCfg->ports[chid].dnd_to_hour, g_pVoIPCfg->ports[chid].dnd_to_min, >,
			g_pVoIPCfg->ports[chid].dnd_from_hour, g_pVoIPCfg->ports[chid].dnd_from_min))
		{
			return TIMECMP(cur_tm->tm_hour, cur_tm->tm_min, >, 
				g_pVoIPCfg->ports[chid].dnd_from_hour, g_pVoIPCfg->ports[chid].dnd_from_min) &&
				TIMECMP(cur_tm->tm_hour, cur_tm->tm_min, <, 
				g_pVoIPCfg->ports[chid].dnd_to_hour, g_pVoIPCfg->ports[chid].dnd_to_min);
		}
		else
		{
			return !TIMECMP(cur_tm->tm_hour, cur_tm->tm_min, >, 
				g_pVoIPCfg->ports[chid].dnd_from_hour, g_pVoIPCfg->ports[chid].dnd_from_min) &&
				TIMECMP(cur_tm->tm_hour, cur_tm->tm_min, <, 
				g_pVoIPCfg->ports[chid].dnd_to_hour, g_pVoIPCfg->ports[chid].dnd_to_min);
		}
	}

	return 0;
}
#undef TIMECMP

int linphone_process_call(TAPP *pApp, int *p_chid, int *p_ssid, eXosip_event_t *ev)
{
	LinphoneCore *lc;
#ifdef CONFIG_RTK_VOIP_DIALPLAN
	const voipCfgPortParam_t *voip_ptr;
	extern void CallForwardByDialPlan( const LinphoneCore *lc,
							const unsigned char *pUserUrl,
							unsigned char *pSipUrl,
							const unsigned char *pszDot );
#endif

	if (linphone_find_chid_ssid(pApp, ev->cid, p_chid, p_ssid) == 0)
		return 0;


g_message("\r\n 111 linphone_process_call p_chid[%d]ssid[%d]\n",*p_chid,*p_ssid);

#ifdef ATTENDED_TRANSFER
		if (ev->rc != NULL)
		{
			if (linphone_find_chid_ssid(pApp,ev->rc->c_id , p_chid, p_ssid) == 0)
			{
				//step 1. terminate the 1st call				
				rcm_tapi_SetRtpSessionState( *p_chid , *p_ssid , rtp_session_inactive ) ;
				linphone_core_terminate_dialog(&pApp->ports[*p_chid], *p_ssid, NULL);
				//the call might be holded by remote, cancel the holding sound
				rcm_tapi_SetPlayTone(*p_chid, *p_ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_BOTH);
				SetSessionState(*p_chid , *p_ssid , SS_STATE_IDLE ) ;
				SetActiveSession( *p_chid , *p_ssid, FALSE ) ;
				*p_chid = GetChannelFromRequest(pApp, ev);//assign chid
				*p_ssid = (p_ssid) ? 0 : 1;
				return 0;
			}
		}
#endif


	// couldn't find match call
	// 1. new call: pre-process if need
	// 2. error
	//[SD6, bohungwu, exosip 3.5 integration
	//if (ev->type == EXOSIP_CALL_NEW)
	if (ev->type == EXOSIP_CALL_INVITE)
	//]
	{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		unsigned char szForward[ 100 ] = { 0 };
		const unsigned char *forward = szForward;
#else
		char forward[100] = { 0 };
#endif
		int active_session;

		*p_chid = GetChannelFromRequest(pApp, ev);
        if(*p_chid<0){
            g_message("No channel number is matched from request\n");
            return -1;
        }
        
		lc = &pApp->ports[*p_chid];
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		voip_ptr = &g_pVoIPCfg->ports[ lc ->chid ];
#endif
		
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		if( voip_ptr ->uc_forward_enable == 1 && voip_ptr ->uc_forward[ 0 ] )
		{
			// uncoditional forward
			g_message("unconditional forward\n");
			forward = voip_ptr ->uc_forward;
		}
#else
		if (lc->sip_conf.uc_forward_enable == 1 && lc->sip_conf.uc_forward[0])
		{
			// uncoditional forward
			g_message("unconditional forward\n");
			strcpy(forward, lc->sip_conf.uc_forward);
		}
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
		else 
		{
			int bBusy;

#ifdef ATTENDED_TRANSFER
			// rock: force terminate call transfer if last transfer not terminated
			if (GetSysState(lc->chid) == SYS_STATE_TRANSFER_ONHOOK)
			{
				int i;

				for (i = 0; i<MAX_SS; i++)
					linphone_core_terminate_dialog(lc, i ,NULL);

				SetSysState(lc->chid, SYS_STATE_IDLE);
			}
#endif

			bBusy = CheckDND(lc->chid);
			if (bBusy == 0)
			switch (GetSysState(lc->chid))
			{
			case SYS_STATE_IDLE:
				*p_ssid = GetIdleSession(lc->chid);
				break;
			case SYS_STATE_CONNECT:
				active_session = GetActiveSession(lc->chid);
//ericchung : baudtac project, when enable busy forwarding, busy_forward priority > call waiting 
// todo : if uc_forward and busy_forward both enable ... need check behavior.

#ifdef CONFIG_RTK_VOIP_DIALPLAN
				if( voip_ptr ->busy_forward_enable && voip_ptr ->busy_forward[ 0 ] )
				{
					bBusy = 1;
				}
				else
#else
				if (lc->sip_conf.busy_forward_enable && lc->sip_conf.busy_forward[0])
				{
					bBusy = 1;				
				}
				else
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
				if ((!lc->parent->bAutoAnswer) && 
					(lc->sip_conf.call_waiting_enable) &&
					(lc->call[active_session]->state == LCStateAVRunning) &&
					//(lc->chid < RTK_VOIP_SLIC_NUM(g_VoIP_Feature)) &&
					RTK_VOIP_IS_SLIC_CH( lc->chid, g_VoIP_Feature ) &&
#ifdef CONFIG_RTK_VOIP_DIALPLAN
					(voip_ptr->uc_forward_enable != 2)
#else
					(lc->sip_conf.uc_forward_enable != 2)
#endif
					)
					*p_ssid = GetIdleSession(lc->chid);
				else
					bBusy = 1;
				break;
			case SYS_STATE_RING:
				bBusy = 1;
				break;
			case SYS_STATE_DAA_CONNECT:
				if ((g_VoIP_Feature & DAA_TYPE_MASK) == REAL_DAA_NEGO)
				{
					*p_ssid = GetIdleSession(lc->chid);
				}
				else
				{
					bBusy = 1;
				}
				break;
			default:
				bBusy = 1;
				break;
			}

			if (bBusy || (*p_ssid) < 0)
			{
				// I am busy 
#ifdef CONFIG_RTK_VOIP_DIALPLAN
				if( voip_ptr ->busy_forward_enable && voip_ptr ->busy_forward[ 0 ] )
				{
					// busy forward
					g_message("busy forward\n");
					forward = voip_ptr ->busy_forward;
				}
#else
				if (lc->sip_conf.busy_forward_enable && lc->sip_conf.busy_forward[0])
				{
					// busy forward
					g_message("busy forward\n");
					strcpy(forward, lc->sip_conf.busy_forward);
				}
#endif /* CONFIG_RTK_VOIP_DIALPLAN */
				else
				{
					// repsonse busy
					g_warning("busy here [%d]\n", *p_chid);
					eXosip_lock();
					//[SD6, bohungwu, exosip 3.5 integration
					//eXosip_answer_call(ev->did, 486, NULL);
					eXosip_call_send_answer(ev->tid, 486, NULL);
					//]
					eXosip_unlock();
					/*+++++add by Jack for adding busy call log+++++*/
					{
						time_t t;
						LinphoneCallLog *cl=g_new0(LinphoneCallLog,1);
						/* SD6, bohungwu, exosip 3.5 integration */
						osip_from_t * from_url = NULL ;
						osip_from_t * to_url = NULL ;
						/* Ends here*/

						cl->lc=lc;
						cl->dir=LinphoneCallIncoming;
						cl->status=LinphoneCallMissed;
						time(&t);
						cl->start_date=g_strdup(ctime(&t));
						//cl->from=g_strdup(ev->local_uri);
						from_url = osip_message_get_from(ev->request);
						if(from_url != NULL) {
							osip_from_to_str(from_url, &(cl->from));		
						}

						//cl->to=g_strdup(ev->remote_uri);
						to_url = osip_message_get_to(ev->request);
						if(to_url != NULL) {
							osip_from_to_str(from_url, &(cl->to));

						}
						

						lc->call_logs=g_list_append(lc->call_logs,(gpointer)cl);
						if (g_list_length(lc->call_logs) > lc->max_call_logs){
							GList *elem;
							elem=g_list_first(lc->call_logs);
							linphone_call_log_destroy((LinphoneCallLog*)elem->data);
#ifdef FIX_MEMORY_LEAK
							lc->call_logs=g_list_delete_link(lc->call_logs,elem);
#else
							lc->call_logs=g_list_remove_link(lc->call_logs,elem);
#endif
						}
					}
			/*-----end-----*/
					return -1;
				}
			}
		}

		if (forward[0])
		{
          	osip_message_t *msg;
          	int code=302;

#ifdef CONFIG_RTK_VOIP_DIALPLAN
			/* 
			 * Catenate forward string from 'forward' to 'szForward', 
			 * and the string will become 'sip: xxxxx@xxxxx.xxxx.xxx'.
			 */
			CallForwardByDialPlan( lc, forward, szForward, "." );
			
			/* Now, point 'forward' to 'szForward' */
			forward = szForward;			
#endif /* CONFIG_RTK_VOIP_DIALPLAN */

			g_message("forward to %s", forward);

 			/* response 302, Moved Temporarily (redirect) */ 
          	eXosip_lock();
          	eXosip_call_build_answer(ev->tid,code,&msg);
          	osip_message_set_contact(msg,forward);
          	eXosip_call_send_answer(ev->tid,code,msg);
          	eXosip_unlock();

			return -1;
		}
		else
		{
			// got chid & ssid, ringing and wait answer
			return 0;
		}
	}
	else if (ev->type == EXOSIP_CALL_RELEASED)
	{
		#if 0 //eric mark 
		*p_chid = GetChannelFromRequest(pApp, ev);

		lc = &pApp->ports[*p_chid];
		g_message("+++++@@@DEBUG:EXOSIP_CALL_RELEASED,state:%d+++++\n",GetSysState(lc->chid));

		/*++Modify by Jack Chan 29/01/07 for VOIP.
			free the dialog that is not freed in eXosip_terminate_call()++*/
		/*if( NULL != ev->jd ){
			osip_dialog_free(ev->jd->d_dialog);
			ev->jd->d_dialog = NULL;
		}*/
		/*--end--*/
		// its not ok!
		// session != transaction, release later always
		#endif
		return -1;
	}else if(ev->type == EXOSIP_MESSAGE_NEW){

	
		*p_chid = GetChannelFromRequest(pApp, ev);
		if(*p_chid!=-1)
			lc = &pApp->ports[*p_chid];
		else
		return -1;

		*p_ssid = GetIdleSession(lc->chid);
		
		return 0;

	}
	else
	{
		g_warning("invalid event (%d): cid = %d, did = %d\n",
			ev->type, ev->cid, ev->did);
		return -1;
	}
}

char *tolower_clrspace(char *str)
{
	int i, j;
	char *new_str;
	
	new_str = osip_strdup(str);
	if (new_str)
	{
		for (i=0, j=0; str[i]; i++)
		{
			if (str[i] == ' ') continue;
			new_str[j++] = tolower(str[i]);
		}
		new_str[j] = 0;
	}

	return new_str;
}

void linphone_core_handle_reinvite(LinphoneCore *lc, int chid, int ssid, eXosip_event_t *ev)
{
	char *answer;
	sdp_context_t *ctx;
	int status;
	sdp_message_t *sdp_msg;
	char *sdp_body_str;
	osip_message_t *msg=NULL;
	int i;
	int codec_changed=0;
	voipCfgProxy_t *proxy_cfg = &g_pVoIPCfg->ports[lc->chid].proxies[lc->default_proxy_index];
	char szbuf[LINE_MAX_LEN];
	char * sdpmesg ;

	//eric add , display audio_params
	StreamParams *audio_params;
	StreamParams *t38_params;
	LinphoneCall *call=lc->call[ssid];
	unsigned long old_audio_ip, old_t38_ip;
	StreamParams old_audio_params;
	StreamParams old_t38_params;
	
	sdp_msg = eXosip_get_sdp_info(ev->request);//receive invite is request

	if(sdp_msg == NULL){
		g_message("reinvite no sdp \n");

		eXosip_lock();
		eXosip_call_build_answer(ev->tid,200,&msg);
		//configure contact header
		if(lc->call[ssid]->proxy!=NULL){
		_osip_list_set_empty(&msg->contacts,(void (*)(void*))osip_contact_free);
		osip_message_set_contact(msg,lc->call[ssid]->proxy->contact);
		}

		if(proxy_cfg->SessionUpdateTimer!=0){
			osip_message_set_supported(msg, "timer");
		}
			
		g_message("response 200k offer for re-invite chid %d ssid %d\n",chid,ssid);
		ctx = lc->call[ssid]->sdpctx;
		sdpmesg = sdp_context_get_offer( ctx ) ;

		osip_message_set_body(msg, sdpmesg, strlen(sdpmesg));
		osip_message_set_content_type(msg, "application/sdp");
		rcm_osip_message_set_content_length(msg, strlen(sdpmesg));


		eXosip_call_send_answer(ev->tid,200,msg);
		eXosip_unlock();

		return;
	}

    sdp_message_free(sdp_msg);	//Alex Hung, 20111114, valgrind

	/*copy old sdp params */
	audio_params = &call->audio_params ;
	t38_params = &call->t38_params;

	memset(&old_audio_params, 0, sizeof(old_audio_params));
	memcpy(&old_audio_params,audio_params,sizeof(StreamParams));
	memset(&old_t38_params, 0, sizeof(old_audio_params));
	memcpy(&old_t38_params,t38_params,sizeof(StreamParams));

	ctx = lc->call[ssid]->sdpctx;

	//get answer(sdpbody) for return response
	answer = rcm_sdp_context_get_answer_by_ev(ctx, ev);

	status = sdp_context_get_status(ctx);
	g_message("+++++DEBUG:linphone_core_handle_reinvite:negoc_status%d+++++\n",ctx->negoc_status);

	if(488 == status || 415 == status){
		eXosip_lock();
		eXosip_call_send_answer(ev->tid,status,NULL);
		eXosip_unlock();
	g_message("+++++DEBUG:linphone_core_handle_reinvite:status%d+++++\n",status);
		return;
	}
	if (answer == NULL)
	{
		g_warning("get sdp answer failed [%d] [%d] [%d]\n", 
			ev->type, ev->cid, ev->did);
		return;
	}

#ifdef NEW_STUN
#if 0
	// stop media for stun
	if (g_pVoIPCfg->ports[lc->chid].stun_enable)
	{
		rcm_tapi_SetRtpSessionState(chid, ssid, rtp_session_inactive);
	}
#endif
#endif

#if 0 //STUN
	/* Mandy: Keep NAT entry alive */
	if (lc->net_conf.nat_address!=NULL && lc->net_conf.use_nat)
	{
		char *port;

		port = (char*)osip_malloc(10);
		if (lc->call[ssid]->t38_params.initialized)
		{
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_T38);
			g_message("SDP get answer: The nat_t38_port is %d.\n", lc->net_conf.nat_t38_port);
			snprintf(port, 9, "%i", lc->net_conf.nat_t38_port);
		}
		else
		{
			linphone_core_set_use_info_for_stun(lc, ssid, STUN_UPDATE_RTP);
			g_message("SDP get answer: The nat_voice_port is %d.\n", lc->net_conf.nat_voice_port[ssid]);
			snprintf(port, 9, "%i", lc->net_conf.nat_voice_port[ssid]);
		}
		sdp_message_m_port_set(ctx->answer, 0, port);
		//[SD6, ericchung, rcm integration
		//sdp_message_to_str(ctx->answer, &answer);
		//osip_free(ctx->answerstr);
		//ctx->answerstr = answer;
		//]
	}
#endif

	//[SD6, bohungwu, exosip 3.5 integration
	//exoisp 3.5 doesn't support EXOSIP_CALL_UPDATE, EXOSIP_CALL_HOLD and EXOSIP_CALL_OFFHOLD events

	g_message("ctx->remotedir is %d\n",ctx->remote_dir);

	/*re-assign sdp point */
	audio_params = &call->audio_params ;
	t38_params = &call->t38_params;

//	printf("new audio ip  is %ld\n",inet_addr(audio_params->remoteaddr));

	if(memcmp(&old_audio_params,audio_params,sizeof(StreamParams))!=0){
		printf("\r\n audio codec change \n");
		codec_changed=1;
	}
	if(memcmp(&old_t38_params,t38_params,sizeof(StreamParams))!=0){
		printf("\r\n t38 codec change \n");
		codec_changed=1;
	}	

#if 0
	// TODO: fax handling
	if (ev->type == EXOSIP_CALL_FAXON)
	{
		g_message("EXOSIP_CALL_FAXON[%d][%d]\n", chid, ssid);
		lc->call[ssid]->state = LCStateBeFax;
		lc->call[ssid]->faxflag = FMS_BEFAX_RUN;
	}
	else if (ev->type == EXOSIP_CALL_FAXOFF)
	{
		g_message("EXOSIP_CALL_FAXOFF[%d][%d]\n", chid, ssid);
		lc->call[ssid]->state = LCStateBeFax;
		lc->call[ssid]->faxflag = FMS_IDLE;
	}
	else if (ev->type == EXOSIP_CALL_MODEM)
	{
		g_message("EXOSIP_CALL_MODEM[%d][%d]\n", chid, ssid);
		lc->call[ssid]->state = LCStateBeFax;
		lc->call[ssid]->faxflag = FMS_BEMODEM_RUN;
	}
#endif

	if(ctx->remote_dir == SalStreamSendRecv){ //reinvite is call update or call resume.
		// update for fax issue or other
		g_message("EXOSIP_CALL_REINVITE[%d][%d]\n", chid, ssid);
		//lc->call[ssid]->state = LCStateUpdate;
		g_message("\r\n 001 should change linphone to LCStateUpdate ???\n");

			osip_negotiation_sdp_message_put_media_mode(ctx->answer, MEDIA_SENDRECV);


			if (lc->call[ssid]->state == LCStateAVRunning)
			{
			if(codec_changed){
			SetCallState(lc->chid,ssid,&(lc->call[ssid]->state),LCStateUpdate);
			}
			g_message("\r\nlinphone a linphone_core_handle_reinvite chid[%d],ssid[ssid], status %d\n",chid,ssid,lc->call[ssid]->state );
			
			}
			else if (lc->call[ssid]->state != LCStateBeResume)
			{
				lc->call[ssid]->state = LCStateBeResume;
			SetCallState(lc->chid,ssid,&(lc->call[ssid]->state) ,LCStateBeResume);
			g_message("\r\nlinphone a linphone_core_handle_reinvite chid[%d],ssid[ssid], status %d\n",chid,ssid,lc->call[ssid]->state );
		}
	}else if(ctx->remote_dir == SalStreamSendOnly){ //reinvite for call hold
		g_message("EXOSIP_CALL_REINVITE for CALL_HOLD[%d][%d]: state = %d\n", chid, ssid, lc->call[ssid]->state);
		osip_negotiation_sdp_message_put_media_mode(ctx->answer, MEDIA_RECVONLY);
		
		if (lc->call[ssid]->state != LCStateBeHold && 
			lc->call[ssid]->state != LCStateBeHoldACK)
		{
			//lc->call[ssid]->state = LCStateBeHold;
			SetCallState(lc->chid,ssid,&(lc->call[ssid]->state),LCStateBeHold);
		}
	}else if(ctx->remote_dir == SalStreamInactive){ //reinvite for call hold
		g_message("anson: EXOSIP_CALL_REINVITE for CALL_HOLD[%d][%d]: state = %d\n", chid, ssid, lc->call[ssid]->state);
		osip_negotiation_sdp_message_put_media_mode(ctx->answer, MEDIA_INACTIVE);
	
		if (lc->call[ssid]->state != LCStateBeHold && 
			lc->call[ssid]->state != LCStateBeHoldACK)
		{
			//lc->call[ssid]->state = LCStateBeHold;
			SetCallState(lc->chid,ssid,&(lc->call[ssid]->state),LCStateBeHold);
		}
	}
			
	if (ctx->answerstr)
	{
		osip_free(ctx->answerstr);
		sdp_message_to_str(ctx->answer, &answer);
		ctx->answerstr = answer;
	}

	lc->call[ssid]->tid=ev->tid;
	i = eXosip_call_build_answer(lc->call[ssid]->tid, 200, &msg);
	if(lc->call[ssid]->proxy!=NULL){
	_osip_list_set_empty(&msg->contacts,(void (*)(void*))osip_contact_free);
	osip_message_set_contact(msg,lc->call[ssid]->proxy->contact);
	}
	//eric.chung ,if support session timer 


	if(proxy_cfg->SessionUpdateTimer!=0){
		//for re-invite response , not need here code
		//sprintf(szbuf, "%d", proxy_cfg->SessionUpdateTimer);
		//osip_message_set_header(msg, "Session-expires",  szbuf);
		//osip_message_set_header(msg, "Min-SE", "90");
	osip_message_set_supported(msg, "timer");
	}


	osip_message_set_allow(msg, "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO, UPDATE");


	eXosip_lock();
	if(i != 0)
	{
		eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
	}
	else
	{
		i = osip_message_set_body(msg, ctx->answerstr, strlen(ctx->answerstr));
		if(i == 0)
		{
			i = osip_message_set_content_type(msg, "application/sdp");
			if(i == 0)
			{
				char size[7];

				sprintf(size,"%i",strlen(ctx->answerstr));
				i = osip_message_set_content_length(msg, size);
				if(i == 0)
				{
							eXosip_call_send_answer(lc->call[ssid]->tid, 200, msg) ;
				}
				else
				{
					eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
					osip_message_free(msg);
				}
			}
			else
			{
				eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
				osip_message_free(msg);
			}
		}
		else
		{
			eXosip_call_send_answer(lc->call[ssid]->tid, 400, NULL);
			osip_message_free(msg);
		}
	}
	eXosip_unlock();


}

void linphone_core_process_event(TAPP *pApp, eXosip_event_t * ev )
{
	int chid = -1, ssid = -1;
	LinphoneCore *lc = NULL;
	int bTransferring = 0;
	int ret=0;
	sdp_message_t *sdp_msg;
	char *sdp_body_str;
	
	g_message("\r\n linphone_core_process_event ev->type is %d ev->rid %d\n",ev->type,ev->rid);

	if(pApp==NULL){
		g_warning("\r\n linphone_core_process_event pApp is null\n");
		return;
	}

	g_message("get ev->type %d\n",ev->type);
	g_message("get ev->tid %d\n",ev->tid);
	g_message("get ev->rid %d\n",ev->rid);
	g_message("get ev->sid %d\n",ev->sid);
	g_message("get ev->nid %d\n",ev->nid);
	g_message("get ev->did %d\n",ev->did);
	g_message("get ev->cid %d\n",ev->cid);

	g_message("get ev->textinfo %s\n",ev->textinfo);

	if (ev->type >= EXOSIP_REGISTRATION_NEW && 
		ev->type <= EXOSIP_REGISTRATION_TERMINATED)
	{
		int i;
		LinphoneProxyConfig *cfg;
	
		for (i=0; i<g_MaxVoIPPorts; i++)
		{
			cfg = linphone_core_get_proxy_config_from_rid(&pApp->ports[i], ev->rid);
			if (cfg != NULL)
			{
				g_message("find lc index %d cfg->rid is %d\n",i, cfg->rid);
				lc = &pApp->ports[i];
				g_message("find lc->chid %d\n", lc->chid);
				break;
			}
		}

		if (lc == NULL)
		{
			//[SD6, ericchung, exosip 3.5 integration
			//eXosip_event_free(ev);
			//]
			return;
		}
		else {
			g_message("222 find lc-> chid %d\n", lc->chid);
		}
	}
	//[SD6, bohunguw, exosip 3.5 integration
	//EXOSIP_CALL_REFER_REQUESTFAILURE and EXOSIP_INFO_NEW are not supported.
	//else if ((ev->type >= EXOSIP_CALL_NOANSWER && ev->type <= EXOSIP_CALL_REFER_REQUESTFAILURE)
	//	|| (ev->type == EXOSIP_INFO_NEW))
	else if ((ev->type >= EXOSIP_CALL_NOANSWER && ev->type <= EXOSIP_CALL_TIMEOUT)
		|| (ev->type == EXOSIP_MESSAGE_NEW))
	{
//[SD6, ericchung, exosip 3.5 integration
#if 0
		if (linphone_process_call(pApp, &chid, &ssid, ev) < 0)
		{
			// rock: handle BYE-auth after On-Hook
			if (linphone_find_chid_ssid_terminated(pApp, ev->cid, &chid, &ssid) == 0)
			{
				lc = &pApp->ports[chid];
				if ((ev->type == EXOSIP_CALL_REQUESTFAILURE) &&
				//[SD6, bohungwu, exosip 3.5 integration
				//	(ev->status_code == 401 || ev->status_code == 407))
					(osip_message_get_status_code(ev->response) == 401 || osip_message_get_status_code(ev->response) == 407))
				//]
				{
					//[SD6, bohungwu, exosip 3.5 integration
					//if (linphone_call_authentication_required(lc, lc->call_terminated[ssid]) != 0)
					if (linphone_call_authentication_required(lc, lc->call_terminated[ssid], ev) != 0)
					//]
						linphone_core_close_terminate_call(lc, ssid);
				}
				//[SD6, bohunguw, exosip 3.5 integration
				//else if (ev->type == EXOSIP_CALL_EXIT)
				else if (ev->type == EXOSIP_CALL_CLOSED)
				//]
				{
					linphone_core_close_terminate_call(lc, ssid);
				}
			}

			eXosip_event_free( ev ) ;
			return;
		}

		if (chid != ev->nPort)
			g_message("call is not match with event!\n");
		else
			lc = &pApp->ports[chid];
//[SD6, ericchung, exosip 3.5 integration
#endif //#if 0

	}
#ifdef SIP_PING
	else if (ev->type == EXOSIP_PING_SUCCESS || ev->type == EXOSIP_PING_CLOSED)
	{
		lc = &pApp->ports[ev->nPort];
	}
#endif
#if 0
	if (lc == NULL)
	{
		g_warning("counldn't find match call\n");
	}
#endif
	switch( ev->type )
	{
		case EXOSIP_CALL_ANSWERED : /*receive 200ok for outgoing call (INVITE) , must return ACK */

			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n EXOSIP_CALL_ANSWERED linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;
			
			lc = &pApp->ports[chid];

			g_message( "EXOSIP_CALL_ANSWERED[%d][%d]\n" , chid , ssid ) ;

			if (GetSysState(lc->chid) == SYS_STATE_TRANSFERRING)
				bTransferring = 1;

			// rock add:
			// avoid auto-answer skip ringing problem
			if (lc->ringstream[ssid] == FALSE) {
				linphone_call_proceeding(lc, ssid, ev->cid, ev->did);
				lc->call[ssid]->tid = ev->tid;
			}
			rcm_exosip_call_ack(lc,ssid,ev);

		#ifdef CHECK_RESOURCE
		//run here
			/* [SD6, bohungwu, exosip 3.5 integration */
			{
#if 0 //ericchung , not need sdp here.	
				sdp_message_t *sdp_msg;
				char *sdp_body_str;

				sdp_msg = eXosip_get_sdp_info(ev->response);
				if(sdp_msg!=NULL){
					sdp_message_to_str(sdp_msg, &sdp_body_str);
					sdp_message_free(sdp_msg);	
				}else{
					printf("no sdp !!\n");
				}
#endif
				//if (linphone_call_accepted(lc, ssid, ev->did, ev->sdp_body) == -1)
				//[SD6, ericchung, exosip 3.5 integration
				//if (linphone_call_accepted(lc, ssid, ev->did, sdp_body_str) == -1)
				if (linphone_call_accepted(lc, ssid, ev) == -1)
				{
					//free(sdp_body_str);
				break;
				}
			}
			//]

		#else
			//[SD6, ericchung, exosip 3.5 integration
			//linphone_call_accepted(lc, ssid, ev->did, ev->sdp_body);
			//old api
			linphone_call_accepted(lc, ssid, ev);
		#endif
/*+++added by Jack for Planet transfer+++*/
#ifdef PLANET_TRANSFER
			if(eXosip.bIVR_transfer){
				if(GetSysState(lc->chid) == SYS_STATE_TRANSFER_EDIT){
					int another_session;
			
					another_session = ssid ? 0 : 1;
					rcm_tapi_SetPlayTone(lc->chid , another_session, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				}
			}
#endif /*PLANET_TRANSFER*/
			/*-----end-----*/
		#if defined(CONFIG_RTK_VOIP_IP_PHONE) || defined(ATTENDED_TRANSFER)
			if (bTransferring)
			{
				int hs; // hold session

				hs = (ssid == 0) ? 1 : 0;
				if (lc->call[hs] && lc->call[hs]->state != LCStateInit)
				{
					//[SD6, bohugnwu, exosip 3.5 integration
					//eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 200 OK");
					rcm_eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 200 OK");
					//]
				}
			}
		#endif
/* Ericchung , TR104 auto test */
#if defined (CONFIG_RTK_VOIP_TR104) && defined (CONFIG_E8B)

			/* caller test , start send dtmf to peer */
			g_message("rcm_tr104_st.e8c_autotest.enable is %d\n",rcm_tr104_st.e8c_autotest.enable);
			g_message("rcm_tr104_st.e8c_autotest.TestType is %d\n",rcm_tr104_st.e8c_autotest.TestType);
            g_message("rcm_tr104_st.e8c_autotest.TestSelector is %d\n",rcm_tr104_st.e8c_autotest.TestSelector);

			if((rcm_tr104_st.e8c_autotest.enable==1) && (rcm_tr104_st.e8c_autotest.TestType==0) &&(rcm_tr104_st.e8c_autotest.TestSelector==1)){
				if(rcm_tr104_st.e8c_autotest.DailDTMFConfirmEnable==1){
					printf("send dtmf %s to peer \n",rcm_tr104_st.e8c_autotest.DailDTMFConfirmNumber);
					rcm_timerLaunch(lc->e8c_tr104_timer,3*1000); //after 3 sec start test.

				}


			}
#endif
			if(lc->fxsfeatureFlags & FXS_MCID_ACTIVATE){
				g_message("\r\n ********send  MCID reinvite \n");
				lc->fxsfeatureFlags &=~FXS_MCID_ACTIVATE;
				rcm_linphone_eXosip_mcid_reinvite(lc,ssid);

			}
			break ;
		case EXOSIP_CALL_CLOSED :
		case EXOSIP_CALL_CANCELLED :
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n close,cancel linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;
			lc = &pApp->ports[chid];
		{
			int another_session;
			SESSIONSTATE another_session_state;

			if( ev->type == EXOSIP_CALL_CLOSED )
				g_message( "EXOSIP_CALL_CLOSED[%d][%d]\n" , chid , ssid ) ;
			else
				g_message( "EXOSIP_CALL_CANCELLED[%d][%d]\n" , chid , ssid ) ;

			linphone_call_terminated(lc, ssid, ev);
			rcm_timerCancel(lc->no_Answer_timer_id);
			if (GetSysState(chid) == SYS_STATE_IDLE)
			{
				// no answer forward: break here to avoid busy tone 
				break;
			}

			rcm_tapi_SetRingFXS( chid , FALSE ) ;	
			SetSessionState( chid , ssid , SS_STATE_IDLE ) ;

			another_session = ssid ? 0 : 1;
			another_session_state = GetSessionState( chid , another_session ) ;
			// check another session state
			// close		| another
			// n/a			| A   (Idle, CallIn, CallOut, Caller, Callee)
			// A			| n/a (Idle, CallIn, CallOut, Caller, Callee)
			if (GetSysState(chid) == SYS_STATE_CONNECT_DAA_RING)
			{
				SetActiveSession(lc->chid, 0, TRUE);
				if (rcm_tapi_DAA_off_hook(lc->chid) == 0xff)
				{
					/* PSTN line not connect or line busy */
					SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
				    rcm_tapi_SetPlayTone(lc->chid, 0, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				}
				else
				{
					SetSysState(lc->chid, SYS_STATE_DAA_CONNECT);
					SetSessionState(lc->chid, 0, SS_STATE_CALLER);
				}
			}
			/*++added by Jack Chan  for transfer++*/
			else if (GetSysState(chid) == SYS_STATE_TRANSFERRING||GetSysState(chid) == SYS_STATE_TRANSFER_FINISH)
			{
				g_message("system state SYS_STATE_TRANSFERRING\n");
			}
			/*--end--*/
#ifdef ATTENDED_TRANSFER
			else if (GetSysState(chid) == SYS_STATE_TRANSFER_ONHOOK)
			{
				// rock: check call transfer done ?
				if (lc->call[0] || lc->call[1])
				{
					// there are still 2 calls, after this bye, this is still one , so keep in this state.
					g_message("system state SYS_STATE_ATT_TRANSFR_ONHOOK\n");
				}
				else
				{
					// all sessions are closed
					g_message("system state SYS_STATE_IDLE\n");
					rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);	
					SetSysState(chid, SYS_STATE_IDLE);
				}
			}
#endif			
			else if (GetActiveSession(chid) == -1)
			{
				// missed call, do nothing here
#ifdef CONFIG_RTK_VOIP_IP_PHONE
				//S2U_DisconnectInd( 1 );
#endif
				SetSysState(chid, SYS_STATE_IDLE);				
			}
			else if (lc->parent->bAutoAnswer)
			{
				// auto answer mode will be in on-hook status always
				SetSysState(chid, SYS_STATE_IDLE);
			}
			else if (GetSysState(chid) == SYS_STATE_IN3WAY)
			{
#ifdef CONFIG_RTK_VOIP_IP_PHONE
				//S2U_ConnectionEstInd( ssid, 7  /* resume from 3 way conference */ );
#endif

				TstVoipMgr3WayCfg stVoipMgr3WayCfg;

				SetSysState(chid, SYS_STATE_CONNECT);
				
				memset(&stVoipMgr3WayCfg, 0, sizeof(stVoipMgr3WayCfg));
				stVoipMgr3WayCfg.ch_id = chid;
				stVoipMgr3WayCfg.enable = FALSE;
				GetRtpCfg(&stVoipMgr3WayCfg.rtp_cfg[another_session], lc, another_session);
				rcm_tapi_SetConference(&stVoipMgr3WayCfg);
				SetActiveSession(chid, another_session, TRUE); 
				//linphone_core_stop_media_streams(lc, another_session);
			#ifdef CHECK_RESOURCE
				if (linphone_core_start_media_streams(lc, another_session, lc->call[another_session]) == -1)
				{
					linphone_core_terminate_dialog(lc, another_session, NULL);
					break;
				}
			#else
				linphone_core_start_media_streams(lc, another_session, lc->call[another_session]);
			#endif
			}
			else if (GetActiveSession(chid) == another_session)
			{
				// closed session is not active
				switch (another_session_state)
				{
				case SS_STATE_IDLE:
					// active session is idle: playing dial tone
					SetSysState( chid , SYS_STATE_EDIT ) ;
					break;
				case SS_STATE_CALLIN:
					// active session is callin: never occur!
					g_error("process_event: SS_STATE_CALLIN in active session\n");
					break;
				case SS_STATE_CALLOUT:
					// active session dial out: maybe play remote ring
					SetSysState( chid , SYS_STATE_EDIT ) ;
					break;
				default: // caller or callee
					// active session is not connected: 
					// 1. close hold session 
					// 2. close pre-conference
#ifdef CONFIG_RTK_VOIP_IP_PHONE
					//S2U_ConnectionEstInd( ssid, 1  /* call waiting is cancelled */ );
#endif
					SetSysState( chid , SYS_STATE_CONNECT ) ;
					// close call waiting tone
					rcm_tapi_SetPlayTone(chid, another_session, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
					break;
				}
			}
			else
			{
				// closed session is active
				switch (another_session_state)
				{
				case SS_STATE_IDLE:
					// another session is idle
					//if (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
					if( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature ) )
						rcm_tapi_FXO_Busy(chid);
					else
						rcm_tapi_SetPlayTone( chid , ssid , DSPCODEC_TONE_BUSY , TRUE , DSPCODEC_TONEDIRECTION_LOCAL ) ;

					SetSysState(chid, SYS_STATE_EDIT_ERROR);
					break;
				case SS_STATE_CALLIN:
					// another session is callin (in call waiting/caller id)
					SetSysState( chid , SYS_STATE_RING ) ;
					//stop call waiting tone
					rcm_tapi_SetPlayTone( lc->chid , ssid, DSPCODEC_TONE_CALL_WAITING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
			
					//printf("linphone_core_accept_dialog 003\n");
					linphone_core_accept_dialog(lc, another_session, NULL);
					// switch to another(new) session
					SetActiveSession( chid , another_session, TRUE ) ;
					break;
				case SS_STATE_CALLOUT:
					// another session is callout
					SetSysState( chid , SYS_STATE_EDIT ) ;
					// switch to another(new) session
					SetActiveSession( chid , another_session, TRUE ) ;
					if (lc->ringstream[another_session]) // if remote ringing
						rcm_tapi_SetPlayTone( chid , another_session , DSPCODEC_TONE_RINGING , TRUE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
					break;
				default: // caller or callee
					// another session is connected: 
					// 1. close active session 
					// 2. close pre-conference
					// 3. resume hold session
#ifdef CONFIG_RTK_VOIP_IP_PHONE
					//S2U_ConnectionEstInd( ssid, 8  /* from IN2CALL to one line */ );
#endif
					SetSysState( chid , SYS_STATE_CONNECT ) ;
					// switch to active session
					rcm_tapi_SetPlayTone( lc->chid , ssid, DSPCODEC_TONE_CALL_WAITING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
					printf(" switch to active session \n");
			
					linphone_core_offhood_event(lc, another_session);
					break;
				}
			}
			rtk_StopCid(chid,2);
			rcm_tapi_SetRingFXS( chid , FALSE ) ;	

		}
			break;
		case EXOSIP_CALL_TIMEOUT :
		case EXOSIP_CALL_NOANSWER :
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n EXOSIP_CALL_TIMEOUT/NOANSWER  linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;
			
			lc = &pApp->ports[chid];
			if( ev->type == EXOSIP_CALL_TIMEOUT )
				g_message( "EXOSIP_CALL_TIMEOUT[%d][%d]\n" , chid , ssid ) ;
			else
				g_message( "EXOSIP_CALL_NOANSWER[%d][%d]\n" , chid , ssid ) ;
			linphone_call_failure(lc, ssid, -110, ev);
			break;
		case EXOSIP_CALL_REQUESTFAILURE :
		case EXOSIP_CALL_GLOBALFAILURE :
		case EXOSIP_CALL_SERVERFAILURE :

			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n 	EXOSIP_CALL_xxx_FAILURE  return %d chid %d ssid %d\n",ret,chid,ssid);		
			if(ret==-1)
				break;
			lc = &pApp->ports[chid];
			
			if( ev->type == EXOSIP_CALL_REQUESTFAILURE )
				g_message( "EXOSIP_CALL_REQUESTFAILURE[%d][%d]\n" , chid , ssid ) ;
			else if( ev->type == EXOSIP_CALL_GLOBALFAILURE )
				g_message( "EXOSIP_CALL_GLOBALFAILURE[%d][%d]\n" , chid , ssid ) ;
			else
				g_message( "EXOSIP_CALL_SERVERFAILURE[%d][%d]\n" , chid , ssid ) ;
			//[SD6, bohungwu, exoisp 3.5 integration 
			//linphone_call_failure(lc, ssid, ev->did, ev->status_code);
			if(lc==NULL)
				g_message("lc is null\n");

			g_message("status code is %d\n",osip_message_get_status_code(ev->response));


			linphone_call_failure(lc, ssid, osip_message_get_status_code(ev->response), ev);
	
			break ;

		case EXOSIP_CALL_INVITE : /* receive incoming call INVITE */
			//g_message( "EXOSIP_CALL_NEW[%d][%d]\n" , chid , ssid ) ;
			g_message( "EXOSIP_CALL_INVITE[%d][%d]\n" , chid , ssid ) ;

			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n 	linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);		
			if(ret==-1)
			break ;
			lc = &pApp->ports[chid];

			/* Mandy add for stun support */
			//[SD6, ericchung, exosip 3.5 integration
			//linphone_core_set_use_info_for_stun(lc, -1, STUN_UPDATE_SIP); // update RTP in 200 ok
			//]

			/* CALL_NEW is used twice in qos mode : 
			 * when you receive invite (textinfo = "With QoS" or "Without QoS")
			 * and when you receive update (textinfo = "New Call") */
			linphone_inc_new_call(lc, ssid, ev);
#if defined(CONFIG_RTK_VOIP_TR104) && defined(CONFIG_E8B)

			/* ericchung  TR104 auto test , callee test*/
			if((rcm_tr104_st.e8c_autotest.enable==1) && (rcm_tr104_st.e8c_autotest.TestType==1)){
				/* offhook to answer */				
				
					rcm_timerLaunch(lc->e8c_tr104_timer,3*1000); //after 3 sec start test.
				
			}		

#endif
			break ;
		case EXOSIP_CALL_ACK :
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n 	linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);		
			if(ret==-1)
				break;
			
			lc = &pApp->ports[chid];

			g_message( "EXOSIP_CALL_ACK[%d][%d]\n" , chid , ssid ) ;

			//ericchung: add delayed offer to support handle ack + sdp		

			sdp_msg = eXosip_get_sdp_info(ev->ack);
			if(sdp_msg!= NULL) {
				g_message( "ACK have SDP, must update media\n");
			
				rcm_sdp_context_read_answer_by_sdp(	lc->call[ssid]->sdpctx , ev->ack);	

				// sdp_message_to_str(sdp_msg, &sdp_body_str);
				// printf("sdp: %s\n",sdp_body_str);
				sdp_message_free(sdp_msg);
				linphone_core_stop_media_streams(lc, ssid);
				linphone_core_start_media_streams(lc, ssid, lc->call[ssid]);

			}

		#if 1
			// its can be 
			// 1. UA response ok
			// 2. UA response re-invite (hold, resume, fax..)
			switch (lc->call[ssid]->state)
			{
			case LCStateRinging:
			{
				//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
				if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
				{
					rcm_tapi_FXO_offhook(lc->chid);
					// rock: check one stage dialing
					if (GetSysState(lc->chid) == SYS_STATE_RING &&
						lc->call[ssid]->one_stage_dial_number[0])
					{
						rcm_tapi_Dial_PSTN_Call_Forward(lc->chid, ssid, lc->call[ssid]->one_stage_dial_number);
					}
				}

				switch (GetSysState(chid))
				{
				case SYS_STATE_CONNECT:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
					S2U_ConnectionEstInd( chid, ssid, 9 );
#endif
					SetSysState(chid, SYS_STATE_IN2CALLS_IN);
					break;
				default:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
					S2U_ConnectionEstInd( chid, ssid, 2 );
#endif
					SetSysState(chid, SYS_STATE_CONNECT);
					break;
				}

				SetSessionState( chid , ssid , SS_STATE_CALLEE ) ;
				SetActiveSession( chid, ssid, TRUE );
			#ifdef CHECK_RESOURCE
				if (linphone_core_start_media_streams(lc, ssid, lc->call[ssid]) == -1) // move down for AC
				{
					linphone_core_terminate_dialog(lc, ssid, NULL);
					break;
				}
			#else
				linphone_core_start_media_streams(lc, ssid, lc->call[ssid]); // move down for AC
			#endif
				break;
			}
			case LCStateBeHold:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_SessionHeldInd( chid, ssid, 1 );
#endif
				//lc->call[ssid]->state = LCStateBeHoldACK;
				SetCallState(lc->chid,ssid,&(lc->call[ssid]->state) ,LCStateBeHoldACK);

				//rcm_tapi_SetRtpSessionState(chid, ssid, rtp_session_recvonly);
				linphone_core_stop_media_streams(lc, ssid);
				if ((GetSysState(chid) != SYS_STATE_IN3WAY) && (GetActiveSession(chid) == ssid))
					rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				break;
			case LCStateBeResume:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_SessionResumeInd( chid, ssid, 1 );
#endif
				//lc->call[ssid]->state = LCStateAVRunning;
				SetCallState(chid,ssid,&(lc->call[ssid]->state),LCStateAVRunning);

				g_message("\r\nlinphone-state aa chid[%d],ssid[ssid], status %d\n",chid,ssid,lc->call[ssid]->state );

				//rcm_tapi_SetRtpSessionState(chid, ssid, rtp_session_sendrecv);
			#ifdef CHECK_RESOURCE
				if (linphone_core_start_media_streams(lc, ssid, lc->call[ssid]) == -1)
				{
					linphone_core_terminate_dialog(lc, ssid, NULL);
					break;
				}
			#else
				linphone_core_start_media_streams(lc, ssid, lc->call[ssid]);
			#endif
				rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
#if 0			
				if (GetSysState(chid) != SYS_STATE_IN3WAY && GetActiveSession(chid) != ssid)
				{
					LinphoneCall *call = lc->call[ssid];	

					// let me hold peer device
					call->state = LCStateHold; // start hold
					call->sdpctx->version++;
					eXosip_lock();
					eXosip_on_hold_call(call->did, call->sdpctx->version);
					eXosip_unlock();
				}
#endif			
				digitcode_init(lc);
				break;
			case LCStateUpdate:
			case LCStateBeFax:
				// reinvite (fax...)
				//lc->call[ssid]->state = LCStateAVRunning;
				SetCallState(chid,ssid,&(lc->call[ssid]->state),LCStateAVRunning);
				//linphone_core_stop_media_streams(lc, ssid);
			#ifdef CHECK_RESOURCE
				if (linphone_core_start_media_streams(lc, ssid, lc->call[ssid]) == -1)
				{
					linphone_core_terminate_dialog(lc, ssid, NULL);
					break;
				}
			#else
				linphone_core_start_media_streams(lc, ssid, lc->call[ssid]);
			#endif
				break;
			default:
				// do what?
				break;
			}
		#endif
			break ;
		case EXOSIP_CALL_REDIRECTED :
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n EXOSIP_CALL_REDIRECTED  linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;
			
			lc = &pApp->ports[chid];
			
			g_message( "EXOSIP_CALL_REDIRECTED[%d][%d]\n" , chid , ssid ) ;
			if (lc->audiostream[ssid])
				linphone_core_stop_media_streams(lc, ssid);
			
			eXosip_default_action(ev);
			
			//]	
			break ;
		case EXOSIP_CALL_PROCEEDING :
			
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n EXOSIP_CALL_PROCEEDING linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;

	
			g_message( "EXOSIP_CALL_PROCEEDING[%d][%d]\n" , chid , ssid ) ;
			lc = &pApp->ports[chid];
				
			linphone_call_proceeding(lc, ssid, ev->cid, ev->did);
			//[SD6, ericchung, exosip 3.5 integration
			lc->call[ssid]->tid = ev->tid;

			break ;
		case EXOSIP_CALL_RINGING :
			ret=linphone_process_call(pApp, &chid, &ssid, ev);

			g_message("\r\n EXOSIP_CALL_RINGING linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;

			lc = &pApp->ports[chid];

			
			g_message( "EXOSIP_CALL_RINGING[%d][%d]\n" , chid , ssid ) ;
			linphone_call_ringing(lc, ssid, ev);
			break ;
		case EXOSIP_IN_SUBSCRIPTION_NEW :
			g_message( "EXOSIP_IN_SUBSCRIPTION_NEW\n" ) ;
			eXosip_message_send_answer(ev->tid,501,NULL);
#if 0
			linphone_subscription_new( lc , ev->did , ev->nid , ev->remote_uri , ev->remote_contact ) ;
#endif
			break ;
		case EXOSIP_SUBSCRIPTION_NOTIFY :
			g_message( "EXOSIP_SUBSCRIPTION_NOTIFY\n" ) ;
			g_message("\r\n TBD need handle Notify \n");
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
				
			g_message("\r\n EXOSIP_SUBSCRIPTION_NOTIFY linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;
		
			lc = &pApp->ports[chid];
			g_message( "EXOSIP_SUBSCRIPTION_NOTIFY[%d][%d]\n" , chid , ssid ) ;



			rcm_linphone_notify_rec(lc,ssid,ev);
			
			break ;
		case EXOSIP_SUBSCRIPTION_ANSWERED :
			g_message( "EXOSIP_SUBSCRIPTION_ANSWERED\n" ) ;
#if 0
			linphone_subscription_answered( lc , ev->remote_uri , ev->did ) ;
#endif
			break ;
		case EXOSIP_SUBSCRIPTION_CLOSED :
			g_message( "EXOSIP_SUBSCRIPTION_CLOSED\n" ) ;
#if 0
			linphone_subscription_closed( lc , ev->remote_uri , ev->did ) ;
#endif
			break ;
		//[SD6, bohungwu, exoisp 3.5 integration 
		//case EXOSIP_CALL_STARTAUDIO :
		//	g_message( "EXOSIP_CALL_STARTAUDIO[%d][%d]\n" , chid , ssid ) ;
		//	break ;
		//]
		case EXOSIP_CALL_RELEASED :
			//[SD6, ericchung, exosip 3.5 integration
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n EXOSIP_CALL_RELEASED linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;

			lc = &pApp->ports[chid];
			g_message( "EXOSIP_CALL_RELEASED[%d][%d]\n" , chid , ssid ) ;
			linphone_call_released(lc, ssid, ev->cid);
			if (chid != -1 && ssid !=-1 &&
				GetSysState(lc->chid) == SYS_STATE_TRANSFERRING)
			{
				int hs; // hold session

				hs = (ssid == 0) ? 1 : 0;
				if (lc->call[hs] && lc->call[hs]->state != LCStateInit)
				{
					SetActiveSession(lc->chid, hs, TRUE);
#ifdef CONFIG_RTK_VOIP_IP_PHONE
					S2U_ConnectionEstInd( chid, ssid, 3 );
#endif
					SetSysState(lc->chid, SYS_STATE_CONNECT);
					//[SD6, bohungwu, exosip 3.5 integration
					//eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 400 Bad Request");
					rcm_eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 400 Bad Request");
					//]
				}
			}
			break ;
		case EXOSIP_REGISTRATION_FAILURE :
			g_message( "EXOSIP_REGISTRATION_FAILURE\n" ) ;
			linphone_registration_faillure( lc , ev ) ;
			break ;
		case EXOSIP_REGISTRATION_SUCCESS :
			g_message( "EXOSIP_REGISTRATION_SUCCESS\n" ) ;
			linphone_registration_success( lc , ev ) ;
			break ;
		case EXOSIP_MESSAGE_NEW :
			g_message( "EXOSIP_MESSAGE_NEW\n" ) ;
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			g_message("\r\n EXOSIP_MESSAGE_NEW linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			/* no dialog, use default lc for response*/
			if (ret==-1){
				lc = &pApp->ports[0];
			}else{
				lc = &pApp->ports[chid];
			}
			
			rcm_process_other_request(lc,ssid,ev);
			break ;

		case EXOSIP_CALL_MESSAGE_NEW:

			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			
			g_message("\r\n EXOSIP_CALL_MESSAGE_NEW linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;

			lc = &pApp->ports[chid];
			g_message( "EXOSIP_CALL_MESSAGE_NEW[%d][%d]\n" , chid , ssid ) ;

			if (ev->request){
				if (MSG_IS_INFO(ev->request)){
					rcm_linphone_process_sipinfo(lc,ssid,ev);
	
				}else if(MSG_IS_REFER(ev->request)){
					rcm_linphone_process_refer(lc,ssid,ev);					

				}else if(MSG_IS_UPDATE(ev->request)){
					osip_message_t *ans=NULL;
					eXosip_lock();
					eXosip_call_build_answer(ev->tid,200,&ans);
					if (ans)
						eXosip_call_send_answer(ev->tid,200,ans);
					eXosip_unlock();
				}else if(MSG_IS_PRACK(ev->request)){ //add for prack 
					osip_message_t *ans=NULL;
					eXosip_lock();
					eXosip_call_build_answer(ev->tid,200,&ans);
					if (ans)
						eXosip_call_send_answer(ev->tid,200,ans);
					eXosip_unlock();					
				}else if (MSG_IS_OPTIONS(ev->request)){
					osip_message_t *ans=NULL;
					eXosip_lock();
					eXosip_call_build_answer(ev->tid,200,&ans);
					if (ans){
						osip_message_set_allow(ans,"INVITE, ACK, BYE, CANCEL, OPTIONS, MESSAGE, SUBSCRIBE, NOTIFY, INFO, UPDATE");
						osip_message_set_accept(ans,"application/sdp");
						eXosip_call_send_answer(ev->tid,200,ans);
					}
					eXosip_unlock();
				}else if (MSG_IS_NOTIFY(ev->request)){
					//for call transfer , need check subscription-state:terminated;reason=completed
					osip_message_t *ans=NULL;
					eXosip_lock();
					eXosip_call_build_answer(ev->tid,200,&ans);
					if (ans){
						eXosip_call_send_answer(ev->tid,200,ans);
					}
					eXosip_unlock();
					if (GetSysState(chid)==SYS_STATE_TRANSFER_ONHOOK)
					{	
						SetSysState(chid, SYS_STATE_TRANSFER_FINISH);
						rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);	
			
					}
					else if (GetSysState(chid)==SYS_STATE_TRANSFER_FINISH)
					{
						
						linphone_core_terminate_dialog(lc,ssid,NULL);
						SetSysState(chid, SYS_STATE_IDLE);
						SetActiveSession(chid, -1, -1);	
			
					}			

				}else if (MSG_IS_BYE(ev->request)){			
					g_message("MSG_IS_BYE on EXOSIP_CALL_MESSAGE_NEW\n");
					//printf("GetSysState(chid) is %d\n",GetSysState(chid));
					//printf("GetSessionState is %d\n",GetSessionState(chid,ssid));
					if((GetSysState(chid)==SYS_STATE_RING) && (GetSessionState(chid,ssid)==SS_STATE_CALLIN)) {
						/*ericchung: early dialog receive bye, send 487 to sip server */
						eXosip_lock();
						eXosip_call_send_answer(lc->call[ssid]->tid,487,NULL);
						eXosip_unlock();
					}
				}else{
					g_message("Other message , need implement \n");
				}
			}
			g_message("\r\n if receive EXOSIP_CALL_MESSAGE_NEW , message should parse \n");

			break ;
		#if 0
		case EXOSIP_CALL_REFERED :
		{
			int si; // idle session

			g_message( "EXOSIP_CALL_REFERED [%d][%d]: refer_to = %s\n", chid, ssid, ev->refer_to ) ;
			si = ssid == 1 ? 0 : 1;
			if (GetSessionState(chid, si) == SS_STATE_IDLE)
			{
				rcm_tapi_SetPlayTone(chid, si, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				SetActiveSession(chid , si, TRUE);
				linphone_core_invite(lc, si, ev->refer_to);
				SetSysState(lc->chid, SYS_STATE_TRANSFERRING);
			}
			else
			{
				g_warning("EXOSIP_CALL_REFERED: no idle session\n");
			}
			break;
		}
		case EXOSIP_INFO_NEW:
		{
			if (ev->i_ctt == NULL || 
				ev->i_ctt->type == NULL || 
				ev->i_ctt->subtype == NULL)
			{
				g_warning("no valid content type in SIP INFO\n");
#if OLD_API //eric temp mark , not need free here
				eXosip_event_free( ev ) ;
#endif
				return;
			}

			if ((strcasecmp(ev->i_ctt->type, "application") == 0) &&
				(strcasecmp(ev->i_ctt->subtype, "dtmf-relay") == 0))
			{
				osip_body_t *body;
				char signal[10], duration[10];
  
				signal[0] = 0; duration[0] = 0;
				body = (osip_body_t *) osip_list_get(ev->i_bodies, 0);
				if (body && body->body)
				{
					char *new_body;
			
					new_body = tolower_clrspace(body->body);
					if (new_body)
					{
						sscanf(new_body, "signal=%s\r\nduration=%s\r\n", signal, duration);
						osip_free(new_body);
					}
				}

				// TODO: duration ?
				if (signal[0] && signal[1] == 0)
				{
				
				#if 1	/* kernel play tone with duration */
					if (signal[0] == '*')
						rcm_tapi_SIP_INFO_play_tone(chid, ssid, DSPCODEC_TONE_STARSIGN, atoi(duration));
					else if (signal[0] == '#')
						rcm_tapi_SIP_INFO_play_tone(chid, ssid, DSPCODEC_TONE_HASHSIGN, atoi(duration));
					else if (signal[0] >= '0' && signal[0] <= '9')
						rcm_tapi_SIP_INFO_play_tone(chid, ssid, signal[0] - '0' + DSPCODEC_TONE_0, atoi(duration));
				
				#else	/* IO Ctrl play tone without duration */
					if (signal[0] == '*')
						rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_STARSIGN, 
							TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					else if (signal[0] == '#')
						rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_HASHSIGN, 
							TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					else if (signal[0] >= '0' && signal[0] <= '9')
						rcm_tapi_SetPlayTone(chid, ssid, signal[0] - '0' + DSPCODEC_TONE_0, 
							TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		
				#endif

				}
			}

			break;
		}
		case EXOSIP_CALL_REFER_STATUS:
		{
			const char *statuscode;
			int status_code;

			g_message("EXOSIP_CALL_REFER_STATUS [%d][%d]: status = %s\n", chid, ssid, ev->sdp_body);
			statuscode = strchr(ev->sdp_body, ' ');
			if (statuscode)
			{		
				if (sscanf(statuscode + 1, "%d", &status_code) == 1)
				{
					/* Ensure numeric status code */
					switch (status_code)
					{
					case 100:
						// transferring...
						break;
					case 200:
						// transfer ok
#ifdef CONFIG_RTK_VOIP_IP_PHONE
						S2U_DisconnectInd( lc ->chid, ( ssid ? 0 : 1 ), 13 );
#endif
						linphone_core_terminate_dialog(lc, ssid, NULL);
#ifdef ATTENDED_TRANSFER
						//received  200 ok for notify, already on-hook
						if (GetSysState(chid) == SYS_STATE_TRANSFER_ONHOOK) 
						{
							// rock: don't change to SYS_STATE_IDLE here,
							//    it should be checked in EXOSIP_CALL_CLOSED
						#if 0
							SetSysState(chid, SYS_STATE_IDLE);
						#endif
							break;
						}
#endif
						SetSysState(chid, SYS_STATE_EDIT_ERROR);
						rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
						break;
					default:
						if (status_code >= 400 && status_code < 700)
						{
							// Failed Transfer, resume old session
#ifdef CONFIG_RTK_VOIP_IP_PHONE
							S2U_DisconnectInd( lc ->chid, ( ssid ? 0 : 1 ), 14 );

							rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, FALSE,   
										DSPCODEC_TONEDIRECTION_LOCAL); 
#endif
#ifdef ATTENDED_TRANSFER
						//received  4xx ok for notify, already on-hook, termina all call
						if (GetSysState(chid) == SYS_STATE_TRANSFER_ONHOOK) 
						{
							int i;
							for( i = 0 ; i < MAX_SS ; i++ )
							{
								linphone_core_terminate_dialog(lc, i, NULL);						
							}
							SetSysState(chid, SYS_STATE_IDLE);
							break;
						}
#endif
							if (session_2_session_event(lc, ssid + 1) == 0)
							{
								SetSysState(lc->chid , SYS_STATE_CONNECT);
							}
							digitcode_init(lc);
						}
						break;
					}
				}
			}
			break;
		}
		case EXOSIP_CALL_REFER_REQUESTFAILURE:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
			S2U_DisconnectInd( lc ->chid, ( ssid ? 0 : 1 ), 15 );
			
			rcm_tapi_PlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, FALSE,   
										DSPCODEC_TONEDIRECTION_LOCAL); 
#endif
#ifdef ATTENDED_TRANSFER
			if (GetSysState(lc->chid) == SYS_STATE_TRANSFER_ONHOOK)
			{
				int i;
				// refer fail and user already onhook, terminate both calls 
				for( i = 0 ; i < MAX_SS ; i++ )
					linphone_core_terminate_dialog(lc, i, NULL);
				break;
			}
#endif
			g_message("EXOSIP_CALL_REFER_REQUESTFAILURE [%d][%d]: status = %d\n", chid, ssid, ev->status_code);
			if (session_2_session_event(lc, ssid + 1) == 0)
			{
				SetSysState(lc->chid , SYS_STATE_CONNECT);
			}
			digitcode_init(lc);
			break;
		case EXOSIP_CALL_UPDATE:
		case EXOSIP_CALL_HOLD:
		case EXOSIP_CALL_OFFHOLD:
		case EXOSIP_CALL_FAXON:
		case EXOSIP_CALL_FAXOFF:
		case EXOSIP_CALL_MODEM:
		#ifdef ATTENDED_TRANSFER
			if (GetSysState(lc->chid) == SYS_STATE_TRANSFER_ONHOOK)
			{
				int i;

				// error handling for UA doesn't know Attended Transfer 
				eXosip_lock();
				eXosip_answer_call(ev->did, 400, NULL);
				eXosip_unlock();
				eXosip_execute();
				for(i = 0; i<MAX_SS; i++)
					linphone_core_terminate_dialog(lc, i, NULL);

				break;
			}
		#endif
			linphone_core_handle_reinvite(lc, chid, ssid, ev);
			break;
		#endif //#if 0
		case EXOSIP_CALL_MESSAGE_REQUESTFAILURE:
			
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			g_message("\r\n EXOSIP_CALL_MESSAGE_REQUESTFAILURE linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;
	
			lc = &pApp->ports[chid];

			if (ev->request){				
				if(MSG_IS_UPDATE(ev->request)){
					//printf("EXOSIP_CALL_MESSAGE_REQUESTFAILURE	request UPDATE fail.\n");
					linphone_core_terminate_dialog(lc, ssid, NULL);
					linphone_call_failure(lc, ssid, -110, ev);
					
				}
			}
			break;
#if 0			
		case EXOSIP_MESSAGE_REQUESTFAILURE:
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
				g_message("\r\n EXOSIP_MESSAGE_REQUESTFAILURE linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
				if (ret==-1)
					break;
			
				lc = &pApp->ports[chid];
			
				if (ev->request){	
					if(MSG_IS_OPTIONS(ev->request)){

						
					}
				}
		

			break;
	#endif		
		case EXOSIP_CALL_REINVITE:
			ret=linphone_process_call(pApp, &chid, &ssid, ev);
			g_message("\r\n EXOSIP_CALL_REINVITE linphone_process_call return %d chid %d ssid %d\n",ret,chid,ssid);
			if (ret==-1)
				break;

			lc = &pApp->ports[chid];
			linphone_core_handle_reinvite(lc, chid, ssid, ev);
			break;
		case EXOSIP_MESSAGE_ANSWERED:
		case EXOSIP_MESSAGE_REQUESTFAILURE:
			if (ev->response){
				if (ev->request && strcmp(osip_message_get_method(ev->request),"OPTIONS")==0){				
					g_message("receive OPTIONS response , tid is  %d\n",ev->tid);
					chid = GetChIDfromOptionsTid(pApp,ev);
					if(chid!=-1){
						voipCfgPortParam_t *voip_ptr;
						lc = &pApp->ports[chid];
						lc->OptionsPingStatus=Option_Next_Ping; 
						voip_ptr = &g_pVoIPCfg->ports[lc->chid];
				
						rcm_timerLaunch(lc->options_ping_timer,voip_ptr->proxies[0].HeartbeatCycle*1000); //next send option time
					}				
				}
			}

			break;
		default :
			g_message( "default[%d][%d]\n" , chid , ssid ) ;
			break ;
	}
}

void rcm_process_other_request(LinphoneCore *lc, guint32 ssid,eXosip_event_t *ev){

	if (ev->request==NULL) 
		return;


	if (MSG_IS_OPTIONS(ev->request)){
		osip_message_t *options=NULL;
		eXosip_options_build_answer(ev->tid,200,&options);
		//printf("\r\n Message is OPTION\n");
		osip_message_set_allow(options,"INVITE, ACK, BYE, CANCEL, OPTIONS, MESSAGE, SUBSCRIBE, NOTIFY, INFO, UPDATE");
		osip_message_set_accept(options,"application/sdp");
		eXosip_options_send_answer(ev->tid,200,options);
	}else if (MSG_IS_NOTIFY(ev->request)){
			/* response for NOTIFY */
			rcm_linphone_notify_rec(lc,ssid,ev);
		
			eXosip_message_send_answer(ev->tid,200,NULL);		
	}else{

		/* ericchung : need check other request , TBD */ 
		char *tmp=NULL;
		size_t msglen=0;
		osip_message_to_str(ev->request,&tmp,&msglen);
		if (tmp){
			printf("Unsupported request received:\n%s",tmp);
			osip_free(tmp);
		}
		/*answer with a 501 Not implemented*/
		eXosip_message_send_answer(ev->tid,501,NULL);

		g_message("rcm_process_other_request: receive not other request, need check \n");

	}
}
	

//[SD6, ericchung, rcm integration
/* Process incoming SIP INFO */
void rcm_linphone_process_sipinfo(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev){

	osip_content_type_t *ct;
	osip_message_t *ans=NULL;
	osip_body_t *body=NULL;
	char signal[10], duration[10];
	
	ct=osip_message_get_content_type(ev->request);
	if (ct && ct->subtype){
		if (strcmp(ct->subtype,"dtmf-relay")==0){
			g_message("\r\n receive dtmf-relay\m");
			osip_message_get_body(ev->request,0,&body);
			
			if (body && body->body){
				char *new_body;
				new_body = tolower_clrspace(body->body);
				if (new_body){
					sscanf(new_body, "signal=%s\r\nduration=%s\r\n", signal, duration);
					osip_free(new_body);
				}
			}
			
			// TODO: duration ?
			if (signal[0] && signal[1] == 0){					
#if 1	/* kernel play tone with duration */
				if (signal[0] == '*')
					rcm_tapi_SIP_INFO_play_tone(lc->chid, ssid, DSPCODEC_TONE_STARSIGN, atoi(duration));
				else if (signal[0] == '#')
					rcm_tapi_SIP_INFO_play_tone(lc->chid, ssid, DSPCODEC_TONE_HASHSIGN, atoi(duration));
				else if (signal[0] >= '0' && signal[0] <= '9')
					rcm_tapi_SIP_INFO_play_tone(lc->chid, ssid, signal[0] - '0' + DSPCODEC_TONE_0, atoi(duration));
						
#else	/* IO Ctrl play tone without duration */
				if (signal[0] == '*')
					rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_STARSIGN, 
						TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				else if (signal[0] == '#')
					rcm_tapi_SetPlayTone(chid, ssid, DSPCODEC_TONE_HASHSIGN, 
						TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				else if (signal[0] >= '0' && signal[0] <= '9')
					rcm_tapi_SetPlayTone(chid, ssid, signal[0] - '0' + DSPCODEC_TONE_0, 
						TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				
#endif
				/*ericchung : if E8C auto test ,must record dtmf string */
#if defined(CONFIG_RTK_VOIP_TR104) && defined(CONFIG_E8B)
				if(rcm_tr104_st.e8c_autotest.enable==1){
					rcm_tr104_st.e8c_autotest.ReceiveDTMFNumber[rcm_tr104_st.e8c_autotest.receive_dtmf_index]=signal[0];
					rcm_tr104_st.e8c_autotest.receive_dtmf_index++;
					strcpy(rcm_tr104_st.e8c_autotest.Simulate_Status,"Receiving");
					
				}
#endif				
			}
		}
	}

	//response 200 ok for info
	eXosip_lock();
	eXosip_call_build_answer(ev->tid,200,&ans);
	if (ans){
		//configure contact header
		if(lc->call[ssid]->proxy!=NULL){
		_osip_list_set_empty(&ans->contacts,(void (*)(void*))osip_contact_free);
		osip_message_set_contact(ans,lc->call[ssid]->proxy->contact);
}
		eXosip_call_send_answer(ev->tid,200,ans);
	}
	eXosip_unlock();
	return;
}

//process REFER message , for call transfer 
void rcm_linphone_process_refer(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev){

	osip_header_t *h=NULL;
	osip_message_t *ans=NULL;
	char *replaces;
	int si; // idle session
	osip_header_t *referred_by=NULL;

	osip_message_header_get_byname(ev->request,"Refer-To",0,&h);
	if (h){
		g_message("rcm_linphone_process_refer h->hvalue is %s\n",h->hvalue);
	}
	osip_message_header_get_byname(ev->request,"Referred-By",0,&referred_by);
	if (referred_by && referred_by->hvalue && referred_by->hvalue[0]!='\0'){
		g_message("rcm_linphone_process_refer referred_by->hvalue is %s\n",referred_by->hvalue);
	}

	eXosip_lock();
	eXosip_call_build_answer(ev->tid,202,&ans);
	if (ans)
		eXosip_call_send_answer(ev->tid,202,ans);
	eXosip_unlock();

	
	rcm_eXosip_transfer_send_notify(lc->call[ssid]->did, EXOSIP_SUBCRSTATE_ACTIVE, "SIP/2.0 200 OK");
	
	si = ssid == 1 ? 0 : 1;
	if (GetSessionState(lc->chid, si) == SS_STATE_IDLE)
	{
		g_message("rcm_linphone_process_refer 002\n");
		rcm_tapi_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
		//rcm_tapi_SetPlayTone(lc->chid, si, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
		SetActiveSession(lc->chid , si, TRUE);
		//rcm_linphone_core_refer_invite(lc, si, h->hvalue,ev);
		linphone_core_invite(lc, si, h->hvalue);
		SetSysState(lc->chid, SYS_STATE_TRANSFERRING);
	}
	else
	{
		g_message("EXOSIP_CALL_REFERED: no idle session\n");
	}
		


	return;
}


void rcm_linphone_notify_rec(LinphoneCore *lc, guint32 ssid,eXosip_event_t *ev){

	osip_body_t *body=NULL;
	

	if (ev->request==NULL) 
		return;

	osip_message_get_body(ev->request,0,&body);
	if (body==NULL){
		g_message("No body in NOTIFY");
		return;
	}else{
		g_message("++++ notidy body +++ \n%s\n",body->body);
	}
#ifdef CONFIG_RTK_VOIP_TR104	
	rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags=0;

	if (strstr(body->body,"special-condition-tone")!=NULL){
		g_message("get nodify special condition tone\n");
		lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags|=IMS_SPECIAL_DIAL_TONE;
	}

	if (strstr(body->body,"immediateCallSetup")!=NULL){
		g_message("immediateCallSetup (hotline)\n");
		//lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags|=IMS_IMMEDIATE_CALLSETUP;
	}	

	if (strstr(body->body,"deferredCallSetup")!=NULL){
		g_message("deferredCallSetup (delay hotline)\n");
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags|=IMS_DEFERE_CALLSETUP;
		//lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
	}	

	if (strstr(body->body,"rejectCall")!=NULL){
		g_message("rejectCall (delay hotline)\n");
		//lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags&=~IMS_DEFERE_CALLSETUP;
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags&=~IMS_IMMEDIATE_CALLSETUP;

		
	}	


	if (strstr(body->body,"call-waiting active=\"true")!=NULL){
		g_message("call waiting enable\n");
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags|=IMS_CALL_WAITING_ACTIVATE;
		//lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
	}	

	if (strstr(body->body,"three-party-conference active=\"true")!=NULL){
		g_message("enable 3 way conference\n");
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags|=IMS_CONFERENCE_ACTIVATE;
		//lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
	}	

	if (strstr(body->body,"malicious-call-identify active=\"true")!=NULL){
		g_message("enable MCID\n");
		rcm_tr104_st.voiceProfileLineStatusMsg[lc->chid].featureFlags|=IMS_MCID_ACTIVATE;
		//lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;
	}	
#endif
	/* add parse IMS  sip server forwarding tone */
	lc->fxsfeatureFlags &= ~FXS_SPECIAL_DIAL_TONE;

	if (strstr(body->body,"<alert-info>pattern6")!=NULL){
			g_message("enable forwarding \n");
			 lc->fxsfeatureFlags|=FXS_SPECIAL_DIAL_TONE;

	}
	
	/*only play one time */	
	if (strstr(body->body,"Messages-Waiting: yes")!=NULL){
			g_message("enable voicemail box tone \n");
			 lc->fxsfeatureFlags|=FXS_VOICEMAIL_TONE;

	}
	//TBD
}


