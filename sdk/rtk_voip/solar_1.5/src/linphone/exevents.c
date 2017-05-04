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
#include "eXosip.h"
#include "eXosip2.h"
#include "linphonecore.h"
#include "misc.h"
#include "uglib.h"
#include "payloadtype.h"
#include "exevents.h"
#include "alarm.h"

#ifdef CONFIG_RTK_VOIP_IP_PHONE
#include "uihandler.h"
#endif

#define CALLER_ID_NAME_LENGTH 51

extern void linphone_register_authentication_required(LinphoneCore *lc,eXosip_event_t *ev);
extern void linphone_invite_authentication_required(LinphoneCore *lc, int sid);
extern int linphone_call_authentication_required(LinphoneCore *lc, LinphoneCall *lcall);
extern void linphone_subscription_new(LinphoneCore *lc,int did, int sid, char *from, char* contact);
extern void linphone_notify_recv(LinphoneCore *lc,char *from,int ss_status);
extern LinphoneProxyConfig *linphone_core_get_proxy_config_from_rid(LinphoneCore *lc, int rid);
void linphone_subscription_answered(LinphoneCore *lc,char *from, int sid);
void linphone_subscription_closed(LinphoneCore *lc,char *from, int did);

/* these are useful messages for the status */
extern gchar ready[] ;
extern gchar end[] ;
extern gchar contacting[] ;
extern gchar connected[] ;
extern gchar cancel[] ;
extern gchar contacted[] ;

//#define VMWI_TEST	/* thlin+ for test VMWI */

//======================== Callee side CID only =========================
char caller_id[21];	// hc+ 1130 for get caller ID from SIP message header
char caller_id_name[CALLER_ID_NAME_LENGTH];	// display the name of caller id.
char fsk_dsil_chk[4]={0};	// hc+ 1215 for FSK CID
char caller_id_mode[4]={0};	// CID mode, 1:FSK, 0:DTMF
char call_waiting_cid[4]={0, 0, 0, 0};// call waiting caller ID gen, 1: enable 0: disable
//===================================================================================

int linphone_call_accepted(LinphoneCore *lc, uint32 ssid, int id, const char *sdpbody)
{
	StreamParams *audio_params;
	LinphoneCall *call=lc->call[ssid];
	uint32 chid=lc->chid;

	if( call == NULL )
	{
		g_warning( "No call to accept.\n" ) ;
		return 0 ;
	}

	audio_params = &call->audio_params ;
	call->auth_failed = FALSE ;

	if( strlen(sdpbody) == 0 )
	{
		g_warning( "No sdp body in 200ok.\n" ) ;
		return 0 ;
	}

	switch (call->state)
	{
	case LCStateHold:
		// receive hold response
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		S2U_SessionHeldInd( chid, ssid, 0 /* hold */ );
#endif
		call->state = LCStateHoldACK;
		SetActiveSession(chid, ssid == 0 ? 1 : 0, TRUE);
//		rtk_SetRtpSessionState(chid, ssid, rtp_session_sendonly);
		linphone_core_stop_media_streams(lc, ssid);
		if (GetSessionState(lc->chid, ssid == 0 ? 1 : 0) == SS_STATE_IDLE)
			rtk_SetPlayTone(lc->chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
		break;
	case LCStateResume:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
		S2U_SessionResumeInd( chid, ssid, 0 );
#endif
		call->state = LCStateAVRunning;
		SetActiveSession(chid, ssid, TRUE);
//		rtk_SetRtpSessionState(chid, ssid, rtp_session_sendrecv);
		rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
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
			rtk_SetConference(&stVoipMgr3WayCfg);
		}
		break;
	case LCStateHoldACK:
		// shouldn't occur
		printf("receive OK on HoldACK!?\n");
		break;
	case LCStateAVRunning:
		// shouldn't occur
		printf("receive OK on AVRnning...\n");
		sdp_context_read_answer(lc->call[ssid]->sdpctx, sdpbody);
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
	case LCStateInit: // if no ringing
	case LCStateRinging:
		SetActiveSession( chid , ssid , TRUE ) ;

		sdp_context_read_answer( lc->call[ssid]->sdpctx , sdpbody ) ;
		linphonec_display_something(lc, "Connected.\n");

		if( lc->ringstream[ssid] )
		{
			lc->ringstream[ssid] = FALSE ;
			rtk_SetRingFXS( chid , FALSE ) ;
			rtk_SetPlayTone( chid , ssid , DSPCODEC_TONE_RINGING , FALSE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
		}

		switch (GetSysState(chid))
		{
		case SYS_STATE_CONNECT_EDIT:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
			S2U_ConnectionEstInd( chid, ssid, 11 );
#endif
			SetSysState(chid, SYS_STATE_IN2CALLS_OUT);
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
				// rtk_FXO_offhook has done
			}
			else
			{
				rtk_FXO_offhook(chid);
			}
		}

		SetSessionState( chid , ssid , SS_STATE_CALLER ) ;
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
		sdp_context_read_answer(lc->call[ssid]->sdpctx, sdpbody);
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
		sdp_context_read_answer(lc->call[ssid]->sdpctx, sdpbody);
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
	lc->ringstream[ssid] = FALSE ;

#ifdef CONFIG_RTK_VOIP_IP_PHONE
	S2U_DisconnectInd( lc ->chid, ssid, 4 );
#endif

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
        rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
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

int linphone_call_failure(LinphoneCore *lc, guint32 ssid, int cid, int code)
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
	if (GetSysState(lc->chid) == SYS_STATE_TRANSFERRING)
	{
		hs = (ssid == 0) ? 1 : 0;
		if (lc->call[hs] && lc->call[hs]->state != LCStateInit)
			bTransferring = 1;
	}

	switch( code )
	{
		case 401 :
		case 407 :
			if (linphone_call_authentication_required(lc, lc->call[ssid]) == 0)
				return 0; // call retry
			break ;
		case 400 :
			linphonec_display_something(lc, _("Bad request"));
			if (bTransferring)
				eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, 
					"SIP/2.0 400 Bad Request");
			break ;
		case 404 :
			linphonec_display_something(lc, _("User cannot be found at given address."));
			if (bTransferring)
				eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, 
					"SIP/2.0 404 Not Found");
			break ;
		case 415 :
			linphonec_display_something(lc, _("Remote user cannot support any of proposed codecs."));
			if (bTransferring)
				eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, 
					"SIP/2.0 415 Unsupported Media Type");
			break ;
		case 480 :
			tmpmsg = msg480 ;
		case 486 :
			linphonec_display_something(lc, tmpmsg);
			if (bTransferring)
				eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, 
					"SIP/2.0 486 Busy Here");
			break ;
		case 487 :
			linphonec_display_something(lc, msg487);
			if (bTransferring)
				eXosip_transfer_send_notify(lc->call[hs]->did, EXOSIP_SUBCRSTATE_TERMINATED, 
					"SIP/2.0 487 Request Cancelled");
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
			if (lc->call[ssid]->faxflag == FAX_RUN &&	// check fax or modem 
				lc->call[ssid]->state == LCStateFax && 
				lc->call[ssid]->t38_params.initialized)
			{
				eXosip_lock();
				lc->call[ssid]->sdpctx->version++;
				lc->call[ssid]->t38_params.initialized = 0;		//reset the t38 parameters
				eXosip_on_fax_call(lc->call[ssid]->did, 0, lc->call[ssid]->sdpctx->version);	//port 0 indicates g.711
				eXosip_unlock();
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
				rtk_SetPlayTone(lc->chid, ssid == 0 ? 1 : 0, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
				SetSysState(lc->chid, SYS_STATE_CONNECT);
				SetActiveSession(lc->chid, ssid, TRUE);
				lc->call[ssid]->state = LCStateAVRunning;
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

	rtk_SetRingFXS(lc->chid, FALSE);

	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
		rtk_FXO_Busy(lc->chid);
	else
		rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);

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
	sdp_context_t * ctx = NULL ;
	osip_from_t * from_url = NULL ;
	int cid = ev->cid ;
	int did = ev->did ;
	char * from = ev->remote_uri ;
	char * sdp_body = ev->sdp_body ;
	int i;
	uint32 chid = lc->chid;
	char *displayname;
	char *to = ev->local_uri;
	osip_to_t *to_url = NULL;
	voipCfgPortParam_t *voip_ptr = &g_pVoIPCfg->ports[lc->chid];
	char one_stage_dial_number[FW_LEN];
	LinphoneProxyConfig *used_proxy;
	int local_call;

	osip_from_init( &from_url ) ;
	osip_from_parse( from_url , from ) ;

	osip_to_init(&to_url);
	osip_to_parse(to_url, to);

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
    //if ( (chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature)) && (rtk_line_check(lc->chid) != 0) )
    if ( RTK_VOIP_IS_DAA_CH( chid, g_VoIP_Feature) && (rtk_line_check(lc->chid) != 0) )
	{
		g_warning("The line%d is not connected now !!\n", chid);
		eXosip_lock() ;
		eXosip_answer_call( did , 486 , NULL ) ;
		eXosip_unlock() ;
		goto end ;
	}

	/* check busy */
	if (lc->call[ssid] != NULL)
	{
		g_warning("The call[%d] is BUSY now !!\n", chid);
		eXosip_lock() ;
		eXosip_answer_call( did , 486 , NULL ) ;
		eXosip_unlock() ;
		goto end ;
	}

	/* global error */
	if( strlen( sdp_body ) == 0 )
	{
		g_warning("No sdp body [%d]!!\n", chid);
		eXosip_lock() ;
		eXosip_answer_call( did , 603 , NULL ) ;
		eXosip_unlock() ;
		goto end ;
	}

	// check local call
	//	- sip:127.0.0.1
	//		* fxs [use*0] -> pstn
	//	- sip:fxs@127.0.0.1
	//		* pstn ring [no hotline] -> fxs0
	//	- sip:number@127.0.0.1
	//		* use PSTN routing prefix
	local_call = 0;
	if (to_url->url && to_url->url->host &&
		strcmp(to_url->url->host, "127.0.0.1") == 0)
	{
		local_call = 1;
	}

	used_proxy = NULL;
#ifdef MULTI_PROXY
	// check request come from which proxy
	if (to_url->url && to_url->url->host)
	{
		for (i=0; i<MAX_PROXY; i++)
		{
			// TODO: consider DNS ?
			if (lc->proxies[i] &&
				strcmp(to_url->url->host, &lc->proxies[i]->reg_proxy[4]) == 0)
			{
				// record used proxy in call
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

	// check direct ip call
	if (used_proxy == NULL && voip_ptr->direct_ip_call == 0 &&
		!local_call)
	{
		g_warning("reject direct ip call\n");
		eXosip_lock();
		eXosip_answer_call(did, 603, NULL);
		eXosip_unlock();
		goto end;
	}

	// rock: check VoIP to PSTN
	one_stage_dial_number[0] = 0;
	//if (lc->chid >= RTK_VOIP_SLIC_NUM(g_VoIP_Feature))
	if( RTK_VOIP_IS_DAA_CH( lc->chid, g_VoIP_Feature ) )
	{
		// is 1st session ?
		if (GetSysState(chid) != SYS_STATE_IDLE)
		{
			g_warning("call-waiting in DAA!?\n");
		}

		// rock: check Unconditional PSTN forward
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		if (!local_call && voip_ptr->uc_forward_enable == 2)
			strcpy(one_stage_dial_number, voip_ptr->uc_forward);
#else
		if (!local_call && lc->sip_conf.uc_forward_enable == 2)
			strcpy(one_stage_dial_number, lc->sip_conf.uc_forward);
#endif
		// rock: check one stage dial
		// pkshih: add "sip:number@127.0.0.1" to pass PSTN routing trick
		else if (to_url->url && to_url->url->username)
		{
			if (!local_call)
			{
				if (used_proxy)
				{
					if (strcmp(to_url->url->username, voip_ptr->proxies[used_proxy->index].number) != 0)
						one_stage_dial_number[0] = 1; // one stage dial via proxy call
					else
						; // two stage dial via proxy call
				}
				else
				{
					for (i=0; i<MAX_PROXY; i++)
						if (strcmp(to_url->url->username, voip_ptr->proxies[i].number) == 0)
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
					eXosip_answer_call(did, 603, NULL);
					eXosip_unlock();
					goto end;
				}
			}

			if (local_call || one_stage_dial_number[0] == 1)
			{
				strncpy(one_stage_dial_number, to_url->url->username, sizeof(one_stage_dial_number) - 1);
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
			eXosip_answer_call(did, 603, NULL);
			eXosip_unlock();
			goto end;
		}
	}

	lc->call[ssid] = linphone_call_new_incoming(lc, ssid, from, ev->local_uri, cid, did);
	if (lc->call[ssid] == NULL)
	{
		g_warning("%s: create call failed\n", __FUNCTION__);
		eXosip_lock();
		eXosip_answer_call(did, 500, NULL);
		eXosip_unlock();
		goto end ;
	}

#ifdef MULTI_PROXY
	lc->call[ssid]->proxy = used_proxy;
#endif

	caller_id[0] = 0;
	caller_id_name[0] = 0;

	//printf("user_name=%s, disp_name=%s mode=%d\n",
	//	from_url->url->username,from_url->displayname,lc->caller_id_mode&7);

	// username -> caller id
	if (from_url->url && from_url->url->username)
	{
		/* At least 3 digits */
		/* 2 digits may be a control signal in some cases */
		if (strlen(from_url->url->username) >=3)
		{
			strncpy(caller_id, from_url->url->username, sizeof(caller_id) - 1);
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

		// if error use private caller id & name
		if (caller_id[0] == 0)
		{
			strcpy(caller_id, "P");
			strcpy(caller_id_name, "P");
		}
	}
	else if (caller_id_name[0] == 0)
	{
		// if error use private caller id name
		strcpy(caller_id_name, "P");
	}

	//printf("caller_id=%s, caller_id_name=%s\n",caller_id,caller_id_name);

	ctx = lc->call[ssid]->sdpctx ;
	/* get the result of the negociation */
	sdp_context_get_answer( ctx , sdp_body ) ;
	status = sdp_context_get_status( ctx ) ;
	if( GetSessionState( chid , ssid ) == SS_STATE_IDLE )
	{
		// avoid callee use hold function problem
		ctx->offer = sdp_context_generate_template( ctx ) ;
		// set version the same with current anwser
		sprintf(ctx->offer->o_sess_version, "%.10d", ctx->version);
	}

	eXosip_set_call_reference(did, (void *) lc->call[ssid]);

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
			if (ev->rc != NULL)
			{
				SetSysState(chid, SYS_STATE_RING);
				SetSessionState(chid, ssid, SS_STATE_CALLIN);
				linphone_call_set_state(lc->call[ssid], LCStateRinging);
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
				if ( !local_call &&  								// local pstn routing
					 lc->call[ssid]->one_stage_dial_number[0] == 0)	// one stage dial
				{
					eXosip_lock();
					eXosip_answer_call(did, 180, NULL);
					eXosip_unlock();
					lc->ringstream[ssid] = TRUE;
				}

				SetSessionState(chid, ssid, SS_STATE_CALLIN);
				rtk_FXO_RingOn(lc->chid);
				linphone_call_set_state(lc->call[ssid], LCStateRinging);
				break;
			}
			else if (lc->parent->bAutoAnswer)
			{
				SetSysState(chid, SYS_STATE_RING);
				SetSessionState(chid, ssid, SS_STATE_CALLIN);
				linphone_call_set_state(lc->call[ssid], LCStateRinging);
				linphone_core_accept_dialog(lc, ssid, NULL);
				goto end;
			}

			eXosip_lock() ;
			eXosip_answer_call( did , 180 , NULL ) ;
			eXosip_unlock() ;

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
					//rtk_Gen_Dtmf_CID(chid, caller_id);
					rtk_Gen_CID_And_FXS_Ring(chid, 0/*dtmf*/, caller_id, NULL, NULL, 0, 0);
				} 
				else
				{
					//rtk_Gen_FSK_CID( chid, caller_id, NULL, caller_id_name, 0);
					rtk_Gen_CID_And_FXS_Ring(chid, 1/*fsk*/, caller_id, NULL, caller_id_name, 0/*type1*/, 0);
				}
#endif /* !CONFIG_RTK_VOIP_IP_PHONE */
			
#ifdef VMWI_TEST 			
				char vmwi_on = 1, vmwi_off = 0;	
				for(i=0; i<6000000; i++);
				
				/* Set VMWI message on in on_hook mode */
				rtk_Gen_FSK_VMWI( chid, &vmwi_on, 0);
				for(i=0; i<6000000; i++);
				
				/* Set VMWI message off in on_hook mode */
				rtk_Gen_FSK_VMWI( chid, &vmwi_off, 0);
				for(i=0; i<6000000; i++);
				rtk_Gen_FSK_VMWI( chid, &vmwi_off, 0);
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
				    &&((caller_id_mode[chid]&7) != CID_DTMF)  )
					//rtk_Gen_FSK_CID( chid, caller_id, NULL, caller_id_name, 1);
					rtk_Gen_CID_And_FXS_Ring(chid, 1/*fsk*/, caller_id, NULL, caller_id_name, 1/*type2*/, 0);
					
				// wait fsk caller id done, then send call waiting tone
				int32 tmp=-1;
				do
				{
					if(rtk_GetFskCIDState(chid, &tmp) != 0)
					break;
					usleep(50000);  // 50ms
				}
				while (tmp);

#endif /* !CONFIG_RTK_VOIP_IP_PHONE */

				// call waiting tone
				rtk_SetPlayTone( chid , ssid ? 0 : 1, DSPCODEC_TONE_CALL_WAITING , TRUE , DSPCODEC_TONEDIRECTION_LOCAL ) ;

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
				//rtk_SetRingFXS( chid , TRUE ) ;
			}

			linphone_call_set_state(lc->call[ssid], LCStateRinging);

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
			eXosip_answer_call( did , status , NULL ) ;
			eXosip_unlock() ;
#ifdef CONFIG_RTK_VOIP_IP_PHONE
			S2U_DisconnectInd( chid, ssid, 8 );
#endif
			linphone_call_destroy(lc->call[ssid]);
			lc->call[ssid] = NULL;
			break;
	}

end :
	osip_from_free( from_url ) ;
	osip_to_free(to_url);
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
		rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
		rtk_FXO_offhook(lc->chid);
		if (g_pVoIPCfg->ports[lc->chid].offhook_passwd[0])
		{
			char text[] = {IVR_TEXT_ID_PLEASE_ENTER_PASSWORD, '\0'};
			SetSysState(lc->chid, SYS_STATE_AUTH);
			rtk_IvrStartPlaying(lc->chid, IVR_DIR_LOCAL, text);
		}
		else
		{
			char text[] = {IVR_TEXT_ID_PLZ_ENTER_NUMBER, '\0'};
			SetSysState(lc->chid, SYS_STATE_EDIT);
			rtk_IvrStartPlaying(lc->chid, IVR_DIR_LOCAL, text);
		}
		lc->bPlayIVR = 1;
#else
		SetSysState(lc->chid, SYS_STATE_EDIT);
		rtk_FXO_offhook(lc->chid);
		rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_DIAL, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
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
#ifdef SUPPORT_RTP_REDUNDANT
	char buff[ 16 ];
#endif
	int static_pt;

	for( i = 0; i < MAX_SS; i++){
		if( call == lc->call[i] ){
			s_id = i;
			break;
		}
	}

#ifdef SUPPORT_RTP_REDUNDANT
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

			if (strcasecmp(codec->mime_type, "SPEEX") == 0)
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
		#endif
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
+			if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(lc->chid, static_pt))
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

#ifdef SUPPORT_RTP_REDUNDANT
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
#ifdef SUPPORT_RTP_REDUNDANT
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

#ifdef SUPPORT_RTP_REDUNDANT
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
#ifdef SUPPORT_RTP_REDUNDANT
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

#ifdef SUPPORT_RTP_REDUNDANT
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
				
		#ifdef SUPPORT_RTP_REDUNDANT
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
			if (params->static_pt == rtpPayload_SPEEX_NB_RATE8 &&
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
		#ifdef AUTO_RESOURCE
			if (VOIP_RESOURCE_UNAVAILABLE == rtk_VoIP_resource_check(lc->chid, params->static_pt))
			{
				g_warning("no resource for payload %s.\n", payload->a_rtpmap);
				return Unsupported;
			}
		#endif

		#else
			params->pt=payload->pt; /* remember the first payload accepted */
		#endif
			params->remoteaddr=payload->c_addr;
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
#ifdef SUPPORT_RTP_REDUNDANT
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

#ifdef SUPPORT_RTP_REDUNDANT
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
			
		#ifdef SUPPORT_RTP_REDUNDANT
			if( rtp_redundant_case ) {
				// it has to be 0, 8 or 18 
				params->local_pt = params->remote_pt = params->static_pt = payload->pt_red;
			}
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
			params->remoteaddr = payload->c_addr ;
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
		params->remoteaddr = payload->c_addr ;
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

void linphone_call_ringing(LinphoneCore *lc, guint32 ssid, eXosip_event_t *ev)
{
	if (lc->audiostream[ssid])
	{
		g_message( "Already listening to remote host\n" ) ;
		return ;
	}

	linphone_call_proceeding(lc, ssid, ev->cid, ev->did);

	if( strlen(ev->sdp_body) > 0 )
	{
		/*	somebody reported a server who uses a 18x answer to establish mediastreams and play
			an informationnal message */
		linphone_call_accepted(lc, ssid, ev->did, ev->sdp_body);
	}
	else if( ev->status_code == 180 )
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
			rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
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
	case REGSTATUS_REG_FAIL_RETRY_S:		/* register failure (retry in short delay) */
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
	case REGSTATUS_REG_FAIL:				/* register failure (don't retry) */
	case REGSTATUS_REG_FAIL_RETRY_L:		/* register failure (retry in long delay) */
	default:
		buf_status = '0';
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
#ifdef FIX_REGISTER_TIME
	eXosip_reg_t *reg;
	osip_message_t *resp;
	osip_header_t *min_expire_header;
	int min_expire;
#endif
	LinphoneProxyConfig *cfg;
	int i;
	ProxyRegStatus regstatus = REGSTATUS_REG_FAIL;

	cfg = linphone_core_get_proxy_config_from_rid(lc, ev->rid);
	g_return_if_fail(cfg != NULL);

	cfg->registered = FALSE;

	switch(ev->status_code){
		case 401:
		case 407:
			linphone_register_authentication_required(lc,ev);
			regstatus = ( cfg->auth_failed ? REGSTATUS_REG_FAIL : 
										REGSTATUS_REG_FAIL_RETRY_S );	
			break;
#ifdef FIX_REGISTER_TIME
		case 423: // Interval too brief
			reg = eXosip_event_get_reginfo(ev);
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

		case 403:	// Forbidden  
		case 405:	// Method Not Allowed 
			regstatus = REGSTATUS_REG_FAIL_RETRY_L;
			break;
			
		default:
			regstatus = REGSTATUS_REG_FAIL_RETRY_S;
			if (strlen(ev->reason_phrase)==0) { 
				msg=_("Registration failed (timeout).");
#ifdef BYPASS_RELAY
				if( cfg->timeout_count < g_SystemParam.proxy_timeout_count_limit )	// retry 4 times, so total is 5 
					cfg->timeout_count ++;
				else {
					cfg->timeout_count = 0;
					regstatus = REGSTATUS_REG_FAIL_RETRY_L;
				}
				//printf( "cfg->timeout_count ++:(%d)%d\n", cfg->index, cfg->timeout_count );
#endif
			} else 
				msg=ev->reason_phrase;

			linphonec_display_something(lc, msg);
			break;
	}

registration_faillure_display:

	// update and display regitration status 
	linphone_update_and_display_registration_status( lc, cfg, regstatus );

#ifdef MULTI_PROXY
  #if 1	/* pkshih: new default proxy policy */
	
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
			rtk_sip_register(lc->chid, 0);
			linphone_core_set_default_proxy(lc, NULL);
			UpdateLedDisplayMode( lc );
		}
  	}  	
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
		rtk_sip_register(lc->chid, 0);
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
		rtk_sip_register(lc->chid, 0);
#endif
}

void linphone_registration_success( LinphoneCore * lc , eXosip_event_t * ev )
{
	LinphoneProxyConfig * cfg ;
	gchar * msg = g_strdup_printf( _( "Registration on %s sucessful." ) , ev->req_uri ) ;

	linphonec_display_something(lc, msg);
	g_free( msg ) ;

	cfg = linphone_core_get_proxy_config_from_rid( lc , ev->rid ) ;
	g_return_if_fail(cfg != NULL);

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
	
#ifdef MULTI_PROXY
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
	rtk_sip_register(lc->chid, 1);
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
	return ev->nPort;
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

#ifdef ATTENDED_TRANSFER
		if (ev->rc != NULL)
		{
			if (linphone_find_chid_ssid(pApp,ev->rc->c_id , p_chid, p_ssid) == 0)
			{
				//step 1. terminate the 1st call				
				rtk_SetRtpSessionState( *p_chid , *p_ssid , rtp_session_inactive ) ;
				linphone_core_terminate_dialog(&pApp->ports[*p_chid], *p_ssid, NULL);
				//the call might be holded by remote, cancel the holding sound
				rtk_SetPlayTone(*p_chid, *p_ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_BOTH);
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
	if (ev->type == EXOSIP_CALL_NEW)
	{
#ifdef CONFIG_RTK_VOIP_DIALPLAN
		unsigned char szForward[ 100 ] = { 0 };
		const unsigned char *forward = szForward;
#else
		char forward[100] = { 0 };
#endif
		int active_session;

		*p_chid = GetChannelFromRequest(pApp, ev);
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
					eXosip_answer_call(ev->did, 486, NULL);
					eXosip_unlock();
					/*+++++add by Jack for adding busy call log+++++*/
					{
						time_t t;
						LinphoneCallLog *cl=g_new0(LinphoneCallLog,1);

						cl->lc=lc;
						cl->dir=LinphoneCallIncoming;
						cl->status=LinphoneCallMissed;
						time(&t);
						cl->start_date=g_strdup(ctime(&t));
						cl->from=g_strdup(ev->local_uri);
						cl->to=g_strdup(ev->remote_uri);

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
						
						linphone_call_log_changed( cl );
					}
			/*-----end-----*/
					return -1;
				}
			}
		}

		if (forward[0])
		{
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
			eXosip_lock();
			eXosip_set_redirection_address(ev->did, forward);
			eXosip_answer_call(ev->did, 302, NULL);
			eXosip_set_redirection_address(ev->did, NULL);
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
		return -1;
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

	ctx = lc->call[ssid]->sdpctx;
	answer = sdp_context_get_answer(ctx, ev->sdp_body);
	status = sdp_context_get_status(ctx);
	g_message("+++++DEBUG:linphone_core_handle_reinvite:negoc_status%d+++++\n",ctx->negoc_status);

	if(488 == status || 415 == status){
		eXosip_answer_call(ev->did, status, NULL);
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
		rtk_SetRtpSessionState(chid, ssid, rtp_session_inactive);
	}
#endif
#endif

#if 1
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
		sdp_message_to_str(ctx->answer, &answer);
		osip_free(ctx->answerstr);
		ctx->answerstr = answer;
	}
#endif

	if (ev->type == EXOSIP_CALL_UPDATE)
	{
		// update for fax issue or other
		g_message("EXOSIP_CALL_UPDATE[%d][%d]\n", chid, ssid);
		lc->call[ssid]->state = LCStateUpdate;
	}
	else
	{
		if (ev->type == EXOSIP_CALL_HOLD)
		{
			g_message("EXOSIP_CALL_HOLD[%d][%d]: state = %d\n", chid, ssid, lc->call[ssid]->state);
			osip_negotiation_sdp_message_put_media_mode(ctx->answer, MEDIA_RECVONLY);

			if (lc->call[ssid]->state != LCStateBeHold && 
				lc->call[ssid]->state != LCStateBeHoldACK)
			{
				lc->call[ssid]->state = LCStateBeHold;
			}
		}
		else	// EXOSIP_CALL_OFFHOLD
		{
			g_message("EXOSIP_CALL_OFFHOLD[%d][%d]\n", chid, ssid);
			osip_negotiation_sdp_message_put_media_mode(ctx->answer, MEDIA_SENDRECV);

			if (lc->call[ssid]->state == LCStateAVRunning)
			{
				lc->call[ssid]->state = LCStateUpdate;
			}
			else if (lc->call[ssid]->state != LCStateBeResume)
			{
				lc->call[ssid]->state = LCStateBeResume;
			}
		}
			
		if (ctx->answerstr)
		{
			osip_free(ctx->answerstr);
			sdp_message_to_str(ctx->answer, &answer);
			ctx->answerstr = answer;
		}
	}

	eXosip_lock();
	eXosip_answer_call_with_body(ev->did, status, "application/sdp", answer);
	eXosip_unlock();

	eXosip_execute();
}

void linphone_core_process_event(TAPP *pApp, eXosip_event_t * ev )
{
	int chid = -1, ssid = -1;
	LinphoneCore *lc = NULL;
	int bTransferring = 0;

	if (ev->type >= EXOSIP_REGISTRATION_NEW && 
		ev->type <= EXOSIP_REGISTRATION_TERMINATED)
	{
		int i;
		LinphoneProxyConfig *cfg;
	
		for (i=0; i<g_MaxVoIPPorts; i++)
		{
			cfg = linphone_core_get_proxy_config_from_rid(&pApp->ports[i], ev->rid);
			if (cfg)
			{
				if (i != ev->nPort)
				{
					g_message("rid is not match with event!\n");
					continue;
				}
				
				lc = &pApp->ports[i];
				break;
			}
		}

		if (lc == NULL)
		{
			eXosip_event_free(ev);
			return;
		}
	}
	else if ((ev->type >= EXOSIP_CALL_NOANSWER && ev->type <= EXOSIP_CALL_REFER_REQUESTFAILURE)
		|| (ev->type == EXOSIP_INFO_NEW))
	{
		if (linphone_process_call(pApp, &chid, &ssid, ev) < 0)
		{
			// rock: handle BYE-auth after On-Hook
			if (linphone_find_chid_ssid_terminated(pApp, ev->cid, &chid, &ssid) == 0)
			{
				lc = &pApp->ports[chid];
				if ((ev->type == EXOSIP_CALL_REQUESTFAILURE) &&
					(ev->status_code == 401 || ev->status_code == 407))
				{
					if (linphone_call_authentication_required(lc, lc->call_terminated[ssid]) != 0)
						linphone_core_close_terminate_call(lc, ssid);
				}
				else if (ev->type == EXOSIP_CALL_EXIT)
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
	}
#ifdef SIP_PING
	else if (ev->type == EXOSIP_PING_SUCCESS || ev->type == EXOSIP_PING_CLOSED)
	{
		lc = &pApp->ports[ev->nPort];
	}
#endif

	if (lc == NULL)
	{
		g_warning("counldn't find match call\n");
	}

	switch( ev->type )
	{
		case EXOSIP_CALL_ANSWERED :
			g_message( "EXOSIP_CALL_ANSWERED[%d][%d]\n" , chid , ssid ) ;

			if (GetSysState(lc->chid) == SYS_STATE_TRANSFERRING)
				bTransferring = 1;

			// rock add:
			// avoid auto-answer skip ringing problem
			if (lc->ringstream[ssid] == FALSE)
				linphone_call_proceeding(lc, ssid, ev->cid, ev->did);

		#ifdef CHECK_RESOURCE
			if (linphone_call_accepted(lc, ssid, ev->did, ev->sdp_body) == -1)
				break;
		#else
			linphone_call_accepted(lc, ssid, ev->did, ev->sdp_body);
		#endif
/*+++added by Jack for Planet transfer+++*/
#ifdef PLANET_TRANSFER
			if(eXosip.bIVR_transfer){
				if(GetSysState(lc->chid) == SYS_STATE_TRANSFER_EDIT){
					int another_session;
			
					another_session = ssid ? 0 : 1;
					rtk_SetPlayTone(lc->chid , another_session, DSPCODEC_TONE_DIAL, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
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
					eXosip_transfer_send_notify(lc->call[hs]->did, 
						EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 200 OK");
				}
			}
		#endif

			break ;
		case EXOSIP_CALL_CLOSED :
		case EXOSIP_CALL_CANCELLED :
		{
			int another_session;
			SESSIONSTATE another_session_state;

			if( ev->type == EXOSIP_CALL_CLOSED )
				g_message( "EXOSIP_CALL_CLOSED[%d][%d]\n" , chid , ssid ) ;
			else
				g_message( "EXOSIP_CALL_CANCELLED[%d][%d]\n" , chid , ssid ) ;

			linphone_call_terminated(lc, ssid, ev);
			if (GetSysState(chid) == SYS_STATE_IDLE)
			{
				// no answer forward: break here to avoid busy tone 
				break;
			}

			rtk_SetRingFXS( chid , FALSE ) ;	
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
				if (rtk_DAA_off_hook(lc->chid) == 0xff)
				{
					/* PSTN line not connect or line busy */
					SetSysState(lc->chid, SYS_STATE_EDIT_ERROR);
				    rtk_SetPlayTone(lc->chid, 0, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				}
				else
				{
					SetSysState(lc->chid, SYS_STATE_DAA_CONNECT);
					SetSessionState(lc->chid, 0, SS_STATE_CALLER);
				}
			}
			/*++added by Jack Chan  for transfer++*/
			else if (GetSysState(chid) == SYS_STATE_TRANSFERRING)
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
				rtk_SetConference(&stVoipMgr3WayCfg);
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
					rtk_SetPlayTone(chid, another_session, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
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
						rtk_FXO_Busy(chid);
					else
						rtk_SetPlayTone( chid , ssid , DSPCODEC_TONE_BUSY , TRUE , DSPCODEC_TONEDIRECTION_LOCAL ) ;

					SetSysState(chid, SYS_STATE_EDIT_ERROR);
					break;
				case SS_STATE_CALLIN:
					// another session is callin (in call waiting/caller id)
					SetSysState( chid , SYS_STATE_RING ) ;
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
						rtk_SetPlayTone( chid , another_session , DSPCODEC_TONE_RINGING , TRUE , DSPCODEC_TONEDIRECTION_LOCAL ) ;
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
					linphone_core_offhood_event(lc, another_session);
					break;
				}
			}
			break;
		}
		case EXOSIP_CALL_TIMEOUT :
		case EXOSIP_CALL_NOANSWER :
			if( ev->type == EXOSIP_CALL_TIMEOUT )
				g_message( "EXOSIP_CALL_TIMEOUT[%d][%d]\n" , chid , ssid ) ;
			else
				g_message( "EXOSIP_CALL_NOANSWER[%d][%d]\n" , chid , ssid ) ;
			linphone_call_failure(lc, ssid, ev->did, -110);
			break;
		case EXOSIP_CALL_REQUESTFAILURE :
		case EXOSIP_CALL_GLOBALFAILURE :
		case EXOSIP_CALL_SERVERFAILURE :
			if( ev->type == EXOSIP_CALL_REQUESTFAILURE )
				g_message( "EXOSIP_CALL_REQUESTFAILURE[%d][%d]\n" , chid , ssid ) ;
			else if( ev->type == EXOSIP_CALL_GLOBALFAILURE )
				g_message( "EXOSIP_CALL_GLOBALFAILURE[%d][%d]\n" , chid , ssid ) ;
			else
				g_message( "EXOSIP_CALL_SERVERFAILURE[%d][%d]\n" , chid , ssid ) ;
			linphone_call_failure(lc, ssid, ev->did, ev->status_code);
			break ;
		case EXOSIP_CALL_NEW :
			g_message( "EXOSIP_CALL_NEW[%d][%d]\n" , chid , ssid ) ;

			/* Mandy add for stun support */
            linphone_core_set_use_info_for_stun(lc, -1, STUN_UPDATE_SIP); // update RTP in 200 ok

			/* CALL_NEW is used twice in qos mode : 
			 * when you receive invite (textinfo = "With QoS" or "Without QoS")
			 * and when you receive update (textinfo = "New Call") */
			linphone_inc_new_call(lc, ssid, ev);
			break ;
		case EXOSIP_CALL_ACK :
			g_message( "EXOSIP_CALL_ACK[%d][%d]\n" , chid , ssid ) ;
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
					rtk_FXO_offhook(lc->chid);
					// rock: check one stage dialing
					if (GetSysState(lc->chid) == SYS_STATE_RING &&
						lc->call[ssid]->one_stage_dial_number[0])
					{
						rtk_Dial_PSTN_Call_Forward(lc->chid, ssid, lc->call[ssid]->one_stage_dial_number);
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
				lc->call[ssid]->state = LCStateBeHoldACK;
				//rtk_SetRtpSessionState(chid, ssid, rtp_session_recvonly);
				linphone_core_stop_media_streams(lc, ssid);
				if ((GetSysState(chid) != SYS_STATE_IN3WAY) && (GetActiveSession(chid) == ssid))
					rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
				break;
			case LCStateBeResume:
#ifdef CONFIG_RTK_VOIP_IP_PHONE
				S2U_SessionResumeInd( chid, ssid, 1 );
#endif
				lc->call[ssid]->state = LCStateAVRunning;
				//rtk_SetRtpSessionState(chid, ssid, rtp_session_sendrecv);
			#ifdef CHECK_RESOURCE
				if (linphone_core_start_media_streams(lc, ssid, lc->call[ssid]) == -1)
				{
					linphone_core_terminate_dialog(lc, ssid, NULL);
					break;
				}
			#else
				linphone_core_start_media_streams(lc, ssid, lc->call[ssid]);
			#endif
				rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
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
				// reinvite (fax...)
				lc->call[ssid]->state = LCStateAVRunning;
				linphone_core_stop_media_streams(lc, ssid);
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
			g_message( "EXOSIP_CALL_REDIRECTED[%d][%d]\n" , chid , ssid ) ;
			linphone_call_redirected(lc, ssid, ev->did, ev->status_code, ev->remote_contact);
			break ;
		case EXOSIP_CALL_PROCEEDING :
			g_message( "EXOSIP_CALL_PROCEEDING[%d][%d]\n" , chid , ssid ) ;
			linphone_call_proceeding(lc, ssid, ev->cid, ev->did);
			break ;
		case EXOSIP_CALL_RINGING :
			g_message( "EXOSIP_CALL_RINGING[%d][%d]\n" , chid , ssid ) ;
			linphone_call_ringing(lc, ssid, ev);
			break ;
		case EXOSIP_IN_SUBSCRIPTION_NEW :
			g_message( "EXOSIP_IN_SUBSCRIPTION_NEW\n" ) ;
#if 0
			linphone_subscription_new( lc , ev->did , ev->nid , ev->remote_uri , ev->remote_contact ) ;
#endif
			break ;
		case EXOSIP_SUBSCRIPTION_NOTIFY :
			g_message( "EXOSIP_SUBSCRIPTION_NOTIFY\n" ) ;
#if 0
			linphone_notify_recv( lc , ev->remote_uri , ev->online_status ) ;
#endif
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
		case EXOSIP_CALL_STARTAUDIO :
			g_message( "EXOSIP_CALL_STARTAUDIO[%d][%d]\n" , chid , ssid ) ;
			break ;
		case EXOSIP_CALL_RELEASED :
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
					eXosip_transfer_send_notify(lc->call[hs]->did, 
						EXOSIP_SUBCRSTATE_TERMINATED, "SIP/2.0 400 Bad Request");
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
#if 0
			linphone_core_text_received( lc , ev->remote_uri , ev->sdp_body ) ;
#endif
			break ;
		case EXOSIP_CALL_REFERED :
		{
			int si; // idle session

			g_message( "EXOSIP_CALL_REFERED [%d][%d]: refer_to = %s\n", chid, ssid, ev->refer_to ) ;
			si = ssid == 1 ? 0 : 1;
			if (GetSessionState(chid, si) == SS_STATE_IDLE)
			{
				rtk_SetPlayTone(chid, si, DSPCODEC_TONE_HOLD, FALSE, DSPCODEC_TONEDIRECTION_LOCAL);
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
				eXosip_event_free( ev ) ;
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
						rtk_SIP_INFO_play_tone(chid, ssid, DSPCODEC_TONE_STARSIGN, atoi(duration));
					else if (signal[0] == '#')
						rtk_SIP_INFO_play_tone(chid, ssid, DSPCODEC_TONE_HASHSIGN, atoi(duration));
					else if (signal[0] >= '0' && signal[0] <= '9')
						rtk_SIP_INFO_play_tone(chid, ssid, signal[0] - '0' + DSPCODEC_TONE_0, atoi(duration));
				
				#else	/* IO Ctrl play tone without duration */
					if (signal[0] == '*')
						rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_STARSIGN, 
							TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					else if (signal[0] == '#')
						rtk_SetPlayTone(chid, ssid, DSPCODEC_TONE_HASHSIGN, 
							TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
					else if (signal[0] >= '0' && signal[0] <= '9')
						rtk_SetPlayTone(chid, ssid, signal[0] - '0' + DSPCODEC_TONE_0, 
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
						rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_BUSY, TRUE, DSPCODEC_TONEDIRECTION_LOCAL);
						break;
					default:
						if (status_code >= 400 && status_code < 700)
						{
							// Failed Transfer, resume old session
#ifdef CONFIG_RTK_VOIP_IP_PHONE
							S2U_DisconnectInd( lc ->chid, ( ssid ? 0 : 1 ), 14 );

							rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, FALSE,   
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
			
			rtk_SetPlayTone(lc->chid, ssid, DSPCODEC_TONE_RINGING, FALSE,   
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
#ifdef SIP_PING
		case EXOSIP_PING_SUCCESS:
		{
			int i;
			osip_contact_t *contact;

			g_message("EXOSIP_PING_SUCCESS: port = %d, contact = %s\n", 
				ev->nPort, ev->remote_contact);
			osip_contact_init(&contact);
			i = osip_contact_parse(contact, ev->remote_contact);
			if (i != 0)
			{
				g_warning("EXOSIP_PING_SUCCESS: parsing contact failed\n");
				osip_contact_free(contact);
				break;
			}
			rtk_ping_update(ev->nPort, contact->url->host, contact->url->port);
			osip_contact_free(contact);
			break;
		}
		case EXOSIP_PING_CLOSED:
			g_message("EXOSIP_PING_CLOSED: port = %d\n", ev->nPort);
			break;
#endif
		default :
			g_message( "default[%d][%d]\n" , chid , ssid ) ;
			break ;
	}

	eXosip_event_free( ev ) ;
}
